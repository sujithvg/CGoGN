/*******************************************************************************
 * CGoGN: Combinatorial and Geometric modeling with Generic N-dimensional Maps  *
 * version 0.1                                                                  *
 * Copyright (C) 2009-2012, IGG Team, LSIIT, University of Strasbourg           *
 *                                                                              *
 * This library is free software; you can redistribute it and/or modify it      *
 * under the terms of the GNU Lesser General Public License as published by the *
 * Free Software Foundation; either version 2.1 of the License, or (at your     *
 * option) any later version.                                                   *
 *                                                                              *
 * This library is distributed in the hope that it will be useful, but WITHOUT  *
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License  *
 * for more details.                                                            *
 *                                                                              *
 * You should have received a copy of the GNU Lesser General Public License     *
 * along with this library; if not, write to the Free Software Foundation,      *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.           *
 *                                                                              *
 * Web site: http://cgogn.unistra.fr/                                           *
 * Contact information: cgogn@unistra.fr                                        *
 *                                                                              *
 *******************************************************************************/

#include "Topology/generic/traversor/traversorCell.h"
#include "Algo/Geometry/centroid.h"
#include "Algo/Modelisation/tetrahedralization.h"

namespace CGoGN
{

namespace Algo
{

namespace Geometry
{

template <typename PFP>
typename PFP::REAL tetrahedronSignedVolume(typename PFP::MAP& map, Vol v, const VertexAttribute<typename PFP::VEC3, typename PFP::MAP>& position)
{
	typedef typename PFP::VEC3 VEC3;

	VEC3 p1 = position[v.dart] ;
	VEC3 p2 = position[map.phi1(v)] ;
	VEC3 p3 = position[map.phi_1(v)] ;
	VEC3 p4 = position[map.phi_1(map.phi2(v))] ;

	return Geom::tetraSignedVolume(p1, p2, p3, p4) ;
}

template <typename PFP>
typename PFP::REAL tetrahedronVolume(typename PFP::MAP& map, Vol v, const VertexAttribute<typename PFP::VEC3, typename PFP::MAP>& position)
{
	typedef typename PFP::VEC3 VEC3;

	VEC3 p1 = position[v.dart] ;
	VEC3 p2 = position[map.phi1(v)] ;
	VEC3 p3 = position[map.phi_1(v)] ;
	VEC3 p4 = position[map.phi_1(map.phi2(v))] ;

	return Geom::tetraVolume(p1, p2, p3, p4) ;
}

template <typename PFP>
typename PFP::REAL convexPolyhedronVolume(typename PFP::MAP& map, Vol v, const VertexAttribute<typename PFP::VEC3, typename PFP::MAP>& position)
{
	typedef typename PFP::MAP MAP;
	typedef typename PFP::VEC3 VEC3;

	if (Volume::Modelisation::Tetrahedralization::isTetrahedron<PFP>(map, v))
		return tetrahedronVolume<PFP>(map, v, position) ;
	else
	{
		typename PFP::REAL vol = 0 ;
		VEC3 vCentroid = Algo::Surface::Geometry::volumeCentroid<PFP>(map, v, position) ;

		DartMarkerStore<MAP> mark(map) ;	// Lock a marker

		std::vector<Face> visitedFaces ;
		visitedFaces.reserve(100) ;

		Face f(v.dart) ;
		visitedFaces.push_back(f) ;
		mark.markOrbit(f) ;

		for(unsigned int iface = 0; iface != visitedFaces.size(); ++iface)
		{
			f = visitedFaces[iface] ;
			if(map.isCycleTriangle(f))
			{
				VEC3 p1 = position[f.dart] ;
				VEC3 p2 = position[map.phi1(f)] ;
				VEC3 p3 = position[map.phi_1(f)] ;
				vol += Geom::tetraVolume(p1, p2, p3, vCentroid) ;
			}
			else
			{
				VEC3 fCentroid = Algo::Surface::Geometry::faceCentroid<PFP>(map, f, position) ;
				Dart d = f.dart ;
				do
				{
					VEC3 p1 = position[d] ;
					VEC3 p2 = position[map.phi1(d)] ;
					vol += Geom::tetraVolume(p1, p2, fCentroid, vCentroid) ;
					d = map.phi1(d) ;
				} while(d != f.dart) ;
			}
			Dart d = f.dart;
			do	// add all face neighbours to the table
			{
				Dart dd = map.phi2(d) ;
				if(!mark.isMarked(dd)) // not already marked
				{
					Face ff(dd);
					visitedFaces.push_back(ff) ;
					mark.markOrbit(ff) ;
				}
				d = map.phi1(d) ;
			} while(d != f.dart) ;
		}

		return vol ;
	}
}

template <typename PFP>
typename PFP::REAL totalVolume(typename PFP::MAP& map, const VertexAttribute<typename PFP::VEC3, typename PFP::MAP>& position)
{
	if (CGoGN::Parallel::NumberOfThreads > 1)
	{
		return Parallel::totalVolume<PFP>(map, position);
	}

	double vol = 0.0 ;

	TraversorW<typename PFP::MAP> t(map) ;
	for(Dart d = t.begin(); d != t.end(); d = t.next())
		vol += convexPolyhedronVolume<PFP>(map, d, position) ;
	return typename PFP::REAL(vol) ;
}


namespace Parallel
{

template <typename PFP>
typename PFP::REAL totalVolume(typename PFP::MAP& map, const VertexAttribute<typename PFP::VEC3, typename PFP::MAP>& position)
{
	// allocate a vector of 1 accumulator for each thread
	std::vector<typename PFP::REAL> vols(CGoGN::Parallel::NumberOfThreads-1, 0.0);

	// foreach volume
	CGoGN::Parallel::foreach_cell<VOLUME>(map, [&] (Vol v, unsigned int thr)
	{
		// add volume to the thread accumulator
		vols[thr-1] += convexPolyhedronVolume<PFP>(map, v, position) ;
	});

	// compute the sum of volumes
	typename PFP::REAL total(0);
	for (int i=0; i< CGoGN::Parallel::NumberOfThreads-1; ++i )
		total += vols[i];

	return total;
}

} // namespace Parallel


} // namespace Geometry

} // namespace Algo

} // namespace CGoGN

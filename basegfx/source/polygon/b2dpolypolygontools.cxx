/*************************************************************************
 *
 *  $RCSfile: b2dpolypolygontools.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2003-11-06 16:30:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGONCUTTER_HXX
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace polygon
    {
        namespace tools
        {
            // B2DPolyPolygon tools

            void correctOrientations(::basegfx::polygon::B2DPolyPolygon& rCandidate)
            {
                const sal_uInt32 nPolygonCount(rCandidate.count());
                sal_uInt32 nIndexOfOutmostPolygon(0L);
                sal_Bool bIndexOfOutmostPolygonSet(sal_False);

                for(sal_uInt32 a(0L); a < nPolygonCount; a++)
                {
                    ::basegfx::polygon::B2DPolygon aCandidate = rCandidate.getPolygon(a);

                    if(aCandidate.count() > 2L)
                    {
                        ::basegfx::vector::B2DVectorOrientation aOrientation =
                            ::basegfx::polygon::tools::getOrientation(aCandidate);
                        sal_Bool bDoFlip(::basegfx::vector::ORIENTATION_POSITIVE != aOrientation);

                        // init values for depth and compare point for
                        // inside test. Since the ordering makes only sense when assuming
                        // that there are no intersections, the inside test is done with
                        // any point of the candidate, so teke the first one.
                        sal_uInt32 nDepth(0L);
                        const ::basegfx::point::B2DPoint aTestPoint(aCandidate.getB2DPoint(0L));

                        // loop over other polygons and calculate depth
                        for(sal_uInt32 b(0L); b < nPolygonCount; b++)
                        {
                            if(b != a)
                            {
                                ::basegfx::polygon::B2DPolygon aComparePolygon = rCandidate.getPolygon(b);

                                if(::basegfx::polygon::tools::isInside(aComparePolygon, aTestPoint))
                                {
                                    nDepth++;
                                }
                            }
                        }

                        // if nDepth is odd it is a hole
                        sal_Bool bIsHole(1L == (nDepth & 0x00000001));

                        // does polygon need to be flipped?
                        if((bDoFlip && !bIsHole) || (!bDoFlip && bIsHole))
                        {
                            aCandidate.flip();

                            // write back changed polygon
                            rCandidate.setPolygon(a, aCandidate);
                        }

                        // remember the index if it's the outmost polygon
                        if(!bIndexOfOutmostPolygonSet && 0L == nDepth)
                        {
                            bIndexOfOutmostPolygonSet = sal_True;
                            nIndexOfOutmostPolygon = a;
                        }
                    }
                }

                // if the outmost polygon is not the first, move it in front
                if(bIndexOfOutmostPolygonSet && nIndexOfOutmostPolygon > 0L)
                {
                    ::basegfx::polygon::B2DPolygon aOutmostPolygon = rCandidate.getPolygon(nIndexOfOutmostPolygon);
                    rCandidate.remove(nIndexOfOutmostPolygon);
                    rCandidate.insert(0L, aOutmostPolygon);
                }
            }

            void removeIntersections(::basegfx::polygon::B2DPolyPolygon& rCandidate,
                sal_Bool bForceOrientation, sal_Bool bInvertRemove)
            {
                ::basegfx::polygon::B2DPolyPolygonCutter aCutter;

                aCutter.addPolyPolygon(rCandidate, bForceOrientation);
                aCutter.removeSelfIntersections();
                aCutter.removeDoubleIntersections();
                aCutter.removeIncludedPolygons(!bInvertRemove);
                rCandidate.clear();
                aCutter.getPolyPolygon(rCandidate);
            }
        } // end of namespace tools
    } // end of namespace polygon
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

// eof

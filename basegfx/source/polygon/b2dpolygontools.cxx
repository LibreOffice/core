/*************************************************************************
 *
 *  $RCSfile: b2dpolygontools.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2003-10-31 10:13:58 $
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

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_CURVE_B2DCUBICBEZIER_HXX
#include <basegfx/curve/b2dcubicbezier.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace polygon
    {
        namespace tools
        {
            // B2DPolygon tools
            void checkClosed(polygon::B2DPolygon& rCandidate)
            {
                while(rCandidate.count() > 1L)
                {
                    bool bFirstLastPointEqual(
                        rCandidate.getB2DPoint(0L) == rCandidate.getB2DPoint(rCandidate.count() - 1L));

                    if(bFirstLastPointEqual)
                    {
                        rCandidate.setClosed(true);
                        rCandidate.remove(rCandidate.count() - 1L);
                    }
                }
            }

            // Checks if one of the control vectors is used
            bool isEdgeBezier(const polygon::B2DPolygon& rPolygon, sal_uInt32 nEdgeIndex)
            {
                if(rPolygon.areControlPointsUsed())
                {
                    DBG_ASSERT(nEdgeIndex < rPolygon.count(), "EdgeIndex out of range (!)");

                    if(!rPolygon.getControlPointA(nEdgeIndex).equalZero())
                        return true;

                    if(!rPolygon.getControlPointB(nEdgeIndex).equalZero())
                        return true;
                }

                return false;
            }

            bool isEdgeTrivialBezier(const polygon::B2DPolygon& rPolygon, sal_uInt32 nEdgeIndex)
            {
                if(rPolygon.areControlPointsUsed())
                {
                    DBG_ASSERT(nEdgeIndex < rPolygon.count(), "EdgeIndex out of range (!)");
                    const sal_uInt32 nEndIndex((nEdgeIndex + 1L) % rPolygon.count());

                    curve::B2DCubicBezier aCubicBezier(
                        rPolygon.getB2DPoint(nEdgeIndex),
                        rPolygon.getControlPointA(nEdgeIndex),
                        rPolygon.getControlPointB(nEdgeIndex),
                        rPolygon.getB2DPoint(nEndIndex));

                    aCubicBezier.testAndSolveTrivialBezier();

                    return !aCubicBezier.isBezier();
                }

                return true;
            }
        } // end of namespace tools
    } // end of namespace polygon
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

// eof

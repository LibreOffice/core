/*************************************************************************
 *
 *  $RCSfile: b2dcubicbezier.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2003-10-31 10:13:54 $
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

#ifndef _BGFX_CURVE_B2DCUBICBEZIER_HXX
#include <basegfx/curve/b2dcubicbezier.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace curve
    {
        B2DCubicBezier::B2DCubicBezier(const B2DCubicBezier& rBezier)
        :   maStartPoint(rBezier.maStartPoint),
            maEndPoint(rBezier.maEndPoint),
            maControlPointA(rBezier.maControlPointA),
            maControlPointB(rBezier.maControlPointB)
        {
        }

        B2DCubicBezier::B2DCubicBezier()
        {
        }

        B2DCubicBezier::B2DCubicBezier(const point::B2DPoint& rStart, const point::B2DPoint& rEnd)
        :   maStartPoint(rStart),
            maEndPoint(rEnd),
            maControlPointA(rStart),
            maControlPointB(rEnd)
        {
        }

        B2DCubicBezier::B2DCubicBezier(const point::B2DPoint& rStart, const point::B2DPoint& rControlPointA,
            const point::B2DPoint& rControlPointB, const point::B2DPoint& rEnd)
        :   maStartPoint(rStart),
            maEndPoint(rEnd),
            maControlPointA(rControlPointA),
            maControlPointB(rControlPointB)
        {
        }

        B2DCubicBezier::~B2DCubicBezier()
        {
        }

        // assignment operator
        B2DCubicBezier& B2DCubicBezier::operator=(const B2DCubicBezier& rBezier)
        {
            maStartPoint = rBezier.maStartPoint;
            maEndPoint = rBezier.maEndPoint;
            maControlPointA = rBezier.maControlPointA;
            maControlPointB = rBezier.maControlPointB;

            return *this;
        }

        // compare operators
        bool B2DCubicBezier::operator==(const B2DCubicBezier& rBezier) const
        {
            return (
                maStartPoint == rBezier.maStartPoint
                && maEndPoint == rBezier.maEndPoint
                && maControlPointA == rBezier.maControlPointA
                && maControlPointB == rBezier.maControlPointB
            );
        }

        bool B2DCubicBezier::operator!=(const B2DCubicBezier& rBezier) const
        {
            return (
                maStartPoint != rBezier.maStartPoint
                || maEndPoint != rBezier.maEndPoint
                || maControlPointA != rBezier.maControlPointA
                || maControlPointB != rBezier.maControlPointB
            );
        }

        // test if vectors are used
        bool B2DCubicBezier::isBezier() const
        {
            if(maControlPointA != maStartPoint || maControlPointB != maEndPoint)
            {
                return true;
            }

            return false;
        }

        void B2DCubicBezier::testAndSolveTrivialBezier()
        {
            // TODO
        }

    } // end of namespace curve
} // end of namespace basegfx

// eof

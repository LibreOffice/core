/*************************************************************************
 *
 *  $RCSfile: b2dquadraticbezier.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2003-11-05 12:25:48 $
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

#ifndef _BGFX_CURVE_B2DQUADRATICBEZIER_HXX
#include <basegfx/curve/b2dquadraticbezier.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#include <basegfx/numeric/ftools.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace curve
    {
        B2DQuadraticBezier::B2DQuadraticBezier(const B2DQuadraticBezier& rBezier)
        :   maStartPoint(rBezier.maStartPoint),
            maEndPoint(rBezier.maEndPoint),
            maControlPoint(rBezier.maControlPoint)
        {
        }

        B2DQuadraticBezier::B2DQuadraticBezier()
        {
        }

        B2DQuadraticBezier::B2DQuadraticBezier(const point::B2DPoint& rStart, const point::B2DPoint& rEnd)
        :   maStartPoint(rStart),
            maEndPoint(rEnd)
        {
        }

        B2DQuadraticBezier::B2DQuadraticBezier(const point::B2DPoint& rStart, const point::B2DPoint& rControl, const point::B2DPoint& rEnd)
        :   maStartPoint(rStart),
            maEndPoint(rEnd),
            maControlPoint(rControl)
        {
        }

        B2DQuadraticBezier::~B2DQuadraticBezier()
        {
        }

        // assignment operator
        B2DQuadraticBezier& B2DQuadraticBezier::operator=(const B2DQuadraticBezier& rBezier)
        {
            maStartPoint = rBezier.maStartPoint;
            maEndPoint = rBezier.maEndPoint;
            maControlPoint = rBezier.maControlPoint;

            return *this;
        }

        // compare operators
        sal_Bool B2DQuadraticBezier::operator==(const B2DQuadraticBezier& rBezier) const
        {
            return (
                maStartPoint == rBezier.maStartPoint
                && maEndPoint == rBezier.maEndPoint
                && maControlPoint == rBezier.maControlPoint
            );
        }

        sal_Bool B2DQuadraticBezier::operator!=(const B2DQuadraticBezier& rBezier) const
        {
            return (
                maStartPoint != rBezier.maStartPoint
                || maEndPoint != rBezier.maEndPoint
                || maControlPoint != rBezier.maControlPoint
            );
        }

        // test if control vector is used
        sal_Bool B2DQuadraticBezier::isBezier() const
        {
            // if control vector is empty, bezier is not used
            if(maControlPoint == maStartPoint || maControlPoint == maEndPoint)
                return sal_False;

            return sal_True;
        }
    } // end of namespace curve
} // end of namespace basegfx

// eof

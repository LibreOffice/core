/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Fridrich Strba  <fridrich.strba@bluewin.ch>
 *       Thorsten Behrens <tbehrens@novell.com>
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#ifndef _BASEGFX_B2DELLIPSE_HXX
#define _BASEGFX_B2DELLIPSE_HXX

#include <sal/types.h>

#include <o3tl/cow_wrapper.hxx>

#include <basegfx/point/b2dpoint.hxx>

#include <basegfx/tuple/b2dtuple.hxx>

#include <basegfx/vector/b2enums.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations
class ImplB2DEllipse;

namespace basegfx
{
    class B2DPoint;
    class B2DVector;
    class B2DHomMatrix;
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class B2DEllipse
    {
    private:
     basegfx::B2DPoint maCenter;
     basegfx::B2DTuple maRadius;

    public:
        B2DEllipse();
        B2DEllipse(const B2DEllipse& rEllipse);
        B2DEllipse(const basegfx::B2DPoint& rCenter, const basegfx::B2DTuple& rRadius);
        ~B2DEllipse();

        // assignment operator
        B2DEllipse& operator=(const B2DEllipse& rEllipse);

        // compare operators
        bool operator==(const B2DEllipse& rEllipse) const;
        bool operator!=(const B2DEllipse& rEllipse) const;

        // Coordinate interface
        basegfx::B2DPoint getB2DEllipseCenter() const;
        void setB2DEllipseCenter(const basegfx::B2DPoint& rCenter);

        basegfx::B2DTuple getB2DEllipseRadius() const;
        void setB2DEllipseRadius(const basegfx::B2DTuple& rRadius);

        // apply transformation given in matrix form to the Ellipse
        void transform(const basegfx::B2DHomMatrix& rMatrix);
    };
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

#endif /* _BASEGFX_B2DELLIPSE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

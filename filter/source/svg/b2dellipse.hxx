/*************************************************************************
 *
 *    OpenOffice.org - a multi-platform office productivity suite
 *
 *    Author:
 *      Fridrich Strba  <fridrich.strba@bluewin.ch>
 *      Thorsten Behrens <tbehrens@novell.com>
 *
 *      Copyright (C) 2008, Novell Inc.
 *
 *   The Contents of this file are made available subject to
 *   the terms of GNU Lesser General Public License Version 3.
 *
 ************************************************************************/

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

/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: spiralwipe.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:59:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#if ! defined INCLUDED_SLIDESHOW_SPIRALWIPE_HXX
#define INCLUDED_SLIDESHOW_SPIRALWIPE_HXX

#include "parametricpolypolygon.hxx"
#include "basegfx/numeric/ftools.hxx"
#include "basegfx/point/b2dpoint.hxx"


namespace presentation {
namespace internal {

/// Generates a topLeftClockWise or
/// bottomLeftCounterClockWise (flipOnYAxis=true) spiral wipe:
class SpiralWipe : public ParametricPolyPolygon
{
public:
    SpiralWipe( sal_Int32 nElements, bool flipOnYAxis = false );
    virtual ::basegfx::B2DPolyPolygon operator () ( double t );
protected:
    ::basegfx::B2DPolyPolygon calcNegSpiral( double t ) const;

    const sal_Int32 m_elements;
    const sal_Int32 m_sqrtElements;
    const bool m_flipOnYAxis;
};

/// Generates a twoBoxLeft or fourBoxHorizontal wipe:
class BoxSnakesWipe : public SpiralWipe
{
public:
    BoxSnakesWipe( sal_Int32 nElements, bool fourBox = false )
        : SpiralWipe(nElements), m_fourBox(fourBox) {}
    virtual ::basegfx::B2DPolyPolygon operator () ( double t );
private:
    const bool m_fourBox;
};

}
}

#endif /* INCLUDED_SLIDESHOW_SPIRALWIPE_HXX */

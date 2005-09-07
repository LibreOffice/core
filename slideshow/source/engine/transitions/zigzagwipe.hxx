/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: zigzagwipe.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:02:45 $
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

#if ! defined INCLUDED_SLIDESHOW_ZIGZAGWIPE_HXX
#define INCLUDED_SLIDESHOW_ZIGZAGWIPE_HXX

#include "parametricpolypolygon.hxx"
#include "basegfx/polygon/b2dpolygon.hxx"


namespace presentation {
namespace internal {

/// Generates a left to right zigZag wipe:
class ZigZagWipe : public ParametricPolyPolygon
{
public:
    ZigZagWipe( sal_Int32 nZigs );
    virtual ::basegfx::B2DPolyPolygon operator () ( double t );
protected:
    const double m_zigEdge;
    ::basegfx::B2DPolygon m_stdZigZag;
};

/// Generates a vertical barnZigZag wipe:
class BarnZigZagWipe : public ZigZagWipe
{
public:
    BarnZigZagWipe( sal_Int32 nZigs ) : ZigZagWipe(nZigs) {}
    virtual ::basegfx::B2DPolyPolygon operator () ( double t );
};

}
}

#endif /* INCLUDED_SLIDESHOW_ZIGZAGWIPE_HXX */

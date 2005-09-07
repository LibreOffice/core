/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: figurewipe.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:53:20 $
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

#if ! defined INCLUDED_SLIDESHOW_FIGUREWIPE_HXX
#define INCLUDED_SLIDESHOW_FIGUREWIPE_HXX

#include "parametricpolypolygon.hxx"


namespace presentation {
namespace internal {

class FigureWipe : public ParametricPolyPolygon
{
public:
    static FigureWipe * createTriangleWipe();
    static FigureWipe * createArrowHeadWipe();
    static FigureWipe * createStarWipe( sal_Int32 nPoints );
    static FigureWipe * createPentagonWipe();
    static FigureWipe * createHexagonWipe();

    virtual ::basegfx::B2DPolyPolygon operator () ( double t );
private:
    FigureWipe( ::basegfx::B2DPolygon const & figure ) : m_figure(figure) {}
    const ::basegfx::B2DPolygon m_figure;
};

}
}

#endif /* INCLUDED_SLIDESHOW_FIGUREWIPE_HXX */

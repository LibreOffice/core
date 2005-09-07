/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: waterfallwipe.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:02:12 $
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

#if ! defined INCLUDED_SLIDESHOW_WATERFALLWIPE_HXX
#define INCLUDED_SLIDESHOW_WATERFALLWIPE_HXX

#include "parametricpolypolygon.hxx"
#include "basegfx/polygon/b2dpolygon.hxx"


namespace presentation {
namespace internal {

/// Generate a vertical left waterfall wipe
class WaterfallWipe : public ParametricPolyPolygon
{
public:
    WaterfallWipe( sal_Int32 nElements, bool flipOnYAxis = false );
    virtual ::basegfx::B2DPolyPolygon operator () ( double t );
private:
    bool m_flipOnYAxis;
    ::basegfx::B2DPolygon m_waterfall;
};

}
}

#endif /* INCLUDED_SLIDESHOW_WATERFALLWIPE_HXX */

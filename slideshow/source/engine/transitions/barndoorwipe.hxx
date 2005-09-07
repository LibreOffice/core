/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: barndoorwipe.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:47:42 $
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

#if ! defined INCLUDED_SLIDESHOW_BARNDOORWIPE_HXX
#define INCLUDED_SLIDESHOW_BARNDOORWIPE_HXX

#include "parametricpolypolygon.hxx"
#include "transitiontools.hxx"
#include "basegfx/polygon/b2dpolygon.hxx"


namespace presentation {
namespace internal {

/// Generate a barn door wipe or double barn door wipe:
class BarnDoorWipe : public ParametricPolyPolygon
{
public:
    BarnDoorWipe( bool doubled = false )
        : m_unitRect( createUnitRect() ), m_doubled(doubled) {}
    virtual ::basegfx::B2DPolyPolygon operator()( double x );
private:
    const ::basegfx::B2DPolygon m_unitRect;
    const bool m_doubled;
};

}
}

#endif /* INCLUDED_SLIDESHOW_BARNDOORWIPE_HXX */

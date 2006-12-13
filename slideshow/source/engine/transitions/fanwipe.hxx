/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fanwipe.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 15:41:12 $
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

#if ! defined INCLUDED_SLIDESHOW_FANWIPE_HXX
#define INCLUDED_SLIDESHOW_FANWIPE_HXX

#include "parametricpolypolygon.hxx"


namespace slideshow {
namespace internal {

/// Generates a centerTop (center=true) or double fan wipe:
class FanWipe : public ParametricPolyPolygon
{
public:
    FanWipe( bool center, bool single = true, bool fanIn = false )
        : m_center(center), m_single(single), m_fanIn(fanIn) {}
    virtual ::basegfx::B2DPolyPolygon operator () ( double t );
private:
    bool m_center, m_single, m_fanIn;
};

}
}

#endif /* INCLUDED_SLIDESHOW_FANWIPE_HXX */

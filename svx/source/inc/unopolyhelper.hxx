/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unopolyhelper.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 13:28:12 $
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

#ifndef _SVX_UNOPOLYHELPER_HXX
#define _SVX_UNOPOLYHELPER_HXX

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

namespace com { namespace sun { namespace star { namespace drawing {
    struct PolyPolygonBezierCoords;
} } } }

namespace basegfx {
    class B2DPolyPolygon;
}

/** convert a drawing::PolyPolygonBezierCoords to a B2DPolyPolygon
*/
basegfx::B2DPolyPolygon SvxConvertPolyPolygonBezierToB2DPolyPolygon( const com::sun::star::drawing::PolyPolygonBezierCoords* pSourcePolyPolygon)
    throw( com::sun::star::lang::IllegalArgumentException );

/** convert a B2DPolyPolygon to a drawing::PolyPolygonBezierCoords
*/
void SvxConvertB2DPolyPolygonToPolyPolygonBezier( const basegfx::B2DPolyPolygon& rPolyPoly, com::sun::star::drawing::PolyPolygonBezierCoords& rRetval );


#endif



/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unopolyhelper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:28:59 $
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

class XPolygon;
class XPolyPolygon;

/** convert a drawing::PolyPolygonBezierCoords to a XPolygon
*/
void SvxConvertPolyPolygonBezierToXPolygon( const com::sun::star::drawing::PolyPolygonBezierCoords* pSourcePolyPolygon, XPolygon& rNewPolygon )
    throw( com::sun::star::lang::IllegalArgumentException );

/** convert a drawing::PolyPolygonBezierCoords to a XPolyPolygon
*/
void SvxConvertPolyPolygonBezierToXPolyPolygon( const com::sun::star::drawing::PolyPolygonBezierCoords* pSourcePolyPolygon, XPolyPolygon& rNewPolygon )
    throw( com::sun::star::lang::IllegalArgumentException );

/** convert a XPolygon to a drawing::PolyPolygonBezierCoords
*/
void SvxConvertXPolygonToPolyPolygonBezier( const XPolygon& rPolygon, com::sun::star::drawing::PolyPolygonBezierCoords& rRetval )
    throw();

void SvxPolyPolygonToPolyPolygonBezierCoords( const XPolyPolygon& rPolyPoly, com::sun::star::drawing::PolyPolygonBezierCoords& rRetval );


#endif



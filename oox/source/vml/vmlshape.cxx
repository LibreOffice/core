/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vmlshape.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:06:07 $
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

#include "oox/vml/shape.hxx"

namespace oox { namespace vml {

Shape::Shape()
: mnShapeType( 0 )
, mnCoordWidth( 0 )
, mnCoordHeight( 0 )
, mnStroked( 0 )
, mnFilled( 0 )
{
}
Shape::~Shape()
{
}

void Shape::applyAttributes( const vml::Shape& rSource )
{
    if ( rSource.msId.getLength() )
        msId = rSource.msId;
    if ( rSource.msType.getLength() )
        msType = rSource.msType;
    if ( rSource.mnShapeType )
        mnShapeType = rSource.mnShapeType;
    if ( rSource.mnCoordWidth )
        mnCoordWidth = rSource.mnCoordWidth;
    if ( rSource.mnCoordHeight )
        mnCoordHeight = rSource.mnCoordHeight;
    if ( rSource.mnStroked )
        mnStroked = rSource.mnStroked;
    if ( rSource.mnFilled )
        mnFilled = rSource.mnFilled;
    if ( rSource.msPath.getLength() )
        msPath = rSource.msPath;
}

} }

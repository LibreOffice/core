/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vmlshape.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:55:39 $
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

#include <rtl/ustring.hxx>
#include "oox/vml/shape.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
using namespace com::sun::star;

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

void Shape::addShape( const ::oox::core::XmlFilterBase& rFilterBase,
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes )
{
    ::rtl::OUString rServiceName( rtl::OUString::createFromAscii( "com.sun.star.drawing.CustomShape" ) );
    uno::Reference< lang::XMultiServiceFactory > xServiceFact( rFilterBase.getModel(), uno::UNO_QUERY_THROW );
    uno::Reference< drawing::XShape > xShape( xServiceFact->createInstance( rServiceName ), uno::UNO_QUERY_THROW );
    rxShapes->add( xShape );

    static const ::rtl::OUString sShapePresetType(RTL_CONSTASCII_USTRINGPARAM("smiley"));

    uno::Reference< drawing::XEnhancedCustomShapeDefaulter > xDefaulter( xShape, uno::UNO_QUERY );
    if( xDefaulter.is() )
        xDefaulter->createCustomShapeDefaults( sShapePresetType );

    uno::Reference< beans::XPropertySet > xPropSet( xShape, uno::UNO_QUERY_THROW );
    static const rtl::OUString sWidth(RTL_CONSTASCII_USTRINGPARAM("Width"));
    static const rtl::OUString sHeight(RTL_CONSTASCII_USTRINGPARAM("Height"));
    xPropSet->setPropertyValue( sWidth, uno::Any( (sal_Int32)5000 ) );
    xPropSet->setPropertyValue( sHeight, uno::Any( (sal_Int32)5000 ) );
}

} }

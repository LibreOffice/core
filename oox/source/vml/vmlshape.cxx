/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vmlshape.cxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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

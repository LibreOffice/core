/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "oox/helper/graphichelper.hxx"
#include <com/sun/star/graphic/GraphicObject.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <comphelper/componentcontext.hxx>
#include <comphelper/seqstream.hxx>

using ::rtl::OUString;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::io::XInputStream;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::graphic::GraphicObject;
using ::com::sun::star::graphic::XGraphic;
using ::com::sun::star::graphic::XGraphicObject;
using ::com::sun::star::graphic::XGraphicProvider;

namespace oox {

// ============================================================================

GraphicHelper::GraphicHelper( const Reference< XMultiServiceFactory >& rxFactory ) :
    mxGraphicProvider( rxFactory->createInstance( CREATE_OUSTRING( "com.sun.star.graphic.GraphicProvider" ) ), UNO_QUERY ),
    maGraphicObjScheme( CREATE_OUSTRING( "vnd.sun.star.GraphicObject:" ) )
{
    ::comphelper::ComponentContext aContext( rxFactory );
    mxCompContext = aContext.getUNOContext();
}

GraphicHelper::~GraphicHelper()
{
}

Reference< XGraphic > GraphicHelper::importGraphic( const Reference< XInputStream >& rxInStrm )
{
    Reference< XGraphic > xGraphic;
    if( rxInStrm.is() && mxGraphicProvider.is() ) try
    {
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[ 0 ].Name = CREATE_OUSTRING( "InputStream" );
        aArgs[ 0 ].Value <<= rxInStrm;
        xGraphic = mxGraphicProvider->queryGraphic( aArgs );
    }
    catch( Exception& )
    {
    }
    return xGraphic;
}

Reference< XGraphic > GraphicHelper::importGraphic( const StreamDataSequence& rGraphicData )
{
    Reference< XGraphic > xGraphic;
    if( rGraphicData.hasElements() )
    {
        Reference< XInputStream > xInStrm( new ::comphelper::SequenceInputStream( rGraphicData ) );
        xGraphic = importGraphic( xInStrm );
    }
    return xGraphic;
}

OUString GraphicHelper::createGraphicObject( const Reference< XGraphic >& rxGraphic )
{
    OUString aGraphicObjUrl;
    if( mxCompContext.is() && rxGraphic.is() ) try
    {
        Reference< XGraphicObject > xGraphicObj( GraphicObject::create( mxCompContext ), UNO_SET_THROW );
        xGraphicObj->setGraphic( rxGraphic );
        maGraphicObjects.push_back( xGraphicObj );
        aGraphicObjUrl = maGraphicObjScheme + xGraphicObj->getUniqueID();
    }
    catch( Exception& )
    {
    }
    return aGraphicObjUrl;
}

OUString GraphicHelper::importGraphicObject( const Reference< XInputStream >& rxInStrm )
{
    return createGraphicObject( importGraphic( rxInStrm ) );
}

OUString GraphicHelper::importGraphicObject( const StreamDataSequence& rGraphicData )
{
    return createGraphicObject( importGraphic( rGraphicData ) );
}

// ============================================================================

} // namespace oox


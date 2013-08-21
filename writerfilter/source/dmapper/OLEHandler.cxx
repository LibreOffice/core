/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#include <OLEHandler.hxx>
#include <PropertyMap.hxx>
#include "GraphicHelpers.hxx"

#include <doctok/resourceids.hxx>
#include <ooxml/resourceids.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/document/XEmbeddedObjectResolver.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XEmbedObjectCreator.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include "dmapperLoggers.hxx"

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;


OLEHandler::OLEHandler() :
LoggedProperties(dmapper_logger, "OLEHandler"),
m_nDxaOrig(0),
m_nDyaOrig(0),
    m_nWrapMode(1)
{
}


OLEHandler::~OLEHandler()
{
}


void OLEHandler::lcl_attribute(Id rName, Value & rVal)
{
    OUString sStringValue = rVal.getString();
    (void)rName;
    switch( rName )
    {
        case NS_ooxml::LN_CT_OLEObject_Type:
            m_sObjectType = sStringValue;
        break;
        case NS_ooxml::LN_CT_OLEObject_ProgID:
            m_sProgId = sStringValue;
        break;
        case NS_ooxml::LN_CT_OLEObject_ShapeID:
            m_sShapeId = sStringValue;
        break;
        case NS_ooxml::LN_CT_OLEObject_DrawAspect:
            m_sDrawAspect = sStringValue;
        break;
        case NS_ooxml::LN_CT_OLEObject_ObjectID:
            m_sObjectId = sStringValue;
        break;
        case NS_ooxml::LN_CT_OLEObject_r_id:
            m_sr_id = sStringValue;
        break;
        case NS_ooxml::LN_inputstream:
            rVal.getAny() >>= m_xInputStream;
        break;
        case NS_ooxml::LN_CT_Object_dxaOrig:
            m_nDxaOrig = rVal.getInt();
        break;
        case NS_ooxml::LN_CT_Object_dyaOrig:
            m_nDyaOrig = rVal.getInt();
        break;
        case NS_ooxml::LN_shape:
        {
            uno::Reference< drawing::XShape > xTempShape;
            rVal.getAny() >>= xTempShape;
            if( xTempShape.is() )
            {
                m_xShape.set( xTempShape );
                uno::Reference< beans::XPropertySet > xShapeProps( xTempShape, uno::UNO_QUERY );
                PropertyNameSupplier& rNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();

                try
                {
                    m_aShapeSize = xTempShape->getSize();
                    m_aShapePosition = xTempShape->getPosition();

                    xShapeProps->getPropertyValue( rNameSupplier.GetName( PROP_BITMAP ) ) >>= m_xReplacement;
                }
                catch( const uno::Exception& e )
                {
                    SAL_WARN("writerfilter", "Exception in OLE Handler: " << e.Message);
                }
                // No need to set the wrapping here as it's either set in oox or will be set later
            }
        }
        break;
        default:
            OSL_FAIL( "unknown attribute");
    }
}


void OLEHandler::lcl_sprm(Sprm & rSprm)
{
    sal_uInt32 nSprmId = rSprm.getId();
    switch( nSprmId )
    {
        case NS_ooxml::LN_OLEObject_OLEObject:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties.get())
            {
                pProperties->resolve(*this);
            }
        }
        break;
        case NS_ooxml::LN_wrap_wrap:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if ( pProperties.get( ) )
            {
                WrapHandlerPtr pHandler( new WrapHandler );
                pProperties->resolve( *pHandler );

                m_nWrapMode = pHandler->getWrapMode( );

                try
                {
                    uno::Reference< beans::XPropertySet > xShapeProps( m_xShape, uno::UNO_QUERY_THROW );
                    PropertyNameSupplier& rNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();

                    xShapeProps->setPropertyValue(
                        rNameSupplier.GetName( PROP_SURROUND ),
                        uno::makeAny( m_nWrapMode ) );
                }
                catch( const uno::Exception& e )
                {
                    SAL_WARN("writerfilter", "Exception in OLE Handler: " << e.Message);
                }
            }
        }
        break;
        default:
        {
            OSL_FAIL( "unknown attribute");
        }
    }
}


OUString OLEHandler::copyOLEOStream( uno::Reference< text::XTextDocument > xTextDocument )
{
    OUString sRet;
    if( !m_xInputStream.is( ) )
        return sRet;
    try
    {
        uno::Reference < lang::XMultiServiceFactory > xFactory(xTextDocument, uno::UNO_QUERY_THROW);
        uno::Reference< document::XEmbeddedObjectResolver > xEmbeddedResolver(
            xFactory->createInstance("com.sun.star.document.ImportEmbeddedObjectResolver"), uno::UNO_QUERY_THROW );
        //hack to work with the ImportEmbeddedObjectResolver
        static sal_Int32 nObjectCount = 100;
        uno::Reference< container::XNameAccess > xNA( xEmbeddedResolver, uno::UNO_QUERY_THROW );
        OUString aURL("Obj");
        aURL += OUString::number( nObjectCount++ );
        uno::Reference < io::XOutputStream > xOLEStream;
        if( (xNA->getByName( aURL ) >>= xOLEStream) && xOLEStream.is() )
        {
            const sal_Int32 nReadRequest = 0x1000;
            uno::Sequence< sal_Int8 > aData;

            while( true )
            {
                sal_Int32 nRead = m_xInputStream->readBytes( aData, nReadRequest );
                xOLEStream->writeBytes( aData );
                if( nRead < nReadRequest )
                {
                    xOLEStream->closeOutput();
                    break;
                }
            }

            static const OUString sProtocol("vnd.sun.star.EmbeddedObject:");
            OUString aPersistName( xEmbeddedResolver->resolveEmbeddedObjectURL( aURL ) );
            sRet = aPersistName.copy( sProtocol.getLength() );

        }
        uno::Reference< lang::XComponent > xComp( xEmbeddedResolver, uno::UNO_QUERY_THROW );
        xComp->dispose();
    }
    catch( const uno::Exception& )
    {
        OSL_FAIL("exception in OLEHandler::createOLEObject");
    }
    return sRet;
}

} //namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

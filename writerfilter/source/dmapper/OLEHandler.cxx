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
#include <OLEHandler.hxx>
#include <PropertyMap.hxx>
#include "GraphicHelpers.hxx"

#include <doctok/resourceids.hxx>
#include <ooxml/resourceids.hxx>
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

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;
/*-- 23.04.2008 10:46:14---------------------------------------------------

  -----------------------------------------------------------------------*/
OLEHandler::OLEHandler() :
    m_nDxaOrig(0),
    m_nDyaOrig(0),
    m_nWrapMode(0)
{
}
/*-- 23.04.2008 10:46:14---------------------------------------------------

  -----------------------------------------------------------------------*/
OLEHandler::~OLEHandler()
{
}
/*-- 23.04.2008 10:46:14---------------------------------------------------

  -----------------------------------------------------------------------*/
void OLEHandler::attribute(Id rName, Value & rVal)
{
    rtl::OUString sStringValue = rVal.getString();
    (void)rName;
    /* WRITERFILTERSTATUS: table: OLEHandler_attributedata */
    switch( rName )
    {
        /* WRITERFILTERSTATUS: done: 1, planned: 0, spent: 0 */
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
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        {
            uno::Reference< drawing::XShape > xTempShape; 
            rVal.getAny() >>= xTempShape;
            if( xTempShape.is() )
            {
                m_xShape.set( xTempShape );
                
                try
                {
                    m_aShapeSize = xTempShape->getSize();
                    m_aShapePosition = xTempShape->getPosition();

                    uno::Reference< beans::XPropertySet > xShapeProps( xTempShape, uno::UNO_QUERY_THROW );
                    PropertyNameSupplier& rNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();

                    xShapeProps->getPropertyValue( rNameSupplier.GetName( PROP_BITMAP ) ) >>= m_xReplacement;
               
                    xShapeProps->setPropertyValue( 
                        rNameSupplier.GetName( PROP_SURROUND ),
                        uno::makeAny( m_nWrapMode ) );
                }
                catch( const uno::Exception& e )
                {
#if DEBUG
                    clog << "Exception in OLE Handler: ";
                    clog << rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr( ) << endl;
#endif
                }    
            }
        }
        break;        
        default:
            OSL_ENSURE( false, "unknown attribute");
    }
}
/*-- 23.04.2008 10:46:14---------------------------------------------------

  -----------------------------------------------------------------------*/
void OLEHandler::sprm(Sprm & rSprm)
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
#if DEBUG
                    clog << "Exception in OLE Handler: ";
                    clog << rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr( ) << endl;
#endif
                } 
            }
        }
        break;
        default:
        {
            OSL_ENSURE( false, "unknown attribute");
        }
    }
}
/*-- 23.04.2008 11:15:19---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString OLEHandler::copyOLEOStream( uno::Reference< text::XTextDocument > xTextDocument )
{
    ::rtl::OUString sRet;
    if( !m_xInputStream.is( ) )
        return sRet;
    try
    {
        uno::Reference < lang::XMultiServiceFactory > xFactory(xTextDocument, uno::UNO_QUERY_THROW);
        uno::Reference< document::XEmbeddedObjectResolver > xEmbeddedResolver(
            xFactory->createInstance(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.ImportEmbeddedObjectResolver" ))), uno::UNO_QUERY_THROW );
        //hack to work with the ImportEmbeddedObjectResolver
        static sal_Int32 nObjectCount = 100;
        uno::Reference< container::XNameAccess > xNA( xEmbeddedResolver, uno::UNO_QUERY_THROW );
        ::rtl::OUString aURL(RTL_CONSTASCII_USTRINGPARAM("Obj" ));
        aURL += ::rtl::OUString::valueOf( nObjectCount++ );
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

            static const ::rtl::OUString sProtocol = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.star.EmbeddedObject:" ));
            ::rtl::OUString aPersistName( xEmbeddedResolver->resolveEmbeddedObjectURL( aURL ) );
            sRet = aPersistName.copy( sProtocol.getLength() );

        }
        uno::Reference< lang::XComponent > xComp( xEmbeddedResolver, uno::UNO_QUERY_THROW );
        xComp->dispose();
    }
    catch( const uno::Exception& rEx)
    {
        (void)rEx;
        OSL_ENSURE(false, "exception in OLEHandler::createOLEObject");
    }
    return sRet;
}

} //namespace dmapper
} //namespace writerfilter

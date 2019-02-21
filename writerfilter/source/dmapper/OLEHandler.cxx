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
#include "OLEHandler.hxx"
#include "DomainMapper.hxx"
#include "PropertyMap.hxx"
#include "GraphicHelpers.hxx"

#include <editeng/unoprnms.hxx>
#include <oox/ole/oleobjecthelper.hxx>
#include <ooxml/resourceids.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <unotools/mediadescriptor.hxx>
#include <officecfg/Office/Common.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/document/XEmbeddedObjectResolver.hpp>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XEmbedObjectCreator.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;


OLEHandler::OLEHandler(DomainMapper& rDomainMapper) :
LoggedProperties("OLEHandler"),
    m_nWrapMode(text::WrapTextMode_THROUGHT),
    m_rDomainMapper(rDomainMapper)
{
}


OLEHandler::~OLEHandler()
{
}


void OLEHandler::lcl_attribute(Id rName, Value & rVal)
{
    OUString sStringValue = rVal.getString();
    switch( rName )
    {
        case NS_ooxml::LN_CT_OLEObject_Type:
        break;
        case NS_ooxml::LN_CT_OLEObject_ProgID:
            m_sProgId = sStringValue;
        break;
        case NS_ooxml::LN_CT_OLEObject_ShapeID:
        break;
        case NS_ooxml::LN_CT_OLEObject_DrawAspect:
            m_sDrawAspect = sStringValue;
        break;
        case NS_ooxml::LN_CT_OLEObject_ObjectID:
        break;
        case NS_ooxml::LN_CT_OLEObject_r_id:
        break;
        case NS_ooxml::LN_inputstream:
            rVal.getAny() >>= m_xInputStream;
        break;
        case NS_ooxml::LN_CT_Object_dxaOrig:
             m_sVisAreaWidth = sStringValue;
        break;
        case NS_ooxml::LN_CT_Object_dyaOrig:
             m_sVisAreaHeight = sStringValue;
        break;
        case NS_ooxml::LN_shape:
        {
            uno::Reference< drawing::XShape > xTempShape;
            rVal.getAny() >>= xTempShape;

            // Control shape is handled on a different code path
            uno::Reference< lang::XServiceInfo > xSInfo( xTempShape, uno::UNO_QUERY_THROW );
            if(xSInfo->supportsService("com.sun.star.drawing.ControlShape"))
            {
                m_rDomainMapper.hasControls(true);
                break;
            }

            if( xTempShape.is() )
            {
                m_xShape.set( xTempShape );
                uno::Reference< beans::XPropertySet > xShapeProps( xTempShape, uno::UNO_QUERY );

                try
                {
                    // Shapes in the header or footer should be in the background, since the default is WrapTextMode_THROUGH.
                    if (m_rDomainMapper.IsInHeaderFooter())
                        xShapeProps->setPropertyValue("Opaque", uno::makeAny(false));

                    m_aShapeSize = xTempShape->getSize();

                    xShapeProps->getPropertyValue( getPropertyName( PROP_BITMAP ) ) >>= m_xReplacement;
                }
                catch( const uno::Exception& e )
                {
                    SAL_WARN("writerfilter", "Exception in OLE Handler: " << e);
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
                tools::SvRef<WrapHandler> pHandler( new WrapHandler );
                pProperties->resolve( *pHandler );

                m_nWrapMode = pHandler->getWrapMode( );

                try
                {
                    uno::Reference< beans::XPropertySet > xShapeProps( m_xShape, uno::UNO_QUERY_THROW );

                    xShapeProps->setPropertyValue(
                        getPropertyName( PROP_SURROUND ),
                        uno::makeAny( static_cast<sal_Int32>(m_nWrapMode) ) );

                    // Through shapes in the header or footer(that spill into the body) should be in the background.
                    // It is just assumed that all shapes will spill into the body.
                    if( m_rDomainMapper.IsInHeaderFooter() )
                        xShapeProps->setPropertyValue("Opaque", uno::makeAny(m_nWrapMode != text::WrapTextMode_THROUGH));
                }
                catch( const uno::Exception& e )
                {
                    SAL_WARN("writerfilter", "Exception in OLE Handler: " << e);
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

void OLEHandler::importStream(const uno::Reference<uno::XComponentContext>& xComponentContext, const uno::Reference<text::XTextDocument>& xTextDocument, const uno::Reference<text::XTextContent>& xOLE)
{
    OUString aFilterService;
    if (m_sProgId == "Word.Document.12")
        aFilterService = "com.sun.star.comp.Writer.WriterFilter";
    else if (m_sProgId == "Excel.Sheet.12")
        aFilterService = "com.sun.star.comp.oox.xls.ExcelFilter";
    else if (m_sProgId == "Equation.3")
        aFilterService = "com.sun.star.comp.Math.MathTypeFilter";
    else
        SAL_WARN("writerfilter", "OLEHandler::importStream: unhandled m_sProgId: " << m_sProgId);

    if (!m_xInputStream.is() || aFilterService.isEmpty())
        return;

    // Create the filter service.
    uno::Reference<uno::XInterface> xInterface = xComponentContext->getServiceManager()->createInstanceWithContext(aFilterService, xComponentContext);

    // Set target document.
    uno::Reference<document::XImporter> xImporter(xInterface, uno::UNO_QUERY);
    uno::Reference<document::XEmbeddedObjectSupplier> xSupplier(xOLE, uno::UNO_QUERY);
    uno::Reference<lang::XComponent> xEmbeddedObject(xSupplier->getEmbeddedObject(), uno::UNO_QUERY);
    if (!xEmbeddedObject.is())
        return;
    xImporter->setTargetDocument( xEmbeddedObject );

    // Import the input stream.
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["InputStream"] <<= m_xInputStream;
    uno::Reference<document::XFilter> xFilter(xInterface, uno::UNO_QUERY);
    xFilter->filter(aMediaDescriptor.getAsConstPropertyValueList());

    // Now that the data is imported, update the (typically) changed stream name.
    uno::Reference<beans::XPropertySet> xPropertySet(xOLE, uno::UNO_QUERY);
    ::oox::ole::SaveInteropProperties(xTextDocument,
        xPropertySet->getPropertyValue("StreamName").get<OUString>(), &m_aURL,
        m_sProgId, m_sDrawAspect);
}

OUString OLEHandler::getCLSID(const uno::Reference<uno::XComponentContext>& xComponentContext) const
{
    OUString aRet;

    // See officecfg/registry/data/org/openoffice/Office/Embedding.xcu.
    if (m_sProgId == "Word.Document.12")
    {
        if (officecfg::Office::Common::Filter::Microsoft::Import::WinWordToWriter::get(xComponentContext))
            aRet = "8BC6B165-B1B2-4EDD-aa47-dae2ee689dd6";
    }
    else if (m_sProgId == "Excel.Sheet.12")
    {
        if (officecfg::Office::Common::Filter::Microsoft::Import::ExcelToCalc::get(xComponentContext))
            aRet = "47BBB4CB-CE4C-4E80-A591-42D9AE74950F";
    }
    else if (m_sProgId == "Equation.3")
    {
        if (officecfg::Office::Common::Filter::Microsoft::Import::MathTypeToMath::get(xComponentContext))
            aRet = "078B7ABA-54FC-457F-8551-6147E776A997";
    }
    else
        SAL_WARN("writerfilter", "OLEHandler::getCLSID: unhandled m_sProgId: " << m_sProgId);

    return aRet;
}

OUString const & OLEHandler::GetDrawAspect() const
{
    return m_sDrawAspect;
}

OUString const & OLEHandler::GetVisAreaWidth() const
{
    return m_sVisAreaWidth;
}

OUString const & OLEHandler::GetVisAreaHeight() const
{
    return m_sVisAreaHeight;
}

OUString OLEHandler::copyOLEOStream(
        uno::Reference<text::XTextDocument> const& xTextDocument)
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

            ::oox::ole::SaveInteropProperties(xTextDocument, aURL, nullptr, m_sProgId, m_sDrawAspect);

            OUString aPersistName( xEmbeddedResolver->resolveEmbeddedObjectURL( aURL ) );
            sRet = aPersistName.copy( strlen("vnd.sun.star.EmbeddedObject:") );

        }
        uno::Reference< lang::XComponent > xComp( xEmbeddedResolver, uno::UNO_QUERY_THROW );
        xComp->dispose();
        m_aURL = aURL;
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

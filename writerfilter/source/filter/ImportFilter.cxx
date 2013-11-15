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

#include <osl/diagnose.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <unotools/mediadescriptor.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <oox/core/filterdetect.hxx>
#include <dmapper/DomainMapper.hxx>
#include <WriterFilter.hxx>
#include <doctok/WW8Document.hxx>
#include <ooxml/OOXMLDocument.hxx>
#ifdef DEBUG_IMPORT
#include <iostream>
#include <osl/process.h>
#endif

#include <resourcemodel/TagLogger.hxx>
#include <oox/ole/olestorage.hxx>
#include <oox/ole/vbaproject.hxx>
#include <oox/helper/graphichelper.hxx>
using namespace ::rtl;
using namespace ::com::sun::star;
using utl::MediaDescriptor;



sal_Bool WriterFilter::filter( const uno::Sequence< beans::PropertyValue >& aDescriptor )
   throw (uno::RuntimeException)
{
    if( m_xSrcDoc.is() )
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF(m_xContext->getServiceManager(), uno::UNO_QUERY_THROW);
        uno::Reference< uno::XInterface > xIfc( xMSF->createInstance("com.sun.star.comp.Writer.DocxExport"), uno::UNO_QUERY_THROW);
        if (!xIfc.is())
            return sal_False;
        uno::Reference< document::XExporter > xExprtr(xIfc, uno::UNO_QUERY_THROW);
        uno::Reference< document::XFilter > xFltr(xIfc, uno::UNO_QUERY_THROW);
        if (!xExprtr.is() || !xFltr.is())
            return sal_False;
        xExprtr->setSourceDocument(m_xSrcDoc);
        return xFltr->filter(aDescriptor);
    }
    else if (m_xDstDoc.is())
    {
        MediaDescriptor aMediaDesc( aDescriptor );
        bool bRepairStorage = aMediaDesc.getUnpackedValueOrDefault( "RepairPackage", false );

        uno::Reference< io::XInputStream > xInputStream;
        try
        {
            // use the oox.core.FilterDetect implementation to extract the decrypted ZIP package
            ::oox::core::FilterDetect aDetector( m_xContext );
            xInputStream = aDetector.extractUnencryptedPackage( aMediaDesc );
        }
        catch( uno::Exception& )
        {
        }

        if ( !xInputStream.is() )
            return sal_False;

#ifdef DEBUG_IMPORT
        OUString sURL = aMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_URL(), OUString() );
        ::std::string sURLc = OUStringToOString(sURL, RTL_TEXTENCODING_ASCII_US).getStr();

        writerfilter::TagLogger::Pointer_t debugLogger
        (writerfilter::TagLogger::getInstance("DEBUG"));
        debugLogger->setFileName(sURLc);
        debugLogger->startDocument();

        writerfilter::TagLogger::Pointer_t dmapperLogger
        (writerfilter::TagLogger::getInstance("DOMAINMAPPER"));
        dmapperLogger->setFileName(sURLc);
        dmapperLogger->startDocument();
#endif

    writerfilter::dmapper::SourceDocumentType eType =
        (m_sFilterName == "writer_MS_Word_2007" || m_sFilterName == "writer_MS_Word_2007_Template" ||
         m_sFilterName == "writer_OOXML" || m_sFilterName == "writer_OOXML_Text_Template" ) ?
            writerfilter::dmapper::DOCUMENT_OOXML : writerfilter::dmapper::DOCUMENT_DOC;

    writerfilter::Stream::Pointer_t pStream(new writerfilter::dmapper::DomainMapper(m_xContext, xInputStream, m_xDstDoc, bRepairStorage, eType, uno::Reference<text::XTextRange>()));
    //create the tokenizer and domain mapper
    if( eType == writerfilter::dmapper::DOCUMENT_OOXML )
    {
        writerfilter::ooxml::OOXMLStream::Pointer_t pDocStream = writerfilter::ooxml::OOXMLDocumentFactory::createStream(m_xContext, xInputStream, bRepairStorage);
        writerfilter::ooxml::OOXMLDocument::Pointer_t pDocument(writerfilter::ooxml::OOXMLDocumentFactory::createDocument(pDocStream));

        uno::Reference<frame::XModel> xModel(m_xDstDoc, uno::UNO_QUERY_THROW);
        pDocument->setModel(xModel);

        uno::Reference<drawing::XDrawPageSupplier> xDrawings
            (m_xDstDoc, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XDrawPage> xDrawPage
            (xDrawings->getDrawPage(), uno::UNO_SET_THROW);
        pDocument->setDrawPage(xDrawPage);

        pDocument->resolve(*pStream);

        // Adding the saved Theme DOM to the document's grab bag
        try
        {
            uno::Reference<beans::XPropertySet> xDocProps(m_xDstDoc, uno::UNO_QUERY);
            if (xDocProps.is())
            {
                uno::Reference<beans::XPropertySetInfo> xPropsInfo = xDocProps->getPropertySetInfo();

                const OUString aGrabBagPropName = "InteropGrabBag";
                if( xPropsInfo.is() && xPropsInfo->hasPropertyByName( aGrabBagPropName ) )
                {
                    uno::Sequence<beans::PropertyValue> aGrabBag;

                    // We want to keep the previous items
                    xDocProps->getPropertyValue( aGrabBagPropName ) >>= aGrabBag;
                    sal_Int32 length = aGrabBag.getLength();
                    aGrabBag.realloc(length+1);

                    uno::Reference<xml::dom::XDocument> aThemeDom = pDocument->getThemeDom();

                    beans::PropertyValue* pValue = aGrabBag.getArray();
                    pValue[length].Name = "OOXTheme";
                    pValue[length].Value = uno::makeAny( aThemeDom );

                    xDocProps->setPropertyValue( aGrabBagPropName, uno::Any( aGrabBag ) );
                }
            }
        }
        catch(const uno::Exception&)
        {
            SAL_WARN("writerfilter","Failed to save theme dom to documents grab bag");
        }

        // Adding the saved custom xml DOM to the document's grab bag
        try
        {
              uno::Reference<beans::XPropertySet> xDocProps(m_xDstDoc, uno::UNO_QUERY);
              if (xDocProps.is())
              {
                  uno::Reference<beans::XPropertySetInfo> xPropsInfo = xDocProps->getPropertySetInfo();

                  const OUString& aGrabBagPropName = OUString("InteropGrabBag");
                  if( xPropsInfo.is() && xPropsInfo->hasPropertyByName( aGrabBagPropName ) )
                  {
                      uno::Sequence<beans::PropertyValue> aGrabBag;

                      // We want to keep the previous items
                      xDocProps->getPropertyValue( aGrabBagPropName ) >>= aGrabBag;
                      sal_Int32 length = aGrabBag.getLength();
                      aGrabBag.realloc(length+2);

                      beans::PropertyValue* pValue = aGrabBag.getArray();
                      pValue[length].Name = "OOXCustomXml";
                      pValue[length].Value = uno::makeAny( pDocument->getCustomXmlDomList() );

                      pValue[length+1].Name = "OOXCustomXmlProps";
                      pValue[length+1].Value = uno::makeAny( pDocument->getCustomXmlDomPropsList() );

                      xDocProps->setPropertyValue( aGrabBagPropName, uno::Any( aGrabBag ) );


                  }
              }
        }
        catch(const uno::Exception&)
        {
            SAL_WARN("writerfilter","Failed to save custom xml dom to documents grab bag");
        }

        // Adding the saved ActiveX DOM to the document's grab bag
        try
        {
            uno::Reference<beans::XPropertySet> xDocProps(m_xDstDoc, uno::UNO_QUERY);
            if (xDocProps.is())
            {
                uno::Reference<beans::XPropertySetInfo> xPropsInfo = xDocProps->getPropertySetInfo();

                const OUString aGrabBagPropName = "InteropGrabBag";
                if( xPropsInfo.is() && xPropsInfo->hasPropertyByName( aGrabBagPropName ) )
                {
                    uno::Sequence<beans::PropertyValue> aGrabBag;

                    // We want to keep the previous items
                    xDocProps->getPropertyValue( aGrabBagPropName ) >>= aGrabBag;
                    sal_Int32 length = aGrabBag.getLength();
                    aGrabBag.realloc(length+2);

                    beans::PropertyValue* pValue = aGrabBag.getArray();
                    pValue[length].Name = "OOXActiveX";
                    pValue[length].Value = uno::makeAny( pDocument->getActiveXDomList() );

                    pValue[length+1].Name = "OOXActiveXBin";
                    pValue[length+1].Value = uno::makeAny( pDocument->getActiveXBinList() );

                    xDocProps->setPropertyValue( aGrabBagPropName, uno::Any( aGrabBag ) );
                 }
             }
         }
         catch(const uno::Exception&)
         {
             SAL_WARN("writerfilter","Failed to save ActiveX dom to documents grab bag");
         }

        writerfilter::ooxml::OOXMLStream::Pointer_t  pVBAProjectStream(writerfilter::ooxml::OOXMLDocumentFactory::createStream( pDocStream, writerfilter::ooxml::OOXMLStream::VBAPROJECT ));
        oox::StorageRef xVbaPrjStrg( new ::oox::ole::OleStorage( m_xContext, pVBAProjectStream->getDocumentStream(), false ) );
        if( xVbaPrjStrg.get() && xVbaPrjStrg->isStorage() )
        {
            ::oox::ole::VbaProject aVbaProject( m_xContext, xModel, "Writer" );
            uno::Reference< frame::XFrame > xFrame = aMediaDesc.getUnpackedValueOrDefault(  MediaDescriptor::PROP_FRAME(), uno::Reference< frame::XFrame > () );

            // if no XFrame try fallback to what we can glean from the Model
            if ( !xFrame.is() )
            {
                uno::Reference< frame::XController > xController =  xModel->getCurrentController();
                xFrame =  xController.is() ? xController->getFrame() : NULL;
            }

            oox::GraphicHelper gHelper( m_xContext, xFrame, xVbaPrjStrg );
            aVbaProject.importVbaProject( *xVbaPrjStrg, gHelper );
        }
    }

    pStream.reset();
#ifdef DEBUG_IMPORT

    dmapperLogger->endDocument();
    debugLogger->endDocument();
#endif

    return sal_True;
    }
    return sal_False;
}


void WriterFilter::cancel(  ) throw (uno::RuntimeException)
{
}



void WriterFilter::setTargetDocument( const uno::Reference< lang::XComponent >& xDoc )
   throw (lang::IllegalArgumentException, uno::RuntimeException)
{
   m_xDstDoc = xDoc;

   // Set some compatibility options that are valid for all the formats
   uno::Reference< lang::XMultiServiceFactory > xFactory( xDoc, uno::UNO_QUERY );
   uno::Reference< beans::XPropertySet > xSettings( xFactory->createInstance("com.sun.star.document.Settings"), uno::UNO_QUERY );

   xSettings->setPropertyValue( "AddFrameOffsets", uno::makeAny( sal_True ) );
   xSettings->setPropertyValue( "UseOldNumbering", uno::makeAny( sal_False ) );
   xSettings->setPropertyValue( "IgnoreFirstLineIndentInNumbering", uno::makeAny( sal_False ) );
   xSettings->setPropertyValue( "DoNotResetParaAttrsForNumFont", uno::makeAny( sal_False ) );
   xSettings->setPropertyValue( "UseFormerLineSpacing", uno::makeAny( sal_False ) );
   xSettings->setPropertyValue( "AddParaSpacingToTableCells", uno::makeAny( sal_True ) );
   xSettings->setPropertyValue( "UseFormerObjectPositioning", uno::makeAny( sal_False ) );
   xSettings->setPropertyValue( "ConsiderTextWrapOnObjPos", uno::makeAny( sal_True ) );
   xSettings->setPropertyValue( "UseFormerTextWrapping", uno::makeAny( sal_False ) );
   xSettings->setPropertyValue( "TableRowKeep", uno::makeAny( sal_True ) );
   xSettings->setPropertyValue( "IgnoreTabsAndBlanksForLineCalculation", uno::makeAny( sal_True ) );
   xSettings->setPropertyValue( "InvertBorderSpacing", uno::makeAny( sal_True ) );
   xSettings->setPropertyValue( "CollapseEmptyCellPara", uno::makeAny( sal_True ) );
   xSettings->setPropertyValue( "TabOverflow", uno::makeAny( sal_True ) );
   xSettings->setPropertyValue( "UnbreakableNumberings", uno::makeAny( sal_True ) );

   // Don't load the default style definitions to avoid weird mix
   xSettings->setPropertyValue( "StylesNoDefault", uno::makeAny( sal_True ) );

   xSettings->setPropertyValue("FloattableNomargins", uno::makeAny( sal_True ));
   xSettings->setPropertyValue( "ClippedPictures", uno::makeAny( sal_True ) );
   xSettings->setPropertyValue( "BackgroundParaOverDrawings", uno::makeAny( sal_True ) );
   xSettings->setPropertyValue( "TabOverMargin", uno::makeAny( sal_True ) );
}

void WriterFilter::setSourceDocument( const uno::Reference< lang::XComponent >& xDoc )
   throw (lang::IllegalArgumentException, uno::RuntimeException)
{
   m_xSrcDoc = xDoc;
}



void WriterFilter::initialize( const uno::Sequence< uno::Any >& aArguments ) throw (uno::Exception, uno::RuntimeException)
{
   uno::Sequence < beans::PropertyValue > aAnySeq;
   sal_Int32 nLength = aArguments.getLength();
   if ( nLength && ( aArguments[0] >>= aAnySeq ) )
   {
       const beans::PropertyValue * pValue = aAnySeq.getConstArray();
       nLength = aAnySeq.getLength();
       for ( sal_Int32 i = 0 ; i < nLength; i++)
       {
           if ( pValue[i].Name == "Type" )
           {
               pValue[i].Value >>= m_sFilterName;
               break;
           }
       }
   }
}


OUString WriterFilter_getImplementationName () throw (uno::RuntimeException)
{
   return OUString ( "com.sun.star.comp.Writer.WriterFilter" );
}

uno::Sequence< OUString > WriterFilter_getSupportedServiceNames(  ) throw (uno::RuntimeException)
{
   uno::Sequence < OUString > aRet(2);
   OUString* pArray = aRet.getArray();
   pArray[0] = "com.sun.star.document.ImportFilter";
   pArray[1] = "com.sun.star.document.ExportFilter";
   return aRet;
}

uno::Reference< uno::XInterface > WriterFilter_createInstance( const uno::Reference< uno::XComponentContext >& xContext)
                throw( uno::Exception )
{
   return (cppu::OWeakObject*) new WriterFilter( xContext );
}



OUString WriterFilter::getImplementationName(  ) throw (uno::RuntimeException)
{
   return WriterFilter_getImplementationName();
}


sal_Bool WriterFilter::supportsService( const OUString& rServiceName ) throw (uno::RuntimeException)
{
    return cppu::supportsService( this, rServiceName );
}


uno::Sequence< OUString > WriterFilter::getSupportedServiceNames(  ) throw (uno::RuntimeException)
{
    return WriterFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

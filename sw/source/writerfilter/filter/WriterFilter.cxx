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

#ifdef DBG_UTIL
#include <iostream>
#endif

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/io/WrongFormatException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <dmapper/DomainMapperFactory.hxx>
#include <oox/core/filterdetect.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/helper/graphichelper.hxx>
#include <oox/ole/olestorage.hxx>
#include <oox/ole/vbaproject.hxx>
#include <ooxml/OOXMLDocument.hxx>
#include <unotools/mediadescriptor.hxx>
#include <rtl/ref.hxx>
#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/scopeguard.hxx>
#include <unotxdoc.hxx>
#include <SwXDocumentSettings.hxx>

using namespace ::com::sun::star;

static OUString lcl_GetExceptionMessageRec(xml::sax::SAXException const& e);

static OUString lcl_GetExceptionMessage(xml::sax::SAXException const& e)
{
    OUString const thisMessage("SAXParseException: \"" + e.Message + "\"");
    OUString const restMessage(lcl_GetExceptionMessageRec(e));
    return restMessage + "\n" + thisMessage;
}
static OUString lcl_GetExceptionMessage(xml::sax::SAXParseException const& e)
{
    OUString const thisMessage("SAXParseException: '" + e.Message + "', Stream '" + e.SystemId
                               + "', Line " + OUString::number(e.LineNumber) + ", Column "
                               + OUString::number(e.ColumnNumber));
    OUString const restMessage(lcl_GetExceptionMessageRec(e));
    return restMessage + "\n" + thisMessage;
}

static OUString lcl_GetExceptionMessageRec(xml::sax::SAXException const& e)
{
    xml::sax::SAXParseException saxpe;
    if (e.WrappedException >>= saxpe)
    {
        return lcl_GetExceptionMessage(saxpe);
    }
    xml::sax::SAXException saxe;
    if (e.WrappedException >>= saxe)
    {
        return lcl_GetExceptionMessage(saxe);
    }
    uno::Exception ue;
    if (e.WrappedException >>= ue)
    {
        return ue.Message;
    }
    return {};
}

namespace
{
/// Common DOCX filter, calls DocxExportFilter via UNO or does the DOCX import.
class WriterFilter
    : public cppu::WeakImplHelper<document::XFilter, document::XImporter, document::XExporter,
                                  lang::XInitialization, lang::XServiceInfo>
{
    uno::Reference<uno::XComponentContext> m_xContext;
    uno::Reference<lang::XComponent> m_xSrcDoc;
    rtl::Reference<SwXTextDocument> m_xDstDoc;
    uno::Sequence<uno::Any> m_xInitializationArguments;

public:
    explicit WriterFilter(uno::Reference<uno::XComponentContext> xContext)
        : m_xContext(std::move(xContext))
    {
    }

    // XFilter
    sal_Bool SAL_CALL filter(const uno::Sequence<beans::PropertyValue>& rDescriptor) override;
    void SAL_CALL cancel() override;

    // XImporter
    void SAL_CALL setTargetDocument(const uno::Reference<lang::XComponent>& xDoc) override;

    // XExporter
    void SAL_CALL setSourceDocument(const uno::Reference<lang::XComponent>& xDoc) override;

    // XInitialization
    void SAL_CALL initialize(const uno::Sequence<uno::Any>& rArguments) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;
    sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;
    uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
};
}

sal_Bool WriterFilter::filter(const uno::Sequence<beans::PropertyValue>& rDescriptor)
{
    if (m_xSrcDoc.is())
    {
        uno::Reference<lang::XMultiServiceFactory> xMSF(m_xContext->getServiceManager(),
                                                        uno::UNO_QUERY_THROW);
        uno::Reference<uno::XInterface> xIfc;
        try
        {
            xIfc.set(xMSF->createInstance("com.sun.star.comp.Writer.DocxExport"),
                     uno::UNO_SET_THROW);
        }
        catch (uno::RuntimeException&)
        {
            throw;
        }
        catch (uno::Exception& e)
        {
            uno::Any a(cppu::getCaughtException());
            throw lang::WrappedTargetRuntimeException("wrapped " + a.getValueTypeName() + ": "
                                                          + e.Message,
                                                      uno::Reference<uno::XInterface>(), a);
        }

        uno::Reference<lang::XInitialization> xInit(xIfc, uno::UNO_QUERY_THROW);
        xInit->initialize(m_xInitializationArguments);

        uno::Reference<document::XExporter> xExprtr(xIfc, uno::UNO_QUERY_THROW);
        uno::Reference<document::XFilter> xFltr(xIfc, uno::UNO_QUERY_THROW);
        xExprtr->setSourceDocument(m_xSrcDoc);
        return xFltr->filter(rDescriptor);
    }
    if (m_xDstDoc.is())
    {
        m_xDstDoc->setPropertyValue("UndocumentedWriterfilterHack", uno::Any(true));
        comphelper::ScopeGuard g([this] {
            m_xDstDoc->setPropertyValue("UndocumentedWriterfilterHack", uno::Any(false));
        });
        utl::MediaDescriptor aMediaDesc(rDescriptor);
        bool bRepairStorage = aMediaDesc.getUnpackedValueOrDefault("RepairPackage", false);
        bool bSkipImages
            = aMediaDesc.getUnpackedValueOrDefault("FilterOptions", OUString()) == "SkipImages";

        uno::Reference<io::XInputStream> xInputStream;
        try
        {
            // use the oox.core.FilterDetect implementation to extract the decrypted ZIP package
            rtl::Reference<::oox::core::FilterDetect> xDetector(
                new ::oox::core::FilterDetect(m_xContext));
            xInputStream = xDetector->extractUnencryptedPackage(aMediaDesc);
        }
        catch (uno::Exception&)
        {
        }

        if (!xInputStream.is())
            return false;

        writerfilter::Stream::Pointer_t pStream(
            writerfilter::dmapper::DomainMapperFactory::createMapper(
                m_xContext, xInputStream, m_xDstDoc, bRepairStorage,
                writerfilter::dmapper::SourceDocumentType::OOXML, aMediaDesc));
        //create the tokenizer and domain mapper
        writerfilter::ooxml::OOXMLStream::Pointer_t pDocStream
            = writerfilter::ooxml::OOXMLDocumentFactory::createStream(m_xContext, xInputStream,
                                                                      bRepairStorage);
        uno::Reference<task::XStatusIndicator> xStatusIndicator
            = aMediaDesc.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_STATUSINDICATOR,
                                                   uno::Reference<task::XStatusIndicator>());
        writerfilter::ooxml::OOXMLDocument::Pointer_t pDocument(
            writerfilter::ooxml::OOXMLDocumentFactory::createDocument(pDocStream, xStatusIndicator,
                                                                      bSkipImages, rDescriptor));

        uno::Reference<frame::XModel> xModel(static_cast<SfxBaseModel*>(m_xDstDoc.get()));
        pDocument->setModel(xModel);

        uno::Reference<drawing::XDrawPage> xDrawPage(m_xDstDoc->getDrawPage(), uno::UNO_SET_THROW);
        pDocument->setDrawPage(xDrawPage);

        try
        {
            pDocument->resolve(*pStream);
        }
        catch (xml::sax::SAXParseException const& e)
        {
            // note: SfxObjectShell checks for WrongFormatException
            io::WrongFormatException wfe(lcl_GetExceptionMessage(e));
            throw lang::WrappedTargetRuntimeException("", getXWeak(), uno::Any(wfe));
        }
        catch (xml::sax::SAXException const& e)
        {
            // note: SfxObjectShell checks for WrongFormatException
            io::WrongFormatException wfe(lcl_GetExceptionMessage(e));
            throw lang::WrappedTargetRuntimeException("", getXWeak(), uno::Any(wfe));
        }
        catch (uno::RuntimeException const&)
        {
            throw;
        }
        catch (uno::Exception const&)
        {
            css::uno::Any anyEx = cppu::getCaughtException();
            SAL_WARN("writerfilter",
                     "WriterFilter::filter(): failed with " << exceptionToString(anyEx));
            throw lang::WrappedTargetRuntimeException("", getXWeak(), anyEx);
        }

        // Adding some properties to the document's grab bag for interoperability purposes:
        comphelper::SequenceAsHashMap aGrabBagProperties;

        // Adding the saved Theme DOM
        aGrabBagProperties["OOXTheme"] <<= pDocument->getThemeDom();

        // Adding the saved custom xml DOM
        aGrabBagProperties["OOXCustomXml"] <<= pDocument->getCustomXmlDomList();
        aGrabBagProperties["OOXCustomXmlProps"] <<= pDocument->getCustomXmlDomPropsList();

        // Adding the saved Glossary Document DOM to the document's grab bag
        aGrabBagProperties["OOXGlossary"] <<= pDocument->getGlossaryDocDom();
        aGrabBagProperties["OOXGlossaryDom"] <<= pDocument->getGlossaryDomList();

        // Adding the saved embedding document to document's grab bag
        aGrabBagProperties["OOXEmbeddings"] <<= pDocument->getEmbeddingsList();

        oox::core::XmlFilterBase::putPropertiesToDocumentGrabBag(
            static_cast<SfxBaseModel*>(m_xDstDoc.get()), aGrabBagProperties);

        writerfilter::ooxml::OOXMLStream::Pointer_t pVBAProjectStream(
            writerfilter::ooxml::OOXMLDocumentFactory::createStream(
                pDocStream, writerfilter::ooxml::OOXMLStream::VBAPROJECT));
        oox::StorageRef xVbaPrjStrg = std::make_shared<::oox::ole::OleStorage>(
            m_xContext, pVBAProjectStream->getDocumentStream(), false);
        if (xVbaPrjStrg && xVbaPrjStrg->isStorage())
        {
            ::oox::ole::VbaProject aVbaProject(m_xContext, xModel, u"Writer");
            uno::Reference<frame::XFrame> xFrame = aMediaDesc.getUnpackedValueOrDefault(
                utl::MediaDescriptor::PROP_FRAME, uno::Reference<frame::XFrame>());

            // if no XFrame try fallback to what we can glean from the Model
            if (!xFrame.is())
            {
                uno::Reference<frame::XController> xController = xModel->getCurrentController();
                xFrame = xController.is() ? xController->getFrame() : nullptr;
            }

            oox::GraphicHelper gHelper(m_xContext, xFrame, xVbaPrjStrg);
            aVbaProject.importVbaProject(*xVbaPrjStrg, gHelper);

            writerfilter::ooxml::OOXMLStream::Pointer_t pVBADataStream(
                writerfilter::ooxml::OOXMLDocumentFactory::createStream(
                    pDocStream, writerfilter::ooxml::OOXMLStream::VBADATA));
            if (pVBADataStream)
            {
                uno::Reference<io::XInputStream> xDataStream = pVBADataStream->getDocumentStream();
                if (xDataStream.is())
                    aVbaProject.importVbaData(xDataStream);
            }
        }

        pStream.clear();

        // note: pStream.clear calls RemoveLastParagraph()

        return true;
    }
    return false;
}

void WriterFilter::cancel() {}

void WriterFilter::setTargetDocument(const uno::Reference<lang::XComponent>& xDoc)
{
    m_xDstDoc = dynamic_cast<SwXTextDocument*>(xDoc.get());
    assert(m_xDstDoc);

    // Set some compatibility options that are valid for the DOCX format
    rtl::Reference<SwXDocumentSettings> xSettings = m_xDstDoc->createDocumentSettings();

    xSettings->setPropertyValue("UseOldNumbering", uno::Any(false));
    xSettings->setPropertyValue("IgnoreFirstLineIndentInNumbering", uno::Any(false));
    xSettings->setPropertyValue(u"NoGapAfterNoteNumber"_ustr, uno::Any(true));
    xSettings->setPropertyValue("DoNotResetParaAttrsForNumFont", uno::Any(false));
    xSettings->setPropertyValue("UseFormerLineSpacing", uno::Any(false));
    xSettings->setPropertyValue("AddParaSpacingToTableCells", uno::Any(true));
    xSettings->setPropertyValue("AddParaLineSpacingToTableCells", uno::Any(true));
    xSettings->setPropertyValue("UseFormerObjectPositioning", uno::Any(false));
    xSettings->setPropertyValue("ConsiderTextWrapOnObjPos", uno::Any(true));
    xSettings->setPropertyValue("UseFormerTextWrapping", uno::Any(false));
    xSettings->setPropertyValue("IgnoreTabsAndBlanksForLineCalculation", uno::Any(true));
    xSettings->setPropertyValue("InvertBorderSpacing", uno::Any(true));
    xSettings->setPropertyValue("CollapseEmptyCellPara", uno::Any(true));
    // tdf#142404 TabOverSpacing (new for compatibilityMode15/Word2013+) is a subset of TabOverMargin
    // (which applied to DOCX <= compatibilityMode14).
    // TabOverMargin looks at tabs beyond the normal text area,
    // while TabOverSpacing only refers to a tab beyond the paragraph margin.
    xSettings->setPropertyValue("TabOverSpacing", uno::Any(true));
    xSettings->setPropertyValue("UnbreakableNumberings", uno::Any(true));

    xSettings->setPropertyValue("ClippedPictures", uno::Any(true));
    xSettings->setPropertyValue("BackgroundParaOverDrawings", uno::Any(true));
    xSettings->setPropertyValue("TreatSingleColumnBreakAsPageBreak", uno::Any(true));
    xSettings->setPropertyValue("PropLineSpacingShrinksFirstLine", uno::Any(true));
    xSettings->setPropertyValue("DoNotCaptureDrawObjsOnPage", uno::Any(true));
    xSettings->setPropertyValue("DisableOffPagePositioning", uno::Any(true));
    xSettings->setPropertyValue("DropCapPunctuation", uno::Any(true));
    // rely on default for HyphenateURLs=false
    // rely on default for APPLY_TEXT_ATTR_TO_EMPTY_LINE_AT_END_OF_PARAGRAPH=true
}

void WriterFilter::setSourceDocument(const uno::Reference<lang::XComponent>& xDoc)
{
    m_xSrcDoc = xDoc;
}

void WriterFilter::initialize(const uno::Sequence<uno::Any>& rArguments)
{
    m_xInitializationArguments = rArguments;
}

OUString WriterFilter::getImplementationName() { return "com.sun.star.comp.Writer.WriterFilter"; }

sal_Bool WriterFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> WriterFilter::getSupportedServiceNames()
{
    uno::Sequence<OUString> aRet = { OUString("com.sun.star.document.ImportFilter"),
                                     OUString("com.sun.star.document.ExportFilter") };
    return aRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_Writer_WriterFilter_get_implementation(
    uno::XComponentContext* component, uno::Sequence<uno::Any> const& /*rSequence*/)
{
    return cppu::acquire(new WriterFilter(component));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

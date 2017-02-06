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

#ifdef DEBUG_WRITERFILTER
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
#include <oox/helper/graphichelper.hxx>
#include <oox/ole/olestorage.hxx>
#include <oox/ole/vbaproject.hxx>
#include <ooxml/OOXMLDocument.hxx>
#include <unotools/mediadescriptor.hxx>
#include <rtl/ref.hxx>

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
    OUString const thisMessage("SAXParseException: '" + e.Message + "', Stream '" + e.SystemId + "', Line " + OUString::number(e.LineNumber)
                               + ", Column " + OUString::number(e.ColumnNumber));
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
    return OUString();
}

/// Common DOCX filter, calls DocxExportFilter via UNO or does the DOCX import.
class WriterFilter : public cppu::WeakImplHelper
    <
    document::XFilter,
    document::XImporter,
    document::XExporter,
    lang::XInitialization,
    lang::XServiceInfo
    >
{

protected:
    uno::Reference<uno::XComponentContext> m_xContext;
    uno::Reference<lang::XComponent> m_xSrcDoc, m_xDstDoc;

public:
    explicit WriterFilter(const uno::Reference<uno::XComponentContext>& rxContext)
        : m_xContext(rxContext)
    {}

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

private:
    void putPropertiesToDocumentGrabBag(const comphelper::SequenceAsHashMap& rProperties);

};

sal_Bool WriterFilter::filter(const uno::Sequence< beans::PropertyValue >& aDescriptor)
{
    if (m_xSrcDoc.is())
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF(m_xContext->getServiceManager(), uno::UNO_QUERY_THROW);
        uno::Reference< uno::XInterface > xIfc;
        try
        {
            xIfc.set(xMSF->createInstance("com.sun.star.comp.Writer.DocxExport"), uno::UNO_QUERY_THROW);
        }
        catch (uno::RuntimeException&)
        {
            throw;
        }
        catch (uno::Exception& e)
        {
            uno::Any a(cppu::getCaughtException());
            throw lang::WrappedTargetRuntimeException("wrapped " + a.getValueTypeName() + ": " + e.Message, uno::Reference<uno::XInterface>(), a);
        }
        uno::Reference< document::XExporter > xExprtr(xIfc, uno::UNO_QUERY_THROW);
        uno::Reference< document::XFilter > xFltr(xIfc, uno::UNO_QUERY_THROW);
        if (!xExprtr.is() || !xFltr.is())
            return false;
        xExprtr->setSourceDocument(m_xSrcDoc);
        return xFltr->filter(aDescriptor);
    }
    else if (m_xDstDoc.is())
    {
        utl::MediaDescriptor aMediaDesc(aDescriptor);
        bool bRepairStorage = aMediaDesc.getUnpackedValueOrDefault("RepairPackage", false);
        bool bSkipImages = aMediaDesc.getUnpackedValueOrDefault("FilterOptions", OUString("")) == "SkipImages";

        uno::Reference< io::XInputStream > xInputStream;
        try
        {
            // use the oox.core.FilterDetect implementation to extract the decrypted ZIP package
            rtl::Reference<::oox::core::FilterDetect> xDetector(new ::oox::core::FilterDetect(m_xContext));
            xInputStream = xDetector->extractUnencryptedPackage(aMediaDesc);
        }
        catch (uno::Exception&)
        {
        }

        if (!xInputStream.is())
            return false;

        writerfilter::dmapper::SourceDocumentType eType = writerfilter::dmapper::SourceDocumentType::OOXML;
        writerfilter::Stream::Pointer_t pStream(writerfilter::dmapper::DomainMapperFactory::createMapper(m_xContext, xInputStream, m_xDstDoc, bRepairStorage, eType, aMediaDesc));
        //create the tokenizer and domain mapper
        writerfilter::ooxml::OOXMLStream::Pointer_t pDocStream = writerfilter::ooxml::OOXMLDocumentFactory::createStream(m_xContext, xInputStream, bRepairStorage);
        uno::Reference<task::XStatusIndicator> xStatusIndicator = aMediaDesc.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_STATUSINDICATOR(), uno::Reference<task::XStatusIndicator>());
        writerfilter::ooxml::OOXMLDocument::Pointer_t pDocument(writerfilter::ooxml::OOXMLDocumentFactory::createDocument(pDocStream, xStatusIndicator, bSkipImages, aDescriptor));

        uno::Reference<frame::XModel> xModel(m_xDstDoc, uno::UNO_QUERY_THROW);
        pDocument->setModel(xModel);

        uno::Reference<drawing::XDrawPageSupplier> xDrawings
        (m_xDstDoc, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XDrawPage> xDrawPage
        (xDrawings->getDrawPage(), uno::UNO_SET_THROW);
        pDocument->setDrawPage(xDrawPage);

        try
        {
            pDocument->resolve(*pStream);
        }
        catch (xml::sax::SAXParseException const& e)
        {
            // note: SfxObjectShell checks for WrongFormatException
            io::WrongFormatException wfe(lcl_GetExceptionMessage(e));
            throw lang::WrappedTargetRuntimeException("",
                    static_cast<OWeakObject*>(this), uno::makeAny(wfe));
        }
        catch (xml::sax::SAXException const& e)
        {
            // note: SfxObjectShell checks for WrongFormatException
            io::WrongFormatException wfe(lcl_GetExceptionMessage(e));
            throw lang::WrappedTargetRuntimeException("",
                    static_cast<OWeakObject*>(this), uno::makeAny(wfe));
        }
        catch (uno::RuntimeException const&)
        {
            throw;
        }
        catch (uno::Exception const& e)
        {
            SAL_WARN("writerfilter", "WriterFilter::filter(): "
                     "failed with exception " << e.Message);
            throw lang::WrappedTargetRuntimeException("",
                    static_cast<OWeakObject*>(this), uno::makeAny(e));
        }

        // Adding some properties to the document's grab bag for interoperability purposes:
        comphelper::SequenceAsHashMap aGrabBagProperties;

        // Adding the saved Theme DOM
        aGrabBagProperties["OOXTheme"] = uno::makeAny(pDocument->getThemeDom());

        // Adding the saved custom xml DOM
        aGrabBagProperties["OOXCustomXml"] = uno::makeAny(pDocument->getCustomXmlDomList());
        aGrabBagProperties["OOXCustomXmlProps"] = uno::makeAny(pDocument->getCustomXmlDomPropsList());

        // Adding the saved ActiveX DOM
        aGrabBagProperties["OOXActiveX"] = uno::makeAny(pDocument->getActiveXDomList());
        aGrabBagProperties["OOXActiveXBin"] = uno::makeAny(pDocument->getActiveXBinList());

        // Adding the saved Glossary Documnet DOM to the document's grab bag
        aGrabBagProperties["OOXGlossary"] = uno::makeAny(pDocument->getGlossaryDocDom());
        aGrabBagProperties["OOXGlossaryDom"] = uno::makeAny(pDocument->getGlossaryDomList());

        // Adding the saved embedding document to document's grab bag
        aGrabBagProperties["OOXEmbeddings"] = uno::makeAny(pDocument->getEmbeddingsList());

        putPropertiesToDocumentGrabBag(aGrabBagProperties);

        writerfilter::ooxml::OOXMLStream::Pointer_t  pVBAProjectStream(writerfilter::ooxml::OOXMLDocumentFactory::createStream(pDocStream, writerfilter::ooxml::OOXMLStream::VBAPROJECT));
        oox::StorageRef xVbaPrjStrg(new ::oox::ole::OleStorage(m_xContext, pVBAProjectStream->getDocumentStream(), false));
        if (xVbaPrjStrg.get() && xVbaPrjStrg->isStorage())
        {
            ::oox::ole::VbaProject aVbaProject(m_xContext, xModel, "Writer");
            uno::Reference< frame::XFrame > xFrame = aMediaDesc.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_FRAME(), uno::Reference< frame::XFrame > ());

            // if no XFrame try fallback to what we can glean from the Model
            if (!xFrame.is())
            {
                uno::Reference< frame::XController > xController =  xModel->getCurrentController();
                xFrame =  xController.is() ? xController->getFrame() : nullptr;
            }

            oox::GraphicHelper gHelper(m_xContext, xFrame, xVbaPrjStrg);
            aVbaProject.importVbaProject(*xVbaPrjStrg, gHelper);
        }

        pStream.reset();

        return true;
    }
    return false;
}


void WriterFilter::cancel()
{
}

void WriterFilter::setTargetDocument(const uno::Reference< lang::XComponent >& xDoc)
{
    m_xDstDoc = xDoc;

    // Set some compatibility options that are valid for all the formats
    uno::Reference< lang::XMultiServiceFactory > xFactory(xDoc, uno::UNO_QUERY);
    uno::Reference< beans::XPropertySet > xSettings(xFactory->createInstance("com.sun.star.document.Settings"), uno::UNO_QUERY);

    xSettings->setPropertyValue("AddFrameOffsets", uno::makeAny(true));
    xSettings->setPropertyValue("AddVerticalFrameOffsets", uno::makeAny(true));
    xSettings->setPropertyValue("UseOldNumbering", uno::makeAny(false));
    xSettings->setPropertyValue("IgnoreFirstLineIndentInNumbering", uno::makeAny(false));
    xSettings->setPropertyValue("DoNotResetParaAttrsForNumFont", uno::makeAny(false));
    xSettings->setPropertyValue("UseFormerLineSpacing", uno::makeAny(false));
    xSettings->setPropertyValue("AddParaSpacingToTableCells", uno::makeAny(true));
    xSettings->setPropertyValue("UseFormerObjectPositioning", uno::makeAny(false));
    xSettings->setPropertyValue("ConsiderTextWrapOnObjPos", uno::makeAny(true));
    xSettings->setPropertyValue("UseFormerTextWrapping", uno::makeAny(false));
    xSettings->setPropertyValue("TableRowKeep", uno::makeAny(true));
    xSettings->setPropertyValue("IgnoreTabsAndBlanksForLineCalculation", uno::makeAny(true));
    xSettings->setPropertyValue("InvertBorderSpacing", uno::makeAny(true));
    xSettings->setPropertyValue("CollapseEmptyCellPara", uno::makeAny(true));
    xSettings->setPropertyValue("TabOverflow", uno::makeAny(true));
    xSettings->setPropertyValue("UnbreakableNumberings", uno::makeAny(true));

    xSettings->setPropertyValue("FloattableNomargins", uno::makeAny(true));
    xSettings->setPropertyValue("ClippedPictures", uno::makeAny(true));
    xSettings->setPropertyValue("BackgroundParaOverDrawings", uno::makeAny(true));
    xSettings->setPropertyValue("TabOverMargin", uno::makeAny(true));
    xSettings->setPropertyValue("TreatSingleColumnBreakAsPageBreak", uno::makeAny(true));
    xSettings->setPropertyValue("PropLineSpacingShrinksFirstLine", uno::makeAny(true));
    xSettings->setPropertyValue("DoNotCaptureDrawObjsOnPage", uno::makeAny(true));
}

void WriterFilter::setSourceDocument(const uno::Reference< lang::XComponent >& xDoc)
{
    m_xSrcDoc = xDoc;
}

void WriterFilter::initialize(const uno::Sequence< uno::Any >& /*rArguments*/)
{
}

OUString WriterFilter::getImplementationName()
{
    return OUString("com.sun.star.comp.Writer.WriterFilter");
}


sal_Bool WriterFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}


uno::Sequence<OUString> WriterFilter::getSupportedServiceNames()
{
    uno::Sequence<OUString> aRet =
    {
        OUString("com.sun.star.document.ImportFilter"),
        OUString("com.sun.star.document.ExportFilter")
    };
    return aRet;
}

void WriterFilter::putPropertiesToDocumentGrabBag(const comphelper::SequenceAsHashMap& rProperties)
{
    try
    {
        uno::Reference<beans::XPropertySet> xDocProps(m_xDstDoc, uno::UNO_QUERY);
        if (xDocProps.is())
        {
            uno::Reference<beans::XPropertySetInfo> xPropsInfo = xDocProps->getPropertySetInfo();

            const OUString aGrabBagPropName = "InteropGrabBag";
            if (xPropsInfo.is() && xPropsInfo->hasPropertyByName(aGrabBagPropName))
            {
                // get existing grab bag
                comphelper::SequenceAsHashMap aGrabBag(xDocProps->getPropertyValue(aGrabBagPropName));

                // put the new items
                aGrabBag.update(rProperties);

                // put it back to the document
                xDocProps->setPropertyValue(aGrabBagPropName, uno::Any(aGrabBag.getAsConstPropertyValueList()));
            }
        }
    }
    catch (const uno::Exception&)
    {
        SAL_WARN("writerfilter","Failed to save documents grab bag");
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface* SAL_CALL com_sun_star_comp_Writer_WriterFilter_get_implementation(uno::XComponentContext* component, uno::Sequence<uno::Any> const&)
{
    return cppu::acquire(new WriterFilter(component));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

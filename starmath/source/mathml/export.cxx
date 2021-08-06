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

#include <mathml/export.hxx>

// TODO Check those includes
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/uno/Any.h>

// TODO Check those includes
#include <officecfg/Office/Common.hxx>
#include <rtl/math.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxsids.hrc>
#include <unotools/saveopt.hxx>
#include <sot/storage.hxx>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <comphelper/fileformat.h>
#include <comphelper/processfactory.hxx>
#include <unotools/streamwrap.hxx>
#include <sax/tools/converter.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/attrlist.hxx>
#include <comphelper/genericpropertyset.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <tools/diagnose_ex.h>

// TODO Check those includes
#include <stack>

#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>

// TODO Check those includes
#include <tools/stream.hxx>

// TODO Check those includes
#include <xparsmlbase.hxx>
#include <strings.hrc>
#include <smmod.hxx>
#include <unomodel.hxx>
#include <document.hxx>
#include <utility.hxx>
#include <cfgitem.hxx>
#include <starmathdatabase.hxx>

// TODO Check those includes
#include <sal/config.h>
#include <sal/log.hxx>

using namespace ::com::sun::star;
using namespace xmloff::token;

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

// SmMlExportWrapper
/*************************************************************************************************/

bool SmMlExportWrapper::Export(SfxMedium& rMedium)
{
    bool bRet = true;
    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());

    // Check all fine
    SAL_WARN_IF(m_xModel == nullptr, "starmath", "Missing model");
    SAL_WARN_IF(xContext == nullptr, "starmath", "Missing context");
    if (m_xModel == nullptr || xContext == nullptr)
        return false;

    //Get model
    uno::Reference<lang::XComponent> xModelComp = m_xModel;
    SAL_WARN_IF(xModelComp == nullptr, "starmath", "Missing model component");
    SmModel* pModel = comphelper::getUnoTunnelImplementation<SmModel>(m_xModel);
    SAL_WARN_IF(pModel == nullptr, "starmath", "Failed to get threw uno tunnel");
    if (xModelComp == nullptr || pModel == nullptr)
        return false;

    // Get doc shell
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    if (pDocShell == nullptr)
    {
        SAL_WARN("starmath", "Failed to fetch sm document");
        return false;
    }

    // Check if it is a standalone window or embed object
    bool bEmbedded = SfxObjectCreateMode::EMBEDDED == pDocShell->GetCreateMode();

    // Medium item set
    SfxItemSet* pMediumItemSet = rMedium.GetItemSet();
    if (pDocShell == nullptr)
    {
        SAL_WARN("starmath", "Failed to get medium item set");
        return false;
    }

    // Progress bar ~
    uno::Reference<task::XStatusIndicator> xStatusIndicator;

    if (!bEmbedded)
    {
        // Extra check to ensure everything is fine
        if (pDocShell->GetMedium() != &rMedium)
        {
            SAL_WARN("starmath", "Input medium and sm document medium do not match");
            return false;
        }

        // Fetch progress bar
        auto pItem = pMediumItemSet->GetItem(SID_PROGRESS_STATUSBAR_CONTROL);
        if (pItem)
        {
            // set progress range and start status indicator
            static_cast<const SfxUnoAnyItem*>(pItem)->GetValue() >>= xStatusIndicator;
            xStatusIndicator->start(SmResId(STR_STATSTR_WRITING), 3);
            xStatusIndicator->setValue(0);
        }
    }

    // create XPropertySet with three properties for status indicator
    comphelper::PropertyMapEntry aInfoMap[]
        = { { OUString("UsePrettyPrinting"), 0, cppu::UnoType<bool>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
            { OUString("BaseURI"), 0, ::cppu::UnoType<OUString>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
            { OUString("StreamRelPath"), 0, ::cppu::UnoType<OUString>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
            { OUString("StreamName"), 0, ::cppu::UnoType<OUString>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
            { OUString(), 0, css::uno::Type(), 0, 0 } };
    uno::Reference<beans::XPropertySet> xInfoSet(
        comphelper::GenericPropertySet_CreateInstance(new comphelper::PropertySetInfo(aInfoMap)));

    // Always print pretty
    xInfoSet->setPropertyValue("UsePrettyPrinting", Any(true));

    // Set base URI
    OUString sPropName("BaseURI");
    xInfoSet->setPropertyValue(sPropName, makeAny(rMedium.GetBaseURL(true)));

    if (!m_bFlat) //Storage (Package) of Stream
    {
        // Fetch the output storage
        uno::Reference<embed::XStorage> xStg = rMedium.GetOutputStorage();
        if (xStg == nullptr)
        {
            SAL_WARN("starmath", "Failed to fetch output storage");
            return false;
        }
        bool bOASIS = SotStorage::GetVersion(xStg) > SOFFICE_FILEFORMAT_60;

        // TODO/LATER: handle the case of embedded links gracefully
        if (bEmbedded) //&& !pStg->IsRoot() )
        {
            auto pDocHierarchItem = pMediumItemSet->GetItem(SID_DOC_HIERARCHICALNAME);
            if (pDocHierarchItem != nullptr)
            {
                OUString aName = static_cast<const SfxStringItem*>(pDocHierarchItem)->GetValue();
                if (!aName.isEmpty())
                    xInfoSet->setPropertyValue("StreamRelPath", makeAny(aName));
            }
        }
        else
        {
            // Write file metadata ( data, LO version ... )
            // Note: export through an XML exporter component (storage version)
            if (xStatusIndicator.is())
                xStatusIndicator->setValue(1);

            bRet = WriteThroughComponentS(xStg, xModelComp, u"meta.xml", xContext, xInfoSet,
                                          bOASIS ? u"com.sun.star.comp.Math.XMLOasisMetaExporter"
                                                 : u"com.sun.star.comp.Math.XMLMetaExporter");
        }

        // Write starmath formula
        // Note: export through an XML exporter component (storage version)
        if (bRet)
        {
            if (xStatusIndicator.is())
                xStatusIndicator->setValue(2);

            bRet = WriteThroughComponentS(xStg, xModelComp, u"content.xml", xContext, xInfoSet,
                                          u"com.sun.star.comp.Math.XMLContentExporter");
        }

        // Write starmath settings
        // Note: export through an XML exporter component (storage version)
        if (bRet)
        {
            if (xStatusIndicator.is())
                xStatusIndicator->setValue(3);

            bRet
                = WriteThroughComponentS(xStg, xModelComp, u"settings.xml", xContext, xInfoSet,
                                         bOASIS ? u"com.sun.star.comp.Math.XMLOasisSettingsExporter"
                                                : u"com.sun.star.comp.Math.XMLSettingsExporter");
        }
    }
    else
    {
        // Fetch the output stream
        SvStream* pStream = rMedium.GetOutStream();
        if (pStream == nullptr)
        {
            SAL_WARN("starmath", "Missing output stream");
            return false;
        }
        uno::Reference<io::XOutputStream> xOut(new utl::OOutputStreamWrapper(*pStream));

        if (xStatusIndicator.is())
            xStatusIndicator->setValue(1);

        // Write everything in the same place
        // Note: export through an XML exporter component (output stream version)
        bRet = WriteThroughComponentOS(xOut, xModelComp, xContext, xInfoSet,
                                       u"com.sun.star.comp.Math.XMLContentExporter");
    }

    if (xStatusIndicator.is())
        xStatusIndicator->end();
    return bRet;
}

OUString SmMlExportWrapper::Export(SmMlElement* pElementTree)
{
    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());

    // Check all fine
    SAL_WARN_IF(m_xModel == nullptr, "starmath", "Missing model");
    SAL_WARN_IF(xContext == nullptr, "starmath", "Missing context");
    if (m_xModel == nullptr || xContext == nullptr)
        return OUString("");

    //Get model
    uno::Reference<lang::XComponent> xModelComp = m_xModel;
    SAL_WARN_IF(xModelComp == nullptr, "starmath", "Missing model component");
    SmModel* pModel = comphelper::getUnoTunnelImplementation<SmModel>(m_xModel);
    SAL_WARN_IF(pModel == nullptr, "starmath", "Failed to get threw uno tunnel");
    if (xModelComp == nullptr || pModel == nullptr)
        return OUString("");

    // Get doc shell
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    if (pDocShell == nullptr)
    {
        SAL_WARN("starmath", "Failed to fetch sm document");
        return OUString("");
    }

    // create XPropertySet with three properties for status indicator
    comphelper::PropertyMapEntry aInfoMap[]
        = { { OUString("UsePrettyPrinting"), 0, cppu::UnoType<bool>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
            { OUString("BaseURI"), 0, ::cppu::UnoType<OUString>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
            { OUString("StreamRelPath"), 0, ::cppu::UnoType<OUString>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
            { OUString("StreamName"), 0, ::cppu::UnoType<OUString>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
            { OUString(), 0, css::uno::Type(), 0, 0 } };
    uno::Reference<beans::XPropertySet> xInfoSet(
        comphelper::GenericPropertySet_CreateInstance(new comphelper::PropertySetInfo(aInfoMap)));

    // Always print pretty
    xInfoSet->setPropertyValue("UsePrettyPrinting", Any(true));

    // Fetch mathml tree
    m_pElementTree = pElementTree;

    // Write stuff
    // Note: export through an XML exporter component (memory stream version)
    return WriteThroughComponentMS(xModelComp, xContext, xInfoSet);
}

// export through an XML exporter component (output stream version)
bool SmMlExportWrapper::WriteThroughComponentOS(const Reference<io::XOutputStream>& xOutputStream,
                                                const Reference<XComponent>& xComponent,
                                                Reference<uno::XComponentContext> const& rxContext,
                                                Reference<beans::XPropertySet> const& rPropSet,
                                                const char16_t* pComponentName)
{
    // We need a output stream but it is already checked by caller
    // We need a component but it is already checked by caller
    // We need a context but it is already checked by caller
    // We need a property set but it is already checked by caller
    // We need a component name but it is already checked by caller

    // get sax writer
    Reference<xml::sax::XWriter> xSaxWriter = xml::sax::Writer::create(rxContext);

    // connect XML writer to output stream
    xSaxWriter->setOutputStream(xOutputStream);
    if (m_bUseHTMLMLEntities)
        xSaxWriter->setCustomEntityNames(starmathdatabase::icustomMathmlHtmlEntitiesExport);

    // prepare arguments (prepend doc handler to given arguments)
    Sequence<Any> aArgs(2);
    aArgs[0] <<= xSaxWriter;
    aArgs[1] <<= rPropSet;

    // get filter component
    auto xExporterData = rxContext->getServiceManager()->createInstanceWithArgumentsAndContext(
        OUString(pComponentName), aArgs, rxContext);
    Reference<document::XExporter> xExporter(xExporterData, UNO_QUERY);

    // Check everything is fine
    if (!xExporter.is())
    {
        SAL_WARN("starmath", "can't instantiate export filter component");
        return false;
    }

    // connect model and filter
    xExporter->setSourceDocument(xComponent);

    // filter!
    Reference<XFilter> xFilter(xExporter, UNO_QUERY);
    uno::Sequence<PropertyValue> aProps(0);
    xFilter->filter(aProps);

    // Execute operation
    auto pFilter = comphelper::getUnoTunnelImplementation<SmMlExport>(xFilter);
    return pFilter == nullptr || pFilter->getSuccess();
}

// export through an XML exporter component (storage version)
bool SmMlExportWrapper::WriteThroughComponentS(const Reference<embed::XStorage>& xStorage,
                                               const Reference<XComponent>& xComponent,
                                               const char16_t* pStreamName,
                                               Reference<uno::XComponentContext> const& rxContext,
                                               Reference<beans::XPropertySet> const& rPropSet,
                                               const char16_t* pComponentName)
{
    // We need a storage name but it is already checked by caller
    // We need a component name but it is already checked by caller
    // We need a stream name but it is already checked by caller
    // We need a context but it is already checked by caller
    // We need a property set but it is already checked by caller
    // We need a component but it is already checked by caller

    // open stream
    Reference<io::XStream> xStream;
    try
    {
        xStream = xStorage->openStreamElement(
            OUString(pStreamName), embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE);
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("starmath", "Can't create output stream in package");
        return false;
    }

    // Set stream as text / xml
    uno::Reference<beans::XPropertySet> xSet(xStream, uno::UNO_QUERY);
    xSet->setPropertyValue("MediaType", Any(OUString(u"text/xml")));

    // all streams must be encrypted in encrypted document
    xSet->setPropertyValue("UseCommonStoragePasswordEncryption", Any(true));

    // set Base URL
    rPropSet->setPropertyValue("StreamName", makeAny(OUString(pStreamName)));

    // write the stuff
    // Note: export through an XML exporter component (output stream version)
    return WriteThroughComponentOS(xStream->getOutputStream(), xComponent, rxContext, rPropSet,
                                   pComponentName);
}

// export through an XML exporter component (memory stream version)
OUString
SmMlExportWrapper::WriteThroughComponentMS(const Reference<XComponent>& xComponent,
                                           Reference<uno::XComponentContext> const& rxContext,
                                           Reference<beans::XPropertySet> const& rPropSet)
{
    // We need a component but it is already checked by caller
    // We need a context but it is already checked by caller
    // We need a property set it is already checked by caller

    // open stream
    SvMemoryStream aMemoryStream(8192, 1024);
    uno::Reference<io::XOutputStream> xStream(new utl::OOutputStreamWrapper(aMemoryStream));

    // Set the stream as text
    uno::Reference<beans::XPropertySet> xSet(xStream, uno::UNO_QUERY);
    xSet->setPropertyValue("MediaType", Any(OUString("text/xml")));

    // write the stuff
    // Note: export through an XML exporter component (output stream version)
    bool bOk = WriteThroughComponentOS(xStream, xComponent, rxContext, rPropSet,
                                       u"com.sun.star.comp.Math.XMLContentExporter");

    // We don't want to read unitzialized data
    if (!bOk)
        return OUString(u"");

    // Recover data and generate string
    OString aString(static_cast<const char*>(aMemoryStream.GetData()),
                    aMemoryStream.GetSize() / sizeof(char));
    return OStringToOUString(aString, RTL_TEXTENCODING_UTF8);
}

// SmMlExport
/*************************************************************************************************/

void SmMlExport::declareMlError()
{
    SAL_WARN("starmath", "Invalid use of mathml.");
    m_bSuccess = false;
}

void SmMlExport::exportMlAttributteLength(xmloff::token::XMLTokenEnum pAttribute,
                                          const SmLengthValue& aLengthValue)
{
    if (!aLengthValue.m_aOriginalText->isEmpty())
    {
        addAttribute(pAttribute, *aLengthValue.m_aOriginalText);
    }
    else
    {
        OUStringBuffer aSizeBuffer(64);
        aSizeBuffer.append(aLengthValue.m_aLengthValue);
        switch (aLengthValue.m_aLengthUnit)
        {
            case SmLengthUnit::MlEm:
                aSizeBuffer.append(u"em");
                break;
            case SmLengthUnit::MlEx:
                aSizeBuffer.append(u"ex");
                break;
            case SmLengthUnit::MlPx:
                aSizeBuffer.append(u"px");
                break;
            case SmLengthUnit::MlIn:
                aSizeBuffer.append(u"in");
                break;
            case SmLengthUnit::MlCm:
                aSizeBuffer.append(u"cm");
                break;
            case SmLengthUnit::MlMm:
                aSizeBuffer.append(u"mm");
                break;
            case SmLengthUnit::MlPt:
                aSizeBuffer.append(u"pt");
                break;
            case SmLengthUnit::MlPc:
                aSizeBuffer.append(u"pc");
                break;
            case SmLengthUnit::MlP:
                aSizeBuffer.append(u"%");
                break;
            case SmLengthUnit::MlM:
                break;
            default:
                declareMlError();
                break;
        }
        addAttribute(pAttribute, aSizeBuffer.makeStringAndClear());
    }
}

void SmMlExport::exportMlAttributtes(const SmMlElement* pMlElement)
{
    size_t nAttributeCount = pMlElement->getAttributeCount();
    for (size_t i = 0; i < nAttributeCount; ++i)
    {
        SmMlAttribute aAttribute = pMlElement->getAttribute(i);
        switch (aAttribute.getMlAttributeValueType())
        {
            case SmMlAttributeValueType::MlAccent:
            {
                auto aAttributeValue = aAttribute.getMlAccent();
                switch (aAttributeValue->m_aAccent)
                {
                    case SmMlAttributeValueAccent::MlFalse:
                        addAttribute(XML_ACCENT, XML_FALSE);
                        break;
                    case SmMlAttributeValueAccent::MlTrue:
                        addAttribute(XML_ACCENT, XML_TRUE);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case SmMlAttributeValueType::MlDir:
            {
                auto aAttributeValue = aAttribute.getMlDir();
                switch (aAttributeValue->m_aDir)
                {
                    case SmMlAttributeValueDir::MlLtr:
                        addAttribute(XML_DIR, XML_LTR);
                        break;
                    case SmMlAttributeValueDir::MlRtl:
                        addAttribute(XML_DIR, XML_RTL);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case SmMlAttributeValueType::MlDisplaystyle:
            {
                auto aAttributeValue = aAttribute.getMlDisplaystyle();
                switch (aAttributeValue->m_aDisplaystyle)
                {
                    case SmMlAttributeValueDisplaystyle::MlTrue:
                        addAttribute(XML_DISPLAYSTYLE, XML_FALSE);
                        break;
                    case SmMlAttributeValueDisplaystyle::MlFalse:
                        addAttribute(XML_DISPLAYSTYLE, XML_TRUE);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case SmMlAttributeValueType::MlFence:
            {
                auto aAttributeValue = aAttribute.getMlFence();
                switch (aAttributeValue->m_aFence)
                {
                    case SmMlAttributeValueFence::MlTrue:
                        addAttribute(XML_FENCE, XML_FALSE);
                        break;
                    case SmMlAttributeValueFence::MlFalse:
                        addAttribute(XML_FENCE, XML_TRUE);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case SmMlAttributeValueType::MlHref:
            {
                auto aAttributeValue = aAttribute.getMlHref();
                switch (aAttributeValue->m_aHref)
                {
                    case SmMlAttributeValueHref::NMlEmpty:
                        break;
                    case SmMlAttributeValueHref::NMlValid:
                        addAttribute(XML_FENCE, *aAttributeValue->m_aLnk);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case SmMlAttributeValueType::MlLspace:
            {
                auto aSizeData = aAttribute.getMlLspace();
                auto aLengthData = aSizeData->m_aLengthValue;
                exportMlAttributteLength(XML_MATHSIZE, aLengthData);
                break;
            }
            case SmMlAttributeValueType::MlMathbackground:
            {
                auto aAttributeValue = aAttribute.getMlMathbackground();
                switch (aAttributeValue->m_aMathbackground)
                {
                    case SmMlAttributeValueMathbackground::MlTransparent:
                        addAttribute(XML_MATHBACKGROUND, "transparent");
                        break;
                    case SmMlAttributeValueMathbackground::MlRgb:
                    {
                        OUString aTextColor = OUString::createFromAscii(
                            starmathdatabase::Identify_Color_MATHML(
                                static_cast<uint32_t>(aAttributeValue->m_aCol))
                                .pIdent);
                        addAttribute(XML_MATHBACKGROUND, aTextColor);
                        break;
                    }
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case SmMlAttributeValueType::MlMathcolor:
            {
                auto aAttributeValue = aAttribute.getMlMathcolor();
                switch (aAttributeValue->m_aMathcolor)
                {
                    case SmMlAttributeValueMathcolor::MlDefault:
                        break;
                    case SmMlAttributeValueMathcolor::MlRgb:
                    {
                        OUString aTextColor = OUString::createFromAscii(
                            starmathdatabase::Identify_Color_MATHML(
                                static_cast<uint32_t>(aAttributeValue->m_aCol))
                                .pIdent);
                        addAttribute(XML_MATHCOLOR, aTextColor);
                        break;
                    }
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case SmMlAttributeValueType::MlMathsize:
            {
                auto aSizeData = aAttribute.getMlMathsize();
                auto aLengthData = aSizeData->m_aLengthValue;
                exportMlAttributteLength(XML_MATHSIZE, aLengthData);
                break;
            }
            case SmMlAttributeValueType::MlMathvariant:
            {
                auto aAttributeValue = aAttribute.getMlMathvariant();
                switch (aAttributeValue->m_aMathvariant)
                {
                    case SmMlAttributeValueMathvariant::normal:
                        addAttribute(XML_MATHVARIANT, "normal");
                        break;
                    case SmMlAttributeValueMathvariant::bold:
                        addAttribute(XML_MATHVARIANT, "bold");
                        break;
                    case SmMlAttributeValueMathvariant::italic:
                        addAttribute(XML_MATHVARIANT, "italic");
                        break;
                    case SmMlAttributeValueMathvariant::double_struck:
                        addAttribute(XML_MATHVARIANT, "double-struck");
                        break;
                    case SmMlAttributeValueMathvariant::script:
                        addAttribute(XML_MATHVARIANT, "script");
                        break;
                    case SmMlAttributeValueMathvariant::fraktur:
                        addAttribute(XML_MATHVARIANT, "fraktur");
                        break;
                    case SmMlAttributeValueMathvariant::sans_serif:
                        addAttribute(XML_MATHVARIANT, "sans-serif");
                        break;
                    case SmMlAttributeValueMathvariant::monospace:
                        addAttribute(XML_MATHVARIANT, "monospace");
                        break;
                    case SmMlAttributeValueMathvariant::bold_italic:
                        addAttribute(XML_MATHVARIANT, "bold-italic");
                        break;
                    case SmMlAttributeValueMathvariant::bold_fraktur:
                        addAttribute(XML_MATHVARIANT, "bold-fracktur");
                        break;
                    case SmMlAttributeValueMathvariant::bold_script:
                        addAttribute(XML_MATHVARIANT, "bold-script");
                        break;
                    case SmMlAttributeValueMathvariant::bold_sans_serif:
                        addAttribute(XML_MATHVARIANT, "bold-sans-serif");
                        break;
                    case SmMlAttributeValueMathvariant::sans_serif_italic:
                        addAttribute(XML_MATHVARIANT, "sans-serif-italic");
                        break;
                    case SmMlAttributeValueMathvariant::sans_serif_bold_italic:
                        addAttribute(XML_MATHVARIANT, "sans-serif-bold-italic");
                        break;
                    case SmMlAttributeValueMathvariant::initial:
                        addAttribute(XML_MATHVARIANT, "initial");
                        break;
                    case SmMlAttributeValueMathvariant::tailed:
                        addAttribute(XML_MATHVARIANT, "tailed");
                        break;
                    case SmMlAttributeValueMathvariant::looped:
                        addAttribute(XML_MATHVARIANT, "looped");
                        break;
                    case SmMlAttributeValueMathvariant::stretched:
                        addAttribute(XML_MATHVARIANT, "stretched");
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case SmMlAttributeValueType::MlMaxsize:
            {
                auto aSizeData = aAttribute.getMlMaxsize();
                auto aLengthData = aSizeData->m_aLengthValue;
                switch (aSizeData->m_aMaxsize)
                {
                    case SmMlAttributeValueMaxsize::MlInfinity:
                    {
                        addAttribute(XML_MAXSIZE, OUString(u"infinity"));
                        break;
                    }
                    case SmMlAttributeValueMaxsize::MlFinite:
                    {
                        exportMlAttributteLength(XML_MAXSIZE, aLengthData);
                        break;
                    }
                }
                break;
            }
            case SmMlAttributeValueType::MlMinsize:
            {
                auto aSizeData = aAttribute.getMlMinsize();
                auto aLengthData = aSizeData->m_aLengthValue;
                exportMlAttributteLength(XML_MINSIZE, aLengthData);
                break;
            }
            case SmMlAttributeValueType::MlMovablelimits:
            {
                auto aAttributeValue = aAttribute.getMlMovablelimits();
                switch (aAttributeValue->m_aMovablelimits)
                {
                    case SmMlAttributeValueMovablelimits::MlFalse:
                        addAttribute(XML_MOVABLELIMITS, XML_FALSE);
                        break;
                    case SmMlAttributeValueMovablelimits::MlTrue:
                        addAttribute(XML_MOVABLELIMITS, XML_TRUE);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case SmMlAttributeValueType::MlRspace:
            {
                auto aSizeData = aAttribute.getMlRspace();
                auto aLengthData = aSizeData->m_aLengthValue;
                exportMlAttributteLength(XML_MATHSIZE, aLengthData);
                break;
            }
            case SmMlAttributeValueType::MlSeparator:
            {
                auto aAttributeValue = aAttribute.getMlSeparator();
                switch (aAttributeValue->m_aSeparator)
                {
                    case SmMlAttributeValueSeparator::MlFalse:
                        addAttribute(XML_SEPARATOR, XML_FALSE);
                        break;
                    case SmMlAttributeValueSeparator::MlTrue:
                        addAttribute(XML_SEPARATOR, XML_TRUE);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case SmMlAttributeValueType::MlStretchy:
            {
                auto aAttributeValue = aAttribute.getMlStretchy();
                switch (aAttributeValue->m_aStretchy)
                {
                    case SmMlAttributeValueStretchy::MlFalse:
                        addAttribute(XML_STRETCHY, XML_FALSE);
                        break;
                    case SmMlAttributeValueStretchy::MlTrue:
                        addAttribute(XML_STRETCHY, XML_TRUE);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case SmMlAttributeValueType::MlSymmetric:
            {
                auto aAttributeValue = aAttribute.getMlSymmetric();
                switch (aAttributeValue->m_aSymmetric)
                {
                    case SmMlAttributeValueSymmetric::MlFalse:
                        addAttribute(XML_SYMMETRIC, XML_FALSE);
                        break;
                    case SmMlAttributeValueSymmetric::MlTrue:
                        addAttribute(XML_SYMMETRIC, XML_TRUE);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            default:
                declareMlError();
                break;
        }
    }
}

void SmMlExport::exportMlElement(const SmMlElement* pMlElement)
{
    SvXMLElementExport* pElementExport;
    switch (pMlElement->getMlElementType())
    {
        case SmMlElementType::MlMath:
            pElementExport = createElementExport(XML_MATH);
            break;
        case SmMlElementType::MlMi:
            pElementExport = createElementExport(XML_MI);
            break;
        case SmMlElementType::MlMerror:
            pElementExport = createElementExport(XML_MERROR);
            break;
        case SmMlElementType::MlMn:
            pElementExport = createElementExport(XML_MN);
            break;
        case SmMlElementType::MlMo:
            pElementExport = createElementExport(XML_MO);
            break;
        case SmMlElementType::MlMrow:
            pElementExport = createElementExport(XML_MROW);
            break;
        case SmMlElementType::MlMtext:
            pElementExport = createElementExport(XML_MTEXT);
            break;
        case SmMlElementType::MlMstyle:
            pElementExport = createElementExport(XML_MSTYLE);
            break;
        default:
            pElementExport = nullptr;
    }
    exportMlAttributtes(pMlElement);
    const OUString& aElementText = pMlElement->getText();
    if (aElementText.isEmpty())
        GetDocHandler()->characters(aElementText);
    (void)pElementExport;
}

SmMlExport::SmMlExport(const css::uno::Reference<css::uno::XComponentContext>& rContext,
                       OUString const& implementationName, SvXMLExportFlags nExportFlags)
    : SvXMLExport(rContext, implementationName, util::MeasureUnit::INCH, XML_MATH, nExportFlags)
    , m_pTree(nullptr)
    , m_bSuccess(true)
    , m_bUseExportTag(false)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

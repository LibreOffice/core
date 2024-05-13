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

// Our mathml
#include <mathml/export.hxx>
#include <mathml/iterator.hxx>

// LO tools to use
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>

// Extra LO tools
#include <comphelper/genericpropertyset.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <comphelper/servicehelper.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <unotools/streamwrap.hxx>
#include <xmloff/namespacemap.hxx>

// Our starmath tools
#include <document.hxx>
#include <smmod.hxx>
#include <strings.hrc>
#include <unomodel.hxx>
#include <xparsmlbase.hxx>
#include <starmathdatabase.hxx>

// Old parser
#include <mathmlexport.hxx>

using namespace ::com::sun::star;
using namespace xmloff::token;

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

// SmMLExportWrapper
/*************************************************************************************************/

bool SmMLExportWrapper::Export(SfxMedium& rMedium)
{
    bool bRet = true;
    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());

    // Check all fine
    SAL_WARN_IF(m_xModel == nullptr, "starmath", "Missing model");
    SAL_WARN_IF(xContext == nullptr, "starmath", "Missing context");
    if (m_xModel == nullptr || xContext == nullptr)
        return false;

    // Get doc shell
    SmDocShell* pDocShell = static_cast<SmDocShell*>(m_xModel->GetObjectShell());
    if (pDocShell == nullptr)
    {
        SAL_WARN("starmath", "Failed to fetch sm document");
        return false;
    }

    // Check if it is a standalone window or embed object
    bool bEmbedded = SfxObjectCreateMode::EMBEDDED == pDocShell->GetCreateMode();

    // Medium item set
    SfxItemSet& rMediumItemSet = rMedium.GetItemSet();
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
            //return false;
        }

        // Fetch progress bar
        const SfxUnoAnyItem* pItem = rMediumItemSet.GetItem(SID_PROGRESS_STATUSBAR_CONTROL);
        if (pItem)
        {
            // set progress range and start status indicator
            pItem->GetValue() >>= xStatusIndicator;
            xStatusIndicator->start(SmResId(STR_STATSTR_WRITING), 3);
            xStatusIndicator->setValue(0);
        }
    }

    // create XPropertySet with three properties for status indicator
    static const comphelper::PropertyMapEntry aInfoMap[]{
        { u"UsePrettyPrinting"_ustr, 0, cppu::UnoType<bool>::get(),
          beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"BaseURI"_ustr, 0, ::cppu::UnoType<OUString>::get(), beans::PropertyAttribute::MAYBEVOID,
          0 },
        { u"StreamRelPath"_ustr, 0, ::cppu::UnoType<OUString>::get(),
          beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"StreamName"_ustr, 0, ::cppu::UnoType<OUString>::get(),
          beans::PropertyAttribute::MAYBEVOID, 0 }
    };
    uno::Reference<beans::XPropertySet> xInfoSet(
        comphelper::GenericPropertySet_CreateInstance(new comphelper::PropertySetInfo(aInfoMap)));

    // Always print pretty
    xInfoSet->setPropertyValue(u"UsePrettyPrinting"_ustr, Any(true));

    // Set base URI
    xInfoSet->setPropertyValue(u"BaseURI"_ustr, Any(rMedium.GetBaseURL(true)));

    if (!m_bFlat) //Storage (Package) of Stream
    {
        // Fetch the output storage
        uno::Reference<embed::XStorage> xStg = rMedium.GetOutputStorage();
        if (xStg == nullptr)
        {
            SAL_WARN("starmath", "Failed to fetch output storage");
            return false;
        }

        // TODO/LATER: handle the case of embedded links gracefully
        if (bEmbedded) //&& !pStg->IsRoot() )
        {
            const SfxStringItem* pDocHierarchItem
                = rMediumItemSet.GetItem(SID_DOC_HIERARCHICALNAME);
            if (pDocHierarchItem != nullptr)
            {
                OUString aName = pDocHierarchItem->GetValue();
                if (!aName.isEmpty())
                    xInfoSet->setPropertyValue(u"StreamRelPath"_ustr, Any(aName));
            }
        }
        else
        {
            // Write file metadata ( data, LO version ... )
            // Note: export through an XML exporter component (storage version)
            if (xStatusIndicator.is())
                xStatusIndicator->setValue(1);

            bRet = WriteThroughComponentS(xStg, m_xModel, u"meta.xml", xContext, xInfoSet,
                                          u"com.sun.star.comp.Math.MLOasisMetaExporter", 6);
        }

        // Write starmath formula
        // Note: export through an XML exporter component (storage version)
        if (bRet)
        {
            if (xStatusIndicator.is())
                xStatusIndicator->setValue(2);

            if (pDocShell->GetSmSyntaxVersion() == 5)
                bRet = WriteThroughComponentS(xStg, m_xModel, u"content.xml", xContext, xInfoSet,
                                              u"com.sun.star.comp.Math.XMLContentExporter", 5);
            else
                bRet = WriteThroughComponentS(xStg, m_xModel, u"content.xml", xContext, xInfoSet,
                                              u"com.sun.star.comp.Math.MLContentExporter", 6);
        }

        // Write starmath settings
        // Note: export through an XML exporter component (storage version)
        if (bRet)
        {
            if (xStatusIndicator.is())
                xStatusIndicator->setValue(3);

            bRet = WriteThroughComponentS(xStg, m_xModel, u"settings.xml", xContext, xInfoSet,
                                          u"com.sun.star.comp.Math.MLOasisSettingsExporter", 6);
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
        if (pDocShell->GetSmSyntaxVersion() == 5)
            bRet = WriteThroughComponentOS(xOut, m_xModel, xContext, xInfoSet,
                                           u"com.sun.star.comp.Math.XMLContentExporter", 5);
        else
            bRet = WriteThroughComponentOS(xOut, m_xModel, xContext, xInfoSet,
                                           u"com.sun.star.comp.Math.MLContentExporter", 6);
    }

    if (xStatusIndicator.is())
        xStatusIndicator->end();
    return bRet;
}

OUString SmMLExportWrapper::Export(SmMlElement* pElementTree)
{
    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());

    // Check all fine
    m_pElementTree = nullptr;
    SAL_WARN_IF(m_xModel == nullptr, "starmath", "Missing model");
    SAL_WARN_IF(xContext == nullptr, "starmath", "Missing context");
    if (m_xModel == nullptr || xContext == nullptr)
        return u""_ustr;

    //Get model
    uno::Reference<lang::XComponent> xModelComp = m_xModel;
    SAL_WARN_IF(xModelComp == nullptr, "starmath", "Missing model component");
    SmModel* pModel = m_xModel.get();
    SAL_WARN_IF(pModel == nullptr, "starmath", "Failed to get threw uno tunnel");
    if (xModelComp == nullptr || pModel == nullptr)
        return u""_ustr;

    // Get doc shell
    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    if (pDocShell == nullptr)
    {
        SAL_WARN("starmath", "Failed to fetch sm document");
        return u""_ustr;
    }

    // create XPropertySet with three properties for status indicator
    static const comphelper::PropertyMapEntry aInfoMap[]{
        { u"UsePrettyPrinting"_ustr, 0, cppu::UnoType<bool>::get(),
          beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"BaseURI"_ustr, 0, ::cppu::UnoType<OUString>::get(), beans::PropertyAttribute::MAYBEVOID,
          0 },
        { u"StreamRelPath"_ustr, 0, ::cppu::UnoType<OUString>::get(),
          beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"StreamName"_ustr, 0, ::cppu::UnoType<OUString>::get(),
          beans::PropertyAttribute::MAYBEVOID, 0 }
    };
    uno::Reference<beans::XPropertySet> xInfoSet(
        comphelper::GenericPropertySet_CreateInstance(new comphelper::PropertySetInfo(aInfoMap)));

    // Always print pretty
    xInfoSet->setPropertyValue(u"UsePrettyPrinting"_ustr, Any(true));

    // Fetch mathml tree
    m_pElementTree = pElementTree;

    // Write stuff
    // Note: export through an XML exporter component (memory stream version)
    return WriteThroughComponentMS(xModelComp, xContext, xInfoSet);
}

// export through an XML exporter component (output stream version)
bool SmMLExportWrapper::WriteThroughComponentOS(const Reference<io::XOutputStream>& xOutputStream,
                                                const Reference<XComponent>& xComponent,
                                                Reference<uno::XComponentContext> const& rxContext,
                                                Reference<beans::XPropertySet> const& rPropSet,
                                                const char16_t* pComponentName,
                                                int_fast16_t nSyntaxVersion)
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
    Sequence<Any> aArgs{ Any(xSaxWriter), Any(rPropSet) };

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
    Reference<XFilter> xFilter(xExporter, UNO_QUERY);
    uno::Sequence<PropertyValue> aProps(0);

    // filter
    if (nSyntaxVersion == 5)
    {
        SmXMLExport* pFilter = dynamic_cast<SmXMLExport*>(xFilter.get());
        if (pFilter == nullptr)
        {
            SAL_WARN("starmath", "Failed to fetch SmMLExport");
            return false;
        }
        xFilter->filter(aProps);
        return pFilter->GetSuccess();
    }

    // filter
    SmMLExport* pFilter = dynamic_cast<SmMLExport*>(xFilter.get());

    // Setup filter
    if (pFilter == nullptr)
    {
        SAL_WARN("starmath", "Failed to fetch SmMLExport");
        return false;
    }
    pFilter->setUseExportTag(m_bUseExportTag);
    pFilter->setElementTree(m_pElementTree);

    // Execute operation
    xFilter->filter(aProps);
    return pFilter->getSuccess();
}

// export through an XML exporter component (storage version)
bool SmMLExportWrapper::WriteThroughComponentS(const Reference<embed::XStorage>& xStorage,
                                               const Reference<XComponent>& xComponent,
                                               const char16_t* pStreamName,
                                               Reference<uno::XComponentContext> const& rxContext,
                                               Reference<beans::XPropertySet> const& rPropSet,
                                               const char16_t* pComponentName,
                                               int_fast16_t nSyntaxVersion)
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
        SAL_WARN("starmath", "Can't create output stream in package");
        return false;
    }

    // Set stream as text / xml
    uno::Reference<beans::XPropertySet> xSet(xStream, uno::UNO_QUERY);
    xSet->setPropertyValue(u"MediaType"_ustr, Any(u"text/xml"_ustr));

    // all streams must be encrypted in encrypted document
    xSet->setPropertyValue(u"UseCommonStoragePasswordEncryption"_ustr, Any(true));

    // set Base URL
    rPropSet->setPropertyValue(u"StreamName"_ustr, Any(OUString(pStreamName)));

    // write the stuff
    // Note: export through an XML exporter component (output stream version)
    return WriteThroughComponentOS(xStream->getOutputStream(), xComponent, rxContext, rPropSet,
                                   pComponentName, nSyntaxVersion);
}

// export through an XML exporter component (memory stream version)
OUString
SmMLExportWrapper::WriteThroughComponentMS(const Reference<XComponent>& xComponent,
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
    xSet->setPropertyValue(u"MediaType"_ustr, Any(u"text/xml"_ustr));

    // write the stuff
    // Note: export through an XML exporter component (output stream version)
    bool bOk = WriteThroughComponentOS(xStream, xComponent, rxContext, rPropSet,
                                       u"com.sun.star.comp.Mathml.MLContentExporter", 6);

    // We don't want to read uninitialized data
    if (!bOk)
        return u""_ustr;

    // Recover data and generate string
    OString aString(static_cast<const char*>(aMemoryStream.GetData()),
                    aMemoryStream.GetSize() / sizeof(char));
    return OStringToOUString(aString, RTL_TEXTENCODING_UTF8);
}

// SmMLExport technical
/*************************************************************************************************/

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Math_MLExporter_get_implementation(css::uno::XComponentContext* context,
                                   css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new SmMLExport(context, u"com.sun.star.comp.Math.XMLExporter"_ustr,
                                        SvXMLExportFlags::OASIS | SvXMLExportFlags::ALL));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Math_MLOasisMetaExporter_get_implementation(css::uno::XComponentContext* context,
                                            css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new SmMLExport(context,
                                        u"com.sun.star.comp.Math.XMLOasisMetaExporter"_ustr,
                                        SvXMLExportFlags::OASIS | SvXMLExportFlags::META));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Math_MLOasisSettingsExporter_get_implementation(css::uno::XComponentContext* context,
                                                css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new SmMLExport(context,
                                        u"com.sun.star.comp.Math.XMLOasisSettingsExporter"_ustr,
                                        SvXMLExportFlags::OASIS | SvXMLExportFlags::SETTINGS));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Math_MLContentExporter_get_implementation(css::uno::XComponentContext* context,
                                          css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new SmMLExport(context, u"com.sun.star.comp.Math.XMLContentExporter"_ustr,
                                        SvXMLExportFlags::OASIS | SvXMLExportFlags::CONTENT));
}

SmDocShell* SmMLExport::getSmDocShell()
{
    SmModel* pModel = comphelper::getFromUnoTunnel<SmModel>(GetModel());
    if (pModel != nullptr)
        return static_cast<SmDocShell*>(pModel->GetObjectShell());
    return nullptr;
}

ErrCode SmMLExport::exportDoc(enum XMLTokenEnum eClass)
{
    if (!(getExportFlags() & SvXMLExportFlags::CONTENT))
    {
        // Everything that isn't the formula itself get's default export
        SvXMLExport::exportDoc(eClass);
        return ERRCODE_NONE;
    }

    // Checks if it has to export a particular tree
    if (m_pElementTree == nullptr)
    {
        // Set element tree
        SmDocShell* pDocShell = getSmDocShell();
        if (pDocShell != nullptr)
            m_pElementTree = pDocShell->GetMlElementTree();
        else
        {
            m_bSuccess = false;
            return SVSTREAM_INVALID_PARAMETER;
        }
    }

    // Start document and encrypt if necessary
    GetDocHandler()->startDocument();
    addChaffWhenEncryptedStorage();

    // make use of a default namespace
    // Math doesn't need namespaces from xmloff, since it now uses default namespaces
    // Because that is common with current MathML usage in the web -> ResetNamespaceMap();
    GetNamespaceMap_().Add(u""_ustr, GetXMLToken(XML_N_MATH), XML_NAMESPACE_MATH);

    // Add xmlns line
    if (m_bUseExportTag)
    {
        GetAttrList().AddAttribute(GetNamespaceMap().GetAttrNameByKey(XML_NAMESPACE_MATH),
                                   GetNamespaceMap().GetNameByKey(XML_NAMESPACE_MATH));
    }

    // Export and close document
    ExportContent_();
    GetDocHandler()->endDocument();

    return ERRCODE_NONE;
}

void SmMLExport::GetViewSettings(Sequence<PropertyValue>& aProps)
{
    // Get the document shell
    SmDocShell* pDocShell = getSmDocShell();
    if (pDocShell == nullptr)
    {
        SAL_WARN("starmath", "Missing document shell so no view settings");
        return;
    }

    // Allocate enough memory
    aProps.realloc(4);
    PropertyValue* pValue = aProps.getArray();

    // The view settings are the formula display settings
    tools::Rectangle aRect(pDocShell->GetVisArea());

    pValue[0].Name = "ViewAreaTop";
    pValue[0].Value <<= aRect.Top();

    pValue[1].Name = "ViewAreaLeft";
    pValue[1].Value <<= aRect.Left();

    pValue[2].Name = "ViewAreaWidth";
    pValue[2].Value <<= aRect.GetWidth();

    pValue[3].Name = "ViewAreaHeight";
    pValue[3].Value <<= aRect.GetHeight();
}

void SmMLExport::GetConfigurationSettings(Sequence<PropertyValue>& rProps)
{
    // Get model property set (settings)
    Reference<XPropertySet> xProps(GetModel(), UNO_QUERY);
    if (!xProps.is())
    {
        SAL_WARN("starmath", "Missing model properties so no configuration settings");
        return;
    }

    // Get model property set info (settings values)
    Reference<XPropertySetInfo> xPropertySetInfo = xProps->getPropertySetInfo();
    if (!xPropertySetInfo.is())
    {
        SAL_WARN("starmath", "Missing model properties info so no configuration settings");
        return;
    }

    // Allocate to store the properties
    Sequence<Property> aProps = xPropertySetInfo->getProperties();
    const sal_Int32 nCount = aProps.getLength();
    rProps.realloc(nCount);
    auto pProps = rProps.getArray();

    // Copy properties
    // This needs further revision
    // Based in code mathmlexport.cxx::GetConfigurationSettings
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        if (aProps[i].Name != "Formula" && aProps[i].Name != "BasicLibraries"
            && aProps[i].Name != "DialogLibraries" && aProps[i].Name != "RuntimeUID")
        {
            pProps[i].Name = aProps[i].Name;
            pProps[i].Value = xProps->getPropertyValue(aProps[i].Name);
        }
    }
}

SmMLExport::SmMLExport(const css::uno::Reference<css::uno::XComponentContext>& rContext,
                       OUString const& implementationName, SvXMLExportFlags nExportFlags)
    : SvXMLExport(rContext, implementationName, util::MeasureUnit::INCH, XML_MATH, nExportFlags)
    , m_pElementTree(nullptr)
    , m_bSuccess(true)
    , m_bUseExportTag(true)
{
}

// SmMLExport
/*************************************************************************************************/

void SmMLExport::declareMlError()
{
    SAL_WARN("starmath", "Invalid use of mathml.");
    m_bSuccess = false;
}

void SmMLExport::exportMlAttributeLength(xmloff::token::XMLTokenEnum pAttribute,
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

void SmMLExport::exportMlAttributes(const SmMlElement* pMlElement)
{
    size_t nAttributeCount = pMlElement->getAttributeCount();
    for (size_t i = 0; i < nAttributeCount; ++i)
    {
        SmMlAttribute aAttribute = pMlElement->getAttribute(i);
        if (!aAttribute.isSet())
            continue;

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
                        addAttribute(XML_HREF, *aAttributeValue->m_aLnk);
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
                exportMlAttributeLength(XML_LSPACE, aLengthData);
                break;
            }
            case SmMlAttributeValueType::MlMathbackground:
            {
                auto aAttributeValue = aAttribute.getMlMathbackground();
                switch (aAttributeValue->m_aMathbackground)
                {
                    case SmMlAttributeValueMathbackground::MlTransparent:
                        addAttribute(XML_MATHBACKGROUND, u"transparent"_ustr);
                        break;
                    case SmMlAttributeValueMathbackground::MlRgb:
                    {
                        const OUString& rTextColor = starmathdatabase::Identify_Color_MATHML(
                                                         sal_uInt32(aAttributeValue->m_aCol))
                                                         .aIdent;
                        addAttribute(XML_MATHBACKGROUND, rTextColor);
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
                        const OUString& rTextColor = starmathdatabase::Identify_Color_MATHML(
                                                         sal_uInt32(aAttributeValue->m_aCol))
                                                         .aIdent;
                        addAttribute(XML_MATHCOLOR, rTextColor);
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
                exportMlAttributeLength(XML_MATHSIZE, aLengthData);
                break;
            }
            case SmMlAttributeValueType::MlMathvariant:
            {
                auto aAttributeValue = aAttribute.getMlMathvariant();
                switch (aAttributeValue->m_aMathvariant)
                {
                    case SmMlAttributeValueMathvariant::normal:
                        addAttribute(XML_MATHVARIANT, u"normal"_ustr);
                        break;
                    case SmMlAttributeValueMathvariant::bold:
                        addAttribute(XML_MATHVARIANT, u"bold"_ustr);
                        break;
                    case SmMlAttributeValueMathvariant::italic:
                        addAttribute(XML_MATHVARIANT, u"italic"_ustr);
                        break;
                    case SmMlAttributeValueMathvariant::double_struck:
                        addAttribute(XML_MATHVARIANT, u"double-struck"_ustr);
                        break;
                    case SmMlAttributeValueMathvariant::script:
                        addAttribute(XML_MATHVARIANT, u"script"_ustr);
                        break;
                    case SmMlAttributeValueMathvariant::fraktur:
                        addAttribute(XML_MATHVARIANT, u"fraktur"_ustr);
                        break;
                    case SmMlAttributeValueMathvariant::sans_serif:
                        addAttribute(XML_MATHVARIANT, u"sans-serif"_ustr);
                        break;
                    case SmMlAttributeValueMathvariant::monospace:
                        addAttribute(XML_MATHVARIANT, u"monospace"_ustr);
                        break;
                    case SmMlAttributeValueMathvariant::bold_italic:
                        addAttribute(XML_MATHVARIANT, u"bold-italic"_ustr);
                        break;
                    case SmMlAttributeValueMathvariant::bold_fraktur:
                        addAttribute(XML_MATHVARIANT, u"bold-fracktur"_ustr);
                        break;
                    case SmMlAttributeValueMathvariant::bold_script:
                        addAttribute(XML_MATHVARIANT, u"bold-script"_ustr);
                        break;
                    case SmMlAttributeValueMathvariant::bold_sans_serif:
                        addAttribute(XML_MATHVARIANT, u"bold-sans-serif"_ustr);
                        break;
                    case SmMlAttributeValueMathvariant::sans_serif_italic:
                        addAttribute(XML_MATHVARIANT, u"sans-serif-italic"_ustr);
                        break;
                    case SmMlAttributeValueMathvariant::sans_serif_bold_italic:
                        addAttribute(XML_MATHVARIANT, u"sans-serif-bold-italic"_ustr);
                        break;
                    case SmMlAttributeValueMathvariant::initial:
                        addAttribute(XML_MATHVARIANT, u"initial"_ustr);
                        break;
                    case SmMlAttributeValueMathvariant::tailed:
                        addAttribute(XML_MATHVARIANT, u"tailed"_ustr);
                        break;
                    case SmMlAttributeValueMathvariant::looped:
                        addAttribute(XML_MATHVARIANT, u"looped"_ustr);
                        break;
                    case SmMlAttributeValueMathvariant::stretched:
                        addAttribute(XML_MATHVARIANT, u"stretched"_ustr);
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
                        addAttribute(XML_MAXSIZE, XML_INFINITY);
                        break;
                    }
                    case SmMlAttributeValueMaxsize::MlFinite:
                    {
                        exportMlAttributeLength(XML_MAXSIZE, aLengthData);
                        break;
                    }
                }
                break;
            }
            case SmMlAttributeValueType::MlMinsize:
            {
                auto aSizeData = aAttribute.getMlMinsize();
                auto aLengthData = aSizeData->m_aLengthValue;
                exportMlAttributeLength(XML_MINSIZE, aLengthData);
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
                exportMlAttributeLength(XML_RSPACE, aLengthData);
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

SvXMLElementExport* SmMLExport::exportMlElement(const SmMlElement* pMlElement)
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
    const OUString& aElementText = pMlElement->getText();
    exportMlAttributes(pMlElement);
    if (aElementText.isEmpty())
        GetDocHandler()->characters(aElementText);
    return pElementExport;
}

namespace
{
struct exportMlElementTreeExecData
{
private:
    SmMLExport* m_pSmMLExport;
    std::vector<SvXMLElementExport*> m_aSvXMLElementExportList;
    size_t m_nDepth;

public:
    inline exportMlElementTreeExecData(SmMLExport* pSmMLExport)
        : m_pSmMLExport(pSmMLExport)
        , m_aSvXMLElementExportList(1024)
        , m_nDepth(0)
    {
    }

    inline void deleteDepthData()
    {
        delete m_aSvXMLElementExportList[m_nDepth];
        --m_nDepth;
    }

    inline void setDepthData(SvXMLElementExport* aSvXMLElementExportList)
    {
        if (m_nDepth == m_aSvXMLElementExportList.size())
            m_aSvXMLElementExportList.resize(m_aSvXMLElementExportList.size() + 1024);
        m_aSvXMLElementExportList[m_nDepth] = aSvXMLElementExportList;
    }

    inline void incrementDepth() { ++m_nDepth; }

    inline SmMLExport* getSmMLExport() { return m_pSmMLExport; };
};

} // end unnamed namespace

static inline void exportMlElementTreeExec(SmMlElement* aSmMlElement, void* aData)
{
    // Prepare data
    exportMlElementTreeExecData* pData = static_cast<exportMlElementTreeExecData*>(aData);
    pData->setDepthData(pData->getSmMLExport()->exportMlElement(aSmMlElement));

    // Prepare for following
    // If it has sub elements, then it will be the next
    if (aSmMlElement->getSubElementsCount() != 0)
        pData->incrementDepth();
    else // Otherwise remounts up to where it should be
    {
        while (aSmMlElement->getParentElement() != nullptr)
        {
            // get parent
            SmMlElement* pParent = aSmMlElement->getParentElement();
            pData->deleteDepthData();
            // was this the last branch ?
            if (aSmMlElement->getSubElementId() + 1 != pParent->getSubElementsCount()) // yes -> up
                break; // no -> stop going up
            // Prepare for next round
            aSmMlElement = pParent;
        }
    }
}

void SmMLExport::exportMlElementTree()
{
    exportMlElementTreeExecData* aData = new exportMlElementTreeExecData(this);
    mathml::SmMlIteratorTopToBottom(m_pElementTree, exportMlElementTreeExec, aData);
    delete aData;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

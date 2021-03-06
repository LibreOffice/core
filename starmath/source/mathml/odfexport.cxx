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

/*
 Warning: The SvXMLElementExport helper class creates the beginning and
 closing tags of xml elements in its constructor and destructor, so there's
 hidden stuff going on, on occasion the ordering of these classes declarations
 may be significant
*/

#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/uno/Any.h>

#include <rtl/math.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxsids.hrc>
#include <osl/diagnose.h>
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
#include <sal/log.hxx>

#include <stack>

#include <odfexport.hxx>
#include <xparsmlbase.hxx>
#include <strings.hrc>
#include <smmod.hxx>
#include <unomodel.hxx>
#include <document.hxx>
#include <utility.hxx>
#include <cfgitem.hxx>
#include <starmathdatabase.hxx>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

namespace
{
bool IsInPrivateUseArea(sal_Unicode cChar) { return 0xE000 <= cChar && cChar <= 0xF8FF; }

sal_Unicode ConvertMathToMathML(sal_Unicode cChar)
{
    sal_Unicode cRes = cChar;
    if (IsInPrivateUseArea(cChar))
    {
        SAL_WARN("starmath", "Error: private use area characters should no longer be in use!");
        cRes = u'@'; // just some character that should easily be notice as odd in the context
    }
    return cRes;
}
}

bool SmXMLExportWrapper::Export(SfxMedium& rMedium)
{
    bool bRet = true;
    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());

    //Get model
    uno::Reference<lang::XComponent> xModelComp = xModel;

    bool bEmbedded = false;
    SmModel* pModel = comphelper::getUnoTunnelImplementation<SmModel>(xModel);

    SmDocShell* pDocShell = pModel ? static_cast<SmDocShell*>(pModel->GetObjectShell()) : nullptr;
    if (pDocShell && SfxObjectCreateMode::EMBEDDED == pDocShell->GetCreateMode())
        bEmbedded = true;

    uno::Reference<task::XStatusIndicator> xStatusIndicator;
    if (!bEmbedded)
    {
        if (pDocShell /*&& pDocShell->GetMedium()*/)
        {
            OSL_ENSURE(pDocShell->GetMedium() == &rMedium, "different SfxMedium found");

            SfxItemSet* pSet = rMedium.GetItemSet();
            if (pSet)
            {
                const SfxUnoAnyItem* pItem = static_cast<const SfxUnoAnyItem*>(
                    pSet->GetItem(SID_PROGRESS_STATUSBAR_CONTROL));
                if (pItem)
                    pItem->GetValue() >>= xStatusIndicator;
            }
        }

        // set progress range and start status indicator
        if (xStatusIndicator.is())
        {
            sal_Int32 nProgressRange = bFlat ? 1 : 3;
            xStatusIndicator->start(SmResId(STR_STATSTR_WRITING), nProgressRange);
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

    SvtSaveOptions aSaveOpt;
    bool bUsePrettyPrinting(bFlat || aSaveOpt.IsPrettyPrinting());
    xInfoSet->setPropertyValue("UsePrettyPrinting", Any(bUsePrettyPrinting));

    // Set base URI
    OUString sPropName("BaseURI");
    xInfoSet->setPropertyValue(sPropName, makeAny(rMedium.GetBaseURL(true)));

    sal_Int32 nSteps = 0;
    if (xStatusIndicator.is())
        xStatusIndicator->setValue(nSteps++);
    if (!bFlat) //Storage (Package) of Stream
    {
        uno::Reference<embed::XStorage> xStg = rMedium.GetOutputStorage();
        bool bOASIS = (SotStorage::GetVersion(xStg) > SOFFICE_FILEFORMAT_60);

        // TODO/LATER: handle the case of embedded links gracefully
        if (bEmbedded) //&& !pStg->IsRoot() )
        {
            OUString aName;
            if (rMedium.GetItemSet())
            {
                const SfxStringItem* pDocHierarchItem = static_cast<const SfxStringItem*>(
                    rMedium.GetItemSet()->GetItem(SID_DOC_HIERARCHICALNAME));
                if (pDocHierarchItem)
                    aName = pDocHierarchItem->GetValue();
            }

            if (!aName.isEmpty())
            {
                sPropName = "StreamRelPath";
                xInfoSet->setPropertyValue(sPropName, makeAny(aName));
            }
        }

        if (!bEmbedded)
        {
            if (xStatusIndicator.is())
                xStatusIndicator->setValue(nSteps++);

            bRet = WriteThroughComponent(xStg, xModelComp, "meta.xml", xContext, xInfoSet,
                                         (bOASIS ? "com.sun.star.comp.Math.XMLOasisMetaExporter"
                                                 : "com.sun.star.comp.Math.XMLMetaExporter"),
                                         m_bUseHTMLMLEntities);
        }
        if (bRet)
        {
            if (xStatusIndicator.is())
                xStatusIndicator->setValue(nSteps++);

            bRet = WriteThroughComponent(xStg, xModelComp, "content.xml", xContext, xInfoSet,
                                         "com.sun.star.comp.Math.XMLContentExporter",
                                         m_bUseHTMLMLEntities);
        }

        if (bRet)
        {
            if (xStatusIndicator.is())
                xStatusIndicator->setValue(nSteps++);

            bRet = WriteThroughComponent(xStg, xModelComp, "settings.xml", xContext, xInfoSet,
                                         (bOASIS ? "com.sun.star.comp.Math.XMLOasisSettingsExporter"
                                                 : "com.sun.star.comp.Math.XMLSettingsExporter"),
                                         m_bUseHTMLMLEntities);
        }
    }
    else
    {
        SvStream* pStream = rMedium.GetOutStream();
        uno::Reference<io::XOutputStream> xOut(new utl::OOutputStreamWrapper(*pStream));

        if (xStatusIndicator.is())
            xStatusIndicator->setValue(nSteps++);

        bRet = WriteThroughComponent(xOut, xModelComp, xContext, xInfoSet,
                                     "com.sun.star.comp.Math.XMLContentExporter",
                                     m_bUseHTMLMLEntities);
    }

    if (xStatusIndicator.is())
        xStatusIndicator->end();

    return bRet;
}

/// export through an XML exporter component (output stream version)
bool SmXMLExportWrapper::WriteThroughComponent(const Reference<io::XOutputStream>& xOutputStream,
                                               const Reference<XComponent>& xComponent,
                                               Reference<uno::XComponentContext> const& rxContext,
                                               Reference<beans::XPropertySet> const& rPropSet,
                                               const char* pComponentName, bool bUseHTMLMLEntities)
{
    OSL_ENSURE(xOutputStream.is(), "I really need an output stream!");
    OSL_ENSURE(xComponent.is(), "Need component!");
    OSL_ENSURE(nullptr != pComponentName, "Need component name!");

    // get component
    Reference<xml::sax::XWriter> xSaxWriter = xml::sax::Writer::create(rxContext);

    // connect XML writer to output stream
    xSaxWriter->setOutputStream(xOutputStream);
    if (bUseHTMLMLEntities)
        xSaxWriter->setCustomEntityNames(starmathdatabase::icustomMathmlHtmlEntitiesExport);

    // prepare arguments (prepend doc handler to given arguments)
    Sequence<Any> aArgs(2);
    aArgs[0] <<= xSaxWriter;
    aArgs[1] <<= rPropSet;

    // get filter component
    Reference<document::XExporter> xExporter(
        rxContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            OUString::createFromAscii(pComponentName), aArgs, rxContext),
        UNO_QUERY);
    OSL_ENSURE(xExporter.is(), "can't instantiate export filter component");
    if (!xExporter.is())
        return false;

    // connect model and filter
    xExporter->setSourceDocument(xComponent);

    // filter!
    Reference<XFilter> xFilter(xExporter, UNO_QUERY);
    uno::Sequence<PropertyValue> aProps(0);
    xFilter->filter(aProps);

    auto pFilter = comphelper::getUnoTunnelImplementation<SmXMLExport>(xFilter);
    return pFilter == nullptr || pFilter->GetSuccess();
}

/// export through an XML exporter component (storage version)
bool SmXMLExportWrapper::WriteThroughComponent(const Reference<embed::XStorage>& xStorage,
                                               const Reference<XComponent>& xComponent,
                                               const char* pStreamName,
                                               Reference<uno::XComponentContext> const& rxContext,
                                               Reference<beans::XPropertySet> const& rPropSet,
                                               const char* pComponentName, bool bUseHTMLMLEntities)
{
    OSL_ENSURE(xStorage.is(), "Need storage!");
    OSL_ENSURE(nullptr != pStreamName, "Need stream name!");

    // open stream
    Reference<io::XStream> xStream;
    OUString sStreamName = OUString::createFromAscii(pStreamName);
    try
    {
        xStream = xStorage->openStreamElement(sStreamName, embed::ElementModes::READWRITE
                                                               | embed::ElementModes::TRUNCATE);
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("starmath", "Can't create output stream in package");
        return false;
    }

    uno::Reference<beans::XPropertySet> xSet(xStream, uno::UNO_QUERY);
    xSet->setPropertyValue("MediaType", Any(OUString("text/xml")));

    // all streams must be encrypted in encrypted document
    xSet->setPropertyValue("UseCommonStoragePasswordEncryption", Any(true));

    // set Base URL
    if (rPropSet.is())
    {
        rPropSet->setPropertyValue("StreamName", makeAny(sStreamName));
    }

    // write the stuff
    bool bRet = WriteThroughComponent(xStream->getOutputStream(), xComponent, rxContext, rPropSet,
                                      pComponentName, bUseHTMLMLEntities);

    return bRet;
}

SmXMLExport::SmXMLExport(const css::uno::Reference<css::uno::XComponentContext>& rContext,
                         OUString const& implementationName, SvXMLExportFlags nExportFlags)
    : SvXMLExport(rContext, implementationName, util::MeasureUnit::INCH, XML_MATH, nExportFlags)
    , pTree(nullptr)
    , bSuccess(false)
{
}

sal_Int64 SAL_CALL SmXMLExport::getSomething(const uno::Sequence<sal_Int8>& rId)
{
    if (isUnoTunnelId<SmXMLExport>(rId))
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_uIntPtr>(this));

    return SvXMLExport::getSomething(rId);
}

namespace
{
class theSmXMLExportUnoTunnelId : public rtl::Static<UnoTunnelIdInit, theSmXMLExportUnoTunnelId>
{
};
}

const uno::Sequence<sal_Int8>& SmXMLExport::getUnoTunnelId() throw()
{
    return theSmXMLExportUnoTunnelId::get().getSeq();
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Math_XMLExporter_get_implementation(css::uno::XComponentContext* context,
                                    css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new SmXMLExport(context, "com.sun.star.comp.Math.XMLExporter",
                                         SvXMLExportFlags::OASIS | SvXMLExportFlags::ALL));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Math_XMLMetaExporter_get_implementation(css::uno::XComponentContext* context,
                                        css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(
        new SmXMLExport(context, "com.sun.star.comp.Math.XMLMetaExporter", SvXMLExportFlags::META));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Math_XMLOasisMetaExporter_get_implementation(css::uno::XComponentContext* context,
                                             css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new SmXMLExport(context, "com.sun.star.comp.Math.XMLOasisMetaExporter",
                                         SvXMLExportFlags::OASIS | SvXMLExportFlags::META));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Math_XMLSettingsExporter_get_implementation(css::uno::XComponentContext* context,
                                            css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new SmXMLExport(context, "com.sun.star.comp.Math.XMLSettingsExporter",
                                         SvXMLExportFlags::SETTINGS));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Math_XMLOasisSettingsExporter_get_implementation(css::uno::XComponentContext* context,
                                                 css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new SmXMLExport(context, "com.sun.star.comp.Math.XMLOasisSettingsExporter",
                                         SvXMLExportFlags::OASIS | SvXMLExportFlags::SETTINGS));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
Math_XMLContentExporter_get_implementation(css::uno::XComponentContext* context,
                                           css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new SmXMLExport(context, "com.sun.star.comp.Math.XMLContentExporter",
                                         SvXMLExportFlags::OASIS | SvXMLExportFlags::CONTENT));
}

ErrCode SmXMLExport::exportDoc(enum XMLTokenEnum eClass)
{
    if (!(getExportFlags() & SvXMLExportFlags::CONTENT))
    {
        SvXMLExport::exportDoc(eClass);
    }
    else
    {
        uno::Reference<frame::XModel> xModel = GetModel();
        SmModel* pModel = comphelper::getUnoTunnelImplementation<SmModel>(xModel);

        if (pModel)
        {
            SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
            pTree = pDocShell->GetFormulaTree();
            aText = pDocShell->GetText();
        }

        GetDocHandler()->startDocument();

        addChaffWhenEncryptedStorage();

        /*Add xmlns line*/
        SvXMLAttributeList& rList = GetAttrList();

        // make use of a default namespace
        ResetNamespaceMap(); // Math doesn't need namespaces from xmloff, since it now uses default namespaces (because that is common with current MathML usage in the web)
        GetNamespaceMap_().Add(OUString(), GetXMLToken(XML_N_MATH), XML_NAMESPACE_MATH);

        rList.AddAttribute(GetNamespaceMap().GetAttrNameByKey(XML_NAMESPACE_MATH),
                           GetNamespaceMap().GetNameByKey(XML_NAMESPACE_MATH));

        //I think we need something like ImplExportEntities();
        ExportContent_();
        GetDocHandler()->endDocument();
    }

    bSuccess = true;
    return ERRCODE_NONE;
}

void SmXMLExport::ExportContent_()
{
    uno::Reference<frame::XModel> xModel = GetModel();
    SmModel* pModel = comphelper::getUnoTunnelImplementation<SmModel>(xModel);
    SmDocShell* pDocShell = pModel ? static_cast<SmDocShell*>(pModel->GetObjectShell()) : nullptr;
    OSL_ENSURE(pDocShell, "doc shell missing");

    if (pDocShell && !pDocShell->GetFormat().IsTextmode())
    {
        // If the Math equation is not in text mode, we attach a display="block"
        // attribute on the <math> root. We don't do anything if it is in
        // text mode, the default display="inline" value will be used.
        AddAttribute(XML_NAMESPACE_MATH, XML_DISPLAY, XML_BLOCK);
    }
    SvXMLElementExport aEquation(*this, XML_NAMESPACE_MATH, XML_MATH, true, true);
    std::unique_ptr<SvXMLElementExport> pSemantics;

    if (!aText.isEmpty())
    {
        pSemantics.reset(
            new SvXMLElementExport(*this, XML_NAMESPACE_MATH, XML_SEMANTICS, true, true));
    }

    ExportNodes(pTree, 0);

    if (aText.isEmpty())
        return;

    SmModule* pMod = SM_MOD();
    sal_uInt16 nSmSyntaxVersion = pMod->GetConfig()->GetDefaultSmSyntaxVersion();

    // Convert symbol names
    if (pDocShell)
    {
        nSmSyntaxVersion = pDocShell->GetSmSyntaxVersion();
        AbstractSmParser* rParser = pDocShell->GetParser();
        bool bVal = rParser->IsExportSymbolNames();
        rParser->SetExportSymbolNames(true);
        auto pTmpTree = rParser->Parse(aText);
        aText = rParser->GetText();
        pTmpTree.reset();
        rParser->SetExportSymbolNames(bVal);
    }

    OUStringBuffer sStrBuf(12);
    sStrBuf.append(u"StarMath ");
    if (nSmSyntaxVersion == 5)
        sStrBuf.append(u"5.0");
    else
        sStrBuf.append(OUString::number(nSmSyntaxVersion));

    AddAttribute(XML_NAMESPACE_MATH, XML_ENCODING, sStrBuf.makeStringAndClear());
    SvXMLElementExport aAnnotation(*this, XML_NAMESPACE_MATH, XML_ANNOTATION, true, false);
    GetDocHandler()->characters(aText);
}

void SmXMLExport::GetViewSettings(Sequence<PropertyValue>& aProps)
{
    uno::Reference<frame::XModel> xModel = GetModel();
    if (!xModel.is())
        return;

    SmModel* pModel = comphelper::getUnoTunnelImplementation<SmModel>(xModel);

    if (!pModel)
        return;

    SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
    if (!pDocShell)
        return;

    aProps.realloc(4);
    PropertyValue* pValue = aProps.getArray();
    sal_Int32 nIndex = 0;

    tools::Rectangle aRect(pDocShell->GetVisArea());

    pValue[nIndex].Name = "ViewAreaTop";
    pValue[nIndex++].Value <<= aRect.Top();

    pValue[nIndex].Name = "ViewAreaLeft";
    pValue[nIndex++].Value <<= aRect.Left();

    pValue[nIndex].Name = "ViewAreaWidth";
    pValue[nIndex++].Value <<= aRect.GetWidth();

    pValue[nIndex].Name = "ViewAreaHeight";
    pValue[nIndex++].Value <<= aRect.GetHeight();
}

void SmXMLExport::GetConfigurationSettings(Sequence<PropertyValue>& rProps)
{
    Reference<XPropertySet> xProps(GetModel(), UNO_QUERY);
    if (!xProps.is())
        return;

    Reference<XPropertySetInfo> xPropertySetInfo = xProps->getPropertySetInfo();
    if (!xPropertySetInfo.is())
        return;

    Sequence<Property> aProps = xPropertySetInfo->getProperties();
    const sal_Int32 nCount = aProps.getLength();
    if (!nCount)
        return;

    rProps.realloc(nCount);
    SmMathConfig* pConfig = SM_MOD()->GetConfig();
    const bool bUsedSymbolsOnly = pConfig && pConfig->IsSaveOnlyUsedSymbols();

    std::transform(aProps.begin(), aProps.end(), rProps.begin(),
                   [bUsedSymbolsOnly, &xProps](Property& prop) {
                       PropertyValue aRet;
                       if (prop.Name != "Formula" && prop.Name != "BasicLibraries"
                           && prop.Name != "DialogLibraries" && prop.Name != "RuntimeUID")
                       {
                           aRet.Name = prop.Name;
                           OUString aActualName(prop.Name);
                           // handle 'save used symbols only'
                           if (bUsedSymbolsOnly && prop.Name == "Symbols")
                               aActualName = "UserDefinedSymbolsInUse";
                           aRet.Value = xProps->getPropertyValue(aActualName);
                       }
                       return aRet;
                   });
}

static bool lcl_HasEffectOnMathvariant(const SmTokenType eType)
{
    return eType == TBOLD || eType == TNBOLD || eType == TITALIC || eType == TNITALIC
           || eType == TSANS || eType == TSERIF || eType == TFIXED;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

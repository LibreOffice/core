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

/*todo: Change characters and tcharacters to accumulate the characters together
into one string, xml parser hands them to us line by line rather than all in
one go*/

#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/FastParser.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/packages/WrongPasswordException.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/task/XStatusIndicator.hpp>

#include <comphelper/fileformat.h>
#include <comphelper/genericpropertyset.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <rtl/character.hxx>
#include <sal/log.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/sfxmodelfactory.hxx>
#include <osl/diagnose.h>
#include <sot/storage.hxx>
#include <svtools/sfxecode.hxx>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <unotools/streamwrap.hxx>
#include <sax/tools/converter.hxx>
#include <xmloff/DocumentSettingsContext.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlmetai.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <tools/diagnose_ex.h>

#include <memory>

#include <mathmlattr.hxx>
#include <xparsmlbase.hxx>
#include <mathmlimport.hxx>
#include <document.hxx>
#include <smdll.hxx>
#include <unomodel.hxx>
#include <utility.hxx>
#include <visitors.hxx>
#include <starmathdatabase.hxx>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

namespace
{
std::unique_ptr<SmNode> popOrZero(SmNodeStack& rStack)
{
    if (rStack.empty())
        return nullptr;
    auto pTmp = std::move(rStack.front());
    rStack.pop_front();
    return pTmp;
}
}

ErrCode SmXMLImportWrapper::Import(SfxMedium& rMedium)
{
    ErrCode nError = ERRCODE_SFX_DOLOADFAILED;

    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());

    //Make a model component from our SmModel
    uno::Reference<lang::XComponent> xModelComp = xModel;
    OSL_ENSURE(xModelComp.is(), "XMLReader::Read: got no model");

    // try to get an XStatusIndicator from the Medium
    uno::Reference<task::XStatusIndicator> xStatusIndicator;

    bool bEmbedded = false;
    SmModel* pModel = comphelper::getUnoTunnelImplementation<SmModel>(xModel);

    SmDocShell* pDocShell = pModel ? static_cast<SmDocShell*>(pModel->GetObjectShell()) : nullptr;
    if (pDocShell)
    {
        OSL_ENSURE(pDocShell->GetMedium() == &rMedium, "different SfxMedium found");

        SfxItemSet* pSet = rMedium.GetItemSet();
        if (pSet)
        {
            const SfxUnoAnyItem* pItem
                = static_cast<const SfxUnoAnyItem*>(pSet->GetItem(SID_PROGRESS_STATUSBAR_CONTROL));
            if (pItem)
                pItem->GetValue() >>= xStatusIndicator;
        }

        if (SfxObjectCreateMode::EMBEDDED == pDocShell->GetCreateMode())
            bEmbedded = true;
    }

    comphelper::PropertyMapEntry aInfoMap[]
        = { { OUString("PrivateData"), 0, cppu::UnoType<XInterface>::get(),
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

    // Set base URI
    OUString const baseURI(rMedium.GetBaseURL());
    // needed for relative URLs; but it's OK to import e.g. MathML from the
    // clipboard without one
    SAL_INFO_IF(baseURI.isEmpty(), "starmath", "SmXMLImportWrapper: no base URL");
    xInfoSet->setPropertyValue("BaseURI", makeAny(baseURI));

    sal_Int32 nSteps = 3;
    if (!(rMedium.IsStorage()))
        nSteps = 1;

    sal_Int32 nProgressRange(nSteps);
    if (xStatusIndicator.is())
    {
        xStatusIndicator->start(SvxResId(RID_SVXSTR_DOC_LOAD), nProgressRange);
    }

    nSteps = 0;
    if (xStatusIndicator.is())
        xStatusIndicator->setValue(nSteps++);

    if (rMedium.IsStorage())
    {
        // TODO/LATER: handle the case of embedded links gracefully
        if (bEmbedded) // && !rMedium.GetStorage()->IsRoot() )
        {
            OUString aName("dummyObjName");
            if (rMedium.GetItemSet())
            {
                const SfxStringItem* pDocHierarchItem = static_cast<const SfxStringItem*>(
                    rMedium.GetItemSet()->GetItem(SID_DOC_HIERARCHICALNAME));
                if (pDocHierarchItem)
                    aName = pDocHierarchItem->GetValue();
            }

            if (!aName.isEmpty())
            {
                xInfoSet->setPropertyValue("StreamRelPath", makeAny(aName));
            }
        }

        bool bOASIS = (SotStorage::GetVersion(rMedium.GetStorage()) > SOFFICE_FILEFORMAT_60);
        if (xStatusIndicator.is())
            xStatusIndicator->setValue(nSteps++);

        auto nWarn
            = ReadThroughComponent(rMedium.GetStorage(), xModelComp, "meta.xml", xContext, xInfoSet,
                                   (bOASIS ? "com.sun.star.comp.Math.XMLOasisMetaImporter"
                                           : "com.sun.star.comp.Math.XMLMetaImporter"),
                                   bbUseHTMLMLEntities);

        if (nWarn != ERRCODE_IO_BROKENPACKAGE)
        {
            if (xStatusIndicator.is())
                xStatusIndicator->setValue(nSteps++);

            nWarn = ReadThroughComponent(rMedium.GetStorage(), xModelComp, "settings.xml", xContext,
                                         xInfoSet,
                                         (bOASIS ? "com.sun.star.comp.Math.XMLOasisSettingsImporter"
                                                 : "com.sun.star.comp.Math.XMLSettingsImporter"),
                                         bbUseHTMLMLEntities);

            if (nWarn != ERRCODE_IO_BROKENPACKAGE)
            {
                if (xStatusIndicator.is())
                    xStatusIndicator->setValue(nSteps++);

                nError = ReadThroughComponent(
                    rMedium.GetStorage(), xModelComp, "content.xml", xContext, xInfoSet,
                    "com.sun.star.comp.Math.XMLImporter", bbUseHTMLMLEntities);
            }
            else
                nError = ERRCODE_IO_BROKENPACKAGE;
        }
        else
            nError = ERRCODE_IO_BROKENPACKAGE;
    }
    else
    {
        Reference<io::XInputStream> xInputStream
            = new utl::OInputStreamWrapper(rMedium.GetInStream());

        if (xStatusIndicator.is())
            xStatusIndicator->setValue(nSteps++);

        nError = ReadThroughComponent(xInputStream, xModelComp, xContext, xInfoSet,
                                      "com.sun.star.comp.Math.XMLImporter", false,
                                      bbUseHTMLMLEntities);
    }

    if (xStatusIndicator.is())
        xStatusIndicator->end();
    return nError;
}

/// read a component (file + filter version)
ErrCode SmXMLImportWrapper::ReadThroughComponent(const Reference<io::XInputStream>& xInputStream,
                                                 const Reference<XComponent>& xModelComponent,
                                                 Reference<uno::XComponentContext> const& rxContext,
                                                 Reference<beans::XPropertySet> const& rPropSet,
                                                 const char* pFilterName, bool bEncrypted,
                                                 bool bUseHTMLMLEntities)
{
    ErrCode nError = ERRCODE_SFX_DOLOADFAILED;
    OSL_ENSURE(xInputStream.is(), "input stream missing");
    OSL_ENSURE(xModelComponent.is(), "document missing");
    OSL_ENSURE(rxContext.is(), "factory missing");
    OSL_ENSURE(nullptr != pFilterName, "I need a service name for the component!");

    // prepare ParserInputSource
    xml::sax::InputSource aParserInput;
    aParserInput.aInputStream = xInputStream;

    Sequence<Any> aArgs(1);
    aArgs[0] <<= rPropSet;

    // get filter
    Reference<XInterface> xFilter
        = rxContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            OUString::createFromAscii(pFilterName), aArgs, rxContext);
    SAL_WARN_IF(!xFilter, "starmath", "Can't instantiate filter component " << pFilterName);
    if (!xFilter.is())
        return nError;

    // connect model and filter
    Reference<XImporter> xImporter(xFilter, UNO_QUERY);
    xImporter->setTargetDocument(xModelComponent);

    // finally, parser the stream
    try
    {
        Reference<css::xml::sax::XFastParser> xFastParser(xFilter, UNO_QUERY);
        Reference<css::xml::sax::XFastDocumentHandler> xFastDocHandler(xFilter, UNO_QUERY);
        if (xFastParser)
        {
            if (bUseHTMLMLEntities)
                xFastParser->setCustomEntityNames(starmathdatabase::icustomMathmlHtmlEntities);
            xFastParser->parseStream(aParserInput);
        }
        else if (xFastDocHandler)
        {
            Reference<css::xml::sax::XFastParser> xParser
                = css::xml::sax::FastParser::create(rxContext);
            if (bUseHTMLMLEntities)
                xParser->setCustomEntityNames(starmathdatabase::icustomMathmlHtmlEntities);
            xParser->setFastDocumentHandler(xFastDocHandler);
            xParser->parseStream(aParserInput);
        }
        else
        {
            Reference<css::xml::sax::XDocumentHandler> xDocHandler(xFilter, UNO_QUERY);
            assert(xDocHandler);
            Reference<css::xml::sax::XParser> xParser = css::xml::sax::Parser::create(rxContext);
            xParser->setDocumentHandler(xDocHandler);
            xParser->parseStream(aParserInput);
        }

        auto pFilter = comphelper::getUnoTunnelImplementation<SmXMLImport>(xFilter);
        if (pFilter && pFilter->GetSuccess())
            nError = ERRCODE_NONE;
    }
    catch (const xml::sax::SAXParseException& r)
    {
        // sax parser sends wrapped exceptions,
        // try to find the original one
        xml::sax::SAXException aSaxEx = *static_cast<const xml::sax::SAXException*>(&r);
        bool bTryChild = true;

        while (bTryChild)
        {
            xml::sax::SAXException aTmp;
            if (aSaxEx.WrappedException >>= aTmp)
                aSaxEx = aTmp;
            else
                bTryChild = false;
        }

        packages::zip::ZipIOException aBrokenPackage;
        if (aSaxEx.WrappedException >>= aBrokenPackage)
            return ERRCODE_IO_BROKENPACKAGE;

        if (bEncrypted)
            nError = ERRCODE_SFX_WRONGPASSWORD;
    }
    catch (const xml::sax::SAXException& r)
    {
        packages::zip::ZipIOException aBrokenPackage;
        if (r.WrappedException >>= aBrokenPackage)
            return ERRCODE_IO_BROKENPACKAGE;

        if (bEncrypted)
            nError = ERRCODE_SFX_WRONGPASSWORD;
    }
    catch (const packages::zip::ZipIOException&)
    {
        nError = ERRCODE_IO_BROKENPACKAGE;
    }
    catch (const io::IOException&)
    {
    }
    catch (const std::range_error&)
    {
    }

    return nError;
}

ErrCode SmXMLImportWrapper::ReadThroughComponent(const uno::Reference<embed::XStorage>& xStorage,
                                                 const Reference<XComponent>& xModelComponent,
                                                 const char* pStreamName,
                                                 Reference<uno::XComponentContext> const& rxContext,
                                                 Reference<beans::XPropertySet> const& rPropSet,
                                                 const char* pFilterName, bool bUseHTMLMLEntities)
{
    OSL_ENSURE(xStorage.is(), "Need storage!");
    OSL_ENSURE(nullptr != pStreamName, "Please, please, give me a name!");

    // open stream (and set parser input)
    OUString sStreamName = OUString::createFromAscii(pStreamName);

    // get input stream
    try
    {
        uno::Reference<io::XStream> xEventsStream
            = xStorage->openStreamElement(sStreamName, embed::ElementModes::READ);

        // determine if stream is encrypted or not
        uno::Reference<beans::XPropertySet> xProps(xEventsStream, uno::UNO_QUERY);
        Any aAny = xProps->getPropertyValue("Encrypted");
        bool bEncrypted = false;
        if (aAny.getValueType() == cppu::UnoType<bool>::get())
            aAny >>= bEncrypted;

        // set Base URL
        if (rPropSet.is())
        {
            rPropSet->setPropertyValue("StreamName", makeAny(sStreamName));
        }

        Reference<io::XInputStream> xStream = xEventsStream->getInputStream();
        return ReadThroughComponent(xStream, xModelComponent, rxContext, rPropSet, pFilterName,
                                    bEncrypted, bUseHTMLMLEntities);
    }
    catch (packages::WrongPasswordException&)
    {
        return ERRCODE_SFX_WRONGPASSWORD;
    }
    catch (packages::zip::ZipIOException&)
    {
        return ERRCODE_IO_BROKENPACKAGE;
    }
    catch (uno::Exception&)
    {
    }

    return ERRCODE_SFX_DOLOADFAILED;
}

SmXMLImport::SmXMLImport(const css::uno::Reference<css::uno::XComponentContext>& rContext,
                         OUString const& implementationName, SvXMLImportFlags nImportFlags)
    : SvXMLImport(rContext, implementationName, nImportFlags)
    , bSuccess(false)
    , nParseDepth(0)
{
}

namespace
{
class theSmXMLImportUnoTunnelId : public rtl::Static<UnoTunnelIdInit, theSmXMLImportUnoTunnelId>
{
};
}

const uno::Sequence<sal_Int8>& SmXMLImport::getUnoTunnelId() throw()
{
    return theSmXMLImportUnoTunnelId::get().getSeq();
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
Math_XMLImporter_get_implementation(uno::XComponentContext* pCtx,
                                    uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(
        new SmXMLImport(pCtx, "com.sun.star.comp.Math.XMLImporter", SvXMLImportFlags::ALL));
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
Math_XMLOasisMetaImporter_get_implementation(uno::XComponentContext* pCtx,
                                             uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SmXMLImport(pCtx, "com.sun.star.comp.Math.XMLOasisMetaImporter",
                                         SvXMLImportFlags::META));
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
Math_XMLOasisSettingsImporter_get_implementation(uno::XComponentContext* pCtx,
                                                 uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SmXMLImport(pCtx, "com.sun.star.comp.Math.XMLOasisSettingsImporter",
                                         SvXMLImportFlags::SETTINGS));
}

sal_Int64 SAL_CALL SmXMLImport::getSomething(const uno::Sequence<sal_Int8>& rId)
{
    if (isUnoTunnelId<SmXMLImport>(rId))
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_uIntPtr>(this));

    return SvXMLImport::getSomething(rId);
}

void SmXMLImport::endDocument()
{
    //Set the resulted tree into the SmDocShell where it belongs
    std::unique_ptr<SmNode> pTree = popOrZero(aNodeStack);
    if (pTree && pTree->GetType() == SmNodeType::Table)
    {
        uno::Reference<frame::XModel> xModel = GetModel();
        SmModel* pModel = comphelper::getUnoTunnelImplementation<SmModel>(xModel);

        if (pModel)
        {
            SmDocShell* pDocShell = static_cast<SmDocShell*>(pModel->GetObjectShell());
            auto pTreeTmp = pTree.get();
            pDocShell->SetFormulaTree(static_cast<SmTableNode*>(pTree.release()));
            if (aText.isEmpty()) //If we picked up no annotation text
            {
                // Get text from imported formula
                SmNodeToTextVisitor tmpvisitor(pTreeTmp, aText);
            }

            // Convert symbol names
            SmParser& rParser = pDocShell->GetParser();
            bool bVal = rParser.IsImportSymbolNames();
            rParser.SetImportSymbolNames(true);
            auto pTmpTree = rParser.Parse(aText);
            aText = rParser.GetText();
            pTmpTree.reset();
            rParser.SetImportSymbolNames(bVal);

            pDocShell->SetText(aText);
        }
        OSL_ENSURE(pModel, "So there *was* a UNO problem after all");

        bSuccess = true;
    }

    SvXMLImport::endDocument();
}

namespace
{
class SmXMLImportContext : public SvXMLImportContext
{
public:
    SmXMLImportContext(SmXMLImport& rImport)
        : SvXMLImportContext(rImport)
    {
        GetSmImport().IncParseDepth();
    }

    virtual ~SmXMLImportContext() override { GetSmImport().DecParseDepth(); }

    SmXMLImport& GetSmImport() { return static_cast<SmXMLImport&>(GetImport()); }

    virtual void TCharacters(const OUString& /*rChars*/);
    virtual void SAL_CALL characters(const OUString& rChars) override;
    virtual void SAL_CALL startFastElement(
        sal_Int32 /*nElement*/,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& /*rAttrList*/) override
    {
        if (GetSmImport().TooDeep())
            throw std::range_error("too deep");
    }
};
}

void SmXMLImportContext::TCharacters(const OUString& /*rChars*/) {}

void SmXMLImportContext::characters(const OUString& rChars)
{
    /*
    Whitespace occurring within the content of token elements is "trimmed"
    from the ends (i.e. all whitespace at the beginning and end of the
    content is removed), and "collapsed" internally (i.e. each sequence of
    1 or more whitespace characters is replaced with one blank character).
    */
    //collapsing not done yet!
    const OUString& rChars2 = rChars.trim();
    if (!rChars2.isEmpty())
        TCharacters(rChars2 /*.collapse()*/);
}

namespace
{
struct SmXMLContext_Helper
{
    sal_Int8 nIsBold;
    sal_Int8 nIsItalic;
    double nFontSize;
    OUString sFontFamily;
    OUString sColor;

    SmXMLImportContext& rContext;

    explicit SmXMLContext_Helper(SmXMLImportContext& rImport)
        : nIsBold(-1)
        , nIsItalic(-1)
        , nFontSize(0.0)
        , rContext(rImport)
    {
    }

    bool IsFontNodeNeeded() const;
    void RetrieveAttrs(const uno::Reference<xml::sax::XFastAttributeList>& xAttrList);
    void ApplyAttrs();
};
}

bool SmXMLContext_Helper::IsFontNodeNeeded() const
{
    return nIsBold != -1 || nIsItalic != -1 || nFontSize != 0.0 || !sFontFamily.isEmpty()
           || !sColor.isEmpty();
}

void SmXMLContext_Helper::RetrieveAttrs(
    const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    bool bMvFound = false;
    for (auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        // sometimes they have namespace, sometimes not?
        switch (aIter.getToken() & TOKEN_MASK)
        {
            case XML_FONTWEIGHT:
                nIsBold = sal_Int8(IsXMLToken(aIter, XML_BOLD));
                break;
            case XML_FONTSTYLE:
                nIsItalic = sal_Int8(IsXMLToken(aIter, XML_ITALIC));
                break;
            case XML_FONTSIZE:
            case XML_MATHSIZE:
            {
                OUString sValue = aIter.toString();
                ::sax::Converter::convertDouble(nFontSize, sValue);
                rContext.GetSmImport().GetMM100UnitConverter().SetXMLMeasureUnit(
                    util::MeasureUnit::POINT);
                if (-1 == sValue.indexOf(GetXMLToken(XML_UNIT_PT)))
                {
                    if (-1 == sValue.indexOf('%'))
                        nFontSize = 0.0;
                    else
                    {
                        rContext.GetSmImport().GetMM100UnitConverter().SetXMLMeasureUnit(
                            util::MeasureUnit::PERCENT);
                    }
                }
                break;
            }
            case XML_FONTFAMILY:
                sFontFamily = aIter.toString();
                break;
            case XML_COLOR:
                sColor = aIter.toString();
                break;
            case XML_MATHCOLOR:
                sColor = aIter.toString();
                break;
            case XML_MATHVARIANT:
                bMvFound = true;
                break;
            default:
                XMLOFF_WARN_UNKNOWN("starmath", aIter);
                break;
        }
    }

    if (bMvFound)
    {
        // Ignore deprecated attributes fontfamily, fontweight, and fontstyle
        // in favor of mathvariant, as specified in
        // <https://www.w3.org/TR/MathML3/chapter3.html#presm.deprecatt>.
        sFontFamily.clear();
        nIsBold = -1;
        nIsItalic = -1;
    }
}

void SmXMLContext_Helper::ApplyAttrs()
{
    SmNodeStack& rNodeStack = rContext.GetSmImport().GetNodeStack();

    if (!IsFontNodeNeeded())
        return;

    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.nLevel = 5;

    if (nIsBold != -1)
    {
        if (nIsBold)
            aToken.eType = TBOLD;
        else
            aToken.eType = TNBOLD;
        std::unique_ptr<SmFontNode> pFontNode(new SmFontNode(aToken));
        pFontNode->SetSubNodes(nullptr, popOrZero(rNodeStack));
        rNodeStack.push_front(std::move(pFontNode));
    }
    if (nIsItalic != -1)
    {
        if (nIsItalic)
            aToken.eType = TITALIC;
        else
            aToken.eType = TNITALIC;
        std::unique_ptr<SmFontNode> pFontNode(new SmFontNode(aToken));
        pFontNode->SetSubNodes(nullptr, popOrZero(rNodeStack));
        rNodeStack.push_front(std::move(pFontNode));
    }
    if (nFontSize != 0.0)
    {
        aToken.eType = TSIZE;
        std::unique_ptr<SmFontNode> pFontNode(new SmFontNode(aToken));

        if (util::MeasureUnit::PERCENT
            == rContext.GetSmImport().GetMM100UnitConverter().GetXMLMeasureUnit())
        {
            if (nFontSize < 100.00)
                pFontNode->SetSizeParameter(Fraction(100.00 / nFontSize), FontSizeType::DIVIDE);
            else
                pFontNode->SetSizeParameter(Fraction(nFontSize / 100.00), FontSizeType::MULTIPLY);
        }
        else
            pFontNode->SetSizeParameter(Fraction(nFontSize), FontSizeType::ABSOLUT);

        pFontNode->SetSubNodes(nullptr, popOrZero(rNodeStack));
        rNodeStack.push_front(std::move(pFontNode));
    }
    if (!sColor.isEmpty())
    {
        std::unique_ptr<SmColorTokenTableEntry> aSmColorTokenTableEntry;
        aSmColorTokenTableEntry = starmathdatabase::Identify_ColorName_HTML(sColor);
        if (aSmColorTokenTableEntry->eType == TRGB)
            aSmColorTokenTableEntry = starmathdatabase::Identify_Color_Parser(
                sal_uInt32(aSmColorTokenTableEntry->cColor));
        if (aSmColorTokenTableEntry->eType != TERROR)
        {
            aToken = aSmColorTokenTableEntry;
            std::unique_ptr<SmFontNode> pFontNode(new SmFontNode(aToken));
            pFontNode->SetSubNodes(nullptr, popOrZero(rNodeStack));
            rNodeStack.push_front(std::move(pFontNode));
        }
        // If not known, not implemented yet. Giving up.
    }
    if (!sFontFamily.isEmpty())
    {
        if (sFontFamily.equalsIgnoreAsciiCase(GetXMLToken(XML_FIXED)))
            aToken.eType = TFIXED;
        else if (sFontFamily.equalsIgnoreAsciiCase("sans"))
            aToken.eType = TSANS;
        else if (sFontFamily.equalsIgnoreAsciiCase("serif"))
            aToken.eType = TSERIF;
        else //Just give up, we need to extend our font mechanism to be
            //more general
            return;

        aToken.aText = sFontFamily;
        std::unique_ptr<SmFontNode> pFontNode(new SmFontNode(aToken));
        pFontNode->SetSubNodes(nullptr, popOrZero(rNodeStack));
        rNodeStack.push_front(std::move(pFontNode));
    }
}

namespace
{
class SmXMLTokenAttrHelper
{
    SmXMLImportContext& mrContext;
    MathMLMathvariantValue meMv;
    bool mbMvFound;

public:
    SmXMLTokenAttrHelper(SmXMLImportContext& rContext)
        : mrContext(rContext)
        , meMv(MathMLMathvariantValue::Normal)
        , mbMvFound(false)
    {
    }

    void RetrieveAttrs(const uno::Reference<xml::sax::XFastAttributeList>& xAttrList);
    void ApplyAttrs(MathMLMathvariantValue eDefaultMv);
};
}

void SmXMLTokenAttrHelper::RetrieveAttrs(
    const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    for (auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        OUString sValue = aIter.toString();
        switch (aIter.getToken())
        {
            case XML_MATHVARIANT:
                if (!GetMathMLMathvariantValue(sValue, meMv))
                    SAL_WARN("starmath", "failed to recognize mathvariant: " << sValue);
                mbMvFound = true;
                break;
            default:
                XMLOFF_WARN_UNKNOWN("starmath", aIter);
                break;
        }
    }
}

void SmXMLTokenAttrHelper::ApplyAttrs(MathMLMathvariantValue eDefaultMv)
{
    assert(eDefaultMv == MathMLMathvariantValue::Normal
           || eDefaultMv == MathMLMathvariantValue::Italic);

    std::vector<SmTokenType> vVariant;
    MathMLMathvariantValue eMv = mbMvFound ? meMv : eDefaultMv;
    switch (eMv)
    {
        case MathMLMathvariantValue::Normal:
            vVariant.push_back(TNITALIC);
            break;
        case MathMLMathvariantValue::Bold:
            vVariant.push_back(TBOLD);
            break;
        case MathMLMathvariantValue::Italic:
            // nothing to do
            break;
        case MathMLMathvariantValue::BoldItalic:
            vVariant.push_back(TITALIC);
            vVariant.push_back(TBOLD);
            break;
        case MathMLMathvariantValue::DoubleStruck:
            // TODO
            break;
        case MathMLMathvariantValue::BoldFraktur:
            // TODO: Fraktur
            vVariant.push_back(TBOLD);
            break;
        case MathMLMathvariantValue::Script:
            // TODO
            break;
        case MathMLMathvariantValue::BoldScript:
            // TODO: Script
            vVariant.push_back(TBOLD);
            break;
        case MathMLMathvariantValue::Fraktur:
            // TODO
            break;
        case MathMLMathvariantValue::SansSerif:
            vVariant.push_back(TSANS);
            break;
        case MathMLMathvariantValue::BoldSansSerif:
            vVariant.push_back(TSANS);
            vVariant.push_back(TBOLD);
            break;
        case MathMLMathvariantValue::SansSerifItalic:
            vVariant.push_back(TITALIC);
            vVariant.push_back(TSANS);
            break;
        case MathMLMathvariantValue::SansSerifBoldItalic:
            vVariant.push_back(TITALIC);
            vVariant.push_back(TBOLD);
            vVariant.push_back(TSANS);
            break;
        case MathMLMathvariantValue::Monospace:
            vVariant.push_back(TFIXED);
            break;
        case MathMLMathvariantValue::Initial:
        case MathMLMathvariantValue::Tailed:
        case MathMLMathvariantValue::Looped:
        case MathMLMathvariantValue::Stretched:
            // TODO
            break;
    }
    if (vVariant.empty())
        return;
    SmNodeStack& rNodeStack = mrContext.GetSmImport().GetNodeStack();
    for (auto eType : vVariant)
    {
        SmToken aToken;
        aToken.eType = eType;
        aToken.cMathChar = '\0';
        aToken.nLevel = 5;
        std::unique_ptr<SmFontNode> pFontNode(new SmFontNode(aToken));
        pFontNode->SetSubNodes(nullptr, popOrZero(rNodeStack));
        rNodeStack.push_front(std::move(pFontNode));
    }
}

namespace
{
class SmXMLDocContext_Impl : public SmXMLImportContext
{
public:
    SmXMLDocContext_Impl(SmXMLImport& rImport)
        : SmXMLImportContext(rImport)
    {
    }

    virtual uno::Reference<xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList) override;

    void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

/*avert the gaze from the originator*/
class SmXMLRowContext_Impl : public SmXMLDocContext_Impl
{
protected:
    size_t nElementCount;

public:
    SmXMLRowContext_Impl(SmXMLImport& rImport)
        : SmXMLDocContext_Impl(rImport)
        , nElementCount(GetSmImport().GetNodeStack().size())
    {
    }

    virtual uno::Reference<xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList) override;

    uno::Reference<xml::sax::XFastContextHandler> StrictCreateChildContext(sal_Int32 nElement);

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

class SmXMLEncloseContext_Impl : public SmXMLRowContext_Impl
{
public:
    // TODO/LATER: convert <menclose notation="horizontalstrike"> into
    // "overstrike{}" and extend the Math syntax to support more notations
    SmXMLEncloseContext_Impl(SmXMLImport& rImport)
        : SmXMLRowContext_Impl(rImport)
    {
    }

    void SAL_CALL endFastElement(sal_Int32 nElement) override;
};
}

void SmXMLEncloseContext_Impl::endFastElement(sal_Int32 nElement)
{
    /*
    <menclose> accepts any number of arguments; if this number is not 1, its
    contents are treated as a single "inferred <mrow>" containing its
    arguments
    */
    if (GetSmImport().GetNodeStack().size() - nElementCount != 1)
        SmXMLRowContext_Impl::endFastElement(nElement);
}

namespace
{
class SmXMLFracContext_Impl : public SmXMLRowContext_Impl
{
public:
    // TODO/LATER: convert <mfrac bevelled="true"> into "wideslash{}{}"
    SmXMLFracContext_Impl(SmXMLImport& rImport)
        : SmXMLRowContext_Impl(rImport)
    {
    }

    void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

class SmXMLSqrtContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLSqrtContext_Impl(SmXMLImport& rImport)
        : SmXMLRowContext_Impl(rImport)
    {
    }

    void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

class SmXMLRootContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLRootContext_Impl(SmXMLImport& rImport)
        : SmXMLRowContext_Impl(rImport)
    {
    }

    void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

class SmXMLStyleContext_Impl : public SmXMLRowContext_Impl
{
protected:
    SmXMLContext_Helper aStyleHelper;

public:
    /*Right now the style tag is completely ignored*/
    SmXMLStyleContext_Impl(SmXMLImport& rImport)
        : SmXMLRowContext_Impl(rImport)
        , aStyleHelper(*this)
    {
    }

    void SAL_CALL endFastElement(sal_Int32 nElement) override;
    void SAL_CALL startFastElement(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList) override;
};
}

void SmXMLStyleContext_Impl::startFastElement(
    sal_Int32 /*nElement*/, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    aStyleHelper.RetrieveAttrs(xAttrList);
}

void SmXMLStyleContext_Impl::endFastElement(sal_Int32 nElement)
{
    /*
    <mstyle> accepts any number of arguments; if this number is not 1, its
    contents are treated as a single "inferred <mrow>" containing its
    arguments
    */
    SmNodeStack& rNodeStack = GetSmImport().GetNodeStack();
    if (rNodeStack.size() - nElementCount != 1)
        SmXMLRowContext_Impl::endFastElement(nElement);
    aStyleHelper.ApplyAttrs();
}

namespace
{
class SmXMLPaddedContext_Impl : public SmXMLRowContext_Impl
{
public:
    /*Right now the style tag is completely ignored*/
    SmXMLPaddedContext_Impl(SmXMLImport& rImport)
        : SmXMLRowContext_Impl(rImport)
    {
    }

    void SAL_CALL endFastElement(sal_Int32 nElement) override;
};
}

void SmXMLPaddedContext_Impl::endFastElement(sal_Int32 nElement)
{
    /*
    <mpadded> accepts any number of arguments; if this number is not 1, its
    contents are treated as a single "inferred <mrow>" containing its
    arguments
    */
    if (GetSmImport().GetNodeStack().size() - nElementCount != 1)
        SmXMLRowContext_Impl::endFastElement(nElement);
}

namespace
{
class SmXMLPhantomContext_Impl : public SmXMLRowContext_Impl
{
public:
    /*Right now the style tag is completely ignored*/
    SmXMLPhantomContext_Impl(SmXMLImport& rImport)
        : SmXMLRowContext_Impl(rImport)
    {
    }

    void SAL_CALL endFastElement(sal_Int32 nElement) override;
};
}

void SmXMLPhantomContext_Impl::endFastElement(sal_Int32 nElement)
{
    /*
    <mphantom> accepts any number of arguments; if this number is not 1, its
    contents are treated as a single "inferred <mrow>" containing its
    arguments
    */
    if (GetSmImport().GetNodeStack().size() - nElementCount != 1)
        SmXMLRowContext_Impl::endFastElement(nElement);

    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.nLevel = 5;
    aToken.eType = TPHANTOM;

    std::unique_ptr<SmFontNode> pPhantom(new SmFontNode(aToken));
    SmNodeStack& rNodeStack = GetSmImport().GetNodeStack();
    pPhantom->SetSubNodes(nullptr, popOrZero(rNodeStack));
    rNodeStack.push_front(std::move(pPhantom));
}

namespace
{
class SmXMLFencedContext_Impl : public SmXMLRowContext_Impl
{
protected:
    sal_Unicode cBegin;
    sal_Unicode cEnd;
    bool bIsStretchy;

public:
    SmXMLFencedContext_Impl(SmXMLImport& rImport)
        : SmXMLRowContext_Impl(rImport)
        , cBegin('(')
        , cEnd(')')
        , bIsStretchy(false)
    {
    }

    void SAL_CALL startFastElement(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList) override;
    void SAL_CALL endFastElement(sal_Int32 nElement) override;
};
}

void SmXMLFencedContext_Impl::startFastElement(
    sal_Int32 /*nElement*/, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    for (auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        switch (aIter.getToken())
        {
            //temp, starmath cannot handle multichar brackets (I think)
            case XML_OPEN:
                cBegin = aIter.toString()[0];
                break;
            case XML_CLOSE:
                cEnd = aIter.toString()[0];
                break;
            case XML_STRETCHY:
                bIsStretchy = IsXMLToken(aIter, XML_TRUE);
                break;
            default:
                XMLOFF_WARN_UNKNOWN("starmath", aIter);
                /*Go to superclass*/
                break;
        }
    }
}

void SmXMLFencedContext_Impl::endFastElement(sal_Int32 /*nElement*/)
{
    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.aText = ",";
    aToken.nLevel = 5;

    std::unique_ptr<SmStructureNode> pSNode(new SmBraceNode(aToken));
    if (bIsStretchy)
        aToken = starmathdatabase::Identify_PrefixPostfix_SmXMLOperatorContext_Impl(cBegin);
    else
        aToken = starmathdatabase::Identify_Prefix_SmXMLOperatorContext_Impl(cBegin);
    if (aToken.eType == TERROR)
        aToken = SmToken(TLPARENT, MS_LPARENT, "(", TG::LBrace, 5);
    std::unique_ptr<SmNode> pLeft(new SmMathSymbolNode(aToken));
    if (bIsStretchy)
        aToken = starmathdatabase::Identify_PrefixPostfix_SmXMLOperatorContext_Impl(cEnd);
    else
        aToken = starmathdatabase::Identify_Postfix_SmXMLOperatorContext_Impl(cEnd);
    if (aToken.eType == TERROR)
        aToken = SmToken(TRPARENT, MS_RPARENT, ")", TG::LBrace, 5);
    std::unique_ptr<SmNode> pRight(new SmMathSymbolNode(aToken));

    SmNodeArray aRelationArray;
    SmNodeStack& rNodeStack = GetSmImport().GetNodeStack();
    aToken.cMathChar = '\0';
    aToken.eType = TIDENT;

    auto i = rNodeStack.size() - nElementCount;
    if (rNodeStack.size() - nElementCount > 1)
        i += rNodeStack.size() - 1 - nElementCount;
    aRelationArray.resize(i);
    while (rNodeStack.size() > nElementCount)
    {
        auto pNode = std::move(rNodeStack.front());
        rNodeStack.pop_front();
        aRelationArray[--i] = pNode.release();
        if (i > 1 && rNodeStack.size() > 1)
            aRelationArray[--i] = new SmGlyphSpecialNode(aToken);
    }

    SmToken aDummy;
    std::unique_ptr<SmStructureNode> pBody(new SmExpressionNode(aDummy));
    pBody->SetSubNodes(std::move(aRelationArray));

    pSNode->SetSubNodes(std::move(pLeft), std::move(pBody), std::move(pRight));
    // mfenced is always scalable. Stretchy keyword is not official, but in case of been in there
    // can be used as a hint.
    pSNode->SetScaleMode(SmScaleMode::Height);
    GetSmImport().GetNodeStack().push_front(std::move(pSNode));
}

namespace
{
class SmXMLErrorContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLErrorContext_Impl(SmXMLImport& rImport)
        : SmXMLRowContext_Impl(rImport)
    {
    }

    void SAL_CALL endFastElement(sal_Int32 nElement) override;
};
}

void SmXMLErrorContext_Impl::endFastElement(sal_Int32 /*nElement*/)
{
    /*Right now the error tag is completely ignored, what
     can I do with it in starmath, ?, maybe we need a
     report window ourselves, do a test for validity of
     the xml input, use mirrors, and then generate
     the markup inside the merror with a big red colour
     of something. For now just throw them all away.
     */
    SmNodeStack& rNodeStack = GetSmImport().GetNodeStack();
    while (rNodeStack.size() > nElementCount)
    {
        rNodeStack.pop_front();
    }
}

namespace
{
class SmXMLNumberContext_Impl : public SmXMLImportContext
{
protected:
    SmToken aToken;

public:
    SmXMLNumberContext_Impl(SmXMLImport& rImport)
        : SmXMLImportContext(rImport)
    {
        aToken.cMathChar = '\0';
        aToken.nLevel = 5;
        aToken.eType = TNUMBER;
    }

    virtual void TCharacters(const OUString& rChars) override;

    void SAL_CALL endFastElement(sal_Int32 nElement) override;
};
}

void SmXMLNumberContext_Impl::TCharacters(const OUString& rChars) { aToken.aText = rChars; }

void SmXMLNumberContext_Impl::endFastElement(sal_Int32)
{
    GetSmImport().GetNodeStack().push_front(std::make_unique<SmTextNode>(aToken, FNT_NUMBER));
}

namespace
{
class SmXMLAnnotationContext_Impl : public SmXMLImportContext
{
    bool bIsStarMath;

public:
    SmXMLAnnotationContext_Impl(SmXMLImport& rImport)
        : SmXMLImportContext(rImport)
        , bIsStarMath(false)
    {
    }

    void SAL_CALL characters(const OUString& rChars) override;

    void SAL_CALL startFastElement(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList) override;
};
}

void SmXMLAnnotationContext_Impl::startFastElement(
    sal_Int32 /*nElement*/, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    for (auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        // sometimes they have namespace, sometimes not?
        switch (aIter.getToken() & TOKEN_MASK)
        {
            case XML_ENCODING:
                bIsStarMath = aIter.toView() == "StarMath 5.0";
                break;
            default:
                XMLOFF_WARN_UNKNOWN("starmath", aIter);
                break;
        }
    }
}

void SmXMLAnnotationContext_Impl::characters(const OUString& rChars)
{
    if (bIsStarMath)
        GetSmImport().SetText(GetSmImport().GetText() + rChars);
}

namespace
{
class SmXMLTextContext_Impl : public SmXMLImportContext
{
protected:
    SmToken aToken;

public:
    SmXMLTextContext_Impl(SmXMLImport& rImport)
        : SmXMLImportContext(rImport)
    {
        aToken.cMathChar = '\0';
        aToken.nLevel = 5;
        aToken.eType = TTEXT;
    }

    virtual void TCharacters(const OUString& rChars) override;

    void SAL_CALL endFastElement(sal_Int32 nElement) override;
};
}

void SmXMLTextContext_Impl::TCharacters(const OUString& rChars) { aToken.aText = rChars; }

void SmXMLTextContext_Impl::endFastElement(sal_Int32)
{
    GetSmImport().GetNodeStack().push_front(std::make_unique<SmTextNode>(aToken, FNT_TEXT));
}

namespace
{
class SmXMLStringContext_Impl : public SmXMLImportContext
{
protected:
    SmToken aToken;

public:
    SmXMLStringContext_Impl(SmXMLImport& rImport)
        : SmXMLImportContext(rImport)
    {
        aToken.cMathChar = '\0';
        aToken.nLevel = 5;
        aToken.eType = TTEXT;
    }

    virtual void TCharacters(const OUString& rChars) override;

    void SAL_CALL endFastElement(sal_Int32 nElement) override;
};
}

void SmXMLStringContext_Impl::TCharacters(const OUString& rChars)
{
    /*
    The content of <ms> elements should be rendered with visible "escaping" of
    certain characters in the content, including at least "double quote"
    itself, and preferably whitespace other than individual blanks. The intent
    is for the viewer to see that the expression is a string literal, and to
    see exactly which characters form its content. For example, <ms>double
    quote is "</ms> might be rendered as "double quote is \"".

    Obviously this isn't fully done here.
    */
    aToken.aText = "\"" + rChars + "\"";
}

void SmXMLStringContext_Impl::endFastElement(sal_Int32)
{
    GetSmImport().GetNodeStack().push_front(std::make_unique<SmTextNode>(aToken, FNT_FIXED));
}

namespace
{
class SmXMLIdentifierContext_Impl : public SmXMLImportContext
{
    SmXMLTokenAttrHelper maTokenAttrHelper;
    SmXMLContext_Helper aStyleHelper;
    SmToken aToken;

public:
    SmXMLIdentifierContext_Impl(SmXMLImport& rImport)
        : SmXMLImportContext(rImport)
        , maTokenAttrHelper(*this)
        , aStyleHelper(*this)
    {
        aToken.cMathChar = '\0';
        aToken.nLevel = 5;
        aToken.eType = TIDENT;
    }

    void TCharacters(const OUString& rChars) override;
    void SAL_CALL
    startFastElement(sal_Int32 /*nElement*/,
                     const uno::Reference<xml::sax::XFastAttributeList>& xAttrList) override
    {
        maTokenAttrHelper.RetrieveAttrs(xAttrList);
        aStyleHelper.RetrieveAttrs(xAttrList);
    };
    void SAL_CALL endFastElement(sal_Int32 nElement) override;
};
}

void SmXMLIdentifierContext_Impl::endFastElement(sal_Int32)
{
    std::unique_ptr<SmTextNode> pNode;
    //we will handle identifier italic/normal here instead of with a standalone
    //font node
    if (((aStyleHelper.nIsItalic == -1) && (aToken.aText.getLength() > 1))
        || ((aStyleHelper.nIsItalic == 0) && (aToken.aText.getLength() == 1)))
    {
        pNode.reset(new SmTextNode(aToken, FNT_FUNCTION));
        pNode->GetFont().SetItalic(ITALIC_NONE);
        aStyleHelper.nIsItalic = -1;
    }
    else
        pNode.reset(new SmTextNode(aToken, FNT_VARIABLE));
    if (aStyleHelper.nIsItalic != -1)
    {
        if (aStyleHelper.nIsItalic)
            pNode->GetFont().SetItalic(ITALIC_NORMAL);
        else
            pNode->GetFont().SetItalic(ITALIC_NONE);
        aStyleHelper.nIsItalic = -1;
    }
    GetSmImport().GetNodeStack().push_front(std::move(pNode));
    aStyleHelper.ApplyAttrs();

    maTokenAttrHelper.ApplyAttrs((aToken.aText.getLength() == 1) ? MathMLMathvariantValue::Italic
                                                                 : MathMLMathvariantValue::Normal);
}

void SmXMLIdentifierContext_Impl::TCharacters(const OUString& rChars) { aToken.aText = rChars; }

namespace
{
class SmXMLOperatorContext_Impl : public SmXMLImportContext
{
    SmXMLTokenAttrHelper maTokenAttrHelper;
    bool bIsStretchy;
    bool bIsFenced;
    bool isPrefix;
    bool isInfix;
    bool isPostfix;
    SmToken aToken;

public:
    SmXMLOperatorContext_Impl(SmXMLImport& rImport)
        : SmXMLImportContext(rImport)
        , maTokenAttrHelper(*this)
        , bIsStretchy(false)
        , bIsFenced(false)
        , isPrefix(false)
        , isInfix(false)
        , isPostfix(false)
    {
        aToken.eType = TSPECIAL;
        aToken.nLevel = 5;
    }

    void TCharacters(const OUString& rChars) override;
    void SAL_CALL startFastElement(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList) override;
    void SAL_CALL endFastElement(sal_Int32 nElement) override;
};
}

void SmXMLOperatorContext_Impl::TCharacters(const OUString& rChars)
{
    aToken.cMathChar = rChars[0];
    SmToken bToken;
    if (bIsFenced)
    {
        if (isPrefix)
            bToken = starmathdatabase::Identify_Prefix_SmXMLOperatorContext_Impl(aToken.cMathChar);
        else if (isInfix)
            bToken = SmToken(TMLINE, MS_VERTLINE, "mline", TG::NONE, 0);
        else if (isPostfix)
            bToken = starmathdatabase::Identify_Postfix_SmXMLOperatorContext_Impl(aToken.cMathChar);
        else
            bToken = starmathdatabase::Identify_PrefixPostfix_SmXMLOperatorContext_Impl(
                aToken.cMathChar);
    }
    else
        bToken
            = starmathdatabase::Identify_SmXMLOperatorContext_Impl(aToken.cMathChar, bIsStretchy);
    if (bToken.eType != TERROR)
        aToken = bToken;
}

void SmXMLOperatorContext_Impl::endFastElement(sal_Int32)
{
    std::unique_ptr<SmMathSymbolNode> pNode(new SmMathSymbolNode(aToken));
    //For stretchy scaling the scaling must be retrieved from this node
    //and applied to the expression itself so as to get the expression
    //to scale the operator to the height of the expression itself
    if (bIsStretchy)
        pNode->SetScaleMode(SmScaleMode::Height);
    GetSmImport().GetNodeStack().push_front(std::move(pNode));

    // TODO: apply to non-alphabetic characters too
    if (rtl::isAsciiAlpha(aToken.cMathChar))
        maTokenAttrHelper.ApplyAttrs(MathMLMathvariantValue::Normal);
}

void SmXMLOperatorContext_Impl::startFastElement(
    sal_Int32 /*nElement*/, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    maTokenAttrHelper.RetrieveAttrs(xAttrList);

    for (auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        switch (aIter.getToken())
        {
            case XML_STRETCHY:
                bIsStretchy = IsXMLToken(aIter, XML_TRUE);
                break;
            case XML_FENCE:
                bIsFenced = IsXMLToken(aIter, XML_TRUE);
                break;
            case XML_FORM:
                isPrefix = IsXMLToken(aIter, XML_PREFIX); // <
                isInfix = IsXMLToken(aIter, XML_INFIX); // |
                isPostfix = IsXMLToken(aIter, XML_POSTFIX); // >
                break;
            default:
                XMLOFF_WARN_UNKNOWN("starmath", aIter);
                break;
        }
    }
}

namespace
{
class SmXMLSpaceContext_Impl : public SmXMLImportContext
{
public:
    SmXMLSpaceContext_Impl(SmXMLImport& rImport)
        : SmXMLImportContext(rImport)
    {
    }

    void SAL_CALL startFastElement(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList) override;
};

bool lcl_CountBlanks(const MathMLAttributeLengthValue& rLV, sal_Int32* pWide, sal_Int32* pNarrow)
{
    assert(pWide);
    assert(pNarrow);
    if (rLV.aNumber.GetNumerator() == 0)
    {
        *pWide = *pNarrow = 0;
        return true;
    }
    // TODO: honor other units than em
    if (rLV.eUnit != MathMLLengthUnit::Em)
        return false;
    if (rLV.aNumber.GetNumerator() < 0)
        return false;
    const Fraction aTwo(2, 1);
    auto aWide = rLV.aNumber / aTwo;
    auto nWide = static_cast<sal_Int32>(static_cast<tools::Long>(aWide));
    if (nWide < 0)
        return false;
    const Fraction aPointFive(1, 2);
    auto aNarrow = (rLV.aNumber - Fraction(nWide, 1) * aTwo) / aPointFive;
    auto nNarrow = static_cast<sal_Int32>(static_cast<tools::Long>(aNarrow));
    if (nNarrow < 0)
        return false;
    *pWide = nWide;
    *pNarrow = nNarrow;
    return true;
}
}

void SmXMLSpaceContext_Impl::startFastElement(
    sal_Int32 /*nElement*/, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    // There is no syntax in Math to specify blank nodes of arbitrary size yet.
    MathMLAttributeLengthValue aLV;
    sal_Int32 nWide = 0, nNarrow = 0;

    for (auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        OUString sValue = aIter.toString();
        switch (aIter.getToken())
        {
            case XML_WIDTH:
                if (ParseMathMLAttributeLengthValue(sValue.trim(), aLV) <= 0
                    || !lcl_CountBlanks(aLV, &nWide, &nNarrow))
                    SAL_WARN("starmath", "ignore mspace's width: " << sValue);
                break;
            default:
                XMLOFF_WARN_UNKNOWN("starmath", aIter);
                break;
        }
    }
    SmToken aToken;
    aToken.eType = TBLANK;
    aToken.cMathChar = '\0';
    aToken.nGroup = TG::Blank;
    aToken.nLevel = 5;
    std::unique_ptr<SmBlankNode> pBlank(new SmBlankNode(aToken));
    if (nWide > 0)
        pBlank->IncreaseBy(aToken, nWide);
    if (nNarrow > 0)
    {
        aToken.eType = TSBLANK;
        pBlank->IncreaseBy(aToken, nNarrow);
    }
    GetSmImport().GetNodeStack().push_front(std::move(pBlank));
}

namespace
{
class SmXMLSubContext_Impl : public SmXMLRowContext_Impl
{
protected:
    void GenericEndElement(SmTokenType eType, SmSubSup aSubSup);

public:
    SmXMLSubContext_Impl(SmXMLImport& rImport)
        : SmXMLRowContext_Impl(rImport)
    {
    }

    void SAL_CALL endFastElement(sal_Int32) override { GenericEndElement(TRSUB, RSUB); }
};
}

void SmXMLSubContext_Impl::GenericEndElement(SmTokenType eType, SmSubSup eSubSup)
{
    /*The <msub> element requires exactly 2 arguments.*/
    const bool bNodeCheck = GetSmImport().GetNodeStack().size() - nElementCount == 2;
    OSL_ENSURE(bNodeCheck, "Sub has not two arguments");
    if (!bNodeCheck)
        return;

    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.eType = eType;
    std::unique_ptr<SmSubSupNode> pNode(new SmSubSupNode(aToken));
    SmNodeStack& rNodeStack = GetSmImport().GetNodeStack();

    // initialize subnodes array
    SmNodeArray aSubNodes;
    aSubNodes.resize(1 + SUBSUP_NUM_ENTRIES);
    for (size_t i = 1; i < aSubNodes.size(); i++)
        aSubNodes[i] = nullptr;

    aSubNodes[eSubSup + 1] = popOrZero(rNodeStack).release();
    aSubNodes[0] = popOrZero(rNodeStack).release();
    pNode->SetSubNodes(std::move(aSubNodes));
    rNodeStack.push_front(std::move(pNode));
}

namespace
{
class SmXMLSupContext_Impl : public SmXMLSubContext_Impl
{
public:
    SmXMLSupContext_Impl(SmXMLImport& rImport)
        : SmXMLSubContext_Impl(rImport)
    {
    }

    void SAL_CALL endFastElement(sal_Int32) override { GenericEndElement(TRSUP, RSUP); }
};

class SmXMLSubSupContext_Impl : public SmXMLRowContext_Impl
{
protected:
    void GenericEndElement(SmTokenType eType, SmSubSup aSub, SmSubSup aSup);

public:
    SmXMLSubSupContext_Impl(SmXMLImport& rImport)
        : SmXMLRowContext_Impl(rImport)
    {
    }

    void SAL_CALL endFastElement(sal_Int32) override { GenericEndElement(TRSUB, RSUB, RSUP); }
};
}

void SmXMLSubSupContext_Impl::GenericEndElement(SmTokenType eType, SmSubSup aSub, SmSubSup aSup)
{
    /*The <msub> element requires exactly 3 arguments.*/
    const bool bNodeCheck = GetSmImport().GetNodeStack().size() - nElementCount == 3;
    OSL_ENSURE(bNodeCheck, "SubSup has not three arguments");
    if (!bNodeCheck)
        return;

    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.eType = eType;
    std::unique_ptr<SmSubSupNode> pNode(new SmSubSupNode(aToken));
    SmNodeStack& rNodeStack = GetSmImport().GetNodeStack();

    // initialize subnodes array
    SmNodeArray aSubNodes;
    aSubNodes.resize(1 + SUBSUP_NUM_ENTRIES);
    for (size_t i = 1; i < aSubNodes.size(); i++)
        aSubNodes[i] = nullptr;

    aSubNodes[aSup + 1] = popOrZero(rNodeStack).release();
    aSubNodes[aSub + 1] = popOrZero(rNodeStack).release();
    aSubNodes[0] = popOrZero(rNodeStack).release();
    pNode->SetSubNodes(std::move(aSubNodes));
    rNodeStack.push_front(std::move(pNode));
}

namespace
{
class SmXMLUnderContext_Impl : public SmXMLSubContext_Impl
{
protected:
    sal_Int16 nAttrCount;

public:
    SmXMLUnderContext_Impl(SmXMLImport& rImport)
        : SmXMLSubContext_Impl(rImport)
        , nAttrCount(0)
    {
    }

    void SAL_CALL startFastElement(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList) override;
    void SAL_CALL endFastElement(sal_Int32 nElement) override;
    void HandleAccent();
};
}

void SmXMLUnderContext_Impl::startFastElement(
    sal_Int32 /*nElement*/, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    sax_fastparser::FastAttributeList& rAttribList
        = sax_fastparser::castToFastAttributeList(xAttrList);
    nAttrCount = rAttribList.getFastAttributeTokens().size();
}

void SmXMLUnderContext_Impl::HandleAccent()
{
    const bool bNodeCheck = GetSmImport().GetNodeStack().size() - nElementCount == 2;
    OSL_ENSURE(bNodeCheck, "Sub has not two arguments");
    if (!bNodeCheck)
        return;

    /*Just one special case for the underline thing*/
    SmNodeStack& rNodeStack = GetSmImport().GetNodeStack();
    std::unique_ptr<SmNode> pTest = popOrZero(rNodeStack);
    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.eType = TUNDERLINE;

    std::unique_ptr<SmNode> pFirst;
    std::unique_ptr<SmStructureNode> pNode(new SmAttributNode(aToken));
    if ((pTest->GetToken().cMathChar & 0x0FFF) == 0x0332)
    {
        pFirst.reset(new SmRectangleNode(aToken));
    }
    else
        pFirst = std::move(pTest);

    std::unique_ptr<SmNode> pSecond = popOrZero(rNodeStack);
    pNode->SetSubNodes(std::move(pFirst), std::move(pSecond));
    pNode->SetScaleMode(SmScaleMode::Width);
    rNodeStack.push_front(std::move(pNode));
}

void SmXMLUnderContext_Impl::endFastElement(sal_Int32)
{
    if (!nAttrCount)
        GenericEndElement(TCSUB, CSUB);
    else
        HandleAccent();
}

namespace
{
class SmXMLOverContext_Impl : public SmXMLSubContext_Impl
{
protected:
    sal_Int16 nAttrCount;

public:
    SmXMLOverContext_Impl(SmXMLImport& rImport)
        : SmXMLSubContext_Impl(rImport)
        , nAttrCount(0)
    {
    }

    void SAL_CALL endFastElement(sal_Int32 nElement) override;
    void SAL_CALL startFastElement(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList) override;
    void HandleAccent();
};
}

void SmXMLOverContext_Impl::startFastElement(
    sal_Int32 /*nElement*/, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    sax_fastparser::FastAttributeList& rAttribList
        = sax_fastparser::castToFastAttributeList(xAttrList);
    nAttrCount = rAttribList.getFastAttributeTokens().size();
}

void SmXMLOverContext_Impl::endFastElement(sal_Int32)
{
    if (!nAttrCount)
        GenericEndElement(TCSUP, CSUP);
    else
        HandleAccent();
}

void SmXMLOverContext_Impl::HandleAccent()
{
    const bool bNodeCheck = GetSmImport().GetNodeStack().size() - nElementCount == 2;
    OSL_ENSURE(bNodeCheck, "Sub has not two arguments");
    if (!bNodeCheck)
        return;

    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.eType = TACUTE;

    std::unique_ptr<SmAttributNode> pNode(new SmAttributNode(aToken));
    SmNodeStack& rNodeStack = GetSmImport().GetNodeStack();

    std::unique_ptr<SmNode> pFirst = popOrZero(rNodeStack);
    std::unique_ptr<SmNode> pSecond = popOrZero(rNodeStack);
    pNode->SetSubNodes(std::move(pFirst), std::move(pSecond));
    pNode->SetScaleMode(SmScaleMode::Width);
    rNodeStack.push_front(std::move(pNode));
}

namespace
{
class SmXMLUnderOverContext_Impl : public SmXMLSubSupContext_Impl
{
public:
    SmXMLUnderOverContext_Impl(SmXMLImport& rImport)
        : SmXMLSubSupContext_Impl(rImport)
    {
    }

    void SAL_CALL endFastElement(sal_Int32) override { GenericEndElement(TCSUB, CSUB, CSUP); }
};

class SmXMLMultiScriptsContext_Impl : public SmXMLSubSupContext_Impl
{
    bool bHasPrescripts;

    void ProcessSubSupPairs(bool bIsPrescript);

public:
    SmXMLMultiScriptsContext_Impl(SmXMLImport& rImport)
        : SmXMLSubSupContext_Impl(rImport)
        , bHasPrescripts(false)
    {
    }

    void SAL_CALL endFastElement(sal_Int32 nElement) override;
    virtual uno::Reference<xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList) override;
};

class SmXMLNoneContext_Impl : public SmXMLImportContext
{
public:
    SmXMLNoneContext_Impl(SmXMLImport& rImport)
        : SmXMLImportContext(rImport)
    {
    }

    void SAL_CALL endFastElement(sal_Int32 nElement) override;
};
}

void SmXMLNoneContext_Impl::endFastElement(sal_Int32)
{
    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.aText.clear();
    aToken.nLevel = 5;
    aToken.eType = TIDENT;
    GetSmImport().GetNodeStack().push_front(std::make_unique<SmTextNode>(aToken, FNT_VARIABLE));
}

namespace
{
class SmXMLPrescriptsContext_Impl : public SmXMLImportContext
{
public:
    SmXMLPrescriptsContext_Impl(SmXMLImport& rImport)
        : SmXMLImportContext(rImport)
    {
    }
};

class SmXMLTableRowContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLTableRowContext_Impl(SmXMLImport& rImport)
        : SmXMLRowContext_Impl(rImport)
    {
    }

    virtual uno::Reference<xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList) override;
};

class SmXMLTableContext_Impl : public SmXMLTableRowContext_Impl
{
public:
    SmXMLTableContext_Impl(SmXMLImport& rImport)
        : SmXMLTableRowContext_Impl(rImport)
    {
    }

    void SAL_CALL endFastElement(sal_Int32 nElement) override;
    virtual uno::Reference<xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList) override;
};

class SmXMLTableCellContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLTableCellContext_Impl(SmXMLImport& rImport)
        : SmXMLRowContext_Impl(rImport)
    {
    }
};

class SmXMLAlignGroupContext_Impl : public SmXMLRowContext_Impl
{
public:
    SmXMLAlignGroupContext_Impl(SmXMLImport& rImport)
        : SmXMLRowContext_Impl(rImport)
    {
    }

    /*Don't do anything with alignment for now*/
};

class SmXMLActionContext_Impl : public SmXMLRowContext_Impl
{
    size_t mnSelection; // 1-based

public:
    SmXMLActionContext_Impl(SmXMLImport& rImport)
        : SmXMLRowContext_Impl(rImport)
        , mnSelection(1)
    {
    }

    void SAL_CALL startFastElement(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList) override;
    void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

// NB: virtually inherit so we can multiply inherit properly
//     in SmXMLFlatDocContext_Impl
class SmXMLOfficeContext_Impl : public virtual SvXMLImportContext
{
public:
    SmXMLOfficeContext_Impl(SmXMLImport& rImport)
        : SvXMLImportContext(rImport)
    {
    }

    virtual css::uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList) override;
};
}

uno::Reference<xml::sax::XFastContextHandler> SmXMLOfficeContext_Impl::createFastChildContext(
    sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& /*xAttrList*/)
{
    if (nElement == XML_ELEMENT(OFFICE, XML_META))
    {
        SAL_WARN("starmath",
                 "XML_TOK_DOC_META: should not have come here, maybe document is invalid?");
    }
    else if (nElement == XML_ELEMENT(OFFICE, XML_SETTINGS))
    {
        return new XMLDocumentSettingsContext(GetImport());
    }
    return nullptr;
}

namespace
{
// context for flat file xml format
class SmXMLFlatDocContext_Impl : public SmXMLOfficeContext_Impl, public SvXMLMetaDocumentContext
{
public:
    SmXMLFlatDocContext_Impl(SmXMLImport& i_rImport,
                             const uno::Reference<document::XDocumentProperties>& i_xDocProps);

    virtual css::uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList) override;
};
}

SmXMLFlatDocContext_Impl::SmXMLFlatDocContext_Impl(
    SmXMLImport& i_rImport, const uno::Reference<document::XDocumentProperties>& i_xDocProps)
    : SvXMLImportContext(i_rImport)
    , SmXMLOfficeContext_Impl(i_rImport)
    , SvXMLMetaDocumentContext(i_rImport, i_xDocProps)
{
}

uno::Reference<xml::sax::XFastContextHandler>
    SAL_CALL SmXMLFlatDocContext_Impl::createFastChildContext(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    // behave like meta base class iff we encounter office:meta
    if (nElement == XML_ELEMENT(OFFICE, XML_META))
    {
        return SvXMLMetaDocumentContext::createFastChildContext(nElement, xAttrList);
    }
    else
    {
        return SmXMLOfficeContext_Impl::createFastChildContext(nElement, xAttrList);
    }
}

uno::Reference<xml::sax::XFastContextHandler> SmXMLDocContext_Impl::createFastChildContext(
    sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& /*xAttrList*/)
{
    uno::Reference<xml::sax::XFastContextHandler> xContext;

    switch (nElement)
    {
        //Consider semantics a dummy except for any starmath annotations
        case XML_ELEMENT(MATH, XML_SEMANTICS):
            xContext = new SmXMLRowContext_Impl(GetSmImport());
            break;
        /*General Layout Schemata*/
        case XML_ELEMENT(MATH, XML_MROW):
            xContext = new SmXMLRowContext_Impl(GetSmImport());
            break;
        case XML_ELEMENT(MATH, XML_MENCLOSE):
            xContext = new SmXMLEncloseContext_Impl(GetSmImport());
            break;
        case XML_ELEMENT(MATH, XML_MFRAC):
            xContext = new SmXMLFracContext_Impl(GetSmImport());
            break;
        case XML_ELEMENT(MATH, XML_MSQRT):
            xContext = new SmXMLSqrtContext_Impl(GetSmImport());
            break;
        case XML_ELEMENT(MATH, XML_MROOT):
            xContext = new SmXMLRootContext_Impl(GetSmImport());
            break;
        case XML_ELEMENT(MATH, XML_MSTYLE):
            xContext = new SmXMLStyleContext_Impl(GetSmImport());
            break;
        case XML_ELEMENT(MATH, XML_MERROR):
            xContext = new SmXMLErrorContext_Impl(GetSmImport());
            break;
        case XML_ELEMENT(MATH, XML_MPADDED):
            xContext = new SmXMLPaddedContext_Impl(GetSmImport());
            break;
        case XML_ELEMENT(MATH, XML_MPHANTOM):
            xContext = new SmXMLPhantomContext_Impl(GetSmImport());
            break;
        case XML_ELEMENT(MATH, XML_MFENCED):
            xContext = new SmXMLFencedContext_Impl(GetSmImport());
            break;
        /*Script and Limit Schemata*/
        case XML_ELEMENT(MATH, XML_MSUB):
            xContext = new SmXMLSubContext_Impl(GetSmImport());
            break;
        case XML_ELEMENT(MATH, XML_MSUP):
            xContext = new SmXMLSupContext_Impl(GetSmImport());
            break;
        case XML_ELEMENT(MATH, XML_MSUBSUP):
            xContext = new SmXMLSubSupContext_Impl(GetSmImport());
            break;
        case XML_ELEMENT(MATH, XML_MUNDER):
            xContext = new SmXMLUnderContext_Impl(GetSmImport());
            break;
        case XML_ELEMENT(MATH, XML_MOVER):
            xContext = new SmXMLOverContext_Impl(GetSmImport());
            break;
        case XML_ELEMENT(MATH, XML_MUNDEROVER):
            xContext = new SmXMLUnderOverContext_Impl(GetSmImport());
            break;
        case XML_ELEMENT(MATH, XML_MMULTISCRIPTS):
            xContext = new SmXMLMultiScriptsContext_Impl(GetSmImport());
            break;
        case XML_ELEMENT(MATH, XML_MTABLE):
            xContext = new SmXMLTableContext_Impl(GetSmImport());
            break;
        case XML_ELEMENT(MATH, XML_MACTION):
            xContext = new SmXMLActionContext_Impl(GetSmImport());
            break;
        default:
            /*Basically there's an implicit mrow around certain bare
             *elements, use a RowContext to see if this is one of
             *those ones*/
            rtl::Reference<SmXMLRowContext_Impl> aTempContext(
                new SmXMLRowContext_Impl(GetSmImport()));

            xContext = aTempContext->StrictCreateChildContext(nElement);
            break;
    }
    return xContext;
}

void SmXMLDocContext_Impl::endFastElement(sal_Int32)
{
    SmNodeStack& rNodeStack = GetSmImport().GetNodeStack();

    std::unique_ptr<SmNode> pContextNode = popOrZero(rNodeStack);

    SmToken aDummy;
    std::unique_ptr<SmStructureNode> pSNode(new SmLineNode(aDummy));
    pSNode->SetSubNodes(std::move(pContextNode), nullptr);
    rNodeStack.push_front(std::move(pSNode));

    SmNodeArray LineArray;
    auto n = rNodeStack.size();
    LineArray.resize(n);
    for (size_t j = 0; j < n; j++)
    {
        auto pNode = std::move(rNodeStack.front());
        rNodeStack.pop_front();
        LineArray[n - (j + 1)] = pNode.release();
    }
    std::unique_ptr<SmStructureNode> pSNode2(new SmTableNode(aDummy));
    pSNode2->SetSubNodes(std::move(LineArray));
    rNodeStack.push_front(std::move(pSNode2));
}

void SmXMLFracContext_Impl::endFastElement(sal_Int32)
{
    SmNodeStack& rNodeStack = GetSmImport().GetNodeStack();
    const bool bNodeCheck = rNodeStack.size() - nElementCount == 2;
    OSL_ENSURE(bNodeCheck, "Fraction (mfrac) tag is missing component");
    if (!bNodeCheck)
        return;

    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.eType = TFRAC;
    std::unique_ptr<SmStructureNode> pSNode(new SmBinVerNode(aToken));
    std::unique_ptr<SmNode> pOper(new SmRectangleNode(aToken));
    std::unique_ptr<SmNode> pSecond = popOrZero(rNodeStack);
    std::unique_ptr<SmNode> pFirst = popOrZero(rNodeStack);
    pSNode->SetSubNodes(std::move(pFirst), std::move(pOper), std::move(pSecond));
    rNodeStack.push_front(std::move(pSNode));
}

void SmXMLRootContext_Impl::endFastElement(sal_Int32)
{
    /*The <mroot> element requires exactly 2 arguments.*/
    const bool bNodeCheck = GetSmImport().GetNodeStack().size() - nElementCount == 2;
    OSL_ENSURE(bNodeCheck, "Root tag is missing component");
    if (!bNodeCheck)
        return;

    SmToken aToken;
    aToken.cMathChar = MS_SQRT; //Temporary: alert, based on StarSymbol font
    aToken.eType = TNROOT;
    std::unique_ptr<SmStructureNode> pSNode(new SmRootNode(aToken));
    std::unique_ptr<SmNode> pOper(new SmRootSymbolNode(aToken));
    SmNodeStack& rNodeStack = GetSmImport().GetNodeStack();
    std::unique_ptr<SmNode> pIndex = popOrZero(rNodeStack);
    std::unique_ptr<SmNode> pBase = popOrZero(rNodeStack);
    pSNode->SetSubNodes(std::move(pIndex), std::move(pOper), std::move(pBase));
    rNodeStack.push_front(std::move(pSNode));
}

void SmXMLSqrtContext_Impl::endFastElement(sal_Int32 nElement)
{
    /*
    <msqrt> accepts any number of arguments; if this number is not 1, its
    contents are treated as a single "inferred <mrow>" containing its
    arguments
    */
    if (GetSmImport().GetNodeStack().size() - nElementCount != 1)
        SmXMLRowContext_Impl::endFastElement(nElement);

    SmToken aToken;
    aToken.cMathChar = MS_SQRT; //Temporary: alert, based on StarSymbol font
    aToken.eType = TSQRT;
    std::unique_ptr<SmStructureNode> pSNode(new SmRootNode(aToken));
    std::unique_ptr<SmNode> pOper(new SmRootSymbolNode(aToken));
    SmNodeStack& rNodeStack = GetSmImport().GetNodeStack();
    pSNode->SetSubNodes(nullptr, std::move(pOper), popOrZero(rNodeStack));
    rNodeStack.push_front(std::move(pSNode));
}

void SmXMLRowContext_Impl::endFastElement(sal_Int32)
{
    SmNodeArray aRelationArray;
    SmNodeStack& rNodeStack = GetSmImport().GetNodeStack();

    if (rNodeStack.size() > nElementCount)
    {
        auto nSize = rNodeStack.size() - nElementCount;

        aRelationArray.resize(nSize);
        for (auto j = nSize; j > 0; j--)
        {
            auto pNode = std::move(rNodeStack.front());
            rNodeStack.pop_front();
            aRelationArray[j - 1] = pNode.release();
        }

        //If the first or last element is an operator with stretchyness
        //set then we must create a brace node here from those elements,
        //removing the stretchness from the operators and applying it to
        //ourselves, and creating the appropriate dummy StarMath none bracket
        //to balance the arrangement
        if (((aRelationArray[0]->GetScaleMode() == SmScaleMode::Height)
             && (aRelationArray[0]->GetType() == SmNodeType::Math))
            || ((aRelationArray[nSize - 1]->GetScaleMode() == SmScaleMode::Height)
                && (aRelationArray[nSize - 1]->GetType() == SmNodeType::Math)))
        {
            SmToken aToken;
            aToken.cMathChar = '\0';
            aToken.nLevel = 5;

            int nLeft = 0, nRight = 0;
            if ((aRelationArray[0]->GetScaleMode() == SmScaleMode::Height)
                && (aRelationArray[0]->GetType() == SmNodeType::Math))
            {
                aToken = aRelationArray[0]->GetToken();
                nLeft = 1;
            }
            else
                aToken.cMathChar = '\0';

            aToken.eType = TLPARENT;
            std::unique_ptr<SmNode> pLeft(new SmMathSymbolNode(aToken));

            if ((aRelationArray[nSize - 1]->GetScaleMode() == SmScaleMode::Height)
                && (aRelationArray[nSize - 1]->GetType() == SmNodeType::Math))
            {
                aToken = aRelationArray[nSize - 1]->GetToken();
                nRight = 1;
            }
            else
                aToken.cMathChar = '\0';

            aToken.eType = TRPARENT;
            std::unique_ptr<SmNode> pRight(new SmMathSymbolNode(aToken));

            SmNodeArray aRelationArray2;

            //!! nSize-nLeft-nRight may be < 0 !!
            int nRelArrSize = nSize - nLeft - nRight;
            if (nRelArrSize > 0)
            {
                aRelationArray2.resize(nRelArrSize);
                for (int i = 0; i < nRelArrSize; i++)
                {
                    aRelationArray2[i] = aRelationArray[i + nLeft];
                    aRelationArray[i + nLeft] = nullptr;
                }
            }

            SmToken aDummy;
            std::unique_ptr<SmStructureNode> pSNode(new SmBraceNode(aToken));
            std::unique_ptr<SmStructureNode> pBody(new SmExpressionNode(aDummy));
            pBody->SetSubNodes(std::move(aRelationArray2));

            pSNode->SetSubNodes(std::move(pLeft), std::move(pBody), std::move(pRight));
            pSNode->SetScaleMode(SmScaleMode::Height);
            rNodeStack.push_front(std::move(pSNode));

            for (auto a : aRelationArray)
                delete a;

            return;
        }
    }
    else
    {
        // The elements msqrt, mstyle, merror, menclose, mpadded, mphantom, mtd, and math
        // treat their content as a single inferred mrow in case their content is empty.
        // Here an empty group {} is used to catch those cases and transform them without error
        // to StarMath.
        aRelationArray.resize(2);
        SmToken aToken;
        aToken.cMathChar = MS_LBRACE;
        aToken.nLevel = 5;
        aToken.eType = TLGROUP;
        aToken.nGroup = TG::NONE;
        aToken.aText = "{";
        aRelationArray[0] = new SmLineNode(aToken);

        aToken.cMathChar = MS_RBRACE;
        aToken.nLevel = 0;
        aToken.eType = TRGROUP;
        aToken.nGroup = TG::NONE;
        aToken.aText = "}";
        aRelationArray[1] = new SmLineNode(aToken);
    }

    SmToken aDummy;
    std::unique_ptr<SmStructureNode> pSNode(new SmExpressionNode(aDummy));
    pSNode->SetSubNodes(std::move(aRelationArray));
    rNodeStack.push_front(std::move(pSNode));
}

uno::Reference<xml::sax::XFastContextHandler>
SmXMLRowContext_Impl::StrictCreateChildContext(sal_Int32 nElement)
{
    uno::Reference<xml::sax::XFastContextHandler> pContext;

    switch (nElement)
    {
        /*Note that these should accept malignmark subelements, but do not*/
        case XML_ELEMENT(MATH, XML_MN):
            pContext = new SmXMLNumberContext_Impl(GetSmImport());
            break;
        case XML_ELEMENT(MATH, XML_MI):
            pContext = new SmXMLIdentifierContext_Impl(GetSmImport());
            break;
        case XML_ELEMENT(MATH, XML_MO):
            pContext = new SmXMLOperatorContext_Impl(GetSmImport());
            break;
        case XML_ELEMENT(MATH, XML_MTEXT):
            pContext = new SmXMLTextContext_Impl(GetSmImport());
            break;
        case XML_ELEMENT(MATH, XML_MSPACE):
            pContext = new SmXMLSpaceContext_Impl(GetSmImport());
            break;
        case XML_ELEMENT(MATH, XML_MS):
            pContext = new SmXMLStringContext_Impl(GetSmImport());
            break;

        /*Note: The maligngroup should only be seen when the row
         * (or descendants) are in a table*/
        case XML_ELEMENT(MATH, XML_MALIGNGROUP):
            pContext = new SmXMLAlignGroupContext_Impl(GetSmImport());
            break;

        case XML_ELEMENT(MATH, XML_ANNOTATION):
            pContext = new SmXMLAnnotationContext_Impl(GetSmImport());
            break;

        default:
            break;
    }
    return pContext;
}

uno::Reference<xml::sax::XFastContextHandler> SmXMLRowContext_Impl::createFastChildContext(
    sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    uno::Reference<xml::sax::XFastContextHandler> xContext = StrictCreateChildContext(nElement);

    if (!xContext)
    {
        //Hmm, unrecognized for this level, check to see if its
        //an element that can have an implicit schema around it
        xContext = SmXMLDocContext_Impl::createFastChildContext(nElement, xAttrList);
    }
    return xContext;
}

uno::Reference<xml::sax::XFastContextHandler> SmXMLMultiScriptsContext_Impl::createFastChildContext(
    sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    uno::Reference<xml::sax::XFastContextHandler> xContext;

    switch (nElement)
    {
        case XML_ELEMENT(MATH, XML_MPRESCRIPTS):
            bHasPrescripts = true;
            ProcessSubSupPairs(false);
            xContext = new SmXMLPrescriptsContext_Impl(GetSmImport());
            break;
        case XML_ELEMENT(MATH, XML_NONE):
            xContext = new SmXMLNoneContext_Impl(GetSmImport());
            break;
        default:
            xContext = SmXMLRowContext_Impl::createFastChildContext(nElement, xAttrList);
            break;
    }
    return xContext;
}

void SmXMLMultiScriptsContext_Impl::ProcessSubSupPairs(bool bIsPrescript)
{
    SmNodeStack& rNodeStack = GetSmImport().GetNodeStack();

    if (rNodeStack.size() <= nElementCount)
        return;

    auto nCount = rNodeStack.size() - nElementCount - 1;
    if (nCount == 0)
        return;

    if (nCount % 2 == 0)
    {
        SmToken aToken;
        aToken.cMathChar = '\0';
        aToken.eType = bIsPrescript ? TLSUB : TRSUB;

        SmNodeStack aReverseStack;
        for (size_t i = 0; i < nCount + 1; i++)
        {
            auto pNode = std::move(rNodeStack.front());
            rNodeStack.pop_front();
            aReverseStack.push_front(std::move(pNode));
        }

        SmSubSup eSub = bIsPrescript ? LSUB : RSUB;
        SmSubSup eSup = bIsPrescript ? LSUP : RSUP;

        for (size_t i = 0; i < nCount; i += 2)
        {
            std::unique_ptr<SmSubSupNode> pNode(new SmSubSupNode(aToken));

            // initialize subnodes array
            SmNodeArray aSubNodes(1 + SUBSUP_NUM_ENTRIES);

            /*On each loop the base and its sub sup pair becomes the
             base for the next loop to which the next sub sup pair is
             attached, i.e. wheels within wheels*/
            aSubNodes[0] = popOrZero(aReverseStack).release();

            std::unique_ptr<SmNode> pScriptNode = popOrZero(aReverseStack);

            if (pScriptNode
                && ((pScriptNode->GetToken().eType != TIDENT)
                    || (!pScriptNode->GetToken().aText.isEmpty())))
                aSubNodes[eSub + 1] = pScriptNode.release();
            pScriptNode = popOrZero(aReverseStack);
            if (pScriptNode
                && ((pScriptNode->GetToken().eType != TIDENT)
                    || (!pScriptNode->GetToken().aText.isEmpty())))
                aSubNodes[eSup + 1] = pScriptNode.release();

            pNode->SetSubNodes(std::move(aSubNodes));
            aReverseStack.push_front(std::move(pNode));
        }
        assert(!aReverseStack.empty());
        auto pNode = std::move(aReverseStack.front());
        aReverseStack.pop_front();
        rNodeStack.push_front(std::move(pNode));
    }
    else
    {
        // Ignore odd number of elements.
        for (size_t i = 0; i < nCount; i++)
        {
            rNodeStack.pop_front();
        }
    }
}

void SmXMLTableContext_Impl::endFastElement(sal_Int32)
{
    SmNodeArray aExpressionArray;
    SmNodeStack& rNodeStack = GetSmImport().GetNodeStack();
    SmNodeStack aReverseStack;
    aExpressionArray.resize(rNodeStack.size() - nElementCount);

    size_t nRows = rNodeStack.size() - nElementCount;
    size_t nCols = 0;

    for (size_t i = nRows; i > 0; --i)
    {
        SmNode* pArray = rNodeStack.front().release();
        rNodeStack.pop_front();
        if (pArray->GetNumSubNodes() == 0)
        {
            //This is a little tricky, it is possible that there was
            //be elements that were not inside a <mtd> pair, in which
            //case they will not be in a row, i.e. they will not have
            //SubNodes, so we have to wait until here before we can
            //resolve the situation. Implicit surrounding tags are
            //surprisingly difficult to get right within this
            //architecture

            SmNodeArray aRelationArray;
            aRelationArray.resize(1);
            aRelationArray[0] = pArray;
            SmToken aDummy;
            SmExpressionNode* pExprNode = new SmExpressionNode(aDummy);
            pExprNode->SetSubNodes(std::move(aRelationArray));
            pArray = pExprNode;
        }

        nCols = std::max(nCols, pArray->GetNumSubNodes());
        aReverseStack.push_front(std::unique_ptr<SmNode>(pArray));
    }
    if (nCols > SAL_MAX_UINT16)
        throw std::range_error("column limit");
    if (nRows > SAL_MAX_UINT16)
        throw std::range_error("row limit");
    aExpressionArray.resize(nCols * nRows);
    size_t j = 0;
    for (auto& elem : aReverseStack)
    {
        std::unique_ptr<SmStructureNode> xArray(static_cast<SmStructureNode*>(elem.release()));
        for (size_t i = 0; i < xArray->GetNumSubNodes(); ++i)
            aExpressionArray[j++] = xArray->GetSubNode(i);
        xArray->ClearSubNodes();
    }
    aReverseStack.clear();

    SmToken aToken;
    aToken.cMathChar = '\0';
    aToken.eType = TMATRIX;
    std::unique_ptr<SmMatrixNode> pSNode(new SmMatrixNode(aToken));
    pSNode->SetSubNodes(std::move(aExpressionArray));
    pSNode->SetRowCol(nRows, nCols);
    rNodeStack.push_front(std::move(pSNode));
}

uno::Reference<xml::sax::XFastContextHandler> SmXMLTableRowContext_Impl::createFastChildContext(
    sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    uno::Reference<xml::sax::XFastContextHandler> xContext;

    switch (nElement)
    {
        case XML_ELEMENT(MATH, XML_MTD):
            xContext = new SmXMLTableCellContext_Impl(GetSmImport());
            break;
        default:
            xContext = SmXMLRowContext_Impl::createFastChildContext(nElement, xAttrList);
            break;
    }
    return xContext;
}

uno::Reference<xml::sax::XFastContextHandler> SmXMLTableContext_Impl::createFastChildContext(
    sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    uno::Reference<xml::sax::XFastContextHandler> xContext;

    switch (nElement)
    {
        case XML_ELEMENT(MATH, XML_MTR):
            xContext = new SmXMLTableRowContext_Impl(GetSmImport());
            break;
        default:
            xContext = SmXMLTableRowContext_Impl::createFastChildContext(nElement, xAttrList);
            break;
    }
    return xContext;
}

void SmXMLMultiScriptsContext_Impl::endFastElement(sal_Int32)
{
    ProcessSubSupPairs(bHasPrescripts);
}

void SmXMLActionContext_Impl::startFastElement(
    sal_Int32 /*nElement*/, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    for (auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        switch (aIter.getToken())
        {
            case XML_SELECTION:
            {
                sal_Int32 n = aIter.toInt32();
                if (n > 0)
                    mnSelection = static_cast<size_t>(n);
            }
            break;
            default:
                XMLOFF_WARN_UNKNOWN("starmath", aIter);
                break;
        }
    }
}

void SmXMLActionContext_Impl::endFastElement(sal_Int32)
{
    SmNodeStack& rNodeStack = GetSmImport().GetNodeStack();
    auto nSize = rNodeStack.size();
    if (nSize <= nElementCount)
    {
        // not compliant to maction's specification, e.g., no subexpressions
        return;
    }
    assert(mnSelection > 0);
    if (nSize < nElementCount + mnSelection)
    {
        // No selected subexpression exists, which is a MathML error;
        // fallback to selecting the first
        mnSelection = 1;
    }
    assert(nSize >= nElementCount + mnSelection);
    for (auto i = nSize - (nElementCount + mnSelection); i > 0; i--)
    {
        rNodeStack.pop_front();
    }
    auto pSelected = std::move(rNodeStack.front());
    rNodeStack.pop_front();
    for (auto i = rNodeStack.size() - nElementCount; i > 0; i--)
    {
        rNodeStack.pop_front();
    }
    rNodeStack.push_front(std::move(pSelected));
}

SvXMLImportContext*
SmXMLImport::CreateFastContext(sal_Int32 nElement,
                               const uno::Reference<xml::sax::XFastAttributeList>& /*xAttrList*/)
{
    SvXMLImportContext* pContext = nullptr;

    switch (nElement)
    {
        case XML_ELEMENT(OFFICE, XML_DOCUMENT):
        case XML_ELEMENT(OFFICE, XML_DOCUMENT_META):
        {
            uno::Reference<document::XDocumentPropertiesSupplier> xDPS(GetModel(),
                                                                       uno::UNO_QUERY_THROW);
            pContext = ((nElement & TOKEN_MASK) == XML_DOCUMENT_META)
                           ? new SvXMLMetaDocumentContext(*this, xDPS->getDocumentProperties())
                           // flat OpenDocument file format -- this has not been tested...
                           : new SmXMLFlatDocContext_Impl(*this, xDPS->getDocumentProperties());
        }
        break;
        default:
            if (IsTokenInNamespace(nElement, XML_NAMESPACE_OFFICE))
                pContext = new SmXMLOfficeContext_Impl(*this);
            else
                pContext = new SmXMLDocContext_Impl(*this);
    }
    return pContext;
}

SmXMLImport::~SmXMLImport() throw() { cleanup(); }

void SmXMLImport::SetViewSettings(const Sequence<PropertyValue>& aViewProps)
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

    tools::Rectangle aRect(pDocShell->GetVisArea());

    tools::Long nTmp = 0;

    for (const PropertyValue& rValue : aViewProps)
    {
        if (rValue.Name == "ViewAreaTop")
        {
            rValue.Value >>= nTmp;
            aRect.SaturatingSetY(nTmp);
        }
        else if (rValue.Name == "ViewAreaLeft")
        {
            rValue.Value >>= nTmp;
            aRect.SaturatingSetX(nTmp);
        }
        else if (rValue.Name == "ViewAreaWidth")
        {
            rValue.Value >>= nTmp;
            Size aSize(aRect.GetSize());
            aSize.setWidth(nTmp);
            aRect.SaturatingSetSize(aSize);
        }
        else if (rValue.Name == "ViewAreaHeight")
        {
            rValue.Value >>= nTmp;
            Size aSize(aRect.GetSize());
            aSize.setHeight(nTmp);
            aRect.SaturatingSetSize(aSize);
        }
    }

    pDocShell->SetVisArea(aRect);
}

void SmXMLImport::SetConfigurationSettings(const Sequence<PropertyValue>& aConfProps)
{
    uno::Reference<XPropertySet> xProps(GetModel(), UNO_QUERY);
    if (!xProps.is())
        return;

    Reference<XPropertySetInfo> xInfo(xProps->getPropertySetInfo());
    if (!xInfo.is())
        return;

    const OUString sFormula("Formula");
    const OUString sBasicLibraries("BasicLibraries");
    const OUString sDialogLibraries("DialogLibraries");
    for (const PropertyValue& rValue : aConfProps)
    {
        if (rValue.Name != sFormula && rValue.Name != sBasicLibraries
            && rValue.Name != sDialogLibraries)
        {
            try
            {
                if (xInfo->hasPropertyByName(rValue.Name))
                    xProps->setPropertyValue(rValue.Name, rValue.Value);
            }
            catch (const beans::PropertyVetoException&)
            {
                // dealing with read-only properties here. Nothing to do...
            }
            catch (const Exception&)
            {
                DBG_UNHANDLED_EXCEPTION("starmath");
            }
        }
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportMML(SvStream& rStream)
{
    SmGlobals::ensure();

    SfxObjectShellLock xDocSh(new SmDocShell(SfxModelFlags::EMBEDDED_OBJECT));
    xDocSh->DoInitNew();
    uno::Reference<frame::XModel> xModel(xDocSh->GetModel());

    uno::Reference<beans::XPropertySet> xInfoSet;
    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
    uno::Reference<io::XInputStream> xStream(new utl::OSeekableInputStreamWrapper(rStream));

    //SetLoading hack because the document properties will be re-initted
    //by the xml filter and during the init, while it's considered uninitialized,
    //setting a property will inform the document it's modified, which attempts
    //to update the properties, which throws cause the properties are uninitialized
    xDocSh->SetLoading(SfxLoadedFlags::NONE);

    ErrCode nRet = ERRCODE_SFX_DOLOADFAILED;

    try
    {
        nRet = SmXMLImportWrapper::ReadThroughComponent(xStream, xModel, xContext, xInfoSet,
                                                        "com.sun.star.comp.Math.XMLImporter", false,
                                                        false);
    }
    catch (...)
    {
    }

    xDocSh->SetLoading(SfxLoadedFlags::ALL);

    xDocSh->DoClose();

    return nRet != ERRCODE_NONE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

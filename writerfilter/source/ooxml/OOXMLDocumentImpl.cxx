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

#include <comphelper/sequenceashashmap.hxx>

#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/graphic/GraphicMapper.hpp>
#include <ooxml/resourceids.hxx>
#include <oox/drawingml/theme.hxx>
#include <oox/shape/ShapeFilterBase.hxx>
#include "OOXMLStreamImpl.hxx"
#include "OOXMLDocumentImpl.hxx"
#include "OOXMLBinaryObjectReference.hxx"
#include "OOXMLFastDocumentHandler.hxx"
#include "OOXMLPropertySet.hxx"

#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <comphelper/sequence.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <unotools/mediadescriptor.hxx>

#include <iostream>
#include <sfx2/objsh.hxx>
#include <utility>

// this extern variable is declared in OOXMLStreamImpl.hxx
OUString customTarget;
OUString embeddingsTarget;
using namespace ::com::sun::star;
namespace writerfilter::ooxml
{

OOXMLDocumentImpl::OOXMLDocumentImpl(OOXMLStream::Pointer_t pStream, uno::Reference<task::XStatusIndicator> xStatusIndicator, bool bSkipImages, const uno::Sequence<beans::PropertyValue>& rDescriptor)
    : mpStream(std::move(pStream))
    , mxStatusIndicator(std::move(xStatusIndicator))
    , mnXNoteId(0)
    , mbIsSubstream(false)
    , mbSkipImages(bSkipImages)
    , mnPercentSize(0)
    , mnProgressLastPos(0)
    , mnProgressCurrentPos(0)
    , mnProgressEndPos(0)
    , m_rBaseURL(utl::MediaDescriptor(rDescriptor).getUnpackedValueOrDefault("DocumentBaseURL", OUString()))
    , maMediaDescriptor(rDescriptor)
    , mxGraphicMapper(graphic::GraphicMapper::create(mpStream->getContext()))
{
    pushShapeContext();
}

OOXMLDocumentImpl::~OOXMLDocumentImpl()
{
}

void OOXMLDocumentImpl::resolveFastSubStream(Stream & rStreamHandler,
                                             OOXMLStream::StreamType_t nType)
{
    OOXMLStream::Pointer_t pStream;
    try
    {
        pStream = OOXMLDocumentFactory::createStream(mpStream, nType);
    }
    catch (uno::Exception const&)
    {
        TOOLS_INFO_EXCEPTION("writerfilter.ooxml", "resolveFastSubStream: exception while "
                "resolving stream " << nType);
        return;
    }
    OOXMLStream::Pointer_t savedStream = mpStream;
    mpStream = pStream;

    uno::Reference<xml::sax::XFastParser> xParser(mpStream->getFastParser());

    if (xParser.is())
    {
        uno::Reference<uno::XComponentContext> xContext(mpStream->getContext());
        rtl::Reference<OOXMLFastDocumentHandler> pDocHandler =
                        new OOXMLFastDocumentHandler(xContext, &rStreamHandler, this, mnXNoteId);

        uno::Reference<xml::sax::XFastTokenHandler> xTokenHandler(mpStream->getFastTokenHandler());

        xParser->setFastDocumentHandler(pDocHandler);
        xParser->setTokenHandler(xTokenHandler);

        uno::Reference<io::XInputStream> xInputStream = mpStream->getDocumentStream();

        if (xInputStream.is())
        {
            struct xml::sax::InputSource oInputSource;
            oInputSource.aInputStream = xInputStream;
            xParser->parseStream(oInputSource);

            xInputStream->closeInput();
        }
    }

    mpStream = savedStream;
}

void OOXMLDocumentImpl::resolveFastSubStreamWithId(Stream & rStream,
                                    const writerfilter::Reference<Stream>::Pointer_t& pStream,
                                    sal_uInt32 nId)
{
    rStream.substream(nId, pStream);
}

uno::Reference<xml::dom::XDocument> OOXMLDocumentImpl::importSubStream(OOXMLStream::StreamType_t nType)
{
    uno::Reference<xml::dom::XDocument> xRet;

    OOXMLStream::Pointer_t pStream;
    try
    {
        pStream = OOXMLDocumentFactory::createStream(mpStream, nType);
    }
    catch (uno::Exception const&)
    {
        TOOLS_INFO_EXCEPTION("writerfilter.ooxml", "importSubStream: exception while "
                "importing stream " << nType);
        return xRet;
    }

    uno::Reference<io::XInputStream> xInputStream = pStream->getDocumentStream();
    if (xInputStream.is())
    {
        try
        {
            uno::Reference<uno::XComponentContext> xContext(mpStream->getContext());
            uno::Reference<xml::dom::XDocumentBuilder> xDomBuilder(xml::dom::DocumentBuilder::create(xContext));
            xRet = xDomBuilder->parse(xInputStream);
        }
        catch (uno::Exception const&)
        {
            TOOLS_INFO_EXCEPTION("writerfilter.ooxml", "importSubStream: exception while "
                     "parsing stream " << nType);
            return xRet;
        }
    }

    if (OOXMLStream::CUSTOMXML == nType)
    {
        importSubStreamRelations(pStream, OOXMLStream::CUSTOMXMLPROPS);
    }
    else if (OOXMLStream::CHARTS == nType)
    {
        importSubStreamRelations(pStream, OOXMLStream::EMBEDDINGS);
    }

    return xRet;
}


void OOXMLDocumentImpl::importSubStreamRelations(const OOXMLStream::Pointer_t& pStream, OOXMLStream::StreamType_t nType)
{
    uno::Reference<xml::dom::XDocument> xRelation;
    OOXMLStream::Pointer_t cStream;
    try
    {
        cStream = OOXMLDocumentFactory::createStream(pStream, nType);
    }
    catch (uno::Exception const&)
    {
        TOOLS_WARN_EXCEPTION("writerfilter.ooxml", "importSubStreamRelations: exception while "
            "importing stream " << nType);
        return;
    }

    uno::Reference<io::XInputStream> xcpInputStream = cStream->getDocumentStream();

    if (!xcpInputStream.is())
        return;

    // importing itemprops files for item.xml from customXml.
    if (OOXMLStream::CUSTOMXMLPROPS == nType)
    {
        try
        {
             uno::Reference<uno::XComponentContext> xcpContext(pStream->getContext());
             uno::Reference<xml::dom::XDocumentBuilder> xDomBuilder(xml::dom::DocumentBuilder::create(xcpContext));
             xRelation = xDomBuilder->parse(xcpInputStream);
        }
        catch (uno::Exception const&)
        {
            TOOLS_WARN_EXCEPTION("writerfilter.ooxml", "importSubStream: exception while "
                     "parsing stream " << nType);
            mxCustomXmlProsDom = xRelation;
        }

        if(xRelation.is())
        {
            mxCustomXmlProsDom = xRelation;
        }
    }
    else if(OOXMLStream::EMBEDDINGS == nType)
    {
        mxEmbeddings = xcpInputStream;
    }
    else if(OOXMLStream::CHARTS == nType)
    {
        importSubStreamRelations(cStream, OOXMLStream::EMBEDDINGS);
    }


}

void OOXMLDocumentImpl::setXNoteId(const sal_Int32 nId)
{
    mnXNoteId = nId;
}

sal_Int32 OOXMLDocumentImpl::getXNoteId() const
{
    return mnXNoteId;
}

const OUString & OOXMLDocumentImpl::getTarget() const
{
    return mpStream->getTarget();
}

writerfilter::Reference<Stream>::Pointer_t
OOXMLDocumentImpl::getSubStream(const OUString & rId)
{
    OOXMLStream::Pointer_t pStream
        (OOXMLDocumentFactory::createStream(mpStream, rId));

    OOXMLDocumentImpl * pTemp;
    // Do not pass status indicator to sub-streams: they are typically marginal in size, so we just track the main document for now.
    writerfilter::Reference<Stream>::Pointer_t pRet( pTemp = new OOXMLDocumentImpl(pStream, uno::Reference<task::XStatusIndicator>(), mbSkipImages, maMediaDescriptor));
    pTemp->setModel(mxModel);
    pTemp->setDrawPage(mxDrawPage);
    pTemp->mbIsSubstream = true;
    return pRet;
}

writerfilter::Reference<Stream>::Pointer_t
OOXMLDocumentImpl::getXNoteStream(OOXMLStream::StreamType_t nType, const sal_Int32 nId)
{
    OOXMLStream::Pointer_t pStream =
        OOXMLDocumentFactory::createStream(mpStream, nType);
    // See above, no status indicator for the note stream, either.
    OOXMLDocumentImpl * pDocument = new OOXMLDocumentImpl(pStream, uno::Reference<task::XStatusIndicator>(), mbSkipImages, maMediaDescriptor);
    pDocument->setXNoteId(nId);
    pDocument->setModel(getModel());
    pDocument->setDrawPage(getDrawPage());

    return writerfilter::Reference<Stream>::Pointer_t(pDocument);
}

void OOXMLDocumentImpl::resolveFootnote(Stream & rStream,
                                        Id aType,
                                        const sal_Int32 nNoteId)
{
    if (!mpXFootnoteStream)
        mpXFootnoteStream = getXNoteStream(OOXMLStream::FOOTNOTES, nNoteId);

    Id nId;
    switch (aType)
    {
    case NS_ooxml::LN_Value_doc_ST_FtnEdn_separator:
    case NS_ooxml::LN_Value_doc_ST_FtnEdn_continuationSeparator:
        nId = aType;
        break;
    default:
        nId = NS_ooxml::LN_footnote;
        break;
    }

    resolveFastSubStreamWithId(rStream, mpXFootnoteStream, nId);
}

void OOXMLDocumentImpl::resolveEndnote(Stream & rStream,
                                       Id aType,
                                       const sal_Int32 nNoteId)
{
    if (!mpXEndnoteStream)
       mpXEndnoteStream = getXNoteStream(OOXMLStream::ENDNOTES, nNoteId);

    Id nId;
    switch (aType)
    {
    case NS_ooxml::LN_Value_doc_ST_FtnEdn_separator:
    case NS_ooxml::LN_Value_doc_ST_FtnEdn_continuationSeparator:
        nId = aType;
        break;
    default:
        nId = NS_ooxml::LN_endnote;
        break;
    }

    resolveFastSubStreamWithId(rStream, mpXEndnoteStream, nId);
}

void OOXMLDocumentImpl::resolveCommentsExtendedStream(Stream& rStream)
{
    resolveFastSubStream(rStream, OOXMLStream::COMMENTS_EXTENDED);
}

void OOXMLDocumentImpl::resolveComment(Stream & rStream,
                                       const sal_Int32 nId)
{
    if (!mbCommentsExtendedResolved)
    {
        resolveCommentsExtendedStream(rStream);
        mbCommentsExtendedResolved = true;
    }

    writerfilter::Reference<Stream>::Pointer_t pStream =
        getXNoteStream(OOXMLStream::COMMENTS, nId);

    resolveFastSubStreamWithId(rStream, pStream, NS_ooxml::LN_annotation);
}

OOXMLPropertySet * OOXMLDocumentImpl::getPicturePropSet
(const OUString & rId)
{
    OOXMLStream::Pointer_t xStream
        (OOXMLDocumentFactory::createStream(mpStream, rId));

    writerfilter::Reference<BinaryObj>::Pointer_t xPicture
        (new OOXMLBinaryObjectReference(std::move(xStream)));

    OOXMLValue::Pointer_t pPayloadValue(new OOXMLBinaryValue(std::move(xPicture)));

    OOXMLPropertySet::Pointer_t pBlipSet(new OOXMLPropertySet);

    pBlipSet->add(NS_ooxml::LN_payload, pPayloadValue, OOXMLProperty::ATTRIBUTE);

    OOXMLValue::Pointer_t pBlipValue(new OOXMLPropertySetValue(pBlipSet));

    OOXMLPropertySet * pProps = new OOXMLPropertySet;

    pProps->add(NS_ooxml::LN_blip, pBlipValue, OOXMLProperty::ATTRIBUTE);

    return pProps;
}

void OOXMLDocumentImpl::resolvePicture(Stream & rStream,
                                       const OUString & rId)
{
    OOXMLPropertySet::Pointer_t pProps(getPicturePropSet(rId));

    rStream.props(pProps.get());
}

OUString OOXMLDocumentImpl::getTargetForId(const OUString & rId)
{
    return mpStream->getTargetForId(rId);
}

void OOXMLDocumentImpl::resolveHeader(Stream & rStream,
                                      const sal_Int32 type,
                                      const OUString & rId)
{
     writerfilter::Reference<Stream>::Pointer_t pStream =
         getSubStream(rId);
     switch (type)
     {
     case NS_ooxml::LN_Value_ST_HdrFtr_even:
         resolveFastSubStreamWithId(rStream, pStream, NS_ooxml::LN_headerl);
        break;
     case NS_ooxml::LN_Value_ST_HdrFtr_default: // here we assume that default is right, but not necessarily true :-(
         resolveFastSubStreamWithId(rStream, pStream, NS_ooxml::LN_headerr);
         break;
     case NS_ooxml::LN_Value_ST_HdrFtr_first:
         resolveFastSubStreamWithId(rStream, pStream, NS_ooxml::LN_headerf);
         break;
     default:
         break;
     }
}

void OOXMLDocumentImpl::resolveFooter(Stream & rStream,
                                      const sal_Int32 type,
                                      const OUString & rId)
{
     writerfilter::Reference<Stream>::Pointer_t pStream =
         getSubStream(rId);

     switch (type)
     {
     case NS_ooxml::LN_Value_ST_HdrFtr_even:
         resolveFastSubStreamWithId(rStream, pStream, NS_ooxml::LN_footerl);
         break;
     case NS_ooxml::LN_Value_ST_HdrFtr_default: // here we assume that default is right, but not necessarily true :-(
         resolveFastSubStreamWithId(rStream, pStream, NS_ooxml::LN_footerr);
         break;
     case NS_ooxml::LN_Value_ST_HdrFtr_first:
         resolveFastSubStreamWithId(rStream, pStream, NS_ooxml::LN_footerf);
         break;
     default:
         break;
     }
}

namespace {
// Ensures that the indicator is reset after exiting OOXMLDocumentImpl::resolve
class StatusIndicatorGuard{
public:
    explicit StatusIndicatorGuard(css::uno::Reference<css::task::XStatusIndicator> xStatusIndicator)
        :mxStatusIndicator(std::move(xStatusIndicator))
    {
    }

    ~StatusIndicatorGuard()
    {
        if (mxStatusIndicator.is())
                mxStatusIndicator->end();
    }

private:
    css::uno::Reference<css::task::XStatusIndicator> mxStatusIndicator;
};
}

void OOXMLDocumentImpl::resolve(Stream & rStream)
{
    StatusIndicatorGuard aStatusIndicatorGuard(mxStatusIndicator);

    if (utl::MediaDescriptor(maMediaDescriptor).getUnpackedValueOrDefault("ReadGlossaries", false))
    {
        resolveFastSubStream(rStream, OOXMLStream::GLOSSARY);
        return;
    }

    uno::Reference<xml::sax::XFastParser> xParser(mpStream->getFastParser());

    if (mxModel.is())
    {
        uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(mxModel, uno::UNO_QUERY);
        uno::Reference<document::XDocumentProperties> xDocumentProperties = xDocumentPropertiesSupplier->getDocumentProperties();
        comphelper::SequenceAsHashMap aMap(xDocumentProperties->getDocumentStatistics());
        if (aMap.find("ParagraphCount") != aMap.end())
        {
            sal_Int32 nValue;
            if (aMap["ParagraphCount"] >>= nValue)
            {
                if (mxStatusIndicator.is())
                {
                    // We want to care about the progress if we know the estimated paragraph count and we have given a status indicator as well.
                    // Set the end position only here, so later it's enough to check if that is non-zero in incrementProgress().
                    mnProgressEndPos = nValue;
                    OUString aDocLoad(SvxResId(RID_SVXSTR_DOC_LOAD));
                    mxStatusIndicator->start(aDocLoad, mnProgressEndPos);
                    mnPercentSize = mnProgressEndPos / 100;
                }
            }
        }
    }

    if (!xParser.is())
        return;

    uno::Reference<uno::XComponentContext> xContext(mpStream->getContext());

    rStream.setDocumentReference(this);

    rtl::Reference<OOXMLFastDocumentHandler> pDocHandler =
                new OOXMLFastDocumentHandler(xContext, &rStream, this, mnXNoteId);
    pDocHandler->setIsSubstream( mbIsSubstream );
    uno::Reference < xml::sax::XFastTokenHandler > xTokenHandler(mpStream->getFastTokenHandler());

    resolveFastSubStream(rStream, OOXMLStream::SETTINGS);
    mxThemeDom = importSubStream(OOXMLStream::THEME);
    resolveFastSubStream(rStream, OOXMLStream::THEME);
    // Convert the oox::Theme to the draw page
    {
        auto pThemePtr = getTheme();
        if (pThemePtr)
            pThemePtr->addTheme(getDrawPage());
    }
    mxGlossaryDocDom = importSubStream(OOXMLStream::GLOSSARY);
    if (mxGlossaryDocDom.is())
        resolveGlossaryStream(rStream);

    resolveEmbeddingsStream(mpStream);

    // Custom xml's are handled as part of grab bag.
    resolveCustomXmlStream(rStream);

    resolveFastSubStream(rStream, OOXMLStream::FONTTABLE);
    resolveFastSubStream(rStream, OOXMLStream::STYLES);
    resolveFastSubStream(rStream, OOXMLStream::NUMBERING);

    xParser->setFastDocumentHandler( pDocHandler );
    xParser->setTokenHandler( xTokenHandler );

    xml::sax::InputSource aParserInput;
    aParserInput.sSystemId = mpStream->getTarget();
    aParserInput.aInputStream = mpStream->getDocumentStream();
    try
    {
        xParser->parseStream(aParserInput);
    }
    catch (xml::sax::SAXException const&)
    {
        // don't silently swallow these - handlers may not have been executed,
        // and the domain mapper is likely in an inconsistent state
        // In case user chooses to try to continue loading, don't ask again for this file
        SfxObjectShell* rShell = SfxObjectShell::GetShellFromComponent(mxModel);
        if (!rShell || !rShell->IsContinueImportOnFilterExceptions())
            throw;
    }
    catch (uno::RuntimeException const&)
    {
        throw;
    }
    // note: cannot throw anything other than SAXException out of here?
    catch (uno::Exception const&)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        SAL_WARN("writerfilter.ooxml", "OOXMLDocumentImpl::resolve(): " << exceptionToString(anyEx));
        throw lang::WrappedTargetRuntimeException("", nullptr, anyEx);
    }
    catch (...)
    {
        SAL_WARN("writerfilter.ooxml",
            "OOXMLDocumentImpl::resolve(): non-UNO exception");
    }
}

void OOXMLDocumentImpl::incrementProgress()
{
    mnProgressCurrentPos++;
    // 1) If we know the end
    // 2) We progressed enough that updating makes sense
    // 3) We did not reach the end yet (possible in case the doc stat is misleading)
    if (mnProgressEndPos && mnProgressCurrentPos > (mnProgressLastPos + mnPercentSize) && mnProgressLastPos < mnProgressEndPos)
    {
        mnProgressLastPos = mnProgressCurrentPos;
        if (mxStatusIndicator.is())
            mxStatusIndicator->setValue(mnProgressLastPos);
    }
}

void OOXMLDocumentImpl::resolveCustomXmlStream(Stream & rStream)
{
    // Resolving all item[n].xml files from CustomXml folder.
    uno::Reference<embed::XRelationshipAccess> xRelationshipAccess;
    xRelationshipAccess.set(dynamic_cast<OOXMLStreamImpl&>(*mpStream).accessDocumentStream(), uno::UNO_QUERY);
    if (!xRelationshipAccess.is())
        return;

    static const char sCustomType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/customXml";
    static const char sCustomTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/customXml";
    bool bFound = false;
    const uno::Sequence<uno::Sequence< beans::StringPair>> aSeqs = xRelationshipAccess->getAllRelationships();
    std::vector<uno::Reference<xml::dom::XDocument>> aCustomXmlDomList;
    std::vector<uno::Reference<xml::dom::XDocument>> aCustomXmlDomPropsList;
    for (const uno::Sequence<beans::StringPair>& aSeq : aSeqs)
    {
        for (const beans::StringPair& aPair : aSeq)
        {
            // Need to resolve only customxml files from document relationships.
            // Skipping other files.
            if (aPair.Second == sCustomType ||
                aPair.Second == sCustomTypeStrict)
                bFound = true;
            else if (aPair.First == "Target" && bFound)
            {
                // Adding value to extern variable customTarget. It will be used in ooxmlstreamimpl
                // to ensure customxml target is visited in lcl_getTarget.
                customTarget = aPair.Second;
            }
        }

        if (bFound)
        {
            uno::Reference<xml::dom::XDocument> customXmlTemp = importSubStream(OOXMLStream::CUSTOMXML);
            // This will add all item[n].xml with its relationship file i.e itemprops.xml to
            // grabbag list.
            if (mxCustomXmlProsDom.is() && customXmlTemp.is())
            {
                aCustomXmlDomList.push_back(customXmlTemp);
                aCustomXmlDomPropsList.push_back(mxCustomXmlProsDom);
                resolveFastSubStream(rStream, OOXMLStream::CUSTOMXML);
            }

            bFound = false;
        }
    }

    mxCustomXmlDomList = comphelper::containerToSequence(aCustomXmlDomList);
    mxCustomXmlDomPropsList = comphelper::containerToSequence(aCustomXmlDomPropsList);
}

namespace
{
const char sSettingsType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings";
const char sStylesType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles";
const char sFonttableType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/fontTable";
const char sWebSettings[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/webSettings";
const char sSettingsTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/settings";
const char sStylesTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/styles";
const char sFonttableTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/fontTable";
const char sWebSettingsStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/webSettings";

constexpr OUString sId = u"Id"_ustr;
constexpr OUStringLiteral sType = u"Type";
constexpr OUString sTarget = u"Target"_ustr;
constexpr OUStringLiteral sTargetMode = u"TargetMode";
constexpr OUString sContentType = u"_contentType"_ustr;
constexpr OUStringLiteral sRelDom = u"_relDom";
constexpr OUStringLiteral sSettingsContentType = u"application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml";
constexpr OUStringLiteral sStylesContentType = u"application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml";
constexpr OUStringLiteral sWebsettingsContentType = u"application/vnd.openxmlformats-officedocument.wordprocessingml.webSettings+xml";
constexpr OUStringLiteral sFonttableContentType = u"application/vnd.openxmlformats-officedocument.wordprocessingml.fontTable+xml";
}

// See DocxExport::WriteGlossary
void OOXMLDocumentImpl::resolveGlossaryStream(Stream & /*rStream*/)
{
    OOXMLStream::Pointer_t pStream;
    try
    {
        pStream = OOXMLDocumentFactory::createStream(mpStream, OOXMLStream::GLOSSARY);
    }
    catch (uno::Exception const&)
    {
        TOOLS_INFO_EXCEPTION("writerfilter.ooxml", "resolveGlossaryStream: exception while "
                 "createStream for glossary" << OOXMLStream::GLOSSARY);
        return;
    }
    uno::Reference<embed::XRelationshipAccess> xRelationshipAccess;
    xRelationshipAccess.set(dynamic_cast<OOXMLStreamImpl&>(*pStream).accessDocumentStream(), uno::UNO_QUERY);
    if (!xRelationshipAccess.is())
        return;


    const uno::Sequence< uno::Sequence< beans::StringPair > >aSeqs = xRelationshipAccess->getAllRelationships();
    std::vector< uno::Sequence<beans::NamedValue> > aGlossaryDomList;
    for (const uno::Sequence< beans::StringPair >& aSeq : aSeqs)
    {
        comphelper::NamedValueCollection aRelDefinition;
        for (const auto& [name, value] : aSeq)
            aRelDefinition.put(name, value);

        const OUString gType = aRelDefinition.getOrDefault(sType, OUString{});
        OOXMLStream::StreamType_t nType(OOXMLStream::UNKNOWN);
        if (gType == sSettingsType || gType == sSettingsTypeStrict)
        {
            nType = OOXMLStream::SETTINGS;
            aRelDefinition.put(sContentType, sSettingsContentType);
        }
        else if (gType == sStylesType || gType == sStylesTypeStrict)
        {
            nType = OOXMLStream::STYLES;
            aRelDefinition.put(sContentType, sStylesContentType);
        }
        else if (gType == sWebSettings || gType == sWebSettingsStrict)
        {
            nType = OOXMLStream::WEBSETTINGS;
            aRelDefinition.put(sContentType, sWebsettingsContentType);
        }
        else if (gType == sFonttableType || gType == sFonttableTypeStrict)
        {
            nType = OOXMLStream::FONTTABLE;
            aRelDefinition.put(sContentType, sFonttableContentType);
        }
        else if (aRelDefinition.getOrDefault(sTargetMode, OUString{}) != "External")
        {
            // Some internal relation, but we don't create a DOM for it here yet?
            SAL_WARN("writerfilter.ooxml", "Unknown type of glossary internal relation: "
                "Id=\"" + aRelDefinition.getOrDefault<OUString>(sId, {}) + "\" "
                "Type=\"" + gType + "\" "
                "Target=\"" + aRelDefinition.getOrDefault<OUString>(sTarget, {}) + "\"");
            continue;
        }

        if (nType != OOXMLStream::UNKNOWN)
        {
            try
            {
                auto gStream = OOXMLDocumentFactory::createStream(pStream, nType);
                uno::Reference xInputStream = gStream->getDocumentStream();
                uno::Reference xContext(pStream->getContext());
                uno::Reference xDomBuilder(xml::dom::DocumentBuilder::create(xContext));
                uno::Reference xDom = xDomBuilder->parse(xInputStream);
                aRelDefinition.put(sRelDom, xDom);
            }
            catch (uno::Exception const&)
            {
                TOOLS_INFO_EXCEPTION("writerfilter.ooxml", "importSubStream: exception while "
                    "parsing stream of Type" << nType);
            }
        }
        aGlossaryDomList.push_back(aRelDefinition.getNamedValues());
    }
    mxGlossaryDomList = comphelper::containerToSequence(aGlossaryDomList);
}

void OOXMLDocumentImpl::resolveEmbeddingsStream(const OOXMLStream::Pointer_t& pStream)
{
    uno::Reference<embed::XRelationshipAccess> xRelationshipAccess;
    xRelationshipAccess.set(dynamic_cast<OOXMLStreamImpl&>(*pStream).accessDocumentStream(), uno::UNO_QUERY);
    if (xRelationshipAccess.is())
    {
        static constexpr OUStringLiteral sChartType(u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/chart");
        static constexpr OUStringLiteral sChartTypeStrict(u"http://purl.oclc.org/ooxml/officeDocument/relationships/chart");
        static constexpr OUStringLiteral sFootersType(u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer");
        static constexpr OUStringLiteral sFootersTypeStrict(u"http://purl.oclc.org/ooxml/officeDocument/relationships/footer");
        static constexpr OUStringLiteral sHeaderType(u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/header");
        static constexpr OUStringLiteral sHeaderTypeStrict(u"http://purl.oclc.org/ooxml/officeDocument/relationships/header");

        bool bFound = false;
        bool bHeaderFooterFound = false;
        OOXMLStream::StreamType_t streamType = OOXMLStream::UNKNOWN;
        const uno::Sequence< uno::Sequence< beans::StringPair > >aSeqs = xRelationshipAccess->getAllRelationships();
        for (const uno::Sequence< beans::StringPair >& aSeq : aSeqs)
        {
            for (const beans::StringPair& aPair : aSeq)
            {
                if (aPair.Second == sChartType ||
                        aPair.Second == sChartTypeStrict)
                {
                    bFound = true;
                }
                else if(aPair.Second == sFootersType ||
                        aPair.Second == sFootersTypeStrict)
                {
                    bHeaderFooterFound = true;
                    streamType = OOXMLStream::FOOTER;
                }
                else if(aPair.Second == sHeaderType ||
                        aPair.Second == sHeaderTypeStrict)
                {
                    bHeaderFooterFound = true;
                    streamType = OOXMLStream::HEADER;
                }
                else if(aPair.First == "Target" && ( bFound || bHeaderFooterFound ))
                {
                    // Adding value to extern variable customTarget. It will be used in ooxmlstreamimpl
                    // to ensure chart.xml target is visited in lcl_getTarget.
                    customTarget = aPair.Second;
                }
            }
            if( bFound || bHeaderFooterFound)
            {
                if(bFound)
                {
                    importSubStreamRelations(pStream, OOXMLStream::CHARTS);
                }
                if(bHeaderFooterFound)
                {
                    try
                    {
                        OOXMLStream::Pointer_t Stream = OOXMLDocumentFactory::createStream(pStream, streamType);
                        if (Stream)
                            resolveEmbeddingsStream(Stream);
                    }
                    catch (uno::Exception const&)
                    {
                        TOOLS_INFO_EXCEPTION("writerfilter.ooxml", "resolveEmbeddingsStream: can't find header/footer whilst "
                               "resolving stream " << streamType);
                        return;
                    }
                }

                beans::PropertyValue embeddingsTemp;
                // This will add all .xlsx and .bin to grabbag list.
                if(bFound && mxEmbeddings.is())
                {
                    embeddingsTemp.Name = embeddingsTarget;
                    embeddingsTemp.Value <<= mxEmbeddings;
                    m_aEmbeddings.push_back(embeddingsTemp);
                    mxEmbeddings.clear();
                }
                bFound = false;
                bHeaderFooterFound = false;
            }
        }
    }
    if (!m_aEmbeddings.empty())
        mxEmbeddingsList = comphelper::containerToSequence(m_aEmbeddings);
}

uno::Reference<xml::dom::XDocument> OOXMLDocumentImpl::getGlossaryDocDom( )
{
    return mxGlossaryDocDom;
}

uno::Sequence<uno::Sequence< beans::NamedValue> > OOXMLDocumentImpl::getGlossaryDomList()
{
    return mxGlossaryDomList;
}

uno::Reference<io::XInputStream> OOXMLDocumentImpl::getInputStreamForId(const OUString & rId)
{
    OOXMLStream::Pointer_t pStream(OOXMLDocumentFactory::createStream(mpStream, rId));

    return pStream->getDocumentStream();
}

void OOXMLDocumentImpl::setModel(uno::Reference<frame::XModel> xModel)
{
    mxModel.set(xModel);
}

uno::Reference<frame::XModel> OOXMLDocumentImpl::getModel()
{
    return mxModel;
}

void OOXMLDocumentImpl::setDrawPage(uno::Reference<drawing::XDrawPage> xDrawPage)
{
    mxDrawPage.set(xDrawPage);
}

uno::Reference<drawing::XDrawPage> OOXMLDocumentImpl::getDrawPage()
{
    return mxDrawPage;
}

const uno::Sequence<beans::PropertyValue>& OOXMLDocumentImpl::getMediaDescriptor() const
{
    return maMediaDescriptor;
}

void OOXMLDocumentImpl::setShapeContext( rtl::Reference<oox::shape::ShapeContextHandler> xContext )
{
    if (!maShapeContexts.empty())
        maShapeContexts.top() = xContext;
}

rtl::Reference<oox::shape::ShapeContextHandler> OOXMLDocumentImpl::getShapeContext( )
{
    if (!maShapeContexts.empty())
        return maShapeContexts.top();
    else
        return {};
}

void OOXMLDocumentImpl::pushShapeContext()
{
    maShapeContexts.push({});
}

void OOXMLDocumentImpl::popShapeContext()
{
    if (!maShapeContexts.empty())
        maShapeContexts.pop();
}

uno::Reference<xml::dom::XDocument> OOXMLDocumentImpl::getThemeDom( )
{
    return mxThemeDom;
}

uno::Sequence<uno::Reference<xml::dom::XDocument> > OOXMLDocumentImpl::getCustomXmlDomList( )
{
    return mxCustomXmlDomList;
}

uno::Sequence<uno::Reference<xml::dom::XDocument> > OOXMLDocumentImpl::getCustomXmlDomPropsList( )
{
    return mxCustomXmlDomPropsList;
}

uno::Sequence<beans::PropertyValue > OOXMLDocumentImpl::getEmbeddingsList( )
{
    return mxEmbeddingsList;
}

const rtl::Reference<oox::shape::ShapeFilterBase>& OOXMLDocumentImpl::getShapeFilterBase()
{
    if (!mxShapeFilterBase)
        mxShapeFilterBase = new oox::shape::ShapeFilterBase(mpStream->getContext());
    return mxShapeFilterBase;
}

const rtl::Reference<oox::drawingml::ThemeFilterBase>& OOXMLDocumentImpl::getThemeFilterBase()
{
    if (!mxThemeFilterBase)
        mxThemeFilterBase = new oox::drawingml::ThemeFilterBase(mpStream->getContext());
    return mxThemeFilterBase;
}

OOXMLDocument *
OOXMLDocumentFactory::createDocument
(const OOXMLStream::Pointer_t& pStream,
 const uno::Reference<task::XStatusIndicator>& xStatusIndicator,
 bool mbSkipImages, const uno::Sequence<beans::PropertyValue>& rDescriptor)
{
    return new OOXMLDocumentImpl(pStream, xStatusIndicator, mbSkipImages, rDescriptor);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

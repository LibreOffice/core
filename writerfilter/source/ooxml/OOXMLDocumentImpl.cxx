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

#include <com/sun/star/xml/sax/XParser.hpp>

#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>
#include <ooxml/resourceids.hxx>
#include "OOXMLStreamImpl.hxx"
#include "OOXMLDocumentImpl.hxx"
#include "OOXMLBinaryObjectReference.hxx"
#include "OOXMLFastDocumentHandler.hxx"
#include "OOXMLPropertySet.hxx"

#include <sal/log.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/resmgr.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <comphelper/sequence.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <unotools/mediadescriptor.hxx>

#include <iostream>
#include <sfx2/objsh.hxx>

// this extern variable is declared in OOXMLStreamImpl.hxx
OUString customTarget;
OUString embeddingsTarget;
using namespace ::com::sun::star;
namespace writerfilter {
namespace ooxml
{

OOXMLDocumentImpl::OOXMLDocumentImpl(OOXMLStream::Pointer_t const & pStream, const uno::Reference<task::XStatusIndicator>& xStatusIndicator, bool bSkipImages, const uno::Sequence<beans::PropertyValue>& rDescriptor)
    : mpStream(pStream)
    , mxStatusIndicator(xStatusIndicator)
    , mnXNoteId(0)
    , mbIsSubstream(false)
    , mbSkipImages(bSkipImages)
    , mnPercentSize(0)
    , mnProgressLastPos(0)
    , mnProgressCurrentPos(0)
    , mnProgressEndPos(0)
    , m_rBaseURL(utl::MediaDescriptor(rDescriptor).getUnpackedValueOrDefault("DocumentBaseURL", OUString()))
    , maMediaDescriptor(rDescriptor)
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
    catch (uno::Exception const& e)
    {
        SAL_INFO("writerfilter.ooxml", "resolveFastSubStream: exception while "
                "resolving stream " << nType << " : " << e);
        return;
    }
    OOXMLStream::Pointer_t savedStream = mpStream;
    mpStream = pStream;

    uno::Reference<xml::sax::XFastParser> xParser(mpStream->getFastParser());

    if (xParser.is())
    {
        uno::Reference<uno::XComponentContext> xContext(mpStream->getContext());
        OOXMLFastDocumentHandler * pDocHandler =
                        new OOXMLFastDocumentHandler(xContext, &rStreamHandler, this, mnXNoteId);

        uno::Reference<xml::sax::XFastDocumentHandler> xDocumentHandler(pDocHandler);
        uno::Reference<xml::sax::XFastTokenHandler> xTokenHandler(mpStream->getFastTokenHandler());

        xParser->setFastDocumentHandler(xDocumentHandler);
        xParser->setTokenHandler(xTokenHandler);

        uno::Reference<io::XInputStream> xInputStream = pStream->getDocumentStream();

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
    catch (uno::Exception const& e)
    {
        SAL_INFO("writerfilter.ooxml", "importSubStream: exception while "
                "importing stream " << nType << " : " << e);
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
        catch (uno::Exception const& e)
        {
            SAL_INFO("writerfilter.ooxml", "importSubStream: exception while "
                     "parsing stream " << nType << " : " << e);
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
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN("writerfilter.ooxml", "importSubStreamRelations: exception while "
            "importing stream " << nType << " : " << exceptionToString(ex));
        return;
    }

    uno::Reference<io::XInputStream> xcpInputStream = cStream->getDocumentStream();

    if (xcpInputStream.is())
    {
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
                css::uno::Any ex( cppu::getCaughtException() );
                SAL_WARN("writerfilter.ooxml", "importSubStream: exception while "
                         "parsing stream " << nType << " : " << exceptionToString(ex));
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


}

void OOXMLDocumentImpl::setXNoteId(const sal_Int32 nId)
{
    mnXNoteId = nId;
}

sal_Int32 OOXMLDocumentImpl::getXNoteId() const
{
    return mnXNoteId;
}

void OOXMLDocumentImpl::setXNoteType(Id /*nId*/)
{
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
OOXMLDocumentImpl::getXNoteStream(OOXMLStream::StreamType_t nType, Id aType,
                                  const sal_Int32 nId)
{
    OOXMLStream::Pointer_t pStream =
        OOXMLDocumentFactory::createStream(mpStream, nType);
    // See above, no status indicator for the note stream, either.
    OOXMLDocumentImpl * pDocument = new OOXMLDocumentImpl(pStream, uno::Reference<task::XStatusIndicator>(), mbSkipImages, maMediaDescriptor);
    pDocument->setXNoteId(nId);
    pDocument->setXNoteType(aType);
    pDocument->setModel(getModel());
    pDocument->setDrawPage(getDrawPage());

    return writerfilter::Reference<Stream>::Pointer_t(pDocument);
}

void OOXMLDocumentImpl::resolveFootnote(Stream & rStream,
                                        Id aType,
                                        const sal_Int32 nNoteId)
{
    writerfilter::Reference<Stream>::Pointer_t pStream =
        getXNoteStream(OOXMLStream::FOOTNOTES, aType, nNoteId);

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

    resolveFastSubStreamWithId(rStream, pStream, nId);
}

void OOXMLDocumentImpl::resolveEndnote(Stream & rStream,
                                       Id aType,
                                       const sal_Int32 nNoteId)
{
    writerfilter::Reference<Stream>::Pointer_t pStream =
        getXNoteStream(OOXMLStream::ENDNOTES, aType, nNoteId);

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

    resolveFastSubStreamWithId(rStream, pStream, nId);
}

void OOXMLDocumentImpl::resolveComment(Stream & rStream,
                                       const sal_Int32 nId)
{
    writerfilter::Reference<Stream>::Pointer_t pStream =
        getXNoteStream(OOXMLStream::COMMENTS, 0, nId);

    resolveFastSubStreamWithId(rStream, pStream, NS_ooxml::LN_annotation);
}

OOXMLPropertySet * OOXMLDocumentImpl::getPicturePropSet
(const OUString & rId)
{
    OOXMLStream::Pointer_t pStream
        (OOXMLDocumentFactory::createStream(mpStream, rId));

    writerfilter::Reference<BinaryObj>::Pointer_t pPicture
        (new OOXMLBinaryObjectReference(pStream));

    OOXMLValue::Pointer_t pPayloadValue(new OOXMLBinaryValue(pPicture));

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
    explicit StatusIndicatorGuard(css::uno::Reference<css::task::XStatusIndicator> const & xStatusIndicator)
        :mxStatusIndicator(xStatusIndicator)
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

    if (xParser.is())
    {
        uno::Reference<uno::XComponentContext> xContext(mpStream->getContext());

        OOXMLFastDocumentHandler * pDocHandler =
                    new OOXMLFastDocumentHandler(xContext, &rStream, this, mnXNoteId);
        pDocHandler->setIsSubstream( mbIsSubstream );
        uno::Reference < xml::sax::XFastDocumentHandler > xDocumentHandler(pDocHandler);
        uno::Reference < xml::sax::XFastTokenHandler > xTokenHandler(mpStream->getFastTokenHandler());

        resolveFastSubStream(rStream, OOXMLStream::SETTINGS);
        mxThemeDom = importSubStream(OOXMLStream::THEME);
        resolveFastSubStream(rStream, OOXMLStream::THEME);
        mxGlossaryDocDom = importSubStream(OOXMLStream::GLOSSARY);
        if (mxGlossaryDocDom.is())
            resolveGlossaryStream(rStream);

        resolveEmbeddingsStream(mpStream);

        // Custom xml's are handled as part of grab bag.
        resolveCustomXmlStream(rStream);

        resolveFastSubStream(rStream, OOXMLStream::FONTTABLE);
        resolveFastSubStream(rStream, OOXMLStream::STYLES);
        resolveFastSubStream(rStream, OOXMLStream::NUMBERING);

        xParser->setFastDocumentHandler( xDocumentHandler );
        xParser->setTokenHandler( xTokenHandler );

        xml::sax::InputSource aParserInput;
        aParserInput.sSystemId = mpStream->getTarget();
        aParserInput.aInputStream = mpStream->getDocumentStream();
        try
        {
            xParser->parseStream(aParserInput);
        }
        catch (xml::sax::SAXException const& rErr)
        {
            // don't silently swallow these - handlers may not have been executed,
            // and the domain mapper is likely in an inconsistent state
            // In case user chooses to try to continue loading, don't ask again for this file
            SfxObjectShell* rShell = SfxObjectShell::GetShellFromComponent(mxModel);
            if (!rShell || !rShell->IsContinueImportOnFilterExceptions("SAXException: " + rErr.Message))
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
    if (xRelationshipAccess.is())
    {
        static const char sCustomType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/customXml";
        static const char sCustomTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/customXml";
        bool bFound = false;
        uno::Sequence<uno::Sequence< beans::StringPair>> aSeqs = xRelationshipAccess->getAllRelationships();
        std::vector<uno::Reference<xml::dom::XDocument>> aCustomXmlDomList;
        std::vector<uno::Reference<xml::dom::XDocument>> aCustomXmlDomPropsList;
        for (sal_Int32 j = 0; j < aSeqs.getLength(); j++)
        {
            const uno::Sequence<beans::StringPair>& aSeq = aSeqs[j];
            for (sal_Int32 i = 0; i < aSeq.getLength(); i++)
            {
                const beans::StringPair& aPair = aSeq[i];
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
}

void OOXMLDocumentImpl::resolveGlossaryStream(Stream & /*rStream*/)
{
    static const char sSettingsType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings";
    static const char sStylesType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles";
    static const char sFonttableType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/fontTable";
    static const char sWebSettings[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/webSettings";
    static const char sSettingsTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/settings";
    static const char sStylesTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/styles";
    static const char sFonttableTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/fontTable";
    static const char sWebSettingsStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/webSettings";

    OOXMLStream::Pointer_t pStream;
    try
    {
        pStream = OOXMLDocumentFactory::createStream(mpStream, OOXMLStream::GLOSSARY);
    }
    catch (uno::Exception const& e)
    {
        SAL_INFO("writerfilter.ooxml", "resolveGlossaryStream: exception while "
                 "createStream for glossary" << OOXMLStream::GLOSSARY << " : " << e);
        return;
    }
    uno::Reference<embed::XRelationshipAccess> xRelationshipAccess;
    xRelationshipAccess.set(dynamic_cast<OOXMLStreamImpl&>(*pStream).accessDocumentStream(), uno::UNO_QUERY);
    if (xRelationshipAccess.is())
    {

        uno::Sequence< uno::Sequence< beans::StringPair > >aSeqs = xRelationshipAccess->getAllRelationships();
        std::vector< uno::Sequence<uno::Any> > aGlossaryDomList;
        for (sal_Int32 j = 0; j < aSeqs.getLength(); j++)
        {
              OOXMLStream::Pointer_t gStream;
              uno::Sequence< beans::StringPair > aSeq = aSeqs[j];
              //Follows following aSeq[0] is Id, aSeq[1] is Type, aSeq[2] is Target
              if (aSeq.getLength() < 3)
              {
                  SAL_WARN("writerfilter.ooxml", "too short sequence");
                  continue;
              }

              OUString gId(aSeq[0].Second);
              OUString gType(aSeq[1].Second);
              OUString gTarget(aSeq[2].Second);
              OUString contentType;

              OOXMLStream::StreamType_t nType(OOXMLStream::UNKNOWN);
              bool bFound = true;
              if(gType == sSettingsType ||
                      gType == sSettingsTypeStrict)
              {
                  nType = OOXMLStream::SETTINGS;
                  contentType = "application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml";
              }
              else if(gType == sStylesType ||
                      gType == sStylesTypeStrict)
              {
                  nType = OOXMLStream::STYLES;
                  contentType = "application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml";
              }
              else if(gType == sWebSettings ||
                      gType == sWebSettingsStrict)
              {
                  nType = OOXMLStream::WEBSETTINGS;
                  contentType = "application/vnd.openxmlformats-officedocument.wordprocessingml.webSettings+xml";
              }
              else if(gType == sFonttableType ||
                      gType == sFonttableTypeStrict)
              {
                  nType = OOXMLStream::FONTTABLE;
                  contentType = "application/vnd.openxmlformats-officedocument.wordprocessingml.fontTable+xml";
              }
              else
              {
                  bFound = false;
                  //"Unhandled content-type while grab bagging Glossary Folder");
              }

              if (bFound)
              {
                  uno::Reference<xml::dom::XDocument> xDom;
                  try
                  {
                      gStream = OOXMLDocumentFactory::createStream(pStream, nType);
                      uno::Reference<io::XInputStream> xInputStream = gStream->getDocumentStream();
                      uno::Reference<uno::XComponentContext> xContext(pStream->getContext());
                      uno::Reference<xml::dom::XDocumentBuilder> xDomBuilder(xml::dom::DocumentBuilder::create(xContext));
                      xDom = xDomBuilder->parse(xInputStream);
                  }
                  catch (uno::Exception const& e)
                  {
                      SAL_INFO("writerfilter.ooxml", "importSubStream: exception while "
                      "parsing stream of Type" << nType << " : " << e);
                      return;
                  }

                  if (xDom.is())
                  {
                      uno::Sequence< uno::Any > glossaryTuple (5);
                      glossaryTuple[0] <<= xDom;
                      glossaryTuple[1] <<= gId;
                      glossaryTuple[2] <<= gType;
                      glossaryTuple[3] <<= gTarget;
                      glossaryTuple[4] <<= contentType;
                      aGlossaryDomList.push_back(glossaryTuple);
                  }
              }
        }
        mxGlossaryDomList = comphelper::containerToSequence(aGlossaryDomList);
    }
}

void OOXMLDocumentImpl::resolveEmbeddingsStream(const OOXMLStream::Pointer_t& pStream)
{
    uno::Reference<embed::XRelationshipAccess> xRelationshipAccess;
    xRelationshipAccess.set(dynamic_cast<OOXMLStreamImpl&>(*pStream).accessDocumentStream(), uno::UNO_QUERY);
    if (xRelationshipAccess.is())
    {
        OUString const sChartType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/chart");
        OUString const sChartTypeStrict("http://purl.oclc.org/ooxml/officeDocument/relationships/chart");
        OUString const sFootersType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer");
        OUString const sFootersTypeStrict("http://purl.oclc.org/ooxml/officeDocument/relationships/footer");
        OUString const sHeaderType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/header");
        OUString const sHeaderTypeStrict("http://purl.oclc.org/ooxml/officeDocument/relationships/header");

        bool bFound = false;
        bool bHeaderFooterFound = false;
        OOXMLStream::StreamType_t streamType = OOXMLStream::UNKNOWN;
        uno::Sequence< uno::Sequence< beans::StringPair > >aSeqs = xRelationshipAccess->getAllRelationships();
        for (sal_Int32 j = 0; j < aSeqs.getLength(); j++)
        {
            uno::Sequence< beans::StringPair > aSeq = aSeqs[j];
            for (sal_Int32 i = 0; i < aSeq.getLength(); i++)
            {
                beans::StringPair aPair = aSeq[i];
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
                    catch (uno::Exception const& e)
                    {
                        SAL_INFO("writerfilter.ooxml", "resolveEmbeddingsStream: can't find header/footer whilst "
                               "resolving stream " << streamType << " : " << e);
                        return;
                    }
                }

                beans::PropertyValue embeddingsTemp;
                // This will add all .xlsx and .bin to grabbag list.
                if(bFound && mxEmbeddings.is())
                {
                    embeddingsTemp.Name = embeddingsTarget;
                    embeddingsTemp.Value <<= mxEmbeddings;
                    aEmbeddings.push_back(embeddingsTemp);
                    mxEmbeddings.clear();
                }
                bFound = false;
                bHeaderFooterFound = false;
            }
        }
    }
    if (!aEmbeddings.empty())
        mxEmbeddingsList = comphelper::containerToSequence(aEmbeddings);
}

uno::Reference<xml::dom::XDocument> OOXMLDocumentImpl::getGlossaryDocDom( )
{
    return mxGlossaryDocDom;
}

uno::Sequence<uno::Sequence< uno::Any> > OOXMLDocumentImpl::getGlossaryDomList()
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

const uno::Sequence<beans::PropertyValue>& OOXMLDocumentImpl::getMediaDescriptor()
{
    return maMediaDescriptor;
}

void OOXMLDocumentImpl::setShapeContext( uno::Reference<xml::sax::XFastShapeContextHandler> xContext )
{
    if (!maShapeContexts.empty())
        maShapeContexts.top() = xContext;
}

uno::Reference<xml::sax::XFastShapeContextHandler> OOXMLDocumentImpl::getShapeContext( )
{
    if (!maShapeContexts.empty())
        return maShapeContexts.top();
    else
        return uno::Reference<xml::sax::XFastShapeContextHandler>();
}

void OOXMLDocumentImpl::pushShapeContext()
{
    maShapeContexts.push(uno::Reference<xml::sax::XFastShapeContextHandler>());
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

OOXMLDocument *
OOXMLDocumentFactory::createDocument
(const OOXMLStream::Pointer_t& pStream,
 const uno::Reference<task::XStatusIndicator>& xStatusIndicator,
 bool mbSkipImages, const uno::Sequence<beans::PropertyValue>& rDescriptor)
{
    return new OOXMLDocumentImpl(pStream, xStatusIndicator, mbSkipImages, rDescriptor);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

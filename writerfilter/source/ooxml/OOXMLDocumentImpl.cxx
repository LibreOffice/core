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
#include "OOXMLPropertySetImpl.hxx"

#include <tools/resmgr.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <svx/dialogs.hrc>
#include <comphelper/sequence.hxx>

#include <iostream>

// this extern variable is declared in OOXMLStreamImpl.hxx
OUString customTarget;
OUString embeddingsTarget;
using namespace ::com::sun::star;
namespace writerfilter {
namespace ooxml
{

OOXMLDocumentImpl::OOXMLDocumentImpl(OOXMLStream::Pointer_t pStream, const uno::Reference<task::XStatusIndicator>& xStatusIndicator, bool bSkipImages)
    : mpStream(pStream)
    , mxStatusIndicator(xStatusIndicator)
    , mnXNoteId(0)
    , mXNoteType(0)
    , mxThemeDom(nullptr)
    , mbIsSubstream(false)
    , mbSkipImages(bSkipImages)
    , mnPercentSize(0)
    , mnProgressLastPos(0)
    , mnProgressCurrentPos(0)
    , mnProgressEndPos(0)
{
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
        SAL_INFO("writerfilter", "resolveFastSubStream: exception while "
                "resolving stream " << nType << " : " << e.Message);
        return;
    }
    OOXMLStream::Pointer_t savedStream = mpStream;
    mpStream = pStream;

    uno::Reference< xml::sax::XFastParser > xParser
        (mpStream->getFastParser());

    if (xParser.is())
    {
        uno::Reference<uno::XComponentContext> xContext(mpStream->getContext());
        OOXMLFastDocumentHandler * pDocHandler =
            new OOXMLFastDocumentHandler(
                xContext, &rStreamHandler, this, mnXNoteId );

        uno::Reference < xml::sax::XFastDocumentHandler > xDocumentHandler
            (pDocHandler);
        uno::Reference < xml::sax::XFastTokenHandler > xTokenHandler(mpStream->getFastTokenHandler());

        xParser->setFastDocumentHandler(xDocumentHandler);
        xParser->setTokenHandler(xTokenHandler);

        uno::Reference<io::XInputStream> xInputStream =
            pStream->getDocumentStream();

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
                                      writerfilter::Reference<Stream>::Pointer_t pStream,
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
        SAL_INFO("writerfilter", "importSubStream: exception while "
                "importing stream " << nType << " : " << e.Message);
        return xRet;
    }

    uno::Reference<io::XInputStream> xInputStream =
        pStream->getDocumentStream();

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
            SAL_INFO("writerfilter", "importSubStream: exception while "
                     "parsing stream " << nType << " : " << e.Message);
            return xRet;
        }
    }

    if(OOXMLStream::CUSTOMXML == nType)
    {
        importSubStreamRelations(pStream, OOXMLStream::CUSTOMXMLPROPS);
    }
    if(OOXMLStream::ACTIVEX == nType)
    {
        importSubStreamRelations(pStream, OOXMLStream::ACTIVEXBIN);
    }
    if(OOXMLStream::CHARTS == nType)
    {
        importSubStreamRelations(pStream, OOXMLStream::EMBEDDINGS);
    }

    return xRet;
}


void OOXMLDocumentImpl::importSubStreamRelations(OOXMLStream::Pointer_t pStream, OOXMLStream::StreamType_t nType)
{
    uno::Reference<xml::dom::XDocument> xRelation;
    OOXMLStream::Pointer_t cStream;
    try
    {
       cStream = OOXMLDocumentFactory::createStream(pStream, nType);
    }
    catch (uno::Exception const& e)
    {
        SAL_WARN("writerfilter", "importSubStreamRelations: exception while "
            "importing stream " << nType << " : " << e.Message);
        return;
    }

    uno::Reference<io::XInputStream> xcpInputStream =
            cStream->getDocumentStream();

    if (xcpInputStream.is())
    {
        // imporing itemprops files for item.xml from customXml.
        if(OOXMLStream::CUSTOMXMLPROPS == nType)
        {
            try
            {
                 uno::Reference<uno::XComponentContext> xcpContext(pStream->getContext());
                 uno::Reference<xml::dom::XDocumentBuilder> xDomBuilder(xml::dom::DocumentBuilder::create(xcpContext));
                 xRelation = xDomBuilder->parse(xcpInputStream);
            }
            catch (uno::Exception const& e)
            {
                SAL_WARN("writerfilter", "importSubStream: exception while "
                         "parsing stream " << nType << " : " << e.Message);
                mxCustomXmlProsDom = xRelation;
            }

            if(xRelation.is())
            {
                mxCustomXmlProsDom = xRelation;
            }
        }
        else if(OOXMLStream::ACTIVEXBIN == nType)
        {
            // imporing activex.bin files for activex.xml from activeX folder.
            mxActiveXBin = xcpInputStream;
        }
        else if(OOXMLStream::EMBEDDINGS == nType)
        {
            // imporing activex.bin files for activex.xml from activeX folder.
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

void OOXMLDocumentImpl::setXNoteType(const Id & nId)
{
    mXNoteType = nId;
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
    writerfilter::Reference<Stream>::Pointer_t pRet( pTemp = new OOXMLDocumentImpl(pStream, uno::Reference<task::XStatusIndicator>(), mbSkipImages ));
    pTemp->setModel(mxModel);
    pTemp->setDrawPage(mxDrawPage);
    pTemp->setIsSubstream( true );
    return pRet;
}

writerfilter::Reference<Stream>::Pointer_t
OOXMLDocumentImpl::getXNoteStream(OOXMLStream::StreamType_t nType, const Id & rType,
                                  const sal_Int32 nId)
{
    OOXMLStream::Pointer_t pStream =
        (OOXMLDocumentFactory::createStream(mpStream, nType));
    // See above, no status indicator for the note stream, either.
    OOXMLDocumentImpl * pDocument = new OOXMLDocumentImpl(pStream, uno::Reference<task::XStatusIndicator>(), mbSkipImages);
    pDocument->setXNoteId(nId);
    pDocument->setXNoteType(rType);
    pDocument->setModel(getModel());
    pDocument->setDrawPage(getDrawPage());

    return writerfilter::Reference<Stream>::Pointer_t(pDocument);
}

void OOXMLDocumentImpl::resolveFootnote(Stream & rStream,
                                        const Id & rType,
                                        const sal_Int32 nNoteId)
{
    writerfilter::Reference<Stream>::Pointer_t pStream =
        getXNoteStream(OOXMLStream::FOOTNOTES, rType, nNoteId);

    Id nId;
    switch (rType)
    {
    case NS_ooxml::LN_Value_doc_ST_FtnEdn_separator:
    case NS_ooxml::LN_Value_doc_ST_FtnEdn_continuationSeparator:
        nId = rType;
        break;
    default:
        nId = NS_ooxml::LN_footnote;
        break;
    }

    resolveFastSubStreamWithId(rStream, pStream, nId);
}

void OOXMLDocumentImpl::resolveEndnote(Stream & rStream,
                                       const Id & rType,
                                       const sal_Int32 nNoteId)
{
    writerfilter::Reference<Stream>::Pointer_t pStream =
        getXNoteStream(OOXMLStream::ENDNOTES, rType, nNoteId);

    Id nId;
    switch (rType)
    {
    case NS_ooxml::LN_Value_doc_ST_FtnEdn_separator:
    case NS_ooxml::LN_Value_doc_ST_FtnEdn_continuationSeparator:
        nId = rType;
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

    OOXMLProperty::Pointer_t pPayloadProperty
        (new OOXMLPropertyImpl(NS_ooxml::LN_payload, pPayloadValue,
                               OOXMLPropertyImpl::ATTRIBUTE));

    OOXMLPropertySet::Pointer_t pBlipSet(new OOXMLPropertySetImpl());

    pBlipSet->add(pPayloadProperty);

    OOXMLValue::Pointer_t pBlipValue(new OOXMLPropertySetValue(pBlipSet));

    OOXMLProperty::Pointer_t pBlipProperty
        (new OOXMLPropertyImpl(NS_ooxml::LN_blip, pBlipValue,
                               OOXMLPropertyImpl::ATTRIBUTE));

    OOXMLPropertySet * pProps = new OOXMLPropertySetImpl();

    pProps->add(pBlipProperty);

    return pProps;
}

void OOXMLDocumentImpl::resolvePicture(Stream & rStream,
                                       const OUString & rId)
{
    OOXMLPropertySet * pProps = getPicturePropSet(rId);

    rStream.props(writerfilter::Reference<Properties>::Pointer_t(pProps));
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

void OOXMLDocumentImpl::resolve(Stream & rStream)
{
    uno::Reference< xml::sax::XFastParser > xParser
        (mpStream->getFastParser());

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
                    static ResMgr* pResMgr = ResMgr::CreateResMgr("svx", Application::GetSettings().GetUILanguageTag());
                    OUString aDocLoad(ResId(RID_SVXSTR_DOC_LOAD, *pResMgr).toString());
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
            new OOXMLFastDocumentHandler(
                xContext, &rStream, this, mnXNoteId );
        pDocHandler->setIsSubstream( mbIsSubstream );
        uno::Reference < xml::sax::XFastDocumentHandler > xDocumentHandler
            (pDocHandler);
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

        resolveActiveXStream(rStream);

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
        catch (xml::sax::SAXException const&)
        {
            // don't swallow these - handlers may not have been executed,
            // and the domain mapper is likely in an inconsistent state
            throw;
        }
        catch (uno::RuntimeException const&)
        {
            throw;
        }
        // note: cannot throw anything other than SAXException out of here?
        catch (uno::Exception const& e)
        {
            SAL_WARN("writerfilter.ooxml",
                "OOXMLDocumentImpl::resolve(): exception: " << e.Message);
            throw lang::WrappedTargetRuntimeException("", nullptr,
                    uno::makeAny(e));
        }
        catch (...)
        {
            SAL_WARN("writerfilter.ooxml",
                "OOXMLDocumentImpl::resolve(): non-UNO exception");
        }
    }

    if (mxStatusIndicator.is())
        mxStatusIndicator->end();
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
        mxStatusIndicator->setValue(mnProgressLastPos);
    }
}

void OOXMLDocumentImpl::resolveCustomXmlStream(Stream & rStream)
{
    // Resolving all item[n].xml files from CustomXml folder.
    uno::Reference<embed::XRelationshipAccess> xRelationshipAccess;
    xRelationshipAccess.set((dynamic_cast<OOXMLStreamImpl&>(*mpStream.get())).accessDocumentStream(), uno::UNO_QUERY_THROW);
    if (xRelationshipAccess.is())
    {
        static const char sCustomType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/customXml";
        static const char sCustomTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/customXml";
        OUString sTarget("Target");
        bool bFound = false;
        sal_Int32 counter = 0;
        uno::Sequence< uno::Sequence< beans::StringPair > >aSeqs = xRelationshipAccess->getAllRelationships();
        std::vector< uno::Reference<xml::dom::XDocument> > aCustomXmlDomList;
        uno::Sequence<uno::Reference<xml::dom::XDocument> > xCustomXmlDomPropsListTemp(aSeqs.getLength());
        for (sal_Int32 j = 0; j < aSeqs.getLength(); j++)
        {
            uno::Sequence< beans::StringPair > aSeq = aSeqs[j];
            for (sal_Int32 i = 0; i < aSeq.getLength(); i++)
            {
                beans::StringPair aPair = aSeq[i];
                // Need to resolve only customxml files from document relationships.
                // Skipping other files.
                if (aPair.Second == sCustomType ||
                        aPair.Second == sCustomTypeStrict)
                    bFound = true;
                else if(aPair.First == sTarget && bFound)
                {
                    // Adding value to extern variable customTarget. It will be used in ooxmlstreamimpl
                    // to ensure customxml target is visited in lcl_getTarget.
                    customTarget = aPair.Second;
                }
            }
            if(bFound)
            {
                uno::Reference<xml::dom::XDocument> customXmlTemp = importSubStream(OOXMLStream::CUSTOMXML);
                // This will add all item[n].xml with its relationship file i.e itemprops.xml to
                // grabbag list.
                if(mxCustomXmlProsDom.is() && customXmlTemp.is())
                {
                    aCustomXmlDomList.push_back(customXmlTemp);
                    xCustomXmlDomPropsListTemp[counter] = mxCustomXmlProsDom;
                    counter++;
                    resolveFastSubStream(rStream, OOXMLStream::CUSTOMXML);
                }
                bFound = false;
            }
        }

        xCustomXmlDomPropsListTemp.realloc(counter);
        mxCustomXmlDomList = comphelper::containerToSequence(aCustomXmlDomList);
        mxCustomXmlDomPropsList = xCustomXmlDomPropsListTemp;
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
        SAL_INFO("writerfilter", "resolveGlossaryStream: exception while "
                 "createStream for glossary" << OOXMLStream::GLOSSARY << " : " << e.Message);
        return;
    }
    uno::Reference<embed::XRelationshipAccess> xRelationshipAccess;
    xRelationshipAccess.set((dynamic_cast<OOXMLStreamImpl&>(*pStream.get())).accessDocumentStream(), uno::UNO_QUERY_THROW);
    if (xRelationshipAccess.is())
    {

        uno::Sequence< uno::Sequence< beans::StringPair > >aSeqs = xRelationshipAccess->getAllRelationships();
        uno::Sequence<uno::Sequence< uno::Any> > xGlossaryDomListTemp(aSeqs.getLength());
         sal_Int32 counter = 0;
         for (sal_Int32 j = 0; j < aSeqs.getLength(); j++)
         {
              OOXMLStream::Pointer_t gStream;
              uno::Sequence< beans::StringPair > aSeq = aSeqs[j];
              //Follows following aSeq[0] is Id, aSeq[1] is Type, aSeq[2] is Target
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
                      SAL_INFO("writerfilter", "importSubStream: exception while "
                      "parsing stream of Type" << nType << " : " << e.Message);
                      return;
                  }

                  if (xDom.is())
                  {
                      uno::Sequence< uno::Any > glossaryTuple (5);
                      glossaryTuple[0] = uno::makeAny(xDom);
                      glossaryTuple[1] = uno::makeAny(gId);
                      glossaryTuple[2] = uno::makeAny(gType);
                      glossaryTuple[3] = uno::makeAny(gTarget);
                      glossaryTuple[4] = uno::makeAny(contentType);
                      xGlossaryDomListTemp[counter] = glossaryTuple;
                      counter++;
                  }
              }
          }
          xGlossaryDomListTemp.realloc(counter);
          mxGlossaryDomList = xGlossaryDomListTemp;
      }
}

void OOXMLDocumentImpl::resolveEmbeddingsStream(OOXMLStream::Pointer_t pStream)
{
    uno::Reference<embed::XRelationshipAccess> xRelationshipAccess;
    xRelationshipAccess.set((dynamic_cast<OOXMLStreamImpl&>(*pStream.get())).accessDocumentStream(), uno::UNO_QUERY_THROW);
    if (xRelationshipAccess.is())
    {
        OUString sChartType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/chart");
        OUString sChartTypeStrict("http://purl.oclc.org/ooxml/officeDocument/relationships/chart");
        OUString sFootersType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer");
        OUString sFootersTypeStrict("http://purl.oclc.org/ooxml/officeDocument/relationships/footer");
        OUString sHeaderType("http://schemas.openxmlformats.org/officeDocument/2006/relationships/header");
        OUString sHeaderTypeStrict("http://purl.oclc.org/ooxml/officeDocument/relationships/header");

        OUString sTarget("Target");
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
                else if(aPair.First == sTarget && ( bFound || bHeaderFooterFound ))
                {
                    // Adding value to extern variable customTarget. It will be used in ooxmlstreamimpl
                    // to ensure chart.xml target is visited in lcl_getTarget.
                    customTarget = aPair.Second;
                }
            }
            if(( bFound || bHeaderFooterFound))
            {
                if(bFound)
                {
                    importSubStreamRelations(pStream, OOXMLStream::CHARTS);
                }
                if(bHeaderFooterFound)
                {
                    OOXMLStream::Pointer_t Stream = OOXMLDocumentFactory::createStream(pStream, streamType);
                    if(Stream)
                        resolveEmbeddingsStream(Stream);
                }

                beans::PropertyValue embeddingsTemp;
                // This will add all .xlsx and .bin to grabbag list.
                if(bFound)
                {
                    if(mxEmbeddings.is())
                    {
                        embeddingsTemp.Name = embeddingsTarget;
                        embeddingsTemp.Value = uno::makeAny(mxEmbeddings);
                        mxEmbeddingsListTemp.push_back(embeddingsTemp);
                        mxEmbeddings.clear();
                    }
                }
                bFound = false;
                bHeaderFooterFound = false;
            }
        }
    }
    if(0 != mxEmbeddingsListTemp.size())
    {
        mxEmbeddingsList.realloc(mxEmbeddingsListTemp.size());
        for (size_t i = 0; i < mxEmbeddingsListTemp.size(); i++)
        {
            mxEmbeddingsList[i] = mxEmbeddingsListTemp[i];
        }
    }
}

void OOXMLDocumentImpl::resolveActiveXStream(Stream & rStream)
{
    // Resolving all ActiveX[n].xml files from ActiveX folder.
    uno::Reference<embed::XRelationshipAccess> xRelationshipAccess;
    xRelationshipAccess.set((dynamic_cast<OOXMLStreamImpl&>(*mpStream.get())).accessDocumentStream(), uno::UNO_QUERY_THROW);
    if (xRelationshipAccess.is())
    {
        static const char sCustomType[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/control";
        static const char sCustomTypeStrict[] = "http://purl.oclc.org/ooxml/officeDocument/relationships/control";
        OUString sTarget("Target");
        bool bFound = false;
        sal_Int32 counter = 0;
        uno::Sequence< uno::Sequence< beans::StringPair > > aSeqs = xRelationshipAccess->getAllRelationships();
        uno::Sequence<uno::Reference<xml::dom::XDocument> > xActiveXDomListTemp(aSeqs.getLength());
        uno::Sequence<uno::Reference<io::XInputStream> > xActiveXBinListTemp(aSeqs.getLength());
        for (sal_Int32 j = 0; j < aSeqs.getLength(); j++)
        {
            uno::Sequence< beans::StringPair > aSeq = aSeqs[j];
            for (sal_Int32 i = 0; i < aSeq.getLength(); i++)
            {
                beans::StringPair aPair = aSeq[i];
                // Need to resolve only ActiveX files from document relationships.
                // Skipping other files.
                if (aPair.Second == sCustomType ||
                        aPair.Second == sCustomTypeStrict)
                    bFound = true;
                else if(aPair.First == sTarget && bFound)
                {
                    // Adding value to extern variable customTarget. It will be used in ooxmlstreamimpl
                    // to ensure ActiveX.xml target is visited in lcl_getTarget.
                    customTarget = aPair.Second;
                }
            }
            if(bFound)
            {
                uno::Reference<xml::dom::XDocument> activeXTemp = importSubStream(OOXMLStream::ACTIVEX);
                // This will add all ActiveX[n].xml to grabbag list.
                if(activeXTemp.is())
                {
                    xActiveXDomListTemp[counter] = activeXTemp;
                    if(mxActiveXBin.is())
                    {
                        xActiveXBinListTemp[counter] = mxActiveXBin;
                    }
                    counter++;
                    resolveFastSubStream(rStream, OOXMLStream::ACTIVEX);
                }
                bFound = false;
            }
        }
        xActiveXDomListTemp.realloc(counter);
        xActiveXBinListTemp.realloc(counter);
        mxActiveXDomList = xActiveXDomListTemp;
        mxActiveXBinList = xActiveXBinListTemp;
    }
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

uno::Reference<io::XInputStream> OOXMLDocumentImpl::getStorageStream()
{
    return mpStream->getStorageStream();
}

void OOXMLDocumentImpl::setShapeContext( uno::Reference<xml::sax::XFastShapeContextHandler> xContext )
{
    mxShapeContext = xContext;
}

uno::Reference<xml::sax::XFastShapeContextHandler> OOXMLDocumentImpl::getShapeContext( )
{
    return mxShapeContext;
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

uno::Sequence<uno::Reference<xml::dom::XDocument> > OOXMLDocumentImpl::getActiveXDomList( )
{
    return mxActiveXDomList;
}

uno::Sequence<uno::Reference<io::XInputStream> > OOXMLDocumentImpl::getActiveXBinList( )
{
    return mxActiveXBinList;
}

uno::Sequence<beans::PropertyValue > OOXMLDocumentImpl::getEmbeddingsList( )
{
    return mxEmbeddingsList;
}

OOXMLDocument *
OOXMLDocumentFactory::createDocument
(OOXMLStream::Pointer_t pStream, const uno::Reference<task::XStatusIndicator>& xStatusIndicator, bool mbSkipImages)
{
    return new OOXMLDocumentImpl(pStream, xStatusIndicator, mbSkipImages);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

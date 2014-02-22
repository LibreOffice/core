/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <com/sun/star/xml/sax/XParser.hpp>

#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>
#include <ooxml/resourceids.hxx>
#include "OOXMLStreamImpl.hxx"
#include "OOXMLDocumentImpl.hxx"
#include "OOXMLBinaryObjectReference.hxx"
#include "OOXMLFastDocumentHandler.hxx"
#include "OOXMLPropertySetImpl.hxx"
#include "ooxmlLoggers.hxx"

#include <iostream>


OUString customTarget;
OUString embeddingsTarget;
using ::com::sun::star::xml::sax::SAXException;
namespace writerfilter {
namespace ooxml
{

#if OSL_DEBUG_LEVEL > 1
TagLogger::Pointer_t debug_logger(TagLogger::getInstance("DEBUG"));
#endif

using namespace ::std;

OOXMLDocumentImpl::OOXMLDocumentImpl(OOXMLStream::Pointer_t pStream)
    : mpStream(pStream)
    , mnXNoteId(0)
    , mXNoteType(0)
    , mxThemeDom(0)
    , mbIsSubstream(false)
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
        uno::Reference < xml::sax::XFastTokenHandler > xTokenHandler
            (mpStream->getFastTokenHandler(xContext));

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
            
            mxActiveXBin = xcpInputStream;
        }
        else if(OOXMLStream::EMBEDDINGS == nType)
        {
            
            mxEmbeddings = xcpInputStream;
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

const Id & OOXMLDocumentImpl::getXNoteType() const
{
    return mXNoteType;
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
    writerfilter::Reference<Stream>::Pointer_t pRet( pTemp = new OOXMLDocumentImpl(pStream) );
    pTemp->setModel(mxModel);
    pTemp->setDrawPage(mxDrawPage);
    pTemp->setIsSubstream( true );
    return pRet;
}

writerfilter::Reference<Stream>::Pointer_t
OOXMLDocumentImpl::getXNoteStream(OOXMLStream::StreamType_t nType, const Id & rType,
                                  const sal_Int32 nId)
{
#ifdef DEBUG_ELEMENT
    debug_logger->startElement("getXNoteStream");
    debug_logger->attribute("id", nId);
    debug_logger->endElement();
#endif

    OOXMLStream::Pointer_t pStream =
        (OOXMLDocumentFactory::createStream(mpStream, nType));
    OOXMLDocumentImpl * pDocument = new OOXMLDocumentImpl(pStream);
    pDocument->setXNoteId(nId);
    pDocument->setXNoteType(rType);

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
    case NS_ooxml::LN_Value_wordprocessingml_ST_FtnEdn_separator:
    case NS_ooxml::LN_Value_wordprocessingml_ST_FtnEdn_continuationSeparator:
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
    case NS_ooxml::LN_Value_wordprocessingml_ST_FtnEdn_separator:
    case NS_ooxml::LN_Value_wordprocessingml_ST_FtnEdn_continuationSeparator:
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
     case NS_ooxml::LN_Value_ST_HdrFtr_default: 
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
     case NS_ooxml::LN_Value_ST_HdrFtr_default: 
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
#ifdef DEBUG_RESOLVE
    debug_logger->startElement("OOXMLDocumentImpl.resolve");
#endif

    uno::Reference< xml::sax::XFastParser > xParser
        (mpStream->getFastParser());

    if (xParser.is())
    {
        uno::Reference<uno::XComponentContext> xContext(mpStream->getContext());

        OOXMLFastDocumentHandler * pDocHandler =
            new OOXMLFastDocumentHandler(
                xContext, &rStream, this, mnXNoteId );
        pDocHandler->setIsSubstream( mbIsSubstream );
        uno::Reference < xml::sax::XFastDocumentHandler > xDocumentHandler
            (pDocHandler);
        uno::Reference < xml::sax::XFastTokenHandler > xTokenHandler
            (mpStream->getFastTokenHandler(xContext));

        resolveFastSubStream(rStream, OOXMLStream::SETTINGS);
        mxThemeDom = importSubStream(OOXMLStream::THEME);
        resolveFastSubStream(rStream, OOXMLStream::THEME);
        mxGlossaryDocDom = importSubStream(OOXMLStream::GLOSSARY);
        if (mxGlossaryDocDom.is())
            resolveGlossaryStream(rStream);

        resolveEmbeddingsStream(rStream);

        
        resolveCustomXmlStream(rStream);

        resolveActiveXStream(rStream);

        resolveFastSubStream(rStream, OOXMLStream::FONTTABLE);
        resolveFastSubStream(rStream, OOXMLStream::STYLES);
        resolveFastSubStream(rStream, OOXMLStream::NUMBERING);

        xParser->setFastDocumentHandler( xDocumentHandler );
        xParser->setTokenHandler( xTokenHandler );

        xml::sax::InputSource aParserInput;
        aParserInput.aInputStream = mpStream->getDocumentStream();
        try
        {
            xParser->parseStream(aParserInput);
        }
        catch (...) {
#ifdef DEBUG_ELEMENT
            debug_logger->element("exception");
#endif
        }
    }

#ifdef DEBUG_RESOLVE
    debug_logger->endElement();
#endif
}

void OOXMLDocumentImpl::resolveCustomXmlStream(Stream & rStream)
{
    
    uno::Reference<embed::XRelationshipAccess> mxRelationshipAccess;
    mxRelationshipAccess.set((*dynamic_cast<OOXMLStreamImpl *>(mpStream.get())).accessDocumentStream(), uno::UNO_QUERY_THROW);
    if (mxRelationshipAccess.is())
    {
        OUString sCustomType("http:
        OUString sTarget("Target");
        bool bFound = false;
        sal_Int32 counter = 0;
        uno::Sequence< uno::Sequence< beans::StringPair > >aSeqs =
                mxRelationshipAccess->getAllRelationships();
        uno::Sequence<uno::Reference<xml::dom::XDocument> > mxCustomXmlDomListTemp(aSeqs.getLength());
        uno::Sequence<uno::Reference<xml::dom::XDocument> > mxCustomXmlDomPropsListTemp(aSeqs.getLength());
        for (sal_Int32 j = 0; j < aSeqs.getLength(); j++)
        {
            uno::Sequence< beans::StringPair > aSeq = aSeqs[j];
            for (sal_Int32 i = 0; i < aSeq.getLength(); i++)
            {
                beans::StringPair aPair = aSeq[i];
                
                
                if (aPair.Second.compareTo(sCustomType) == 0)
                    bFound = true;
                else if(aPair.First.compareTo(sTarget) == 0 && bFound)
                {
                    
                    
                    customTarget = aPair.Second;
                }
            }
            if(bFound)
            {
                uno::Reference<xml::dom::XDocument> customXmlTemp = importSubStream(OOXMLStream::CUSTOMXML);
                
                
                if(mxCustomXmlProsDom.is() && customXmlTemp.is())
                {
                    mxCustomXmlDomListTemp[counter] = customXmlTemp;
                    mxCustomXmlDomPropsListTemp[counter] = mxCustomXmlProsDom;
                    counter++;
                    resolveFastSubStream(rStream, OOXMLStream::CUSTOMXML);
                }
                bFound = false;
            }
        }

        mxCustomXmlDomListTemp.realloc(counter);
        mxCustomXmlDomPropsListTemp.realloc(counter);
        mxCustomXmlDomList = mxCustomXmlDomListTemp;
        mxCustomXmlDomPropsList = mxCustomXmlDomPropsListTemp;
    }
}

void OOXMLDocumentImpl::resolveGlossaryStream(Stream & /*rStream*/)
{
    static OUString sSettingsType("http:
    static OUString sStylesType("http:
    static OUString sFonttableType("http:
    static OUString sWebSettings("http:

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
    uno::Reference<embed::XRelationshipAccess> mxRelationshipAccess;
    mxRelationshipAccess.set((*dynamic_cast<OOXMLStreamImpl *>(pStream.get())).accessDocumentStream(), uno::UNO_QUERY_THROW);
    if (mxRelationshipAccess.is())
    {

        uno::Sequence< uno::Sequence< beans::StringPair > >aSeqs =
                mxRelationshipAccess->getAllRelationships();
        uno::Sequence<uno::Sequence< uno::Any> > mxGlossaryDomListTemp(aSeqs.getLength());
         sal_Int32 counter = 0;
         for (sal_Int32 j = 0; j < aSeqs.getLength(); j++)
         {
              OOXMLStream::Pointer_t gStream;
              uno::Sequence< beans::StringPair > aSeq = aSeqs[j];
              
              OUString gId(aSeq[0].Second);
              OUString gType(aSeq[1].Second);
              OUString gTarget(aSeq[2].Second);
              OUString contentType;

              OOXMLStream::StreamType_t nType(OOXMLStream::UNKNOWN);
              bool bFound = true;
              if(gType.compareTo(sSettingsType) == 0)
              {
                  nType = OOXMLStream::SETTINGS;
                  contentType = "application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml";
              }
              else if(gType.compareTo(sStylesType) == 0)
              {
                  nType = OOXMLStream::STYLES;
                  contentType = "application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml";
              }
              else if(gType.compareTo(sWebSettings) == 0)
              {
                  nType = OOXMLStream::WEBSETTINGS;
                  contentType = "application/vnd.openxmlformats-officedocument.wordprocessingml.webSettings+xml";
              }
              else if(gType.compareTo(sFonttableType) == 0)
              {
              nType = OOXMLStream::FONTTABLE;
              contentType = "application/vnd.openxmlformats-officedocument.wordprocessingml.fontTable+xml";
              }
              else
              {
                  bFound = false;
                  
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
                      mxGlossaryDomListTemp[counter] = glossaryTuple;
                      counter++;
                  }
              }
          }
          mxGlossaryDomListTemp.realloc(counter);
          mxGlossaryDomList = mxGlossaryDomListTemp;
      }
}

void OOXMLDocumentImpl::resolveEmbeddingsStream(Stream & /*rStream*/)
{
    uno::Reference<embed::XRelationshipAccess> mxRelationshipAccess;
    mxRelationshipAccess.set((*dynamic_cast<OOXMLStreamImpl *>(mpStream.get())).accessDocumentStream(), uno::UNO_QUERY_THROW);
    if (mxRelationshipAccess.is())
    {
        OUString sChartType("http:
        OUString sTarget("Target");
        bool bFound = false;
        sal_Int32 counter = 0;
        uno::Sequence< uno::Sequence< beans::StringPair > >aSeqs =
                mxRelationshipAccess->getAllRelationships();
        uno::Sequence<beans::PropertyValue > mxEmbeddingsListTemp(aSeqs.getLength());
        for (sal_Int32 j = 0; j < aSeqs.getLength(); j++)
        {
            uno::Sequence< beans::StringPair > aSeq = aSeqs[j];
            for (sal_Int32 i = 0; i < aSeq.getLength(); i++)
            {
                beans::StringPair aPair = aSeq[i];
                if (aPair.Second.compareTo(sChartType) == 0)
                    bFound = true;
                else if(aPair.First.compareTo(sTarget) == 0 && bFound)
                {
                    
                    
                    customTarget = aPair.Second;
                }
            }
            if(bFound)
            {
                uno::Reference<xml::dom::XDocument> chartTemp = importSubStream(OOXMLStream::CHARTS);
                beans::PropertyValue embeddingsTemp;
                
                if(chartTemp.is())
                {
                    if(mxEmbeddings.is())
                    {
                        embeddingsTemp.Name = embeddingsTarget;
                        embeddingsTemp.Value = uno::makeAny(mxEmbeddings);
                        mxEmbeddingsListTemp[counter] = embeddingsTemp;
                    }
                    counter++;
                }
                bFound = false;
            }
        }
        mxEmbeddingsListTemp.realloc(counter);
        mxEmbeddingsList = mxEmbeddingsListTemp;
    }
}

void OOXMLDocumentImpl::resolveActiveXStream(Stream & rStream)
{
    
    uno::Reference<embed::XRelationshipAccess> mxRelationshipAccess;
    mxRelationshipAccess.set((*dynamic_cast<OOXMLStreamImpl *>(mpStream.get())).accessDocumentStream(), uno::UNO_QUERY_THROW);
    if (mxRelationshipAccess.is())
    {
        OUString sCustomType("http:
        OUString sTarget("Target");
        bool bFound = false;
        sal_Int32 counter = 0;
        uno::Sequence< uno::Sequence< beans::StringPair > >aSeqs =
                mxRelationshipAccess->getAllRelationships();
        uno::Sequence<uno::Reference<xml::dom::XDocument> > mxActiveXDomListTemp(aSeqs.getLength());
        uno::Sequence<uno::Reference<io::XInputStream> > mxActiveXBinListTemp(aSeqs.getLength());
        for (sal_Int32 j = 0; j < aSeqs.getLength(); j++)
        {
            uno::Sequence< beans::StringPair > aSeq = aSeqs[j];
            for (sal_Int32 i = 0; i < aSeq.getLength(); i++)
            {
                beans::StringPair aPair = aSeq[i];
                
                
                if (aPair.Second.compareTo(sCustomType) == 0)
                    bFound = true;
                else if(aPair.First.compareTo(sTarget) == 0 && bFound)
                {
                    
                    
                    customTarget = aPair.Second;
                }
            }
            if(bFound)
            {
                uno::Reference<xml::dom::XDocument> activeXTemp = importSubStream(OOXMLStream::ACTIVEX);
                
                if(activeXTemp.is())
                {
                    mxActiveXDomListTemp[counter] = activeXTemp;
                    if(mxActiveXBin.is())
                    {
                        mxActiveXBinListTemp[counter] = mxActiveXBin;
                    }
                    counter++;
                    resolveFastSubStream(rStream, OOXMLStream::ACTIVEX);
                }
                bFound = false;
            }
        }
        mxActiveXDomListTemp.realloc(counter);
        mxActiveXBinListTemp.realloc(counter);
        mxActiveXDomList = mxActiveXDomListTemp;
        mxActiveXBinList = mxActiveXBinListTemp;
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

string OOXMLDocumentImpl::getType() const
{
    return "OOXMLDocumentImpl";
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

uno::Reference<io::XInputStream> OOXMLDocumentImpl::getInputStream()
{
    return mpStream->getDocumentStream();
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

void OOXMLDocumentImpl::setThemeDom( uno::Reference<xml::dom::XDocument> xThemeDom )
{
    mxThemeDom = xThemeDom;
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
(OOXMLStream::Pointer_t pStream)
{
    return new OOXMLDocumentImpl(pStream);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include <com/sun/star/xml/sax/XParser.hpp>

#include <com/sun/star/xml/sax/SAXException.hpp>
#include <doctok/resourceids.hxx>
#include <ooxml/resourceids.hxx>
#include "OOXMLDocumentImpl.hxx"
#include "OOXMLBinaryObjectReference.hxx"
#include "OOXMLFastDocumentHandler.hxx"
#include "OOXMLPropertySetImpl.hxx"
#include "ooxmlLoggers.hxx"

#include <iostream>

using ::com::sun::star::xml::sax::SAXException;
namespace writerfilter {
namespace ooxml
{

#ifdef DEBUG
TagLogger::Pointer_t debug_logger(TagLogger::getInstance("DEBUG"));
#endif

using namespace ::std;

OOXMLDocumentImpl::OOXMLDocumentImpl(
    OOXMLStream::Pointer_t pStream )
    : mpStream(pStream)
    , mnIDForXNoteStream( -1 )
    , mxModel()
    , mxDrawPage()
    , mbIsSubstream( false )
{
}

OOXMLDocumentImpl::OOXMLDocumentImpl(
    OOXMLStream::Pointer_t pStream,
    uno::Reference<frame::XModel> xModel,
    uno::Reference<drawing::XDrawPage> xDrawPage,
    const bool bIsSubstream )
    : mpStream(pStream)
    , mnIDForXNoteStream( -1 )
    , mxModel( xModel )
    , mxDrawPage( xDrawPage )
    , mbIsSubstream( bIsSubstream )
{
}

OOXMLDocumentImpl::OOXMLDocumentImpl(
    OOXMLStream::Pointer_t pStream,
    const sal_Int32 nIDForXNoteStream )
    : mpStream(pStream)
    , mnIDForXNoteStream( nIDForXNoteStream )
    , mxModel()
    , mxDrawPage()
    , mbIsSubstream( false )
{
}

OOXMLDocumentImpl::~OOXMLDocumentImpl()
{
}

void OOXMLDocumentImpl::resolveFastSubStream(Stream & rStreamHandler,
                                             OOXMLStream::StreamType_t nType)
{
    OOXMLStream::Pointer_t pStream
        (OOXMLDocumentFactory::createStream(mpStream, nType));

    uno::Reference< xml::sax::XFastParser > xParser
        (mpStream->getFastParser());

    if (xParser.is())
    {
        uno::Reference<uno::XComponentContext> xContext(mpStream->getContext());
        OOXMLFastDocumentHandler * pDocHandler =
            new OOXMLFastDocumentHandler( xContext, &rStreamHandler, this );

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
}

void OOXMLDocumentImpl::resolveFastSubStreamWithId(Stream & rStream,
                                      writerfilter::Reference<Stream>::Pointer_t pStream,
                      sal_uInt32 nId)
{
    rStream.substream(nId, pStream);
}

void OOXMLDocumentImpl::setIDForXNoteStream( const sal_Int32 nID )
{
    mnIDForXNoteStream = nID;
}

const sal_Int32 OOXMLDocumentImpl::getIDForXNoteStream() const
{
    return mnIDForXNoteStream;
}


const ::rtl::OUString & OOXMLDocumentImpl::getTarget() const
{
    return mpStream->getTarget();
}

writerfilter::Reference<Stream>::Pointer_t
OOXMLDocumentImpl::getSubStream(const rtl::OUString & rId)
{
    OOXMLStream::Pointer_t pStream
        (OOXMLDocumentFactory::createStream(mpStream, rId));

    writerfilter::Reference<Stream>::Pointer_t pRet(
        new OOXMLDocumentImpl(
            pStream,
            mxModel,
            mxDrawPage,
            true ) );

    return pRet;
}

writerfilter::Reference<Stream>::Pointer_t OOXMLDocumentImpl::getXNoteStream(
    OOXMLStream::StreamType_t nType,
    const sal_Int32 nIDForXNoteStream )
{
#ifdef DEBUG_ELEMENT
    debug_logger->startElement("getXNoteStream");
    debug_logger->attribute("id", rId);
    debug_logger->endElement("getXNoteStream");
#endif

    OOXMLStream::Pointer_t pStream =
        (OOXMLDocumentFactory::createStream(mpStream, nType));
    OOXMLDocumentImpl * pDocument =
        new OOXMLDocumentImpl(
            pStream,
            nIDForXNoteStream );

    return writerfilter::Reference<Stream>::Pointer_t(pDocument);
}

void OOXMLDocumentImpl::resolveFootnote(
    Stream & rStream,
    const Id & rType,
    const sal_Int32 nIDForXNoteStream )
{
    writerfilter::Reference<Stream>::Pointer_t pStream =
        getXNoteStream( OOXMLStream::FOOTNOTES, nIDForXNoteStream );

    Id nId;
    switch (rType)
    {
    case NS_ooxml::LN_Value_wordprocessingml_ST_FtnEdn_separator:
    case NS_ooxml::LN_Value_wordprocessingml_ST_FtnEdn_continuationSeparator:
        nId = rType;
        break;
    default:
        nId = NS_rtf::LN_footnote;
        break;
    }

    resolveFastSubStreamWithId( rStream, pStream, nId );
}

void OOXMLDocumentImpl::resolveEndnote(
    Stream & rStream,
    const Id & rType,
    const sal_Int32 nIDForXNoteStream )
{
    writerfilter::Reference<Stream>::Pointer_t pStream =
        getXNoteStream( OOXMLStream::ENDNOTES, nIDForXNoteStream );

    Id nId;
    switch (rType)
    {
    case NS_ooxml::LN_Value_wordprocessingml_ST_FtnEdn_separator:
    case NS_ooxml::LN_Value_wordprocessingml_ST_FtnEdn_continuationSeparator:
        nId = rType;
        break;
    default:
        nId = NS_rtf::LN_endnote;
        break;
    }

    resolveFastSubStreamWithId( rStream, pStream, nId );
}

void OOXMLDocumentImpl::resolveComment(
    Stream & rStream,
    const sal_Int32 nIDForXNoteStream )
{
    writerfilter::Reference<Stream>::Pointer_t pStream =
        getXNoteStream(OOXMLStream::COMMENTS, nIDForXNoteStream );

    resolveFastSubStreamWithId( rStream, pStream, NS_rtf::LN_annotation );
}

OOXMLPropertySet * OOXMLDocumentImpl::getPicturePropSet
(const ::rtl::OUString & rId)
{
    OOXMLStream::Pointer_t pStream
        (OOXMLDocumentFactory::createStream(mpStream, rId));

    writerfilter::Reference<BinaryObj>::Pointer_t pPicture
        (new OOXMLBinaryObjectReference(pStream));

    OOXMLValue::Pointer_t pPayloadValue(new OOXMLBinaryValue(pPicture));

    OOXMLProperty::Pointer_t pPayloadProperty
        (new OOXMLPropertyImpl(NS_rtf::LN_payload, pPayloadValue,
                               OOXMLPropertyImpl::ATTRIBUTE));

    OOXMLPropertySet::Pointer_t pBlipSet(new OOXMLPropertySetImpl());

    pBlipSet->add(pPayloadProperty);

    OOXMLValue::Pointer_t pBlipValue(new OOXMLPropertySetValue(pBlipSet));

    OOXMLProperty::Pointer_t pBlipProperty
        (new OOXMLPropertyImpl(NS_rtf::LN_blip, pBlipValue,
                               OOXMLPropertyImpl::ATTRIBUTE));

    OOXMLPropertySet * pProps = new OOXMLPropertySetImpl();

    pProps->add(pBlipProperty);

    return pProps;
}

void OOXMLDocumentImpl::resolvePicture(Stream & rStream,
                                       const rtl::OUString & rId)
{
    OOXMLPropertySet * pProps = getPicturePropSet(rId);

    rStream.props(writerfilter::Reference<Properties>::Pointer_t(pProps));
}

::rtl::OUString OOXMLDocumentImpl::getTargetForId(const ::rtl::OUString & rId)
{
    return mpStream->getTargetForId(rId);
}

void OOXMLDocumentImpl::resolveHeader(Stream & rStream,
                                      const sal_Int32 type,
                                      const rtl::OUString & rId)
{
     writerfilter::Reference<Stream>::Pointer_t pStream =
         getSubStream(rId);
     switch (type)
     {
     case NS_ooxml::LN_Value_ST_HrdFtr_even:
         resolveFastSubStreamWithId(rStream, pStream, NS_rtf::LN_headerl);
        break;
     case NS_ooxml::LN_Value_ST_HrdFtr_default: // here we assume that default is right, but not necessarily true :-(
         resolveFastSubStreamWithId(rStream, pStream, NS_rtf::LN_headerr);
         break;
     case NS_ooxml::LN_Value_ST_HrdFtr_first:
         resolveFastSubStreamWithId(rStream, pStream, NS_rtf::LN_headerf);
         break;
     default:
         break;
     }
}

void OOXMLDocumentImpl::resolveFooter(Stream & rStream,
                                      const sal_Int32 type,
                                      const rtl::OUString & rId)
{
     writerfilter::Reference<Stream>::Pointer_t pStream =
         getSubStream(rId);

     switch (type)
     {
     case NS_ooxml::LN_Value_ST_HrdFtr_even:
         resolveFastSubStreamWithId(rStream, pStream, NS_rtf::LN_footerl);
         break;
     case NS_ooxml::LN_Value_ST_HrdFtr_default: // here we assume that default is right, but not necessarily true :-(
         resolveFastSubStreamWithId(rStream, pStream, NS_rtf::LN_footerr);
         break;
     case NS_ooxml::LN_Value_ST_HrdFtr_first:
         resolveFastSubStreamWithId(rStream, pStream, NS_rtf::LN_footerf);
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
            new OOXMLFastDocumentHandler( xContext, &rStream, this );
        pDocHandler->setIsSubstream( mbIsSubstream );
        uno::Reference < xml::sax::XFastDocumentHandler > xDocumentHandler
            (pDocHandler);
        uno::Reference < xml::sax::XFastTokenHandler > xTokenHandler
            (mpStream->getFastTokenHandler(xContext));

        resolveFastSubStream(rStream, OOXMLStream::SETTINGS);
        resolveFastSubStream(rStream, OOXMLStream::THEME);
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
    debug_logger->endElement("OOXMLDocumentImpl.resolve");
#endif
}

uno::Reference<io::XInputStream> OOXMLDocumentImpl::getInputStreamForId(const ::rtl::OUString & rId)
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

OOXMLDocument * OOXMLDocumentFactory::createDocument(
    OOXMLStream::Pointer_t pStream )
{
    return new OOXMLDocumentImpl(pStream);
}

}}

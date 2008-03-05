/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OOXMLDocumentImpl.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:03:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif

#include <doctok/resourceids.hxx>
#include <ooxml/resourceids.hxx>
#include "OOXMLDocumentImpl.hxx"
#include "OOXMLBinaryObjectReference.hxx"
#include "OOXMLFastDocumentHandler.hxx"
#include "OOXMLPropertySetImpl.hxx"

#include <iostream>

namespace writerfilter {
namespace ooxml
{

using namespace ::std;

OOXMLDocumentImpl::OOXMLDocumentImpl
(OOXMLStream::Pointer_t pStream)
: mpStream(pStream)
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
            new OOXMLFastDocumentHandler(xContext);
        pDocHandler->setStream(&rStreamHandler);
        pDocHandler->setDocument(this);
        pDocHandler->setXNoteId(msXNoteId);

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

void OOXMLDocumentImpl::setXNoteId(const rtl::OUString & rId)
{
    msXNoteId = rId;
}

const rtl::OUString & OOXMLDocumentImpl::getXNoteId() const
{
    return msXNoteId;
}

writerfilter::Reference<Stream>::Pointer_t
OOXMLDocumentImpl::getSubStream(const rtl::OUString & rId)
{
    OOXMLStream::Pointer_t pStream
        (OOXMLDocumentFactory::createStream(mpStream, rId));

    return writerfilter::Reference<Stream>::Pointer_t(new OOXMLDocumentImpl(pStream));
}

writerfilter::Reference<Stream>::Pointer_t
OOXMLDocumentImpl::getXNoteStream(OOXMLStream::StreamType_t nType, const rtl::OUString & rId)
{
#ifdef DEBUG_ELEMENT
    string tmp = "<getXNoteStream id=\"";
    tmp += OUStringToOString(rId, RTL_TEXTENCODING_ASCII_US).getStr();
    tmp += "\"/>";
    logger("DEBUG", tmp);
#endif

    OOXMLStream::Pointer_t pStream =
        (OOXMLDocumentFactory::createStream(mpStream, nType));
    OOXMLDocumentImpl * pDocument = new OOXMLDocumentImpl(pStream);
    pDocument->setXNoteId(rId);

    return writerfilter::Reference<Stream>::Pointer_t(pDocument);
}

void OOXMLDocumentImpl::resolveFootnote(Stream & rStream,
                                        const rtl::OUString & rNoteId)
{
    writerfilter::Reference<Stream>::Pointer_t pStream =
        getXNoteStream(OOXMLStream::FOOTNOTES, rNoteId);

    resolveFastSubStreamWithId(rStream, pStream, NS_rtf::LN_footnote);
}

void OOXMLDocumentImpl::resolveEndnote(Stream & rStream,
                                       const rtl::OUString & rNoteId)
{
    writerfilter::Reference<Stream>::Pointer_t pStream =
        getXNoteStream(OOXMLStream::ENDNOTES, rNoteId);

    resolveFastSubStreamWithId(rStream, pStream, NS_rtf::LN_endnote);
}

void OOXMLDocumentImpl::resolveComment(Stream & rStream,
                                       const rtl::OUString & rId)
{
    writerfilter::Reference<Stream>::Pointer_t pStream =
        getXNoteStream(OOXMLStream::COMMENTS, rId);

    resolveFastSubStreamWithId(rStream, pStream, NS_rtf::LN_annotation);
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
    uno::Reference< xml::sax::XFastParser > xParser
        (mpStream->getFastParser());

    if (xParser.is())
    {
        uno::Reference<uno::XComponentContext> xContext(mpStream->getContext());

        OOXMLFastDocumentHandler * pDocHandler =
            new OOXMLFastDocumentHandler(xContext);
        pDocHandler->setStream(&rStream);
        pDocHandler->setDocument(this);
        pDocHandler->setXNoteId(msXNoteId);
        uno::Reference < xml::sax::XFastDocumentHandler > xDocumentHandler
            (pDocHandler);
        uno::Reference < xml::sax::XFastTokenHandler > xTokenHandler
            (mpStream->getFastTokenHandler(xContext));

        resolveFastSubStream(rStream, OOXMLStream::THEME);
        resolveFastSubStream(rStream, OOXMLStream::NUMBERING);
        resolveFastSubStream(rStream, OOXMLStream::FONTTABLE);
        resolveFastSubStream(rStream, OOXMLStream::STYLES);

        xParser->setFastDocumentHandler( xDocumentHandler );
        xParser->setTokenHandler( xTokenHandler );

        xml::sax::InputSource aParserInput;
        aParserInput.aInputStream = mpStream->getDocumentStream();
        xParser->parseStream(aParserInput);
    }
}

uno::Reference<io::XInputStream> OOXMLDocumentImpl::getInputStreamForId(const ::rtl::OUString & rId)
{
    OOXMLStream::Pointer_t pStream(OOXMLDocumentFactory::createStream(mpStream, rId));

    return pStream->getInputStream();
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

void OOXMLDocumentImpl::setShapes(uno::Reference<drawing::XShapes> xShapes)
{
    mxShapes.set(xShapes);
}

uno::Reference<drawing::XShapes> OOXMLDocumentImpl::getShapes()
{
    return mxShapes;
}

uno::Reference<io::XInputStream> OOXMLDocumentImpl::getInputStream()
{
    return mpStream->getInputStream();
}

OOXMLDocument *
OOXMLDocumentFactory::createDocument
(OOXMLStream::Pointer_t pStream)
{
    return new OOXMLDocumentImpl(pStream);
}

}}

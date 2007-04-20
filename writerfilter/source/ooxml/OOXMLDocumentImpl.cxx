/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OOXMLDocumentImpl.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hbrinkm $ $Date: 2007-04-20 14:36:20 $
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

#include "OOXMLDocumentImpl.hxx"
#include "OOXMLSaxHandler.hxx"

#include <iostream>

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

void OOXMLDocumentImpl::resolveSubStream(Stream & rStream,
                                         OOXMLStream::StreamType_t nType)
{
    OOXMLStream::Pointer_t pStream
        (OOXMLDocumentFactory::createStream(mpStream, nType));

    uno::Reference < xml::sax::XParser > oSaxParser =
        pStream->getParser();

    if (oSaxParser.is())
    {
        uno::Reference<xml::sax::XDocumentHandler>
            xDocumentHandler
            (static_cast<cppu::OWeakObject *>
             (new OOXMLSaxHandler(rStream)), uno::UNO_QUERY);
        oSaxParser->setDocumentHandler( xDocumentHandler );

        uno::Reference<io::XInputStream> xInputStream =
            pStream->getInputStream();

        if (xInputStream.is())
        {

            //                 uno::Sequence<sal_Int8> aSeq(1024);
            //                 while (xStylesInputStream->readBytes(aSeq, 1024) > 0)
            //                 {
            //                     string tmpStr(reinterpret_cast<char *>(&aSeq[0]));

            //                     clog << tmpStr;
            //                 }
            struct xml::sax::InputSource oInputSource;
            oInputSource.aInputStream = xInputStream;
            oSaxParser->parseStream(oInputSource);

            xInputStream->closeInput();
        }
    }
}

void OOXMLDocumentImpl::resolve(Stream & rStream)
{
    {
        uno::Reference < xml::sax::XParser > oSaxParser = mpStream->getParser();

        if (oSaxParser.is())
        {
            uno::Reference<xml::sax::XDocumentHandler>
                xDocumentHandler
                (static_cast<cppu::OWeakObject *>
                 (new OOXMLSaxHandler(rStream)), uno::UNO_QUERY);
            oSaxParser->setDocumentHandler( xDocumentHandler );

            resolveSubStream(rStream, OOXMLStream::NUMBERING);
            resolveSubStream(rStream, OOXMLStream::FONTTABLE);
            resolveSubStream(rStream, OOXMLStream::STYLES);

            uno::Reference<io::XInputStream> xInputStream
                (mpStream->getInputStream());

            struct xml::sax::InputSource oInputSource;
            oInputSource.aInputStream = xInputStream;
            oSaxParser->parseStream(oInputSource);

            xInputStream->closeInput();
        }
    }
}

string OOXMLDocumentImpl::getType() const
{
    return "OOXMLDocumentImpl";
}

OOXMLDocument *
OOXMLDocumentFactory::createDocument
(OOXMLStream::Pointer_t pStream)
{
    return new OOXMLDocumentImpl(pStream);
}

}

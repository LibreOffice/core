/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OOXMLDocumentImpl.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: hbrinkm $ $Date: 2007-02-21 12:26:24 $
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

namespace ooxml
{

OOXMLDocumentImpl::OOXMLDocumentImpl
(OOXMLStream::Pointer_t pStream)
: mpStream(pStream)
{
}

OOXMLDocumentImpl::~OOXMLDocumentImpl()
{
}

void OOXMLDocumentImpl::resolve(Stream & rStream)
{
    uno::Reference < xml::sax::XParser > oSaxParser = mpStream->getParser();

    if (oSaxParser.is())
    {
        uno::Reference<xml::sax::XDocumentHandler>
            xDocumentHandler
            (static_cast<cppu::OWeakObject *>
             (new OOXMLSaxHandler(rStream)), uno::UNO_QUERY);
        oSaxParser->setDocumentHandler( xDocumentHandler );

        uno::Reference<io::XInputStream> xInputStream =
            mpStream->getInputStream();

        struct xml::sax::InputSource oInputSource;
        oInputSource.aInputStream = xInputStream;
        oSaxParser->parseStream(oInputSource);

        xInputStream->closeInput();
    }

    rStream.info("Test");
}

string OOXMLDocumentImpl::getType() const
{
    return "OOXMLDocumentImpl";
}

OOXMLDocument::Pointer_t
OOXMLDocumentFactory::createDocument
(OOXMLStream::Pointer_t pStream)
{
    return OOXMLDocument::Pointer_t(new OOXMLDocumentImpl(pStream));
}

}

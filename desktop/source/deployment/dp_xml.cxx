/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dp_xml.cxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "dp_misc.h"
#include "dp_xml.h"
#include "rtl/ustrbuf.hxx"
#include "ucbhelper/content.hxx"
#include "com/sun/star/xml/sax/XParser.hpp"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace dp_misc
{

//==============================================================================
void xml_parse(
    Reference<xml::sax::XDocumentHandler> const & xDocHandler,
    ::ucbhelper::Content & ucb_content,
    Reference<XComponentContext> const & xContext )
{
    // raise sax parser:
    Reference<xml::sax::XParser> xParser(
        xContext->getServiceManager()->createInstanceWithContext(
            OUSTR("com.sun.star.xml.sax.Parser"), xContext ), UNO_QUERY_THROW );

    // error handler, entity resolver omitted
    xParser->setDocumentHandler( xDocHandler );
    xml::sax::InputSource source;
    source.aInputStream = ucb_content.openStream();
    source.sSystemId = ucb_content.getURL();
    xParser->parseStream( source );
}

//==============================================================================
void xml_parse(
    Reference<xml::input::XRoot> const & xRoot,
    ::ucbhelper::Content & ucb_content,
    Reference<XComponentContext> const & xContext )
{
    const Any arg(xRoot);
    const Reference<xml::sax::XDocumentHandler> xDocHandler(
        xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            OUSTR("com.sun.star.xml.input.SaxDocumentHandler"),
            Sequence<Any>( &arg, 1 ), xContext ), UNO_QUERY_THROW );
    xml_parse( xDocHandler, ucb_content, xContext );
}

//##############################################################################

//______________________________________________________________________________
XmlRootElement::XmlRootElement(
    OUString const & uri, OUString const & localname )
    : m_uri( uri )
{
    m_localname = localname;
}

//______________________________________________________________________________
XmlRootElement::~XmlRootElement()
{
}

// XRoot
//______________________________________________________________________________
void XmlRootElement::startDocument(
    Reference<xml::input::XNamespaceMapping> const & xMapping )
    throw (xml::sax::SAXException, RuntimeException)
{
    m_xNamespaceMapping = xMapping;

    try {
        m_uid = m_xNamespaceMapping->getUidByUri( m_uri );
    }
    catch (container::NoSuchElementException & exc) {
        throw xml::sax::SAXException(
            exc.Message, static_cast<OWeakObject *>(this), Any(exc) );
    }
}

//______________________________________________________________________________
void XmlRootElement::endDocument()
    throw (xml::sax::SAXException, RuntimeException)
{
}

//______________________________________________________________________________
void XmlRootElement::processingInstruction(
    OUString const &, OUString const & )
    throw (xml::sax::SAXException, RuntimeException)
{
}

//______________________________________________________________________________
void XmlRootElement::setDocumentLocator(
    Reference<xml::sax::XLocator> const & )
    throw (xml::sax::SAXException, RuntimeException)
{
}

//______________________________________________________________________________
Reference<xml::input::XElement> XmlRootElement::startRootElement(
    sal_Int32 uid, OUString const & localname,
    Reference<xml::input::XAttributes> const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    check_xmlns( uid );
    if (! localname.equals( m_localname )) {
        throw xml::sax::SAXException(
            OUSTR("unexpected root element ") + localname,
            static_cast<OWeakObject *>(this), Any() );
    }
    m_xAttributes = xAttributes;

    return this;
}

//##############################################################################

//______________________________________________________________________________
XmlElement::~XmlElement()
{
}

//______________________________________________________________________________
Reference<xml::input::XNamespaceMapping> const &
XmlElement::getNamespaceMapping() const
{
    if (! m_xNamespaceMapping.is()) {
        throw RuntimeException(
            OUSTR("document has not been parsed yet!"),
            static_cast<OWeakObject *>( const_cast<XmlElement *>(this) ) );
    }
    return m_xNamespaceMapping;
}

//______________________________________________________________________________
void XmlElement::check_xmlns( sal_Int32 uid ) const
    throw (xml::sax::SAXException)
{
    if (uid != m_uid)
    {
        ::rtl::OUStringBuffer buf;
        buf.appendAscii(
            RTL_CONSTASCII_STRINGPARAM("illegal xml namespace uri=\"") );
        try {
            buf.append( m_xNamespaceMapping->getUriByUid( uid ) );
        }
        catch (container::NoSuchElementException & exc) {
            throw xml::sax::SAXException(
                exc.Message, static_cast<OWeakObject *>(
                    const_cast<XmlElement *>(this) ), Any(exc) );
        }
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\"!") );
        throw xml::sax::SAXException(
            buf.makeStringAndClear(),
            static_cast<OWeakObject *>( const_cast<XmlElement *>(this) ),
            Any() );
    }
}

//______________________________________________________________________________
void XmlElement::check_parsed() const
    throw (xml::sax::SAXException)
{
    if (! isParsed()) {
        ::rtl::OUStringBuffer buf;
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("missing closing element "
                                                    "event for \"") );
        buf.append( m_localname );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\"!") );
        throw xml::sax::SAXException(
            buf.makeStringAndClear(),
            static_cast<OWeakObject *>( const_cast<XmlElement *>(this) ),
            Any() );
    }
}

// XElement
//______________________________________________________________________________
Reference<xml::input::XElement> XmlElement::getParent()
    throw (RuntimeException)
{
    return m_xParent;
}

//______________________________________________________________________________
OUString XmlElement::getLocalName()
    throw (RuntimeException)
{
    return m_localname;
}

//______________________________________________________________________________
sal_Int32 XmlElement::getUid()
    throw (RuntimeException)
{
    return m_uid;
}

//______________________________________________________________________________
Reference<xml::input::XAttributes> XmlElement::getAttributes()
    throw (RuntimeException)
{
    return m_xAttributes;
}

//______________________________________________________________________________
void XmlElement::ignorableWhitespace(
    OUString const & )
    throw (xml::sax::SAXException, RuntimeException)
{
}

//______________________________________________________________________________
void XmlElement::characters( OUString const & chars )
    throw (xml::sax::SAXException, RuntimeException)
{
    m_characters += chars;
}

//______________________________________________________________________________
void XmlElement::processingInstruction(
    OUString const &, OUString const & )
    throw (xml::sax::SAXException, RuntimeException)
{
}

//______________________________________________________________________________
void XmlElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    m_got_endElement = true;
}

//______________________________________________________________________________
Reference<xml::input::XElement> XmlElement::startChildElement(
    sal_Int32 uid, OUString const & localName,
    Reference<xml::input::XAttributes> const & )
    throw (xml::sax::SAXException, RuntimeException)
{
    ::rtl::OUStringBuffer buf;
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("unexpected element "
                                                "{ tag=\"") );
    buf.append( localName );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\", uri=\"") );
    try {
        buf.append( m_xNamespaceMapping->getUriByUid( uid ) );
    }
    catch (container::NoSuchElementException & exc) {
        throw xml::sax::SAXException(
            exc.Message, static_cast<OWeakObject *>(this), Any(exc) );
    }
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\" }!") );
    throw xml::sax::SAXException(
        buf.makeStringAndClear(), static_cast<OWeakObject *>(this), Any() );
}

}

/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dp_xml.h,v $
 * $Revision: 1.7 $
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

#if ! defined INCLUDED_DP_XML_H
#define INCLUDED_DP_XML_H

#include "rtl/ref.hxx"
#include "rtl/ustrbuf.hxx"
#include "cppuhelper/implbase1.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/xml/input/XRoot.hpp"
#include "com/sun/star/xml/sax/XDocumentHandler.hpp"


namespace ucbhelper
{
class Content;
}

namespace css = ::com::sun::star;

namespace dp_misc
{

//==============================================================================
void xml_parse(
    css::uno::Reference< css::xml::sax::XDocumentHandler > const & xDocHandler,
    ::ucbhelper::Content & ucb_content,
    css::uno::Reference< css::uno::XComponentContext > const & xContext );

//==============================================================================
void xml_parse(
    css::uno::Reference< css::xml::input::XRoot > const & xRoot,
    ::ucbhelper::Content & ucb_content,
    css::uno::Reference< css::uno::XComponentContext > const & xContext );

//==============================================================================
class XmlElement : public ::cppu::WeakImplHelper1< css::xml::input::XElement >
{
protected:
    css::uno::Reference<css::xml::input::XNamespaceMapping> m_xNamespaceMapping;
    const css::uno::Reference<css::xml::input::XElement> m_xParent;
    sal_Int32 m_uid;
    ::rtl::OUString m_localname;
    css::uno::Reference<css::xml::input::XAttributes> m_xAttributes;
    ::rtl::OUString m_characters;
    bool m_got_endElement;

    void check_xmlns( sal_Int32 uid ) const throw (css::xml::sax::SAXException);

    inline XmlElement()
        : m_uid( -1 ),
          m_got_endElement( false )
        {}
    virtual ~XmlElement();
public:
    css::uno::Reference<css::xml::input::XNamespaceMapping> const &
    getNamespaceMapping() const;

    inline bool isParsed() const { return m_got_endElement; }
    void check_parsed() const throw (css::xml::sax::SAXException);

    inline ::rtl::OUString getCharacters() const;

    inline XmlElement(
        css::uno::Reference<css::xml::input::XNamespaceMapping>
        const & xMapping,
        css::uno::Reference<css::xml::input::XElement> const & xParent,
        sal_Int32 uid, ::rtl::OUString const & localname,
        css::uno::Reference< css::xml::input::XAttributes >
        const & xAttributes )
        : m_xNamespaceMapping( xMapping ),
          m_xParent( xParent ),
          m_uid( uid ),
          m_localname( localname ),
          m_xAttributes( xAttributes ),
          m_got_endElement( false )
        {}

    // XElement
    virtual css::uno::Reference<css::xml::input::XElement> SAL_CALL
    getParent() throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getLocalName()
        throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getUid()
        throw (css::uno::RuntimeException);
    virtual css::uno::Reference<css::xml::input::XAttributes> SAL_CALL
    getAttributes() throw (css::uno::RuntimeException);
    virtual void SAL_CALL ignorableWhitespace(
        ::rtl::OUString const & rWhitespaces )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException);
    virtual void SAL_CALL characters( ::rtl::OUString const & rChars )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException);
    virtual void SAL_CALL processingInstruction(
        ::rtl::OUString const & Target, ::rtl::OUString const & Data )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException);
    virtual void SAL_CALL endElement()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException);
    virtual css::uno::Reference<css::xml::input::XElement> SAL_CALL
    startChildElement(
        sal_Int32 nUid, ::rtl::OUString const & rLocalName,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException);
};

//______________________________________________________________________________
inline ::rtl::OUString XmlElement::getCharacters() const
{
    check_parsed();
    return m_characters;
}


//==============================================================================
class XmlRootElement : public ::cppu::ImplInheritanceHelper1<
                                        XmlElement, css::xml::input::XRoot >
{
    const ::rtl::OUString m_uri;

protected:
    virtual ~XmlRootElement();
public:
    inline ::rtl::OUString const & getUri() const
        { return m_uri; }

    XmlRootElement(
        ::rtl::OUString const & uri, ::rtl::OUString const & localname );

    // XRoot
    virtual void SAL_CALL startDocument(
        css::uno::Reference<css::xml::input::XNamespaceMapping>
        const & xMapping )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException);
    virtual void SAL_CALL endDocument()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException);
    virtual void SAL_CALL processingInstruction(
        ::rtl::OUString const & target, ::rtl::OUString const & data )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException);
    virtual void SAL_CALL setDocumentLocator(
        css::uno::Reference<css::xml::sax::XLocator> const & xLocator )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException);
    virtual css::uno::Reference<css::xml::input::XElement> SAL_CALL
    startRootElement(
        sal_Int32 uid, ::rtl::OUString const & localname,
        css::uno::Reference<css::xml::input::XAttributes> const & xAttributes )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException);
};

}

#endif

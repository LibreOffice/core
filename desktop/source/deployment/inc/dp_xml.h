/*************************************************************************
 *
 *  $RCSfile: dp_xml.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2004-06-11 12:07:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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


namespace ucb
{
class Content;
}

namespace css = ::com::sun::star;

namespace dp_misc
{

//==============================================================================
void xml_parse(
    css::uno::Reference< css::xml::sax::XDocumentHandler > const & xDocHandler,
    ::ucb::Content & ucb_content,
    css::uno::Reference< css::uno::XComponentContext > const & xContext );

//==============================================================================
void xml_parse(
    css::uno::Reference< css::xml::input::XRoot > const & xRoot,
    ::ucb::Content & ucb_content,
    css::uno::Reference< css::uno::XComponentContext > const & xContext );

//==============================================================================
class XmlElement : public ::cppu::WeakImplHelper1< css::xml::input::XElement >
{
protected:
    css::uno::Reference< css::xml::input::XNamespaceMapping >
    m_xNamespaceMapping;
    css::uno::Reference< css::xml::input::XElement > m_xParent;
    sal_Int32 m_uid;
    ::rtl::OUString m_localname;
    css::uno::Reference< css::xml::input::XAttributes > m_xAttributes;
    ::rtl::OUString m_characters;
    bool m_got_endElement;

    void check_xmlns( sal_Int32 uid ) const throw (css::xml::sax::SAXException);

    inline XmlElement()
        : m_uid( -1 ),
          m_got_endElement( false )
        {}
    virtual ~XmlElement();
public:
    css::uno::Reference< css::xml::input::XNamespaceMapping > const &
    getNamespaceMapping() const;

    inline bool isParsed() const { return m_got_endElement; }
    void check_parsed() const throw (css::xml::sax::SAXException);

    inline ::rtl::OUString getCharacters() const;

    inline XmlElement(
        css::uno::Reference< css::xml::input::XNamespaceMapping >
        const & xMapping,
        css::uno::Reference< css::xml::input::XElement > const & xParent,
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
    virtual css::uno::Reference< css::xml::input::XElement > SAL_CALL
    getParent() throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getLocalName()
        throw (css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getUid()
        throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::xml::input::XAttributes > SAL_CALL
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
    virtual css::uno::Reference< css::xml::input::XElement > SAL_CALL
    startChildElement(
        sal_Int32 nUid, ::rtl::OUString const & rLocalName,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes
        ) throw (css::xml::sax::SAXException, css::uno::RuntimeException);
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
    ::rtl::OUString m_uri;

protected:
    virtual ~XmlRootElement();
public:
    inline ::rtl::OUString const & getUri() const
        { return m_uri; }

    XmlRootElement(
        ::rtl::OUString const & uri, ::rtl::OUString const & localname );

    // XRoot
    virtual void SAL_CALL startDocument(
        css::uno::Reference< css::xml::input::XNamespaceMapping >
        const & xMapping )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException);
    virtual void SAL_CALL endDocument()
        throw (css::xml::sax::SAXException, css::uno::RuntimeException);
    virtual void SAL_CALL processingInstruction(
        ::rtl::OUString const & target, ::rtl::OUString const & data )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException);
    virtual void SAL_CALL setDocumentLocator(
        css::uno::Reference< css::xml::sax::XLocator > const & xLocator )
        throw (css::xml::sax::SAXException, css::uno::RuntimeException);
    virtual css::uno::Reference< css::xml::input::XElement > SAL_CALL
    startRootElement(
        sal_Int32 uid, ::rtl::OUString const & localname,
        css::uno::Reference< css::xml::input::XAttributes > const & xAttributes
        ) throw (css::xml::sax::SAXException, css::uno::RuntimeException);
};

}

#endif

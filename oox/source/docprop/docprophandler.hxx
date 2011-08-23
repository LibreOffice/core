/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef OOX_DOCPROP_DOCPROPHANDLER_HXX
#define OOX_DOCPROP_DOCPROPHANDLER_HXX

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/xml/sax/XFastDocumentHandler.hpp>

#include <cppuhelper/implbase1.hxx>

namespace oox {
namespace docprop {

class OOXMLDocPropHandler : public ::cppu::WeakImplHelper1< ::com::sun::star::xml::sax::XFastDocumentHandler >
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentProperties > m_xDocProp;

    sal_Int32 m_nState;
    sal_Int32 m_nBlock;
    sal_Int32 m_nType;

    sal_Int32 m_nInBlock;

    ::rtl::OUString m_aCustomPropertyName;

public:
    explicit            OOXMLDocPropHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext, const ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentProperties > xDocProp );

    virtual             ~OOXMLDocPropHandler();

    void InitNew();
    void AddCustomProperty( const ::com::sun::star::uno::Any& aAny );

    ::com::sun::star::util::DateTime GetDateTimeFromW3CDTF( const ::rtl::OUString& aChars );
    ::com::sun::star::uno::Sequence< ::rtl::OUString > GetKeywordsSet( const ::rtl::OUString& aChars );
    ::com::sun::star::lang::Locale GetLanguage( const ::rtl::OUString& aChars );
    void UpdateDocStatistic( const ::rtl::OUString& aChars );

    // com.sun.star.xml.sax.XFastDocumentHandler

    virtual void SAL_CALL startDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDocumentLocator( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >& rxLocator ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

    // com.sun.star.xml.sax.XFastContextHandler

    virtual void SAL_CALL startFastElement( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL startUnknownElement( const ::rtl::OUString& Namespace, const ::rtl::OUString& Name, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endFastElement( ::sal_Int32 Element ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endUnknownElement( const ::rtl::OUString& Namespace, const ::rtl::OUString& Name ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createUnknownChildContext( const ::rtl::OUString& Namespace, const ::rtl::OUString& Name, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL characters( const ::rtl::OUString& aChars ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL ignorableWhitespace( const ::rtl::OUString& aWhitespaces ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL processingInstruction( const ::rtl::OUString& aTarget, const ::rtl::OUString& aData ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

};

// ============================================================================

} // namespace core
} // namespace oox

#endif


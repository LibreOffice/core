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

#ifndef INCLUDED_OOX_SOURCE_DOCPROP_DOCPROPHANDLER_HXX
#define INCLUDED_OOX_SOURCE_DOCPROP_DOCPROPHANDLER_HXX

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/xml/sax/XFastDocumentHandler.hpp>

#include <cppuhelper/implbase.hxx>

#include "oox/token/namespaces.hxx"
#include "oox/token/tokens.hxx"

namespace oox {
namespace docprop {

#define COREPR_TOKEN( token )   (::oox::NMSP_packageMetaCorePr | XML_##token)
#define CUSTPR_TOKEN( token )   (::oox::NMSP_officeCustomPr | XML_##token)
#define EXTPR_TOKEN( token )    (::oox::NMSP_officeExtPr | XML_##token)
#define VT_TOKEN( token )       (::oox::NMSP_officeDocPropsVT | XML_##token)
#define DC_TOKEN( token )       (::oox::NMSP_dc | XML_##token)
#define DCT_TOKEN( token )      (::oox::NMSP_dcTerms | XML_##token)

class OOXMLDocPropHandler : public ::cppu::WeakImplHelper< css::xml::sax::XFastDocumentHandler >
{
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::document::XDocumentProperties > m_xDocProp;

    sal_Int32 m_nState;
    sal_Int32 m_nBlock;
    sal_Int32 m_nType;

    sal_Int32 m_nInBlock;

    enum { NONE, INSERTED } m_CustomStringPropertyState;
    OUString m_aCustomPropertyName;

public:
    explicit            OOXMLDocPropHandler( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::document::XDocumentProperties >& rDocProp );

    virtual             ~OOXMLDocPropHandler();

    void InitNew();
    void AddCustomProperty( const css::uno::Any& aAny );

    static css::util::DateTime GetDateTimeFromW3CDTF( const OUString& aChars );
    static css::uno::Sequence< OUString > GetKeywordsSet( const OUString& aChars );
    void UpdateDocStatistic( const OUString& aChars );

    // com.sun.star.xml.sax.XFastDocumentHandler

    virtual void SAL_CALL startDocument() throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endDocument() throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDocumentLocator( const css::uno::Reference< css::xml::sax::XLocator >& rxLocator ) throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

    // com.sun.star.xml.sax.XFastContextHandler

    virtual void SAL_CALL startFastElement( ::sal_Int32 Element, const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs ) throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL startUnknownElement( const OUString& Namespace, const OUString& Name, const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs ) throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endFastElement( ::sal_Int32 Element ) throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL endUnknownElement( const OUString& Namespace, const OUString& Name ) throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs ) throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createUnknownChildContext( const OUString& Namespace, const OUString& Name, const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs ) throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL characters( const OUString& aChars ) throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;

};

} // namespace core
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

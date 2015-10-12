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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_REGISTRY_SFWK_DP_PARCELDESC_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_REGISTRY_SFWK_DP_PARCELDESC_HXX

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>

#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
namespace dp_registry
{
namespace backend
{
namespace sfwk
{

class ParcelDescDocHandler : public ::cppu::WeakImplHelper< css::xml::sax::XDocumentHandler >
{
private:
    bool m_bIsParsed;
    OUString m_sLang;
    sal_Int32 skipIndex;
public:
    ParcelDescDocHandler():m_bIsParsed( false ), skipIndex( 0 ){}
    OUString getParcelLanguage() { return m_sLang; }
    bool isParsed() { return m_bIsParsed; }
    // XDocumentHandler
    virtual void SAL_CALL startDocument()
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL endDocument()
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL startElement( const OUString& aName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttribs )
        throw ( css::xml::sax::SAXException,
            css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL endElement( const OUString & aName )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL characters( const OUString & aChars )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL ignorableWhitespace( const OUString & aWhitespaces )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL processingInstruction(
        const OUString & aTarget, const OUString & aData )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL setDocumentLocator(
        const css::uno::Reference< css::xml::sax::XLocator >& xLocator )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) override;
};
}
}
}

#endif // INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_REGISTRY_SFWK_DP_PARCELDESC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

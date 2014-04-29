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

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>

class PersonasDocHandler : public ::cppu::WeakImplHelper1< css::xml::sax::XDocumentHandler >
{
private:
    OUString m_sHeaderURL;
    OUString m_sFooterURL;
    OUString m_sTextColor;
    OUString m_sAccentColor;
public:
    PersonasDocHandler(){}
    OUString getHeaderURL() { return m_sHeaderURL; }
    OUString getFooterURL() { return m_sFooterURL; }
    OUString getTextColor() { return m_sTextColor; }
    OUString getAccentColor() { return m_sAccentColor; }
    // XDocumentHandler
    virtual void SAL_CALL startDocument()
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL endDocument()
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL startElement( const OUString& aName,
        const css::uno::Reference< css::xml::sax::XAttributeList > & xAttribs )
        throw ( css::xml::sax::SAXException,
            css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL endElement( const OUString & aName )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL characters( const OUString & aChars )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL ignorableWhitespace( const OUString & aWhitespaces )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL processingInstruction(
        const OUString & aTarget, const OUString & aData )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual void SAL_CALL setDocumentLocator(
        const css::uno::Reference< css::xml::sax::XLocator >& xLocator )
        throw ( css::xml::sax::SAXException, css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

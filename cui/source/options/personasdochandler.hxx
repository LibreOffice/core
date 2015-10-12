/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CUI_SOURCE_OPTIONS_PERSONASDOCHANDLER_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_PERSONASDOCHANDLER_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <vector>

class PersonasDocHandler : public ::cppu::WeakImplHelper< css::xml::sax::XDocumentHandler >
{
private:
    std::vector<OUString> m_vLearnmoreURLs;
    bool m_isLearnmoreTag, m_hasResults;
public:
    PersonasDocHandler(){ m_isLearnmoreTag = false; m_hasResults = false; }
    std::vector<OUString> getLearnmoreURLs() { return m_vLearnmoreURLs; }
    bool hasResults() { return m_hasResults; }

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

#endif // INCLUDED_CUI_SOURCE_OPTIONS_PERSONASDOCHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

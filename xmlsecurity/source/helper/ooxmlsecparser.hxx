/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

#include <cppuhelper/implbase.hxx>

class XSecController;
class XMLSignatureHelper;

/// Parses an OOXML digital signature.
class OOXMLSecParser: public cppu::WeakImplHelper
    <
    css::xml::sax::XDocumentHandler,
    css::lang::XInitialization
    >
{
    XSecController* m_pXSecController;
    css::uno::Reference<css::xml::sax::XDocumentHandler> m_xNextHandler;

    bool m_bInDigestValue;
    OUString m_aDigestValue;
    bool m_bInSignatureValue;
    OUString m_aSignatureValue;
    bool m_bInX509Certificate;
    OUString m_aX509Certificate;
    bool m_bInMdssiValue;
    OUString m_aMdssiValue;
    bool m_bInSignatureComments;
    OUString m_aSignatureComments;
    bool m_bInX509IssuerName;
    OUString m_aX509IssuerName;
    bool m_bInX509SerialNumber;
    OUString m_aX509SerialNumber;
    bool m_bInCertDigest;
    OUString m_aCertDigest;
    bool m_bInValidSignatureImage;
    OUString m_aValidSignatureImage;
    bool m_bInInvalidSignatureImage;
    OUString m_aInvalidSignatureImage;
    bool m_bInSignatureLineId;
    OUString m_aSignatureLineId;

    /// Last seen <Reference URI="...">.
    OUString m_aReferenceURI;
    /// Already called addStreamReference() for this reference.
    bool m_bReferenceUnresolved;
    XMLSignatureHelper& m_rXMLSignatureHelper;

public:
    explicit OOXMLSecParser(XMLSignatureHelper& rXMLSignatureHelper, XSecController* pXSecController);
    virtual ~OOXMLSecParser() override;

    // XDocumentHandler
    virtual void SAL_CALL startDocument() override;

    virtual void SAL_CALL endDocument() override;

    virtual void SAL_CALL startElement(const OUString& aName, const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;

    virtual void SAL_CALL endElement(const OUString& aName) override;

    virtual void SAL_CALL characters(const OUString& aChars) override;

    virtual void SAL_CALL ignorableWhitespace(const OUString& aWhitespaces) override;

    virtual void SAL_CALL processingInstruction(const OUString& aTarget, const OUString& aData) override;

    virtual void SAL_CALL setDocumentLocator(const css::uno::Reference<css::xml::sax::XLocator>& xLocator) override;

    // XInitialization
    virtual void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& rArguments) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

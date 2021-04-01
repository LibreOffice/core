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

#include <xmloff/namespacemap.hxx>

#include <stack>

class XSecController;
class XMLSignatureHelper;

/// Parses an OOXML digital signature.
class OOXMLSecParser: public cppu::WeakImplHelper
    <
    css::xml::sax::XDocumentHandler,
    css::lang::XInitialization
    >
{
public:
    class Context;
private:
    class UnknownContext;
    class ReferencedContextImpl;
    class DsX509CertificateContext;
    class DsX509SerialNumberContext;
    class DsX509IssuerNameContext;
    class DsX509IssuerSerialContext;
    class DsX509DataContext;
    class DsKeyInfoContext;
    class DsSignatureValueContext;
    class DsDigestValueContext;
    class DsDigestMethodContext;
    class DsTransformContext;
    class DsTransformsContext;
    class DsReferenceContext;
    class DsSignatureMethodContext;
    class DsSignedInfoContext;
    class XadesEncapsulatedX509CertificateContext;
    class XadesCertificateValuesContext;
    class XadesUnsignedSignaturePropertiesContext;
    class XadesUnsignedPropertiesContext;
    class XadesCertDigestContext;
    class XadesCertContext;
    class XadesSigningCertificateContext;
    class XadesSigningTimeContext;
    class XadesSignedSignaturePropertiesContext;
    class XadesSignedPropertiesContext;
    class XadesQualifyingPropertiesContext;
    class MdssiValueContext;
    class MdssiSignatureTimeContext;
    class MsodigsigSetupIDContext;
    class MsodigsigSignatureCommentsContext;
    class MsodigsigSignatureInfoV1Context;
    class DsSignaturePropertyContext;
    class DsSignaturePropertiesContext;
    class DsManifestContext;
    class DsObjectContext;
    class DsSignatureContext;
    class DsigSignaturesContext;

    std::stack<std::unique_ptr<Context>> m_ContextStack;
    std::unique_ptr<SvXMLNamespaceMap> m_pNamespaceMap;

    XSecController* m_pXSecController;
    css::uno::Reference<css::xml::sax::XDocumentHandler> m_xNextHandler;

    XMLSignatureHelper& m_rXMLSignatureHelper;

    OUString HandleIdAttr(css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs);

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

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

#ifndef INCLUDED_XMLSECURITY_SOURCE_HELPER_XSECPARSER_HXX
#define INCLUDED_XMLSECURITY_SOURCE_HELPER_XSECPARSER_HXX

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

#include <cppuhelper/implbase.hxx>

#include <xmloff/namespacemap.hxx>

#include <stack>

class XMLSignatureHelper;
class XSecController;

class XSecParser: public cppu::WeakImplHelper
<
    css::xml::sax::XDocumentHandler,
    css::lang::XInitialization
>
/****** XSecController.hxx/CLASS XSecParser ***********************************
 *
 *   NAME
 *  XSecParser -- a SAX parser that can detect security elements
 *
 *   FUNCTION
 *  The XSecParser object is connected on the SAX chain and detects
 *  security elements in the SAX event stream, then notifies
 *  the XSecController.
 *
 *   NOTES
 *  This class is used when importing a document.
 ******************************************************************************/
{
    friend class XSecController;
public:
    class Context;
private:
    class UnknownContext;
    class LoPGPOwnerContext;
    class DsPGPKeyPacketContext;
    class DsPGPKeyIDContext;
    class DsPGPDataContext;
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
    class LoSignatureLineIdContext;
    class LoSignatureLineValidImageContext;
    class LoSignatureLineInvalidImageContext;
    class LoSignatureLineContext;
    class XadesCertDigestContext;
    class XadesCertContext;
    class XadesSigningCertificateContext;
    class XadesSigningTimeContext;
    class XadesSignedSignaturePropertiesContext;
    class XadesSignedPropertiesContext;
    class XadesQualifyingPropertiesContext;
    class DcDateContext;
    class DcDescriptionContext;
    class DsSignaturePropertyContext;
    class DsSignaturePropertiesContext;
    class DsObjectContext;
    class DsSignatureContext;
    class DsigSignaturesContext;

    /*
     * the following members are used to reserve the signature information,
     * including X509IssuerName, X509SerialNumber, and X509Certificate,etc.
     */
    OUString m_ouDate;

    std::stack<std::unique_ptr<Context>> m_ContextStack;
    std::unique_ptr<SvXMLNamespaceMap> m_pNamespaceMap;

    /*
     * the XSecController collaborating with XSecParser
     */
    XSecController* m_pXSecController;

    /*
     * the next XDocumentHandler on the SAX chain
     */
    css::uno::Reference<
        css::xml::sax::XDocumentHandler > m_xNextHandler;

    XMLSignatureHelper& m_rXMLSignatureHelper;

    OUString HandleIdAttr(css::uno::Reference<css::xml::sax::XAttributeList> const& xAttrs);
    static OUString getIdAttr(const css::uno::Reference<
            css::xml::sax::XAttributeList >& xAttribs );

public:
    XSecParser(XMLSignatureHelper& rXMLSignatureHelper, XSecController* pXSecController);

    /*
     * XDocumentHandler
     */
    virtual void SAL_CALL startDocument(  ) override;

    virtual void SAL_CALL endDocument(  ) override;

    virtual void SAL_CALL startElement(
        const OUString& aName,
        const css::uno::Reference<
            css::xml::sax::XAttributeList >& xAttribs ) override;

    virtual void SAL_CALL endElement( const OUString& aName ) override;

    virtual void SAL_CALL characters( const OUString& aChars ) override;

    virtual void SAL_CALL ignorableWhitespace( const OUString& aWhitespaces ) override;

    virtual void SAL_CALL processingInstruction(
        const OUString& aTarget,
        const OUString& aData ) override;

    virtual void SAL_CALL setDocumentLocator(
        const css::uno::Reference<
            css::xml::sax::XLocator >& xLocator ) override;

    /*
     * XInitialization
     */
    virtual void SAL_CALL initialize(
        const css::uno::Sequence< css::uno::Any >& aArguments ) override;
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

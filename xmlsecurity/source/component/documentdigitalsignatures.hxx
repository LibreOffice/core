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

#ifndef INCLUDED_XMLSECURITY_SOURCE_COMPONENT_DOCUMENTDIGITALSIGNATURES_HXX
#define INCLUDED_XMLSECURITY_SOURCE_COMPONENT_DOCUMENTDIGITALSIGNATURES_HXX

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <xmlsecurity/documentsignaturehelper.hxx>

namespace com { namespace  sun { namespace star {

    namespace uno {
        class XComponentContext;
    }
}}}

class DocumentDigitalSignatures : public cppu::WeakImplHelper
<
    css::security::XDocumentDigitalSignatures,
    css::lang::XInitialization,
    css::lang::XServiceInfo
>
{
private:
    css::uno::Reference< css::uno::XComponentContext > mxCtx;
    // will be set by XInitialization. If not we assume true. false means an earlier version.
    OUString m_sODFVersion;
    //The number of arguments which were passed in XInitialization::initialize
    int m_nArgumentsCount;
    //Indicates if the document already contains a document signature
    bool m_bHasDocumentSignature;

    bool ImplViewSignatures( const css::uno::Reference< css::embed::XStorage >& rxStorage, const css::uno::Reference< css::io::XStream >& xSignStream, DocumentSignatureMode eMode, bool bReadOnly ) throw (css::uno::RuntimeException, std::exception);
    void ImplViewSignatures( const css::uno::Reference< css::embed::XStorage >& rxStorage, const css::uno::Reference< css::io::XInputStream >& xSignStream, DocumentSignatureMode eMode, bool bReadOnly ) throw (css::uno::RuntimeException, std::exception);
    css::uno::Sequence< css::security::DocumentSignatureInformation > ImplVerifySignatures( const css::uno::Reference< css::embed::XStorage >& rxStorage, const ::com::sun::star::uno::Reference< css::io::XInputStream >& xSignStream, DocumentSignatureMode eMode ) throw (css::uno::RuntimeException);

public:
    explicit DocumentDigitalSignatures( const css::uno::Reference< css::uno::XComponentContext>& rxCtx );

    // for service registration...
    static OUString GetImplementationName() throw (css::uno::RuntimeException);
    static css::uno::Sequence < OUString > GetSupportedServiceNames() throw (css::uno::RuntimeException);

    //XInitialization
    void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

    // XDocumentDigitalSignatures
    sal_Bool SAL_CALL signDocumentContent( const css::uno::Reference< css::embed::XStorage >& xStorage, const css::uno::Reference< css::io::XStream >& xSignStream ) throw (css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< css::security::DocumentSignatureInformation > SAL_CALL verifyDocumentContentSignatures( const css::uno::Reference< css::embed::XStorage >& xStorage, const css::uno::Reference< css::io::XInputStream >& xSignInStream ) throw (css::uno::RuntimeException, std::exception) override;
    void SAL_CALL showDocumentContentSignatures( const css::uno::Reference< css::embed::XStorage >& xStorage, const css::uno::Reference< css::io::XInputStream >& xSignInStream ) throw (css::uno::RuntimeException, std::exception) override;
    OUString SAL_CALL getDocumentContentSignatureDefaultStreamName(  ) throw (css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL signScriptingContent( const css::uno::Reference< css::embed::XStorage >& xStorage, const css::uno::Reference< css::io::XStream >& xSignStream ) throw (css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< css::security::DocumentSignatureInformation > SAL_CALL verifyScriptingContentSignatures( const css::uno::Reference< css::embed::XStorage >& xStorage, const css::uno::Reference< css::io::XInputStream >& xSignInStream ) throw (css::uno::RuntimeException, std::exception) override;
    void SAL_CALL showScriptingContentSignatures( const css::uno::Reference< css::embed::XStorage >& xStorage, const css::uno::Reference< css::io::XInputStream >& xSignInStream ) throw (css::uno::RuntimeException, std::exception) override;
    OUString SAL_CALL getScriptingContentSignatureDefaultStreamName(  ) throw (css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL signPackage( const css::uno::Reference< css::embed::XStorage >& Storage, const css::uno::Reference< css::io::XStream >& xSignStream ) throw (css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< css::security::DocumentSignatureInformation > SAL_CALL verifyPackageSignatures( const css::uno::Reference< css::embed::XStorage >& Storage, const css::uno::Reference< css::io::XInputStream >& xSignInStream ) throw (css::uno::RuntimeException, std::exception) override;
    void SAL_CALL showPackageSignatures( const css::uno::Reference< css::embed::XStorage >& xStorage, const css::uno::Reference< css::io::XInputStream >& xSignInStream ) throw (css::uno::RuntimeException, std::exception) override;
    OUString SAL_CALL getPackageSignatureDefaultStreamName(  ) throw (css::uno::RuntimeException, std::exception) override;
    void SAL_CALL showCertificate( const css::uno::Reference< css::security::XCertificate >& Certificate ) throw (css::uno::RuntimeException, std::exception) override;
    void SAL_CALL manageTrustedSources(  ) throw (css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL isAuthorTrusted( const css::uno::Reference< css::security::XCertificate >& Author ) throw (css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL isLocationTrusted( const OUString& Location ) throw (css::uno::RuntimeException, std::exception) override;
    void SAL_CALL addAuthorToTrustedSources( const css::uno::Reference< css::security::XCertificate >& Author ) throw (css::uno::RuntimeException, std::exception) override;
    void SAL_CALL addLocationToTrustedSources( const OUString& Location ) throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Reference< css::security::XCertificate > SAL_CALL chooseCertificate( ) throw (css::uno::RuntimeException, std::exception) override;
};

css::uno::Reference< css::uno::XInterface > SAL_CALL DocumentDigitalSignatures_CreateInstance(
    const css::uno::Reference< css::uno::XComponentContext >& rCtx) throw ( css::uno::Exception );

#endif // INCLUDED_XMLSECURITY_SOURCE_COMPONENT_DOCUMENTDIGITALSIGNATURES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

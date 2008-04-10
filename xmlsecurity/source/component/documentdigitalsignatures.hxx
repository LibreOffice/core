/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: documentdigitalsignatures.hxx,v $
 * $Revision: 1.8 $
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

#ifndef _XMLSECURITY_DOCUMENTDIGITALSIGNATURES_HXX
#define _XMLSECURITY_DOCUMENTDIGITALSIGNATURES_HXX

#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <xmlsecurity/documentsignaturehelper.hxx>


class DocumentDigitalSignatures : public cppu::WeakImplHelper1
<
    com::sun::star::security::XDocumentDigitalSignatures
>
{
private:
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > mxMSF;

    sal_Bool ImplViewSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xSignStream, DocumentSignatureMode eMode, bool bReadOnly ) throw (::com::sun::star::uno::RuntimeException);
    sal_Bool ImplViewSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xSignStream, DocumentSignatureMode eMode, bool bReadOnly ) throw (::com::sun::star::uno::RuntimeException);
    com::sun::star::uno::Sequence< ::com::sun::star::security::DocumentSignatureInformation > ImplVerifySignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xSignStream, DocumentSignatureMode eMode ) throw (::com::sun::star::uno::RuntimeException);

public:
    DocumentDigitalSignatures( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory> rxMSF );

    // for service registration...
    static ::rtl::OUString GetImplementationName() throw (com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Sequence < ::rtl::OUString > GetSupportedServiceNames() throw (com::sun::star::uno::RuntimeException);

    // XDocumentDigitalSignatures
    ::sal_Bool SAL_CALL signDocumentContent( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xSignStream ) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::com::sun::star::security::DocumentSignatureInformation > SAL_CALL verifyDocumentContentSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xSignInStream ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL showDocumentContentSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xSignInStream ) throw (::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getDocumentContentSignatureDefaultStreamName(  ) throw (::com::sun::star::uno::RuntimeException);
    ::sal_Bool SAL_CALL signScriptingContent( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xSignStream ) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::com::sun::star::security::DocumentSignatureInformation > SAL_CALL verifyScriptingContentSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xSignInStream ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL showScriptingContentSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xSignInStream ) throw (::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getScriptingContentSignatureDefaultStreamName(  ) throw (::com::sun::star::uno::RuntimeException);
    ::sal_Bool SAL_CALL signPackage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& Storage, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xSignStream ) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::com::sun::star::security::DocumentSignatureInformation > SAL_CALL verifyPackageSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& Storage, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xSignInStream ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL showPackageSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xSignInStream ) throw (::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getPackageSignatureDefaultStreamName(  ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL showCertificate( const ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate >& Certificate ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL manageTrustedSources(  ) throw (::com::sun::star::uno::RuntimeException);
    ::sal_Bool SAL_CALL isAuthorTrusted( const ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate >& Author ) throw (::com::sun::star::uno::RuntimeException);
    ::sal_Bool SAL_CALL isLocationTrusted( const ::rtl::OUString& Location ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL addAuthorToTrustedSources( const ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate >& Author ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL addLocationToTrustedSources( const ::rtl::OUString& Location ) throw (::com::sun::star::uno::RuntimeException);

};

com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL DocumentDigitalSignatures_CreateInstance(
    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rSMgr) throw ( com::sun::star::uno::Exception );

#endif // _XMLSECURITY_DOCUMENTDIGITALSIGNATURES_HXX

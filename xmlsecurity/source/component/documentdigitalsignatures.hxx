/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: documentdigitalsignatures.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:09:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    com::sun::star::uno::Sequence< ::com::sun::star::security::DocumentSignaturesInformation > ImplVerifySignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xSignStream, DocumentSignatureMode eMode ) throw (::com::sun::star::uno::RuntimeException);

public:
    DocumentDigitalSignatures( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory> rxMSF );

    // for service registration...
    static ::rtl::OUString GetImplementationName() throw (com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Sequence < ::rtl::OUString > GetSupportedServiceNames() throw (com::sun::star::uno::RuntimeException);

    // XDocumentDigitalSignatures
    ::sal_Bool SAL_CALL signDocumentContent( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xSignStream ) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::com::sun::star::security::DocumentSignaturesInformation > SAL_CALL verifyDocumentContentSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xSignInStream ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL showDocumentContentSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xSignInStream ) throw (::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getDocumentContentSignatureDefaultStreamName(  ) throw (::com::sun::star::uno::RuntimeException);
    ::sal_Bool SAL_CALL signScriptingContent( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xSignStream ) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::com::sun::star::security::DocumentSignaturesInformation > SAL_CALL verifyScriptingContentSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xSignInStream ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL showScriptingContentSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xSignInStream ) throw (::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getScriptingContentSignatureDefaultStreamName(  ) throw (::com::sun::star::uno::RuntimeException);
    ::sal_Bool SAL_CALL signPackage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& Storage, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XStream >& xSignStream ) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::com::sun::star::security::DocumentSignaturesInformation > SAL_CALL verifyPackageSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& Storage, const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xSignInStream ) throw (::com::sun::star::uno::RuntimeException);
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

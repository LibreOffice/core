/*************************************************************************
 *
 *  $RCSfile: documentdigitalsignatures.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-16 15:58:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLSECURITY_DOCUMENTDIGITALSIGNATURES_HXX
#define _XMLSECURITY_DOCUMENTDIGITALSIGNATURES_HXX

#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#include <xmlsecurity/documentsignaturehelper.hxx>


class DocumentDigitalSignatures : public cppu::WeakImplHelper1
<
    com::sun::star::security::XDocumentDigitalSignatures
>
{
private:
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > mxMSF;

    sal_Bool ImplViewSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage, DocumentSignatureMode eMode, bool bReadOnly ) throw (::com::sun::star::uno::RuntimeException);
    com::sun::star::uno::Sequence< ::com::sun::star::security::DocumentSignaturesInformation > ImplVerifySignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage, DocumentSignatureMode eMode ) throw (::com::sun::star::uno::RuntimeException);

public:
    DocumentDigitalSignatures( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory> rxMSF );

    // for service registration...
    static ::rtl::OUString GetImplementationName() throw (com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Sequence < ::rtl::OUString > GetSupportedServiceNames() throw (com::sun::star::uno::RuntimeException);

    // XDocumentDigitalSignatures
    sal_Bool SAL_CALL SignDocumentContent( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage ) throw (::com::sun::star::uno::RuntimeException);
    com::sun::star::uno::Sequence< ::com::sun::star::security::DocumentSignaturesInformation > SAL_CALL VerifyDocumentContentSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL ShowDocumentContentSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& Storage ) throw (::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL SignScriptingContent( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage ) throw (::com::sun::star::uno::RuntimeException);
    com::sun::star::uno::Sequence< ::com::sun::star::security::DocumentSignaturesInformation > SAL_CALL VerifyScriptingContentSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL ShowScriptingContentSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& Storage ) throw (::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL SignPackage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage ) throw (::com::sun::star::uno::RuntimeException);
    com::sun::star::uno::Sequence< ::com::sun::star::security::DocumentSignaturesInformation > SAL_CALL VerifyPackageSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL ShowPackageSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& Storage ) throw (::com::sun::star::uno::RuntimeException);

    void SAL_CALL manageTrustedSources(  ) throw (::com::sun::star::uno::RuntimeException);
    ::sal_Bool SAL_CALL isAuthorTrusted( const ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate >& Author ) throw (::com::sun::star::uno::RuntimeException);
    ::sal_Bool SAL_CALL isLocationTrusted( const ::rtl::OUString& Location ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL addAuthorToTrustedSources( const ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate >& Author ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL addLocationToTrustedSources( const ::rtl::OUString& Location ) throw (::com::sun::star::uno::RuntimeException);

};

com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL DocumentDigitalSignatures_CreateInstance(
    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rSMgr) throw ( com::sun::star::uno::Exception );

#endif // _XMLSECURITY_DOCUMENTDIGITALSIGNATURES_HXX

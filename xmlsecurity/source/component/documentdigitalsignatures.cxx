/*************************************************************************
 *
 *  $RCSfile: documentdigitalsignatures.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: gt $ $Date: 2004-07-23 09:44:07 $
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


#include <documentdigitalsignatures.hxx>
#include <xmlsecurity/digitalsignaturesdialog.hxx>
#include <xmlsecurity/macrosecurity.hxx>
#include <xmlsecurity/baseencoding.hxx>
#include <../dialogs/resourcemanager.hxx>

#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef INCLUDED_SVTOOLS_SECURITYOPTIONS_HXX
#include <svtools/securityoptions.hxx>
#endif



using namespace ::com::sun::star;
using namespace ::com::sun::star;


DocumentDigitalSignatures::DocumentDigitalSignatures( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory> rxMSF )
{
    mxMSF = rxMSF;
}

sal_Bool DocumentDigitalSignatures::SignDocumentContent( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage ) throw (::com::sun::star::uno::RuntimeException)
{
    return ImplViewSignatures( rxStorage, SignatureModeDocumentContent, false );
}

::com::sun::star::uno::Sequence< ::com::sun::star::security::DocumentSignaturesInformation > DocumentDigitalSignatures::VerifyDocumentContentSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage ) throw (::com::sun::star::uno::RuntimeException)
{
    return ImplVerifySignatures( rxStorage, SignatureModeDocumentContent );
}

void DocumentDigitalSignatures::ShowDocumentContentSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage ) throw (::com::sun::star::uno::RuntimeException)
{
    ImplViewSignatures( rxStorage, SignatureModeDocumentContent, true );
}

sal_Bool DocumentDigitalSignatures::SignScriptingContent( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage ) throw (::com::sun::star::uno::RuntimeException)
{
    return ImplViewSignatures( rxStorage, SignatureModeMacros, false );
}

::com::sun::star::uno::Sequence< ::com::sun::star::security::DocumentSignaturesInformation > DocumentDigitalSignatures::VerifyScriptingContentSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage ) throw (::com::sun::star::uno::RuntimeException)
{
    return ImplVerifySignatures( rxStorage, SignatureModeMacros );
}

void DocumentDigitalSignatures::ShowScriptingContentSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage ) throw (::com::sun::star::uno::RuntimeException)
{
    ImplViewSignatures( rxStorage, SignatureModeMacros, true );
}

sal_Bool DocumentDigitalSignatures::SignPackage( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage ) throw (::com::sun::star::uno::RuntimeException)
{
    return ImplViewSignatures( rxStorage, SignatureModePackage, false );
}

::com::sun::star::uno::Sequence< ::com::sun::star::security::DocumentSignaturesInformation > DocumentDigitalSignatures::VerifyPackageSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage ) throw (::com::sun::star::uno::RuntimeException)
{
    return ImplVerifySignatures( rxStorage, SignatureModePackage );
}

void DocumentDigitalSignatures::ShowPackageSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage ) throw (::com::sun::star::uno::RuntimeException)
{
    ImplViewSignatures( rxStorage, SignatureModePackage, true );
}

sal_Bool DocumentDigitalSignatures::ImplViewSignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage, DocumentSignatureMode eMode, bool bReadOnly ) throw (::com::sun::star::uno::RuntimeException)
{
    DigitalSignaturesDialog aSignaturesDialog( NULL, mxMSF, eMode, bReadOnly );

    bool bInit = aSignaturesDialog.Init( rtl::OUString() );
    DBG_ASSERT( bInit, "Error initializing security context!" );
    if ( bInit )
    {
        aSignaturesDialog.SetStorage( rxStorage );
        aSignaturesDialog.Execute();
    }

    return aSignaturesDialog.SignaturesChanged();
}

com::sun::star::uno::Sequence< ::com::sun::star::security::DocumentSignaturesInformation > DocumentDigitalSignatures::ImplVerifySignatures( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rxStorage, DocumentSignatureMode eMode ) throw (::com::sun::star::uno::RuntimeException)
{
    XMLSignatureHelper aSignatureHelper( mxMSF );
    aSignatureHelper.Init( rtl::OUString() );
    aSignatureHelper.SetStorage( rxStorage );

    aSignatureHelper.StartMission();

    SignatureStreamHelper aStreamHelper = DocumentSignatureHelper::OpenSignatureStream( rxStorage, embed::ElementModes::READ, eMode );
    if ( aStreamHelper.xSignatureStream.is() )
    {
        uno::Reference< io::XInputStream > xInputStream( aStreamHelper.xSignatureStream, uno::UNO_QUERY );
        bool bVerifyOK = aSignatureHelper.ReadAndVerifySignature( xInputStream );

        if ( bVerifyOK )
        {
            // SignatureInformations aInformations = aSignatureHelper.GetSignatureInformations();
            // ...
        }
    }

    aStreamHelper.Clear();

    aSignatureHelper.EndMission();

    uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > xSecEnv = aSignatureHelper.GetSecurityEnvironment();

    SignatureInformations aSignInfos = aSignatureHelper.GetSignatureInformations();
    int nInfos = aSignInfos.size();
    ::com::sun::star::uno::Sequence< ::com::sun::star::security::DocumentSignaturesInformation > aInfos(nInfos);

    if ( nInfos )
    {
        std::vector< rtl::OUString > aElementsToBeVerified = DocumentSignatureHelper::CreateElementList( rxStorage, ::rtl::OUString(), eMode );
        for( int n = 0; n < nInfos; ++n )
        {
            const SignatureInformation& rInfo = aSignInfos[n];
            aInfos[n].Signer = xSecEnv->getCertificate( rInfo.ouX509IssuerName, numericStringToBigInteger( rInfo.ouX509SerialNumber ) );
            if ( !aInfos[n].Signer.is() )
                aInfos[n].Signer = xSecEnv->createCertificateFromAscii( rInfo.ouX509Certificate ) ;

            aInfos[n].SignatureDate = String( rInfo.ouDate ).ToInt32();
            aInfos[n].SignatureTime = String( rInfo.ouTime ).ToInt32();

            aInfos[n].SignatureIsValid = ( rInfo.nStatus == STATUS_VERIFY_SUCCEED );

            if ( aInfos[n].SignatureIsValid )
            {
                // Can only be valid if ALL streams are signed, which means real stream count == signed stream count
                int nRealCount = 0;
                for ( int i = rInfo.vSignatureReferenceInfors.size(); i; )
                {
                    const SignatureReferenceInformation& rInf = rInfo.vSignatureReferenceInfors[--i];
                    // There is also an extra entry of type TYPE_SAMEDOCUMENT_REFERENCE because of signature date.
                    if ( ( rInf.nType == TYPE_BINARYSTREAM_REFERENCE ) || ( rInf.nType == TYPE_XMLSTREAM_REFERENCE ) )
                        nRealCount++;
                }
                aInfos[n].SignatureIsValid = ( aElementsToBeVerified.size() == nRealCount );
            }

        }
    }
    return aInfos;

}

void DocumentDigitalSignatures::manageTrustedSources(  ) throw (::com::sun::star::uno::RuntimeException)
{
    cssu::Reference< css::lang::XMultiServiceFactory >          xMSF;
    cssu::Reference< dcss::xml::crypto::XSecurityEnvironment >  xSecurityEnvironment;
    MacroSecurity   aDlg( NULL, xMSF, xSecurityEnvironment );
    aDlg.Execute();
}

::sal_Bool DocumentDigitalSignatures::isAuthorTrusted( const ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate >& Author ) throw (::com::sun::star::uno::RuntimeException)
{
    return sal_False;
}

::sal_Bool DocumentDigitalSignatures::isLocationTrusted( const ::rtl::OUString& Location ) throw (::com::sun::star::uno::RuntimeException)
{
    return sal_False;
}

void DocumentDigitalSignatures::addAuthorToTrustedSources( const ::com::sun::star::uno::Reference< ::com::sun::star::security::XCertificate >& Author ) throw (::com::sun::star::uno::RuntimeException)
{
    SvtSecurityOptions aSecOpts;

    SvtSecurityOptions::Certificate aNewCert;
    aNewCert[ 0 ] = XmlSec::GetContentPart( Author->getIssuerName(), String::CreateFromAscii( "CN" ) );
    aNewCert[ 1 ] = XmlSec::GetHexString( Author->getIssuerUniqueID(), " " );
    aNewCert[ 2 ] = baseEncode( Author->getEncoded(), BASE64 );

    uno::Sequence< SvtSecurityOptions::Certificate > aTrustedAuthors = aSecOpts.GetTrustedAuthors();
    sal_Int32 nCnt = aTrustedAuthors.getLength();
    aTrustedAuthors.realloc( nCnt + 1 );
    aTrustedAuthors[ nCnt ] = aNewCert;

    aSecOpts.SetTrustedAuthors( aTrustedAuthors );
}

void DocumentDigitalSignatures::addLocationToTrustedSources( const ::rtl::OUString& Location ) throw (::com::sun::star::uno::RuntimeException)
{
    SvtSecurityOptions aSecOpt;

    ::com::sun::star::uno::Sequence< ::rtl::OUString > aSecURLs = aSecOpt.GetSecureURLs();
    sal_Int32 nCnt = aSecURLs.getLength();
    aSecURLs.realloc( nCnt + 1 );
    aSecURLs[ nCnt ] = Location;

    aSecOpt.SetSecureURLs( aSecURLs );
}



rtl::OUString DocumentDigitalSignatures::GetImplementationName() throw (uno::RuntimeException)
{
    return rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.security.DocumentDigitalSignatures" ) );
}

uno::Sequence< rtl::OUString > DocumentDigitalSignatures::GetSupportedServiceNames() throw (cssu::RuntimeException)
{
    uno::Sequence < rtl::OUString > aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] =  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.security.DocumentDigitalSignatures" ) );
    return aRet;
}


com::sun::star::uno::Reference< com::sun::star::uno::XInterface > DocumentDigitalSignatures_CreateInstance(
    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rSMgr) throw ( com::sun::star::uno::Exception )
{
    return (cppu::OWeakObject*) new DocumentDigitalSignatures( rSMgr );
}

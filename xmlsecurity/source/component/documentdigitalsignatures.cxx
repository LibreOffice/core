/*************************************************************************
 *
 *  $RCSfile: documentdigitalsignatures.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-18 14:33:25 $
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
#include <xmlsecurity/certificateviewer.hxx>
#include <xmlsecurity/macrosecurity.hxx>
#include <xmlsecurity/baseencoding.hxx>
#include <xmlsecurity/biginteger.hxx>

#include <../dialogs/resourcemanager.hxx>

#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SECURITYOPTIONS_HXX
#include <svtools/securityoptions.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

DocumentDigitalSignatures::DocumentDigitalSignatures( const Reference< com::sun::star::lang::XMultiServiceFactory> rxMSF )
{
    mxMSF = rxMSF;
}

sal_Bool DocumentDigitalSignatures::SignDocumentContent( const Reference< ::com::sun::star::embed::XStorage >& rxStorage ) throw (RuntimeException)
{
    return ImplViewSignatures( rxStorage, SignatureModeDocumentContent, false );
}

Sequence< ::com::sun::star::security::DocumentSignaturesInformation > DocumentDigitalSignatures::VerifyDocumentContentSignatures( const Reference< ::com::sun::star::embed::XStorage >& rxStorage ) throw (RuntimeException)
{
    return ImplVerifySignatures( rxStorage, SignatureModeDocumentContent );
}

void DocumentDigitalSignatures::ShowDocumentContentSignatures( const Reference< ::com::sun::star::embed::XStorage >& rxStorage ) throw (RuntimeException)
{
    ImplViewSignatures( rxStorage, SignatureModeDocumentContent, true );
}

sal_Bool DocumentDigitalSignatures::SignScriptingContent( const Reference< ::com::sun::star::embed::XStorage >& rxStorage ) throw (RuntimeException)
{
    return ImplViewSignatures( rxStorage, SignatureModeMacros, false );
}

Sequence< ::com::sun::star::security::DocumentSignaturesInformation > DocumentDigitalSignatures::VerifyScriptingContentSignatures( const Reference< ::com::sun::star::embed::XStorage >& rxStorage ) throw (RuntimeException)
{
    return ImplVerifySignatures( rxStorage, SignatureModeMacros );
}

void DocumentDigitalSignatures::ShowScriptingContentSignatures( const Reference< ::com::sun::star::embed::XStorage >& rxStorage ) throw (RuntimeException)
{
    ImplViewSignatures( rxStorage, SignatureModeMacros, true );
}

sal_Bool DocumentDigitalSignatures::SignPackage( const Reference< ::com::sun::star::embed::XStorage >& rxStorage ) throw (RuntimeException)
{
    return ImplViewSignatures( rxStorage, SignatureModePackage, false );
}

Sequence< ::com::sun::star::security::DocumentSignaturesInformation > DocumentDigitalSignatures::VerifyPackageSignatures( const Reference< ::com::sun::star::embed::XStorage >& rxStorage ) throw (RuntimeException)
{
    return ImplVerifySignatures( rxStorage, SignatureModePackage );
}

void DocumentDigitalSignatures::ShowPackageSignatures( const Reference< ::com::sun::star::embed::XStorage >& rxStorage ) throw (RuntimeException)
{
    ImplViewSignatures( rxStorage, SignatureModePackage, true );
}

sal_Bool DocumentDigitalSignatures::ImplViewSignatures( const Reference< ::com::sun::star::embed::XStorage >& rxStorage, DocumentSignatureMode eMode, bool bReadOnly ) throw (RuntimeException)
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

Sequence< ::com::sun::star::security::DocumentSignaturesInformation > DocumentDigitalSignatures::ImplVerifySignatures( const Reference< ::com::sun::star::embed::XStorage >& rxStorage, DocumentSignatureMode eMode ) throw (RuntimeException)
{
    XMLSignatureHelper aSignatureHelper( mxMSF );
    aSignatureHelper.Init( rtl::OUString() );
    aSignatureHelper.SetStorage( rxStorage );

    aSignatureHelper.StartMission();

    SignatureStreamHelper aStreamHelper = DocumentSignatureHelper::OpenSignatureStream( rxStorage, embed::ElementModes::READ, eMode );
    if ( aStreamHelper.xSignatureStream.is() )
    {
        Reference< io::XInputStream > xInputStream( aStreamHelper.xSignatureStream, UNO_QUERY );
        aSignatureHelper.ReadAndVerifySignature( xInputStream );
    }

    aSignatureHelper.EndMission();

    aStreamHelper.Clear();

    Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > xSecEnv = aSignatureHelper.GetSecurityEnvironment();

    SignatureInformations aSignInfos = aSignatureHelper.GetSignatureInformations();
    int nInfos = aSignInfos.size();
    Sequence< ::com::sun::star::security::DocumentSignaturesInformation > aInfos(nInfos);

    if ( nInfos )
    {
        std::vector< rtl::OUString > aElementsToBeVerified = DocumentSignatureHelper::CreateElementList( rxStorage, ::rtl::OUString(), eMode );
        for( int n = 0; n < nInfos; ++n )
        {
            const SignatureInformation& rInfo = aSignInfos[n];
            aInfos[n].Signer = xSecEnv->getCertificate( rInfo.ouX509IssuerName, numericStringToBigInteger( rInfo.ouX509SerialNumber ) );
            if ( !aInfos[n].Signer.is() )
                aInfos[n].Signer = xSecEnv->createCertificateFromAscii( rInfo.ouX509Certificate ) ;

            // --> PB 2004-12-14 #i38744# time support again
            Date aDate( rInfo.stDateTime.Day, rInfo.stDateTime.Month, rInfo.stDateTime.Year );
            Time aTime( rInfo.stDateTime.Hours, rInfo.stDateTime.Minutes,
                        rInfo.stDateTime.Seconds, rInfo.stDateTime.HundredthSeconds );
            aInfos[n].SignatureDate = aDate.GetDate();
            aInfos[n].SignatureTime = aTime.GetTime();
            // <--

            DBG_ASSERT( rInfo.nStatus != ::com::sun::star::xml::crypto::SecurityOperationStatus_STATUS_UNKNOWN, "Signature not processed!" );

            aInfos[n].SignatureIsValid = ( rInfo.nStatus == ::com::sun::star::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED );

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

void DocumentDigitalSignatures::manageTrustedSources(  ) throw (RuntimeException)
{
    XMLSignatureHelper aSignatureHelper( mxMSF );
    aSignatureHelper.Init( rtl::OUString() );
    MacroSecurity aDlg( NULL, aSignatureHelper.GetSecurityEnvironment() );
    aDlg.Execute();
}

void DocumentDigitalSignatures::ShowCertificate( const Reference< ::com::sun::star::security::XCertificate >& _Certificate ) throw (RuntimeException)
{
    XMLSignatureHelper aSignatureHelper( mxMSF );
    aSignatureHelper.Init( rtl::OUString() );
    CertificateViewer aViewer( NULL, aSignatureHelper.GetSecurityEnvironment(), _Certificate );
    aViewer.Execute();

}

::sal_Bool DocumentDigitalSignatures::isAuthorTrusted( const Reference< ::com::sun::star::security::XCertificate >& Author ) throw (RuntimeException)
{
    sal_Bool bFound = sal_False;
    ::rtl::OUString sSerialNum = bigIntegerToNumericString( Author->getSerialNumber() );

    Sequence< SvtSecurityOptions::Certificate > aTrustedAuthors = SvtSecurityOptions().GetTrustedAuthors();
    sal_Int32 nCnt = aTrustedAuthors.getLength();
    const SvtSecurityOptions::Certificate* pAuthors = aTrustedAuthors.getConstArray();
    const SvtSecurityOptions::Certificate* pAuthorsEnd = pAuthors + aTrustedAuthors.getLength();
    for ( ; pAuthors != pAuthorsEnd; ++pAuthors )
    {
        SvtSecurityOptions::Certificate aAuthor = *pAuthors;
        if ( ( aAuthor[0] == Author->getIssuerName() ) && ( aAuthor[1] == sSerialNum ) )
        {
            bFound = sal_True;
            break;
        }
    }

    return bFound;
}

::sal_Bool DocumentDigitalSignatures::isLocationTrusted( const ::rtl::OUString& Location ) throw (RuntimeException)
{
    sal_Bool bFound = sal_False;
    INetURLObject aLocObj( Location );

    Sequence< ::rtl::OUString > aSecURLs = SvtSecurityOptions().GetSecureURLs();
    sal_Int32 nCnt = aSecURLs.getLength();
    const ::rtl::OUString* pSecURLs = aSecURLs.getConstArray();
    const ::rtl::OUString* pSecURLsEnd = pSecURLs + aSecURLs.getLength();
    for ( ; pSecURLs != pSecURLsEnd; ++pSecURLs )
    {
        INetURLObject aSecURL( *pSecURLs );
        if ( aSecURL == aLocObj )
        {
            bFound = sal_True;
            break;
        }
    }

    return bFound;
}

void DocumentDigitalSignatures::addAuthorToTrustedSources( const Reference< ::com::sun::star::security::XCertificate >& Author ) throw (RuntimeException)
{
    SvtSecurityOptions aSecOpts;

    SvtSecurityOptions::Certificate aNewCert( 3 );
    aNewCert[ 0 ] = Author->getIssuerName();
    aNewCert[ 1 ] = bigIntegerToNumericString( Author->getSerialNumber() );
    aNewCert[ 2 ] = baseEncode( Author->getEncoded(), BASE64 );

    Sequence< SvtSecurityOptions::Certificate > aTrustedAuthors = aSecOpts.GetTrustedAuthors();
    sal_Int32 nCnt = aTrustedAuthors.getLength();
    aTrustedAuthors.realloc( nCnt + 1 );
    aTrustedAuthors[ nCnt ] = aNewCert;

    aSecOpts.SetTrustedAuthors( aTrustedAuthors );
}

void DocumentDigitalSignatures::addLocationToTrustedSources( const ::rtl::OUString& Location ) throw (RuntimeException)
{
    SvtSecurityOptions aSecOpt;

    Sequence< ::rtl::OUString > aSecURLs = aSecOpt.GetSecureURLs();
    sal_Int32 nCnt = aSecURLs.getLength();
    aSecURLs.realloc( nCnt + 1 );
    aSecURLs[ nCnt ] = Location;

    aSecOpt.SetSecureURLs( aSecURLs );
}



rtl::OUString DocumentDigitalSignatures::GetImplementationName() throw (RuntimeException)
{
    return rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.security.DocumentDigitalSignatures" ) );
}

Sequence< rtl::OUString > DocumentDigitalSignatures::GetSupportedServiceNames() throw (cssu::RuntimeException)
{
    Sequence < rtl::OUString > aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] =  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.security.DocumentDigitalSignatures" ) );
    return aRet;
}


Reference< XInterface > DocumentDigitalSignatures_CreateInstance(
    const Reference< com::sun::star::lang::XMultiServiceFactory >& rSMgr) throw ( Exception )
{
    return (cppu::OWeakObject*) new DocumentDigitalSignatures( rSMgr );
}


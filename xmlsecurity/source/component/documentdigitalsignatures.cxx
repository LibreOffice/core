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

#include <documentdigitalsignatures.hxx>
#include <xmlsecurity/digitalsignaturesdialog.hxx>
#include <xmlsecurity/certificatechooser.hxx>
#include <xmlsecurity/certificateviewer.hxx>
#include <xmlsecurity/macrosecurity.hxx>
#include <xmlsecurity/biginteger.hxx>
#include <xmlsecurity/global.hrc>

#include <sax/tools/converter.hxx>

#include <../dialogs/resourcemanager.hxx>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/ucb/Command.hpp>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/securityoptions.hxx>
#include <com/sun/star/security/CertificateValidity.hpp>
#include <com/sun/star/security/SerialNumberAdapter.hpp>
#include <unotools/ucbhelper.hxx>
#include <comphelper/componentcontext.hxx>
#include "comphelper/documentconstants.hxx"

#include "com/sun/star/lang/IllegalArgumentException.hpp"

#include <stdio.h>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
namespace css = ::com::sun::star;

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )

DocumentDigitalSignatures::DocumentDigitalSignatures( const Reference< XComponentContext >& rxCtx ):
    mxCtx(rxCtx),
    m_sODFVersion(ODFVER_012_TEXT),
    m_nArgumentsCount(0),
    m_bHasDocumentSignature(false)
{
}

void DocumentDigitalSignatures::initialize( const Sequence< Any >& aArguments)
        throw (css::uno::Exception, css::uno::RuntimeException)
{
    if (aArguments.getLength() > 2)
        throw css::lang::IllegalArgumentException(
        OUSTR("DocumentDigitalSignatures::initialize requires zero, one, or two arguments"),
        Reference<XInterface>(static_cast<XInitialization*>(this), UNO_QUERY), 0);

    m_nArgumentsCount = aArguments.getLength();

    if (aArguments.getLength() > 0)
    {
        if (!(aArguments[0] >>= m_sODFVersion))
            throw css::lang::IllegalArgumentException(
                OUSTR("DocumentDigitalSignatures::initialize: the first arguments must be a string"),
                Reference<XInterface>(static_cast<XInitialization*>(this), UNO_QUERY), 0);

        if (aArguments.getLength() == 2
            && !(aArguments[1] >>= m_bHasDocumentSignature))
            throw css::lang::IllegalArgumentException(
                OUSTR("DocumentDigitalSignatures::initialize: the second arguments must be a bool"),
                Reference<XInterface>(static_cast<XInitialization*>(this), UNO_QUERY), 1);

        //the Version is supported as of ODF1.2, so for and 1.1 document or older we will receive the
        //an empty string. In this case we set it to ODFVER_010_TEXT. Then we can later check easily
        //if initialize was called. Only then m_sODFVersion.getLength() is greater than 0
        if (m_sODFVersion.isEmpty())
            m_sODFVersion = ODFVER_010_TEXT;
    }
}

sal_Bool DocumentDigitalSignatures::signDocumentContent(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XStream >& xSignStream)
        throw (RuntimeException)
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(), "DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    return ImplViewSignatures( rxStorage, xSignStream, SignatureModeDocumentContent, false );
}

Sequence< css::security::DocumentSignatureInformation >
DocumentDigitalSignatures::verifyDocumentContentSignatures(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XInputStream >& xSignInStream ) throw (RuntimeException)
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),"DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    return ImplVerifySignatures( rxStorage, xSignInStream, SignatureModeDocumentContent );
}

void DocumentDigitalSignatures::showDocumentContentSignatures(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XInputStream >& xSignInStream ) throw (RuntimeException)
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),"DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    ImplViewSignatures( rxStorage, xSignInStream, SignatureModeDocumentContent, true );
}

::rtl::OUString DocumentDigitalSignatures::getDocumentContentSignatureDefaultStreamName()
    throw (css::uno::RuntimeException)
{
    return DocumentSignatureHelper::GetDocumentContentSignatureDefaultStreamName();
}

sal_Bool DocumentDigitalSignatures::signScriptingContent(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XStream >& xSignStream ) throw (RuntimeException)
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),"DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    OSL_ENSURE(m_nArgumentsCount == 2, "DocumentDigitalSignatures: Service was not initialized properly");
    return ImplViewSignatures( rxStorage, xSignStream, SignatureModeMacros, false );
}

Sequence< css::security::DocumentSignatureInformation >
DocumentDigitalSignatures::verifyScriptingContentSignatures(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XInputStream >& xSignInStream ) throw (RuntimeException)
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),"DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    return ImplVerifySignatures( rxStorage, xSignInStream, SignatureModeMacros );
}

void DocumentDigitalSignatures::showScriptingContentSignatures(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XInputStream >& xSignInStream ) throw (RuntimeException)
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),"DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    ImplViewSignatures( rxStorage, xSignInStream, SignatureModeMacros, true );
}

::rtl::OUString DocumentDigitalSignatures::getScriptingContentSignatureDefaultStreamName()
    throw (css::uno::RuntimeException)
{
    return DocumentSignatureHelper::GetScriptingContentSignatureDefaultStreamName();
}


sal_Bool DocumentDigitalSignatures::signPackage(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XStream >& xSignStream  ) throw (RuntimeException)
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),"DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    return ImplViewSignatures( rxStorage, xSignStream, SignatureModePackage, false );
}

Sequence< css::security::DocumentSignatureInformation >
DocumentDigitalSignatures::verifyPackageSignatures(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XInputStream >& xSignInStream ) throw (RuntimeException)
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),"DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    return ImplVerifySignatures( rxStorage, xSignInStream, SignatureModePackage );
}

void DocumentDigitalSignatures::showPackageSignatures(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XInputStream >& xSignInStream ) throw (RuntimeException)
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),"DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    ImplViewSignatures( rxStorage, xSignInStream, SignatureModePackage, true );
}

::rtl::OUString DocumentDigitalSignatures::getPackageSignatureDefaultStreamName(  )
    throw (::com::sun::star::uno::RuntimeException)
{
    return DocumentSignatureHelper::GetPackageSignatureDefaultStreamName();
}


sal_Bool DocumentDigitalSignatures::ImplViewSignatures(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XInputStream >& xSignStream,
    DocumentSignatureMode eMode, bool bReadOnly ) throw (RuntimeException)
{
    Reference< io::XStream > xStream;
    if ( xSignStream.is() )
        xStream = Reference< io::XStream >( xSignStream, UNO_QUERY );
    return ImplViewSignatures( rxStorage, xStream, eMode, bReadOnly );
}

sal_Bool DocumentDigitalSignatures::ImplViewSignatures(
    const Reference< css::embed::XStorage >& rxStorage, const Reference< css::io::XStream >& xSignStream,
    DocumentSignatureMode eMode, bool bReadOnly ) throw (RuntimeException)
{
    sal_Bool bChanges = sal_False;
    DigitalSignaturesDialog aSignaturesDialog(
        NULL, mxCtx, eMode, bReadOnly, m_sODFVersion, m_bHasDocumentSignature);
    bool bInit = aSignaturesDialog.Init();
    DBG_ASSERT( bInit, "Error initializing security context!" );
    if ( bInit )
    {
        aSignaturesDialog.SetStorage( rxStorage );
        aSignaturesDialog.SetSignatureStream( xSignStream );
        if ( aSignaturesDialog.Execute() )
        {
            if ( aSignaturesDialog.SignaturesChanged() )
            {
                bChanges = sal_True;
                // If we have a storage and no stream, we are responsible for commit
                if ( rxStorage.is() && !xSignStream.is() )
                {
                    uno::Reference< embed::XTransactedObject > xTrans( rxStorage, uno::UNO_QUERY );
                    xTrans->commit();
                }
            }
        }
    }
    else
    {
        WarningBox aBox( NULL, XMLSEC_RES( RID_XMLSECWB_NO_MOZILLA_PROFILE ) );
        aBox.Execute();
    }

    return bChanges;
}

Sequence< css::security::DocumentSignatureInformation >
DocumentDigitalSignatures::ImplVerifySignatures(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XInputStream >& xSignStream, DocumentSignatureMode eMode ) throw (RuntimeException)
{
    if (!rxStorage.is())
    {
        DBG_ASSERT(0, "Error, no XStorage provided");
        return Sequence<css::security::DocumentSignatureInformation>();
    }
    // First check for the InputStream, to avoid unnecessary initialization of the security environemnt...
    SignatureStreamHelper aStreamHelper;
    Reference< io::XInputStream > xInputStream = xSignStream;

    if ( !xInputStream.is() )
    {
        aStreamHelper = DocumentSignatureHelper::OpenSignatureStream( rxStorage, embed::ElementModes::READ, eMode );
        if ( aStreamHelper.xSignatureStream.is() )
            xInputStream = Reference< io::XInputStream >( aStreamHelper.xSignatureStream, UNO_QUERY );
    }

    if ( !xInputStream.is() )
        return Sequence< ::com::sun::star::security::DocumentSignatureInformation >(0);


    XMLSignatureHelper aSignatureHelper( mxCtx );

    bool bInit = aSignatureHelper.Init();

    DBG_ASSERT( bInit, "Error initializing security context!" );

    if ( !bInit )
        return Sequence< ::com::sun::star::security::DocumentSignatureInformation >(0);

    aSignatureHelper.SetStorage(rxStorage, m_sODFVersion);

    aSignatureHelper.StartMission();

    aSignatureHelper.ReadAndVerifySignature( xInputStream );

    aSignatureHelper.EndMission();

    Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > xSecEnv = aSignatureHelper.GetSecurityEnvironment();

    SignatureInformations aSignInfos = aSignatureHelper.GetSignatureInformations();
    int nInfos = aSignInfos.size();
    Sequence< css::security::DocumentSignatureInformation > aInfos(nInfos);
    css::security::DocumentSignatureInformation* arInfos = aInfos.getArray();

    if ( nInfos )
    {
       Reference<security::XSerialNumberAdapter> xSerialNumberAdapter =
            ::com::sun::star::security::SerialNumberAdapter::create(mxCtx);

        for( int n = 0; n < nInfos; ++n )
        {
            DocumentSignatureAlgorithm mode = DocumentSignatureHelper::getDocumentAlgorithm(
                m_sODFVersion, aSignInfos[n]);
            const std::vector< rtl::OUString > aElementsToBeVerified =
                DocumentSignatureHelper::CreateElementList(
                rxStorage, ::rtl::OUString(), eMode, mode);

            const SignatureInformation& rInfo = aSignInfos[n];
            css::security::DocumentSignatureInformation& rSigInfo = arInfos[n];

            if (!rInfo.ouX509Certificate.isEmpty())
               rSigInfo.Signer = xSecEnv->createCertificateFromAscii( rInfo.ouX509Certificate ) ;
            if (!rSigInfo.Signer.is())
                rSigInfo.Signer = xSecEnv->getCertificate( rInfo.ouX509IssuerName, xSerialNumberAdapter->toSequence( rInfo.ouX509SerialNumber ) );

            // Time support again (#i38744#)
            Date aDate( rInfo.stDateTime.Day, rInfo.stDateTime.Month, rInfo.stDateTime.Year );
            Time aTime( rInfo.stDateTime.Hours, rInfo.stDateTime.Minutes,
                        rInfo.stDateTime.Seconds, rInfo.stDateTime.HundredthSeconds );
            rSigInfo.SignatureDate = aDate.GetDate();
            rSigInfo.SignatureTime = aTime.GetTime();

            // Verify certificate
            //We have patched our version of libxmlsec, so that it does not verify the certificates. This has two
            //reasons. First we want two separate status for signature and certificate. Second libxmlsec calls
            //CERT_VerifyCertificate (solaris, linux) falsly, so that it always regards the certificate as valid.
            //On Window the checking of the certificate path is buggy. It does name matching (issuer, subject name)
            //to find the parent certificate. It does not take into account that there can be several certificates
            //with the same subject name.
            if (rSigInfo.Signer.is())
            {
                try {
                    rSigInfo.CertificateStatus = xSecEnv->verifyCertificate(rSigInfo.Signer,
                        Sequence<Reference<css::security::XCertificate> >());
                } catch (SecurityException& ) {
                    OSL_FAIL("Verification of certificate failed");
                    rSigInfo.CertificateStatus = css::security::CertificateValidity::INVALID;
                }
            }
            else
            {
                //We should always be aible to get the certificates because it is contained in the document,
                //unless the document is damaged so that signature xml file could not be parsed.
                rSigInfo.CertificateStatus = css::security::CertificateValidity::INVALID;
            }

            rSigInfo.SignatureIsValid = ( rInfo.nStatus == ::com::sun::star::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED );


            if ( rSigInfo.SignatureIsValid )
            {
                 rSigInfo.SignatureIsValid =
                      DocumentSignatureHelper::checkIfAllFilesAreSigned(
                      aElementsToBeVerified, rInfo, mode);
            }
            if (eMode == SignatureModeDocumentContent)
                rSigInfo.PartialDocumentSignature =
                    ! DocumentSignatureHelper::isOOo3_2_Signature(aSignInfos[n]);

        }
    }
    return aInfos;

}

void DocumentDigitalSignatures::manageTrustedSources(  ) throw (RuntimeException)
{
    // MT: i45295
    // SecEnv is only needed to display certificate information from trusted sources.
    // Macro Security also has some options where no security environment is needed, so raise dialog anyway.
    // Later I should change the code so the Dialog creates the SecEnv on demand...

    Reference< dcss::xml::crypto::XSecurityEnvironment > xSecEnv;

    XMLSignatureHelper aSignatureHelper( mxCtx );
    if ( aSignatureHelper.Init() )
        xSecEnv = aSignatureHelper.GetSecurityEnvironment();

    MacroSecurity aDlg( NULL, mxCtx, xSecEnv );
    aDlg.Execute();
}

void DocumentDigitalSignatures::showCertificate(
    const Reference< css::security::XCertificate >& _Certificate ) throw (RuntimeException)
{
    XMLSignatureHelper aSignatureHelper( mxCtx );

    bool bInit = aSignatureHelper.Init();

    DBG_ASSERT( bInit, "Error initializing security context!" );

    if ( bInit )
    {
        CertificateViewer aViewer( NULL, aSignatureHelper.GetSecurityEnvironment(), _Certificate, sal_False );
        aViewer.Execute();
    }

}

::sal_Bool DocumentDigitalSignatures::isAuthorTrusted(
    const Reference< css::security::XCertificate >& Author ) throw (RuntimeException)
{
    sal_Bool bFound = sal_False;

    Reference<security::XSerialNumberAdapter> xSerialNumberAdapter =
        ::com::sun::star::security::SerialNumberAdapter::create(mxCtx);

    ::rtl::OUString sSerialNum = xSerialNumberAdapter->toString( Author->getSerialNumber() );

    Sequence< SvtSecurityOptions::Certificate > aTrustedAuthors = SvtSecurityOptions().GetTrustedAuthors();
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

Reference< css::security::XCertificate > DocumentDigitalSignatures::chooseCertificate() throw (RuntimeException)
{
    Reference< dcss::xml::crypto::XSecurityEnvironment > xSecEnv;

    XMLSignatureHelper aSignatureHelper( mxCtx );
    if ( aSignatureHelper.Init() )
        xSecEnv = aSignatureHelper.GetSecurityEnvironment();

    CertificateChooser aChooser( NULL, mxCtx, xSecEnv, aSignatureHelper.GetSignatureInformations());

    if (aChooser.Execute() != RET_OK)
        return Reference< css::security::XCertificate >(0);

    Reference< css::security::XCertificate > xCert = aChooser.GetSelectedCertificate();

    if ( !xCert.is() )
        return Reference< css::security::XCertificate >(0);

    return xCert;
}


::sal_Bool DocumentDigitalSignatures::isLocationTrusted( const ::rtl::OUString& Location ) throw (RuntimeException)
{
    sal_Bool bFound = sal_False;
    INetURLObject aLocObj( Location );
    INetURLObject aLocObjLowCase( Location.toAsciiLowerCase() ); // will be used for case insensitive comparing

    Sequence< ::rtl::OUString > aSecURLs = SvtSecurityOptions().GetSecureURLs();
    const ::rtl::OUString* pSecURLs = aSecURLs.getConstArray();
    const ::rtl::OUString* pSecURLsEnd = pSecURLs + aSecURLs.getLength();
    for ( ; pSecURLs != pSecURLsEnd && !bFound; ++pSecURLs )
        bFound = ::utl::UCBContentHelper::IsSubPath( *pSecURLs, Location );

    return bFound;
}

void DocumentDigitalSignatures::addAuthorToTrustedSources(
    const Reference< css::security::XCertificate >& Author ) throw (RuntimeException)
{
    SvtSecurityOptions aSecOpts;

    Reference<security::XSerialNumberAdapter> xSerialNumberAdapter =
        ::com::sun::star::security::SerialNumberAdapter::create(mxCtx);

    SvtSecurityOptions::Certificate aNewCert( 3 );
    aNewCert[ 0 ] = Author->getIssuerName();
    aNewCert[ 1 ] = xSerialNumberAdapter->toString( Author->getSerialNumber() );

    rtl::OUStringBuffer aStrBuffer;
    ::sax::Converter::encodeBase64(aStrBuffer, Author->getEncoded());
    aNewCert[ 2 ] = aStrBuffer.makeStringAndClear();


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
    const Reference< XComponentContext >& rCtx) throw ( Exception )
{
    return (cppu::OWeakObject*) new DocumentDigitalSignatures( rCtx );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include "documentdigitalsignatures.hxx"
#include <xmlsecurity/digitalsignaturesdialog.hxx>
#include <xmlsecurity/certificatechooser.hxx>
#include <xmlsecurity/certificateviewer.hxx>
#include <xmlsecurity/macrosecurity.hxx>
#include <xmlsecurity/biginteger.hxx>
#include <xmlsecurity/global.hrc>

#include <sax/tools/converter.hxx>

#include <../dialogs/resourcemanager.hxx>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/ucb/Command.hpp>
#include <vcl/layout.hxx>
#include <unotools/securityoptions.hxx>
#include <com/sun/star/security/CertificateValidity.hpp>
#include <com/sun/star/security/SerialNumberAdapter.hpp>
#include <comphelper/documentconstants.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

using namespace css;
using namespace css::uno;

DocumentDigitalSignatures::DocumentDigitalSignatures( const Reference< XComponentContext >& rxCtx ):
    mxCtx(rxCtx),
    m_sODFVersion(ODFVER_012_TEXT),
    m_nArgumentsCount(0),
    m_bHasDocumentSignature(false)
{
}

void DocumentDigitalSignatures::initialize( const Sequence< Any >& aArguments)
        throw (css::uno::Exception, css::uno::RuntimeException, std::exception)
{
    if (aArguments.getLength() > 2)
        throw css::lang::IllegalArgumentException(
          "DocumentDigitalSignatures::initialize requires zero, one, or two arguments",
          static_cast<XInitialization*>(this), 0);

    m_nArgumentsCount = aArguments.getLength();

    if (aArguments.getLength() > 0)
    {
        if (!(aArguments[0] >>= m_sODFVersion))
            throw css::lang::IllegalArgumentException(
                "DocumentDigitalSignatures::initialize: the first arguments must be a string",
                static_cast<XInitialization*>(this), 0);

        if (aArguments.getLength() == 2
            && !(aArguments[1] >>= m_bHasDocumentSignature))
            throw css::lang::IllegalArgumentException(
                "DocumentDigitalSignatures::initialize: the second arguments must be a bool",
                static_cast<XInitialization*>(this), 1);

        //the Version is supported as of ODF1.2, so for and 1.1 document or older we will receive the
        //an empty string. In this case we set it to ODFVER_010_TEXT. Then we can later check easily
        //if initialize was called. Only then m_sODFVersion.getLength() is greater than 0
        if (m_sODFVersion.isEmpty())
            m_sODFVersion = ODFVER_010_TEXT;
    }
}

OUString DocumentDigitalSignatures::getImplementationName()
    throw (css::uno::RuntimeException, std::exception)
{
    return GetImplementationName();
}

sal_Bool DocumentDigitalSignatures::supportsService(
    OUString const & ServiceName)
    throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString>
DocumentDigitalSignatures::getSupportedServiceNames()
    throw (css::uno::RuntimeException, std::exception)
{
    return GetSupportedServiceNames();
}

sal_Bool DocumentDigitalSignatures::signDocumentContent(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XStream >& xSignStream)
        throw (RuntimeException, std::exception)
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(), "DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    return ImplViewSignatures( rxStorage, xSignStream, SignatureModeDocumentContent, false );
}

Sequence< css::security::DocumentSignatureInformation >
DocumentDigitalSignatures::verifyDocumentContentSignatures(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XInputStream >& xSignInStream ) throw (RuntimeException, std::exception)
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),"DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    return ImplVerifySignatures( rxStorage, xSignInStream, SignatureModeDocumentContent );
}

void DocumentDigitalSignatures::showDocumentContentSignatures(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XInputStream >& xSignInStream ) throw (RuntimeException, std::exception)
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),"DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    ImplViewSignatures( rxStorage, xSignInStream, SignatureModeDocumentContent, true );
}

OUString DocumentDigitalSignatures::getDocumentContentSignatureDefaultStreamName()
    throw (css::uno::RuntimeException, std::exception)
{
    return DocumentSignatureHelper::GetDocumentContentSignatureDefaultStreamName();
}

sal_Bool DocumentDigitalSignatures::signScriptingContent(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XStream >& xSignStream ) throw (RuntimeException, std::exception)
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),"DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    OSL_ENSURE(m_nArgumentsCount == 2, "DocumentDigitalSignatures: Service was not initialized properly");
    return ImplViewSignatures( rxStorage, xSignStream, SignatureModeMacros, false );
}

Sequence< css::security::DocumentSignatureInformation >
DocumentDigitalSignatures::verifyScriptingContentSignatures(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XInputStream >& xSignInStream ) throw (RuntimeException, std::exception)
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),"DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    return ImplVerifySignatures( rxStorage, xSignInStream, SignatureModeMacros );
}

void DocumentDigitalSignatures::showScriptingContentSignatures(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XInputStream >& xSignInStream ) throw (RuntimeException, std::exception)
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),"DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    ImplViewSignatures( rxStorage, xSignInStream, SignatureModeMacros, true );
}

OUString DocumentDigitalSignatures::getScriptingContentSignatureDefaultStreamName()
    throw (css::uno::RuntimeException, std::exception)
{
    return DocumentSignatureHelper::GetScriptingContentSignatureDefaultStreamName();
}


sal_Bool DocumentDigitalSignatures::signPackage(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XStream >& xSignStream  ) throw (RuntimeException, std::exception)
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),"DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    return ImplViewSignatures( rxStorage, xSignStream, SignatureModePackage, false );
}

Sequence< css::security::DocumentSignatureInformation >
DocumentDigitalSignatures::verifyPackageSignatures(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XInputStream >& xSignInStream ) throw (RuntimeException, std::exception)
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),"DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    return ImplVerifySignatures( rxStorage, xSignInStream, SignatureModePackage );
}

void DocumentDigitalSignatures::showPackageSignatures(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XInputStream >& xSignInStream ) throw (RuntimeException, std::exception)
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),"DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    ImplViewSignatures( rxStorage, xSignInStream, SignatureModePackage, true );
}

OUString DocumentDigitalSignatures::getPackageSignatureDefaultStreamName(  )
    throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return DocumentSignatureHelper::GetPackageSignatureDefaultStreamName();
}


bool DocumentDigitalSignatures::ImplViewSignatures(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XInputStream >& xSignStream,
    DocumentSignatureMode eMode, bool bReadOnly ) throw (RuntimeException, std::exception)
{
    Reference< io::XStream > xStream;
    if ( xSignStream.is() )
        xStream.set( xSignStream, UNO_QUERY );
    return ImplViewSignatures( rxStorage, xStream, eMode, bReadOnly );
}

bool DocumentDigitalSignatures::ImplViewSignatures(
    const Reference< css::embed::XStorage >& rxStorage, const Reference< css::io::XStream >& xSignStream,
    DocumentSignatureMode eMode, bool bReadOnly ) throw (RuntimeException, std::exception)
{
    bool bChanges = false;
    ScopedVclPtrInstance<DigitalSignaturesDialog> aSignaturesDialog(
        nullptr, mxCtx, eMode, bReadOnly, m_sODFVersion,
        m_bHasDocumentSignature);
    bool bInit = aSignaturesDialog->Init();
    DBG_ASSERT( bInit, "Error initializing security context!" );
    if ( bInit )
    {
        aSignaturesDialog->SetStorage( rxStorage );
        aSignaturesDialog->SetSignatureStream( xSignStream );
        if ( aSignaturesDialog->Execute() )
        {
            if ( aSignaturesDialog->SignaturesChanged() )
            {
                bChanges = true;
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
        ScopedVclPtrInstance< MessageDialog > aBox(nullptr, XMLSEC_RES(RID_XMLSECWB_NO_MOZILLA_PROFILE), VCL_MESSAGE_WARNING);
        aBox->Execute();
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
        SAL_WARN( "xmlsecurity", "Error, no XStorage provided");
        return Sequence<css::security::DocumentSignatureInformation>();
    }
    // First check for the InputStream, to avoid unnecessary initialization of the security environment...
    SignatureStreamHelper aStreamHelper;
    Reference< io::XInputStream > xInputStream = xSignStream;

    if ( !xInputStream.is() )
    {
        aStreamHelper = DocumentSignatureHelper::OpenSignatureStream( rxStorage, embed::ElementModes::READ, eMode );
        if ( aStreamHelper.xSignatureStream.is() )
            xInputStream.set( aStreamHelper.xSignatureStream, UNO_QUERY );
    }

    if (!xInputStream.is() && aStreamHelper.nStorageFormat != embed::StorageFormats::OFOPXML)
        return Sequence< ::com::sun::star::security::DocumentSignatureInformation >(0);


    XMLSignatureHelper aSignatureHelper( mxCtx );

    bool bInit = aSignatureHelper.Init();

    DBG_ASSERT( bInit, "Error initializing security context!" );

    if ( !bInit )
        return Sequence< ::com::sun::star::security::DocumentSignatureInformation >(0);

    aSignatureHelper.SetStorage(rxStorage, m_sODFVersion);

    aSignatureHelper.StartMission();

    if (xInputStream.is())
        aSignatureHelper.ReadAndVerifySignature(xInputStream);
    else if (aStreamHelper.nStorageFormat == embed::StorageFormats::OFOPXML)
        aSignatureHelper.ReadAndVerifySignatureStorage(aStreamHelper.xSignatureStorage);

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
            const std::vector< OUString > aElementsToBeVerified =
                DocumentSignatureHelper::CreateElementList(
                rxStorage, eMode, mode);

            const SignatureInformation& rInfo = aSignInfos[n];
            css::security::DocumentSignatureInformation& rSigInfo = arInfos[n];

            if (!rInfo.ouX509Certificate.isEmpty())
               rSigInfo.Signer = xSecEnv->createCertificateFromAscii( rInfo.ouX509Certificate ) ;
            if (!rSigInfo.Signer.is())
                rSigInfo.Signer = xSecEnv->getCertificate( rInfo.ouX509IssuerName, xSerialNumberAdapter->toSequence( rInfo.ouX509SerialNumber ) );

            // Time support again (#i38744#)
            Date aDate( rInfo.stDateTime.Day, rInfo.stDateTime.Month, rInfo.stDateTime.Year );
            tools::Time aTime( rInfo.stDateTime.Hours, rInfo.stDateTime.Minutes,
                        rInfo.stDateTime.Seconds, rInfo.stDateTime.NanoSeconds );
            rSigInfo.SignatureDate = aDate.GetDate();
            rSigInfo.SignatureTime = aTime.GetTime();

            // Verify certificate
            //We have patched our version of libxmlsec, so that it does not verify the certificates. This has two
            //reasons. First we want two separate status for signature and certificate. Second libxmlsec calls
            //CERT_VerifyCertificate (Solaris, Linux) falsely, so that it always regards the certificate as valid.
            //On Windows the checking of the certificate path is buggy. It does name matching (issuer, subject name)
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
                //We should always be able to get the certificates because it is contained in the document,
                //unless the document is damaged so that signature xml file could not be parsed.
                rSigInfo.CertificateStatus = css::security::CertificateValidity::INVALID;
            }

            rSigInfo.SignatureIsValid = ( rInfo.nStatus == ::com::sun::star::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED );


            // OOXML intentionally doesn't sign metadata.
            if ( rSigInfo.SignatureIsValid && aStreamHelper.nStorageFormat != embed::StorageFormats::OFOPXML)
            {
                 rSigInfo.SignatureIsValid =
                      DocumentSignatureHelper::checkIfAllFilesAreSigned(
                      aElementsToBeVerified, rInfo, mode);
            }
            if (eMode == SignatureModeDocumentContent)
            {
                if (aStreamHelper.nStorageFormat == embed::StorageFormats::OFOPXML)
                    rSigInfo.PartialDocumentSignature = true;
                else
                    rSigInfo.PartialDocumentSignature = !DocumentSignatureHelper::isOOo3_2_Signature(aSignInfos[n]);
            }

        }
    }
    return aInfos;

}

void DocumentDigitalSignatures::manageTrustedSources(  ) throw (RuntimeException, std::exception)
{
    // MT: i45295
    // SecEnv is only needed to display certificate information from trusted sources.
    // Macro Security also has some options where no security environment is needed, so raise dialog anyway.
    // Later I should change the code so the Dialog creates the SecEnv on demand...

    Reference< css::xml::crypto::XSecurityEnvironment > xSecEnv;

    XMLSignatureHelper aSignatureHelper( mxCtx );
    if ( aSignatureHelper.Init() )
        xSecEnv = aSignatureHelper.GetSecurityEnvironment();

    ScopedVclPtrInstance< MacroSecurity > aDlg( nullptr, mxCtx, xSecEnv );
    aDlg->Execute();
}

void DocumentDigitalSignatures::showCertificate(
    const Reference< css::security::XCertificate >& _Certificate ) throw (RuntimeException, std::exception)
{
    XMLSignatureHelper aSignatureHelper( mxCtx );

    bool bInit = aSignatureHelper.Init();

    DBG_ASSERT( bInit, "Error initializing security context!" );

    if ( bInit )
    {
        ScopedVclPtrInstance< CertificateViewer > aViewer( nullptr, aSignatureHelper.GetSecurityEnvironment(), _Certificate, false );
        aViewer->Execute();
    }

}

sal_Bool DocumentDigitalSignatures::isAuthorTrusted(
    const Reference< css::security::XCertificate >& Author ) throw (RuntimeException, std::exception)
{
    bool bFound = false;

    Reference<security::XSerialNumberAdapter> xSerialNumberAdapter =
        ::com::sun::star::security::SerialNumberAdapter::create(mxCtx);

    OUString sSerialNum = xSerialNumberAdapter->toString( Author->getSerialNumber() );

    Sequence< SvtSecurityOptions::Certificate > aTrustedAuthors = SvtSecurityOptions().GetTrustedAuthors();
    const SvtSecurityOptions::Certificate* pAuthors = aTrustedAuthors.getConstArray();
    const SvtSecurityOptions::Certificate* pAuthorsEnd = pAuthors + aTrustedAuthors.getLength();
    for ( ; pAuthors != pAuthorsEnd; ++pAuthors )
    {
        SvtSecurityOptions::Certificate aAuthor = *pAuthors;
        if ( ( aAuthor[0] == Author->getIssuerName() ) && ( aAuthor[1] == sSerialNum ) )
        {
            bFound = true;
            break;
        }
    }

    return bFound;
}

Reference< css::security::XCertificate > DocumentDigitalSignatures::chooseCertificate() throw (RuntimeException, std::exception)
{
    Reference< css::xml::crypto::XSecurityEnvironment > xSecEnv;

    XMLSignatureHelper aSignatureHelper( mxCtx );
    if ( aSignatureHelper.Init() )
        xSecEnv = aSignatureHelper.GetSecurityEnvironment();

    ScopedVclPtrInstance< CertificateChooser > aChooser(nullptr, mxCtx, xSecEnv);

    if (aChooser->Execute() != RET_OK)
        return Reference< css::security::XCertificate >(nullptr);

    Reference< css::security::XCertificate > xCert = aChooser->GetSelectedCertificate();

    if ( !xCert.is() )
        return Reference< css::security::XCertificate >(nullptr);

    return xCert;
}


sal_Bool DocumentDigitalSignatures::isLocationTrusted( const OUString& Location ) throw (RuntimeException, std::exception)
{
    return SvtSecurityOptions().isTrustedLocationUri(Location);
}

void DocumentDigitalSignatures::addAuthorToTrustedSources(
    const Reference< css::security::XCertificate >& Author ) throw (RuntimeException, std::exception)
{
    SvtSecurityOptions aSecOpts;

    Reference<security::XSerialNumberAdapter> xSerialNumberAdapter =
        ::com::sun::star::security::SerialNumberAdapter::create(mxCtx);

    SvtSecurityOptions::Certificate aNewCert( 3 );
    aNewCert[ 0 ] = Author->getIssuerName();
    aNewCert[ 1 ] = xSerialNumberAdapter->toString( Author->getSerialNumber() );

    OUStringBuffer aStrBuffer;
    ::sax::Converter::encodeBase64(aStrBuffer, Author->getEncoded());
    aNewCert[ 2 ] = aStrBuffer.makeStringAndClear();


    Sequence< SvtSecurityOptions::Certificate > aTrustedAuthors = aSecOpts.GetTrustedAuthors();
    sal_Int32 nCnt = aTrustedAuthors.getLength();
    aTrustedAuthors.realloc( nCnt + 1 );
    aTrustedAuthors[ nCnt ] = aNewCert;

    aSecOpts.SetTrustedAuthors( aTrustedAuthors );
}

void DocumentDigitalSignatures::addLocationToTrustedSources( const OUString& Location ) throw (RuntimeException, std::exception)
{
    SvtSecurityOptions aSecOpt;

    Sequence< OUString > aSecURLs = aSecOpt.GetSecureURLs();
    sal_Int32 nCnt = aSecURLs.getLength();
    aSecURLs.realloc( nCnt + 1 );
    aSecURLs[ nCnt ] = Location;

    aSecOpt.SetSecureURLs( aSecURLs );
}

OUString DocumentDigitalSignatures::GetImplementationName() throw (RuntimeException)
{
    return OUString( "com.sun.star.security.DocumentDigitalSignatures" );
}

Sequence< OUString > DocumentDigitalSignatures::GetSupportedServiceNames() throw (css::uno::RuntimeException)
{
    Sequence<OUString> aRet { "com.sun.star.security.DocumentDigitalSignatures" };
    return aRet;
}


Reference< XInterface > DocumentDigitalSignatures_CreateInstance(
    const Reference< XComponentContext >& rCtx) throw ( Exception )
{
    return static_cast<cppu::OWeakObject*>(new DocumentDigitalSignatures( rCtx ));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

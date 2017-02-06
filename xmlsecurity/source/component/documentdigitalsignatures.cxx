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
#include "resourcemanager.hxx"

#include <digitalsignaturesdialog.hxx>
#include <certificatechooser.hxx>
#include <certificateviewer.hxx>
#include <macrosecurity.hxx>
#include <biginteger.hxx>
#include <global.hrc>
#include <pdfsignaturehelper.hxx>
#include <sax/tools/converter.hxx>

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/XContentIdentifierFactory.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/ucb/Command.hpp>
#include <com/sun/star/uno/SecurityException.hpp>
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

DocumentDigitalSignatures::~DocumentDigitalSignatures()
{
}

void DocumentDigitalSignatures::initialize( const Sequence< Any >& aArguments)
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
{
    return GetImplementationName();
}

sal_Bool DocumentDigitalSignatures::supportsService(
    OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString>
DocumentDigitalSignatures::getSupportedServiceNames()
{
    return GetSupportedServiceNames();
}

sal_Bool DocumentDigitalSignatures::signDocumentContent(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XStream >& xSignStream)
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(), "DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    return ImplViewSignatures( rxStorage, xSignStream, DocumentSignatureMode::Content, false );
}

Sequence< css::security::DocumentSignatureInformation >
DocumentDigitalSignatures::verifyDocumentContentSignatures(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XInputStream >& xSignInStream )
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),"DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    return ImplVerifySignatures( rxStorage, xSignInStream, DocumentSignatureMode::Content );
}

void DocumentDigitalSignatures::showDocumentContentSignatures(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XInputStream >& xSignInStream )
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),"DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    ImplViewSignatures( rxStorage, xSignInStream, DocumentSignatureMode::Content, true );
}

OUString DocumentDigitalSignatures::getDocumentContentSignatureDefaultStreamName()
{
    return DocumentSignatureHelper::GetDocumentContentSignatureDefaultStreamName();
}

sal_Bool DocumentDigitalSignatures::signScriptingContent(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XStream >& xSignStream )
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),"DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    OSL_ENSURE(m_nArgumentsCount == 2, "DocumentDigitalSignatures: Service was not initialized properly");
    return ImplViewSignatures( rxStorage, xSignStream, DocumentSignatureMode::Macros, false );
}

Sequence< css::security::DocumentSignatureInformation >
DocumentDigitalSignatures::verifyScriptingContentSignatures(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XInputStream >& xSignInStream )
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),"DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    return ImplVerifySignatures( rxStorage, xSignInStream, DocumentSignatureMode::Macros );
}

void DocumentDigitalSignatures::showScriptingContentSignatures(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XInputStream >& xSignInStream )
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),"DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    ImplViewSignatures( rxStorage, xSignInStream, DocumentSignatureMode::Macros, true );
}

OUString DocumentDigitalSignatures::getScriptingContentSignatureDefaultStreamName()
{
    return DocumentSignatureHelper::GetScriptingContentSignatureDefaultStreamName();
}


sal_Bool DocumentDigitalSignatures::signPackage(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XStream >& xSignStream  )
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),"DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    return ImplViewSignatures( rxStorage, xSignStream, DocumentSignatureMode::Package, false );
}

Sequence< css::security::DocumentSignatureInformation >
DocumentDigitalSignatures::verifyPackageSignatures(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XInputStream >& xSignInStream )
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),"DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    return ImplVerifySignatures( rxStorage, xSignInStream, DocumentSignatureMode::Package );
}

void DocumentDigitalSignatures::showPackageSignatures(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XInputStream >& xSignInStream )
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),"DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    ImplViewSignatures( rxStorage, xSignInStream, DocumentSignatureMode::Package, true );
}

OUString DocumentDigitalSignatures::getPackageSignatureDefaultStreamName(  )
{
    return DocumentSignatureHelper::GetPackageSignatureDefaultStreamName();
}


void DocumentDigitalSignatures::ImplViewSignatures(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XInputStream >& xSignStream,
    DocumentSignatureMode eMode, bool bReadOnly )
{
    Reference< io::XStream > xStream;
    if ( xSignStream.is() )
        xStream.set( xSignStream, UNO_QUERY );
    ImplViewSignatures( rxStorage, xStream, eMode, bReadOnly );
}

bool DocumentDigitalSignatures::ImplViewSignatures(
    const Reference< css::embed::XStorage >& rxStorage, const Reference< css::io::XStream >& xSignStream,
    DocumentSignatureMode eMode, bool bReadOnly )
{
    bool bChanges = false;
    ScopedVclPtrInstance<DigitalSignaturesDialog> aSignaturesDialog(
        nullptr, mxCtx, eMode, bReadOnly, m_sODFVersion,
        m_bHasDocumentSignature);
    bool bInit = aSignaturesDialog->Init();
    SAL_WARN_IF( !bInit, "xmlsecurity.comp", "Error initializing security context!" );
    if ( bInit )
    {
        if (rxStorage.is())
            // Something ZIP based: ODF or OOXML.
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
        ScopedVclPtrInstance< MessageDialog > aBox(nullptr, XMLSEC_RES(RID_XMLSECWB_NO_MOZILLA_PROFILE), VclMessageType::Warning);
        aBox->Execute();
    }

    return bChanges;
}

Sequence< css::security::DocumentSignatureInformation >
DocumentDigitalSignatures::ImplVerifySignatures(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XInputStream >& xSignStream, DocumentSignatureMode eMode )
{
    DocumentSignatureManager aSignatureManager(mxCtx, eMode);

    bool bInit = aSignatureManager.init();

    SAL_WARN_IF(!bInit, "xmlsecurity.comp", "Error initializing security context!");

    if (!bInit)
        return uno::Sequence<security::DocumentSignatureInformation>(0);

    if (!rxStorage.is())
    {
        if (xSignStream.is())
        {
            // Something not ZIP-based, try PDF.
            PDFSignatureHelper& rSignatureHelper = aSignatureManager.getPDFSignatureHelper();
            if (rSignatureHelper.ReadAndVerifySignature(xSignStream))
                return rSignatureHelper.GetDocumentSignatureInformations(aSignatureManager.getSecurityEnvironment());
        }

        SAL_WARN( "xmlsecurity.comp", "Error, no XStorage provided");
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
        return Sequence< css::security::DocumentSignatureInformation >(0);


    XMLSignatureHelper& rSignatureHelper = aSignatureManager.maSignatureHelper;
    rSignatureHelper.SetStorage(rxStorage, m_sODFVersion);

    rSignatureHelper.StartMission(aSignatureManager.mxSecurityContext);

    if (xInputStream.is())
        rSignatureHelper.ReadAndVerifySignature(xInputStream);
    else if (aStreamHelper.nStorageFormat == embed::StorageFormats::OFOPXML)
        rSignatureHelper.ReadAndVerifySignatureStorage(aStreamHelper.xSignatureStorage);

    rSignatureHelper.EndMission();

    uno::Reference<xml::crypto::XSecurityEnvironment> xSecEnv = aSignatureManager.getSecurityEnvironment();

    SignatureInformations aSignInfos = rSignatureHelper.GetSignatureInformations();
    int nInfos = aSignInfos.size();
    Sequence< css::security::DocumentSignatureInformation > aInfos(nInfos);
    css::security::DocumentSignatureInformation* arInfos = aInfos.getArray();

    if ( nInfos )
    {
       Reference<security::XSerialNumberAdapter> xSerialNumberAdapter =
            css::security::SerialNumberAdapter::create(mxCtx);

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

            rSigInfo.SignatureIsValid = ( rInfo.nStatus == css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED );


            // OOXML intentionally doesn't sign metadata.
            if ( rSigInfo.SignatureIsValid && aStreamHelper.nStorageFormat != embed::StorageFormats::OFOPXML)
            {
                 rSigInfo.SignatureIsValid =
                      DocumentSignatureHelper::checkIfAllFilesAreSigned(
                      aElementsToBeVerified, rInfo, mode);
            }
            if (eMode == DocumentSignatureMode::Content)
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

void DocumentDigitalSignatures::manageTrustedSources(  )
{
    // MT: i45295
    // SecEnv is only needed to display certificate information from trusted sources.
    // Macro Security also has some options where no security environment is needed, so raise dialog anyway.
    // Later I should change the code so the Dialog creates the SecEnv on demand...

    Reference< css::xml::crypto::XSecurityEnvironment > xSecEnv;

    DocumentSignatureMode eMode{};
    DocumentSignatureManager aSignatureManager(mxCtx, eMode);
    if (aSignatureManager.init())
        xSecEnv = aSignatureManager.getSecurityEnvironment();

    ScopedVclPtrInstance< MacroSecurity > aDlg( nullptr, mxCtx, xSecEnv );
    aDlg->Execute();
}

void DocumentDigitalSignatures::showCertificate(
    const Reference< css::security::XCertificate >& Certificate )
{
    DocumentSignatureMode eMode{};
    DocumentSignatureManager aSignatureManager(mxCtx, eMode);

    bool bInit = aSignatureManager.init();

    SAL_WARN_IF( !bInit, "xmlsecurity.comp", "Error initializing security context!" );

    if ( bInit )
    {
        ScopedVclPtrInstance<CertificateViewer> aViewer(nullptr, aSignatureManager.getSecurityEnvironment(), Certificate, false);
        aViewer->Execute();
    }

}

sal_Bool DocumentDigitalSignatures::isAuthorTrusted(
    const Reference< css::security::XCertificate >& Author )
{
    bool bFound = false;

    Reference<security::XSerialNumberAdapter> xSerialNumberAdapter =
        css::security::SerialNumberAdapter::create(mxCtx);

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

Reference< css::security::XCertificate > DocumentDigitalSignatures::chooseCertificate(OUString& rDescription)
{
    Reference< css::xml::crypto::XSecurityEnvironment > xSecEnv;

    DocumentSignatureMode eMode{};
    DocumentSignatureManager aSignatureManager(mxCtx, eMode);
    if (aSignatureManager.init())
        xSecEnv = aSignatureManager.getSecurityEnvironment();

    ScopedVclPtrInstance< CertificateChooser > aChooser(nullptr, mxCtx, xSecEnv);

    if (aChooser->Execute() != RET_OK)
        return Reference< css::security::XCertificate >(nullptr);

    Reference< css::security::XCertificate > xCert = aChooser->GetSelectedCertificate();
    rDescription = aChooser->GetDescription();

    if ( !xCert.is() )
        return Reference< css::security::XCertificate >(nullptr);

    return xCert;
}


sal_Bool DocumentDigitalSignatures::isLocationTrusted( const OUString& Location )
{
    return SvtSecurityOptions().isTrustedLocationUri(Location);
}

void DocumentDigitalSignatures::addAuthorToTrustedSources(
    const Reference< css::security::XCertificate >& Author )
{
    SvtSecurityOptions aSecOpts;

    Reference<security::XSerialNumberAdapter> xSerialNumberAdapter =
        css::security::SerialNumberAdapter::create(mxCtx);

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

void DocumentDigitalSignatures::addLocationToTrustedSources( const OUString& Location )
{
    SvtSecurityOptions aSecOpt;

    Sequence< OUString > aSecURLs = aSecOpt.GetSecureURLs();
    sal_Int32 nCnt = aSecURLs.getLength();
    aSecURLs.realloc( nCnt + 1 );
    aSecURLs[ nCnt ] = Location;

    aSecOpt.SetSecureURLs( aSecURLs );
}

OUString DocumentDigitalSignatures::GetImplementationName()
{
    return OUString( "com.sun.star.security.DocumentDigitalSignatures" );
}

Sequence< OUString > DocumentDigitalSignatures::GetSupportedServiceNames()
{
    Sequence<OUString> aRet { "com.sun.star.security.DocumentDigitalSignatures" };
    return aRet;
}


Reference< XInterface > DocumentDigitalSignatures_CreateInstance(
    const Reference< XComponentContext >& rCtx)
{
    return static_cast<cppu::OWeakObject*>(new DocumentDigitalSignatures( rCtx ));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

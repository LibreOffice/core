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

#include <resourcemanager.hxx>

#include <certificate.hxx>
#include <certificatechooser.hxx>
#include <certificateviewer.hxx>
#include <digitalsignaturesdialog.hxx>
#include <macrosecurity.hxx>
#include <biginteger.hxx>
#include <strings.hrc>
#include <pdfsignaturehelper.hxx>

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/SecurityException.hpp>
#include <vcl/svapp.hxx>
#include <tools/time.hxx>
#include <unotools/securityoptions.hxx>
#include <com/sun/star/security/CertificateValidity.hpp>
#include <com/sun/star/security/CertificateKind.hpp>
#include <comphelper/base64.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/xmlsechelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#include <sfx2/digitalsignatures.hxx>

#include <map>

using namespace css;
using namespace css::uno;
using namespace css::lang;
using namespace css::security;
using namespace css::xml::crypto;

namespace {
class DocumentDigitalSignatures
    : public cppu::WeakImplHelper<css::security::XDocumentDigitalSignatures,
                                  css::lang::XInitialization, css::lang::XServiceInfo>,
      public sfx2::DigitalSignatures
{
private:
    css::uno::Reference<css::uno::XComponentContext> mxCtx;
    css::uno::Reference<css::awt::XWindow> mxParentWindow;

    /// will be set by XInitialization. If not we assume true. false means an earlier version (whatever that means,
    /// this is a string, not a boolean).
    /// Note that the code talks about "ODF version" even if this class is also used to sign OOXML.
    OUString m_sODFVersion;
    /// The number of arguments which were passed in XInitialization::initialize
    int m_nArgumentsCount;
    /// Indicates if the document already contains a document signature
    bool m_bHasDocumentSignature;

    /// @throws css::uno::RuntimeException
    bool ImplViewSignatures(const css::uno::Reference<css::embed::XStorage>& rxStorage,
                            const css::uno::Reference<css::io::XStream>& xSignStream,
                            DocumentSignatureMode eMode, bool bReadOnly);
    /// @throws css::uno::RuntimeException
    void ImplViewSignatures(const css::uno::Reference<css::embed::XStorage>& rxStorage,
                            const css::uno::Reference<css::io::XInputStream>& xSignStream,
                            DocumentSignatureMode eMode, bool bReadOnly);
    /// @throws css::uno::RuntimeException
    css::uno::Sequence<css::security::DocumentSignatureInformation>
    ImplVerifySignatures(const css::uno::Reference<css::embed::XStorage>& rxStorage,
                         const ::com::sun::star::uno::Reference<css::io::XInputStream>& xSignStream,
                         DocumentSignatureMode eMode);

    css::uno::Sequence<css::uno::Reference<css::security::XCertificate>>
    chooseCertificatesImpl(std::map<OUString, OUString>& rProperties, const CertificateChooserUserAction eAction,
                           const CertificateKind certificateKind=CertificateKind_NONE);

    bool
    signWithCertificateImpl(const uno::Reference<frame::XModel>& /*xModel*/,
                            css::uno::Reference<css::security::XCertificate> const& xCertificate,
                            css::uno::Reference<css::embed::XStorage> const& xStorage,
                            css::uno::Reference<css::io::XStream> const& xStream,
                            DocumentSignatureMode eMode);

public:
    explicit DocumentDigitalSignatures(
        const css::uno::Reference<css::uno::XComponentContext>& rxCtx);

    //XInitialization
    void SAL_CALL initialize(const css::uno::Sequence<css::uno::Any>& aArguments) override;

    OUString SAL_CALL getImplementationName() override;

    sal_Bool SAL_CALL supportsService(OUString const& ServiceName) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    // XDocumentDigitalSignatures
    sal_Bool SAL_CALL
    signDocumentContent(const css::uno::Reference<css::embed::XStorage>& xStorage,
                        const css::uno::Reference<css::io::XStream>& xSignStream) override;
    sal_Bool SAL_CALL signSignatureLine(
        const css::uno::Reference<css::embed::XStorage>& Storage,
        const css::uno::Reference<css::io::XStream>& xSignStream, const OUString& aSignatureLineId,
        const Reference<css::security::XCertificate>& xCertificate,
        const Reference<css::graphic::XGraphic>& xValidGraphic,
        const Reference<css::graphic::XGraphic>& xInvalidGraphic,
        const OUString& aComment) override;
    css::uno::Sequence<css::security::DocumentSignatureInformation>
        SAL_CALL verifyDocumentContentSignatures(
            const css::uno::Reference<css::embed::XStorage>& xStorage,
            const css::uno::Reference<css::io::XInputStream>& xSignInStream) override;
    void SAL_CALL showDocumentContentSignatures(
        const css::uno::Reference<css::embed::XStorage>& xStorage,
        const css::uno::Reference<css::io::XInputStream>& xSignInStream) override;
    OUString SAL_CALL getDocumentContentSignatureDefaultStreamName() override;
    sal_Bool SAL_CALL
    signScriptingContent(const css::uno::Reference<css::embed::XStorage>& xStorage,
                         const css::uno::Reference<css::io::XStream>& xSignStream) override;
    css::uno::Sequence<css::security::DocumentSignatureInformation>
        SAL_CALL verifyScriptingContentSignatures(
            const css::uno::Reference<css::embed::XStorage>& xStorage,
            const css::uno::Reference<css::io::XInputStream>& xSignInStream) override;
    void SAL_CALL showScriptingContentSignatures(
        const css::uno::Reference<css::embed::XStorage>& xStorage,
        const css::uno::Reference<css::io::XInputStream>& xSignInStream) override;
    OUString SAL_CALL getScriptingContentSignatureDefaultStreamName() override;
    sal_Bool SAL_CALL
    signPackage(const css::uno::Reference<css::embed::XStorage>& Storage,
                const css::uno::Reference<css::io::XStream>& xSignStream) override;
    css::uno::Sequence<css::security::DocumentSignatureInformation>
        SAL_CALL verifyPackageSignatures(
            const css::uno::Reference<css::embed::XStorage>& Storage,
            const css::uno::Reference<css::io::XInputStream>& xSignInStream) override;
    void SAL_CALL
    showPackageSignatures(const css::uno::Reference<css::embed::XStorage>& xStorage,
                          const css::uno::Reference<css::io::XInputStream>& xSignInStream) override;
    OUString SAL_CALL getPackageSignatureDefaultStreamName() override;
    void SAL_CALL
    showCertificate(const css::uno::Reference<css::security::XCertificate>& Certificate) override;
    void SAL_CALL manageTrustedSources() override;
    sal_Bool SAL_CALL
    isAuthorTrusted(const css::uno::Reference<css::security::XCertificate>& Author) override;
    sal_Bool SAL_CALL isLocationTrusted(const OUString& Location) override;
    void SAL_CALL addAuthorToTrustedSources(
        const css::uno::Reference<css::security::XCertificate>& Author) override;
    void SAL_CALL addLocationToTrustedSources(const OUString& Location) override;

    css::uno::Reference<css::security::XCertificate>
        SAL_CALL chooseCertificate(OUString& rDescription) override;
    css::uno::Reference<css::security::XCertificate>
        SAL_CALL chooseSigningCertificate(OUString& rDescription) override;
    css::uno::Reference<css::security::XCertificate>
        SAL_CALL selectSigningCertificate(OUString& rDescription) override;
    css::uno::Reference<css::security::XCertificate>
        SAL_CALL selectSigningCertificateWithType(const CertificateKind certificateKind,
                                                  OUString& rDescription) override;
    css::uno::Sequence<css::uno::Reference<css::security::XCertificate>>
        SAL_CALL chooseEncryptionCertificate() override;
    css::uno::Reference<css::security::XCertificate> SAL_CALL chooseCertificateWithProps(
        css::uno::Sequence<::com::sun::star::beans::PropertyValue>& Properties) override;

    sal_Bool SAL_CALL signDocumentWithCertificate(
                            css::uno::Reference<css::security::XCertificate> const & xCertificate,
                            css::uno::Reference<css::embed::XStorage> const & xStoragexStorage,
                            css::uno::Reference<css::io::XStream> const & xStream) override;

    sal_Bool SAL_CALL signPackageWithCertificate(
                            css::uno::Reference<css::security::XCertificate> const& xCertificate,
                            css::uno::Reference<css::embed::XStorage> const& xStoragexStorage,
                            css::uno::Reference<css::io::XStream> const& xStream) override;

    sal_Bool SAL_CALL trustUntrustedCertificate(
                            css::uno::Reference<css::security::XCertificate> const& xCertificate) override;

    sal_Bool SAL_CALL signScriptingContentWithCertificate(
                            css::uno::Reference<css::security::XCertificate> const& xCertificate,
                            css::uno::Reference<css::embed::XStorage> const& xStoragexStorage,
                            css::uno::Reference<css::io::XStream> const& xStream) override;

    void SAL_CALL setParentWindow(const css::uno::Reference<css::awt::XWindow>& rParentwindow) override
    {
        mxParentWindow = rParentwindow;
    }

    /// See sfx2::DigitalSignatures::SignModelWithCertificate().
    bool
    SignModelWithCertificate(const css::uno::Reference<css::frame::XModel>& xModel,
                             const css::uno::Reference<css::security::XCertificate>& xCertificate,
                             const css::uno::Reference<css::embed::XStorage>& xStorage,
                             const css::uno::Reference<css::io::XStream>& xStream) override;
};

}

DocumentDigitalSignatures::DocumentDigitalSignatures( const Reference< XComponentContext >& rxCtx ):
    mxCtx(rxCtx),
    m_sODFVersion(ODFVER_013_TEXT),
    m_nArgumentsCount(0),
    m_bHasDocumentSignature(false)
{
}

void DocumentDigitalSignatures::initialize( const Sequence< Any >& aArguments)
{
    if (aArguments.getLength() > 2)
        throw css::lang::IllegalArgumentException(
          u"DocumentDigitalSignatures::initialize requires zero, one, or two arguments"_ustr,
          static_cast<XInitialization*>(this), 0);

    m_nArgumentsCount = aArguments.getLength();

    if (!aArguments.hasElements())
        return;

    if (!(aArguments[0] >>= m_sODFVersion))
        throw css::lang::IllegalArgumentException(
            u"DocumentDigitalSignatures::initialize: the first arguments must be a string"_ustr,
            static_cast<XInitialization*>(this), 0);

    if (aArguments.getLength() == 2
        && !(aArguments[1] >>= m_bHasDocumentSignature))
        throw css::lang::IllegalArgumentException(
            u"DocumentDigitalSignatures::initialize: the second arguments must be a bool"_ustr,
            static_cast<XInitialization*>(this), 1);

    //the Version is supported as of ODF1.2, so for and 1.1 document or older we will receive the
    //an empty string. In this case we set it to ODFVER_010_TEXT. Then we can later check easily
    //if initialize was called. Only then m_sODFVersion.getLength() is greater than 0
    if (m_sODFVersion.isEmpty())
        m_sODFVersion = ODFVER_010_TEXT;
}

OUString DocumentDigitalSignatures::getImplementationName()
{
    return u"com.sun.star.security.DocumentDigitalSignatures"_ustr;
}

sal_Bool DocumentDigitalSignatures::supportsService(
    OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString>
DocumentDigitalSignatures::getSupportedServiceNames()
{
    Sequence<OUString> aRet{ u"com.sun.star.security.DocumentDigitalSignatures"_ustr };
    return aRet;
}

sal_Bool DocumentDigitalSignatures::signDocumentContent(
    const Reference< css::embed::XStorage >& rxStorage,
    const Reference< css::io::XStream >& xSignStream)
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(), "DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");
    return ImplViewSignatures( rxStorage, xSignStream, DocumentSignatureMode::Content, false );
}

sal_Bool DocumentDigitalSignatures::signSignatureLine(
    const Reference<css::embed::XStorage>& rxStorage,
    const Reference<css::io::XStream>& xSignStream,
    const OUString& aSignatureLineId,
    const Reference<css::security::XCertificate>& xCertificate,
    const Reference<css::graphic::XGraphic>& xValidGraphic,
    const Reference<css::graphic::XGraphic>& xInvalidGraphic,
    const OUString& aComment)
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),
               "DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");

    DocumentSignatureManager aSignatureManager(mxCtx, DocumentSignatureMode::Content);

    if (!aSignatureManager.init())
        return false;

    aSignatureManager.setStore(rxStorage);
    aSignatureManager.getSignatureHelper().SetStorage(rxStorage, m_sODFVersion);
    aSignatureManager.setSignatureStream(xSignStream);

    Reference<XXMLSecurityContext> xSecurityContext;
    Reference<XServiceInfo> xServiceInfo(xCertificate, UNO_QUERY);
    if (xServiceInfo->getImplementationName()
        == "com.sun.star.xml.security.gpg.XCertificate_GpgImpl")
        xSecurityContext = aSignatureManager.getGpgSecurityContext();
    else
        xSecurityContext = aSignatureManager.getSecurityContext();

    sal_Int32 nSecurityId;
    bool bSuccess = aSignatureManager.add(xCertificate, xSecurityContext, aComment, nSecurityId,
                                          true, aSignatureLineId, xValidGraphic, xInvalidGraphic);
    if (!bSuccess)
        return false;

    // Need to have this to verify the signature
    aSignatureManager.read(/*bUseTempStream=*/true, /*bCacheLastSignature=*/false);
    aSignatureManager.write(true);

    if (rxStorage.is() && !xSignStream.is())
    {
        uno::Reference<embed::XTransactedObject> xTrans(rxStorage, uno::UNO_QUERY);
        xTrans->commit();
    }

    return true;
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
    auto xSignaturesDialog = std::make_shared<DigitalSignaturesDialog>(
        Application::GetFrameWeld(mxParentWindow), mxCtx, eMode, bReadOnly, m_sODFVersion,
        m_bHasDocumentSignature);
    bool bInit = xSignaturesDialog->Init();
    SAL_WARN_IF( !bInit, "xmlsecurity.comp", "Error initializing security context!" );
    if ( bInit )
    {
        xSignaturesDialog->SetStorage(rxStorage);

        xSignaturesDialog->SetSignatureStream( xSignStream );

        if (bReadOnly)
        {
            xSignaturesDialog->beforeRun();
            weld::DialogController::runAsync(xSignaturesDialog, [] (sal_Int32) {});
            return false;
        }
        else if (xSignaturesDialog->run() == RET_OK)
        {
            if (xSignaturesDialog->SignaturesChanged())
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
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(Application::GetFrameWeld(mxParentWindow),
                                                  VclMessageType::Warning, VclButtonsType::Ok,
                                                  XsResId(RID_XMLSECWB_NO_MOZILLA_PROFILE)));
        xBox->run();
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
        return {};

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
        return {};


    XMLSignatureHelper& rSignatureHelper = aSignatureManager.getSignatureHelper();
    rSignatureHelper.SetStorage(rxStorage, m_sODFVersion);

    rSignatureHelper.StartMission(aSignatureManager.getSecurityContext());

    if (xInputStream.is())
        rSignatureHelper.ReadAndVerifySignature(xInputStream);
    else if (aStreamHelper.nStorageFormat == embed::StorageFormats::OFOPXML)
        rSignatureHelper.ReadAndVerifySignatureStorage(aStreamHelper.xSignatureStorage);

    rSignatureHelper.EndMission();

    uno::Reference<xml::crypto::XSecurityEnvironment> xSecEnv = aSignatureManager.getSecurityEnvironment();
    uno::Reference<xml::crypto::XSecurityEnvironment> xGpgSecEnv = aSignatureManager.getGpgSecurityEnvironment();

    SignatureInformations aSignInfos = rSignatureHelper.GetSignatureInformations();
    int nInfos = aSignInfos.size();
    Sequence< css::security::DocumentSignatureInformation > aInfos(nInfos);
    css::security::DocumentSignatureInformation* arInfos = aInfos.getArray();

    for (int n = 0; n < nInfos; ++n)
    {
        DocumentSignatureAlgorithm mode
            = DocumentSignatureHelper::getDocumentAlgorithm(m_sODFVersion, aSignInfos[n]);
        const std::vector<OUString> aElementsToBeVerified
            = DocumentSignatureHelper::CreateElementList(rxStorage, eMode, mode);

        const SignatureInformation& rInfo = aSignInfos[n];
        css::security::DocumentSignatureInformation& rSigInfo = arInfos[n];

        if (!rInfo.X509Datas.empty()) // X.509
        {
            std::vector<uno::Reference<XCertificate>> certs(
                rSignatureHelper.CheckAndUpdateSignatureInformation(
                    xSecEnv, rInfo));
            if (certs.empty())
            {
                rSigInfo.CertificateStatus = css::security::CertificateValidity::INVALID;
            }
            else
            {
                rSigInfo.Signer = certs.back();
                // get only intermediates
                certs.pop_back();
            // On Windows checking the certificate path is buggy. It does name matching (issuer, subject name)
            // to find the parent certificate. It does not take into account that there can be several certificates
            // with the same subject name.
                try
                {
                    rSigInfo.CertificateStatus = xSecEnv->verifyCertificate(
                        rSigInfo.Signer, comphelper::containerToSequence(certs));
                }
                catch (SecurityException&)
                {
                    SAL_WARN("xmlsecurity.comp", "Verification of certificate failed");
                    rSigInfo.CertificateStatus = css::security::CertificateValidity::INVALID;
                }
            }
        }
        else if (!rInfo.ouGpgCertificate.isEmpty() && xGpgSecEnv.is()) // GPG
        {
            // TODO not ideal to retrieve cert by keyID, might
            // collide, or PGPKeyID format might change - can't we
            // keep the xCert itself in rInfo?
            rSigInfo.Signer = xGpgSecEnv->getCertificate(
                rInfo.ouGpgKeyID, xmlsecurity::numericStringToBigInteger(u""));
            rSigInfo.CertificateStatus = xGpgSecEnv->verifyCertificate(
                rSigInfo.Signer, Sequence<Reference<css::security::XCertificate>>());
        }

        // Time support again (#i38744#)
        Date aDate(rInfo.stDateTime.Day, rInfo.stDateTime.Month, rInfo.stDateTime.Year);
        tools::Time aTime(rInfo.stDateTime.Hours, rInfo.stDateTime.Minutes,
                          rInfo.stDateTime.Seconds, rInfo.stDateTime.NanoSeconds);
        rSigInfo.SignatureDate = aDate.GetDate();
        rSigInfo.SignatureTime = aTime.GetTime() / tools::Time::nanoPerCenti;

        rSigInfo.SignatureIsValid
            = (rInfo.nStatus == css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED);

        // Signature line info (ID + Images)
        if (!rInfo.ouSignatureLineId.isEmpty())
            rSigInfo.SignatureLineId = rInfo.ouSignatureLineId;

        if (rInfo.aValidSignatureImage.is())
            rSigInfo.ValidSignatureLineImage = rInfo.aValidSignatureImage;

        if (rInfo.aInvalidSignatureImage.is())
            rSigInfo.InvalidSignatureLineImage = rInfo.aInvalidSignatureImage;

        // OOXML intentionally doesn't sign metadata.
        if (rSigInfo.SignatureIsValid
            && aStreamHelper.nStorageFormat != embed::StorageFormats::OFOPXML)
        {
            rSigInfo.SignatureIsValid = DocumentSignatureHelper::checkIfAllFilesAreSigned(
                aElementsToBeVerified, rInfo, mode);
        }
        if (eMode == DocumentSignatureMode::Content)
        {
            if (aStreamHelper.nStorageFormat == embed::StorageFormats::OFOPXML)
                rSigInfo.PartialDocumentSignature = true;
            else
                rSigInfo.PartialDocumentSignature
                    = !DocumentSignatureHelper::isOOo3_2_Signature(aSignInfos[n]);
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

    DocumentSignatureManager aSignatureManager(mxCtx, {});
    if (aSignatureManager.init())
        xSecEnv = aSignatureManager.getSecurityEnvironment();

    MacroSecurity aDlg(Application::GetFrameWeld(mxParentWindow), xSecEnv);
    aDlg.run();
}

void DocumentDigitalSignatures::showCertificate(
    const Reference< css::security::XCertificate >& Certificate )
{
    DocumentSignatureManager aSignatureManager(mxCtx, {});

    bool bInit = aSignatureManager.init();

    SAL_WARN_IF( !bInit, "xmlsecurity.comp", "Error initializing security context!" );

    if ( bInit )
    {
        CertificateViewer aViewer(Application::GetFrameWeld(mxParentWindow), aSignatureManager.getSecurityEnvironment(), Certificate, false, nullptr);
        aViewer.run();
    }
}

sal_Bool DocumentDigitalSignatures::isAuthorTrusted(
    const Reference<css::security::XCertificate>& xAuthor)
{
    if (!xAuthor.is())
    {
        return false;
    }
    OUString sSerialNum = xmlsecurity::bigIntegerToNumericString(xAuthor->getSerialNumber());

    std::vector< SvtSecurityOptions::Certificate > aTrustedAuthors = SvtSecurityOptions::GetTrustedAuthors();

    return std::any_of(aTrustedAuthors.begin(), aTrustedAuthors.end(),
        [this, &xAuthor, &sSerialNum](const SvtSecurityOptions::Certificate& rAuthor) {
            if (!xmlsecurity::EqualDistinguishedNames(rAuthor.SubjectName, xAuthor->getIssuerName(), xmlsecurity::NOCOMPAT))
                return false;
            if (rAuthor.SerialNumber != sSerialNum)
                return false;

            DocumentSignatureManager aSignatureManager(mxCtx, {});
            if (!aSignatureManager.init())
                return false;
            uno::Reference<css::security::XCertificate> xCert = aSignatureManager.getSecurityEnvironment()->createCertificateFromAscii(rAuthor.RawData);

            auto pAuthor = dynamic_cast<xmlsecurity::Certificate*>(xAuthor.get());
            auto pCert = dynamic_cast<xmlsecurity::Certificate*>(xCert.get());
            if (pAuthor && pCert)
                return pCert->getSHA256Thumbprint() == pAuthor->getSHA256Thumbprint();

            return xCert->getSHA1Thumbprint() == xAuthor->getSHA1Thumbprint();
        });
}

uno::Sequence<Reference<css::security::XCertificate>>
DocumentDigitalSignatures::chooseCertificatesImpl(std::map<OUString, OUString>& rProperties,
                                                  const CertificateChooserUserAction eAction,
                                                  const CertificateKind certificateKind)
{
    std::vector< Reference< css::xml::crypto::XXMLSecurityContext > > xSecContexts;

    DocumentSignatureManager aSignatureManager(mxCtx, {});
    if (aSignatureManager.init()) {
        xSecContexts.push_back(aSignatureManager.getSecurityContext());
        // Don't include OpenPGP if only X.509 certs are requested
        if (certificateKind == CertificateKind_NONE || certificateKind == CertificateKind_OPENPGP)
            xSecContexts.push_back(aSignatureManager.getGpgSecurityContext());
    }

    std::unique_ptr<CertificateChooser> aChooser = CertificateChooser::getInstance(Application::GetFrameWeld(mxParentWindow), std::move(xSecContexts), eAction);

    if (aChooser->run() != RET_OK)
        return { Reference< css::security::XCertificate >(nullptr) };

    uno::Sequence< Reference< css::security::XCertificate > >  xCerts = aChooser->GetSelectedCertificates();
    rProperties[u"Description"_ustr] = aChooser->GetDescription();
    rProperties[u"Usage"_ustr] = aChooser->GetUsageText();

    return xCerts;
}

Reference< css::security::XCertificate > DocumentDigitalSignatures::chooseCertificate(OUString& rDescription)
{
    return chooseSigningCertificate( rDescription );
}

Reference< css::security::XCertificate > DocumentDigitalSignatures::chooseSigningCertificate(OUString& rDescription)
{
    std::map<OUString, OUString> aProperties;
    Reference< css::security::XCertificate > xCert = chooseCertificatesImpl( aProperties, CertificateChooserUserAction::Sign )[0];
    rDescription = aProperties[u"Description"_ustr];
    return xCert;
}

Reference< css::security::XCertificate > DocumentDigitalSignatures::selectSigningCertificate(OUString& rDescription)
{
    std::map<OUString, OUString> aProperties;
    Reference< css::security::XCertificate > xCert = chooseCertificatesImpl( aProperties, CertificateChooserUserAction::SelectSign )[0];
    rDescription = aProperties[u"Description"_ustr];
    return xCert;
}

Reference<css::security::XCertificate>
DocumentDigitalSignatures::selectSigningCertificateWithType(const CertificateKind certificateKind,
                                                            OUString& rDescription)
{
    std::map<OUString, OUString> aProperties;
    Reference<css::security::XCertificate> xCert
        = chooseCertificatesImpl(aProperties, CertificateChooserUserAction::SelectSign, certificateKind)[0];
    rDescription = aProperties[u"Description"_ustr];
    return xCert;
}

css::uno::Sequence< Reference< css::security::XCertificate > > DocumentDigitalSignatures::chooseEncryptionCertificate()
{
    std::map<OUString, OUString> aProperties;
    uno::Sequence< Reference< css::security::XCertificate > > aCerts=
        chooseCertificatesImpl( aProperties, CertificateChooserUserAction::Encrypt );
    if (aCerts.getLength() == 1 && !aCerts[0].is())
        // our error case contract is: empty sequence, so map that!
        return uno::Sequence< Reference< css::security::XCertificate > >();
    else
        return aCerts;
}

css::uno::Reference< css::security::XCertificate > DocumentDigitalSignatures::chooseCertificateWithProps(Sequence<::com::sun::star::beans::PropertyValue>& rProperties)
{
    std::map<OUString, OUString> aProperties;
    auto xCert = chooseCertificatesImpl( aProperties, CertificateChooserUserAction::Sign )[0];

    std::vector<css::beans::PropertyValue> vec;
    vec.reserve(aProperties.size());
    for (const auto& pair : aProperties)
    {
        vec.emplace_back(comphelper::makePropertyValue(pair.first, pair.second));
    }

    rProperties = comphelper::containerToSequence(vec);
    return xCert;
}

sal_Bool DocumentDigitalSignatures::isLocationTrusted( const OUString& Location )
{
    return SvtSecurityOptions::isTrustedLocationUri(Location);
}

void DocumentDigitalSignatures::addAuthorToTrustedSources(
    const Reference< css::security::XCertificate >& Author )
{
    SvtSecurityOptions::Certificate aNewCert;
    aNewCert.SubjectName = Author->getIssuerName();
    aNewCert.SerialNumber = xmlsecurity::bigIntegerToNumericString( Author->getSerialNumber() );

    OUStringBuffer aStrBuffer;
    ::comphelper::Base64::encode(aStrBuffer, Author->getEncoded());
    aNewCert.RawData = aStrBuffer.makeStringAndClear();

    std::vector< SvtSecurityOptions::Certificate > aTrustedAuthors = SvtSecurityOptions::GetTrustedAuthors();
    aTrustedAuthors.push_back( aNewCert );
    SvtSecurityOptions::SetTrustedAuthors( aTrustedAuthors );
}

void DocumentDigitalSignatures::addLocationToTrustedSources( const OUString& Location )
{
    std::vector< OUString > aSecURLs = SvtSecurityOptions::GetSecureURLs();
    aSecURLs.push_back(Location);

    SvtSecurityOptions::SetSecureURLs( std::move(aSecURLs) );
}

sal_Bool DocumentDigitalSignatures::signDocumentWithCertificate(
            css::uno::Reference<css::security::XCertificate> const & xCertificate,
            css::uno::Reference<css::embed::XStorage> const & xStorage,
            css::uno::Reference<css::io::XStream> const & xStream)
{
    uno::Reference<frame::XModel> xModel;
    return signWithCertificateImpl(xModel, xCertificate, xStorage, xStream,
                                   DocumentSignatureMode::Content);
}

bool DocumentDigitalSignatures::SignModelWithCertificate(
    const uno::Reference<frame::XModel>& xModel,
    const css::uno::Reference<css::security::XCertificate>& xCertificate,
    const css::uno::Reference<css::embed::XStorage>& xStorage,
    const css::uno::Reference<css::io::XStream>& xStream)
{
    return signWithCertificateImpl(xModel, xCertificate, xStorage, xStream,
                                   DocumentSignatureMode::Content);
}

sal_Bool DocumentDigitalSignatures::signPackageWithCertificate(
    css::uno::Reference<css::security::XCertificate> const& xCertificate,
    css::uno::Reference<css::embed::XStorage> const& xStorage,
    css::uno::Reference<css::io::XStream> const& xStream)
{
    uno::Reference<frame::XModel> xModel;
    return signWithCertificateImpl(xModel, xCertificate, xStorage, xStream,
                                   DocumentSignatureMode::Package);
}

sal_Bool DocumentDigitalSignatures::trustUntrustedCertificate(
    css::uno::Reference<css::security::XCertificate> const& xCertificate)
{
    OUString aSubjectName(comphelper::xmlsec::GetContentPart(xCertificate->getSubjectName(), xCertificate->getCertificateKind()));
    OUString aMsg(XsResId(STR_TRUST_UNTRUSTED_PUBKEY));
    aMsg = aMsg.replaceFirst("%{data}", aSubjectName);
    std::unique_ptr<weld::MessageDialog> m_xQueryBox(Application::CreateMessageDialog(nullptr, VclMessageType::Error, VclButtonsType::YesNo, aMsg));
    m_xQueryBox->set_default_response(RET_NO);
    return m_xQueryBox->run() == RET_YES;
}

sal_Bool DocumentDigitalSignatures::signScriptingContentWithCertificate(
    css::uno::Reference<css::security::XCertificate> const& xCertificate,
    css::uno::Reference<css::embed::XStorage> const& xStorage,
    css::uno::Reference<css::io::XStream> const& xStream)
{
    uno::Reference<frame::XModel> xModel;
    return signWithCertificateImpl(xModel, xCertificate, xStorage, xStream,
                                   DocumentSignatureMode::Macros);
}

bool DocumentDigitalSignatures::signWithCertificateImpl(
    const uno::Reference<frame::XModel>& xModel,
    css::uno::Reference<css::security::XCertificate> const& xCertificate,
    css::uno::Reference<css::embed::XStorage> const& xStorage,
    css::uno::Reference<css::io::XStream> const& xStream, DocumentSignatureMode eMode)
{
    OSL_ENSURE(!m_sODFVersion.isEmpty(),
               "DocumentDigitalSignatures: ODF Version not set, assuming minimum 1.2");

    DocumentSignatureManager aSignatureManager(mxCtx, eMode);

    if (!aSignatureManager.init())
        return false;

    aSignatureManager.setStore(xStorage);
    aSignatureManager.getSignatureHelper().SetStorage(xStorage, m_sODFVersion);
    aSignatureManager.setSignatureStream(xStream);
    aSignatureManager.setModel(xModel);

    Reference<XXMLSecurityContext> xSecurityContext;
    Reference<XServiceInfo> xServiceInfo(xCertificate, UNO_QUERY);
    if (xServiceInfo->getImplementationName()
        == "com.sun.star.xml.security.gpg.XCertificate_GpgImpl")
        xSecurityContext = aSignatureManager.getGpgSecurityContext();
    else
        xSecurityContext = aSignatureManager.getSecurityContext();

    sal_Int32 nSecurityId;

    bool bSuccess = aSignatureManager.add(xCertificate, xSecurityContext, u""_ustr, nSecurityId, true);
    if (!bSuccess)
        return false;

    aSignatureManager.read(/*bUseTempStream=*/true, /*bCacheLastSignature=*/false);
    aSignatureManager.write(true);

    if (xStorage.is() && !xStream.is())
    {
        uno::Reference<embed::XTransactedObject> xTransaction(xStorage, uno::UNO_QUERY);
        xTransaction->commit();
    }

    return true;
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_security_DocumentDigitalSignatures_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(
        new DocumentDigitalSignatures(uno::Reference<uno::XComponentContext>(pCtx)));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

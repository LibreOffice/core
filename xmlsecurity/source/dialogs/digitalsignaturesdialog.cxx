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

#include <rtl/ustring.hxx>
#include <sal/config.h>

#include <string_view>

#include <digitalsignaturesdialog.hxx>
#include <certificatechooser.hxx>
#include <certificateviewer.hxx>
#include <biginteger.hxx>
#include <sax/tools/converter.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/configuration.hxx>
#include <officecfg/Office/Common.hxx>

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/security/NoPasswordException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/security/CertificateValidity.hpp>
#include <com/sun/star/packages/WrongPasswordException.hpp>
#include <com/sun/star/security/CertificateKind.hpp>
#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/system/SystemShellExecuteException.hpp>

#include <osl/file.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/uri.hxx>
#include <sal/log.hxx>

#include <tools/date.hxx>
#include <tools/time.hxx>
#include <unotools/datetime.hxx>

#include <bitmaps.hlst>
#include <strings.hrc>
#include <resourcemanager.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/xmlsechelper.hxx>
#include <comphelper/processfactory.hxx>

#include <utility>
#include <vcl/weld.hxx>
#include <vcl/svapp.hxx>
#include <unotools/configitem.hxx>

#ifdef _WIN32
#include <o3tl/char16_t2wchar_t.hxx>
#include <systools/win32/comtools.hxx>
#include <Shlobj.h>
#endif

#if defined MACOSX
#include <sys/stat.h>
#endif

using namespace comphelper;
using namespace css::security;
using namespace css::uno;
using namespace css;

namespace
{

#ifdef _WIN32
    constexpr std::u16string_view aGUIServers[]
        = { u"Gpg4win\\kleopatra.exe",
            u"Gpg4win\\bin\\kleopatra.exe",
            u"GNU\\GnuPG\\kleopatra.exe",
            u"GNU\\GnuPG\\launch-gpa.exe",
            u"GNU\\GnuPG\\gpa.exe",
            u"GnuPG\\bin\\gpa.exe",
            u"GNU\\GnuPG\\bin\\kleopatra.exe",
            u"GNU\\GnuPG\\bin\\launch-gpa.exe",
            u"GNU\\GnuPG\\bin\\gpa.exe"};
#elif defined MACOSX
    constexpr std::u16string_view aGUIServers[]
        = { u"/Applications/GPG Keychain.app",
            u"/Applications/Trusted Key Manager.app", // tdf#147291
            u"/Applications/SCinterface/scManager.app", // tdf#147291
            u"/System/Applications/Utilities/Keychain Access.app"};
#else
    constexpr std::u16string_view aGUIServers[]
        = { u"kleopatra", u"seahorse", u"gpa", u"kgpg"};
#endif

bool GetPathAllOS(OUString& aPath)
{
#ifdef _WIN32
    sal::systools::CoTaskMemAllocated<wchar_t> sPath;
    HRESULT hr
        = SHGetKnownFolderPath(FOLDERID_ProgramFilesX86, KF_FLAG_DEFAULT, nullptr, &sPath);

    if (FAILED(hr))
        return false;
    aPath = o3tl::toU(sPath);
#else
    const char* cPath = getenv("PATH");
    if (!cPath)
        return false;
    aPath = OUString(cPath, strlen(cPath), osl_getThreadTextEncoding());
#endif
    return (!aPath.isEmpty());
}

void GetCertificateManager(OUString& sExecutable)
{
    OUString aPath, aFoundGUIServer;
    if (!GetPathAllOS(aPath))
        return;

    OUString aCetMgrConfig = officecfg::Office::Common::Security::Scripting::CertMgrPath::get();
    if (!aCetMgrConfig.isEmpty())
    {
        if (aCetMgrConfig.indexOf('/') != -1
#ifdef _WIN32
            || aCetMgrConfig.indexOf('\\') != -1
#endif
           )
        {
            sExecutable = aCetMgrConfig;
            return;
        }
        osl::FileBase::RC searchError = osl::File::searchFileURL(
            aCetMgrConfig, aPath,
            aFoundGUIServer);
        if (searchError == osl::FileBase::E_None)
        {
            osl::File::getSystemPathFromFileURL(aFoundGUIServer, sExecutable);
            return;
        }
    }

    for (const auto& rServer: aGUIServers)
    {
        bool bSetCertMgrPath = false;

#ifdef MACOSX
        // On macOS, the list of default certificate manager applications
        // includes absolute paths so check if the path exists and is a
        // directory
        if (rServer.starts_with('/'))
        {
            OString aSysPath = OUString(rServer).toUtf8();
            if (struct stat st; stat(aSysPath.getStr(), &st) == 0 && S_ISDIR(st.st_mode))
            {
                bSetCertMgrPath = true;
                sExecutable = rServer;
            }
        }
#endif

        if (!bSetCertMgrPath)
        {
            osl::FileBase::RC searchError = osl::File::searchFileURL(
                OUString(rServer), aPath,
                aFoundGUIServer);
            if (searchError == osl::FileBase::E_None && osl::File::getSystemPathFromFileURL(aFoundGUIServer, sExecutable) == osl::FileBase::E_None)
                bSetCertMgrPath = true;
        }

        if (bSetCertMgrPath)
        {
            std::shared_ptr<comphelper::ConfigurationChanges> pBatch(
                comphelper::ConfigurationChanges::create());
            officecfg::Office::Common::Security::Scripting::CertMgrPath::set(sExecutable,
                                                                                pBatch);
            pBatch->commit();

            return;
        }
    }
}

bool IsThereCertificateMgr()
{
    OUString sExecutable;
    GetCertificateManager(sExecutable);
    return (!sExecutable.isEmpty());
}
}//anonymous namespace

DigitalSignaturesDialog::DigitalSignaturesDialog(
    weld::Window* pParent,
    const uno::Reference< uno::XComponentContext >& rxCtx, DocumentSignatureMode eMode,
    bool bReadOnly, OUString sODFVersion, bool bHasDocumentSignature)
    : GenericDialogController(pParent, "xmlsec/ui/digitalsignaturesdialog.ui", "DigitalSignaturesDialog")
    , maSignatureManager(rxCtx, eMode)
    , m_sODFVersion (std::move(sODFVersion))
    , m_bHasDocumentSignature(bHasDocumentSignature)
    , m_bWarningShowSignMacro(false)
    , m_xHintDocFT(m_xBuilder->weld_label("dochint"))
    , m_xHintBasicFT(m_xBuilder->weld_label("macrohint"))
    , m_xHintPackageFT(m_xBuilder->weld_label("packagehint"))
    , m_xSignaturesLB(m_xBuilder->weld_tree_view("signatures"))
    , m_xSigsValidImg(m_xBuilder->weld_image("validimg"))
    , m_xSigsValidFI(m_xBuilder->weld_label("validft"))
    , m_xSigsInvalidImg(m_xBuilder->weld_image("invalidimg"))
    , m_xSigsInvalidFI(m_xBuilder->weld_label("invalidft"))
    , m_xSigsNotvalidatedImg(m_xBuilder->weld_image("notvalidatedimg"))
    , m_xSigsNotvalidatedFI(m_xBuilder->weld_label("notvalidatedft"))
    , m_xSigsOldSignatureImg(m_xBuilder->weld_image("oldsignatureimg"))
    , m_xSigsOldSignatureFI(m_xBuilder->weld_label("oldsignatureft"))
    , m_xViewBtn(m_xBuilder->weld_button("view"))
    , m_xAddBtn(m_xBuilder->weld_button("sign"))
    , m_xRemoveBtn(m_xBuilder->weld_button("remove"))
    , m_xStartCertMgrBtn(m_xBuilder->weld_button("start_certmanager"))
    , m_xCloseBtn(m_xBuilder->weld_button("close"))
{
    auto nControlWidth = m_xSignaturesLB->get_approximate_digit_width() * 105;
    m_xSignaturesLB->set_size_request(nControlWidth, m_xSignaturesLB->get_height_rows(10));

    // Give the first column 6 percent, try to distribute the rest equally.
    std::vector<int> aWidths;
    aWidths.push_back(6*nControlWidth/100);
    auto nColWidth = (nControlWidth - aWidths[0]) / 4;
    aWidths.push_back(nColWidth);
    aWidths.push_back(nColWidth);
    aWidths.push_back(nColWidth);
    m_xSignaturesLB->set_column_fixed_widths(aWidths);

    mbVerifySignatures = true;
    mbSignaturesChanged = false;

    m_xSignaturesLB->connect_changed( LINK( this, DigitalSignaturesDialog, SignatureHighlightHdl ) );
    m_xSignaturesLB->connect_row_activated( LINK( this, DigitalSignaturesDialog, SignatureSelectHdl ) );

    m_xViewBtn->connect_clicked( LINK( this, DigitalSignaturesDialog, ViewButtonHdl ) );
    m_xViewBtn->set_sensitive(false);

    m_xAddBtn->connect_clicked( LINK( this, DigitalSignaturesDialog, AddButtonHdl ) );
    if ( bReadOnly  )
        m_xAddBtn->set_sensitive(false);

    m_xRemoveBtn->connect_clicked( LINK( this, DigitalSignaturesDialog, RemoveButtonHdl ) );
    m_xRemoveBtn->set_sensitive(false);

    m_xStartCertMgrBtn->connect_clicked( LINK( this, DigitalSignaturesDialog, CertMgrButtonHdl ) );

    m_xCloseBtn->connect_clicked( LINK( this, DigitalSignaturesDialog, OKButtonHdl) );

    switch( maSignatureManager.getSignatureMode() )
    {
        case DocumentSignatureMode::Content:
            m_xHintDocFT->show();
            break;
        case DocumentSignatureMode::Macros:
            m_xHintBasicFT->show();
            break;
        case DocumentSignatureMode::Package:
            m_xHintPackageFT->show();
            break;
    }

    if (comphelper::LibreOfficeKit::isActive())
    {
        m_xAddBtn->hide();
        m_xRemoveBtn->hide();
        m_xStartCertMgrBtn->hide();
    }

    if (!IsThereCertificateMgr())
    {
        m_xStartCertMgrBtn->set_sensitive(false);
    }
}

DigitalSignaturesDialog::~DigitalSignaturesDialog()
{
    if (m_xViewer)
        m_xViewer->response(RET_OK);

    if (m_xInfoBox)
        m_xInfoBox->response(RET_OK);
}

bool DigitalSignaturesDialog::Init()
{
    bool bInit = maSignatureManager.init();

    SAL_WARN_IF( !bInit, "xmlsecurity.dialogs", "Error initializing security context!" );

    if ( bInit )
    {
        maSignatureManager.getSignatureHelper().SetStartVerifySignatureHdl( LINK( this, DigitalSignaturesDialog, StartVerifySignatureHdl ) );
    }

    return bInit;
}

void DigitalSignaturesDialog::SetStorage( const css::uno::Reference < css::embed::XStorage >& rxStore )
{
    if (!rxStore.is())
    {
        // PDF supports AdES.
        m_bAdESCompliant = true;
        return;
    }

    // only ODF 1.1 wants to be non-XAdES (m_sODFVersion="1.0" for OOXML somehow?)
    m_bAdESCompliant = !rxStore->hasByName("META-INF") // it's a Zip storage
                    || !DocumentSignatureHelper::isODFPre_1_2(m_sODFVersion);

    maSignatureManager.setStore(rxStore);
    maSignatureManager.getSignatureHelper().SetStorage( maSignatureManager.getStore(), m_sODFVersion);
}

void DigitalSignaturesDialog::SetSignatureStream( const css::uno::Reference < css::io::XStream >& rxStream )
{
    maSignatureManager.setSignatureStream(rxStream);
}

bool DigitalSignaturesDialog::canAddRemove()
{
    //FIXME: this func needs some cleanup, such as real split between
    //'canAdd' and 'canRemove' case
    uno::Reference<container::XNameAccess> xNameAccess = maSignatureManager.getStore();
    if (xNameAccess.is() && xNameAccess->hasByName("[Content_Types].xml"))
        // It's always possible to append an OOXML signature.
        return true;

    if (!maSignatureManager.getStore().is())
        // It's always possible to append a PDF signature.
        return true;

    OSL_ASSERT(maSignatureManager.getStore().is());
    bool bDoc1_1 = DocumentSignatureHelper::isODFPre_1_2(m_sODFVersion);

    // see specification
    //cvs: specs/www/appwide/security/Electronic_Signatures_and_Security.sxw
    //Paragraph 'Behavior with regard to ODF 1.2'
    //For both, macro and document
    if ( bDoc1_1 )
    {
        //#4
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                  VclMessageType::Warning, VclButtonsType::Ok,
                                                  XsResId(STR_XMLSECDLG_OLD_ODF_FORMAT)));
        xBox->run();
        return false;
    }

    //As of OOo 3.2 the document signature includes in macrosignatures.xml. That is
    //adding a macro signature will break an existing document signature.
    //The sfx2 will remove the documentsignature when the user adds a macro signature
    if (maSignatureManager.getSignatureMode() == DocumentSignatureMode::Macros)
    {
        if (m_bHasDocumentSignature && !m_bWarningShowSignMacro)
        {
            //The warning says that the document signatures will be removed if the user
            //continues. He can then either press 'OK' or 'NO'
            //It the user presses 'Add' or 'Remove' several times then, then the warning
            //is shown every time until the user presses 'OK'. From then on, the warning
            //is not displayed anymore as long as the signatures dialog is alive.
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
                m_xDialog.get(), VclMessageType::Question, VclButtonsType::YesNo,
                XsResId(STR_XMLSECDLG_QUERY_REMOVEDOCSIGNBEFORESIGN)));
            if (xBox->run() == RET_NO)
                return false;

            m_bWarningShowSignMacro = true;
        }
    }
    return true;
}

bool DigitalSignaturesDialog::canAdd() { return canAddRemove(); }

bool DigitalSignaturesDialog::canRemove()
{
    bool bRet = true;

    if ( maSignatureManager.getSignatureMode() == DocumentSignatureMode::Content )
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                  VclMessageType::Question, VclButtonsType::YesNo,
                                                  XsResId(STR_XMLSECDLG_QUERY_REALLYREMOVE)));
        short nDlgRet = xBox->run();
        bRet = ( nDlgRet == RET_YES );
    }

    return (bRet && canAddRemove());
}

void DigitalSignaturesDialog::beforeRun()
{
    // Verify Signatures and add certificates to ListBox...
    mbVerifySignatures = true;
    ImplGetSignatureInformations(/*bUseTempStream=*/false, /*bCacheLastSignature=*/true);
    ImplFillSignaturesBox();

    // FIXME: Disable the "Use XAdES compliant signatures" checkbox if it is irrelevant. If it is
    // enabled, set its initial state based on existing signatures, if any.

    // If it is OOXML, the checkbox is irrelevant.

    // How to find out here whether it is OOXML? I don't want to create a SignatureStreamHelper and
    // check its nStorageFormat as that seems overly complicated and seems to have weird indirect
    // consequences, as I noticed when I tried to use DocumentSignatureManager::IsXAdESRelevant()
    // (which now is in #if 0).

    if (!maSignatureManager.getCurrentSignatureInformations().empty())
    {
        // If the document has only SHA-1 signatures we probably want it to stay that way?
    }

    // Only verify once, content will not change.
    // But for refreshing signature information, StartVerifySignatureHdl will be called after each add/remove
    mbVerifySignatures = false;
}

short DigitalSignaturesDialog::run()
{
    beforeRun();
    return GenericDialogController::run();
}

IMPL_LINK_NOARG(DigitalSignaturesDialog, SignatureHighlightHdl, weld::TreeView&, void)
{
    bool bSel = m_xSignaturesLB->get_selected_index() != -1;
    m_xViewBtn->set_sensitive( bSel );
    if ( m_xAddBtn->get_sensitive() ) // not read only
        m_xRemoveBtn->set_sensitive( bSel );
}

IMPL_LINK_NOARG(DigitalSignaturesDialog, OKButtonHdl, weld::Button&, void)
{
    if (mbSignaturesChanged)
        maSignatureManager.write(m_bAdESCompliant);

    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(DigitalSignaturesDialog, SignatureSelectHdl, weld::TreeView&, bool)
{
    ImplShowSignaturesDetails();
    return true;
}

IMPL_LINK_NOARG(DigitalSignaturesDialog, ViewButtonHdl, weld::Button&, void)
{
    ImplShowSignaturesDetails();
}

IMPL_LINK_NOARG(DigitalSignaturesDialog, AddButtonHdl, weld::Button&, void)
{
    if( ! canAdd())
        return;
    try
    {
        std::vector<uno::Reference<xml::crypto::XXMLSecurityContext>> xSecContexts
        {
            maSignatureManager.getSecurityContext()
        };
        // Gpg signing is only possible with ODF >= 1.2 documents
        if (DocumentSignatureHelper::CanSignWithGPG(maSignatureManager.getStore(), m_sODFVersion))
            xSecContexts.push_back(maSignatureManager.getGpgSecurityContext());

        std::unique_ptr<CertificateChooser> aChooser = CertificateChooser::getInstance(m_xDialog.get(), std::move(xSecContexts), CertificateChooserUserAction::Sign);
        if (aChooser->run() == RET_OK)
        {
            sal_Int32 nSecurityId;
            if (!maSignatureManager.add(aChooser->GetSelectedCertificates()[0], aChooser->GetSelectedSecurityContext(),
                                        aChooser->GetDescription(), nSecurityId, m_bAdESCompliant))
                return;
            mbSignaturesChanged = true;

            xml::crypto::SecurityOperationStatus nStatus = xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED;

            if (maSignatureManager.getStore().is())
                // In the PDF case the signature information is only available after parsing.
                nStatus = maSignatureManager.getSignatureHelper().GetSignatureInformation( nSecurityId ).nStatus;

            if ( nStatus == css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED )
            {
                mbSignaturesChanged = true;

                // Can't simply remember current information, need parsing for getting full information :(
                // We need to verify the signatures again, otherwise the status in the signature information
                // will not contain
                // SecurityOperationStatus_OPERATION_SUCCEEDED
                mbVerifySignatures = true;
                ImplGetSignatureInformations(/*bUseTempStream=*/true, /*bCacheLastSignature=*/false);
                ImplFillSignaturesBox();
            }
        }
    }
    catch ( uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION( "xmlsecurity.dialogs", "adding a signature!" );
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                  VclMessageType::Error, VclButtonsType::Ok,
                                                  XsResId(STR_XMLSECDLG_SIGNING_FAILED)));
        xBox->run();
        // Don't keep invalid entries...
        ImplGetSignatureInformations(/*bUseTempStream=*/true, /*bCacheLastSignature=*/false);
        ImplFillSignaturesBox();
    }
}

IMPL_LINK_NOARG(DigitalSignaturesDialog, RemoveButtonHdl, weld::Button&, void)
{
    if (!canRemove())
        return;
    int nEntry = m_xSignaturesLB->get_selected_index();
    if (nEntry == -1)
        return;

    try
    {
        sal_uInt16 nSelected = m_xSignaturesLB->get_id(nEntry).toUInt32();
        maSignatureManager.remove(nSelected);

        mbSignaturesChanged = true;

        ImplFillSignaturesBox();
    }
    catch ( uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION( "xmlsecurity.dialogs", "Exception while removing a signature!" );
        // Don't keep invalid entries...
        ImplGetSignatureInformations(/*bUseTempStream=*/true, /*bCacheLastSignature=*/true);
        ImplFillSignaturesBox();
    }
}


IMPL_LINK_NOARG(DigitalSignaturesDialog, CertMgrButtonHdl, weld::Button&, void)
{
    OUString sExecutable;
    GetCertificateManager(sExecutable);

    if (!sExecutable.isEmpty())
    {
        uno::Reference<uno::XComponentContext> xContext
            = ::comphelper::getProcessComponentContext();
        uno::Reference<css::system::XSystemShellExecute> xSystemShell(
            css::system::SystemShellExecute::create(xContext));

        try
        {
            xSystemShell->execute(sExecutable, OUString(),
                                  css::system::SystemShellExecuteFlags::DEFAULTS);
        }
        catch (...)
        {
            // Related tdf#159307 fix uncloseable windows due to uncaught exception
            // XSystemShellExecute::execute() throws an exception for a variety
            // of common error conditions such as files or directories that
            // are non-existent or non-executable. Failure to catch such
            // exceptions would cause the document window to be uncloseable
            // and the application to be unquittable.
            TOOLS_WARN_EXCEPTION( "xmlsecurity.dialogs", "executable failed!" );
            sExecutable = OUString();
        }
    }

    OUString sDialogText = (sExecutable.isEmpty() ?
        XsResId(STR_XMLSECDLG_NO_CERT_MANAGER) : XsResId(STR_XMLSECDLG_OPENED_CRTMGR) + sExecutable);

    std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(
        m_xDialog.get(), VclMessageType::Info, VclButtonsType::Ok,
        sDialogText));
    xInfoBox->run();
}

IMPL_LINK_NOARG(DigitalSignaturesDialog, StartVerifySignatureHdl, LinkParamNone*, bool)
{
    return mbVerifySignatures;
}

void DigitalSignaturesDialog::ImplFillSignaturesBox()
{
    m_xSignaturesLB->clear();

    size_t nInfos = maSignatureManager.getCurrentSignatureInformations().size();
    size_t nValidSigs = 0, nValidCerts = 0;
    bool bAllNewSignatures = true;
    bool bSomePartial = false;

    if( nInfos )
    {
        for( size_t n = 0; n < nInfos; ++n )
        {
            DocumentSignatureAlgorithm mode = DocumentSignatureHelper::getDocumentAlgorithm(
                m_sODFVersion, maSignatureManager.getCurrentSignatureInformations()[n]);
            std::vector< OUString > aElementsToBeVerified;
            if (maSignatureManager.getStore().is())
                aElementsToBeVerified = DocumentSignatureHelper::CreateElementList(maSignatureManager.getStore(), maSignatureManager.getSignatureMode(), mode);

            const SignatureInformation& rInfo = maSignatureManager.getCurrentSignatureInformations()[n];
            uno::Reference< css::security::XCertificate > xCert = getCertificate(rInfo);

            OUString aSubject;
            OUString aIssuer;
            OUString aDateTimeStr;
            OUString aDescription;
            OUString aType;

            bool bCertValid = false;
            if( xCert.is() )
            {
                //check the validity of the cert
                try {
                    sal_Int32 certResult = getSecurityEnvironmentForCertificate(xCert)->verifyCertificate(xCert,
                                                                                                          Sequence<uno::Reference<security::XCertificate> >());

                    bCertValid = certResult == css::security::CertificateValidity::VALID;
                    if ( bCertValid )
                        nValidCerts++;

                } catch (css::uno::SecurityException& ) {
                    OSL_FAIL("Verification of certificate failed");
                    bCertValid = false;
                }

                aSubject = xmlsec::GetContentPart( xCert->getSubjectName(), xCert->getCertificateKind() );
                aIssuer = xmlsec::GetContentPart( xCert->getIssuerName(), xCert->getCertificateKind() );
            }
            else if (!rInfo.ouGpgCertificate.isEmpty())
            {
                // In case we don't have the gpg key locally, get some data from the document
                aIssuer = rInfo.ouGpgOwner;
            }

            aDateTimeStr = utl::GetDateTimeString( rInfo.stDateTime );
            aDescription = rInfo.ouDescription;

            // Decide type string.
            if (maSignatureManager.getStore().is())
            {
                // OpenPGP
                if (!rInfo.ouGpgCertificate.isEmpty())
                    aType = "OpenPGP";
                // XML based: XAdES or not.
                else if (rInfo.GetSigningCertificate() && !rInfo.GetSigningCertificate()->CertDigest.isEmpty())
                    aType = "XAdES";
                else
                    aType = "XML-DSig";
            }
            else
            {
                // Assume PDF: PAdES or not.
                if (rInfo.bHasSigningCertificate)
                    aType = "PAdES";
                else
                    aType = "PDF";
            }

            bool bSigValid = rInfo.nStatus == css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED;

            if ( bSigValid )
            {
                if (maSignatureManager.getStore().is())
                {
                    // ZIP based.
                    bSigValid = DocumentSignatureHelper::checkIfAllFilesAreSigned(
                          aElementsToBeVerified, rInfo, mode);
                }
                else
                {
                    // Assume PDF.
                    bSigValid = !rInfo.bPartialDocumentSignature;
                }

                if( bSigValid )
                    nValidSigs++;
                else
                {
                    bSomePartial = true;
                }
            }

            OUString sImage;
            if (!bSigValid)
            {
                sImage = BMP_SIG_INVALID;
            }
            else if (!bCertValid)
            {
                sImage = BMP_SIG_NOT_VALIDATED;
            }
            //Check if the signature is a "old" document signature, that is, which was created
            //by a version of OOo previous to 3.2
            // If there is no storage, then it's pointless to check storage
            // stream references.
            else if (maSignatureManager.getSignatureMode() == DocumentSignatureMode::Content
                && (maSignatureManager.getStore().is() && !DocumentSignatureHelper::isOOo3_2_Signature(
                maSignatureManager.getCurrentSignatureInformations()[n])))
            {
                sImage = BMP_SIG_NOT_VALIDATED;
                bAllNewSignatures = false;
            }
            else if (maSignatureManager.getSignatureMode() == DocumentSignatureMode::Content
                && DocumentSignatureHelper::isOOo3_2_Signature(
                maSignatureManager.getCurrentSignatureInformations()[n]))
            {
                sImage = BMP_SIG_VALID;
            }
            else if (maSignatureManager.getSignatureMode() == DocumentSignatureMode::Macros)
            {
                sImage = BMP_SIG_VALID;
            }

            m_xSignaturesLB->insert(nullptr, n, nullptr, nullptr,
                                    &sImage, nullptr, false, nullptr);
            m_xSignaturesLB->set_text(n, aSubject, 1);
            m_xSignaturesLB->set_text(n, aIssuer, 2);
            m_xSignaturesLB->set_text(n, aDateTimeStr, 3);
            m_xSignaturesLB->set_text(n, aDescription, 4);
            m_xSignaturesLB->set_text(n, aType, 5);
            m_xSignaturesLB->set_id(n, OUString::number(n)); // misuse user data as index
        }
    }

    bool bAllSigsValid = (nValidSigs == nInfos);
    bool bAllCertsValid = (nValidCerts == nInfos);
    bool bShowValidState = nInfos && (bAllSigsValid && bAllCertsValid && bAllNewSignatures);

    m_xSigsValidImg->set_visible( bShowValidState);
    m_xSigsValidFI->set_visible( bShowValidState );

    bool bShowInvalidState = nInfos && !bAllSigsValid;

    m_xSigsInvalidImg->set_visible( bShowInvalidState && !bSomePartial);
    m_xSigsInvalidFI->set_visible( bShowInvalidState && !bSomePartial);

    bool bShowNotValidatedState = nInfos && bAllSigsValid && !bAllCertsValid;

    m_xSigsNotvalidatedImg->set_visible(bShowNotValidatedState);
    m_xSigsNotvalidatedFI->set_visible(bShowNotValidatedState);

    //bAllNewSignatures is always true if we are not in document mode
    bool bShowOldSignature = nInfos && bAllSigsValid && bAllCertsValid && !bAllNewSignatures;
    m_xSigsOldSignatureImg->set_visible(bShowOldSignature || bSomePartial);
    m_xSigsOldSignatureFI->set_visible(bShowOldSignature || bSomePartial);

    SignatureHighlightHdl(*m_xSignaturesLB);
}

uno::Reference<security::XCertificate> DigitalSignaturesDialog::getCertificate(const SignatureInformation& rInfo)
{
    uno::Reference<xml::crypto::XSecurityEnvironment> xSecEnv = maSignatureManager.getSecurityEnvironment();
    uno::Reference<xml::crypto::XSecurityEnvironment> xGpgSecEnv = maSignatureManager.getGpgSecurityEnvironment();
    uno::Reference<security::XCertificate> xCert;

    //First we try to get the certificate which is embedded in the XML Signature
    if (xSecEnv.is() && rInfo.GetSigningCertificate() && !rInfo.GetSigningCertificate()->X509Certificate.isEmpty())
        xCert = xSecEnv->createCertificateFromAscii(rInfo.GetSigningCertificate()->X509Certificate);
    else {
        //There must be an embedded certificate because we use it to get the
        //issuer name. We cannot use /Signature/KeyInfo/X509Data/X509IssuerName
        //because it could be modified by an attacker. The issuer is displayed
        //in the digital signature dialog.
        //Comparing the X509IssuerName with the one from the X509Certificate in order
        //to find out if the X509IssuerName was modified does not work. See #i62684
        SAL_WARN( "xmlsecurity.dialogs", "Could not find embedded certificate!");
    }

    //In case there is no embedded certificate we try to get it from a local store
    if (!xCert.is() && xSecEnv.is() && rInfo.GetSigningCertificate())
    {
        xCert = xSecEnv->getCertificate(rInfo.GetSigningCertificate()->X509IssuerName,
            xmlsecurity::numericStringToBigInteger(rInfo.GetSigningCertificate()->X509SerialNumber));
    }
    if (!xCert.is() && xGpgSecEnv.is() && !rInfo.ouGpgKeyID.isEmpty())
        xCert = xGpgSecEnv->getCertificate( rInfo.ouGpgKeyID, xmlsecurity::numericStringToBigInteger(u"") );

    SAL_WARN_IF( !xCert.is(), "xmlsecurity.dialogs", "Certificate not found and can't be created!" );

    return xCert;
}

uno::Reference<xml::crypto::XSecurityEnvironment> DigitalSignaturesDialog::getSecurityEnvironmentForCertificate(const uno::Reference<security::XCertificate>& xCert)
{
    switch(xCert->getCertificateKind())
    {
        case CertificateKind_OPENPGP:
            return maSignatureManager.getGpgSecurityEnvironment();
        case CertificateKind_X509:
            return maSignatureManager.getSecurityEnvironment();
        default:
            throw RuntimeException("Unknown certificate kind");
    }
}

//If bUseTempStream is true then the temporary signature stream is used.
//Otherwise the real signature stream is used.
void DigitalSignaturesDialog::ImplGetSignatureInformations(bool bUseTempStream, bool bCacheLastSignature)
{
    maSignatureManager.read(bUseTempStream, bCacheLastSignature);
    mbVerifySignatures = false;
}

void DigitalSignaturesDialog::ImplShowSignaturesDetails()
{
    int nEntry = m_xSignaturesLB->get_selected_index();
    if (nEntry == -1)
        return;

    sal_uInt16 nSelected = m_xSignaturesLB->get_id(nEntry).toUInt32();
    const SignatureInformation& rInfo = maSignatureManager.getCurrentSignatureInformations()[ nSelected ];
    uno::Reference<security::XCertificate> xCert = getCertificate(rInfo);

    if ( xCert.is() )
    {
        if (m_xViewer)
            m_xViewer->response(RET_OK);

        uno::Reference<xml::crypto::XSecurityEnvironment> xSecEnv = getSecurityEnvironmentForCertificate(xCert);
        m_xViewer = std::make_shared<CertificateViewer>(m_xDialog.get(), xSecEnv, xCert, false, nullptr);
        weld::DialogController::runAsync(m_xViewer, [this] (sal_Int32) { m_xViewer = nullptr; });
    }
    else
    {
        if (m_xInfoBox)
            m_xInfoBox->response(RET_OK);

        m_xInfoBox = std::shared_ptr<weld::MessageDialog>(Application::CreateMessageDialog(m_xDialog.get(),
                                                          VclMessageType::Info, VclButtonsType::Ok,
                                                          XsResId(STR_XMLSECDLG_NO_CERT_FOUND)));
        m_xInfoBox->runAsync(m_xInfoBox, [this] (sal_Int32) { m_xInfoBox = nullptr; });
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

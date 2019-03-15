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

#include <digitalsignaturesdialog.hxx>
#include <certificatechooser.hxx>
#include <certificateviewer.hxx>
#include <biginteger.hxx>
#include <sax/tools/converter.hxx>

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
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
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
#include <comphelper/xmlsechelper.hxx>
#include <comphelper/processfactory.hxx>

#include <vcl/weld.hxx>
#include <vcl/svapp.hxx>
#include <unotools/configitem.hxx>

using namespace comphelper;
using namespace css::security;
using namespace css::uno;
using namespace css;

namespace
{
    class SaveODFItem: public utl::ConfigItem
    {
    private:
        sal_Int16 m_nODF;

        virtual void ImplCommit() override;

    public:
    virtual void Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;
        SaveODFItem();
        //See group ODF in Common.xcs
        bool isLessODF1_2()
        {
            return m_nODF < 3;
        }
    };

    void SaveODFItem::ImplCommit() {}
    void SaveODFItem::Notify( const css::uno::Sequence< OUString >& ) {}

    SaveODFItem::SaveODFItem(): utl::ConfigItem("Office.Common/Save"), m_nODF(0)
    {
        OUString sDef("ODF/DefaultVersion");
        Sequence< css::uno::Any > aValues = GetProperties( Sequence<OUString>(&sDef,1) );
        if ( aValues.getLength() != 1)
            throw uno::RuntimeException(
                "[xmlsecurity] Could not open property Office.Common/Save/ODF/DefaultVersion",
                nullptr);

        sal_Int16 nTmp = 0;
        if ( !(aValues[0] >>= nTmp) )
            throw uno::RuntimeException(
                "[xmlsecurity]SaveODFItem::SaveODFItem(): Wrong Type!",
                nullptr );

        m_nODF = nTmp;
    }
}

DigitalSignaturesDialog::DigitalSignaturesDialog(
    weld::Window* pParent,
    uno::Reference< uno::XComponentContext >& rxCtx, DocumentSignatureMode eMode,
    bool bReadOnly, const OUString& sODFVersion, bool bHasDocumentSignature)
    : GenericDialogController(pParent, "xmlsec/ui/digitalsignaturesdialog.ui", "DigitalSignaturesDialog")
    , maSignatureManager(rxCtx, eMode)
    , m_sODFVersion (sODFVersion)
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
    , m_xAdESCompliantCB(m_xBuilder->weld_check_button("adescompliant"))
    , m_xViewBtn(m_xBuilder->weld_button("view"))
    , m_xAddBtn(m_xBuilder->weld_button("sign"))
    , m_xRemoveBtn(m_xBuilder->weld_button("remove"))
    , m_xStartCertMgrBtn(m_xBuilder->weld_button("start_certmanager"))
    , m_xCloseBtn(m_xBuilder->weld_button("close"))
{
    m_bAdESCompliant = !DocumentSignatureHelper::isODFPre_1_2(m_sODFVersion);

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

    m_xAdESCompliantCB->connect_toggled( LINK( this, DigitalSignaturesDialog, AdESCompliantCheckBoxHdl ) );
    m_xAdESCompliantCB->set_active(m_bAdESCompliant);

    m_xViewBtn->connect_clicked( LINK( this, DigitalSignaturesDialog, ViewButtonHdl ) );
    m_xViewBtn->set_sensitive(false);

    m_xAddBtn->connect_clicked( LINK( this, DigitalSignaturesDialog, AddButtonHdl ) );
    if ( bReadOnly  )
        m_xAddBtn->set_sensitive(false);

    m_xRemoveBtn->connect_clicked( LINK( this, DigitalSignaturesDialog, RemoveButtonHdl ) );
    m_xRemoveBtn->set_sensitive(false);

    m_xStartCertMgrBtn->connect_clicked( LINK( this, DigitalSignaturesDialog, CertMgrButtonHdl ) );

    m_xCloseBtn->connect_clicked( LINK( this, DigitalSignaturesDialog, OKButtonHdl) );

    switch( maSignatureManager.meSignatureMode )
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
}

DigitalSignaturesDialog::~DigitalSignaturesDialog()
{
}

bool DigitalSignaturesDialog::Init()
{
    bool bInit = maSignatureManager.init();

    SAL_WARN_IF( !bInit, "xmlsecurity.dialogs", "Error initializing security context!" );

    if ( bInit )
    {
        maSignatureManager.maSignatureHelper.SetStartVerifySignatureHdl( LINK( this, DigitalSignaturesDialog, StartVerifySignatureHdl ) );
    }

    return bInit;
}

void DigitalSignaturesDialog::SetStorage( const css::uno::Reference < css::embed::XStorage >& rxStore )
{
    if (!rxStore.is())
    {
        // PDF supports AdES.
        m_bAdESCompliant = true;
        m_xAdESCompliantCB->set_active(m_bAdESCompliant);
        return;
    }

    maSignatureManager.mxStore = rxStore;
    maSignatureManager.maSignatureHelper.SetStorage( maSignatureManager.mxStore, m_sODFVersion);
}

void DigitalSignaturesDialog::SetSignatureStream( const css::uno::Reference < css::io::XStream >& rxStream )
{
    maSignatureManager.mxSignatureStream = rxStream;
}

bool DigitalSignaturesDialog::canAddRemove()
{
    //FIXME: this func needs some cleanup, such as real split between
    //'canAdd' and 'canRemove' case
    bool ret = true;

    uno::Reference<container::XNameAccess> xNameAccess(maSignatureManager.mxStore, uno::UNO_QUERY);
    if (xNameAccess.is() && xNameAccess->hasByName("[Content_Types].xml"))
        // It's always possible to append an OOXML signature.
        return ret;

    if (!maSignatureManager.mxStore.is())
        // It's always possible to append a PDF signature.
        return ret;

    OSL_ASSERT(maSignatureManager.mxStore.is());
    bool bDoc1_1 = DocumentSignatureHelper::isODFPre_1_2(m_sODFVersion);
    SaveODFItem item;
    bool bSave1_1 = item.isLessODF1_2();

    // see specification
    //cvs: specs/www/appwide/security/Electronic_Signatures_and_Security.sxw
    //Paragraph 'Behavior with regard to ODF 1.2'
    //For both, macro and document
    if ( (!bSave1_1  && bDoc1_1) || (bSave1_1 && bDoc1_1) )
    {
        //#4
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                  VclMessageType::Warning, VclButtonsType::Ok,
                                                  XsResId(STR_XMLSECDLG_OLD_ODF_FORMAT)));
        xBox->run();
        ret = false;
    }

    //As of OOo 3.2 the document signature includes in macrosignatures.xml. That is
    //adding a macro signature will break an existing document signature.
    //The sfx2 will remove the documentsignature when the user adds a macro signature
    if (maSignatureManager.meSignatureMode == DocumentSignatureMode::Macros
        && ret)
    {
        if (m_bHasDocumentSignature && !m_bWarningShowSignMacro)
        {
            //The warning says that the document signatures will be removed if the user
            //continues. He can then either press 'OK' or 'NO'
            //It the user presses 'Add' or 'Remove' several times then, then the warning
            //is shown every time until the user presses 'OK'. From then on, the warning
            //is not displayed anymore as long as the signatures dialog is alive.
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                      VclMessageType::Question, VclButtonsType::YesNo,
                                                      XsResId(STR_XMLSECDLG_QUERY_REMOVEDOCSIGNBEFORESIGN)));
            if (xBox->run() == RET_NO)
                ret = false;
            else
                m_bWarningShowSignMacro = true;

        }
    }
    return ret;
}

bool DigitalSignaturesDialog::canAdd()
{
    return canAddRemove();
}

bool DigitalSignaturesDialog::canRemove()
{
    bool bRet = true;

    if ( maSignatureManager.meSignatureMode == DocumentSignatureMode::Content )
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                  VclMessageType::Question, VclButtonsType::YesNo,
                                                  XsResId(STR_XMLSECDLG_QUERY_REALLYREMOVE)));
        short nDlgRet = xBox->run();
        bRet = ( nDlgRet == RET_YES );
    }

    return (bRet && canAddRemove());
}

short DigitalSignaturesDialog::run()
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

    if (!maSignatureManager.maCurrentSignatureInformations.empty())
    {
        // If the document has only SHA-1 signatures we probably want it to stay that way?
    }

    // Only verify once, content will not change.
    // But for refreshing signature information, StartVerifySignatureHdl will be called after each add/remove
    mbVerifySignatures = false;

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

IMPL_LINK_NOARG(DigitalSignaturesDialog, SignatureSelectHdl, weld::TreeView&, void)
{
    ImplShowSignaturesDetails();
}

IMPL_LINK_NOARG(DigitalSignaturesDialog, AdESCompliantCheckBoxHdl, weld::ToggleButton&, void)
{
    m_bAdESCompliant = m_xAdESCompliantCB->get_active();
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
        std::vector<uno::Reference<xml::crypto::XXMLSecurityContext>> xSecContexts;
        xSecContexts.push_back(maSignatureManager.getSecurityContext());
        // Gpg signing is only possible with ODF >= 1.2 documents
        if (DocumentSignatureHelper::CanSignWithGPG(maSignatureManager.mxStore, m_sODFVersion))
            xSecContexts.push_back(maSignatureManager.getGpgSecurityContext());

        CertificateChooser aChooser(m_xDialog.get(), xSecContexts, UserAction::Sign);
        if (aChooser.run() == RET_OK)
        {
            sal_Int32 nSecurityId;
            if (!maSignatureManager.add(aChooser.GetSelectedCertificates()[0], aChooser.GetSelectedSecurityContext(),
                                        aChooser.GetDescription(), nSecurityId, m_bAdESCompliant))
                return;
            mbSignaturesChanged = true;

            xml::crypto::SecurityOperationStatus nStatus = xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED;

            if (maSignatureManager.mxStore.is())
                // In the PDF case the signature information is only available after parsing.
                nStatus = maSignatureManager.maSignatureHelper.GetSignatureInformation( nSecurityId ).nStatus;

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
        OSL_FAIL( "Exception while adding a signature!" );
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
    if (nEntry != -1)
    {
        try
        {
            sal_uInt16 nSelected = m_xSignaturesLB->get_id(nEntry).toUInt32();
            maSignatureManager.remove(nSelected);

            mbSignaturesChanged = true;

            ImplFillSignaturesBox();
        }
        catch ( uno::Exception& )
        {
            OSL_FAIL( "Exception while removing a signature!" );
            // Don't keep invalid entries...
            ImplGetSignatureInformations(/*bUseTempStream=*/true, /*bCacheLastSignature=*/true);
            ImplFillSignaturesBox();
        }
    }
}

IMPL_LINK_NOARG(DigitalSignaturesDialog, CertMgrButtonHdl, weld::Button&, void)
{
#ifdef _WIN32
    // FIXME: call GpgME::dirInfo("bindir") somewhere in
    // SecurityEnvironmentGpg or whatnot
    // FIXME: perhaps poke GpgME for uiserver, and hope it returns something useful?
    const OUString aGUIServers[] = { OUString("Gpg4win\\kleopatra.exe"), OUString("GNU\\GnuPG\\kleopatra.exe"),
                                     OUString("GNU\\GnuPG\\launch-gpa.exe"), OUString("GNU\\GnuPG\\gpa.exe"),
                                     OUString("GNU\\GnuPG\\bin\\kleopatra.exe"), OUString("GNU\\GnuPG\\bin\\launch-gpa.exe"),
                                     OUString("GNU\\GnuPG\\bin\\gpa.exe") };
    const char* const cPath = "C:\\Program Files (x86)";
#else
    const OUString aGUIServers[] = { OUString("kleopatra"), OUString("seahorse"),  OUString("gpa"), OUString("kgpg") };
    const char* cPath = getenv("PATH");
    if (!cPath)
        return;
#endif

    OUString aPath(cPath, strlen(cPath), osl_getThreadTextEncoding());
    OUString sFoundGUIServer, sExecutable;

    for ( auto const &rServer : aGUIServers )
    {
        osl::FileBase::RC searchError = osl::File::searchFileURL(rServer, aPath, sFoundGUIServer );
        if (searchError == osl::FileBase::E_None)
        {
            osl::File::getSystemPathFromFileURL( sFoundGUIServer, sExecutable );
            break;
        }

    }

    if ( !sExecutable.isEmpty() )
    {
        uno::Reference< uno::XComponentContext > xContext =
            ::comphelper::getProcessComponentContext();
        uno::Reference< css::system::XSystemShellExecute > xSystemShell(
                 css::system::SystemShellExecute::create(xContext) );

        xSystemShell->execute( sExecutable, OUString(),
            css::system::SystemShellExecuteFlags::DEFAULTS );
    }
    else
    {
        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                      XsResId(STR_XMLSECDLG_NO_CERT_MANAGER)));
        xInfoBox->run();
    }
}

IMPL_LINK_NOARG(DigitalSignaturesDialog, StartVerifySignatureHdl, LinkParamNone*, bool)
{
    return mbVerifySignatures;
}

void DigitalSignaturesDialog::ImplFillSignaturesBox()
{
    m_xSignaturesLB->clear();

    size_t nInfos = maSignatureManager.maCurrentSignatureInformations.size();
    size_t nValidSigs = 0, nValidCerts = 0;
    bool bAllNewSignatures = true;

    if( nInfos )
    {
        for( size_t n = 0; n < nInfos; ++n )
        {
            DocumentSignatureAlgorithm mode = DocumentSignatureHelper::getDocumentAlgorithm(
                m_sODFVersion, maSignatureManager.maCurrentSignatureInformations[n]);
            std::vector< OUString > aElementsToBeVerified;
            if (maSignatureManager.mxStore.is())
                aElementsToBeVerified = DocumentSignatureHelper::CreateElementList(maSignatureManager.mxStore, maSignatureManager.meSignatureMode, mode);

            const SignatureInformation& rInfo = maSignatureManager.maCurrentSignatureInformations[n];
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

                aSubject = xmlsec::GetContentPart( xCert->getSubjectName() );
                aIssuer = xmlsec::GetContentPart( xCert->getIssuerName() );
            }
            else if (!rInfo.ouGpgCertificate.isEmpty())
            {
                // In case we don't have the gpg key locally, get some data from the document
                aIssuer = rInfo.ouGpgOwner;
            }

            aDateTimeStr = utl::GetDateTimeString( rInfo.stDateTime );
            aDescription = rInfo.ouDescription;

            // Decide type string.
            if (maSignatureManager.mxStore.is())
            {
                // OpenPGP
                if (!rInfo.ouGpgCertificate.isEmpty())
                    aType = "OpenPGP";
                // XML based: XAdES or not.
                else if (!rInfo.ouCertDigest.isEmpty())
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
                bSigValid = DocumentSignatureHelper::checkIfAllFilesAreSigned(
                      aElementsToBeVerified, rInfo, mode);

                if( bSigValid )
                    nValidSigs++;
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
            //by an version of OOo previous to 3.2
            // If there is no storage, then it's pointless to check storage
            // stream references.
            else if (maSignatureManager.meSignatureMode == DocumentSignatureMode::Content
                && (maSignatureManager.mxStore.is() && !DocumentSignatureHelper::isOOo3_2_Signature(
                maSignatureManager.maCurrentSignatureInformations[n])))
            {
                sImage = BMP_SIG_NOT_VALIDATED;
                bAllNewSignatures = false;
            }
            else if (maSignatureManager.meSignatureMode == DocumentSignatureMode::Content
                && DocumentSignatureHelper::isOOo3_2_Signature(
                maSignatureManager.maCurrentSignatureInformations[n]))
            {
                sImage = BMP_SIG_VALID;
            }
            else if (maSignatureManager.meSignatureMode == DocumentSignatureMode::Macros)
            {
                sImage = BMP_SIG_VALID;
            }

            m_xSignaturesLB->insert(nullptr, n, nullptr, nullptr,
                                    &sImage, nullptr, nullptr, false, nullptr);
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

    m_xSigsInvalidImg->set_visible( bShowInvalidState );
    m_xSigsInvalidFI->set_visible( bShowInvalidState );

    bool bShowNotValidatedState = nInfos && bAllSigsValid && !bAllCertsValid;

    m_xSigsNotvalidatedImg->set_visible(bShowNotValidatedState);
    m_xSigsNotvalidatedFI->set_visible(bShowNotValidatedState);

    //bAllNewSignatures is always true if we are not in document mode
    bool bShowOldSignature = nInfos && bAllSigsValid && bAllCertsValid && !bAllNewSignatures;
    m_xSigsOldSignatureImg->set_visible(bShowOldSignature);
    m_xSigsOldSignatureFI->set_visible(bShowOldSignature);

    SignatureHighlightHdl(*m_xSignaturesLB);
}

uno::Reference<security::XCertificate> DigitalSignaturesDialog::getCertificate(const SignatureInformation& rInfo)
{
    uno::Reference<xml::crypto::XSecurityEnvironment> xSecEnv = maSignatureManager.getSecurityEnvironment();
    uno::Reference<xml::crypto::XSecurityEnvironment> xGpgSecEnv = maSignatureManager.getGpgSecurityEnvironment();
    uno::Reference<security::XCertificate> xCert;

    //First we try to get the certificate which is embedded in the XML Signature
    if (xSecEnv.is() && !rInfo.ouX509Certificate.isEmpty())
        xCert = xSecEnv->createCertificateFromAscii(rInfo.ouX509Certificate);
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
    if (!xCert.is() && xSecEnv.is())
        xCert = xSecEnv->getCertificate( rInfo.ouX509IssuerName, xmlsecurity::numericStringToBigInteger( rInfo.ouX509SerialNumber ) );
    if (!xCert.is() && xGpgSecEnv.is())
        xCert = xGpgSecEnv->getCertificate( rInfo.ouGpgKeyID, xmlsecurity::numericStringToBigInteger("") );

    SAL_WARN_IF( !xCert.is(), "xmlsecurity.dialogs", "Certificate not found and can't be created!" );

    return xCert;
}

uno::Reference<xml::crypto::XSecurityEnvironment> DigitalSignaturesDialog::getSecurityEnvironmentForCertificate(const uno::Reference<security::XCertificate>& xCert)
{
    if (xCert->getCertificateKind() == CertificateKind_OPENPGP)
        return maSignatureManager.getGpgSecurityEnvironment();
    else if (xCert->getCertificateKind() == CertificateKind_X509)
        return maSignatureManager.getSecurityEnvironment();

    throw RuntimeException("Unknown certificate kind");
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
    if (nEntry != -1)
    {
        sal_uInt16 nSelected = m_xSignaturesLB->get_id(nEntry).toUInt32();
        const SignatureInformation& rInfo = maSignatureManager.maCurrentSignatureInformations[ nSelected ];
        uno::Reference<security::XCertificate> xCert = getCertificate(rInfo);

        if ( xCert.is() )
        {
            uno::Reference<xml::crypto::XSecurityEnvironment> xSecEnv = getSecurityEnvironmentForCertificate(xCert);
            CertificateViewer aViewer(m_xDialog.get(), xSecEnv, xCert, false, nullptr);
            aViewer.run();
        }
        else
        {
            std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                          VclMessageType::Info, VclButtonsType::Ok,
                                                          XsResId(STR_XMLSECDLG_NO_CERT_FOUND)));
            xInfoBox->run();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

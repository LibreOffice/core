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
#include <com/sun/star/packages/manifest/ManifestReader.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/system/SystemShellExecuteException.hpp>

#include <osl/file.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/uri.hxx>

#include <tools/date.hxx>
#include <tools/time.hxx>
#include <svtools/treelistentry.hxx>

#include <strings.hrc>
#include <resourcemanager.hxx>

#include <vcl/weld.hxx>
#include <unotools/configitem.hxx>
#include <comphelper/storagehelper.hxx>

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
    vcl::Window* pParent,
    uno::Reference< uno::XComponentContext >& rxCtx, DocumentSignatureMode eMode,
    bool bReadOnly, const OUString& sODFVersion, bool bHasDocumentSignature)
    : ModalDialog(pParent, "DigitalSignaturesDialog", "xmlsec/ui/digitalsignaturesdialog.ui")
    , mxCtx(rxCtx)
    , maSignatureManager(rxCtx, eMode)
    , m_sODFVersion (sODFVersion)
    , m_bHasDocumentSignature(bHasDocumentSignature)
    , m_bWarningShowSignMacro(false)
{
    get(m_pHintDocFT, "dochint");
    get(m_pHintBasicFT, "macrohint");
    get(m_pHintPackageFT, "packagehint");
    get(m_pAdESCompliantCB, "adescompliant");
    get(m_pViewBtn, "view");
    get(m_pAddBtn, "sign");
    get(m_pRemoveBtn, "remove");
    get(m_pCloseBtn, "close");
    get(m_pStartCertMgrBtn, "start_certmanager");
    get(m_pSigsValidImg, "validimg");
    get(m_pSigsValidFI, "validft");
    get(m_pSigsInvalidImg, "invalidimg");
    get(m_pSigsInvalidFI, "invalidft");
    get(m_pSigsNotvalidatedImg, "notvalidatedimg");
    get(m_pSigsNotvalidatedFI, "notvalidatedft");
    get(m_pSigsOldSignatureImg, "oldsignatureimg");
    get(m_pSigsOldSignatureFI, "oldsignatureft");

    m_bAdESCompliant = !DocumentSignatureHelper::isODFPre_1_2(m_sODFVersion);

    Size aControlSize(375, 109);
    const long nControlWidth = aControlSize.Width();
    aControlSize = LogicToPixel(aControlSize, MapMode(MapUnit::MapAppFont));
    SvSimpleTableContainer *pSignatures = get<SvSimpleTableContainer>("signatures");
    pSignatures->set_width_request(aControlSize.Width());
    pSignatures->set_height_request(aControlSize.Height());

    m_pSignaturesLB = VclPtr<SvSimpleTable>::Create(*pSignatures);
    // Give the first column 6 percent, try to distribute the rest equally.
    static long aTabs[] = { 0, 6*nControlWidth/100, 25*nControlWidth/100, 44*nControlWidth/100, 62*nControlWidth/100, 81*nControlWidth/100 };
    m_pSignaturesLB->SetTabs(SAL_N_ELEMENTS(aTabs), aTabs);

    m_pSignaturesLB->InsertHeaderEntry("\t" + get<FixedText>("signed")->GetText() + "\t"
               + get<FixedText>("issued")->GetText() + "\t" + get<FixedText>("date")->GetText() + "\t"
               + get<FixedText>("description")->GetText() + "\t" + get<FixedText>("type")->GetText());

    mbVerifySignatures = true;
    mbSignaturesChanged = false;

    m_pSignaturesLB->SetSelectHdl( LINK( this, DigitalSignaturesDialog, SignatureHighlightHdl ) );
    m_pSignaturesLB->SetDoubleClickHdl( LINK( this, DigitalSignaturesDialog, SignatureSelectHdl ) );

    m_pAdESCompliantCB->SetToggleHdl( LINK( this, DigitalSignaturesDialog, AdESCompliantCheckBoxHdl ) );
    m_pAdESCompliantCB->Check(m_bAdESCompliant);

    m_pViewBtn->SetClickHdl( LINK( this, DigitalSignaturesDialog, ViewButtonHdl ) );
    m_pViewBtn->Disable();

    m_pAddBtn->SetClickHdl( LINK( this, DigitalSignaturesDialog, AddButtonHdl ) );
    if ( bReadOnly  )
        m_pAddBtn->Disable();

    m_pRemoveBtn->SetClickHdl( LINK( this, DigitalSignaturesDialog, RemoveButtonHdl ) );
    m_pRemoveBtn->Disable();

    m_pStartCertMgrBtn->SetClickHdl( LINK( this, DigitalSignaturesDialog, CertMgrButtonHdl ) );

    m_pCloseBtn->SetClickHdl( LINK( this, DigitalSignaturesDialog, OKButtonHdl) );

    switch( maSignatureManager.meSignatureMode )
    {
        case DocumentSignatureMode::Content:
            m_pHintDocFT->Show();
            break;
        case DocumentSignatureMode::Macros:
            m_pHintBasicFT->Show();
            break;
        case DocumentSignatureMode::Package:
            m_pHintPackageFT->Show();
            break;
    }
}

DigitalSignaturesDialog::~DigitalSignaturesDialog()
{
    disposeOnce();
}

void DigitalSignaturesDialog::dispose()
{
    m_pSignaturesLB.disposeAndClear();
    m_pHintDocFT.clear();
    m_pHintBasicFT.clear();
    m_pHintPackageFT.clear();
    m_pSigsValidImg.clear();
    m_pSigsValidFI.clear();
    m_pSigsInvalidImg.clear();
    m_pSigsInvalidFI.clear();
    m_pSigsNotvalidatedImg.clear();
    m_pSigsNotvalidatedFI.clear();
    m_pSigsOldSignatureImg.clear();
    m_pSigsOldSignatureFI.clear();
    m_pAdESCompliantCB.clear();
    m_pViewBtn.clear();
    m_pAddBtn.clear();
    m_pRemoveBtn.clear();
    m_pCloseBtn.clear();
    m_pStartCertMgrBtn.clear();
    ModalDialog::dispose();
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
        m_pAdESCompliantCB->Check(m_bAdESCompliant);
        return;
    }

    maSignatureManager.mxStore = rxStore;
    maSignatureManager.maSignatureHelper.SetStorage( maSignatureManager.mxStore, m_sODFVersion);

    Reference < css::packages::manifest::XManifestReader > xReader =
        css::packages::manifest::ManifestReader::create(mxCtx);

    uno::Reference<container::XNameAccess> xNameAccess(rxStore, uno::UNO_QUERY);
    if (!xNameAccess.is())
        return;

    if (xNameAccess->hasByName("META-INF"))
    {
        //Get the manifest.xml
        Reference < css::embed::XStorage > xSubStore(rxStore->openStorageElement(
                    "META-INF", css::embed::ElementModes::READ), UNO_QUERY_THROW);

        Reference< css::io::XInputStream > xStream(
            xSubStore->openStreamElement("manifest.xml", css::embed::ElementModes::READ),
            UNO_QUERY_THROW);

        maSignatureManager.m_manifest = xReader->readManifestSequence(xStream);
    }
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
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(),
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
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(),
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
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                  VclMessageType::Question, VclButtonsType::YesNo,
                                                  XsResId(STR_XMLSECDLG_QUERY_REALLYREMOVE)));
        short nDlgRet = xBox->run();
        bRet = ( nDlgRet == RET_YES );
    }

    return (bRet && canAddRemove());
}

short DigitalSignaturesDialog::Execute()
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

    if (maSignatureManager.maCurrentSignatureInformations.size() > 0)
    {
        // If the document has only SHA-1 signatures we probably want it to stay that way?
    }

    // Only verify once, content will not change.
    // But for refreshing signature information, StartVerifySignatureHdl will be called after each add/remove
    mbVerifySignatures = false;

    return Dialog::Execute();
}

IMPL_LINK_NOARG(DigitalSignaturesDialog, SignatureHighlightHdl, SvTreeListBox*, void)
{
    bool bSel = m_pSignaturesLB->FirstSelected();
    m_pViewBtn->Enable( bSel );
    if ( m_pAddBtn->IsEnabled() ) // not read only
        m_pRemoveBtn->Enable( bSel );
}

IMPL_LINK_NOARG(DigitalSignaturesDialog, OKButtonHdl, Button*, void)
{
    if (mbSignaturesChanged)
        maSignatureManager.write(m_bAdESCompliant);

    EndDialog(RET_OK);
}

IMPL_LINK_NOARG(DigitalSignaturesDialog, SignatureSelectHdl, SvTreeListBox*, bool)
{
    ImplShowSignaturesDetails();
    return false;
}

IMPL_LINK_NOARG(DigitalSignaturesDialog, AdESCompliantCheckBoxHdl, CheckBox&, void)
{
    m_bAdESCompliant = m_pAdESCompliantCB->IsChecked();
}

IMPL_LINK_NOARG(DigitalSignaturesDialog, ViewButtonHdl, Button*, void)
{
    ImplShowSignaturesDetails();
}

IMPL_LINK_NOARG(DigitalSignaturesDialog, AddButtonHdl, Button*, void)
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

        ScopedVclPtrInstance< CertificateChooser > aChooser( this, mxCtx, xSecContexts, UserAction::Sign );
        if ( aChooser->Execute() == RET_OK )
        {
            sal_Int32 nSecurityId;
            if (!maSignatureManager.add(aChooser->GetSelectedCertificates()[0], aChooser->GetSelectedSecurityContext(),
                                        aChooser->GetDescription(), nSecurityId, m_bAdESCompliant))
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
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                  VclMessageType::Error, VclButtonsType::Ok,
                                                  XsResId(STR_XMLSECDLG_SIGNING_FAILED)));
        xBox->run();
        // Don't keep invalid entries...
        ImplGetSignatureInformations(/*bUseTempStream=*/true, /*bCacheLastSignature=*/false);
        ImplFillSignaturesBox();
    }
}

IMPL_LINK_NOARG(DigitalSignaturesDialog, RemoveButtonHdl, Button*, void)
{
    if (!canRemove())
        return;
    if( m_pSignaturesLB->FirstSelected() )
    {
        try
        {
            sal_uInt16 nSelected = static_cast<sal_uInt16>(reinterpret_cast<sal_uIntPtr>( m_pSignaturesLB->FirstSelected()->GetUserData() ));
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

IMPL_STATIC_LINK(DigitalSignaturesDialog, CertMgrButtonHdl, Button*, pButton, void)
{
#ifdef _WIN32
    // FIXME: call GpgME::dirInfo("bindir") somewhere in
    // SecurityEnvironmentGpg or whatnot
    // FIXME: perhaps poke GpgME for uiserver, and hope it returns something useful?
    const OUString aGUIServers[] = { OUString("kleopatra.exe"), OUString("launch-gpa.exe"), OUString("gpa.exe"),
                                     OUString("bin\\kleopatra.exe"), OUString("bin\\launch-gpa.exe"), OUString("bin\\gpa.exe") };
    const char* const cPath = "C:\\Program Files (x86)\\GNU\\GnuPG";
#else
    const OUString aGUIServers[] = { OUString("kleopatra"), OUString("seahorse"),  OUString("gpa"), OUString("kgpg") };
    const char* cPath = getenv("PATH");
#endif

    if (cPath)
    {
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
           std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(pButton->GetFrameWeld(),
                                                         VclMessageType::Info, VclButtonsType::Ok,
                                                         XsResId(STR_XMLSECDLG_NO_CERT_MANAGER)));
           xInfoBox->run();
       }
    }
}

IMPL_LINK_NOARG(DigitalSignaturesDialog, StartVerifySignatureHdl, LinkParamNone*, bool)
{
    return mbVerifySignatures;
}

void DigitalSignaturesDialog::ImplFillSignaturesBox()
{
    m_pSignaturesLB->Clear();

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

            bool bSigValid = false;
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

                aSubject = XmlSec::GetContentPart( xCert->getSubjectName() );
                aIssuer = XmlSec::GetContentPart( xCert->getIssuerName() );
            }
            else if (!rInfo.ouGpgCertificate.isEmpty())
            {
                // In case we don't have the gpg key locally, get some data from the document
                aIssuer = rInfo.ouGpgOwner;
            }

            aDateTimeStr = XmlSec::GetDateTimeString( rInfo.stDateTime );
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

            bSigValid = ( rInfo.nStatus == css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED );

            if ( bSigValid )
            {
                 bSigValid = DocumentSignatureHelper::checkIfAllFilesAreSigned(
                      aElementsToBeVerified, rInfo, mode);

                if( bSigValid )
                    nValidSigs++;
            }

            Image aImage;
            if (!bSigValid)
            {
                aImage = m_pSigsInvalidImg->GetImage();
            }
            else if (bSigValid && !bCertValid)
            {
                aImage = m_pSigsNotvalidatedImg->GetImage();
            }
            //Check if the signature is a "old" document signature, that is, which was created
            //by an version of OOo previous to 3.2
            // If there is no storage, then it's pointless to check storage
            // stream references.
            else if (maSignatureManager.meSignatureMode == DocumentSignatureMode::Content
                && bSigValid && bCertValid && (maSignatureManager.mxStore.is() && !DocumentSignatureHelper::isOOo3_2_Signature(
                maSignatureManager.maCurrentSignatureInformations[n])))
            {
                aImage = m_pSigsNotvalidatedImg->GetImage();
                bAllNewSignatures = false;
            }
            else if (maSignatureManager.meSignatureMode == DocumentSignatureMode::Content
                && bSigValid && bCertValid && DocumentSignatureHelper::isOOo3_2_Signature(
                maSignatureManager.maCurrentSignatureInformations[n]))
            {
                aImage = m_pSigsValidImg->GetImage();
            }
            else if (maSignatureManager.meSignatureMode == DocumentSignatureMode::Macros
                && bSigValid && bCertValid)
            {
                aImage = m_pSigsValidImg->GetImage();
            }

            SvTreeListEntry* pEntry = m_pSignaturesLB->InsertEntry( OUString(), aImage, aImage );
            m_pSignaturesLB->SetEntryText( aSubject, pEntry, 1 );
            m_pSignaturesLB->SetEntryText( aIssuer, pEntry, 2 );
            m_pSignaturesLB->SetEntryText( aDateTimeStr, pEntry, 3 );
            m_pSignaturesLB->SetEntryText(aDescription, pEntry, 4);
            m_pSignaturesLB->SetEntryText(aType, pEntry, 5);
            pEntry->SetUserData( reinterpret_cast<void*>(n) );     // misuse user data as index
        }
    }

    bool bAllSigsValid = (nValidSigs == nInfos);
    bool bAllCertsValid = (nValidCerts == nInfos);
    bool bShowValidState = nInfos && (bAllSigsValid && bAllCertsValid && bAllNewSignatures);

    m_pSigsValidImg->Show( bShowValidState);
    m_pSigsValidFI->Show( bShowValidState );

    bool bShowInvalidState = nInfos && !bAllSigsValid;

    m_pSigsInvalidImg->Show( bShowInvalidState );
    m_pSigsInvalidFI->Show( bShowInvalidState );

    bool bShowNotValidatedState = nInfos && bAllSigsValid && !bAllCertsValid;

    m_pSigsNotvalidatedImg->Show(bShowNotValidatedState);
    m_pSigsNotvalidatedFI->Show(bShowNotValidatedState);

    //bAllNewSignatures is always true if we are not in document mode
    bool bShowOldSignature = nInfos && bAllSigsValid && bAllCertsValid && !bAllNewSignatures;
    m_pSigsOldSignatureImg->Show(bShowOldSignature);
    m_pSigsOldSignatureFI->Show(bShowOldSignature);

    SignatureHighlightHdl( nullptr );
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
    if( m_pSignaturesLB->FirstSelected() )
    {
        sal_uInt16 nSelected = static_cast<sal_uInt16>(reinterpret_cast<sal_uIntPtr>( m_pSignaturesLB->FirstSelected()->GetUserData() ));
        const SignatureInformation& rInfo = maSignatureManager.maCurrentSignatureInformations[ nSelected ];
        uno::Reference<security::XCertificate> xCert = getCertificate(rInfo);

        if ( xCert.is() )
        {
            uno::Reference<xml::crypto::XSecurityEnvironment> xSecEnv = getSecurityEnvironmentForCertificate(xCert);
            ScopedVclPtrInstance<CertificateViewer> aViewer(this, xSecEnv, xCert, false);
            aViewer->Execute();
        }
        else
        {
            std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                          VclMessageType::Info, VclButtonsType::Ok,
                                                          XsResId(STR_XMLSECDLG_NO_CERT_FOUND)));
            xInfoBox->run();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

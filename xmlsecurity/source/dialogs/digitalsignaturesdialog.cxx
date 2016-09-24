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
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/security/NoPasswordException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/security/CertificateValidity.hpp>
#include <com/sun/star/packages/WrongPasswordException.hpp>
#include <com/sun/star/security/SerialNumberAdapter.hpp>
#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/packages/manifest/ManifestReader.hpp>


#include <rtl/ustrbuf.hxx>
#include <rtl/uri.hxx>

#include <tools/date.hxx>
#include <tools/time.hxx>
#include <svtools/treelistentry.hxx>

#include "dialogs.hrc"
#include "digitalsignaturesdialog.hrc"
#include "resourcemanager.hxx"

#include <vcl/layout.hxx>
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

    SaveODFItem::SaveODFItem(): utl::ConfigItem(OUString(
        "Office.Common/Save")), m_nODF(0)
    {
        OUString sDef("ODF/DefaultVersion");
        Sequence< css::uno::Any > aValues = GetProperties( Sequence<OUString>(&sDef,1) );
        if ( aValues.getLength() == 1)
        {
            sal_Int16 nTmp = 0;
            if ( aValues[0] >>= nTmp )
                m_nODF = nTmp;
            else
                throw uno::RuntimeException(
                    OUString("[xmlsecurity]SaveODFItem::SaveODFItem(): Wrong Type!"),
                    nullptr );

        }
        else
            throw uno::RuntimeException(
                OUString("[xmlsecurity] Could not open property Office.Common/Save/ODF/DefaultVersion"),
                nullptr);
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
    get(m_pViewBtn, "view");
    get(m_pAddBtn, "sign");
    get(m_pRemoveBtn, "remove");
    get(m_pCloseBtn, "close");
    get(m_pSigsValidImg, "validimg");
    get(m_pSigsValidFI, "validft");
    get(m_pSigsInvalidImg, "invalidimg");
    get(m_pSigsInvalidFI, "invalidft");
    get(m_pSigsNotvalidatedImg, "notvalidatedimg");
    get(m_pSigsNotvalidatedFI, "notvalidatedft");
    get(m_pSigsOldSignatureImg, "oldsignatureimg");
    get(m_pSigsOldSignatureFI, "oldsignatureft");

    Size aControlSize(275, 109);
    const long nControlWidth = aControlSize.Width();
    aControlSize = LogicToPixel(aControlSize, MAP_APPFONT);
    SvSimpleTableContainer *pSignatures = get<SvSimpleTableContainer>("signatures");
    pSignatures->set_width_request(aControlSize.Width());
    pSignatures->set_height_request(aControlSize.Height());

    m_pSignaturesLB = VclPtr<SvSimpleTable>::Create(*pSignatures);
    // Give the first column 6 percent, try to distribute the rest equally.
    static long aTabs[] = { 5, 0, 6*nControlWidth/100, 30*nControlWidth/100, 54*nControlWidth/100, 78*nControlWidth/100 };
    m_pSignaturesLB->SetTabs(aTabs);

    m_pSignaturesLB->InsertHeaderEntry("\t" + get<FixedText>("signed")->GetText() + "\t"
               + get<FixedText>("issued")->GetText() + "\t" + get<FixedText>("date")->GetText() + "\t"
               + get<FixedText>("description")->GetText());

    mbVerifySignatures = true;
    mbSignaturesChanged = false;

    m_pSignaturesLB->SetSelectHdl( LINK( this, DigitalSignaturesDialog, SignatureHighlightHdl ) );
    m_pSignaturesLB->SetDoubleClickHdl( LINK( this, DigitalSignaturesDialog, SignatureSelectHdl ) );

    m_pViewBtn->SetClickHdl( LINK( this, DigitalSignaturesDialog, ViewButtonHdl ) );
    m_pViewBtn->Disable();

    m_pAddBtn->SetClickHdl( LINK( this, DigitalSignaturesDialog, AddButtonHdl ) );
    if ( bReadOnly  )
        m_pAddBtn->Disable();

    m_pRemoveBtn->SetClickHdl( LINK( this, DigitalSignaturesDialog, RemoveButtonHdl ) );
    m_pRemoveBtn->Disable();

    m_pCloseBtn->SetClickHdl( LINK( this, DigitalSignaturesDialog, OKButtonHdl) );

    switch( maSignatureManager.meSignatureMode )
    {
        case SignatureModeDocumentContent:  m_pHintDocFT->Show();     break;
        case SignatureModeMacros:           m_pHintBasicFT->Show();   break;
        case SignatureModePackage:          m_pHintPackageFT->Show(); break;
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
    m_pViewBtn.clear();
    m_pAddBtn.clear();
    m_pRemoveBtn.clear();
    m_pCloseBtn.clear();
    ModalDialog::dispose();
}

bool DigitalSignaturesDialog::Init()
{
    bool bInit = maSignatureManager.maSignatureHelper.Init();

    SAL_WARN_IF( !bInit, "xmlsecurity.dialogs", "Error initializing security context!" );

    if ( bInit )
    {
        maSignatureManager.maSignatureHelper.SetStartVerifySignatureHdl( LINK( this, DigitalSignaturesDialog, StartVerifySignatureHdl ) );
    }

    return bInit;
}

void DigitalSignaturesDialog::SetStorage( const css::uno::Reference < css::embed::XStorage >& rxStore )
{
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
    //m56
    bool ret = true;
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
        ScopedVclPtrInstance< MessageDialog > err(nullptr, XMLSEC_RES(STR_XMLSECDLG_OLD_ODF_FORMAT));
        err->Execute();
        ret = false;
    }

    //As of OOo 3.2 the document signature includes in macrosignatures.xml. That is
    //adding a macro signature will break an existing document signature.
    //The sfx2 will remove the documentsignature when the user adds a macro signature
    if (maSignatureManager.meSignatureMode == SignatureModeMacros
        && ret)
    {
        if (m_bHasDocumentSignature && !m_bWarningShowSignMacro)
        {
            //The warning says that the document signatures will be removed if the user
            //continues. He can then either press 'OK' or 'NO'
            //It the user presses 'Add' or 'Remove' several times then, then the warning
            //is shown every time until the user presses 'OK'. From then on, the warning
            //is not displayed anymore as long as the signatures dialog is alive.
            if (ScopedVclPtrInstance<MessageDialog>(
                  nullptr, XMLSEC_RES(STR_XMLSECDLG_QUERY_REMOVEDOCSIGNBEFORESIGN), VclMessageType::Question, VCL_BUTTONS_YES_NO)->Execute() == RET_NO)
                ret = false;
            else
                m_bWarningShowSignMacro = true;

        }
    }
    return ret;
}

bool DigitalSignaturesDialog::canAdd()
{
    if (canAddRemove())
        return true;
    return false;
}

bool DigitalSignaturesDialog::canRemove()
{
    if (canAddRemove())
        return true;
    return false;
}

short DigitalSignaturesDialog::Execute()
{
    // Verify Signatures and add certificates to ListBox...
    mbVerifySignatures = true;
    ImplGetSignatureInformations(false);
    ImplFillSignaturesBox();

    // Only verify once, content will not change.
    // But for refreshing signature information, StartVerifySignatureHdl will be called after each add/remove
    mbVerifySignatures = false;

    return Dialog::Execute();
}

IMPL_LINK_NOARG_TYPED(DigitalSignaturesDialog, SignatureHighlightHdl, SvTreeListBox*, void)
{
    bool bSel = m_pSignaturesLB->FirstSelected();
    m_pViewBtn->Enable( bSel );
    if ( m_pAddBtn->IsEnabled() ) // not read only
        m_pRemoveBtn->Enable( bSel );
}

IMPL_LINK_NOARG_TYPED(DigitalSignaturesDialog, OKButtonHdl, Button*, void)
{
    maSignatureManager.write();

    EndDialog(RET_OK);
}

IMPL_LINK_NOARG_TYPED(DigitalSignaturesDialog, SignatureSelectHdl, SvTreeListBox*, bool)
{
    ImplShowSignaturesDetails();
    return false;
}

IMPL_LINK_NOARG_TYPED(DigitalSignaturesDialog, ViewButtonHdl, Button*, void)
{
    ImplShowSignaturesDetails();
}

IMPL_LINK_NOARG_TYPED(DigitalSignaturesDialog, AddButtonHdl, Button*, void)
{
    if( ! canAdd())
        return;
    try
    {
        uno::Reference<css::xml::crypto::XSecurityEnvironment> xSecEnv = maSignatureManager.maSignatureHelper.GetSecurityEnvironment();

        ScopedVclPtrInstance< CertificateChooser > aChooser( this, mxCtx, xSecEnv );
        if ( aChooser->Execute() == RET_OK )
        {
            sal_Int32 nSecurityId;
            if (!maSignatureManager.add(aChooser->GetSelectedCertificate(), aChooser->GetDescription(), nSecurityId))
                return;
            mbSignaturesChanged = true;

            sal_Int32 nStatus = maSignatureManager.maSignatureHelper.GetSignatureInformation( nSecurityId ).nStatus;

            if ( nStatus == css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED )
            {
                mbSignaturesChanged = true;

                // Can't simply remember current information, need parsing for getting full information :(
                // We need to verify the signatures again, otherwise the status in the signature information
                // will not contain
                // SecurityOperationStatus_OPERATION_SUCCEEDED
                mbVerifySignatures = true;
                ImplGetSignatureInformations(true, /*bCacheLastSignature=*/false);
                ImplFillSignaturesBox();
            }
        }
    }
    catch ( uno::Exception& )
    {
        OSL_FAIL( "Exception while adding a signature!" );
        // Don't keep invalid entries...
        ImplGetSignatureInformations(true, /*bCacheLastSignature=*/false);
        ImplFillSignaturesBox();
    }
}

IMPL_LINK_NOARG_TYPED(DigitalSignaturesDialog, RemoveButtonHdl, Button*, void)
{
    if (!canRemove())
        return;
    if( m_pSignaturesLB->FirstSelected() )
    {
        try
        {
            sal_uInt16 nSelected = (sal_uInt16) reinterpret_cast<sal_uIntPtr>( m_pSignaturesLB->FirstSelected()->GetUserData() );
            maSignatureManager.remove(nSelected);

            mbSignaturesChanged = true;

            ImplFillSignaturesBox();
        }
        catch ( uno::Exception& )
        {
            OSL_FAIL( "Exception while removing a signature!" );
            // Don't keep invalid entries...
            ImplGetSignatureInformations(true);
            ImplFillSignaturesBox();
        }
    }
}

IMPL_LINK_NOARG_TYPED(DigitalSignaturesDialog, StartVerifySignatureHdl, LinkParamNone*, bool)
{
    return mbVerifySignatures;
}

void DigitalSignaturesDialog::ImplFillSignaturesBox()
{
    m_pSignaturesLB->Clear();

    uno::Reference< css::xml::crypto::XSecurityEnvironment > xSecEnv = maSignatureManager.maSignatureHelper.GetSecurityEnvironment();
    uno::Reference<css::security::XSerialNumberAdapter> xSerialNumberAdapter =
        css::security::SerialNumberAdapter::create(mxCtx);

    uno::Reference< css::security::XCertificate > xCert;

    size_t nInfos = maSignatureManager.maCurrentSignatureInformations.size();
    size_t nValidSigs = 0, nValidCerts = 0;
    bool bAllNewSignatures = true;

    if( nInfos )
    {
        for( size_t n = 0; n < nInfos; ++n )
        {
            DocumentSignatureAlgorithm mode = DocumentSignatureHelper::getDocumentAlgorithm(
                m_sODFVersion, maSignatureManager.maCurrentSignatureInformations[n]);
            std::vector< OUString > aElementsToBeVerified =
                DocumentSignatureHelper::CreateElementList(
                maSignatureManager.mxStore, maSignatureManager.meSignatureMode, mode);

            const SignatureInformation& rInfo = maSignatureManager.maCurrentSignatureInformations[n];
            //First we try to get the certificate which is embedded in the XML Signature
            if (!rInfo.ouX509Certificate.isEmpty())
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
            //Todo: This probably could be removed, see above.
            if (!xCert.is())
                xCert = xSecEnv->getCertificate( rInfo.ouX509IssuerName, xSerialNumberAdapter->toSequence( rInfo.ouX509SerialNumber ) );

            SAL_WARN_IF( !xCert.is(), "xmlsecurity.dialogs", "Certificate not found and can't be created!" );

            OUString aSubject;
            OUString aIssuer;
            OUString aDateTimeStr;
            OUString aDescription;

            bool bSigValid = false;
            bool bCertValid = false;
            if( xCert.is() )
            {
                //check the validity of the cert
                try {
                    sal_Int32 certResult = xSecEnv->verifyCertificate(xCert,
                        Sequence<css::uno::Reference<css::security::XCertificate> >());

                    bCertValid = certResult == css::security::CertificateValidity::VALID;
                    if ( bCertValid )
                        nValidCerts++;

                } catch (css::uno::SecurityException& ) {
                    OSL_FAIL("Verification of certificate failed");
                    bCertValid = false;
                }

                aSubject = XmlSec::GetContentPart( xCert->getSubjectName() );
                aIssuer = XmlSec::GetContentPart( xCert->getIssuerName() );
                // String with date and time information (#i20172#)
                aDateTimeStr = XmlSec::GetDateTimeString( rInfo.stDateTime );
                aDescription = rInfo.ouDescription;
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
            else if (maSignatureManager.meSignatureMode == SignatureModeDocumentContent
                && bSigValid && bCertValid && !DocumentSignatureHelper::isOOo3_2_Signature(
                maSignatureManager.maCurrentSignatureInformations[n]))
            {
                aImage = m_pSigsNotvalidatedImg->GetImage();
                bAllNewSignatures &= false;
            }
            else if (maSignatureManager.meSignatureMode == SignatureModeDocumentContent
                && bSigValid && bCertValid && DocumentSignatureHelper::isOOo3_2_Signature(
                maSignatureManager.maCurrentSignatureInformations[n]))
            {
                aImage = m_pSigsValidImg->GetImage();
            }
            else if (maSignatureManager.meSignatureMode == SignatureModeMacros
                && bSigValid && bCertValid)
            {
                aImage = m_pSigsValidImg->GetImage();
            }

            SvTreeListEntry* pEntry = m_pSignaturesLB->InsertEntry( OUString(), aImage, aImage );
            m_pSignaturesLB->SetEntryText( aSubject, pEntry, 1 );
            m_pSignaturesLB->SetEntryText( aIssuer, pEntry, 2 );
            m_pSignaturesLB->SetEntryText( aDateTimeStr, pEntry, 3 );
            m_pSignaturesLB->SetEntryText(aDescription, pEntry, 4);
            pEntry->SetUserData( reinterpret_cast<void*>(n) );     // missuse user data as index
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
        sal_uInt16 nSelected = (sal_uInt16) reinterpret_cast<sal_uIntPtr>( m_pSignaturesLB->FirstSelected()->GetUserData() );
        const SignatureInformation& rInfo = maSignatureManager.maCurrentSignatureInformations[ nSelected ];
        css::uno::Reference<css::xml::crypto::XSecurityEnvironment > xSecEnv =
            maSignatureManager.maSignatureHelper.GetSecurityEnvironment();
        css::uno::Reference<com::sun::star::security::XSerialNumberAdapter> xSerialNumberAdapter =
            css::security::SerialNumberAdapter::create(mxCtx);
        // Use Certificate from doc, not from key store
        uno::Reference< css::security::XCertificate > xCert;
        if (!rInfo.ouX509Certificate.isEmpty())
            xCert = xSecEnv->createCertificateFromAscii(rInfo.ouX509Certificate);
        //fallback if no certificate is embedded, get if from store
        if (!xCert.is())
            xCert = xSecEnv->getCertificate( rInfo.ouX509IssuerName, xSerialNumberAdapter->toSequence( rInfo.ouX509SerialNumber ) );

        SAL_WARN_IF( !xCert.is(), "xmlsecurity.dialogs", "Error getting Certificate!" );
        if ( xCert.is() )
        {
            ScopedVclPtrInstance< CertificateViewer > aViewer( this, maSignatureManager.maSignatureHelper.GetSecurityEnvironment(), xCert, false );
            aViewer->Execute();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <sal/config.h>

#include <string_view>

#include <certificatechooser.hxx>
#include <certificateviewer.hxx>
#include <com/sun/star/security/XCertificate.hpp>

#include <com/sun/star/security/CertificateCharacters.hpp>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/security/CertificateValidity.hpp>

#include <unotools/localedatawrapper.hxx>
#include <unotools/datetime.hxx>

#include <strings.hrc>
#include <resourcemanager.hxx>
#include <comphelper/xmlsechelper.hxx>
#include <tools/datetime.hxx>
#include <bitmaps.hlst>

#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

using namespace comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

CertificateViewer::CertificateViewer(weld::Window* _pParent,
        const css::uno::Reference< css::xml::crypto::XSecurityEnvironment >& _rxSecurityEnvironment,
        const css::uno::Reference< css::security::XCertificate >& _rXCert, bool bCheckForPrivateKey,
        CertificateChooser* pParentChooser)
    : GenericDialogController(_pParent, u"xmlsec/ui/viewcertdialog.ui"_ustr, u"ViewCertDialog"_ustr)
    , mbCheckForPrivateKey(bCheckForPrivateKey)
    , mpParentChooser(pParentChooser)
    , mxTabCtrl(m_xBuilder->weld_notebook(u"tabcontrol"_ustr))
{
    mxTabCtrl->connect_enter_page(LINK(this, CertificateViewer, ActivatePageHdl));

    mxSecurityEnvironment = _rxSecurityEnvironment;
    mxCert = _rXCert;

    mxGeneralPage.reset(new CertificateViewerGeneralTP(mxTabCtrl->get_page(u"general"_ustr), this));
    mxDetailsPage.reset(new CertificateViewerDetailsTP(mxTabCtrl->get_page(u"details"_ustr), this));
    if (!mxSecurityEnvironment->buildCertificatePath(mxCert).hasElements())
        mxTabCtrl->remove_page(u"path"_ustr);
    else
        mxPathId.reset(new CertificateViewerCertPathTP(mxTabCtrl->get_page(u"path"_ustr), this));
    mxTabCtrl->set_current_page(u"general"_ustr);
}

IMPL_LINK(CertificateViewer, ActivatePageHdl, const OUString&, rPage, void)
{
    if (rPage == "path")
        mxPathId->ActivatePage();
}

CertificateViewerTP::CertificateViewerTP(weld::Container* pParent, const OUString& rUIXMLDescription,
                                         const OUString& rID, CertificateViewer* pDlg)
    : mxBuilder(Application::CreateBuilder(pParent, rUIXMLDescription))
    , mxContainer(mxBuilder->weld_container(rID))
    , mpDlg(pDlg)
{
}

CertificateViewerGeneralTP::CertificateViewerGeneralTP(weld::Container* pParent, CertificateViewer* pDlg)
    : CertificateViewerTP(pParent, u"xmlsec/ui/certgeneral.ui"_ustr, u"CertGeneral"_ustr, pDlg)
    , m_xCertImg(mxBuilder->weld_image(u"certimage"_ustr))
    , m_xHintNotTrustedFT(mxBuilder->weld_label(u"hintnotrust"_ustr))
    , m_xIssuedToLabelFT(mxBuilder->weld_label(u"issued_to"_ustr))
    , m_xIssuedToFT(mxBuilder->weld_label(u"issued_to_value"_ustr))
    , m_xIssuedByLabelFT(mxBuilder->weld_label(u"issued_by"_ustr))
    , m_xIssuedByFT(mxBuilder->weld_label(u"issued_by_value"_ustr))
    , m_xValidFromDateFT(mxBuilder->weld_label(u"valid_from_value"_ustr))
    , m_xValidToDateFT(mxBuilder->weld_label(u"valid_to_value"_ustr))
    , m_xKeyTypeFT(mxBuilder->weld_label(u"key_type_value"_ustr))
    , m_xKeyImg(mxBuilder->weld_image(u"keyimage"_ustr))
    , m_xHintCorrespPrivKeyFT(mxBuilder->weld_label(u"privatekey"_ustr))
{
    //Verify the certificate
    sal_Int32 certStatus = mpDlg->mxSecurityEnvironment->verifyCertificate(mpDlg->mxCert,
         Sequence<Reference<css::security::XCertificate> >());

    bool bCertValid = certStatus == css::security::CertificateValidity::VALID;

    if ( !bCertValid )
    {
        m_xCertImg->set_from_icon_name(BMP_STATE_NOT_VALIDATED);
        m_xHintNotTrustedFT->set_label(XsResId(STR_CERTIFICATE_NOT_VALIDATED));
    }

    // insert data
    css::uno::Reference< css::security::XCertificate > xCert = mpDlg->mxCert;

    OUString sSubjectName(xmlsec::GetContentPart(xCert->getSubjectName(), xCert->getCertificateKind()));
    if (!sSubjectName.isEmpty())
        m_xIssuedToFT->set_label(sSubjectName);
    else
        m_xIssuedToLabelFT->hide();
    OUString sIssuerName(xmlsec::GetContentPart(xCert->getIssuerName(), xCert->getCertificateKind()));
    if (!sIssuerName.isEmpty())
        m_xIssuedByFT->set_label(sIssuerName);
    else
        m_xIssuedByLabelFT->hide();

    DateTime aDateTimeStart( DateTime::EMPTY );
    DateTime aDateTimeEnd( DateTime::EMPTY );
    utl::typeConvert( xCert->getNotValidBefore(), aDateTimeStart );
    utl::typeConvert( xCert->getNotValidAfter(), aDateTimeEnd );

    OUString sValidFromDate = Application::GetSettings().GetUILocaleDataWrapper().getDate(Date(aDateTimeStart.GetDate()));
    OUString sValidToDate = Application::GetSettings().GetUILocaleDataWrapper().getDate(Date(aDateTimeEnd.GetDate()));

    m_xValidFromDateFT->set_label(sValidFromDate);
    m_xValidToDateFT->set_label(sValidToDate);
    m_xKeyTypeFT->set_label(xmlsec::GetCertificateKind(xCert->getCertificateKind()));

    // Check if we have the private key...
    bool bHasPrivateKey = false;
    // #i41270# Check only if we have that certificate in our security environment
    if (pDlg->mbCheckForPrivateKey)
    {
        tools::Long nCertificateCharacters = pDlg->mxSecurityEnvironment->getCertificateCharacters(xCert);
        bHasPrivateKey = (nCertificateCharacters & security::CertificateCharacters::HAS_PRIVATE_KEY);
    }
    if (!bHasPrivateKey)
    {
        m_xKeyImg->hide();
        m_xHintCorrespPrivKeyFT->hide();
    }
}

void CertificateViewerDetailsTP::InsertElement(const OUString& rField, const OUString& rValue,
                                               const OUString& rDetails, bool bFixedWidthFont)
{
    m_aUserData.emplace_back(std::make_unique<Details_UserDatat>(rDetails, bFixedWidthFont));
    OUString sId(weld::toId(m_aUserData.back().get()));
    m_xElementsLB->append(sId, rField);
    m_xElementsLB->set_text(m_xElementsLB->n_children() -1, rValue, 1);
}

// X.509 + GPG
CertificateViewerDetailsTP::CertificateViewerDetailsTP(weld::Container* pParent, CertificateViewer* pDlg)
    : CertificateViewerTP(pParent, u"xmlsec/ui/certdetails.ui"_ustr, u"CertDetails"_ustr, pDlg)
    , m_xElementsLB(mxBuilder->weld_tree_view(u"tablecontainer"_ustr))
    , m_xValueDetails(mxBuilder->weld_text_view(u"valuedetails"_ustr))
{
    const int nWidth = m_xElementsLB->get_approximate_digit_width() * 100;
    const int nHeight = m_xElementsLB->get_height_rows(14);
    m_xElementsLB->set_size_request(nWidth, nHeight);
    m_xValueDetails->set_size_request(nWidth, nHeight);
    m_xValueDetails->set_editable(false);
    m_xElementsLB->set_column_fixed_widths( { nWidth / 2 } );

    // fill list box
    Reference< security::XCertificate > xCert = mpDlg->mxCert;
    sal_uInt16                  nLineBreak = 16;
    const char* const pHexSep = " ";
    OUString                aLBEntry;
    OUString                aDetails;
    // Certificate Versions are reported wrong (#i35107#) - 0 == "V1", 1 == "V2", ..., n = "V(n+1)"
    aLBEntry = "V" + OUString::number( xCert->getVersion() + 1 );
    InsertElement( XsResId( STR_VERSION ), aLBEntry, aLBEntry );
    Sequence< sal_Int8 >    aSeq = xCert->getSerialNumber();
    aLBEntry = xmlsec::GetHexString( aSeq, pHexSep );
    aDetails = xmlsec::GetHexString( aSeq, pHexSep, nLineBreak );
    InsertElement( XsResId( STR_SERIALNUM ), aLBEntry, aDetails, true );

    std::pair< OUString, OUString> pairIssuer =
        xmlsec::GetDNForCertDetailsView(xCert->getIssuerName());
    aLBEntry = pairIssuer.first;
    aDetails = pairIssuer.second;
    InsertElement( XsResId( STR_ISSUER ), aLBEntry, aDetails );

    DateTime aDateTime( DateTime::EMPTY );
    utl::typeConvert( xCert->getNotValidBefore(), aDateTime );
    aLBEntry = Application::GetSettings().GetUILocaleDataWrapper().getDate(Date(aDateTime.GetDate())) + " ";
    aLBEntry += Application::GetSettings().GetUILocaleDataWrapper().getTime(tools::Time(aDateTime.GetTime()));
    InsertElement( XsResId( STR_VALIDFROM ), aLBEntry, aLBEntry  );
    utl::typeConvert( xCert->getNotValidAfter(), aDateTime );
    aLBEntry = Application::GetSettings().GetUILocaleDataWrapper().getDate(Date(aDateTime.GetDate()) ) + " ";
    aLBEntry += Application::GetSettings().GetUILocaleDataWrapper().getTime(tools::Time(aDateTime.GetTime()));
    InsertElement( XsResId( STR_VALIDTO ), aLBEntry, aLBEntry );

    std::pair< OUString, OUString > pairSubject =
        xmlsec::GetDNForCertDetailsView(xCert->getSubjectName());
    aLBEntry = pairSubject.first;
    aDetails = pairSubject.second;
    InsertElement( XsResId( STR_SUBJECT ), aLBEntry, aDetails );

    aLBEntry = aDetails = xCert->getSubjectPublicKeyAlgorithm();
    InsertElement( XsResId( STR_SUBJECT_PUBKEY_ALGO ), aLBEntry, aDetails );
    aSeq = xCert->getSubjectPublicKeyValue();
    aLBEntry = xmlsec::GetHexString( aSeq, pHexSep );
    aDetails = xmlsec::GetHexString( aSeq, pHexSep, nLineBreak );
    InsertElement( XsResId( STR_SUBJECT_PUBKEY_VAL ), aLBEntry, aDetails, true );

    aLBEntry = aDetails = xCert->getSignatureAlgorithm();
    InsertElement( XsResId( STR_SIGNATURE_ALGO ), aLBEntry, aDetails );

    CertificateChooser* pChooser = mpDlg->GetParentChooser();
    if (pChooser)
    {
        aLBEntry = CertificateChooser::UsageInClearText( mpDlg->mxCert->getCertificateUsage() );
        InsertElement( XsResId( STR_USE ), aLBEntry, aLBEntry );
    }

    aSeq = xCert->getSHA1Thumbprint();
    aLBEntry = xmlsec::GetHexString( aSeq, pHexSep );
    aDetails = xmlsec::GetHexString( aSeq, pHexSep, nLineBreak );
    InsertElement( XsResId( STR_THUMBPRINT_SHA1 ), aLBEntry, aDetails, true );

    aSeq = xCert->getMD5Thumbprint();
    aLBEntry = xmlsec::GetHexString( aSeq, pHexSep );
    aDetails = xmlsec::GetHexString( aSeq, pHexSep, nLineBreak );
    InsertElement( XsResId( STR_THUMBPRINT_MD5 ), aLBEntry, aDetails, true );

    m_xElementsLB->columns_autosize();
    m_xElementsLB->connect_changed(LINK(this, CertificateViewerDetailsTP, ElementSelectHdl));
}

IMPL_LINK_NOARG(CertificateViewerDetailsTP, ElementSelectHdl, weld::TreeView&, void)
{
    int nEntry = m_xElementsLB->get_selected_index();
    OUString aElementText;
    bool bFixedWidthFont;
    if (nEntry != -1)
    {
        const Details_UserDatat* p = weld::fromId<Details_UserDatat*>(m_xElementsLB->get_id(nEntry));
        aElementText = p->maTxt;
        bFixedWidthFont = p->mbFixedWidthFont;
    }
    else
        bFixedWidthFont = false;

    m_xValueDetails->set_monospace(bFixedWidthFont);
    m_xValueDetails->set_text(aElementText);
}

// X.509
CertificateViewerCertPathTP::CertificateViewerCertPathTP(weld::Container* pParent, CertificateViewer* pDlg)
    : CertificateViewerTP(pParent, u"xmlsec/ui/certpage.ui"_ustr, u"CertPage"_ustr, pDlg)
    , mpParent(pDlg)
    , mbFirstActivateDone(false)
    , mxCertPathLB(mxBuilder->weld_tree_view(u"signatures"_ustr))
    , mxScratchIter(mxCertPathLB->make_iterator())
    , mxViewCertPB(mxBuilder->weld_button(u"viewcert"_ustr))
    , mxCertStatusML(mxBuilder->weld_text_view(u"status"_ustr))
    , mxCertOK(mxBuilder->weld_label(u"certok"_ustr))
    , mxCertNotValidated(mxBuilder->weld_label(u"certnotok"_ustr))
{
    const int nWidth = mxCertPathLB->get_approximate_digit_width() * 100;
    const int nHeight = mxCertPathLB->get_height_rows(6);
    mxCertPathLB->set_size_request(nWidth, nHeight);
    mxCertStatusML->set_size_request(nWidth, nHeight);
    mxCertStatusML->set_editable(false);

    mxCertPathLB->columns_autosize();
    mxCertPathLB->connect_changed( LINK( this, CertificateViewerCertPathTP, CertSelectHdl ) );
    mxViewCertPB->connect_clicked( LINK( this, CertificateViewerCertPathTP, ViewCertHdl ) );
}

CertificateViewerCertPathTP::~CertificateViewerCertPathTP()
{
    if (mxCertificateViewer)
        mxCertificateViewer->response(RET_OK);
}

void CertificateViewerCertPathTP::ActivatePage()
{
    if ( mbFirstActivateDone )
        return;

    mbFirstActivateDone = true;
    Sequence< Reference< security::XCertificate > > aCertPath =
        mpParent->mxSecurityEnvironment->buildCertificatePath( mpParent->mxCert );
    const Reference< security::XCertificate >* pCertPath = aCertPath.getConstArray();

    sal_Int32 i, nCnt = aCertPath.getLength();
    std::unique_ptr<weld::TreeIter> xParent;
    for (i = nCnt-1; i >= 0; i--)
    {
        const Reference< security::XCertificate > rCert = pCertPath[ i ];
        OUString sName = xmlsec::GetContentPart( rCert->getSubjectName(), rCert->getCertificateKind() );
        //Verify the certificate
        sal_Int32 certStatus = mpDlg->mxSecurityEnvironment->verifyCertificate(rCert,
             Sequence<Reference<css::security::XCertificate> >());
        bool bCertValid = certStatus == css::security::CertificateValidity::VALID;
        InsertCert(xParent.get(), sName, rCert, bCertValid);
        if (!xParent)
        {
            xParent = mxCertPathLB->make_iterator();
            (void)mxCertPathLB->get_iter_first(*xParent);
        }
        else
        {
            (void)mxCertPathLB->iter_children(*xParent);
        }
    }

    if (xParent)
        mxCertPathLB->select(*xParent);
    mxViewCertPB->set_sensitive(false); // Own certificate selected

    while (xParent)
    {
        mxCertPathLB->expand_row(*xParent);
        if (!mxCertPathLB->iter_parent(*xParent))
            xParent.reset();
    }

    CertSelectHdl(*mxCertPathLB);
}

IMPL_LINK_NOARG(CertificateViewerCertPathTP, ViewCertHdl, weld::Button&, void)
{
    std::unique_ptr<weld::TreeIter> xIter = mxCertPathLB->make_iterator();
    if (mxCertPathLB->get_selected(xIter.get()))
    {
        if (mxCertificateViewer)
            mxCertificateViewer->response(RET_OK);

        CertPath_UserData* pData = weld::fromId<CertPath_UserData*>(mxCertPathLB->get_id(*xIter));
        mxCertificateViewer = std::make_shared<CertificateViewer>(mpDlg->getDialog(), mpDlg->mxSecurityEnvironment,
                pData->mxCert, false, nullptr);
        weld::DialogController::runAsync(mxCertificateViewer, [this] (sal_Int32) { mxCertificateViewer = nullptr; });
    }
}

IMPL_LINK_NOARG(CertificateViewerCertPathTP, CertSelectHdl, weld::TreeView&, void)
{
    OUString sStatus;

    std::unique_ptr<weld::TreeIter> xIter = mxCertPathLB->make_iterator();
    bool bEntry = mxCertPathLB->get_selected(xIter.get());
    if (bEntry)
    {
        CertPath_UserData* pData = weld::fromId<CertPath_UserData*>(mxCertPathLB->get_id(*xIter));
        if (pData)
            sStatus = pData->mbValid ? mxCertOK->get_label() : mxCertNotValidated->get_label();
    }

    mxCertStatusML->set_text(sStatus);

    bool bSensitive = false;
    if (bEntry)
    {
        // if has children, so not the last one in the chain
        if (mxCertPathLB->iter_children(*xIter))
            bSensitive = true;
    }
    mxViewCertPB->set_sensitive(bSensitive);
}

void CertificateViewerCertPathTP::InsertCert(const weld::TreeIter* pParent, const OUString& rName,
                                             const css::uno::Reference< css::security::XCertificate >& rxCert,
                                             bool bValid)
{
    auto const sImage = bValid ? std::u16string_view(u"" BMP_CERT_OK) : std::u16string_view(u"" BMP_CERT_NOT_OK);
    maUserData.emplace_back(std::make_unique<CertPath_UserData>(rxCert, bValid));
    OUString sId(weld::toId(maUserData.back().get()));
    mxCertPathLB->insert(pParent, -1, &rName, &sId, nullptr, nullptr, false, mxScratchIter.get());
    mxCertPathLB->set_image(*mxScratchIter, OUString(sImage));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

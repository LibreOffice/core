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
    : GenericDialogController(_pParent, "xmlsec/ui/viewcertdialog.ui", "ViewCertDialog")
    , mbCheckForPrivateKey(bCheckForPrivateKey)
    , mpParentChooser(pParentChooser)
    , mxTabCtrl(m_xBuilder->weld_notebook("tabcontrol"))
{
    mxTabCtrl->connect_enter_page(LINK(this, CertificateViewer, ActivatePageHdl));

    mxSecurityEnvironment = _rxSecurityEnvironment;
    mxCert = _rXCert;

    mxGeneralPage.reset(new CertificateViewerGeneralTP(mxTabCtrl->get_page("general"), this));
    mxDetailsPage.reset(new CertificateViewerDetailsTP(mxTabCtrl->get_page("details"), this));
    if (mxSecurityEnvironment->buildCertificatePath(mxCert).getLength() == 0)
        mxTabCtrl->remove_page("path");
    else
        mxPathId.reset(new CertificateViewerCertPathTP(mxTabCtrl->get_page("path"), this));
    mxTabCtrl->set_current_page("general");
}

IMPL_LINK(CertificateViewer, ActivatePageHdl, const OString&, rPage, void)
{
    if (rPage == "path")
        mxPathId->ActivatePage();
}

CertificateViewerTP::CertificateViewerTP(weld::Container* pParent, const OUString& rUIXMLDescription,
                                         const OString& rID, CertificateViewer* pDlg)
    : mxBuilder(Application::CreateBuilder(pParent, rUIXMLDescription))
    , mxContainer(mxBuilder->weld_container(rID))
    , mpDlg(pDlg)
{
}

CertificateViewerGeneralTP::CertificateViewerGeneralTP(weld::Container* pParent, CertificateViewer* pDlg)
    : CertificateViewerTP(pParent, "xmlsec/ui/certgeneral.ui", "CertGeneral", pDlg)
    , m_xCertImg(mxBuilder->weld_image("certimage"))
    , m_xHintNotTrustedFT(mxBuilder->weld_label("hintnotrust"))
    , m_xIssuedToLabelFT(mxBuilder->weld_label("issued_to"))
    , m_xIssuedToFT(mxBuilder->weld_label("issued_to_value"))
    , m_xIssuedByLabelFT(mxBuilder->weld_label("issued_by"))
    , m_xIssuedByFT(mxBuilder->weld_label("issued_by_value"))
    , m_xValidFromDateFT(mxBuilder->weld_label("valid_from_value"))
    , m_xValidToDateFT(mxBuilder->weld_label("valid_to_value"))
    , m_xKeyImg(mxBuilder->weld_image("keyimage"))
    , m_xHintCorrespPrivKeyFT(mxBuilder->weld_label("privatekey"))
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

    OUString sSubjectName(xmlsec::GetContentPart(xCert->getSubjectName()));
    if (!sSubjectName.isEmpty())
        m_xIssuedToFT->set_label(sSubjectName);
    else
        m_xIssuedToLabelFT->hide();
    OUString sIssuerName(xmlsec::GetContentPart(xCert->getIssuerName()));
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

    // Check if we have the private key...
    bool bHasPrivateKey = false;
    // #i41270# Check only if we have that certificate in our security environment
    if (pDlg->mbCheckForPrivateKey)
    {
        long nCertificateCharacters = pDlg->mxSecurityEnvironment->getCertificateCharacters(xCert);
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
    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(m_aUserData.back().get())));
    m_xElementsLB->append(sId, rField);
    m_xElementsLB->set_text(m_xElementsLB->n_children() -1, rValue, 1);
}

CertificateViewerDetailsTP::CertificateViewerDetailsTP(weld::Container* pParent, CertificateViewer* pDlg)
    : CertificateViewerTP(pParent, "xmlsec/ui/certdetails.ui", "CertDetails", pDlg)
    , m_xElementsLB(mxBuilder->weld_tree_view("tablecontainer"))
    , m_xValueDetails(mxBuilder->weld_text_view("valuedetails"))
{
    const int nWidth = m_xElementsLB->get_approximate_digit_width() * 60;
    const int nHeight = m_xElementsLB->get_height_rows(8);
    m_xElementsLB->set_size_request(nWidth, nHeight);
    m_xValueDetails->set_size_request(nWidth, nHeight);
    std::vector<int> aWidths;
    aWidths.push_back(nWidth / 2);
    m_xElementsLB->set_column_fixed_widths(aWidths);

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
    aLBEntry = Application::GetSettings().GetUILocaleDataWrapper().getDate(Date(aDateTime.GetDate()));
    aLBEntry += " ";
    aLBEntry += Application::GetSettings().GetUILocaleDataWrapper().getTime(tools::Time(aDateTime.GetTime()));
    InsertElement( XsResId( STR_VALIDFROM ), aLBEntry, aLBEntry  );
    utl::typeConvert( xCert->getNotValidAfter(), aDateTime );
    aLBEntry = Application::GetSettings().GetUILocaleDataWrapper().getDate(Date(aDateTime.GetDate()) );
    aLBEntry += " ";
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

    m_xElementsLB->connect_changed(LINK(this, CertificateViewerDetailsTP, ElementSelectHdl));
}

IMPL_LINK_NOARG(CertificateViewerDetailsTP, ElementSelectHdl, weld::TreeView&, void)
{
    int nEntry = m_xElementsLB->get_selected_index();
    OUString aElementText;
    bool bFixedWidthFont;
    if (nEntry != -1)
    {
        const Details_UserDatat* p = reinterpret_cast<Details_UserDatat*>(m_xElementsLB->get_id(nEntry).toInt64());
        aElementText = p->maTxt;
        bFixedWidthFont = p->mbFixedWidthFont;
    }
    else
        bFixedWidthFont = false;

    m_xValueDetails->set_monospace(bFixedWidthFont);
    m_xValueDetails->set_text(aElementText);
}

CertificateViewerCertPathTP::CertificateViewerCertPathTP(weld::Container* pParent, CertificateViewer* pDlg)
    : CertificateViewerTP(pParent, "xmlsec/ui/certpage.ui", "CertPage", pDlg)
    , mpParent(pDlg)
    , mbFirstActivateDone(false)
    , mxCertPathLB(mxBuilder->weld_tree_view("signatures"))
    , mxViewCertPB(mxBuilder->weld_button("viewcert"))
    , mxCertStatusML(mxBuilder->weld_text_view("status"))
    , mxCertOK(mxBuilder->weld_label("certok"))
    , mxCertNotValidated(mxBuilder->weld_label("certnotok"))
{
    const int nWidth = mxCertPathLB->get_approximate_digit_width() * 60;
    const int nHeight = mxCertPathLB->get_height_rows(6);
    mxCertPathLB->set_size_request(nWidth, nHeight);
    mxCertStatusML->set_size_request(nWidth, nHeight);

    mxCertPathLB->connect_changed( LINK( this, CertificateViewerCertPathTP, CertSelectHdl ) );
    mxViewCertPB->connect_clicked( LINK( this, CertificateViewerCertPathTP, ViewCertHdl ) );
}

void CertificateViewerCertPathTP::ActivatePage()
{
    if ( !mbFirstActivateDone )
    {
        mbFirstActivateDone = true;
        Sequence< Reference< security::XCertificate > > aCertPath =
            mpParent->mxSecurityEnvironment->buildCertificatePath( mpParent->mxCert );
        const Reference< security::XCertificate >* pCertPath = aCertPath.getConstArray();

        sal_Int32 i, nCnt = aCertPath.getLength();
        std::unique_ptr<weld::TreeIter> xParent;
        for (i = nCnt-1; i >= 0; i--)
        {
            const Reference< security::XCertificate > rCert = pCertPath[ i ];
            OUString sName = xmlsec::GetContentPart( rCert->getSubjectName() );
            //Verify the certificate
            sal_Int32 certStatus = mpDlg->mxSecurityEnvironment->verifyCertificate(rCert,
                 Sequence<Reference<css::security::XCertificate> >());
            bool bCertValid = certStatus == css::security::CertificateValidity::VALID;
            InsertCert(xParent.get(), sName, rCert, bCertValid);
            if (!xParent)
            {
                xParent = mxCertPathLB->make_iterator();
                mxCertPathLB->get_iter_first(*xParent);
            }
            else
            {
                mxCertPathLB->iter_children(*xParent);
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
}

IMPL_LINK_NOARG(CertificateViewerCertPathTP, ViewCertHdl, weld::Button&, void)
{
    std::unique_ptr<weld::TreeIter> xIter = mxCertPathLB->make_iterator();
    if (mxCertPathLB->get_selected(xIter.get()))
    {
        CertPath_UserData* pData = reinterpret_cast<CertPath_UserData*>(mxCertPathLB->get_id(*xIter).toInt64());
        CertificateViewer aViewer(mpDlg->getDialog(), mpDlg->mxSecurityEnvironment,
                pData->mxCert, false, nullptr);
        aViewer.run();
    }
}

IMPL_LINK_NOARG(CertificateViewerCertPathTP, CertSelectHdl, weld::TreeView&, void)
{
    OUString sStatus;

    std::unique_ptr<weld::TreeIter> xIter = mxCertPathLB->make_iterator();
    bool bEntry = mxCertPathLB->get_selected(xIter.get());
    if (bEntry)
    {
        CertPath_UserData* pData = reinterpret_cast<CertPath_UserData*>(mxCertPathLB->get_id(*xIter).toInt64());
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

void CertificateViewerCertPathTP::InsertCert(weld::TreeIter* pParent, const OUString& rName,
                                             const css::uno::Reference< css::security::XCertificate >& rxCert,
                                             bool bValid)
{
    OUString sImage = bValid ? OUStringLiteral(BMP_CERT_OK) : OUStringLiteral(BMP_CERT_NOT_OK);
    maUserData.emplace_back(std::make_unique<CertPath_UserData>(rxCert, bValid));
    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(maUserData.back().get())));
    mxCertPathLB->insert(pParent, -1, &rName, &sId, nullptr, nullptr, &sImage, false, nullptr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

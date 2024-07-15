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

#include <config_gpgme.h>
#include <certificatechooser.hxx>
#include <certificateviewer.hxx>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/xmlsechelper.hxx>

#include <com/sun/star/security/NoPasswordException.hpp>
#include <com/sun/star/security/CertificateCharacters.hpp>

#include <unotools/datetime.hxx>
#include <unotools/charclass.hxx>
#include <unotools/useroptions.hxx>

#include <resourcemanager.hxx>
#include <strings.hrc>

using namespace comphelper;
using namespace css;

CertificateChooser::CertificateChooser(weld::Window* _pParent,
                                       std::vector< css::uno::Reference< css::xml::crypto::XXMLSecurityContext > > && rxSecurityContexts,
                                       CertificateChooserUserAction eAction)
    : GenericDialogController(_pParent, u"xmlsec/ui/selectcertificatedialog.ui"_ustr, u"SelectCertificateDialog"_ustr)
    , meAction(eAction)
    , m_xFTSign(m_xBuilder->weld_label(u"sign"_ustr))
    , m_xFTEncrypt(m_xBuilder->weld_label(u"encrypt"_ustr))
    , m_xCertLB(m_xBuilder->weld_tree_view(u"signatures"_ustr))
    , m_xViewBtn(m_xBuilder->weld_button(u"viewcert"_ustr))
    , m_xOKBtn(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xFTDescription(m_xBuilder->weld_label(u"description-label"_ustr))
    , m_xDescriptionED(m_xBuilder->weld_entry(u"description"_ustr))
    , m_xSearchBox(m_xBuilder->weld_entry(u"searchbox"_ustr))
    , m_xReloadBtn(m_xBuilder->weld_button(u"reloadcert"_ustr))
{
    auto nControlWidth = m_xCertLB->get_approximate_digit_width() * 105;
    m_xCertLB->set_size_request(nControlWidth, m_xCertLB->get_height_rows(12));

    std::vector<int> aWidths
    {
        o3tl::narrowing<int>(30*nControlWidth/100),
        o3tl::narrowing<int>(30*nControlWidth/100),
        o3tl::narrowing<int>(10*nControlWidth/100),
        o3tl::narrowing<int>(20*nControlWidth/100)
    };
    m_xCertLB->set_column_fixed_widths(aWidths);
    m_xCertLB->connect_changed( LINK( this, CertificateChooser, CertificateHighlightHdl ) );
    m_xCertLB->connect_row_activated( LINK( this, CertificateChooser, CertificateSelectHdl ) );
    m_xViewBtn->connect_clicked( LINK( this, CertificateChooser, ViewButtonHdl ) );
    m_xSearchBox->connect_changed(LINK(this, CertificateChooser, SearchModifyHdl));
    m_xReloadBtn->connect_clicked( LINK( this, CertificateChooser, ReloadButtonHdl ) );

    mxSecurityContexts = std::move(rxSecurityContexts);
    mbInitialized = false;

    // disable buttons
    CertificateHighlightHdl(*m_xCertLB);
}

CertificateChooser::~CertificateChooser()
{
}

short CertificateChooser::run()
{
    // #i48432#
    // We can't check for personal certificates before raising this dialog,
    // because the mozilla implementation throws a NoPassword exception,
    // if the user pressed cancel, and also if the database does not exist!
    // But in the later case, the is no password query, and the user is confused
    // that nothing happens when pressing "Add..." in the SignatureDialog.

    // PostUserEvent( LINK( this, CertificateChooser, Initialize ) );

    // PostUserLink behavior is too slow, so do it directly before Execute().
    // Problem: This Dialog should be visible right now, and the parent should not be accessible.
    // Show, Update, DisableInput...

    m_xDialog->show();
    ImplInitialize();
    return GenericDialogController::run();
}

void CertificateChooser::HandleOneUsageBit(OUString& string, int& bits, int bit, TranslateId pResId)
{
    if (bits & bit)
    {
        if (!string.isEmpty())
            string += ", ";
        string += XsResId(pResId);
        bits &= ~bit;
    }
}

OUString CertificateChooser::UsageInClearText(int bits)
{
    OUString result;

    HandleOneUsageBit(result, bits, 0x80, STR_DIGITAL_SIGNATURE);
    HandleOneUsageBit(result, bits, 0x40, STR_NON_REPUDIATION);
    HandleOneUsageBit(result, bits, 0x20, STR_KEY_ENCIPHERMENT);
    HandleOneUsageBit(result, bits, 0x10, STR_DATA_ENCIPHERMENT);
    HandleOneUsageBit(result, bits, 0x08, STR_KEY_AGREEMENT);
    HandleOneUsageBit(result, bits, 0x04, STR_KEY_CERT_SIGN);
    HandleOneUsageBit(result, bits, 0x02, STR_CRL_SIGN);
    HandleOneUsageBit(result, bits, 0x01, STR_ENCIPHER_ONLY);

    // Check for mystery leftover bits
    if (bits != 0)
    {
        if (!result.isEmpty())
            result += ", ";
        result += "0x" + OUString::number(bits, 16);
    }

    return result;
}

void CertificateChooser::ImplInitialize(bool mbSearch)
{
    if (mbInitialized && !mbSearch)
        return;

    m_xCertLB->clear();
    m_xCertLB->make_unsorted();
    m_xCertLB->freeze();

    SvtUserOptions aUserOpts;

    SvtSysLocale aSysLocale;
    const CharClass& rCharClass = aSysLocale.GetCharClass();
    const OUString aSearchStr(rCharClass.uppercase(m_xSearchBox->get_text()));

    switch (meAction)
    {
        case CertificateChooserUserAction::Sign:
            m_xFTSign->show();
            m_xOKBtn->set_label(XsResId(STR_SIGN));
            msPreferredKey = aUserOpts.GetSigningKey();
            break;

        case CertificateChooserUserAction::SelectSign:
            m_xFTSign->show();
            m_xOKBtn->set_label(XsResId(STR_SELECTSIGN));
            msPreferredKey = aUserOpts.GetSigningKey();
            break;

        case CertificateChooserUserAction::Encrypt:
            m_xFTEncrypt->show();
            m_xFTDescription->hide();
            m_xDescriptionED->hide();
            m_xCertLB->set_selection_mode(SelectionMode::Multiple);
            m_xOKBtn->set_label(XsResId(STR_ENCRYPT));
            msPreferredKey = aUserOpts.GetEncryptionKey();
            break;

    }

    ::std::optional<int> oSelectRow;
    uno::Sequence<uno::Reference< security::XCertificate>> xCerts;
    for (auto& secContext : mxSecurityContexts)
    {
        if (!secContext.is())
            continue;
        auto secEnvironment = secContext->getSecurityEnvironment();
        if (!secEnvironment.is())
            continue;

        try
        {
            if (xMemCerts.count(secContext))
            {
                xCerts = xMemCerts[secContext];
            }
            else
            {
                if (meAction == CertificateChooserUserAction::Sign || meAction == CertificateChooserUserAction::SelectSign)
                    xCerts = secEnvironment->getPersonalCertificates();
                else
                    xCerts = secEnvironment->getAllCertificates();

                for (sal_Int32 nCert = xCerts.getLength(); nCert;)
                {
                    uno::Reference< security::XCertificate > xCert = xCerts[ --nCert ];
                    // Check if we have a private key for this...
                    tools::Long nCertificateCharacters = secEnvironment->getCertificateCharacters(xCert);

                    if (!(nCertificateCharacters & security::CertificateCharacters::HAS_PRIVATE_KEY))
                    {
                        ::comphelper::removeElementAt( xCerts, nCert );
                    }
                }
                xMemCerts[secContext] = xCerts;
            }
        }
        catch (security::NoPasswordException&)
        {
        }

        // fill list of certificates; the first entry will be selected
        for (const auto& xCert : xCerts)
        {
            std::shared_ptr<CertificateChooserUserData> userData = std::make_shared<CertificateChooserUserData>();
            userData->xCertificate = xCert;
            userData->xSecurityContext = secContext;
            userData->xSecurityEnvironment = secEnvironment;

            // Needed to keep userData alive. (reference to shared_ptr prevents delete)
            mvUserData.push_back(userData);

            OUString sIssuer = xmlsec::GetContentPart( xCert->getIssuerName(), xCert->getCertificateKind());

            // If we are searching and there is no match skip
            if (mbSearch
                && rCharClass.uppercase(sIssuer).indexOf(aSearchStr) < 0
                && rCharClass.uppercase(sIssuer).indexOf(aSearchStr) < 0
                && !aSearchStr.isEmpty())
                    continue;

            m_xCertLB->append();
            int nRow = m_xCertLB->n_children() - 1;
            m_xCertLB->set_text(nRow, xmlsec::GetContentPart(xCert->getSubjectName(), xCert->getCertificateKind()), 0);
            m_xCertLB->set_text(nRow, sIssuer, 1);
            m_xCertLB->set_text(nRow, xmlsec::GetCertificateKind(xCert->getCertificateKind()), 2);
            m_xCertLB->set_text(nRow, utl::GetDateString(xCert->getNotValidAfter()), 3);
            m_xCertLB->set_text(nRow, UsageInClearText(xCert->getCertificateUsage()), 4);
            OUString sId(weld::toId(userData.get()));
            m_xCertLB->set_id(nRow, sId);

#if HAVE_FEATURE_GPGME
            // only GPG has preferred keys
            if ( !sIssuer.isEmpty() && !msPreferredKey.isEmpty() ) {
                if ( sIssuer == msPreferredKey )
                {
                    if ( meAction == CertificateChooserUserAction::Sign || meAction == CertificateChooserUserAction::SelectSign )
                    {
                        oSelectRow.emplace(nRow);
                    }
                    else if ( meAction == CertificateChooserUserAction::Encrypt &&
                              aUserOpts.GetEncryptToSelf() )
                        mxEncryptToSelf = xCert;
                }
            }
#endif
        }
    }

    m_xCertLB->thaw();
    m_xCertLB->unselect_all();
    m_xCertLB->make_sorted();

    if (oSelectRow)
    {
        m_xCertLB->select(*oSelectRow);
    }

    CertificateHighlightHdl(*m_xCertLB);
    mbInitialized = true;
}

uno::Sequence<uno::Reference< css::security::XCertificate > > CertificateChooser::GetSelectedCertificates()
{
    std::vector< uno::Reference< css::security::XCertificate > > aRet;
    if (meAction == CertificateChooserUserAction::Encrypt)
    {
        // for encryption, multiselection is enabled
        m_xCertLB->selected_foreach([this, &aRet](weld::TreeIter& rEntry){
            CertificateChooserUserData* userData = weld::fromId<CertificateChooserUserData*>(m_xCertLB->get_id(rEntry));
            aRet.push_back( userData->xCertificate );
            return false;
        });
    }
    else
    {
        uno::Reference< css::security::XCertificate > xCert;
        int nSel = m_xCertLB->get_selected_index();
        if (nSel != -1)
        {
            CertificateChooserUserData* userData = weld::fromId<CertificateChooserUserData*>(m_xCertLB->get_id(nSel));
            xCert = userData->xCertificate;
        }
        aRet.push_back( xCert );
    }

#if HAVE_FEATURE_GPGME
    if ( mxEncryptToSelf.is())
        aRet.push_back( mxEncryptToSelf );
#endif

    return comphelper::containerToSequence(aRet);
}

uno::Reference<xml::crypto::XXMLSecurityContext> CertificateChooser::GetSelectedSecurityContext() const
{
    int nSel = m_xCertLB->get_selected_index();
    if (nSel == -1)
        return uno::Reference<xml::crypto::XXMLSecurityContext>();

    CertificateChooserUserData* userData = weld::fromId<CertificateChooserUserData*>(m_xCertLB->get_id(nSel));
    uno::Reference<xml::crypto::XXMLSecurityContext> xCert = userData->xSecurityContext;
    return xCert;
}

OUString CertificateChooser::GetDescription() const
{
    return m_xDescriptionED->get_text();
}

OUString CertificateChooser::GetUsageText()
{
    uno::Sequence< uno::Reference<css::security::XCertificate> > xCerts =
        GetSelectedCertificates();
    return (xCerts.hasElements() && xCerts[0].is()) ?
        UsageInClearText(xCerts[0]->getCertificateUsage()) : OUString();
}

void CertificateChooser::ImplReloadCertificates()
{
    xMemCerts.clear();
}

IMPL_LINK_NOARG(CertificateChooser, ReloadButtonHdl, weld::Button&, void)
{
    ImplReloadCertificates();
    mbInitialized = false;
    ImplInitialize();
}

IMPL_LINK_NOARG(CertificateChooser, SearchModifyHdl, weld::Entry&, void)
{
    ImplInitialize(true);
}

IMPL_LINK_NOARG(CertificateChooser, CertificateHighlightHdl, weld::TreeView&, void)
{
    bool bEnable = m_xCertLB->get_selected_index() != -1;
    m_xViewBtn->set_sensitive(bEnable);
    m_xOKBtn->set_sensitive(bEnable);
    m_xDescriptionED->set_sensitive(bEnable);
}

IMPL_LINK_NOARG(CertificateChooser, CertificateSelectHdl, weld::TreeView&, bool)
{
    m_xDialog->response(RET_OK);
    return true;
}

IMPL_LINK_NOARG(CertificateChooser, ViewButtonHdl, weld::Button&, void)
{
    ImplShowCertificateDetails();
}

void CertificateChooser::ImplShowCertificateDetails()
{
    int nSel = m_xCertLB->get_selected_index();
    if (nSel == -1)
        return;

    CertificateChooserUserData* userData = weld::fromId<CertificateChooserUserData*>(m_xCertLB->get_id(nSel));

    if (!userData->xSecurityEnvironment.is() || !userData->xCertificate.is())
        return;

    CertificateViewer aViewer(m_xDialog.get(), userData->xSecurityEnvironment, userData->xCertificate, true, this);
    aViewer.run();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

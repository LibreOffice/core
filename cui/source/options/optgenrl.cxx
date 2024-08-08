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

#include <comphelper/diagnose_ex.hxx>
#include <comphelper/string.hxx>
#include <comphelper/processfactory.hxx>

#include <config_gpgme.h>
#if HAVE_FEATURE_GPGME
# include <com/sun/star/xml/crypto/GPGSEInitializer.hpp>
# include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#endif
#include <com/sun/star/xml/crypto/SEInitializer.hpp>
#include <comphelper/xmlsechelper.hxx>
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>

#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <o3tl/safeint.hxx>
#include <vcl/svapp.hxx>
#include <svl/intitem.hxx>
#include <vcl/settings.hxx>

#include <officecfg/Office/Common.hxx>
#include <unotools/useroptions.hxx>
#include <cuioptgenrl.hxx>
#include <svx/svxids.hrc>
#include <svx/optgenrl.hxx>

using namespace css;

namespace
{

// rows
enum RowType
{
    Row_Company,
    Row_Name,
    Row_Name_Russian,
    Row_Name_Eastern,
    Row_Street,
    Row_Street_Russian,
    Row_City,
    Row_City_US,
    Row_Country,
    Row_TitlePos,
    Row_Phone,
    Row_FaxMail,

    nRowCount
};

// language flags
namespace Lang
{
    unsigned const Others = 1;
    unsigned const Russian = 2;
    unsigned const Eastern = 4;
    unsigned const US = 8;
    unsigned const All = static_cast<unsigned>(-1);
}


// vRowInfo[] -- rows (text + one or more edit boxes)
// The order is the same as in RowType above, which is up to down.

struct
{
    // id of the lockimage
    OUString pLockId;
    // id of the text
    OUString pTextId;
    // language flags (see Lang above):
    // which language is this row for?
    unsigned nLangFlags;
}
const vRowInfo[] =
{
    { u"lockcompanyft"_ustr,  u"companyft"_ustr,   Lang::All },
    { u"locknameft"_ustr,     u"nameft"_ustr,      Lang::All & ~Lang::Russian & ~Lang::Eastern },
    { u"lockrusnameft"_ustr,  u"rusnameft"_ustr,   Lang::Russian },
    { u"lockeastnameft"_ustr, u"eastnameft"_ustr,  Lang::Eastern },
    { u"lockstreetft"_ustr,   u"streetft"_ustr,    Lang::All & ~Lang::Russian },
    { u"lockrusstreetft"_ustr,u"russtreetft"_ustr, Lang::Russian },
    { u"lockicityft"_ustr,    u"icityft"_ustr,     Lang::All & ~Lang::US },
    { u"lockcityft"_ustr,     u"cityft"_ustr,      Lang::US },
    { u"lockcountryft"_ustr,  u"countryft"_ustr,   Lang::All },
    { u"locktitleft"_ustr,    u"titleft"_ustr,     Lang::All },
    { u"lockphoneft"_ustr,    u"phoneft"_ustr,     Lang::All },
    { u"lockfaxft"_ustr,      u"faxft"_ustr,       Lang::All },
};


// vFieldInfo[] -- edit boxes
// The order is up to down, and then left to right.

struct
{
    // in which row?
    RowType eRow;
    // id of the edit box
    OUString pEditId;
    // id for SvtUserOptions in unotools/useroptions.hxx
    UserOptToken nUserOptionsId;
    // id for settings the focus (defined in svx/optgenrl.hxx)
    EditPosition nGrabFocusId;
}
const vFieldInfo[] =
{
    // Company
    { Row_Company, u"company"_ustr, UserOptToken::Company, EditPosition::COMPANY },
    // Name
    { Row_Name, u"firstname"_ustr, UserOptToken::FirstName, EditPosition::FIRSTNAME },
    { Row_Name, u"lastname"_ustr, UserOptToken::LastName, EditPosition::LASTNAME  },
    { Row_Name, u"shortname"_ustr, UserOptToken::ID, EditPosition::SHORTNAME },
    // Name (russian)
    { Row_Name_Russian, u"ruslastname"_ustr, UserOptToken::LastName, EditPosition::LASTNAME  },
    { Row_Name_Russian, u"rusfirstname"_ustr, UserOptToken::FirstName, EditPosition::FIRSTNAME },
    { Row_Name_Russian, u"rusfathersname"_ustr, UserOptToken::FathersName, EditPosition::UNKNOWN },
    { Row_Name_Russian, u"russhortname"_ustr, UserOptToken::ID, EditPosition::SHORTNAME },
    // Name (eastern: reversed name ord
    { Row_Name_Eastern, u"eastlastname"_ustr, UserOptToken::LastName, EditPosition::LASTNAME  },
    { Row_Name_Eastern, u"eastfirstname"_ustr, UserOptToken::FirstName, EditPosition::FIRSTNAME },
    { Row_Name_Eastern, u"eastshortname"_ustr, UserOptToken::ID, EditPosition::SHORTNAME },
    // Street
    { Row_Street, u"street"_ustr, UserOptToken::Street, EditPosition::STREET },
    // Street (russian)
    { Row_Street_Russian, u"russtreet"_ustr, UserOptToken::Street, EditPosition::STREET },
    { Row_Street_Russian, u"apartnum"_ustr, UserOptToken::Apartment, EditPosition::UNKNOWN },
    // City
    { Row_City, u"izip"_ustr, UserOptToken::Zip, EditPosition::PLZ },
    { Row_City, u"icity"_ustr, UserOptToken::City, EditPosition::CITY },
    // City (US)
    { Row_City_US, u"city"_ustr, UserOptToken::City, EditPosition::CITY },
    { Row_City_US, u"state"_ustr, UserOptToken::State, EditPosition::STATE },
    { Row_City_US, u"zip"_ustr, UserOptToken::Zip, EditPosition::PLZ },
    // Country
    { Row_Country, u"country"_ustr, UserOptToken::Country, EditPosition::COUNTRY },
    // Title/Position
    { Row_TitlePos, u"title"_ustr, UserOptToken::Title,    EditPosition::TITLE },
    { Row_TitlePos, u"position"_ustr, UserOptToken::Position, EditPosition::POSITION },
    // Phone
    { Row_Phone, u"home"_ustr, UserOptToken::TelephoneHome, EditPosition::TELPRIV },
    { Row_Phone, u"work"_ustr, UserOptToken::TelephoneWork, EditPosition::TELCOMPANY },
    // Fax/Mail
    { Row_FaxMail, u"fax"_ustr, UserOptToken::Fax, EditPosition::FAX },
    { Row_FaxMail, u"email"_ustr, UserOptToken::Email, EditPosition::EMAIL },
};


} // namespace


// Row

struct SvxGeneralTabPage::Row
{
    // row lockdown icon
    std::unique_ptr<weld::Widget> xLockImg;
    // row label
    std::unique_ptr<weld::Label> xLabel;
    // first and last field in the row (last is exclusive)
    unsigned nFirstField, nLastField;

public:
    explicit Row (std::unique_ptr<weld::Widget> xLockImg_, std::unique_ptr<weld::Label> xLabel_)
        : xLockImg(std::move(xLockImg_))
        , xLabel(std::move(xLabel_))
        , nFirstField(0)
        , nLastField(0)
    {
        xLabel->show();
    }
};


// Field

struct SvxGeneralTabPage::Field
{
    // which field is this? (in vFieldInfo[] above)
    unsigned iField;
    // edit box
    std::unique_ptr<weld::Entry> xEdit;
    std::unique_ptr<weld::Container> xParent;

public:
    Field (std::unique_ptr<weld::Entry> xEdit_, unsigned iField_)
        : iField(iField_)
        , xEdit(std::move(xEdit_))
        , xParent(xEdit->weld_parent())
    {
        //We want all widgets inside a container, so each row of the toplevel
        //grid has another container in it. To avoid adding spacing to these
        //empty grids they all default to invisible, so show them if their
        //children are visible
        xParent->show();
        xEdit->show();
    }
};

SvxGeneralTabPage::SvxGeneralTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreSet)
    : SfxTabPage(pPage, pController, u"cui/ui/optuserpage.ui"_ustr, u"OptUserPage"_ustr, &rCoreSet)
    , m_xUseDataCB(m_xBuilder->weld_check_button(u"usefordocprop"_ustr))
    , m_xUseDataImg(m_xBuilder->weld_widget(u"lockusefordocprop"_ustr))
    , m_xCryptoFrame(m_xBuilder->weld_widget( u"cryptography"_ustr))
    , m_xSigningKeyLB(m_xBuilder->weld_entry(u"signingkey"_ustr))
    , m_xSigningKeyFT(m_xBuilder->weld_label(u"signingkeylabel"_ustr))
    , m_xSigningKeyImg(m_xBuilder->weld_widget(u"locksigningkey"_ustr))
    , m_xSigningKeyButton(m_xBuilder->weld_button(u"picksigningkey"_ustr))
    , m_xRemoveSigningKeyButton(m_xBuilder->weld_button(u"removesigningkey"_ustr))
    , m_xEncryptionKeyLB(m_xBuilder->weld_entry(u"encryptionkey"_ustr))
    , m_xEncryptionKeyFT(m_xBuilder->weld_label(u"encryptionkeylabel"_ustr))
    , m_xEncryptionKeyImg(m_xBuilder->weld_widget(u"lockencryptionkey"_ustr))
    , m_xEncryptionKeyButton(m_xBuilder->weld_button(u"pickencryptionkey"_ustr))
    , m_xRemoveEncryptionKeyButton(m_xBuilder->weld_button(u"removeencryptionkey"_ustr))
    , m_xEncryptToSelfCB(m_xBuilder->weld_check_button(u"encrypttoself"_ustr))
    , m_xEncryptToSelfImg(m_xBuilder->weld_widget(u"lockencrypttoself"_ustr))
{
    InitControls();
#if HAVE_FEATURE_GPGME
    InitCryptography();
#else
    m_xCryptoFrame->hide();
#endif

    SetExchangeSupport(); // this page needs ExchangeSupport
    SetLinks();
}

SvxGeneralTabPage::~SvxGeneralTabPage()
{
}

// Initializes the titles and the edit boxes,
// according to vRowInfo[] and vFieldInfo[] above.
void SvxGeneralTabPage::InitControls ()
{
    // which language bit do we use? (see Lang and vRowInfo[] above)
    unsigned LangBit;
    LanguageType l = Application::GetSettings().GetUILanguageTag().getLanguageType();
    if (l == LANGUAGE_ENGLISH_US)
        LangBit = Lang::US;
    else if (l == LANGUAGE_RUSSIAN)
        LangBit = Lang::Russian;
    else
    {
        if (MsLangId::isFamilyNameFirst(l))
            LangBit = Lang::Eastern;
        else
            LangBit = Lang::Others;
    }

    // creating rows
    unsigned iField = 0;
    for (unsigned iRow = 0; iRow != nRowCount; ++iRow)
    {
        RowType const eRow = static_cast<RowType>(iRow);
        // is the row visible?
        if (!(vRowInfo[iRow].nLangFlags & LangBit))
            continue;
        // creating row
        vRows.push_back(std::make_shared<Row>(m_xBuilder->weld_widget(vRowInfo[iRow].pLockId),
            m_xBuilder->weld_label(vRowInfo[iRow].pTextId)));
        Row& rRow = *vRows.back();
        // fields in the row
        static unsigned const nFieldCount = std::size(vFieldInfo);
        // skipping other (invisible) rows
        while (iField != nFieldCount && vFieldInfo[iField].eRow != eRow)
            ++iField;
        // fields in the row
        rRow.nFirstField = vFields.size();
        for ( ; iField != nFieldCount && vFieldInfo[iField].eRow == eRow; ++iField)
        {
            // creating edit field
            vFields.push_back(std::make_shared<Field>(
                m_xBuilder->weld_entry(vFieldInfo[iField].pEditId), iField));
            // "short name" field?
            if (vFieldInfo[iField].nUserOptionsId == UserOptToken::ID)
            {
                nNameRow = vRows.size() - 1;
                nShortNameField = vFields.size() - 1;
            }
        }
        rRow.nLastField = vFields.size();
    }
}

void SvxGeneralTabPage::InitCryptography()
{
#if HAVE_FEATURE_GPGME
    m_xCryptoFrame->show();
    m_xSigningKeyButton->connect_clicked(LINK(this, SvxGeneralTabPage, ChooseKeyButtonHdl));
    m_xEncryptionKeyButton->connect_clicked(LINK(this, SvxGeneralTabPage, ChooseKeyButtonHdl));

    m_xRemoveSigningKeyButton->connect_clicked(LINK(this, SvxGeneralTabPage, RemoveKeyButtonHdl));
    m_xRemoveEncryptionKeyButton->connect_clicked(LINK(this, SvxGeneralTabPage, RemoveKeyButtonHdl));
#endif

}

IMPL_LINK(SvxGeneralTabPage, ChooseKeyButtonHdl, weld::Button&, rButton, void)
{
    try
    {
        uno::Reference<security::XDocumentDigitalSignatures> xD(
            security::DocumentDigitalSignatures::createDefault(
                comphelper::getProcessComponentContext()));
        xD->setParentWindow(GetDialogController()->getDialog()->GetXWindow());

        OUString aDescription;

        uno::Reference<security::XCertificate> xCertificate;
        if (m_xSigningKeyButton.get() == &rButton)
        {
            xCertificate = xD->selectSigningCertificate(aDescription);
        }
        else if (m_xEncryptionKeyButton.get() == &rButton)
        {
            auto xCerts = xD->chooseEncryptionCertificate(css::security::CertificateKind_OPENPGP);
            if(xCerts.hasElements())
                xCertificate = xCerts[0];
        }

        if(!xCertificate.is())
            return;

        OUString aKeyThumbprint
            = comphelper::xmlsec::GetHexString(xCertificate->getSHA1Thumbprint(), "");
        OUString aIssuer = comphelper::xmlsec::GetContentPart(xCertificate->getIssuerName(),
                                                              xCertificate->getCertificateKind());
        OUString aSubject = comphelper::xmlsec::GetContentPart(xCertificate->getSubjectName(),
                                                               xCertificate->getCertificateKind());
        OUString aKeyDisplayName;
        switch (xCertificate->getCertificateKind())
        {
            case security::CertificateKind::CertificateKind_X509:
                aKeyDisplayName = u"(X.509) "_ustr + aIssuer + u" "_ustr + aSubject;
                break;
            case security::CertificateKind::CertificateKind_OPENPGP:
                aKeyDisplayName = u"(OpenPGP) "_ustr + aIssuer;
                break;
            default:
                break;
        }

        if (m_xSigningKeyButton.get() == &rButton)
        {
            msCurrentSigningKey = aKeyThumbprint;
            m_xSigningKeyLB->set_text(aKeyDisplayName);
        }
        else if (m_xEncryptionKeyButton.get() == &rButton)
        {
            msCurrentEncryptionKey = aKeyThumbprint;
            m_xEncryptionKeyLB->set_text(aKeyDisplayName);
        }
    }
    catch (const css::uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("cui.options", "" );
    }
}

IMPL_LINK(SvxGeneralTabPage, RemoveKeyButtonHdl, weld::Button&, rButton, void)
{
        if (m_xRemoveSigningKeyButton.get() == &rButton)
        {
            msCurrentSigningKey.clear();
            m_xSigningKeyLB->set_text(u""_ustr);
        }
        else if (m_xRemoveEncryptionKeyButton.get() == &rButton)
        {
            msCurrentEncryptionKey.clear();
            m_xEncryptionKeyLB->set_text(u""_ustr);
        }
}

void SvxGeneralTabPage::SetLinks ()
{
    // link for updating the initials
    Link<weld::Entry&,void> aLink = LINK( this, SvxGeneralTabPage, ModifyHdl_Impl );
    Row& rNameRow = *vRows[nNameRow];
    for (unsigned i = rNameRow.nFirstField; i != rNameRow.nLastField - 1; ++i)
        vFields[i]->xEdit->connect_changed(aLink);
}


std::unique_ptr<SfxTabPage> SvxGeneralTabPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet )
{
    return std::make_unique<SvxGeneralTabPage>( pPage, pController, *rAttrSet );
}

OUString SvxGeneralTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[]
        = { u"label1"_ustr,     u"companyft"_ustr,         u"nameft"_ustr,          u"rusnameft"_ustr,
            u"eastnameft"_ustr, u"streetft"_ustr,          u"russtreetft"_ustr,     u"icityft"_ustr,
            u"cityft"_ustr,     u"countryft"_ustr,         u"titleft"_ustr,         u"phoneft"_ustr,
            u"faxft"_ustr,      u"cryptographylabel"_ustr, u"signingkeylabel"_ustr, u"encryptionkeylabel"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    sAllStrings += m_xUseDataCB->get_label() + " " + m_xEncryptToSelfCB->get_label() + " ";

    return sAllStrings.replaceAll("_", "");
}

bool SvxGeneralTabPage::FillItemSet( SfxItemSet* )
{
    // remove leading and trailing whitespaces
    for (auto const & i: vFields)
        i->xEdit->set_text(comphelper::string::strip(i->xEdit->get_text(), ' '));

    bool bModified = false;
    bModified |= GetData_Impl();
    if (m_xUseDataCB->get_active() != officecfg::Office::Common::Save::Document::UseUserData::get())
    {
        auto xChanges = comphelper::ConfigurationChanges::create();
        officecfg::Office::Common::Save::Document::UseUserData::set(m_xUseDataCB->get_active(), xChanges);
        xChanges->commit();
        bModified = true;
    }
    return bModified;
}

void SvxGeneralTabPage::Reset( const SfxItemSet* rSet )
{
    SetData_Impl();

    if (rSet->GetItemState(SID_FIELD_GRABFOCUS) == SfxItemState::SET)
    {
        EditPosition nField = static_cast<EditPosition>(rSet->Get(SID_FIELD_GRABFOCUS).GetValue());
        if (nField != EditPosition::UNKNOWN)
        {
            for (auto const & i: vFields)
                if (nField == vFieldInfo[i->iField].nGrabFocusId)
                    i->xEdit->grab_focus();
        }
        else
            vFields.front()->xEdit->grab_focus();
    }

    m_xUseDataCB->set_active(officecfg::Office::Common::Save::Document::UseUserData::get());
}


// ModifyHdl_Impl()
// This handler updates the initials (short name)
// when one of the name fields was updated.
IMPL_LINK( SvxGeneralTabPage, ModifyHdl_Impl, weld::Entry&, rEdit, void )
{
    // short name field and row
    Field& rShortName = *vFields[nShortNameField];
    Row& rNameRow = *vRows[nNameRow];
    // number of initials
    unsigned const nInits = rNameRow.nLastField - rNameRow.nFirstField - 1;
    // which field was updated? (in rNameRow)
    unsigned nField = nInits;
    for (unsigned i = 0; i != nInits; ++i)
    {
        if (vFields[rNameRow.nFirstField + i]->xEdit.get() == &rEdit)
            nField = i;
    }
    // updating the initial
    if (!(nField < nInits && rShortName.xEdit->get_sensitive()))
        return;

    OUString sShortName = rShortName.xEdit->get_text();
    // clear short name if it contains more characters than the number of initials
    if (o3tl::make_unsigned(sShortName.getLength()) > nInits)
    {
        rShortName.xEdit->set_text(OUString());
    }
    while (o3tl::make_unsigned(sShortName.getLength()) < nInits)
        sShortName += " ";
    OUString sName = rEdit.get_text();
    OUString sLetter = sName.isEmpty()
        ? OUString(u' ') : sName.copy(0, 1);
    rShortName.xEdit->set_text(sShortName.replaceAt(nField, 1, sLetter).trim());
}


bool SvxGeneralTabPage::GetData_Impl()
{
    // updating
    SvtUserOptions aUserOpt;
    for (auto const & i: vFields)
        aUserOpt.SetToken(
            vFieldInfo[i->iField].nUserOptionsId,
            i->xEdit->get_text()
        );

    // modified?
    bool bModified = false;
    for (auto const & i: vFields)
    {
        if (i->xEdit->get_value_changed_from_saved())
        {
            bModified = true;
            break;
        }
    }

#if HAVE_FEATURE_GPGME
    aUserOpt.SetToken( UserOptToken::SigningKey, msCurrentSigningKey );
    aUserOpt.SetToken( UserOptToken::SigningKeyDisplayName, m_xSigningKeyLB->get_text() );
    aUserOpt.SetToken( UserOptToken::EncryptionKey, msCurrentEncryptionKey );
    aUserOpt.SetToken( UserOptToken::EncryptionKeyDisplayName, m_xEncryptionKeyLB->get_text() );
    aUserOpt.SetBoolValue( UserOptToken::EncryptToSelf, m_xEncryptToSelfCB->get_active() );

    bModified |= m_xSigningKeyLB->get_value_changed_from_saved() ||
                 m_xEncryptionKeyLB->get_value_changed_from_saved() ||
                 m_xEncryptToSelfCB->get_state_changed_from_saved();
#endif

    return bModified;
}


void SvxGeneralTabPage::SetData_Impl()
{
    // updating and disabling edit boxes
    SvtUserOptions aUserOpt;
    for (auto const & i: vRows)
    {
        Row& rRow = *i;
        // the label is enabled if any of its edit fields are enabled
        bool bEnableLabel = false;
        for (unsigned iField = rRow.nFirstField; iField != rRow.nLastField; ++iField)
        {
            Field& rField = *vFields[iField];
            // updating content
            UserOptToken const nToken = vFieldInfo[rField.iField].nUserOptionsId;
            rField.xEdit->set_text(aUserOpt.GetToken(nToken));
            // is enabled?
            bool const bEnableEdit = !aUserOpt.IsTokenReadonly(nToken);
            rField.xEdit->set_sensitive(bEnableEdit);
            bEnableLabel = bEnableLabel || bEnableEdit;
        }
        rRow.xLabel->set_sensitive(bEnableLabel);
        rRow.xLockImg->set_visible(!bEnableLabel);
    }

    // saving
    for (auto const & i: vFields)
        i->xEdit->save_value();

    //enabling and disabling remaining fields
    bool bEnable = !officecfg::Office::Common::Save::Document::UseUserData::isReadOnly();
    m_xUseDataCB->set_sensitive(bEnable);
    m_xUseDataImg->set_visible(!bEnable);

#if HAVE_FEATURE_GPGME
    bEnable = !aUserOpt.IsTokenReadonly(UserOptToken::SigningKey);
    m_xSigningKeyButton->set_sensitive(bEnable);
    m_xSigningKeyFT->set_sensitive(bEnable);
    m_xSigningKeyImg->set_visible(!bEnable);

    bEnable = !aUserOpt.IsTokenReadonly(UserOptToken::EncryptionKey);
    m_xEncryptionKeyButton->set_sensitive(bEnable);
    m_xEncryptionKeyFT->set_sensitive(bEnable);
    m_xEncryptionKeyImg->set_visible(!bEnable);

    bEnable = !aUserOpt.IsTokenReadonly(UserOptToken::EncryptToSelf);
    m_xEncryptToSelfCB->set_sensitive(bEnable);
    m_xEncryptToSelfImg->set_visible(!bEnable);

    msCurrentSigningKey = aUserOpt.GetToken(UserOptToken::SigningKey);
    m_xSigningKeyLB->set_text(aUserOpt.GetToken(UserOptToken::SigningKeyDisplayName));

    msCurrentEncryptionKey = aUserOpt.GetToken(UserOptToken::EncryptionKey);
    m_xEncryptionKeyLB->set_text(aUserOpt.GetToken(UserOptToken::EncryptionKeyDisplayName));

    m_xEncryptToSelfCB->set_active( aUserOpt.GetEncryptToSelf() );
#endif
}


DeactivateRC SvxGeneralTabPage::DeactivatePage( SfxItemSet* pSet_ )
{
    if ( pSet_ )
        FillItemSet( pSet_ );
    return DeactivateRC::LeavePage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

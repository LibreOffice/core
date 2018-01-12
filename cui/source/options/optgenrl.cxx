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

#include <comphelper/string.hxx>

#include <config_gpgme.h>
#if HAVE_FEATURE_GPGME
# include <com/sun/star/xml/crypto/GPGSEInitializer.hpp>
# include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#endif

#include <i18nlangtag/mslangid.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/saveopt.hxx>
#include <svl/intitem.hxx>
#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/settings.hxx>

#include <unotools/useroptions.hxx>
#include <cuioptgenrl.hxx>
#include <dialmgr.hxx>
#include <svx/dlgutil.hxx>
#include <svx/svxids.hrc>

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
    // id of the text
    const char *pTextId;
    // language flags (see Lang above):
    // which language is this row for?
    unsigned nLangFlags;
}
const vRowInfo[] =
{
    { "companyft",   Lang::All },
    { "nameft",      Lang::All & ~Lang::Russian & ~Lang::Eastern },
    { "rusnameft",   Lang::Russian },
    { "eastnameft",  Lang::Eastern },
    { "streetft",    Lang::All & ~Lang::Russian },
    { "russtreetft", Lang::Russian },
    { "icityft",     Lang::All & ~Lang::US },
    { "cityft",      Lang::US },
    { "countryft",   Lang::All },
    { "titleft",     Lang::All },
    { "phoneft",     Lang::All },
    { "faxft",       Lang::All },
};


// vFieldInfo[] -- edit boxes
// The order is up to down, and then left to right.

struct
{
    // in which row?
    RowType eRow;
    // id of the edit box
    const char *pEditId;
    // id for SvtUserOptions in unotools/useroptions.hxx
    UserOptToken nUserOptionsId;
    // id for settings the focus (defined in svx/optgenrl.hxx)
    EditPosition nGrabFocusId;
}
const vFieldInfo[] =
{
    // Company
    { Row_Company, "company", UserOptToken::Company, EditPosition::COMPANY },
    // Name
    { Row_Name, "firstname", UserOptToken::FirstName, EditPosition::FIRSTNAME },
    { Row_Name, "lastname", UserOptToken::LastName, EditPosition::LASTNAME  },
    { Row_Name, "shortname", UserOptToken::ID, EditPosition::SHORTNAME },
    // Name (russian)
    { Row_Name_Russian, "ruslastname", UserOptToken::LastName, EditPosition::LASTNAME  },
    { Row_Name_Russian, "rusfirstname", UserOptToken::FirstName, EditPosition::FIRSTNAME },
    { Row_Name_Russian, "rusfathersname", UserOptToken::FathersName, EditPosition::UNKNOWN },
    { Row_Name_Russian, "russhortname", UserOptToken::ID, EditPosition::SHORTNAME },
    // Name (eastern: reversed name ord
    { Row_Name_Eastern, "eastlastname", UserOptToken::LastName, EditPosition::LASTNAME  },
    { Row_Name_Eastern, "eastfirstname", UserOptToken::FirstName, EditPosition::FIRSTNAME },
    { Row_Name_Eastern, "eastshortname", UserOptToken::ID, EditPosition::SHORTNAME },
    // Street
    { Row_Street, "street", UserOptToken::Street, EditPosition::STREET },
    // Street (russian)
    { Row_Street_Russian, "russtreet", UserOptToken::Street, EditPosition::STREET },
    { Row_Street_Russian, "apartnum", UserOptToken::Apartment, EditPosition::UNKNOWN },
    // City
    { Row_City, "izip", UserOptToken::Zip, EditPosition::PLZ },
    { Row_City, "icity", UserOptToken::City, EditPosition::CITY },
    // City (US)
    { Row_City_US, "city", UserOptToken::City, EditPosition::CITY },
    { Row_City_US, "state", UserOptToken::State, EditPosition::STATE },
    { Row_City_US, "zip", UserOptToken::Zip, EditPosition::PLZ },
    // Country
    { Row_Country, "country", UserOptToken::Country, EditPosition::COUNTRY },
    // Title/Position
    { Row_TitlePos, "title", UserOptToken::Title,    EditPosition::TITLE },
    { Row_TitlePos, "position", UserOptToken::Position, EditPosition::POSITION },
    // Phone
    { Row_Phone, "home", UserOptToken::TelephoneHome, EditPosition::TELPRIV },
    { Row_Phone, "work", UserOptToken::TelephoneWork, EditPosition::TELCOMPANY },
    // Fax/Mail
    { Row_FaxMail, "fax", UserOptToken::Fax, EditPosition::FAX },
    { Row_FaxMail, "email", UserOptToken::Email, EditPosition::EMAIL },
};


} // namespace


// Row

struct SvxGeneralTabPage::Row
{
    // row label
    VclPtr<FixedText> pLabel;
    // first and last field in the row (last is exclusive)
    unsigned nFirstField, nLastField;

public:
    explicit Row (FixedText *pLabel_)
        : pLabel(pLabel_)
        , nFirstField(0)
        , nLastField(0)
    {
        pLabel->Show();
    }
};


// Field

struct SvxGeneralTabPage::Field
{
    // which field is this? (in vFieldInfo[] above)
    unsigned iField;
    // edit box
    VclPtr<Edit> pEdit;

public:
    Field (Edit *pEdit_, unsigned iField_)
        : iField(iField_)
        , pEdit(pEdit_)
    {
        //We want all widgets inside a container, so each row of the toplevel
        //grid has another container in it. To avoid adding spacing to these
        //empty grids they all default to invisible, so show them if their
        //children are visible
        pEdit->GetParent()->Show();
        pEdit->Show();
    }
};


SvxGeneralTabPage::SvxGeneralTabPage(vcl::Window* pParent, const SfxItemSet& rCoreSet)

    : SfxTabPage(pParent, "OptUserPage", "cui/ui/optuserpage.ui", &rCoreSet)
{
    get(m_pUseDataCB, "usefordocprop");

    get(m_pCryptoFrame, "cryptography");
    get(m_pSigningKeyLB, "signingkey");
    get(m_pEncryptionKeyLB, "encryptionkey");
    get(m_pEncryptToSelfCB, "encrypttoself");
    InitControls();
#if HAVE_FEATURE_GPGME
    InitCryptography();
#else
    m_pCryptoFrame->Hide();
#endif

    SetExchangeSupport(); // this page needs ExchangeSupport
    SetLinks();
}

SvxGeneralTabPage::~SvxGeneralTabPage()
{
    disposeOnce();
}

void SvxGeneralTabPage::dispose()
{
    m_pUseDataCB.clear();
    m_pSigningKeyLB.clear();
    m_pEncryptionKeyLB.clear();
    m_pEncryptToSelfCB.clear();
    m_pCryptoFrame.clear();
    SfxTabPage::dispose();
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
        vRows.push_back(std::make_shared<Row>(
            get<FixedText>(vRowInfo[iRow].pTextId)));
        Row& rRow = *vRows.back();
        // fields in the row
        static unsigned const nFieldCount = SAL_N_ELEMENTS(vFieldInfo);
        // skipping other (invisible) rows
        while (iField != nFieldCount && vFieldInfo[iField].eRow != eRow)
            ++iField;
        // fields in the row
        rRow.nFirstField = vFields.size();
        for ( ; iField != nFieldCount && vFieldInfo[iField].eRow == eRow; ++iField)
        {
            // creating edit field
            vFields.push_back(std::make_shared<Field>(
                get<Edit>(vFieldInfo[iField].pEditId), iField));
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
    m_pCryptoFrame->Show();
    // unused yet, I just wanted to see if this delivers the desired results
    uno::Reference< xml::crypto::XSEInitializer > xSEInitializer;
    try
    {
        xSEInitializer = xml::crypto::GPGSEInitializer::create( comphelper::getProcessComponentContext() );
        uno::Reference<xml::crypto::XXMLSecurityContext> xSC = xSEInitializer->createSecurityContext( OUString() );
        if (xSC.is())
        {
            uno::Reference<xml::crypto::XSecurityEnvironment> xSE = xSC->getSecurityEnvironment();
            uno::Sequence<uno::Reference<security::XCertificate>> xCertificates = xSE->getPersonalCertificates();

            if (xCertificates.hasElements())
            {
                for (auto& xCert : xCertificates)
                {
                    m_pSigningKeyLB->InsertEntry( xCert->getIssuerName());
                    m_pEncryptionKeyLB->InsertEntry( xCert->getIssuerName());
                }
            }
        }
    }
    catch ( uno::Exception const & )
    {}
#endif

}

void SvxGeneralTabPage::SetLinks ()
{
    // link for updating the initials
    Link<Edit&,void> aLink = LINK( this, SvxGeneralTabPage, ModifyHdl_Impl );
    Row& rNameRow = *vRows[nNameRow];
    for (unsigned i = rNameRow.nFirstField; i != rNameRow.nLastField - 1; ++i)
        vFields[i]->pEdit->SetModifyHdl(aLink);
}


VclPtr<SfxTabPage> SvxGeneralTabPage::Create( vcl::Window* pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<SvxGeneralTabPage>::Create( pParent, *rAttrSet );
}

bool SvxGeneralTabPage::FillItemSet( SfxItemSet* )
{
    // remove leading and trailing whitespaces
    for (auto const & i: vFields)
        i->pEdit->SetText(comphelper::string::strip(i->pEdit->GetText(), ' '));

    bool bModified = false;
    bModified |= GetData_Impl();
    SvtSaveOptions aSaveOpt;
    if ( m_pUseDataCB->IsChecked() != aSaveOpt.IsUseUserData() )
    {
        aSaveOpt.SetUseUserData( m_pUseDataCB->IsChecked() );
        bModified = true;
    }
    return bModified;
}

void SvxGeneralTabPage::Reset( const SfxItemSet* rSet )
{
    SetData_Impl();

    sal_uInt16 const nWhich = GetWhich(SID_FIELD_GRABFOCUS);

    if (rSet->GetItemState(nWhich) == SfxItemState::SET)
    {
        EditPosition nField = static_cast<EditPosition>(static_cast<const SfxUInt16Item&>(rSet->Get(nWhich)).GetValue());
        if (nField != EditPosition::UNKNOWN)
        {
            for (auto const & i: vFields)
                if (nField == vFieldInfo[i->iField].nGrabFocusId)
                    i->pEdit->GrabFocus();
        }
        else
            vFields.front()->pEdit->GrabFocus();
    }

    m_pUseDataCB->Check( SvtSaveOptions().IsUseUserData() );
}


// ModifyHdl_Impl()
// This handler updates the initials (short name)
// when one of the name fields was updated.
IMPL_LINK( SvxGeneralTabPage, ModifyHdl_Impl, Edit&, rEdit, void )
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
        if (vFields[rNameRow.nFirstField + i]->pEdit == &rEdit)
            nField = i;
    }
    // updating the initial
    if (nField < nInits && rShortName.pEdit->IsEnabled())
    {
        OUString sShortName = rShortName.pEdit->GetText();
        // clear short name if it contains more characters than the number of initials
        if (static_cast<unsigned>(sShortName.getLength()) > nInits)
        {
            rShortName.pEdit->SetText(OUString());
        }
        while (static_cast<unsigned>(sShortName.getLength()) < nInits)
            sShortName += " ";
        OUString sName = rEdit.GetText();
        OUString sLetter = sName.isEmpty()
            ? OUString(u' ') : sName.copy(0, 1);
        rShortName.pEdit->SetText(sShortName.replaceAt(nField, 1, sLetter).trim());
    }
}


bool SvxGeneralTabPage::GetData_Impl()
{
    // updating
    SvtUserOptions aUserOpt;
    for (auto const & i: vFields)
        aUserOpt.SetToken(
            vFieldInfo[i->iField].nUserOptionsId,
            i->pEdit->GetText()
        );

    // modified?
    bool bModified = false;
    for (auto const & i: vFields)
    {
        if (i->pEdit->IsValueChangedFromSaved())
        {
            bModified = true;
            break;
        }
    }

#if HAVE_FEATURE_GPGME
    OUString aSK = m_pSigningKeyLB->GetSelectedEntryPos() == 0 ? OUString() //i.e. no key
                       : m_pSigningKeyLB->GetSelectedEntry();
    OUString aEK = m_pEncryptionKeyLB->GetSelectedEntryPos() == 0 ? OUString()
                       : m_pEncryptionKeyLB->GetSelectedEntry();

    aUserOpt.SetToken( UserOptToken::SigningKey, aSK );
    aUserOpt.SetToken( UserOptToken::EncryptionKey, aEK );
    aUserOpt.SetBoolValue( UserOptToken::EncryptToSelf, m_pEncryptToSelfCB->IsChecked() );

    bModified |= m_pSigningKeyLB->IsValueChangedFromSaved() ||
                 m_pEncryptionKeyLB->IsValueChangedFromSaved() ||
                 m_pEncryptToSelfCB->IsValueChangedFromSaved();
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
            rField.pEdit->SetText(aUserOpt.GetToken(nToken));
            // is enabled?
            bool const bEnableEdit = !aUserOpt.IsTokenReadonly(nToken);
            rField.pEdit->Enable(bEnableEdit);
            bEnableLabel = bEnableLabel || bEnableEdit;
        }
        rRow.pLabel->Enable(bEnableLabel);
    }

    // saving
    for (auto const & i: vFields)
        i->pEdit->SaveValue();

#if HAVE_FEATURE_GPGME
    OUString aSK = aUserOpt.GetToken(UserOptToken::SigningKey);
    aSK.isEmpty() ? m_pSigningKeyLB->SelectEntryPos( 0 ) //i.e. 'No Key'
                  : m_pSigningKeyLB->SelectEntry( aSK );

    OUString aEK = aUserOpt.GetToken(UserOptToken::EncryptionKey);
    aEK.isEmpty() ? m_pEncryptionKeyLB->SelectEntryPos( 0 ) //i.e. 'No Key'
                  : m_pEncryptionKeyLB->SelectEntry( aEK );

    m_pEncryptToSelfCB->Check( aUserOpt.GetEncryptToSelf() );
#endif
}


DeactivateRC SvxGeneralTabPage::DeactivatePage( SfxItemSet* pSet_ )
{
    if ( pSet_ )
        FillItemSet( pSet_ );
    return DeactivateRC::LeavePage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

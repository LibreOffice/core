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
#include <i18nlangtag/mslangid.hxx>
#include <tools/shl.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/saveopt.hxx>
#include <svl/intitem.hxx>
#include <vcl/edit.hxx>

#include <cuires.hrc>
#include <unotools/useroptions.hxx>
#include "cuioptgenrl.hxx"
#include <dialmgr.hxx>
#include <svx/dlgutil.hxx>
#include <svx/svxids.hrc> // SID_FIELD_GRABFOCUS

#include <boost/ref.hpp>
#include <boost/make_shared.hpp>

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
    unsigned const All = -1;
}

//
// vRowInfo[] -- rows (text + one or more edit boxes)
// The order is the same as in RowType above, which is up to down.
//
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

//
// vFieldInfo[] -- edit boxes
// The order is up to down, and then left to right.
//
struct
{
    // in which row?
    RowType eRow;
    // id of the edit box
    const char *pEditId;
    // id for SvtUserOptions in unotools/useroptions.hxx
    int nUserOptionsId;
    // id for settings the focus (defined in svx/optgenrl.hxx)
    int nGrabFocusId;
}
const vFieldInfo[] =
{
    // Company
    { Row_Company, "company", USER_OPT_COMPANY, COMPANY_EDIT },
    // Name
    { Row_Name, "firstname", USER_OPT_FIRSTNAME, FIRSTNAME_EDIT },
    { Row_Name, "lastname", USER_OPT_LASTNAME, LASTNAME_EDIT  },
    { Row_Name, "shortname", USER_OPT_ID, SHORTNAME_EDIT },
    // Name (russian)
    { Row_Name_Russian, "ruslastname", USER_OPT_LASTNAME, LASTNAME_EDIT  },
    { Row_Name_Russian, "rusfirstname", USER_OPT_FIRSTNAME, FIRSTNAME_EDIT },
    { Row_Name_Russian, "rusfathersname", USER_OPT_FATHERSNAME, 0 },
    { Row_Name_Russian, "russhortname", USER_OPT_ID, SHORTNAME_EDIT },
    // Name (eastern: reversed name ord
    { Row_Name_Eastern, "eastlastname", USER_OPT_LASTNAME, LASTNAME_EDIT  },
    { Row_Name_Eastern, "eastfirstname", USER_OPT_FIRSTNAME, FIRSTNAME_EDIT },
    { Row_Name_Eastern, "eastshortname", USER_OPT_ID, SHORTNAME_EDIT },
    // Street
    { Row_Street, "street", USER_OPT_STREET, STREET_EDIT },
    // Street (russian)
    { Row_Street_Russian, "russtreet", USER_OPT_STREET, STREET_EDIT },
    { Row_Street_Russian, "apartnum", USER_OPT_APARTMENT, 0 },
    // City
    { Row_City, "izip", USER_OPT_ZIP, PLZ_EDIT },
    { Row_City, "icity", USER_OPT_CITY, CITY_EDIT },
    // City (US)
    { Row_City_US, "city", USER_OPT_CITY, CITY_EDIT },
    { Row_City_US, "state", USER_OPT_STATE, STATE_EDIT },
    { Row_City_US, "zip", USER_OPT_ZIP, PLZ_EDIT },
    // Country
    { Row_Country, "country", USER_OPT_COUNTRY, COUNTRY_EDIT },
    // Title/Position
    { Row_TitlePos, "title", USER_OPT_TITLE,    TITLE_EDIT },
    { Row_TitlePos, "position", USER_OPT_POSITION, POSITION_EDIT },
    // Phone
    { Row_Phone, "home", USER_OPT_TELEPHONEHOME, TELPRIV_EDIT },
    { Row_Phone, "work", USER_OPT_TELEPHONEWORK, TELCOMPANY_EDIT },
    // Fax/Mail
    { Row_FaxMail, "fax", USER_OPT_FAX, FAX_EDIT },
    { Row_FaxMail, "email", USER_OPT_EMAIL, EMAIL_EDIT },
};


} // namespace

// -----------------------------------------------------------------------

//
// Row
//
struct SvxGeneralTabPage::Row
{
    // which row is it?
    RowType eRow;
    // row label
    FixedText* pLabel;
    // first and last field in the row (last is exclusive)
    unsigned nFirstField, nLastField;

public:
    Row (FixedText *pLabel_, RowType eRow_)
        : eRow(eRow_)
        , pLabel(pLabel_)
        , nFirstField(0)
        , nLastField(0)
    {
        pLabel->Show();
    }
};

// -----------------------------------------------------------------------

//
// Field
//
struct SvxGeneralTabPage::Field
{
    // which field is this? (in vFieldInfo[] above)
    unsigned iField;
    // edit box
    Edit *pEdit;

public:
    Field (Edit *pEdit_, unsigned iField_)
        : iField(iField_)
        , pEdit(pEdit_)
    {
        //We want all widgets inside a container, so each row of the toplevel
        //grid has another container in it. To avoid adding spacing to these
        //empty grids they all default to invisible, so show them if their
        //children are visibles
        pEdit->GetParent()->Show();
        pEdit->Show();
    }
};

// -----------------------------------------------------------------------

SvxGeneralTabPage::SvxGeneralTabPage(Window* pParent, const SfxItemSet& rCoreSet)

    : SfxTabPage(pParent, "OptUserPage", "cui/ui/optuserpage.ui", rCoreSet)
{
    get(m_pUseDataCB, "usefordocprop");
    InitControls();
    SetExchangeSupport(); // this page needs ExchangeSupport
    SetLinks();
}

//------------------------------------------------------------------------

SvxGeneralTabPage::~SvxGeneralTabPage ()
{ }

//------------------------------------------------------------------------

// Initializes the titles and the edit boxes,
// according to vRowInfo[] and vFieldInfo[] above.
void SvxGeneralTabPage::InitControls ()
{
    // which language bit do we use? (see Lang and vRowInfo[] above)
    unsigned LangBit;
    switch (LanguageType const eLang = Application::GetSettings().GetUILanguageTag().getLanguageType())
    {
        case LANGUAGE_ENGLISH_US:
            LangBit = Lang::US;
            break;
        case LANGUAGE_RUSSIAN:
            LangBit = Lang::Russian;
            break;
        default:
            if (MsLangId::isFamilyNameFirst(eLang))
                LangBit = Lang::Eastern;
            else
                LangBit = Lang::Others;
            break;
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
        vRows.push_back(boost::make_shared<Row>(
            get<FixedText>(vRowInfo[iRow].pTextId), eRow));
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
            vFields.push_back(boost::make_shared<Field>(
                get<Edit>(vFieldInfo[iField].pEditId), iField));
            // "short name" field?
            if (vFieldInfo[iField].nUserOptionsId == USER_OPT_ID)
            {
                nNameRow = vRows.size() - 1;
                nShortNameField = vFields.size() - 1;
            }
        }
        rRow.nLastField = vFields.size();
    }
}

//------------------------------------------------------------------------

void SvxGeneralTabPage::SetLinks ()
{
    // link for updating the initials
    Link aLink = LINK( this, SvxGeneralTabPage, ModifyHdl_Impl );
    Row& rNameRow = *vRows[nNameRow];
    for (unsigned i = rNameRow.nFirstField; i != rNameRow.nLastField - 1; ++i)
        vFields[i]->pEdit->SetModifyHdl(aLink);
}

//------------------------------------------------------------------------

SfxTabPage* SvxGeneralTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return ( new SvxGeneralTabPage( pParent, rAttrSet ) );
}

//------------------------------------------------------------------------

sal_Bool SvxGeneralTabPage::FillItemSet( SfxItemSet& )
{
    // remove leading and trailing whitespaces
    for (unsigned i = 0; i != vFields.size(); ++i)
        vFields[i]->pEdit->SetText(comphelper::string::strip(vFields[i]->pEdit->GetText(), ' '));

    sal_Bool bModified = sal_False;
    bModified |= GetAddress_Impl();
    SvtSaveOptions aSaveOpt;
    if ( m_pUseDataCB->IsChecked() != aSaveOpt.IsUseUserData() )
    {
        aSaveOpt.SetUseUserData( m_pUseDataCB->IsChecked() );
        bModified |= sal_True;
    }
    return bModified;
}

//------------------------------------------------------------------------

void SvxGeneralTabPage::Reset( const SfxItemSet& rSet )
{
    SetAddress_Impl();

    sal_uInt16 const nWhich = GetWhich(SID_FIELD_GRABFOCUS);

    if (rSet.GetItemState(nWhich) == SFX_ITEM_SET)
    {
        if (sal_uInt16 const nField = ((SfxUInt16Item&)rSet.Get(nWhich)).GetValue())
        {
            for (unsigned i = 0; i != vFields.size(); ++i)
                if (nField == vFieldInfo[vFields[i]->iField].nGrabFocusId)
                    vFields[i]->pEdit->GrabFocus();
        }
        else
            vFields.front()->pEdit->GrabFocus();
    }

    m_pUseDataCB->Check( SvtSaveOptions().IsUseUserData() );
}

//------------------------------------------------------------------------

// ModifyHdl_Impl()
// This handler updates the initials (short name)
// when one of the name fields was updated.
IMPL_LINK( SvxGeneralTabPage, ModifyHdl_Impl, Edit *, pEdit )
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
        if (vFields[rNameRow.nFirstField + i]->pEdit == pEdit)
            nField = i;
    }
    // updating the initial
    if (nField < nInits && rShortName.pEdit->IsEnabled())
    {
        OUString sShortName = rShortName.pEdit->GetText();
        // clear short name if it contains more characters than the number of initials
        if ((unsigned)sShortName.getLength() > nInits)
        {
            rShortName.pEdit->SetText(OUString());
        }
        while ((unsigned)sShortName.getLength() < nInits)
            sShortName += OUString(' ');
        OUString sName = pEdit->GetText();
        OUString sLetter = OUString(sName.getLength() ? sName.toChar() : ' ');
        rShortName.pEdit->SetText(sShortName.replaceAt(nField, 1, sLetter).trim());
    }
    return 0;
}

//------------------------------------------------------------------------

sal_Bool SvxGeneralTabPage::GetAddress_Impl()
{
    // updating
    SvtUserOptions aUserOpt;
    for (unsigned i = 0; i != vFields.size(); ++i)
        aUserOpt.SetToken(
            vFieldInfo[vFields[i]->iField].nUserOptionsId,
            vFields[i]->pEdit->GetText()
        );

    // modified?
    for (unsigned i = 0; i != vFields.size(); ++i)
        if (vFields[i]->pEdit->GetSavedValue() != vFields[i]->pEdit->GetText())
            return true;
    return false;
}

//------------------------------------------------------------------------

void SvxGeneralTabPage::SetAddress_Impl()
{
    // updating and disabling edit boxes
    SvtUserOptions aUserOpt;
    for (unsigned iRow = 0; iRow != vRows.size(); ++iRow)
    {
        Row& rRow = *vRows[iRow];
        // the label is enabled if any of its edit fields are enabled
        bool bEnableLabel = false;
        for (unsigned iField = rRow.nFirstField; iField != rRow.nLastField; ++iField)
        {
            Field& rField = *vFields[iField];
            // updating content
            unsigned const nToken = vFieldInfo[rField.iField].nUserOptionsId;
            rField.pEdit->SetText(aUserOpt.GetToken(nToken));
            // is enabled?
            bool const bEnableEdit = !aUserOpt.IsTokenReadonly(nToken);
            rField.pEdit->Enable(bEnableEdit);
            bEnableLabel = bEnableLabel || bEnableEdit;
        }
        rRow.pLabel->Enable(bEnableLabel);
    }

    // saving
    for (unsigned i = 0; i != vFields.size(); ++i)
        vFields[i]->pEdit->SaveValue();
}

// -----------------------------------------------------------------------

int SvxGeneralTabPage::DeactivatePage( SfxItemSet* pSet_ )
{
    if ( pSet_ )
        FillItemSet( *pSet_ );
    return LEAVE_PAGE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

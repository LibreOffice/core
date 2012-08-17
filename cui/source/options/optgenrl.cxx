/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <comphelper/string.hxx>
#include <i18npool/mslangid.hxx>
#include <tools/shl.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/saveopt.hxx>
#include <svl/intitem.hxx>
#include <vcl/edit.hxx>

#define SVX_OPTGENRL_CXX

#include <cuires.hrc>
#include "optgenrl.hrc"
#include <unotools/useroptions.hxx>
#include "cuioptgenrl.hxx"
#include <dialmgr.hxx>
#include <svx/dlgutil.hxx>
#include <svx/svxids.hrc> // SID_FIELD_GRABFOCUS

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
    int nTextId;
    // language flags (see Lang above):
    // which language is this row for?
    unsigned nLangFlags;
}
const vRowInfo[] =
{
    { FT_COMPANY,      Lang::All },
    { FT_NAME,         Lang::All & ~Lang::Russian & ~Lang::Eastern },
    { FT_NAME_RUSS,    Lang::Russian },
    { FT_NAME_EASTERN, Lang::Eastern },
    { FT_STREET,       Lang::All & ~Lang::Russian },
    { FT_STREET_RUSS,  Lang::Russian },
    { FT_CITY,         Lang::All & ~Lang::US },
    { FT_CITY_US,      Lang::US },
    { FT_COUNTRY,      Lang::All },
    { FT_TITLEPOS,     Lang::All },
    { FT_PHONE,        Lang::All },
    { FT_FAXMAIL,      Lang::All },
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
    int nEditId;
    // relative width
    // The actual width is calculated from the relative width to fill
    // the entire row. See PositionControls() below.
    float fRelativeWidth;
    // id for SvtUserOptions in unotools/useroptions.hxx
    int nUserOptionsId;
    // id for settings the focus (defined in svx/optgenrl.hxx)
    int nGrabFocusId;
}
const vFieldInfo[] =
{
    // Company
    { Row_Company,  ED_COMPANY, 1,  USER_OPT_COMPANY,  COMPANY_EDIT },
    // Name
    { Row_Name,         ED_FIRSTNAME,  5,  USER_OPT_FIRSTNAME, FIRSTNAME_EDIT },
    { Row_Name,         ED_NAME,       5,  USER_OPT_LASTNAME,  LASTNAME_EDIT  },
    { Row_Name,         ED_SHORTNAME,  2,  USER_OPT_ID,        SHORTNAME_EDIT },
    // Name (russian)
    { Row_Name_Russian, ED_NAME,       5,  USER_OPT_LASTNAME,  LASTNAME_EDIT  },
    { Row_Name_Russian, ED_FIRSTNAME,  5,  USER_OPT_FIRSTNAME, FIRSTNAME_EDIT },
    { Row_Name_Russian, ED_FATHERNAME, 5,  USER_OPT_FATHERSNAME, 0 },
    { Row_Name_Russian, ED_SHORTNAME,  2,  USER_OPT_ID,        SHORTNAME_EDIT },
    // Name (eastern: reversed name order)
    { Row_Name_Eastern, ED_NAME,       5,  USER_OPT_LASTNAME,  LASTNAME_EDIT  },
    { Row_Name_Eastern, ED_FIRSTNAME,  5,  USER_OPT_FIRSTNAME, FIRSTNAME_EDIT },
    { Row_Name_Eastern, ED_SHORTNAME,  2,  USER_OPT_ID,        SHORTNAME_EDIT },
    // Street
    { Row_Street,          ED_STREET,      1,  USER_OPT_STREET, STREET_EDIT },
    // Street (russian)
    { Row_Street_Russian,  ED_STREET,      8,  USER_OPT_STREET, STREET_EDIT },
    { Row_Street_Russian,  ED_APARTMENTNR, 1,  USER_OPT_APARTMENT, 0 },
    // City
    { Row_City,     ED_PLZ,        1,  USER_OPT_ZIP,   PLZ_EDIT },
    { Row_City,     ED_CITY,       5,  USER_OPT_CITY,  CITY_EDIT },
    // City (US)
    { Row_City_US,  ED_US_CITY,   15,  USER_OPT_CITY,  CITY_EDIT },
    { Row_City_US,  ED_US_STATE,   5,  USER_OPT_STATE, STATE_EDIT },
    { Row_City_US,  ED_US_ZIPCODE, 4,  USER_OPT_ZIP,   PLZ_EDIT },
    // Country
    { Row_Country,  ED_COUNTRY,    1,  USER_OPT_COUNTRY, COUNTRY_EDIT },
    // Title/Position
    { Row_TitlePos, ED_TITLE,      1,  USER_OPT_TITLE,    TITLE_EDIT },
    { Row_TitlePos, ED_POSITION,   1,  USER_OPT_POSITION, POSITION_EDIT },
    // Phone
    { Row_Phone,    ED_TELPRIVAT,  1,  USER_OPT_TELEPHONEHOME, TELPRIV_EDIT },
    { Row_Phone,    ED_TELCOMPANY, 1,  USER_OPT_TELEPHONEWORK, TELCOMPANY_EDIT },
    // Fax/Mail
    { Row_FaxMail,  ED_FAX,        1,  USER_OPT_FAX,   FAX_EDIT },
    { Row_FaxMail,  ED_EMAIL,      1,  USER_OPT_EMAIL, EMAIL_EDIT },
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
    FixedText aLabel;
    // first and last field in the row (last is exclusive)
    unsigned nFirstField, nLastField;

public:
    Row (Window& rParent, int nResId, RowType eRow_) :
        eRow(eRow_),
        aLabel(&rParent, CUI_RES(nResId), true),
        nFirstField(0), nLastField(0)
    { }
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
    Edit aEdit;

public:
    Field (Window& rParent, int nResId, unsigned iField_) :
        iField(iField_),
        aEdit(&rParent, CUI_RES(nResId), true)
    { }
};

// -----------------------------------------------------------------------

SvxGeneralTabPage::SvxGeneralTabPage( Window* pParent, const SfxItemSet& rCoreSet ) :

    SfxTabPage( pParent, CUI_RES(RID_SFXPAGE_GENERAL), rCoreSet ),

    aAddrFrm        ( this, CUI_RES( GB_ADDRESS ) ),
    aUseDataCB      ( this, CUI_RES( CB_USEDATA ) )
{
    CreateControls();
    PositionControls();
    SetExchangeSupport(); // this page needs ExchangeSupport
    SetLinks();
    SetAccessibleNames();
}

//------------------------------------------------------------------------

SvxGeneralTabPage::~SvxGeneralTabPage ()
{ }

//------------------------------------------------------------------------

// Creates and initializes the titles and the edit boxes,
// according to vRowInfo[] and vFieldInfo[] above.
void SvxGeneralTabPage::CreateControls ()
{
    // which language bit do we use? (see Lang and vRowInfo[] above)
    unsigned LangBit;
    switch (LanguageType const eLang = Application::GetSettings().GetUILanguage())
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
            *this, vRowInfo[iRow].nTextId, eRow
        ));
        Row& rRow = *vRows.back();
        // fields in the row
        static unsigned const nFieldCount = sizeof vFieldInfo / sizeof vFieldInfo[0];
        // skipping other (invisible) rows
        while (iField != nFieldCount && vFieldInfo[iField].eRow != eRow)
            ++iField;
        // fields in the row
        rRow.nFirstField = vFields.size();
        for ( ; iField != nFieldCount && vFieldInfo[iField].eRow == eRow; ++iField)
        {
            // creating edit field
            vFields.push_back(boost::make_shared<Field>(
                *this, vFieldInfo[iField].nEditId, iField
            ));
            // "short name" field?
            if (vFieldInfo[iField].nEditId == ED_SHORTNAME)
            {
                nNameRow = vRows.size() - 1;
                nShortNameField = vFields.size() - 1;
            }
        }
        rRow.nLastField = vFields.size();
    }

    FreeResource();
}

//------------------------------------------------------------------------

// sets the size and the position of the controls
void SvxGeneralTabPage::PositionControls ()
{
    // sizes and locations
    int const nLeft = 12, nMid = 100, nRight = 250;
    int const nTop = 14;
    unsigned const nHSpace = 2, nVSpace = 3;
    unsigned const nRowHeight = 15, nTextVMargin = 2;

    Point aLabelPos(nLeft, nTop + nTextVMargin);
    Size aLabelSize(nMid - nLeft - nHSpace, nRowHeight - nVSpace - 2*nTextVMargin);
    for (unsigned iRow = 0; iRow != vRows.size(); ++iRow, aLabelPos.Y() += nRowHeight)
    {
        Row& rRow = *vRows[iRow];
        // label
        rRow.aLabel.SetPosSizePixel(
            LogicToPixel(aLabelPos,  MAP_APPFONT),
            LogicToPixel(aLabelSize, MAP_APPFONT)
        );
        // field position
        Point aFieldPos(nMid, aLabelPos.Y() - nTextVMargin);
        Size aFieldSize(0, nRowHeight - nVSpace);
        // sum of the relative widths
        float fRelWidthSum = 0;
        for (unsigned iField = rRow.nFirstField; iField != rRow.nLastField; ++iField)
            fRelWidthSum += vFieldInfo[vFields[iField]->iField].fRelativeWidth;
        // sum of the actual widths (total width - spaces)
        unsigned const nActWidthSum =
            (nRight - nMid) - nHSpace*(rRow.nLastField - rRow.nFirstField);
        // calculating the actual widths
        float X = nMid; // starting position
        for (unsigned iField = rRow.nFirstField; iField != rRow.nLastField; ++iField)
        {
            // calculating position and size
            Field& rField = *vFields[iField];
            aFieldPos.X() = X;
            X += vFieldInfo[rField.iField].fRelativeWidth * nActWidthSum / fRelWidthSum;
            aFieldSize.Width() = X - aFieldPos.X();
            X += 2;
            // setting size
            rField.aEdit.SetPosSizePixel(
                LogicToPixel(aFieldPos,  MAP_APPFONT),
                LogicToPixel(aFieldSize, MAP_APPFONT)
            );
        }
    }
}

//------------------------------------------------------------------------

void SvxGeneralTabPage::SetLinks ()
{
    // link for updating the initials
    Link aLink = LINK( this, SvxGeneralTabPage, ModifyHdl_Impl );
    Row& rNameRow = *vRows[nNameRow];
    for (unsigned i = rNameRow.nFirstField; i != rNameRow.nLastField - 1; ++i)
        vFields[i]->aEdit.SetModifyHdl(aLink);
}

//------------------------------------------------------------------------

void SvxGeneralTabPage::SetAccessibleNames ()
{
    // Because some labels have text for more than one edit field we have to
    // split these texts and set these texts as accessible name
    // of the corresponding edit fields.
    // E.g. "City/State/Zip" -> "City", "State", "Zip" or
    // "Tel. (Home/Work)" -> "Tel. (Home)", "Tel. (Work)"
    for (unsigned i = 0; i != vRows.size(); ++i)
    {
        Row& rRow = *vRows[i];
        rtl::OUString const sLabel = rRow.aLabel.GetDisplayText();
        rtl::OUString sList = sLabel; // between brackets or the whole label
        // brackets?
        int iBracket = sLabel.indexOf('(');
        if (iBracket != -1)
            sList = sList.copy(iBracket + 1, sLabel.lastIndexOf(')') - iBracket - 1);
        // cutting at '/'s
        int nIndex = 0;
        for (unsigned iField = rRow.nFirstField; iField != rRow.nLastField; ++iField)
        {
            // the token
            rtl::OUString sPart = sList.getToken(0, '/', nIndex).trim();
            Edit& rEdit = vFields[iField]->aEdit;
            // creating the accessible name
            if (iBracket != -1)
                rEdit.SetAccessibleName(sLabel.copy(0, iBracket) + "(" + sPart + ")");
            else
                rEdit.SetAccessibleName(sPart);
        }
    }
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
        vFields[i]->aEdit.SetText( comphelper::string::strip(vFields[i]->aEdit.GetText(), ' ') );

    sal_Bool bModified = sal_False;
    bModified |= GetAddress_Impl();
    SvtSaveOptions aSaveOpt;
    if ( aUseDataCB.IsChecked() != aSaveOpt.IsUseUserData() )
    {
        aSaveOpt.SetUseUserData( aUseDataCB.IsChecked() );
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
                    vFields[i]->aEdit.GrabFocus();
        }
        else
            vFields.front()->aEdit.GrabFocus();
    }

    aUseDataCB.Check( SvtSaveOptions().IsUseUserData() );
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
        if (&vFields[rNameRow.nFirstField + i]->aEdit == pEdit)
            nField = i;
    // updating the initial
    if (nField < nInits && rShortName.aEdit.IsEnabled())
    {
        rtl::OUString sShortName = rShortName.aEdit.GetText();
        while ((unsigned)sShortName.getLength() < nInits)
            sShortName += rtl::OUString(' ');
        rtl::OUString sName = pEdit->GetText();
        rtl::OUString sLetter = rtl::OUString(sName.getLength() ? sName.toChar() : ' ');
        rShortName.aEdit.SetText(sShortName.replaceAt(nField, 1, sLetter).trim());
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
            vFields[i]->aEdit.GetText()
        );

    // modified?
    for (unsigned i = 0; i != vFields.size(); ++i)
        if (vFields[i]->aEdit.GetSavedValue() != vFields[i]->aEdit.GetText())
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
            rField.aEdit.SetText(aUserOpt.GetToken(nToken));
            // is enabled?
            bool const bEnableEdit = !aUserOpt.IsTokenReadonly(nToken);
            rField.aEdit.Enable(bEnableEdit);
            bEnableLabel = bEnableLabel || bEnableEdit;
        }
        rRow.aLabel.Enable(bEnableLabel);
    }

    // saving
    for (unsigned i = 0; i != vFields.size(); ++i)
        vFields[i]->aEdit.SaveValue();
}

// -----------------------------------------------------------------------

int SvxGeneralTabPage::DeactivatePage( SfxItemSet* pSet_ )
{
    if ( pSet_ )
        FillItemSet( *pSet_ );
    return LEAVE_PAGE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

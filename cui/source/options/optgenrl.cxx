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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cui.hxx"

// include ---------------------------------------------------------------
#include <tools/shl.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/saveopt.hxx>
#include <svl/intitem.hxx>

#define _SVX_OPTGENRL_CXX

#include <cuires.hrc>
#include "optgenrl.hrc"
#include <unotools/useroptions.hxx>
#include "cuioptgenrl.hxx"
#include <dialmgr.hxx>
#include <svx/dlgutil.hxx>
#include <svx/strarray.hxx>
#include <svx/svxids.hrc> // SID_FIELD_GRABFOCUS

#define TRIM(s) s.EraseLeadingChars().EraseTrailingChars()

// struct GeneralTabPage_Impl --------------------------------------------

struct GeneralTabPage_Impl
{
    sal_Bool    mbStreetEnabled;
    sal_Bool    mbPLZEnabled;
    sal_Bool    mbCityEnabled;
    sal_Bool    mbUsCityEnabled;
    sal_Bool    mbUsZipEnabled;

    String  maQueryStr;

    GeneralTabPage_Impl() :
        mbStreetEnabled ( sal_False ),
        mbPLZEnabled    ( sal_False ),
        mbCityEnabled   ( sal_False ),
        mbUsCityEnabled ( sal_False ),
        mbUsZipEnabled  ( sal_False ) {}
};

// -----------------------------------------------------------------------

SvxGeneralTabPage::SvxGeneralTabPage( Window* pParent, const SfxItemSet& rCoreSet ) :

    SfxTabPage( pParent, CUI_RES(RID_SFXPAGE_GENERAL), rCoreSet ),

    aAddrFrm        ( this, CUI_RES( GB_ADDRESS ) ),
    aCompanyLbl     ( this, CUI_RES( FT_COMPANY ), true ),
    aCompanyEdit    ( this, CUI_RES( ED_COMPANY ), INDEX_NOTSET, &aCompanyLbl ),
    aNameLbl        ( this, CUI_RES( FT_NAME ), true ),
    aNameLblRuss    ( this, CUI_RES( FT_NAME_RUSS ), true ),
    aNameLblEastern ( this, CUI_RES( FT_NAME_EASTERN ), true ),
    aFirstName      ( this, CUI_RES( ED_FIRSTNAME ), 0, &aNameLbl ),
    aFatherName     ( this, CUI_RES( ED_FATHERNAME ) ),
    aName           ( this, CUI_RES( ED_NAME ), 1, &aNameLbl ),
    aShortName      ( this, CUI_RES( ED_SHORTNAME ), 2, &aNameLbl ),
    aStreetLbl      ( this, CUI_RES( FT_STREET ), true ),
    aStreetLblRuss  ( this, CUI_RES( FT_STREET_RUSS ), true ),
    aStreetEdit     ( this, CUI_RES( ED_STREET ), 0, &aStreetLbl ),
    aApartmentNrEdit( this, CUI_RES( ED_APARTMENTNR ), 1, &aStreetLblRuss ),
    aCityLbl        ( this, CUI_RES( FT_CITY ), true ),
    aPLZEdit        ( this, CUI_RES( ED_PLZ ), 0, &aCityLbl ),
    aCityEdit       ( this, CUI_RES( ED_CITY ), 1, &aCityLbl ),
    aUsCityEdit     ( this, CUI_RES( ED_US_CITY ), 0, &aCityLbl ),
    aUsStateEdit    ( this, CUI_RES( ED_US_STATE ), 1, &aCityLbl ),
    aUsZipEdit      ( this, CUI_RES( ED_US_ZIPCODE ), 2, &aCityLbl ),
    aCountryLbl     ( this, CUI_RES( FT_COUNTRY ), true ),
    aCountryEdit    ( this, CUI_RES( ED_COUNTRY ), INDEX_NOTSET, &aCountryLbl ),
    aTitlePosLbl    ( this, CUI_RES( FT_TITLEPOS ), true ),
    aTitleEdit      ( this, CUI_RES( ED_TITLE ), 0, &aTitlePosLbl ),
    aPositionEdit   ( this, CUI_RES( ED_POSITION ), 1, &aTitlePosLbl ),
    aPhoneLbl       ( this, CUI_RES( FT_PHONE ), true ),
    aTelPrivEdit    ( this, CUI_RES( ED_TELPRIVAT ), 0, &aPhoneLbl ),
    aTelCompanyEdit ( this, CUI_RES( ED_TELCOMPANY ), 1, &aPhoneLbl ),
    aFaxMailLbl     ( this, CUI_RES( FT_FAXMAIL ), true ),
    aFaxEdit        ( this, CUI_RES( ED_FAX ), 0, &aFaxMailLbl ),
    aEmailEdit      ( this, CUI_RES( ED_EMAIL ), 1, &aFaxMailLbl ),
    aUseDataCB      ( this, CUI_RES( CB_USEDATA ) ),
    pImpl           ( new GeneralTabPage_Impl )

{
    LanguageType eLang = Application::GetSettings().GetUILanguage();
    pImpl->maQueryStr = String( CUI_RES( STR_QUERY_REG ) );

    if ( LANGUAGE_ENGLISH_US == eLang )
    {
        // amerikanische Postadresse aufbauen
        aPLZEdit.Hide();
        aCityEdit.Hide();
        aCityLbl.SetText( CUI_RES( STR_US_STATE ) );
    }
    else if ( LANGUAGE_RUSSIAN == eLang )
    {
        aUsCityEdit.Hide();
        aUsStateEdit.Hide();
        aUsZipEdit.Hide();
        aNameLbl.Hide();
        aNameLblRuss.Show();
        aStreetLbl.Hide();
        aStreetLblRuss.Show();
        aFatherName.Show();
        aName.SetIndex( 0 );
        aName.SetLabel( &aNameLblRuss );
        aFirstName.SetIndex( 1 );
        aFirstName.SetLabel( &aNameLblRuss );
        aFatherName.SetIndex( 2 );
        aFatherName.SetLabel( &aNameLblRuss );
        aShortName.SetIndex( 3 );
        aShortName.SetLabel( &aNameLblRuss );

        Point aEditPoint = LogicToPixel( Point( MID, LINE(1) ), MAP_APPFONT );
        Point aRightPoint = LogicToPixel( Point( RIGHT, LINE(1) ), MAP_APPFONT );
        Size aEditSize = LogicToPixel( Size( 42, 12 ), MAP_APPFONT );
        Size a2Size = LogicToPixel( Size( 2, 2 ), MAP_APPFONT );
        long nDelta = aEditSize.Width() + a2Size.Width();
        aName.SetPosSizePixel( aEditPoint, aEditSize );
        aEditPoint.X() = aEditPoint.X() + nDelta;
        aFirstName.SetPosSizePixel( aEditPoint, aEditSize );
        aEditPoint.X() = aEditPoint.X() + nDelta;
        aFatherName.SetPosSizePixel( aEditPoint, aEditSize );
        aEditPoint.X() = aEditPoint.X() + nDelta;
        aEditSize.Width() = aRightPoint.X() - aEditPoint.X();
        aShortName.SetPosSizePixel( aEditPoint, aEditSize );

        Size aStreetSize = aStreetEdit.GetSizePixel();
        aStreetSize.Width() = aStreetSize.Width() - aEditSize.Width() - a2Size.Width();
        aStreetEdit.SetSizePixel( aStreetSize );
        aApartmentNrEdit.Show();
        Point aApartmentPoint = LogicToPixel( Point( MID, LINE(2) ), MAP_APPFONT );
        aApartmentPoint.X() = aEditPoint.X();
        aApartmentNrEdit.SetPosSizePixel( aApartmentPoint, aEditSize );

        aName.SetZOrder( &aNameLblRuss, WINDOW_ZORDER_BEHIND );
        aFirstName.SetZOrder( &aName, WINDOW_ZORDER_BEHIND );
        aFatherName.SetZOrder( &aFirstName, WINDOW_ZORDER_BEHIND );
    }
    else if ( LANGUAGE_JAPANESE == eLang ||
          LANGUAGE_KOREAN == eLang ||
          LANGUAGE_CHINESE_TRADITIONAL == eLang ||
          LANGUAGE_CHINESE_SIMPLIFIED == eLang)
    {
        aUsCityEdit.Hide();
        aUsStateEdit.Hide();
        aUsZipEdit.Hide();
        aNameLbl.Hide();
        aNameLblEastern.Show();

        // swap "first name" field and "last name" field
        Point aPosTmp = aFirstName.GetPosPixel();
        aFirstName.SetPosPixel( aName.GetPosPixel() );
        aName.SetPosPixel( aPosTmp );
        aFirstName.SetZOrder( &aName, WINDOW_ZORDER_BEHIND );
    }
    else
    {
        aUsCityEdit.Hide();
        aUsStateEdit.Hide();
        aUsZipEdit.Hide();
    }

    FreeResource();

    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    Link aLink = LINK( this, SvxGeneralTabPage, ModifyHdl_Impl );
    aFirstName.SetModifyHdl( aLink );
    aName.SetModifyHdl( aLink );

    // because some labels have text for more than one edit field we have to split these texts
    // and set these texts as accessible name of the corresponding edit fields
    SvxUserEdit* pEdits[] =
    {
        &aCompanyEdit, &aFirstName, &aFatherName, &aName, &aShortName, &aStreetEdit,
        &aApartmentNrEdit, &aPLZEdit, &aCityEdit, &aUsCityEdit, &aUsStateEdit, &aUsZipEdit,
        &aCountryEdit, &aTitleEdit, &aPositionEdit, &aTelPrivEdit, &aTelCompanyEdit,
        &aFaxEdit, &aEmailEdit, NULL
    };
    SvxUserEdit** pCurrent = pEdits;
    while ( *pCurrent )
    {
        Window* pLabel = (*pCurrent)->GetLabel();
        if ( pLabel )
        {
            String sName, sText = pLabel->GetDisplayText();
            sal_Int16 nIndex = (*pCurrent)->GetIndex();
            if ( INDEX_NOTSET == nIndex )
                sName = sText;
            else
                sName = sText.GetToken( nIndex, '/' );
            sName.EraseAllChars( '(' );
            sName.EraseAllChars( ')' );
            if ( sName.Len() > 0 )
                (*pCurrent)->SetAccessibleName( sName );
        }
        pCurrent++;
    }
}

//------------------------------------------------------------------------

SvxGeneralTabPage::~SvxGeneralTabPage()
{
    delete pImpl;
}

//------------------------------------------------------------------------

SfxTabPage* SvxGeneralTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return ( new SvxGeneralTabPage( pParent, rAttrSet ) );
}

//------------------------------------------------------------------------

sal_Bool SvxGeneralTabPage::FillItemSet( SfxItemSet& )
{
    // Eingaben trimmen (f"uhrende und nachfolgende Leerzeichen entfernen)
    aCompanyEdit.SetText( TRIM(aCompanyEdit.GetText()) );
    aFirstName.SetText( TRIM(aFirstName.GetText()) );
    aName.SetText( TRIM(aName.GetText()) );
    aShortName.SetText( TRIM(aShortName.GetText()) );
    aStreetEdit.SetText( TRIM(aStreetEdit.GetText()) );
    aCountryEdit.SetText( TRIM(aCountryEdit.GetText()) );
    aPLZEdit.SetText( TRIM(aPLZEdit.GetText()) );
    aCityEdit.SetText( TRIM(aCityEdit.GetText()) );
    aUsCityEdit.SetText( TRIM(aUsCityEdit.GetText()) );
    aUsStateEdit.SetText( TRIM(aUsStateEdit.GetText()) );
    aUsZipEdit.SetText( TRIM(aUsZipEdit.GetText()) );
    aTitleEdit.SetText( TRIM(aTitleEdit.GetText()) );
    aPositionEdit.SetText( TRIM(aPositionEdit.GetText()) );
    aTelPrivEdit.SetText( TRIM(aTelPrivEdit.GetText()) );
    aTelCompanyEdit.SetText( TRIM(aTelCompanyEdit.GetText()) );
    aFaxEdit.SetText( TRIM(aFaxEdit.GetText()) );
    aEmailEdit.SetText( TRIM(aEmailEdit.GetText()) );

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

    sal_uInt16 nWhich = GetWhich( SID_FIELD_GRABFOCUS );
    if ( rSet.GetItemState( nWhich ) == SFX_ITEM_SET )
    {
        sal_uInt16 nField = ( (SfxUInt16Item&)rSet.Get( nWhich ) ).GetValue();

        switch ( nField )
        {
            case COMPANY_EDIT:      aCompanyEdit.GrabFocus(); break;
            case FIRSTNAME_EDIT:    aFirstName.GrabFocus(); break;
            case LASTNAME_EDIT:     aName.GrabFocus(); break;
            case STREET_EDIT:       aStreetEdit.GrabFocus(); break;
            case COUNTRY_EDIT:      aCountryEdit.GrabFocus(); break;
            case PLZ_EDIT:
                if ( aPLZEdit.IsVisible() )
                    aPLZEdit.GrabFocus();
                else
                    aUsZipEdit.GrabFocus();
                break;
            case CITY_EDIT:
                if ( aCityEdit.IsVisible() )
                    aCityEdit.GrabFocus();
                else
                    aUsCityEdit.GrabFocus();
                break;
            case STATE_EDIT:
                if ( aUsStateEdit.IsVisible() )
                    aUsStateEdit.GrabFocus();
                else
                    aCityEdit.GrabFocus();
                break;
            case TITLE_EDIT:        aTitleEdit.GrabFocus(); break;
            case POSITION_EDIT:     aPositionEdit.GrabFocus(); break;
            case SHORTNAME_EDIT:    aShortName.GrabFocus(); break;
            case TELPRIV_EDIT:      aTelPrivEdit.GrabFocus(); break;
            case TELCOMPANY_EDIT:   aTelCompanyEdit.GrabFocus(); break;
            case FAX_EDIT:          aFaxEdit.GrabFocus(); break;
            case EMAIL_EDIT:        aEmailEdit.GrabFocus(); break;
            default:                aCompanyEdit.GrabFocus();
        }
    }

    aUseDataCB.Check( SvtSaveOptions().IsUseUserData() );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxGeneralTabPage, ModifyHdl_Impl, Edit *, pEdit )
{
    if ( aShortName.IsEnabled() )
    {
        String aShortStr( aShortName.GetText() );
        switch ( aShortStr.Len() )
        {
            case 0:
                aShortStr = String( RTL_CONSTASCII_STRINGPARAM("  ") );
                break;

            case 1:
                aShortStr += ' ';
                break;
        }

        sal_uInt16 nPos = ( pEdit == &aFirstName ) ? 0 : 1;
        String aTxt = pEdit->GetText();
        sal_Unicode cChar = ( aTxt.Len() > 0 ) ? aTxt.GetChar(0) : ' ';
        aShortStr.SetChar( nPos, cChar );
        aShortStr.EraseTrailingChars();
        aShortName.SetText( aShortStr );
    }
    return 0;
}

//------------------------------------------------------------------------

sal_Bool SvxGeneralTabPage::GetAddress_Impl()
{
    sal_Bool bRet =
    (   aCompanyEdit.GetSavedValue()  !=        aCompanyEdit.GetText()  ||
        aFirstName.GetSavedValue()  !=          aFirstName.GetText()  ||
        aFatherName.GetSavedValue()  !=         aFatherName.GetText()  ||
        aName.GetSavedValue()  !=               aName.GetText()  ||
        aShortName.GetSavedValue()  !=          aShortName.GetText()  ||
        aStreetEdit.GetSavedValue()  !=         aStreetEdit.GetText()  ||
        aApartmentNrEdit.GetSavedValue()  !=    aApartmentNrEdit.GetText()  ||
        aPLZEdit.GetSavedValue()  !=            aPLZEdit.GetText()  ||
        aCityEdit.GetSavedValue()  !=           aCityEdit.GetText()  ||
        aUsCityEdit.GetSavedValue()  !=         aUsCityEdit.GetText()  ||
        aUsStateEdit.GetSavedValue()  !=        aUsStateEdit.GetText()  ||
        aUsZipEdit.GetSavedValue()  !=          aUsZipEdit.GetText()  ||
        aCountryEdit.GetSavedValue()  !=        aCountryEdit.GetText()  ||
        aTitleEdit.GetSavedValue()  !=          aTitleEdit.GetText()  ||
        aPositionEdit.GetSavedValue()  !=       aPositionEdit.GetText()  ||
        aTelPrivEdit.GetSavedValue()  !=        aTelPrivEdit.GetText()  ||
        aTelCompanyEdit.GetSavedValue()  !=     aTelCompanyEdit.GetText()  ||
        aFaxEdit.GetSavedValue()  !=            aFaxEdit.GetText()  ||
        aEmailEdit.GetSavedValue()  !=          aEmailEdit.GetText() );

    LanguageType eLang = Application::GetSettings().GetUILanguage();
    sal_Bool bUS = ( LANGUAGE_ENGLISH_US == eLang );

    SvtUserOptions aUserOpt;
    aUserOpt.SetCompany(aCompanyEdit.GetText());
    aUserOpt.SetFirstName(aFirstName.GetText());
    aUserOpt.SetLastName(aName.GetText());
    aUserOpt.SetID( aShortName.GetText());

    aUserOpt.SetStreet(aStreetEdit.GetText() );

    aUserOpt.SetCountry(aCountryEdit.GetText() );

    aUserOpt.SetZip(bUS ? aUsZipEdit.GetText() : aPLZEdit.GetText() );
    aUserOpt.SetCity(bUS ? aUsCityEdit.GetText() : aCityEdit.GetText() );

    aUserOpt.SetTitle( aTitleEdit.GetText() );
    aUserOpt.SetPosition(aPositionEdit.GetText() );
    aUserOpt.SetTelephoneHome( aTelPrivEdit.GetText() );
    aUserOpt.SetTelephoneWork( aTelCompanyEdit.GetText() );
    aUserOpt.SetFax( aFaxEdit.GetText() );
    aUserOpt.SetEmail( aEmailEdit.GetText() );
    aUserOpt.SetState( bUS ? aUsStateEdit.GetText() : String() );

    if ( LANGUAGE_RUSSIAN == eLang )
    {
        aUserOpt.SetFathersName( aFatherName.GetText() );
        aUserOpt.SetApartment( aApartmentNrEdit.GetText() );
    }
    return bRet;
}

//------------------------------------------------------------------------

void SvxGeneralTabPage::SetAddress_Impl()
{
    LanguageType eLang = Application::GetSettings().GetUILanguage();
    sal_Bool bUS = ( LANGUAGE_ENGLISH_US == eLang );
    SvtUserOptions aUserOpt;
    aCompanyEdit.SetText( aUserOpt.GetCompany() );
    if ( aUserOpt.IsTokenReadonly( USER_OPT_COMPANY ) )
    {
        aCompanyLbl.Disable();
        aCompanyEdit.Disable();
    }
    sal_Int16 nEditCount = 0;
    aFirstName.SetText( aUserOpt.GetFirstName() );
    if ( aUserOpt.IsTokenReadonly( USER_OPT_FIRSTNAME ) )
    {
        aFirstName.Disable();
        nEditCount++;
    }
    aName.SetText( aUserOpt.GetLastName() );
    if ( aUserOpt.IsTokenReadonly( USER_OPT_LASTNAME ) )
    {
        aName.Disable();
        nEditCount++;
    }
    aShortName.SetText( aUserOpt.GetID() );
    if ( aUserOpt.IsTokenReadonly( USER_OPT_ID ) )
    {
        aShortName.Disable();
        nEditCount++;
    }
    aNameLbl.Enable( ( nEditCount != 3 ) );
    aStreetEdit.SetText( aUserOpt.GetStreet() );
    if ( aUserOpt.IsTokenReadonly( USER_OPT_STREET ) )
    {
        aStreetLbl.Disable();
        aStreetEdit.Disable();
    }
    Edit* pPLZEdit = bUS ? &aUsZipEdit : &aPLZEdit;
    Edit* pCityEdit = bUS ? &aUsCityEdit : &aCityEdit;
    pPLZEdit->SetText( aUserOpt.GetZip() );
    pCityEdit->SetText( aUserOpt.GetCity() );
    nEditCount = 0;
    if ( aUserOpt.IsTokenReadonly( USER_OPT_ZIP ) )
    {
        pPLZEdit->Disable();
        nEditCount++;
    }
    if ( aUserOpt.IsTokenReadonly( USER_OPT_CITY ) )
    {
        pCityEdit->Disable();
        nEditCount++;
    }
    if ( bUS )
    {
        aUsStateEdit.SetText( aUserOpt.GetState() );
        if ( aUserOpt.IsTokenReadonly( USER_OPT_STATE ) )
        {
            aUsStateEdit.Disable();
            nEditCount++;
        }
    }
    aCityLbl.Enable( ( nEditCount != ( bUS ? 3 : 2 ) ) );
    aCountryEdit.SetText( aUserOpt.GetCountry() );
    if ( aUserOpt.IsTokenReadonly( USER_OPT_COUNTRY ) )
    {
        aCountryLbl.Disable();
        aCountryEdit.Disable();
    }
    aTitleEdit.SetText( aUserOpt.GetTitle() );
    aPositionEdit.SetText( aUserOpt.GetPosition() );
    nEditCount = 0;
    if ( aUserOpt.IsTokenReadonly( USER_OPT_TITLE ) )
    {
        aTitleEdit.Disable();
        nEditCount++;
    }
    if ( aUserOpt.IsTokenReadonly( USER_OPT_POSITION ) )
    {
        aPositionEdit.Disable();
        nEditCount++;
    }
    aTitlePosLbl.Enable( ( nEditCount != 2 ) );
    aTelPrivEdit.SetText( aUserOpt.GetTelephoneHome() );
    aTelCompanyEdit.SetText( aUserOpt.GetTelephoneWork() );
    nEditCount = 0;
    if ( aUserOpt.IsTokenReadonly( USER_OPT_TELEPHONEHOME ) )
    {
        aTelPrivEdit.Disable();
        nEditCount++;
    }
    if ( aUserOpt.IsTokenReadonly( USER_OPT_TELEPHONEWORK ) )
    {
        aTelCompanyEdit.Disable();
        nEditCount++;
    }
    aPhoneLbl.Enable( ( nEditCount != 2 ) );
    aFaxEdit.SetText( aUserOpt.GetFax() );
    aEmailEdit.SetText( aUserOpt.GetEmail() );
    nEditCount = 0;
    if ( aUserOpt.IsTokenReadonly( USER_OPT_FAX ) )
    {
        aFaxEdit.Disable();
        nEditCount++;
    }
    if ( aUserOpt.IsTokenReadonly( USER_OPT_EMAIL ) )
    {
        aEmailEdit.Disable();
        nEditCount++;
    }
    aFaxMailLbl.Enable( ( nEditCount != 2 ) );

    if ( LANGUAGE_RUSSIAN == eLang )
    {
        aFatherName.SetText( aUserOpt.GetFathersName() );
        aApartmentNrEdit.SetText( aUserOpt.GetApartment() );
    }

    aCompanyEdit.SaveValue();
    aFirstName.SaveValue();
    aFatherName.SaveValue();
    aName.SaveValue();
    aShortName.SaveValue();
    aStreetEdit.SaveValue();
    aApartmentNrEdit.SaveValue();
    aPLZEdit.SaveValue();
    aCityEdit.SaveValue();
    aUsCityEdit.SaveValue();
    aUsStateEdit.SaveValue();
    aUsZipEdit.SaveValue();
    aCountryEdit.SaveValue();
    aTitleEdit.SaveValue();
    aPositionEdit.SaveValue();
    aTelPrivEdit.SaveValue();
    aTelCompanyEdit.SaveValue();
    aFaxEdit.SaveValue();
    aEmailEdit.SaveValue();
}

// -----------------------------------------------------------------------

int SvxGeneralTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( *_pSet );
    return LEAVE_PAGE;
}


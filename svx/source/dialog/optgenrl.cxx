/*************************************************************************
 *
 *  $RCSfile: optgenrl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-04-29 16:23:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// include ---------------------------------------------------------------

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _PVER_HXX //autogen
#include <svtools/pver.hxx>
#endif
#include <svtools/saveopt.hxx>
#pragma hdrstop

#define _SVX_OPTGENRL_CXX

#include "dialogs.hrc"
#include "optgenrl.hrc"

#ifndef INCLUDED_SVTOOLS_USEROPTIONS_HXX
#include <svtools/useroptions.hxx>
#endif
#include "cuioptgenrl.hxx"
#include "dialmgr.hxx"
#include "dlgutil.hxx"
#include "strarray.hxx"

#define TRIM(s) s.EraseLeadingChars().EraseTrailingChars()

// struct GeneralTabPage_Impl --------------------------------------------

struct GeneralTabPage_Impl
{
    BOOL    mbStreetEnabled;
    BOOL    mbPLZEnabled;
    BOOL    mbCityEnabled;
    BOOL    mbUsCityEnabled;
    BOOL    mbUsZipEnabled;

    String  maQueryStr;

    GeneralTabPage_Impl() :
        mbStreetEnabled ( FALSE ),
        mbPLZEnabled    ( FALSE ),
        mbCityEnabled   ( FALSE ),
        mbUsCityEnabled ( FALSE ),
        mbUsZipEnabled  ( FALSE ) {}
};

// -----------------------------------------------------------------------

// kommt aus adritem.cxx
//CHINA001 extern String ConvertToStore_Impl( const String& );
//copy from adritem.cxx, since it will leave in svx.
String ConvertToStore_Impl( const String& rText )
{
    String sRet;
    USHORT i = 0;

    while ( i < rText.Len() )
    {
        if ( rText.GetChar(i) == '\\' || rText.GetChar(i) == '#' )
            sRet += '\\';
        sRet += rText.GetChar(i++);
    }
    return sRet;
}


// -----------------------------------------------------------------------

SvxGeneralTabPage::SvxGeneralTabPage( Window* pParent, const SfxItemSet& rCoreSet ) :

    SfxTabPage( pParent, SVX_RES(RID_SFXPAGE_GENERAL), rCoreSet ),

    aCompanyLbl     ( this, ResId( FT_COMPANY ) ),
    aCompanyEdit    ( this, ResId( ED_COMPANY ) ),
    aNameLbl        ( this, ResId( FT_NAME ) ),
    aNameLblRuss    ( this, ResId( FT_NAME_RUSS ) ),
    aFatherName     ( this, ResId( ED_FATHERNAME ) ),
    aFirstName      ( this, ResId( ED_FIRSTNAME ) ),
    aName           ( this, ResId( ED_NAME ) ),
    aShortName      ( this, ResId( ED_SHORTNAME ) ),
    aStreetLbl      ( this, ResId( FT_STREET ) ),
    aStreetLblRuss  ( this, ResId( FT_STREET_RUSS ) ),
    aStreetEdit     ( this, ResId( ED_STREET ) ),
    aApartmentNrEdit( this, ResId( ED_APARTMENTNR ) ),
    aCityLbl        ( this, ResId( FT_CITY ) ),
    aPLZEdit        ( this, ResId( ED_PLZ ) ),
    aCityEdit       ( this, ResId( ED_CITY ) ),
    aUsCityEdit     ( this, ResId( ED_US_CITY ) ),
    aUsStateEdit    ( this, ResId( ED_US_STATE ) ),
    aUsZipEdit      ( this, ResId( ED_US_ZIPCODE ) ),
    aCountryLbl     ( this, ResId( FT_COUNTRY ) ),
    aCountryEdit    ( this, ResId( ED_COUNTRY ) ),
    aTitlePosLbl    ( this, ResId( FT_TITLEPOS ) ),
    aTitleEdit      ( this, ResId( ED_TITLE ) ),
    aPositionEdit   ( this, ResId( ED_POSITION ) ),
    aPhoneLbl       ( this, ResId( FT_PHONE ) ),
    aTelPrivEdit    ( this, ResId( ED_TELPRIVAT ) ),
    aTelCompanyEdit ( this, ResId( ED_TELCOMPANY ) ),
    aFaxMailLbl     ( this, ResId( FT_FAXMAIL ) ),
    aFaxEdit        ( this, ResId( ED_FAX ) ),
    aEmailEdit      ( this, ResId( ED_EMAIL ) ),
    aAddrFrm        ( this, ResId( GB_ADDRESS ) ),
    aUseDataCB      ( this, ResId( CB_USEDATA ) ),

    pImpl           ( new GeneralTabPage_Impl )

{
    LanguageType eLang = Application::GetSettings().GetUILanguage();
    pImpl->maQueryStr = String( ResId( STR_QUERY_REG ) );

    if ( LANGUAGE_ENGLISH_US == eLang )
    {
        // amerikanische Postadresse aufbauen
        aPLZEdit.Hide();
        aCityEdit.Hide();
        aCityLbl.SetText( ResId( STR_US_STATE ) );
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
        aFirstName.SetPosSizePixel( LogicToPixel( Point( 88, LINE(1) ), MAP_APPFONT ),
                                    LogicToPixel( Size( 46, 12 ), MAP_APPFONT ) );
        aFatherName.SetPosSizePixel( LogicToPixel( Point( 136, LINE(1) ), MAP_APPFONT ),
                                       LogicToPixel( Size( 46, 12 ), MAP_APPFONT ) );
        aName.SetPosSizePixel( LogicToPixel( Point( 184, LINE(1) ), MAP_APPFONT ),
                               LogicToPixel( Size( 46, 12 ), MAP_APPFONT ) );

        aStreetEdit.SetSizePixel( LogicToPixel( Size( 132, 12 ), MAP_APPFONT ) );
        aApartmentNrEdit.Show();
        aApartmentNrEdit.SetPosSizePixel( LogicToPixel( Point( 222, LINE(2) ), MAP_APPFONT ),
                                          LogicToPixel( Size( 26, 12 ), MAP_APPFONT ) );
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

BOOL SvxGeneralTabPage::FillItemSet( SfxItemSet& rCoreSet )
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

    BOOL bModified = FALSE, bChgAddr = FALSE;
    const SfxItemSet& rOldSet = GetItemSet();
    bModified |= GetAddress_Impl();
    SvtSaveOptions aSaveOpt;
    if ( aUseDataCB.IsChecked() != aSaveOpt.IsUseUserData() )
    {
        aSaveOpt.SetUseUserData( aUseDataCB.IsChecked() );
        bModified |= TRUE;
    }
    return bModified;
}

//------------------------------------------------------------------------

void SvxGeneralTabPage::Reset( const SfxItemSet& rSet )
{
    SetAddress_Impl();

    USHORT nWhich = GetWhich( SID_FIELD_GRABFOCUS );
    if ( rSet.GetItemState( nWhich ) == SFX_ITEM_SET )
    {
        USHORT nField = ( (SfxUInt16Item&)rSet.Get( nWhich ) ).GetValue();

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

        USHORT nPos = ( pEdit == &aFirstName ) ? 0 : 1;
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
    BOOL bRet =
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
    BOOL bUS = ( LANGUAGE_ENGLISH_US == eLang );

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
    BOOL bUS = ( LANGUAGE_ENGLISH_US == eLang );
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

int SvxGeneralTabPage::DeactivatePage( SfxItemSet* pSet )
{
    if ( pSet )
        FillItemSet( *pSet );
    return LEAVE_PAGE;
}


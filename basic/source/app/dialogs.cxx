/*************************************************************************
 *
 *  $RCSfile: dialogs.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: kz $ $Date: 2003-08-25 14:50:49 $
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

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#include <algorithm>

HACK( #define protected public )
#define protected public        // Kleine Schweinerei um an FreeResource ranzukommen
#ifndef _TOOLS_RC_HXX //autogen
#include <tools/rc.hxx>
#endif
#undef protected

#ifndef _SV_METRIC_HXX
#include <vcl/metric.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _CONFIG_HXX //autogen
#include <vcl/config.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _SV_FILEDLG_HXX //autogen
#include <svtools/filedlg.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _FSYS_HXX //autogen
#include <tools/fsys.hxx>
#endif
#ifndef _SVTOOLS_STRINGTRANSFER_HXX_
#include <svtools/stringtransfer.hxx>
#endif
#ifndef _SV_SPLITWIN_HXX //autogen
#include <vcl/splitwin.hxx>
#endif
#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zformat.hxx>
#endif
#ifndef _CTRLTOOL_HXX
#include <svtools/ctrltool.hxx>
#endif

#include <svtools/pver.hxx>

// Ohne Includeschutz
#include <svtools/svtdata.hxx>
#include <svtools/solar.hrc>


#ifndef _BASIC_DISPDEFS_HXX
#include "dispdefs.hxx"
#endif
#include "testtool.hxx"
#include "dialogs.hxx"
#include "resids.hrc"
#include "basic.hrc"

#include "app.hxx"

AboutDialog::AboutDialog( Window* pParent, const ResId& id )
: ModalDialog( pParent, id )
, a1( this, ResId( 1 ) )
, a3( this, ResId( 3 ) )
, a4( this, ResId( 4 ) )
, aVersionString( this, ResId( RID_VERSIONSTRING ) )
//, aIcon( this, ResId( RID_ICON ) )
, aOk  ( this, ResId( RID_OK ) )
{
    aVersionString.SetText( ProductVersion::GetMajorVersion().Append( ProductVersion::GetMinorVersion() )
                            .AppendAscii( " BUILD:" ).Append( ProductVersion::GetBuildNumber() ) );
    FreeResource();
}

////////////////////////////////////////////////////////////////////

FindDialog::FindDialog( Window* pParent, const ResId& id, String& Text )
: ModalDialog( pParent, id )
, aFT1( this, ResId( RID_FIXEDTEXT1 ) )
, aFind( this, ResId( RID_FIND ) )
, aOk( this, ResId( RID_OK ) )
, aCancel( this, ResId( RID_CANCEL ) )
{
    FreeResource();
    pFind = &Text;
    // Button-Handler:
    aOk.SetClickHdl( LINK( this, FindDialog, ButtonClick ) );
    aCancel.SetClickHdl( LINK( this, FindDialog, ButtonClick ) );
    aFind.SetText( Text );
}

IMPL_LINK_INLINE_START( FindDialog, ButtonClick, Button *, pB )
{
    if( pB == &aOk ) {
        *pFind = aFind.GetText();
        EndDialog( TRUE );
    } else EndDialog( FALSE );
    return TRUE;
}
IMPL_LINK_INLINE_END( FindDialog, ButtonClick, Button *, pB )

ReplaceDialog::ReplaceDialog(Window* pParent, const ResId& id, String& Old, String& New )
: ModalDialog( pParent, id )
, aFT1( this, ResId( RID_FIXEDTEXT1 ) )
, aFT2( this, ResId( RID_FIXEDTEXT2 ) )
, aFind( this, ResId( RID_FIND ) )
, aReplace(this, ResId( RID_REPLACE ) )
, aOk( this, ResId( RID_OK ) )
, aCancel( this, ResId( RID_CANCEL ) )
{
    FreeResource();
    pFind = &Old;
    pReplace = &New;
    // Button-Handler:
    aOk.SetClickHdl( LINK( this, ReplaceDialog, ButtonClick ) );
    aCancel.SetClickHdl( LINK( this, ReplaceDialog, ButtonClick ) );
    aFind.SetText( Old );
    aReplace.SetText( New );
}

IMPL_LINK( ReplaceDialog, ButtonClick, Button *, pB )
{
    if( pB == &aOk ) {
        *pFind = aFind.GetText();
        *pReplace = aReplace.GetText();
        EndDialog( TRUE );
    } else EndDialog( FALSE );
    return TRUE;
}

////////////////////////////////////////////////////////////////////


void CheckButtons( ComboBox &aCB, Button &aNewB, Button &aDelB )
{
    aNewB.Enable( aCB.GetEntryPos( aCB.GetText() ) == COMBOBOX_ENTRY_NOTFOUND && aCB.GetText().Len() );
    aDelB.Enable( aCB.GetEntryPos( aCB.GetText() ) != COMBOBOX_ENTRY_NOTFOUND );
}


ConfEdit::ConfEdit( Window* pParent, USHORT nResText, USHORT nResEdit, USHORT nResButton, const ByteString& aKN, Config &aConf )
: PushButton( pParent, ResId(nResButton) )
, aText( pParent, ResId(nResText) )
, aEdit( pParent, ResId(nResEdit) )
, aKeyName(aKN)
{
    aConf.SetGroup("Misc");
    ByteString aCurrentProfile = aConf.ReadKey( "CurrentProfile", "Path" );
    aConf.SetGroup( aCurrentProfile );

    String aTemp = UniString( aConf.ReadKey( aKeyName ), RTL_TEXTENCODING_UTF8 );
    aEdit.SetText( aTemp );
}

void ConfEdit::Save( Config &aConf )
{
    aConf.SetGroup("Misc");
    ByteString aCurrentProfile = aConf.ReadKey( "CurrentProfile", "Path" );
    aConf.SetGroup( aCurrentProfile );
    aConf.WriteKey( aKeyName, ByteString( aEdit.GetText(), RTL_TEXTENCODING_UTF8 ) );
}

void ConfEdit::Reload( Config &aConf )
{
    aConf.SetGroup("Misc");
    ByteString aCurrentProfile = aConf.ReadKey( "CurrentProfile", "Path" );
    aConf.SetGroup( aCurrentProfile );
    String aValue = String( aConf.ReadKey( aKeyName ), RTL_TEXTENCODING_UTF8 );
    aEdit.SetText( aValue );
}

void ConfEdit::Click()
{
    PathDialog aPD( this );
    aPD.SetPath( aEdit.GetText() );
    if ( aPD.Execute() )
        aEdit.SetText( aPD.GetPath() );
}


OptionsDialog::OptionsDialog( Window* pParent, const ResId& aResId )
: ModelessDialog( pParent, aResId )
, aTabCtrl( this, ResId( RES_TC_OPTIONS ) )
, aOK( this, ResId( RID_OK ) )
, aCancel( this, ResId( RID_CANCEL ) )
, aConfig( Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ) )
{
    aConfig.EnablePersistence( FALSE );
    aTabCtrl.FreeResource();
    FreeResource();
    aTabCtrl.SetActivatePageHdl( LINK( this, OptionsDialog, ActivatePageHdl ) );
    aTabCtrl.SetCurPageId( RID_TP_GEN );
    ActivatePageHdl( &aTabCtrl );

    aOK.SetClickHdl( LINK( this, OptionsDialog, OKClick ) );
}

OptionsDialog::~OptionsDialog()
{
    for ( USHORT i = 0; i < aTabCtrl.GetPageCount(); i++ )
        delete aTabCtrl.GetTabPage( aTabCtrl.GetPageId( i ) );
};

BOOL OptionsDialog::Close()
{
    if ( ModelessDialog::Close() )
    {
        delete this;
        return TRUE;
    }
    else
        return FALSE;
}


IMPL_LINK( OptionsDialog, ActivatePageHdl, TabControl *, pTabCtrl )
{
    USHORT nId = pTabCtrl->GetCurPageId();
    // Wenn TabPage noch nicht erzeugt wurde, dann erzeugen
    if ( !pTabCtrl->GetTabPage( nId ) )
    {
        TabPage *pNewTabPage = NULL;
        switch ( nId )
        {
            case RID_TP_GEN:
                pNewTabPage = new GenericOptions( pTabCtrl, aConfig );
                break;
            case RID_TP_PRO:
                pNewTabPage = new ProfileOptions( pTabCtrl, aConfig );
                break;
            case RID_TP_MIS:
                pNewTabPage = new MiscOptions( pTabCtrl, aConfig );
                break;
            case RID_TP_FON:
                pNewTabPage = new FontOptions( pTabCtrl, aConfig );
                break;
            default:    DBG_ERROR( "PageHdl: Unbekannte ID!" );
        }
        DBG_ASSERT( pNewTabPage, "Keine Page!" );
        pTabCtrl->SetTabPage( nId, pNewTabPage );
    }
    return 0;
}




IMPL_LINK( OptionsDialog, OKClick, Button *, pButton )
{
    aConfig.EnablePersistence();
    GenericOptions *pGeneric;
    pGeneric = (GenericOptions*)aTabCtrl.GetTabPage( RID_TP_GEN );
    if ( pGeneric )
        pGeneric->Save( aConfig );

    ProfileOptions *pProfile;
    pProfile = (ProfileOptions*)aTabCtrl.GetTabPage( RID_TP_PRO );
    if ( pProfile )
        pProfile->Save( aConfig );

    MiscOptions *pMisc;
    pMisc = (MiscOptions*)aTabCtrl.GetTabPage( RID_TP_MIS );
    if ( pMisc )
        pMisc->Save( aConfig );

    FontOptions *pFonts;
    pFonts = (FontOptions*)aTabCtrl.GetTabPage( RID_TP_FON );
    if ( pFonts )
        pFonts->Save( aConfig );

    aConfig.Flush();

    ((BasicApp*)GetpApp())->LoadIniFile();
    Close();
    return 0;
}

const ByteString ProfilePrefix("_profile_");
const USHORT ProfilePrefixLen = ProfilePrefix.Len();

ProfileOptions::ProfileOptions( Window* pParent, Config &rConfig )
: TabPage( pParent, ResId( RID_TP_PROFILE ) )
, rConf( rConfig )

, aFlProfile( this, ResId( RID_FL_PROFILE ) )
, aCbProfile( this, ResId( RID_CB_PROFILE ) )
, aPbNewProfile( this, ResId( RID_PB_NEW_PROFILE ) )
, aPbDelProfile( this, ResId( RID_PD_DEL_PROFILE ) )

, aDirs( this, ResId(FL_DIRECTORIES) )
, aLog( this, LOG_TEXT, LOG_NAME, LOG_SET ,"LogBaseDir", rConfig )
, aBasis( this, BASIS_TEXT, BASIS_NAME, BASIS_SET ,"BaseDir", rConfig )
, aHID( this, HID_TEXT, HID_NAME, HID_SET ,"HIDDir", rConfig )

, aAutoReload( this, ResId(CB_AUTORELOAD) )
, aAutoSave( this, ResId(CB_AUTOSAVE) )
, aStopOnSyntaxError( this, ResId(CB_STOPONSYNTAXERRORS) )
{
    FreeResource();

    aCbProfile.EnableAutocomplete( TRUE );

    aCbProfile.SetSelectHdl( LINK( this, ProfileOptions, Select ) );

    aPbNewProfile.SetClickHdl( LINK( this, ProfileOptions, NewProfile ) );
    aPbDelProfile.SetClickHdl( LINK( this, ProfileOptions, DelProfile ) );
    aCbProfile.SetModifyHdl( LINK( this, ProfileOptions, CheckButtonsHdl ) );

    LoadData();
    ReloadProfile();
}

void ProfileOptions::LoadData()
{
    // collect all profiles (all groups starting with the ProfilePrefix)
    for ( USHORT i = 0 ; i < rConf.GetGroupCount() ; i++ )
    {
        ByteString aProfile = rConf.GetGroupName( i );
        if ( aProfile.Match( ProfilePrefix ) )
            aCbProfile.InsertEntry( String( aProfile.Copy( ProfilePrefixLen ), RTL_TEXTENCODING_UTF8 ) );
    }
    // set the current profile
    rConf.SetGroup( "Misc" );
    ByteString aCurrentProfile = rConf.ReadKey( "CurrentProfile", "Path" );
    aCbProfile.SetText( String( aCurrentProfile.Copy( ProfilePrefixLen ), RTL_TEXTENCODING_UTF8 ) );
    CheckButtons( aCbProfile, aPbNewProfile, aPbDelProfile );
}

IMPL_LINK( ProfileOptions, Select, ComboBox*, EMPTYARG )
{
    if ( aCbProfile.GetEntryPos( aCbProfile.GetText() ) == LISTBOX_ENTRY_NOTFOUND )
        return 1;
    Save();
    ByteString aProfileKey( ByteString( ProfilePrefix ).Append( ByteString( aCbProfile.GetText(), RTL_TEXTENCODING_UTF8 ) ) );
    rConf.SetGroup( "Misc" );
    rConf.WriteKey( "CurrentProfile", aProfileKey );
    ReloadProfile();

    return 0;
}

void ProfileOptions::ReloadProfile()
{
    aLog.Reload( rConf );
    aBasis.Reload( rConf );
    aHID.Reload( rConf );

    ByteString aTemp;
    rConf.SetGroup( "Misc" );
    ByteString aCurrentProfile = rConf.ReadKey( "CurrentProfile", "Misc" );
    rConf.SetGroup( aCurrentProfile );
    aTemp = rConf.ReadKey( "AutoReload", "0" );
    aAutoReload.Check( aTemp.Equals("1") );
    aTemp = rConf.ReadKey( "AutoSave", "0" );
    aAutoSave.Check( aTemp.Equals("1") );
    aTemp = rConf.ReadKey( "StopOnSyntaxError", "0" );
    aStopOnSyntaxError.Check( aTemp.Equals("1") );

    CheckButtons( aCbProfile, aPbNewProfile, aPbDelProfile );
}

IMPL_LINK( ProfileOptions, DelProfile, Button*, EMPTYARG )
{
    String aProfile = aCbProfile.GetText();
    ByteString aProfileKey( ByteString( ProfilePrefix ).Append( ByteString( aProfile, RTL_TEXTENCODING_UTF8 ) ) );
    if ( aCbProfile.GetEntryPos( aProfile ) != COMBOBOX_ENTRY_NOTFOUND )
    {
        aCbProfile.RemoveEntry( aProfile );
        rConf.DeleteGroup( aProfileKey );
    }
    // Set first remaining profile as current profile
    aCbProfile.SetText( aCbProfile.GetEntry( 0 ) );
    aProfile = aCbProfile.GetText();
    aProfileKey = ByteString( ProfilePrefix ).Append( ByteString( aProfile, RTL_TEXTENCODING_UTF8 ) );
    rConf.SetGroup( "Misc" );
    rConf.WriteKey( "CurrentProfile", aProfileKey );
    ReloadProfile();
    CheckButtons( aCbProfile, aPbNewProfile, aPbDelProfile );

    return 0;
}

IMPL_LINK( ProfileOptions, NewProfile, Button*, EMPTYARG )
{
    aCbProfile.InsertEntry( aCbProfile.GetText() );
    ByteString aProfileKey( ByteString( ProfilePrefix ).Append( ByteString( aCbProfile.GetText(), RTL_TEXTENCODING_UTF8 ) ) );
    rConf.SetGroup( "Misc" );
    rConf.WriteKey( "CurrentProfile", aProfileKey );
    // save last profile as new data for new profile
    Save();
    CheckButtons( aCbProfile, aPbNewProfile, aPbDelProfile );

    return 0;
}

IMPL_LINK( ProfileOptions, CheckButtonsHdl, ComboBox*, pCB )
{
    CheckButtons( aCbProfile, aPbNewProfile, aPbDelProfile );
    return 0;
}

void ProfileOptions::Save()
{
    Save(rConf);
}

void ProfileOptions::Save( Config &rConfig )
{
    // save data to current profile
    aLog.Save( rConfig );
    aBasis.Save( rConfig );
    aHID.Save( rConfig );

    rConfig.SetGroup( "Misc" );
    ByteString aCurrentProfile = rConfig.ReadKey( "CurrentProfile", "Misc" );
    rConfig.SetGroup( aCurrentProfile );
    rConfig.WriteKey( "AutoReload", aAutoReload.IsChecked()?"1":"0" );
    rConfig.WriteKey( "AutoSave", aAutoSave.IsChecked()?"1":"0" );
    rConfig.WriteKey( "StopOnSyntaxError", aStopOnSyntaxError.IsChecked()?"1":"0" );
}

MiscOptions::MiscOptions( Window* pParent, Config &aConfig )
: TabPage( pParent, ResId( RID_TP_MISC ) )
, aFLCommunication( this, ResId(FL_COMMUNICATION) )
, aFTHost( this, ResId(FT_HOST) )
, aEDHost( this, ResId(ED_HOST) )
, aFTTTPort( this, ResId(FT_TTPORT) )
, aNFTTPort( this, ResId(NF_TTPORT) )
, aFTUNOPort( this, ResId(FT_UNOPORT) )
, aNFUNOPort( this, ResId(NF_UNOPORT) )
, aOther( this, ResId(FL_OTHER) )
, aTimeoutText( this, ResId(TIMEOUT_TEXT) )
, aServerTimeout( this, ResId(SERVER_TIMEOUT) )
, aFTLRU( this, ResId(FT_LRU) )
, aTFMaxLRU( this, ResId(TF_MAX_LRU) )
{
    FreeResource();

    International aInt;
    aNFTTPort.SetUseThousandSep( FALSE );
    aNFUNOPort.SetUseThousandSep( FALSE );
    aTFMaxLRU.SetUseThousandSep( FALSE );

    ByteString aTemp;

    aConfig.SetGroup("Communication");
    aTemp = aConfig.ReadKey( "Host", DEFAULT_HOST );
    aEDHost.SetText( String( aTemp, RTL_TEXTENCODING_UTF8 ) );
    aTemp = aConfig.ReadKey( "TTPort", ByteString::CreateFromInt32( TESTTOOL_DEFAULT_PORT ) );
    aNFTTPort.SetValue( aTemp.ToInt32() );
    aTemp = aConfig.ReadKey( "UnoPort", ByteString::CreateFromInt32( UNO_DEFAULT_PORT ) );
    aNFUNOPort.SetValue( aTemp.ToInt32() );

    aConfig.SetGroup("Misc");
    aTemp = aConfig.ReadKey( "ServerTimeout", "10000" );    // Vorgabe 1 Minute
    aServerTimeout.SetTime( Time(aTemp.ToInt32()) );

    aConfig.SetGroup("LRU");
    aTemp = aConfig.ReadKey( "MaxLRU", "4" );
    aTFMaxLRU.SetValue( aTemp.ToInt32() );
}


void MiscOptions::Save( Config &aConfig )
{
    aConfig.SetGroup("Communication");
    aConfig.WriteKey( "Host", ByteString( aEDHost.GetText(), RTL_TEXTENCODING_UTF8 ) );
    aConfig.WriteKey( "TTPort", ByteString::CreateFromInt32( aNFTTPort.GetValue() ) );
    aConfig.WriteKey( "UnoPort", ByteString::CreateFromInt32( aNFUNOPort.GetValue() ) );

    aConfig.SetGroup("Misc");
    aConfig.WriteKey( "ServerTimeout", ByteString::CreateFromInt32( aServerTimeout.GetTime().GetTime() ) );

    aConfig.SetGroup("LRU");
    ByteString aTemp = aConfig.ReadKey( "MaxLRU", "4" );
    USHORT nOldMaxLRU = (USHORT)aTemp.ToInt32();
    USHORT n;
    for ( n = nOldMaxLRU ; n > aTFMaxLRU.GetValue() ; n-- )
        aConfig.DeleteKey( ByteString("LRU").Append( ByteString::CreateFromInt32( n ) ) );
    aConfig.WriteKey( "MaxLRU", ByteString::CreateFromInt32( aTFMaxLRU.GetValue() ) );
}


FontOptions::FontOptions( Window* pParent, Config &aConfig )
: TabPage( pParent, ResId( RID_TP_FONT ) )
, aFTFontName( this, ResId(FT_FONTNAME) )
, aFontName( this, ResId(CB_FONTNAME) )
, aFTStyle( this, ResId(FT_FONTSTYLE) )
, aFontStyle( this, ResId(CB_FONTSTYLE) )
, aFTSize( this, ResId(FT_FONTSIZE) )
, aFontSize( this, ResId(MB_FONTSIZE) )
, aFTPreview( this, ResId(FT_PREVIEW) )
, aFontList( this )
{
    FreeResource();

    aFontName.Fill( &aFontList );
    aFontName.EnableWYSIWYG();
    aFontName.EnableSymbols();

//    aFontSize.SetUnit( FUNIT_POINT );
//    MapMode aMode( MAP_POINT );
//    aFTPreview.SetMapMode( aMode );

    aFontName.SetModifyHdl( LINK( this, FontOptions, FontNameChanged ) );
    aFontStyle.SetModifyHdl( LINK( this, FontOptions, FontStyleChanged ) );
    aFontSize.SetModifyHdl( LINK( this, FontOptions, FontSizeChanged ) );

    ByteString aTemp;
    aConfig.SetGroup("Misc");
    aTemp = aConfig.ReadKey( "ScriptFontName", "Courier" );
       aFontName.SetText( String( aTemp, RTL_TEXTENCODING_UTF8 ) );
    aFontName.Modify();
    aTemp = aConfig.ReadKey( "ScriptFontStyle", "normal" );
       aFontStyle.SetText( String( aTemp, RTL_TEXTENCODING_UTF8 ) );
    aFontStyle.Modify();
    aTemp = aConfig.ReadKey( "ScriptFontSize", "12" );
       aFontSize.SetText( String( aTemp, RTL_TEXTENCODING_UTF8 ) );
    aFontSize.Modify();
}

IMPL_LINK( FontOptions, FontNameChanged, void*, EMPTYARG )
{
    aFontStyle.Fill( aFontName.GetText(), &aFontList );
    FontStyleChanged( NULL );
    return 0;
}

IMPL_LINK( FontOptions, FontStyleChanged, void*, EMPTYARG )
{
    aFontSize.Fill( aFontList.Get( aFontName.GetText(), aFontStyle.GetText() ), &aFontList );
    FontSizeChanged( NULL );
    return 0;
}

IMPL_LINK( FontOptions, FontSizeChanged, void*, EMPTYARG )
{
    UpdatePreview();
    return 0;
}

void FontOptions::UpdatePreview()
{
    Font aFont = aFontList.Get( aFontName.GetText(), aFontStyle.GetText() );
//    ULONG nFontSize = aFontSize.GetValue( FUNIT_POINT );
    ULONG nFontSize = (aFontSize.GetValue() + 5) / 10;
    aFont.SetHeight( nFontSize );
    aFTPreview.SetFont( aFont );
    aFTPreview.SetText( aFontName.GetText() );
    aFTPreview.Invalidate();
}


void FontOptions::Save( Config &aConfig )
{
    aConfig.SetGroup("Misc");
    aConfig.WriteKey( "ScriptFontName", aFontName.GetText(), RTL_TEXTENCODING_UTF8 );
    aConfig.WriteKey( "ScriptFontStyle", aFontStyle.GetText(), RTL_TEXTENCODING_UTF8 );
    aConfig.WriteKey( "ScriptFontSize", aFontSize.GetText(), RTL_TEXTENCODING_UTF8 );
}


GenericOptions::GenericOptions( Window* pParent, Config &aConfig )
: TabPage( pParent, ResId( RID_TP_GENERIC ) )
, aConf( aConfig )

, aFlArea( this, ResId( RID_FL_AREA ) )
, aCbArea( this, ResId( RID_CB_AREA ) )
, aPbNewArea( this, ResId( RID_PB_NEW_AREA ) )
, aPbDelArea( this, ResId( RID_PD_DEL_AREA ) )

, aFlValue( this, ResId( RID_FL_VALUE ) )
, aCbValue( this, ResId( RID_CB_VALUE ) )
, aPbNewValue( this, ResId( RID_PB_NEW_VALUE ) )
, aPbDelValue( this, ResId( RID_PB_DEL_VALUE ) )
{
    FreeResource();
    LoadData();

    aCbArea.EnableAutocomplete( TRUE );
    aCbValue.EnableAutocomplete( TRUE );

    aCbArea.SetSelectHdl( LINK( this, GenericOptions, LoadGroup ) );

    aPbNewArea.SetClickHdl( LINK( this, GenericOptions, NewGroup ) );
    aPbDelArea.SetClickHdl( LINK( this, GenericOptions, DelGroup ) );

    aPbNewValue.SetClickHdl( LINK( this, GenericOptions, NewValue ) );
    aPbDelValue.SetClickHdl( LINK( this, GenericOptions, DelValue ) );

    aCbArea.SetModifyHdl( LINK( this, GenericOptions, CheckButtonsHdl ) );
    aCbValue.SetModifyHdl( LINK( this, GenericOptions, CheckButtonsHdl ) );
    aCbValue.SetSelectHdl( LINK( this, GenericOptions, CheckButtonsHdl ) );
}

GenericOptions::~GenericOptions()
{
}

StringList* GenericOptions::GetAllGroups()
{
    StringList* pGroups = new StringList();
    for ( USHORT i = 0 ; i < aConf.GetGroupCount() ; i++ )
    {
        String *pGroup = new String( aConf.GetGroupName( i ), RTL_TEXTENCODING_UTF8 );
        pGroups->Insert( pGroup );
    }
    return pGroups;
}

void GenericOptions::LoadData()
{
    StringList* pGroups = GetAllGroups();
    String* pGroup;
    while ( pGroup = pGroups->First() )
    {
        pGroups->Remove( pGroup );
        aConf.SetGroup( ByteString( *pGroup, RTL_TEXTENCODING_UTF8 ) );
        if ( aConf.ReadKey( C_KEY_AKTUELL ).Len() > 0 )
        {
            aCbArea.InsertEntry( *pGroup );
        }
        delete pGroup;
    }
    aCbArea.SetText( aCbArea.GetEntry( 0 ) );
    CheckButtons( aCbArea, aPbNewArea, aPbDelArea );

    // Und auch die Daten laden
    LINK( this, GenericOptions, LoadGroup ).Call( NULL );
}


String GenericOptions::ReadKey( const ByteString &aGroup, const ByteString &aKey )
{
    aConf.SetGroup( aGroup );
    return UniString( aConf.ReadKey( aKey ), RTL_TEXTENCODING_UTF8 );
}

IMPL_LINK( GenericOptions, LoadGroup, ComboBox*, EMPTYARG )
{
    String aCurrentValue;
    String aAllValues;

    if ( aLastGroupName.Len() )
    {   // Werte zwischenspeichern?
        aCurrentValue = aCbValue.GetText();
        if ( aCbValue.GetEntryPos( aCurrentValue ) == COMBOBOX_ENTRY_NOTFOUND )
        {   // Dann legen wir mal einen neuen Wert an
            LINK( this, GenericOptions, NewValue ).Call( NULL );
        }

        aConf.SetGroup( aLastGroupName );
        aConf.WriteKey( C_KEY_AKTUELL, ByteString( aCurrentValue, RTL_TEXTENCODING_UTF8 ) );
        USHORT i;
        for ( i=0 ; i < aCbValue.GetEntryCount() ; i++ )
        {
            if ( i )    // ab Entry 1
                aAllValues += ';';
            aAllValues += aCbValue.GetEntry( i );
        }
        aConf.WriteKey( C_KEY_ALLE, ByteString( aAllValues, RTL_TEXTENCODING_UTF8 ) );
    }

    aCbValue.Clear();

    ByteString aGroupName = ByteString( aCbArea.GetText(), RTL_TEXTENCODING_UTF8 );
    aCurrentValue = ReadKey( aGroupName, C_KEY_AKTUELL );
    aAllValues = ReadKey( aGroupName, C_KEY_ALLE );

    xub_StrLen i;
    for ( i=0 ; i < aAllValues.GetTokenCount() ; i++ )
    {
        aCbValue.InsertEntry( aAllValues.GetToken( i ) );
    }
    aCbValue.SetText( aCurrentValue );

    aLastGroupName = aGroupName;
    CheckButtons( aCbArea, aPbNewArea, aPbDelArea );
    CheckButtons( aCbValue, aPbNewValue, aPbDelValue );
    return 0;
}

IMPL_LINK( GenericOptions, DelGroup, Button*, EMPTYARG )
{
    String aGroup = aCbArea.GetText();
    if ( aCbArea.GetEntryPos( aGroup ) != COMBOBOX_ENTRY_NOTFOUND )
    {
        aCbArea.RemoveEntry( aGroup );
        ByteString aByteGroup( aGroup, RTL_TEXTENCODING_UTF8 );
        aConf.DeleteGroup( aByteGroup );
    }

    aCbArea.SetText( aCbArea.GetEntry( 0 ) );
    LINK( this, GenericOptions, LoadGroup ).Call( NULL );

    return 0;
}

IMPL_LINK( GenericOptions, NewGroup, Button*, EMPTYARG )
{
    aCbArea.InsertEntry( aCbArea.GetText() );

    LINK( this, GenericOptions, LoadGroup ).Call( NULL );

    return 0;
}

IMPL_LINK( GenericOptions, DelValue, Button*, EMPTYARG )
{
    String aValue = aCbValue.GetText();
    if ( aCbValue.GetEntryPos( aValue ) != COMBOBOX_ENTRY_NOTFOUND )
    {
        aCbValue.RemoveEntry( aValue );
    }

    aCbValue.SetText( aCbValue.GetEntry( 0 ) );
    CheckButtons( aCbValue, aPbNewValue, aPbDelValue );
    return 0;
}

IMPL_LINK( GenericOptions, NewValue, Button*, EMPTYARG )
{
    aCbValue.InsertEntry( aCbValue.GetText() );
    CheckButtons( aCbValue, aPbNewValue, aPbDelValue );
    return 0;
}

IMPL_LINK( GenericOptions, CheckButtonsHdl, ComboBox*, pCB )
{
    if ( pCB == &aCbArea )
        CheckButtons( aCbArea, aPbNewArea, aPbDelArea );
    if ( pCB == &aCbValue )
        CheckButtons( aCbValue, aPbNewValue, aPbDelValue );
    return 0;
}

void GenericOptions::Save( Config &aConfig )
{
    DBG_ASSERT( &aConfig == &aConf, "Saving to different Configuration" )

    // eventuelle Änderungen Speichern
    LINK( this, GenericOptions, LoadGroup ).Call( NULL );
}


class TextAndWin : public DockingWindow
{
    FixedText *pFt;
    Window *pWin;
    Window* pFtOriginalParent;
    Window* pWinOriginalParent;
    long nSpace;    // Standardabstand
    BOOL bAlignTop;

public:
    TextAndWin( Window *pParent, FixedText *pFtP, Window *pWinP, long nSpaceP, BOOL bAlignTopP );
    ~TextAndWin();

    virtual void Resize();
};

TextAndWin::TextAndWin( Window *pParent, FixedText *pFtP, Window *pWinP, long nSpaceP, BOOL bAlignTopP )
: DockingWindow( pParent )
, pFt( pFtP )
, pWin( pWinP )
, nSpace( nSpaceP )
, bAlignTop( bAlignTopP )
{
    pFtOriginalParent = pFt->GetParent();
    pWinOriginalParent = pWin->GetParent();
    pFt->SetParent( this );
    pWin->SetParent( this );
}

TextAndWin::~TextAndWin()
{
    pFt->SetParent( pFtOriginalParent );
    pWin->SetParent( pWinOriginalParent );
}

void TextAndWin::Resize()
{
    long nTopSpace = bAlignTop ? 0 : nSpace;
    long nBottomSpace = bAlignTop ? nSpace : 0;

    long nFixedTextOffset;
    if ( GetOutputSizePixel().Height() < 3 * pFt->GetSizePixel().Height() )
    {
        pFt->Hide();
        nFixedTextOffset = 0;
    }
    else
    {
        pFt->Show();
        nFixedTextOffset = pFt->GetSizePixel().Height() + nSpace;

        // FixedText Positionieren
        pFt->SetPosPixel( Point( 0, nTopSpace ) );
    }

    // Window Positionieren
    long nWinPosY = nFixedTextOffset;
    nWinPosY += nTopSpace;
    pWin->SetPosPixel( Point( 0, nWinPosY ) );

    // Größe des Window anpassen
    long nWinHeight = GetOutputSizePixel().Height();
    nWinHeight -= nWinPosY;
    nWinHeight -= nBottomSpace;
    pWin->SetSizePixel( Size( GetOutputSizePixel().Width(), nWinHeight ) );
}

DisplayHidDlg::DisplayHidDlg( Window * pParent )
: FloatingWindow( pParent, ResId( IDD_DISPLAY_HID ) )
, aTbConf( this, ResId( RID_TB_CONF ) )
, aFtControls( this, ResId( RID_FT_CONTROLS ) )
, aMlbControls( this, ResId( RID_MLB_CONTROLS ) )
, aFtSlots( this, ResId( RID_FT_SLOTS ) )
, aMlbSlots( this, ResId( RID_MLB_SLOTS ) )
, aPbKopieren( this, ResId( RID_PB_KOPIEREN ) )
, aPbBenennen( this, ResId( RID_PB_BENENNEN ) )
, aPbSelectAll( this, ResId( RID_PB_SELECTALL ) )
, aOKClose( this, ResId( RID_OK_CLOSE ) )
, nDisplayMode( DH_MODE_KURZNAME | DH_MODE_LANGNAME )   // Falls wir ein altes Office haben diesen Default verwenden
{
    FreeResource();

/*  ResMgr* pRM = CREATERESMGR( svt );
    ToolBox aOrig( this, ResId( 12345, pRM ) );
    delete pRM;

    aTbConf.CopyItem( aOrig, 4 );
    aTbConf.InsertSeparator();
    aTbConf.CopyItem( aOrig, 5 );
    aTbConf.CopyItem( aOrig, 6 );
    aTbConf.CopyItem( aOrig, 7 );             */
    aTbConf.SetOutStyle( TOOLBOX_STYLE_FLAT );

#if OSL_DEBUG_LEVEL < 2
    aTbConf.Hide();
#endif

    pSplit = new SplitWindow( this );
    pControls = new TextAndWin( pSplit, &aFtControls, &aMlbControls, aMlbControls.GetPosPixel().X(), TRUE );
    pSlots = new TextAndWin( pSplit, &aFtSlots, &aMlbSlots, aMlbControls.GetPosPixel().X(), FALSE );

    pSplit->SetPosPixel( aFtControls.GetPosPixel() );
    pSplit->InsertItem( 1, pControls, 70, SPLITWINDOW_APPEND, 0, SWIB_PERCENTSIZE );
    pSplit->InsertItem( 2, pSlots, 30, SPLITWINDOW_APPEND, 0, SWIB_PERCENTSIZE );
    pSplit->Show();

    aTbConf.SetBorderStyle( WINDOW_BORDER_NORMAL );
    aPbKopieren.SetClickHdl( LINK( this, DisplayHidDlg, CopyToClipboard ) );
    aPbSelectAll.SetClickHdl( LINK( this, DisplayHidDlg, SelectAll ) );

    aMlbControls.SetSelectHdl( LINK( this, DisplayHidDlg, Select ) );
    aMlbSlots.SetSelectHdl( LINK( this, DisplayHidDlg, Select ) );
    Select( NULL );
}

DisplayHidDlg::~DisplayHidDlg()
{
    delete pControls;
    delete pSlots;
    delete pSplit;
}

IMPL_LINK( DisplayHidDlg, CopyToClipboard, void*, EMPTYARG )
{
    String aSammel;
    USHORT i;

    for ( i=0 ; i < aMlbControls.GetSelectEntryCount() ; i++ )
    {
        if ( aSammel.Len() )
            aSammel += '\n';
        aSammel += aMlbControls.GetSelectEntry( i );
    }
    for ( i=0 ; i < aMlbSlots.GetSelectEntryCount() ; i++ )
    {
        if ( aSammel.Len() )
            aSammel += '\n';
        aSammel += aMlbSlots.GetSelectEntry( i );
    }
    ::svt::OStringTransfer::CopyString( aSammel, this );
    return 0;
}

IMPL_LINK( DisplayHidDlg, SelectAll, PushButton*, pButton )
{
    if ( pButton->GetState() != STATE_CHECK )
    {
        USHORT i;
        for ( i=0 ; i < aMlbControls.GetEntryCount() ; i++ )
            aMlbControls.SelectEntryPos( i );
        for ( i=0 ; i < aMlbSlots.GetEntryCount() ; i++ )
            aMlbSlots.SelectEntryPos( i );
    }
    else
    {
        aMlbControls.SetNoSelection();
        aMlbControls.Invalidate();
        aMlbSlots.SetNoSelection();
        aMlbSlots.Invalidate();
    }
    Select( NULL );
    return 0;
}

IMPL_LINK( DisplayHidDlg, Select, void*, EMPTYARG )
{
    if ( !aMlbControls.GetSelectEntryCount() && !aMlbSlots.GetSelectEntryCount() )
        aPbSelectAll.SetState( STATE_NOCHECK );
    else if ( aMlbControls.GetSelectEntryCount() == aMlbControls.GetEntryCount()
        && aMlbSlots.GetSelectEntryCount() == aMlbSlots.GetEntryCount() )
        aPbSelectAll.SetState( STATE_CHECK );
    else
        aPbSelectAll.SetState( STATE_DONTKNOW );
    return 0;
}

void DisplayHidDlg::AddData( WinInfoRec* pWinInfo )
{
    if ( pWinInfo->bIsReset )
    {
        aMlbControls.Clear();
        aMlbSlots.Clear();

        if ( pWinInfo->aUId.GetULONG() & DH_MODE_DATA_VALID )   // kein altes Office
            nDisplayMode = pWinInfo->aUId.GetULONG();   // Wird im Reset zur Übermittlung des Modus verwendet

        return;
    }

    String aMsg;
    if ( ( nDisplayMode & DH_MODE_KURZNAME ) )
    {
        if ( pWinInfo->aKurzname.Len() > 0 )
            aMsg += pWinInfo->aKurzname;
        else
        {
            aMsg.AppendAscii( "--" );
            aMsg += pWinInfo->aUId.GetString();
            aMsg.AppendAscii( ": " );
            aMsg += pWinInfo->aRName;
        }
        aMsg.Expand(20);
    }
    else
    {
        aMsg += pWinInfo->aUId.GetString();
        aMsg.Expand(13);
    }
    aMsg.AppendAscii( "   " );          // Mindestens 3 Blanks sollten schon sein.

    if ( nDisplayMode & DH_MODE_LANGNAME )
    {
        if ( pWinInfo->aLangname.Len() > 0 )
            aMsg += pWinInfo->aLangname;
        else
            aMsg += String( ResId( IDS_NO_LONGNAME ) );
    }

    aMlbControls.InsertEntry( aMsg );


    // Haben wir noch einen Slotname?
    if ( ( nDisplayMode & DH_MODE_KURZNAME ) && pWinInfo->aSlotname.Len() > 0 )
    {
        aMsg = pWinInfo->aSlotname;
        aMsg.Expand(20);
        aMsg.AppendAscii( "   " );          // Mindestens 3 Blanks sollten schon sein.

        if ( nDisplayMode & DH_MODE_LANGNAME )
        {
            if ( pWinInfo->aLangname.Len() > 0 )
                aMsg += pWinInfo->aLangname;
            else
                aMsg += String( ResId( IDS_NO_LONGNAME ) );
        }

        aMlbSlots.InsertEntry( aMsg );
    }
}

void DisplayHidDlg::Resize()
{

    if ( IsRollUp() )
    {
        // Wir wollen nur die Toolbox sehen
        SetOutputSizePixel( aTbConf.GetSizePixel() );
    }
    else
    {
//      SetUpdateMode( FALSE );

        // Minimalgröße
        Size aSize( GetOutputSizePixel() );
        aSize.Width() = std::max( aSize.Width(), (long)(aOKClose.GetSizePixel().Width() * 3 ));
        aSize.Height() = std::max( aSize.Height(), (long)(aOKClose.GetSizePixel().Height() * 8 ));
        SetOutputSizePixel( aSize );

        // Standardabstand
        long nSpace = pSplit->GetPosPixel().X();

        // ToolBox Breite anpassen
        aTbConf.SetSizePixel( Size ( GetSizePixel().Width(), aTbConf.CalcWindowSizePixel().Height() ) );
        aTbConf.SetSizePixel( Size() );     // Vorerst verstecken!

        // SplitWindow Positionieren
        pSplit->SetPosPixel( Point( nSpace, nSpace + aTbConf.GetPosPixel().Y() + aTbConf.GetSizePixel().Height() ) );

        // Breite des SplitWindows bestimmen
        long nSplitWidth = GetSizePixel().Width();
        nSplitWidth -= aPbBenennen.GetSizePixel().Width();
        nSplitWidth -= 3 * nSpace;  // Die Zwischenräume
        nSplitWidth -= nSpace / 2;  // Etwas mehr Platz am rechten Rand

        // Höhe des SplitWindows bestimmen
        long nSplitHeight = GetOutputSizePixel().Height();
        nSplitHeight -= pSplit->GetPosPixel().Y();
        nSplitHeight -= nSpace; // der Abstand unten

        // Größe des SplitWindows setzen
        pSplit->SetSizePixel( Size( nSplitWidth, nSplitHeight ) );

        Point aPos;

        // Button "Kopieren" Positionieren
        aPos = pSplit->GetPosPixel();
        aPos.Move( nSplitWidth, 0 );
        aPos.Move( nSpace, 0 );
        aPbKopieren.SetPosPixel( aPos );

        // Button "Alles Wählen" gleich darunter positionieren
        aPos.Move( 0, aPbKopieren.GetSizePixel().Height() );
        aPos.Move( 0, nSpace );
        aPbSelectAll.SetPosPixel( aPos );

        // Button "Benennen" gleich darunter positionieren
        aPos.Move( 0, aPbSelectAll.GetSizePixel().Height() );
        aPos.Move( 0, nSpace );
        aPbBenennen.SetPosPixel( aPos );

        // Und zum Schluß noch den "Close" Button positionieren
        aPos = pSplit->GetPosPixel();
        aPos.Move( nSpace, -aOKClose.GetSizePixel().Height() );
        aPos.Move( pSplit->GetSizePixel().Width(), pSplit->GetSizePixel().Height() );
        aOKClose.SetPosPixel( aPos );

//      SetUpdateMode( TRUE );
//      Invalidate();
    }
    FloatingWindow::Resize();
}


VarEditDialog::VarEditDialog( Window * pParent, SbxVariable *pPVar )
: ModelessDialog( pParent, ResId( IDD_EDIT_VAR ) )
, aFixedTextRID_FT_NAME( this, ResId( RID_FT_NAME ) )
, aFixedTextRID_FT_CONTENT( this, ResId( RID_FT_CONTENT ) )
, aFixedTextRID_FT_NEW_CONTENT( this, ResId( RID_FT_NEW_CONTENT ) )
, aFixedTextRID_FT_NAME_VALUE( this, ResId( RID_FT_NAME_VALUE ) )
, aFixedTextRID_FT_CONTENT_VALUE( this, ResId( RID_FT_CONTENT_VALUE ) )

, aRadioButtonRID_RB_NEW_BOOL_T( this, ResId( RID_RB_NEW_BOOL_T ) )
, aRadioButtonRID_RB_NEW_BOOL_F( this, ResId( RID_RB_NEW_BOOL_F ) )
, aNumericFieldRID_NF_NEW_INTEGER( this, ResId( RID_NF_NEW_INTEGER ) )
, aNumericFieldRID_NF_NEW_LONG( this, ResId( RID_NF_NEW_LONG ) )
, aEditRID_ED_NEW_STRING( this, ResId( RID_ED_NEW_STRING ) )

, aOKButtonRID_OK( this, ResId( RID_OK ) )
, aCancelButtonRID_CANCEL( this, ResId( RID_CANCEL ) )
, pVar( pPVar )
{
    aFixedTextRID_FT_NAME_VALUE.SetText( pVar->GetName() );
    aFixedTextRID_FT_CONTENT_VALUE.SetText( pVar->GetString() );

    SbxDataType eType = pVar->GetType();
    if ( ( eType & ( SbxVECTOR | SbxARRAY | SbxBYREF )) == 0 )
    {
        switch ( eType )
        {
            case SbxBOOL:
                aRadioButtonRID_RB_NEW_BOOL_T.Show();
                aRadioButtonRID_RB_NEW_BOOL_F.Show();
                if ( pVar->GetBool() )
                    aRadioButtonRID_RB_NEW_BOOL_T.Check();
                else
                    aRadioButtonRID_RB_NEW_BOOL_F.Check();
                break;
//              case SbxCURRENCY:
//              case SbxDATE:
                break;
            case SbxINTEGER:
                aNumericFieldRID_NF_NEW_INTEGER.Show();
                aNumericFieldRID_NF_NEW_INTEGER.SetText( pVar->GetString() );
                aNumericFieldRID_NF_NEW_INTEGER.Reformat();
                break;
            case SbxLONG:
                aNumericFieldRID_NF_NEW_LONG.Show();
                aNumericFieldRID_NF_NEW_LONG.SetText( pVar->GetString() );
                aNumericFieldRID_NF_NEW_LONG.Reformat();
                // Müssen hart gesetzt werden, da der Rsc Compiler damit nicht klar kommt.
                aNumericFieldRID_NF_NEW_LONG.SetMin( -aNumericFieldRID_NF_NEW_LONG.GetMax()-1 );
                aNumericFieldRID_NF_NEW_LONG.SetFirst( -aNumericFieldRID_NF_NEW_LONG.GetLast()-1 );
                break;
//              case SbxOBJECT:     // kann nicht editiert werden
                break;
            case SbxSINGLE:
            case SbxDOUBLE:
            case SbxSTRING:
            case SbxVARIANT:    // Taucht wohl auch nicht auf. stattdessen SbxEMPTY
            case SbxEMPTY:
                aEditRID_ED_NEW_STRING.Show();
                aEditRID_ED_NEW_STRING.SetText( pVar->GetString() );
                break;
        }
    }


    aOKButtonRID_OK.SetClickHdl( LINK( this, VarEditDialog, OKClick ) );
}


IMPL_LINK( VarEditDialog, OKClick, Button *, pButton )
{
    BOOL bWasError = SbxBase::IsError();    // Da eventuell ein Fehler geschmissen wird.


    SbxDataType eType = pVar->GetType();
/*
Boolean
Currency
Date
Double
Integer
Long
Object
Single
String
Variant


atof

  ecvt
  f
  gcvt

SvNumberformat::
    static double StringToDouble( const xub_Unicode* pStr,
                                const International& rIntl,
                                int& nErrno,
                                const xub_Unicode** ppEnd = NULL );
        // Konvertiert analog strtod einen dezimalen String in einen double,
        // Dezimalseparator und Tausenderseparator werden aus International
        // genommen, fuehrende Leerzeichen werden weggeparst.
        // Ist ppEnd!=NULL wird *ppEnd hinter das Weggeparste gesetzt.
        // Enthaelt pStr nur den zu parsenden String, ist also bei Erfolg
        // **ppEnd=='\0' und *ppEnd-pStr==strlen(pStr).
        // Bei Ueberlauf wird fVal=+/-HUGE_VAL gesetzt, bei Unterlauf 0,
        // nErrno wird in diesen Faellen auf ERANGE gesetzt, sonst 0.
        // "+/-1.#INF" werden als +/-HUGE_VAL erkannt.

    */



    String aContent( aEditRID_ED_NEW_STRING.GetText() );
    BOOL bError = FALSE;
    switch ( eType )
    {
        case SbxBOOL:
            pVar->PutBool( aRadioButtonRID_RB_NEW_BOOL_T.IsChecked() );
            break;
//      case SbxCURRENCY:
//          pVar->PutCurrency( aContent );
            break;
//      case SbxDATE:
//          pVar->PutDate( aContent );
            break;
        case SbxINTEGER:
            pVar->PutInteger( (INT16)aNumericFieldRID_NF_NEW_INTEGER.GetValue() );
            break;
        case SbxLONG:
            pVar->PutLong( aNumericFieldRID_NF_NEW_LONG.GetValue() );
            break;
        case SbxDOUBLE:
        case SbxSINGLE:
            bError = !pVar->PutStringExt( aContent );
            break;
        case SbxSTRING:
            pVar->PutString( aContent );
            break;
        case SbxVARIANT:
        case SbxEMPTY:
            bError = !pVar->PutStringExt( aContent );
            break;
    }


//  pVar->PutStringExt( aEditRID_ED_NEW_STRING.GetText() );
    if ( !bWasError && SbxBase::IsError() )
    {
        bError = TRUE;
        SbxBase::ResetError();
    }

    if ( bError )
    {
//      ErrorBox( this, WB_OK | WB_DEF_OK, "Der Wert ist ungültig und kann daher nicht gesetzt werden" ).Execute();
        ErrorBox( this, ResId( IDS_INVALID_VALUE ) ).Execute();
        return 1;
    }

//  if ( aEditRID_ED_NEW_STRING.GetText().Compare( pVar->GetString() ) != COMPARE_EQUAL )
//  {
//      aFixedTextRID_FT_CONTENT_VALUE.SetText( pVar->GetString() );
//      aEditRID_ED_NEW_STRING.SetText( pVar->GetString() );
//      return 1;
//  }

    Close();
    return 0;
}




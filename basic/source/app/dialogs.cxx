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
#include "precompiled_basic.hxx"
#include <tools/solar.h>

#include <algorithm>
#include <tools/rc.hxx>
#include <vcl/metric.hxx>
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
#include <tools/config.hxx>
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#include <tools/debug.hxx>
#ifndef _SV_FILEDLG_HXX //autogen
#include <svtools/filedlg.hxx>
#endif
#include <tools/stream.hxx>
#include <tools/fsys.hxx>
#include <svtools/stringtransfer.hxx>
#include <vcl/splitwin.hxx>
#ifndef _ZFORLIST_HXX //autogen
#include <svl/zformat.hxx>
#endif
#include <svtools/ctrltool.hxx>

// Ohne Includeschutz
#include <svtools/svtdata.hxx>
#include <svl/solar.hrc>


#include <basic/dispdefs.hxx>
#include <basic/testtool.hxx>
#include "dialogs.hxx"
#include "resids.hrc"
#include "basic.hrc"

#include "app.hxx"
#include "basrid.hxx"

#include "_version.h"

AboutDialog::AboutDialog( Window* pParent, const ResId& id )
: ModalDialog( pParent, id )
, a1( this, ResId( 1, *id.GetResMgr() ) )
, a4( this, ResId( 4, *id.GetResMgr() ) )
, aVersionString( this, ResId( RID_VERSIONSTRING, *id.GetResMgr() ) )
, aOk  ( this, ResId( RID_OK, *id.GetResMgr() ) )
{
    FreeResource();
}

////////////////////////////////////////////////////////////////////

FindDialog::FindDialog( Window* pParent, const ResId& id, String& Text )
: ModalDialog( pParent, id )
, aFT1( this, ResId( RID_FIXEDTEXT1, *id.GetResMgr() ) )
, aFind( this, ResId( RID_FIND, *id.GetResMgr() ) )
, aOk( this, ResId( RID_OK, *id.GetResMgr() ) )
, aCancel( this, ResId( RID_CANCEL, *id.GetResMgr() ) )
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
        EndDialog( sal_True );
    } else EndDialog( sal_False );
    return sal_True;
}
IMPL_LINK_INLINE_END( FindDialog, ButtonClick, Button *, pB )

ReplaceDialog::ReplaceDialog(Window* pParent, const ResId& id, String& Old, String& New )
: ModalDialog( pParent, id )
, aFT1( this, ResId( RID_FIXEDTEXT1, *id.GetResMgr() ) )
, aFT2( this, ResId( RID_FIXEDTEXT2, *id.GetResMgr() ) )
, aFind( this, ResId( RID_FIND, *id.GetResMgr() ) )
, aReplace(this, ResId( RID_REPLACE, *id.GetResMgr() ) )
, aOk( this, ResId( RID_OK, *id.GetResMgr() ) )
, aCancel( this, ResId( RID_CANCEL, *id.GetResMgr() ) )
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
        EndDialog( sal_True );
    } else EndDialog( sal_False );
    return sal_True;
}

////////////////////////////////////////////////////////////////////


void CheckButtons( ComboBox &aCB, Button &aNewB, Button &aDelB )
{
    aNewB.Enable( aCB.GetEntryPos( aCB.GetText() ) == COMBOBOX_ENTRY_NOTFOUND && aCB.GetText().Len() );
    aDelB.Enable( aCB.GetEntryPos( aCB.GetText() ) != COMBOBOX_ENTRY_NOTFOUND );
}


void ConfEdit::Init( Config &aConf )
{
    aConf.SetGroup("Misc");
    ByteString aCurrentProfile = aConf.ReadKey( "CurrentProfile", "Path" );
    aConf.SetGroup( aCurrentProfile );

    String aTemp = UniString( aConf.ReadKey( aKeyName ), RTL_TEXTENCODING_UTF8 );
    aEdit.SetText( aTemp );
}

ConfEdit::ConfEdit( Window* pParent, sal_uInt16 nResText, sal_uInt16 nResEdit, sal_uInt16 nResButton, const ByteString& aKN, Config &aConf )
: PushButton( pParent, SttResId(nResButton) )
, aText( pParent, SttResId(nResText) )
, aEdit( pParent, SttResId(nResEdit) )
, aKeyName(aKN)
{
    Init( aConf );
}

ConfEdit::ConfEdit( Window* pParent, sal_uInt16 nResEdit, sal_uInt16 nResButton, const ByteString& aKN, Config &aConf )
: PushButton( pParent, SttResId(nResButton) )
, aText( pParent )
, aEdit( pParent, SttResId(nResEdit) )
, aKeyName(aKN)
{
    Init( aConf );
    aText.Hide();
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
    {
        aEdit.SetText( aPD.GetPath() );
        aEdit.Modify();
    }
}

OptConfEdit::OptConfEdit( Window* pParent, sal_uInt16 nResCheck, sal_uInt16 nResEdit, sal_uInt16 nResButton, const ByteString& aKN, ConfEdit& rBaseEdit, Config& aConf )
: ConfEdit( pParent, nResEdit, nResButton, aKN, aConf )
, aCheck( pParent, SttResId( nResCheck ) )
, rBase( rBaseEdit )
{
    aCheck.SetToggleHdl( LINK( this, OptConfEdit, ToggleHdl ) );
    rBase.SetModifyHdl( LINK( this, OptConfEdit, BaseModifyHdl ) );
}

void OptConfEdit::Reload( Config &aConf )
{
    ConfEdit::Reload( aConf );

    DirEntry aCalculatedHIDDir( rBase.GetValue() );
    aCalculatedHIDDir += DirEntry( "global/hid", FSYS_STYLE_FAT );

    DirEntry aCurrentHIDDir( aEdit.GetText() );

    aCheck.Check( aCalculatedHIDDir == aCurrentHIDDir || aEdit.GetText().Len() == 0 );
       aEdit.Enable( !aCheck.IsChecked() );
    Enable( !aCheck.IsChecked() );
}

IMPL_LINK( OptConfEdit, ToggleHdl, CheckBox*, EMPTYARG )
{
    BaseModifyHdl( &aEdit );
       aEdit.Enable( !aCheck.IsChecked() );
    Enable( !aCheck.IsChecked() );
    return 0;
}

IMPL_LINK( OptConfEdit, BaseModifyHdl, Edit*, EMPTYARG )
{
    if ( aCheck.IsChecked() )
    {
        DirEntry aCalculatedHIDDir( rBase.GetValue() );
        aCalculatedHIDDir += DirEntry( "global/hid", FSYS_STYLE_FAT );
        aEdit.SetText( aCalculatedHIDDir.GetFull() );
    }
    return 0;
}


OptionsDialog::OptionsDialog( Window* pParent, const ResId& aResId )
: TabDialog( pParent, aResId )
, aTabCtrl( this, ResId( RES_TC_OPTIONS, *aResId.GetResMgr() ) )
, aOK( this )
, aCancel( this )
, aConfig( Config::GetConfigName( Config::GetDefDirectory(), CUniString("testtool") ) )
{
    aConfig.EnablePersistence( sal_False );
    FreeResource();
    aTabCtrl.SetActivatePageHdl( LINK( this, OptionsDialog, ActivatePageHdl ) );
    aTabCtrl.SetCurPageId( RID_TP_PRO );
    ActivatePageHdl( &aTabCtrl );

    aOK.SetClickHdl( LINK( this, OptionsDialog, OKClick ) );

    aOK.Show();
    aCancel.Show();
}

OptionsDialog::~OptionsDialog()
{
    for ( sal_uInt16 i = 0; i < aTabCtrl.GetPageCount(); i++ )
        delete aTabCtrl.GetTabPage( aTabCtrl.GetPageId( i ) );
};

sal_Bool OptionsDialog::Close()
{
  if ( TabDialog::Close() )
  {
    delete this;
    return sal_True;
  }
  else
    return sal_False;
}


IMPL_LINK( OptionsDialog, ActivatePageHdl, TabControl *, pTabCtrl )
{
    sal_uInt16 nId = pTabCtrl->GetCurPageId();
    // If TabPage was not yet created, do it
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
            case RID_TP_CRA:
                pNewTabPage = new CrashreportOptions( pTabCtrl, aConfig );
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
    (void) pButton; /* avoid warning about unused parameter */
    aConfig.EnablePersistence();
    GenericOptions *pGeneric;
    pGeneric = (GenericOptions*)aTabCtrl.GetTabPage( RID_TP_GEN );
    if ( pGeneric )
        pGeneric->Save( aConfig );

    ProfileOptions *pProfile;
    pProfile = (ProfileOptions*)aTabCtrl.GetTabPage( RID_TP_PRO );
    if ( pProfile )
        pProfile->Save( aConfig );

    CrashreportOptions *pCrash;
    pCrash = (CrashreportOptions*)aTabCtrl.GetTabPage( RID_TP_CRA );
    if ( pCrash )
        pCrash->Save( aConfig );

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
const sal_uInt16 ProfilePrefixLen = ProfilePrefix.Len();

ProfileOptions::ProfileOptions( Window* pParent, Config &rConfig )
: TabPage( pParent, SttResId( RID_TP_PROFILE ) )
, rConf( rConfig )

, aFlProfile( this, SttResId( RID_FL_PROFILE ) )
, aCbProfile( this, SttResId( RID_CB_PROFILE ) )
, aPbNewProfile( this, SttResId( RID_PB_NEW_PROFILE ) )
, aPbDelProfile( this, SttResId( RID_PD_DEL_PROFILE ) )

, aDirs( this, SttResId(FL_DIRECTORIES) )
, aLog( this, LOG_TEXT, LOG_NAME, LOG_SET, "LogBaseDir", rConfig )
, aBasis( this, BASIS_TEXT, BASIS_NAME, BASIS_SET, "BaseDir", rConfig )
, aHID( this, HID_CHECK, HID_NAME, HID_SET, "HIDDir", aBasis, rConfig )

, aAutoReload( this, SttResId(CB_AUTORELOAD) )
, aAutoSave( this, SttResId(CB_AUTOSAVE) )
, aStopOnSyntaxError( this, SttResId(CB_STOPONSYNTAXERRORS) )
{
    FreeResource();

    aCbProfile.EnableAutocomplete( sal_True );

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
    for ( sal_uInt16 i = 0 ; i < rConf.GetGroupCount() ; i++ )
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
    (void) pCB; /* avoid warning about unused parameter */
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

CrashreportOptions::CrashreportOptions( Window* pParent, Config &aConfig )
: TabPage( pParent, SttResId( RID_TP_CRASH ) )
, aFLCrashreport( this, SttResId( FL_CRASHREPORT ) )
, aCBUseProxy( this, SttResId( CB_USEPROXY ) )
, aFTCRHost( this, SttResId( FT_CRHOST ) )
, aEDCRHost( this, SttResId( ED_CRHOST ) )
, aFTCRPort( this, SttResId( FT_CRPORT ) )
, aNFCRPort( this, SttResId( NF_CRPORT ) )
, aCBAllowContact( this, SttResId( CB_ALLOWCONTACT ) )
, aFTEMail( this, SttResId( FT_EMAIL ) )
, aEDEMail( this, SttResId( ED_EMAIL ) )
{
    FreeResource();

    aNFCRPort.SetUseThousandSep( sal_False );

    ByteString aTemp;

    aConfig.SetGroup("Crashreporter");

    aTemp = aConfig.ReadKey( "UseProxy", "false" );
    if ( aTemp.EqualsIgnoreCaseAscii( "true" ) || aTemp.Equals( "1" ) )
        aCBUseProxy.Check();
    else
        aCBUseProxy.Check( sal_False );

    aCBUseProxy.SetToggleHdl( LINK( this, CrashreportOptions, CheckProxy ) );
    LINK( this, CrashreportOptions, CheckProxy ).Call( NULL );  // call once to initialize

    aTemp = aConfig.ReadKey( "ProxyServer" );
    aEDCRHost.SetText( String( aTemp, RTL_TEXTENCODING_UTF8 ) );
    aTemp = aConfig.ReadKey( "ProxyPort", "8080" );
    aNFCRPort.SetValue( aTemp.ToInt32() );


    aTemp = aConfig.ReadKey( "AllowContact", "false" );
    if ( aTemp.EqualsIgnoreCaseAscii( "true" ) || aTemp.Equals( "1" ) )
        aCBAllowContact.Check();
    else
        aCBAllowContact.Check( sal_False );

    aCBAllowContact.SetToggleHdl( LINK( this, CrashreportOptions, CheckResponse ) );
    LINK( this, CrashreportOptions, CheckResponse ).Call( NULL );  // call once to initialize

    aTemp = aConfig.ReadKey( "ReturnAddress" );
    aEDEMail.SetText( String( aTemp, RTL_TEXTENCODING_UTF8 ) );
}


void CrashreportOptions::Save( Config &aConfig )
{
    aConfig.SetGroup("Crashreporter");

    if ( aCBUseProxy.IsChecked() )
        aConfig.WriteKey( "UseProxy", "true" );
    else
        aConfig.WriteKey( "UseProxy", "false" );

    aConfig.WriteKey( "ProxyServer", ByteString( aEDCRHost.GetText(), RTL_TEXTENCODING_UTF8 ) );
    aConfig.WriteKey( "ProxyPort", ByteString::CreateFromInt64( aNFCRPort.GetValue() ) );

    if ( aCBAllowContact.IsChecked() )
        aConfig.WriteKey( "AllowContact", "true" );
    else
        aConfig.WriteKey( "AllowContact", "false" );

    aConfig.WriteKey( "ReturnAddress", ByteString( aEDEMail.GetText(), RTL_TEXTENCODING_UTF8 ) );
}

IMPL_LINK( CrashreportOptions, CheckProxy, void*, EMPTYARG )
{
    aFTCRHost.Enable( aCBUseProxy.IsChecked() );
    aEDCRHost.Enable( aCBUseProxy.IsChecked() );
    aFTCRPort.Enable( aCBUseProxy.IsChecked() );
    aNFCRPort.Enable( aCBUseProxy.IsChecked() );

    return 0;
}
IMPL_LINK( CrashreportOptions, CheckResponse, void*, EMPTYARG )
{
    aFTEMail.Enable( aCBAllowContact.IsChecked() );
    aEDEMail.Enable( aCBAllowContact.IsChecked() );
    return 0;
}

MiscOptions::MiscOptions( Window* pParent, Config &aConfig )
: TabPage( pParent, SttResId( RID_TP_MISC ) )
, aFLCommunication( this, SttResId(FL_COMMUNICATION) )
, aFTHost( this, SttResId(FT_HOST) )
, aEDHost( this, SttResId(ED_HOST) )
, aFTTTPort( this, SttResId(FT_TTPORT) )
, aNFTTPort( this, SttResId(NF_TTPORT) )
, aFTUNOPort( this, SttResId(FT_UNOPORT) )
, aNFUNOPort( this, SttResId(NF_UNOPORT) )
, aOther( this, SttResId(FL_OTHER) )
, aTimeoutText( this, SttResId(TIMEOUT_TEXT) )
, aServerTimeout( this, SttResId(SERVER_TIMEOUT) )
, aFTLRU( this, SttResId(FT_LRU) )
, aTFMaxLRU( this, SttResId(TF_MAX_LRU) )
, aFTProgDir( this, SttResId(FT_PROGDIR) )
, aEDProgDir( this, SttResId(ED_PROGDIR) )
, aPBProgDir( this, SttResId(PB_PROGDIR) )
{
    FreeResource();

    aNFTTPort.SetUseThousandSep( sal_False );
    aNFUNOPort.SetUseThousandSep( sal_False );
    aTFMaxLRU.SetUseThousandSep( sal_False );

    ByteString aTemp;

    aConfig.SetGroup("Communication");
    aTemp = aConfig.ReadKey( "Host", DEFAULT_HOST );
    aEDHost.SetText( String( aTemp, RTL_TEXTENCODING_UTF8 ) );
    aTemp = aConfig.ReadKey( "TTPort", ByteString::CreateFromInt32( TESTTOOL_DEFAULT_PORT ) );
    aNFTTPort.SetValue( aTemp.ToInt32() );
    aTemp = aConfig.ReadKey( "UnoPort", ByteString::CreateFromInt32( UNO_DEFAULT_PORT ) );
    aNFUNOPort.SetValue( aTemp.ToInt32() );

    aConfig.SetGroup("Misc");
    aTemp = aConfig.ReadKey( "ServerTimeout", "10000" );    // Default 1 Minute
    aServerTimeout.SetTime( Time(aTemp.ToInt32()) );

    aConfig.SetGroup("LRU");
    aTemp = aConfig.ReadKey( "MaxLRU", "4" );
    aTFMaxLRU.SetValue( aTemp.ToInt32() );

    aConfig.SetGroup("OOoProgramDir");
    aTemp = aConfig.ReadKey( "Current" );
    aEDProgDir.SetText( String( aTemp, RTL_TEXTENCODING_UTF8 ) );
    aPBProgDir.SetClickHdl( LINK( this, MiscOptions, Click ) );
}

IMPL_LINK( MiscOptions, Click, void*, EMPTYARG )
{
    PathDialog aPD( this );
    aPD.SetPath( aEDProgDir.GetText() );
    if ( aPD.Execute() )
        aEDProgDir.SetText( aPD.GetPath() );
    return 0;
}


void MiscOptions::Save( Config &aConfig )
{
    aConfig.SetGroup("Communication");
    aConfig.WriteKey( "Host", ByteString( aEDHost.GetText(), RTL_TEXTENCODING_UTF8 ) );
    aConfig.WriteKey( "TTPort", ByteString::CreateFromInt64( aNFTTPort.GetValue() ) );
    aConfig.WriteKey( "UnoPort", ByteString::CreateFromInt64( aNFUNOPort.GetValue() ) );

    aConfig.SetGroup("Misc");
    aConfig.WriteKey( "ServerTimeout", ByteString::CreateFromInt32( aServerTimeout.GetTime().GetTime() ) );

    aConfig.SetGroup("LRU");
    ByteString aTemp = aConfig.ReadKey( "MaxLRU", "4" );
    sal_uInt16 nOldMaxLRU = (sal_uInt16)aTemp.ToInt32();
    sal_uInt16 n;
    for ( n = nOldMaxLRU ; n > aTFMaxLRU.GetValue() ; n-- )
        aConfig.DeleteKey( ByteString("LRU").Append( ByteString::CreateFromInt32( n ) ) );
    aConfig.WriteKey( "MaxLRU", ByteString::CreateFromInt64( aTFMaxLRU.GetValue() ) );

    aConfig.SetGroup("OOoProgramDir");
    aConfig.WriteKey( C_KEY_AKTUELL, ByteString( aEDProgDir.GetText(), RTL_TEXTENCODING_UTF8 ) );
    aConfig.WriteKey( C_KEY_TYPE, ByteString( "PATH" ) );
}


FontOptions::FontOptions( Window* pParent, Config &aConfig )
: TabPage( pParent, SttResId( RID_TP_FONT ) )
, aFTFontName( this, SttResId(FT_FONTNAME) )
, aFontName( this, SttResId(CB_FONTNAME) )
, aFTStyle( this, SttResId(FT_FONTSTYLE) )
, aFontStyle( this, SttResId(CB_FONTSTYLE) )
, aFTSize( this, SttResId(FT_FONTSIZE) )
, aFontSize( this, SttResId(MB_FONTSIZE) )
, aFTPreview( this, SttResId(FT_PREVIEW) )
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
    FontInfo aFontInfo( aFontList.Get( aFontName.GetText(), aFontStyle.GetText() ) );
    aFontSize.Fill( &aFontInfo, &aFontList );
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
//    sal_uIntPtr nFontSize = aFontSize.GetValue( FUNIT_POINT );
    sal_uIntPtr nFontSize = static_cast<sal_uIntPtr>((aFontSize.GetValue() + 5) / 10);
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
: TabPage( pParent, SttResId( RID_TP_GENERIC ) )
, aConf( aConfig )

, aFlArea( this, SttResId( RID_FL_AREA ) )
, aCbArea( this, SttResId( RID_CB_AREA ) )
, aPbNewArea( this, SttResId( RID_PB_NEW_AREA ) )
, aPbDelArea( this, SttResId( RID_PD_DEL_AREA ) )

, aFlValue( this, SttResId( RID_FL_VALUE ) )
, aCbValue( this, SttResId( RID_CB_VALUE ) )

, aPbSelectPath( this, SttResId( RID_PB_SELECT_FILE ) )
, aPbNewValue( this, SttResId( RID_PB_NEW_VALUE ) )
, aPbDelValue( this, SttResId( RID_PB_DEL_VALUE ) )

, nMoveButtons( 0 )
, bShowSelectPath( sal_False )
{
    FreeResource();
    LoadData();

    aCbArea.EnableAutocomplete( sal_True );
    aCbValue.EnableAutocomplete( sal_True );

    aCbArea.SetSelectHdl( LINK( this, GenericOptions, LoadGroup ) );

    aPbNewArea.SetClickHdl( LINK( this, GenericOptions, NewGroup ) );
    aPbDelArea.SetClickHdl( LINK( this, GenericOptions, DelGroup ) );

    aPbSelectPath.SetClickHdl( LINK( this, GenericOptions, SelectPath ) );
    aPbNewValue.SetClickHdl( LINK( this, GenericOptions, NewValue ) );
    aPbDelValue.SetClickHdl( LINK( this, GenericOptions, DelValue ) );

    aCbArea.SetModifyHdl( LINK( this, GenericOptions, CheckButtonsHdl ) );
    aCbValue.SetModifyHdl( LINK( this, GenericOptions, CheckButtonsHdl ) );
    aCbValue.SetSelectHdl( LINK( this, GenericOptions, CheckButtonsHdl ) );

    aMoveTimer.SetTimeout( 60 );
    aMoveTimer.SetTimeoutHdl( LINK( this, GenericOptions, MoveButtons ) );
}

GenericOptions::~GenericOptions()
{
}

StringList* GenericOptions::GetAllGroups()
{
    StringList* pGroups = new StringList();
    for ( sal_uInt16 i = 0 ; i < aConf.GetGroupCount() ; i++ )
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
    while ( (pGroup = pGroups->First()) != NULL )
    {
        pGroups->Remove( pGroup );
        aConf.SetGroup( ByteString( *pGroup, RTL_TEXTENCODING_UTF8 ) );
        if ( aConf.ReadKey( C_KEY_AKTUELL ).Len() > 0 )
        {
            aCbArea.InsertEntry( *pGroup );
        }
        delete pGroup;
    }
    delete pGroups;
    aCbArea.SetText( aCbArea.GetEntry( 0 ) );
    CheckButtons( aCbArea, aPbNewArea, aPbDelArea );

    // Add load the data
    LINK( this, GenericOptions, LoadGroup ).Call( NULL );
}

void GenericOptions::ShowSelectPath( const String aType )
{
    Point aNPos = aPbNewValue.GetPosPixel();
    Point aDPos = aPbDelValue.GetPosPixel();
    long nDelta = aDPos.Y() - aNPos.Y();
    if ( aType.EqualsIgnoreCaseAscii( "PATH" ) && !bShowSelectPath )
    {   // Show Path button
        nMoveButtons += nDelta;
        aMoveTimer.Start();
        bShowSelectPath = sal_True;
        aPbSelectPath.Show( sal_True );
        aPbSelectPath.Enable( sal_True );
    }
    else if ( !aType.EqualsIgnoreCaseAscii( "PATH" ) && bShowSelectPath )
    {   // Hide Path button
        nMoveButtons -= nDelta;
        aMoveTimer.Start();
        bShowSelectPath = sal_False;
        aPbSelectPath.Enable( sal_False );
    }
}

IMPL_LINK( GenericOptions, MoveButtons, AutoTimer*, aTimer )
{
    if ( nMoveButtons == 0 )
    {
        aTimer->Stop();
        aPbSelectPath.Show( bShowSelectPath );
        return 0;
    }

    int nStep = (nMoveButtons > 0) ? 2 : -2;
    if ( nMoveButtons <= 1 && nMoveButtons >= -1 )
        nStep = nMoveButtons;

    nMoveButtons -= nStep ;

    Point aPos;

    aPos = aPbNewValue.GetPosPixel();
    aPos.Y() += nStep;
    aPbNewValue.SetPosPixel( aPos );

    aPos = aPbDelValue.GetPosPixel();
    aPos.Y() += nStep;
    aPbDelValue.SetPosPixel( aPos );
    return 0;
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
    String aType;

    if ( aLastGroupName.Len() )
    {   // Cache values?
        aCurrentValue = aCbValue.GetText();
        if ( aCbValue.GetEntryPos( aCurrentValue ) == COMBOBOX_ENTRY_NOTFOUND )
        {   // Create a new value
            LINK( this, GenericOptions, NewValue ).Call( NULL );
        }

        aConf.SetGroup( aLastGroupName );
        aConf.WriteKey( C_KEY_AKTUELL, ByteString( aCurrentValue, RTL_TEXTENCODING_UTF8 ) );
        sal_uInt16 i;
        for ( i=0 ; i < aCbValue.GetEntryCount() ; i++ )
        {
            if ( i > 0 )
                aAllValues += ';';
            aAllValues += aCbValue.GetEntry( i );
        }
        aConf.WriteKey( C_KEY_ALLE, ByteString( aAllValues, RTL_TEXTENCODING_UTF8 ) );
    }

    aCbValue.Clear();

    ByteString aGroupName = ByteString( aCbArea.GetText(), RTL_TEXTENCODING_UTF8 );
    aCurrentValue = ReadKey( aGroupName, C_KEY_AKTUELL );
    aAllValues = ReadKey( aGroupName, C_KEY_ALLE );
    aType = ReadKey( aGroupName, C_KEY_TYPE );

    xub_StrLen i;
    for ( i=0 ; i < aAllValues.GetTokenCount() ; i++ )
    {
        aCbValue.InsertEntry( aAllValues.GetToken( i ) );
    }
    aCbValue.SetText( aCurrentValue );

    aLastGroupName = aGroupName;
    CheckButtons( aCbArea, aPbNewArea, aPbDelArea );
    CheckButtons( aCbValue, aPbNewValue, aPbDelValue );
    ShowSelectPath( aType );
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

IMPL_LINK( GenericOptions, SelectPath, Button*, EMPTYARG )
{
    PathDialog aPD( this );
    aPD.SetPath( aCbValue.GetText() );
    if ( aPD.Execute() )
    {
        aCbValue.SetText( aPD.GetPath() );
        CheckButtons( aCbValue, aPbNewValue, aPbDelValue );
        if ( aPbNewValue.IsEnabled() )
        {
            LINK( this, GenericOptions, NewValue ).Call( NULL );
        }
    }
    return 1;
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
    (void) aConfig; /* avoid warning about unused parameter */
    DBG_ASSERT( &aConfig == &aConf, "Saving to different Configuration" );

    // Save changes
    LINK( this, GenericOptions, LoadGroup ).Call( NULL );
}


class TextAndWin : public DockingWindow
{
    FixedText *pFt;
    Window *pWin;
    Window* pFtOriginalParent;
    Window* pWinOriginalParent;
    long nSpace;    // default space
    sal_Bool bAlignTop;

public:
    TextAndWin( Window *pParent, FixedText *pFtP, Window *pWinP, long nSpaceP, sal_Bool bAlignTopP );
    ~TextAndWin();

    virtual void Resize();
};

TextAndWin::TextAndWin( Window *pParent, FixedText *pFtP, Window *pWinP, long nSpaceP, sal_Bool bAlignTopP )
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

        // FixedText positioning
        pFt->SetPosPixel( Point( 0, nTopSpace ) );
    }

    // Window positioning
    long nWinPosY = nFixedTextOffset;
    nWinPosY += nTopSpace;
    pWin->SetPosPixel( Point( 0, nWinPosY ) );

    // Set size of window
    long nWinHeight = GetOutputSizePixel().Height();
    nWinHeight -= nWinPosY;
    nWinHeight -= nBottomSpace;
    pWin->SetSizePixel( Size( GetOutputSizePixel().Width(), nWinHeight ) );
}

DisplayHidDlg::DisplayHidDlg( Window * pParent )
: FloatingWindow( pParent, SttResId( IDD_DISPLAY_HID ) )
, aTbConf( this, SttResId( RID_TB_CONF ) )
, aFtControls( this, SttResId( RID_FT_CONTROLS ) )
, aMlbControls( this, SttResId( RID_MLB_CONTROLS ) )
, aFtSlots( this, SttResId( RID_FT_SLOTS ) )
, aMlbSlots( this, SttResId( RID_MLB_SLOTS ) )
, aPbKopieren( this, SttResId( RID_PB_KOPIEREN ) )
, aPbBenennen( this, SttResId( RID_PB_BENENNEN ) )
, aPbSelectAll( this, SttResId( RID_PB_SELECTALL ) )
, aOKClose( this, SttResId( RID_OK_CLOSE ) )
, nDisplayMode( DH_MODE_KURZNAME | DH_MODE_LANGNAME ) // If we have an old office use this default
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
    pControls = new TextAndWin( pSplit, &aFtControls, &aMlbControls, aMlbControls.GetPosPixel().X(), sal_True );
    pSlots = new TextAndWin( pSplit, &aFtSlots, &aMlbSlots, aMlbControls.GetPosPixel().X(), sal_False );

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
    sal_uInt16 i;

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
        sal_uInt16 i;
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

        if ( pWinInfo->nRType & DH_MODE_DATA_VALID )    // no old office
            nDisplayMode = pWinInfo->nRType; // Is used for mode transmission while reset
//        if ( pWinInfo->aUId.GetULONG() & DH_MODE_DATA_VALID ) // kein altes Office
//          nDisplayMode = pWinInfo->aUId.GetULONG();   // Wird im Reset zur �bermittlung des Modus verwendet

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
            aMsg += pWinInfo->aUId;
            aMsg.AppendAscii( ": " );
            aMsg += pWinInfo->aRName;
        }
        aMsg.Expand(20);
    }
    else
    {
        aMsg += pWinInfo->aUId;
        aMsg.Expand(13);
    }
    aMsg.AppendAscii( "   " );          // At least three blanks

    if ( nDisplayMode & DH_MODE_LANGNAME )
    {
        if ( pWinInfo->aLangname.Len() > 0 )
            aMsg += pWinInfo->aLangname;
        else
            aMsg += String( SttResId( IDS_NO_LONGNAME ) );
    }

    aMlbControls.InsertEntry( aMsg );


    // Do we have a Slotname?
    if ( ( nDisplayMode & DH_MODE_KURZNAME ) && pWinInfo->aSlotname.Len() > 0 )
    {
        aMsg = pWinInfo->aSlotname;
        aMsg.Expand(20);
        aMsg.AppendAscii( "   " );

        if ( nDisplayMode & DH_MODE_LANGNAME )
        {
            if ( pWinInfo->aLangname.Len() > 0 )
                aMsg += pWinInfo->aLangname;
            else
                aMsg += String( SttResId( IDS_NO_LONGNAME ) );
        }

        aMlbSlots.InsertEntry( aMsg );
    }
}

void DisplayHidDlg::Resize()
{

    if ( IsRollUp() )
    {
        // We want only the toolbox to be seend
        SetOutputSizePixel( aTbConf.GetSizePixel() );
    }
    else
    {
//      SetUpdateMode( sal_False );

        // Minimum size
        Size aSize( GetOutputSizePixel() );
        aSize.Width() = std::max( aSize.Width(), (long)(aOKClose.GetSizePixel().Width() * 3 ));
        aSize.Height() = std::max( aSize.Height(), (long)(aOKClose.GetSizePixel().Height() * 8 ));
        SetOutputSizePixel( aSize );

        // Default space
        long nSpace = pSplit->GetPosPixel().X();

        // Adapt ToolBox width
        aTbConf.SetSizePixel( Size ( GetSizePixel().Width(), aTbConf.CalcWindowSizePixel().Height() ) );
        aTbConf.SetSizePixel( Size() ); // Hide at first

        // SplitWindow positioning
        pSplit->SetPosPixel( Point( nSpace, nSpace + aTbConf.GetPosPixel().Y() + aTbConf.GetSizePixel().Height() ) );

        // Calculate width of SplitWindows
        long nSplitWidth = GetSizePixel().Width();
        nSplitWidth -= aPbBenennen.GetSizePixel().Width();
        nSplitWidth -= 3 * nSpace;  // Spaces
        nSplitWidth -= nSpace / 2;  // Little more space at right margin

        // Calculate hight of SplitWindows
        long nSplitHeight = GetOutputSizePixel().Height();
        nSplitHeight -= pSplit->GetPosPixel().Y();
        nSplitHeight -= nSpace; // bottom margin

        // Set size of SplitWindows
        pSplit->SetSizePixel( Size( nSplitWidth, nSplitHeight ) );

        Point aPos;

        // Button "Copy" positioning
        aPos = pSplit->GetPosPixel();
        aPos.Move( nSplitWidth, 0 );
        aPos.Move( nSpace, 0 );
        aPbKopieren.SetPosPixel( aPos );

        // Button "Get all"
        aPos.Move( 0, aPbKopieren.GetSizePixel().Height() );
        aPos.Move( 0, nSpace );
        aPbSelectAll.SetPosPixel( aPos );

        // Button "Name"
        aPos.Move( 0, aPbSelectAll.GetSizePixel().Height() );
        aPos.Move( 0, nSpace );
        aPbBenennen.SetPosPixel( aPos );

        // "Close" Button
        aPos = pSplit->GetPosPixel();
        aPos.Move( nSpace, -aOKClose.GetSizePixel().Height() );
        aPos.Move( pSplit->GetSizePixel().Width(), pSplit->GetSizePixel().Height() );
        aOKClose.SetPosPixel( aPos );

//      SetUpdateMode( sal_True );
//      Invalidate();
    }
    FloatingWindow::Resize();
}


VarEditDialog::VarEditDialog( Window * pParent, SbxVariable *pPVar )
: ModelessDialog( pParent, SttResId( IDD_EDIT_VAR ) )
, aFixedTextRID_FT_NAME( this, SttResId( RID_FT_NAME ) )
, aFixedTextRID_FT_CONTENT( this, SttResId( RID_FT_CONTENT ) )
, aFixedTextRID_FT_NEW_CONTENT( this, SttResId( RID_FT_NEW_CONTENT ) )
, aFixedTextRID_FT_NAME_VALUE( this, SttResId( RID_FT_NAME_VALUE ) )
, aFixedTextRID_FT_CONTENT_VALUE( this, SttResId( RID_FT_CONTENT_VALUE ) )

, aRadioButtonRID_RB_NEW_BOOL_T( this, SttResId( RID_RB_NEW_BOOL_T ) )
, aRadioButtonRID_RB_NEW_BOOL_F( this, SttResId( RID_RB_NEW_BOOL_F ) )
, aNumericFieldRID_NF_NEW_INTEGER( this, SttResId( RID_NF_NEW_INTEGER ) )
, aNumericFieldRID_NF_NEW_LONG( this, SttResId( RID_NF_NEW_LONG ) )
, aEditRID_ED_NEW_STRING( this, SttResId( RID_ED_NEW_STRING ) )

, aOKButtonRID_OK( this, SttResId( RID_OK ) )
, aCancelButtonRID_CANCEL( this, SttResId( RID_CANCEL ) )
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
//              break;
            case SbxINTEGER:
                aNumericFieldRID_NF_NEW_INTEGER.Show();
                aNumericFieldRID_NF_NEW_INTEGER.SetText( pVar->GetString() );
                aNumericFieldRID_NF_NEW_INTEGER.Reformat();
                break;
            case SbxLONG:
                aNumericFieldRID_NF_NEW_LONG.Show();
                aNumericFieldRID_NF_NEW_LONG.SetText( pVar->GetString() );
                aNumericFieldRID_NF_NEW_LONG.Reformat();
                // Must be hardcoded otherwise the Rsc Compiler will fail
                aNumericFieldRID_NF_NEW_LONG.SetMin( -aNumericFieldRID_NF_NEW_LONG.GetMax()-1 );
                aNumericFieldRID_NF_NEW_LONG.SetFirst( -aNumericFieldRID_NF_NEW_LONG.GetLast()-1 );
                break;
//              case SbxOBJECT:     // cannot be edited
//              break;
            case SbxSINGLE:
            case SbxDOUBLE:
            case SbxSTRING:
            case SbxVARIANT:
            case SbxEMPTY:
                aEditRID_ED_NEW_STRING.Show();
                aEditRID_ED_NEW_STRING.SetText( pVar->GetString() );
                break;
            default:    // don't know how to edit
                ;
        }
    }


    aOKButtonRID_OK.SetClickHdl( LINK( this, VarEditDialog, OKClick ) );
}


IMPL_LINK( VarEditDialog, OKClick, Button *, pButton )
{
    (void) pButton; /* avoid warning about unused parameter */
    sal_Bool bWasError = SbxBase::IsError();    // Probably an error is thrown


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
    // Converts just as strtod a decimal string to a double.
    // Decimal and thousand separators come from International,
    // leading spaces are omitted.
    // If ppEnd!=NULL then *ppEnd is set after the parsed data.
    // If pStr contains only the String to be parsed, then if success:
    // **ppEnd=='\0' and *ppEnd-pStr==strlen(pStr).
    // If overflow fVal=+/-HUGE_VAL, if underflow 0,
    // nErrno is in this cases set to ERANGE otherwise 0.
    // "+/-1.#INF" are recognized as +/-HUGE_VAL.

    */



    String aContent( aEditRID_ED_NEW_STRING.GetText() );
    sal_Bool bError = sal_False;
    switch ( eType )
    {
        case SbxBOOL:
            pVar->PutBool( aRadioButtonRID_RB_NEW_BOOL_T.IsChecked() );
            break;
//      case SbxCURRENCY:
//          pVar->PutCurrency( aContent );
//          break;
//      case SbxDATE:
//          pVar->PutDate( aContent );
//          break;
        case SbxINTEGER:
            pVar->PutInteger( (sal_Int16)aNumericFieldRID_NF_NEW_INTEGER.GetValue() );
            break;
        case SbxLONG:
            pVar->PutLong( static_cast<sal_Int32>(aNumericFieldRID_NF_NEW_LONG.GetValue()) );
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
        default:    // don't know how to edit
            ;
    }


//  pVar->PutStringExt( aEditRID_ED_NEW_STRING.GetText() );
    if ( !bWasError && SbxBase::IsError() )
    {
        bError = sal_True;
        SbxBase::ResetError();
    }

    if ( bError )
    {
        ErrorBox( this, SttResId( IDS_INVALID_VALUE ) ).Execute();
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




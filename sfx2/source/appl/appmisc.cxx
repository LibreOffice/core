/*************************************************************************
 *
 *  $RCSfile: appmisc.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: as $ $Date: 2000-11-08 14:25:41 $
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

#ifndef _SV_STATUS_HXX
#include <vcl/status.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _VOS_PROCESS_HXX //autogen
#include <vos/process.hxx>
#endif
#ifndef _XCEPTION_HXX_
#include <vos/xception.hxx>
#endif
#ifndef _SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _FILTER_HXX //autogen
#include <svtools/filter.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_INTERNALOPTIONS_HXX
#include <svtools/internaloptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif

#ifndef _COM_SUN_STAR_REGISTRY_INVALIDREGISTRYEXCEPTION_HPP_
#include <com/sun/star/registry/InvalidRegistryException.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#include <vcl/config.hxx>
#include <tools/rcid.h>
#include <vos/mutex.hxx>
#include <svtools/pver.hxx>
#pragma hdrstop
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif

#include <osl/file.hxx>

#include "sfxresid.hxx"
#include "app.hxx"
#include "picklist.hxx"
#include "appdata.hxx"
#include "arrdecl.hxx"
#include "cfgmgr.hxx"
#include "cfgitem.hxx"
#include "tbxctrl.hxx"
#include "tbxconf.hxx"
#include "stbitem.hxx"
#include "accitem.hxx"
#include "mnuitem.hxx"
#include "docfac.hxx"
#include "docfile.hxx"
#include "docfilt.hxx"
#include "request.hxx"
#include "bindings.hxx"
#include "dispatch.hxx"
#include "workwin.hxx"
#include "iodlg.hxx"
#include "intro.hxx"
#include "about.hxx"
#include "fcontnr.hxx"
#include "sfxlocal.hrc"
#include "sfx.hrc"
#include "app.hrc"
#include "templdlg.hxx"
#include "module.hxx"
#include "msgpool.hxx"
#include "topfrm.hxx"
#include "openflag.hxx"
#include "viewsh.hxx"
#include "appimp.hxx"
#include "bmkmenu.hxx"
#include "objface.hxx"
#include "helper.hxx"   // SfxContentHelper::Kill()

using namespace vos;

//===================================================================

SV_IMPL_PTRARR( SfxTbxCtrlFactArr_Impl, SfxTbxCtrlFactory* );
SV_IMPL_PTRARR( SfxStbCtrlFactArr_Impl, SfxStbCtrlFactory* );
SV_IMPL_PTRARR( SfxMenuCtrlFactArr_Impl, SfxMenuCtrlFactory* );
SV_IMPL_PTRARR( SfxChildWinFactArr_Impl, SfxChildWinFactory* );
SV_IMPL_PTRARR( SfxModuleArr_Impl, SfxModule* );

//===================================================================

#define SfxApplication
#include "sfxslots.hxx"

class SfxSpecialConfigError_Impl
{
    String aError;

public:

    SfxSpecialConfigError_Impl( const String& rStr );
    DECL_LINK( TimerHdl, Timer*);
};


SfxSpecialConfigError_Impl::SfxSpecialConfigError_Impl( const String& rStr ) :
    aError( rStr )
{
    Timer *pTimer = new Timer;
    pTimer->SetTimeoutHdl( LINK(this, SfxSpecialConfigError_Impl, TimerHdl) );
    pTimer->SetTimeout( 0 );
    pTimer->Start();
}

IMPL_LINK( SfxSpecialConfigError_Impl, TimerHdl, Timer*, pTimer )
{
    delete pTimer;
    ErrorBox( 0, WinBits( WB_OK ) , aError ).Execute();
    delete this;
    SFX_APP()->GetAppDispatcher_Impl()->Execute( SID_QUITAPP );
    return 0L;
}

//====================================================================


SFX_IMPL_INTERFACE(SfxApplication,SfxShell,SfxResId(0))
{
    SFX_CHILDWINDOW_REGISTRATION( SID_CUSTOMIZETOOLBOX );

    // Menu ist auf dem Macintosh sowieso sichtbar
//    SFX_OBJECTBAR_REGISTRATION(
//            SFX_OBJECTBAR_FULLSCREEN | SFX_VISIBILITY_FULLSCREEN,
//            SfxResId(RID_FULLSCREENTOOLBOX) );
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_APPLICATION |
            SFX_VISIBILITY_DESKTOP | SFX_VISIBILITY_STANDARD | SFX_VISIBILITY_CLIENT,
            SfxResId(RID_ENVTOOLBOX) );

    SFX_STATUSBAR_REGISTRATION(SfxResId(SFX_ITEMTYPE_STATBAR));
}

//--------------------------------------------------------------------


void SfxApplicationClass::ActivateExtHelp()
{
    SFX_APP()->Invalidate( SID_EXTENDEDHELP );
    ShowStatusText(String());
}

//-------------------------------------------------------------------------


void SfxApplicationClass::DeactivateExtHelp()
{
    SFX_APP()->Invalidate( SID_EXTENDEDHELP );
    HideStatusText();
}

//-------------------------------------------------------------------------

SfxViewFrame* SfxApplication::CreateViewFrame(
    SfxObjectShell& rDoc, sal_uInt16 nViewId, sal_Bool bHidden )
{
    SfxItemSet *pSet = rDoc.GetMedium()->GetItemSet();
    if ( nViewId )
        pSet->Put( SfxUInt16Item( SID_VIEW_ID, nViewId ) );
    if ( bHidden )
        pSet->Put( SfxBoolItem( SID_HIDDEN, sal_True ) );

    SfxFrame *pFrame = SfxTopFrame::Create( &rDoc, 0, bHidden );
    return pFrame->GetCurrentViewFrame();
}

void SfxApplication::InitializeDisplayName_Impl()
{
    SfxAppData_Impl* pAppData_Impl = Get_Impl();
    if ( !pAppData_Impl->pLabelResMgr )
        return;

    String aTitle = Application::GetDisplayName();
    if ( !aTitle.Len() )
    {
        OClearableGuard aGuard( OMutex::getGlobalMutex() );

        // create version string
/*!!! (pb) don't show a version number at the moment
        USHORT nProductVersion = ProductVersion::GetVersion().ToInt32();
        String aVersion( String::CreateFromInt32( nProductVersion / 10 ) );
        aVersion += 0x002E ; // 2Eh ^= '.'
        aVersion += ( String::CreateFromInt32( nProductVersion % 10 ) );
*/
        // load application title
        aTitle = String( ResId( RID_APPTITLE, pAppData_Impl->pLabelResMgr ) );
        // merge version into title
        aTitle.SearchAndReplaceAscii( "$(VER)", String() /*aVersion*/ );

        aGuard.clear();

#ifndef PRODUCT
        aTitle += DEFINE_CONST_UNICODE(" [");
        ResId aVerId( RID_BUILDVERSION, pAppData_Impl->pLabelResMgr );
        ResMgr* pResMgr = pAppData_Impl->pLabelResMgr->IsAvailable( aVerId.SetRT( RSC_STRING ) )
                        ? pAppData_Impl->pLabelResMgr
                        : NULL;
        aVerId.SetResMgr( pResMgr );
        aTitle += String( aVerId );
        aTitle += 0x005D ; // 5Dh ^= ']'
#endif
        if ( GetDemoKind() == SFX_DEMOKIND_DEMO )
            aTitle += DEFINE_CONST_UNICODE(" (Demo Version)");

        Application::SetDisplayName( aTitle );
    }
}

//--------------------------------------------------------------------

IMPL_LINK( SfxApplication, OpenClients_Impl, void*, pvoid )
{
    OpenClients();
    return 0;
}

void SfxApplication::OpenClients()

/*  [Beschreibung]

    Diese virtuelle Hook-Methode gibt dem Applikationsentwickler
    Gelegenheit, bei bereits sichtbarem Applikationsfenster unmittelbar
    vor Application::Execute() z.B. initiale Fenster (insbesondere
    das initiale Dokumentfenster) zu "offnen.

    Die Basisklasse sollte nicht gerufen werden.
*/
{
    if ( !( pAppData_Impl->nAppEvent & DISPATCH_SERVER ) )
    {
        // Crash-Recovery
#if SUPD<613//MUSTINI
        SfxIniManager *pIni = GetIniManager();
        sal_Bool bSendMail = (sal_uInt16) pIni->ReadKey( DEFINE_CONST_UNICODE("Common"), DEFINE_CONST_UNICODE("SendCrashMail") ).ToInt32();
        String aFileName = pIni->ReadKey( pIni->GetGroupName( SFX_GROUP_WORKINGSET_IMPL ), DEFINE_CONST_UNICODE("Info") );

        if ( bSendMail && aFileName.Len() )
        {
            SvFileStream aStr( aFileName, STREAM_STD_READ );
            if ( !aStr.GetError() )
            {
                aStr.Seek( STREAM_SEEK_TO_END );
                sal_uInt32 nLen = aStr.Tell();
                aStr.Seek( STREAM_SEEK_TO_BEGIN );
                char *pBuffer = new char[nLen+1];
                aStr.Read( pBuffer, nLen );
                pBuffer[nLen] = 0;
                String aBodyText( pBuffer, nLen );
                delete pBuffer;

                SfxStringItem aTarget( SID_TARGETNAME, DEFINE_CONST_UNICODE("_blank") );
                SfxStringItem aBody( SID_POSTSTRING, aBodyText );
                SfxStringItem aMail( SID_FILE_NAME, DEFINE_CONST_UNICODE("mailto:staroffice-bugs@staroffice.de") );
                SfxStringItem aReferer( SID_REFERER, DEFINE_CONST_UNICODE("private:user") );
                SfxBoolItem aSend( SID_MAIL_SEND, sal_False );

                pAppDispat->Execute( SID_OPENDOC, SFX_CALLMODE_SLOT, &aReferer, &aBody, &aMail, &aSend, &aTarget, 0L );
                pIni->DeleteKey( pIni->GetGroupName( SFX_GROUP_WORKINGSET_IMPL ), DEFINE_CONST_UNICODE("Info") );
                aStr.Close();
                SfxContentHelper::Kill( aFileName );
            }
        }
#else
        sal_Bool bSendMail = SvtInternalOptions().CrashMailEnabled();
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
        DBG_ASSERT(!(bSendMail==sal_True),"SfxApplication::OpenClients()\nSendCrashMail not full supported yet!\n");
#endif
#endif


        sal_Bool bCancel = sal_False;
        for ( sal_uInt16 n = 0; sal_True; ++n )
        {
#if SUPD<613//MUSTINI
            String aEntry( pIni->Get( SFX_GROUP_WORKINGSET_IMPL, DEFINE_CONST_UNICODE("Recover"), n ) );
#else
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
            DBG_ASSERT(sal_False, "SfxApplication::OpenClients()\nsoffice.ini key \"WorkingSet\\Recover\" no longer supported ...\n");
#endif
            String aEntry;
#endif
            if ( !aEntry.GetTokenCount() )
                break;

            sal_Bool bIsURL = ( aEntry.GetToken( 2 ).CompareToAscii("url") == COMPARE_EQUAL );
            String aRealFileName( aEntry.GetToken( 3 ) );
            String aTempFileName( aEntry.GetToken( 0 ) );
            sal_uInt16 nRet;
            if ( bCancel )
                nRet = RET_NO;
            else
            {
                String aMsg( SfxResId( STR_RECOVER_QUERY ) );
                aMsg.SearchAndReplaceAscii( "$1", aRealFileName );
                MessBox aBox( NULL, WB_YES_NO_CANCEL | WB_DEF_YES | WB_3DLOOK,
                                String( SfxResId( STR_RECOVER_TITLE ) ), aMsg );
                nRet = aBox.Execute();
            }

            switch ( nRet )
            {
                case RET_YES: // recover a file
                {
                    SfxStringItem aTargetName( SID_TARGETNAME, DEFINE_CONST_UNICODE("_blank") );
                    SfxStringItem aReferer( SID_REFERER, DEFINE_CONST_UNICODE("private:user") );
                    SfxStringItem aTempFileItem( SID_FILE_NAME, aTempFileName );
                    SfxStringItem aFilterItem( SID_FILTER_NAME, aEntry.GetToken( 1 ) );
                    SfxBoolItem aReadOnlyItem( SID_DOC_READONLY, sal_False );
                    if ( !bIsURL )
                        aRealFileName.Erase();
                    SfxStringItem aSalvageItem( SID_DOC_SALVAGE, aRealFileName );
                    if ( bIsURL )
                    {
                        SfxStringItem aRealURLItem( SID_ORIGURL, aRealFileName );
                        pAppDispat->Execute( SID_OPENDOC, SFX_CALLMODE_SYNCHRON,
                                &aTempFileItem, &aFilterItem, &aSalvageItem, &aTargetName,
                                &aRealURLItem, &aReadOnlyItem, &aReferer, 0L );
                    }
                    else
                    {
                        SfxBoolItem aAsTemplateItem( SID_TEMPLATE, !bIsURL );
                        pAppDispat->Execute( SID_OPENDOC, SFX_CALLMODE_SYNCHRON,
                                &aTempFileItem, &aFilterItem, &aSalvageItem, &aTargetName,
                                &aAsTemplateItem, &aReadOnlyItem, &aReferer, 0L );
                    }
                }

                case RET_NO: // skip this file
                    // remove ini-entry
#if SUPD<613//MUSTINI
                    pIni->Delete( SFX_GROUP_WORKINGSET_IMPL, DEFINE_CONST_UNICODE("Recover"), n );
#endif
                    if ( nRet == RET_NO )
                        SfxContentHelper::Kill( aTempFileName );
                    break;

                case RET_CANCEL: // cancel recovering
                    // remove ini-entry
#if SUPD<613//MUSTINI
                    pIni->Delete( SFX_GROUP_WORKINGSET_IMPL, DEFINE_CONST_UNICODE("Recover"), n );
#endif
                    SfxContentHelper::Kill( aTempFileName );
                    bCancel = sal_True; // and all following
                    break;
            }
        }
    }

    String aEmptyStr;
    if ( pAppData_Impl->aOpenList.Len() )
    {
        ApplicationEvent* pAppEvt = new ApplicationEvent( aEmptyStr, aEmptyStr,
                                        APPEVENT_OPEN_STRING,
                                        pAppData_Impl->aOpenList );
        GetpApp()->AppEvent( *pAppEvt );
        delete pAppEvt;
    }

    if ( pAppData_Impl->aPrintList.Len() )
    {
        ApplicationEvent* pAppEvt = new ApplicationEvent( aEmptyStr, aEmptyStr,
                                        APPEVENT_PRINT_STRING,
                                        pAppData_Impl->aPrintList );
        GetpApp()->AppEvent( *pAppEvt );
        delete pAppEvt;
    }

    if ( SfxObjectShell::GetFirst() || ( pAppData_Impl->nAppEvent & DISPATCH_SERVER ) )
        return;

    if( pAppData_Impl->bBean )
        return;

//(mba/task): neu zu implementieren
//  if ( SvtSaveOptions().IsSaveWorkingSet() )
//            SfxTaskManager::RestoreWorkingSet();
//  else
    {
        SfxAllItemSet aSet( GetPool() );

        // Dateiname
#if SUPD<613//MUSTINI
        String aName = SFX_INIMANAGER()->ReadKey( DEFINE_CONST_UNICODE("Common"), DEFINE_CONST_UNICODE("StartDocument") );
#else
#ifdef ENABLE_MISSINGKEYASSERTIONS//MUSTINI
        DBG_ASSERT(sal_False, "SfxApplication::OpenClients()\nsoffice.ini key \"Common\\StartDocument\" no longer supported ...\n");
#endif
        String aName;
#endif
        if ( !aName.Len() )
            aName = String( DEFINE_CONST_UNICODE("private:factory/swriter" ) );
        SfxStringItem aNameItem( SID_FILE_NAME, aName );
        aSet.Put( aNameItem, aNameItem.Which() );
        aSet.Put( SfxStringItem( SID_TARGETNAME, DEFINE_CONST_UNICODE("_blank") ) );

        // Referer
        aSet.Put( SfxStringItem( SID_REFERER, DEFINE_CONST_UNICODE( "private/user" ) ) );
        pAppDispat->Execute( SID_OPENDOC, SFX_CALLMODE_SYNCHRON, aSet );
    }
}

//--------------------------------------------------------------------

void SfxApplication::FillStatusBar( StatusBar& rBar )

/*  [Beschreibung]

    Diese virtuelle Factory-Methode wird vom SFx gerufen, um die
    StatusBar f"ur das Applikationsfenster zu erzeugen. Im Ggs. zu
    den anderen Standard-Controllern Menu, Accelerator und ToolBoxen
    kann die Status-Zeile nur mit dieser Factory erzeugt werden,
    da in der SV-Resource keine StatusBar-Items angegeben werden k"oennen.
*/

{
//  rBar.InsertItem( SID_MAIL_NOTIFY, 18, SIB_IN|SIB_USERDRAW );
    rBar.InsertItem( SID_EXPLORER_SELECTIONSIZE, 200, SIB_IN|SIB_LEFT|SIB_AUTOSIZE );
//    rBar.InsertItem( SID_CURRENTTIME, rBar.GetTextSize(
//        String("::") + GetAppInternational().GetTime( ::com::sun::star::util::Time() ) ).Width() );
//    rBar.InsertItem( SID_CURRENTDATE, rBar.GetTextSize(
//        String("::") + GetAppInternational().GetDate( Date() ) ).Width() );
}

//--------------------------------------------------------------------

void SfxApplication::IntroSlide()

/*  [Beschreibung]

    Diese virtuelle Methode wird vom SFx gerufen, um die bei einem
    Mehrbild-Intro das n"achte Bild anzuzeigen. Sie sollte w"ahrend
    des Startups mehrmals gerufen werden, wenn l"angere Initialisierungen
    ausgef"uhrt werden.
*/

{
    if ( pImp->pIntro )
        pImp->pIntro->Slide();
}

//--------------------------------------------------------------------

SfxFileDialog* SfxApplication::CreateDocFileDialog
(
    sal_uInt32                   nFlags, //  arithmetische Veroderung der u.g. Sfx-Winbits
    const SfxObjectFactory& rFact,  /*  <SfxObjectFactory>, deren Filter angezeigt
                                        werden sollten */
    const SfxItemSet* pSet          // to forward on FileDialog
)

/*  [Beschreibung]

    Diese virtuelle Factory-Methode wird vom SFx gerufen, um den
    f"ur Dokumente den Dialog zum "Offnen und Speichern unter einem
    neuen Namen zu erzeugen.

    Die Default-Implementierung erzeugt abh"angig von nFlags einen Dialog
    mit
    - WB_SAVEAS:    SFXWB_PASSWORD
    - WB_OPEN:      SFXWB_READONLY
*/

{
    SfxItemSet aSet( GetPool(),
            SID_DOC_READONLY, SID_DOC_READONLY,
            SID_HAS_PASSWORD, SID_HAS_PASSWORD,
            SID_PASSWORD, SID_PASSWORD,
            SID_LOAD_LAYOUT, SID_LOAD_LAYOUT,
            SID_FILE_FILTEROPTIONS, SID_FILE_FILTEROPTIONS,
            SID_FILE_NAME, SID_FILE_NAME,
            0 );
    if ( pSet )
        aSet.Put( *pSet );
    if ( WB_SAVEAS == (nFlags & WB_SAVEAS) )
        nFlags |= SFXWB_PASSWORD;
    else
    {
        if ( SFXWB_INSERT != ( nFlags & SFXWB_INSERT ) )
            nFlags |= SFXWB_READONLY;
    }
    return new SfxFileDialog( NULL, nFlags, rFact, aSet );
}

//--------------------------------------------------------------------

PrinterDialog* SfxApplication::CreatePrinterDialog()

/*  [Beschreibung]

    Diese virtuelle Factory-Methode wird vom SFx gerufen, um einen
    Applikations-spezifischen PrinterDialog zu erzeugen.

    Die Default-Implementierung liefert einen 0-Pointer zur"uck.
*/

{
    return 0;
}
//--------------------------------------------------------------------

ModalDialog* SfxApplication::CreateAboutDialog()

/*  [Beschreibung]

    Diese virtuelle Factory-Methode wird vom SFx gerufen, um einen
    Modul-spezifischen AboutDialog zu erzeugen.

    Die Default-Implementierung erzeugt einen AboutDialog aus
    der Resource 'ModalDialog RID_DEFAULTABOUT' und verwendet
    zus"atzlich einen String RID_BUILDVERSION, f"ur die Update-Version
    der Applikation. Letztere wird angezeigt, sobald der Anwender
    eine spezielle, im Dialog definierte, Sequenz von Zeichen
    eingibt (z.B. 'Ctrl' 's' 'f' 'x').
*/

{
    // Buildversion suchen
    ResId aVerId( RID_BUILDVERSION, pAppData_Impl->pLabelResMgr );
    ResMgr *pResMgr = pAppData_Impl->pLabelResMgr->IsAvailable(
                            aVerId.SetRT( RSC_STRING ) )
                    ? pAppData_Impl->pLabelResMgr
                    : 0;
    aVerId.SetResMgr( pResMgr );
    if ( !Resource::GetResManager()->IsAvailable( aVerId ) )
        DBG_ERROR( "No RID_BUILD_VERSION in label-resource-dll" );
    String aVersion( '[' );
    ( aVersion += String( aVerId ) ) += ']';

    // About-Dialog suchen
    ResId aDialogResId( RID_DEFAULTABOUT, pAppData_Impl->pLabelResMgr );
    pResMgr = pAppData_Impl->pLabelResMgr->IsAvailable(
                        aDialogResId.SetRT( RSC_MODALDIALOG ) )
                    ? pAppData_Impl->pLabelResMgr
                    : 0;
    aDialogResId.SetResMgr( pResMgr );
    if ( !Resource::GetResManager()->IsAvailable( aDialogResId ) )
        DBG_ERROR( "No RID_DEFAULTABOUT in label-resource-dll" );

    // About-Dialog anzeigen
    AboutDialog* pDlg = new AboutDialog( 0, aDialogResId, aVersion );
    return pDlg;
}

//--------------------------------------------------------------------

void SfxApplication::HandleConfigError_Impl
(
    sal_uInt16 nErrorCode

)   const

/*  [Beschreibung]

    Hilfsroutine zum Anzeigen einer ErrorBox mit einer von nErrorCode
    abh"angigen Fehlermeldung.
*/

{
    sal_uInt16 nResId = 0;
    switch(nErrorCode)
    {
        case SfxConfigManager::ERR_READ:
            nResId = MSG_ERR_READ_CFG;
            break;
        case SfxConfigManager::ERR_WRITE:
            nResId = MSG_ERR_WRITE_CFG;
            break;
        case SfxConfigManager::ERR_OPEN:
            nResId = MSG_ERR_OPEN_CFG;
            break;
        case SfxConfigManager::ERR_FILETYPE:
            nResId = MSG_ERR_FILETYPE_CFG;
            break;
        case SfxConfigManager::ERR_VERSION:
            nResId = MSG_ERR_VERSION_CFG;
            break;
    }
    DBG_ASSERT(nResId != 0, "unbekannte Fehlerkonstante aus Konfiguration");
    if(nResId)
    {
        ErrorBox aErrorBox(NULL, SfxResId(nResId));
        aErrorBox.Execute();
    }
}

//--------------------------------------------------------------------

void SfxApplication::LoadConfig()

/*  [Beschreibung]

    Interne Routine zum Einlesen der Konfiguration aus SfxIniManager
    und SfxConfigManager.
*/

{
    if ( pImp->bConfigLoaded )
        return;
/*
    sal_Bool bUpdateMode;
    WorkWindow* pAppWin = Application::GetAppWindow();
    if ( pAppWin->IsVisible() )
    {
        bUpdateMode = pAppWin->IsUpdateMode();
        pAppWin->SetUpdateMode(sal_False);
    }
*/
    if (!pCfgMgr->LoadConfig())
        HandleConfigError_Impl((sal_uInt16)pCfgMgr->GetErrorCode());

/*
    if ( pAppWin->IsVisible()  )
    {
        pAppWin->SetUpdateMode( bUpdateMode );
        SfxWorkWindow *pWork = GetWorkWindow_Impl();
        pWork->ArrangeChilds_Impl();
        pWork->ShowChilds_Impl();
        pAppWin->Invalidate();
    }
*/
    pImp->bConfigLoaded = sal_True;
}

//--------------------------------------------------------------------

void SfxApplication::StoreConfig()

/*  [Beschreibung]

    Interne Routine zum Speichern der Konfiguration in SfxIniManager
    und SfxConfigManager.
*/

{
    // Workingset schreiben?
//    if ( SvtOptions().IsSaveWorkingSet() )
//        SfxTaskManager::SaveWorkingSet();
//(mba/task): Implementierung fehlt

    if (!pAppData_Impl->pAppCfg->StoreConfig())
        HandleConfigError_Impl((sal_uInt16)pCfgMgr->GetErrorCode());
    else
        SaveConfiguration();

    utl::ConfigManager::GetConfigManager()->StoreConfigItems();
}

//--------------------------------------------------------------------
#ifdef WNT
extern String GetUserID();
#endif
#ifdef ENABLE_INIMANAGER//MUSTINI
SfxIniManager* SfxApplication::CreateIniManager()
{
    SfxIniManager *pIniMgr = NULL;
    try
    {
        pIniMgr = SfxIniManager::Get();
        if ( pIniMgr )
        {
            pIniMgr->EnterLock();

            // Dialog-Mnemonics/Scaling
            LanguageType eLang = Application::GetAppInternational().GetLanguage();
            Application::EnableAutoMnemonic( pIniMgr->Get( SFX_KEY_INTERNATIONAL_AUTOMNEMONIC,(sal_uInt16) eLang ).CompareToAscii("1") == COMPARE_EQUAL );
            Application::SetDialogScaleX( (short)
                    pIniMgr->Get( SFX_KEY_INTERNATIONAL_DIALOGSCALEX,
                                (sal_uInt16) eLang ).ToInt32() );
            return pIniMgr;
        }
    }
    catch ( ::com::sun::star::registry::InvalidRegistryException& )
    {
        pIniMgr = NULL;
    }

    // If some configurtation files are missing or corrupt
    // try to start setup. If starting failed show a errorbox and exit application with an error code.
    INetURLObject aSetupObj( Application::GetAppFileName(), INET_PROT_FILE );
    #if defined(UNX)
    aSetupObj.setName( DEFINE_CONST_UNICODE("setup") );
    #endif
    #if defined(WIN) || defined(WNT) || defined(OS2)
    aSetupObj.setName( DEFINE_CONST_UNICODE("setup.exe") );
    #endif
    #if defined(MAC)
    aSetupObj.setName( DEFINE_CONST_UNICODE("Setup") );
    #endif

    // We must use different messages for fat office and portal.
    // A fat office can be repaired by user himself ...
    // but portal problems must fixed by an admin!
    String aMsg;
    if( Application::IsRemoteServer())
    {
        aMsg += DEFINE_CONST_UNICODE("Your user account is not configured correctly.\n");
        aMsg += DEFINE_CONST_UNICODE("Please contact your StarPortal administator.\n");
    }
    else
    {
        aMsg += DEFINE_CONST_UNICODE("Configuration files could not be found.\n");
        aMsg += DEFINE_CONST_UNICODE("Can't start neither StarOffice nor Setup.\n");
        aMsg += DEFINE_CONST_UNICODE("Please try to start setup by yourself.");
    }

    String aImageName( aSetupObj.PathToFileName() );
    ::vos::OProcess aProcess( aImageName.GetBuffer() );
    ::rtl::OUString aArg = ::rtl::OUString::createFromAscii( "/officemode" );
    ::vos::OArgumentList aList( 1, &aArg );
    if ( 0 != aProcess.execute( ::vos::OProcess::TOption_Detached, aList ) )
        Application::Abort( aMsg );
    exit(-1);
    return 0;
}

//------------------------------------------------------------------------

SfxIniManager* SfxApplication::GetIniManager() const

/*  [Beschreibung]

    Diese Methode liefert den Ini-Manager der Dokument-Factory
    des aktiven Dokuments, insofern ein Dokument aktiv ist.
    Ansonsten liefert sie den Ini-Manager der Applikation.

    W"ahrend 'Application:Execute()' ist der R"uckgabewert
    immer ein g"ultiger Pointer, ansonsten kann es auch ein
    0-Pointer sein.
*/

{
    return pViewFrame ? pViewFrame->GetIniManager() : pAppIniMgr;
}
#endif//MUSTINI!
//------------------------------------------------------------------------

SfxProgress* SfxApplication::GetProgress() const

/*  [Beschreibung]

    Liefert den f"ur die gesamte Applikation laufenden SfxProgress
    oder 0, falls keiner f"ur die gesamte Applikation l"auft.


    [Querverweise]

    <SfxProgress::GetActiveProgress(SfxViewFrame*)>
    <SfxViewFrame::GetProgress()const>
*/

{
    return pAppData_Impl->pProgress;
}

//------------------------------------------------------------------------

void SfxApplication::ToolboxExec_Impl( SfxRequest &rReq )
{
    // Object-Bar-Id ermitteln
    sal_uInt16 nSID = rReq.GetSlot(), nTbxID;
    switch ( nSID )
    {
        case SID_TOGGLEFUNCTIONBAR:     nTbxID = SFX_OBJECTBAR_APPLICATION; break;
        case SID_TOGGLEOBJECTBAR:       nTbxID = SFX_OBJECTBAR_OBJECT; break;
        case SID_TOGGLETOOLBAR:         nTbxID = SFX_OBJECTBAR_TOOLS; break;
        case SID_TOGGLEMACROBAR:        nTbxID = SFX_OBJECTBAR_MACRO; break;
        case SID_TOGGLEOPTIONBAR:       nTbxID = SFX_OBJECTBAR_OPTIONS; break;
        case SID_TOGGLECOMMONTASKBAR:   nTbxID = SFX_OBJECTBAR_COMMONTASK; break;
        case SID_TOGGLENAVBAR:          nTbxID = SFX_OBJECTBAR_NAVIGATION; break;
        //case SID_TOGGLERECORDINGBAR:  nTbxID = SFX_OBJECTBAR_RECORDING; break;
        //case SID_TOGGLEFULLSCREENBAR: nTbxID = SFX_OBJECTBAR_FULLSCREEN; break;
        default:
            DBG_ERROR( "invalid ObjectBar`s SID" );
    }

    // Parameter auswerten
    SfxToolBoxConfig *pTbxConfig = SfxToolBoxConfig::GetOrCreate();
    SFX_REQUEST_ARG(rReq, pShowItem, SfxBoolItem, nSID, sal_False);
    sal_Bool bShow = pShowItem ? pShowItem->GetValue() : !pTbxConfig->IsToolBoxPositionVisible(nTbxID);

    // ausfuehren
    pTbxConfig->SetToolBoxPositionVisible(nTbxID, bShow);
    Invalidate( nSID );

    SfxViewFrame* pViewFrame = SfxViewFrame::GetFirst();
    while ( pViewFrame )
    {
        // update all "final" dispatchers
        if ( !pViewFrame->GetActiveChildFrame_Impl() )
            pViewFrame->GetDispatcher()->Update_Impl(sal_True);
        pViewFrame = SfxViewFrame::GetNext(*pViewFrame);
    }

    // ggf. recorden
    if ( !rReq.IsAPI() )
        rReq.AppendItem( SfxBoolItem( nSID, bShow ) );
    rReq.Done();
}

//------------------------------------------------------------------------


void SfxApplication::ToolboxState_Impl( SfxItemSet &rSet )
{
    SfxWhichIter aIter(rSet);
    for ( sal_uInt16 nSID = aIter.FirstWhich(); nSID; nSID = aIter.NextWhich() )
    {
        SfxToolBoxConfig *pTbxConfig = SfxToolBoxConfig::GetOrCreate();
        switch ( nSID )
        {
            case SID_TOGGLEFUNCTIONBAR:
                    rSet.Put( SfxBoolItem( nSID, pTbxConfig->
                        IsToolBoxPositionVisible(SFX_OBJECTBAR_APPLICATION)));
                break;

            case SID_TOGGLEOBJECTBAR:
                    rSet.Put( SfxBoolItem( nSID, pTbxConfig->
                        IsToolBoxPositionVisible(SFX_OBJECTBAR_OBJECT)));
                break;

            case SID_TOGGLEOPTIONBAR:
                    rSet.Put( SfxBoolItem( nSID, pTbxConfig->
                        IsToolBoxPositionVisible(SFX_OBJECTBAR_OPTIONS)));
                break;

            case SID_TOGGLETOOLBAR:
                    rSet.Put( SfxBoolItem( nSID, pTbxConfig->
                        IsToolBoxPositionVisible(SFX_OBJECTBAR_TOOLS)));
                break;

            case SID_TOGGLEMACROBAR:
                    rSet.Put( SfxBoolItem( nSID, pTbxConfig->
                        IsToolBoxPositionVisible(SFX_OBJECTBAR_MACRO)));
                break;

            case SID_TOGGLECOMMONTASKBAR:
                    rSet.Put( SfxBoolItem( nSID, pTbxConfig->
                        IsToolBoxPositionVisible(SFX_OBJECTBAR_COMMONTASK)));
                break;

            case SID_TOGGLENAVBAR:
                    rSet.Put( SfxBoolItem( nSID, pTbxConfig->
                        IsToolBoxPositionVisible(SFX_OBJECTBAR_NAVIGATION)));
                break;

            default:
                DBG_ERROR( "invalid ObjectBar`s SID" );
        }
    }
}

//------------------------------------------------------------------------


VclFileDialog* SfxApplicationClass::CreateFileDialog( Window* pParent, sal_uInt32 nWinBits )
{
    return new SfxFileDialog( pParent, nWinBits );
}

SvUShorts* SfxApplication::GetDisabledSlotList_Impl()
{
    sal_Bool bError = sal_False;
    SvUShorts* pList = pAppData_Impl->pDisabledSlotList;
    if ( !pList )
    {
        // Gibt es eine Slotdatei ?
#if SUPD<613//MUSTINI
        INetURLObject aObj( GetIniManager()->Get( SFX_KEY_CONFIG_DIR ), INET_PROT_FILE );
#else
        INetURLObject aObj( SvtPathOptions().GetConfigPath(), INET_PROT_FILE );
#endif
        aObj.insertName( DEFINE_CONST_UNICODE( "slots.cfg" ) );
        SvFileStream aStrm( aObj.GetMainURL(), STREAM_STD_READ );

        // Speziell f"ur AK: wenn in der INI "Slots=AK" steht, slots.cfg
        // ignorieren und weitermachen
#if SUPD<613//MUSTINI
        String aSlotEntry = GetIniManager()->Get( SFX_KEY_SLOTLIST );
        if ( aSlotEntry.CompareIgnoreCaseToAscii( "AK" ) == COMPARE_EQUAL )
            return NULL;
        sal_uInt16 nSlotEntry = (sal_uInt16) aSlotEntry.ToInt32();
        if ( nSlotEntry )
        {
#else
        if( SvtInternalOptions().SlotCFGEnabled() == sal_True )
        {
#endif

            // Gibt es einen "Slotlist"-Eintrag ??
            if ( aStrm.GetError() )
            {
                bError = sal_True;
            }
            else
            {
                // SlotDatei einlesen
                String aTitle;
                aStrm.ReadByteString(aTitle);
                if ( aTitle.CompareToAscii("SfxSlotFile" ) == COMPARE_EQUAL )
                {
                    sal_uInt16 nCount;
                    aStrm >> nCount;
                    pList = pAppData_Impl->pDisabledSlotList =
                            new SvUShorts( nCount < 255 ? (sal_Int8) nCount : 255, 255 );

                    sal_uInt16 nSlot;
                    for ( sal_uInt16 n=0; n<nCount; n++ )
                    {
                        aStrm >> nSlot;
                        pList->Insert( nSlot, n );
                    }

                    aStrm.ReadByteString(aTitle);
                    if ( aTitle.CompareToAscii("END" ) != COMPARE_EQUAL || aStrm.GetError() )
                    {
                        // Lesen schief gegangen
                        DELETEZ( pList );
                        bError = sal_True;
                    }
                }
                else
                {
                    // Streamerkennung  fehlgeschlagen
                    bError = sal_True;
                }
            }
        }
        else
        {
            // Wenn kein Slotlist-Eintrag, dann darf auch keine SlotDatei
            // vorhanden sein
            if ( !aStrm.GetError() )
                bError = sal_True;
        }
    }
    else if ( pList == (SvUShorts*) -1L )
    {
        return NULL;
    }

    if ( !pList )
        pAppData_Impl->pDisabledSlotList = (SvUShorts*) -1L;

    if ( bError )
    {
        // Wenn ein Sloteintrag vorhanden ist, aber keine oder eine fehlerhafte
        // SlotDatei, oder aber eine Slotdatei, aber kein Sloteintrag, dann
        // gilt dies als fehlerhafte Konfiguration
        new SfxSpecialConfigError_Impl(
            String( SfxResId( RID_SPECIALCONFIG_ERROR ) ) );
    }

       return pList;
}


Config* SfxApplication::GetFilterIni()
{
#if SUPD<613//MUSTINI
    if( !pAppData_Impl->pFilterIni )
    {
        String aIniFile( DEFINE_CONST_UNICODE( FILTER_INI ) );
        if ( GetIniManager()->SearchFile( aIniFile, SFX_KEY_MODULES_PATH ) )
            pAppData_Impl->pFilterIni = new Config( aIniFile );
        else
            // da fr"uher immer ein pointer != NULL returniert wurde, m"ussen
            // wir das jetzt wohl auch tun
            pAppData_Impl->pFilterIni = new Config();
    }
    return pAppData_Impl->pFilterIni;
#else
    if( !pAppData_Impl->pFilterIni )
    {
        String aIniFile( SvtPathOptions().GetModulePath() + DEFINE_CONST_UNICODE("/") + DEFINE_CONST_UNICODE( FILTER_INI ) );
        pAppData_Impl->pFilterIni = new Config( aIniFile );
    }
    return pAppData_Impl->pFilterIni;
#endif
}


SfxModule* SfxApplication::GetActiveModule( SfxViewFrame *pFrame ) const
{
    if ( !pFrame )
        pFrame = SfxViewFrame::Current();
    SfxObjectShell* pSh = 0;
    if( pFrame ) pSh = pFrame->GetObjectShell();
    return pSh ? pSh->GetModule() : 0;
}

SfxModule* SfxApplication::GetModule_Impl()
{
    SfxModule* pModule = GetActiveModule();
    if ( !pModule )
        pModule = GetActiveModule( SfxViewFrame::GetFirst( FALSE ) );
    if( pModule )
        return pModule;
    else
    {
        DBG_ERROR( "No module!" );
        return NULL;
    }
}


SfxSlotPool& SfxApplication::GetSlotPool( SfxViewFrame *pFrame ) const
{
    SfxModule *pMod = GetActiveModule( pFrame );
    if ( pMod && pMod->GetSlotPool() )
        return *pMod->GetSlotPool();
    else
        return *pSlotPool;
}


SfxAcceleratorManager* SfxApplication::GetAcceleratorManager() const
{
    // Accelerator immer mit ContainerBindings
    SfxViewFrame *pFrame = pViewFrame;
    if ( !pFrame )
        return pAcceleratorMgr;

    while ( pFrame->GetParentViewFrame_Impl() )
        pFrame = pFrame->GetParentViewFrame_Impl();

    SfxViewShell* pSh = 0;
    if( pFrame )
        pSh = pFrame->GetViewShell();
    if ( pSh )
    {
        SfxAcceleratorManager *pMgr = pSh->GetAccMgr_Impl();
        if ( pMgr )
            return pMgr;
    }

    return pAcceleratorMgr;
}

ISfxTemplateCommon* SfxApplication::GetCurrentTemplateCommon()
{
    if( pAppData_Impl->pTemplateCommon )
        return pAppData_Impl->pTemplateCommon;
    return NULL;
}


ISfxTemplateCommon* SfxApplication::GetCurrentTemplateCommon( SfxBindings& rBindings )
{
    if( pAppData_Impl->pTemplateCommon )
        return pAppData_Impl->pTemplateCommon;
    SfxChildWindow *pChild = rBindings.GetWorkWindow_Impl()->GetChildWindow_Impl(
        SfxTemplateDialogWrapper::GetChildWindowId() );
    if ( pChild )
        return ((SfxTemplateDialog*) pChild->GetWindow())->GetISfxTemplateCommon();
    return 0;
}

PopupMenu* SfxAppData_Impl::GetPopupMenu( sal_uInt16 nSID, sal_Bool bBig, sal_Bool bNew )
{
#if SUPD<613//MUSTINI
    String aPath;
    SfxBmkMenu** ppMenu;
    sal_uInt16 nKey;
    switch( nSID )
    {
        case SID_NEWDOCDIRECT:
            ppMenu = &pNewMenu;
            nKey = SFX_KEY_NEW_DIR;
            break;
        case SID_AUTOPILOTMENU:
            ppMenu = &pAutoPilotMenu;
            nKey = SFX_KEY_AUTOPILOT_DIR;
            break;
        default:
            ppMenu = 0;
            DBG_ERROR( "Menu ID unknown!" );
            break;
    }

    if( ppMenu && ( !*ppMenu || bNew ) )
    {
        INetURLObject aObj( SFX_INIMANAGER()->Get( nKey ), INET_PROT_FILE );
        String aURL = aObj.GetMainURL();
        if ( *ppMenu )
            delete *ppMenu;
        *ppMenu = new SfxBmkMenu( aURL, aURL );
        (*ppMenu)->Initialize();
    }
    return ppMenu ? *ppMenu : NULL;
#else
    String aPath;
    SfxBmkMenu** ppMenu;
    String sKey;
    switch( nSID )
    {
        case SID_NEWDOCDIRECT:
            ppMenu = &pNewMenu;
            sKey = SvtPathOptions().GetNewMenuPath();
            break;
        case SID_AUTOPILOTMENU:
            ppMenu = &pAutoPilotMenu;
            sKey = SvtPathOptions().GetAutoPilotPath();
            break;
        default:
            ppMenu = 0;
            DBG_ERROR( "Menu ID unknown!" );
            break;
    }

    if( ppMenu && ( !*ppMenu || bNew ) )
    {
        INetURLObject aObj( sKey, INET_PROT_FILE );
        String aURL = aObj.GetMainURL();
        if ( *ppMenu )
            delete *ppMenu;
        *ppMenu = new SfxBmkMenu( aURL, aURL );
        (*ppMenu)->Initialize();
    }
    return ppMenu ? *ppMenu : NULL;
#endif
}

SfxMenuBarManager* SfxApplication::GetMenuBarManager() const
{
    SfxViewFrame *pFrame = SfxViewFrame::Current();
    if ( pFrame )
        return pFrame->GetViewShell()->GetMenuBar_Impl();
    else
        return 0;
}



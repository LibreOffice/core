/*************************************************************************
 *
 *  $RCSfile: appinit.cxx,v $
 *
 *  $Revision: 1.34 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 07:56:15 $
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

#include "app.hxx"

#ifndef _COM_SUN_STAR_FRAME_XTERMINATELISTENER_HPP_
#include <com/sun/star/frame/XTerminateListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif

#include <svtools/svtools.hrc>
#include <svtools/saveopt.hxx>
#include <svtools/localisationoptions.hxx>

#ifndef _SV_CONFIG_HXX
#include <vcl/config.hxx>
#endif
#ifndef _SV_RESARY_HXX
#include <vcl/resary.hxx>
#endif

#ifndef _SOERR_HXX //autogen
#include <so3/soerr.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _INETBND_HXX //autogen
#include <so3/inetbnd.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif
#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif
#include <svtools/inethist.hxx>
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _VOS_SECURITY_HXX_
#include <vos/security.hxx>
#endif
#ifndef _UCBHELPER_CONFIGURATIONKEYS_HXX_
#include <ucbhelper/configurationkeys.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_HISTORYOPTIONS_HXX
#include <svtools/historyoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#include <rtl/logfile.hxx>

#pragma hdrstop

#include "unoctitm.hxx"
#include "appimp.hxx"
#include "accmgr.hxx"
#include "app.hrc"
#include "sfxlocal.hrc"
#include "appdata.hxx"
#include "arrdecl.hxx"
#include "cfgmgr.hxx"
#include "dispatch.hxx"
#include "docfac.hxx"
#include "evntconf.hxx"
#include "frameobj.hxx"
#include "imgmgr.hxx"
#include "interno.hxx"
#include "intro.hxx"
#include "macrconf.hxx"
#include "mnumgr.hxx"
#include "msgpool.hxx"
#include "newhdl.hxx"
#include "plugobj.hxx"
#include "progress.hxx"
#include "sfxhelp.hxx"
#include "sfxresid.hxx"
#include "sfxtypes.hxx"
#include "stbmgr.hxx"
#include "tbxconf.hxx"
#include "viewsh.hxx"
#include "eacopier.hxx"
#include "nochaos.hxx"
#include "fcontnr.hxx"
#include "helper.hxx"   // SfxContentHelper::Kill()
#include "sfxpicklist.hxx"

#ifdef UNX
#define stricmp(a,b) strcmp(a,b)
#endif

#ifdef MAC
int svstricmp( const char* pStr1, const char* pStr2);
#define stricmp svstricmp
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star;

void doFirstTimeInit();

class SfxTerminateListener_Impl : public ::cppu::WeakImplHelper1< XTerminateListener  >
{
public:
    virtual void SAL_CALL queryTermination( const EventObject& aEvent ) throw( TerminationVetoException, RuntimeException );
    virtual void SAL_CALL notifyTermination( const EventObject& aEvent ) throw( RuntimeException );
    virtual void SAL_CALL disposing( const EventObject& Source ) throw( RuntimeException );
};

void SAL_CALL SfxTerminateListener_Impl::disposing( const EventObject& Source ) throw( RuntimeException )
{
}

void SAL_CALL SfxTerminateListener_Impl::queryTermination( const EventObject& aEvent ) throw(TerminationVetoException, RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( !SFX_APP()->QueryExit_Impl() )
        throw TerminationVetoException();
}

void SAL_CALL SfxTerminateListener_Impl::notifyTermination( const EventObject& aEvent ) throw(RuntimeException )
{
    Reference< XDesktop > xDesktop( aEvent.Source, UNO_QUERY );
    if( xDesktop.is() == sal_True )
        xDesktop->removeTerminateListener( this );

    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    SfxApplication* pApp = SFX_APP();
    pApp->Get_Impl()->aLateInitTimer.Stop();
    pApp->Broadcast( SfxSimpleHint( SFX_HINT_DEINITIALIZING ) );
    pApp->Get_Impl()->pAppDispatch->ReleaseAll();
    pApp->Get_Impl()->pAppDispatch->release();
    pApp->NotifyEvent(SfxEventHint( SFX_EVENT_CLOSEAPP) );
    pApp->Deinitialize();
    Application::Quit();
}

//====================================================================

//====================================================================

FASTBOOL SfxApplication::Initialize_Impl()
{
#if SUPD>637
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mb93783) ::SfxApplication::Initialize_Impl" );
#endif

#ifdef TLX_VALIDATE
    StgIo::SetErrorLink( LINK( this, SfxStorageErrHdl, Error ) );
#endif

//! FSysEnableSysErrorBox( FALSE ); (pb) replaceable?

    Reference < XDesktop > xDesktop ( ::comphelper::getProcessServiceFactory()->createInstance( DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );
    xDesktop->addTerminateListener( new SfxTerminateListener_Impl() );

    if( !CheckTryBuy_Impl() )
    {
        exit(-1);
        return FALSE;;
    }

    Application::EnableAutoHelpId();

    pAppData_Impl->pAppDispatch = new SfxStatusDispatcher;
    pAppData_Impl->pAppDispatch->acquire();

    // SV-Look
    Help::EnableContextHelp();
    Help::EnableExtHelp();

    SvtLocalisationOptions aLocalisation;
    Application::EnableAutoMnemonic ( aLocalisation.IsAutoMnemonic() );
    Application::SetDialogScaleX    ( (short)(aLocalisation.GetDialogScale()) );

    // StarObjects initialisieren
    if ( !SvFactory::Init() )
        ErrorBox( 0, SfxResId(MSG_ERR_SOINIT) ).Execute();

    // Factory f"ur das SfxFrameObject anlegen; da der Pointer in den AppDaten
    // liegt, dieser aber nicht exportierbar ist, mu\s ein exportierbarer
    // Wrapper angelegt werden
    pAppData_Impl->pSfxFrameObjectFactoryPtr = new SfxFrameObjectFactoryPtr;
    pAppData_Impl->pSfxFrameObjectFactoryPtr->pSfxFrameObjectFactory = SfxFrameObject::ClassFactory();
    SvBindStatusCallback::SetProgressCallback( STATIC_LINK( 0, SfxProgress, DefaultBindingProgress ) );
    INetURLHistory::GetOrCreate()->SetLocation( SvtPathOptions().GetUserConfigPath() );

    pImp->pEventHdl = new UniqueIndex( 1, 4, 4 );
    //InitializeDisplayName_Impl();

#ifdef DBG_UTIL
    // Der SimplerErrorHandler dient Debugzwecken. In der Product werden
    // nichtgehandelte Fehler durch Errorcode 1 an SFX gegeben.
    new SimpleErrorHandler;
#endif
    new SfxErrorHandler(RID_ERRHDL, ERRCODE_AREA_TOOLS, ERRCODE_AREA_LIB1);
    new SfxErrorHandler(
        RID_SO_ERROR_HANDLER, ERRCODE_AREA_SO, ERRCODE_AREA_SO_END);
    new SfxErrorHandler(
        (RID_SJ_START +1), ERRCODE_AREA_JAVA, ERRCODE_AREA_JAVA_END);
    new SfxErrorHandler(
        RID_BASIC_START, ERRCODE_AREA_SBX, ERRCODE_AREA_SBX_END );

    // diverse Pointer
    pImp->pAutoSaveTimer = new Timer;
    SfxPickList::GetOrCreate( SvtHistoryOptions().GetSize( ePICKLIST ) );

    /////////////////////////////////////////////////////////////////

    DBG_ASSERT( !pAppDispat, "AppDispatcher already exists" );
    pAppDispat = new SfxDispatcher((SfxDispatcher*)0);
    pSlotPool = new SfxSlotPool;
    pImp->pTbxCtrlFac = new SfxTbxCtrlFactArr_Impl;
    pImp->pStbCtrlFac = new SfxStbCtrlFactArr_Impl;
    pImp->pMenuCtrlFac = new SfxMenuCtrlFactArr_Impl;
    pImp->pViewFrames = new SfxViewFrameArr_Impl;
    pImp->pViewShells = new SfxViewShellArr_Impl;
    pImp->pObjShells = new SfxObjectShellArr_Impl;
    nInterfaces = SFX_INTERFACE_APP+8;
    pInterfaces = new SfxInterface*[nInterfaces];
    memset( pInterfaces, 0, sizeof(SfxInterface*) * nInterfaces );

    pAcceleratorMgr = new SfxAcceleratorManager( pCfgMgr );

    pImageMgr = new SfxImageManager( NULL );
    SfxNewHdl* pNewHdl = SfxNewHdl::GetOrCreate();

    // Die Strings muessen leider zur Laufzeit gehalten werden, da wir bei
    // einer ::com::sun::star::uno::Exception keine Resourcen mehr laden duerfen.
    pImp->aMemExceptionString = pNewHdl->GetMemExceptionString();
    pImp->aResWarningString       = String( SfxResId( STR_RESWARNING ) );
    pImp->aResExceptionString     = String( SfxResId( STR_RESEXCEPTION ) );
    pImp->aSysResExceptionString  = String( SfxResId( STR_SYSRESEXCEPTION ) );

    Registrations_Impl();

//    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    ResStringArray aEventNames( SfxResId( EVENT_NAMES_ARY ) );

    SfxEventConfiguration::RegisterEvent(SFX_EVENT_STARTAPP,        String(SfxResId(STR_EVENT_STARTAPP)),   aEventNames.GetString( 0 ) );
    SfxEventConfiguration::RegisterEvent(SFX_EVENT_CLOSEAPP,        String(SfxResId(STR_EVENT_CLOSEAPP)),   aEventNames.GetString( 1 ) );
    SfxEventConfiguration::RegisterEvent(SFX_EVENT_CREATEDOC,       String(SfxResId(STR_EVENT_CREATEDOC)),  aEventNames.GetString( 2 ) );
    SfxEventConfiguration::RegisterEvent(SFX_EVENT_OPENDOC,         String(SfxResId(STR_EVENT_OPENDOC)),    aEventNames.GetString( 3 ) );
    SfxEventConfiguration::RegisterEvent(SFX_EVENT_SAVEASDOC,       String(SfxResId(STR_EVENT_SAVEASDOC)),  aEventNames.GetString( 4 ) );
    SfxEventConfiguration::RegisterEvent(SFX_EVENT_SAVEASDOCDONE,   String(SfxResId(STR_EVENT_SAVEASDOCDONE)),  aEventNames.GetString( 5 ) );
    SfxEventConfiguration::RegisterEvent(SFX_EVENT_SAVEDOC,         String(SfxResId(STR_EVENT_SAVEDOC)),        aEventNames.GetString( 6 ) );
    SfxEventConfiguration::RegisterEvent(SFX_EVENT_SAVEDOCDONE,     String(SfxResId(STR_EVENT_SAVEDOCDONE)),    aEventNames.GetString( 7 ) );
    SfxEventConfiguration::RegisterEvent(SFX_EVENT_PREPARECLOSEDOC, String(SfxResId(STR_EVENT_PREPARECLOSEDOC)),aEventNames.GetString( 8 ) );
    SfxEventConfiguration::RegisterEvent(SFX_EVENT_CLOSEDOC,        String(SfxResId(STR_EVENT_CLOSEDOC)),       aEventNames.GetString( 9 ) );
    SfxEventConfiguration::RegisterEvent(SFX_EVENT_ACTIVATEDOC,     String(SfxResId(STR_EVENT_ACTIVATEDOC)),    aEventNames.GetString( 10 ) );
    SfxEventConfiguration::RegisterEvent(SFX_EVENT_DEACTIVATEDOC,   String(SfxResId(STR_EVENT_DEACTIVATEDOC)),  aEventNames.GetString( 11 ) );
    SfxEventConfiguration::RegisterEvent(SFX_EVENT_PRINTDOC,        String(SfxResId(STR_EVENT_PRINTDOC)),       aEventNames.GetString( 12 ) );
    SfxEventConfiguration::RegisterEvent(SFX_EVENT_MODIFYCHANGED,   String(SfxResId(STR_EVENT_MODIFYCHANGED)), aEventNames.GetString( 13 ) );

    // Subklasse initialisieren
    bDowning = sal_False;
    bInInit = sal_True;
    Init();

    // get CHAOS item pool...
    pAppData_Impl->pPool = NoChaos::GetItemPool();
    SetPool( pAppData_Impl->pPool );

    InsertLateInitHdl( LINK(pNewHdl, SfxNewHdl, InitMem_Impl) );
    InsertLateInitHdl( LINK(this, SfxApplication,SpecialService_Impl) );
    InsertLateInitHdl( STATIC_LINK( pAppData_Impl, SfxAppData_Impl, CreateDocumentTemplates ) );

    bInInit = sal_False;
    if ( bDowning )
        return sal_False;

    // App-Dispatcher aufbauen
    pAppDispat->Push(*this);
    pAppDispat->Flush();
    pAppDispat->DoActivate_Impl( sal_True );

    SvtSaveOptions aSaveOptions;
    pImp->pAutoSaveTimer->SetTimeout( aSaveOptions.GetAutoSaveTime() * 60000 );
    pImp->pAutoSaveTimer->SetTimeoutHdl( LINK( pApp, SfxApplication, AutoSaveHdl_Impl ) );

//(dv)  if ( !pAppData_Impl->bBean )
//(mba)        doFirstTimeInit();

//    Application::PostUserEvent( LINK( this, SfxApplication, OpenClients_Impl ) );

//  DELETEZ(pImp->pIntro);

    // start LateInit
    SfxAppData_Impl *pAppData = Get_Impl();
    pAppData->aLateInitTimer.SetTimeout( 250 );
    pAppData->aLateInitTimer.SetTimeoutHdl( LINK( this, SfxApplication, LateInitTimerHdl_Impl ) );
    pAppData->aLateInitTimer.Start();

    return sal_True;
}

IMPL_LINK( SfxApplication, SpecialService_Impl, void*, pVoid )
{
    if ( pAppData_Impl->bBean )
            return 0;

#if SUPD<613//MUSTINI
    String aWizard = GetIniManager()->Get( DEFINE_CONST_UNICODE("Common"), 0, 0, DEFINE_CONST_UNICODE("RunWizard") );
    sal_Bool bRunWizard = (sal_Bool) (sal_uInt16) aWizard.ToInt32();
    if ( bRunWizard )
    {
        SfxStringItem aReferer( SID_REFERER, DEFINE_CONST_UNICODE("private:user") );
        SfxStringItem aMacro( SID_FILE_NAME, DEFINE_CONST_UNICODE("macro://#InternetSetup.Run.Main()") );
//(mba)        pAppDispat->Execute( SID_OPENDOC, SFX_CALLMODE_ASYNCHRON, &aMacro, &aReferer, 0L );
        GetIniManager()->DeleteKey( DEFINE_CONST_UNICODE("Common"), DEFINE_CONST_UNICODE("RunWizard") );
        GetIniManager()->Flush();
    }
    else if ( !pAppData_Impl->bBean )
    {
        // StarOffice registration
        INetURLObject aORegObj( GetIniManager()->Get( SFX_KEY_USERCONFIG_PATH ), INET_PROT_FILE );
        aORegObj.insertName( DEFINE_CONST_UNICODE( "oreg.ini" ) );
        Config aCfg( aORegObj.PathToFileName() );
        aCfg.SetGroup( "reg" );
        sal_uInt16 nRegKey = (sal_uInt16) aCfg.ReadKey( "registration", "0" ).ToInt32();
        if( nRegKey == 0 )
            GetAppDispatcher_Impl()->Execute(SID_ONLINE_REGISTRATION_DLG, SFX_CALLMODE_ASYNCHRON);
    }
#else
    if ( !pAppData_Impl->bBean )
    {
        // StarOffice registration
        INetURLObject aORegObj( SvtPathOptions().GetUserConfigPath(), INET_PROT_FILE );
        aORegObj.insertName( DEFINE_CONST_UNICODE( "oreg.ini" ) );
        Config aCfg( aORegObj.PathToFileName() );
        aCfg.SetGroup( "reg" );
        sal_uInt16 nRegKey = (sal_uInt16) aCfg.ReadKey( "registration", "0" ).ToInt32();
        if( nRegKey == 0 )
            GetAppDispatcher_Impl()->Execute(SID_ONLINE_REGISTRATION_DLG, SFX_CALLMODE_ASYNCHRON);
    }
#endif
    return 0;
}


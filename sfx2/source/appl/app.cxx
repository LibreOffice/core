/*************************************************************************
 *
 *  $RCSfile: app.cxx,v $
 *
 *  $Revision: 1.98 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-18 12:04:03 $
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

#if defined UNX
#include <limits.h>
#else // UNX
#include <stdlib.h>
#define PATH_MAX _MAX_PATH
#endif // UNX

#include "app.hxx"
#include "frame.hxx"

#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif
#ifndef _TOOLS_SIMPLERESMGR_HXX_
#include <tools/simplerm.hxx>
#endif
#ifndef _CONFIG_HXX
#include <tools/config.hxx>
#endif
#ifndef _BASRDLL_HXX
#include <basic/basrdll.hxx>
#endif

#ifndef SVTOOLS_ASYNCLINK_HXX
#include <svtools/asynclink.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SOUND_HXX //autogen
#include <vcl/sound.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _URLBMK_HXX //autogen
#include <svtools/urlbmk.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif

#include <svtools/svdde.hxx>
#include <tools/urlobj.hxx>
#include <unotools/tempfile.hxx>
#include <osl/file.hxx>
#ifndef GCC
#pragma hdrstop
#endif

#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/lang/XInitialization.hpp>

#ifndef _COM_SUN_STAR_FRAME_XFRAMEACTIONLISTENER_HPP_
#include <com/sun/star/frame/XFrameActionListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMEACTIONEVENT_HPP_
#include <com/sun/star/frame/FrameActionEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMEACTION_HPP_
#include <com/sun/star/frame/FrameAction.hpp>
#endif
#ifndef _COM_SUN_STAR_LOADER_XIMPLEMENTATIONLOADER_HPP_
#include <com/sun/star/loader/XImplementationLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_LOADER_CANNOTACTIVATEFACTORYEXCEPTION_HPP_
#include <com/sun/star/loader/CannotActivateFactoryException.hpp>
#endif
#ifndef _COM_SUN_STAR_MOZILLA_XPLUGININSTANCE_HPP_
#include <com/sun/star/mozilla/XPluginInstance.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_URI_XURIREFERENCEFACTORY_HPP_
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_URI_XVNDSUNSTARSCRIPTURL_HPP_
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>
#endif

#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#include <basic/basmgr.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>

#if SUPD>637
#include <rtl/logfile.hxx>
#endif

#include "appuno.hxx"
#include "sfxhelp.hxx"
#include "request.hxx"
#include "sfxtypes.hxx"
#include "sfxresid.hxx"
#include "arrdecl.hxx"
#include "progress.hxx"
#include "objsh.hxx"
#include "docfac.hxx"
#include "docfile.hxx"
#include "docfilt.hxx"
#include "fltfnc.hxx"
#include "nfltdlg.hxx"
#include "new.hxx"
#include "bindings.hxx"
#include "dispatch.hxx"
#include "viewsh.hxx"
#include "genlink.hxx"
#include "topfrm.hxx"
#include "newhdl.hxx"
#include "appdata.hxx"
#include "openflag.hxx"
#include "app.hrc"
#include "intfrm.hxx"
#include "virtmenu.hxx"
#include "module.hxx"
#include "sfxdir.hxx"
#include "event.hxx"
#include "appimp.hxx"
#include "imestatuswindow.hxx"
#include "workwin.hxx"
#include "module.hxx"
#include "tbxctrl.hxx"
#include "sfxdlg.hxx"
#include "stbitem.hxx"
#include "eventsupplier.hxx"

#ifdef DBG_UTIL
#include "tbxctrl.hxx"
#include "mnuitem.hxx"
#endif

#if defined( WIN ) || defined( WNT ) || defined( OS2 )
#define DDE_AVAILABLE
#endif

#include <svtools/saveopt.hxx>
#include <svtools/undoopt.hxx>
#include <svtools/helpopt.hxx>
#include <svtools/pathoptions.hxx>
#include <svtools/viewoptions.hxx>
#include <svtools/moduleoptions.hxx>
#include <svtools/historyoptions.hxx>
#include <svtools/menuoptions.hxx>
#include <svtools/addxmltostorageoptions.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/useroptions.hxx>
#include <svtools/startoptions.hxx>
#include <svtools/securityoptions.hxx>
#include <svtools/localisationoptions.hxx>
#include <svtools/inetoptions.hxx>
#include <svtools/fontoptions.hxx>
#include <svtools/internaloptions.hxx>
#include <svtools/workingsetoptions.hxx>
#include <svtools/syslocaleoptions.hxx>
#include <svtools/syslocale.hxx>
#include <framework/addonsoptions.hxx>
#ifndef _SVTOOLS_TTPROPS_HXX // handmade
#include <svtools/ttprops.hxx>
#endif
#include <svtools/extendedsecurityoptions.hxx>

using namespace ::com::sun::star;

// Static member
SfxApplication* SfxApplication::pApp = NULL;
static BasicDLL*       pBasic   = NULL;

class SfxPropertyHandler : public PropertyHandler
{
    virtual void Property( ApplicationProperty& );
};

static SfxPropertyHandler*  pPropertyHandler = 0;

SfxPropertyHandler* GetOrCreatePropertyHandler()
{
    if ( !pPropertyHandler )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if ( !pPropertyHandler )
            pPropertyHandler = new SfxPropertyHandler;
    }

    return pPropertyHandler;
}

void SfxPropertyHandler::Property( ApplicationProperty& rProp )
{
    TTProperties* pTTProperties = PTR_CAST( TTProperties, &rProp );
    if ( pTTProperties )
    {
        pTTProperties->nPropertyVersion = TT_PROPERTIES_VERSION;
        switch ( pTTProperties->nActualPR )
        {
            case TT_PR_SLOTS:
            {
                pTTProperties->nSidOpenUrl = SID_OPENURL;
                pTTProperties->nSidFileName = SID_FILE_NAME;
                pTTProperties->nSidNewDocDirect = SID_NEWDOCDIRECT;
                pTTProperties->nSidCopy = SID_COPY;
                pTTProperties->nSidPaste = SID_PASTE;
                pTTProperties->nSidSourceView = SID_SOURCEVIEW;
                pTTProperties->nSidSelectAll = SID_SELECTALL;
                pTTProperties->nSidReferer = SID_REFERER;
                pTTProperties->nActualPR = 0;
            }
            break;
            case TT_PR_DISPATCHER:
            {
                // interface for TestTool
                SfxViewFrame* pViewFrame;
                SfxDispatcher* pDispatcher;
                pViewFrame = SfxViewFrame::Current();
                if ( !pViewFrame )
                    pViewFrame = SfxViewFrame::GetFirst();
                if ( pViewFrame )
                    pDispatcher = pViewFrame->GetDispatcher();
                else
                    pDispatcher = NULL;
                if ( !pDispatcher )
                    pTTProperties->nActualPR = TT_PR_ERR_NODISPATCHER;
                else
                {
                    pDispatcher->SetExecuteMode(EXECUTEMODE_DIALOGASYNCHRON);
                    if ( pTTProperties->mnSID == SID_NEWDOCDIRECT
                      || pTTProperties->mnSID == SID_OPENDOC )
                    {
                        SfxPoolItem** pArgs = pTTProperties->mppArgs;
                        SfxAllItemSet aSet( SFX_APP()->GetPool() );
                        if ( pArgs && *pArgs )
                        {
                            for ( SfxPoolItem **pArg = pArgs; *pArg; ++pArg )
                                aSet.Put( **pArg );
                        }
                        if ( pTTProperties->mnSID == SID_NEWDOCDIRECT )
                        {
                            String aFactory = String::CreateFromAscii("private:factory/");
                            if ( pArgs && *pArgs )
                            {
                                SFX_ITEMSET_ARG( &aSet, pFactoryName, SfxStringItem, SID_NEWDOCDIRECT, FALSE );
                                if ( pFactoryName )
                                    aFactory += pFactoryName->GetValue();
                                else
                                    aFactory += String::CreateFromAscii("swriter");
                            }
                            else
                                aFactory += String::CreateFromAscii("swriter");

                            aSet.Put( SfxStringItem( SID_FILE_NAME, aFactory ) );
                            aSet.ClearItem( SID_NEWDOCDIRECT );
                            pTTProperties->mnSID = SID_OPENDOC;
                        }

                        aSet.Put( SfxStringItem( SID_TARGETNAME, DEFINE_CONST_UNICODE("_blank") ) );
                        if ( pDispatcher->ExecuteFunction( pTTProperties->mnSID, aSet, pTTProperties->mnMode )
                                    == EXECUTE_NO )
                            pTTProperties->nActualPR = TT_PR_ERR_NOEXECUTE;
                        else
                            pTTProperties->nActualPR = 0;
                    }
                    else
                    {
                        if ( pDispatcher->ExecuteFunction(
                                pTTProperties->mnSID, pTTProperties->mppArgs, pTTProperties->mnMode )
                            == EXECUTE_NO )
                            pTTProperties->nActualPR = TT_PR_ERR_NOEXECUTE;
                        else
                            pTTProperties->nActualPR = 0;
                    }
                }
            }
            break;
/*
            case TT_PR_IMG:
            {
                SvDataMemberObjectRef aDataObject = new SvDataMemberObject();
                SvData* pDataBmp = new SvData( FORMAT_BITMAP );
                pDataBmp->SetData( pTTProperties->mpBmp );
                aDataObject->Append( pDataBmp );
                aDataObject->CopyClipboard();
                pTTProperties->nActualPR = 0;
            }
            break;
*/
            default:
            {
                pTTProperties->nPropertyVersion = 0;
            }
        }
        return;
    }
}

#include <framework/imageproducer.hxx>
#include <framework/acceleratorinfo.hxx>
#include <framework/sfxhelperfunctions.hxx>
#include "imagemgr.hxx"

::osl::Mutex SfxApplication::gMutex;

SfxApplication* SfxApplication::GetOrCreate()
{
    // SFX on demand
    ::osl::MutexGuard aGuard(SfxApplication::gMutex);
    if ( !pApp )
    {
        SfxApplication *pNew = new SfxApplication;
        SetApp( pNew );

        ::framework::SetImageProducer( GetImage );
//        ::framework::SetCommandURLFromKeyCode( GetCommandURLFromKeyCode );
//        ::framework::SetKeyCodeFromCommandURL( GetKeyCodeFromCommandURL );
        ::framework::SetToolBoxControllerCreator( SfxToolBoxControllerFactory );
        ::framework::SetStatusBarControllerCreator( SfxStatusBarControllerFactory );

        SfxHelp* pSfxHelp = new SfxHelp;
        Application::SetHelp( pSfxHelp );
        if ( SvtHelpOptions().IsHelpTips() )
            Help::EnableQuickHelp();
        else
            Help::DisableQuickHelp();
        if ( SvtHelpOptions().IsHelpTips() && SvtHelpOptions().IsExtendedHelp() )
            Help::EnableBalloonHelp();
        else
            Help::DisableBalloonHelp();
    }
    return pApp;
}

void SfxApplication::SetApp( SfxApplication* pSfxApp )
{
    static ::osl::Mutex aProtector;
    ::osl::MutexGuard aGuard( aProtector );

    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mb93783) ::SfxApplication::SetApp" );
    DBG_ASSERT( !pApp, "SfxApplication already created!" );
    if ( pApp )
        DELETEZ( pApp );

    pApp = pSfxApp;

    // at the moment a bug may occur when Initialize_Impl returns FALSE, but this is only temporary because all code that may cause such a
    // fault will be moved outside the SFX
    pApp->Initialize_Impl();
}

SfxApplication::SfxApplication()
    : pImp( 0 )
    , _nFeatures( ULONG_MAX )
    , pViewFrame( 0 )
    , pSlotPool( 0 )
    , pResMgr( 0 )
    , pAppDispat( 0 )
    , pMenuMgr( 0 )
    , pAcceleratorMgr( 0 )
    , pStatusBarMgr( 0 )
    , pCfgMgr( 0 )
    , pOptions( 0 )
    , pAppData_Impl( 0 )
    , nInterfaces( 0 )
    , pInterfaces( 0 )
    , bInInit( sal_False )
    , bInExit( sal_False )
    , bDowning( sal_True )
    , bCreatedExternal( sal_False )
{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mb93783) ::SfxApplication::SfxApplication" );

    SetName( DEFINE_CONST_UNICODE("StarOffice") );
    GetpApp()->SetPropertyHandler( GetOrCreatePropertyHandler() );

    SvtViewOptions::AcquireOptions();

    pImp = new SfxApplication_Impl;
    pImp->bConfigLoaded = sal_False;
    pImp->pEmptyMenu = 0;
    pImp->nDocNo = 0;
    pImp->pIntro = 0;
    pImp->pTbxCtrlFac = 0;
    pImp->pStbCtrlFac = 0;
    pImp->pViewFrames = 0;
    pImp->pObjShells = 0;
    pImp->pTemplateDlg = 0;
    pImp->pBasicLibContainer = 0;
    pImp->pDialogLibContainer = 0;
    pImp->pBasicTestWin = 0;
    pImp->pSfxResManager = 0;
    pImp->pOfaResMgr = 0;
    pImp->pSimpleResManager = 0;
    pImp->nWarnLevel = 0;

    pAppData_Impl = new SfxAppData_Impl( this );
    pAppData_Impl->UpdateApplicationSettings( SvtMenuOptions().IsEntryHidingEnabled() );
    pAppData_Impl->m_xImeStatusWindow->init();
    pApp->PreInit();

    RTL_LOGFILE_CONTEXT_TRACE( aLog, "{ initialize DDE" );

#ifdef DDE_AVAILABLE
#ifdef PRODUCT
    InitializeDde();
#else
    if( !InitializeDde() )
    {
        ByteString aStr( "Kein DDE-Service moeglich. Fehler: " );
        if( GetDdeService() )
            aStr += ByteString::CreateFromInt32(GetDdeService()->GetError());
        else
            aStr += '?';
        DBG_ASSERT( sal_False, aStr.GetBuffer() )
    }
#endif
#endif

    if ( !InitLabelResMgr( "iso" ) )
        // no "iso" resource -> search for "ooo" resource
        InitLabelResMgr( "ooo", true );
    pBasic   = new BasicDLL;

    StarBASIC::SetGlobalErrorHdl( LINK( this, SfxApplication, GlobalBasicErrorHdl_Impl ) );



    RTL_LOGFILE_CONTEXT_TRACE( aLog, "} initialize DDE" );
}

SfxApplication::~SfxApplication()
{
    Broadcast( SfxSimpleHint(SFX_HINT_DYING) );

    SfxModule::DestroyModules_Impl();

    // delete global options
    SvtViewOptions::ReleaseOptions();
    delete pBasic;

    if ( !bDowning )
        Deinitialize();

    delete pImp;
    delete pAppData_Impl;
    pApp = 0;
}

//====================================================================

class SfxResourceTimer : public Timer
{
    sal_uInt16 *pnWarnLevel;
public:
    SfxResourceTimer(sal_uInt16 *pn, sal_uInt32 nTimeOut) : pnWarnLevel(pn)
    { SetTimeout(nTimeOut); Start(); }
    virtual void Timeout() { --*pnWarnLevel; delete this; }
};

//--------------------------------------------------------------------

//====================================================================

SfxObjectShell* SfxApplication::GetActiveObjectShell() const

/*  [Beschreibung]

    Diese Methode liefert einen Zeiger auf die aktive <SfxObjectShell>-
    Instanz oder einen 0-Pointer, falls keine SfxObjectShell-Instanz
    aktiv ist.
*/

{
    if ( pViewFrame )
        return pViewFrame->GetObjectShell();
    return 0;
}

//--------------------------------------------------------------------

const String& SfxApplication::GetLastDir_Impl() const

/*  [Beschreibung]

    Interne Methode, mit der im SFx das zuletzt mit der Methode
    <SfxApplication::SetLastDir_Impl()> gesetzte Verzeichnis
    zurueckgegeben wird.

    Dieses ist i.d.R. das zuletzt durch den SfxFileDialog
    angesprochene Verzeichnis.

    [Querverweis]
    <SfxApplication::SetLastDir_Impl()>
*/

{
    return pAppData_Impl->aLastDir;
}

const String& SfxApplication::GetLastSaveDirectory() const

/*  [Beschreibung]

    Wie <SfxApplication::GetLastDir_Impl()>, nur extern

    [Querverweis]
    <SfxApplication::GetLastDir_Impl()>
*/

{
    return GetLastDir_Impl();
}

//--------------------------------------------------------------------

void SfxApplication::SetLastDir_Impl
(
    const String&   rNewDir     /*  kompletter Verzeichnis-Pfad als String */
    )

/*  [Beschreibung]

    Interne Methode, mit der ein Verzeichnis-Pfad gesetzt wird, der
    zuletzt (z.B. durch den SfxFileDialog) angesprochen wurde.

    [Querverweis]
    <SfxApplication::GetLastDir_Impl()>
*/

{
    pAppData_Impl->aLastDir = rNewDir;
}

//--------------------------------------------------------------------

const String& SfxApplication::GetLastFilter_Impl() const
{
    return pAppData_Impl->aLastFilter;
}

//--------------------------------------------------------------------

void SfxApplication::SetLastFilter_Impl( const String &rNewFilter )
{
    pAppData_Impl->aLastFilter = rNewFilter;
}

//--------------------------------------------------------------------

SfxDispatcher* SfxApplication::GetDispatcher_Impl()
{
    return pViewFrame? pViewFrame->GetDispatcher(): pAppDispat;
}

//--------------------------------------------------------------------
void SfxApplication::SetViewFrame( SfxViewFrame *pFrame )
{
    if( pFrame && !pFrame->IsSetViewFrameAllowed_Impl() )
        return;

    if ( pFrame != pViewFrame )
    {
        // get the containerframes ( if one of the frames is an InPlaceFrame )
        SfxViewFrame *pOldContainerFrame = pViewFrame;
        while ( pOldContainerFrame && pOldContainerFrame->GetParentViewFrame_Impl() )
            pOldContainerFrame = pOldContainerFrame->GetParentViewFrame_Impl();
        SfxViewFrame *pNewContainerFrame = pFrame;
        while ( pNewContainerFrame && pNewContainerFrame->GetParentViewFrame_Impl() )
            pNewContainerFrame = pNewContainerFrame->GetParentViewFrame_Impl();

        // DocWinActivate : both frames belong to the same TopWindow
        // TopWinActivate : both frames belong to different TopWindows
        BOOL bDocWinActivate = pOldContainerFrame && pNewContainerFrame &&
                    pOldContainerFrame->GetTopViewFrame() == pNewContainerFrame->GetTopViewFrame();
        BOOL bTaskActivate = pOldContainerFrame != pNewContainerFrame;
        if ( pViewFrame )
        {
            if ( bTaskActivate )
                // prepare UI for deacivation
                pViewFrame->GetFrame()->Deactivate_Impl();
        }

        if ( pOldContainerFrame )
        {
            if ( bTaskActivate )
                NotifyEvent( SfxEventHint( SFX_EVENT_DEACTIVATEDOC, pOldContainerFrame->GetObjectShell() ) );
            pOldContainerFrame->DoDeactivate( bTaskActivate, pFrame );

            if( pOldContainerFrame->GetProgress() )
                pOldContainerFrame->GetProgress()->Suspend();
        }

        pViewFrame = pFrame;

        const SfxObjectShell* pSh = pViewFrame ? pViewFrame->GetObjectShell() : 0;
        //if ( !pSh )
        //{
        //    // otherwise BaseURL is set in activation of document
        //    INetURLObject aObject( SvtPathOptions().GetWorkPath() );
        //    aObject.setFinalSlash();
        //    INetURLObject::SetBaseURL( aObject.GetMainURL( INetURLObject::NO_DECODE ) );
        //}

        if( pNewContainerFrame )
        {
            pNewContainerFrame->DoActivate( bTaskActivate );
            if ( bTaskActivate && pNewContainerFrame->GetObjectShell() )
            {
                pNewContainerFrame->GetObjectShell()->PostActivateEvent_Impl( pNewContainerFrame );
                NotifyEvent(SfxEventHint(SFX_EVENT_ACTIVATEDOC, pNewContainerFrame->GetObjectShell() ) );
            }

            SfxProgress *pProgress = pNewContainerFrame->GetProgress();
            if ( pProgress )
            {
                if( pProgress->IsSuspended() )
                    pProgress->Resume();
                else
                    pProgress->SetState( pProgress->GetState() );
            }

            if ( pViewFrame->GetViewShell() )
            {
                SfxDispatcher* pDisp = pViewFrame->GetDispatcher();
                pDisp->Flush();
                pDisp->Update_Impl(sal_True);
            }
        }
    }
}

//--------------------------------------------------------------------

SfxNewFileDialog*  SfxApplication::CreateNewDialog()
{
    return new SfxNewFileDialog(GetTopWindow(), SFXWB_DOCINFO | SFXWB_PREVIEW );
}

//--------------------------------------------------------------------

short SfxApplication::QuerySave_Impl( SfxObjectShell& rDoc, sal_Bool bAutoSave )
{
    if ( !rDoc.IsModified() )
        return RET_NO;

    String aMsg( SfxResId( STR_ISMODIFIED ) );
    aMsg.SearchAndReplaceAscii( "%1", rDoc.GetTitle() );

    SfxFrame *pFrame = SfxViewFrame::GetFirst(&rDoc)->GetFrame();
    pFrame->Appear();

    WinBits nBits = WB_YES_NO_CANCEL | WB_DEF_NO;
    QueryBox aBox( &pFrame->GetWindow(), nBits, aMsg );

    return aBox.Execute();
}

//--------------------------------------------------------------------

sal_Bool SfxApplication::IsInException() const
{
    return pAppData_Impl->bInException;
}

//--------------------------------------------------------------------

sal_uInt16 SfxApplication::Exception( sal_uInt16 nError )
{
    return 0;
}

//---------------------------------------------------------------------

ResMgr* SfxApplication::CreateResManager( const char *pPrefix )
{
    String aMgrName = String::CreateFromAscii( pPrefix );
    aMgrName += String::CreateFromInt32(SOLARUPD); // aktuelle Versionsnummer
    return ResMgr::CreateResMgr(U2S(aMgrName));
}

//---------------------------------------------------------------------

SimpleResMgr* SfxApplication::CreateSimpleResManager()
{
    SimpleResMgr    *pRet;
    const AllSettings& rAllSettings = Application::GetSettings();
    ::com::sun::star::lang::Locale aLocale = rAllSettings.GetUILocale();
    pRet = new SimpleResMgr( CREATEVERSIONRESMGR_NAME(sfx), aLocale );

    return pRet;
}

//--------------------------------------------------------------------

ResMgr* SfxApplication::GetSfxResManager()
{
    return SfxResId::GetResMgr();
}

//--------------------------------------------------------------------

ResMgr* SfxApplication::GetLabelResManager() const
{
    return pAppData_Impl->pLabelResMgr;
}

//--------------------------------------------------------------------

SimpleResMgr* SfxApplication::GetSimpleResManager()
{
    if ( !pImp->pSimpleResManager )
    {
        pImp->pSimpleResManager = CreateSimpleResManager();
    }
    return pImp->pSimpleResManager;
}

//------------------------------------------------------------------------

void SfxApplication::SetProgress_Impl
(
    SfxProgress *pProgress

)
{
    DBG_ASSERT( ( !pAppData_Impl->pProgress && pProgress ) ||
                ( pAppData_Impl->pProgress && !pProgress ),
                "Progress acitivation/deacitivation mismatch" );

    if ( pAppData_Impl->pProgress && pProgress )
    {
        pAppData_Impl->pProgress->Suspend();
        pAppData_Impl->pProgress->UnLock();
        delete pAppData_Impl->pProgress;
    }

    pAppData_Impl->pProgress = pProgress;
}

//------------------------------------------------------------------------

sal_uInt16 SfxApplication::GetFreeIndex()
{
    return pAppData_Impl->aIndexBitSet.GetFreeIndex()+1;
}

//------------------------------------------------------------------------

void SfxApplication::ReleaseIndex(sal_uInt16 i)
{
    pAppData_Impl->aIndexBitSet.ReleaseIndex(i-1);
}

//--------------------------------------------------------------------

void SfxApplication::EnterAsynchronCall_Impl()
{
    ++pAppData_Impl->nAsynchronCalls;
}

//--------------------------------------------------------------------

void SfxApplication::LeaveAsynchronCall_Impl()
{
    --pAppData_Impl->nAsynchronCalls;
}

//--------------------------------------------------------------------

FASTBOOL SfxApplication::IsInAsynchronCall_Impl() const
{
    return pAppData_Impl->nAsynchronCalls > 0;
}

//--------------------------------------------------------------------

Window* SfxApplication::GetTopWindow() const
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl( SfxViewFrame::Current() );
    return pWork ? pWork->GetWindow() : NULL;
}

//--------------------------------------------------------------------

void SfxApplication::SetTopWindow( WorkWindow *pWindow )
{
    // MT: Removed out commented code, which didn't work because AppWindow usage...
}

sal_Bool SfxApplication::IsPlugin()
{
    // ask property of desktop to get this information
    sal_Bool bReturn = sal_False;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xDesktop( ::comphelper::getProcessServiceFactory()->createInstance( OUSTRING(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ), ::com::sun::star::uno::UNO_QUERY );
    if(xDesktop.is())
    {
        ::com::sun::star::uno::Any aVal = xDesktop->getPropertyValue( ::rtl::OUString::createFromAscii("IsPlugged") );
        aVal >>= bReturn;
    }

    return bReturn;
}

//--------------------------------------------------------------------

String SfxApplication::LocalizeDBName
(
    SfxDBNameConvert eConvert,
    const String& rList,
    char aDel
) const
{
    return rList;
}

//--------------------------------------------------------------------

IMPL_STATIC_LINK( SfxApplication, CookieAlertHdl_Impl, void*, EMPTYARG )
{
    return 0;
}

//--------------------------------------------------------------------

void SfxApplication::SetUserEMailAddress( const String &rEMail )
{
    DBG_ERROR( "Obsolete call!" );
}

//-------------------------------------------------------------------------

void SfxApplication::SetDefFocusWindow( Window *pWin )

/*  [Beschreibung]

    Mit dieser Methode wird das Window gesetzt, auf das beim n"achsten
    <SfxApplication::GrabFocus()> der Focus gesetzt werden soll.

    Ein 'SetDefFocusWindow()' wirkt f"ur genau einen einzigen Aufruf von
    'SfxApplication::GrabFocus()'.

    Damit kann z.B. das in verschiedenen Situationen von Windows kommende
    Focus-Setzen auf MDIWindows verhindert werden.
*/

{
    pAppData_Impl->pDefFocusWin = pWin;
}

//-------------------------------------------------------------------------

void SfxApplication::GrabFocus( Window *pAlternate )

/*  [Beschreibung]

    Mit dieser Methode wird der Focus auf das zuvor mit der Methode
    <SfxApplicaton::SetDefFocusWindow()> gesetzte Window gegrabt. Ist
    keins mehr gesetzt oder wurde es bereits verwendet, wird der Focus
    auf 'pAlternate' gesetzt. Ein 'SetDefFocusWindow()' wirkt f"ur genau
    ein einziges 'SfxApplication::GrabFocus()'.
*/

{
    Window *pWin = pAppData_Impl->pDefFocusWin
                        ? pAppData_Impl->pDefFocusWin
                        : pAlternate;
    pWin->GrabFocus();
    pAppData_Impl->pDefFocusWin = 0;
}

uno::Reference< task::XStatusIndicator > SfxApplication::GetStatusIndicator() const
{
    if ( !pViewFrame )
        return uno::Reference< task::XStatusIndicator >();

    SfxViewFrame *pTop = pViewFrame;
    while ( pTop->GetParentViewFrame_Impl() )
        pTop = pTop->GetParentViewFrame_Impl();

    return pTop->GetFrame()->GetWorkWindow_Impl()->GetStatusIndicator();
}

SfxViewFrame* SfxApplication::GetViewFrame()
{
    return pViewFrame;
}

UniqueIndex* SfxApplication::GetEventHandler_Impl()
{
    return pImp->pEventHdl;
}

SfxTbxCtrlFactArr_Impl&     SfxApplication::GetTbxCtrlFactories_Impl() const
{
    return *pImp->pTbxCtrlFac;
}

SfxStbCtrlFactArr_Impl&     SfxApplication::GetStbCtrlFactories_Impl() const
{
    return *pImp->pStbCtrlFac;
}

SfxMenuCtrlFactArr_Impl&    SfxApplication::GetMenuCtrlFactories_Impl() const
{
    return *pImp->pMenuCtrlFac;
}

SfxViewFrameArr_Impl&       SfxApplication::GetViewFrames_Impl() const
{
    return *pImp->pViewFrames;
}

SfxViewShellArr_Impl&       SfxApplication::GetViewShells_Impl() const
{
    return *pImp->pViewShells;
}

SfxObjectShellArr_Impl&     SfxApplication::GetObjectShells_Impl() const
{
    return *pImp->pObjShells;
}

void SfxApplication::Invalidate( USHORT nId )
{
    for( SfxViewFrame* pFrame = SfxViewFrame::GetFirst(); pFrame; pFrame = SfxViewFrame::GetNext( *pFrame ) )
        Invalidate_Impl( pFrame->GetBindings(), nId );
}

#define DOSTRING( x )                       #x
#define STRING( x )                         DOSTRING( x )

typedef long (SAL_CALL *basicide_handle_basic_error)(void*);
typedef rtl_uString* (SAL_CALL *basicide_choose_macro)(BOOL, BOOL, rtl_uString*);
typedef void* (SAL_CALL *basicide_macro_organizer)(INT16);
IMPL_LINK( SfxApplication, GlobalBasicErrorHdl_Impl, StarBASIC*, pBasic )
{
    // get basctl dllname
    String sLibName = String::CreateFromAscii( STRING( DLL_NAME ) );
    sLibName.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "sfx" ) ), String( RTL_CONSTASCII_USTRINGPARAM( "basctl" ) ) );
    ::rtl::OUString aLibName( sLibName );

    // load module
    oslModule handleMod = osl_loadModule( aLibName.pData, 0 );

    // get symbol
    ::rtl::OUString aSymbol( RTL_CONSTASCII_USTRINGPARAM( "basicide_handle_basic_error" ) );
    basicide_handle_basic_error pSymbol = (basicide_handle_basic_error) osl_getSymbol( handleMod, aSymbol.pData );

    // call basicide_handle_basic_error in basctl
    long nRet = pSymbol( pBasic );

    return nRet;
}

sal_Bool SfxApplication::IsXScriptURL( const String& rScriptURL )
{
    sal_Bool result = FALSE;

    ::com::sun::star::uno::Reference
        < ::com::sun::star::lang::XMultiServiceFactory > xSMgr =
            ::comphelper::getProcessServiceFactory();

    ::com::sun::star::uno::Reference
        < ::com::sun::star::uri::XUriReferenceFactory >
            xFactory( xSMgr->createInstance(
                ::rtl::OUString::createFromAscii(
                    "com.sun.star.uri.UriReferenceFactory" ) ),
                ::com::sun::star::uno::UNO_QUERY );

    if ( xFactory.is() )
    {
        try
        {
            ::com::sun::star::uno::Reference
                < ::com::sun::star::uri::XVndSunStarScriptUrl >
                    xUrl( xFactory->parse( rScriptURL ),
                        ::com::sun::star::uno::UNO_QUERY );

            if ( xUrl.is() )
            {
                result = TRUE;
            }
        }
        catch ( ::com::sun::star::uno::RuntimeException& re )
        {
            // ignore, will just return FALSE
        }
    }
    return result;
}

::rtl::OUString
SfxApplication::ChooseScript()
{
    ::rtl::OUString aScriptURL;

    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    if ( pFact )
    {
        OSL_TRACE("create selector dialog");

          AbstractScriptSelectorDialog* pDlg =
            pFact->CreateScriptSelectorDialog( NULL );

        OSL_TRACE("done, now exec it");

          USHORT nRet = pDlg->Execute();

        OSL_TRACE("has returned");

        if ( nRet == RET_OK )
        {
            aScriptURL = pDlg->GetScriptURL();
        }

          delete pDlg;
    }
    return aScriptURL;
}

::rtl::OUString SfxApplication::ChooseMacro( BOOL bExecute, BOOL bChooseOnly, const ::rtl::OUString& rMacroDesc )
{
    // get basctl dllname
    String sLibName = String::CreateFromAscii( STRING( DLL_NAME ) );
    sLibName.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "sfx" ) ), String( RTL_CONSTASCII_USTRINGPARAM( "basctl" ) ) );
    ::rtl::OUString aLibName( sLibName );

    // load module
    oslModule handleMod = osl_loadModule( aLibName.pData, 0 );

    // get symbol
    ::rtl::OUString aSymbol( RTL_CONSTASCII_USTRINGPARAM( "basicide_choose_macro" ) );
    basicide_choose_macro pSymbol = (basicide_choose_macro) osl_getSymbol( handleMod, aSymbol.pData );

    // call basicide_choose_macro in basctl
    rtl_uString* pScriptURL = pSymbol( bExecute, bChooseOnly, rMacroDesc.pData );

    ::rtl::OUString aScriptURL( pScriptURL );
    rtl_uString_release( pScriptURL );
    return aScriptURL;
}

void SfxApplication::MacroOrganizer( INT16 nTabId )
{
    // get basctl dllname
    String sLibName = String::CreateFromAscii( STRING( DLL_NAME ) );
    sLibName.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "sfx" ) ), String( RTL_CONSTASCII_USTRINGPARAM( "basctl" ) ) );
    ::rtl::OUString aLibName( sLibName );

    // load module
    oslModule handleMod = osl_loadModule( aLibName.pData, 0 );

    // get symbol
    ::rtl::OUString aSymbol( RTL_CONSTASCII_USTRINGPARAM( "basicide_macro_organizer" ) );
    basicide_macro_organizer pSymbol = (basicide_macro_organizer) osl_getSymbol( handleMod, aSymbol.pData );

    // call basicide_choose_macro in basctl
    pSymbol( nTabId );
}


/*************************************************************************
 *
 *  $RCSfile: app.cxx,v $
 *
 *  $Revision: 1.69 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 07:55:44 $
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
#ifndef _CONFIG_HXX //autogen
#include <vcl/config.hxx>
#endif
#ifndef _SYSTEM_HXX //autogen
#include <vcl/system.hxx>
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
#ifndef _FILELIST_HXX //autogen
#include <so3/filelist.hxx>
#endif
#ifndef _URLBMK_HXX //autogen
#include <svtools/urlbmk.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _EXTATTR_HXX //autogen
#include <svtools/extattr.hxx>
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
#pragma hdrstop

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
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#include <basic/basmgr.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>

#if SUPD>637
#include <rtl/logfile.hxx>
#endif

#include <appuno.hxx>
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
#include "cfgmgr.hxx"
#include "fltfnc.hxx"
#include "nfltdlg.hxx"
#include "new.hxx"
#include "bindings.hxx"
#include "dispatch.hxx"
#include "viewsh.hxx"
#include "genlink.hxx"
#include "accmgr.hxx"
#include "tbxmgr.hxx"
#include "mnumgr.hxx"
#include "topfrm.hxx"
#include "newhdl.hxx"
#include "appdata.hxx"
#include "openflag.hxx"
#include "app.hrc"
#include "interno.hxx"
#include "ipenv.hxx"
#include "intfrm.hxx"
#include "virtmenu.hxx"
#include "module.hxx"
#include "sfxdir.hxx"
#include "event.hxx"
#include "appimp.hxx"
#include "imestatuswindow.hxx"

#ifdef DBG_UTIL
#include "tbxctrl.hxx"
#include "stbitem.hxx"
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

// Static member
SfxApplication* SfxApplication::pApp = NULL;

static SvtSaveOptions *pSaveOptions = NULL;
static SvtUndoOptions *pUndoOptions = NULL;
static SvtHelpOptions *pHelpOptions = NULL;
static SvtModuleOptions *pModuleOptions = NULL;
static SvtHistoryOptions *pHistoryOptions = NULL;
static SvtMenuOptions *pMenuOptions = NULL;
static SvtAddXMLToStorageOptions *pXMLOptions = NULL;
static SvtMiscOptions *pMiscOptions = NULL;
static SvtUserOptions *pUserOptions = NULL;
static SvtStartOptions *pStartOptions = NULL;
static SvtSecurityOptions *pSecurityOptions = NULL;
static SvtLocalisationOptions *pLocalisationOptions = NULL;
static SvtInetOptions *pInetOptions = NULL;
static SvtFontOptions *pFontOptions = NULL;
static SvtInternalOptions *pInternalOptions = NULL;
static SvtSysLocaleOptions *pSysLocaleOptions = NULL;
static SvtSysLocale *pSysLocale = NULL;
static SvtExtendedSecurityOptions* pExtendedSecurityOptions = NULL;
static framework::AddonsOptions* pAddonsOptions = NULL;


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
#if SUPD<613//MUSTINI
    SfxApplication* pApp = SFX_APP();
    SfxIniManager* pIni = pApp->GetIniManager();

    // AppIniManger?
    SfxAppIniManagerProperty* pAppIniMgr = PTR_CAST(SfxAppIniManagerProperty, &rProp);
    if ( pAppIniMgr )
    {
        pAppIniMgr->SetIniManager( pIni );
        return;
    }
#endif
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
                        pTTProperties->mnSID;
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

SfxApplication* SfxApplication::GetOrCreate()
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );

    // SFX on demand
    if ( !pApp )
    {
        com::sun::star::uno::Reference < com::sun::star::lang::XInitialization >
            xWrp(::comphelper::getProcessServiceFactory()->createInstance( DEFINE_CONST_UNICODE("com.sun.star.office.OfficeWrapper")), com::sun::star::uno::UNO_QUERY );
            xWrp->initialize( com::sun::star::uno::Sequence < com::sun::star::uno::Any >() );
//        SfxApplication *pNew = new SfxApplication;
//        pNew->StartUpScreen( NULL );
//        SetApp( pNew );
    }

    return pApp;
}

void SfxApplication::SetApp( SfxApplication* pSfxApp )
{
    static ::osl::Mutex aProtector;
    ::osl::MutexGuard aGuard( aProtector );

#if SUPD>637
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mb93783) ::SfxApplication::SetApp" );
#endif
    DBG_ASSERT( !pApp, "SfxApplication already created!" );
    if ( pApp )
        DELETEZ( pApp );

    pApp = pSfxApp;

    // at the moment a bug may occur when Initialize_Impl returns FALSE, but this is only temporary because all code that may cause such a
    // fault will be moved outside the SFX
    pApp->Initialize_Impl();
}

SfxApplication::SfxApplication()
    : _nFeatures( ULONG_MAX )
    , pImp( 0 )
    , pAppData_Impl( 0 )
    , pMenuMgr( 0 )
    , pAcceleratorMgr( 0 )
    , pStatusBarMgr( 0 )
    , pAppDispat( 0 )
    , bDispatcherLocked( sal_False )
    , pResMgr( 0 )
    , pCfgMgr( 0 )
    , pSlotPool( 0 )
    , pInterfaces( 0 )
    , bInInit( sal_False )
    , bInExit( sal_False )
    , bDowning( sal_True )
    , bCreatedExternal( sal_False )
    , pOptions( 0 )
    , pViewFrame( 0 )
    , pImageMgr( 0 )
    , nInterfaces( 0 )
{
#if SUPD>637
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mb93783) ::SfxApplication::SfxApplication" );
#endif

    GetpApp()->SetPropertyHandler( GetOrCreatePropertyHandler() );

#if SUPD>637
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "{ precreate svtools options objects" );
#endif
    pSaveOptions = new SvtSaveOptions;
    pUndoOptions = new SvtUndoOptions;
    pHelpOptions = new SvtHelpOptions;
    pModuleOptions = new SvtModuleOptions;
    pHistoryOptions = new SvtHistoryOptions;
    pMenuOptions = new SvtMenuOptions;
    pXMLOptions = new SvtAddXMLToStorageOptions;
    pMiscOptions = new SvtMiscOptions;
    pUserOptions = new SvtUserOptions;
    pStartOptions = new SvtStartOptions;
    pSecurityOptions = new SvtSecurityOptions;
    pLocalisationOptions = new SvtLocalisationOptions;
    pInetOptions = new SvtInetOptions;
    pFontOptions = new SvtFontOptions;
    pInternalOptions = new SvtInternalOptions;
    pSysLocaleOptions = new SvtSysLocaleOptions;
    pExtendedSecurityOptions = new SvtExtendedSecurityOptions;
    pAddonsOptions = new framework::AddonsOptions;
    SvtViewOptions::AcquireOptions();
#if SUPD>637
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "} precreate svtools options objects" );
#endif

/*
#if SUPD>637
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "{ UCB_Helper::Initialize" );
#endif
    UCB_Helper::Initialize();
#if SUPD>637
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "} UCB_Helper::Initialize" );
#endif
*/
    pImp = new SfxApplication_Impl;
    pImp->bConfigLoaded = sal_False;
    pImp->pEmptyMenu = 0;
    pImp->nDocNo = 0;
    pImp->pIntro = 0;
    pImp->pTbxCtrlFac = 0;
    pImp->pStbCtrlFac = 0;
    pImp->pViewFrames = 0;
    pImp->pObjShells = 0;
    pImp->bAutoSaveNow = sal_False;
    pImp->pTemplateDlg = 0;
    pImp->pBasicMgr = 0;
    pImp->pBasicLibContainer = 0;
    pImp->pDialogLibContainer = 0;
    pImp->pBasicTestWin = 0;
    pImp->pSfxResManager = 0;
    pImp->pOfaResMgr = 0;
    pImp->pSimpleResManager = 0;
    pImp->nWarnLevel = 0;
    pImp->pAutoSaveTimer = 0;
/*AS
#if SUPD>637
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "{ set locale settings" );
#endif
    String sLanguage = SvtPathOptions().SubstituteVariable(String::CreateFromAscii("$(langid)"));
    LanguageType eUILanguage = (LanguageType) sLanguage.ToInt32();
    LanguageType eLanguage = pSysLocaleOptions->GetLocaleLanguageType();
    AllSettings aSettings( Application::GetSettings() );
    aSettings.SetUILanguage( eUILanguage );
    aSettings.SetLanguage( eLanguage );
    Application::SetSettings( aSettings );

#if SUPD>637
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "} set locale settings" );
#endif
*/
    // Create instance of SvtSysLocale _after_ setting the locale at the application,
    // so that it can initialize itself correctly.
    pSysLocale = new SvtSysLocale;

    pAppData_Impl = new SfxAppData_Impl( this );
    pAppData_Impl->UpdateApplicationSettings( SvtMenuOptions().IsEntryHidingEnabled() );
    pAppData_Impl->m_xImeStatusWindow->init();
    pApp->PreInit();

#if SUPD>637
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "{ create SfxConfigManager" );
#endif
    pCfgMgr = new SfxConfigManager;
#if SUPD>637
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "} create SfxConfigManager" );
#endif

#if SUPD>637
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "{ initialize DDE" );
#endif
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
#if SUPD>637
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "} initialize DDE" );
#endif
}

SfxApplication::~SfxApplication()
{
    // delete global options
    SvtViewOptions::ReleaseOptions();
    delete pSaveOptions;
    delete pUndoOptions;
    delete pHelpOptions;
    delete pModuleOptions;
    delete pHistoryOptions;
    delete pMenuOptions;
    delete pXMLOptions;
    delete pMiscOptions;
    delete pUserOptions;
    delete pStartOptions;
    delete pSecurityOptions;
    delete pLocalisationOptions;
    delete pInetOptions;
    delete pFontOptions;
    delete pInternalOptions;
    delete pSysLocaleOptions;
    delete pSysLocale;
    delete pExtendedSecurityOptions;
    delete pAddonsOptions;

    if ( !bDowning )
        Deinitialize();

    Broadcast( SfxSimpleHint(SFX_HINT_DYING) );

    // better call SvFactory::DeInit, because this will remove ALL factories,
    // but it will fail because the ConfigManager has a storage that is a SvObject
    SfxObjectFactory::RemoveAll_Impl();

//    UCB_Helper::Deinitialize();

    delete pCfgMgr;
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

void SfxApplication::LockDispatcher
(
    sal_Bool bLock              /*  sal_True
                                schaltet alle SfxDispatcher ein

                                sal_False
                                schaltet alle SfxDispatcher aus */
)

/*  [Beschreibung]

    Mit dieser Methode werden alle Dispatcher der Applikation global
    blockiert (bLock==sal_True) bzw. grundsaetzlich freigegeben
    (bLock==sal_False).

    Unabhaengig von diesem zentralen Schalter kann jeder Dispatcher
    einzeln gelockt sein:

        Dispatcher X    global      =>  gesamt

        gelockt         gelockt     =>  gelockt
        freigegeben     gelockt     =>  gelockt
        gelockt         freigegeben =>  gelockt
        freigegeben     freigegeben =>  freigegeben

    Wenn der aktive Dispatcher gelockt ist, werden keine Requests mehr
    dispatcht.

    [Querverweise]
    <SfxDispatcher::Lock(sal_Bool)> */

{
    bDispatcherLocked = bLock;
    if ( !bLock )
    {
        GetDispatcher_Impl()->InvalidateBindings_Impl( pAppData_Impl->bInvalidateOnUnlock );
        pAppData_Impl->bInvalidateOnUnlock = sal_False;
    }
}

//--------------------------------------------------------------------

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

sal_Bool IsTemplate_Impl( const String& aPath )
{
    INetURLObject aObj( aPath );
    DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL!" );

    if ( aObj.getExtension().CompareIgnoreCaseToAscii( "vor" ) == COMPARE_EQUAL )
        return sal_True;

    SvEaMgr aMgr( aPath );
    String aType;

    if ( aMgr.GetFileType(aType) )
    {
        const SfxFilter* pFilter = SFX_APP()->GetFilterMatcher().GetFilter4EA( aType );
        if( pFilter && pFilter->IsOwnTemplateFormat() )
            return sal_True;
    }

    return sal_False;
}

extern void FATToVFat_Impl( String& );

#if 0
String GetURL_Impl( const String& rName )
{
    // if the filename is a physical name, it is the client file system, not the file system
    // of the machine where the office is running ( if this are different machines )
    // so in the remote case we can't handle relative filenames as arguments, because they
    // are parsed relative to the program path
    // the file system of the client is addressed through the "file:" protocol
    ::rtl::OUString aProgName, aTmp;
    ::vos::OStartupInfo aInfo;
    aInfo.getExecutableFile( aProgName );
    aTmp = aProgName;
    INetURLObject aObj( aTmp );
    bool bWasAbsolute;
    INetURLObject aURL = aObj.smartRel2Abs( rName, bWasAbsolute );
    return aURL.GetMainURL(INetURLObject::NO_DECODE);
}

void SfxApplication::HandleAppEvent( const ApplicationEvent& rAppEvent )
{
    if ( rAppEvent.IsOpenEvent() )
    {
        // die Parameter enthalten die zu "offnenden Dateien
        for(sal_uInt16 i=0;i<rAppEvent.GetParamCount();i++)
        {
            // Dateiname rausholen
            String aName( rAppEvent.GetParam(i) );
            if ( COMPARE_EQUAL == aName.CompareToAscii("/userid:",8) )
                continue;
#ifdef WNT
            FATToVFat_Impl( aName );
#endif
            aName = GetURL_Impl(aName);
            SfxStringItem aFileName( SID_FILE_NAME, aName );

            // is it a template ?
            const SfxPoolItem* pItem = NULL;
            SfxBoolItem aTemplate( SID_TEMPLATE, TRUE );
            if ( IsTemplate_Impl( aName ) )
                pItem = &aTemplate;

            // open the document
            if ( pItem || !DocAlreadyLoaded( aName, sal_True, sal_True, sal_False ) )
            {
                SfxBoolItem aNewView( SID_OPEN_NEW_VIEW, sal_False );
                SfxStringItem aTargetName( SID_TARGETNAME, DEFINE_CONST_UNICODE("_blank") );
                SfxStringItem aReferer( SID_REFERER, DEFINE_CONST_UNICODE("private:OpenEvent") );
                pAppDispat->Execute( SID_OPENDOC, SFX_CALLMODE_SYNCHRON,
                        &aTargetName, &aFileName, &aNewView, &aReferer, pItem, 0L );
            }
        }
    }
    else if(rAppEvent.IsPrintEvent() )
    {
        // loop on parameters: files to print and name of printer
        SfxStringItem aPrinterName(SID_PRINTER_NAME, String());
        for (sal_uInt16 i=0;i<rAppEvent.GetParamCount();i++)
        {
            // is the parameter a printername ?
            String aName(rAppEvent.GetParam(i));
            if(aName.Len()>1 && *aName.GetBuffer()=='@')
            {
                aPrinterName.SetValue( aName.Copy(1) );
                continue;
            }

#ifdef WNT
            FATToVFat_Impl( aName );
#endif
            SfxStringItem aTargetName( SID_TARGETNAME, DEFINE_CONST_UNICODE("_blank") );
            SfxStringItem aFileName( SID_FILE_NAME, GetURL_Impl( aName ) );
            SfxBoolItem aNewView(SID_OPEN_NEW_VIEW, sal_True);
            SfxBoolItem aHidden(SID_HIDDEN, sal_True);
            SfxBoolItem aSilent(SID_SILENT, sal_True);
            const SfxPoolItem *pRet = pAppDispat->Execute( SID_OPENDOC, SFX_CALLMODE_SYNCHRON,
                    &aTargetName, &aFileName, &aNewView, &aHidden, &aSilent, 0L );
            if ( !pRet )
                continue;

            const SfxViewFrameItem *pFrameItem = PTR_CAST(SfxViewFrameItem, pRet);
            if ( pFrameItem && pFrameItem->GetFrame() )
            {
                SfxViewFrame *pFrame = pFrameItem->GetFrame();
                SfxBoolItem aSilent( SID_SILENT, sal_True );
                pFrame->GetDispatcher()->Execute( SID_PRINTDOC, SFX_CALLMODE_SYNCHRON,
                        &aPrinterName, &aSilent, 0L );
                pFrame->GetFrame()->DoClose();
            }
        }
    }
    else if ( rAppEvent.GetEvent() == "APPEAR" )
    {
        if( !pAppData_Impl->bInvisible )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFramesSupplier >
                    xDesktop( ::comphelper::getProcessServiceFactory()->createInstance( OUSTRING(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ),
                    ::com::sun::star::uno::UNO_QUERY );
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xTask = xDesktop->getActiveFrame();
            if ( !xTask.is() )
            {
                // If no frame is currently active - we searh for any other one which exist in general.
                ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > xList( xDesktop->getFrames(), ::com::sun::star::uno::UNO_QUERY );
                sal_Int32 nCount = xList->getCount();
                if (nCount>0)
                {
                    ::com::sun::star::uno::Any aItem = xList->getByIndex(0);
                    if ( !(aItem>>=xTask) || !xTask.is() )
                        pAppData_Impl->bInvisible = TRUE;
                }
            }

            if ( xTask.is() )
            {
                Window* pWindow = VCLUnoHelper::GetWindow( xTask->getContainerWindow() );
                pWindow->ToTop();
            }
        }

        if( pAppData_Impl->bInvisible )
        {
            pAppData_Impl->bInvisible = FALSE;
            OpenClients();
        }
    }
}
#endif

//--------------------------------------------------------------------

long SfxAppFocusChanged_Impl( void* pObj, void* pArg )
{
/*
    SfxApplication *pApp = SFX_APP();
    if ( pApp && !pApp->IsDowning() )
    {
        Help* pHelp = Application::GetHelp();
        Window* pFocusWindow = Application::GetFocusWindow();
        if ( pHelp && pFocusWindow )
        {
            sal_uInt32 nId = pFocusWindow->GetHelpId();
            while ( !nId && pFocusWindow )
            {
                pFocusWindow = pFocusWindow->GetParent();
                nId = pFocusWindow ? pFocusWindow->GetHelpId() : 0;
            }
            ((SfxHelp_Impl*)pHelp)->SlotExecutedOrFocusChanged(
                nId, sal_False, SvtHelpOptions().IsHelpAgentAutoStartMode() );
        }
    }
 */
    return 0;
}

void SfxApplication::FocusChanged()
{
    static svtools::AsynchronLink *pFocusCallback = new svtools::AsynchronLink( Link( 0, SfxAppFocusChanged_Impl ) );
    pFocusCallback->Call( this, sal_True );
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
        if ( !pFrame && !bDowning )
        {
            // activate any frame to avoid CurrentViewFrame == NULL
            SfxFrameArr_Impl& rArr = *pAppData_Impl->pTopFrames;
            for( sal_uInt16 nPos = rArr.Count(); nPos--; )
            {
                SfxFrame* pCurFrame = rArr[ nPos ];
                SfxViewFrame* pView = pCurFrame->GetCurrentViewFrame();
                if ( pView && pView != pViewFrame )
                {
                    pFrame = pView;
                    break;
                }
            }
        }

        // check if activated or deactivated frame is a InPlaceFrame
        SfxInPlaceFrame *pOld = PTR_CAST( SfxInPlaceFrame, pViewFrame );
        SfxInPlaceFrame *pNew = PTR_CAST( SfxInPlaceFrame, pFrame );

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
            {
                // prepare UI for deacivation
                pViewFrame->GetFrame()->Deactivate_Impl();

                if ( pOld )
                {
                    // broadcast deactivation event
                    NotifyEvent( SfxEventHint( SFX_EVENT_DEACTIVATEDOC, pViewFrame->GetObjectShell() ) );

                    // inplace deactivation needed
                    SvInPlaceClient *pCli = pOldContainerFrame->GetViewShell() ? pOldContainerFrame->GetViewShell()->GetIPClient() : NULL;
                    if ( pCli && pCli->GetProtocol().IsUIActive() )
                    {
                        if ( bDocWinActivate )
                        {
                            pCli->GetIPObj()->GetIPEnv()->DoShowUITools( sal_False );
                            pCli->GetProtocol().DocWinActivate( sal_False );
                        }
                        else
                            pCli->GetProtocol().TopWinActivate( sal_False );
                    }
                }
            }
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

        SfxWorkWindow* pWork = pViewFrame ? pViewFrame->GetFrame()->GetWorkWindow_Impl() : NULL;
        Window* pWin = pWork ? pWork->GetTopWindow() : NULL;
        const SfxObjectShell* pSh = pViewFrame ? pViewFrame->GetObjectShell() : 0;
        if ( !pSh )
        {
            // otherwise BaseURL is set in activation of document
            INetURLObject aObject( SvtPathOptions().GetWorkPath() );
            aObject.setFinalSlash();
            INetURLObject::SetBaseURL( aObject.GetMainURL( INetURLObject::NO_DECODE ) );
        }

        if( pNewContainerFrame )
        {
            pNewContainerFrame->DoActivate( bTaskActivate );
            if ( bTaskActivate )
                pNewContainerFrame->GetObjectShell()->PostActivateEvent_Impl();

            SfxProgress *pProgress = pNewContainerFrame->GetProgress();
            if ( pProgress )
            {
                if( pProgress->IsSuspended() )
                    pProgress->Resume();
                else
                    pProgress->SetState( pProgress->GetState() );
            }

            if ( !pNew )
            {
                SfxDispatcher* pDisp = pViewFrame->GetDispatcher();
                pDisp->Flush();
                pDisp->Update_Impl(sal_True);
            }
        }

        if ( pViewFrame && pViewFrame->GetViewShell() )
        {
            if ( bTaskActivate )
            {
                if ( pNew )
                {
                    // Activate IPClient if present
                    SvInPlaceClient *pCli = pNewContainerFrame->GetViewShell()->GetIPClient();
                    if ( pCli && pCli->GetProtocol().IsUIActive() )
                    {
                        if ( bDocWinActivate )
                        {
                            pCli->GetIPObj()->GetIPEnv()->DoShowUITools( sal_True );
                            pCli->GetProtocol().DocWinActivate( sal_True );
                        }
                        else
                            pCli->GetProtocol().TopWinActivate( sal_True );
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------

//--------------------------------------------------------------------
/*
sal_uInt32 SfxApplication::DetectFilter( const String &rFileName,
                                    const SfxFilter **ppFilter,
                                    sal_uInt16 nFilterClass )
{
    SfxMedium aSfxMedium(rFileName,(STREAM_READ | STREAM_SHARE_DENYNONE),sal_False);

    return DetectFilter(aSfxMedium, ppFilter, nFilterClass );
}

//-------------------------------------------------------------------------

sal_uInt32 SfxApplication::DetectFilter(
    SfxMedium& rMedium,const SfxFilter **ppFilter, sal_uInt16 nFilterClass )
{
    const SfxFilter *pFilter=0;
    SvEaMgr aMgr( rMedium.GetName() );
    String aType;
    if ( !SfxObjectFactory::HasObjectFactories() )
        return 1; HACK(Error-Code verwenden) ;

    SfxFilterMatcher rMatcher( SfxObjectFactory::GetDefaultFactory().GetFilterContainer()  );
    if( aMgr.GetFileType( aType ))
        pFilter = rMatcher.GetFilter4EA( aType );

    if( !pFilter)
    {
        if ( !rMedium.IsRemote() )
        {
            SvStorageRef aStor = rMedium.GetStorage();
            if ( !aStor.Is() )
                return ERRCODE_IO_GENERAL;
            pFilter = rMatcher.GetFilter4ClipBoardId(aStor->GetFormat());
        }
        else
        {
            // Finden anhand der Extension
            pFilter = rMatcher.GetFilter4Extension( INetURLObject( rMedium.GetName() ).GetName() );
            if ( pFilter && pFilter->UsesStorage() )
                pFilter = 0;
        }
    }

    if(pFilter)
        *ppFilter=pFilter;

    return pFilter? 0: 1; HACK(Error-Code verwenden)
}
*/

//--------------------------------------------------------------------

SfxNewFileDialog*  SfxApplication::CreateNewDialog()
{
    return new SfxNewFileDialog(GetTopWindow(), SFXWB_DOCINFO | SFXWB_PREVIEW );
}

//--------------------------------------------------------------------

const SfxFilter* SfxApplication::GetFilter
(
    const SfxObjectFactory &rFact,
    const String &rFilterName
    )   const
{
    DBG_ASSERT( rFilterName.Search( ':' ) == STRING_NOTFOUND,
                "SfxApplication::GetFilter erwartet unqualifizierte Namen" );
    return rFact.GetFilterContainer()->GetFilter4FilterName(rFilterName);
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

    WinBits nBits = WB_YES_NO_CANCEL;
    nBits |= bAutoSave ? WB_DEF_YES : WB_DEF_NO;
    QueryBox aBox( &pFrame->GetWindow(), nBits, aMsg );

    if ( bAutoSave )
        aBox.SetText( String( SfxResId( STR_AUTOSAVE ) ) );

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
    if ( pAppData_Impl->bInException )
        Application::Abort( pImp->aDoubleExceptionString );

    pAppData_Impl->bInException = sal_True;

    if( SfxNewHdl::Get() )
    {
        SfxNewHdl::Get()->FlushWarnMem();
        SfxNewHdl::Get()->FlushExceptMem();
    }

    INetURLObject aSaveObj( SvtPathOptions().GetBackupPath() );

    // save all modified documents and close all documents
    // Do it only, if it's allowed! Ask configuration for right flag.
    if( Application::IsInExecute() )
    {
        SfxObjectShell *pIter, *pNext;
        for(pIter = SfxObjectShell::GetFirst(); pIter; pIter = pNext)
        {
            pNext = SfxObjectShell::GetNext(*pIter);
            if( pIter->IsModified() && pIter->GetName().CompareToAscii("BasicIDE") != COMPARE_EQUAL && !pIter->IsLoading() )
            {
                //try
                {
                    // backup unsaved document
                    SFX_ITEMSET_ARG( pIter->GetMedium()->GetItemSet(), pPassItem, SfxStringItem, SID_PASSWORD, sal_False );
                    SfxRequest aReq(SID_SAVEASDOC, SFX_CALLMODE_SYNCHRON, pIter->GetPool());

                    sal_Bool        bHadName    = pIter->HasName()                  ;
                    INetURLObject   aOldURL( pIter->GetMedium()->GetName() );
                    String          aOldName    = pIter->GetTitle()                 ;

                    const SfxFilter *pFilter = pIter->GetMedium()->GetFilter();
                    const SfxFilter *pOrigFilter = pFilter;
                    if ( !pFilter || ( pFilter->GetFilterFlags() & SFX_FILTER_PACKED ) || !( pFilter->GetFilterFlags() & SFX_FILTER_EXPORT ) )
                        // packed files must be saved with default format, but remember original filter !
                        pFilter = pIter->GetFactory().GetFilter(0);

                    String aSaveName, aSavePath = aSaveObj.GetMainURL( INetURLObject::NO_DECODE );
                    String aFilterName;
                    if ( pFilter )
                    {
                        aFilterName = pFilter->GetFilterName();
                        ::utl::TempFile aTempFile( &aSavePath );
                        aSaveName = aTempFile.GetURL();
                    }
                    else
                    {
                        String aExt( DEFINE_CONST_UNICODE( ".sav" ) );
                        ::utl::TempFile aTempFile( DEFINE_CONST_UNICODE( "exc" ), &aExt, &aSavePath );
                        aSaveName = aTempFile.GetURL();
                    }

                    aReq.AppendItem( SfxStringItem( SID_FILE_NAME, aSaveName ) );
                    aReq.AppendItem( SfxStringItem( SID_FILTER_NAME, aFilterName ) );
                    if ( pPassItem )
                        aReq.AppendItem( *pPassItem );

                    pIter->ExecuteSlot(aReq);

                    pInternalOptions->PushRecoveryItem( bHadName ? aOldURL.GetMainURL( INetURLObject::NO_DECODE ) : aOldName                ,
                                                        pOrigFilter ? pOrigFilter->GetFilterName() : aFilterName      ,
                                                        aSaveName                                               );
                }
                /*catch ( ::Exception & )
                {
                }*/
            }
        }

        if ( ( nError & EXC_MAJORTYPE ) != EXC_DISPLAY && ( nError & EXC_MAJORTYPE ) != EXC_REMOTE )
        {
            Window *pTopWindow = GetTopWindow(); // GCC needs temporary
            WarningBox( pTopWindow, SfxResId(STR_RECOVER_PREPARED) ).Execute();
        }
    }

    // transfer configuration data
    ::utl::ConfigManager::GetConfigManager()->StoreConfigItems();

    // make sure that it is written to disk
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
        xProvider( ::utl::ConfigManager::GetConfigManager()->GetConfigurationProvider(), ::com::sun::star::uno::UNO_QUERY );
    if ( xProvider.is() )
        xProvider->dispose();

    switch( nError & EXC_MAJORTYPE )
    {
        case EXC_USER:
            if( nError == EXC_OUTOFMEMORY )
                Application::Abort( pImp->aMemExceptionString );
            break;

        case EXC_RSCNOTLOADED:
            Application::Abort( pImp->aResExceptionString );
            break;

        case EXC_SYSOBJNOTCREATED:
            Application::Abort( pImp->aSysResExceptionString );
            break;
    }

    pAppData_Impl->bInException = sal_False;
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
    ::rtl::OUString sAppName;

    if ( ::vos::OStartupInfo().getExecutableFile(sAppName) != ::vos::OStartupInfo::E_None )
        sAppName = ::rtl::OUString();

    const AllSettings& rAllSettings = Application::GetSettings();
    LanguageType nType = rAllSettings.GetUILanguage();
    String sTemp( sAppName );
    pRet = new SimpleResMgr( CREATEVERSIONRESMGR_NAME(sfx), nType, &sTemp, 0 );

    return pRet;
}

//--------------------------------------------------------------------

ResMgr* SfxApplication::GetSfxResManager()
{
    if ( !pImp->pSfxResManager )
        pImp->pSfxResManager = CreateResManager("sfx");
    return pImp->pSfxResManager;
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
    SfxProgress *pProgress  /*  zu startender <SfxProgress> oder 0, falls
                                der Progress zurueckgesetzt werden soll */
)

/*  [Beschreibung]

    Interne Methode zum setzen oder zuruecksetzen des Progress-Modes
    fuer die gesamte Applikation.
*/

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
/*
    sal_Bool bMode = sal_True;
    if ( !pWindow || pWindow == GetAppWindow() )
        bMode = sal_False;

    SfxPlugInFrame* pPlug = PTR_CAST( SfxPlugInFrame,
        GetViewFrame()->GetTopViewFrame() );
    if ( pPlug )
        pPlug->GetEnv_Impl()->SetPresentationMode( bMode, pWindow );
    else
        GetAppWindow()->SetPresentationMode( bMode, pWindow );
*/
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
/*    String  aActName;
    String  aResult;
    String  aNationalName = SfxResId(STR_ADDRESS_NAME);
    String  aIniName( "Address" );
    sal_uInt16  nCnt = rList.GetTokenCount( aDel );

    for( sal_uInt16 i=0 ; i<nCnt ; i++ )
    {
        aActName = rList.GetToken( i, aDel );

        if( eConvert == INI2NATIONAL )
        {
            if( aActName == aIniName )
                aResult += aNationalName;
            else
                aResult += aActName;
        }
        else
        {
            if( aActName == aNationalName )
                aResult += aIniName;
            else
                aResult += aActName;
        }

        aResult += aDel;
    }

    aResult.EraseTrailingChars( aDel );

    return aResult;*/

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


SfxStatusBarManager* SfxApplication::GetStatusBarManager() const
{
    if ( !pViewFrame )
        return NULL;

    SfxViewFrame *pTop = pViewFrame;
    while ( pTop->GetParentViewFrame_Impl() )
        pTop = pTop->GetParentViewFrame_Impl();

    return pTop->GetFrame()->GetWorkWindow_Impl()->GetStatusBarManager_Impl();
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

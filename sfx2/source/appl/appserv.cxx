/*************************************************************************
 *
 *  $RCSfile: appserv.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mba $ $Date: 2000-10-23 12:23:17 $
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

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_FRAME_XTASK_HPP_
#include <com/sun/star/frame/XTask.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XTASKSSUPPLIER_HPP_
#include <com/sun/star/frame/XTasksSupplier.hpp>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif

#ifndef _SV_CONFIG_HXX
#include <vcl/config.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_SYSTEM_HXX
#include <vcl/system.hxx>
#endif
#ifndef _SFXCANCEL_HXX //autogen
#include <svtools/cancel.hxx>
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
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif
#ifndef _BASMGR_HXX //autogen
#include <basic/basmgr.hxx>
#endif
#ifndef _BASRDLL_HXX
#include <basic/basrdll.hxx>
#endif
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif

#pragma hdrstop

#include "appimp.hxx"
#include "referers.hxx"
#include "app.hxx"
#include "request.hxx"
#include "dispatch.hxx"
#include "bindings.hxx"
#include "msg.hxx"
#include "objface.hxx"
#include "objitem.hxx"
#include "objsh.hxx"
#include "hintpost.hxx"
#include "stbmgr.hxx"
#include "viewsh.hxx"
#include "docfac.hxx"
#include "docfile.hxx"
#include "docfilt.hxx"
#include "docvor.hxx"
#include "cfg.hxx"
#include "new.hxx"
#include "docinf.hxx"
#include "templdlg.hxx"
#include "inimgr.hxx"
#include "sfxtypes.hxx"
#include "sfxbasic.hxx"
#include "tabdlg.hxx"
#include "arrdecl.hxx"
#include "fltfnc.hxx"
#include "iodlg.hxx"
#include "picklist.hxx"
#include "sfx.hrc"
#include "app.hrc"
#include "tbxcust.hxx"
#include "passwd.hxx"
#include "interno.hxx"
#include "ipenv.hxx"
#include "sfxresid.hxx"
#include "arrdecl.hxx"
#include "childwin.hxx"
#include "appdata.hxx"
#include "tbxconf.hxx"
#include "macrconf.hxx"
#include "minfitem.hxx"
#include "event.hxx"
#include "cfgmgr.hxx"
#include "accmgr.hxx"
#include "mnumgr.hxx"
#include "intfrm.hxx"
#include "urlframe.hxx"
#include "module.hxx"
#include "topfrm.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;

#define SFX_KEY_MULTIQUICKSEARCH    "ExplorerMultiQuickSearch"

//-------------------------------------------------------------------------
long QuitAgain_Impl( void* pObj, void* pArg )
{
    SfxApplication* pApp = (SfxApplication*)pObj;
    Timer* pTimer = (Timer*)pArg;
    delete pTimer;
    pApp->GetDispatcher_Impl()->Execute( SID_QUITAPP, SFX_CALLMODE_ASYNCHRON );
    return 0;
}

struct ApplicationType
{
    String aPathName;
    String aParams;
    String aDomainName;
};

BOOL SfxApplication::InitOfficeAppType_Impl( USHORT nAppId, ApplicationType& rType, BOOL bEmbed )
{
    if ( nAppId < SID_START_BEGIN || nAppId > SID_START_END )
        return FALSE;
    // App in Config suchen
    String aAppName = String( SfxResId( nAppId ) );
    SfxIniManager* pIni = SFX_INIMANAGER();
    DBG_ASSERT( pIni, "Kein IniManager?!" );
    String aFullName;
    while ( pIni && !aFullName.Len() )
    {
        aFullName = pIni->ReadKey( DEFINE_CONST_UNICODE("OfficeApplications"), aAppName );
        if ( !aFullName.Len() )
            pIni = pIni->GetSubManager();
    }

    if ( !aFullName.Len() )
    {
        // not found, but new try
        INetURLObject aTryObj( Application::GetAppFileName(), INET_PROT_FILE );
        aTryObj.setBase( aAppName );
        aFullName = aTryObj.PathToFileName();
    }

    rType.aPathName = aFullName;
    if ( bEmbed )
        rType.aParams = DEFINE_CONST_UNICODE( "/embedding" );
    INetURLObject aObj( aFullName, INET_PROT_FILE );
    rType.aDomainName = aObj.getBase();
    return TRUE;
}

FASTBOOL SfxApplication::PostOfficeAppEvent( USHORT nAppId, const String& rEvent, const String& rParam )
{

//(mba)/task    SfxWaitCursor aWait;

    ApplicationType aType;
    if( !InitOfficeAppType_Impl( nAppId, aType ) )
        return FALSE;

    // Event posten, ggf. App mit ï.ï auf Kommandozeile starten
    ApplicationAddress aAppAdr;
    String aAppParam( rParam.Len() ? DEFINE_CONST_UNICODE(".") : String() );
    SvFactory::IncAliveCount(); // Quit verhindern
    BOOL bOk = FALSE; //! (pb) OldSV: Application::PostAppEvent( aType.aPathName, aAppAdr, rEvent, rParam, TRUE, &aAppParam );
    SvFactory::DecAliveCount();
    return bOk;
}

void SfxApplication::BasicLibExec_Impl( SfxRequest &rReq, BasicManager *pMgr )
{
    // Zuerst den LibName holen
    BOOL bRet = FALSE;
    USHORT nItemId = rReq.GetSlot();
    SFX_REQUEST_ARG( rReq, pNameItem, SfxStringItem, nItemId == SID_ADD_LIBRARY ? SID_FILE_NAME : rReq.GetSlot(), FALSE );

    String aFileName;   // F"ur AddLibrary
    String aLibName;
    if ( pNameItem )
    {
        if ( nItemId == SID_ADD_LIBRARY )
        {
            // Bei AddLibrary ist der LibName optional, er kann mit dem
            // FileName identisch sein
            INetURLObject aObj( pNameItem->GetValue(), INET_PROT_FILE );
            aFileName = aObj.GetMainURL();

            // Nach optionalem LibName suchen
            SFX_REQUEST_ARG( rReq, pItem, SfxStringItem, SID_LOAD_LIBRARY, FALSE );
            if ( pItem )
                aLibName = pItem->GetValue();
            else
                aLibName = aObj.GetBase();
        }
        else
            aLibName = pNameItem->GetValue();
    }

    if ( aLibName.Len() )
    {
        // Die Library zum "ubergebenen Namen suchen
        const International &rInter = Application::GetAppInternational();
        USHORT nLibCount = pMgr->GetLibCount();
        StarBASIC *pLib = NULL;
        USHORT nLib;
        for ( nLib = 0; nLib < nLibCount; ++nLib )
        {
            if ( COMPARE_EQUAL == rInter.Compare( pMgr->GetLibName( nLib ), aLibName, INTN_COMPARE_IGNORECASE ) )
            {
                pLib = pMgr->GetLib( nLib );
                break;
            }
        }

        switch ( rReq.GetSlot() )
        {
            case SID_LOAD_LIBRARY:
            {
                // Wenn eine Library gefunden wurde, die noch nicht
                // geladen ist
                if( nLib<nLibCount && !pLib )
                    bRet = pMgr->LoadLib( nLib );
                break;
            }

            case SID_UNLOAD_LIBRARY:
            {
                // Wenn eine Library gefunden wurde, die geladen ist
                if( pLib )
                    bRet = pMgr->UnloadLib( nLib );
                break;
            }

            case SID_REMOVE_LIBRARY:
            {
                // Wenn eine Library gefunden wurde
                // Lib wird nur physikalisch gel"oscht, wenn sie im
                // Storage des BasicMgr liegt
                if ( nLib<nLibCount )
                    bRet = pMgr->RemoveLib( nLib,
                        !pMgr->IsReference(nLib) && !pMgr->IsExtern(nLib) );
                break;
            }

            case SID_ADD_LIBRARY:
            {
                // Library laden
                SvStorageRef aStor = new SvStorage( aFileName, STREAM_STD_READ );
                if ( aStor->GetError() )
                    break;

                // Weitere Parameter: Art des Ladens der Library
                // ( einbinden, per Referenz oder extern ) und Replace
                BOOL bReplace = TRUE;
                BOOL bReference = FALSE;
                BOOL bExternal = FALSE;

                SFX_REQUEST_ARG( rReq, pRefItem, SfxUInt16Item, SID_ADD_LIBRARY, FALSE );
                if ( pRefItem )
                {
                    bReference = pRefItem->GetValue() == SFX_BASICLIB_ADDMODE_REFERENCE;
                    bExternal = pRefItem->GetValue() == SFX_BASICLIB_ADDMODE_EXTERNAL;
                    if ( pMgr != GetBasicManager() )
                        bExternal = FALSE;
                }

                SFX_REQUEST_ARG( rReq, pReplaceItem, SfxBoolItem, SID_REMOVE_LIBRARY, FALSE );
                if ( pReplaceItem )
                    bReplace = pReplaceItem->GetValue();

                if ( nLib<nLibCount )
                {
                    // Es gibt schon eine Library dieses Namens
                    if ( bReplace && ( pMgr->GetLib( nLib ) == pMgr->GetStdLib() ) )
                        // die Standard-Lib kann nicht ersetzt werden
                        break;

                    if ( bReplace )
                        // Library soll eine vorhandene ersetzen
                        pMgr->RemoveLib( nLib );

                    else if ( bReference )
                        // Referenz nicht moeglich, wenn Lib mit
                        // Namen schon existiert, ausser bei Replace.
                        break;
                }

                // Library einf"ugen
                StarBASIC *pLib = pMgr->AddLib( *aStor, aLibName, bReference );
                if ( pLib )
                {
                    nLib = pMgr->GetLibId( pLib );

                    // Einf"ugen extern
                    if ( bExternal )
                    {
                        INetURLObject aAppBasic( pMgr->GetStorageName(), INET_PROT_FILE );
                        String aExt = aAppBasic.GetExtension();

                        // Optionaler Parameter fuer Name der sbl-Datei
                        String aDest;
                        SFX_REQUEST_ARG( rReq, pDestItem, SfxStringItem, SID_DOCTITLE, FALSE );
                        if ( pDestItem && pDestItem->GetValue().Len() )
                            aDest = pDestItem->GetValue();
                        else
                        {
                            INetURLObject aOld( aFileName, INET_PROT_FILE );
                            aDest = aOld.GetName();
                        }

                        INetURLObject aNew( SFX_INIMANAGER()->Get( SFX_KEY_BASIC_PATH ).GetToken( 0, ';' ), INET_PROT_FILE );
                        aNew.SetExtension( aExt );
                        pMgr->SetLibStorageName( nLib, aNew.GetFull() );
                        SaveBasicManager();
                    }

                    bRet = TRUE;
                }
            }
        }
    }

    rReq.SetReturnValue( SfxBoolItem( 0, bRet ) );
}

void SfxApplication::MiscExec_Impl( SfxRequest& rReq )
{
    DBG_MEMTEST();
    FASTBOOL bDone = FALSE;
    switch ( rReq.GetSlot() )
    {
        case SID_LOAD_LIBRARY:
        case SID_UNLOAD_LIBRARY:
        case SID_REMOVE_LIBRARY:
        case SID_ADD_LIBRARY:
        {
            // Diese Funktionen sind nur f"ur Aufrufe aus dem Basic gedacht
            if ( IsInBasicCall() )
                BasicLibExec_Impl( rReq, GetBasicManager() );
            break;
        }

        case SID_NEXTWINDOW :
        case SID_PREVWINDOW :
        {
            SfxWorkWindow *pWork = GetWorkWindow_Impl();
            pWork->ActivateNextChild_Impl( rReq.GetSlot() == SID_NEXTWINDOW ? TRUE :FALSE );
            rReq.Done();
            break;
        }

        case SID_SETOPTIONS:
        {
            if( rReq.GetArgs() )
                SetOptions_Impl( *rReq.GetArgs() );
            break;
        }

        case SID_QUITAPP:
        case SID_EXITANDRETURN:
        {
            if ( pAppData_Impl->bInQuit )
                return;

            // aus verschachtelten Requests nach 100ms nochmal probieren
            if( Application::GetDispatchLevel() > 1 )
            {
                Timer *pTimer = new Timer;
                pTimer->SetTimeout( 100 );
                pTimer->SetTimeoutHdl( Link( this, QuitAgain_Impl ) );
                pTimer->Start();
                DBG_TRACE1( "QueryExit => FALSE (DispatchLevel == %u)", Application::GetDispatchLevel() );
                return;
            }

            // prepare documents for closing
            Reference < XDesktop > xDesktop ( ::comphelper::getProcessServiceFactory()->createInstance( DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );
            BOOL bQuit = xDesktop->terminate();

            // Returnwert setzten, ggf. terminieren
            rReq.SetReturnValue(SfxBoolItem(rReq.GetSlot(), bQuit));
            return;
        }

        case SID_PICK1:
        case SID_PICK2:
        case SID_PICK3:
        case SID_PICK4:
        case SID_PICK5:
        case SID_PICK6:
        case SID_PICK7:
        case SID_PICK8:
        case SID_PICK9:
        {
            USHORT nPickNo = rReq.GetSlot()-SID_PICK1;
            if ( nPickNo >= SfxPickList_Impl::Get()->GetAllowedMenuSize() )
                break;

            rReq.SetSlot( SID_OPENDOC );
            SfxPickEntry_Impl *pPick = SfxPickList_Impl::Get()->GetMenuPickEntry( nPickNo );
            rReq.AppendItem(SfxStringItem(SID_FILE_NAME, pPick->aName ));
            rReq.AppendItem( SfxStringItem( SID_REFERER, DEFINE_CONST_UNICODE(SFX_REFERER_USER) ) );
            rReq.AppendItem( SfxStringItem( SID_TARGETNAME, DEFINE_CONST_UNICODE("_blank") ) );
            rReq.AppendItem( SfxBoolItem( SID_DOC_READONLY, FALSE ) );
            String aFilter(pPick->aFilter);
            USHORT nPos=aFilter.Search('|');
            if( nPos != STRING_NOTFOUND )
            {
                String aOptions(aFilter.Copy( nPos ).GetBuffer()+1);
                aFilter.Erase( nPos );
                rReq.AppendItem(
                    SfxStringItem(SID_FILE_FILTEROPTIONS, aOptions));
            }

            rReq.AppendItem(SfxStringItem(SID_FILTER_NAME, aFilter));
            ExecuteSlot( rReq );
            return;
        }

        case SID_CONFIG:
        case SID_TOOLBOXOPTIONS:
        case SID_CONFIGSTATUSBAR:
        case SID_CONFIGMENU:
        case SID_CONFIGACCEL:
        case SID_CONFIGEVENT:
        {
            Window *pParent = GetTopWindow();
            const SfxStringItem *pStringItem=0;
            const SfxUInt16Item *pItem=0;
            USHORT nId=0;

            // Versuche, einen "ubergebenen ConfigNamen zu holen
            const SfxItemSet *pArgs = rReq.GetArgs();
            if ( pArgs && pArgs->GetItemState(SID_CFGFILE) >= SFX_ITEM_AVAILABLE )
            {
                const SfxPoolItem *pSfxItem = &pArgs->Get(SID_CFGFILE);
                DBG_ASSERT(pSfxItem->ISA(SfxStringItem),"Fehlerhafte Parameter!");
                pStringItem = (const SfxStringItem*) pSfxItem;

            }

            // Versuche, eine "ubergebene ConfigID zu holen
            if (pArgs && pArgs->GetItemState(SID_CONFIGITEMID) >= SFX_ITEM_AVAILABLE)
            {
                const SfxPoolItem *pSfxItem = &pArgs->Get(SID_CONFIGITEMID);
                DBG_ASSERT(pSfxItem->ISA(SfxUInt16Item),"Fehlerhafte Parameter!");
                pItem = (const SfxUInt16Item*) pSfxItem;
                nId = pItem->GetValue();
                bDone = TRUE;
            }
            else
            {
                switch ( rReq.GetSlot() )
                {
                    case SID_CONFIGSTATUSBAR:
                        if ( GetStatusBarManager() )
                            nId = GetStatusBarManager()->GetType();
                        break;
                    case SID_CONFIGMENU:
                        nId = GetMenuBarManager()->GetType();
                        break;
                    case SID_CONFIGACCEL:
                        nId = GetAcceleratorManager()->GetType();
                        break;
                    case SID_TOOLBOXOPTIONS:
                        nId = SfxToolBoxConfig::GetOrCreate()->GetType();
                        break;
                    default:
                        break;
                }
            }

            if ( pStringItem && nId )
            {
                // Ausf"uhren ohne Dialog
                SfxConfigManager *pCfgMgr = new SfxConfigManager(pStringItem->GetValue());
                pAppData_Impl->pAppCfg->CopyItem(nId, pCfgMgr);
                GetDispatcher_Impl()->Update_Impl(TRUE);
                if ( nId == GetMenuBarManager()->GetType() || nId == SID_CONFIG )
                    GetMenuBarManager()->ReconfigureObjectMenus();
            }
            else
            {
                SfxItemSet aSet( GetPool(), SID_ATTR_MACROITEM, SID_ATTR_MACROITEM );
                SfxConfigDialog *pDlg = new SfxConfigDialog( pParent, &aSet );

                switch ( rReq.GetSlot() )
                {
                    case SID_CONFIG:
                    {
                        // Soll ein Macro vorselektiert werden ?
                        if (pArgs && pArgs->GetItemState(SID_MACROINFO) >= SFX_ITEM_AVAILABLE)
                        {
                            const SfxPoolItem *pSfxItem = &pArgs->Get(SID_MACROINFO);
                            DBG_ASSERT(pSfxItem->ISA(SfxMacroInfoItem),"Fehlerhafte Parameter!");
                            const SfxMacroInfoItem *pMItem = (const SfxMacroInfoItem*) pSfxItem;
                            if (pMItem)
                                pDlg->ActivateMacroConfig(pMItem);
                        }
                        break;
                    }

                    case SID_TOOLBOXOPTIONS:
                    {
                        if ( nId )
                            // Es soll eine Objektleiste vorselektiert werden
                            pDlg->ActivateToolBoxConfig(nId);
                    }
                }

                const short nRet = pDlg->Execute();
                GetDispatcher_Impl()->Update_Impl(TRUE);

                if (nRet)
                    bDone = TRUE;

                if (nRet == 3)
                {
                    pViewFrame->SetChildWindow( SfxToolboxCustomWindow::GetChildWindowId(), TRUE );
                    Invalidate(rReq.GetSlot());
                }
                delete pDlg;
            }
            break;
        }

        case SID_SAVECONFIG:
        {
            SfxConfigManager *pMgr = 0;
            BOOL bCreated = TRUE;
            String aCfgName;

            // Versuche, einen "ubergebenen ConfigNamen zu holen
            const SfxItemSet *pArgs = rReq.GetArgs();
            if ( !rReq.IsAPI() || pArgs )
            {
                if ( !pArgs )
                {
                    // Kein Parameter, kein API, also Dialog
                    aCfgName = SfxConfigDialog::FileDialog_Impl(
                        GetTopWindow(), WB_SAVEAS | WB_STDMODAL | WB_3DLOOK, String() );
                }
                else
                {
                    // Name "uber Parameter
                    const SfxPoolItem *pSfxItem = &pArgs->Get( SID_CFGFILE );
                    DBG_ASSERT( pSfxItem && pSfxItem->ISA(SfxStringItem), "Fehlerhafte Parameter!" );
                    const SfxStringItem *pStringItem = (const SfxStringItem*) pSfxItem;
                    if ( pStringItem )
                    {
                        aCfgName = pStringItem->GetValue();
                        INetURLObject aObj( pStringItem->GetValue(), INET_PROT_FILE );
                        if ( aObj.HasError() )
                        {
                            // Wenn relativ, ConfigDir verwenden
                            aObj.SetSmartURL( SFX_INIMANAGER()->Get( SFX_KEY_USERCONFIG_PATH) );
                            aObj.insertName( pStringItem->GetValue() );
                            aCfgName = aObj.PathToFileName();
                        }
                    }
                }
            }
            else
            {
                pAppData_Impl->pAppCfg->Backup();
            }

            if ( aCfgName.Len() )
            {
                // ConfigManager anlegen
//(mba)/task                SfxWaitCursor aWait;
                BOOL bCreated = FALSE;
                pMgr = SfxConfigDialog::MakeCfgMgr_Impl( aCfgName, bCreated );

                // Wenn es nicht der globale ConfigManager ist, kopieren
                if ( pMgr && pAppData_Impl->pAppCfg != pMgr )
                    pMgr->CopyItems( pAppData_Impl->pAppCfg );

                // Dann abspeichern
                if ( !pMgr->SaveConfig() )
                    HandleConfigError_Impl( (USHORT)pMgr->GetErrorCode() );
                if ( bCreated )
                    delete pMgr;
            }

            bDone = TRUE;
            break;
        }

        case SID_LOADCONFIG:
        {
            SfxConfigManager *pMgr = 0;
            BOOL bCreated = TRUE;
            String aCfgName;

            // Versuche, einen "ubergebenen ConfigNamen zu holen
            const SfxItemSet *pArgs = rReq.GetArgs();
            if ( !rReq.IsAPI() || pArgs )
            {
                if ( !pArgs )
                {
                    // Kein Parameter, kein API, also Dialog
                    aCfgName = SfxConfigDialog::FileDialog_Impl(
                        GetTopWindow(), WB_OPEN | WB_STDMODAL | WB_3DLOOK, String() );
                }
                else
                {
                    // Name "uber Parameter
                    const SfxPoolItem *pSfxItem = &pArgs->Get( SID_CFGFILE );
                    DBG_ASSERT( pSfxItem && pSfxItem->ISA(SfxStringItem), "Fehlerhafte Parameter!" );
                    const SfxStringItem *pStringItem = (const SfxStringItem*) pSfxItem;
                    if ( pStringItem )
                    {
                        aCfgName = pStringItem->GetValue();
                        INetURLObject aObj( pStringItem->GetValue(), INET_PROT_FILE );
                        if ( aObj.HasError() )
                        {
                            // Wenn relativ, ConfigDir verwenden
                            aObj.SetSmartURL( SFX_INIMANAGER()->Get( SFX_KEY_USERCONFIG_PATH) );
                            aObj.insertName( pStringItem->GetValue() );
                            aCfgName = aObj.PathToFileName();
                        }
                    }
                }
            }
            else
            {
                // Aus dem aktuellen Backup laden
                pAppData_Impl->pAppCfg->Restore();
            }

            if ( aCfgName.Len() )
            {
                // ConfigManager anlegen
//(mba)/task                SfxWaitCursor aWait;
                BOOL bCreated = FALSE;
                pMgr = SfxConfigDialog::MakeCfgMgr_Impl( aCfgName, bCreated );

                // Wenn es nicht der globale ConfigManager ist, kopieren
                if ( pMgr && pAppData_Impl->pAppCfg != pMgr )
                    pAppData_Impl->pAppCfg->CopyItems( pMgr );

                if ( bCreated )
                    delete pMgr;
            }

            GetDispatcher_Impl()->Update_Impl(TRUE);
            bDone = TRUE;
            break;
        }

        case SID_CLOSEDOCS:
        case SID_CLOSEWINS:
        {

            Reference < XTasksSupplier > xDesktop ( ::comphelper::getProcessServiceFactory()->createInstance( DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );
            Reference< XIndexAccess > xTasks( xDesktop->getTasks(), UNO_QUERY );
            if ( !xTasks.is() )
                break;

            sal_Int32 n=0;
            do
            {
                if ( xTasks->getCount() <= n )
                    break;

                Any aAny = xTasks->getByIndex(n);
                Reference < XTask > xTask;
                aAny >>= xTask;
                if ( !xTask->close() )
                    n++;
            }
            while( sal_True );

            BOOL bOk = ( n == 0);
            rReq.SetReturnValue( SfxBoolItem( 0, bOk ) );
            bDone = TRUE;
            break;
        }

        case SID_SAVEDOCS:
        {
            BOOL bOK = TRUE;
            BOOL bDone = TRUE;
            for ( SfxObjectShell *pObjSh = SfxObjectShell::GetFirst();
                  pObjSh;
                  pObjSh = SfxObjectShell::GetNext( *pObjSh ) )
            {
                SfxRequest aReq( SID_SAVEDOC, 0, pObjSh->GetPool() );
                if ( pObjSh->IsModified() )
                {
                    pObjSh->ExecuteSlot( aReq );
                    SfxBoolItem *pItem = PTR_CAST( SfxBoolItem, aReq.GetReturnValue() );
                    bDone = aReq.IsDone();
                    if ( !pItem || !pItem->GetValue() )
                        bOK = FALSE;
                }
            }

            rReq.SetReturnValue( SfxBoolItem( 0, bOK ) );
            rReq.Done();
            break;
        }

        case SID_HELPINDEX:
        {
            SfxViewFrame::Current()->SetChildWindow( SID_HELP_PI, TRUE );
            break;
        }

/*! (pb) what about help?
        case SID_HELPINDEX:
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
            {
                pHelp->Start( HELP_INDEX );
                bDone = TRUE;
            }
            break;
        }

        case SID_EXTENDEDHELP:
        {
            // Parameter auswerten
            SFX_REQUEST_ARG(rReq, pOnItem, SfxBoolItem, SID_EXTENDEDHELP, FALSE);
            FASTBOOL bOn = pOnItem
                            ? ((SfxBoolItem*)pOnItem)->GetValue()
                            : !Help::IsExtHelpActive();

            // ausf"uhren
            if ( bOn )
                Help::StartExtHelp();
            else
                Help::EndExtHelp();
            bDone = TRUE;

            // ggf. recorden
            if ( !rReq.IsAPI() )
                rReq.AppendItem( SfxBoolItem( SID_EXTENDEDHELP, bOn) );
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_HELPBALLOONS:
        {
            // Parameter auswerten
            SFX_REQUEST_ARG(rReq, pOnItem, SfxBoolItem, SID_HELPBALLOONS, FALSE);
            FASTBOOL bOn = pOnItem
                            ? ((SfxBoolItem*)pOnItem)->GetValue()
                            : !Help::IsBalloonHelpEnabled();

            // ausf"uhren
            if ( bOn )
                Help::EnableBalloonHelp();
            else
                Help::DisableBalloonHelp();
            SvtHelpOptions().SetHelpBalloons(bOn);
            bDone = TRUE;

            // ggf. recorden
            if ( !rReq.IsAPI() )
                rReq.AppendItem( SfxBoolItem( SID_HELPBALLOONS, bOn) );
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_HELPTIPS:
        {
            // Parameter aus werten
            SFX_REQUEST_ARG(rReq, pOnItem, SfxBoolItem, SID_HELPTIPS, FALSE);
            FASTBOOL bOn = pOnItem
                            ? ((SfxBoolItem*)pOnItem)->GetValue()
                            : !Help::IsQuickHelpEnabled();

            // ausf"uhren
            if ( bOn )
                Help::EnableQuickHelp();
            else
                Help::DisableQuickHelp();
            SvtHelpOptions().SetHelpTips(bOn);
            Invalidate(SID_HELPTIPS);
            bDone = TRUE;

            // ggf. recorden
            if ( !rReq.IsAPI() )
                rReq.AppendItem( SfxBoolItem( SID_HELPTIPS, bOn) );
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_HELPONHELP:
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
            {
                pHelp->Start( HELP_HELPONHELP );
                bDone = TRUE;
            }
            break;
        }
(pb) what about help? */

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_ABOUT:
        {
            ModalDialog *pDlg = CreateAboutDialog();
            pDlg->Execute();
            delete pDlg;
            bDone = TRUE;
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_ORGANIZER:
        {
            SfxTemplateOrganizeDlg  *pDlg =
                new SfxTemplateOrganizeDlg(GetTopWindow());
            pDlg->Execute();
            delete pDlg;
            bDone = TRUE;
            break;
        }

        case SID_BASICSTOP:
            StarBASIC::Stop();
            break;

        case SID_BASICBREAK :
            BASIC_DLL()->BasicBreak();
            break;

        case SID_CRASH :
        {
            Exception( EXC_SYSTEM );
            abort();
        }

        default:
            break;
    }

    if ( bDone )
        rReq.Done();
}

//--------------------------------------------------------------------

void SfxApplication::MiscState_Impl(SfxItemSet &rSet)
{
    DBG_MEMTEST();

    int bSearchedMDI = FALSE;
    int bFoundNormMDI = FALSE;
    int bFoundMiniMDI = FALSE;
    int bFoundNonDesktopMDI = FALSE;

    const USHORT *pRanges = rSet.GetRanges();
    DBG_ASSERT(pRanges && *pRanges, "Set ohne Bereich");
    while ( *pRanges )
    {
        for(USHORT nWhich = *pRanges++; nWhich <= *pRanges; ++nWhich)
        {
            switch(nWhich)
            {
                case SID_EXITANDRETURN:
                case SID_QUITAPP:
                {
                    if ( pAppData_Impl->nDocModalMode )
                        rSet.DisableItem(nWhich);
                    else
                    {
                        SfxObjectShell *pDoc = pViewFrame ? pViewFrame->GetObjectShell() : 0;
                        SfxInPlaceObject *pIPObj = pDoc ? pDoc->GetInPlaceObject() : 0;
                        if (pIPObj && pIPObj->GetProtocol().IsEmbed() &&
                            !pIPObj->GetClient()->Owner())
                        {
                            String aEntry(SfxResId(STR_EXITANDRETURN));
                            aEntry += pIPObj->GetDocumentName();
                            rSet.Put(SfxStringItem(nWhich, aEntry));
                        }
                        else
                            rSet.Put(SfxStringItem(nWhich, String(SfxResId(STR_QUITAPP))));
                    }
                    break;
                }

                case SID_BASICSTOP:
                    if ( !StarBASIC::IsRunning() )
                        rSet.DisableItem(nWhich);
                    break;

                case SID_PICK1:
                case SID_PICK2:
                case SID_PICK3:
                case SID_PICK4:
                case SID_PICK5:
                case SID_PICK6:
                case SID_PICK7:
                case SID_PICK8:
                case SID_PICK9: {
                    if ( ( nWhich - SID_PICK1 ) <
                           (USHORT)SfxPickList_Impl::Get()->GetAllowedMenuSize() )
                        rSet.Put( SfxStringItem( nWhich,
                            SfxPickList_Impl::Get()->GetMenuPickEntry(nWhich - SID_PICK1)->aTitle ) );
                    break;
                }

                case SID_CURRENTTIME:
                {
                    rSet.Put( SfxStringItem( nWhich, Application::GetAppInternational().GetTime( Time(), FALSE ) ) );
                    break;
                }
                case SID_CURRENTDATE:
                {
                    rSet.Put( SfxStringItem( nWhich, Application::GetAppInternational().GetDate( Date() ) ) );
                    break;
                }

                case SID_CONFIGTOOLBOX:
                    break;

                case SID_HELPTIPS:
                {
/*! (pb) what about help?
                    rSet.Put( SfxBoolItem( SID_HELPTIPS, Help::IsQuickHelpEnabled() ) );
*/
                    rSet.DisableItem( SID_HELPTIPS );
                }
                break;
                case SID_HELPBALLOONS:
                {
/*! (pb) what about help?
                    short nHelpMode = ImplGetHelpMode();
                    if ( ( nHelpMode & HELPTEXTMODE_NORESHELPTEXT ) && !( nHelpMode & HELPTEXTMODE_EXTERN ) )
                    {
                        rSet.DisableItem( SID_HELPBALLOONS );
                    }
                    else
                        rSet.Put( SfxBoolItem( SID_HELPBALLOONS, Help::IsBalloonHelpEnabled() ) );
*/
                    rSet.DisableItem( SID_HELPBALLOONS );
                }
                break;
                case SID_EXTENDEDHELP:
                {
/*! (pb) what about help?
                    BOOL bExtHelp = Help::IsExtHelpActive();
                    // Wenn weder aus Resource noch aus Hilfesystem:
                    short nHelpMode = ImplGetHelpMode();
                    if ( ( nHelpMode & HELPTEXTMODE_NORESHELPTEXT ) &&
                        !( nHelpMode & HELPTEXTMODE_EXTERN ) )
                        bExtHelp = FALSE;
                    rSet.Put( SfxBoolItem( SID_EXTENDEDHELP, bExtHelp ) );
*/
                }
                break;

                case SID_CLOSEDOCS:
                case SID_CLOSEWINS:
                {
                    Reference < XTasksSupplier > xDesktop ( ::comphelper::getProcessServiceFactory()->createInstance( DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );
                    Reference< XIndexAccess > xTasks( xDesktop->getTasks(), UNO_QUERY );
                    if ( !xTasks.is() || !xTasks->getCount() )
                        rSet.DisableItem(nWhich);
                    break;
                }

                case SID_SAVEDOCS:
                {
                    BOOL bModified = FALSE;
                    for ( SfxObjectShell *pObjSh = SfxObjectShell::GetFirst();
                          pObjSh;
                          pObjSh = SfxObjectShell::GetNext( *pObjSh ) )
                    {
                        if ( pObjSh->IsModified() )
                        {
                            bModified = TRUE;
                            break;
                        }
                    }

                    if ( !bModified )
                        rSet.DisableItem( nWhich );
                    break;
                }

                default:
                    break;
            }
        }

        ++pRanges;
    }
}



/*************************************************************************
 *
 *  $RCSfile: appserv.cxx,v $
 *
 *  $Revision: 1.40 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 13:53:19 $
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

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMultiServiceFactory_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_DISPATCHRESULTEVENT_HPP_
#include <com/sun/star/frame/DispatchResultEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_DISPATCHRESULTSTATE_HPP_
#include <com/sun/star/frame/DispatchResultState.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XJOBEXECUTOR_HPP_
#include <com/sun/star/task/XJobExecutor.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHHELPER_HPP_
#include <com/sun/star/frame/XDispatchHelper.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_CloseVetoException_HPP_
#include <com/sun/star/util/CloseVetoException.hpp>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#ifndef _SVT_DOC_ADDRESSTEMPLATE_HXX_
#include <svtools/addresstemplate.hxx>
#endif
#ifndef _UNOTOOLS_INTLWRAPPER_HXX
#include <unotools/intlwrapper.hxx>
#endif

#ifndef _UNOTOOLS_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _CONFIG_HXX
#include <tools/config.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
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
#ifndef _SV_HELP_HXX
#include <vcl/help.hxx>
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif

#include <svtools/pathoptions.hxx>
#include <svtools/moduleoptions.hxx>
#include <svtools/regoptions.hxx>
#include <svtools/helpopt.hxx>

#include <drafts/com/sun/star/script/provider/XScriptProviderFactory.hpp>
#include <drafts/com/sun/star/script/provider/ScriptFrameworkErrorException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

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
#if SUPD<613//MUSTINI
#include "inimgr.hxx"
#endif
#include "sfxtypes.hxx"
#include "sfxbasic.hxx"
#include "tabdlg.hxx"
#include "arrdecl.hxx"
#include "fltfnc.hxx"
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
#include "sfxpicklist.hxx"
#include "imestatuswindow.hxx"
#include "sfxdlg.hxx"
#include "dialogs.hrc"
#include "sorgitm.hxx"

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::drafts::com::sun::star::script;

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
            aFileName = aObj.GetMainURL( INetURLObject::NO_DECODE );

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
        IntlWrapper aIntlWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
        const CollatorWrapper* pCollator = aIntlWrapper.getCollator();
        USHORT nLibCount = pMgr->GetLibCount();
        StarBASIC *pLib = NULL;
        USHORT nLib;
        for ( nLib = 0; nLib < nLibCount; ++nLib )
        {
            if ( COMPARE_EQUAL == pCollator->compareString( pMgr->GetLibName( nLib ), aLibName ) )
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

                        INetURLObject aNew( SvtPathOptions().GetBasicPath().GetToken( 0, ';' ), INET_PROT_FILE );
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
        case SID_UPDATE_CONFIG:
        {
            SFX_REQUEST_ARG( rReq, pItem, SfxStringItem, SID_UPDATE_CONFIG, FALSE );
            if ( pItem )
                GetConfigManager_Impl()->ReInitialize( pItem->GetValue() );
            break;
        }

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

        case SID_SETOPTIONS:
        {
            if( rReq.GetArgs() )
                SetOptions_Impl( *rReq.GetArgs() );
            break;
        }

        case SID_QUITAPP:
        case SID_EXITANDRETURN:
        case SID_LOGOUT:
        {
            // protect against reentrant calls
            if ( pAppData_Impl->bInQuit )
                return;

            if ( rReq.GetSlot() == SID_LOGOUT )
            {
                for ( SfxObjectShell *pObjSh = SfxObjectShell::GetFirst();
                    pObjSh; pObjSh = SfxObjectShell::GetNext( *pObjSh ) )
                {
                    if ( !pObjSh->IsModified() )
                        continue;

                    SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pObjSh );
                    if ( !pFrame || !pFrame->GetWindow().IsReallyVisible() )
                        continue;

                    if ( pObjSh->PrepareClose(2) )
                        pObjSh->SetModified( FALSE );
                    else
                        return;
                }

                String aName = String::CreateFromAscii("vnd.sun.star.cmd:logout");
                SfxStringItem aNameItem( SID_FILE_NAME, aName );
                SfxStringItem aReferer( SID_REFERER, DEFINE_CONST_UNICODE( "private/user" ) );
                pAppDispat->Execute( SID_OPENDOC, SFX_CALLMODE_SLOT, &aNameItem, &aReferer, 0L );
                return;
            }

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

            // block reentrant calls
            pAppData_Impl->bInQuit = TRUE;
            Reference < XDesktop > xDesktop ( ::comphelper::getProcessServiceFactory()->createInstance( DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );

            // if terminate() failed, pAppData_Impl->bInQuit will now be FALSE, allowing further calls of SID_QUITAPP
            BOOL bTerminated = xDesktop->terminate();
            if (!bTerminated)
                // if terminate() was successful, SfxApplication is now dead!
                pAppData_Impl->bInQuit = FALSE;

            // Returnwert setzten, ggf. terminieren
            rReq.SetReturnValue( SfxBoolItem( rReq.GetSlot(), bTerminated ) );
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
            SfxPickList::Get()->ExecuteEntry( rReq.GetSlot() - SID_PICK1 );
/*
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
*/
            return;
        }

        case SID_CONFIG:
        case SID_TOOLBOXOPTIONS:
        case SID_CONFIGSTATUSBAR:
        case SID_CONFIGMENU:
        case SID_CONFIGACCEL:
        case SID_CONFIGEVENT:
        {
            // Check configuration to see whether new Configure dialog
            // should be shown.
            Any value;
            sal_Bool tmp;

            value = ::utl::ConfigManager::GetConfigManager()->GetLocalProperty(
                ::rtl::OUString::createFromAscii(
            "Office.Scripting/ScriptDisplaySettings/UseNewToolsConfigure" ) );

            value >>= tmp;

            if (tmp == sal_True) {
                SfxAbstractDialogFactory* pFact =
                    SfxAbstractDialogFactory::Create();

                if ( pFact )
                {
                    SFX_REQUEST_ARG(rReq, pStringItem,
                        SfxStringItem, SID_CONFIG, sal_False);

                    SfxItemSet aSet(
                        GetPool(), SID_CONFIG, SID_CONFIG );

                    if ( pStringItem )
                    {
                        aSet.Put( SfxStringItem(
                            SID_CONFIG, pStringItem->GetValue() ) );
                    }

                    SfxAbstractTabDialog* pDlg = pFact->CreateTabDialog(
                        ResId( RID_SVXDLG_CUSTOMIZE ),
                        NULL, &aSet, pViewFrame );

                      if ( pDlg )
                    {
                        const short nRet = pDlg->Execute();

                        for( SfxViewFrame* pFrame = SfxViewFrame::GetFirst();
                              pFrame;
                              pFrame = SfxViewFrame::GetNext( *pFrame ) )
                        {
                            pFrame->GetDispatcher()->Update_Impl( TRUE );
                        }

                        if ( nRet )
                            bDone = TRUE;

                        delete pDlg;
                    }
                }
            }
            else
            {
            SfxItemSet aSet( GetPool(), SID_ATTR_MACROITEM, SID_ATTR_MACROITEM );
            SfxConfigDialog *pDlg = new SfxConfigDialog( NULL, &aSet, pViewFrame );

            switch ( rReq.GetSlot() )
            {
                case SID_TOOLBOXOPTIONS:
                {
                    // Versuche, eine "ubergebene ConfigID zu holen
                    SFX_REQUEST_ARG( rReq, pItem, SfxUInt16Item, SID_CONFIGITEMID, FALSE );
                    if ( pItem )
                    {
                        bDone = TRUE;
                        USHORT nId = pItem->GetValue();
                        if ( nId )
                            // Es soll eine Objektleiste vorselektiert werden
                            pDlg->ActivateToolBoxConfig(nId);
                    }
                    break;
                }

                default:
                {
                    // Soll ein Macro vorselektiert werden ?
                    SFX_REQUEST_ARG( rReq, pMacroItem, SfxMacroInfoItem, SID_MACROINFO, FALSE );
                    if ( pMacroItem )
                        pDlg->ActivateMacroConfig( pMacroItem );
                    pDlg->ActivateTabPage( rReq.GetSlot() );
                    break;
                }
            }

            const short nRet = pDlg->Execute();
//          DBG_ERROR("Notify is missing!");

            for( SfxViewFrame* pFrame = SfxViewFrame::GetFirst(); pFrame; pFrame = SfxViewFrame::GetNext( *pFrame ) )
                pFrame->GetDispatcher()->Update_Impl( TRUE );

            if ( nRet )
                bDone = TRUE;

            if ( nRet == 3 )
            {
                pViewFrame->SetChildWindow( SfxToolboxCustomWindow::GetChildWindowId(), TRUE );
                Invalidate(rReq.GetSlot());
            }

            delete pDlg;
            }

            break;
        }

        case SID_CLOSEDOCS:
        case SID_CLOSEWINS:
        {

            Reference < XFramesSupplier > xDesktop ( ::comphelper::getProcessServiceFactory()->createInstance( DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );
            Reference< XIndexAccess > xTasks( xDesktop->getFrames(), UNO_QUERY );
            if ( !xTasks.is() )
                break;

            sal_Int32 n=0;
            do
            {
                if ( xTasks->getCount() <= n )
                    break;

                Any aAny = xTasks->getByIndex(n);
                Reference < XCloseable > xTask;
                aAny >>= xTask;
                try
                {
                    xTask->close(sal_True);
                    n++;
                }
                catch( CloseVetoException& )
                {
                }
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

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_HELPINDEX:
        case SID_HELP_SUPPORTPAGE:
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
            {
                ULONG nHelpId = ( rReq.GetSlot() == SID_HELP_SUPPORTPAGE ) ? 66056 : 0;
                pHelp->Start( nHelpId, NULL ); // show start or support page
                bDone = TRUE;
            }
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
            SvtHelpOptions().SetHelpTips( bOn );
            Invalidate(SID_HELPTIPS);
            bDone = TRUE;

            // ggf. recorden
            if ( !rReq.IsAPI() )
                rReq.AppendItem( SfxBoolItem( SID_HELPTIPS, bOn) );
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
            SvtHelpOptions().SetExtendedHelp( bOn );
            Invalidate(SID_HELPBALLOONS);
            bDone = TRUE;

            // ggf. recorden
            if ( !rReq.IsAPI() )
                rReq.AppendItem( SfxBoolItem( SID_HELPBALLOONS, bOn) );
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_HELP_PI:
        {
            SvtHelpOptions aHelpOpt;
            SFX_REQUEST_ARG(rReq, pOnItem, SfxBoolItem, SID_HELP_PI, FALSE);
            sal_Bool bOn = pOnItem
                            ? ((SfxBoolItem*)pOnItem)->GetValue()
                            : !aHelpOpt.IsHelpAgentAutoStartMode();
            aHelpOpt.SetHelpAgentAutoStartMode( bOn );
            Invalidate(SID_HELP_PI);
            bDone = TRUE;
            break;
        }

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
                new SfxTemplateOrganizeDlg(NULL);
            pDlg->Execute();
            delete pDlg;
            bDone = TRUE;
            break;
        }

        case SID_TEMPLATE_ADDRESSBOKSOURCE:
        {
            svt::AddressBookSourceDialog aDialog(GetTopWindow(), ::comphelper::getProcessServiceFactory());
            aDialog.Execute();
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
            GetpApp()->Exception( EXC_SYSTEM );
            abort();
        }

        case SID_SHOW_IME_STATUS_WINDOW:
            if (pAppData_Impl->m_xImeStatusWindow->canToggle())
            {
                SfxBoolItem const * pItem = static_cast< SfxBoolItem const * >(
                    rReq.GetArg(SID_SHOW_IME_STATUS_WINDOW, false,
                                TYPE(SfxBoolItem)));
                bool bShow = pItem == 0
                    ? !pAppData_Impl->m_xImeStatusWindow->isShowing()
                    : ( pItem->GetValue() == TRUE );
                pAppData_Impl->m_xImeStatusWindow->show(bShow);
                if (pItem == 0)
                    rReq.AppendItem(SfxBoolItem(SID_SHOW_IME_STATUS_WINDOW,
                                                bShow));
            }
            bDone = true;
            break;

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

    LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
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
                case SID_PICK9:
                {
                    SfxPickList* pPickList = SfxPickList::Get();
                    if (( nWhich - SID_PICK1 ) < (USHORT)pPickList->GetAllowedMenuSize() )
                    {
                        String aTitle = pPickList->GetMenuEntryTitle( nWhich - SID_PICK1 );
                        rSet.Put( SfxStringItem( nWhich, aTitle ));
                    }
                    break;
                }

                case SID_CURRENTTIME:
                {
                    rSet.Put( SfxStringItem( nWhich, aLocaleWrapper.getTime( Time(), FALSE ) ) );
                    break;
                }
                case SID_CURRENTDATE:
                {
                    rSet.Put( SfxStringItem( nWhich, aLocaleWrapper.getDate( Date() ) ) );
                    break;
                }

                case SID_HELPTIPS:
                {
                    rSet.Put( SfxBoolItem( SID_HELPTIPS, Help::IsQuickHelpEnabled() ) );
                }
                break;
                case SID_HELPBALLOONS:
                {
                    rSet.Put( SfxBoolItem( SID_HELPBALLOONS, Help::IsBalloonHelpEnabled() ) );
                }
                break;
                case SID_HELP_PI:
                {
                    rSet.Put( SfxBoolItem( SID_HELP_PI, SvtHelpOptions().IsHelpAgentAutoStartMode() ) );
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
                    Reference < XFramesSupplier > xDesktop ( ::comphelper::getProcessServiceFactory()->createInstance( DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );
                    Reference< XIndexAccess > xTasks( xDesktop->getFrames(), UNO_QUERY );
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

                case SID_SHOW_IME_STATUS_WINDOW:
                    if (pAppData_Impl->m_xImeStatusWindow->canToggle())
                        rSet.Put(SfxBoolItem(
                                     SID_SHOW_IME_STATUS_WINDOW,
                                     pAppData_Impl->m_xImeStatusWindow->
                                         isShowing()));
                    else
                        rSet.DisableItem(SID_SHOW_IME_STATUS_WINDOW);
                    break;

                default:
                    break;
            }
        }

        ++pRanges;
    }
}

static const ::rtl::OUString& getProductRegistrationServiceName( )
{
    static ::rtl::OUString s_sServiceName = ::rtl::OUString::createFromAscii( "com.sun.star.setup.ProductRegistration" );
    return s_sServiceName;
}

typedef rtl_uString* (SAL_CALL *basicide_choose_macro)(BOOL, BOOL, rtl_uString*);
typedef void (SAL_CALL *basicide_macro_organizer)( INT16 );

#define DOSTRING( x )                       #x
#define STRING( x )                         DOSTRING( x )

::rtl::OUString ChooseMacro( BOOL bExecute, BOOL bChooseOnly, const ::rtl::OUString& rMacroDesc = ::rtl::OUString() )
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

void MacroOrganizer( INT16 nTabId )
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

    // call basicide_macro_organizer in basctl
    pSymbol( nTabId );
}

#define RID_ERRBOX_MODULENOTINSTALLED     (RID_OFA_START + 72)

ResMgr* SfxApplication::GetOffResManager_Impl()
{
    if ( !pImp->pOfaResMgr )
        pImp->pOfaResMgr = CreateResManager( "ofa");
    return pImp->pOfaResMgr;
}

void SfxApplication::OfaExec_Impl( SfxRequest& rReq )
{
    DBG_MEMTEST();
    FASTBOOL bDone = FALSE;
    switch ( rReq.GetSlot() )
    {
        case SID_OPTIONS_TREEDIALOG:
        {
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            if ( pFact )
            {
                  VclAbstractDialog* pDlg = pFact->CreateVclDialog( NULL, ResId( rReq.GetSlot() ) );
                  pDlg->Execute();
                  delete pDlg;
            }
            break;
        }

        case SID_ONLINE_REGISTRATION:
        {
            try
            {
                // create the ProductRegistration component
                Reference< com::sun::star::lang::XMultiServiceFactory > xORB( ::comphelper::getProcessServiceFactory() );
                Reference< com::sun::star::task::XJobExecutor > xProductRegistration;
                if ( xORB.is() )
                    xProductRegistration = xProductRegistration.query( xORB->createInstance( getProductRegistrationServiceName() ) );
                DBG_ASSERT( xProductRegistration.is(), "OfficeApplication::ExecuteApp_Impl: could not create the service!" );

                // tell it that the user wants to register
                if ( xProductRegistration.is() )
                {
                    xProductRegistration->trigger( ::rtl::OUString::createFromAscii( "RegistrationRequired" ) );
                }
            }
            catch( const ::com::sun::star::uno::Exception& )
            {
                DBG_ERROR( "OfficeApplication::ExecuteApp_Impl(SID_ONLINE_REGISTRATION): caught an exception!" );
            }
        }
        break;

        case SID_BASICIDE_APPEAR:
        {
            SfxViewFrame* pView = SfxViewFrame::GetFirst();
            ::rtl::OUString aBasicName = ::rtl::OUString::createFromAscii("com.sun.star.comp.basic.BasicIDE");
            while ( pView )
            {
                if ( pView->GetObjectShell()->GetFactory().GetDocumentServiceName() == aBasicName )
                    break;
                pView = SfxViewFrame::GetNext( *pView );
            }

            if ( !pView )
            {
                SfxObjectShell* pDocShell = SfxObjectShell::CreateObject( aBasicName );
                pDocShell->DoInitNew( 0 );
                pDocShell->SetModified( FALSE );
                pView = SFX_APP()->CreateViewFrame( *pDocShell, 0 );
                pView->SetName( String( RTL_CONSTASCII_USTRINGPARAM( "BASIC:1" ) ) );
            }
            else
                pView->GetFrame()->Appear();

            const SfxItemSet* pArgs = rReq.GetArgs();
            if ( pArgs && pView )
            {
                SfxViewShell* pViewShell = pView->GetViewShell();
                SfxObjectShell* pObjShell = pView->GetObjectShell();
                if ( pViewShell && pObjShell )
                {
                    SfxRequest aReq( SID_BASICIDE_SHOWWINDOW, SFX_CALLMODE_SYNCHRON, pObjShell->GetPool() );
                    aReq.SetArgs( *pArgs );
                    pViewShell->ExecuteSlot( aReq );
                }
            }

            rReq.Done();
        }
        break;

        case SID_BASICCHOOSER:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();
            const SfxPoolItem* pItem;
            BOOL bChooseOnly = FALSE, bExecute = TRUE;
            if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(SID_RECORDMACRO, sal_False, &pItem) )
            {
                BOOL bRecord = ((SfxBoolItem*)pItem)->GetValue();
                if ( bRecord )
                {
                    // !Hack
                    bChooseOnly = FALSE;
                    bExecute = FALSE;
                }
            }

            rReq.SetReturnValue( SfxStringItem( rReq.GetSlot(), ChooseMacro( bExecute, bChooseOnly ) ) );
            rReq.Done();
        }
        break;

        case SID_MACROORGANIZER:
        {
            OSL_TRACE("handling SID_MACROORGANIZER");
            const SfxItemSet* pArgs = rReq.GetArgs();
            const SfxPoolItem* pItem;
            INT16 nTabId = 0;
            if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(SID_MACROORGANIZER, sal_False, &pItem) )
            {
                nTabId = ((SfxUInt16Item*)pItem)->GetValue();
            }

            SfxApplication::MacroOrganizer( nTabId );
            rReq.Done();
        }
        break;

        case SID_RUNMACRO:
        {
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            OSL_TRACE("SfxApplication::OfaExec_Impl: case ScriptOrg");
            AbstractScriptSelectorDialog* pDlg = pFact->CreateScriptSelectorDialog( GetTopWindow() );
            if( pDlg )
            {
                pDlg->SetRunLabel();
                short ret = pDlg->Execute();
                if ( ret )
                {
                    const String scriptURL = pDlg->GetScriptURL();
                    SfxViewFrame* pView = SfxViewFrame::Current();
                    SfxObjectShell* pObjShell = NULL;
                    Sequence< Any > args(0);
                    Any aRet;
                    Sequence< sal_Int16 > outIndex;
                    Sequence< Any > outArgs( 0 );
                    if ( pView && ( pObjShell = pView->GetObjectShell() ) )
                    {
                        pObjShell->CallXScript(scriptURL, args, aRet, outIndex, outArgs);
                    }
                    else
                    {
                        bool bCaughtException = FALSE;
                        Any aException;

                        try
                        {
                            Reference< XPropertySet > xProps( ::comphelper::getProcessServiceFactory(), UNO_QUERY_THROW );
                            Reference< XComponentContext > xCtx( xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))), UNO_QUERY_THROW );

                            Reference< provider::XScriptProviderFactory > xFac(
                                xCtx->getValueByName(
                                    ::rtl::OUString::createFromAscii( "/singletons/drafts.com.sun.star.script.provider.theMasterScriptProviderFactory") ), UNO_QUERY_THROW );

                            Any aContext;

                            Reference< provider::XScriptProvider > xScriptProvider( xFac->createScriptProvider( aContext ),
                                UNO_QUERY_THROW );
                            Reference< provider::XScript > xScript(
                                xScriptProvider->getScript( scriptURL ), UNO_QUERY_THROW );

                            Any aRet = xScript->invoke( args, outIndex, outArgs );
                        }
                        catch ( provider::ScriptFrameworkErrorException& se )
                        {
                            aException = makeAny( se );
                            bCaughtException = TRUE;
                        }
                        catch ( ::com::sun::star::reflection::InvocationTargetException& ite )
                        {
                            aException = makeAny( ite );
                            bCaughtException = TRUE;
                        }
                        catch ( ::com::sun::star::lang::IllegalArgumentException& iae )
                        {
                            aException = makeAny( iae );
                            bCaughtException = TRUE;
                        }
                        catch ( ::com::sun::star::uno::RuntimeException& rte )
                        {
                            aException = makeAny( rte );
                            bCaughtException = TRUE;
                        }
                        catch ( ::com::sun::star::uno::Exception& e )
                        {
                            aException = makeAny( e );
                            bCaughtException = TRUE;
                        }

                         if ( bCaughtException )
                        {
                            SfxAbstractDialogFactory* pFact =
                                SfxAbstractDialogFactory::Create();

                            if ( pFact != NULL )
                            {
                                VclAbstractDialog* pDlg =
                                    pFact->CreateScriptErrorDialog(
                                        NULL, aException );

                                if ( pDlg != NULL )
                                {
                                    pDlg->Execute();
                                    delete pDlg;
                                }
                            }
                           }
                    }
                }
            }
            else
            {
                OSL_TRACE("no dialog!!!");
            }
            rReq.Done();
        }
        break;

        case SID_SCRIPTORGANIZER:
        {
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            OSL_TRACE("SfxApplication::OfaExec_Impl: case ScriptOrg");
            const SfxItemSet* pArgs = rReq.GetArgs();
            const SfxPoolItem* pItem;
            String aLanguage;
            if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(SID_SCRIPTORGANIZER, sal_False, &pItem) )
            {
                aLanguage = ((SfxScriptOrganizerItem*)pItem)->getLanguage();
            }

            ::rtl::OUString aLang( aLanguage );
            OSL_TRACE("SfxApplication::OfaExec_Impl: about to create dialog for: %s", ::rtl::OUStringToOString( aLang , RTL_TEXTENCODING_ASCII_US ).pData->buffer);
            // not sure about the Window*
            VclAbstractDialog* pDlg = pFact->CreateSvxScriptOrgDialog( GetTopWindow(), aLanguage );
            if( pDlg )
            {
                pDlg->Execute();
            }
            else
            {
                OSL_TRACE("no dialog!!!");
            }
            rReq.Done();
        }
        break;

        case SID_OFFICE_CHECK_PLZ:
        {
            sal_Bool bRet = sal_False;
            SFX_REQUEST_ARG(rReq, pStringItem, SfxStringItem, rReq.GetSlot(), sal_False);

            if ( pStringItem )
            {
                String aPLZ = pStringItem->GetValue();
                bRet = TRUE /*!!!SfxIniManager::CheckPLZ( aPLZ )*/;
            }
            else
                SbxBase::SetError( SbxERR_WRONG_ARGS );
            rReq.SetReturnValue( SfxBoolItem( rReq.GetSlot(), bRet ) );
        }
        break;

        case SID_AUTO_CORRECT_DLG:
        {
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            if ( pFact )
            {
                SfxItemSet aSet(GetPool(), SID_AUTO_CORRECT_DLG, SID_AUTO_CORRECT_DLG);
                const SfxPoolItem* pItem=NULL;
                const SfxItemSet* pSet = rReq.GetArgs();
                SfxItemPool* pPool = pSet ? pSet->GetPool() : NULL;
                if ( pSet && pSet->GetItemState( pPool->GetWhich( SID_AUTO_CORRECT_DLG ), FALSE, &pItem ) == SFX_ITEM_SET )
                    aSet.Put( *pItem );

                  SfxAbstractTabDialog* pDlg = pFact->CreateTabDialog( ResId( RID_OFA_AUTOCORR_DLG ), NULL, &aSet, NULL );
                  pDlg->Execute();
                  delete pDlg;
            }

            break;
        }

        case SID_SD_AUTOPILOT :
        case SID_NEWSD :
        {
            SvtModuleOptions aModuleOpt;
            if ( !aModuleOpt.IsImpress() )
            {
                ErrorBox( 0, ResId( RID_ERRBOX_MODULENOTINSTALLED, GetOffResManager_Impl() )).Execute();
                return;
            }

            Reference< com::sun::star::lang::XMultiServiceFactory > xORB = ::comphelper::getProcessServiceFactory();
            Reference< com::sun::star::frame::XDispatchProvider > xProv(
                xORB->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.drawing.ModuleDispatcher")), UNO_QUERY );

            if ( xProv.is() )
            {
                ::rtl::OUString aCmd = ::rtl::OUString::createFromAscii( GetInterface()->GetSlot( rReq.GetSlot() )->GetUnoName() );
                Reference< com::sun::star::frame::XDispatchHelper > xHelper(
                    xORB->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.frame.DispatchHelper")), UNO_QUERY );
                if ( xHelper.is() )
                {
                    Sequence < com::sun::star::beans::PropertyValue > aSeq;
                    if ( rReq.GetArgs() )
                        TransformItems( rReq.GetSlot(), *rReq.GetArgs(), aSeq );
                    Any aResult = xHelper->executeDispatch( xProv, aCmd, ::rtl::OUString(), 0, aSeq );
                    ::com::sun::star::frame::DispatchResultEvent aEvent;
                    sal_Bool bSuccess = (
                                         (aResult >>= aEvent) &&
                                         (aEvent.State == ::com::sun::star::frame::DispatchResultState::SUCCESS)
                                        );
                    rReq.SetReturnValue( SfxBoolItem( rReq.GetSlot(), bSuccess ) );
                }
            }
        }
        break;

        case SID_SW_AGENDA_WIZZARD :
        case SID_SW_FAX_WIZZARD :
        case SID_SW_LETTER_WIZZARD :
        case SID_SW_MEMO_WIZZARD :
        case FN_LABEL :
        case FN_BUSINESS_CARD :
        {
            Reference< com::sun::star::lang::XMultiServiceFactory > xORB = ::comphelper::getProcessServiceFactory();
            Reference< com::sun::star::frame::XDispatchProvider > xProv(
                xORB->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.text.ModuleDispatcher")), UNO_QUERY );

            if ( xProv.is() )
            {
                ::rtl::OUString aCmd = ::rtl::OUString::createFromAscii( GetInterface()->GetSlot( rReq.GetSlot() )->GetUnoName() );
                Reference< com::sun::star::frame::XDispatchHelper > xHelper(
                    xORB->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.frame.DispatchHelper")), UNO_QUERY );
                if ( xHelper.is() )
                {
                    Sequence < com::sun::star::beans::PropertyValue > aSeq;
                    if ( rReq.GetArgs() )
                        TransformItems( rReq.GetSlot(), *rReq.GetArgs(), aSeq );
                    Any aResult = xHelper->executeDispatch( xProv, aCmd, ::rtl::OUString(), 0, aSeq );
                    ::com::sun::star::frame::DispatchResultEvent aEvent;
                    sal_Bool bSuccess = (
                                         (aResult >>= aEvent) &&
                                         (aEvent.State == ::com::sun::star::frame::DispatchResultState::SUCCESS)
                                        );
                    rReq.SetReturnValue( SfxBoolItem( rReq.GetSlot(), bSuccess ) );
                }
            }
        }
        break;

        case SID_ADDRESS_DATA_SOURCE:
        case SID_DATASOURCE_ADMINISTRATION:
        case SID_SDB52_IMPORT_WIZARD:
        {
            ::rtl::OUString sDialogServiceName;
            if ( rReq.GetSlot() == SID_ADDRESS_DATA_SOURCE )
                sDialogServiceName = ::rtl::OUString::createFromAscii("com.sun.star.ui.dialogs.AddressBookSourcePilot");
            else if ( rReq.GetSlot() == SID_DATASOURCE_ADMINISTRATION )
                sDialogServiceName = ::rtl::OUString::createFromAscii("com.sun.star.sdb.DatasourceAdministrationDialog");
            else
                sDialogServiceName = ::rtl::OUString::createFromAscii("com.sun.star.sdb.DatabaseImportWizard");
            try
            {
                Reference< com::sun::star::lang::XMultiServiceFactory > xORB = ::comphelper::getProcessServiceFactory();
                Reference< com::sun::star::ui::dialogs::XExecutableDialog > xDialog;
                if (xORB.is())
                    xDialog = Reference< com::sun::star::ui::dialogs::XExecutableDialog >(xORB->createInstance(sDialogServiceName), UNO_QUERY);
                if (xDialog.is())
                    xDialog->execute();
                else
                    ShowServiceNotAvailableError(NULL, sDialogServiceName, TRUE);
            }
            catch(::com::sun::star::uno::Exception&)
            {
            }
        }
        break;

        case SID_COMP_BIBLIOGRAPHY:
        {
            SfxStringItem aURL(SID_FILE_NAME, String::CreateFromAscii(".component:Bibliography/View1"));
            SfxStringItem aRef(SID_REFERER, String::CreateFromAscii("private:user"));
            SfxStringItem aTarget(SID_TARGETNAME, String::CreateFromAscii("_blank"));
            SfxViewFrame::Current()->GetDispatcher()->Execute( SID_OPENDOC, SFX_CALLMODE_ASYNCHRON, &aURL, &aRef, &aTarget, 0L);
        }
        break;
    }
}

void SfxApplication::OfaState_Impl(SfxItemSet &rSet)
{
    const USHORT *pRanges = rSet.GetRanges();
    DBG_ASSERT(pRanges && *pRanges, "Set ohne Bereich");
    while ( *pRanges )
    {
        for(USHORT nWhich = *pRanges++; nWhich <= *pRanges; ++nWhich)
        {
            switch(nWhich)
            {
                case SID_ONLINE_REGISTRATION:
                {
                    ::svt::RegOptions aOptions;
                    if ( !aOptions.allowMenu() )
                        rSet.DisableItem( SID_ONLINE_REGISTRATION );
                }
                break;
            }
        }
    }

    SvtModuleOptions aModuleOpt;

    if( !aModuleOpt.IsWriter())
    {
        rSet.DisableItem( SID_SW_AGENDA_WIZZARD );
        rSet.DisableItem( SID_SW_FAX_WIZZARD );
        rSet.DisableItem( SID_SW_LETTER_WIZZARD );
        rSet.DisableItem( SID_SW_MEMO_WIZZARD );
        rSet.DisableItem( FN_LABEL );
        rSet.DisableItem( FN_BUSINESS_CARD );
    }

    if ( !aModuleOpt.IsImpress() )
        rSet.DisableItem( SID_SD_AUTOPILOT );
}

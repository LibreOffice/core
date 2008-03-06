/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: appserv.cxx,v $
 *
 *  $Revision: 1.74 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:50:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMultiServiceFactory_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
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
#ifndef _COM_SUN_STAR_FRAME_XLAYOUTMANAGER_HPP_
#include <com/sun/star/frame/XLayoutManager.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEMBEDDEDSCRIPTS_HPP_
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif

#ifndef _COM_SUN_STAR_SYSTEM_XSYSTEMSHELLEXECUTE_HPP_
#include <com/sun/star/system/XSystemShellExecute.hpp>
#endif
#ifndef _COM_SUN_STAR_SYSTEM_SYSTEMSHELLEXECUTEFLAGS_HPP_
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#endif
#ifndef _COM_SUN_STAR_SYSTEM_SYSTEMSHELLEXECUTEEXCEPTION_HPP_
#include <com/sun/star/system/SystemShellExecuteException.hpp>
#endif

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COMPHELPER_STORAGEHELPER_HXX
#include <comphelper/storagehelper.hxx>
#endif

#ifndef _SVT_DOC_ADDRESSTEMPLATE_HXX_
#include <svtools/addresstemplate.hxx>
#endif
#ifndef _SFXVISIBILITYITEM_HXX
#include <svtools/visitem.hxx>
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
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
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
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#include <svtools/pathoptions.hxx>
#include <svtools/moduleoptions.hxx>
#include <svtools/regoptions.hxx>
#include <svtools/helpopt.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/shl.hxx>
#include <unotools/bootstrap.hxx>
#include <vos/process.hxx>
#include <rtl/bootstrap.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <com/sun/star/script/provider/XScriptProviderFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include "about.hxx"
#include "referers.hxx"
#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/hintpost.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include "docvor.hxx"
#include <sfx2/new.hxx>
#include <sfx2/templdlg.hxx>
#include "sfxtypes.hxx"
#include "sfxbasic.hxx"
#include <sfx2/tabdlg.hxx>
#include "arrdecl.hxx"
#include "fltfnc.hxx"
#include <sfx2/sfx.hrc>
#include "app.hrc"
#include <sfx2/passwd.hxx>
#include "sfxresid.hxx"
#include "arrdecl.hxx"
#include <sfx2/childwin.hxx>
#include "appdata.hxx"
#include <sfx2/macrconf.hxx>
#include "minfitem.hxx"
#include <sfx2/event.hxx>
#include <sfx2/module.hxx>
#include <sfx2/topfrm.hxx>
#include "sfxpicklist.hxx"
#include "imestatuswindow.hxx"
#include <sfx2/sfxdlg.hxx>
#include <sfx2/dialogs.hrc>
#include "sorgitm.hxx"
#include "sfxhelp.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::system;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;

//-------------------------------------------------------------------------
long QuitAgain_Impl( void* pObj, void* pArg )
{
    SfxApplication* pApp = (SfxApplication*)pObj;
    Timer* pTimer = (Timer*)pArg;
    delete pTimer;
    pApp->GetDispatcher_Impl()->Execute( SID_QUITAPP, SFX_CALLMODE_ASYNCHRON );
    return 0;
}

void SfxApplication::MiscExec_Impl( SfxRequest& rReq )
{
    DBG_MEMTEST();
    FASTBOOL bDone = FALSE;
    switch ( rReq.GetSlot() )
    {
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
                pAppData_Impl->pAppDispat->Execute( SID_OPENDOC, SFX_CALLMODE_SLOT, &aNameItem, &aReferer, 0L );
                return;
            }

            // aus verschachtelten Requests nach 100ms nochmal probieren
            if( Application::GetDispatchLevel() > 1 )
            {
                /* Dont save the request for closing the application and try it later
                   again. This is an UI bound functionality ... and the user will  try it again
                   if the dialog is closed. But we shouldnt close the application automaticly
                   if this dialog is closed by the user ...
                   So we ignore this request now and wait for a new user decision.
                */
                DBG_TRACE1( "QueryExit => FALSE (DispatchLevel == %u)", Application::GetDispatchLevel() );
                return;
            }

            // block reentrant calls
            pAppData_Impl->bInQuit = TRUE;
            Reference < XDesktop > xDesktop ( ::comphelper::getProcessServiceFactory()->createInstance( DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );

            rReq.ForgetAllArgs();

            // if terminate() failed, pAppData_Impl->bInQuit will now be FALSE, allowing further calls of SID_QUITAPP
            BOOL bTerminated = xDesktop->terminate();
            if (!bTerminated)
                // if terminate() was successful, SfxApplication is now dead!
                pAppData_Impl->bInQuit = FALSE;

            // Returnwert setzten, ggf. terminieren
            rReq.SetReturnValue( SfxBoolItem( rReq.GetSlot(), bTerminated ) );
            return;
        }

        case SID_CONFIG:
        case SID_TOOLBOXOPTIONS:
        case SID_CONFIGSTATUSBAR:
        case SID_CONFIGMENU:
        case SID_CONFIGACCEL:
        case SID_CONFIGEVENT:
        {
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

                Reference< XFrame > xFrame;
                const SfxItemSet* pIntSet = rReq.GetInternalArgs_Impl();
                SFX_ITEMSET_ARG( pIntSet, pFrame, SfxUnoAnyItem, SID_FILLFRAME, FALSE );
                if (pFrame)
                    pFrame->GetValue() >>= xFrame;

                SfxAbstractTabDialog* pDlg = pFact->CreateTabDialog(
                    RID_SVXDLG_CUSTOMIZE,
                    NULL, &aSet, xFrame );

                  if ( pDlg )
                {
                    const short nRet = pDlg->Execute();

                    if ( nRet )
                        bDone = TRUE;

                    delete pDlg;
                }
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
            BOOL bTmpDone = TRUE;
            for ( SfxObjectShell *pObjSh = SfxObjectShell::GetFirst();
                  pObjSh;
                  pObjSh = SfxObjectShell::GetNext( *pObjSh ) )
            {
                SfxRequest aReq( SID_SAVEDOC, 0, pObjSh->GetPool() );
                if ( pObjSh->IsModified() )
                {
                    pObjSh->ExecuteSlot( aReq );
                    SfxBoolItem *pItem = PTR_CAST( SfxBoolItem, aReq.GetReturnValue() );
                    bTmpDone = aReq.IsDone();
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
                if ( 66056 == nHelpId )
                {
                    // show Support page with new URL
                    String sHelpURL = SfxHelp::CreateHelpURL( nHelpId, String() );
                    String sParams = sHelpURL.Copy( sHelpURL.Search( '?' ) );
                    sHelpURL = String::CreateFromAscii("vnd.sun.star.help://shared/text/shared/05/00000001.xhp");
                    sHelpURL += sParams;
                    sHelpURL += String::CreateFromAscii("&UseDB=no");
                    pHelp->Start( sHelpURL, NULL );
                }
                else
                    pHelp->Start( nHelpId, NULL ); // show start page
                bDone = TRUE;
            }
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_HELPTIPS:
        {
            // Parameter aus werten
            SFX_REQUEST_ARG(rReq, pOnItem, SfxBoolItem, SID_HELPTIPS, FALSE);
            bool bOn = pOnItem
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
        case SID_EXTENDEDHELP:
        {
            Help::StartExtHelp();
            break;
        }
        case SID_HELPBALLOONS:
        {
            // Parameter auswerten
            SFX_REQUEST_ARG(rReq, pOnItem, SfxBoolItem, SID_HELPBALLOONS, FALSE);
            bool bOn = pOnItem
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
            ::rtl::OUString aDefault;
            String          aVerId( utl::Bootstrap::getBuildIdData( aDefault ));

            OSL_ENSURE( aVerId.Len() != 0, "No BUILDID in bootstrap file" );

            String aVersion( '[' );
            ( aVersion += aVerId ) += ']';

            // About-Dialog suchen
            ResId aDialogResId( RID_DEFAULTABOUT, *pAppData_Impl->pLabelResMgr );
            ResMgr* pResMgr = pAppData_Impl->pLabelResMgr;
            if( ! pResMgr->IsAvailable( aDialogResId.SetRT( RSC_MODALDIALOG ) ) )
                pResMgr = GetOffResManager_Impl();

            aDialogResId.SetResMgr( pResMgr );
            if ( !pResMgr->IsAvailable( aDialogResId ) )
            {
                DBG_ERROR( "No RID_DEFAULTABOUT in label-resource-dll" );
            }

            // About-Dialog anzeigen
            AboutDialog* pDlg = new AboutDialog( 0, aDialogResId, aVersion );
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

        case SID_AVAILABLE_TOOLBARS:
        {
            SfxStringItem const * pToolbarName = static_cast< SfxStringItem const *>(
                    rReq.GetArg(SID_AVAILABLE_TOOLBARS, false, TYPE(SfxStringItem)));

            if ( pToolbarName )
            {
                com::sun::star::uno::Reference< com::sun::star::frame::XFrame > xFrame;
                Reference < XFramesSupplier > xDesktop ( ::comphelper::getProcessServiceFactory()->createInstance(
                                                            DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );
                xFrame = xDesktop->getActiveFrame();

                Reference< com::sun::star::beans::XPropertySet > xPropSet( xFrame, UNO_QUERY );
                Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;
                if ( xPropSet.is() )
                {
                    try
                    {
                        Any aValue = xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" )));
                        aValue >>= xLayoutManager;
                    }
                    catch ( ::com::sun::star::uno::RuntimeException& e )
                    {
                        throw e;
                    }
                    catch ( ::com::sun::star::uno::Exception& )
                    {
                    }
                }

                if ( xLayoutManager.is() )
                {
                    rtl::OUString aToolbarResName( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/" ));
                    rtl::OUStringBuffer aBuf( aToolbarResName );
                    aBuf.append( pToolbarName->GetValue() );

                    // Parameter auswerten
                    rtl::OUString aToolbarName( aBuf.makeStringAndClear() );
                    BOOL bShow( !xLayoutManager->isElementVisible( aToolbarName ));

                    if ( bShow )
                    {
                        xLayoutManager->createElement( aToolbarName );
                        xLayoutManager->showElement( aToolbarName );
                    }
                    else
                        xLayoutManager->hideElement( aToolbarName );
                }
            }

            bDone = true;
            break;
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

    LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
    const USHORT *pRanges = rSet.GetRanges();
    DBG_ASSERT(pRanges && *pRanges, "Set ohne Bereich");
    while ( *pRanges )
    {
        for(USHORT nWhich = *pRanges++; nWhich <= *pRanges; ++nWhich)
        {
            switch(nWhich)
            {
                case SID_TEMPLATE_ADDRESSBOKSOURCE:
                    if ( !SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::E_SDATABASE) )
                        rSet.Put(SfxVisibilityItem(nWhich, sal_False));
                    break;
                case SID_EXITANDRETURN:
                case SID_QUITAPP:
                {
                    if ( pAppData_Impl->nDocModalMode )
                        rSet.DisableItem(nWhich);
                    else
                        rSet.Put(SfxStringItem(nWhich, String(SfxResId(STR_QUITAPP))));
                    break;
                }

                case SID_BASICSTOP:
                    if ( !StarBASIC::IsRunning() )
                        rSet.DisableItem(nWhich);
                    break;

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

typedef rtl_uString* (SAL_CALL *basicide_choose_macro)(XModel*, BOOL, rtl_uString*);
typedef void (SAL_CALL *basicide_macro_organizer)( INT16 );

#define DOSTRING( x )                       #x
#define STRING( x )                         DOSTRING( x )

extern "C" { static void SAL_CALL thisModule() {} }

::rtl::OUString ChooseMacro( const Reference< XModel >& rxLimitToDocument, BOOL bChooseOnly, const ::rtl::OUString& rMacroDesc = ::rtl::OUString() )
{
    // get basctl dllname
    String sLibName = String::CreateFromAscii( STRING( DLL_NAME ) );
    sLibName.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "sfx" ) ), String( RTL_CONSTASCII_USTRINGPARAM( "basctl" ) ) );
    ::rtl::OUString aLibName( sLibName );

    // load module
    oslModule handleMod = osl_loadModuleRelative(
        &thisModule, aLibName.pData, 0 );

    // get symbol
    ::rtl::OUString aSymbol( RTL_CONSTASCII_USTRINGPARAM( "basicide_choose_macro" ) );
    basicide_choose_macro pSymbol = (basicide_choose_macro) osl_getFunctionSymbol( handleMod, aSymbol.pData );

    // call basicide_choose_macro in basctl
    rtl_uString* pScriptURL = pSymbol( rxLimitToDocument.get(), bChooseOnly, rMacroDesc.pData );
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
    oslModule handleMod = osl_loadModuleRelative(
        &thisModule, aLibName.pData, 0 );

    // get symbol
    ::rtl::OUString aSymbol( RTL_CONSTASCII_USTRINGPARAM( "basicide_macro_organizer" ) );
    basicide_macro_organizer pSymbol = (basicide_macro_organizer) osl_getFunctionSymbol( handleMod, aSymbol.pData );

    // call basicide_macro_organizer in basctl
    pSymbol( nTabId );
}

#define RID_ERRBOX_MODULENOTINSTALLED     (RID_OFA_START + 72)

ResMgr* SfxApplication::GetOffResManager_Impl()
{
    if ( !pAppData_Impl->pOfaResMgr )
        pAppData_Impl->pOfaResMgr = CreateResManager( "ofa");
    return pAppData_Impl->pOfaResMgr;
}

namespace
{
    Window* lcl_getDialogParent( const Reference< XFrame >& _rxFrame, Window* _pFallback )
    {
        if ( !_rxFrame.is() )
            return _pFallback;

        try
        {
            Reference< awt::XWindow > xContainerWindow( _rxFrame->getContainerWindow(), UNO_SET_THROW );
            Window* pWindow = VCLUnoHelper::GetWindow( xContainerWindow );
            OSL_ENSURE( pWindow, "lcl_getDialogParent: cool, somebody implemented a VCL-less toolkit!" );

            if ( pWindow )
                return pWindow->GetSystemWindow();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return _pFallback;
    }
}

void SfxApplication::OfaExec_Impl( SfxRequest& rReq )
{
    DBG_MEMTEST();
    switch ( rReq.GetSlot() )
    {
        case SID_OPTIONS_TREEDIALOG:
        {
            String sPageURL;
            SFX_REQUEST_ARG( rReq, pURLItem, SfxStringItem, SID_OPTIONS_PAGEURL, sal_False );
            if ( pURLItem )
                sPageURL = pURLItem->GetValue();
            const SfxItemSet* pArgs = rReq.GetInternalArgs_Impl();
            const SfxPoolItem* pItem = NULL;
            Reference < XFrame > xFrame;
            if ( pArgs && pArgs->GetItemState( SID_FILLFRAME, sal_False, &pItem ) == SFX_ITEM_SET )
                 ( (SfxUnoAnyItem*)pItem )->GetValue() >>= xFrame;
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            if ( pFact )
            {
                VclAbstractDialog* pDlg =
                    pFact->CreateFrameDialog( NULL, xFrame, rReq.GetSlot(), sPageURL );
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
            ::rtl::OUString aBasicName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.script.BasicIDE" ) );
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
                pView = SfxViewFrame::CreateViewFrame( *pDocShell, 0 );
                pView->SetName( String( RTL_CONSTASCII_USTRINGPARAM( "BASIC:1" ) ) );
            }

            if ( pView )
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
            BOOL bChooseOnly = FALSE;
            Reference< XModel > xLimitToModel;
            if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(SID_RECORDMACRO, sal_False, &pItem) )
            {
                BOOL bRecord = ((SfxBoolItem*)pItem)->GetValue();
                if ( bRecord )
                {
                    // !Hack
                    bChooseOnly = FALSE;
                    SfxObjectShell* pCurrentShell = SfxObjectShell::Current();
                    OSL_ENSURE( pCurrentShell, "macro recording outside an SFX document?" );
                    if ( pCurrentShell )
                        xLimitToModel = pCurrentShell->GetModel();
                }
            }

            rReq.SetReturnValue( SfxStringItem( rReq.GetSlot(), ChooseMacro( xLimitToModel, bChooseOnly ) ) );
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

            Reference< XFrame > xFrame;
            const SfxItemSet* pIntSet = rReq.GetInternalArgs_Impl();
            SFX_ITEMSET_ARG( pIntSet, pFrameItem, SfxUnoAnyItem, SID_FILLFRAME, FALSE );
            if ( pFrameItem )
                pFrameItem->GetValue() >>= xFrame;

            if ( !xFrame.is() )
            {
                const SfxViewFrame* pViewFrame = SfxViewFrame::Current();
                const SfxFrame* pFrame = pViewFrame ? pViewFrame->GetFrame() : NULL;
                if ( pFrame )
                    xFrame = pFrame->GetFrameInterface();
            }

            do  // artificial loop for flow control
            {
                AbstractScriptSelectorDialog* pDlg = pFact->CreateScriptSelectorDialog(
                    lcl_getDialogParent( xFrame, GetTopWindow() ), FALSE, xFrame );
                OSL_ENSURE( pDlg, "SfxApplication::OfaExec_Impl( SID_RUNMACRO ): no dialog!" );
                if ( !pDlg )
                    break;
                pDlg->SetRunLabel();

                short nDialogResult = pDlg->Execute();
                if ( !nDialogResult )
                    break;

                Sequence< Any > args;
                Sequence< sal_Int16 > outIndex;
                Sequence< Any > outArgs;
                Any ret;

                Reference< XInterface > xScriptContext;

                Reference< XController > xController;
                if ( xFrame.is() )
                    xController = xFrame->getController();
                if ( xController.is() )
                    xScriptContext = xController->getModel();
                if ( !xScriptContext.is() )
                    xScriptContext = xController;

                SfxObjectShell::CallXScript( xScriptContext, pDlg->GetScriptURL(), args, ret, outIndex, outArgs );
            }
            while ( false );
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
                SfxItemPool* pSetPool = pSet ? pSet->GetPool() : NULL;
                if ( pSet && pSet->GetItemState( pSetPool->GetWhich( SID_AUTO_CORRECT_DLG ), FALSE, &pItem ) == SFX_ITEM_SET )
                    aSet.Put( *pItem );

                  SfxAbstractTabDialog* pDlg = pFact->CreateTabDialog( RID_OFA_AUTOCORR_DLG, NULL, &aSet, NULL );
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
                ErrorBox( 0, ResId( RID_ERRBOX_MODULENOTINSTALLED, *GetOffResManager_Impl() )).Execute();
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

        case FN_LABEL :
        case FN_BUSINESS_CARD :
        case FN_XFORMS_INIT :
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
        {
            ::rtl::OUString sDialogServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.AddressBookSourcePilot" ) );
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
        rSet.DisableItem( FN_LABEL );
        rSet.DisableItem( FN_BUSINESS_CARD );
        rSet.DisableItem( FN_XFORMS_INIT );
    }

    if ( !aModuleOpt.IsImpress() )
        rSet.DisableItem( SID_SD_AUTOPILOT );
}

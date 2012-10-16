/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/frame/DispatchResultEvent.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/task/XJobExecutor.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/system/SystemShellExecuteException.hpp>

#include <com/sun/star/frame/XComponentLoader.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include "comphelper/configurationhelper.hxx"

#include <svtools/addresstemplate.hxx>
#include <svtools/miscopt.hxx>
#include <svl/visitem.hxx>
#include <unotools/intlwrapper.hxx>

#include <unotools/configmgr.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/msgbox.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <basic/sbstar.hxx>
#include <basic/basmgr.hxx>
#include <basic/basrdll.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <vcl/help.hxx>
#include <vcl/stdtext.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/file.hxx>

#include <unotools/pathoptions.hxx>
#include <unotools/moduleoptions.hxx>
#include <svtools/helpopt.hxx>
#include <svtools/miscopt.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/shl.hxx>
#include <unotools/bootstrap.hxx>
#include <rtl/bootstrap.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <com/sun/star/script/provider/XScriptProviderFactory.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include "frmload.hxx"
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
#include <sfx2/tabdlg.hxx>
#include "arrdecl.hxx"
#include "fltfnc.hxx"
#include <sfx2/sfx.hrc>
#include "app.hrc"
#include <sfx2/passwd.hxx>
#include "sfx2/sfxresid.hxx"
#include <sfx2/childwin.hxx>
#include "appdata.hxx"
#include "sfx2/minfitem.hxx"
#include <sfx2/event.hxx>
#include <sfx2/module.hxx>
#include <sfx2/viewfrm.hxx>
#include "sfxpicklist.hxx"
#include "imestatuswindow.hxx"
#include <sfx2/sfxdlg.hxx>
#include <sfx2/dialogs.hrc>
#include "sorgitm.hxx"
#include "sfx2/sfxhelp.hxx"
#include <sfx2/zoomitem.hxx>
#include "templatedlg.hxx"

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

namespace css = com::sun::star;

/// Find the correct location of the document (LICENSE.odt, etc.), and return
/// it in rURL if found.
static sal_Bool checkURL( const char *pName, const char *pExt, rtl::OUString &rURL )
{
    using namespace osl;
    DirectoryItem aDirItem;

    rURL = rtl::OUString( "$BRAND_BASE_DIR/" );
    rURL += rtl::OUString::createFromAscii( pName );
    rURL += rtl::OUString::createFromAscii( pExt );
    rtl::Bootstrap::expandMacros( rURL );

    if (!rURL.isEmpty())
        return DirectoryItem::get( rURL, aDirItem ) == DirectoryItem::E_None;
    else
        return sal_False;
}

/// Displays CREDITS or LICENSE in any of the available version
static void showDocument( const char* pBaseName )
{
    try {
        Reference < XComponentLoader > xLoader( ::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString("com.sun.star.frame.Desktop") ), UNO_QUERY );
        Sequence < com::sun::star::beans::PropertyValue > args(2);
        args[0].Name = ::rtl::OUString("ViewOnly");
        args[0].Value <<= sal_True;
        args[1].Name = ::rtl::OUString("ReadOnly");
        args[1].Value <<= sal_True;

        rtl::OUString aURL;
        if ( checkURL ( pBaseName, ".odt", aURL ) ||
                checkURL ( pBaseName, ".html", aURL ) ||
                checkURL ( pBaseName, "", aURL ) ) {
            xLoader->loadComponentFromURL( aURL, ::rtl::OUString("_blank"), 0, args );
        }
    } catch (const ::com::sun::star::uno::Exception &) {
    }
}

namespace
{
    class LicenseDialog : public ModalDialog
    {
    private:
        FixedText aText;
        OKButton aShow;
        CancelButton aClose;

        DECL_LINK(CancelHdl, void *);
        DECL_LINK(ShowHdl, void *);
    public:
        LicenseDialog(Window *pParent=NULL);
    };

    LicenseDialog::LicenseDialog(Window *pParent)
        : ModalDialog(pParent, SfxResId(DLG_HELP_LICENSING))
        , aText( this )
        , aShow( this, SfxResId( PB_LICENSING_SHOW ) )
        , aClose( this, SfxResId( PB_LICENSING_CLOSE ) )
    {
        aClose.SetClickHdl( LINK( this, LicenseDialog, CancelHdl ) );
        aShow.SetClickHdl( LINK( this, LicenseDialog, ShowHdl ) );

        String aLicensing;
        for ( int i = STR_LICENSING_INFORMATION_1; i <= STR_LICENSING_INFORMATION_5; ++i )
        {
            if ( i != STR_LICENSING_INFORMATION_1 )
                aLicensing += String( RTL_CONSTASCII_USTRINGPARAM( "\n\n" ) );
            aLicensing += SfxResId( i ).toString();
        }

        aText.SetText( aLicensing );

        // positions and sizes are computed to always fit the language
        Size aTextSize( aText.GetOptimalSize( WINDOWSIZE_PREFERRED ) );
        Size aShowSize( aShow.GetOptimalSize( WINDOWSIZE_PREFERRED ) );
        Size aCloseSize( aClose.GetOptimalSize( WINDOWSIZE_PREFERRED ) );

        long nDelimX = 12;
        long nDelimY = 12;
        long nWidth = aTextSize.Width() + 2*nDelimX;
        long nButtonY = aTextSize.Height() + 2*nDelimY;
        Size aButtonSize( std::max( aShowSize.Width(), aCloseSize.Width() ) + nDelimX,
                std::max( aShowSize.Height(), aCloseSize.Height() ) );

        SetSizePixel( Size( nWidth, aTextSize.Height() + 3*nDelimY + aButtonSize.Height() ) );
        aText.SetPosSizePixel( Point( nDelimX, nDelimY ), aTextSize );
        aShow.SetPosSizePixel( Point( ( nWidth - nDelimX ) / 2 - aButtonSize.Width(), nButtonY ), aButtonSize );
        aClose.SetPosSizePixel( Point( aShow.GetPosPixel().X() + aButtonSize.Width() + nDelimX, nButtonY ), aButtonSize );

        aText.Show();

        FreeResource();
    }

    IMPL_LINK_NOARG(LicenseDialog, CancelHdl)
    {
        Close();
        return 0;
    }

    IMPL_LINK_NOARG(LicenseDialog, ShowHdl)
    {
        EndDialog(RET_OK);
        showDocument("LICENSE");
        return 0;
    }

}


void SfxApplication::MiscExec_Impl( SfxRequest& rReq )
{
    bool bDone = false;
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
                        pObjSh->SetModified( sal_False );
                    else
                        return;
                }

                SfxStringItem aNameItem( SID_FILE_NAME, rtl::OUString("vnd.sun.star.cmd:logout") );
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
                OSL_TRACE( "QueryExit => sal_False (DispatchLevel == %u)", Application::GetDispatchLevel() );
                return;
            }

            // block reentrant calls
            pAppData_Impl->bInQuit = sal_True;
            Reference < XDesktop > xDesktop ( ::comphelper::getProcessServiceFactory()->createInstance( DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );

            rReq.ForgetAllArgs();

            // if terminate() failed, pAppData_Impl->bInQuit will now be sal_False, allowing further calls of SID_QUITAPP
            sal_Bool bTerminated = xDesktop->terminate();
            if (!bTerminated)
                // if terminate() was successful, SfxApplication is now dead!
                pAppData_Impl->bInQuit = sal_False;

            // Set return value, terminate if possible
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
                SFX_ITEMSET_ARG( pIntSet, pFrameItem, SfxUnoFrameItem, SID_FILLFRAME, sal_False );
                if ( pFrameItem )
                    xFrame = pFrameItem->GetFrame();

                SfxAbstractTabDialog* pDlg = pFact->CreateTabDialog(
                    RID_SVXDLG_CUSTOMIZE,
                    NULL, &aSet, xFrame );

                  if ( pDlg )
                {
                    const short nRet = pDlg->Execute();

                    if ( nRet )
                        bDone = true;

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

            sal_Bool bOk = ( n == 0);
            rReq.SetReturnValue( SfxBoolItem( 0, bOk ) );
            bDone = true;
            break;
        }

        case SID_SAVEDOCS:
        {
            sal_Bool bOK = sal_True;
            for ( SfxObjectShell *pObjSh = SfxObjectShell::GetFirst();
                  pObjSh;
                  pObjSh = SfxObjectShell::GetNext( *pObjSh ) )
            {
                SfxRequest aReq( SID_SAVEDOC, 0, pObjSh->GetPool() );
                if ( pObjSh->IsModified() )
                {
                    pObjSh->ExecuteSlot( aReq );
                    SfxBoolItem *pItem = PTR_CAST( SfxBoolItem, aReq.GetReturnValue() );
                    if ( !pItem || !pItem->GetValue() )
                        bOK = sal_False;
                }
            }

            rReq.SetReturnValue( SfxBoolItem( 0, bOK ) );
            rReq.Done();
            break;
        }

        case SID_SEND_FEEDBACK:
        {
            ::rtl::OUString sURL("http://hub.libreoffice.org/file-a-bug/");
            try
            {
                uno::Reference< com::sun::star::system::XSystemShellExecute > xSystemShellExecute(
                    com::sun::star::system::SystemShellExecute::create(::comphelper::getProcessComponentContext()) );
                xSystemShellExecute->execute( sURL, ::rtl::OUString(), com::sun::star::system::SystemShellExecuteFlags::URIS_ONLY );
            }
            catch ( uno::Exception& )
            {
            }
            break;
        }

        case SID_SHOW_LICENSE:
        {
            LicenseDialog aDialog;
            aDialog.Execute();
            break;
        }

        case SID_SHOW_CREDITS:
        {
            showDocument( "CREDITS" );
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_HELPINDEX:
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
            {
                pHelp->Start( rtl::OUString(".uno:HelpIndex"), NULL ); // show start page
                bDone = true;
            }
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_HELPTIPS:
        {
            // Evaluate Parameter
            SFX_REQUEST_ARG(rReq, pOnItem, SfxBoolItem, SID_HELPTIPS, sal_False);
            bool bOn = pOnItem
                            ? ((SfxBoolItem*)pOnItem)->GetValue()
                            : !Help::IsQuickHelpEnabled();

            if ( bOn )
                Help::EnableQuickHelp();
            else
                Help::DisableQuickHelp();
            SvtHelpOptions().SetHelpTips( bOn );
            Invalidate(SID_HELPTIPS);
            bDone = true;

            // Record if possible
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
            // Evaluate Parameter
            SFX_REQUEST_ARG(rReq, pOnItem, SfxBoolItem, SID_HELPBALLOONS, sal_False);
            bool bOn = pOnItem
                            ? ((SfxBoolItem*)pOnItem)->GetValue()
                            : !Help::IsBalloonHelpEnabled();

            if ( bOn )
                Help::EnableBalloonHelp();
            else
                Help::DisableBalloonHelp();
            SvtHelpOptions().SetExtendedHelp( bOn );
            Invalidate(SID_HELPBALLOONS);
            bDone = true;

            // Record if possible
            if ( !rReq.IsAPI() )
                rReq.AppendItem( SfxBoolItem( SID_HELPBALLOONS, bOn) );
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_HELP_PI:
        {
            SvtHelpOptions aHelpOpt;
            SFX_REQUEST_ARG(rReq, pOnItem, SfxBoolItem, SID_HELP_PI, sal_False);
            sal_Bool bOn = pOnItem
                            ? ((SfxBoolItem*)pOnItem)->GetValue()
                            : !aHelpOpt.IsHelpAgentAutoStartMode();
            aHelpOpt.SetHelpAgentAutoStartMode( bOn );
            Invalidate(SID_HELP_PI);
            bDone = true;
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_ABOUT:
        {
             SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            if ( pFact )
            {
                VclAbstractDialog* pDlg = pFact->CreateVclDialog( 0, RID_DEFAULTABOUT );
                pDlg->Execute();
                delete pDlg;
            bDone = true;
            }
            break;
        }

        case SID_TEMPLATE_MANAGER:
        {
            SfxTemplateManagerDlg dlg;
            dlg.Execute();
            bDone = true;
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_ORGANIZER:
        {
            SfxTemplateOrganizeDlg  *pDlg =
                new SfxTemplateOrganizeDlg(NULL);
            pDlg->Execute();
            delete pDlg;
            bDone = true;
            break;
        }

        case SID_TEMPLATE_ADDRESSBOKSOURCE:
        {
            svt::AddressBookSourceDialog aDialog(GetTopWindow(), ::comphelper::getProcessServiceFactory());
            aDialog.Execute();
            bDone = true;
            break;
        }

#ifndef DISABLE_SCRIPTING
        case SID_BASICSTOP:
            StarBASIC::Stop();
            break;

        case SID_BASICBREAK :
            BASIC_DLL()->BasicBreak();
            break;
#endif

        case SID_CRASH :
        {
            // Provoke a crash:
            char * crash = 0;
            *crash = 0;
            break;
        }

        case SID_SHOW_IME_STATUS_WINDOW:
            if (pAppData_Impl->m_xImeStatusWindow->canToggle())
            {
                SfxBoolItem const * pItem = static_cast< SfxBoolItem const * >(
                    rReq.GetArg(SID_SHOW_IME_STATUS_WINDOW, false,
                                TYPE(SfxBoolItem)));
                bool bShow = pItem == 0
                    ? !pAppData_Impl->m_xImeStatusWindow->isShowing()
                    : ( pItem->GetValue() == sal_True );
                pAppData_Impl->m_xImeStatusWindow->show(bShow);
                if (pItem == 0)
                    rReq.AppendItem(SfxBoolItem(SID_SHOW_IME_STATUS_WINDOW,
                                                bShow));
            }
            bDone = true;
            break;

        case SID_ZOOM_50_PERCENT:
        case SID_ZOOM_75_PERCENT:
        case SID_ZOOM_100_PERCENT:
        case SID_ZOOM_150_PERCENT:
        case SID_ZOOM_200_PERCENT:
        case SID_ZOOM_OPTIMAL:
        case SID_ZOOM_ENTIRE_PAGE:
        case SID_ZOOM_PAGE_WIDTH:
        {
            // make sure aZoom is initialized with a proper value if SetType
            // doesn't work
            SvxZoomItem aZoom( SVX_ZOOM_PERCENT, 100 );

            switch (rReq.GetSlot())
            {
                case SID_ZOOM_50_PERCENT:
                    aZoom.SetValue(50);
                    break;
                case SID_ZOOM_75_PERCENT:
                    aZoom.SetValue(75);
                    break;
                case SID_ZOOM_100_PERCENT:
                    aZoom.SetValue(100);
                    break;
                case SID_ZOOM_150_PERCENT:
                    aZoom.SetValue(150);
                    break;
                case SID_ZOOM_200_PERCENT:
                    aZoom.SetValue(200);
                    break;
                case SID_ZOOM_OPTIMAL:
                    aZoom.SetType( SVX_ZOOM_OPTIMAL );
                    break;
                case SID_ZOOM_ENTIRE_PAGE:
                    aZoom.SetType( SVX_ZOOM_WHOLEPAGE );
                    break;
                case SID_ZOOM_PAGE_WIDTH:
                    aZoom.SetType( SVX_ZOOM_PAGEWIDTH );
                    break;
            }

            SfxViewFrame::Current()->GetDispatcher()->Execute(SID_ATTR_ZOOM, SFX_CALLMODE_ASYNCHRON, &aZoom, 0L);

            break;
        }
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
                        Any aValue = xPropSet->getPropertyValue( rtl::OUString( "LayoutManager" ));
                        aValue >>= xLayoutManager;
                    }
                    catch ( const ::com::sun::star::uno::RuntimeException& )
                    {
                        throw;
                    }
                    catch ( ::com::sun::star::uno::Exception& )
                    {
                    }
                }

                if ( xLayoutManager.is() )
                {
                    rtl::OUString aToolbarResName( "private:resource/toolbar/" );
                    rtl::OUStringBuffer aBuf( aToolbarResName );
                    aBuf.append( pToolbarName->GetValue() );

                    // Evaluate Parameter
                    rtl::OUString aToolbarName( aBuf.makeStringAndClear() );
                    sal_Bool bShow( !xLayoutManager->isElementVisible( aToolbarName ));

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
    LocaleDataWrapper aLocaleWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
    const sal_uInt16 *pRanges = rSet.GetRanges();
    DBG_ASSERT(pRanges && *pRanges, "Set without range");
    while ( *pRanges )
    {
        for(sal_uInt16 nWhich = *pRanges++; nWhich <= *pRanges; ++nWhich)
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
                        rSet.Put(SfxStringItem(nWhich, SfxResId(STR_QUITAPP).toString()));
                    break;
                }

                case SID_CONFIG:
                case SID_TOOLBOXOPTIONS:
                case SID_CONFIGSTATUSBAR:
                case SID_CONFIGMENU:
                case SID_CONFIGACCEL:
                case SID_CONFIGEVENT:
                {
                    if( SvtMiscOptions().DisableUICustomization() )
                        rSet.DisableItem(nWhich);
                    break;
                }

#ifndef DISABLE_SCRIPTING
                case SID_BASICSTOP:
                    if ( !StarBASIC::IsRunning() )
                        rSet.DisableItem(nWhich);
                    break;
#endif

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
                    sal_Bool bModified = sal_False;
                    for ( SfxObjectShell *pObjSh = SfxObjectShell::GetFirst();
                          pObjSh;
                          pObjSh = SfxObjectShell::GetNext( *pObjSh ) )
                    {
                        if ( pObjSh->IsModified() )
                        {
                            bModified = sal_True;
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

                case SID_TEMPLATE_MANAGER:
                    {
                        SvtMiscOptions aMiscOptions;
                        if ( !aMiscOptions.IsExperimentalMode() )
                        {
                           rSet.DisableItem( nWhich );
                           rSet.Put( SfxVisibilityItem( nWhich, sal_False ) );
                        }
                    }
                    break;

                default:
                    break;
            }
        }

        ++pRanges;
    }
}

#ifndef DISABLE_SCRIPTING

#ifndef DISABLE_DYNLOADING

typedef rtl_uString* (SAL_CALL *basicide_choose_macro)(XModel*, sal_Bool, rtl_uString*);
typedef void (SAL_CALL *basicide_macro_organizer)( sal_Int16 );

extern "C" { static void SAL_CALL thisModule() {} }

#else

extern "C" rtl_uString* basicide_choose_macro(XModel*, sal_Bool, rtl_uString*);
extern "C" void basicide_macro_organizer( sal_Int16 );

#endif

::rtl::OUString ChooseMacro( const Reference< XModel >& rxLimitToDocument, sal_Bool bChooseOnly, const ::rtl::OUString& rMacroDesc = ::rtl::OUString() )
{
#ifndef DISABLE_DYNLOADING
    // get basctl dllname
    static ::rtl::OUString aLibName( SVLIBRARY( "basctl"  ) );

    // load module
    oslModule handleMod = osl_loadModuleRelative(
        &thisModule, aLibName.pData, 0 );

    // get symbol
    ::rtl::OUString aSymbol( "basicide_choose_macro"  );
    basicide_choose_macro pSymbol = (basicide_choose_macro) osl_getFunctionSymbol( handleMod, aSymbol.pData );
#else
#define pSymbol basicide_choose_macro
#endif

    // call basicide_choose_macro in basctl
    rtl_uString* pScriptURL = pSymbol( rxLimitToDocument.get(), bChooseOnly, rMacroDesc.pData );
    ::rtl::OUString aScriptURL( pScriptURL );
    rtl_uString_release( pScriptURL );
    return aScriptURL;

#ifdef DISABLE_DYNLOADING
#undef pSymbol
#endif
}

void MacroOrganizer( sal_Int16 nTabId )
{
#ifndef DISABLE_DYNLOADING
    // get basctl dllname
    static ::rtl::OUString aLibName( SVLIBRARY( "basctl"  ) );

    // load module
    oslModule handleMod = osl_loadModuleRelative(
        &thisModule, aLibName.pData, 0 );

    // get symbol
    ::rtl::OUString aSymbol( "basicide_macro_organizer"  );
    basicide_macro_organizer pSymbol = (basicide_macro_organizer) osl_getFunctionSymbol( handleMod, aSymbol.pData );
    // call basicide_macro_organizer in basctl
    pSymbol( nTabId );
#else
    basicide_macro_organizer( nTabId );
#endif

}

#endif


#define RID_ERRBOX_MODULENOTINSTALLED     (RID_OFA_START + 72)

ResMgr* SfxApplication::GetOffResManager_Impl()
{
    if ( !pAppData_Impl->pOfaResMgr )
        pAppData_Impl->pOfaResMgr = CreateResManager( "ofa");
    return pAppData_Impl->pOfaResMgr;
}

namespace
{
#ifndef DISABLE_SCRIPTING
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

    const ::rtl::OUString& lcl_getBasicIDEServiceName()
    {
        static const ::rtl::OUString s_sBasicName( "com.sun.star.script.BasicIDE"  );
        return s_sBasicName;
    }

    SfxViewFrame* lcl_getBasicIDEViewFrame( SfxObjectShell* i_pBasicIDE )
    {
        SfxViewFrame* pView = SfxViewFrame::GetFirst( i_pBasicIDE );
        while ( pView )
        {
            if ( pView->GetObjectShell()->GetFactory().GetDocumentServiceName() == lcl_getBasicIDEServiceName() )
                break;
            pView = SfxViewFrame::GetNext( *pView, i_pBasicIDE );
        }
        return pView;
    }
    Reference< XFrame > lcl_findStartModuleFrame( const ::comphelper::ComponentContext& i_rContext )
    {
        try
        {
            Reference < XFramesSupplier > xSupplier( i_rContext.createComponent( "com.sun.star.frame.Desktop" ), UNO_QUERY_THROW );
            Reference < XIndexAccess > xContainer( xSupplier->getFrames(), UNO_QUERY_THROW );

            Reference< XModuleManager2 > xCheck( ModuleManager::create(i_rContext.getUNOContext()) );

            sal_Int32 nCount = xContainer->getCount();
            for ( sal_Int32 i=0; i<nCount; ++i )
            {
                try
                {
                    Reference < XFrame > xFrame( xContainer->getByIndex(i), UNO_QUERY_THROW );
                    ::rtl::OUString sModule = xCheck->identify( xFrame );
                    if ( sModule == "com.sun.star.frame.StartModule" )
                        return xFrame;
                }
                catch( const UnknownModuleException& )
                {
                    // silence
                }
                catch(const Exception&)
                {
                    // re-throw, caught below
                    throw;
                }
            }
        }
        catch( const Exception& )
        {
               DBG_UNHANDLED_EXCEPTION();
        }
        return NULL;
    }
#endif // !DISABLE_SCRIPTING
}

static ::rtl::OUString getConfigurationStringValue(
    const ::rtl::OUString& rPackage,
    const ::rtl::OUString& rRelPath,
    const ::rtl::OUString& rKey,
    const ::rtl::OUString& rDefaultValue )
{
    ::rtl::OUString aDefVal( rDefaultValue );

    try
    {
        ::comphelper::ConfigurationHelper::readDirectKey(
            comphelper::getProcessComponentContext(),
            rPackage,
            rRelPath,
            rKey,
            ::comphelper::ConfigurationHelper::E_READONLY) >>= aDefVal;
    }
    catch(const com::sun::star::uno::RuntimeException&)
    { throw; }
    catch(const com::sun::star::uno::Exception&)
    {}

    return aDefVal;
}

void SfxApplication::OfaExec_Impl( SfxRequest& rReq )
{
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
            {
                OSL_ENSURE( pItem->ISA( SfxUnoFrameItem ), "SfxApplication::OfaExec_Impl: XFrames are to be transported via SfxUnoFrameItem by now!" );
                xFrame = static_cast< const SfxUnoFrameItem*>( pItem )->GetFrame();
            }
            SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
            if ( pFact )
            {
                VclAbstractDialog* pDlg =
                    pFact->CreateFrameDialog( NULL, xFrame, rReq.GetSlot(), sPageURL );
                  short nRet = pDlg->Execute();
                  delete pDlg;
                SfxViewFrame* pView = SfxViewFrame::GetFirst();
                while ( pView )
                {
                    if (nRet == RET_OK)
                    {
                        SfxObjectShell* pObjSh = pView->GetObjectShell();
                        if (pObjSh)
                            pObjSh->SetConfigOptionsChecked(false);
                    }
                    pView->GetBindings().InvalidateAll(sal_False);
                    pView = SfxViewFrame::GetNext( *pView );
                }
            }
            break;
        }

        case SID_MORE_DICTIONARIES:
        {
            try
            {
                uno::Reference< lang::XMultiServiceFactory > xSMGR =
                    ::comphelper::getProcessServiceFactory();
                uno::Reference< uno::XComponentContext > xContext =
                    ::comphelper::getProcessComponentContext();
                uno::Reference< css::system::XSystemShellExecute > xSystemShell(
                    css::system::SystemShellExecute::create(xContext) );

                // read repository URL from configuration
                ::rtl::OUString sTemplRepoURL =
                    getConfigurationStringValue(
                        ::rtl::OUString("org.openoffice.Office.Common"),
                        ::rtl::OUString("Dictionaries"),
                        ::rtl::OUString("RepositoryURL"),
                        ::rtl::OUString());

                if ( xSystemShell.is() && !sTemplRepoURL.isEmpty() )
                {
                    ::rtl::OUStringBuffer aURLBuf( sTemplRepoURL );
                    aURLBuf.appendAscii(RTL_CONSTASCII_STRINGPARAM("?lang="));

                    // read locale from configuration
                    ::rtl::OUString sLocale = getConfigurationStringValue(
                        ::rtl::OUString("org.openoffice.Setup"),
                        ::rtl::OUString("L10N"),
                        ::rtl::OUString("ooLocale"),
                        ::rtl::OUString("en-US"));

                    aURLBuf.append( sLocale );
                    xSystemShell->execute(
                        aURLBuf.makeStringAndClear(),
                        ::rtl::OUString(),
                        css::system::SystemShellExecuteFlags::URIS_ONLY );
                }
            }
            catch( const ::com::sun::star::uno::Exception& )
            {
                SAL_WARN( "sfx2.appl", "SfxApplication::OfaExec_Impl(SID_MORE_DICTIONARIES): caught an exception!" );
            }
            break;
        }
#ifndef DISABLE_SCRIPTING
        case SID_BASICIDE_APPEAR:
        {
            SfxViewFrame* pView = lcl_getBasicIDEViewFrame( NULL );
            if ( !pView )
            {
                SfxObjectShell* pBasicIDE = SfxObjectShell::CreateObject( lcl_getBasicIDEServiceName() );
                pBasicIDE->DoInitNew( 0 );
                pBasicIDE->SetModified( sal_False );
                try
                {
                    // load the Basic IDE via direct access to the SFX frame loader. A generic loadComponentFromURL
                    // (which could be done via SfxViewFrame::LoadDocumentIntoFrame) is not feasible here, since the Basic IDE
                    // does not really play nice with the framework's concept. For instance, it is a "singleton document",
                    // which conflicts, at the latest, with the framework's concept of loading into _blank frames.
                    // So, since we know that our frame loader can handle it, we skip the generic framework loader
                    // mechanism, and the type detection (which doesn't know about the Basic IDE).
                    ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
                    Reference< XSynchronousFrameLoader > xLoader( aContext.createComponent(
                        SfxFrameLoader_Impl::impl_getStaticImplementationName() ), UNO_QUERY_THROW );
                    ::comphelper::NamedValueCollection aLoadArgs;
                    aLoadArgs.put( "Model", pBasicIDE->GetModel() );
                    aLoadArgs.put( "URL", ::rtl::OUString( "private:factory/sbasic"  ) );

                    Reference< XFrame > xTargetFrame( lcl_findStartModuleFrame( aContext ) );
                    if ( !xTargetFrame.is() )
                        xTargetFrame = SfxFrame::CreateBlankFrame();
                    ENSURE_OR_THROW( xTargetFrame.is(), "could not obtain a frameto load the Basic IDE into!" );

                    xLoader->load( aLoadArgs.getPropertyValues(), xTargetFrame );
                }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION();
                }

                pView = lcl_getBasicIDEViewFrame( pBasicIDE );
                if ( pView )
                    pView->SetName( String( RTL_CONSTASCII_USTRINGPARAM( "BASIC:1" ) ) );
            }

            if ( pView )
                pView->GetFrame().Appear();

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
            sal_Bool bChooseOnly = sal_False;
            Reference< XModel > xLimitToModel;
            if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(SID_RECORDMACRO, sal_False, &pItem) )
            {
                sal_Bool bRecord = ((SfxBoolItem*)pItem)->GetValue();
                if ( bRecord )
                {
                    // !Hack
                    bChooseOnly = sal_False;
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
            sal_Int16 nTabId = 0;
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
            SFX_ITEMSET_ARG( pIntSet, pFrameItem, SfxUnoFrameItem, SID_FILLFRAME, sal_False );
            if ( pFrameItem )
                xFrame = pFrameItem->GetFrame();

            if ( !xFrame.is() )
            {
                const SfxViewFrame* pViewFrame = SfxViewFrame::Current();
                if ( pViewFrame )
                    xFrame = pViewFrame->GetFrame().GetFrameInterface();
            }

            do  // artificial loop for flow control
            {
                AbstractScriptSelectorDialog* pDlg = pFact->CreateScriptSelectorDialog(
                    lcl_getDialogParent( xFrame, GetTopWindow() ), sal_False, xFrame );
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
#endif // !DISABLE_SCRIPTING

        case SID_OFFICE_CHECK_PLZ:
        {
            sal_Bool bRet = sal_False;
            SFX_REQUEST_ARG(rReq, pStringItem, SfxStringItem, rReq.GetSlot(), sal_False);

            if ( pStringItem )
            {
                String aPLZ = pStringItem->GetValue();
                bRet = sal_True /*!!!SfxIniManager::CheckPLZ( aPLZ )*/;
            }
#ifndef DISABLE_SCRIPTING
            else
                SbxBase::SetError( SbxERR_WRONG_ARGS );
#endif
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
                if ( pSet && pSet->GetItemState( pSetPool->GetWhich( SID_AUTO_CORRECT_DLG ), sal_False, &pItem ) == SFX_ITEM_SET )
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
            Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
            Reference< com::sun::star::frame::XDispatchProvider > xProv(
                xORB->createInstance( ::rtl::OUString("com.sun.star.drawing.ModuleDispatcher")), UNO_QUERY );

            if ( xProv.is() )
            {
                ::rtl::OUString aCmd = ::rtl::OUString::createFromAscii( GetInterface()->GetSlot( rReq.GetSlot() )->GetUnoName() );
                Reference< frame::XDispatchHelper > xHelper( frame::DispatchHelper::create(xContext) );
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
        break;

        case FN_LABEL :
        case FN_BUSINESS_CARD :
        case FN_XFORMS_INIT :
        {
            Reference< com::sun::star::lang::XMultiServiceFactory > xORB = ::comphelper::getProcessServiceFactory();
            Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
            Reference< com::sun::star::frame::XDispatchProvider > xProv(
                xORB->createInstance( ::rtl::OUString("com.sun.star.text.ModuleDispatcher")), UNO_QUERY );

            if ( xProv.is() )
            {
                ::rtl::OUString aCmd = ::rtl::OUString::createFromAscii( GetInterface()->GetSlot( rReq.GetSlot() )->GetUnoName() );
                Reference< frame::XDispatchHelper > xHelper( frame::DispatchHelper::create(xContext) );
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
        break;

        case SID_ADDRESS_DATA_SOURCE:
        {
            ::rtl::OUString sDialogServiceName( "com.sun.star.ui.dialogs.AddressBookSourcePilot"  );
            try
            {
                Reference< com::sun::star::lang::XMultiServiceFactory > xORB = ::comphelper::getProcessServiceFactory();
                Reference< com::sun::star::ui::dialogs::XExecutableDialog > xDialog;
                if (xORB.is())
                    xDialog = Reference< com::sun::star::ui::dialogs::XExecutableDialog >(xORB->createInstance(sDialogServiceName), UNO_QUERY);
                if (xDialog.is())
                    xDialog->execute();
                else
                    ShowServiceNotAvailableError(NULL, sDialogServiceName, sal_True);
            }
            catch(::com::sun::star::uno::Exception&)
            {
            }
        }
        break;

        case SID_COMP_BIBLIOGRAPHY:
        {
            SfxStringItem aURL(SID_FILE_NAME, rtl::OUString(".component:Bibliography/View1"));
            SfxStringItem aRef(SID_REFERER, rtl::OUString("private:user"));
            SfxStringItem aTarget(SID_TARGETNAME, rtl::OUString("_blank"));
            SfxViewFrame::Current()->GetDispatcher()->Execute( SID_OPENDOC, SFX_CALLMODE_ASYNCHRON, &aURL, &aRef, &aTarget, 0L);
        }
        break;
    }
}

void SfxApplication::OfaState_Impl(SfxItemSet &rSet)
{
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/*************************************************************************
 *
 *  $RCSfile: uivwimp.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jp $ $Date: 2001-07-04 18:18:43 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop
#include <cmdid.h>
#include "globals.hrc"


#ifndef _COM_SUN_STAR_SCANNER_XSCANNERMANAGER_HPP_
#include <com/sun/star/scanner/XScannerManager.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDNOTIFIER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARD_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif


#ifndef _UIVWIMP_HXX
#include <uivwimp.hxx>
#endif
#ifndef _SWWVIEW_HXX //autogen
#include <wview.hxx>
#endif
#ifndef _UNOTXVW_HXX
#include <unotxvw.hxx>
#endif
#ifndef _UNODISPATCH_HXX
#include <unodispatch.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _SWDTFLVR_HXX
#include <swdtflvr.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif

#include <view.hrc>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::scanner;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::datatransfer::clipboard;

/* -----------------02.06.98 15:31-------------------
 *
 * --------------------------------------------------*/
SwView_Impl::SwView_Impl(SwView* pShell) :
        pView(pShell),
        pxXTextView(new uno::Reference<view::XSelectionSupplier>),
        eShellMode(SEL_TEXT)
{
    *pxXTextView = new SwXTextView(pView);
    xDisProvInterceptor = new SwXDispatchProviderInterceptor(*pView);
}

/*-----------------13.12.97 09:51-------------------

--------------------------------------------------*/
SwView_Impl::~SwView_Impl()
{
    view::XSelectionSupplier* pTextView = pxXTextView->get();
    ((SwXTextView*)pTextView)->Invalidate();
    delete pxXTextView;
    if( xScanEvtLstnr.is() )
           pScanEvtLstnr->ViewDestroyed();
    if( xClipEvtLstnr.is() )
    {
        pClipEvtLstnr->AddRemoveListener( FALSE );
        pClipEvtLstnr->ViewDestroyed();
    }
}

/*-----------------13.12.97 09:54-------------------

--------------------------------------------------*/
void SwView_Impl::SetShellMode(ShellModes eSet)
{
    eShellMode = eSet;
}
/*-----------------13.12.97 09:59-------------------

--------------------------------------------------*/
view::XSelectionSupplier*   SwView_Impl::GetUNOObject()
{
    return pxXTextView->get();
}
/* -----------------02.06.98 15:29-------------------
 *
 * --------------------------------------------------*/
SwXTextView*    SwView_Impl::GetUNOObject_Impl()
{
        view::XSelectionSupplier* pTextView = pxXTextView->get();
        return ((SwXTextView*)pTextView);
}
/* -----------------------------29.05.00 09:04--------------------------------

 ---------------------------------------------------------------------------*/
void SwView_Impl::ExcuteScan(USHORT nSlot)
{
    switch(nSlot)
    {
        case SID_TWAIN_SELECT:
        {
            BOOL bDone = FALSE;
            Reference< XScannerManager > xScanMgr = SW_MOD()->GetScannerManager();

            if( xScanMgr.is() )
            {
//JP 26.06.00: the appwindow doen't exist
//              Application::GetAppWindow()->EnableInput( FALSE );

                try
                {
                    const Sequence< ScannerContext >
                        aContexts( xScanMgr->getAvailableScanners() );

                    if( aContexts.getLength() )
                    {
                        ScannerContext aContext( aContexts.getConstArray()[ 0 ] );
                        bDone = xScanMgr->configureScanner( aContext );
                    }
                }
                catch(...)
                {
                }

//JP 26.06.00: the appwindow doen't exist
//              Application::GetAppWindow()->EnableInput( TRUE );
            }
            if( !bDone )
                InfoBox( 0, SW_RES(MSG_SCAN_NOSOURCE) ).Execute();
        }
        break;

        case SID_TWAIN_TRANSFER:
        {
            BOOL bDone = FALSE;

            Reference< XScannerManager > xScanMgr = SW_MOD()->GetScannerManager();
            if( xScanMgr.is() )
            {
                SwScannerEventListener& rListener = GetScannerEventListener();
//JP 26.06.00: the appwindow doen't exist
//              Application::GetAppWindow()->EnableInput( FALSE );
                try
                {
                    const Sequence< scanner::ScannerContext >aContexts( xScanMgr->getAvailableScanners() );
                    if( aContexts.getLength() )
                    {
                        Reference< XEventListener > xLstner = &rListener;
                        xScanMgr->startScan( aContexts.getConstArray()[ 0 ], xLstner );
                        bDone = TRUE;
                    }
                }
                catch(...)
                {
                }
            }

            if( !bDone )
            {
//JP 26.06.00: the appwindow doen't exist
//              Application::GetAppWindow()->EnableInput( TRUE );
                InfoBox( 0, SW_RES(MSG_SCAN_NOSOURCE) ).Execute();
            }
            else
            {
                SfxBindings& rBind = pView->GetViewFrame()->GetBindings();
                rBind.Invalidate( SID_TWAIN_SELECT );
                rBind.Invalidate( SID_TWAIN_TRANSFER );
            }
        }
        break;
    }
}

/* -----------------------------29.05.00 08:26--------------------------------

 ---------------------------------------------------------------------------*/
SwScannerEventListener& SwView_Impl::GetScannerEventListener()
{
    if(!xScanEvtLstnr.is())
        xScanEvtLstnr = pScanEvtLstnr = new SwScannerEventListener(*pView);
    return *pScanEvtLstnr;
}


void SwView_Impl::AddClipboardListener()
{
    if(!xClipEvtLstnr.is())
    {
        xClipEvtLstnr = pClipEvtLstnr = new SwClipboardChangeListener( *pView );
        pClipEvtLstnr->AddRemoveListener( TRUE );
    }
}



// ------------------------- SwScannerEventListener ---------------------

SwScannerEventListener::~SwScannerEventListener()
{
}

void SAL_CALL SwScannerEventListener::disposing( const EventObject& rEventObject )
{
    if( pView )
        pView->ScannerEventHdl( rEventObject );
}

// ------------------------- SwClipboardChangeListener ---------------------

SwClipboardChangeListener::~SwClipboardChangeListener()
{
}

void SAL_CALL SwClipboardChangeListener::disposing(
                                            const EventObject& rEventObject )
{
}

void SAL_CALL SwClipboardChangeListener::changedContents(
                            const CLIP_NMSPC::ClipboardEvent& rEventObject )
{
    if( pView )
    {
        {
        const ::vos::OGuard aGuard( Application::GetSolarMutex() );

        TransferableDataHelper aDataHelper( rEventObject.Contents );
        SwWrtShell& rSh = pView->GetWrtShell();

        pView->nLastPasteDestination = SwTransferable::GetSotDestination( rSh );
        pView->bPasteState = aDataHelper.GetTransferable().is() &&
                        SwTransferable::IsPaste( rSh, aDataHelper );

        pView->bPasteSpecialState = aDataHelper.GetTransferable().is() &&
                    SwTransferable::IsPasteSpecial( rSh, aDataHelper );
        }

        SfxBindings& rBind = pView->GetViewFrame()->GetBindings();
        rBind.Invalidate( SID_PASTE );
        rBind.Invalidate( FN_PASTESPECIAL );
    }
}

void SwClipboardChangeListener::AddRemoveListener( BOOL bAdd )
{
    try
    {
        do {

#ifdef _DONT_WORD_FOR_WEBTOP_
JP 4.7.2001: change for WebTop - get Clipboard from the Window.
            Reference< XMultiServiceFactory > xFact(
                                ::comphelper::getProcessServiceFactory() );
            if( !xFact.is() )
                break;
            Reference< XClipboard > xClipboard( xFact->createInstance(
                ::rtl::OUString::createFromAscii(
                    "com.sun.star.datatransfer.clipboard.SystemClipboard" )),
                UNO_QUERY );
#else
            Reference< XClipboard > xClipboard(
                    pView->GetEditWin().GetClipboard() );
#endif
            if( !xClipboard.is() )
                break;

            Reference< XClipboardNotifier > xClpbrdNtfr( xClipboard, UNO_QUERY );
            if( xClpbrdNtfr.is() )
            {
                Reference< XClipboardListener > xClipEvtLstnr( this );
                if( bAdd )
                    xClpbrdNtfr->addClipboardListener( xClipEvtLstnr );
                else
                    xClpbrdNtfr->removeClipboardListener( xClipEvtLstnr );
            }
        }  while ( FALSE );
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }
}


/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.3  2001/04/30 15:59:56  jp
    use Clipboard state listener instead of polling

    Revision 1.2  2000/11/07 14:38:25  os
    Dispatch interface for database component

    Revision 1.1.1.1  2000/09/18 17:14:49  hr
    initial import

    Revision 1.14  2000/09/18 16:06:11  willem.vandorp
    OpenOffice header added.

    Revision 1.13  2000/09/07 15:59:32  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.12  2000/06/26 10:44:13  jp
    must change: GetAppWindow->GetDefaultDevice

    Revision 1.11  2000/05/29 08:03:23  os
    new scanner interface

    Revision 1.10  2000/05/09 14:43:13  os
    BASIC interface partially removed

    Revision 1.9  2000/03/23 07:50:24  os
    UNO III

    Revision 1.8  1999/01/27 08:58:26  OS
    #56371# TF_ONE51


      Rev 1.7   27 Jan 1999 09:58:26   OS
   #56371# TF_ONE51

      Rev 1.6   30 Sep 1998 11:37:50   OS
   #52654# ueberfluessigen Aufruf entfernt

      Rev 1.5   02 Jun 1998 15:51:16   OS
   TF_STARONE raus; Ctor nicht mehr inline

      Rev 1.4   03 Apr 1998 14:38:28   OS
   UnoObject fuer die View reaktiviert

      Rev 1.3   04 Feb 1998 17:28:44   OS
   Starone raus

      Rev 1.2   29 Jan 1998 09:21:06   OS
   TF_STARONE

      Rev 1.1   19 Jan 1998 14:59:14   OS
   UNO-Aenderungen

      Rev 1.0   16 Dec 1997 11:58:48   OS
   Impl-Pointer fuer UNO

------------------------------------------------------------------------*/



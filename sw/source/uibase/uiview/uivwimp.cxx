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

#include <config_features.h>

#include <cmdid.h>
#include "globals.hrc"

#include <com/sun/star/scanner/XScannerManager2.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <osl/mutex.hxx>
#include <vcl/layout.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>

#include <sfx2/docinsert.hxx>
#include <sfx2/request.hxx>
#include <uivwimp.hxx>
#include <wview.hxx>
#include <unotxvw.hxx>
#include <unodispatch.hxx>
#include <swmodule.hxx>
#include <swdtflvr.hxx>
#include <edtwin.hxx>
#include <mmconfigitem.hxx>

#include <view.hrc>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::scanner;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::datatransfer::clipboard;

SwView_Impl::SwView_Impl(SwView* pShell)
    : mxXTextView()
    , pView(pShell)
    , pScanEvtLstnr(nullptr)
    , pClipEvtLstnr(nullptr)
    , eShellMode(SHELL_MODE_TEXT)
#if HAVE_FEATURE_DBCONNECTIVITY
    , pConfigItem(nullptr)
    , nMailMergeRestartPage(0)
    , bMailMergeSourceView(true)
#endif
    , m_pDocInserter(nullptr)
    , m_pRequest(nullptr)
    , m_nParam(0)
    , m_bSelectObject(false)
    , m_bEditingPositionSet(false)
{
    mxXTextView = new SwXTextView(pView);
    xDisProvInterceptor = new SwXDispatchProviderInterceptor(*pView);
}

SwView_Impl::~SwView_Impl()
{
    Reference<XUnoTunnel> xDispTunnel(xDisProvInterceptor, UNO_QUERY);
    SwXDispatchProviderInterceptor* pInterceptor = nullptr;
    if(xDispTunnel.is() &&
        nullptr != (pInterceptor = reinterpret_cast< SwXDispatchProviderInterceptor * >(
                    sal::static_int_cast< sal_IntPtr >(
                    xDispTunnel->getSomething(SwXDispatchProviderInterceptor::getUnoTunnelId())))))
    {
        pInterceptor->Invalidate();
    }
    view::XSelectionSupplier* pTextView = mxXTextView.get();
    static_cast<SwXTextView*>(pTextView)->Invalidate();
    mxXTextView.clear();
    if( xScanEvtLstnr.is() )
           pScanEvtLstnr->ViewDestroyed();
    if( xClipEvtLstnr.is() )
    {
        pClipEvtLstnr->AddRemoveListener( false );
        pClipEvtLstnr->ViewDestroyed();
    }
#if HAVE_FEATURE_DBCONNECTIVITY
    if(bMailMergeSourceView)
        delete pConfigItem;
#endif
    delete m_pDocInserter;
    delete m_pRequest;
}

void SwView_Impl::SetShellMode(ShellModes eSet)
{
    eShellMode = eSet;
}

view::XSelectionSupplier*   SwView_Impl::GetUNOObject()
{
    return mxXTextView.get();
}

SwXTextView*    SwView_Impl::GetUNOObject_Impl()
{
    view::XSelectionSupplier* pTextView = mxXTextView.get();
    return static_cast<SwXTextView*>(pTextView);
}

void SwView_Impl::ExecuteScan( SfxRequest& rReq )
{
    switch(rReq.GetSlot())
    {
        case SID_TWAIN_SELECT:
        {
            bool bDone = false;
            Reference< XScannerManager2 > xScanMgr = SW_MOD()->GetScannerManager();

            if( xScanMgr.is() )
            {
                try
                {
                    SwScannerEventListener& rListener = GetScannerEventListener();
                    const Sequence< ScannerContext >
                        aContexts( xScanMgr->getAvailableScanners() );

                    if( aContexts.getLength() )
                    {
                        Reference< XEventListener > xLstner = &rListener;
                        ScannerContext aContext( aContexts.getConstArray()[ 0 ] );
                        bDone = xScanMgr->configureScannerAndScan( aContext, xLstner );
                    }
                }
                catch(...)
                {
                }

            }
            if( bDone )
                rReq.Done();
            else
            {
                rReq.Ignore();
            }
        }
        break;

        case SID_TWAIN_TRANSFER:
        {
            bool bDone = false;

            Reference< XScannerManager2 > xScanMgr = SW_MOD()->GetScannerManager();
            if( xScanMgr.is() )
            {
                SwScannerEventListener& rListener = GetScannerEventListener();
                try
                {
                    const Sequence< scanner::ScannerContext >aContexts( xScanMgr->getAvailableScanners() );
                    if( aContexts.getLength() )
                    {
                        Reference< XEventListener > xLstner = &rListener;
                        xScanMgr->startScan( aContexts.getConstArray()[ 0 ], xLstner );
                        bDone = true;
                    }
                }
                catch(...)
                {
                }
            }

            if( !bDone )
            {
                ScopedVclPtrInstance<MessageDialog>( nullptr, SW_RES(STR_SCAN_NOSOURCE), VCL_MESSAGE_INFO )->Execute();
                rReq.Ignore();
            }
            else
            {
                rReq.Done();
                SfxBindings& rBind = pView->GetViewFrame()->GetBindings();
                rBind.Invalidate( SID_TWAIN_SELECT );
                rBind.Invalidate( SID_TWAIN_TRANSFER );
            }
        }
        break;
    }
}

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
        pClipEvtLstnr->AddRemoveListener( true );
    }
}

void SwView_Impl::Invalidate()
{
    GetUNOObject_Impl()->Invalidate();
    Reference< XUnoTunnel > xTunnel(xTransferable.get(), UNO_QUERY);
    if(xTunnel.is())

    {
        SwTransferable* pTransferable = reinterpret_cast< SwTransferable * >(
                sal::static_int_cast< sal_IntPtr >(
                xTunnel->getSomething(SwTransferable::getUnoTunnelId())));
        if(pTransferable)
            pTransferable->Invalidate();
    }
}

void SwView_Impl::AddTransferable(SwTransferable& rTransferable)
{
    //prevent removing of the non-referenced SwTransferable
    rTransferable.m_refCount++;
    {
        xTransferable = Reference<XUnoTunnel> (&rTransferable);
    }
    rTransferable.m_refCount--;
}

void SwView_Impl::StartDocumentInserter( const OUString& rFactory, const Link<sfx2::FileDialogHelper*,void>& rEndDialogHdl )
{
    delete m_pDocInserter;
    m_pDocInserter = new ::sfx2::DocumentInserter( rFactory );
    m_pDocInserter->StartExecuteModal( rEndDialogHdl );
}

SfxMedium* SwView_Impl::CreateMedium()
{
    return m_pDocInserter->CreateMedium();
}

void SwView_Impl::InitRequest( const SfxRequest& rRequest )
{
    delete m_pRequest;
    m_pRequest = new SfxRequest( rRequest );
}

SwScannerEventListener::~SwScannerEventListener()
{
}

void SAL_CALL SwScannerEventListener::disposing( const EventObject& rEventObject) throw(uno::RuntimeException, std::exception)
{
#if defined(_WIN32) || defined UNX
    SolarMutexGuard aGuard;
    if( pView )
        pView->ScannerEventHdl( rEventObject );
#endif
}

SwClipboardChangeListener::~SwClipboardChangeListener()
{
}

void SAL_CALL SwClipboardChangeListener::disposing( const EventObject& /*rEventObject*/ )
    throw ( RuntimeException, std::exception )
{
}

void SAL_CALL SwClipboardChangeListener::changedContents( const css::datatransfer::clipboard::ClipboardEvent& rEventObject )
    throw (RuntimeException, std::exception)

{
    const SolarMutexGuard aGuard;
    if( pView )
    {
        {
            TransferableDataHelper aDataHelper( rEventObject.Contents );
            SwWrtShell& rSh = pView->GetWrtShell();

            pView->m_nLastPasteDestination = SwTransferable::GetSotDestination( rSh );
            pView->m_bPasteState = aDataHelper.GetXTransferable().is() &&
                            SwTransferable::IsPaste( rSh, aDataHelper );

            pView->m_bPasteSpecialState = aDataHelper.GetXTransferable().is() &&
                        SwTransferable::IsPasteSpecial( rSh, aDataHelper );
        }

        SfxBindings& rBind = pView->GetViewFrame()->GetBindings();
        rBind.Invalidate( SID_PASTE );
        rBind.Invalidate( SID_PASTE_SPECIAL );
        rBind.Invalidate( SID_CLIPBOARD_FORMAT_ITEMS );
    }
}

void SwClipboardChangeListener::AddRemoveListener( bool bAdd )
{
    pView->AddRemoveClipboardListener( Reference< XClipboardListener >( this ), bAdd );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

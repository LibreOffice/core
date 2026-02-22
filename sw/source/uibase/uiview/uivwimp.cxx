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
#include <config_fuzzers.h>

#include <cmdid.h>

#include <com/sun/star/scanner/XScannerManager2.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <comphelper/propertysequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <vcl/weld/MessageDialog.hxx>
#include <vcl/weld/weld.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/docfile.hxx>

#include <sfx2/docinsert.hxx>
#include <sfx2/request.hxx>
#include <uivwimp.hxx>
#include <unotxvw.hxx>
#include <unodispatch.hxx>
#include <swmodule.hxx>
#include <swdtflvr.hxx>

#include <strings.hrc>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::scanner;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::datatransfer::clipboard;

SwView_Impl::SwView_Impl(SwView* pShell)
    : m_pView(pShell)
    , m_eShellMode(ShellMode::Text)
    , m_nParam(0)
    , m_bSelectObject(false)
    , m_bEditingPositionSet(false)
{
    mxXTextView = new SwXTextView(m_pView);
    m_xDispatchProviderInterceptor = new SwXDispatchProviderInterceptor(*m_pView);
}

SwView_Impl::~SwView_Impl()
{
    if(m_xDispatchProviderInterceptor)
        m_xDispatchProviderInterceptor->Invalidate();
    mxXTextView->Invalidate();
    mxXTextView.clear();

    if( mxScanEvtLstnr.is() )
           mxScanEvtLstnr->ViewDestroyed();
    if( mxClipEvtLstnr.is() )
    {
        mxClipEvtLstnr->AddRemoveListener( false );
        mxClipEvtLstnr->ViewDestroyed();
    }
    DisconnectTransferableDDE();

#if HAVE_FEATURE_DBCONNECTIVITY && !ENABLE_FUZZERS
    m_xConfigItem.reset();
#endif
    m_pDocInserter.reset();
    m_pRequest.reset();
}

void SwView_Impl::SetShellMode(ShellMode eSet)
{
    m_eShellMode = eSet;
}

view::XSelectionSupplier*   SwView_Impl::GetUNOObject()
{
    return mxXTextView.get();
}

SwXTextView*    SwView_Impl::GetUNOObject_Impl()
{
    return mxXTextView.get();
}

void SwView_Impl::ExecuteScan( SfxRequest& rReq )
{
    switch(rReq.GetSlot())
    {
        case SID_TWAIN_SELECT:
        {
            bool bDone = false;
            Reference<XScannerManager2> xScanMgr = SwModule::get()->GetScannerManager();

            if( xScanMgr.is() )
            {
                try
                {
                    SwScannerEventListener& rListener = GetScannerEventListener();
                    const Sequence< ScannerContext >
                        aContexts( xScanMgr->getAvailableScanners() );

                    if( aContexts.hasElements() )
                    {
                        Reference< XEventListener > xLstner = &rListener;
                        ScannerContext aContext( aContexts.getConstArray()[ 0 ] );

                        Reference<lang::XInitialization> xInit(xScanMgr, UNO_QUERY);
                        if (xInit.is())
                        {
                            //  initialize dialog
                            weld::Window* pWindow = rReq.GetFrameWeld();
                            uno::Sequence<uno::Any> aSeq(comphelper::InitAnyPropertySequence(
                            {
                                {"ParentWindow", pWindow ? uno::Any(pWindow->GetXWindow()) : uno::Any(Reference<awt::XWindow>())}
                            }));
                            xInit->initialize( aSeq );
                        }

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

            Reference<XScannerManager2> xScanMgr = SwModule::get()->GetScannerManager();
            if( xScanMgr.is() )
            {
                SwScannerEventListener& rListener = GetScannerEventListener();
                try
                {
                    const Sequence< scanner::ScannerContext >aContexts( xScanMgr->getAvailableScanners() );
                    if( aContexts.hasElements() )
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
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(rReq.GetFrameWeld(),
                                                          VclMessageType::Info, VclButtonsType::Ok,
                                                          SwResId(STR_SCAN_NOSOURCE)));
                xBox->run();
                rReq.Ignore();
            }
            else
            {
                rReq.Done();
                SfxBindings& rBind = m_pView->GetViewFrame().GetBindings();
                rBind.Invalidate( SID_TWAIN_SELECT );
                rBind.Invalidate( SID_TWAIN_TRANSFER );
            }
        }
        break;
    }
}

SwScannerEventListener& SwView_Impl::GetScannerEventListener()
{
    if(!mxScanEvtLstnr.is())
        mxScanEvtLstnr = new SwScannerEventListener(*m_pView);
    return *mxScanEvtLstnr;
}

void SwView_Impl::AddClipboardListener()
{
    if(!mxClipEvtLstnr.is())
    {
        mxClipEvtLstnr = new SwClipboardChangeListener( *m_pView );
        mxClipEvtLstnr->AddRemoveListener( true );
    }
}

void SwView_Impl::Invalidate()
{
    GetUNOObject_Impl()->Invalidate();
    for (const auto& xTransferable: mxTransferables)
    {
        rtl::Reference<SwTransferable> pTransferable = xTransferable.get();
        if(pTransferable)
            pTransferable->Invalidate();
    }
}

void SwView_Impl::DisconnectTransferableDDE()
{
    for (const auto& xTransferable: mxTransferables)
    {
        rtl::Reference<SwTransferable> pTransferable = xTransferable.get();
        if(pTransferable)
            pTransferable->DisconnectDDE();
    }
}

void SwView_Impl::AddTransferable(SwTransferable& rTransferable)
{
    //prevent removing of the non-referenced SwTransferable
    osl_atomic_increment(&rTransferable.m_refCount);
    {
        // Remove previously added, but no longer existing weak references.
        std::erase_if(mxTransferables,
            [](const unotools::WeakReference<SwTransferable>& rTunnel) {
                return !rTunnel.get();
            });

        mxTransferables.emplace_back(&rTransferable);
    }
    osl_atomic_decrement(&rTransferable.m_refCount);
}

void SwView_Impl::StartDocumentInserter(
    const OUString& rFactory,
    const Link<sfx2::FileDialogHelper*,void>& rEndDialogHdl,
    const sal_uInt16 nSlotId
)
{
    sfx2::DocumentInserter::Mode mode {sfx2::DocumentInserter::Mode::Insert};
    switch( nSlotId )
    {
        case SID_DOCUMENT_MERGE:
            mode = sfx2::DocumentInserter::Mode::Merge;
            break;
        case SID_DOCUMENT_COMPARE:
            mode = sfx2::DocumentInserter::Mode::Compare;
            break;
        default:
            break;
    }

    m_pDocInserter.reset(new ::sfx2::DocumentInserter(m_pView->GetFrameWeld(), rFactory, mode));
    m_pDocInserter->StartExecuteModal( rEndDialogHdl );
}

std::unique_ptr<SfxMedium> SwView_Impl::CreateMedium()
{
    return m_pDocInserter->CreateMedium();
}

void SwView_Impl::InitRequest( const SfxRequest& rRequest )
{
    m_pRequest.reset(new SfxRequest( rRequest ));
}

SwScannerEventListener::~SwScannerEventListener()
{
}

void SAL_CALL SwScannerEventListener::disposing( const EventObject& /*rEventObject*/)
{
#if defined(_WIN32) || defined UNX
    SolarMutexGuard aGuard;
    if( m_pView )
        m_pView->ScannerEventHdl();
#endif
}

SwClipboardChangeListener::~SwClipboardChangeListener()
{
}

void SAL_CALL SwClipboardChangeListener::disposing( const EventObject& /*rEventObject*/ )
{
    SolarMutexGuard aGuard;
    m_pView = nullptr; // so we don't touch the view if changedContents somehow fires afterwards
}

void SAL_CALL SwClipboardChangeListener::changedContents( const css::datatransfer::clipboard::ClipboardEvent& rEventObject )

{
    const SolarMutexGuard aGuard;
    if( !m_pView )
        return;

    {
        TransferableDataHelper aDataHelper( rEventObject.Contents );
        SwWrtShell& rSh = m_pView->GetWrtShell();

        m_pView->m_nLastPasteDestination = SwTransferable::GetSotDestination( rSh );
        m_pView->m_bPasteState = aDataHelper.GetXTransferable().is() &&
                        SwTransferable::IsPaste( rSh, aDataHelper );

        m_pView->m_bPasteSpecialState = aDataHelper.GetXTransferable().is() &&
                    SwTransferable::IsPasteSpecial( rSh, aDataHelper );
    }

    SfxBindings& rBind = m_pView->GetViewFrame().GetBindings();
    rBind.Invalidate( SID_PASTE );
    rBind.Invalidate( SID_PASTE_SPECIAL );
    rBind.Invalidate( SID_CLIPBOARD_FORMAT_ITEMS );
}

void SwClipboardChangeListener::AddRemoveListener( bool bAdd )
{
    m_pView->AddRemoveClipboardListener( Reference< XClipboardListener >( this ), bAdd );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

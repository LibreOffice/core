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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_UIVWIMP_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_UIVWIMP_HXX

#include <view.hxx>

#include <sfx2/objsh.hxx>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <unotools/weakref.hxx>

class SwXTextView;
class SfxRequest;
class SwTransferable;

namespace sfx2 { class DocumentInserter; }

class SwScannerEventListener final : public ::cppu::WeakImplHelper<
    css::lang::XEventListener >
{
    SwView* m_pView;

public:

    SwScannerEventListener( SwView& rView ) : m_pView( &rView )  {}
    virtual ~SwScannerEventListener() override;

    // XEventListener
    virtual void SAL_CALL disposing(
                    const css::lang::EventObject& rEventObject ) override;

    void ViewDestroyed() { m_pView = nullptr; }
};

// Clipboard EventListener
class SwClipboardChangeListener final : public ::cppu::WeakImplHelper<
    css::datatransfer::clipboard::XClipboardListener >
{
    SwView* m_pView;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& rEventObject ) override;

    // XClipboardListener
    virtual void SAL_CALL changedContents( const css::datatransfer::clipboard::ClipboardEvent& rEventObject ) override;

public:
    SwClipboardChangeListener( SwView& rView ) : m_pView( &rView ) {}
    virtual ~SwClipboardChangeListener() override;

    void ViewDestroyed() { m_pView = nullptr; }

    void AddRemoveListener( bool bAdd );
};

class SwMailMergeConfigItem;
class SwXDispatchProviderInterceptor;

class SwView_Impl
{
    rtl::Reference< SwXDispatchProviderInterceptor > m_xDispatchProviderInterceptor;
    rtl::Reference< SwXTextView > mxXTextView;       // UNO object
    std::vector< unotools::WeakReference< SwTransferable > > mxTransferables;

    // temporary document for printing text of selection / multi selection
    // in PDF export.
    SfxObjectShellLock          m_xTmpSelDocShell;

    SwView*                     m_pView;
    rtl::Reference<SwScannerEventListener>
                                mxScanEvtLstnr;
    rtl::Reference<SwClipboardChangeListener>
                                mxClipEvtLstnr;
    ShellMode                   m_eShellMode;

    std::shared_ptr<SwMailMergeConfigItem>
                                m_xConfigItem;

    std::unique_ptr<sfx2::DocumentInserter> m_pDocInserter;
    std::unique_ptr<SfxRequest>             m_pRequest;
    sal_Int16                   m_nParam;

    Point                       m_aEditingPosition;
    bool                        m_bSelectObject;
    bool                        m_bEditingPositionSet;

    void                        DisconnectTransferableDDE();

public:
    /// Redline author that's specific to this view.
    OUString m_sRedlineAuthor;

    SwView_Impl(SwView* pShell);
    ~SwView_Impl();

    void                            SetShellMode(ShellMode eSet);

    css::view::XSelectionSupplier* GetUNOObject();
    SwXTextView*                    GetUNOObject_Impl();
    void                            Invalidate();

    ShellMode                       GetShellMode() const {return m_eShellMode;}

    void                            ExecuteScan(SfxRequest& rReq);
    SwScannerEventListener&         GetScannerEventListener();

    void                            AddClipboardListener();

    void                            AddTransferable(SwTransferable& rTransferable);

    void SetMailMergeConfigItem(std::shared_ptr<SwMailMergeConfigItem> const & rItem)
    {
        m_xConfigItem = rItem;
    }
    std::shared_ptr<SwMailMergeConfigItem> const & GetMailMergeConfigItem() const {return m_xConfigItem;}

    //#i33307# restore editing position
    void                    SetRestorePosition(const Point& rCursorPos, bool bSelectObj)
                            {
                                m_aEditingPosition = rCursorPos;
                                m_bSelectObject = bSelectObj;
                                m_bEditingPositionSet = true;
                            }
    bool                    GetRestorePosition(Point& rCursorPos, bool& rbSelectObj)
                            {
                                rCursorPos = m_aEditingPosition;
                                rbSelectObj = m_bSelectObject;
                                return m_bEditingPositionSet;
                            }

    void                    StartDocumentInserter(
                                const OUString& rFactory,
                                const Link<sfx2::FileDialogHelper*,void>& rEndDialogHdl,
                                const sal_uInt16 nSlotId
                            );
    std::unique_ptr<SfxMedium> CreateMedium();
    void                    InitRequest( const SfxRequest& rRequest );

    SfxRequest*      GetRequest() const { return m_pRequest.get(); }
    sal_Int16        GetParam() const { return m_nParam; }
    void             SetParam( sal_Int16 nParam ) { m_nParam = nParam; }
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

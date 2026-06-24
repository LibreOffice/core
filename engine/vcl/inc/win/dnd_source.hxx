/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#include <com/sun/star/datatransfer/dnd/XDragSourceContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <oleidl.h>

#include <systools/win32/comtools.hxx>

namespace com::sun::star::uno { class XComponentContext; }

class SourceContext;
// RIGHT MOUSE BUTTON drag and drop not supported currently.
// ALT modifier is considered to effect a user selection of effects
class DragSource:
      public cppu::BaseMutex,
      public cppu::WeakComponentImplHelper<css::datatransfer::dnd::XDragSource,
                                           css::lang::XInitialization,
                                           css::lang::XServiceInfo>,
      public IDropSource

{
    css::uno::Reference<css::uno::XComponentContext> m_xContext;
    HWND m_hAppWindow;

    // The mouse button that set off the drag and drop operation
    short m_MouseButton;

    // First starting a new drag and drop thread if
    // the last one has finished
    void StartDragImpl(
        const css::datatransfer::dnd::DragGestureEvent& trigger,
        sal_Int8 sourceActions,
        sal_Int32 cursor,
        sal_Int32 image,
        const css::uno::Reference<css::datatransfer::XTransferable >& trans,
        const css::uno::Reference<css::datatransfer::dnd::XDragSourceListener >& listener);

public:
    LONG m_RunningDndOperationCount;

public:
    // only valid for one dnd operation
    // the thread ID of the thread which created the window
    DWORD m_threadIdWindow;
    // The context notifies the XDragSourceListener s
    css::uno::Reference<css::datatransfer::dnd::XDragSourceContext> m_currentContext;

    // the wrapper for the Transferable ( startDrag)
    sal::systools::COMReference<IDataObject> m_spDataObject;

    sal_Int8 m_sourceActions;

public:
    explicit DragSource(const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~DragSource() override;
    DragSource(const DragSource&) = delete;
    DragSource &operator= ( const DragSource&) = delete;

  // XInitialization
    virtual void SAL_CALL initialize(const css::uno::Sequence<cpo::uno::Any>& aArguments) override;

    // XDragSource
    virtual bool SAL_CALL isDragImageSupported(  ) override;
    virtual sal_Int32 SAL_CALL getDefaultCursor( sal_Int8 dragAction ) override;
    virtual void SAL_CALL startDrag( const css::datatransfer::dnd::DragGestureEvent& trigger,
                                     sal_Int8 sourceActions,
                                     sal_Int32 cursor,
                                     sal_Int32 image,
                                     const css::uno::Reference<css::datatransfer::XTransferable>& trans,
                                     const css::uno::Reference<css::datatransfer::dnd::XDragSourceListener>& listener ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject) override;

    virtual ULONG STDMETHODCALLTYPE AddRef( ) override;

    virtual ULONG STDMETHODCALLTYPE Release( ) override;

    // IDropSource
    virtual HRESULT STDMETHODCALLTYPE QueryContinueDrag(
        /* [in] */ BOOL fEscapePressed,
        /* [in] */ DWORD grfKeyState) override;

    virtual HRESULT STDMETHODCALLTYPE GiveFeedback(
        /* [in] */ DWORD dwEffect) override;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

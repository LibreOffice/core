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
#pragma once

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/datatransfer/dnd/DropTargetDragEnterEvent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <osl/mutex.hxx>

#include <oleidl.h>
#include "globals.hxx"

namespace com::sun::star::uno { class XComponentContext; }

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace cppu;
using namespace osl;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::dnd;

// The client
// has to call XComponent::dispose. The thread that calls initialize
// must also execute the destruction of the instance. This is because
// initialize calls OleInitialize and the destructor calls OleUninitialize.
// If the service calls OleInitialize then it also calls OleUnitialize when
// it is destroyed. Therefore no second instance may exist which was
// created in the same thread and still needs OLE.
class DropTarget: public MutexDummy,
                  public WeakComponentImplHelper< XInitialization, XDropTarget, XServiceInfo>

{
private:
    friend DWORD WINAPI DndTargetOleSTAFunc(LPVOID pParams);
    // The native window which acts as drop target.
    // It is set in initialize. In case RegisterDragDrop fails it is set
    // to NULL
    HWND m_hWnd; // set by initialize
    // Holds the thread id of the thread which created the window that is the
    // drop target. Only used when DropTarget::initialize is called from an MTA
    // thread
    DWORD m_threadIdWindow;
    // This is the thread id of the OLE thread that is created in DropTarget::initialize
    // when the calling thread is an MTA
    DWORD m_threadIdTarget;
    // The handle of the thread that is created in DropTarget::initialize
    // when the calling thread is an MTA
    HANDLE m_hOleThread;
    // The thread id of the thread which called initialize. When the service dies
    // than m_oleThreadId is used to determine if the service successfully called
    // OleInitialize. If so then OleUninitialize has to be called.
    DWORD m_oleThreadId;
    // An Instance of IDropTargetImpl which receives calls from the system's drag
    // and drop implementation. It delegate the calls to name alike functions in
    // this class.
    IDropTarget* m_pDropTarget;

    Reference<XComponentContext> m_xContext;
    // If m_bActive == sal_True then events are fired to XDropTargetListener s,
    // none otherwise. The default value is sal_True.
    bool m_bActive;
    sal_Int8    m_nDefaultActions;

    // This value is set when a XDropTargetListener calls accept or reject on
    // the XDropTargetDropContext or  XDropTargetDragContext.
    // The values are from the DNDConstants group.
    sal_Int8 m_nCurrentDropAction;
    // This value is manipulated by the XDropTargetListener
    sal_Int8 m_nLastDropAction;

    Reference<XTransferable> m_currentData;
    // The current action is used to determine if the USER
    // action has changed (dropActionChanged)
//  sal_Int8 m_userAction;
    // Set by listeners when they call XDropTargetDropContext::dropComplete
    bool m_bDropComplete;
    Reference<XDropTargetDragContext> m_currentDragContext;
    Reference<XDropTargetDropContext> m_currentDropContext;

public:
    explicit DropTarget(const Reference<XComponentContext>& rxContext);
    virtual ~DropTarget() override;
    DropTarget(DropTarget const &) = delete;
    DropTarget &operator= (DropTarget const &) = delete;

    // Overrides WeakComponentImplHelper::disposing which is called by
    // WeakComponentImplHelper::dispose
    // Must be called.
    virtual void SAL_CALL disposing() override;
   // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) override;

    // XDropTarget
    virtual void SAL_CALL addDropTargetListener( const Reference< XDropTargetListener >& dtl ) override;
    virtual void SAL_CALL removeDropTargetListener( const Reference< XDropTargetListener >& dtl ) override;
    // Default is not active
    virtual sal_Bool SAL_CALL isActive(  ) override;
    virtual void SAL_CALL setActive( sal_Bool isActive ) override;
    virtual sal_Int8 SAL_CALL getDefaultActions(  ) override;
    virtual void SAL_CALL setDefaultActions( sal_Int8 actions ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // Functions called from the IDropTarget implementation ( m_pDropTarget)
    virtual HRESULT DragEnter(
            /* [unique][in] */ IDataObject *pDataObj,
            /* [in] */ DWORD grfKeyState,
            /* [in] */ POINTL pt,
            /* [out][in] */ DWORD *pdwEffect);

    virtual HRESULT STDMETHODCALLTYPE DragOver(
            /* [in] */ DWORD grfKeyState,
            /* [in] */ POINTL pt,
            /* [out][in] */ DWORD *pdwEffect);

    virtual HRESULT STDMETHODCALLTYPE DragLeave( ) ;

    virtual HRESULT STDMETHODCALLTYPE Drop(
            /* [unique][in] */ IDataObject *pDataObj,
            /* [in] */ DWORD grfKeyState,
            /* [in] */ POINTL pt,
            /* [out][in] */ DWORD *pdwEffect);

// Non - interface functions --------------------------------------------------
// XDropTargetDropContext delegated from DropContext

    void _acceptDrop( sal_Int8 dropOperation, const Reference<XDropTargetDropContext>& context);
    void _rejectDrop( const Reference<XDropTargetDropContext>& context);
    void _dropComplete( bool success, const Reference<XDropTargetDropContext>& context);

// XDropTargetDragContext delegated from DragContext
    void _acceptDrag( sal_Int8 dragOperation, const Reference<XDropTargetDragContext>& context);
    void _rejectDrag( const Reference<XDropTargetDragContext>& context);

protected:
    // Gets the current action dependent on the pressed modifiers, the effects
    // supported by the drop source (IDropSource) and the default actions of the
    // drop target (XDropTarget, this class))
    inline sal_Int8 getFilteredActions( DWORD grfKeyState, DWORD sourceActions);
    // Only filters with the default actions
    inline sal_Int8 getFilteredActions( DWORD grfKeyState);

    void fire_drop( const DropTargetDropEvent& dte);
    void fire_dragEnter( const DropTargetDragEnterEvent& dtde );
    void fire_dragExit( const DropTargetEvent& dte );
    void fire_dragOver( const DropTargetDragEvent& dtde );
    void fire_dropActionChanged( const DropTargetDragEvent& dtde );

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

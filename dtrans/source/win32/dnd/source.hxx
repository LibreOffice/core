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

#ifndef _SOURCE_HXX_
#define _SOURCE_HXX_

#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#include <com/sun/star/datatransfer/dnd/XDragSourceContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/compbase3.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include "../../inc/DtObjFactory.hxx"
#include "globals.hxx"
#include <oleidl.h>

#include <systools/win32/comtools.hxx>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace cppu;
using namespace osl;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::dnd;

class SourceContext;
// RIGHT MOUSE BUTTON drag and drop not supportet currently.
// ALT modifier is considered to effect a user selection of effects
class DragSource:
      public MutexDummy,
      public WeakComponentImplHelper3<XDragSource, XInitialization, XServiceInfo>,
      public IDropSource

{
    Reference<XComponentContext> m_xContext;
    HWND m_hAppWindow;

    // The mouse button that set off the drag and drop operation
    short m_MouseButton;
    // Converts XTransferable objects to IDataObject objects.
    CDTransObjFactory m_aDataConverter;

    DragSource();
    DragSource(const DragSource&);
    DragSource &operator= ( const DragSource&);

    // First starting a new drag and drop thread if
    // the last one has finished
    void StartDragImpl(
        const DragGestureEvent& trigger,
        sal_Int8 sourceActions,
        sal_Int32 cursor,
        sal_Int32 image,
        const Reference<XTransferable >& trans,
        const Reference<XDragSourceListener >& listener);

public:
    long m_RunningDndOperationCount;

public:
    // only valid for one dnd operation
    // the thread ID of the thread which created the window
    DWORD m_threadIdWindow;
    // The context notifies the XDragSourceListener s
    Reference<XDragSourceContext>   m_currentContext;

    // the wrapper for the Transferable ( startDrag)
    IDataObjectPtr m_spDataObject;

    sal_Int8 m_sourceActions;

public:
    DragSource(const Reference<XComponentContext>& rxContext);
    virtual ~DragSource();

  // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments )
        throw(Exception, RuntimeException);


    // XDragSource
    virtual sal_Bool SAL_CALL isDragImageSupported(  ) throw(RuntimeException);
    virtual sal_Int32 SAL_CALL getDefaultCursor( sal_Int8 dragAction )
        throw( IllegalArgumentException, RuntimeException);
    virtual void SAL_CALL startDrag( const DragGestureEvent& trigger,
                                     sal_Int8 sourceActions,
                                     sal_Int32 cursor,
                                     sal_Int32 image,
                                     const Reference<XTransferable >& trans,
                                     const Reference<XDragSourceListener >& listener )
                throw( RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (RuntimeException);



    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);

    virtual ULONG STDMETHODCALLTYPE AddRef( );

    virtual ULONG STDMETHODCALLTYPE Release( );


    // IDropSource
    virtual HRESULT STDMETHODCALLTYPE QueryContinueDrag(
        /* [in] */ BOOL fEscapePressed,
        /* [in] */ DWORD grfKeyState);

    virtual HRESULT STDMETHODCALLTYPE GiveFeedback(
        /* [in] */ DWORD dwEffect);

};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

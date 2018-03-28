/**************************************************************
*
* Licensed to the Apache Software Foundation (ASF) under one
* or more contributor license agreements.  See the NOTICE file
* distributed with this work for additional information
* regarding copyright ownership.  The ASF licenses this file
* to you under the Apache License, Version 2.0 (the
* "License"); you may not use this file except in compliance
* with the License.  You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an
* "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
* KIND, either express or implied.  See the License for the
* specific language governing permissions and limitations
* under the License.
*
*************************************************************/


#ifndef _DROPTARGET_HXX_
#define _DROPTARGET_HXX_

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTargetDragContext.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTargetDropContext.hpp>
#include <com/sun/star/datatransfer/dnd/DropTargetDragEnterEvent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase5.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <osl/mutex.hxx>

#include "globals.hxx"


using namespace cppu;
using namespace osl;
using namespace rtl;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::dnd;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;


// The client
// has to call XComponent::dispose. The thread that calls initialize
// must also execute the destruction of the instance. This is because
// initialize calls OleInitialize and the destructor calls OleUninitialize.
// If the service calls OleInitialize then it also calls OleUnitialize when
// it is destroyed. Therefore no second instance may exist which was
// created in the same thread and still needs OLE.
class DropTarget: public cppu::BaseMutex,
        public WeakComponentImplHelper5< XInitialization,
        XDropTarget,
        XDropTargetDragContext,
        XDropTargetDropContext,
        XServiceInfo>
{
private:
    Reference<XMultiServiceFactory> m_serviceFactory;
    // The native window which acts as drop target.
    HWND m_hWnd;
    POINTL ptlMouse;
    // OS/2 does not provide a DM_DRAGENTER message, provide emulation
    bool        dragEnterEmulation;
    // If mbActive == sal_True then events are fired to XDropTargetListener s,
    // none otherwise. The default value is sal_True.
    bool        mbActive;
    sal_Int8    mDragSourceSupportedActions;
    sal_Int8    mSelectedDropAction;
    sal_Int8    mDefaultActions;
    Reference<XTransferable> mXTransferable;

public:
    DropTarget( const Reference<XMultiServiceFactory>& sf);
    virtual ~DropTarget();

    // Overrides WeakComponentImplHelper::disposing which is called by
    // WeakComponentImplHelper::dispose
    // Must be called.
    virtual void SAL_CALL disposing();

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments)
        throw(Exception);

    // XDropTarget
    virtual void SAL_CALL addDropTargetListener( const Reference< XDropTargetListener >& dtl )
        throw(RuntimeException);
    virtual void SAL_CALL removeDropTargetListener( const Reference<  XDropTargetListener >& dtl )
        throw(RuntimeException);

    // Default is not active
    virtual sal_Bool SAL_CALL isActive() throw(RuntimeException);
    virtual void SAL_CALL setActive(sal_Bool) throw(RuntimeException);
    virtual sal_Int8 SAL_CALL getDefaultActions() throw(RuntimeException);
    virtual void SAL_CALL setDefaultActions(sal_Int8) throw(RuntimeException);

    // XDropTargetDragContext
    virtual void SAL_CALL acceptDrag( sal_Int8) throw(RuntimeException);
    virtual void SAL_CALL rejectDrag() throw(RuntimeException);

    // XDropTargetDropContext
    virtual void SAL_CALL acceptDrop(sal_Int8) throw (RuntimeException);
    virtual void SAL_CALL rejectDrop() throw (RuntimeException);
    virtual void SAL_CALL dropComplete(sal_Bool) throw (RuntimeException);

    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw (RuntimeException);
    virtual Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() throw (RuntimeException);

    // OS/2 window messaging handlers
    MRESULT dragEnter( PDRAGINFO dragInfo);
    MRESULT dragOver( PDRAGINFO dragInfo);
    MRESULT dragLeave( PDRAGINFO dragInfo);
    MRESULT drop( PDRAGINFO dragInfo);
    MRESULT renderComplete( PDRAGTRANSFER);

public:
    // default window frame procedure
    PFNWP defWndProc;

private:
    void fire_drop(const DropTargetDropEvent& dte);
    void fire_dragEnter(const DropTargetDragEnterEvent& dtdee);
    void fire_dragExit(const DropTargetEvent& dte);
    void fire_dragOver(const DropTargetDragEvent& dtde);
    void fire_dropActionChanged(const DropTargetDragEvent& dtde);

};
#endif // _DROPTARGET_HXX_

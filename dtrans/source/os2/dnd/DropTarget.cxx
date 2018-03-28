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


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dtrans.hxx"
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <rtl/ustring.h>
#include <cppuhelper/implbase1.hxx>

#include <vcl/window.hxx>

#include "globals.hxx"
#include "DropTarget.hxx"
#include "DragSource.hxx"
#include "OTransferable.hxx"


using namespace com::sun::star;
using namespace com::sun::star::io;
using namespace com::sun::star::datatransfer::dnd::DNDConstants;


DropTarget::DropTarget( const Reference<XMultiServiceFactory>& sf):
    WeakComponentImplHelper5< XInitialization,
        XDropTarget,
        XDropTargetDragContext,
        XDropTargetDropContext,
        XServiceInfo>(m_aMutex),
    m_serviceFactory( sf),
    dragEnterEmulation( true),
    mbActive(false),
    mDragSourceSupportedActions(ACTION_NONE),
    mSelectedDropAction(ACTION_NONE),
    mDefaultActions(ACTION_COPY_OR_MOVE | ACTION_LINK | ACTION_DEFAULT)
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
}

DropTarget::~DropTarget()
{
    debug_printf("DropTarget::~DropTarget");

    // This will free the previous instance if present,
    // so it removes the tmp file
    mXTransferable = Reference<XTransferable>();

    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

void SAL_CALL DropTarget::initialize(const Sequence< Any >& aArguments)
throw(Exception)
{
    if (aArguments.getLength() < 2) {
        throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("DropTarget::initialize: Cannot install window event handler")),
                               static_cast<OWeakObject*>(this));
    }

    m_hWnd = *(HWND*) aArguments[0].getValue();
    debug_printf("DropTarget::initialize hwnd %x", m_hWnd);

    // subclass window to allow intercepting D&D messages
    defWndProc = WinSubclassWindow( m_hWnd, dndFrameProc);
    SetWindowDropTargetPtr( m_hWnd, this);
}

// called from WeakComponentImplHelperX::dispose
// WeakComponentImplHelper calls disposing before it destroys
// itself.
void SAL_CALL DropTarget::disposing()
{
    debug_printf("DropTarget::disposing hwnd %x", m_hWnd);

    // revert window subclassing
    WinSubclassWindow( m_hWnd, defWndProc);
    defWndProc = NULL;
    SetWindowDropTargetPtr( m_hWnd, 0);
}

void SAL_CALL DropTarget::addDropTargetListener(const uno::Reference<XDropTargetListener>& dtl)
throw(RuntimeException)
{
    debug_printf("DropTarget::addDropTargetListener hwnd %x", m_hWnd);
    rBHelper.addListener(::getCppuType(&dtl), dtl);
}

void SAL_CALL DropTarget::removeDropTargetListener(const uno::Reference<XDropTargetListener>& dtl)
throw(RuntimeException)
{
    debug_printf("DropTarget::removeDropTargetListener hwnd %x", m_hWnd);
    rBHelper.removeListener(::getCppuType(&dtl), dtl);
}

sal_Bool SAL_CALL DropTarget::isActive(  ) throw(RuntimeException)
{
    debug_printf("DropTarget::isActive %d", mbActive);
    return mbActive;
}

void SAL_CALL DropTarget::setActive(sal_Bool active) throw(RuntimeException)
{
    debug_printf("DropTarget::setActive %d", active);
    mbActive = active;
}

sal_Int8 SAL_CALL DropTarget::getDefaultActions() throw(RuntimeException)
{
    debug_printf("DropTarget::getDefaultActions %d", mDefaultActions);
    return mDefaultActions;
}

void SAL_CALL DropTarget::setDefaultActions(sal_Int8 actions) throw(RuntimeException)
{
    OSL_ENSURE( actions < 8, "No valid default actions");
    mDefaultActions= actions;
}

//
// XDropTargetDragContext
//
// Non - interface functions ============================================================
// DropTarget fires events to XDropTargetListeners. The event object can contains an
// XDropTargetDragContext implementaion. When the listener calls on that interface
// then the calls are delegated from DragContext (XDropTargetDragContext) to these
// functions.
// Only one listener which visible area is affected is allowed to call on
// XDropTargetDragContext

void SAL_CALL DropTarget::acceptDrag(sal_Int8 dragOperation) throw (RuntimeException)
{
    debug_printf("DropTarget::acceptDrag hwnd %x, dragOperation  %d", m_hWnd, dragOperation);
    mSelectedDropAction = dragOperation;
}

void SAL_CALL DropTarget::rejectDrag() throw (RuntimeException)
{
    debug_printf("DropTarget::rejectDrag hwnd %x", m_hWnd);
    mSelectedDropAction = ACTION_NONE;
}

//
// XDropTargetDropContext
//
// Non - interface functions ============================================================
// DropTarget fires events to XDropTargetListeners. The event object contains an
// XDropTargetDropContext implementaion. When the listener calls on that interface
// then the calls are delegated from DropContext (XDropTargetDropContext) to these
// functions.
// Only one listener which visible area is affected is allowed to call on
// XDropTargetDropContext
// Returning sal_False would cause the XDropTargetDropContext or ..DragContext implementation
// to throw an InvalidDNDOperationException, meaning that a Drag is not currently performed.
// return sal_False results in throwing a InvalidDNDOperationException in the caller.
//
void SAL_CALL DropTarget::acceptDrop(sal_Int8 dropOperation) throw( RuntimeException)
{
    debug_printf("DropTarget::acceptDrop hwnd %x, dragOperation  %d", m_hWnd, dropOperation);
    mSelectedDropAction = dropOperation;
}

void SAL_CALL DropTarget::rejectDrop() throw (RuntimeException)
{
    debug_printf("DropTarget::rejectDrop hwnd %x", m_hWnd);
    mSelectedDropAction = ACTION_NONE;
}

void SAL_CALL DropTarget::dropComplete(sal_Bool success) throw (RuntimeException)
{
    debug_printf("DropTarget::dropComplete hwnd %x", m_hWnd);

    // reset action flags
    mDragSourceSupportedActions = ACTION_NONE;
    mSelectedDropAction = ACTION_NONE;
    // enable drag enter emulation again
    dragEnterEmulation = true;
    // free local transferable list on next d&d or destruction

    // post a dummy message to source window to allow DragSource
    // release resources and close internal d&d
    if (DragSource::g_DragSourceHwnd != NULLHANDLE) {
        debug_printf("DropTarget::renderComplete post DM_AOO_ENDCONVERSATION to source");
        WinPostMsg( DragSource::g_DragSourceHwnd, DM_AOO_ENDCONVERSATION, 0,
                    MPFROMSHORT(success ? DMFL_TARGETSUCCESSFUL : DMFL_TARGETFAIL));
    }

}

//
// XServiceInfo
//
OUString SAL_CALL DropTarget::getImplementationName() throw (RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM(OS2_DNDTARGET_IMPL_NAME));;
}

sal_Bool SAL_CALL DropTarget::supportsService( const OUString& ServiceName ) throw (RuntimeException)
{
    return ServiceName.equals(OUString(RTL_CONSTASCII_USTRINGPARAM( OS2_DNDTARGET_SERVICE_NAME)));
}

Sequence< OUString > SAL_CALL DropTarget::getSupportedServiceNames(  ) throw (RuntimeException)
{
    OUString names[1]= {OUString(RTL_CONSTASCII_USTRINGPARAM( OS2_DNDTARGET_SERVICE_NAME))};
    return Sequence<OUString>(names, 1);
}

//
// AOO private interface events
//
void DropTarget::fire_drop( const DropTargetDropEvent& dte)
{
    OInterfaceContainerHelper* pContainer= rBHelper.getContainer( getCppuType( (uno::Reference<XDropTargetListener>* )0 ) );
    if( pContainer)
    {
        OInterfaceIteratorHelper iter( *pContainer);
        while( iter.hasMoreElements())
        {
            uno::Reference<XDropTargetListener> listener( static_cast<XDropTargetListener*>( iter.next()));

            try { listener->drop( dte); }
            catch(RuntimeException&) {}
        }
    }
    debug_printf("DropTarget::fire_drop fired");
}

void DropTarget::fire_dragEnter(const DropTargetDragEnterEvent& e)
{
    OInterfaceContainerHelper* pContainer= rBHelper.getContainer( getCppuType( (uno::Reference<XDropTargetListener>* )0 ) );
    if( pContainer)
    {
        OInterfaceIteratorHelper iter( *pContainer);
        while( iter.hasMoreElements())
        {
            uno::Reference<XDropTargetListener> listener( static_cast<XDropTargetListener*>( iter.next()));

            try { listener->dragEnter( e); }
            catch (RuntimeException&) {}
        }
    }
    debug_printf("DropTarget::fire_dragEnter fired");
}

void DropTarget::fire_dragExit(const DropTargetEvent& dte)
{
    OInterfaceContainerHelper* pContainer= rBHelper.getContainer( getCppuType( (uno::Reference<XDropTargetListener>* )0 ) );

    if( pContainer)
    {
        OInterfaceIteratorHelper iter( *pContainer);
        while( iter.hasMoreElements())
        {
            uno::Reference<XDropTargetListener> listener( static_cast<XDropTargetListener*>( iter.next()));

            try { listener->dragExit( dte); }
            catch (RuntimeException&) {}
        }
    }
    debug_printf("DropTarget::fire_dragExit fired");
}

void DropTarget::fire_dragOver(const DropTargetDragEvent& dtde)
{
    OInterfaceContainerHelper* pContainer= rBHelper.getContainer( getCppuType( (uno::Reference<XDropTargetListener>* )0 ) );
    if( pContainer)
    {
        OInterfaceIteratorHelper iter( *pContainer );
        while( iter.hasMoreElements())
        {
            uno::Reference<XDropTargetListener> listener( static_cast<XDropTargetListener*>( iter.next()));

            try { listener->dragOver( dtde); }
            catch (RuntimeException&) {}
        }
    }
    debug_printf("DropTarget::fire_dragOver fired");
}

void DropTarget::fire_dropActionChanged(const DropTargetDragEvent& dtde)
{
    OInterfaceContainerHelper* pContainer= rBHelper.getContainer( getCppuType( (uno::Reference<XDropTargetListener>* )0 ) );
    if( pContainer)
    {
        OInterfaceIteratorHelper iter( *pContainer);
        while( iter.hasMoreElements())
        {
            uno::Reference<XDropTargetListener> listener( static_cast<XDropTargetListener*>( iter.next()));

            try { listener->dropActionChanged( dtde); }
            catch (RuntimeException&) {}
        }
    }
    debug_printf("DropTarget::fire_dropActionChanged fired");
}

//
// OS/2 specific platform code
//

MRESULT DropTarget::dragEnter( PDRAGINFO dragInfo)
{
    debug_printf("DropTarget::dragEnter start hwnd 0x%x", m_hWnd);

    // disable drag enter emulation until next DM_DRAGLEAVE
    dragEnterEmulation = false;

    // Get access to the DRAGINFO data structure
    DrgAccessDraginfo( dragInfo);

    // Initially when DnD will be started no modifier key can be pressed yet
    // thus we are getting all actions that the drag source supports, we save
    // this value because later the system masks the drag source actions if
    // a modifier key will be pressed
    mDragSourceSupportedActions =
            SystemToOfficeDragActions( dragInfo->usOperation);

    // Only if the drop target is really interested in the drag actions
    // supported by the source
    if (mDragSourceSupportedActions & mDefaultActions) {

        //sal_Int8 currentAction = determineDropAction(mDragSourceSupportedActions, sender);
        sal_Int8 currentAction = mDragSourceSupportedActions;

        // map from desktop to client window
        MapWindowPoint( m_hWnd, dragInfo, &ptlMouse);

        // This will free the previous instance if present,
        // so it removes the tmp file
        mXTransferable = Reference<XTransferable>();

        // if g_XTransferable is empty this is an external drop operation,
        // create a new transferable set
        mXTransferable = DragSource::g_XTransferable;
        if (!mXTransferable.is()) {
            mXTransferable  =
                    //new OTransferable( OUString::createFromAscii( "TestString" ) );
                    new OTransferable( m_hWnd, dragInfo);
        }

#if 1
        // dump data flavours
        Sequence<DataFlavor> seq = mXTransferable->getTransferDataFlavors();
        for( int i=0; i<seq.getLength(); i++) {
            DataFlavor df = seq[i];
            debug_printf("DropTarget::dragEnter mimetype %s",
                         ::rtl::OUStringToOString( df.MimeType, RTL_TEXTENCODING_UTF8 ).getStr());
        }
#endif

        debug_printf("DropTarget::dragEnter (%dx%d) mDragSourceSupportedActions %d",
                     ptlMouse.x, ptlMouse.y,
                     mDragSourceSupportedActions);

        DropTargetDragEnterEvent dtdee(static_cast<OWeakObject*>(this),
                                       0, this, currentAction,
                                       ptlMouse.x, ptlMouse.y,
                                       mDragSourceSupportedActions,
                                       mXTransferable->getTransferDataFlavors());
        fire_dragEnter(dtdee);
    }

    // Release the draginfo data structure
    DrgFreeDraginfo(dragInfo);

    return OfficeToSystemDragActions( mSelectedDropAction);
}

MRESULT DropTarget::dragOver( PDRAGINFO dragInfo)
{
    MRESULT     dragOp = MRFROM2SHORT( DOR_NODROPOP, 0);

    if (dragEnterEmulation)
        return dragEnter( dragInfo);

    // Get access to the DRAGINFO data structure
    DrgAccessDraginfo( dragInfo);

    sal_Int8 currentDragSourceActions =
            SystemToOfficeDragActions( dragInfo->usOperation);

    // Only if the drop target is really interessted in the drag actions
    // supported by the source
    if (currentDragSourceActions & mDefaultActions) {
        //sal_Int8 currentAction = determineDropAction(mDragSourceSupportedActions, sender);
        sal_Int8 currentAction = currentDragSourceActions;

        // map from desktop to client window
        MapWindowPoint( m_hWnd, dragInfo, &ptlMouse);

        DropTargetDragEvent dtde(static_cast<OWeakObject*>(this),
                                 0, this, currentAction,
                                 ptlMouse.x, ptlMouse.y,
                                 mDragSourceSupportedActions);
        // firing the event will result in a XDropTargetDragContext event
        fire_dragOver(dtde);

        dragOp = OfficeToSystemDragActions(mSelectedDropAction);
    }

    // Release the draginfo data structure
    DrgFreeDraginfo(dragInfo);
    return dragOp;
}

MRESULT DropTarget::dragLeave( PDRAGINFO /* dragInfo */)
{
    debug_printf("DropTarget::dragLeave");

    DropTargetEvent dte(static_cast<OWeakObject*>(this), 0);
    fire_dragExit(dte);

    // reset action flags
    mDragSourceSupportedActions = ACTION_NONE;
    mSelectedDropAction = ACTION_NONE;
    // enable drag enter emulation again
    dragEnterEmulation = true;
    // free local transferable list on next d&d or destruction

    return 0;
}

MRESULT DropTarget::drop( PDRAGINFO dragInfo)
{
    debug_printf("DropTarget::drop");

    // Get access to the DRAGINFO data structure
    DrgAccessDraginfo( dragInfo);

    MRESULT dropOp = MRFROM2SHORT( DOR_NODROPOP, 0);

    if (mSelectedDropAction != ACTION_NONE) {

        bool rr = false;

        // map from desktop to client window
        MapWindowPoint( m_hWnd, dragInfo, &ptlMouse);

        // if external d&d, request rendering
        OTransferable* ot = dynamic_cast<OTransferable*>(mXTransferable.get());
        if (ot != NULL) {
            // request rendering, if operation is already possible it
            // will return false
            rr = ot->requestRendering();
            debug_printf("DropTarget::drop requestRendering=%d", rr);
        }

        // no rendering requested, post a DM_RENDERCOMPLETE to ourselves
        // to fire AOO drop event
        if (rr == false)
            WinPostMsg( m_hWnd, DM_RENDERCOMPLETE, 0, 0);

        dropOp = OfficeToSystemDragActions(mSelectedDropAction);
    }

    // Release the draginfo data structure
    DrgFreeDraginfo(dragInfo);

    return dropOp;

}

MRESULT DropTarget::renderComplete( PDRAGTRANSFER dragTransfer)
{
    debug_printf("DropTarget::renderComplete dragTransfer 0x%x", dragTransfer);

    if (dragTransfer != NULL) {
        OTransferable* ot = dynamic_cast<OTransferable*>(mXTransferable.get());
        // DM_RENDERCOMPLETE cannot be received in internal AOO d&d
        if (ot == NULL) {
            debug_printf("DropTarget::renderComplete INTERNAL ERROR null dragtransfer");
            return 0;
        }

        // set rendered data
        ot->renderComplete( dragTransfer);
    }

    debug_printf("DropTarget::renderComplete mXTransferable.is() %d", mXTransferable.is());

    // complete AOO drop event, this will make AOO call
    // XTransferable::getTransferData() for external ops,
    // then acceptDrop(), dropComplete() are called from listeners
    DropTargetDropEvent dtde( static_cast<OWeakObject*>(this),
                              0, this, mSelectedDropAction,
                              ptlMouse.x, ptlMouse.y,
                              mDragSourceSupportedActions,
                              mXTransferable);
    fire_drop(dtde);

    // Reserved value, should be 0
    return 0;
}

/*************************************************************************
 *
 *  $RCSfile: target.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: jl $ $Date: 2001-02-26 15:33:29 $
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
#ifndef _TARGET_HXX_
#define _TARGET_HXX_

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDROPTARGET_HPP_
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_DND_DROPTARGETDRAGENTEREVENT_HPP_
#include <com/sun/star/datatransfer/dnd/DropTargetDragEnterEvent.hpp>
#endif

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _OSL_MUTEX_H_
#include <osl/mutex.hxx>
#endif

#include <oleidl.h>
#include "globals.hxx"


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
                  public WeakComponentImplHelper2< XInitialization, XDropTarget>

{
private:
    // The native window which acts as drop target.
    // It is set in initialize. In case RegisterDragDrop fails it is set
    // to NULL
    HWND m_hWnd; // set by initialize
    // The thread id of the thread which called initialize. When the service dies
    // than m_oleThreadId is used to determine if the service successfully called
    // OleInitialize. If so then OleUninitialize has to be called.
    DWORD m_oleThreadId;
    // An Instance of IDropTargetImpl which receives calls from the system's drag
    // and drop implementation. It delegate the calls to name alike functions in
    // this class.
    IDropTarget* m_pDropTarget;

    Reference<XMultiServiceFactory> m_serviceFactory;
    // If m_bActive == sal_True then events are fired to XDropTargetListener s,
    // none otherwise. The default value is sal_True.
    sal_Bool m_bActive;
    sal_Int8    m_nDefaultActions;

    // This value is set when a XDropTargetListener calls accept or reject on
    // the XDropTargetDropContext or  XDropTargetDragContext.
    // The values are from the DNDConstants group.
    sal_Int8 m_nListenerDropAction;
    Reference<XTransferable> m_currentData;
    // The current action is used to determine if the USER
    // action has changed (dropActionChanged)
    sal_Int8 m_userAction;
    // Set by listeners when they call XDropTargetDropContext::dropComplete
    sal_Bool m_bDropComplete;

    // Used to find out wheter the listeners callbacks through the Context interfaces
    // contained in the event objects are valid.

    Reference<XDropTargetDragContext> m_currentDragContext;
    Reference<XDropTargetDropContext> m_currentDropContext;


private:
    DropTarget();
    DropTarget(DropTarget&);
    DropTarget &operator= (DropTarget&);

public:
    DropTarget(const Reference<XMultiServiceFactory>& sf);
    virtual ~DropTarget();

#ifdef DEBUG
    virtual void SAL_CALL release();
#endif
    // Overrides WeakComponentImplHelper::disposing which is called by
    // WeakComponentImplHelper::dispose
    // Must be called.
    virtual void SAL_CALL disposing();
   // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments )
        throw(Exception, RuntimeException);

    // XDropTarget
    virtual void SAL_CALL addDropTargetListener( const Reference< XDropTargetListener >& dtl )
        throw(RuntimeException);
    virtual void SAL_CALL removeDropTargetListener( const Reference< XDropTargetListener >& dtl )
        throw(RuntimeException);
    // Default is not active
    virtual sal_Bool SAL_CALL isActive(  ) throw(RuntimeException);
    virtual void SAL_CALL setActive( sal_Bool isActive ) throw(RuntimeException);
    virtual sal_Int8 SAL_CALL getDefaultActions(  ) throw(RuntimeException);
    virtual void SAL_CALL setDefaultActions( sal_Int8 actions ) throw(RuntimeException);

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
    void _dropComplete( sal_Bool success, const Reference<XDropTargetDropContext>& context);

// XDropTargetDragContext delegated from DragContext
    void _acceptDrag( sal_Int8 dragOperation, const Reference<XDropTargetDragContext>& context);
    void _rejectDrag( const Reference<XDropTargetDragContext>& context);


protected:
    // Gets the current action dependend on the pressed modifiers, the effects
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


#endif
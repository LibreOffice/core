/*************************************************************************
 *
 *  $RCSfile: source.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mh $ $Date: 2001-01-31 15:37:17 $
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

#ifndef _SOURCE_HXX_
#define _SOURCE_HXX_

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDRAGSOURCE_HPP_
#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDROPTARGETFACTORY_HPP_
#include <com/sun/star/datatransfer/dnd/XDropTargetFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _OSL_MUTEX_H_
#include <osl/mutex.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/compbase3.hxx>
#endif



#include <wtypes.h>
#include <oleidl.h>
// the service names
#define DNDTARGET_SERVICE_NAME  "com.sun.star.datatransfer.dnd.OleDragAndDrop"

// the implementation names
#define DNDTARGET_IMPL_NAME  "com.sun.star.comp.datatransfer.dnd.OleDragAndDrop_V1"

// the registry key names
// a key under which this service will be registered, Format: -> "/ImplName/UNO/SERVICES/ServiceName"
//                        <     Implementation-Name    ></UNO/SERVICES/><    Service-Name
// TargetW32         >
#define DNDTARGET_REGKEY_NAME  "/com.sun.star.comp.datatransfer.dnd.OleDragAndDrop_V1/UNO/SERVICES/com.sun.star.datatransfer.dnd.OleDragAndDrop"



using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace cppu;
using namespace osl;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::dnd;

struct MutexDummy
{
    Mutex m_mutex;
};


// RIGHT MOUSE BUTTON drag and drop not supportet currently.
// ALT modifier is considered to effect a user selection of effects
class DragSource:
      public MutexDummy,
      public WeakComponentImplHelper3<XInitialization, XDragSource,
             XDropTargetFactory>,
      public IDropSource

{
    Reference<XMultiServiceFactory> m_serviceFactory;
    Reference<XDragSourceListener > m_sourceListener;
    sal_Int8 m_sourceActions;



    DragSourcet();
    DragSource(const DragSource&);
    DragSource &operator= ( const DragSource&);

public:
    DragSource(const Reference<XMultiServiceFactory>& sf);
    virtual ~DragSource();

#ifdef DEBUG
    virtual void SAL_CALL release();
#endif

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
                throw( InvalidDNDOperationException, RuntimeException);

    // XDropTargetFactory
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);

    virtual ULONG STDMETHODCALLTYPE AddRef( );

    virtual ULONG STDMETHODCALLTYPE Release( );

    virtual Reference<XDropTarget > SAL_CALL createDropTarget( const Sequence< sal_Int8 >& windowId )
            throw( RuntimeException);

    // IDropSource
    virtual HRESULT STDMETHODCALLTYPE QueryContinueDrag(
        /* [in] */ BOOL fEscapePressed,
        /* [in] */ DWORD grfKeyState);

    virtual HRESULT STDMETHODCALLTYPE GiveFeedback(
        /* [in] */ DWORD dwEffect);

};

#endif
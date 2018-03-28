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


#ifndef _DRAGSOURCE_HXX_
#define _DRAGSOURCE_HXX_

#include <svpm.h>

#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#include <com/sun/star/datatransfer/dnd/XDragSourceContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase4.hxx>
#include <osl/mutex.hxx>

#include "globals.hxx"

using namespace cppu;
using namespace osl;
using namespace rtl;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::dnd;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;


class DragSource:
        public cppu::BaseMutex,
        public WeakComponentImplHelper4<XDragSource,
            XInitialization,
            XDragSourceContext,
            XServiceInfo>
{
public:
    // used also in DropTarget in AOO internal d&d
    static Reference<XTransferable> g_XTransferable;
    // the handle of the window starting the drag
    static HWND g_DragSourceHwnd;

private:
    Reference<XMultiServiceFactory> m_serviceFactory;
    // The native window which acts as source.
    HWND m_hWnd;
    PDRAGINFO pSourceDraginfo;
    char *pSharedMem;
    char *pDTShareMem;

    Reference<XDragSourceListener> dragSourceListener;

public:
    DragSource( const Reference<XMultiServiceFactory>& sf);
    virtual ~DragSource();

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments )
        throw(Exception, RuntimeException);
    virtual void SAL_CALL disposing();

    // XDragSource
    virtual sal_Bool SAL_CALL isDragImageSupported(  ) throw(RuntimeException);
    virtual sal_Int32 SAL_CALL getDefaultCursor(sal_Int8 dragAction)
        throw(IllegalArgumentException, RuntimeException);
    virtual void SAL_CALL startDrag( const DragGestureEvent& trigger,
                                     sal_Int8 sourceActions,
                                     sal_Int32 cursor,
                                     sal_Int32 image,
                                     const Reference< XTransferable>& transferable,
                                     const Reference< XDragSourceListener>& listener)
        throw(RuntimeException);

    // XDragSourceContext
    virtual sal_Int32 SAL_CALL getCurrentCursor() throw( RuntimeException);
    virtual void SAL_CALL setCursor( sal_Int32) throw( RuntimeException);
    virtual void SAL_CALL setImage( sal_Int32) throw( RuntimeException);
    virtual void SAL_CALL transferablesFlavorsChanged() throw( RuntimeException);

    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw (RuntimeException);
    virtual Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() throw (RuntimeException);

    // OS/2 window messaging handlers
    MRESULT render( PDRAGTRANSFER);
    MRESULT endConversation( ULONG, ULONG);

};
#endif // _DRAGSOURCE_HXX_

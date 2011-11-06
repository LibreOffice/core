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



#ifndef _TARGETLISTENER_HXX_
#define _TARGETLISTENER_HXX_

#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/datatransfer/dnd/XDropTargetListener.hpp>
#include <com/sun/star/datatransfer/dnd/DropTargetDropEvent.hpp>
#include <com/sun/star/datatransfer/dnd/DropTargetDragEvent.hpp>
#include <com/sun/star/datatransfer/dnd/DropTargetDragEnterEvent.hpp>

using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::dnd;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

class DropTargetListener: public WeakImplHelper1<XDropTargetListener>
{
    // this is a window where droped data are shown as text (only text)
    HWND m_hEdit;
public:
    DropTargetListener( HWND hEdit);
    ~DropTargetListener();

    virtual void SAL_CALL disposing( const EventObject& Source )
        throw(RuntimeException);


    virtual void SAL_CALL drop( const DropTargetDropEvent& dtde )
        throw(RuntimeException);
    virtual void SAL_CALL dragEnter( const DropTargetDragEnterEvent& dtde )
        throw(RuntimeException);
    virtual void SAL_CALL dragExit( const DropTargetEvent& dte )
        throw(RuntimeException);
    virtual void SAL_CALL dragOver( const DropTargetDragEvent& dtde )
        throw(RuntimeException);
    virtual void SAL_CALL dropActionChanged( const DropTargetDragEvent& dtde )
        throw(RuntimeException);
};

#endif

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



#ifndef _SOURCELISTENER_HXX_
#define _SOURCELISTENER_HXX_

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/datatransfer/dnd/XDragSourceListener.hpp>
#include <com/sun/star/datatransfer/dnd/DragSourceDropEvent.hpp>
#include <com/sun/star/datatransfer/dnd/DragSourceDragEvent.hpp>

using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::dnd;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

class DragSourceListener: public WeakImplHelper1<XDragSourceListener>
{
    // this is a window where droped data are shown as text (only text)
public:
    DragSourceListener( );
    ~DragSourceListener();

    virtual void SAL_CALL disposing( const EventObject& Source )
        throw(RuntimeException);

    virtual void SAL_CALL dragDropEnd( const DragSourceDropEvent& dsde )
        throw(RuntimeException);
    virtual void SAL_CALL dragEnter( const DragSourceDragEvent& dsde )
        throw(RuntimeException);
    virtual void SAL_CALL dragExit( const DragSourceEvent& dse )
        throw(RuntimeException);
    virtual void SAL_CALL dragOver( const DragSourceDragEvent& dsde )
        throw(RuntimeException);
    virtual void SAL_CALL dropActionChanged( const DragSourceDragEvent& dsde )
        throw(RuntimeException);

};

#endif

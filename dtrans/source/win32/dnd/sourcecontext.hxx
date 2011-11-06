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


#ifndef _SOURCECONTEXT_HXX_
#define _SOURCECONTEXT_HXX_

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/datatransfer/dnd/XDragSourceContext.hpp>
#include <cppuhelper/compbase1.hxx>


#include "source.hxx"

using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::dnd;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;


// This class fires events to XDragSourceListener implementations.
// Of that interface only dragDropEnd and dropActionChanged are called.
// The functions dragEnter, dragExit and dragOver are not supported
// currently.
// An instance of SourceContext only lives as long as the drag and drop
// operation lasts.
class SourceContext: public MutexDummy,
                     public WeakComponentImplHelper1<XDragSourceContext>
{
    DragSource* m_pDragSource;
    Reference<XDragSource> m_dragSource;
    // the action ( copy, move etc)
    sal_Int8 m_currentAction;

    SourceContext();
    SourceContext( const SourceContext&);
    SourceContext &operator= (const SourceContext& );

public:
    SourceContext( DragSource* pSource, const Reference<XDragSourceListener>& listener);
    ~SourceContext();

    virtual void SAL_CALL addDragSourceListener( const Reference<XDragSourceListener >& dsl )
        throw( RuntimeException);
    virtual void SAL_CALL removeDragSourceListener( const Reference<XDragSourceListener >& dsl )
        throw( RuntimeException);
    virtual sal_Int32 SAL_CALL getCurrentCursor(  )
        throw( RuntimeException);
    virtual void SAL_CALL setCursor( sal_Int32 cursorId )
        throw( RuntimeException);
    virtual void SAL_CALL setImage( sal_Int32 imageId )
        throw( RuntimeException);
    virtual void SAL_CALL transferablesFlavorsChanged(  )
        throw( RuntimeException);



    // non - interface functions
    void fire_dragDropEnd( sal_Bool success, sal_Int8 byte);
    void fire_dropActionChanged( sal_Int8 dropAction, sal_Int8 userAction);

};



#endif

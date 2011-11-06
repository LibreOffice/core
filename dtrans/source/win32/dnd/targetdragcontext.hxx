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



#ifndef _TARGETDRAGCONTEXT_HXX_
#define _TARGETDRAGCONTEXT_HXX_


#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/datatransfer/dnd/XDropTargetDragContext.hpp>
#include <com/sun/star/datatransfer/DataFlavor.hpp>

#include "target.hxx"
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::dnd;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

class TargetDragContext: public WeakImplHelper1<XDropTargetDragContext>
{
    // some calls to the functions of XDropTargetDragContext are delegated
    // to non-interface functions of m_pDropTarget
    DropTarget* m_pDropTarget;

    TargetDragContext();
    TargetDragContext( const TargetDragContext&);
    TargetDragContext &operator= ( const TargetDragContext&);
public:
    TargetDragContext( DropTarget* pTarget);
    ~TargetDragContext();

    virtual void SAL_CALL acceptDrag( sal_Int8 dragOperation )
        throw( RuntimeException);
    virtual void SAL_CALL rejectDrag(  )
        throw( RuntimeException);
};

#endif

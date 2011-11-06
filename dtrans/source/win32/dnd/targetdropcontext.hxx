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



#ifndef _TARGETDROPCONTEXT_HXX_
#define _TARGETDROPCONTEXT_HXX_

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/datatransfer/dnd/XDropTargetDropContext.hpp>

#include "target.hxx"

using namespace ::com::sun::star::datatransfer::dnd;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

class TargetDropContext: public WeakImplHelper1<XDropTargetDropContext>
{
    // calls to the functions of XDropTargetDropContext are delegated
    // to non-interface functions of m_pDropTarget
    DropTarget* m_pDropTarget;

    TargetDropContext();
    TargetDropContext( const TargetDropContext&);
    TargetDropContext &operator= ( const TargetDropContext&);
public:
    TargetDropContext( DropTarget* pTarget);
    ~TargetDropContext();


    // XDropTargetDragContext
    virtual void SAL_CALL acceptDrop( sal_Int8 dropOperation )
        throw( RuntimeException);
    virtual void SAL_CALL rejectDrop(  )
        throw( RuntimeException);


/*  virtual Sequence< DataFlavor > SAL_CALL getCurrentDataFlavors(  )
        throw( RuntimeException);
    virtual sal_Bool SAL_CALL isDataFlavorSupported( const DataFlavor& df )
        throw( RuntimeException);
*/

    // XDropTargetDropContext (inherits XDropTargetDragContext)
    virtual void SAL_CALL dropComplete( sal_Bool success )
        throw(  RuntimeException);
};
#endif

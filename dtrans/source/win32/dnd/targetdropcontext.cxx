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
#include <rtl/unload.h>

#include "targetdropcontext.hxx"

using namespace ::com::sun::star::datatransfer::dnd;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

extern rtl_StandardModuleCount g_moduleCount;
TargetDropContext::TargetDropContext( DropTarget* p)
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
    m_pDropTarget= p;
    p->acquire();
}

TargetDropContext::~TargetDropContext()
{
    m_pDropTarget->release();
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

void SAL_CALL TargetDropContext::acceptDrop( sal_Int8 dropOperation )
        throw( RuntimeException)
{
    m_pDropTarget->_acceptDrop( dropOperation, static_cast<XDropTargetDropContext*>( this) );
}

void SAL_CALL TargetDropContext::rejectDrop( )
        throw( RuntimeException)
{
    m_pDropTarget->_rejectDrop(  static_cast<XDropTargetDropContext*>( this) );
}

void SAL_CALL TargetDropContext::dropComplete( sal_Bool success )
        throw( RuntimeException)
{
    m_pDropTarget->_dropComplete( success, static_cast<XDropTargetDropContext*>( this) );
}

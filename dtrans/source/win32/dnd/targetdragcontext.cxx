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

#include "targetdragcontext.hxx"

extern rtl_StandardModuleCount g_moduleCount;
TargetDragContext::TargetDragContext( DropTarget* p)
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
    m_pDropTarget= p;
    p->acquire();
}

TargetDragContext::~TargetDragContext()
{
    m_pDropTarget->release();
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}

void SAL_CALL TargetDragContext::acceptDrag( sal_Int8 dragOperation )
    throw( RuntimeException)
{
    m_pDropTarget->_acceptDrag( dragOperation, static_cast<XDropTargetDragContext*>( this) );

}
void SAL_CALL TargetDragContext::rejectDrag( )
    throw( RuntimeException)
{
    m_pDropTarget->_rejectDrag( static_cast<XDropTargetDragContext*>( this) );
}


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
#include "precompiled_vcl.hxx"


#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>

#include "DragSourceContext.hxx"
#include <rtl/unload.h>


using namespace com::sun::star::datatransfer::dnd;
using namespace com::sun::star::datatransfer::dnd::DNDConstants;
using namespace com::sun::star::uno;
using namespace cppu;

DragSourceContext::DragSourceContext( DragSource* pSource) :
        WeakComponentImplHelper1<XDragSourceContext>(m_aMutex),
        m_pDragSource( pSource)
{
}

DragSourceContext::~DragSourceContext()
{
}

sal_Int32 SAL_CALL DragSourceContext::getCurrentCursor(  )
    throw( RuntimeException)
{
    return 0;
}

void SAL_CALL DragSourceContext::setCursor( sal_Int32 /*cursorId*/ )
    throw( RuntimeException)
{
}

void SAL_CALL DragSourceContext::setImage( sal_Int32 /*imageId*/ )
    throw( RuntimeException)
{
}

void SAL_CALL DragSourceContext::transferablesFlavorsChanged(  )
    throw( RuntimeException)
{
}



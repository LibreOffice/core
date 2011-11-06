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
#include "precompiled_extensions.hxx"

#include "initwindowpeer.hxx"

using namespace ::com::sun::star;


uno::Any SAL_CALL InitWindowPeer::getWindowHandle( const uno::Sequence< sal_Int8 >& ProcessId, sal_Int16 SystemType )
    throw ( uno::RuntimeException )
{
    uno::Any aRes;
    sal_Int32 nHwnd = (sal_Int32)m_hwnd;
    aRes <<= nHwnd;
    return aRes;
}


uno::Reference< awt::XToolkit > SAL_CALL InitWindowPeer::getToolkit()
    throw ( uno::RuntimeException )
{
    return uno::Reference< awt::XToolkit >();
}

void SAL_CALL InitWindowPeer::setPointer( const uno::Reference< awt::XPointer >& Pointer )
    throw ( uno::RuntimeException )
{
}

void SAL_CALL InitWindowPeer::setBackground( sal_Int32 Color )
    throw ( uno::RuntimeException )
{
}

void SAL_CALL InitWindowPeer::invalidate( sal_Int16 Flags )
    throw ( uno::RuntimeException )
{
}

void SAL_CALL InitWindowPeer::invalidateRect( const awt::Rectangle& Rect, sal_Int16 Flags )
    throw ( uno::RuntimeException )
{
}

void SAL_CALL InitWindowPeer::dispose()
    throw ( uno::RuntimeException )
{
}

void SAL_CALL InitWindowPeer::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw ( uno::RuntimeException )
{
}

void SAL_CALL InitWindowPeer::removeEventListener( const uno::Reference< lang::XEventListener >& aListener )
    throw ( uno::RuntimeException )
{
}


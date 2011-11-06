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


#ifndef COMPHELPER_EVENTLISTENERHELPER_HXX
#define COMPHELPER_EVENTLISTENERHELPER_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/lang/XEventListener.hpp>
#include <osl/diagnose.h>
#include <cppuhelper/weakref.hxx>
#include "comphelper/comphelperdllapi.h"

//........................................................................
namespace comphelper
{
//........................................................................

    //==========================================================================
    //= OCommandsListener
    // is helper class to avoid a cycle in refcount between the XEventListener
    // and the member XEventBroadcaster
    //==========================================================================
    class COMPHELPER_DLLPUBLIC OEventListenerHelper : public ::cppu::WeakImplHelper1< ::com::sun::star::lang::XEventListener >
    {
        ::com::sun::star::uno::WeakReference< ::com::sun::star::lang::XEventListener> m_xListener;
    public:
        OEventListenerHelper(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener>& _rxListener);
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);
    };
//........................................................................
}   // namespace comphelper
//........................................................................
#endif // COMPHELPER_EVENTLISTENERHELPER_HXX

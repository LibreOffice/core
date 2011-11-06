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



#ifndef _SC_DRAWMODELBROADCASTER_HXX
#define _SC_DRAWMODELBROADCASTER_HXX


#include <svl/lstner.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/document/XEventBroadcaster.hpp>

class SdrModel;

class ScDrawModelBroadcaster : public SfxListener,
    public ::cppu::WeakImplHelper1< com::sun::star::document::XEventBroadcaster >
{
    mutable ::osl::Mutex maListenerMutex;
    ::cppu::OInterfaceContainerHelper maEventListeners;
    SdrModel *mpDrawModel;

public:

    ScDrawModelBroadcaster( SdrModel *pDrawModel );
    virtual ~ScDrawModelBroadcaster();

    virtual void SAL_CALL addEventListener( const com::sun::star::uno::Reference< com::sun::star::document::XEventListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const com::sun::star::uno::Reference< com::sun::star::document::XEventListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};

#endif

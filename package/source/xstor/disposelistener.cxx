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
#include "precompiled_package.hxx"

#include "disposelistener.hxx"
#include "xstorage.hxx"

using namespace ::com::sun::star;

OChildDispListener_Impl::OChildDispListener_Impl( OStorage& aStorage )
: m_pStorage( &aStorage )
{}

OChildDispListener_Impl::~OChildDispListener_Impl()
{}

void OChildDispListener_Impl::OwnerIsDisposed()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    m_pStorage = NULL;
}

void SAL_CALL OChildDispListener_Impl::disposing( const lang::EventObject& Source )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    // ObjectIsDisposed must not contain any locking!
    if ( m_pStorage && Source.Source.is() )
        m_pStorage->ChildIsDisposed( Source.Source );
}


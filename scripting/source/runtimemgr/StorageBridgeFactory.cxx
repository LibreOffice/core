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
#include "precompiled_scripting.hxx"


#include "StorageBridgeFactory.hxx"

#include "StorageBridge.hxx"

using namespace ::drafts::com::sun::star::script::framework;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;

namespace scripting_runtimemgr
{

//*************************************************************************
StorageBridgeFactory::StorageBridgeFactory(
    const Reference< XComponentContext >& xContext ) :
    m_xContext( xContext )
{}



//*************************************************************************
// StorageBridge is a bridge between us an storage, the StorageBridge implements the
// XScriptInfoAccess
// interface, it allows both the "real" storage and a java test storage component to be
// configured.
// @param location contains a url for the document.
// @returns a storage instance for the location "location".
// @see StorageBridge.cxx  for flag "STORAGEPROXY", set to use java "test" component or
// use "real" storage.

Reference< storage::XScriptInfoAccess >
StorageBridgeFactory::getStorageInstance( sal_Int32 sid )
{

    Reference< storage::XScriptInfoAccess > storage = new StorageBridge( m_xContext,
            sid ) ;
    return storage;
}
} //scripting_runtimemgr

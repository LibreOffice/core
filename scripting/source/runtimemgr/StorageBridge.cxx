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

#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <drafts/com/sun/star/script/framework/storage/XScriptStorageManager.hpp>

#include "StorageBridge.hxx"
#include <util/util.hxx>

using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::script::framework;

namespace scripting_runtimemgr
{

const char* const SCRIPTIMPLACCESS_SERVICE =
    "drafts.com.sun.star.script.framework.storage.StorageProxy";
const char* const SCRIPTSTORAGEMANAGER_SERVICE =
    "/singletons/drafts.com.sun.star.script.framework.storage.theScriptStorageManager";
const int STORAGEID = 0;
const int STORAGEPROXY = 0;


//*************************************************************************
// StorageBridge Constructor
StorageBridge::StorageBridge( const Reference< XComponentContext >& xContext,
                              sal_Int32 sid ) : m_xContext( xContext, UNO_SET_THROW ), m_sid( sid )
{
    try
    {
        initStorage();
    }
    catch ( RuntimeException & re )
    {
        OUString temp = OUSTR( "StorageBridge::StorageBridge(salIn32&): " );
        throw RuntimeException( temp.concat( re.Message ), Reference< XInterface >() );
    }
}

//*************************************************************************
void
StorageBridge::initStorage() throw ( ::com::sun::star::uno::RuntimeException )
{
    try
    {
        Reference< lang::XMultiComponentFactory > xMultiComFac( m_xContext->getServiceManager(), UNO_SET_THROW );
        Reference< XInterface > temp( m_xContext->getValueByName(
                    OUString::createFromAscii( SCRIPTSTORAGEMANAGER_SERVICE ) ), UNO_QUERY_THROW );
        Reference< storage::XScriptStorageManager > xScriptStorageManager( temp, UNO_QUERY_THROW );
        Reference< XInterface > xScriptStorage( xScriptStorageManager->getScriptStorage( m_sid ), UNO_SET_THROW );
        m_xScriptInfoAccess.set( xScriptStorage, UNO_QUERY_THROW );
    }
    catch ( RuntimeException & re )
    {
        OUString temp = OUSTR( "StorageBridge::StorageBridge: " );
        throw RuntimeException( temp.concat( re.Message ), Reference< XInterface >() );
    }
    catch ( Exception & e )
    {
        OUString temp = OUSTR( "StorageBridge::StorageBridge: " );
        throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
    }
}
//*************************************************************************
Sequence< ::rtl::OUString >
StorageBridge::getScriptLogicalNames()
throw ( lang::IllegalArgumentException,
        RuntimeException )
{
    OSL_TRACE( "In StorageBridge getScriptLogicalNames...\n" );
    Sequence < ::rtl::OUString  > results;
    try
    {
        results = m_xScriptInfoAccess->getScriptLogicalNames();
    }
    catch ( Exception e )
    {
        OUString temp = OUSTR( "StorageBridge::getScriptLogicalNames: " );
        throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
    }
    return results;
}

//*************************************************************************
Sequence < Reference< storage::XScriptInfo > >
StorageBridge::getImplementations( const ::rtl::OUString& queryURI )
throw ( lang::IllegalArgumentException, RuntimeException )
{
    OSL_TRACE( "In StorageBridge getImplementations...\n" );
    Sequence < Reference< storage::XScriptInfo > > results;
    try
    {
        results = m_xScriptInfoAccess->getImplementations( queryURI );
    }
    catch ( Exception e )
    {
        OUString temp = OUSTR( "StorageBridge::getImplementations: " );
        throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
    }
    return results;
}
}// namespace

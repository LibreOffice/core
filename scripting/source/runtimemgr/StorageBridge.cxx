/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

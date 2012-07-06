/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


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
    catch ( const RuntimeException & re )
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
    catch ( const RuntimeException & re )
    {
        OUString temp = OUSTR( "StorageBridge::StorageBridge: " );
        throw RuntimeException( temp.concat( re.Message ), Reference< XInterface >() );
    }
    catch ( const Exception & e )
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
    OSL_TRACE( "In StorageBridge getScriptLogicalNames..." );
    Sequence < ::rtl::OUString  > results;
    try
    {
        results = m_xScriptInfoAccess->getScriptLogicalNames();
    }
    catch ( const Exception &e )
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
    OSL_TRACE( "In StorageBridge getImplementations..." );
    Sequence < Reference< storage::XScriptInfo > > results;
    try
    {
        results = m_xScriptInfoAccess->getImplementations( queryURI );
    }
    catch ( const Exception &e )
    {
        OUString temp = OUSTR( "StorageBridge::getImplementations: " );
        throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
    }
    return results;
}
}// namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

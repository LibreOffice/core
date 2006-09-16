/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StorageBridge.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 12:30:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
                              sal_Int32 sid ) : m_xContext( xContext ), m_sid( sid )
{
    validateXRef( m_xContext, "StorageBridge::StorageBridge: invalid context" );
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
        Reference< lang::XMultiComponentFactory > xMultiComFac =
            m_xContext->getServiceManager();
        validateXRef( xMultiComFac,
                      "StorageBridge::StorageBridge: cannot get multicomponentfactory from multiservice factory" );
        Reference< XInterface > temp;

        Any a = m_xContext->getValueByName(
                    OUString::createFromAscii( SCRIPTSTORAGEMANAGER_SERVICE ) );
        if ( sal_False == ( a >>= temp ) )
        {
            throw RuntimeException(
                OUSTR( "StorageBridge::StorageBridge: could not obtain ScriptStorageManager singleton" ),
                Reference< XInterface >() );
        }
        validateXRef( temp,
                      "StorageBridge::StorageBridge: cannot get Storage service" );
        Reference< storage::XScriptStorageManager > xScriptStorageManager( temp, UNO_QUERY_THROW );
        validateXRef( xScriptStorageManager,
                      "StorageBridge::StorageBridge: cannot get Script Storage Manager service" );
        Reference< XInterface > xScriptStorage =
            xScriptStorageManager->getScriptStorage( m_sid );
        validateXRef( xScriptStorage,
                      "StorageBridge::StorageBridge: cannot get Script Storage service" );
        m_xScriptInfoAccess =
            Reference< storage::XScriptInfoAccess > ( xScriptStorage, UNO_QUERY_THROW );
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

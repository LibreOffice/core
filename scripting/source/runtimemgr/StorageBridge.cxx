/*************************************************************************
 *
 *  $RCSfile: StorageBridge.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jmrice $ $Date: 2002-09-27 12:16:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
                              sal_uInt16 sid ) : m_xContext( xContext ), m_sid( sid )
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
        Reference< lang::XMultiComponentFactory > xMultiComFac =
            m_xContext->getServiceManager();
        validateXRef( xMultiComFac,
                      "StorageBridge::StorageBridge: cannot get multicomponentfactory from multiservice factory" );
        Reference< XInterface > temp;

        if ( STORAGEPROXY )
        {
            temp = xMultiComFac->createInstanceWithContext(
                       OUString::createFromAscii( SCRIPTIMPLACCESS_SERVICE ), m_xContext );
            validateXRef( temp, "StorageBridge::StorageBridge: cannot get Storage service" );
            m_xScriptImplAccess = Reference< storage::XScriptImplAccess > ( temp,
                                  UNO_QUERY_THROW );
        }
        else
        {
            Any a = m_xContext->getValueByName(
                        OUString::createFromAscii( SCRIPTSTORAGEMANAGER_SERVICE ) );
            a >>= temp;
            validateXRef( temp,
                          "StorageBridge::StorageBridge: cannot get Storage service" );
            Reference< storage::XScriptStorageManager > xScriptStorageManager =
                Reference< storage::XScriptStorageManager > ( temp, UNO_QUERY_THROW );
            validateXRef( xScriptStorageManager,
                          "StorageBridge::StorageBridge: cannot get Script Storage Manager service" );
            Reference< XInterface > xScriptStorage =
                xScriptStorageManager->getScriptStorage( m_sid );
            validateXRef( xScriptStorage,
                          "StorageBridge::StorageBridge: cannot get Script Storage service" );
            m_xScriptImplAccess =
                Reference< storage::XScriptImplAccess > ( xScriptStorage, UNO_QUERY_THROW );
        }
    }
    catch ( Exception e )
    {
        OUString temp = OUSTR( "StorageBridge::StorageBridge: " );
        throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
    }
}
//*************************************************************************
Sequence < Reference< scripturi::XScriptURI > >
StorageBridge::getImplementations( const Reference< scripturi::XScriptURI >& queryURI )
throw ( lang::IllegalArgumentException, RuntimeException )
{
    OSL_TRACE( "In StorageBridge getImplementations...\n" );
    Sequence < Reference< scripturi::XScriptURI > > results;
    try
    {
        results = m_xScriptImplAccess->getImplementations( queryURI );
    }
    catch ( Exception e )
    {
        OUString temp = OUSTR( "StorageBridge::getImplementations: " );
        throw RuntimeException( temp.concat( e.Message ), Reference< XInterface >() );
    }
    return results;
}
}// namespace

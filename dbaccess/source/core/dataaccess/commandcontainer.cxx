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
#include "precompiled_dbaccess.hxx"

#ifndef _DBA_COREDATAACCESS_COMMANDCONTAINER_HXX_
#include "commandcontainer.hxx"
#endif
#ifndef _DBA_COREDATAACESS_COMMANDDEFINITION_HXX_
#include "commanddefinition.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ucb;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;

//........................................................................
namespace dbaccess
{
//........................................................................

//==========================================================================
//= OCommandContainer
//==========================================================================
DBG_NAME(OCommandContainer)
//--------------------------------------------------------------------------
OCommandContainer::OCommandContainer( const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xORB
                                     ,const Reference< XInterface >&    _xParentContainer
                                     ,const TContentPtr& _pImpl
                                     ,sal_Bool _bTables
                                     )
    :ODefinitionContainer(_xORB,_xParentContainer,_pImpl,!_bTables)
    ,m_bTables(_bTables)
{
    DBG_CTOR(OCommandContainer, NULL);
}
//--------------------------------------------------------------------------
OCommandContainer::~OCommandContainer()
{
    DBG_DTOR(OCommandContainer, NULL);
}
// -----------------------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2( OCommandContainer,ODefinitionContainer,OCommandContainer_BASE)
IMPLEMENT_TYPEPROVIDER2(OCommandContainer,ODefinitionContainer,OCommandContainer_BASE);
//--------------------------------------------------------------------------
Reference< XContent > OCommandContainer::createObject( const ::rtl::OUString& _rName)
{
    const ODefinitionContainer_Impl& rDefinitions( getDefinitions() );
    OSL_ENSURE( rDefinitions.find(_rName) != rDefinitions.end(), "OCommandContainer::createObject: Invalid entry in map!" );

    const TContentPtr& pElementContent( rDefinitions.find( _rName )->second );
    if ( m_bTables )
        return new OComponentDefinition( *this, _rName, m_aContext.getLegacyServiceFactory(), pElementContent, m_bTables );
    return new OCommandDefinition( *this, _rName, m_aContext.getLegacyServiceFactory(), pElementContent );
}

// -----------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OCommandContainer::createInstanceWithArguments(const Sequence< Any >& /*aArguments*/ ) throw (Exception, RuntimeException)
{
    return createInstance( );
}

// -----------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OCommandContainer::createInstance( ) throw (Exception, RuntimeException)
{
    return m_aContext.createComponent( (::rtl::OUString)( m_bTables ? SERVICE_SDB_TABLEDEFINITION : SERVICE_SDB_COMMAND_DEFINITION ) );
}

// -----------------------------------------------------------------------------
::rtl::OUString OCommandContainer::determineContentType() const
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "application/vnd.org.openoffice.DatabaseCommandDefinitionContainer" ) );
}

//........................................................................
}   // namespace dbaccess
//........................................................................


/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: commandcontainer.cxx,v $
 * $Revision: 1.16 $
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


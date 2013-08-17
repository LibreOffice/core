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

#include "commandcontainer.hxx"
#include "commanddefinition.hxx"

#include <tools/debug.hxx>
#include "dbastrings.hrc"
#include <com/sun/star/sdb/TableDefinition.hpp>
#include <com/sun/star/sdb/CommandDefinition.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ucb;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;

namespace dbaccess
{

// OCommandContainer
DBG_NAME(OCommandContainer)

OCommandContainer::OCommandContainer( const Reference< ::com::sun::star::uno::XComponentContext >& _xORB
                                     ,const Reference< XInterface >&    _xParentContainer
                                     ,const TContentPtr& _pImpl
                                     ,sal_Bool _bTables
                                     )
    :ODefinitionContainer(_xORB,_xParentContainer,_pImpl,!_bTables)
    ,m_bTables(_bTables)
{
    DBG_CTOR(OCommandContainer, NULL);
}

OCommandContainer::~OCommandContainer()
{
    DBG_DTOR(OCommandContainer, NULL);
}

IMPLEMENT_FORWARD_XINTERFACE2( OCommandContainer,ODefinitionContainer,OCommandContainer_BASE)
IMPLEMENT_TYPEPROVIDER2(OCommandContainer,ODefinitionContainer,OCommandContainer_BASE);

Reference< XContent > OCommandContainer::createObject( const OUString& _rName)
{
    const ODefinitionContainer_Impl& rDefinitions( getDefinitions() );
    OSL_ENSURE( rDefinitions.find(_rName) != rDefinitions.end(), "OCommandContainer::createObject: Invalid entry in map!" );

    const TContentPtr& pElementContent( rDefinitions.find( _rName )->second );
    if ( m_bTables )
        return new OComponentDefinition( *this, _rName, m_aContext, pElementContent, m_bTables );
    else
        return static_cast< css::sdb::XQueryDefinition * > ( new OCommandDefinition( *this, _rName, m_aContext, pElementContent ) );
}

Reference< XInterface > SAL_CALL OCommandContainer::createInstanceWithArguments(const Sequence< Any >& /*aArguments*/ ) throw (Exception, RuntimeException)
{
    return createInstance( );
}

Reference< XInterface > SAL_CALL OCommandContainer::createInstance( ) throw (Exception, RuntimeException)
{
    if(m_bTables)
        return css::sdb::TableDefinition::createDefault( m_aContext );
    else
        return css::sdb::CommandDefinition::create( m_aContext );
}

OUString OCommandContainer::determineContentType() const
{
    return OUString( "application/vnd.org.openoffice.DatabaseCommandDefinitionContainer" );
}

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#include "precompiled_xmlhelp.hxx"
#include <com/sun/star/sdbc/XResultSet.hpp>

#include "resultset.hxx"
#include "resultsetfactory.hxx"

using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;


using namespace chelp;

//=========================================================================
//=========================================================================
//
// DynamicResultSet Implementation.
//
//=========================================================================
//=========================================================================

DynamicResultSet::DynamicResultSet(
    const Reference< XMultiServiceFactory >& rxSMgr,
    const vos::ORef< Content >& rxContent,
    const OpenCommandArgument2& rCommand,
    const Reference< XCommandEnvironment >& rxEnv,
    ResultSetFactory* pFactory )
    : ResultSetImplHelper( rxSMgr, rCommand ),
      m_xContent( rxContent ),
      m_xEnv( rxEnv ),
      m_pFactory( pFactory )
{
}

DynamicResultSet::~DynamicResultSet()
{
    delete m_pFactory;
}

//=========================================================================
//
// Non-interface methods.
//
//=========================================================================

void DynamicResultSet::initStatic()
{
    m_xResultSet1 = Reference< XResultSet >( m_pFactory->createResultSet() );
}

//=========================================================================
void DynamicResultSet::initDynamic()
{
    m_xResultSet1 = Reference< XResultSet >( m_pFactory->createResultSet() );

    m_xResultSet2 = m_xResultSet1;
}





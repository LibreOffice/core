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
#include "precompiled_xmloff.hxx"
#include "TransformerActions.hxx"


using ::rtl::OUString;

using namespace ::osl;
using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;

XMLTransformerActions::XMLTransformerActions( XMLTransformerActionInit *pInit )
{
    if( pInit )
    {
        XMLTransformerActions::key_type aKey;
        XMLTransformerActions::mapped_type aData;
        while( pInit->m_nActionType != XML_TACTION_EOT )
        {
            aKey.m_nPrefix = pInit->m_nPrefix;
            aKey.SetLocalName( pInit->m_eLocalName );

            OSL_ENSURE( find( aKey ) == end(), "duplicate action map entry" );

            aData.m_nActionType  = pInit->m_nActionType;
            aData.m_nParam1 = pInit->m_nParam1;
            aData.m_nParam2 = pInit->m_nParam2;
            aData.m_nParam3 = pInit->m_nParam3;
            XMLTransformerActions::value_type aVal( aKey, aData );

            insert( aVal );
            ++pInit;
        }
    }
}

XMLTransformerActions::~XMLTransformerActions()
{
}

void XMLTransformerActions::Add( XMLTransformerActionInit *pInit )
{
    if( pInit )
    {
        XMLTransformerActions::key_type aKey;
        XMLTransformerActions::mapped_type aData;
        while( pInit->m_nActionType != XML_TACTION_EOT )
        {
            aKey.m_nPrefix = pInit->m_nPrefix;
            aKey.SetLocalName( pInit->m_eLocalName );
            XMLTransformerActions::iterator aIter = find( aKey );
            if( aIter == end() )
            {
                aData.m_nActionType  = pInit->m_nActionType;
                aData.m_nParam1 = pInit->m_nParam1;
                aData.m_nParam2 = pInit->m_nParam2;
                aData.m_nParam3 = pInit->m_nParam3;
                XMLTransformerActions::value_type aVal( aKey, aData );
                insert( aVal );
            }

            ++pInit;
        }
    }
}

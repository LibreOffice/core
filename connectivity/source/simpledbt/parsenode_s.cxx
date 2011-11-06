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
#include "precompiled_connectivity.hxx"
#include <connectivity/virtualdbtools.hxx>
#include "parsenode_s.hxx"
#include <connectivity/sqlnode.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;

//........................................................................
namespace connectivity
{
//........................................................................

    //================================================================
    //= OSimpleParseNode
    //================================================================
    //----------------------------------------------------------------
    OSimpleParseNode::OSimpleParseNode(const OSQLParseNode* _pNode, sal_Bool _bTakeOwnership)
        :m_pFullNode(_pNode)
        ,m_bOwner(_bTakeOwnership)
    {
        OSL_ENSURE(m_pFullNode, "OSimpleParseNode::OSimpleParseNode: invalid node given!");
    }

    //----------------------------------------------------------------
    OSimpleParseNode::~OSimpleParseNode()
    {
        if (m_bOwner)
            delete m_pFullNode;
    }

    //----------------------------------------------------------------
    oslInterlockedCount SAL_CALL OSimpleParseNode::acquire()
    {
        return ORefBase::acquire();
    }

    //----------------------------------------------------------------
    oslInterlockedCount SAL_CALL OSimpleParseNode::release()
    {
        return ORefBase::release();
    }

    //----------------------------------------------------------------
    void OSimpleParseNode::parseNodeToStr(::rtl::OUString& _rString, const Reference< XConnection >& _rxConnection,const IParseContext* _pContext) const
    {
        m_pFullNode->parseNodeToStr( _rString, _rxConnection, _pContext );
    }

    //----------------------------------------------------------------
    void OSimpleParseNode::parseNodeToPredicateStr(::rtl::OUString& _rString, const Reference< XConnection >& _rxConnection,
        const Reference< XNumberFormatter >& _rxFormatter, const Reference< XPropertySet >& _rxField,
        const Locale& _rIntl, const sal_Char _cDecSeparator,const IParseContext* _pContext) const
    {
        m_pFullNode->parseNodeToPredicateStr( _rString, _rxConnection, _rxFormatter, _rxField, _rIntl, _cDecSeparator, _pContext );
    }

//........................................................................
}   // namespace connectivity
//........................................................................



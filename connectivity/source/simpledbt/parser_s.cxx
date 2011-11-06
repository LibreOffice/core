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
#include "parser_s.hxx"
#include "parsenode_s.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

//........................................................................
namespace connectivity
{
//........................................................................

    //================================================================
    //= OSimpleSQLParser
    //================================================================
    //----------------------------------------------------------------
    OSimpleSQLParser::OSimpleSQLParser(const Reference< XMultiServiceFactory >& _rxServiceFactory,const IParseContext* _pContext)
        :m_aFullParser(_rxServiceFactory,_pContext)
    {
    }

    //----------------------------------------------------------------
    oslInterlockedCount SAL_CALL OSimpleSQLParser::acquire()
    {
        return ORefBase::acquire();
    }

    //----------------------------------------------------------------
    oslInterlockedCount SAL_CALL OSimpleSQLParser::release()
    {
        return ORefBase::release();
    }

    //----------------------------------------------------------------
    const IParseContext& OSimpleSQLParser::getContext() const
    {
        return m_aFullParser.getContext();
    }

    //----------------------------------------------------------------
    ::rtl::Reference< simple::ISQLParseNode > OSimpleSQLParser::predicateTree(::rtl::OUString& rErrorMessage, const ::rtl::OUString& rStatement,
        const Reference< XNumberFormatter >& _rxFormatter, const Reference< XPropertySet >& _rxField) const
    {
        OSimpleParseNode* pReturn = NULL;
        OSQLParseNode* pFullNode = const_cast<OSimpleSQLParser*>(this)->m_aFullParser.predicateTree(rErrorMessage, rStatement, _rxFormatter, _rxField);
        if (pFullNode)
            pReturn = new OSimpleParseNode(pFullNode, sal_True);
        return pReturn;
    }

//........................................................................
}   // namespace connectivity
//........................................................................



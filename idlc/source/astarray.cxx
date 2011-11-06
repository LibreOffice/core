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
#include "precompiled_idlc.hxx"
#include <idlc/astarray.hxx>

using namespace ::rtl;

AstArray::AstArray(const OString& name, AstType* pType, const ExprList& rDimExpr, AstScope* pScope)
    : AstType(NT_array, name, pScope)
    , m_pType(pType)
    , m_dimension((sal_uInt32)(rDimExpr.size()))
    , m_dimExpressions(rDimExpr)
{
    if ( m_pType )
        setName(makeName());
}

AstArray::AstArray(AstType* pType, const ExprList& rDimExpr, AstScope* pScope)
    : AstType(NT_array, OString("arrary_"), pScope)
    , m_pType(pType)
    , m_dimension((sal_uInt32)(rDimExpr.size()))
    , m_dimExpressions(rDimExpr)
{
    if ( m_pType )
        setName(makeName());
}

OString AstArray::makeName()
{
    if ( m_pType )
    {
        OString name(m_pType->getScopedName());
        OString openBracket("[");
        OString closeBracket("]");
        ExprList::iterator iter = m_dimExpressions.begin();
        ExprList::iterator end = m_dimExpressions.end();

        while ( iter != end )
        {
            name += openBracket;
            name += (*iter)->toString();
            name += closeBracket;
            ++iter;
        }
        return name;
    }
    return OString();
}

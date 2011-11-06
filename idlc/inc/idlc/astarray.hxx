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


#ifndef _IDLC_ASTARRAY_HXX_
#define _IDLC_ASTARRAY_HXX_

#include <idlc/asttype.hxx>
#include <idlc/astscope.hxx>

class AstArray : public AstType
{
public:
    AstArray(const ::rtl::OString& name, AstType* pType, const ExprList& rDimExpr, AstScope* pScope);
    AstArray(AstType* pType, const ExprList& rDimExpr, AstScope* pScope);

    virtual ~AstArray() {}

    AstType* getType()
        { return m_pType; }
    void setType(AstType* pType)
        {
            m_pType = pType;
            setName(makeName());
        }
    ExprList* getDimExpressions()
        {   return &m_dimExpressions; }
    sal_uInt32 getDimension()
        {   return m_dimension; }

private:
    ::rtl::OString makeName();

    AstType*    m_pType;
    sal_uInt32  m_dimension;
    ExprList    m_dimExpressions;
};

#endif // _IDLC_ASTARRAY_HXX_


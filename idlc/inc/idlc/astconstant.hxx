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


#ifndef _IDLC_ASTCONSTANT_HXX_
#define _IDLC_ASTCONSTANT_HXX_

#include <idlc/astdeclaration.hxx>
#include <idlc/astexpression.hxx>

namespace typereg { class Writer; }

class AstConstant : public AstDeclaration
{
public:
    AstConstant(const ExprType type, const NodeType nodeType,
                AstExpression* pExpr, const ::rtl::OString& name, AstScope* pScope);
    AstConstant(const ExprType type, AstExpression* pExpr,
                const ::rtl::OString& name, AstScope* pScope);
    virtual ~AstConstant();

    AstExpression* getConstValue()
        { return m_pConstValue; }
    ExprType getConstValueType() const
        { return m_constValueType; }

    sal_Bool dumpBlob(
        typereg::Writer & rBlob, sal_uInt16 index, bool published);
private:
    AstExpression*                  m_pConstValue;
    const ExprType  m_constValueType;
};

#endif // _IDLC_ASTCONSTANT_HXX_


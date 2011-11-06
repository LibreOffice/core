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


#ifndef _IDLC_ASTBASETYPE_HXX_
#define _IDLC_ASTBASETYPE_HXX_

#include <idlc/asttype.hxx>
#include <idlc/astexpression.hxx>


class AstBaseType : public AstType
{
public:
    AstBaseType(const ExprType type, const ::rtl::OString& name, AstScope* pScope)
        : AstType(NT_predefined, name, pScope)
        , m_exprType(type)
        {}

    virtual ~AstBaseType() {}

    virtual bool isUnsigned() const {
        switch (m_exprType) {
        case ET_ushort:
        case ET_ulong:
        case ET_uhyper:
            return true;
        default:
            return false;
        }
    }

    ExprType getExprType() const
        { return m_exprType; }
private:
    const ExprType m_exprType;
};

#endif // _IDLC_ASTBASETYPE_HXX_


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


#ifndef _IDLC_ASTOPERATION_HXX_
#define _IDLC_ASTOPERATION_HXX_

#include <idlc/astdeclaration.hxx>
#include <idlc/astscope.hxx>

namespace typereg { class Writer; }

#define OP_NONE         0x0000
#define OP_ONEWAY       0x0001

class AstType;

class AstOperation : public AstDeclaration
                   , public AstScope
{
public:
    AstOperation(sal_uInt32 flags, AstType* pReturnType, const ::rtl::OString& name, AstScope* pScope)
        : AstDeclaration(NT_operation, name, pScope)
        , AstScope(NT_operation)
        , m_flags(flags)
        , m_pReturnType(pReturnType)
        {}
    virtual ~AstOperation() {}

    sal_Bool isOneway()
        { return ((m_flags & OP_ONEWAY) == OP_ONEWAY); }

    bool isVariadic() const;

    bool isConstructor() const { return m_pReturnType == 0; }

    void setExceptions(DeclList const * pExceptions);
    const DeclList& getExceptions()
        { return m_exceptions; }
    sal_uInt16 nExceptions()
        { return (sal_uInt16)(m_exceptions.size()); }

    sal_Bool dumpBlob(typereg::Writer & rBlob, sal_uInt16 index);

    // scope management
    virtual AstDeclaration* addDeclaration(AstDeclaration* pDecl);
private:
    sal_uInt32  m_flags;
    AstType*    m_pReturnType;
    DeclList    m_exceptions;
};

#endif // _IDLC_ASTOPERATION_HXX_


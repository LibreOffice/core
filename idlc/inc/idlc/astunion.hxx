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


#ifndef _IDLC_ASTUNION_HXX_
#define _IDLC_ASTUNION_HXX_

#include <idlc/aststruct.hxx>
#include <idlc/astunionbranch.hxx>

class AstUnion : public AstStruct
{
public:
    AstUnion(const ::rtl::OString& name, AstType* pDiscType, AstScope* pScope);
    virtual ~AstUnion();

    AstType* getDiscrimantType()
        { return m_pDiscriminantType; }
    ExprType getDiscrimantExprType()
        { return m_discExprType; }

    virtual sal_Bool dump(RegistryKey& rKey);

    virtual AstDeclaration* addDeclaration(AstDeclaration* pDecl);
protected:
    // Look up a branch by node pointer
    AstUnionBranch* lookupBranch(AstUnionBranch* pBranch);

    // Look up the branch with the "default" label
    AstUnionBranch* lookupDefault(sal_Bool bReportError = sal_True );

    // Look up a branch given a branch with a label. This is used to
    // check for duplicate labels
    AstUnionBranch* lookupLabel(AstUnionBranch* pBranch);

    // Look up a union branch given an enumerator. This is used to
    // check for duplicate enum labels
    AstUnionBranch* lookupEnum(AstUnionBranch* pBranch);

private:
    AstType*    m_pDiscriminantType;
    ExprType    m_discExprType;
};

#endif // _IDLC_ASTUNION_HXX_


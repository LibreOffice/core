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


#ifndef _IDLC_ASTSTRUCT_HXX_
#define _IDLC_ASTSTRUCT_HXX_

#include <idlc/asttype.hxx>
#include <idlc/astscope.hxx>
#include "idlc/idlctypes.hxx"

class AstStruct;
typedef ::std::vector< AstStruct* > InheritedTypes;

class AstStruct : public AstType
                , public AstScope
{
public:
    AstStruct(
        const ::rtl::OString& name,
        std::vector< rtl::OString > const & typeParameters,
        AstStruct* pBaseType, AstScope* pScope);

    AstStruct(const NodeType type,
              const ::rtl::OString& name,
              AstStruct* pBaseType,
              AstScope* pScope);
    virtual ~AstStruct();

    AstStruct* getBaseType()
        { return m_pBaseType; }

    DeclList::size_type getTypeParameterCount() const
    { return m_typeParameters.size(); }

    AstDeclaration const * findTypeParameter(rtl::OString const & name) const;

    virtual bool isType() const;

    virtual sal_Bool dump(RegistryKey& rKey);
private:
    AstStruct* m_pBaseType;
    DeclList m_typeParameters;
};

#endif // _IDLC_ASTSTRUCT_HXX_


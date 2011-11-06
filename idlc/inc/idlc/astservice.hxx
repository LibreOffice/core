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


#ifndef _IDLC_ASTSERVICE_HXX_
#define _IDLC_ASTSERVICE_HXX_

#include <idlc/astdeclaration.hxx>
#include <idlc/astscope.hxx>

class AstService : public AstDeclaration
                 , public AstScope
{
public:
    AstService(const ::rtl::OString& name, AstScope* pScope)
        : AstDeclaration(NT_service, name, pScope)
        , AstScope(NT_service)
        , m_defaultConstructor(false)
        {}
    AstService(const NodeType type, const ::rtl::OString& name, AstScope* pScope)
        : AstDeclaration(type, name, pScope)
        , AstScope(type)
        , m_defaultConstructor(false)
        {}
    virtual ~AstService() {}

    virtual sal_Bool dump(RegistryKey& rKey);

    void setDefaultConstructor(bool b) { m_defaultConstructor = b; }

    bool checkLastConstructor() const;

private:
    bool m_defaultConstructor;
};

#endif // _IDLC_ASTSERVICE_HXX_


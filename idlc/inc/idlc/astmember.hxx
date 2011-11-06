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



#ifndef _IDLC_ASTMEMBER_HXX_
#define _IDLC_ASTMEMBER_HXX_

#include "idlc/astdeclaration.hxx"

namespace rtl { class OString; }
class AstScope;
class AstType;

class AstMember: public AstDeclaration {
public:
    AstMember(
        AstType const * pType, rtl::OString const & name, AstScope * pScope):
        AstDeclaration(NT_member, name, pScope), m_pType(pType) {}

    virtual ~AstMember() {}

    AstType const * getType() const { return m_pType; }

protected:
    AstMember(
        NodeType type, AstType const * pType, rtl::OString const & name,
        AstScope * pScope):
        AstDeclaration(type, name, pScope), m_pType(pType) {}

private:
    AstType const * m_pType;
};

#endif

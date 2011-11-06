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


#ifndef _IDLC_ASTSEQUENCE_HXX_
#define _IDLC_ASTSEQUENCE_HXX_

#include <idlc/asttype.hxx>

class AstSequence : public AstType
{
public:
    AstSequence(AstType* pMemberType, AstScope* pScope)
        : AstType(NT_sequence, ::rtl::OString("[]")+pMemberType->getScopedName(), pScope)
        , m_pMemberType(pMemberType)
        , m_pRelativName(NULL)
    {}
    virtual ~AstSequence()
    {
        if ( m_pRelativName )
            delete m_pRelativName;
    }

    AstType* getMemberType() const
        { return m_pMemberType; }

    virtual bool isUnsigned() const
    { return m_pMemberType != 0 && m_pMemberType->isUnsigned(); }

    virtual const sal_Char* getRelativName() const;
private:
    AstType*        m_pMemberType;
    mutable ::rtl::OString* m_pRelativName;
};

#endif // _IDLC_ASTSEQUENCE_HXX_


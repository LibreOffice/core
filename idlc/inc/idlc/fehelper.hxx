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


#ifndef _IDLC_FEHELPER_HXX_
#define _IDLC_FEHELPER_HXX_

#include <idlc/asttype.hxx>
#include <idlc/astinterface.hxx>

#include <vector>

class FeDeclarator
{
public:
    // Enum to denote types of declarators
    enum DeclaratorType
    {
        FD_simple,      // Simple declarator
        FD_complex      // Complex declarator (complex_part field used)
    };

    FeDeclarator(const ::rtl::OString& name, DeclaratorType declType, AstDeclaration* pComplPart);
    virtual ~FeDeclarator();

    AstDeclaration* getComplexPart()
        { return m_pComplexPart; }
    const ::rtl::OString& getName()
        { return m_name; }
    DeclaratorType  getDeclType()
        { return m_declType; }

    sal_Bool checkType(AstDeclaration const * pType);
    AstType const * compose(AstDeclaration const * pDecl);
private:
    AstDeclaration* m_pComplexPart;
    ::rtl::OString  m_name;
    DeclaratorType  m_declType;
};

typedef ::std::list< FeDeclarator* > FeDeclList;

class FeInheritanceHeader
{
public:
    FeInheritanceHeader(
        NodeType nodeType, ::rtl::OString* pName, ::rtl::OString* pInherits,
        std::vector< rtl::OString > * typeParameters);

    virtual ~FeInheritanceHeader()
    {
        if ( m_pName )
            delete m_pName;
     }

    NodeType getNodeType()
        { return m_nodeType; }
    ::rtl::OString* getName()
        { return m_pName; }
    AstDeclaration* getInherits()
        { return m_pInherits; }

    std::vector< rtl::OString > const & getTypeParameters() const
    { return m_typeParameters; }

private:
    void initializeInherits(::rtl::OString* pinherits);

    NodeType        m_nodeType;
    ::rtl::OString* m_pName;
    AstDeclaration* m_pInherits;
    std::vector< rtl::OString > m_typeParameters;
};

#endif // _IDLC_FEHELPER_HXX_


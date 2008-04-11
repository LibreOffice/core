/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fehelper.hxx,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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


/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: astscope.hxx,v $
 * $Revision: 1.7 $
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
#ifndef _IDLC_ASTSCOPE_HXX_
#define _IDLC_ASTSCOPE_HXX_

#include <idlc/idlc.hxx>

class AstExpression;
#include <idlc/astdeclaration.hxx>
#include <idlc/astexpression.hxx>

class AstScope
{
public:
    AstScope(NodeType nodeType);
    virtual ~AstScope();

    const NodeType getScopeNodeType()
        { return m_nodeType; }

    virtual AstDeclaration* addDeclaration(AstDeclaration* pDecl);

    sal_uInt32 nMembers() const
        { return (sal_uInt32)(m_declarations.size()); }
    DeclList::const_iterator getIteratorBegin() const
        { return m_declarations.begin(); }
    DeclList::const_iterator getIteratorEnd() const
        { return m_declarations.end(); }
    sal_uInt16 getNodeCount(NodeType nType);

    // Name look up mechanism
    AstDeclaration* lookupByName(const ::rtl::OString& scopedName);
    // Look up the identifier 'name' specified only in the local scope
    AstDeclaration* lookupByNameLocal(const ::rtl::OString& name) const;

    AstDeclaration* lookupInForwarded(const ::rtl::OString& scopedName);
    AstDeclaration* lookupInInherited(const ::rtl::OString& scopedName) const;

    // Look up a predefined type by its ExprType
    AstDeclaration* lookupPrimitiveType(ExprType type);

    AstDeclaration* lookupForAdd(AstDeclaration* pDecl);

protected:
    inline AstDeclaration const * getLast() const
    { return m_declarations.back(); }

private:
    DeclList    m_declarations;
    NodeType    m_nodeType;
};

#endif // _IDLC_ASTSCOPE_HXX_


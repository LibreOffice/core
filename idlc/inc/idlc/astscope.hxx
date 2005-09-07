/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: astscope.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:00:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _IDLC_ASTSCOPE_HXX_
#define _IDLC_ASTSCOPE_HXX_

#ifndef _IDLC_IDLC_HXX_
#include <idlc/idlc.hxx>
#endif

class AstExpression;
#ifndef _IDLC_ASTDECLARATION_HXX_
#include <idlc/astdeclaration.hxx>
#endif
#ifndef _IDLC_ASTEXPRESSION_HXX_
#include <idlc/astexpression.hxx>
#endif

class AstScope
{
public:
    AstScope(NodeType nodeType);
    virtual ~AstScope();

    const NodeType getScopeNodeType()
        { return m_nodeType; }

    virtual AstDeclaration* addDeclaration(AstDeclaration* pDecl);

    sal_uInt32 nMembers()
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

private:
    DeclList        m_declarations;
    const NodeType  m_nodeType;
};

#endif // _IDLC_ASTSCOPE_HXX_


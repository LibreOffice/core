/*************************************************************************
 *
 *  $RCSfile: astscope.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jsc $ $Date: 2001-03-15 12:23:01 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
        { return m_declarations.size(); }
    DeclList::iterator getIteratorBegin()
        { return m_declarations.begin(); }
    DeclList::iterator getIteratorEnd()
        { return m_declarations.end(); }
    sal_uInt16 getNodeCount(NodeType nType);

    // Name look up mechanism
    AstDeclaration* lookupByName(const ::rtl::OString& scopedName);
    // Look up the identifier 'name' specified only in the local scope
    AstDeclaration* lookupByNameLocal(const ::rtl::OString& name);

    AstDeclaration* lookupInForwarded(const ::rtl::OString& scopedName);
    AstDeclaration* lookupInInherited(const ::rtl::OString& scopedName);

    // Look up a predefined type by its ExprType
    AstDeclaration* lookupPrimitiveType(ExprType type);

    AstDeclaration* lookupForAdd(AstDeclaration* pDecl);

private:
    DeclList        m_declarations;
    const NodeType  m_nodeType;
};

#endif // _IDLC_ASTSCOPE_HXX_


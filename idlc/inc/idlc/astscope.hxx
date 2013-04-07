/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
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

    NodeType getScopeNodeType() const
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
    AstDeclaration* lookupByName(const OString& scopedName);
    // Look up the identifier 'name' specified only in the local scope
    AstDeclaration* lookupByNameLocal(const OString& name) const;

    AstDeclaration* lookupInForwarded(const OString& scopedName);
    AstDeclaration* lookupInInherited(const OString& scopedName) const;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

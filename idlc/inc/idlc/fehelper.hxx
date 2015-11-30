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
#ifndef INCLUDED_IDLC_INC_IDLC_FEHELPER_HXX
#define INCLUDED_IDLC_INC_IDLC_FEHELPER_HXX

#include <idlc/asttype.hxx>
#include <idlc/astinterface.hxx>

#include <vector>

class FeDeclarator
{
public:
    FeDeclarator(const OString& name);
    virtual ~FeDeclarator();
    const OString& getName()
        { return m_name; }
    bool checkType(AstDeclaration const * pType);
    static AstType const * compose(AstDeclaration const * pDecl);
private:
    OString  m_name;
};

typedef ::std::list< FeDeclarator* > FeDeclList;

class FeInheritanceHeader
{
public:
    FeInheritanceHeader(
        NodeType nodeType, OString* pName, OString* pInherits,
        std::vector< OString > * typeParameters);

    virtual ~FeInheritanceHeader()
    {
        if ( m_pName )
            delete m_pName;
     }

    NodeType getNodeType()
        { return m_nodeType; }
    OString* getName()
        { return m_pName; }
    AstDeclaration* getInherits()
        { return m_pInherits; }

    std::vector< OString > const & getTypeParameters() const
    { return m_typeParameters; }

private:
    void initializeInherits(OString* pinherits);

    NodeType        m_nodeType;
    OString* m_pName;
    AstDeclaration* m_pInherits;
    std::vector< OString > m_typeParameters;
};

#endif // INCLUDED_IDLC_INC_IDLC_FEHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

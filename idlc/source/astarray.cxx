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

#include <idlc/astarray.hxx>

using namespace ::rtl;

AstArray::AstArray(const OString& name, AstType* pType, const ExprList& rDimExpr, AstScope* pScope)
    : AstType(NT_array, name, pScope)
    , m_pType(pType)
    , m_dimension((sal_uInt32)(rDimExpr.size()))
    , m_dimExpressions(rDimExpr)
{
    if ( m_pType )
        setName(makeName());
}

AstArray::AstArray(AstType* pType, const ExprList& rDimExpr, AstScope* pScope)
    : AstType(NT_array, OString("arrary_"), pScope)
    , m_pType(pType)
    , m_dimension((sal_uInt32)(rDimExpr.size()))
    , m_dimExpressions(rDimExpr)
{
    if ( m_pType )
        setName(makeName());
}

OString AstArray::makeName()
{
    if ( m_pType )
    {
        OString name(m_pType->getScopedName());
        OString openBracket("[");
        OString closeBracket("]");
        ExprList::iterator iter = m_dimExpressions.begin();
        ExprList::iterator end = m_dimExpressions.end();

        while ( iter != end )
        {
            name += openBracket;
            name += (*iter)->toString();
            name += closeBracket;
            ++iter;
        }
        return name;
    }
    return OString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: astarray.cxx,v $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_idlc.hxx"
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

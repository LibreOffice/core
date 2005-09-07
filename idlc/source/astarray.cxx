/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: astarray.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:05:39 $
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
#ifndef _IDLC_ASTARRAY_HXX_
#include <idlc/astarray.hxx>
#endif

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

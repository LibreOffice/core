/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: astarray.hxx,v $
 * $Revision: 1.3 $
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
#ifndef _IDLC_ASTARRAY_HXX_
#define _IDLC_ASTARRAY_HXX_

#include <idlc/asttype.hxx>
#include <idlc/astscope.hxx>

class AstArray : public AstType
{
public:
    AstArray(const ::rtl::OString& name, AstType* pType, const ExprList& rDimExpr, AstScope* pScope);
    AstArray(AstType* pType, const ExprList& rDimExpr, AstScope* pScope);

    virtual ~AstArray() {}

    AstType* getType()
        { return m_pType; }
    void setType(AstType* pType)
        {
            m_pType = pType;
            setName(makeName());
        }
    ExprList* getDimExpressions()
        {   return &m_dimExpressions; }
    sal_uInt32 getDimension()
        {   return m_dimension; }

private:
    ::rtl::OString makeName();

    AstType*    m_pType;
    sal_uInt32  m_dimension;
    ExprList    m_dimExpressions;
};

#endif // _IDLC_ASTARRAY_HXX_


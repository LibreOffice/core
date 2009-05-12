/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: astenum.hxx,v $
 * $Revision: 1.4 $
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
#ifndef _IDLC_ASTENUM_HXX_
#define _IDLC_ASTENUM_HXX_

#include <idlc/asttype.hxx>
#include <idlc/astscope.hxx>
#include <idlc/astconstant.hxx>

class AstEnum : public AstType
              , public AstScope
{
public:
    AstEnum(const ::rtl::OString& name, AstScope* pScope);

    virtual ~AstEnum();

    void setEnumValueCount(sal_Int32 count)
        { m_enumValueCount = count; }
    sal_Int32 getEnumValueCount()
        { return m_enumValueCount++; }

    AstConstant* checkValue(AstExpression* pExpr);

    virtual sal_Bool dump(RegistryKey& rKey);

    virtual AstDeclaration* addDeclaration(AstDeclaration* pDecl);
private:
    sal_Int32   m_enumValueCount;
};

#endif // _IDLC_ASTENUM_HXX_


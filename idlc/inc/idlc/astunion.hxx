/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: astunion.hxx,v $
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
#ifndef _IDLC_ASTUNION_HXX_
#define _IDLC_ASTUNION_HXX_

#include <idlc/aststruct.hxx>
#include <idlc/astunionbranch.hxx>

class AstUnion : public AstStruct
{
public:
    AstUnion(const ::rtl::OString& name, AstType* pDiscType, AstScope* pScope);
    virtual ~AstUnion();

    AstType* getDiscrimantType()
        { return m_pDiscriminantType; }
    ExprType getDiscrimantExprType()
        { return m_discExprType; }

    virtual sal_Bool dump(RegistryKey& rKey);

    virtual AstDeclaration* addDeclaration(AstDeclaration* pDecl);
protected:
    // Look up a branch by node pointer
    AstUnionBranch* lookupBranch(AstUnionBranch* pBranch);

    // Look up the branch with the "default" label
    AstUnionBranch* lookupDefault(sal_Bool bReportError = sal_True );

    // Look up a branch given a branch with a label. This is used to
    // check for duplicate labels
    AstUnionBranch* lookupLabel(AstUnionBranch* pBranch);

    // Look up a union branch given an enumerator. This is used to
    // check for duplicate enum labels
    AstUnionBranch* lookupEnum(AstUnionBranch* pBranch);

private:
    AstType*    m_pDiscriminantType;
    ExprType    m_discExprType;
};

#endif // _IDLC_ASTUNION_HXX_


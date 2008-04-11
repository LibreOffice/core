/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: astoperation.hxx,v $
 * $Revision: 1.5 $
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
#ifndef _IDLC_ASTOPERATION_HXX_
#define _IDLC_ASTOPERATION_HXX_

#include <idlc/astdeclaration.hxx>
#include <idlc/astscope.hxx>

namespace typereg { class Writer; }

#define OP_NONE         0x0000
#define OP_ONEWAY       0x0001

class AstType;

class AstOperation : public AstDeclaration
                   , public AstScope
{
public:
    AstOperation(sal_uInt32 flags, AstType* pReturnType, const ::rtl::OString& name, AstScope* pScope)
        : AstDeclaration(NT_operation, name, pScope)
        , AstScope(NT_operation)
        , m_flags(flags)
        , m_pReturnType(pReturnType)
        {}
    virtual ~AstOperation() {}

    sal_Bool isOneway()
        { return ((m_flags & OP_ONEWAY) == OP_ONEWAY); }
    sal_Bool isVoid();

    bool isVariadic() const;

    bool isConstructor() const { return m_pReturnType == 0; }

    void setExceptions(DeclList const * pExceptions);
    const DeclList& getExceptions()
        { return m_exceptions; }
    sal_uInt16 nExceptions()
        { return (sal_uInt16)(m_exceptions.size()); }

    sal_Bool dumpBlob(typereg::Writer & rBlob, sal_uInt16 index);

    // scope management
    virtual AstDeclaration* addDeclaration(AstDeclaration* pDecl);
private:
    sal_uInt32  m_flags;
    AstType*    m_pReturnType;
    DeclList    m_exceptions;
};

#endif // _IDLC_ASTOPERATION_HXX_


/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: astenum.cxx,v $
 * $Revision: 1.10 $
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
#include <idlc/astenum.hxx>

#include "registry/version.h"
#include "registry/writer.hxx"

using namespace ::rtl;

AstEnum::AstEnum(const ::rtl::OString& name, AstScope* pScope)
    : AstType(NT_enum, name, pScope)
    , AstScope(NT_enum)
    , m_enumValueCount(0)
{
}

AstEnum::~AstEnum()
{
}

AstConstant* AstEnum::checkValue(AstExpression* pExpr)
{
    DeclList::const_iterator iter = getIteratorBegin();
    DeclList::const_iterator end = getIteratorEnd();
    AstConstant*        pConst = NULL;
    AstDeclaration*     pDecl = NULL;

    while ( iter != end)
    {
        pDecl = *iter;
        pConst = (AstConstant*)pDecl;

        if (pConst->getConstValue()->compare(pExpr))
            return pConst;

        ++iter;
    }

    if ( pExpr->getExprValue()->u.lval > m_enumValueCount )
        m_enumValueCount = pExpr->getExprValue()->u.lval + 1;

    return NULL;
}

sal_Bool AstEnum::dump(RegistryKey& rKey)
{
    RegistryKey localKey;
    if (rKey.createKey( OStringToOUString(getFullName(), RTL_TEXTENCODING_UTF8 ), localKey))
    {
        fprintf(stderr, "%s: warning, could not create key '%s' in '%s'\n",
                idlc()->getOptions()->getProgramName().getStr(),
                getFullName().getStr(), OUStringToOString(rKey.getRegistryName(), RTL_TEXTENCODING_UTF8).getStr());
        return sal_False;
    }

    OUString emptyStr;
    sal_uInt16 nConst = getNodeCount(NT_enum_val);
    if ( nConst > 0 )
    {
        typereg::Writer aBlob(
            m_bPublished ? TYPEREG_VERSION_1 : TYPEREG_VERSION_0,
            getDocumentation(), emptyStr, RT_TYPE_ENUM, m_bPublished,
            OStringToOUString(getRelativName(), RTL_TEXTENCODING_UTF8), 0,
            nConst, 0, 0);

        DeclList::const_iterator iter = getIteratorBegin();
        DeclList::const_iterator end = getIteratorEnd();
        AstDeclaration* pDecl = NULL;
        sal_uInt16 index = 0;
        while ( iter != end )
        {
            pDecl = *iter;
            if ( pDecl->getNodeType() == NT_enum_val )
                ((AstConstant*)pDecl)->dumpBlob(aBlob, index++, false);

            ++iter;
        }

        sal_uInt32 aBlobSize;
        void const * pBlob = aBlob.getBlob(&aBlobSize);

        if (localKey.setValue(emptyStr, RG_VALUETYPE_BINARY,
                                (RegValue)pBlob, aBlobSize))
        {
            fprintf(stderr, "%s: warning, could not set value of key \"%s\" in %s\n",
                    idlc()->getOptions()->getProgramName().getStr(),
                    getFullName().getStr(), OUStringToOString(localKey.getRegistryName(), RTL_TEXTENCODING_UTF8).getStr());
            return sal_False;
        }
    }

    return sal_True;
}

AstDeclaration* AstEnum::addDeclaration(AstDeclaration* pDecl)
{
    return AstScope::addDeclaration(pDecl);
}

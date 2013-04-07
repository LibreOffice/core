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

#include <idlc/astenum.hxx>

#include "registry/version.h"
#include "registry/writer.hxx"

using namespace ::rtl;

AstEnum::AstEnum(const OString& name, AstScope* pScope)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

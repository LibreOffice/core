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
#ifndef _IDLC_ASTOPERATION_HXX_
#define _IDLC_ASTOPERATION_HXX_

#include <idlc/astdeclaration.hxx>
#include <idlc/astscope.hxx>

namespace typereg { class Writer; }

class AstType;

class AstOperation : public AstDeclaration
                   , public AstScope
{
public:
    AstOperation(AstType* pReturnType, const OString& name, AstScope* pScope)
        : AstDeclaration(NT_operation, name, pScope)
        , AstScope(NT_operation)
        , m_pReturnType(pReturnType)
        {}
    virtual ~AstOperation() {}

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
    AstType*    m_pReturnType;
    DeclList    m_exceptions;
};

#endif // _IDLC_ASTOPERATION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

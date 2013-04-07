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
#ifndef _IDLC_ASTUNION_HXX_
#define _IDLC_ASTUNION_HXX_

#include <idlc/aststruct.hxx>
#include <idlc/astunionbranch.hxx>

class AstUnion : public AstStruct
{
public:
    AstUnion(const OString& name, AstType* pDiscType, AstScope* pScope);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

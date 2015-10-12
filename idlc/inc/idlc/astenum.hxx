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
#ifndef INCLUDED_IDLC_INC_IDLC_ASTENUM_HXX
#define INCLUDED_IDLC_INC_IDLC_ASTENUM_HXX

#include <idlc/asttype.hxx>
#include <idlc/astscope.hxx>
#include <idlc/astconstant.hxx>

class AstEnum : public AstType
              , public AstScope
{
public:
    AstEnum(const OString& name, AstScope* pScope);

    virtual ~AstEnum();

    sal_Int32 getEnumValueCount()
        { return m_enumValueCount++; }

    AstConstant* checkValue(AstExpression* pExpr);

    virtual bool dump(RegistryKey& rKey) override;

    virtual AstDeclaration* addDeclaration(AstDeclaration* pDecl) override;
private:
    sal_Int32   m_enumValueCount;
};

#endif // INCLUDED_IDLC_INC_IDLC_ASTENUM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

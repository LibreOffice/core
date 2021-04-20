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
#ifndef INCLUDED_IDLC_INC_ASTSEQUENCE_HXX
#define INCLUDED_IDLC_INC_ASTSEQUENCE_HXX

#include "asttype.hxx"

class AstSequence final : public AstType
{
public:
    AstSequence(AstType const* pMemberType, AstScope* pScope)
        : AstType(NT_sequence, OString::Concat("[]") + pMemberType->getScopedName(), pScope)
        , m_pMemberType(pMemberType)
    {
    }

    AstType const* getMemberType() const { return m_pMemberType; }

    virtual bool isUnsigned() const override
    {
        return m_pMemberType != nullptr && m_pMemberType->isUnsigned();
    }

    virtual const char* getRelativName() const override;

private:
    AstType const* m_pMemberType;
    mutable std::optional<OString> m_xRelativName;
};

#endif // INCLUDED_IDLC_INC_ASTSEQUENCE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#ifndef _IDLC_ASTSTRUCT_HXX_
#define _IDLC_ASTSTRUCT_HXX_

#include <idlc/asttype.hxx>
#include <idlc/astscope.hxx>
#include "idlc/idlctypes.hxx"

class AstStruct;
typedef ::std::vector< AstStruct* > InheritedTypes;

class AstStruct : public AstType
                , public AstScope
{
public:
    AstStruct(
        const OString& name,
        std::vector< OString > const & typeParameters,
        AstStruct const* pBaseType, AstScope* pScope);

    AstStruct(const NodeType type,
              const OString& name,
              AstStruct const* pBaseType,
              AstScope* pScope);
    virtual ~AstStruct();

    AstStruct const* getBaseType()
        { return m_pBaseType; }

    DeclList::size_type getTypeParameterCount() const
    { return m_typeParameters.size(); }

    AstDeclaration const * findTypeParameter(OString const & name) const;

    virtual bool isType() const;

    virtual sal_Bool dump(RegistryKey& rKey);
private:
    AstStruct const* m_pBaseType;
    DeclList m_typeParameters;
};

#endif // _IDLC_ASTSTRUCT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

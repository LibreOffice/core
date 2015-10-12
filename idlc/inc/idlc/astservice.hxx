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
#ifndef INCLUDED_IDLC_INC_IDLC_ASTSERVICE_HXX
#define INCLUDED_IDLC_INC_IDLC_ASTSERVICE_HXX

#include <idlc/astdeclaration.hxx>
#include <idlc/astscope.hxx>

class AstService : public AstDeclaration
                 , public AstScope
{
public:
    AstService(const OString& name, AstScope* pScope)
        : AstDeclaration(NT_service, name, pScope)
        , AstScope(NT_service)
        , m_singleInterfaceBasedService(false)
        , m_defaultConstructor(false)
        {}
    AstService(const NodeType type, const OString& name, AstScope* pScope)
        : AstDeclaration(type, name, pScope)
        , AstScope(type)
        , m_singleInterfaceBasedService(false)
        , m_defaultConstructor(false)
        {}
    virtual ~AstService() {}

    virtual bool dump(RegistryKey& rKey) override;

    void setSingleInterfaceBasedService()
    { m_singleInterfaceBasedService = true; }

    void setDefaultConstructor(bool b) { m_defaultConstructor = b; }

    bool isSingleInterfaceBasedService() const
    { return m_singleInterfaceBasedService; }

    bool checkLastConstructor() const;

private:
    bool m_singleInterfaceBasedService;
    bool m_defaultConstructor;
};

#endif // INCLUDED_IDLC_INC_IDLC_ASTSERVICE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

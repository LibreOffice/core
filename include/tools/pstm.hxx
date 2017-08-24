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
#ifndef INCLUDED_TOOLS_PSTM_HXX
#define INCLUDED_TOOLS_PSTM_HXX

#include <tools/toolsdllapi.h>
#include <tools/ref.hxx>
#include <tools/stream.hxx>
#include <map>
#include <unordered_map>

#define SV_DECL_PERSIST1( Class, CLASS_ID )                 \
    static  sal_Int32  StaticClassId() { return CLASS_ID; }         \
    virtual sal_Int32  GetClassId() const override;              \

#define SV_IMPL_PERSIST1( Class )                           \
    sal_Int32       Class::GetClassId() const                       \
                    { return StaticClassId(); }                     \

class SvPersistBase : public SvRefBase
{
public:
    virtual sal_Int32  GetClassId() const = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

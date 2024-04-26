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

#include <sal/config.h>

#include <cassert>

#include "abi.hxx"

ReturnKind getReturnKind(typelib_TypeDescription const* type)
{
    switch (type->eTypeClass)
    {
        default:
            assert(false);
            [[fallthrough]];
        case typelib_TypeClass_VOID:
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
        case typelib_TypeClass_FLOAT:
        case typelib_TypeClass_DOUBLE:
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_ENUM:
            assert(type->nSize <= 16);
            return RETURN_KIND_REG;
        case typelib_TypeClass_STRING:
        case typelib_TypeClass_TYPE:
        case typelib_TypeClass_ANY:
        case typelib_TypeClass_SEQUENCE:
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_INTERFACE:
            return RETURN_KIND_INDIRECT;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

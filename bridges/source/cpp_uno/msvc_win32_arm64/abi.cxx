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
#include <sal/types.h>

#include <cassert>

#include "abi.hxx"

enum StructKind
{
    STRUCT_KIND_EMPTY,
    STRUCT_KIND_FLOAT,
    STRUCT_KIND_DOUBLE,
    STRUCT_KIND_POD,
    STRUCT_KIND_DTOR
};

static StructKind getStructKind(typelib_CompoundTypeDescription const* type)
{
    StructKind k = type->pBaseTypeDescription == 0 ? STRUCT_KIND_EMPTY
                                                   : getStructKind(type->pBaseTypeDescription);

    for (sal_Int32 i = 0; i != type->nMembers; ++i)
    {
        StructKind k2 = StructKind();
        switch (type->ppTypeRefs[i]->eTypeClass)
        {
            case typelib_TypeClass_BOOLEAN:
            case typelib_TypeClass_BYTE:
            case typelib_TypeClass_SHORT:
            case typelib_TypeClass_UNSIGNED_SHORT:
            case typelib_TypeClass_LONG:
            case typelib_TypeClass_UNSIGNED_LONG:
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
            case typelib_TypeClass_CHAR:
            case typelib_TypeClass_ENUM:
                k2 = STRUCT_KIND_POD;
                break;
            case typelib_TypeClass_FLOAT:
                k2 = STRUCT_KIND_FLOAT;
                break;
            case typelib_TypeClass_DOUBLE:
                k2 = STRUCT_KIND_DOUBLE;
                break;
            case typelib_TypeClass_STRING:
            case typelib_TypeClass_TYPE:
            case typelib_TypeClass_ANY:
            case typelib_TypeClass_SEQUENCE:
            case typelib_TypeClass_INTERFACE:
                k2 = STRUCT_KIND_DTOR;
                break;
            case typelib_TypeClass_STRUCT:
            {
                typelib_TypeDescription* td = 0;
                TYPELIB_DANGER_GET(&td, type->ppTypeRefs[i]);
                k2 = getStructKind(reinterpret_cast<typelib_CompoundTypeDescription const*>(td));
                TYPELIB_DANGER_RELEASE(td);
                break;
            }
            default:
                assert(false);
        }
        switch (k2)
        {
            case STRUCT_KIND_EMPTY:
                // this means an empty sub-object, which nevertheless obtains a byte
                // of storage (TODO: does it?), so the full object cannot be a
                // homogeneous collection of float or double
            case STRUCT_KIND_POD:
                assert(k != STRUCT_KIND_DTOR);
                k = STRUCT_KIND_POD;
                break;
            case STRUCT_KIND_FLOAT:
            case STRUCT_KIND_DOUBLE:
                if (k == STRUCT_KIND_EMPTY)
                {
                    k = k2;
                }
                else if (k != k2)
                {
                    assert(k != STRUCT_KIND_DTOR);
                    k = STRUCT_KIND_POD;
                }
                break;
            case STRUCT_KIND_DTOR:
                return STRUCT_KIND_DTOR;
        }
    }
    return k;
}

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
        case typelib_TypeClass_INTERFACE:
            return RETURN_KIND_INDIRECT;
        case typelib_TypeClass_STRUCT:
            if (type->nSize > 16)
            {
                return RETURN_KIND_INDIRECT;
            }
            switch (getStructKind(reinterpret_cast<typelib_CompoundTypeDescription const*>(type)))
            {
                case STRUCT_KIND_FLOAT:
                    return RETURN_KIND_INDIRECT;
                case STRUCT_KIND_DOUBLE:
                    return RETURN_KIND_INDIRECT;
                case STRUCT_KIND_DTOR:
                    return RETURN_KIND_INDIRECT;
                default:
                    return RETURN_KIND_INDIRECT;
            }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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


#include "bridges/cpp_uno/shared/types.hxx"

#include "typelib/typeclass.h"
#include "typelib/typedescription.h"

namespace bridges { namespace cpp_uno { namespace shared {

bool isSimpleType(typelib_TypeClass typeClass) {
    return typeClass <= typelib_TypeClass_DOUBLE
        || typeClass == typelib_TypeClass_ENUM;
}

bool isSimpleType(typelib_TypeDescriptionReference const * type) {
    return isSimpleType(type->eTypeClass);
}

bool isSimpleType(typelib_TypeDescription const * type) {
    return isSimpleType(type->eTypeClass);
}

bool relatesToInterfaceType(typelib_TypeDescription const * type) {
    switch (type->eTypeClass) {
    case typelib_TypeClass_ANY:
    case typelib_TypeClass_INTERFACE:
        return true;

    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
        {
            typelib_CompoundTypeDescription const * p
                = reinterpret_cast< typelib_CompoundTypeDescription const * >(
                    type);
            for (sal_Int32 i = 0; i < p->nMembers; ++i) {
                switch (p->ppTypeRefs[i]->eTypeClass) {
                case typelib_TypeClass_ANY:
                case typelib_TypeClass_INTERFACE:
                    return true;

                case typelib_TypeClass_STRUCT:
                case typelib_TypeClass_EXCEPTION:
                case typelib_TypeClass_SEQUENCE:
                    {
                        typelib_TypeDescription * t = nullptr;
                        TYPELIB_DANGER_GET(&t, p->ppTypeRefs[i]);
                        bool b = relatesToInterfaceType(t);
                        TYPELIB_DANGER_RELEASE(t);
                        if (b) {
                            return true;
                        }
                    }
                    break;

                default:
                    break;
                }
            }
            if (p->pBaseTypeDescription != nullptr) {
                return relatesToInterfaceType(&p->pBaseTypeDescription->aBase);
            }
        }
        break;

    case typelib_TypeClass_SEQUENCE:
        switch (reinterpret_cast< typelib_IndirectTypeDescription const * >(
                    type)->pType->eTypeClass) {
        case typelib_TypeClass_ANY:
        case typelib_TypeClass_INTERFACE:
            return true;

        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION:
        case typelib_TypeClass_SEQUENCE:
            {
                typelib_TypeDescription * t = nullptr;
                TYPELIB_DANGER_GET(
                    &t,
                    reinterpret_cast< typelib_IndirectTypeDescription const * >(
                        type)->pType);
                bool b = relatesToInterfaceType(t);
                TYPELIB_DANGER_RELEASE(t);
                return b;
            }

        default:
            break;
        }
        break;

    default:
        break;
    }
    return false;
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

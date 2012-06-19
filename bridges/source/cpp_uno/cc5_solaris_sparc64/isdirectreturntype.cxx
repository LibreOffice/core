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

#include "sal/config.h"

#include <cstddef>

#include "sal/types.h"
#include "typelib/typeclass.h"
#include "typelib/typedescription.h"

#include "isdirectreturntype.hxx"

namespace {

bool isPodStruct(typelib_CompoundTypeDescription * type) {
    for (; type != NULL; type = type->pBaseTypeDescription) {
        for (sal_Int32 i = 0; i < type->nMembers; ++i) {
            if (!bridges::cpp_uno::cc5_solaris_sparc64::isDirectReturnType(
                    type->ppTypeRefs[i]))
            {
                return false;
            }
        }
    }
    return true;
}

}

namespace bridges { namespace cpp_uno { namespace cc5_solaris_sparc64 {

bool isDirectReturnType(typelib_TypeDescriptionReference * type) {
    // Is POD of size <= 32 bytes:
    switch (type->eTypeClass) {
    default:
        return true;
    case typelib_TypeClass_STRING:
    case typelib_TypeClass_TYPE:
    case typelib_TypeClass_ANY:
    case typelib_TypeClass_SEQUENCE:
    case typelib_TypeClass_INTERFACE:
        return false;
    case typelib_TypeClass_STRUCT:
        {
            typelib_TypeDescription * t = NULL;
            TYPELIB_DANGER_GET(&t, type);
            bool b = t->nSize <= 32 && isPodStruct(
                reinterpret_cast< typelib_CompoundTypeDescription * >(t));
            TYPELIB_DANGER_RELEASE(t);
            return b;
        }
    }
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

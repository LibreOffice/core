/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "precompiled_bridges.hxx"
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

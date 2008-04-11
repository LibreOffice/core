/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: types.cxx,v $
 * $Revision: 1.7 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_bridges.hxx"

#include "bridges/cpp_uno/shared/types.hxx"
#define INCLUDED_BRIDGES_CPP_UNO_SHARED_VTABLES_HXX

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
                        typelib_TypeDescription * t = 0;
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
            if (p->pBaseTypeDescription != 0) {
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
                typelib_TypeDescription * t = 0;
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

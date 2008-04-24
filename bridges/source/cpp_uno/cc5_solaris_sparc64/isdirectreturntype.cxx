/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: isdirectreturntype.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2008-04-24 16:26:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2008 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

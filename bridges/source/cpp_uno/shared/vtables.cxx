/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vtables.cxx,v $
 * $Revision: 1.5 $
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

#include "bridges/cpp_uno/shared/vtables.hxx"

#include "osl/diagnose.h"
#include "sal/types.h"
#include "typelib/typedescription.h"

#include <algorithm>

namespace
{

/**
 *  Calculates the number of vtables associated with an interface type.
 *
 * <p>Multiple-inheritance C++ classes have more than one vtable.</p>
 *
 * @param type a non-null pointer to an interface type description
 * @return the number of vtables associated with the given interface type
 */
sal_Int32 getVtableCount(typelib_InterfaceTypeDescription const * type) {
    sal_Int32 n = 0;
    for (sal_Int32 i = 0; i < type->nBaseTypes; ++i) {
        n += getVtableCount(type->ppBaseTypes[i]);
    }
    return std::max< sal_Int32 >(n, 1);
}

/**
 * Maps a local member index to a local function index.
 *
 * <p><em>Local</em> members/functions are those not inherited from any base
 * types.  The number of <em>functions</em> is potentially larger than the
 * number of <em>members</em>, as each read&ndash;write attribute member counts
 * as two functions.</p>
 *
 * @param type a non-null pointer to an interface type description
 * @param localMember a local member index, relative to the given interface type
 * @return the local function index corresponding to the given local member
 *     index, relative to the given interface type
 */
sal_Int32 mapLocalMemberToLocalFunction(
    typelib_InterfaceTypeDescription * type, sal_Int32 localMember)
{
    typelib_typedescription_complete(
        reinterpret_cast< typelib_TypeDescription ** >(&type));
    sal_Int32 localMemberOffset = type->nAllMembers - type->nMembers;
    sal_Int32 localFunctionOffset = type->nMapFunctionIndexToMemberIndex
        - bridges::cpp_uno::shared::getLocalFunctions(type);
    return type->pMapMemberIndexToFunctionIndex[localMemberOffset + localMember]
        - localFunctionOffset;
}

// Since on Solaris we compile with --instances=static, getVtableSlot cannot be
// a template function, with explicit instantiates for
// T = typelib_InterfaceAttributeTypeDescription and
// T = typelib_InterfaceMethodTypeDescription in this file; hence, there are two
// overloaded versions of getVtableSlot that both delegate to this template
// function:
template< typename T > bridges::cpp_uno::shared::VtableSlot doGetVtableSlot(
    T const * ifcMember)
{
    bridges::cpp_uno::shared::VtableSlot slot;
    slot.offset = 0;
    T * member = const_cast< T * >(ifcMember);
    while (member->pBaseRef != 0) {
        OSL_ASSERT(member->nIndex < member->pInterface->nBaseTypes);
        for (sal_Int32 i = 0; i < member->nIndex; ++i) {
            slot.offset += getVtableCount(member->pInterface->ppBaseTypes[i]);
        }
        typelib_TypeDescription * desc = 0;
        typelib_typedescriptionreference_getDescription(
            &desc, member->pBaseRef);
        OSL_ASSERT(
            desc != 0 && desc->eTypeClass == member->aBase.aBase.eTypeClass);
        if (member != ifcMember) {
            typelib_typedescription_release(&member->aBase.aBase);
        }
        member = reinterpret_cast< T * >(desc);
    }
    slot.index
        = bridges::cpp_uno::shared::getPrimaryFunctions(
            member->pInterface->pBaseTypeDescription)
        + mapLocalMemberToLocalFunction(member->pInterface, member->nIndex);
    if (member != ifcMember) {
        typelib_typedescription_release(&member->aBase.aBase);
    }
    return slot;
}

}

namespace bridges { namespace cpp_uno { namespace shared {

sal_Int32 getLocalFunctions(typelib_InterfaceTypeDescription const * type) {
    return type->nMembers == 0
        ? 0
        : (type->nMapFunctionIndexToMemberIndex
           - type->pMapMemberIndexToFunctionIndex[
               type->nAllMembers - type->nMembers]);
}

sal_Int32 getPrimaryFunctions(typelib_InterfaceTypeDescription * type) {
    sal_Int32 n = 0;
    for (; type != 0; type = type->pBaseTypeDescription) {
        typelib_typedescription_complete(
            reinterpret_cast< typelib_TypeDescription ** >(&type));
        n += getLocalFunctions(type);
    }
    return n;
}

VtableSlot getVtableSlot(
    typelib_InterfaceAttributeTypeDescription const * ifcMember)
{
    return doGetVtableSlot(ifcMember);
}

VtableSlot getVtableSlot(
    typelib_InterfaceMethodTypeDescription const * ifcMember)
{
    return doGetVtableSlot(ifcMember);
}

} } }

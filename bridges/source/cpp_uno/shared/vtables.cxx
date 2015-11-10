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


#include "bridges/cpp_uno/shared/vtables.hxx"

#include "sal/types.h"
#include "typelib/typedescription.h"

#include <algorithm>
#include <cassert>

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
        assert(member->nIndex < member->pInterface->nBaseTypes);
        for (sal_Int32 i = 0; i < member->nIndex; ++i) {
            slot.offset += getVtableCount(member->pInterface->ppBaseTypes[i]);
        }
        typelib_TypeDescription * desc = nullptr;
        typelib_typedescriptionreference_getDescription(
            &desc, member->pBaseRef);
        assert(
            desc != nullptr && desc->eTypeClass == member->aBase.aBase.eTypeClass);
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
    for (; type != nullptr; type = type->pBaseTypeDescription) {
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

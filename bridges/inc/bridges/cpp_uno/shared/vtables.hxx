/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vtables.hxx,v $
 * $Revision: 1.4 $
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

#ifndef INCLUDED_BRIDGES_CPP_UNO_SHARED_VTABLES_HXX
#define INCLUDED_BRIDGES_CPP_UNO_SHARED_VTABLES_HXX

#include "sal/types.h"
#include "typelib/typedescription.h"

namespace bridges { namespace cpp_uno { namespace shared {

/**
 * Calculate the number of local functions of an interface type.
 *
 * <p><em>Local</em> functions are those not inherited from any base types.  The
 * number of <em>functions</em> is potentially larger than the number of
 * <em>members</em>, as each read&ndash;write attribute member counts as two
 * functions.</p>
 *
 * @param type a non-null pointer to an interface type description, for which
 *     <code>typelib_typedescription_complete</code> must already have been
 *     executed
 * @return the number of local functions of the given interface type
 */
sal_Int32 getLocalFunctions(typelib_InterfaceTypeDescription const * type);

/**
 * Calculate the number of primary functions of an interface type.
 *
 * <p>The number of primary functions of an interface is the number of local
 * functions of that interface (see <code>getLocalFunctions</code>), plus the
 * number of primary functions of that interface's first base type (if it has at
 * least one base type).</p>
 *
 * @param type a pointer to an interface type description; may be null
 * @return the number of primary functions of the given interface type, or zero
 *     if the given interface type is null
 */
sal_Int32 getPrimaryFunctions(typelib_InterfaceTypeDescription * type);

/**
 * Represents a vtable slot of a C++ class.
 */
struct VtableSlot {
    /**
     * The offset of the vtable.
     *
     * <p>Multiple-inheritance C++ classes have more than one vtable.  The
     * offset is logical (<em>not</em> a byte offset), and must be
     * non-negative.</p>
     */
    sal_Int32 offset;

    /**
     * The index within the vtable.
     *
     * <p>The index is logical (<em>not</em> a byte offset), and must be
     * non-negative.</p>
     */
    sal_Int32 index;
};

/**
 * Calculates the vtable slot associated with an interface attribute member.
 *
 * @param ifcMember a non-null pointer to an interface attribute member
 *     description
 * @return the vtable slot associated with the given interface member
 */
VtableSlot getVtableSlot(
    typelib_InterfaceAttributeTypeDescription const * ifcMember);

/**
 * Calculates the vtable slot associated with an interface method member.
 *
 * @param ifcMember a non-null pointer to an interface method member description
 * @return the vtable slot associated with the given interface member
 */
VtableSlot getVtableSlot(
    typelib_InterfaceMethodTypeDescription const * ifcMember);

} } }

#endif

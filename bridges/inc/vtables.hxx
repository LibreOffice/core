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

#pragma once

#include <sal/types.h>
#include <typelib/typedescription.h>

namespace bridges::cpp_uno::shared {

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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

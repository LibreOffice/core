/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vtables.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 22:10:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
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

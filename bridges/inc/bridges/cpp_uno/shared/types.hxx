/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: types.hxx,v $
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

#ifndef INCLUDED_BRIDGES_CPP_UNO_SHARED_TYPES_HXX
#define INCLUDED_BRIDGES_CPP_UNO_SHARED_TYPES_HXX

#include "typelib/typeclass.h"
#include "typelib/typedescription.h"

namespace bridges { namespace cpp_uno { namespace shared {

/**
 * Determines whether a type is a "simple" type (VOID, BOOLEAN, BYTE, SHORT,
 * UNSIGNED SHORT, LONG, UNSIGNED LONG, HYPER, UNSIGNED HYPER, FLOAT, DOUBLE,
 * CHAR, or an enum type).
 *
 * @param typeClass a type class
 * @return true if the given type is "simple"
 */
bool isSimpleType(typelib_TypeClass typeClass);

/**
 * Determines whether a type is a "simple" type (VOID, BOOLEAN, BYTE, SHORT,
 * UNSIGNED SHORT, LONG, UNSIGNED LONG, HYPER, UNSIGNED HYPER, FLOAT, DOUBLE,
 * CHAR, or an enum type).
 *
 * @param type a non-null pointer to a type description reference
 * @return true if the given type is "simple"
 */
bool isSimpleType(typelib_TypeDescriptionReference const * type);

/**
 * Determines whether a type is a "simple" type (VOID, BOOLEAN, BYTE, SHORT,
 * UNSIGNED SHORT, LONG, UNSIGNED LONG, HYPER, UNSIGNED HYPER, FLOAT, DOUBLE,
 * CHAR, or an enum type).
 *
 * @param type a non-null pointer to a type description
 * @return true if the given type is "simple"
 */
bool isSimpleType(typelib_TypeDescription const * type);

/**
 * Determines whether a type relates to an interface type (is itself an
 * interface type, or might contain entities of interface type).
 *
 * @param type a non-null pointer to a type description
 * @return true if the given type relates to an interface type
 */
bool relatesToInterfaceType(typelib_TypeDescription const * type);

} } }

#endif

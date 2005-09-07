/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: types.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 22:09:50 $
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

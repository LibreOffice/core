/*************************************************************************
 *
 *  $RCSfile: types.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2005-02-21 12:08:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

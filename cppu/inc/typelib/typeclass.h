/*************************************************************************
 *
 *  $RCSfile: typeclass.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:55:42 $
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
#ifndef _TYPELIB_TYPECLASS_H_
#define _TYPELIB_TYPECLASS_H_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

/** This type class enum is binary compatible with the IDL enum com.sun.star.uno.TypeClass.
*/
typedef enum _typelib_TypeClass
{
    /** type class of void */
    typelib_TypeClass_VOID = 0,
    /** type class of char */
    typelib_TypeClass_CHAR = 1,
    /** type class of boolean */
    typelib_TypeClass_BOOLEAN = 2,
    /** type class of byte */
    typelib_TypeClass_BYTE = 3,
    /** type class of short */
    typelib_TypeClass_SHORT = 4,
    /** type class of unsigned short */
    typelib_TypeClass_UNSIGNED_SHORT = 5,
    /** type class of long */
    typelib_TypeClass_LONG = 6,
    /** type class of unsigned long */
    typelib_TypeClass_UNSIGNED_LONG = 7,
    /** type class of hyper */
    typelib_TypeClass_HYPER = 8,
    /** type class of unsigned hyper */
    typelib_TypeClass_UNSIGNED_HYPER = 9,
    /** type class of float */
    typelib_TypeClass_FLOAT = 10,
    /** type class of double */
    typelib_TypeClass_DOUBLE = 11,
    /** type class of string */
    typelib_TypeClass_STRING = 12,
    /** type class of type */
    typelib_TypeClass_TYPE = 13,
    /** type class of any */
    typelib_TypeClass_ANY = 14,
    /** type class of enum */
    typelib_TypeClass_ENUM = 15,
    /** type class of typedef */
    typelib_TypeClass_TYPEDEF = 16,
    /** type class of struct */
    typelib_TypeClass_STRUCT = 17,
    /** type class of union (not implemented) */
    typelib_TypeClass_UNION = 18,
    /** type class of exception */
    typelib_TypeClass_EXCEPTION = 19,
    /** type class of sequence */
    typelib_TypeClass_SEQUENCE = 20,
    /** type class of array (not implemented) */
    typelib_TypeClass_ARRAY = 21,
    /** type class of interface */
    typelib_TypeClass_INTERFACE = 22,
    /** type class of service (not implemented) */
    typelib_TypeClass_SERVICE = 23,
    /** type class of module (not implemented) */
    typelib_TypeClass_MODULE = 24,
    /** type class of interface method */
    typelib_TypeClass_INTERFACE_METHOD = 25,
    /** type class of interface attribute */
    typelib_TypeClass_INTERFACE_ATTRIBUTE = 26,
    /** type class of unknown type */
    typelib_TypeClass_UNKNOWN = 27,
    /** type class of properties */
    typelib_TypeClass_PROPERTY = 28,
    /** type class of constants */
    typelib_TypeClass_CONSTANT = 29,
    /** type class of constants groups */
    typelib_TypeClass_CONSTANTS = 30,
    /** type class of singletons */
    typelib_TypeClass_SINGLETON = 31,
    /** fixing enum size */
    typelib_TypeClass_MAKE_FIXED_SIZE = SAL_MAX_ENUM
} typelib_TypeClass;

#endif

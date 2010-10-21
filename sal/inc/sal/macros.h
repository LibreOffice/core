/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _SAL_MACROS_H_
#define _SAL_MACROS_H_

#ifndef SAL_MAX
#    define SAL_MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef SAL_MIN
#    define SAL_MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef SAL_FIELDOFFSET
#    define SAL_FIELDOFFSET(type, field) ((sal_Int32)(&((type *)16)->field) - 16)
#endif

#ifndef SAL_N_ELEMENTS
#    define SAL_N_ELEMENTS(arr)     (sizeof (arr) / sizeof ((arr)[0]))
#endif

#ifndef SAL_BOUND
#    define SAL_BOUND(x,l,h)        ((x) <= (l) ? (l) : ((x) >= (h) ? (h) : (x)))
#endif

#ifndef SAL_ABS
#    define SAL_ABS(a)              (((a) < 0) ? (-(a)) : (a))
#endif

#endif

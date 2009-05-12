/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dbgmacros.hxx,v $
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

#ifndef DBGMACROS_HXX_INCLUDED
#define DBGMACROS_HXX_INCLUDED

void DbgAssert(bool condition, const char* message);

#if OSL_DEBUG_LEVEL > 0

    #define PRE_CONDITION(x, msg) DbgAssert(x, msg)

    #define POST_CONDITION(x, msg) DbgAssert(x, msg)

    #define ENSURE(x ,msg) DbgAssert(x, msg)

#else // OSL_DEBUG_LEVEL == 0

    #define PRE_CONDITION(x, msg)  ((void)0)

    #define POST_CONDITION(x, msg) ((void)0)

    #define ENSURE(x, msg) ((void)0)

#endif

#endif

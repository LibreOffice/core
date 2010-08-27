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


#ifndef _VOS_DIAGNOSE_H_
#define _VOS_DIAGNOSE_H_


#ifndef _OSL_DIAGNOSE_H_
    #include <osl/diagnose.h>
#endif


/*
    Diagnostic support
*/

#define VOS_THIS_FILE       __FILE__

#define VOS_DEBUG_ONLY(s)   _OSL_DEBUG_ONLY(s)
#define VOS_TRACE           _OSL_TRACE
#define VOS_ASSERT(c)       _OSL_ASSERT(c, VOS_THIS_FILE, __LINE__)
#define VOS_VERIFY(c)       OSL_VERIFY(c)
#define VOS_ENSHURE(c, m)   _OSL_ENSURE(c, VOS_THIS_FILE, __LINE__, m)
#define VOS_ENSURE(c, m)    _OSL_ENSURE(c, VOS_THIS_FILE, __LINE__, m)

#define VOS_PRECOND(c, m)   VOS_ENSHURE(c, m)
#define VOS_POSTCOND(c, m)  VOS_ENSHURE(c, m)

#endif /* _VOS_DIAGNOSE_H_ */



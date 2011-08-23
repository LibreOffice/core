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

#ifndef _SDSWERROR_H
#define _SDSWERROR_H

#ifndef _ERRCODE_HXX
#include <tools/errcode.hxx>
#endif

#define ERROR_SD_BASE			(ERRCODE_AREA_SD)
#define ERROR_SD_READ_BASE		(ERROR_SD_BASE | ERRCODE_CLASS_READ)
#define ERROR_SD_WRITE_BASE		(ERROR_SD_BASE | ERRCODE_CLASS_WRITE)

#define WARN_SD_BASE			(ERRCODE_AREA_SD | ERRCODE_WARNING_MASK)
#define WARN_SD_READ_BASE		(WARN_SD_BASE | ERRCODE_CLASS_READ )
#define WARN_SD_WRITE_BASE		(WARN_SD_BASE | ERRCODE_CLASS_WRITE )


// Import errors
#define ERR_FORMAT_ROWCOL               (ERROR_SD_READ_BASE | 1)
#define ERR_FORMAT_FILE_ROWCOL          (ERROR_SD_READ_BASE | 2)

// ----- Warnings ---------------------------

#define WARN_FORMAT_FILE_ROWCOL         (WARN_SD_READ_BASE | 100)

#ifndef __RSC

namespace binfilter {
inline FASTBOOL IsWarning( ULONG nErr )
{
    return 0 != ( nErr & ERRCODE_WARNING_MASK & nErr );
}

inline FASTBOOL IsError( ULONG nErr )
{
    return nErr && 0 == ( ERRCODE_WARNING_MASK & nErr );
}
} //namespace binfilter
#endif


#endif

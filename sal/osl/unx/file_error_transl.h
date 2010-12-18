/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _FILE_ERROR_TRANSL_H_
#define _FILE_ERROR_TRANSL_H_

#include <osl/file.h>
#include <sal/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************
   oslTranslateFileError
   Translate errno's to osl file errors

   @param bIsError [in] specifies if nErrno
           should be interpreted as error,
          some libc functions signaling an error
           but errno is nevertheless 0 in this
           case the function should at least
           return osl_File_E_Unknown but in no
           case osl_File_E_None!

   @param nErrno [in] the errno if errno is 0
            and bIsError is true the function
           returns osl_File_E_Unknown

   @returns the osl error code appropriate to
              the errno

 *********************************************/

#define OSL_FET_SUCCESS sal_False
#define OSL_FET_ERROR   sal_True

oslFileError oslTranslateFileError(sal_Bool bIsError, int Errno);


#ifdef __cplusplus
}
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

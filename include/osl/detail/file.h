/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Tor Lillqvist <tml@iki.fi> (initial developer)
 * Copyright (C) 2012 SUSE Linux http://suse.com (initial developer's employer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef INCLUDED_OSL_DETAIL_FILE_H
#define INCLUDED_OSL_DETAIL_FILE_H

#include "sal/config.h"

#include <sys/stat.h>

#include "sal/saldllapi.h"
#include "sal/types.h"

/** @cond INTERNAL */

/* Some additions to the osl file functions for LibreOffice internal
   use. Needed for details in the Android support.
 */

#if defined __cplusplus
extern "C" {
#endif

/* More flags needed for semantics that match the open() call that
   used to be in SvFileStream::Open().
*/
#define osl_File_OpenFlag_Trunc     0x00000010L
#define osl_File_OpenFlag_NoExcl    0x00000020L

SAL_DLLPUBLIC oslFileError SAL_CALL osl_openFilePath(
    const char *cpFilePath,
    oslFileHandle* pHandle,
    sal_uInt32 uFlags );

/*  Get the OS specific "handle" of an open file. */
SAL_DLLPUBLIC oslFileError SAL_CALL osl_getFileOSHandle(
    oslFileHandle Handle,
    sal_IntPtr *piFileHandle );

/* for unit testing. */
SAL_DLLPUBLIC oslFileError SAL_CALL osl_openMemoryAsFile( void *address, size_t size, oslFileHandle *pHandle );

#if defined __cplusplus
}
#endif

/** @endcond */

#endif /* INCLUDED_OSL_DETAIL_FILE_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

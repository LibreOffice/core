/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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

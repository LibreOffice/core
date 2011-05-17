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

#include "macros.h"

// GetDiskSpaceExA wrapper for Win 95A

IMPLEMENT_THUNK( kernel32, TRYLOAD, BOOL, WINAPI, GetDiskFreeSpaceExA,(
  LPCSTR lpRootPathName,                  // directory name
  PULARGE_INTEGER lpFreeBytesAvailable,    // bytes available to caller
  PULARGE_INTEGER lpTotalNumberOfBytes,    // bytes on disk
  PULARGE_INTEGER lpTotalNumberOfFreeBytes // free bytes on disk
))
{
    DWORD   dwSectorsPerCluster, dwBytesPerSector, dwNumberOfFreeClusters, dwTotalNumberOfClusters;

    BOOL    fSuccess = GetDiskFreeSpaceA( lpRootPathName, &dwSectorsPerCluster, &dwBytesPerSector, &dwNumberOfFreeClusters, &dwTotalNumberOfClusters );

    if ( fSuccess )
    {
        ULONGLONG   ulBytesPerCluster = (ULONGLONG)dwSectorsPerCluster * (ULONGLONG)dwBytesPerSector;

        if ( lpFreeBytesAvailable )
            lpFreeBytesAvailable->QuadPart = ulBytesPerCluster * (ULONGLONG)dwNumberOfFreeClusters;

        if ( lpTotalNumberOfBytes )
            lpTotalNumberOfBytes->QuadPart = ulBytesPerCluster * (ULONGLONG)dwTotalNumberOfClusters;

        if ( lpTotalNumberOfFreeBytes )
            lpTotalNumberOfFreeBytes->QuadPart = ulBytesPerCluster * (ULONGLONG)dwNumberOfFreeClusters;
    }

    return fSuccess;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

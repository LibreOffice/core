/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


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



#if 0 // YD

#include <windows.h>

static BOOL  g_bInit = FALSE;
static DWORD g_dwPlatformId = VER_PLATFORM_WIN32_WINDOWS;

DWORD GetPlatformId()
{

    if (!g_bInit)
    {
        OSVERSIONINFO aInfo;

        aInfo.dwOSVersionInfoSize = sizeof(aInfo);
        if (GetVersionEx(&aInfo))
            g_dwPlatformId = aInfo.dwPlatformId;

        g_bInit = TRUE;
    }

    return g_dwPlatformId;
}

#endif // 0

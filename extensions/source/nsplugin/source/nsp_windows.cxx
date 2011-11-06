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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#ifdef WNT
    #pragma warning (push,1)
    #pragma warning (disable:4668)
    #include <windows.h>
    #pragma warning (pop)
#endif

int NSP_ResetWinStyl(long hParent)
{
    int ret = 0;
#ifdef WNT
    LONG dOldStyl = GetWindowLong( (HWND)hParent, GWL_STYLE);
    ret = dOldStyl;

    if(ret != 0)
    {
        LONG dNewStyl = dOldStyl|WS_CLIPCHILDREN;
        if(0 == SetWindowLong((HWND)hParent, GWL_STYLE, dNewStyl))
            ret = 0;
    }
#endif
    return ret;

}

int NSP_RestoreWinStyl(long hParent, long dOldStyle)
{
    int ret = 0;
#ifdef WNT
    ret = SetWindowLong((HWND)hParent, GWL_STYLE, dOldStyle);
#endif
    return ret;
}

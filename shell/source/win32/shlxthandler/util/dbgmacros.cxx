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
#include "precompiled_shell.hxx"

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <stdio.h>
#include "internal/dbgmacros.hxx"

void DbgAssert(bool /*condition*/, const char* /*message*/)
{

    //if (!condition)
    //{
    //  char msg[1024];
    //
    //  sprintf(msg, "Assertion in file %s at line %d\n%s", __FILE__, __LINE__, message);

    //  int nRet = MessageBoxA(
    //      0,
    //      msg,
    //      "Assertion violation",
    //      MB_ICONEXCLAMATION | MB_ABORTRETRYIGNORE);
    //}

}


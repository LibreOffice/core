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



#ifndef _PLATFORM_XP_H_
#define _PLATFORM_XP_H_

#pragma once

// Change these values to use different versions
#undef WINVER
#undef _WIN32_WINNT
#undef _WIN32_IE
#undef _WTL_NO_CSTRING

#define WINVER          0x0500
#define _WIN32_WINNT    0x0500
#define _WIN32_IE       0x0600
#define _WTL_NO_CSTRING

#if defined _MSC_VER
    #pragma warning(push, 1)
#endif

#include <windows.h>

#if defined _MSC_VER
    #pragma warning(pop)
#endif

#endif


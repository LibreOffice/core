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

// stdafx1.cpp : source file that includes just the standard includes
//  stdafx1.pch will be the pre-compiled header
//  stdafx1.obj will contain the pre-compiled type information

#ifdef __MINGW32__
#define _INIT_ATL_COMMON_VARS
#endif
#include "stdafx2.h"

#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1300)
#undef _DEBUG
#endif
#include <atlimpl.cpp>

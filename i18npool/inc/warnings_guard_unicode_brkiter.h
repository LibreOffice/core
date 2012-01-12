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



#ifndef INCLUDED_WARNINGS_GUARD_UNICODE_BRKITER_H
#define INCLUDED_WARNINGS_GUARD_UNICODE_BRKITER_H

// Because the GCC system_header mechanism doesn't work in .c/.cxx compilation
// units and more important affects the rest of the current include file, the
// warnings guard is separated into this header file on its own.

// External unicode includes (from icu) cause warning C4668 on Windows.
// We want to minimize the patches to external headers, so the warnings are
// disabled here instead of in the header file itself.
#ifdef _MSC_VER
#pragma warning(push, 1)
#elif defined __GNUC__
#pragma GCC system_header
#endif
#include <unicode/brkiter.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // INCLUDED_WARNINGS_GUARD_UNICODE_BRKITER_H

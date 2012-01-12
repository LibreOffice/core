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



#ifndef INCLUDED_SAL_ALLOCA_H
#define INCLUDED_SAL_ALLOCA_H

#if defined (SOLARIS) || defined (LINUX) || defined(__EMX__)

#ifndef INCLUDED_ALLOCA_H
#include <alloca.h>
#define INCLUDED_ALLOCA_H
#endif

#elif defined (FREEBSD) || defined(NETBSD)

#ifndef INCLUDED_STDLIB_H
#include <stdlib.h>
#define INCLUDED_STDLIB_H
#endif

#elif defined (MACOSX)

#ifndef INCLUDED_SYS_TYPES_H
#include <sys/types.h>
#define INCLUDED_SYS_TYPES_H
#endif

#elif defined (WNT)

#ifndef INCLUDED_MALLOC_H
#include <malloc.h>
#define INCLUDED_MALLOC_H
#endif

#else

#error "unknown platform: please check for alloca"

#endif

#endif  /* INCLUDED_SAL_ALLOCA_H */


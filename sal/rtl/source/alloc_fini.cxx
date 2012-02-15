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



/*
  Issue http://udk.openoffice.org/issues/show_bug.cgi?id=92388

  Mac OS X does not seem to support "__cxa__atexit", thus leading
  to the situation that "__attribute__((destructor))__" functions
  (in particular "rtl_{memory|cache|arena}_fini") become called
  _before_ global C++ object d'tors.

  Using a C++ dummy object instead.
*/

extern "C" void rtl_memory_fini (void);
extern "C" void rtl_cache_fini (void);
extern "C" void rtl_arena_fini (void);

struct RTL_Alloc_Fini
{
  ~RTL_Alloc_Fini() ;
};

RTL_Alloc_Fini::~RTL_Alloc_Fini()
{
  rtl_memory_fini();
  rtl_cache_fini();
  rtl_arena_fini();
}

static RTL_Alloc_Fini g_RTL_Alloc_Fini;

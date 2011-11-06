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



#ifndef _TXTCFG_HXX
#define _TXTCFG_HXX

#if OSL_DEBUG_LEVEL > 1
#include "dbgloop.hxx"  // DBG_LOOP
#else
#ifdef DBG_LOOP     //kann per precompiled hereinkommen
#undef DBG_LOOP
#undef DBG_LOOP_RESET
#endif
#define DBG_LOOP
#define DBG_LOOP_RESET
#endif

// Toleranzwert in der Formatierung und Textausgabe.
#define SLOPPY_TWIPS    5

#define CONSTCHAR( name, string ) static const sal_Char __FAR_DATA name[] = string

// Allgemeines ...

#ifndef CONST
#define CONST const
#endif


#endif

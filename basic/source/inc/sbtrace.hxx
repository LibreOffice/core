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



#ifndef _SBTRACE_HXX
#define _SBTRACE_HXX

//#define DBG_TRACE_BASIC

// ###############################################################################
// ###
// ###    ATTENTION:
// ###
// ###  - DBG_TRACE_PROFILING can only be activated together with DBG_TRACE_BASIC
// ###
// ###  - If you activate DBG_TRACE_PROFILING you also need to uncomment line
// ###    # SHL1STDLIBS+=$(CANVASTOOLSLIB) in basic/util/makefile.mk (search
// ###    for DBG_TRACE_PROFILING there)
// ###
// ###############################################################################
//#define DBG_TRACE_PROFILING

#ifdef DBG_TRACE_BASIC
void dbg_InitTrace( void );
void dbg_DeInitTrace( void );
void dbg_traceStep( SbModule* pModule, sal_uInt32 nPC, sal_Int32 nCallLvl );
void dbg_traceNotifyCall( SbModule* pModule, SbMethod* pMethod, sal_Int32 nCallLvl, bool bLeave = false );
void dbg_traceNotifyError( SbError nTraceErr, const String& aTraceErrMsg, bool bTraceErrHandled, sal_Int32 nCallLvl );
void dbg_RegisterTraceTextForPC( SbModule* pModule, sal_uInt32 nPC,
    const String& aTraceStr_STMNT, const String& aTraceStr_PCode );
void RTL_Impl_TraceCommand( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite );
#endif

#endif

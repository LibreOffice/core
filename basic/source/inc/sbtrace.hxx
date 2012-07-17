/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
void dbg_traceStep( SbModule* pModule, UINT32 nPC, INT32 nCallLvl );
void dbg_traceNotifyCall( SbModule* pModule, SbMethod* pMethod, INT32 nCallLvl, bool bLeave = false );
void dbg_traceNotifyError( SbError nTraceErr, const String& aTraceErrMsg, bool bTraceErrHandled, INT32 nCallLvl );
void dbg_RegisterTraceTextForPC( SbModule* pModule, UINT32 nPC,
    const String& aTraceStr_STMNT, const String& aTraceStr_PCode );
void RTL_Impl_TraceCommand( StarBASIC* pBasic, SbxArray& rPar, BOOL bWrite );
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

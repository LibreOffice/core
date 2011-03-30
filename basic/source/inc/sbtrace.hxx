/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

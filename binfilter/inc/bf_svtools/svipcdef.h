/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SVIPCDEF_H
#define _SVIPCDEF_H

namespace binfilter
{

#ifdef __cplusplus
extern "C" {
#endif

#if defined (WNT) || defined (WIN)
#define CDECL __cdecl
#elif defined CSET
#define CDECL _System
#else
#define CDECL
#endif

#if defined WIN
#define _EXTLIBCALL_ _pascal
#else
#define _EXTLIBCALL_ CDECL
#endif

typedef void (CDECL *IPCCallbackFunc)(void*);

extern void _EXTLIBCALL_ IPCFreeMemory(void*);
extern short _EXTLIBCALL_ IPCGetStatus(void*);
extern short _EXTLIBCALL_ IPCInit(void);
extern void  _EXTLIBCALL_ IPCDeInit(void);
extern void*  _EXTLIBCALL_ IPCConnectServer(const char*, IPCCallbackFunc);
extern void  _EXTLIBCALL_ IPCDisconnectServer(void *);
#ifdef WIN
extern long CDECL IPCCALLFUNCTION(void *,unsigned long,void *,
                                               short,const char *, char, ...);
#else
extern long CDECL IPCCallFunction(void *,unsigned long,void *,
                                               short,const char *, char, ...);
#endif
#ifdef __cplusplus
}
#endif

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

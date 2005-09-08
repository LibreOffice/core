/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svipcdef.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:01:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SVIPCDEF_H
#define _SVIPCDEF_H

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

#endif

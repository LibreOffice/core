/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: crypt.hxx,v $
 * $Revision: 1.4 $
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

// include ---------------------------------------------------------------
#include <tools/gen.hxx>

// define ----------------------------------------------------------------

#define     nInOfs      0x00010000UL
#define     nOutOfs     0x00020000UL

// cryptit ---------------------------------------------------------------

extern "C"
{
//ULONG testdll(const String& ,const String& , const String&, BOOL DoCrypt );
//ULONG __EXPORT testdll(const char* pCrInName, const char* pCrOutName,
//    const char* pCrKey, BOOL DoCrypt=1);
#if defined (UNX)
extern ULONG cryptit(const char* pCrInName, const char* pCrOutName,
#else
#ifdef WNT
extern ULONG __stdcall cryptit(const char* pCrInName, const char* pCrOutName,
#else
#if defined( MTW ) || defined( ICC )
extern ULONG cryptit(const char* pCrInName, const char* pCrOutName,
#else
extern ULONG __pascal cryptit(const char* pCrInName, const char* pCrOutName,
#endif
#endif
#endif
                const char* pCrKey, USHORT DoCrypt);
}



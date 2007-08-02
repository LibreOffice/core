/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: crypt.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-02 18:24:51 $
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



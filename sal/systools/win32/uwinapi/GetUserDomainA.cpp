/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GetUserDomainA.cpp,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:18:33 $
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


#include "macros.h"

EXTERN_C DWORD WINAPI GetUserDomainA_NT( LPSTR lpBuffer, DWORD nSize )
#include "GetUserDomain_NT.cpp"

EXTERN_C DWORD WINAPI GetUserDomainA_WINDOWS( LPSTR lpBuffer, DWORD nSize )
#include "GetUserDomain_WINDOWS.cpp"

EXTERN_C void WINAPI ResolveThunk_GetUserDomainA( FARPROC *lppfn, LPCSTR lpLibFileName, LPCSTR lpFuncName )
{
    if ( (LONG)GetVersion() < 0 )
        *lppfn = (FARPROC)GetUserDomainA_WINDOWS;
    else
        *lppfn = (FARPROC)GetUserDomainA_NT;
}

DEFINE_CUSTOM_THUNK( kernel32, GetUserDomainA, DWORD, WINAPI, GetUserDomainA, ( LPSTR lpBuffer, DWORD nSize ) );


/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ResolveThunk.cpp,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:25:07 $
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


EXTERN_C void WINAPI ResolveThunk_WINDOWS( FARPROC *lppfn, LPCSTR lpLibFileName, LPCSTR lpFuncName, FARPROC lpfnEmulate, FARPROC lpfnFailure )
{
    FARPROC lpfnResult = (LONG)GetVersion() < 0 ? lpfnEmulate : GetProcAddress( LoadLibraryA( lpLibFileName ), lpFuncName );

    if ( !lpfnResult )
        lpfnResult = lpfnEmulate;

    if ( !lpfnResult )
        lpfnResult = lpfnFailure;

    *lppfn = lpfnResult;
}


EXTERN_C void WINAPI ResolveThunk_TRYLOAD( FARPROC *lppfn, LPCSTR lpLibFileName, LPCSTR lpFuncName, FARPROC lpfnEmulate, FARPROC lpfnFailure )
{
    FARPROC lpfnResult = GetProcAddress( LoadLibraryA( lpLibFileName ), lpFuncName );

    if ( !lpfnResult )
        lpfnResult = lpfnEmulate;

    if ( !lpfnResult )
        lpfnResult = lpfnFailure;

    *lppfn = lpfnResult;
}


EXTERN_C void WINAPI ResolveThunk_ALLWAYS( FARPROC *lppfn, LPCSTR lpLibFileName, LPCSTR lpFuncName, FARPROC lpfnEmulate, FARPROC lpfnFailure )
{
    *lppfn = lpfnEmulate ? lpfnEmulate : lpfnFailure;
}



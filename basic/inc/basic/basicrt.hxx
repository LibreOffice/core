/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basicrt.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 12:49:37 $
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
#ifndef _BASICRT_HXX
#define _BASICRT_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SBXDEF_HXX //autogen
#include <basic/sbxdef.hxx>
#endif

class SbiRuntime;
class SbErrorStackEntry;

class BasicRuntime
{
    SbiRuntime* pRun;
public:
    BasicRuntime( SbiRuntime* p ) : pRun ( p ){;}
    const String GetSourceRevision();
    const String GetModuleName( SbxNameType nType );
    const String GetMethodName( SbxNameType nType );
    xub_StrLen GetLine();
    xub_StrLen GetCol1();
    xub_StrLen GetCol2();
    BOOL IsRun();
    BOOL IsValid() { return pRun != NULL; }
    BasicRuntime GetNextRuntime();
};

class BasicErrorStackEntry
{
    SbErrorStackEntry *pEntry;
public:
    BasicErrorStackEntry( SbErrorStackEntry *p ) : pEntry ( p ){;}
    const String GetSourceRevision();
    const String GetModuleName( SbxNameType nType );
    const String GetMethodName( SbxNameType nType );
    xub_StrLen GetLine();
    xub_StrLen GetCol1();
    xub_StrLen GetCol2();
};

class BasicRuntimeAccess
{
public:
    static BasicRuntime GetRuntime();
    static bool HasRuntime();
    static USHORT GetStackEntryCount();
    static BasicErrorStackEntry GetStackEntry( USHORT nIndex );
    static BOOL HasStack();
    static void DeleteStack();

    static BOOL IsRunInit();
};

#endif


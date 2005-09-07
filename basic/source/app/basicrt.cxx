/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basicrt.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:13:21 $
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


#include "sbintern.hxx"
#include "runtime.hxx"
#include "basicrt.hxx"

const String BasicRuntime::GetSourceRevision()
{
    return pRun->GetModule()->GetComment();
}

const String BasicRuntime::GetModuleName( SbxNameType nType )
{
    return pRun->GetModule()->GetName( nType );
}

const String BasicRuntime::GetMethodName( SbxNameType nType )
{
    return pRun->GetMethod()->GetName( nType );
}

xub_StrLen BasicRuntime::GetLine()
{
    return pRun->nLine;
}

xub_StrLen BasicRuntime::GetCol1()
{
    return pRun->nCol1;
}

xub_StrLen BasicRuntime::GetCol2()
{
    return pRun->nCol2;
}

BOOL BasicRuntime::IsRun()
{
    return pRun->IsRun();
}

BasicRuntime BasicRuntime::GetNextRuntime()
{
    return BasicRuntime ( pRun->pNext );
}




const String BasicErrorStackEntry::GetSourceRevision()
{
    return pEntry->aMethod->GetModule()->GetComment();
}

const String BasicErrorStackEntry::GetModuleName( SbxNameType nType )
{
    return pEntry->aMethod->GetModule()->GetName( nType );
}

const String BasicErrorStackEntry::GetMethodName( SbxNameType nType )
{
    return pEntry->aMethod->GetName( nType );
}

xub_StrLen BasicErrorStackEntry::GetLine()
{
    return pEntry->nLine;
}

xub_StrLen BasicErrorStackEntry::GetCol1()
{
    return pEntry->nCol1;
}

xub_StrLen BasicErrorStackEntry::GetCol2()
{
    return pEntry->nCol2;
}




BasicRuntime BasicRuntimeAccess::GetRuntime()
{
    return BasicRuntime( pINST->pRun );
}

BOOL BasicRuntimeAccess::HasRuntime()
{
    return pINST ? ( pINST->pRun != NULL ) : FALSE;
}

USHORT BasicRuntimeAccess::GetStackEntryCount()
{
    return GetSbData()->pErrStack->Count();
}

BasicErrorStackEntry BasicRuntimeAccess::GetStackEntry( USHORT nIndex )
{
    return BasicErrorStackEntry( GetSbData()->pErrStack->GetObject( nIndex ) );
}

BOOL BasicRuntimeAccess::HasStack()
{
    return GetSbData()->pErrStack != NULL;
}

void BasicRuntimeAccess::DeleteStack()
{
    delete GetSbData()->pErrStack;
    GetSbData()->pErrStack = NULL;
}

BOOL BasicRuntimeAccess::IsRunInit()
{
    return GetSbData()->bRunInit;
}

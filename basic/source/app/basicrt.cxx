/*************************************************************************
 *
 *  $RCSfile: basicrt.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:08 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include "sbintern.hxx"
#include "runtime.hxx"
#include "basicrt.hxx"


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

BasicRuntime BasicRuntime::GetNextRuntime()
{
    return BasicRuntime ( pRun->pNext );
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

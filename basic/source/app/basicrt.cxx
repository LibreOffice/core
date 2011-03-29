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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basic.hxx"


#include "sbintern.hxx"
#include "runtime.hxx"
#include <basic/basicrt.hxx>

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

sal_Bool BasicRuntime::IsRun()
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

bool BasicRuntimeAccess::HasRuntime()
{
  return pINST && pINST->pRun != NULL;
}

sal_uInt16 BasicRuntimeAccess::GetStackEntryCount()
{
  return GetSbData()->pErrStack->Count();
}

BasicErrorStackEntry BasicRuntimeAccess::GetStackEntry( sal_uInt16 nIndex )
{
  return BasicErrorStackEntry( GetSbData()->pErrStack->GetObject( nIndex ) );
}

sal_Bool BasicRuntimeAccess::HasStack()
{
  return GetSbData()->pErrStack != NULL;
}

void BasicRuntimeAccess::DeleteStack()
{
  delete GetSbData()->pErrStack;
  GetSbData()->pErrStack = NULL;
}

sal_Bool BasicRuntimeAccess::IsRunInit()
{
  return GetSbData()->bRunInit;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

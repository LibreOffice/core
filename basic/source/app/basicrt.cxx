/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

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


#ifndef _BASICRT_HXX
#define _BASICRT_HXX

#include <tools/string.hxx>
#include <basic/sbxdef.hxx>

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
    sal_Bool IsRun();
    sal_Bool IsValid() { return pRun != NULL; }
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
    static sal_uInt16 GetStackEntryCount();
    static BasicErrorStackEntry GetStackEntry( sal_uInt16 nIndex );
    static sal_Bool HasStack();
    static void DeleteStack();

    static sal_Bool IsRunInit();
};

#endif


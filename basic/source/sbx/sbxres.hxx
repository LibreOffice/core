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



#ifndef _SBXRES_HXX
#define _SBXRES_HXX

#include <tools/string.hxx>

// Zur Zeit sind Ressources im SVTOOLS-Projekt nicht vorgesehen.
// Da es sich um unkritische Ressourcen handelt (BASIC-Keywords),
// koennen wir mit Dummies arbeiten.

#define STRING_TYPES        0
#define STRING_EMPTY        0
#define STRING_NULL         1
#define STRING_INTEGER      2
#define STRING_LONG         3
#define STRING_SINGLE       4
#define STRING_DOUBLE       5
#define STRING_CURRENCY     6
#define STRING_DATE         7
#define STRING_STRING       8
#define STRING_OBJECT       9
#define STRING_ERROR        10
#define STRING_BOOL         11
#define STRING_VARIANT      12
#define STRING_ANY          13
#define STRING_CHAR         16
#define STRING_BYTE         17
#define STRING_USHORT       18
#define STRING_ULONG        19
#define STRING_INT          22
#define STRING_UINT         23
#define STRING_LPSTR        30
#define STRING_LPWSTR       31
#define STRING_AS           32
#define STRING_OPTIONAL     33
#define STRING_BYREF        34

#define STRING_NAMEPROP     35
#define STRING_PARENTPROP   36
#define STRING_APPLPROP     37
#define STRING_COUNTPROP    38
#define STRING_ADDMETH      39
#define STRING_ITEMMETH     40
#define STRING_REMOVEMETH   41

#define STRING_ERRORMSG     42
#define STRING_FALSE        43
#define STRING_TRUE         44

#define SBXRES_MAX          44

class SbxRes : public ::rtl::OUString
{
public:
    SbxRes( sal_uInt16 );
};

const char* GetSbxRes( sal_uInt16 );


#endif

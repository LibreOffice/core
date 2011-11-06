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



#ifndef _SBXBASE_HXX
#define _SBXBASE_HXX

#include <i18npool/lang.h>
#include "tools/list.hxx"
#include "svl/svarray.hxx"
#include <basic/sbxdef.hxx>

class SbxFactory;
class SbxVariable;
class SbxBasicFormater;

SV_DECL_PTRARR_DEL(SbxFacs,SbxFactory*,5,5)
DECLARE_LIST(SbxVarList_Impl, SbxVariable*)

// AppData-Struktur for SBX:
struct SbxAppData
{
    SbxError            eSbxError;  // Error code
    SbxFacs             aFacs;      // Factories
    SbxVarList_Impl     aVars;      // for Dump
    SbxBasicFormater    *pBasicFormater;    // Pointer to Format()-Command helper class

    LanguageType        eBasicFormaterLangType;
        // It might be useful to store this class 'global' because some string reosurces are saved here

    SbxAppData() : eSbxError( SbxERR_OK ), aFacs(), pBasicFormater( NULL ) {}
    ~SbxAppData();
};

SbxAppData* GetSbxData_Impl();

#endif

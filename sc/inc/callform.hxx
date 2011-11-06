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



#ifndef SC_CALLFORM_HXX
#define SC_CALLFORM_HXX

#include "collect.hxx"

//------------------------------------------------------------------------
#define MAXFUNCPARAM    16
#define MAXARRSIZE      0xfffe

//------------------------------------------------------------------------
#ifndef WNT
#define CALLTYPE
#else
#define CALLTYPE            __cdecl
#endif

extern "C" {
typedef void (CALLTYPE* AdvData)( double& nHandle, void* pData );
}

//------------------------------------------------------------------------
enum ParamType
{
    PTR_DOUBLE,
    PTR_STRING,
    PTR_DOUBLE_ARR,
    PTR_STRING_ARR,
    PTR_CELL_ARR,
    NONE
};

//------------------------------------------------------------------------
class ModuleData;
class FuncData : public ScDataObject
{
friend class FuncCollection;
    const ModuleData* pModuleData;
    String      aInternalName;
    String      aFuncName;
    sal_uInt16      nNumber;
    sal_uInt16      nParamCount;
    ParamType   eAsyncType;
    ParamType   eParamType[MAXFUNCPARAM];
private:
    FuncData(const String& rIName);
public:
    FuncData(const ModuleData*pModule,
             const String&    rIName,
             const String&    rFName,
                   sal_uInt16     nNo,
                   sal_uInt16     nCount,
             const ParamType* peType,
                   ParamType  eType);
    FuncData(const FuncData& rData);
    virtual ScDataObject*   Clone() const { return new FuncData(*this); }

    const   String&     GetModuleName() const;
    const   String&     GetInternalName() const { return aInternalName; }
    const   String&     GetFuncName() const { return aFuncName; }
            sal_uInt16      GetParamCount() const { return nParamCount; }
            ParamType   GetParamType(sal_uInt16 nIndex) const { return eParamType[nIndex]; }
            ParamType   GetReturnType() const { return eParamType[0]; }
            ParamType   GetAsyncType() const { return eAsyncType; }
            sal_Bool        Call(void** ppParam);
            sal_Bool        Unadvice(double nHandle);

                        // Name und Beschreibung des Parameters nParam.
                        // nParam==0 => Desc := Funktions-Beschreibung,
                        // Name := n/a
            sal_Bool        GetParamDesc( String& aName, String& aDesc, sal_uInt16 nParam );
};


//------------------------------------------------------------------------
class FuncCollection : public ScSortedCollection
{
public:
    FuncCollection(sal_uInt16 nLim = 4, sal_uInt16 nDel = 4, sal_Bool bDup = sal_False) : ScSortedCollection ( nLim, nDel, bDup ) {}
    FuncCollection(const FuncCollection& rFuncCollection) : ScSortedCollection ( rFuncCollection ) {}

    virtual ScDataObject*   Clone() const { return new FuncCollection(*this); }
            FuncData*   operator[]( const sal_uInt16 nIndex) const {return (FuncData*)At(nIndex);}
    virtual short       Compare(ScDataObject* pKey1, ScDataObject* pKey2) const;
            sal_Bool        SearchFunc( const String& rName, sal_uInt16& rIndex ) const;
};


sal_Bool InitExternalFunc(const rtl::OUString& rModuleName);
void ExitExternalFunc();

#endif

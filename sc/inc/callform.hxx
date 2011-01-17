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

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

#ifndef SC_CALLFORM_HXX
#define SC_CALLFORM_HXX

#include <rtl/ustring.hxx>

#include <boost/ptr_container/ptr_map.hpp>

#define MAXFUNCPARAM    16
#define MAXARRSIZE      0xfffe

#ifndef WNT
#define CALLTYPE
#else
#define CALLTYPE            __cdecl
#endif

extern "C" {
typedef void (CALLTYPE* AdvData)( double& nHandle, void* pData );
}

enum ParamType
{
    PTR_DOUBLE,
    PTR_STRING,
    PTR_DOUBLE_ARR,
    PTR_STRING_ARR,
    PTR_CELL_ARR,
    NONE
};

class ModuleData;

class FuncData
{
    friend class FuncCollection;

    const ModuleData* pModuleData;
    rtl::OUString aInternalName;
    rtl::OUString aFuncName;
    sal_uInt16      nNumber;
    sal_uInt16      nParamCount;
    ParamType   eAsyncType;
    ParamType   eParamType[MAXFUNCPARAM];
public:
    FuncData(const ModuleData*pModule,
             const rtl::OUString& rIName,
             const rtl::OUString& rFName,
                   sal_uInt16     nNo,
                   sal_uInt16     nCount,
             const ParamType* peType,
                   ParamType  eType);
    FuncData(const FuncData& rData);

    const rtl::OUString& GetModuleName() const;
    const rtl::OUString& GetInternalName() const { return aInternalName; }
    const rtl::OUString& GetFuncName() const { return aFuncName; }
            sal_uInt16      GetParamCount() const { return nParamCount; }
            ParamType   GetParamType(sal_uInt16 nIndex) const { return eParamType[nIndex]; }
            ParamType   GetReturnType() const { return eParamType[0]; }
            ParamType   GetAsyncType() const { return eAsyncType; }
    bool        Call(void** ppParam) const;
    bool        Unadvice(double nHandle);

                // name and description of parameter nParam.
                // nParam==0 => Desc := function description,
                // Name := n/a
    bool getParamDesc( ::rtl::OUString& aName, ::rtl::OUString& aDesc, sal_uInt16 nParam ) const;
};


class FuncCollection
{
    typedef boost::ptr_map<rtl::OUString, FuncData> MapType;
    MapType maData;
public:
    typedef MapType::const_iterator const_iterator;

    FuncCollection();
    FuncCollection(const FuncCollection& r);

    const FuncData* findByName(const rtl::OUString& rName) const;
    FuncData* findByName(const rtl::OUString& rName);
    void insert(FuncData* pNew);

    const_iterator begin() const;
    const_iterator end() const;
};


bool InitExternalFunc(const rtl::OUString& rModuleName);
void ExitExternalFunc();

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

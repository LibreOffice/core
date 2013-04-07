/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
    OUString aInternalName;
    OUString aFuncName;
    sal_uInt16      nNumber;
    sal_uInt16      nParamCount;
    ParamType   eAsyncType;
    ParamType   eParamType[MAXFUNCPARAM];
public:
    FuncData(const ModuleData*pModule,
             const OUString& rIName,
             const OUString& rFName,
                   sal_uInt16     nNo,
                   sal_uInt16     nCount,
             const ParamType* peType,
                   ParamType  eType);
    FuncData(const FuncData& rData);

    const OUString& GetModuleName() const;
    const OUString& GetInternalName() const { return aInternalName; }
    const OUString& GetFuncName() const { return aFuncName; }
            sal_uInt16      GetParamCount() const { return nParamCount; }
            ParamType   GetParamType(sal_uInt16 nIndex) const { return eParamType[nIndex]; }
            ParamType   GetReturnType() const { return eParamType[0]; }
            ParamType   GetAsyncType() const { return eAsyncType; }
    bool        Call(void** ppParam) const;
    bool        Unadvice(double nHandle);

                // name and description of parameter nParam.
                // nParam==0 => Desc := function description,
                // Name := n/a
    bool getParamDesc( OUString& aName, OUString& aDesc, sal_uInt16 nParam ) const;
};


class FuncCollection
{
    typedef boost::ptr_map<OUString, FuncData> MapType;
    MapType maData;
public:
    typedef MapType::const_iterator const_iterator;

    FuncCollection();
    FuncCollection(const FuncCollection& r);

    const FuncData* findByName(const OUString& rName) const;
    FuncData* findByName(const OUString& rName);
    void insert(FuncData* pNew);

    const_iterator begin() const;
    const_iterator end() const;
};


bool InitExternalFunc(const OUString& rModuleName);
void ExitExternalFunc();

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

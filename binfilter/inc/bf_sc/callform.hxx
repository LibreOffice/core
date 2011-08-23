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

#include "collect.hxx"
namespace binfilter {

//------------------------------------------------------------------------
#define MAXFUNCPARAM	16
#define MAXARRSIZE		0xfffe

//------------------------------------------------------------------------
#ifndef WIN
#ifndef WNT
#define CALLTYPE
#else
#define CALLTYPE			__cdecl
#endif
#else
#define PASCAL				_pascal
#define FAR					_far
#define CALLTYPE			FAR PASCAL
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
class FuncData : public DataObject
{
friend class FuncCollection;
    const ModuleData* pModuleData;
    String		aInternalName;
    String		aFuncName;
    USHORT      nNumber;
    USHORT		nParamCount;
    ParamType	eAsyncType;
    ParamType	eParamType[MAXFUNCPARAM];
private:
    FuncData(const String& rIName);
public:
    FuncData(const FuncData& rData);
    virtual	DataObject*	Clone() const { return new FuncData(*this); }

    const	String&		GetInternalName() const { return aInternalName; }
    const	String&		GetFuncName() const { return aFuncName; }
            USHORT		GetParamCount() const { return nParamCount; }
            ParamType	GetParamType(USHORT nIndex) const { return eParamType[nIndex]; }
            ParamType	GetReturnType() const { return eParamType[0]; }
            ParamType	GetAsyncType() const { return eAsyncType; }
            BOOL        Call(void** ppParam);

                        // Name und Beschreibung des Parameters nParam.
                        // nParam==0 => Desc := Funktions-Beschreibung,
                        // Name := n/a
};


//------------------------------------------------------------------------
class FuncCollection : public SortedCollection
{
public:
    FuncCollection(USHORT nLim = 4, USHORT nDel = 4, BOOL bDup = FALSE) : SortedCollection ( nLim, nDel, bDup ) {}
    FuncCollection(const FuncCollection& rFuncCollection) : SortedCollection ( rFuncCollection ) {}

    virtual	DataObject*	Clone() const { return new FuncCollection(*this); }
    virtual	short		Compare(DataObject* pKey1, DataObject* pKey2) const;
            BOOL 		SearchFunc( const String& rName, USHORT& rIndex ) const;
};


void ExitExternalFunc();

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

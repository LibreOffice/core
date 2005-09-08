/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: callform.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:24:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_CALLFORM_HXX
#define SC_CALLFORM_HXX

#ifndef SC_COLLECT_HXX
#include "collect.hxx"
#endif

//------------------------------------------------------------------------
#define MAXFUNCPARAM    16
#define MAXARRSIZE      0xfffe

//------------------------------------------------------------------------
#ifndef WIN
#ifndef WNT
#define CALLTYPE
#else
#define CALLTYPE            __cdecl
#endif
#else
#define PASCAL              _pascal
#define FAR                 _far
#define CALLTYPE            FAR PASCAL
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
    String      aInternalName;
    String      aFuncName;
    USHORT      nNumber;
    USHORT      nParamCount;
    ParamType   eAsyncType;
    ParamType   eParamType[MAXFUNCPARAM];
private:
    FuncData(const String& rIName);
public:
    FuncData(const ModuleData*pModule,
             const String&    rIName,
             const String&    rFName,
                   USHORT     nNo,
                   USHORT     nCount,
             const ParamType* peType,
                   ParamType  eType);
    FuncData(const FuncData& rData);
    virtual DataObject* Clone() const { return new FuncData(*this); }

    const   String&     GetModuleName() const;
    const   String&     GetInternalName() const { return aInternalName; }
    const   String&     GetFuncName() const { return aFuncName; }
            USHORT      GetParamCount() const { return nParamCount; }
            ParamType   GetParamType(USHORT nIndex) const { return eParamType[nIndex]; }
            ParamType   GetReturnType() const { return eParamType[0]; }
            ParamType   GetAsyncType() const { return eAsyncType; }
            BOOL        Call(void** ppParam);
            BOOL        Advice(AdvData pfCallback);
            BOOL        Unadvice(double nHandle);

                        // Name und Beschreibung des Parameters nParam.
                        // nParam==0 => Desc := Funktions-Beschreibung,
                        // Name := n/a
            BOOL        GetParamDesc( String& aName, String& aDesc, USHORT nParam );
};


//------------------------------------------------------------------------
class FuncCollection : public SortedCollection
{
public:
    FuncCollection(USHORT nLim = 4, USHORT nDel = 4, BOOL bDup = FALSE) : SortedCollection ( nLim, nDel, bDup ) {}
    FuncCollection(const FuncCollection& rFuncCollection) : SortedCollection ( rFuncCollection ) {}

    virtual DataObject* Clone() const { return new FuncCollection(*this); }
            FuncData*   operator[]( const USHORT nIndex) const {return (FuncData*)At(nIndex);}
    virtual short       Compare(DataObject* pKey1, DataObject* pKey2) const;
            BOOL        SearchFunc( const String& rName, USHORT& rIndex ) const;
};


BOOL InitExternalFunc(const rtl::OUString& rModuleName);
void ExitExternalFunc();

#endif

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

#ifndef SC_DPSDBTAB_HXX
#define SC_DPSDBTAB_HXX

#include <com/sun/star/uno/Reference.hxx>

#include "dptabdat.hxx"

#include <vector>
#include <set>

class ScDPCacheTable;
class ScDocument;

// --------------------------------------------------------------------
//
//  implementation of ScDPTableData with database data
//

struct ScImportSourceDesc
{
    String  aDBName;
    String  aObject;
    sal_uInt16  nType;          // enum DataImportMode
    sal_Bool    bNative;

    ScImportSourceDesc() : nType(0), bNative(sal_False) {}

    sal_Bool operator== ( const ScImportSourceDesc& rOther ) const
        { return aDBName == rOther.aDBName &&
                 aObject == rOther.aObject &&
                 nType   == rOther.nType &&
                 bNative == rOther.bNative; }

        // Wang Xu Ming -- 2009-9-15
        // DataPilot Migration - Cache&&Performance
    ScDPTableDataCache* GetExistDPObjectCache( ScDocument* pDoc ) const;
    ScDPTableDataCache* CreateCache(  ScDocument* pDoc , long nID  ) const;
    ScDPTableDataCache* GetCache( ScDocument* pDoc, long nID ) const;
    long    GetCacheId( ScDocument* pDoc, long nID ) const;
        // End Comments
};

class ScDatabaseDPData : public ScDPTableData
{
private:
     ScDPCacheTable      aCacheTable;
public:
                    ScDatabaseDPData(ScDocument* pDoc, const ScImportSourceDesc& rImport, long nCacheId = -1);
    virtual         ~ScDatabaseDPData();

    virtual long                    GetColumnCount();
    virtual String                  getDimensionName(long nColumn);
    virtual sal_Bool                    getIsDataLayoutDimension(long nColumn);
    virtual sal_Bool                    IsDateDimension(long nDim);
    virtual void                    DisposeData();
    virtual void                    SetEmptyFlags( sal_Bool bIgnoreEmptyRows, sal_Bool bRepeatIfEmpty );

    virtual void                    CreateCacheTable();
    virtual void                    FilterCacheTable(const ::std::vector<ScDPCacheTable::Criterion>& rCriteria, const ::std::hash_set<sal_Int32>& rDataDims);
    virtual void                    GetDrillDownData(const ::std::vector<ScDPCacheTable::Criterion>& rCriteria,
                                                     const ::std::hash_set<sal_Int32>& rCatDims,
                                                     ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& rData);
    virtual void                    CalcResults(CalcInfo& rInfo, bool bAutoShow);
    virtual const ScDPCacheTable&   GetCacheTable() const;
};



#endif


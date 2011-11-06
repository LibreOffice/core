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


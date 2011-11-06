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



#ifndef SC_DPSHTTAB_HXX
#define SC_DPSHTTAB_HXX

#include "dptabdat.hxx"
#include "global.hxx"
#include "address.hxx"
#include "scdllapi.h"
#include "queryparam.hxx"

#include <vector>

namespace com { namespace sun { namespace star { namespace sheet {
    struct DataPilotFieldFilter;
}}}}

class ScDPDimension;
// Wang Xu Ming -- 2009-8-17
// DataPilot Migration - Cache&&Performance
class ScDPItemData;
// End Comments
// --------------------------------------------------------------------
//
//  implementation of ScDPTableData with sheet data
//

struct ScSheetSourceDesc
{
    ScRange         aSourceRange;
    ScQueryParam    aQueryParam;

    sal_Bool operator== ( const ScSheetSourceDesc& rOther ) const
        { return aSourceRange == rOther.aSourceRange &&
                 aQueryParam  == rOther.aQueryParam; }
// Wang Xu Ming - DataPilot migration
// Buffer&&Performance
    ScDPTableDataCache* CreateCache( ScDocument* pDoc, long nID = -1) const;
    sal_uLong CheckValidate( ScDocument* pDoc  ) const;
    ScDPTableDataCache* GetCache( ScDocument* pDoc, long nID ) const;
    ScDPTableDataCache*  GetExistDPObjectCache ( ScDocument* pDoc  ) const;
    long    GetCacheId( ScDocument* pDoc, long nID ) const;

// End Comments
};

class SC_DLLPUBLIC ScSheetDPData : public ScDPTableData
{
private:
    ScQueryParam    aQuery;
    sal_Bool*                pSpecial;
    sal_Bool            bIgnoreEmptyRows;
    sal_Bool            bRepeatIfEmpty;

       ScDPCacheTable  aCacheTable;

public:
    // Wang Xu Ming -- 2009-8-17
    // DataPilot Migration - Cache&&Performance
    ScSheetDPData( ScDocument* pD, const ScSheetSourceDesc& rDesc, long nCacheId = -1 );
    virtual         ~ScSheetDPData();
    // End Comments
    virtual long                    GetColumnCount();
    virtual String                  getDimensionName(long nColumn);
    virtual sal_Bool                    getIsDataLayoutDimension(long nColumn);
    virtual sal_Bool                    IsDateDimension(long nDim);
    virtual sal_uLong                   GetNumberFormat(long nDim);
    virtual void                    DisposeData();
    virtual void                    SetEmptyFlags( sal_Bool bIgnoreEmptyRows, sal_Bool bRepeatIfEmpty );

    virtual bool                    IsRepeatIfEmpty();

    virtual void                    CreateCacheTable();
    virtual void                    FilterCacheTable(const ::std::vector<ScDPCacheTable::Criterion>& rCriteria, const ::std::hash_set<sal_Int32>& rCatDims);
    virtual void                    GetDrillDownData(const ::std::vector<ScDPCacheTable::Criterion>& rCriteria,
                                                     const ::std::hash_set<sal_Int32>& rCatDims,
                                                     ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& rData);
    virtual void                    CalcResults(CalcInfo& rInfo, bool bAutoShow);
    virtual const ScDPCacheTable&   GetCacheTable() const;
};



#endif


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


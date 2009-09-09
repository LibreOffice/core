/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dpshttab.hxx,v $
 * $Revision: 1.8 $
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
struct ScDPItemData;

// --------------------------------------------------------------------
//
//  implementation of ScDPTableData with sheet data
//

struct ScSheetSourceDesc
{
    ScRange         aSourceRange;
    ScQueryParam    aQueryParam;

    BOOL operator== ( const ScSheetSourceDesc& rOther ) const
        { return aSourceRange == rOther.aSourceRange &&
                 aQueryParam  == rOther.aQueryParam; }
};

class ScSheetDPData_Impl;

class SC_DLLPUBLIC ScSheetDPData : public ScDPTableData
{
private:
    ScSheetDPData_Impl* pImpl;

public:
                    ScSheetDPData( ScDocument* pD, const ScSheetSourceDesc& rDesc );
    virtual         ~ScSheetDPData();

    virtual long                    GetColumnCount();
    virtual const TypedScStrCollection& GetColumnEntries(long nColumn);
    virtual String                  getDimensionName(long nColumn);
    virtual BOOL                    getIsDataLayoutDimension(long nColumn);
    virtual BOOL                    IsDateDimension(long nDim);
    virtual UINT32                  GetNumberFormat(long nDim);
    virtual void                    DisposeData();
    virtual void                    SetEmptyFlags( BOOL bIgnoreEmptyRows, BOOL bRepeatIfEmpty );

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


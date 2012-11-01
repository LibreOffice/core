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

#ifndef SC_DPSHTTAB_HXX
#define SC_DPSHTTAB_HXX

#include "dptabdat.hxx"
#include "global.hxx"
#include "address.hxx"
#include "scdllapi.h"
#include "queryparam.hxx"

#include <boost/unordered_set.hpp>
#include <vector>

namespace com { namespace sun { namespace star { namespace sheet {
    struct DataPilotFieldFilter;
}}}}

class ScDPDimensionSaveData;

/**
 * This class contains authoritative information on the internal reference
 * used as the data source for datapilot table.  <i>The range name takes
 * precedence over the source range when it's non-empty.</i>  When the range
 * name is empty, the source range gets used.
 */
class ScSheetSourceDesc
{
    ScSheetSourceDesc(); // disabled

public:
    SC_DLLPUBLIC ScSheetSourceDesc(ScDocument* pDoc);

    SC_DLLPUBLIC void SetSourceRange(const ScRange& rRange);

    /**
     * Get the range that contains the source data.  In case the source data
     * is referred to via a range name, it returns the range that the range
     * name points to.
     *
     * <i>Note that currently only a single range is supported; if the
     * range name contains multiple ranges, only the first range is used.</i>
     *
     * @return source range.
     */
    SC_DLLPUBLIC const ScRange& GetSourceRange() const;
    SC_DLLPUBLIC void SetRangeName(const ::rtl::OUString& rName);
    SC_DLLPUBLIC const ::rtl::OUString& GetRangeName() const;
    bool HasRangeName() const;
    void SetQueryParam(const ScQueryParam& rParam);
    const ScQueryParam& GetQueryParam() const;

    bool operator== ( const ScSheetSourceDesc& rOther ) const;
    SC_DLLPUBLIC const ScDPCache* CreateCache(const ScDPDimensionSaveData* pDimData) const;

    /**
     * Check the sanity of the data source range.
     *
     * @return 0 if the source range is sane, otherwise an error message ID is
     *         returned.
     */
    sal_uLong CheckSourceRange() const;

private:
    mutable ScRange maSourceRange;
    ::rtl::OUString maRangeName;
    ScQueryParam    maQueryParam;
    ScDocument*     mpDoc;
};

/**
 * Implementation of ScDPTableData with sheet data.
 */
class SC_DLLPUBLIC ScSheetDPData : public ScDPTableData
{
private:
    ScQueryParam    aQuery;
    bool            bIgnoreEmptyRows;
    bool            bRepeatIfEmpty;

    ScDPCacheTable  aCacheTable;

public:
    ScSheetDPData(ScDocument* pD, const ScSheetSourceDesc& rDesc, const ScDPCache& rCache);
    virtual ~ScSheetDPData();

    virtual long                    GetColumnCount();
    virtual rtl::OUString           getDimensionName(long nColumn);
    virtual sal_Bool                    getIsDataLayoutDimension(long nColumn);
    virtual sal_Bool                    IsDateDimension(long nDim);
    virtual sal_uLong                   GetNumberFormat(long nDim);
    virtual void                    DisposeData();
    virtual void                    SetEmptyFlags( sal_Bool bIgnoreEmptyRows, sal_Bool bRepeatIfEmpty );

    virtual bool                    IsRepeatIfEmpty();

    virtual void                    CreateCacheTable();
    virtual void                    FilterCacheTable(const ::std::vector<ScDPCacheTable::Criterion>& rCriteria, const ::boost::unordered_set<sal_Int32>& rCatDims);
    virtual void                    GetDrillDownData(const ::std::vector<ScDPCacheTable::Criterion>& rCriteria,
                                                     const ::boost::unordered_set<sal_Int32>& rCatDims,
                                                     ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& rData);
    virtual void                    CalcResults(CalcInfo& rInfo, bool bAutoShow);
    virtual const ScDPCacheTable&   GetCacheTable() const;
    virtual void ReloadCacheTable();
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

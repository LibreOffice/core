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
    SC_DLLPUBLIC void SetRangeName(const OUString& rName);
    SC_DLLPUBLIC const OUString& GetRangeName() const;
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
    OUString maRangeName;
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

    ScDPFilteredCache  aCacheTable;

public:
    ScSheetDPData(ScDocument* pD, const ScSheetSourceDesc& rDesc, const ScDPCache& rCache);
    virtual ~ScSheetDPData();

    virtual long                    GetColumnCount() SAL_OVERRIDE;
    virtual OUString                getDimensionName(long nColumn) SAL_OVERRIDE;
    virtual bool                    getIsDataLayoutDimension(long nColumn) SAL_OVERRIDE;
    virtual bool                    IsDateDimension(long nDim) SAL_OVERRIDE;
    virtual sal_uLong               GetNumberFormat(long nDim) SAL_OVERRIDE;
    virtual void                    DisposeData() SAL_OVERRIDE;
    virtual void                    SetEmptyFlags( bool bIgnoreEmptyRows, bool bRepeatIfEmpty ) SAL_OVERRIDE;

    virtual bool                    IsRepeatIfEmpty() SAL_OVERRIDE;

    virtual void                    CreateCacheTable() SAL_OVERRIDE;
    virtual void                    FilterCacheTable(const ::std::vector<ScDPFilteredCache::Criterion>& rCriteria, const ::boost::unordered_set<sal_Int32>& rCatDims) SAL_OVERRIDE;
    virtual void                    GetDrillDownData(const ::std::vector<ScDPFilteredCache::Criterion>& rCriteria,
                                                     const ::boost::unordered_set<sal_Int32>& rCatDims,
                                                     ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& rData) SAL_OVERRIDE;
    virtual void                    CalcResults(CalcInfo& rInfo, bool bAutoShow) SAL_OVERRIDE;
    virtual const ScDPFilteredCache&   GetCacheTable() const SAL_OVERRIDE;
    virtual void ReloadCacheTable() SAL_OVERRIDE;
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

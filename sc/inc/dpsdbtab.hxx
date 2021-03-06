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

#pragma once

#include "dptabdat.hxx"

#include <com/sun/star/sheet/DataImportMode.hpp>

#include <unordered_set>
#include <vector>

class ScDocument;
class ScDPCache;
class ScDPDimensionSaveData;

struct ScImportSourceDesc
{
    OUString aDBName;
    OUString aObject;
    css::sheet::DataImportMode nType;
    bool    bNative;
    ScDocument* mpDoc;

    ScImportSourceDesc(ScDocument* pDoc) : nType(css::sheet::DataImportMode_NONE), bNative(false), mpDoc(pDoc) {}

    bool operator== ( const ScImportSourceDesc& rOther ) const
        { return aDBName == rOther.aDBName &&
                 aObject == rOther.aObject &&
                 nType   == rOther.nType &&
                 bNative == rOther.bNative &&
                mpDoc == rOther.mpDoc; }

    sal_Int32 GetCommandType() const;
    const ScDPCache* CreateCache(const ScDPDimensionSaveData* pDimData) const;
};

/**
 * This class represents source data from database source.
 */
class ScDatabaseDPData final : public ScDPTableData
{
private:
    ScDPFilteredCache aCacheTable;
public:
    ScDatabaseDPData(const ScDocument* pDoc, const ScDPCache& rCache);
    virtual ~ScDatabaseDPData() override;

    virtual sal_Int32               GetColumnCount() override;
    virtual OUString                getDimensionName(sal_Int32 nColumn) override;
    virtual bool                    getIsDataLayoutDimension(sal_Int32 nColumn) override;
    virtual bool                    IsDateDimension(sal_Int32 nDim) override;
    virtual void                    DisposeData() override;
    virtual void                    SetEmptyFlags( bool bIgnoreEmptyRows, bool bRepeatIfEmpty ) override;

    virtual void                    CreateCacheTable() override;
    virtual void                    FilterCacheTable(const ::std::vector<ScDPFilteredCache::Criterion>& rCriteria, const std::unordered_set<sal_Int32>& rDataDims) override;
    virtual void                    GetDrillDownData(const ::std::vector<ScDPFilteredCache::Criterion>& rCriteria,
                                                     const std::unordered_set<sal_Int32>& rCatDims,
                                                     css::uno::Sequence< css::uno::Sequence< css::uno::Any > >& rData) override;
    virtual void                    CalcResults(CalcInfo& rInfo, bool bAutoShow) override;
    virtual const ScDPFilteredCache&   GetCacheTable() const override;
    virtual void ReloadCacheTable() override;

#if DUMP_PIVOT_TABLE
    virtual void Dump() const override;
#endif
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

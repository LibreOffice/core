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

#ifndef INCLUDED_SC_INC_DPSDBTAB_HXX
#define INCLUDED_SC_INC_DPSDBTAB_HXX

#include <com/sun/star/uno/Reference.hxx>

#include "dptabdat.hxx"

#include <unordered_set>
#include <vector>

class ScDPFilteredCache;
class ScDocument;
class ScDPCache;
class ScDPDimensionSaveData;

struct ScImportSourceDesc
{
    OUString aDBName;
    OUString aObject;
    sal_uInt16  nType;          // enum DataImportMode
    bool    bNative;
    ScDocument* mpDoc;

    ScImportSourceDesc(ScDocument* pDoc) : nType(0), bNative(false), mpDoc(pDoc) {}

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
class ScDatabaseDPData : public ScDPTableData
{
private:
    ScDPFilteredCache aCacheTable;
public:
    ScDatabaseDPData(ScDocument* pDoc, const ScDPCache& rCache);
    virtual ~ScDatabaseDPData();

    virtual long                    GetColumnCount() override;
    virtual OUString                getDimensionName(long nColumn) override;
    virtual bool                    getIsDataLayoutDimension(long nColumn) override;
    virtual bool                    IsDateDimension(long nDim) override;
    virtual void                    DisposeData() override;
    virtual void                    SetEmptyFlags( bool bIgnoreEmptyRows, bool bRepeatIfEmpty ) override;

    virtual void                    CreateCacheTable() override;
    virtual void                    FilterCacheTable(const ::std::vector<ScDPFilteredCache::Criterion>& rCriteria, const std::unordered_set<sal_Int32>& rDataDims) override;
    virtual void                    GetDrillDownData(const ::std::vector<ScDPFilteredCache::Criterion>& rCriteria,
                                                     const std::unordered_set<sal_Int32>& rCatDims,
                                                     ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& rData) override;
    virtual void                    CalcResults(CalcInfo& rInfo, bool bAutoShow) override;
    virtual const ScDPFilteredCache&   GetCacheTable() const override;
    virtual void ReloadCacheTable() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

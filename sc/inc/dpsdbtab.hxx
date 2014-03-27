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

#ifndef SC_DPSDBTAB_HXX
#define SC_DPSDBTAB_HXX

#include <com/sun/star/uno/Reference.hxx>

#include "dptabdat.hxx"

#include <vector>
#include <boost/unordered_set.hpp>

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

    virtual long                    GetColumnCount() SAL_OVERRIDE;
    virtual OUString                getDimensionName(long nColumn) SAL_OVERRIDE;
    virtual bool                    getIsDataLayoutDimension(long nColumn) SAL_OVERRIDE;
    virtual bool                    IsDateDimension(long nDim) SAL_OVERRIDE;
    virtual void                    DisposeData() SAL_OVERRIDE;
    virtual void                    SetEmptyFlags( bool bIgnoreEmptyRows, bool bRepeatIfEmpty ) SAL_OVERRIDE;

    virtual void                    CreateCacheTable() SAL_OVERRIDE;
    virtual void                    FilterCacheTable(const ::std::vector<ScDPFilteredCache::Criterion>& rCriteria, const ::boost::unordered_set<sal_Int32>& rDataDims) SAL_OVERRIDE;
    virtual void                    GetDrillDownData(const ::std::vector<ScDPFilteredCache::Criterion>& rCriteria,
                                                     const ::boost::unordered_set<sal_Int32>& rCatDims,
                                                     ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& rData) SAL_OVERRIDE;
    virtual void                    CalcResults(CalcInfo& rInfo, bool bAutoShow) SAL_OVERRIDE;
    virtual const ScDPFilteredCache&   GetCacheTable() const SAL_OVERRIDE;
    virtual void ReloadCacheTable() SAL_OVERRIDE;
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

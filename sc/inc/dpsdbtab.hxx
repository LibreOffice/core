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
    ::rtl::OUString aDBName;
    ::rtl::OUString aObject;
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

    virtual long                    GetColumnCount();
    virtual rtl::OUString           getDimensionName(long nColumn);
    virtual sal_Bool                    getIsDataLayoutDimension(long nColumn);
    virtual sal_Bool                    IsDateDimension(long nDim);
    virtual void                    DisposeData();
    virtual void                    SetEmptyFlags( sal_Bool bIgnoreEmptyRows, sal_Bool bRepeatIfEmpty );

    virtual void                    CreateCacheTable();
    virtual void                    FilterCacheTable(const ::std::vector<ScDPFilteredCache::Criterion>& rCriteria, const ::boost::unordered_set<sal_Int32>& rDataDims);
    virtual void                    GetDrillDownData(const ::std::vector<ScDPFilteredCache::Criterion>& rCriteria,
                                                     const ::boost::unordered_set<sal_Int32>& rCatDims,
                                                     ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& rData);
    virtual void                    CalcResults(CalcInfo& rInfo, bool bAutoShow);
    virtual const ScDPFilteredCache&   GetCacheTable() const;
    virtual void ReloadCacheTable();
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

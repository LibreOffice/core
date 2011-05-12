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

#ifndef SC_DBCOLECT_HXX
#define SC_DBCOLECT_HXX

#include "scdllapi.h"
#include "collect.hxx"
#include "global.hxx"       // MAXQUERY
#include "sortparam.hxx"    // MAXSORT
#include "refreshtimer.hxx"
#include "address.hxx"
#include "scdllapi.h"
#include "subtotalparam.hxx"
#include "queryparam.hxx"

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/ptr_container/ptr_set.hpp>
#include <boost/scoped_ptr.hpp>

class ScDocument;

class ScDBData : public ScRefreshTimer
{
private:
    ScSortParam maSortParam;
    ScQueryParam maQueryParam;
    ScSubTotalParam maSubTotal;
    ScImportParam maImportParam;

    // DBParam
    ::rtl::OUString aName;
    SCTAB           nTable;
    SCCOL           nStartCol;
    SCROW           nStartRow;
    SCCOL           nEndCol;
    SCROW           nEndRow;
    bool            bByRow;
    bool            bHasHeader;
    bool            bDoSize;
    bool            bKeepFmt;
    bool            bStripData;

    // QueryParam
    bool            bIsAdvanced;        // true if created by advanced filter
    ScRange         aAdvSource;         // source range

    bool            bDBSelection;       // not in Param: if selection, block update

    sal_uInt16      nIndex;             // unique index formulas
    bool            bAutoFilter;        // AutoFilter? (not saved)
    bool            bModified;          // is set/cleared for/by(?) UpdateReference

    using ScRefreshTimer::operator==;

public:
    struct less : public ::std::binary_function<ScDBData, ScDBData, bool>
    {
        bool operator() (const ScDBData& left, const ScDBData& right) const;
    };

            SC_DLLPUBLIC ScDBData(const ::rtl::OUString& rName,
                     SCTAB nTab,
                     SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                     bool bByR = true, bool bHasH = true);
            ScDBData(const ScDBData& rData);
            ~ScDBData();

            ScDBData&   operator= (const ScDBData& rData);

            bool        operator== (const ScDBData& rData) const;

            SCTAB       GetTable() const;
            const ::rtl::OUString& GetName() const { return aName; }
            void        SetName(const ::rtl::OUString& rName) { aName = rName; }
            void        GetArea(SCTAB& rTab, SCCOL& rCol1, SCROW& rRow1, SCCOL& rCol2, SCROW& rRow2) const;
            SC_DLLPUBLIC void GetArea(ScRange& rRange) const;
            void        SetArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
            void        MoveTo(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
            bool        IsByRow() const                 { return bByRow; }
            void        SetByRow(bool bByR)             { bByRow = bByR; }
            bool        HasHeader() const               { return bHasHeader; }
            void        SetHeader(bool bHasH)           { bHasHeader = bHasH; }
            void        SetIndex(sal_uInt16 nInd)           { nIndex = nInd; }
            sal_uInt16  GetIndex() const                { return nIndex; }
            bool        IsDoSize() const                { return bDoSize; }
            void        SetDoSize(bool bSet)            { bDoSize = bSet; }
            bool        IsKeepFmt() const               { return bKeepFmt; }
            void        SetKeepFmt(bool bSet)           { bKeepFmt = bSet; }
            bool        IsStripData() const             { return bStripData; }
            void        SetStripData(bool bSet)         { bStripData = bSet; }

            ::rtl::OUString GetSourceString() const;
            ::rtl::OUString GetOperations() const;

            void        GetSortParam(ScSortParam& rSortParam) const;
            void        SetSortParam(const ScSortParam& rSortParam);

            SC_DLLPUBLIC void       GetQueryParam(ScQueryParam& rQueryParam) const;
            SC_DLLPUBLIC void       SetQueryParam(const ScQueryParam& rQueryParam);
            SC_DLLPUBLIC bool       GetAdvancedQuerySource(ScRange& rSource) const;
            SC_DLLPUBLIC void       SetAdvancedQuerySource(const ScRange* pSource);

            void        GetSubTotalParam(ScSubTotalParam& rSubTotalParam) const;
            void        SetSubTotalParam(const ScSubTotalParam& rSubTotalParam);

            void        GetImportParam(ScImportParam& rImportParam) const;
            void        SetImportParam(const ScImportParam& rImportParam);

            bool        IsDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, bool bStartOnly) const;
            bool        IsDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) const;

            bool        HasImportParam() const   { return maImportParam.bImport; }
            SC_DLLPUBLIC bool HasQueryParam() const;
            bool        HasSortParam() const     { return maSortParam.bDoSort[0]; }
            bool        HasSubTotalParam() const { return maSubTotal.bGroupActive[0]; }

            bool        HasImportSelection() const      { return bDBSelection; }
            void        SetImportSelection(bool bSet)   { bDBSelection = bSet; }

            bool        HasAutoFilter() const       { return bAutoFilter; }
            void        SetAutoFilter(bool bSet)    { bAutoFilter = bSet; }

            bool        IsModified() const          { return bModified; }
            void        SetModified(bool bMod)      { bModified = bMod; }

            void    UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos );
            void    UpdateReference(ScDocument* pDoc, UpdateRefMode eUpdateRefMode,
                                SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                SCsCOL nDx, SCsROW nDy, SCsTAB nDz);

            void ExtendDataArea(ScDocument* pDoc);
};

class SC_DLLPUBLIC ScDBCollection
{
    typedef ::boost::ptr_set<ScDBData, ScDBData::less> NamedDBsType;
public:
    typedef ::boost::ptr_vector<ScDBData> AnonDBsType;

private:
    Link        aRefreshHandler;
    ScDocument* pDoc;
    sal_uInt16 nEntryIndex;         // counter for unique indices
    NamedDBsType maNamedDBs;
    AnonDBsType maAnonDBs;

public:
    ScDBCollection(ScDocument* pDocument);
    ScDBCollection(const ScDBCollection& r);

    const ScDBData* GetDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, sal_Bool bStartOnly) const;
    const ScDBData* GetDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) const;
    const ScDBData* GetFilterDBAtTable(SCTAB nTab) const;
    ScDBData* GetDBNearCursor(SCCOL nCol, SCROW nRow, SCTAB nTab );

    void    DeleteOnTab( SCTAB nTab );
    void    UpdateReference(UpdateRefMode eUpdateRefMode,
                                SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                SCsCOL nDx, SCsROW nDy, SCsTAB nDz);
    void    UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos );

    sal_uInt16  GetEntryIndex()                 { return nEntryIndex; }
    void    SetEntryIndex(sal_uInt16 nInd)      { nEntryIndex = nInd; }

    void            SetRefreshHandler( const Link& rLink )
                        { aRefreshHandler = rLink; }
    const Link&     GetRefreshHandler() const   { return aRefreshHandler; }

    ScDBData* findByIndex(sal_uInt16 nIndex);
    ScDBData* findByName(const ::rtl::OUString& rName);
    bool insert(ScDBData* p);
    bool empty() const;
    size_t size() const;

    const ScDBData* findAnonAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, bool bStartOnly) const;
    const ScDBData* findAnonByRange(const ScRange& rRange) const;
    ScDBData* getAnonByRange(const ScRange& rRange);
    void insertAnonRange(ScDBData* pData);

    const AnonDBsType& getAnonRanges() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

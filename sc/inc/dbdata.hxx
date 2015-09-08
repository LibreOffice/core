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

#ifndef INCLUDED_SC_INC_DBDATA_HXX
#define INCLUDED_SC_INC_DBDATA_HXX

#include "scdllapi.h"
#include "refreshtimer.hxx"
#include "address.hxx"
#include "global.hxx"
#include "rangelst.hxx"

#include <svl/listener.hxx>

#include <boost/scoped_ptr.hpp>

#include <memory>
#include <set>
#include <vector>

class ScDocument;
struct ScSortParam;
struct ScQueryParam;
struct ScSubTotalParam;
struct ScImportParam;

/** Enum used to indicate which portion of the DBArea is to be considered. */
enum class ScDBDataPortion
{
    TOP_LEFT,   ///< top left cell of area
    HEADER,     ///< header row of area, if headers are present
    AREA        ///< entire area
};

/** Container base class to provide selected access for ScDBData. */
class ScDBDataContainerBase
{
public:
    ScDBDataContainerBase( ScDocument& rDoc ) : mrDoc(rDoc) {}
    virtual ~ScDBDataContainerBase() {}
    ScDocument& GetDocument() const;
    ScRangeList& GetDirtyTableColumnNames();

protected:
    ScDocument& mrDoc;
    ScRangeList maDirtyTableColumnNames;
};

class ScDBData : public SvtListener, public ScRefreshTimer
{
private:
    boost::scoped_ptr<ScSortParam> mpSortParam;
    boost::scoped_ptr<ScQueryParam> mpQueryParam;
    boost::scoped_ptr<ScSubTotalParam> mpSubTotal;
    boost::scoped_ptr<ScImportParam> mpImportParam;

    ScDBDataContainerBase* mpContainer;

    /// DBParam
    const OUString aName;
    OUString aUpper;
    SCTAB           nTable;
    SCCOL           nStartCol;
    SCROW           nStartRow;
    SCCOL           nEndCol;
    SCROW           nEndRow;
    bool            bByRow;
    bool            bHasHeader;
    bool            bHasTotals;
    bool            bDoSize;
    bool            bKeepFmt;
    bool            bStripData;

    /// QueryParam
    bool            bIsAdvanced;        ///< true if created by advanced filter
    ScRange         aAdvSource;         ///< source range

    bool            bDBSelection;       ///< not in Param: if selection, block update

    sal_uInt16      nIndex;             ///< unique index formulas
    bool            bAutoFilter;        ///< AutoFilter? (not saved)
    bool            bModified;          ///< is set/cleared for/by(?) UpdateReference

    ::std::vector< OUString > maTableColumnNames;   ///< names of table columns
    bool            mbTableColumnNamesDirty;

    using ScRefreshTimer::operator==;

public:
    struct less : public ::std::binary_function<std::unique_ptr<ScDBData>, std::unique_ptr<ScDBData>, bool>
    {
        bool operator() (const std::unique_ptr<ScDBData>& left, const std::unique_ptr<ScDBData>& right) const;
    };

    SC_DLLPUBLIC ScDBData(const OUString& rName,
             SCTAB nTab,
             SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
             bool bByR = true, bool bHasH = true, bool bTotals = false);
    ScDBData(const ScDBData& rData);
    ScDBData(const OUString& rName, const ScDBData& rData);
    virtual ~ScDBData();

    virtual void Notify( const SfxHint& rHint ) SAL_OVERRIDE;

    ScDBData&   operator= (const ScDBData& rData) ;

    bool        operator== (const ScDBData& rData) const;

    const OUString& GetName() const { return aName; }
    const OUString& GetUpperName() const { return aUpper; }
    void        GetArea(SCTAB& rTab, SCCOL& rCol1, SCROW& rRow1, SCCOL& rCol2, SCROW& rRow2) const;
    SC_DLLPUBLIC void GetArea(ScRange& rRange) const;
    void        SetArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
    void        MoveTo(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
    void        SetByRow(bool bByR)             { bByRow = bByR; }
    bool        HasHeader() const               { return bHasHeader; }
    void        SetHeader(bool bHasH)           { bHasHeader = bHasH; }
    bool        HasTotals() const               { return bHasTotals; }
    void        SetTotals(bool bTotals)         { bHasTotals = bTotals; }
    void        SetIndex(sal_uInt16 nInd)           { nIndex = nInd; }
    sal_uInt16  GetIndex() const                { return nIndex; }
    bool        IsDoSize() const                { return bDoSize; }
    void        SetDoSize(bool bSet)            { bDoSize = bSet; }
    bool        IsKeepFmt() const               { return bKeepFmt; }
    void        SetKeepFmt(bool bSet)           { bKeepFmt = bSet; }
    bool        IsStripData() const             { return bStripData; }
    void        SetStripData(bool bSet)         { bStripData = bSet; }

    void        SetContainer( ScDBDataContainerBase* pContainer ) { mpContainer = pContainer; }
    /** Returns header row range if has headers, else invalid range. */
    ScRange     GetHeaderArea() const;
    void        StartTableColumnNamesListener();
    void        EndTableColumnNamesListener();
    SC_DLLPUBLIC void SetTableColumnNames( const ::std::vector< OUString >& rNames );
    SC_DLLPUBLIC const ::std::vector< OUString >& GetTableColumnNames() const { return maTableColumnNames; }
    bool        AreTableColumnNamesDirty() const { return mbTableColumnNamesDirty; }

    /** Refresh/update the column names with the header row's cell contents. */
    SC_DLLPUBLIC void RefreshTableColumnNames( ScDocument* pDoc );

    /** Refresh/update the column names with the header row's cell contents
        within the given range. */
    void RefreshTableColumnNames( ScDocument* pDoc, const ScRange& rRange );

    /** Finds the column named rName and returns the corresponding offset
        within the table.
        @returns -1 if not found.

        XXX NOTE: there is no refresh of names or anything implemented yet, use
        this only during document load time.
     */
    sal_Int32   GetColumnNameOffset( const OUString& rName ) const;

    /** Returns table column name if nCol is within column range and name
        is stored, else empty string. */
    const OUString& GetTableColumnName( SCCOL nCol ) const;

    OUString GetSourceString() const;
    OUString GetOperations() const;

    void        GetSortParam(ScSortParam& rSortParam) const;
    void        SetSortParam(const ScSortParam& rSortParam);

    /** Remember some more settings of ScSortParam, only to be called at
        anonymous DB ranges as it at least overwrites bHasHeader. */
    void        UpdateFromSortParam( const ScSortParam& rSortParam );

    SC_DLLPUBLIC void       GetQueryParam(ScQueryParam& rQueryParam) const;
    SC_DLLPUBLIC void       SetQueryParam(const ScQueryParam& rQueryParam);
    SC_DLLPUBLIC bool       GetAdvancedQuerySource(ScRange& rSource) const;
    SC_DLLPUBLIC void       SetAdvancedQuerySource(const ScRange* pSource);

    void        GetSubTotalParam(ScSubTotalParam& rSubTotalParam) const;
    void        SetSubTotalParam(const ScSubTotalParam& rSubTotalParam);

    void        GetImportParam(ScImportParam& rImportParam) const;
    void        SetImportParam(const ScImportParam& rImportParam);

    bool        IsDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, ScDBDataPortion ePortion) const;
    bool        IsDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) const;

    bool        HasImportParam() const;
    SC_DLLPUBLIC bool HasQueryParam() const;
    bool        HasSortParam() const;
    bool        HasSubTotalParam() const;

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

private:

    void AdjustTableColumnNames( UpdateRefMode eUpdateRefMode, SCCOL nDx, SCCOL nCol1,
            SCCOL nOldCol1, SCCOL nOldCol2, SCCOL nNewCol1, SCCOL nNewCol2 );
    void InvalidateTableColumnNames( bool bSwapToEmptyNames );
};

class SC_DLLPUBLIC ScDBCollection
{
public:
    enum RangeType { GlobalNamed, GlobalAnonymous, SheetAnonymous };

    /**
     * Stores global named database ranges.
     */
    class SC_DLLPUBLIC NamedDBs : public ScDBDataContainerBase
    {
        friend class ScDBCollection;

        typedef ::std::set<std::unique_ptr<ScDBData>, ScDBData::less> DBsType;
        DBsType m_DBs;
        ScDBCollection& mrParent;
        NamedDBs(ScDBCollection& rParent, ScDocument& rDoc);
        NamedDBs(const NamedDBs& r);
        virtual ~NamedDBs();
        NamedDBs & operator=(NamedDBs const&) = delete;
        void initInserted( ScDBData* p );

    public:
        typedef DBsType::iterator iterator;
        typedef DBsType::const_iterator const_iterator;

        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        ScDBData* findByIndex(sal_uInt16 nIndex);
        ScDBData* findByUpperName(const OUString& rName);
        iterator findByUpperName2(const OUString& rName);
        // Takes ownership of p iff it returns true:
        SAL_WARN_UNUSED_RESULT bool insert(ScDBData* p);
        void erase(iterator itr);
        bool empty() const;
        size_t size() const;
        bool operator== (const NamedDBs& r) const;
    };

    /**
     * Stores global anonymous database ranges.
     */
    class AnonDBs
    {
        typedef ::std::vector<std::unique_ptr<ScDBData>> DBsType;
        DBsType m_DBs;

        AnonDBs& operator=(AnonDBs const&) = delete;

    public:
        AnonDBs();
        AnonDBs(AnonDBs const&);

        typedef DBsType::iterator iterator;
        typedef DBsType::const_iterator const_iterator;

        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        const ScDBData* findAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, ScDBDataPortion ePortion) const;
        const ScDBData* findByRange(const ScRange& rRange) const;
        void deleteOnTab(SCTAB nTab);
        ScDBData* getByRange(const ScRange& rRange);
        void insert(ScDBData* p);
        bool empty() const;
        bool has( const ScDBData* p ) const;
        bool operator== (const AnonDBs& r) const;
    };

private:
    Link<Timer *, void> aRefreshHandler;
    ScDocument* pDoc;
    sal_uInt16 nEntryIndex;         ///< counter for unique indices
    NamedDBs maNamedDBs;
    AnonDBs maAnonDBs;

public:
    ScDBCollection(ScDocument* pDocument);
    ScDBCollection(const ScDBCollection& r);

    NamedDBs& getNamedDBs() { return maNamedDBs;}
    const NamedDBs& getNamedDBs() const { return maNamedDBs;}

    AnonDBs& getAnonDBs() { return maAnonDBs;}
    const AnonDBs& getAnonDBs() const { return maAnonDBs;}

    const ScDBData* GetDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, ScDBDataPortion ePortion) const;
    ScDBData* GetDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, ScDBDataPortion ePortion);
    const ScDBData* GetDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) const;
    ScDBData* GetDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
    ScDBData* GetDBNearCursor(SCCOL nCol, SCROW nRow, SCTAB nTab );

    void RefreshDirtyTableColumnNames();

    void    DeleteOnTab( SCTAB nTab );
    void    UpdateReference(UpdateRefMode eUpdateRefMode,
                                SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                SCsCOL nDx, SCsROW nDy, SCsTAB nDz);
    void    UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos );

    void            SetRefreshHandler( const Link<Timer *, void>& rLink )
                        { aRefreshHandler = rLink; }
    const Link<Timer *, void>& GetRefreshHandler() const { return aRefreshHandler; }

    bool empty() const;
    bool operator== (const ScDBCollection& r) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

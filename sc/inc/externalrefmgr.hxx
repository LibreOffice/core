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

#ifndef INCLUDED_SC_INC_EXTERNALREFMGR_HXX
#define INCLUDED_SC_INC_EXTERNALREFMGR_HXX

#include "address.hxx"
#include <sfx2/objsh.hxx>
#include <sfx2/lnkbase.hxx>
#include <tools/time.hxx>
#include <vcl/timer.hxx>
#include <svl/zforlist.hxx>
#include <svl/lstner.hxx>
#include "types.hxx"
#include "rangelst.hxx"
#include <osl/mutex.hxx>
#include <formula/types.hxx>

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <set>
#include <formula/ExternalReferenceHelper.hxx>

class ScDocument;
class ScTokenArray;
namespace weld { class Window; }

class ScFormulaCell;

namespace sc {

class ColumnSpanSet;

}

class ScExternalRefLink : public ::sfx2::SvBaseLink
{
public:
    ScExternalRefLink(ScDocument* pDoc, sal_uInt16 nFileId);
    virtual ~ScExternalRefLink() override;

    virtual void Closed() override;
    virtual ::sfx2::SvBaseLink::UpdateResult DataChanged(
        const OUString& rMimeType, const css::uno::Any & rValue) override;
    virtual void Edit(weld::Window* pParent, const Link<SvBaseLink&,void>& rEndEditHdl) override;

    void SetDoReferesh(bool b);

private:
    ScExternalRefLink() = delete;
    ScExternalRefLink(const ScExternalRefLink&) = delete;

    sal_uInt16 const  mnFileId;
    ScDocument* const mpDoc;
    bool        mbDoRefresh;
};

/**
 * Cache table for external reference data.
 */
class ScExternalRefCache
{
public:
    typedef ::formula::FormulaTokenRef          TokenRef;
    typedef std::shared_ptr<ScTokenArray>   TokenArrayRef;

    struct TableName
    {
        OUString maUpperName;
        OUString maRealName;

        explicit TableName(const OUString& rUppper, const OUString& rReal);
    };

    struct CellFormat
    {
        bool      mbIsSet;
        SvNumFormatType mnType;
        sal_uLong mnIndex;

        explicit CellFormat();
    };

private:
    /** individual cell within cached external ref table. */
    struct Cell
    {
        TokenRef   mxToken;
        sal_uLong  mnFmtIndex;
    };
    typedef std::unordered_map<SCCOL, Cell>            RowDataType;
    typedef std::unordered_map<SCROW, RowDataType>     RowsDataType;

public:
    /**
     * Represents a single cached table in an external document.  It only
     * stores non-empty cells; empty cells should never be stored in the data
     * cache. Instead, cached ranges should be used to determine whether or
     * not a cell is empty or needs fetching from the source document.  If a
     * cell's value is not stored but its address is within the cached ranges,
     * that cell is already queried in the source document and we know it's
     * empty.
     */
    class Table
    {
    public:

        Table();
        ~Table();

        void clear();

        /**
         * Add cell value to the cache.
         *
         * @param bSetCacheRange if true, mark this cell 'cached'.  This is
         *                       false _only when_ adding a range of cell
         *                       values, for performance reasons.
         */
        SC_DLLPUBLIC void setCell(SCCOL nCol, SCROW nRow, TokenRef const & pToken, sal_uLong nFmtIndex = 0, bool bSetCacheRange = true);
        SC_DLLPUBLIC TokenRef getCell(SCCOL nCol, SCROW nRow, sal_uInt32* pnFmtIndex = nullptr) const;
        bool hasRow( SCROW nRow ) const;
        /** Set/clear referenced status flag only if current status is not
            REFERENCED_PERMANENT. */
        void setReferenced( bool bReferenced );
        bool isReferenced() const;
        /// Obtain a sorted vector of rows.
        void getAllRows(::std::vector<SCROW>& rRows, SCROW nLow = 0, SCROW nHigh = MAXROW) const;
        /// Returns the half-open range of used rows in this table. Returns [0,0) if table is empty.
        SC_DLLPUBLIC ::std::pair< SCROW, SCROW > getRowRange() const;
        /// Obtain a sorted vector of columns.
        void getAllCols(SCROW nRow, ::std::vector<SCCOL>& rCols, SCCOL nLow = 0, SCCOL nHigh = MAXCOL) const;
        /// Returns the half-open range of used columns in the specified row. Returns [0,0) if row is empty.
        SC_DLLPUBLIC ::std::pair< SCCOL, SCCOL > getColRange( SCROW nRow ) const;
        void getAllNumberFormats(::std::vector<sal_uInt32>& rNumFmts) const;
        bool isRangeCached(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) const;

        void setCachedCell(SCCOL nCol, SCROW nRow);
        void setCachedCellRange(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);

        /**
         * Call this to mark the entire table "cached".  This will prevent all
         * future attempts to access the source document even when non-cached
         * cells are queried.  In such case, non-cached cells are treated as
         * empty cells.  Useful when loading a document with own external data
         * cache.
         */
        SC_DLLPUBLIC void setWholeTableCached();
    private:
        bool isInCachedRanges(SCCOL nCol, SCROW nRow) const;
        TokenRef getEmptyOrNullToken(SCCOL nCol, SCROW nRow) const;

    private:
        /** Data cache */
        RowsDataType                    maRows;
        /** Collection of individual cached ranges.  The table ranges are
         *  not used & always zero. */
        ScRangeList                     maCachedRanges;
        bool                            mbReferenced;
    };

    typedef std::shared_ptr<Table> TableTypeRef;
    typedef std::unordered_map< OUString, size_t>
        TableNameIndexMap;

    ScExternalRefCache();
    ~ScExternalRefCache();

    const OUString* getRealTableName(sal_uInt16 nFileId, const OUString& rTabName) const;
    const OUString* getRealRangeName(sal_uInt16 nFileId, const OUString& rRangeName) const;

    /**
     * Get a cached cell data at specified cell location.
     *
     * @param nFileId file ID of an external document
     * @param rTabName sheet name
     * @param nCol
     * @param nRow
     *
     * @return pointer to the token instance in the cache.
     */
    ScExternalRefCache::TokenRef getCellData(
        sal_uInt16 nFileId, const OUString& rTabName, SCCOL nCol, SCROW nRow, sal_uInt32* pnFmtIndex);

    /**
     * Get a cached cell range data.
     *
     * @return a new token array instance.  Note that <i>the caller must
     *         manage the life cycle of the returned instance</i>, which is
     *         guaranteed if the TokenArrayRef is properly used..
     */
    ScExternalRefCache::TokenArrayRef getCellRangeData(
        sal_uInt16 nFileId, const OUString& rTabName, const ScRange& rRange);

    ScExternalRefCache::TokenArrayRef getRangeNameTokens(sal_uInt16 nFileId, const OUString& rName);
    void setRangeNameTokens(sal_uInt16 nFileId, const OUString& rName, TokenArrayRef pArray);
    bool isValidRangeName(sal_uInt16 nFileId, const OUString& rName) const;
    void setRangeName(sal_uInt16 nFileId, const OUString& rName);

    void setCellData(sal_uInt16 nFileId, const OUString& rTabName,
                     SCCOL nCol, SCROW nRow, TokenRef const & pToken, sal_uLong nFmtIndex);

    struct SingleRangeData
    {
        /** This name must be in upper-case. */
        OUString maTableName;
        ScMatrixRef mpRangeData;
    };
    void setCellRangeData(sal_uInt16 nFileId, const ScRange& rRange, const ::std::vector<SingleRangeData>& rData,
                          const TokenArrayRef& pArray);

    bool isDocInitialized(sal_uInt16 nFileId);
    void initializeDoc(sal_uInt16 nFileId, const ::std::vector<OUString>& rTabNames, const OUString& rBaseName);
    OUString getTableName(sal_uInt16 nFileId, size_t nCacheId) const;
    void getAllTableNames(sal_uInt16 nFileId, ::std::vector<OUString>& rTabNames) const;
    SCTAB getTabSpan( sal_uInt16 nFileId, const OUString& rStartTabName, const OUString& rEndTabName ) const;
    void getAllNumberFormats(::std::vector<sal_uInt32>& rNumFmts) const;

    /**
     * Set all tables of a document as referenced, used only during
     * store-to-file.
     * @returns <TRUE/> if ALL tables of ALL documents are marked.
     */
    bool setCacheDocReferenced( sal_uInt16 nFileId );

    /**
     * Set a table as referenced, used only during store-to-file.
     * @returns <TRUE/> if ALL tables of ALL documents are marked.
     */
    bool setCacheTableReferenced( sal_uInt16 nFileId, const OUString& rTabName, size_t nSheets );
    void setAllCacheTableReferencedStati( bool bReferenced );
    bool areAllCacheTablesReferenced() const { return maReferenced.mbAllReferenced;}

    /**
     * Collect all cached non-empty cell positions, inferred directly from the
     * cached data, not the cached range metadata stored separately in the
     * Table.
     */
    void getAllCachedDataSpans( sal_uInt16 nFileId, sc::ColumnSpanSet& rSet ) const;

    bool getSrcDocTable( const ScDocument& rSrcDoc, const OUString& rTabName, SCTAB& rTab, sal_uInt16 nFileId ) const;

private:
    struct ReferencedStatus
    {
        struct DocReferenced
        {
            ::std::vector<bool> maTables;
            bool                mbAllTablesReferenced;
            // Initially, documents have no tables but all referenced.
            DocReferenced() : mbAllTablesReferenced(true) {}
        };
        typedef ::std::vector<DocReferenced> DocReferencedVec;

        DocReferencedVec maDocs;
        bool             mbAllReferenced;

                    ReferencedStatus();
        void        reset( size_t nDocs );
        void        checkAllDocs();

    } maReferenced;
    void addCacheTableToReferenced( sal_uInt16 nFileId, size_t nIndex );
    void addCacheDocToReferenced( sal_uInt16 nFileId );
public:

    ScExternalRefCache::TableTypeRef getCacheTable(sal_uInt16 nFileId, size_t nTabIndex) const;
    ScExternalRefCache::TableTypeRef getCacheTable(sal_uInt16 nFileId, const OUString& rTabName, bool bCreateNew,
            size_t* pnIndex, const OUString* pExtUrl);

    /**
     * Clear all caches including the cache tables.
     */
    void clearCache(sal_uInt16 nFileId);

    /**
     * Clear all caches but keep the tables.  All cache tables will be empty
     * after the call, but the tables will not be removed.
     */
    void clearCacheTables(sal_uInt16 nFileId);

private:
    struct RangeHash
    {
        size_t operator()(const ScRange& rRange) const
        {
            const ScAddress& s = rRange.aStart;
            const ScAddress& e = rRange.aEnd;
            return s.Tab() + s.Col() + s.Row() + e.Tab() + e.Col() + e.Row();
        }
    };

    typedef std::unordered_map<OUString, TokenArrayRef> RangeNameMap;
    typedef std::unordered_map<ScRange, TokenArrayRef, RangeHash> RangeArrayMap;
    typedef std::unordered_map<OUString, OUString> NamePairMap;

    /** Represents data cached for a single external document. */
    struct DocItem
    {
        /** The raw cache tables. */
        ::std::vector<TableTypeRef> maTables;
        /** Table name list in correct order, in both upper- and real-case. */
        ::std::vector<TableName>    maTableNames;
        /** Table name to index map.  The names must be stored upper-case. */
        TableNameIndexMap           maTableNameIndex;
        /** Range name cache. */
        RangeNameMap                maRangeNames;
        /** Token array cache for cell ranges. */
        RangeArrayMap               maRangeArrays;
        /** Upper- to real-case mapping for range names. */
        NamePairMap                 maRealRangeNameMap;

        /** Either the base name that was stored as sheet name for CSV files if
            sheet name is Sheet1, or Sheet1 name if sheet name is base name.
         */
        OUString                    maSingleTableNameAlias;

        bool mbInitFromSource;

        DocItem() : mbInitFromSource(false) {}

        TableNameIndexMap::const_iterator findTableNameIndex( const OUString& rTabName ) const;
        bool getTableDataIndex( const OUString& rTabName, size_t& rIndex ) const;
        bool getSingleTableNameAlternative( OUString& rTabName ) const;
    };
    typedef std::unordered_map<sal_uInt16, DocItem>  DocDataType;
    DocItem* getDocItem(sal_uInt16 nFileId) const;

private:
    mutable osl::Mutex maMtxDocs;
    mutable DocDataType maDocs;
};

class SC_DLLPUBLIC ScExternalRefManager : public formula::ExternalReferenceHelper, public SfxListener
{
public:

    typedef std::set<ScFormulaCell*>                      RefCellSet;
    typedef std::unordered_map<sal_uInt16, RefCellSet>         RefCellMap;

    enum LinkUpdateType { LINK_MODIFIED, LINK_BROKEN };

    /**
     * Base class for objects that need to listen to link updates.  When a
     * link to a certain external file is updated, the notify() method gets
     * called.
     */
    class LinkListener
    {
    public:
        LinkListener();
        virtual ~LinkListener() = 0;
        virtual void notify(sal_uInt16 nFileId, LinkUpdateType eType) = 0;

        struct Hash
        {
            size_t operator() (const LinkListener* p) const
            {
                return reinterpret_cast<size_t>(p);
            }
        };
    };

    /**
     * Use this guard when performing something from the API that might query
     * values from external references.  Interpreting formula strings is one
     * such example.
     */
    class SC_DLLPUBLIC ApiGuard
    {
    public:
        ApiGuard(const ScDocument* pDoc);
        ~ApiGuard();
    private:
        ScExternalRefManager* mpMgr;
        bool const mbOldInteractionEnabled;
    };

private:
    /** Shell instance for a source document. */
    struct SrcShell
    {
        SfxObjectShellRef   maShell;
        tools::Time         maLastAccess;

        SrcShell() : maLastAccess( tools::Time::SYSTEM ) {}
    };

    typedef std::unordered_map<sal_uInt16, SrcShell>           DocShellMap;
    typedef std::unordered_map<sal_uInt16, bool>               LinkedDocMap;

    typedef std::unordered_map<sal_uInt16, SvNumberFormatterMergeMap> NumFmtMap;

    typedef std::unordered_set<LinkListener*, LinkListener::Hash>  LinkListeners;
    typedef std::unordered_map<sal_uInt16, LinkListeners>          LinkListenerMap;

public:
    /** Source document meta-data container. */
    struct SrcFileData
    {
        OUString maFileName;      /// original file name as loaded from the file.
        OUString maRealFileName;  /// file name created from the relative name.
        OUString maRelativeName;
        OUString maFilterName;
        OUString maFilterOptions;

        void maybeCreateRealFileName(const OUString& rOwnDocName);
    };

public:
    explicit ScExternalRefManager(ScDocument* pDoc);
    virtual ~ScExternalRefManager() override;

    virtual OUString getCacheTableName(sal_uInt16 nFileId, size_t nTabIndex) const override;

    /**
     * Get a cache table instance for specified table and table index.  Unlike
     * the other method that takes a table name, this method does not create a
     * new table when a table is not available for specified index.
     *
     * @param nFileId file ID
     * @param nTabIndex cache table index
     *
     * @return shared_ptr to the cache table instance
     */
    ScExternalRefCache::TableTypeRef getCacheTable(sal_uInt16 nFileId, size_t nTabIndex) const;

    /**
     * Get a cache table instance for specified file and table name.  If the
     * table instance is not already present, it'll instantiate a new one and
     * append it to the end of the table array.  <I>It's important to be
     * aware of this fact especially for multi-table ranges for which
     * table orders are critical.</I>
     *
     * Excel filter calls this method to populate the cache table from the
     * XCT/CRN records. ODF import calls it for cached tables for external
     * references.
     *
     * @param nFileId file ID
     * @param rTabName table name
     * @param bCreateNew if true, create a new table instance if it's not
     *                   already present.  If false, it returns NULL if the
     *                   specified table's cache doesn't exist.
     * @param pnIndex if non-NULL pointer is passed, it stores the internal
     *                index of a cache table instance.
     * @param pExtUrl if non-NULL and bCreateNew==true, the base name will be
     *                propagated as an alias for the first table (and removed
     *                later if further tables are created).
     *
     * @return shared_ptr to the cache table instance
     */
    ScExternalRefCache::TableTypeRef getCacheTable(sal_uInt16 nFileId, const OUString& rTabName, bool bCreateNew,
            size_t* pnIndex = nullptr, const OUString* pExtUrl = nullptr);

    /** Returns a vector containing all (real) table names and cache tables of
        the specified file.

        The index in the returned vector corresponds to the table index used to
        access the cache table, e.g. in getCacheTable().
     */
    void getAllCachedTableNames(sal_uInt16 nFileId, ::std::vector<OUString>& rTabNames) const;

    /**
     * Get the span (distance+sign(distance)) of two sheets of a specified
     * file.
     *
     * @param nFileId file ID
     * @param rStartTabName name of first sheet (sheet1)
     * @param rEndTabName name of second sheet (sheet2)
     *
     * @return span
     *         1 if sheet2 == sheet1
     *      >  1 if sheet2 > sheet1
     *      < -1 if sheet2 < sheet1
     *        -1 if nFileId or rStartTabName not found
     *         0 if rEndTabName not found
     */
    SCTAB getCachedTabSpan(
        sal_uInt16 nFileId, const OUString& rStartTabName, const OUString& rEndTabName) const;

    /**
     * Get all unique number format indices that are used in the cache tables.
     * The retrieved indices are sorted in ascending order.
     *
     * @param rNumFmts (reference) all unique number format indices.
     */
    void getAllCachedNumberFormats(::std::vector<sal_uInt32>& rNumFmts) const;

    sal_uInt16 getExternalFileCount() const;

    /**
     * Mark all tables as referenced that are used by any LinkListener, used
     * only during store-to-file.
     */
    void markUsedByLinkListeners();

    void markUsedExternalRefCells();

    /**
     * Set a table as referenced, used only during store-to-file.
     * @returns <TRUE/> if ALL tables of ALL external documents are marked.
     */
    bool setCacheTableReferenced( sal_uInt16 nFileId, const OUString& rTabName, size_t nSheets );
    void setAllCacheTableReferencedStati( bool bReferenced );

    /**
     * @returns <TRUE/> if setAllCacheTableReferencedStati(false) was called,
     * <FALSE/> if setAllCacheTableReferencedStati(true) was called.
     */
    bool isInReferenceMarking() const   { return mbInReferenceMarking; }

    void storeRangeNameTokens(sal_uInt16 nFileId, const OUString& rName, const ScTokenArray& rArray);

    ScExternalRefCache::TokenRef getSingleRefToken(
        sal_uInt16 nFileId, const OUString& rTabName, const ScAddress& rCell,
        const ScAddress* pCurPos, SCTAB* pTab, ScExternalRefCache::CellFormat* pFmt = nullptr);

    /**
     * Get an array of tokens that consist of the specified external cell
     * range.
     *
     * @param nFileId file ID for an external document
     * @param rTabName referenced sheet name
     * @param rRange referenced cell range
     * @param pCurPos current cursor position to keep track of cells that
     *                reference an external data.
     *
     * @return shared_ptr to a token array instance.  <i>The caller must not
     *         delete the instance returned by this method.</i>
     */
    ScExternalRefCache::TokenArrayRef getDoubleRefTokens(
        sal_uInt16 nFileId, const OUString& rTabName, const ScRange& rRange, const ScAddress* pCurPos);

    /**
     * Get an array of tokens corresponding with a specified name in a
     * specified file.
     *
     * @param pCurPos current cell address where this name token is used.
     *                This is purely to keep track of all cells containing
     *                external names for refreshing purposes.  If this is
     *                NULL, then the cell will not be added to the list.
     *
     * @return shared_ptr to array of tokens composing the name
     */
    ScExternalRefCache::TokenArrayRef getRangeNameTokens(
        sal_uInt16 nFileId, const OUString& rName, const ScAddress* pCurPos = nullptr);

    bool isValidRangeName(sal_uInt16 nFileId, const OUString& rName);

    OUString getOwnDocumentName() const;
    bool isOwnDocument(const OUString& rFile) const;

    /**
     * Takes a flat file name, and convert it to an absolute URL path.  An
     * absolute URL path begins with 'file:///.
     *
     * @param rFile file name to convert
     */
    void convertToAbsName(OUString& rFile) const;
    sal_uInt16 getExternalFileId(const OUString& rFile);

    /**
     * It returns a pointer to the name of the URI associated with a given
     * external file ID.  In case the original document has moved, it returns
     * an URI adjusted for the relocation.
     *
     * @param nFileId file ID for an external document
     * @param bForceOriginal If true, it always returns the original document
     *                       URI even if the referring document has relocated.
     *                       If false, it returns an URI adjusted for
     *                       relocated document.
     *
     * @return const OUString* external document URI.
     */
    const OUString* getExternalFileName(sal_uInt16 nFileId, bool bForceOriginal = false);

    /**
     * Get all cached external file names as an array. Array indices of the
     * returned name array correspond with external file ID's.
     */
    std::vector<OUString> getAllCachedExternalFileNames() const;

    bool hasExternalFile(sal_uInt16 nFileId) const;
    bool hasExternalFile(const OUString& rFile) const;
    const SrcFileData* getExternalFileData(sal_uInt16 nFileId) const;

    const OUString* getRealTableName(sal_uInt16 nFileId, const OUString& rTabName) const;
    const OUString* getRealRangeName(sal_uInt16 nFileId, const OUString& rRangeName) const;
    void clearCache(sal_uInt16 nFileId);
    bool refreshSrcDocument(sal_uInt16 nFileId);
    void breakLink(sal_uInt16 nFileId);
    void switchSrcFile(sal_uInt16 nFileId, const OUString& rNewFile, const OUString& rNewFilter);

    /**
     * Set a relative file path for the specified file ID.  Note that the
     * caller must ensure that the passed URL is a valid relative URL.
     *
     * @param nFileId file ID for an external document
     * @param rRelUrl relative URL
     */
    void setRelativeFileName(sal_uInt16 nFileId, const OUString& rRelUrl);

    /**
     * Set the filter name and options if any for a given source document.
     * These values get reset when the source document ever gets reloaded.
     *
     * @param nFileId
     * @param rFilterName
     * @param rOptions
     */
    void setFilterData(sal_uInt16 nFileId, const OUString& rFilterName, const OUString& rOptions);

    void clear();

    bool hasExternalData() const;

    /**
     * Re-generates relative names for all stored source files.  This is
     * necessary when exporting to an ods document, to ensure that all source
     * files have their respective relative names for xlink:href export.
     *
     * @param rBaseFileUrl Absolute URL of the content.xml fragment of the
     *                     document being exported.
     */
    void resetSrcFileData(const OUString& rBaseFileUrl);

    /**
     * Replace the original URL with the real URL that was generated from the relative URL.
     */
    void updateAbsAfterLoad();

    /**
     * Stop tracking a specific formula cell.
     *
     * @param pCell pointer to cell that formerly contained external
     *              reference.
     */
    void removeRefCell(ScFormulaCell* pCell);

    /**
     * Register a new link listener to a specified external document.  Note
     * that the caller is responsible for managing the life cycle of the
     * listener object.
     */
    void addLinkListener(sal_uInt16 nFileId, LinkListener* pListener);

    /**
     * Remove an existing link listener.  Note that removing a listener
     * pointer here does not delete the listener object instance.
     */
    void removeLinkListener(sal_uInt16 nFileId, LinkListener* pListener);

    void removeLinkListener(LinkListener* pListener);

    /**
     * Notify all listeners that are listening to a specified external
     * document.
     *
     * @param nFileId file ID for an external document.
     */
    void notifyAllLinkListeners(sal_uInt16 nFileId, LinkUpdateType eType);

    /**
     * Check if the file specified by the path is a legitimate file that
     * exists & can be loaded.
     */
    bool isFileLoadable(const OUString& rFile) const;

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    /**
     * If we still contain unsaved files we should warn the user before saving
     *
     * @return true if the document still contains references to an unsaved file
     */
    bool containsUnsavedReferences() const { return !maUnsavedDocShells.empty(); }

    void insertRefCell(sal_uInt16 nFileId, const ScAddress& rCell);
    /**
     * Add a cell to reference the same files as the template cell.
     */
    void insertRefCellFromTemplate( ScFormulaCell* pTemplateCell, ScFormulaCell* pCell );

    bool hasCellExternalReference(const ScAddress& rCell);

    void enableDocTimer( bool bEnable );

    /** Add all known external files to the LinkManager. */
    void addFilesToLinkManager();

private:
    ScExternalRefManager(const ScExternalRefManager&) = delete;

    void refreshAllRefCells(sal_uInt16 nFileId);

    void fillCellFormat(sal_uLong nFmtIndex, ScExternalRefCache::CellFormat* pFmt) const;

    bool getSrcDocTable( const ScDocument& rSrcDoc, const OUString& rTabName, SCTAB& rTab, sal_uInt16 nFileId ) const;

    ScExternalRefCache::TokenRef getSingleRefTokenFromSrcDoc(
        sal_uInt16 nFileId, ScDocument* pSrcDoc, const ScAddress& rPos,
        ScExternalRefCache::CellFormat* pFmt);

    /**
     * Retrieve a range token array from a source document instance.
     *
     * @param pSrcDoc pointer to the source document instance.
     * @param rTabName name of the first table.
     * @param rRange range specified.  Upon successful retrieval, this range
     *               gets modified to contain the correct table IDs, and in
     *               case the range is larger than the data area of the source
     *               document, it gets reduced to the data area.
     * @param rCacheData an array of structs, with each struct containing the
     *                   table name and the data in the specified range.
     *
     * @return range token array
     */
    ScExternalRefCache::TokenArrayRef getDoubleRefTokensFromSrcDoc(
        const ScDocument* pSrcDoc, const OUString& rTabName, ScRange& rRange,
        ::std::vector<ScExternalRefCache::SingleRangeData>& rCacheData);

    /**
     * Retrieve range name token array from a source document instance.
     *
     * @param nFileId file ID of the source document.
     * @param pSrcDoc pointer to the source document instance
     * @param rName range name to retrieve.  Note that the range name lookup
     *              is case <i>in</i>-sensitive, and upon successful retrieval
     *              of the range name array, this name gets updated to the
     *              actual range name with the correct casing.
     *
     * @return range name token array
     */
    static ScExternalRefCache::TokenArrayRef getRangeNameTokensFromSrcDoc(
        sal_uInt16 nFileId, const ScDocument* pSrcDoc, OUString& rName);

    ScDocument* getInMemorySrcDocument(sal_uInt16 nFileId);
    ScDocument* getSrcDocument(sal_uInt16 nFileId);
    SfxObjectShellRef loadSrcDocument(sal_uInt16 nFileId, OUString& rFilter);

    /**
     * Caller must ensure that the passed shell is not already stored.
     */
    ScDocument& cacheNewDocShell( sal_uInt16 nFileId, SrcShell& rSrcShell );

    void maybeLinkExternalFile( sal_uInt16 nFileId, bool bDeferFilterDetection = false );

    /**
     * Try to create a "real" file name from the relative path.  The original
     * file name may not point to the real document when the referencing and
     * referenced documents have been moved.
     *
     * For the real file name to be created, the relative name should not be
     * empty before calling this method, or the real file name will not be
     * created.
     *
     * @param nFileId file ID for an external document
     */
    void maybeCreateRealFileName(sal_uInt16 nFileId);

    /**
     * Purge those source document instances that have not been accessed for
     * the specified duration.
     *
     * @param nTimeOut time out value in 100th of a second
     */
    void purgeStaleSrcDocument(sal_Int32 nTimeOut);

    sal_uInt32 getMappedNumberFormat(sal_uInt16 nFileId, sal_uInt32 nNumFmt, const ScDocument* pSrcDoc);

    /**
     * If in maUnsavedDocShells move it to maDocShells and create a correct
     * external reference entry
     *
     * @param Pointer to the newly saved DocumentShell
     */
    void transformUnsavedRefToSavedRef( SfxObjectShell* pShell );

private:
    ScDocument* mpDoc;

    /** cache of referenced ranges and names from source documents. */
    ScExternalRefCache maRefCache;

    /**
     * Source document cache.  This stores the original source document shell
     * instances.  They get purged after a certain period of time.
     */
    DocShellMap maDocShells;

    /**
     * DocShells to unsaved but referenced documents. If not empty ask before saving!
     * Move to maDocShells if document referenced here is saved
     */
    DocShellMap maUnsavedDocShells;

    /** list of source documents that are managed by the link manager. */
    LinkedDocMap maLinkedDocs;

    /**
     * List of referencing cells that may contain external names.  There is
     * one list per source document.
     */
    RefCellMap maRefCells;

    LinkListenerMap maLinkListeners;

    NumFmtMap maNumFormatMap;

    /**
     * List of external source document meta-data, used to keep track of
     * external document identifiers.
     */
    std::vector<SrcFileData> maSrcFiles;

    /** Status whether in reference marking state. See isInReferenceMarking(). */
    bool mbInReferenceMarking:1;

    /**
     * Controls whether or not to allow user interaction.  We don't want any
     * user interaction when calling from the API.
     */
    bool mbUserInteractionEnabled:1;

    bool mbDocTimerEnabled:1;

    AutoTimer maSrcDocTimer;
    DECL_LINK(TimeOutHdl, Timer*, void);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

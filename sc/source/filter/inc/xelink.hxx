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

#ifndef SC_XELINK_HXX
#define SC_XELINK_HXX

#include "markdata.hxx"
#include "xllink.hxx"
#include "xerecord.hxx"
#include "xehelper.hxx"
#include "xeformula.hxx"
#include "externalrefmgr.hxx"
#include <boost/shared_ptr.hpp>

struct ScSingleRefData;
struct ScComplexRefData;

/* ============================================================================
Classes for export of different kinds of internal/external references.
- 3D cell and cell range links
- External cell and cell range links
- External defined names
- Macro calls
- Add-in functions
- DDE links
- OLE object links
============================================================================ */

// Excel sheet indexes ========================================================

/** Stores the correct Excel sheet index for each Calc sheet.
    @descr  The class knows all sheets which will not exported
    (i.e. external link sheets, scenario sheets). */
class XclExpTabInfo : protected XclExpRoot
{
public:
    /** Initializes the complete buffer from the current exported document. */
    explicit            XclExpTabInfo( const XclExpRoot& rRoot );

    /** Returns true, if the specified Calc sheet will be exported. */
    bool                IsExportTab( SCTAB nScTab ) const;
    /** Returns true, if the specified Calc sheet is used to store external cell contents. */
    bool                IsExternalTab( SCTAB nScTab ) const;
    /** Returns true, if the specified Calc sheet is visible and will be exported. */
    bool                IsVisibleTab( SCTAB nScTab ) const;
    /** Returns true, if the specified Calc sheet is selected and will be exported. */
    bool                IsSelectedTab( SCTAB nScTab ) const;
    /** Returns true, if the specified Calc sheet is the displayed (active) sheet. */
    bool                IsDisplayedTab( SCTAB nScTab ) const;
    /** Returns true, if the specified Calc sheet is displayed in right-to-left mode. */
    bool                IsMirroredTab( SCTAB nScTab ) const;
    /** Returns the Calc name of the specified sheet. */
    OUString       GetScTabName( SCTAB nScTab ) const;

    /** Returns the Excel sheet index for a given Calc sheet. */
    sal_uInt16          GetXclTab( SCTAB nScTab ) const;

    /** Returns the Calc sheet index of the nSortedTab-th entry in the sorted sheet names list. */
    SCTAB               GetRealScTab( SCTAB nSortedScTab ) const;

    /** Returns the number of Calc sheets. */
    inline SCTAB        GetScTabCount() const { return mnScCnt; }

    /** Returns the number of Excel sheets to be exported. */
    inline sal_uInt16   GetXclTabCount() const { return mnXclCnt; }
    /** Returns the number of external linked sheets. */
    inline sal_uInt16   GetXclExtTabCount() const { return mnXclExtCnt; }
    /** Returns the number of exported selected sheets. */
    inline sal_uInt16   GetXclSelectedCount() const { return mnXclSelCnt; }

    /** Returns the Excel index of the active, displayed sheet. */
    inline sal_uInt16   GetDisplayedXclTab() const { return mnDisplXclTab; }
    /** Returns the Excel index of the first visible sheet. */
    inline sal_uInt16   GetFirstVisXclTab() const { return mnFirstVisXclTab; }

private:
    /** Returns true, if any of the passed flags is set for the specified Calc sheet. */
    bool                GetFlag( SCTAB nScTab, sal_uInt8 nFlags ) const;
    /** Sets or clears (depending on bSet) all passed flags for the specified Calc sheet. */
    void                SetFlag( SCTAB nScTab, sal_uInt8 nFlags, bool bSet = true );

    /** Searches for sheets not to be exported. */
    void                CalcXclIndexes();
    /** Sorts the names of all tables and stores the indexes of the sorted indexes. */
    void                CalcSortedIndexes();

private:
    /** Data structure with infoemation about one Calc sheet. */
    struct XclExpTabInfoEntry
    {
        OUString       maScName;
        sal_uInt16          mnXclTab;
        sal_uInt8           mnFlags;
        inline explicit     XclExpTabInfoEntry() : mnXclTab( 0 ), mnFlags( 0 ) {}
    };

    typedef ::std::vector< XclExpTabInfoEntry > XclExpTabInfoVec;
    typedef ::std::vector< SCTAB >              ScTabVec;

    XclExpTabInfoVec    maTabInfoVec;       /// Array of Calc sheet index information.

    SCTAB               mnScCnt;            /// Count of Calc sheets.
    sal_uInt16          mnXclCnt;           /// Count of Excel sheets to be exported.
    sal_uInt16          mnXclExtCnt;        /// Count of external link sheets.
    sal_uInt16          mnXclSelCnt;        /// Count of selected and exported sheets.
    sal_uInt16          mnDisplXclTab;      /// Displayed (active) sheet.
    sal_uInt16          mnFirstVisXclTab;   /// First visible sheet.

    ScTabVec            maFromSortedVec;    /// Sorted Calc sheet index -> real Calc sheet index.
    ScTabVec            maToSortedVec;      /// Real Calc sheet index -> sorted Calc sheet index.
};

// Export link manager ========================================================

class XclExpLinkManagerImpl;

/** Stores all data for internal/external references (the link table). */
class XclExpLinkManager : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit            XclExpLinkManager( const XclExpRoot& rRoot );
    virtual             ~XclExpLinkManager();

    /** Searches for an EXTERNSHEET index for the given Calc sheet.
        @descr  See above for the meaning of EXTERNSHEET indexes.
        @param rnExtSheet  (out-param)  Returns the EXTERNSHEET index.
        @param rnXclTab  (out-param)  Returns the Excel sheet index.
        @param nScTab  The Calc sheet index to process.
        param pRefLogEntry  If not 0, data about the external link is stored here. */
    void                FindExtSheet( sal_uInt16& rnExtSheet,
                            sal_uInt16& rnXclTab, SCTAB nScTab,
                            XclExpRefLogEntry* pRefLogEntry = 0 );
    /** Searches for an EXTERNSHEET index for the given Calc sheet range.
        @descr  See above for the meaning of EXTERNSHEET indexes.
        @param rnExtSheet  (out-param)  Returns the EXTERNSHEET index.
        @param rnFirstXclTab  (out-param)  Returns the Excel sheet index of the first sheet.
        @param rnXclTab  (out-param)  Returns the Excel sheet index of the last sheet.
        @param nFirstScTab  The first Calc sheet index to process.
        @param nLastScTab  The last Calc sheet index to process.
        param pRefLogEntry  If not 0, data about the external link is stored here. */
    void                FindExtSheet( sal_uInt16& rnExtSheet,
                            sal_uInt16& rnFirstXclTab, sal_uInt16& rnLastXclTab,
                            SCTAB nFirstScTab, SCTAB nLastScTab,
                            XclExpRefLogEntry* pRefLogEntry = 0 );
    /** Searches for a special EXTERNSHEET index for the own document. */
    sal_uInt16          FindExtSheet( sal_Unicode cCode );

    void                FindExtSheet( sal_uInt16 nFileId, const String& rTabName, sal_uInt16 nXclTabSpan,
                                      sal_uInt16& rnExtSheet, sal_uInt16& rnFirstSBTab, sal_uInt16& rnLastSBTab,
                                      XclExpRefLogEntry* pRefLogEntry = NULL );

    /** Stores the cell with the given address in a CRN record list. */
    void                StoreCell( const ScSingleRefData& rRef );
    /** Stores all cells in the given range in a CRN record list. */
    void                StoreCellRange( const ScComplexRefData& rRef );

    void StoreCell( sal_uInt16 nFileId, const String& rTabName, const ScAddress& rPos );

    void StoreCellRange( sal_uInt16 nFileId, const String& rTabName, const ScRange& rRange );

    /** Finds or inserts an EXTERNNAME record for an add-in function name.
        @param rnExtSheet  (out-param) Returns the index of the EXTSHEET structure for the add-in function name.
        @param rnExtName  (out-param) Returns the 1-based EXTERNNAME record index.
        @return  true = add-in function inserted; false = error (i.e. not supported in current BIFF). */
    bool                InsertAddIn(
                            sal_uInt16& rnExtSheet, sal_uInt16& rnExtName,
                            const String& rName );
    /** InsertEuroTool */
    bool                InsertEuroTool(
                            sal_uInt16& rnExtSheet, sal_uInt16& rnExtName,
                            const String& rName );
    /** Finds or inserts an EXTERNNAME record for DDE links.
        @param rnExtSheet  (out-param) Returns the index of the EXTSHEET structure for the DDE link.
        @param rnExtName  (out-param) Returns the 1-based EXTERNNAME record index.
        @return  true = DDE link inserted; false = error (i.e. not supported in current BIFF). */
    bool                InsertDde(
                            sal_uInt16& rnExtSheet, sal_uInt16& rnExtName,
                            const String& rApplic, const String& rTopic, const String& rItem );

    bool                InsertExtName(
                            sal_uInt16& rnExtSheet, sal_uInt16& rnExtName, const String& rUrl,
                            const String& rName, const ScExternalRefCache::TokenArrayRef pArray );

    /** Writes the entire Link table. */
    virtual void        Save( XclExpStream& rStrm );

private:
    typedef boost::shared_ptr< XclExpLinkManagerImpl > XclExpLinkMgrImplPtr;
    XclExpLinkMgrImplPtr mxImpl;
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

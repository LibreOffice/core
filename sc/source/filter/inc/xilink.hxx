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

#ifndef SC_XILINK_HXX
#define SC_XILINK_HXX

#include <map>
#include "xllink.hxx"
#include "xiroot.hxx"
#include "types.hxx"

/* ============================================================================
Classes for import of different kinds of internal/external references.
- 3D cell and cell range links
- External cell and cell range links
- External defined names
- Add-in functions
- DDE links
- OLE object links
============================================================================ */

// Excel sheet indexes ========================================================

/** A buffer containing information about names and creation order of sheets.

    The first purpose of this buffer is to translate original Excel
    sheet names into Calc sheet indexes. This is not trivial because the filter
    may rename the Calc sheets during creation. This buffer stores the original
    Excel sheet names with the corresponding Calc sheet indexes.

    The second purpose is to store the creation order of all sheets inside the
    Excel workbook. The creation order list is contained in the TABID record
    and needed to import the change log. Example: If the list contains 3;1;2
    this means that the second sheet in the file was created first, than the
    third sheet in the file was created and finally the first sheet.
 */
class XclImpTabInfo
{
public:
    // original Excel sheet names ---------------------------------------------

    /** Appends an original Excel sheet name with corresponding Calc sheet index. */
    void                AppendXclTabName( const String& rXclTabName, SCTAB nScTab );
    /** Inserts a Calc sheet index (increases all following sheet indexes). */
    void                InsertScTab( SCTAB nScTab );

    /** Returns the Calc sheet index from the passed original Excel sheet name. */
    SCTAB               GetScTabFromXclName( const String& rXclTabName ) const;

    // record creation order - TABID record -----------------------------------

    /** Reads the TABID record. */
    void                ReadTabid( XclImpStream& rStrm );

    /** Returns the current sheet index calculated from creation index.
        @param nCreatedId  The creation index of the sheet (1-based).
        @param nMaxTabId  All values greater than this parameter are not used to find the index.
        @return  The 0-based index of the sheet nCreatedId if it is contained in the list.
        Example: The buffer is 3;5;2;4;1, nCreatedId is 1 and nMaxTabId is 3. The function will
        return 2 which is the 0-based index of sheet 1 in the list 3;2;1. */
    sal_uInt16          GetCurrentIndex( sal_uInt16 nCreatedId, sal_uInt16 nMaxTabId = 0xFFFF ) const;

private:
    typedef ::std::map< String, SCTAB > XclTabNameMap;

    XclTabNameMap       maTabNames;     /// All Excel sheet names with Calc sheet index.
    ScfUInt16Vec        maTabIdVec;     /// The vector with sheet indexes.
};

// External names =============================================================

/** Type of an external name. */
enum XclImpExtNameType
{
    xlExtName,                  /// An external defined name.
    xlExtAddIn,                 /// An add-in function name.
    xlExtDDE,                   /// A DDE link range.
    xlExtOLE,                   /// An OLE object link.
    xlExtEuroConvert            /// An external in Excel, but internal in OO function name.
};

// ----------------------------------------------------------------------------

class XclImpCachedMatrix;
class ScTokenArray;
class XclImpSupbook;

/** Stores contents of an external name.
    @descr Supported: External defined names, AddIn names, DDE links and OLE objects. */
class XclImpExtName
{
    /**
     * MOper, multiple operands, stores cached values of external range
     * specified in the record.
     */
    class MOper
    {
    public:
        MOper(XclImpStream& rStrm);
        const ScMatrix& GetCache() const;
    private:
        ScMatrixRef mxCached;
    };

public:
    /** Reads the external name from the stream. */
    explicit            XclImpExtName( const XclImpSupbook& rSupbook, XclImpStream& rStrm,
                                        XclSupbookType eSubType, ExcelToSc* pFormulaConv );
                        ~XclImpExtName();

    /** Create and apply the cached list of this DDE Link to the document. */
    void                CreateDdeData( ScDocument& rDoc,
                            const String& rApplc, const String& rExtDoc ) const;

    void                CreateExtNameData( ScDocument& rDoc, sal_uInt16 nFileId ) const;

    /**
     * Create OLE link data.  OLE link data is converted to external
     * reference, since OLE link doesn't work cross-platform, and is not very
     * reliable even on Windows.
     */
    bool CreateOleData(ScDocument& rDoc, const OUString& rUrl,
                       sal_uInt16& rFileId, OUString& rTabName, ScRange& rRange) const;

    bool                HasFormulaTokens() const;

    XclImpExtNameType GetType() const { return meType; }
    const OUString& GetName() const { return maName; }
    sal_uInt32   GetStorageId() const { return mnStorageId; }

private:
    typedef ::std::auto_ptr< XclImpCachedMatrix > XclImpCachedMatrixPtr;
    typedef ::std::auto_ptr< ScTokenArray >       TokenArrayPtr;

    XclImpCachedMatrixPtr mxDdeMatrix;      /// Cached results of the DDE link.
    MOper*              mpMOper;            /// Cached values for OLE link
    TokenArrayPtr       mxArray;            /// Formula tokens for external name.
    OUString            maName;             /// The name of the external name.
    sal_uInt32          mnStorageId;        /// Storage ID for OLE object storages.
    XclImpExtNameType   meType;             /// Type of the external name.
};

// Import link manager ========================================================

class XclImpLinkManagerImpl;

/** This is the central class for the import of all internal/external links.
    @descr  This manager stores all data about external documents with their sheets
    and cached cell contents. Additionally it handles external names, such as add-in
    function names, DDE links, and OLE object links.
    File contents in BIFF8:
    - Record SUPBOOK: Contains the name of an external document and the names of its sheets.
    This record is optionally followed by NAME, EXTERNNAME, XCT and CRN records.
    - Record XCT: Contains the number and sheet index of the following CRN records.
    - Record CRN: Contains addresses (row and column) and values of external referenced cells.
    - Record NAME: Contains defined names of the own workbook.
    - Record EXTERNNAME: Contains external defined names, DDE links, or OLE object links.
    - Record EXTERNSHEET: Contains indexes to URLs of external documents (SUPBOOKs)
    and sheet indexes for each external reference used anywhere in the workbook.
    This record follows a list of SUPBOOK records (with their attached records).
*/
class XclImpLinkManager : protected XclImpRoot
{
public:
    explicit            XclImpLinkManager( const XclImpRoot& rRoot );
                        ~XclImpLinkManager();

    /** Reads the EXTERNSHEET record. */
    void                ReadExternsheet( XclImpStream& rStrm );
    /** Reads a SUPBOOK record. */
    void                ReadSupbook( XclImpStream& rStrm );
    /** Reads an XCT record and appends it to the current SUPBOOK. */
    void                ReadXct( XclImpStream& rStrm );
    /** Reads a CRN record and appends it to the current SUPBOOK. */
    void                ReadCrn( XclImpStream& rStrm );
    /** Reads an EXTERNNAME record and appends it to the current SUPBOOK. */
    void                ReadExternname( XclImpStream& rStrm, ExcelToSc* pFormulaConv = NULL );

    /** Returns true, if the specified XTI entry contains an internal reference. */
    bool                IsSelfRef( sal_uInt16 nXtiIndex ) const;
    /** Returns the Calc sheet index range of the specified XTI entry.
        @return  true = XTI data found, returned sheet index range is valid. */
    bool                GetScTabRange(
                            SCTAB& rnFirstScTab, SCTAB& rnLastScTab,
                            sal_uInt16 nXtiIndex ) const;
    /** Returns the specified external name or 0 on error. */
    const XclImpExtName* GetExternName( sal_uInt16 nXtiIndex, sal_uInt16 nExtName ) const;

    const OUString* GetSupbookUrl( sal_uInt16 nXtiIndex ) const;

    const String& GetSupbookTabName( sal_uInt16 nXti, sal_uInt16 nXtiTab ) const;

    /** Tries to decode the URL of the specified XTI entry to OLE or DDE link components.
        @descr  For DDE links: Decodes to application name and topic.
        For OLE object links: Decodes to class name and document URL.
        @return  true = decoding was successful, returned strings are valid (not empty). */
    bool                GetLinkData( String& rApplic, String& rTopic, sal_uInt16 nXtiIndex ) const;
    /** Returns the specified macro name or an empty string on error. */
    const String&       GetMacroName( sal_uInt16 nExtSheet, sal_uInt16 nExtName ) const;

private:
    typedef ::std::auto_ptr< XclImpLinkManagerImpl > XclImpLinkMgrImplPtr;
    XclImpLinkMgrImplPtr mxImpl;
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

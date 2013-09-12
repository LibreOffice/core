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

#ifndef SC_XICONTENT_HXX
#define SC_XICONTENT_HXX

#include "global.hxx"
#include "rangelst.hxx"
#include "xlcontent.hxx"
#include "xistring.hxx"
#include "xiroot.hxx"
#include "validat.hxx"

#include <map>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/noncopyable.hpp>

/* ============================================================================
Classes to import the big Excel document contents (related to several cells or
globals for the document).
- Shared string tables
- Hyperlinks
- Label ranges
- Conditional formatting
- Data validation
- Web queries
- Stream decryption
============================================================================ */

// Shared string table ========================================================

/** The SST (shared string table) contains all strings used in a BIFF8 file.

    This class loads the SST, provides access to the strings, and is able to
    create Calc string or edit cells.
 */
class XclImpSst : protected XclImpRoot
{
public:
    explicit            XclImpSst( const XclImpRoot& rRoot );

    /** Reads the entire SST record.
        @descr  Import stream must be located at start of a SST record. */
    void                ReadSst( XclImpStream& rStrm );

    /** Returns a pointer to the string with the passed index. */
    const XclImpString* GetString( sal_uInt32 nSstIndex ) const;

private:
    typedef ::std::vector< XclImpString > XclImpStringVec;
    XclImpStringVec     maStrings;          /// List with all strings in the SST.
};

// Hyperlinks =================================================================

/** Provides importing hyperlinks and inserting them into a document. */
class XclImpHyperlink : private boost::noncopyable
{
public:
    /** Reads a HLINK record and inserts it into the document.
        @descr  Import stream must be located at start of a HLINK record. */
    static void         ReadHlink( XclImpStream& rStrm );

    /** Reads the (undocumented) embedded hyperlink data and returns the URL. */
    static OUString     ReadEmbeddedData( XclImpStream& rStrm );

    /** Inserts the URL into a range of cells. Does not modify value or formula cells. */
    static void InsertUrl( XclImpRoot& rRoot, const XclRange& rXclRange, const String& rUrl );

    /** Convert the sheet name with invalid character(s) in URL when the URL is
        to a location within the same document (e.g. #'Sheet&Name'.A1). */
    static void         ConvertToValidTabName(String& rName);

private:
    /** We don't want anybody to instantiate this class, since it is just a
        collection of static methods. To enforce this, the default constructor
        is made private */
    XclImpHyperlink();
};

// Label ranges ===============================================================

/** Provides importing label ranges and inserting them into a document. */
class XclImpLabelranges : private  boost::noncopyable
{
public:
    /** Reads a LABELRANGES record and inserts the label ranges into the document.
        @descr  Import stream must be located at start of a LABELRANGES record. */
    static void         ReadLabelranges( XclImpStream& rStrm );

private:
    /** We don't want anybody to instantiate this class, since it is just a
        collection of static methods. To enforce this, the default constructor
        is made private */
    XclImpLabelranges();
};

// Conditional formatting =====================================================

class ScConditionalFormat;

/** Represents a conditional format with condition formulas, and formatting attributes. */
class XclImpCondFormat : protected XclImpRoot
{
public:
    explicit            XclImpCondFormat( const XclImpRoot& rRoot, sal_uInt32 nFormatIndex );
    virtual             ~XclImpCondFormat();

    /** Reads a CONDFMT record and initializes this conditional format. */
    void                ReadCondfmt( XclImpStream& rStrm );
    /** Reads a CF record and adds a new condition and the formatting attributes. */
    void                ReadCF( XclImpStream& rStrm );

    /** Inserts this conditional format into the document. */
    void                Apply();

private:
    typedef ::std::auto_ptr< ScConditionalFormat > ScCondFmtPtr;

    ScRangeList         maRanges;           /// Destination cell ranges.
    ScCondFmtPtr        mxScCondFmt;        /// Calc conditional format.
    sal_uInt32          mnFormatIndex;      /// Index of this conditional format in list.
    sal_uInt16          mnCondCount;        /// Number of conditions to be inserted.
    sal_uInt16          mnCondIndex;        /// Condition index to be inserted next.
};

// ----------------------------------------------------------------------------

/** Imports and collects all conditional formatting of a sheet. */
class XclImpCondFormatManager : protected XclImpRoot
{
public:
    explicit            XclImpCondFormatManager( const XclImpRoot& rRoot );

    /** Reads a CONDFMT record and starts a new conditional format to be filled from CF records. */
    void                ReadCondfmt( XclImpStream& rStrm );
    /** Reads a CF record and inserts the formatting data to the current conditional format. */
    void                ReadCF( XclImpStream& rStrm );

    /** Inserts the conditional formatting into the document. */
    void                Apply();

private:
    typedef boost::ptr_vector< XclImpCondFormat > XclImpCondFmtList;
    XclImpCondFmtList   maCondFmtList;      /// List with all conditional formatting.
};

// Data Validation ============================================================

/** Imports validation data. */
class XclImpValidationManager : protected XclImpRoot
{
public:
    explicit            XclImpValidationManager( const XclImpRoot& rRoot );

    /** Reads a DVAL record and sets marks the dropdown arrow control to be ignored. */
    void                ReadDval( XclImpStream& rStrm );
    /** Reads a DV record and inserts validation data into the document. */
    void                ReadDV( XclImpStream& rStrm );

    void                Apply();
private:
    struct DVItem
    {
        ScRangeList         maRanges;
        ScValidationData    maValidData;

        explicit DVItem ( const ScRangeList& rRanges, const ScValidationData& rValidData );
    };
    typedef ::boost::ptr_vector<DVItem> DVItemList;

    DVItemList maDVItems;
};

// Web queries ================================================================

/** Stores the data of one web query. */
class XclImpWebQuery : private boost::noncopyable
{
public:
    explicit            XclImpWebQuery( const ScRange& rDestRange );

    /** Reads a PARAMQRY record and sets data to the web query. */
    void                ReadParamqry( XclImpStream& rStrm );
    /** Reads a WQSTRING record and sets URL. */
    void                ReadWqstring( XclImpStream& rStrm );
    /** Reads a WEBQRYSETTINGS record and sets refresh rate. */
    void                ReadWqsettings( XclImpStream& rStrm );
    /** Reads a WEBQRYTABLES record and sets source range list. */
    void                ReadWqtables( XclImpStream& rStrm );

    /** Inserts the web query into the document. */
    void                Apply( ScDocument& rDoc, const OUString& rFilterName );

private:
    /** Specifies the type of the web query (which ranges are imported). */
    enum XclImpWebQueryMode
    {
        xlWQUnknown,                /// Not specified.
        xlWQDocument,               /// Entire document.
        xlWQAllTables,              /// All tables.
        xlWQSpecTables              /// Specific tables.
    };

    String              maURL;          /// Source document URL.
    OUString            maTables;       /// List of source range names.
    ScRange             maDestRange;    /// Destination range.
    XclImpWebQueryMode  meMode;         /// Current mode of the web query.
    sal_uInt16          mnRefresh;      /// Refresh time in minutes.
};

// ----------------------------------------------------------------------------

class XclImpWebQueryBuffer : protected XclImpRoot
{
public:
    explicit            XclImpWebQueryBuffer( const XclImpRoot& rRoot );

    /** Reads the QSI record and creates a new web query in the buffer. */
    void                ReadQsi( XclImpStream& rStrm );
    /** Reads a PARAMQRY record and sets data to the current web query. */
    void                ReadParamqry( XclImpStream& rStrm );
    /** Reads a WQSTRING record and sets URL to the current web query. */
    void                ReadWqstring( XclImpStream& rStrm );
    /** Reads a WEBQRYSETTINGS record and sets refresh rate to the current web query. */
    void                ReadWqsettings( XclImpStream& rStrm );
    /** Reads a WEBQRYTABLES record and sets source range list to the current web query. */
    void                ReadWqtables( XclImpStream& rStrm );

    /** Inserts all web queries into the document. */
    void                Apply();

private:
    typedef boost::ptr_vector< XclImpWebQuery > XclImpWebQueryList;
    XclImpWebQueryList  maWQList;       /// List of the web query objects.
};

// Decryption =================================================================

/** Provides static functions to import stream decryption settings. */
class XclImpDecryptHelper : private boost::noncopyable
{
public:
    /** Reads the FILEPASS record, queries a password and sets decryption algorihm.
        @return  Error code that may cause an error message after import. */
    static ErrCode      ReadFilepass( XclImpStream& rStrm );

private:
    /** We don't want anybody to instantiate this class, since it is just a
        collection of static methods. To enforce this, the default constructor
        is made private */
    XclImpDecryptHelper();
};

// ============================================================================

// Document protection ========================================================

class XclImpDocProtectBuffer : protected XclImpRoot
{
public:
    explicit            XclImpDocProtectBuffer( const XclImpRoot& rRoot );

    /** document structure protection flag  */
    void                ReadDocProtect( XclImpStream& rStrm );

    /** document windows properties protection flag */
    void                ReadWinProtect( XclImpStream& rStrm );

    void                ReadPasswordHash( XclImpStream& rStrm );

    void                Apply() const;

private:
    sal_uInt16      mnPassHash;
    bool            mbDocProtect:1;
    bool            mbWinProtect:1;
};

// Sheet protection ===========================================================

class XclImpSheetProtectBuffer : protected XclImpRoot
{
public:
    explicit            XclImpSheetProtectBuffer( const XclImpRoot& rRoot );

    void                ReadProtect( XclImpStream& rStrm, SCTAB nTab );

    void                ReadOptions( XclImpStream& rStrm, SCTAB nTab );

    void                ReadPasswordHash( XclImpStream& rStrm, SCTAB nTab );

    void                Apply() const;

private:
    struct Sheet
    {
        bool        mbProtected;
        sal_uInt16  mnPasswordHash;
        sal_uInt16  mnOptions;

        Sheet();
        Sheet(const Sheet& r);
    };

    Sheet* GetSheetItem( SCTAB nTab );

private:
    typedef ::std::map<SCTAB, Sheet> ProtectedSheetMap;
    ProtectedSheetMap   maProtectedSheets;
};

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

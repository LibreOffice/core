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

#include <memory>
#include <rangelst.hxx>
#include "xladdress.hxx"
#include "xerecord.hxx"
#include "xeroot.hxx"
#include "xestring.hxx"
#include "xeextlst.hxx"
#include "xlformula.hxx"

#include <colorscale.hxx>

/* ============================================================================
Classes to export the big Excel document contents (related to several cells or
globals for the sheet or document).
- Shared string table
- Merged cells
- Hyperlinks
- Label ranges
- Conditional formatting
- Data validation
- Web Queries
============================================================================ */

// Shared string table ========================================================

class XclExpSstImpl;

/** Provides export of the SST (shared string table) record.
    @descr  Contains all strings in the document and writes the SST. */
class XclExpSst : public XclExpRecordBase
{
public:
    explicit            XclExpSst();
    virtual             ~XclExpSst() override;

    /** Inserts a new string into the table.
        @return  The index of the string in the SST, used in other records. */
    sal_uInt32          Insert( const XclExpStringRef& xString );

    /** Writes the complete SST and EXTSST records. */
    virtual void        Save( XclExpStream& rStrm ) override;
    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

private:
    typedef std::unique_ptr< XclExpSstImpl > XclExpSstImplPtr;
    XclExpSstImplPtr    mxImpl;
};

// Merged cells ===============================================================

/** Represents a MERGEDCELLS record containing all merged cell ranges in a sheet. */
class XclExpMergedcells : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit            XclExpMergedcells( const XclExpRoot& rRoot );

    /** Appends a new range to the list of merged cell ranges. */
    void                AppendRange( const ScRange& rRange, sal_uInt32 nBaseXFId );
    /** Returns the XF identifier of the top-left cell in a merged range. */
    sal_uInt32          GetBaseXFId( const ScAddress& rPos ) const;

    /** Writes the record, if it contains at least one merged cell range. */
    virtual void        Save( XclExpStream& rStrm ) override;
    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

private:
    ScRangeList         maMergedRanges;     /// All merged cell ranges of the sheet.
    ScfUInt32Vec        maBaseXFIds;        /// The XF identifiers of the top-left cells.
};

// Hyperlinks =================================================================

class SvxURLField;

/** Provides export of hyperlink data. */
class XclExpHyperlink : public XclExpRecord
{
public:
    /** Constructs the HLINK record from a URL text field. */
    explicit            XclExpHyperlink( const XclExpRoot& rRoot,
                            const SvxURLField& rUrlField, const ScAddress& rScPos );
    virtual             ~XclExpHyperlink() override;

    /** Returns the cell representation text or 0, if not available. */
    const OUString* GetRepr() const { return m_Repr.isEmpty() ? nullptr : &m_Repr; }

    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

    void                WriteEmbeddedData( XclExpStream& rStrm );

    /** Builds file name from the passed file URL. Tries to convert to relative file name.
        @param rnLevel  (out-param) The parent directory level.
        @param rbRel  (out-param) true = path is relative.
        @param bEncoded if true return an IURI encoded name, not a DOS name. */
    static OUString     BuildFileName(
                            sal_uInt16& rnLevel, bool& rbRel,
                            const OUString& rUrl, const XclExpRoot& rRoot, bool bEncoded );
private:

    /** Writes the body of the HLINK record. */
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    typedef std::unique_ptr< SvStream > SvStreamPtr;

    ScAddress           maScPos;            /// Position of the hyperlink.
    OUString            m_Repr;             /// Cell representation text.
    SvStreamPtr         mxVarData;          /// Buffer stream with variable data.
    sal_uInt32          mnFlags;            /// Option flags.
    XclExpStringRef     mxTextMark;         /// Location within m_Repr
    OUString            msTarget;           /// Target URL
};

typedef XclExpRecordList< XclExpHyperlink > XclExpHyperlinkList;

// Label ranges ===============================================================

/** Provides export of the row/column label range list of a sheet. */
class XclExpLabelranges : public XclExpRecordBase, protected XclExpRoot
{
public:
    /** Fills the cell range lists with all ranges of the current sheet. */
    explicit            XclExpLabelranges( const XclExpRoot& rRoot );

    /** Writes the LABELRANGES record if it contains at least one range. */
    virtual void        Save( XclExpStream& rStrm ) override;

private:
    /** Fills the specified range list with all label headers of the current sheet.
        @param rRanges  The cell range list to fill.
        @param xLabelRangesRef  The core range list with all ranges.
        @param nScTab  The current Calc sheet index. */
    static void         FillRangeList( ScRangeList& rScRanges,
                            const ScRangePairListRef& xLabelRangesRef, SCTAB nScTab );

private:
    ScRangeList         maRowRanges;    /// Cell range list for row labels.
    ScRangeList         maColRanges;    /// Cell range list for column labels.
};

// Conditional formatting =====================================================

class XclExpCFImpl;

/** Represents a CF record that contains one condition of a conditional format. */
class XclExpCF : public XclExpRecord, protected XclExpRoot
{
public:
    explicit            XclExpCF( const XclExpRoot& rRoot, const ScCondFormatEntry& rFormatEntry, sal_Int32 nPriority, ScAddress aOrigin );
    virtual             ~XclExpCF() override;

    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

private:
    /** Writes the body of the CF record. */
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    typedef std::unique_ptr< XclExpCFImpl > XclExpCFImplPtr;
    XclExpCFImplPtr     mxImpl;
};

class XclExpDateFormat : public XclExpRecord, protected XclExpRoot
{
public:
    explicit XclExpDateFormat( const XclExpRoot& rRoot, const ScCondDateFormatEntry& rFormatEntry, sal_Int32 nPriority );
    virtual ~XclExpDateFormat() override;

    virtual void SaveXml( XclExpXmlStream& rStrm ) override;

private:
    const ScCondDateFormatEntry& mrFormatEntry;
    sal_Int32 mnPriority;
};

class XclExpCfvo : public XclExpRecord, protected XclExpRoot
{
public:
    explicit XclExpCfvo( const XclExpRoot& rRoot, const ScColorScaleEntry& rFormatEntry, const ScAddress& rPos, bool bFirst = true);

    virtual void SaveXml( XclExpXmlStream& rStrm ) override;
private:
    const ScColorScaleEntry& mrEntry;
    ScAddress maSrcPos;
    bool mbFirst;
};

class XclExpColScaleCol : public XclExpRecord, protected XclExpRoot
{
public:
    explicit XclExpColScaleCol( const XclExpRoot& rRoot, const Color& rColor);
    virtual ~XclExpColScaleCol() override;

    virtual void SaveXml( XclExpXmlStream& rStrm ) override;
private:
    const Color& mrColor;
};

/** Represents a CONDFMT record that contains all conditions of a conditional format.
    @descr  Contains the conditions which are stored in CF records. */
class XclExpCondfmt : public XclExpRecord, protected XclExpRoot
{
public:
    explicit            XclExpCondfmt( const XclExpRoot& rRoot, const ScConditionalFormat& rCondFormat, const XclExtLstRef& xExtLst, sal_Int32& rIndex );
    virtual             ~XclExpCondfmt() override;

    /** Returns true, if this conditional format contains at least one cell range and CF record. */
    bool                IsValidForBinary() const;
    bool                IsValidForXml() const;

    /** Writes the CONDFMT record with following CF records, if there is valid data. */
    virtual void        Save( XclExpStream& rStrm ) override;
    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

private:
    /** Writes the body of the CONDFMT record. */
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    XclExpRecordList< XclExpRecord >
                        maCFList;       /// List of CF records.
    XclRangeList        maXclRanges;    /// Cell ranges for this conditional format.
    OUString            msSeqRef;       /// OOXML Sequence of References
};

class XclExpColorScale: public XclExpRecord, protected XclExpRoot
{
public:
    explicit XclExpColorScale( const XclExpRoot& rRoot, const ScColorScaleFormat& rFormat, sal_Int32 nPriority );

    virtual void SaveXml( XclExpXmlStream& rStrm ) override;
private:
    typedef XclExpRecordList< XclExpCfvo > XclExpCfvoList;
    typedef XclExpRecordList< XclExpColScaleCol > XclExpColScaleColList;

    XclExpCfvoList maCfvoList;
    XclExpColScaleColList maColList;
    sal_Int32 mnPriority;
};

class XclExpDataBar : public XclExpRecord, protected XclExpRoot
{
public:
    explicit XclExpDataBar( const XclExpRoot& rRoot, const ScDataBarFormat& rFormat, sal_Int32 nPriority, const OString& rGUID);

    virtual void SaveXml( XclExpXmlStream& rStrm ) override;
private:
    std::unique_ptr<XclExpCfvo> mpCfvoLowerLimit;
    std::unique_ptr<XclExpCfvo> mpCfvoUpperLimit;
    std::unique_ptr<XclExpColScaleCol> mpCol;

    const ScDataBarFormat& mrFormat;
    sal_Int32 mnPriority;
    OString maGUID;
};

class XclExpIconSet : public XclExpRecord, protected XclExpRoot
{
public:
    explicit XclExpIconSet( const XclExpRoot& rRoot, const ScIconSetFormat& rFormat, sal_Int32 nPriority );

    virtual void SaveXml( XclExpXmlStream& rStrm ) override;
private:
    typedef XclExpRecordList< XclExpCfvo > XclExpCfvoList;

    XclExpCfvoList maCfvoList;
    const ScIconSetFormat& mrFormat;
    sal_Int32 mnPriority;
};

/** Contains all conditional formats of a specific sheet. */
class XclExpCondFormatBuffer : public XclExpRecordBase, protected XclExpRoot
{
public:
    /** Constructs CONDFMT and CF records containing the conditional formats of the current sheet. */
    explicit            XclExpCondFormatBuffer( const XclExpRoot& rRoot, const XclExtLstRef& xExtLst );

    /** Writes all contained CONDFMT records with their CF records. */
    virtual void        Save( XclExpStream& rStrm ) override;
    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

private:
    typedef XclExpRecordList< XclExpCondfmt > XclExpCondfmtList;
    XclExpCondfmtList   maCondfmtList;  /// List of CONDFMT records.
};

// Data Validation ============================================================

/** Provides export of the data of a DV record.
    @descr  This record contains the settings for a data validation. In detail
    this is a pointer to the core validation data and a cell range list with all
    affected cells. The handle index is used to optimize list search algorithm. */
class XclExpDV : public XclExpRecord, protected XclExpRoot
{
public:
    explicit            XclExpDV( const XclExpRoot& rRoot, sal_uLong nScHandle );
    virtual             ~XclExpDV() override;

    /** Returns the core handle of the validation data. */
    sal_uLong        GetScHandle() const { return mnScHandle; }

    /** Inserts a new cell range into the cell range list. */
    void                InsertCellRange( const ScRange& rPos );
    /** Converts the Calc range list to the Excel range list.
        @return  false = Resulting range list empty - do not write this record. */
    bool                Finalize();

    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

private:
    /** Writes the body of the DV record. */
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    ScRangeList         maScRanges;     /// Calc range list with all affected cells.
    XclRangeList        maXclRanges;    /// Excel range list with all affected cells.
    XclExpString        maPromptTitle;  /// The prompt title.
    XclExpString        maPromptText;   /// The prompt text.
    XclExpString        maErrorTitle;   /// The error title.
    XclExpString        maErrorText;    /// The error text.
    XclExpStringRef     mxString1;      /// String for first condition formula.
    XclTokenArrayRef    mxTokArr1;      /// Formula for first condition.
    OUString            msFormula1;     /// OOXML Formula for first condition.
    OUString            msList;         /// x12ac:list for first condition.
    XclTokenArrayRef    mxTokArr2;      /// Formula for second condition.
    OUString            msFormula2;     /// OOXML Formula for second condition.
    sal_uInt32          mnFlags;        /// Miscellaneous flags.
    sal_uLong               mnScHandle;     /// The core handle for quick list search.
};

/** This class contains the DV record list following the DVAL record. */
class XclExpDval : public XclExpRecord, protected XclExpRoot
{
public:
    explicit            XclExpDval( const XclExpRoot& rRoot );
    virtual             ~XclExpDval() override;

    /** Inserts the cell range into the range list of the DV record with the specified handle. */
    void                InsertCellRange( const ScRange& rRange, sal_uLong nScHandle );

    /** Writes the DVAL record and the DV record list. */
    virtual void        Save( XclExpStream& rStrm ) override;
    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;

private:
    /** Searches for or creates a XclExpDV record object with the specified handle. */
    XclExpDV&           SearchOrCreateDv( sal_uLong nScHandle );

    /** Writes the body of the DVAL record. */
    virtual void        WriteBody( XclExpStream& rStrm ) override;

private:
    typedef XclExpRecordList< XclExpDV >    XclExpDVList;
    typedef XclExpDVList::RecordRefType     XclExpDVRef;

    XclExpDVList        maDVList;       /// List of DV records
    XclExpDVRef         mxLastFoundDV;  /// For search optimization.
};

// Web Queries ================================================================

/** Contains all records for a web query (linked tables in an HTML document).
    @descr  mode 1 (entire document): mpQryTables==0, mbEntireDoc==true;
    mode 2 (all tables): mpQryTables==0, mbEntireDoc==false;
    mode 3 (custom range list): mpQryTables!=0, mbEntireDoc==false. */
class XclExpWebQuery : public XclExpRecordBase
{
public:
    /** Constructs a web query record container with settings from Calc. */
    explicit            XclExpWebQuery(
                            const OUString& rRangeName,
                            const OUString& rUrl,
                            const OUString& rSource,
                            sal_Int32 nRefrSecs );
    virtual             ~XclExpWebQuery() override;

    /** Writes all needed records for this web query. */
    virtual void        Save( XclExpStream& rStrm ) override;

private:
    XclExpString        maDestRange;    /// Destination range.
    XclExpString        maUrl;          /// Source document URL.
    XclExpStringRef     mxQryTables;    /// List of source range names.
    sal_Int16           mnRefresh;      /// Refresh time in minutes.
    bool                mbEntireDoc;    /// true = entire document.
};

/** Contains all web query records for this document. */
class XclExpWebQueryBuffer : public XclExpRecordList< XclExpWebQuery >
{
public:
    explicit            XclExpWebQueryBuffer( const XclExpRoot& rRoot );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/*************************************************************************
 *
 *  $RCSfile: xecontent.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-16 08:18:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// ============================================================================

#ifndef SC_XECONTENT_HXX
#define SC_XECONTENT_HXX

#ifndef SC_RANGELST_HXX
#include "rangelst.hxx"
#endif

#ifndef SC_XLCONTENT_HXX
#include "xlcontent.hxx"
#endif
#ifndef SC_XERECORD_HXX
#include "xerecord.hxx"
#endif
#ifndef SC_XEHELPER_HXX
#include "xehelper.hxx"
#endif


/* ============================================================================
Classes to export the big Excel document contents (related to several cells or
globals for the document).
- Shared string table
- Background bitmap
- Hyperlinks
- Label ranges
- Conditional formatting
- Data validation
- Web Queries
============================================================================ */

// Shared string table ========================================================

/** Provides export of the SST (shared string table) record.
    @descr  Contains all strings in the document and writes the SST. */
class XclExpSst : public XclExpRecordBase
{
private:
    ScfDelList< XclExpString >  maStringList; /// List with formatted and unformatted strings.

public:
    inline explicit             XclExpSst() {}
    virtual                     ~XclExpSst();

    /** Inserts a new string into the table.
        @return  The index of the string in the SST. */
    sal_uInt32                  Insert( XclExpString* pString );

    /** Writes the complete SST and EXTSST records. */
    virtual void                Save( XclExpStream& rStrm );
};


// Background bitmap ==========================================================

class Graphic;

/** Provides export of a background bitmap of a sheet. */
class XclExpBitmap : public XclExpRecord
{
private:
    const Graphic*              mpGraphic;      /// Pointer to the bitmap in a brush item.

public:
    explicit                    XclExpBitmap( const XclExpRoot& rRoot );

    /** Writes the BITMAP record, if mpGraphic points to a bitmap. */
    virtual void                Save( XclExpStream& rStrm );
};


// Hyperlinks =================================================================

class SvxURLField;
class INetURLObject;

/** Provides export of hyperlink data. */
class XclExpHyperlink : public XclExpRecord
{
private:
    typedef ::std::auto_ptr< String >   StringPtr;
    typedef ::std::auto_ptr< SvStream > SvStreamPtr;

private:
    ScAddress                   maPos;              /// Position of the hyperlink.
    sal_uInt32                  mnFlags;            /// Option flags.
    StringPtr                   mpRepr;             /// Cell representation text.
    SvStreamPtr                 mpVarData;          /// Buffer stream with variable data.

public:
    /** Constructs the HLINK record from an URL text field. */
    explicit                    XclExpHyperlink( const XclExpRoot& rRoot, const SvxURLField& rUrlField );
    virtual                     ~XclExpHyperlink();

    /** Sets the cell position of the hyperlink. */
    inline void                 SetPosition( const ScAddress& rPos ) { maPos = rPos; }

    /** Returns the cell representation text or NULL, if not available. */
    inline const String*        GetRepr() const { return mpRepr.get(); }

private:
    /** Builds path and file name from the passed URL object.
        @param rName  (out) The path + file name for export.
        @param rnLevel  (out) The parent directory level.
        @param rbRel  (out) true = path is relative. */
    void                        BuildFileName(
                                    String& rName, sal_uInt16& rnLevel, bool& rbRel,
                                    const INetURLObject& rUrlObj, const XclExpRoot& rRoot ) const;

    /** Writes the body of the HLINK record. */
    virtual void                WriteBody( XclExpStream& rStrm );
};


// ----------------------------------------------------------------------------

typedef XclExpRecordList< XclExpHyperlink > XclExpHyperlinkList;


// Label ranges ===============================================================

/** Provides export of the row/column label range list of a sheet. */
class XclExpLabelranges : public XclExpRecord
{
public:
    /** Fills the cell range lists with all ranges of the current sheet. */
    explicit                    XclExpLabelranges( const XclExpRoot& rRoot );

    /** Writes the LABELRANGES record if it contains at least one range. */
    virtual void                Save( XclExpStream& rStrm );

private:
    /** Fills the specified range list with all label headers of the current sheet.
        @param rRanges  The cell range list to fill.
        @param xLabelRangesRef  The core range list with all ranges.
        @param nTab  The current sheet index. */
    void                        FillRangeList( ScRangeList& rRanges, ScRangePairListRef xLabelRangesRef, sal_uInt16 nTab );

    /** Writes the body of the LABELRANGES record. */
    virtual void                WriteBody( XclExpStream& rStrm );

private:
    ScRangeList                 maRowRanges;    /// Cell range list for row labels.
    ScRangeList                 maColRanges;    /// Cell range list for column labels.
};


// Conditional formatting =====================================================

class ScCondFormatEntry;
class XclExpCF_Impl;

/** Represents a CF record that contains one condition of a conditional format. */
class XclExpCF : public XclExpRecord, protected XclExpRoot
{
public:
    explicit                    XclExpCF( const XclExpRoot& rRoot, const ScCondFormatEntry& rFormatEntry );
    virtual                     ~XclExpCF();

private:
    /** Writes the body of the CF record. */
    virtual void                WriteBody( XclExpStream& rStrm );

private:
    typedef ::std::auto_ptr< XclExpCF_Impl > XclExpCF_ImplPtr;
    XclExpCF_ImplPtr            mpImpl;
};


// ----------------------------------------------------------------------------

class ScConditionalFormat;

/** Represents a CONDFMT record that contains all conditions of a conditional format.
    @descr  Contains the conditions which are stored in CF records. */
class XclExpCondfmt : public XclExpRecord, protected XclExpRoot
{
public:
    explicit                    XclExpCondfmt( const XclExpRoot& rRoot, const ScConditionalFormat& rCondFormat );
    virtual                     ~XclExpCondfmt();

    /** Returns true, if this conditional format contains at least one cell range and CF record. */
    bool                        IsValid() const;

    /** Writes the CONDFMT record with following CF records, if there is valid data. */
    virtual void                Save( XclExpStream& rStrm );

private:
    /** Writes the body of the CONDFMT record. */
    virtual void                WriteBody( XclExpStream& rStrm );

private:
    typedef XclExpRecordList< XclExpCF > XclExpCFList;

    XclExpCFList                maCFList;       /// List of CF records.
    ScRangeList                 maRanges;       /// Cell ranges for this conditional format.
};


// ----------------------------------------------------------------------------

/** Contains all conditional formats of a specific sheet. */
class XclExpCondFormatBuffer : public XclExpRecordBase
{
public:
    /** Constructs CONDFMT and CF records containing the conditional formats of the current sheet. */
    explicit                    XclExpCondFormatBuffer( const XclExpRoot& rRoot );

    /** Writes all contained CONDFMT records with their CF records. */
    virtual void                Save( XclExpStream& rStrm );

private:
    typedef XclExpRecordList< XclExpCondfmt > XclExpCondfmtList;
    XclExpCondfmtList           maCondfmtList;  /// List of CONDFMT records.
};


// Data Validation ============================================================

class ScValidationData;

/** Provides export of the data of a DV record.
    @descr  This record contains the settings for a data validation. In detail
    this is a pointer to the core validation data and a cell range list with all
    affected cells. The handle index is used to optimize list search algorithm. */
class XclExpDV : public XclExpRecord, protected XclExpRoot
{
public:
    explicit                    XclExpDV( const XclExpRoot& rRoot, sal_uInt32 nHandle );

    /** Returns the core handle of the validation data. */
    inline sal_uInt32           GetHandle() const { return mnHandle; }

    /** Inserts a new cell range into the cell range list. */
    void                        InsertCellRange( const ScRange& rPos );
    /** Checks the record contents and crops the range list.
        @return  false = Do not write this record. */
    bool                        CheckWriteRecord();

private:
    /** Writes the body of the DV record. */
    virtual void                WriteBody( XclExpStream& rStrm );

private:
    ScRangeList                 maRanges;       /// A list with all affected cells.
    const ScValidationData*     mpValData;      /// Pointer to the core data (no ownership).
    sal_uInt32                  mnHandle;       /// The core handle for quick list search.
};


// ----------------------------------------------------------------------------

/** This class contains the DV record list following the DVAL record. */
class XclExpDval : public XclExpRecord, protected XclExpRoot
{
public:
    explicit                    XclExpDval( const XclExpRoot& rRoot );

    /** Inserts the specified cell range into the range list of the DV record with
        the specified handle. */
    void                        InsertCellRange( const ScRange& rRange, sal_uInt32 nHandle );

    /** Writes the DVAL record and the DV record list. */
    virtual void                Save( XclExpStream& rStrm );

private:
    /** Searches for or creates a XclExpDV record object with the specified handle. */
    XclExpDV&                   SearchOrCreateDv( sal_uInt32 nHandle );

    /** Writes the body of the DVAL record. */
    virtual void                WriteBody( XclExpStream& rStrm );

private:
    typedef XclExpRecordList< XclExpDV > XclExpDVList;

    XclExpDVList                maDVList;       /// List of DV records
    XclExpDV*                   mpLastFoundDV;  /// For search optimization.
};


// Web Queries ================================================================

/** Contains all records for a web query (linked tables in an HTML document).
    @descr  mode 1 (entire document): mpQryTables==NULL, mbEntireDoc==true;
    mode 2 (all tables): mpQryTables==NULL, mbEntireDoc==false;
    mode 3 (custom range list): mpQryTables!=NULL, mbEntireDoc==false. */
class XclExpWebQuery : public XclExpRecordBase
{
public:
    /** Constructs a web query record container with settings from Calc. */
    explicit                    XclExpWebQuery(
                                    const String& rRangeName,
                                    const String& rUrl,
                                    const String& rSource,
                                    sal_Int32 nRefrSecs );
    virtual                     ~XclExpWebQuery();

    /** Writes all needed records for this web query. */
    virtual void                Save( XclExpStream& rStrm );

private:
    XclExpString                maDestRange;    /// Destination range.
    XclExpString                maUrl;          /// Source document URL.
    XclExpString*               mpQryTables;    /// List of source range names.
    sal_Int16                   mnRefresh;      /// Refresh time in minutes.
    bool                        mbEntireDoc;    /// true = entire document.
};


// ----------------------------------------------------------------------------

/** Contains all web query records for this document. */
class XclExpWebQueryBuffer : public XclExpRecordList< XclExpWebQuery >
{
public:
    explicit                    XclExpWebQueryBuffer( const XclExpRoot& rRoot );
};


// ============================================================================

#endif


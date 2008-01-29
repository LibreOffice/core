/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xicontent.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:31:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_XICONTENT_HXX
#define SC_XICONTENT_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
#ifndef SC_RANGELST_HXX
#include "rangelst.hxx"
#endif

#ifndef SC_XLCONTENT_HXX
#include "xlcontent.hxx"
#endif
#ifndef SC_XISTRING_HXX
#include "xistring.hxx"
#endif
#ifndef SC_XIROOT_HXX
#include "xiroot.hxx"
#endif

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

class ScBaseCell;

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

    /** Creates a new text cell or edit cell for a Calc document.
        @param nXFIndex  Index to XF for first text portion (checks escapement). */
    ScBaseCell*         CreateCell( sal_uInt32 nSstIndex, sal_uInt16 nXFIndex = 0 ) const;

private:
    typedef ::std::vector< XclImpString > XclImpStringVec;
    XclImpStringVec     maStrings;          /// List with all strings in the SST.
};

// Hyperlinks =================================================================

/** Provides importing hyperlinks and inserting them into a document. */
class XclImpHyperlink : ScfNoInstance
{
public:
    /** Reads a HLINK record and inserts it into the document.
        @descr  Import stream must be located at start of a HLINK record. */
    static void         ReadHlink( XclImpStream& rStrm );

    /** Reads the (undocumented) embedded hyperlink data and returns the URL. */
    static String       ReadEmbeddedData( XclImpStream& rStrm );

    /** Inserts the URL into a range of cells. Does not modify value or formula cells. */
    static void         InsertUrl( const XclImpRoot& rRoot, const XclRange& rXclRange, const String& rUrl );

    /** Convert the sheet name with invalid character(s) in URL when the URL is
        to a location within the same document (e.g. #'Sheet&Name'.A1). */
    static void         ConvertToValidTabName(String& rName);
};

// Label ranges ===============================================================

/** Provides importing label ranges and inserting them into a document. */
class XclImpLabelranges : ScfNoInstance
{
public:
    /** Reads a LABELRANGES record and inserts the label ranges into the document.
        @descr  Import stream must be located at start of a LABELRANGES record. */
    static void         ReadLabelranges( XclImpStream& rStrm );
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

    /** Inserts the conditional formattings into the document. */
    void                Apply();

private:
    typedef ScfDelList< XclImpCondFormat > XclImpCondFmtList;
    XclImpCondFmtList   maCondFmtList;      /// List with all conditional formattings.
};

// Data Validation ============================================================

/** Provides importing validation data and inserting it into a document. */
class XclImpValidation : ScfNoInstance
{
public:
    /** Reads a DVAL record and sets marks the dropdown arrow control to be ignored. */
    static void         ReadDval( XclImpStream& rStrm );
    /** Reads a DV record and inserts validation data into the document. */
    static void         ReadDV( XclImpStream& rStrm );
};

// Web queries ================================================================

/** Stores the data of one web query. */
class XclImpWebQuery : ScfNoCopy
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
    void                Apply( ScDocument& rDoc, const String& rFilterName );

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
    String              maTables;       /// List of source range names.
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
    typedef ScfDelList< XclImpWebQuery > XclImpWebQueryList;
    XclImpWebQueryList  maWQList;       /// List of the web query objects.
};

// Decryption =================================================================

/** Provides static functions to import stream decryption settings. */
class XclImpDecryptHelper : ScfNoInstance
{
public:
    /** Reads the FILEPASS record, queries a password and sets decryption algorihm.
        @return  Error code that may cause an error message after import. */
    static ErrCode      ReadFilepass( XclImpStream& rStrm );
};

// ============================================================================

#endif


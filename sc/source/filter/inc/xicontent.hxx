/*************************************************************************
 *
 *  $RCSfile: xicontent.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 09:05:46 $
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
#ifndef SC_XIHELPER_HXX
#include "xihelper.hxx"
#endif

class XclImpStream;

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
    @descr  This class loads the SST and provides access to the strings. */
class XclImpSst : protected XclImpRoot
{
public:
    explicit            XclImpSst( const XclImpRoot& rRoot );

    /** Reads the entire SST record.
        @descr  Import stream must be located at start of a SST record. */
    void                ReadSst( XclImpStream& rStrm );

    /** Returns a pointer to the string with the passed index. */
    inline const XclImpString* GetString( sal_uInt32 nSstIndex ) const;

    /** Creates a new text cell or edit cell for a Calc document.
        @param nXFIndex  Index to XF for first text portion (checks escapement). */
    ScBaseCell*         CreateCell( sal_uInt32 nSstIndex, sal_uInt32 nXFIndex = 0 ) const;

private:
    typedef ScfDelList< XclImpString > XclImpStringList;

    XclImpStringList    maStringList;       /// List with formatted and unformatted strings.
    XclImpString        maErrorString;      /// Placeholder for strings not found in the list.
};

inline const XclImpString* XclImpSst::GetString( sal_uInt32 nSstIndex ) const
{
    return maStringList.GetObject( nSstIndex );
}

// Hyperlinks =================================================================

/** Provides importing hyperlinks and inserting them into a document. */
class XclImpHyperlink : ScfNoInstance
{
public:
    /** Reads a HLINK record and inserts it into the document.
        @descr  Import stream must be located at start of a HLINK record. */
    static void         ReadHlink( XclImpStream& rStrm );
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
    typedef ::std::auto_ptr< ScConditionalFormat > ScConditionalFormatPtr;

    ScRangeList         maRanges;           /// Destination cell ranges.
    ScConditionalFormatPtr mpScCondFormat;  /// Calc conditional format.
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
    typedef ScfDelList< XclImpCondFormat > XclImpCondFormatList;
    XclImpCondFormatList maCondFmtList; /// List with all conditional formattings.
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
    inline explicit     XclImpWebQueryBuffer( const XclImpRoot& rRoot ) : XclImpRoot( rRoot ) {}

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
    ScfDelList< XclImpWebQuery > maWQList;      /// List of the web query objects.
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


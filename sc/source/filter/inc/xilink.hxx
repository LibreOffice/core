/*************************************************************************
 *
 *  $RCSfile: xilink.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-16 08:19:43 $
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

#ifndef SC_XILINK_HXX
#define SC_XILINK_HXX

#ifndef SC_XIHELPER_HXX
#include "xihelper.hxx"
#endif

#include "excdefs.hxx"

class ScDocument;
class ScTokenArray;
class XclImpStream;


/* ============================================================================
Classes for import of different kinds of internal/external references.
- 3D cell and cell range links
- External cell and cell range links
- Add-in functions
- DDE links
- OLE object links
============================================================================ */

// Excel sheet indexes ========================================================

/** This buffer contains the creation order of all sheets inside the Excel workbook.
    @descr  The creation order list is contained in the TABID record.
    Example: If the list contains 3;1;2 this means that the second sheet in the file
    was created first, than the third sheet in the file was created and finally the
    first sheet. */
class XclImpTabIdBuffer
{
private:
    ScfUInt16Vec                maTabIdVec;     /// The vector with sheet indexes.

public:
    /** Reads the TABID record. */
    void                        ReadTabid( XclImpStream& rStrm );

    /** Returns the current sheet index calculated from creation index.
        @param nCreatedId  The creation index of the sheet (1-based).
        @param nMaxTabId  All values greater than this parameter are not used to find the index.
        @return  The 0-based index of the sheet nCreatedId if it is contained in the list.
        Example: The buffer is 3;5;2;4;1, nCreatedId is 1 and nMaxTabId is 3. The function will
        return 2 which is the 0-based index of sheet 1 in the list 3;2;1. */
    sal_uInt16                  GetCurrentIndex( sal_uInt16 nCreatedId, sal_uInt16 nMaxTabId = 0xFFFF ) const;
};


// External names =============================================================

/** Type of an external name. */
enum XclImpExtNameType
{
    xlExtName,                  /// An external defined name or AddIn function name.
    xlExtDDE,                   /// A DDE link range.
    xlExtOLE                    /// An OLE object link.
};


// ----------------------------------------------------------------------------

/** Stores contents of an external name.
    @descr Supported: External defined names, AddIn names, DDE links and OLE objects. */
class XclImpExtName
{
private:
    typedef ::std::auto_ptr< XclImpCachedMatrix > XclImpCachedMatrixPtr;

    XclImpCachedMatrixPtr       mpDdeMatrix;        /// Cached results of the DDE link.
    String                      maName;             /// The name of the external name.
    String                      maAddInName;        /// The converted Calc add-in function name.
    sal_uInt32                  mnStorageId;        /// Storage ID for OLE object storages.
    XclImpExtNameType           meType;             /// Type of the external name.

public:
    /** Reads the external name from the stream. */
    explicit                    XclImpExtName( XclImpStream& rStrm );

    /** Create and apply the cached list of this DDE Link to the document. */
    void                        CreateDdeData( ScDocument& rDoc,
                                    const String& rApplc,
                                    const String& rExtDoc) const;

    inline XclImpExtNameType    GetType() const         { return meType; }
    inline const String&        GetName() const         { return maName; }
    inline const String&        GetAddInName() const    { return maAddInName; }
    inline sal_uInt32           GetStorageId() const    { return mnStorageId; }
};


// ----------------------------------------------------------------------------

/** Buffer for all external names of one SUPBOOK (external document). */
class XclImpExtNameList : public ScfDelList< XclImpExtName >
{
public:
    /** Returns the external name specified by an index from the Excel document (one-based). */
    const XclImpExtName*        GetName( sal_uInt16 nXclIndex ) const;
};


// Cached external cells ======================================================

/** Contains the address and value of an external referenced cell. */
class XclImpCrn : public XclImpCachedValue
{
public:
    /** Reads a cached value and stores it with its cell address. */
    explicit                    XclImpCrn( XclImpStream& rStrm, sal_uInt16 nCol, sal_uInt16 nRow );

    /** Copies the cached value to sheet nTab in the document. */
    void                        SetCell( ScDocument& rDoc, sal_uInt16 nTab ) const;
};


// External documents =========================================================

/** Contains the name and sheet index of one sheet in an external document. */
class XclImpSupbookTab
{
private:
    ScfDelList< XclImpCrn >     maCrnList;      /// List of CRN records (cached cell values).
    String                      maName;         /// Name of the external sheet.
    sal_uInt16                  mnScTab;        /// New sheet index in Calc document.

public:
    /** Stores the sheet name and marks the sheet index as invalid.
        The sheet index is set while creating a sheet with CreateSheet(). */
    explicit                    XclImpSupbookTab( const String& rName );

    inline const String&        GetName() const     { return maName; }
    inline sal_uInt16           GetScTab() const    { return mnScTab; }

    /** Stores the contents of an external referenced cell in the own CRN list. */
    inline void                 AppendCrn( XclImpCrn* pCrn ) { maCrnList.Append( pCrn ); }

    /** Creates a new sheet in the Calc document and stores all external cells in it. */
    void                        CreateTable( ScDocument& rDoc, const String& rUrl );
};


// ----------------------------------------------------------------------------

/** This class represents an external linked document (record SUPBOOK).
    @descr  Contains a list of all referenced sheets in the document. */
class XclImpSupbook
{
private:
    typedef ScfDelList< XclImpSupbookTab > XclImpSupbookTabList;

    XclImpSupbookTabList        maSupbookTabList;   /// All sheet names of the document.
    XclImpExtNameList           maExtNameList;      /// All external names of the document.
    String                      maUrl;              /// URL of the external document.
    sal_uInt16                  mnCurrExcTab;       /// Current Excel sheet index of external cells.
    bool                        mbSelf;             /// true = internal 3D references.
    bool                        mbAddIn;            /// true = Add-in function names.

public:
    /** Reads the SUPBOOK record from stream. */
    explicit                    XclImpSupbook( XclImpStream& rStrm );

    /** Reads and decodes an encoded URL. */
    static void                 ReadUrl( XclImpStream& rStrm, String& rUrl, bool& rbSelf );
    /** Reads an external sheet name. */
    static void                 ReadTabName( XclImpStream& rStrm, String& rTabName );

    /** Reads an XCT record (count of following CRNs and current sheet). */
    void                        ReadXct( XclImpStream& rStrm );
    /** Reads a CRN record (external referenced cell). */
    void                        ReadCrn( XclImpStream& rStrm );
    /** Reads an EXTERNNAME record. */
    void                        ReadExternname( XclImpStream& rStrm );

    /** Returns true, if this SUPBOOK contains internal 3D references. */
    inline bool                 IsSelf() const { return mbSelf; }
    /** Returns true, if this SUPBOOK contains add-in function names. */
    inline bool                 IsAddIn() const { return mbAddIn; }

    /** Returns the URL of the external document. */
    inline const String&        GetUrl() const { return maUrl; }
    /** Returns the external name specified by an index from the Excel document (one-based). */
    const XclImpExtName*        GetExtName( sal_uInt16 nXclIndex ) const;
    /** Decodes the URL to special links.
        @descr  For DDE links: Decodes application and document name.
        For OLE object links: Decodes class and document name.
        @return  true = decoding was successful. */
    bool                        GetLink( String& rApplic, String& rDoc ) const;

    /** Returns Calc sheet index from Excel sheet index. */
    sal_uInt16                  GetScTabNum( sal_uInt16 nExcTabNum ) const;
    /** Returns Calc sheet index from sheet name. */
    sal_uInt16                  GetScTabNum( const String& rTabName ) const;

    /** Creates all sheets of this external document.
        @param nFirstTab  The external Excel index of the first sheet to be created.
        @param nLastTab  The external Excel index of the last sheet to be created. */
    void                        CreateTables(
                                    const XclImpRoot& rRoot,
                                    sal_uInt16 nFirstTab, sal_uInt16 nLastTab ) const;
};


// ----------------------------------------------------------------------------

/** Contains a list of all external documents (SUPBOOKs) used in this workbook. */
class XclImpSupbookBuffer
{
private:
    typedef ScfDelList< XclImpSupbook > XclImpSupbookList;

    XclImpSupbookList           maSupbookList;      /// List of external documents.

public:
    /** Reads and appends a SUPBOOK record (BIFF8). */
    inline void                 ReadSupbook( XclImpStream& rStrm )
                                    { maSupbookList.Append( new XclImpSupbook( rStrm ) ); }

    /** Returns the number of SUPBOOKs. */
    inline sal_uInt32           Count() const { return maSupbookList.Count(); }

    /** Returns the specified SUPBOOK record data. */
    inline const XclImpSupbook* GetSupbook( sal_uInt32 nIndex ) const
                                    { return maSupbookList.GetObject( nIndex ); }
    /** Returns the SUPBOOK record data specified by the URL of the external document. */
    const XclImpSupbook*        GetSupbook( const String& rUrl ) const;
    /** Returns the current SUPBOOK record data (last SUPBOOK in the list). */
    inline XclImpSupbook*       GetCurrSupbook() const { return maSupbookList.Last(); }
};


// Import link manager ========================================================

/** Contains the SUPBOOK index and sheet indexes of an external link.
    @descr  It is possible to enter a formula like =SUM(Sheet1:Sheet3!A1),
    therefore here occurs a sheet range. */
struct XclImpXti
{
    sal_uInt16                  mnSupbook;          /// Index to SUPBOOK.
    sal_uInt16                  mnFirst;            /// Index of first sheet.
    sal_uInt16                  mnLast;             /// Index of last sheet.
};


// ----------------------------------------------------------------------------

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
private:
    typedef ScfDelList< XclImpXti > XclImpXtiList;

    XclImpXtiList               maXtiList;          /// List of all XTI structures.
    XclImpSupbookBuffer         maSupbookBuffer;    /// Buffer for all external workbooks.
    bool                        mbCreated;          /// true = Calc sheets already created.

public:
    explicit                    XclImpLinkManager( const XclImpRoot& rRoot );
                                ~XclImpLinkManager();

    /** Reads the EXTERNSHEET record. */
    void                        ReadExternsheet( XclImpStream& rStrm );
    /** Reads a SUPBOOK record. */
    void                        ReadSupbook( XclImpStream& rStrm );
    /** Reads an XCT record and appends it to the current SUPBOOK. */
    void                        ReadXct( XclImpStream& rStrm );
    /** Reads a CRN record and appends it to the current SUPBOOK. */
    void                        ReadCrn( XclImpStream& rStrm );
    /** Reads an EXTERNNAME record and appends it to the current SUPBOOK. */
    void                        ReadExternname( XclImpStream& rStrm );

    /** Returns the XTI record data (reference indexes) from position nXtiIndex. */
    inline const XclImpXti*     GetXti( sal_uInt32 nXtiIndex ) const
                                    { return maXtiList.GetObject( nXtiIndex ); }

    /** Returns the specified SUPBOOK (external document). */
    const XclImpSupbook*        GetSupbook( sal_uInt32 nXtiIndex ) const;
    /** Returns the SUPBOOK (external workbook) specified by its URL. */
    inline const XclImpSupbook* GetSupbook( const String& rUrl ) const
                                    { return maSupbookBuffer.GetSupbook( rUrl ); }

private:
    /** Creates all external sheets in the Calc document. */
    void                        CreateTables();

    /** Finds the largest range of sheet indexes in a SUPBOOK after a start sheet index.
        @param nSupb  The list index of the SUPBOOK.
        @param nStart  The first allowed sheet index. Sheet ranges with an earlier start index are ignored.
        @param rnFirst  The first index of the range is returned here.
        @param rnLast  The last index of the range is returned here (incusive).
        @return  true = the return values are valid; false = nothing found. */
    bool                        FindNextTabRange(
                                    sal_uInt16& rnFirst, sal_uInt16& rnLast,
                                    sal_uInt16 nSupb, sal_uInt16 nStart ) const;
};


// ============================================================================

#endif


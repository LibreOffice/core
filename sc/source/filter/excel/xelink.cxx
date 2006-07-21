/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xelink.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 11:57:50 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#ifndef SC_XELINK_HXX
#include "xelink.hxx"
#endif

#include <algorithm>

#ifndef _UNOTOOLS_COLLATORWRAPPER_HXX
#include <unotools/collatorwrapper.hxx>
#endif
#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif

#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_CELL_HXX
#include "cell.hxx"
#endif
#ifndef SC_SCEXTOPT_HXX
#include "scextopt.hxx"
#endif

// ============================================================================
// *** Helper classes ***
// ============================================================================

// External names =============================================================

/** This is a base class for any external name (i.e. add-in names or DDE links).
    @descr  Derived classes implement creation and export of the external names. */
class XclExpExtNameBase : public XclExpRecord, protected XclExpRoot
{
public:
    /** @param nFlags  The flags to export. */
    explicit            XclExpExtNameBase( const XclExpRoot& rRoot,
                            const String& rName, sal_uInt16 nFlags = 0 );
    virtual             ~XclExpExtNameBase();

    /** Returns the name string of the external name. */
    inline const String& GetName() const { return maName; }

private:
    /** Writes the start of the record that is equal in all EXTERNNAME records and calls WriteAddData(). */
    virtual void        WriteBody( XclExpStream& rStrm );
    /** Called to write additional data following the common record contents.
        @descr  Derived classes should overwrite this function to write their data. */
    virtual void        WriteAddData( XclExpStream& rStrm );

private:
    String              maName;         /// Calc name (title) of the external name.
    XclExpStringRef     mxName;         /// Excel name (title) of the external name.
    sal_uInt16          mnFlags;        /// Flags for record export.
};

// ----------------------------------------------------------------------------

/** Represents an EXTERNNAME record for an add-in function name. */
class XclExpExtNameAddIn : public XclExpExtNameBase
{
public:
    explicit            XclExpExtNameAddIn( const XclExpRoot& rRoot, const String& rName );

private:
    /** Writes additional record contents. */
    virtual void        WriteAddData( XclExpStream& rStrm );
};

// ----------------------------------------------------------------------------

/** Represents an EXTERNNAME record for a DDE link. */
class XclExpExtNameDde : public XclExpExtNameBase
{
public:
    explicit            XclExpExtNameDde( const XclExpRoot& rRoot, const String& rName,
                            sal_uInt16 nFlags, const ScMatrix* pResults = 0 );

private:
    /** Writes additional record contents. */
    virtual void        WriteAddData( XclExpStream& rStrm );

private:
    typedef ScfRef< XclExpCachedMatrix > XclExpCachedMatRef;
    XclExpCachedMatRef  mxMatrix;       /// Cached results of the DDE link.
};

// List of external names =====================================================

/** List of all external names of a sheet. */
class XclExpExtNameBuffer : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit            XclExpExtNameBuffer( const XclExpRoot& rRoot );

    /** Inserts an add-in function name
        @return  The 1-based (Excel-like) list index of the name. */
    sal_uInt16          InsertAddIn( const String& rName );
    /** Inserts a DDE link.
        @return  The 1-based (Excel-like) list index of the DDE link. */
    sal_uInt16          InsertDde( const String& rApplic, const String& rTopic, const String& rItem );

    /** Writes the EXTERNNAME record list. */
    virtual void        Save( XclExpStream& rStrm );

private:
    typedef XclExpRecordList< XclExpExtNameBase >   XclExpExtNameList;
    typedef XclExpExtNameList::RecordRefType        XclExpExtNameRef;

private:
    /** Returns the 1-based (Excel-like) list index of the external name or 0, if not found. */
    sal_uInt16          GetIndex( const String& rName ) const;
    /** Appends the passed newly crested external name.
        @return  The 1-based (Excel-like) list index of the appended name. */
    sal_uInt16          AppendNew( XclExpExtNameBase* pExtName );

private:
    XclExpExtNameList   maNameList;     /// The list with all EXTERNNAME records.
};

// Cached external cells ======================================================

/** Base class to store the contents of one external cell (record CRN). */
class XclExpCrn : public XclExpRecord
{
protected:
    /** @param nAddSize  The size of additional data derived classes will write. */
    explicit            XclExpCrn( SCCOL nScCol, SCROW nScRow, sal_uInt8 nId, sal_uInt32 nAddLen = 0 );

private:
    /** Writes the start of the record that is equal in all CRN records and calls WriteAddData(). */
    virtual void        WriteBody( XclExpStream& rStrm );

    /** Called to write additional data following the common record contents.
        @descr  Derived classes should overwrite this function to write their data. */
    virtual void        WriteAddData( XclExpStream& rStrm ) = 0;

private:
    sal_uInt16          mnXclCol;   /// Column index of the external cell.
    sal_uInt16          mnXclRow;   /// Row index of the external cell.
    sal_uInt8           mnId;       /// Identifier for data type (EXC_CACHEDVAL_***).
};

// ----------------------------------------------------------------------------

/** Cached data of an external value cell. */
class XclExpCrnDouble : public XclExpCrn
{
public:
    explicit            XclExpCrnDouble( SCCOL nScCol, SCROW nScRow, double fVal );

private:
    /** Writes the double value following the common record contents. */
    virtual void        WriteAddData( XclExpStream& rStrm );

private:
    double              mfVal;      /// Value of the cached cell.
};

// ----------------------------------------------------------------------------

/** Cached data of an external text cell. */
class XclExpCrnString : public XclExpCrn
{
public:
    explicit            XclExpCrnString( SCCOL nScCol, SCROW nScRow, const String& rText );

private:
    /** Writes the string following the common record contents. */
    virtual void        WriteAddData( XclExpStream& rStrm );

private:
    XclExpString        maText;     /// Text of the cached cell.
};

// ----------------------------------------------------------------------------

/// Cached data of an external Boolean cell. */
class XclExpCrnBool : public XclExpCrn
{
public:
    explicit            XclExpCrnBool( SCCOL nScCol, SCROW nScRow, bool bBoolVal );

private:
    /** Writes the Boolean value following the common record contents. */
    virtual void        WriteAddData( XclExpStream& rStrm );

private:
    sal_uInt16          mnBool;     /// Boolean value of the cached cell.
};

// Cached cells of a sheet ====================================================

/// Represents the record XCT which is the header record of a CRN record list. */
class XclExpXct : public XclExpRecord
{
public:
    explicit            XclExpXct( const String& rTabName, sal_uInt16 nSBTab );

    /** Returns the external sheet name. */
    inline const XclExpString& GetTabName() const { return maTabName; }

    /** Stores all cells in the given range in the CRN list. */
    void                StoreCellRange( const XclExpRoot& rRoot, const ScRange& rRange );

    /** Writes the XCT and all CRN records. */
    virtual void        Save( XclExpStream& rStrm );

private:
    /** Writes the XCT record contents. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    typedef XclExpRecordList< XclExpCrn >   XclExpCrnList;
    typedef XclExpCrnList::RecordRefType    XclExpCrnRef;

    XclExpCrnList       maCrnList;      /// CRN records that follow this record.
    ScMarkData          maUsedCells;    /// Contains addresses of all stored cells.
    XclExpString        maTabName;      /// Sheet name of the external sheet.
    sal_uInt16          mnSBTab;        /// Referred sheet index in SUPBOOK record.
};

// External documents (EXTERNSHEET/SUPBOOK), base class =======================

/** Base class for records representing external sheets/documents.

    In BIFF5/BIFF7, this record is the EXTERNSHEET record containing one sheet
    of the own or an external document. In BIFF8, this record is the SUPBOOK
    record representing the entire own or external document with all referenced
    sheets.
 */
class XclExpExternSheetBase : public XclExpRecord, protected XclExpRoot
{
public:
    explicit            XclExpExternSheetBase( const XclExpRoot& rRoot,
                            sal_uInt16 nRecId, sal_uInt32 nRecSize = 0 );

protected:
    /** Creates and returns the list of EXTERNNAME records. */
    XclExpExtNameBuffer& GetExtNameBuffer();
    /** Creates and returns the list of EXTERNNAME records. */
    void                WriteExtNameBuffer( XclExpStream& rStrm );

private:
    typedef ScfRef< XclExpExtNameBuffer >   XclExpExtNameBfrRef;
    XclExpExtNameBfrRef mxExtNameBfr;   /// List of EXTERNNAME records.
};

// External documents (EXTERNSHEET, BIFF5/BIFF7) ==============================

/** Represents an EXTERNSHEET record containing the URL and sheet name of a sheet.
    @descr  This class is used up to BIFF7 only, writing a BIFF8 EXTERNSHEET
        record is implemented directly in the link manager. */
class XclExpExternSheet : public XclExpExternSheetBase
{
public:
    /** Creates an EXTERNSHEET record containing a special code (i.e. own document or sheet). */
    explicit            XclExpExternSheet( const XclExpRoot& rRoot, sal_Unicode cCode );
    /** Creates an EXTERNSHEET record referring to an internal sheet. */
    explicit            XclExpExternSheet( const XclExpRoot& rRoot, const String& rTabName );

    /** Finds or inserts an EXTERNNAME record for add-ins.
        @return  The 1-based EXTERNNAME record index; or 0, if the record list is full. */
    sal_uInt16          InsertAddIn( const String& rName );

    /** Writes the EXTERNSHEET and all EXTERNNAME, XCT and CRN records. */
    virtual void        Save( XclExpStream& rStrm );

private:
    /** Initializes the record data with the passed encoded URL. */
    void                Init( const String& rEncUrl );
    /** Writes the contents of the EXTERNSHEET  record. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    XclExpString        maTabName;      /// The name of the sheet.
};

// External documents (SUPBOOK, BIFF8) ========================================

/** The SUPBOOK record contains data for an external document (URL, sheet names, external values). */
class XclExpSupbook : public XclExpExternSheetBase
{
public:
    /** Creates a SUPBOOK record for internal references. */
    explicit            XclExpSupbook( const XclExpRoot& rRoot, sal_uInt16 nXclTabCount );
    /** Creates a SUPBOOK record for add-in functions. */
    explicit            XclExpSupbook( const XclExpRoot& rRoot );
    /** Creates a SUPBOOK record for an external document. */
    explicit            XclExpSupbook( const XclExpRoot& rRoot, const String& rUrl );
    /** Creates a SUPBOOK record for a DDE link. */
    explicit            XclExpSupbook( const XclExpRoot& rRoot, const String& rApplic, const String& rTopic );

    /** Returns true, if this SUPBOOK contains the passed URL of an external document. */
    bool                IsUrlLink( const String& rUrl ) const;
    /** Returns true, if this SUPBOOK contains the passed DDE link. */
    bool                IsDdeLink( const String& rApplic, const String& rTopic ) const;
    /** Fills the passed reference log entry with the URL and sheet names. */
    void                FillRefLogEntry( XclExpRefLogEntry& rRefLogEntry,
                            sal_uInt16 nFirstSBTab, sal_uInt16 nLastSBTab ) const;

    /** Stores all cells in the given range in the CRN list of the specified SUPBOOK sheet. */
    void                StoreCellRange( const ScRange& rRange, sal_uInt16 nSBTab );

    /** Inserts a new sheet name into the SUPBOOK and returns the SUPBOOK internal sheet index. */
    sal_uInt16          InsertTabName( const String& rTabName );
    /** Finds or inserts an EXTERNNAME record for add-ins.
        @return  The 1-based EXTERNNAME record index; or 0, if the record list is full. */
    sal_uInt16          InsertAddIn( const String& rName );
    /** Finds or inserts an EXTERNNAME record for DDE links.
        @return  The 1-based EXTERNNAME record index; or 0, if the record list is full. */
    sal_uInt16          InsertDde( const String& rItem );

    /** Writes the SUPBOOK and all EXTERNNAME, XCT and CRN records. */
    virtual void        Save( XclExpStream& rStrm );

private:
    /** Returns the sheet name inside of this SUPBOOK. */
    const XclExpString* GetTabName( sal_uInt16 nSBTab ) const;

    /** Writes the SUPBOOK record contents. */
    virtual void        WriteBody( XclExpStream& rStrm );

private:
    typedef XclExpRecordList< XclExpXct >   XclExpXctList;
    typedef XclExpXctList::RecordRefType    XclExpXctRef;

    XclExpXctList       maXctList;      /// List of XCT records (which contain CRN records).
    String              maUrl;          /// URL of the external document or application name for DDE.
    String              maDdeTopic;     /// Topic of an DDE link.
    XclExpString        maUrlEncoded;   /// Document name encoded for Excel.
    XclSupbookType      meType;         /// Type of this SUPBOOK record.
    sal_uInt16          mnXclTabCount;  /// Number of internal sheets.
};

// All SUPBOOKS in a document =================================================

/** This struct contains a sheet index range for 3D references.
    @descr  This reference consists of an index to a SUPBOOK record and indexes
    to SUPBOOK sheet names. */
struct XclExpXti
{
    sal_uInt16          mnSupbook;      /// Index to SUPBOOK record.
    sal_uInt16          mnFirstSBTab;   /// Index to the first sheet of the range in the SUPBOOK.
    sal_uInt16          mnLastSBTab;    /// Index to the last sheet of the range in the SUPBOOK.

    inline explicit     XclExpXti() : mnSupbook( 0 ), mnFirstSBTab( 0 ), mnLastSBTab( 0 ) {}
    inline explicit     XclExpXti( sal_uInt16 nSupbook, sal_uInt16 nFirstSBTab, sal_uInt16 nLastSBTab ) :
                            mnSupbook( nSupbook ), mnFirstSBTab( nFirstSBTab ), mnLastSBTab( nLastSBTab ) {}

    /** Writes this XTI structure (inside of the EXTERNSHEET record). */
    inline void         Save( XclExpStream& rStrm ) const
                            { rStrm << mnSupbook << mnFirstSBTab << mnLastSBTab; }
};

inline bool operator==( const XclExpXti& rLeft, const XclExpXti& rRight )
{
    return
        (rLeft.mnSupbook    == rRight.mnSupbook)    &&
        (rLeft.mnFirstSBTab == rRight.mnFirstSBTab) &&
        (rLeft.mnLastSBTab  == rRight.mnLastSBTab);
}

// ----------------------------------------------------------------------------

/** Contains a list of all SUPBOOK records and index arrays of external sheets. */
class XclExpSupbookBuffer : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit            XclExpSupbookBuffer( const XclExpRoot& rRoot );

    /** Finds SUPBOOK index and SUPBOOK sheet range from given Excel sheet range.
        @return  An XTI structure containing SUPBOOK and sheet indexes. */
    XclExpXti           GetXti( sal_uInt16 nFirstXclTab, sal_uInt16 nLastXclTab,
                            XclExpRefLogEntry* pRefLogEntry = 0 ) const;

    /** Stores all cells in the given range in a CRN record list. */
    void                StoreCellRange( const ScRange& rRange );

    /** Finds or inserts an EXTERNNAME record for an add-in function name.
        @param rnSupbook  Returns the index of the SUPBOOK record which contains the add-in function name.
        @param rnExtName  Returns the 1-based EXTERNNAME record index. */
    bool                InsertAddIn(
                            sal_uInt16& rnSupbook, sal_uInt16& rnExtName,
                            const String& rName );
    /** Finds or inserts an EXTERNNAME record for DDE links.
        @param rnSupbook  Returns the index of the SUPBOOK record which contains the DDE link.
        @param rnExtName  Returns the 1-based EXTERNNAME record index. */
    bool                InsertDde(
                            sal_uInt16& rnSupbook, sal_uInt16& rnExtName,
                            const String& rApplic, const String& rTopic, const String& rItem );

    /** Writes all SUPBOOK records with their sub records. */
    virtual void        Save( XclExpStream& rStrm );

private:
    typedef XclExpRecordList< XclExpSupbook >   XclExpSupbookList;
    typedef XclExpSupbookList::RecordRefType    XclExpSupbookRef;

private:
    /** Searches for the SUPBOOK record containing the passed document URL.
        @param rxSupbook  (out-param) Returns a reference to the SUPBOOK record, or 0.
        @param rnIndex  (out-param) Returns the list index, if the SUPBOOK exists.
        @return  True, if the SUPBOOK record exists (out-parameters are valid). */
    bool                GetSupbookUrl( XclExpSupbookRef& rxSupbook, sal_uInt16& rnIndex,
                            const String& rUrl ) const;
    /** Searches for the SUPBOOK record containing the passed DDE link.
        @param rxSupbook  (out-param) Returns a reference to the SUPBOOK record, or 0.
        @param rnIndex  (out-param) Returns the list index, if the SUPBOOK exists.
        @return  True, if the SUPBOOK record exists (out-parameters are valid). */
    bool                GetSupbookDde( XclExpSupbookRef& rxSupbook, sal_uInt16& rnIndex,
                            const String& rApplic, const String& rTopic ) const;

    /** Appends a new SUPBOOK to the list.
        @return  The list index of the SUPBOOK record. */
    sal_uInt16          Append( XclExpSupbookRef xSupbook );
    /** Creates and appends an external SUPBOOK record from the Calc sheet nScTab. */
    void                AddExtSupbook( SCTAB nScTab );

private:
    struct XclExpSBIndex
    {
        sal_uInt16          mnSupbook;          /// SUPBOOK index for an Excel sheet.
        sal_uInt16          mnSBTab;            /// Sheet name in SUPBOOK for an Excel sheet.
        inline void         Set( sal_uInt16 nSupbook, sal_uInt16 nSBTab )
                                { mnSupbook = nSupbook; mnSBTab = nSBTab; }
    };
    typedef ::std::vector< XclExpSBIndex > XclExpSBIndexVec;

    XclExpSupbookList   maSupbookList;      /// List of all SUPBOOK records.
    XclExpSBIndexVec    maSBIndexVec;       /// SUPBOOK and sheet name index for each Excel sheet.
    sal_uInt16          mnOwnDocSB;         /// Index to SUPBOOK for own document.
    sal_uInt16          mnAddInSB;          /// Index to add-in SUPBOOK.
};

// Export link manager ========================================================

/** Abstract base class for implementation classes of the link manager. */
class XclExpLinkManagerImpl : protected XclExpRoot
{
public:
    /** Derived classes search for an EXTSHEET structure for the given Calc sheet range. */
    virtual void        FindExtSheet( sal_uInt16& rnExtSheet,
                            sal_uInt16& rnFirstXclTab, sal_uInt16& rnLastXclTab,
                            SCTAB nFirstScTab, SCTAB nLastScTab,
                            XclExpRefLogEntry* pRefLogEntry ) = 0;
    /** Derived classes search for a special EXTERNSHEET index for the own document. */
    virtual sal_uInt16  FindExtSheet( sal_Unicode cCode ) = 0;

    /** Derived classes store all cells in the given range in a CRN record list. */
    virtual void        StoreCellRange( const SingleRefData& rRef1, const SingleRefData& rRef2 ) = 0;

    /** Derived classes find or insert an EXTERNNAME record for an add-in function name. */
    virtual bool        InsertAddIn(
                            sal_uInt16& rnExtSheet, sal_uInt16& rnExtName,
                            const String& rName ) = 0;
    /** Derived classes find or insert an EXTERNNAME record for DDE links. */
    virtual bool        InsertDde(
                            sal_uInt16& rnExtSheet, sal_uInt16& rnExtName,
                            const String& rApplic, const String& rTopic, const String& rItem ) = 0;

    /** Derived classes write the entire link table to the passed stream. */
    virtual void        Save( XclExpStream& rStrm ) = 0;

protected:
    explicit            XclExpLinkManagerImpl( const XclExpRoot& rRoot );
};

// ----------------------------------------------------------------------------

/** Implementation of the link manager for BIFF5/BIFF7. */
class XclExpLinkManagerImpl5 : public XclExpLinkManagerImpl
{
public:
    explicit            XclExpLinkManagerImpl5( const XclExpRoot& rRoot );

    virtual void        FindExtSheet( sal_uInt16& rnExtSheet,
                            sal_uInt16& rnFirstXclTab, sal_uInt16& rnLastXclTab,
                            SCTAB nFirstScTab, SCTAB nLastScTab,
                            XclExpRefLogEntry* pRefLogEntry );
    virtual sal_uInt16  FindExtSheet( sal_Unicode cCode );

    virtual void        StoreCellRange( const SingleRefData& rRef1, const SingleRefData& rRef2 );

    virtual bool        InsertAddIn(
                            sal_uInt16& rnExtSheet, sal_uInt16& rnExtName,
                            const String& rName );
    virtual bool        InsertDde(
                            sal_uInt16& rnExtSheet, sal_uInt16& rnExtName,
                            const String& rApplic, const String& rTopic, const String& rItem );

    virtual void        Save( XclExpStream& rStrm );

private:
    typedef XclExpRecordList< XclExpExternSheet >   XclExpExtSheetList;
    typedef XclExpExtSheetList::RecordRefType       XclExpExtSheetRef;
    typedef ::std::map< SCTAB, sal_uInt16 >         XclExpIntTabMap;
    typedef ::std::map< sal_Unicode, sal_uInt16 >   XclExpCodeMap;

private:
    /** Returns the number of EXTERNSHEET records. */
    sal_uInt16          GetExtSheetCount() const;

    /** Appends an internal EXTERNSHEET record and returns the one-based index. */
    sal_uInt16          AppendInternal( XclExpExtSheetRef xExtSheet );
    /** Creates all EXTERNSHEET records for internal sheets on first call. */
    void                CreateInternal();

    /** Returns the specified internal EXTERNSHEET record. */
    XclExpExtSheetRef   GetInternal( sal_uInt16 nExtSheet );
    /** Returns the EXTERNSHEET index of an internal Calc sheet, or a deleted reference. */
    XclExpExtSheetRef   FindInternal( sal_uInt16& rnExtSheet, sal_uInt16& rnXclTab, SCTAB nScTab );
    /** Finds or creates the EXTERNSHEET index of an internal special EXTERNSHEET. */
    XclExpExtSheetRef   FindInternal( sal_uInt16& rnExtSheet, sal_Unicode cCode );

private:
    XclExpExtSheetList  maExtSheetList;     /// List with EXTERNSHEET records.
    XclExpIntTabMap     maIntTabMap;        /// Maps internal Calc sheets to EXTERNSHEET records.
    XclExpCodeMap       maCodeMap;          /// Maps special external codes to EXTERNSHEET records.
};

// ----------------------------------------------------------------------------

/** Implementation of the link manager for BIFF8. */
class XclExpLinkManagerImpl8 : public XclExpLinkManagerImpl
{
public:
    explicit            XclExpLinkManagerImpl8( const XclExpRoot& rRoot );

    virtual void        FindExtSheet( sal_uInt16& rnExtSheet,
                            sal_uInt16& rnFirstXclTab, sal_uInt16& rnLastXclTab,
                            SCTAB nFirstScTab, SCTAB nLastScTab,
                            XclExpRefLogEntry* pRefLogEntry );
    virtual sal_uInt16  FindExtSheet( sal_Unicode cCode );

    virtual void        StoreCellRange( const SingleRefData& rRef1, const SingleRefData& rRef2 );

    virtual bool        InsertAddIn(
                            sal_uInt16& rnExtSheet, sal_uInt16& rnExtName,
                            const String& rName );
    virtual bool        InsertDde(
                            sal_uInt16& rnExtSheet, sal_uInt16& rnExtName,
                            const String& rApplic, const String& rTopic, const String& rItem );

    virtual void        Save( XclExpStream& rStrm );

private:
    /** Searches for or inserts a new XTI structure.
        @return  The 0-based list index of the XTI structure. */
    sal_uInt16          InsertXti( const XclExpXti& rXti );

private:
    typedef ::std::vector< XclExpXti > XclExpXtiVec;

    XclExpSupbookBuffer maSBBuffer;     /// List of all SUPBOOK records.
    XclExpXtiVec        maXtiVec;       /// List of XTI structures for the EXTERNSHEET record.
};

// ============================================================================
// *** Implementation ***
// ============================================================================

// Excel sheet indexes ========================================================

const sal_uInt8 EXC_TABBUF_IGNORE   = 0x01;     /// Sheet will be ignored completely.
const sal_uInt8 EXC_TABBUF_EXTERN   = 0x02;     /// Sheet is linked externally.
const sal_uInt8 EXC_TABBUF_SKIPMASK = 0x0F;     /// Sheet will be skipped, if any flag is set.
const sal_uInt8 EXC_TABBUF_VISIBLE  = 0x10;     /// Sheet is visible.
const sal_uInt8 EXC_TABBUF_SELECTED = 0x20;     /// Sheet is selected.
const sal_uInt8 EXC_TABBUF_MIRRORED = 0x40;     /// Sheet is mirrored (right-to-left).

// ----------------------------------------------------------------------------

XclExpTabInfo::XclExpTabInfo( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    mnScCnt( 0 ),
    mnXclCnt( 0 ),
    mnXclExtCnt( 0 ),
    mnXclSelCnt( 0 ),
    mnDisplXclTab( 0 ),
    mnFirstVisXclTab( 0 )
{
    ScDocument& rDoc = GetDoc();
    ScExtDocOptions& rDocOpt = GetExtDocOptions();

    mnScCnt = rDoc.GetTableCount();

    SCTAB nScTab;
    SCTAB nFirstVisScTab = SCTAB_INVALID;   // first visible sheet
    SCTAB nFirstExpScTab = SCTAB_INVALID;   // first exported sheet

    // --- initialize the flags in the index buffer ---

    maTabInfoVec.resize( mnScCnt );
    for( nScTab = 0; nScTab < mnScCnt; ++nScTab )
    {
        // ignored sheets (skipped by export, with invalid Excel sheet index)
        if( rDoc.IsScenario( nScTab ) )
        {
            SetFlag( nScTab, EXC_TABBUF_IGNORE );
        }

        // external sheets (skipped, but with valid Excel sheet index for ref's)
        else if( rDoc.GetLinkMode( nScTab ) == SC_LINK_VALUE )
        {
            SetFlag( nScTab, EXC_TABBUF_EXTERN );
        }

        // exported sheets
        else
        {
            // sheet name
            rDoc.GetName( nScTab, maTabInfoVec[ nScTab ].maScName );

            // remember first exported sheet
            if( nFirstExpScTab == SCTAB_INVALID )
               nFirstExpScTab = nScTab;
            // remember first visible exported sheet
            if( (nFirstVisScTab == SCTAB_INVALID) && rDoc.IsVisible( nScTab ) )
               nFirstVisScTab = nScTab;

            // sheet visible (only exported sheets)
            SetFlag( nScTab, EXC_TABBUF_VISIBLE, rDoc.IsVisible( nScTab ) );

            // sheet selected (only exported sheets)
            if( const ScExtTabSettings* pTabSett = rDocOpt.GetTabSettings( nScTab ) )
                SetFlag( nScTab, EXC_TABBUF_SELECTED, pTabSett->mbSelected );

            // sheet mirrored (only exported sheets)
            SetFlag( nScTab, EXC_TABBUF_MIRRORED, rDoc.IsLayoutRTL( nScTab ) );
        }
    }

    // --- visible/selected sheets ---

    SCTAB nDisplScTab = rDocOpt.GetDocSettings().mnDisplTab;

    // #112908# find first visible exported sheet
    if( (nFirstVisScTab == SCTAB_INVALID) || !IsExportTab( nFirstVisScTab ) )
    {
        // no exportable visible sheet -> use first exportable sheet
        nFirstVisScTab = nFirstExpScTab;
        if( (nFirstVisScTab == SCTAB_INVALID) || !IsExportTab( nFirstVisScTab ) )
        {
            // no exportable sheet at all -> use active sheet and export it
            nFirstVisScTab = nDisplScTab;
            SetFlag( nFirstVisScTab, EXC_TABBUF_SKIPMASK, false ); // clear skip flags
        }
        SetFlag( nFirstVisScTab, EXC_TABBUF_VISIBLE ); // must be visible, even if originally hidden
    }

    // find currently displayed sheet
    if( !IsExportTab( nDisplScTab ) )   // selected sheet not exported (i.e. scenario) -> use first visible
        nDisplScTab = nFirstVisScTab;
    SetFlag( nDisplScTab, EXC_TABBUF_VISIBLE | EXC_TABBUF_SELECTED );

    // number of selected sheets
    for( nScTab = 0; nScTab < mnScCnt; ++nScTab )
        if( IsSelectedTab( nScTab ) )
            ++mnXclSelCnt;

    // --- calculate resulting Excel sheet indexes ---

    CalcXclIndexes();
    mnFirstVisXclTab = GetXclTab( nFirstVisScTab );
    mnDisplXclTab = GetXclTab( nDisplScTab );

    // --- sorted vectors for index lookup ---

    CalcSortedIndexes();
}

bool XclExpTabInfo::IsExportTab( SCTAB nScTab ) const
{
    /*  Check sheet index before to avoid assertion in GetFlag(). */
    return (nScTab < mnScCnt) && !GetFlag( nScTab, EXC_TABBUF_SKIPMASK );
}

bool XclExpTabInfo::IsExternalTab( SCTAB nScTab ) const
{
    /*  Check sheet index before to avoid assertion (called from formula
        compiler also for deleted references). */
    return (nScTab < mnScCnt) && GetFlag( nScTab, EXC_TABBUF_EXTERN );
}

bool XclExpTabInfo::IsVisibleTab( SCTAB nScTab ) const
{
    return GetFlag( nScTab, EXC_TABBUF_VISIBLE );
}

bool XclExpTabInfo::IsSelectedTab( SCTAB nScTab ) const
{
    return GetFlag( nScTab, EXC_TABBUF_SELECTED );
}

bool XclExpTabInfo::IsDisplayedTab( SCTAB nScTab ) const
{
    DBG_ASSERT( nScTab < mnScCnt, "XclExpTabInfo::IsActiveTab - sheet out of range" );
    return GetXclTab( nScTab ) == mnDisplXclTab;
}

bool XclExpTabInfo::IsMirroredTab( SCTAB nScTab ) const
{
    return GetFlag( nScTab, EXC_TABBUF_MIRRORED );
}

const String& XclExpTabInfo::GetScTabName( SCTAB nScTab ) const
{
    DBG_ASSERT( nScTab < mnScCnt, "XclExpTabInfo::IsActiveTab - sheet out of range" );
    return (nScTab < mnScCnt) ? maTabInfoVec[ nScTab ].maScName : EMPTY_STRING;
}

sal_uInt16 XclExpTabInfo::GetXclTab( SCTAB nScTab ) const
{
    return (nScTab < mnScCnt) ? maTabInfoVec[ nScTab ].mnXclTab : EXC_TAB_DELETED;
}

SCTAB XclExpTabInfo::GetRealScTab( SCTAB nSortedScTab ) const
{
    DBG_ASSERT( nSortedScTab < mnScCnt, "XclExpTabInfo::GetRealScTab - sheet out of range" );
    return (nSortedScTab < mnScCnt) ? maFromSortedVec[ nSortedScTab ] : SCTAB_INVALID;
}

SCTAB XclExpTabInfo::GetSortedScTab( SCTAB nScTab ) const
{
    DBG_ASSERT( nScTab < mnScCnt, "XclExpTabInfo::GetSortedScTab - sheet out of range" );
    return (nScTab < mnScCnt) ? maToSortedVec[ nScTab ] : SCTAB_INVALID;
}

bool XclExpTabInfo::GetFlag( SCTAB nScTab, sal_uInt8 nFlags ) const
{
    DBG_ASSERT( nScTab < mnScCnt, "XclExpTabInfo::GetFlag - sheet out of range" );
    return (nScTab < mnScCnt) && ::get_flag( maTabInfoVec[ nScTab ].mnFlags, nFlags );
}

void XclExpTabInfo::SetFlag( SCTAB nScTab, sal_uInt8 nFlags, bool bSet )
{
    DBG_ASSERT( nScTab < mnScCnt, "XclExpTabInfo::SetFlag - sheet out of range" );
    if( nScTab < mnScCnt )
        ::set_flag( maTabInfoVec[ nScTab ].mnFlags, nFlags, bSet );
}

void XclExpTabInfo::CalcXclIndexes()
{
    sal_uInt16 nXclTab = 0;
    SCTAB nScTab = 0;

    // --- pass 1: process regular sheets ---
    for( nScTab = 0; nScTab < mnScCnt; ++nScTab )
    {
        if( IsExportTab( nScTab ) )
        {
            maTabInfoVec[ nScTab ].mnXclTab = nXclTab;
            ++nXclTab;
        }
        else
            maTabInfoVec[ nScTab ].mnXclTab = EXC_TAB_DELETED;
    }
    mnXclCnt = nXclTab;

    // --- pass 2: process external sheets (nXclTab continues) ---
    for( nScTab = 0; nScTab < mnScCnt; ++nScTab )
    {
        if( IsExternalTab( nScTab ) )
        {
            maTabInfoVec[ nScTab ].mnXclTab = nXclTab;
            ++nXclTab;
            ++mnXclExtCnt;
        }
    }

    // result: first occur all exported sheets, followed by all external sheets
}

typedef ::std::pair< String, SCTAB >    XclExpTabName;
typedef ::std::vector< XclExpTabName >  XclExpTabNameVec;

inline bool operator<( const XclExpTabName& rArg1, const XclExpTabName& rArg2 )
{
    // compare the sheet names only
    return ScGlobal::pCollator->compareString( rArg1.first, rArg2.first ) == COMPARE_LESS;
}

void XclExpTabInfo::CalcSortedIndexes()
{
    ScDocument& rDoc = GetDoc();
    XclExpTabNameVec aVec( mnScCnt );
    SCTAB nScTab;

    // fill with sheet names
    for( nScTab = 0; nScTab < mnScCnt; ++nScTab )
    {
        rDoc.GetName( nScTab, aVec[ nScTab ].first );
        aVec[ nScTab ].second = nScTab;
    }
    ::std::sort( aVec.begin(), aVec.end() );

    // fill index vectors from sorted sheet name vector
    maFromSortedVec.resize( mnScCnt );
    maToSortedVec.resize( mnScCnt );
    for( nScTab = 0; nScTab < mnScCnt; ++nScTab )
    {
        maFromSortedVec[ nScTab ] = aVec[ nScTab ].second;
        maToSortedVec[ aVec[ nScTab ].second ] = nScTab;
    }
}

// External names =============================================================

XclExpExtNameBase::XclExpExtNameBase(
        const XclExpRoot& rRoot, const String& rName, sal_uInt16 nFlags ) :
    XclExpRecord( EXC_ID_EXTERNNAME ),
    XclExpRoot( rRoot ),
    maName( rName ),
    mxName( XclExpStringHelper::CreateString( rRoot, rName, EXC_STR_8BITLENGTH ) ),
    mnFlags( nFlags )
{
    DBG_ASSERT( maName.Len() <= 255, "XclExpExtNameBase::XclExpExtNameBase - string too long" );
    SetRecSize( 6 + mxName->GetSize() );
}

XclExpExtNameBase::~XclExpExtNameBase()
{
}

void XclExpExtNameBase::WriteBody( XclExpStream& rStrm )
{
    rStrm   << mnFlags
            << sal_uInt32( 0 )
            << *mxName;
    WriteAddData( rStrm );
}

void XclExpExtNameBase::WriteAddData( XclExpStream& rStrm )
{
}

// ----------------------------------------------------------------------------

XclExpExtNameAddIn::XclExpExtNameAddIn( const XclExpRoot& rRoot, const String& rName ) :
    XclExpExtNameBase( rRoot, rName )
{
    AddRecSize( 4 );
}

void XclExpExtNameAddIn::WriteAddData( XclExpStream& rStrm )
{
    // write a #REF! error formula
    rStrm << sal_uInt16( 2 ) << EXC_TOKID_ERR << EXC_ERR_REF;
}

// ----------------------------------------------------------------------------

XclExpExtNameDde::XclExpExtNameDde( const XclExpRoot& rRoot,
        const String& rName, sal_uInt16 nFlags, const ScMatrix* pResults ) :
    XclExpExtNameBase( rRoot, rName, nFlags )
{
    if( pResults )
    {
        mxMatrix.reset( new XclExpCachedMatrix( *pResults ) );
        AddRecSize( mxMatrix->GetSize() );
    }
}

void XclExpExtNameDde::WriteAddData( XclExpStream& rStrm )
{
    if( mxMatrix.is() )
        mxMatrix->Save( rStrm );
}

// List of external names =====================================================

XclExpExtNameBuffer::XclExpExtNameBuffer( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot )
{
}

sal_uInt16 XclExpExtNameBuffer::InsertAddIn( const String& rName )
{
    sal_uInt16 nIndex = GetIndex( rName );
    return nIndex ? nIndex : AppendNew( new XclExpExtNameAddIn( GetRoot(), rName ) );
}

sal_uInt16 XclExpExtNameBuffer::InsertDde(
        const String& rApplic, const String& rTopic, const String& rItem )
{
    sal_uInt16 nIndex = GetIndex( rItem );
    if( nIndex == 0 )
    {
        USHORT nPos;
        if( GetDoc().FindDdeLink( rApplic, rTopic, rItem, SC_DDE_IGNOREMODE, nPos ) )
        {
            // create the leading 'StdDocumentName' EXTERNNAME record
            if( maNameList.Empty() )
                AppendNew( new XclExpExtNameDde(
                    GetRoot(), CREATE_STRING( "StdDocumentName" ), EXC_EXTN_EXPDDE_STDDOC ) );

            // try to find DDE result array, but create EXTERNNAME record without them too
            const ScMatrix* pScMatrix = GetDoc().GetDdeLinkResultMatrix( nPos );
            nIndex = AppendNew( new XclExpExtNameDde( GetRoot(), rItem, EXC_EXTN_EXPDDE, pScMatrix ) );
        }
    }
    return nIndex;
}

void XclExpExtNameBuffer::Save( XclExpStream& rStrm )
{
    maNameList.Save( rStrm );
}

sal_uInt16 XclExpExtNameBuffer::GetIndex( const String& rName ) const
{
    for( size_t nPos = 0, nSize = maNameList.Size(); nPos < nSize; ++nPos )
        if( maNameList.GetRecord( nPos )->GetName() == rName )
            return static_cast< sal_uInt16 >( nPos + 1 );
    return 0;
}

sal_uInt16 XclExpExtNameBuffer::AppendNew( XclExpExtNameBase* pExtName )
{
    XclExpExtNameRef xExtName( pExtName );
    size_t nSize = maNameList.Size();
    if( nSize < 0x7FFF )
    {
        maNameList.AppendRecord( xExtName );
        return static_cast< sal_uInt16 >( nSize + 1 );
    }
    return 0;
}

// Cached external cells ======================================================

XclExpCrn::XclExpCrn( SCCOL nScCol, SCROW nScRow, sal_uInt8 nId, sal_uInt32 nAddLen ) :
    XclExpRecord( EXC_ID_CRN, 5 + nAddLen ),
    mnXclCol( static_cast< sal_uInt16 >( nScCol ) ),
    mnXclRow( static_cast< sal_uInt16 >( nScRow ) ),
    mnId( nId )
{
}

void XclExpCrn::WriteBody( XclExpStream& rStrm )
{
    rStrm   << static_cast< sal_uInt8 >( mnXclCol )
            << static_cast< sal_uInt8 >( mnXclCol )
            << mnXclRow
            << mnId;
    WriteAddData( rStrm );
}

// ----------------------------------------------------------------------------

XclExpCrnDouble::XclExpCrnDouble( SCCOL nScCol, SCROW nScRow, double fVal ) :
    XclExpCrn( nScCol, nScRow, EXC_CACHEDVAL_DOUBLE, 8 ),
    mfVal( fVal )
{
}

void XclExpCrnDouble::WriteAddData( XclExpStream& rStrm )
{
    rStrm << mfVal;
}

// ----------------------------------------------------------------------------

XclExpCrnString::XclExpCrnString( SCCOL nScCol, SCROW nScRow, const String& rText ) :
    XclExpCrn( nScCol, nScRow, EXC_CACHEDVAL_STRING ),
    maText( rText )
{
    // set correct size after maText is initialized
    AddRecSize( maText.GetSize() );
}

void XclExpCrnString::WriteAddData( XclExpStream& rStrm )
{
    rStrm << maText;
}

// ----------------------------------------------------------------------------

XclExpCrnBool::XclExpCrnBool( SCCOL nScCol, SCROW nScRow, bool bBoolVal ) :
    XclExpCrn( nScCol, nScRow, EXC_CACHEDVAL_BOOL, 8 ),
    mnBool( bBoolVal ? 1 : 0 )
{
}

void XclExpCrnBool::WriteAddData( XclExpStream& rStrm )
{
    rStrm << mnBool;
    rStrm.WriteZeroBytes( 6 );
}

// Cached cells of a sheet ====================================================

XclExpXct::XclExpXct( const String& rTabName, sal_uInt16 nSBTab ) :
    XclExpRecord( EXC_ID_XCT, 4 ),
    maTabName( rTabName ),
    mnSBTab( nSBTab )
{
}

void XclExpXct::StoreCellRange( const XclExpRoot& rRoot, const ScRange& rRange )
{
    ScDocument& rDoc = rRoot.GetDoc();
    SvNumberFormatter& rFormatter = rRoot.GetFormatter();
    SCTAB nScTab = rRange.aStart.Tab();
    SCCOL nScLastCol = rRange.aEnd.Col();
    SCROW nScLastRow = rRange.aEnd.Row();

    for( SCROW nScRow = rRange.aStart.Row(); nScRow <= nScLastRow; ++nScRow )
    {
        for( SCCOL nScCol = rRange.aStart.Col(); nScCol <= nScLastCol; ++nScCol )
        {
            if( !maUsedCells.IsCellMarked( nScCol, nScRow, TRUE ) )
            {
                XclExpCrnRef xCrn;
                if( rDoc.HasValueData( nScCol, nScRow, nScTab ) )
                {
                    ScAddress   aAddr( nScCol, nScRow, nScTab );
                    double      fVal    = rDoc.GetValue( aAddr );
                    ULONG       nFormat = rDoc.GetNumberFormat( aAddr );
                    short       nType   = rFormatter.GetType( nFormat );
                    bool        bIsBool = (nType == NUMBERFORMAT_LOGICAL);

                    if( !bIsBool && ((nFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0) &&
                            (rDoc.GetCellType( aAddr ) == CELLTYPE_FORMULA) )
                        if( ScFormulaCell* pCell = static_cast< ScFormulaCell* >( rDoc.GetCell( aAddr ) ) )
                            bIsBool = (pCell->GetFormatType() == NUMBERFORMAT_LOGICAL);

                    if( bIsBool && ((fVal == 0.0) || (fVal == 1.0)) )
                        xCrn.reset( new XclExpCrnBool( nScCol, nScRow, (fVal == 1.0) ) );
                    else
                        xCrn.reset( new XclExpCrnDouble( nScCol, nScRow, fVal ) );
                }
                else
                {
                    String aText;
                    rDoc.GetString( nScCol, nScRow, nScTab, aText );
                    xCrn.reset( new XclExpCrnString( nScCol, nScRow, aText ) );
                }
                maCrnList.AppendRecord( xCrn );
            }
        }
    }

    maUsedCells.SetMultiMarkArea( rRange );
}

void XclExpXct::Save( XclExpStream& rStrm )
{
    XclExpRecord::Save( rStrm );
    maCrnList.Save( rStrm );
}

void XclExpXct::WriteBody( XclExpStream& rStrm )
{
    sal_uInt16 nCount = ulimit_cast< sal_uInt16 >( maCrnList.Size() );
    rStrm << nCount << mnSBTab;
}

// External documents (EXTERNSHEET/SUPBOOK), base class =======================

XclExpExternSheetBase::XclExpExternSheetBase( const XclExpRoot& rRoot, sal_uInt16 nRecId, sal_uInt32 nRecSize ) :
    XclExpRecord( nRecId, nRecSize ),
    XclExpRoot( rRoot )
{
}

XclExpExtNameBuffer& XclExpExternSheetBase::GetExtNameBuffer()
{
    if( !mxExtNameBfr )
        mxExtNameBfr.reset( new XclExpExtNameBuffer( GetRoot() ) );
    return *mxExtNameBfr;
}

void XclExpExternSheetBase::WriteExtNameBuffer( XclExpStream& rStrm )
{
    if( mxExtNameBfr.is() )
        mxExtNameBfr->Save( rStrm );
}

// External documents (EXTERNSHEET, BIFF5/BIFF7) ==============================

XclExpExternSheet::XclExpExternSheet( const XclExpRoot& rRoot, sal_Unicode cCode ) :
    XclExpExternSheetBase( rRoot, EXC_ID_EXTERNSHEET )
{
    Init( String( cCode ) );
}

XclExpExternSheet::XclExpExternSheet( const XclExpRoot& rRoot, const String& rTabName ) :
    XclExpExternSheetBase( rRoot, EXC_ID_EXTERNSHEET )
{
    // reference to own sheet: \03<sheetname>
    Init( String( EXC_EXTSH_TABNAME ).Append( rTabName ) );
}

void XclExpExternSheet::Save( XclExpStream& rStrm )
{
    // EXTERNSHEET record
    XclExpRecord::Save( rStrm );
    // EXTERNNAME records
    WriteExtNameBuffer( rStrm );
}

void XclExpExternSheet::Init( const String& rEncUrl )
{
    DBG_ASSERT_BIFF( GetBiff() <= EXC_BIFF5 );
    maTabName.AssignByte( rEncUrl, GetCharSet(), EXC_STR_8BITLENGTH );
    SetRecSize( maTabName.GetSize() );
}

sal_uInt16 XclExpExternSheet::InsertAddIn( const String& rName )
{
    return GetExtNameBuffer().InsertAddIn( rName );
}

void XclExpExternSheet::WriteBody( XclExpStream& rStrm )
{
    sal_uInt8 nNameSize = static_cast< sal_uInt8 >( maTabName.Len() );
    // special case: reference to own sheet (starting with '\03') needs wrong string length
    if( maTabName.GetChar( 0 ) == EXC_EXTSH_TABNAME )
        --nNameSize;
    rStrm << nNameSize;
    maTabName.WriteBuffer( rStrm );
}

// External document (SUPBOOK, BIFF8) =========================================

XclExpSupbook::XclExpSupbook( const XclExpRoot& rRoot, sal_uInt16 nXclTabCount ) :
    XclExpExternSheetBase( rRoot, EXC_ID_SUPBOOK, 4 ),
    meType( EXC_SBTYPE_SELF ),
    mnXclTabCount( nXclTabCount )
{
}

XclExpSupbook::XclExpSupbook( const XclExpRoot& rRoot ) :
    XclExpExternSheetBase( rRoot, EXC_ID_SUPBOOK, 4 ),
    meType( EXC_SBTYPE_ADDIN ),
    mnXclTabCount( 1 )
{
}

XclExpSupbook::XclExpSupbook( const XclExpRoot& rRoot, const String& rUrl ) :
    XclExpExternSheetBase( rRoot, EXC_ID_SUPBOOK ),
    maUrl( rUrl ),
    maUrlEncoded( XclExpUrlHelper::EncodeUrl( rRoot, rUrl ) ),
    meType( EXC_SBTYPE_EXTERN ),
    mnXclTabCount( 0 )
{
    SetRecSize( 2 + maUrlEncoded.GetSize() );
}

XclExpSupbook::XclExpSupbook( const XclExpRoot& rRoot, const String& rApplic, const String& rTopic ) :
    XclExpExternSheetBase( rRoot, EXC_ID_SUPBOOK, 4 ),
    maUrl( rApplic ),
    maDdeTopic( rTopic ),
    maUrlEncoded( XclExpUrlHelper::EncodeDde( rApplic, rTopic ) ),
    meType( EXC_SBTYPE_SPECIAL ),
    mnXclTabCount( 0 )
{
    SetRecSize( 2 + maUrlEncoded.GetSize() );
}

bool XclExpSupbook::IsUrlLink( const String& rUrl ) const
{
    return (meType == EXC_SBTYPE_EXTERN) && (maUrl == rUrl);
}

bool XclExpSupbook::IsDdeLink( const String& rApplic, const String& rTopic ) const
{
    return (meType == EXC_SBTYPE_SPECIAL) && (maUrl == rApplic) && (maDdeTopic == rTopic);
}

void XclExpSupbook::FillRefLogEntry( XclExpRefLogEntry& rRefLogEntry,
        sal_uInt16 nFirstSBTab, sal_uInt16 nLastSBTab ) const
{
    rRefLogEntry.mpUrl = maUrlEncoded.IsEmpty() ? 0 : &maUrlEncoded;
    rRefLogEntry.mpFirstTab = GetTabName( nFirstSBTab );
    rRefLogEntry.mpLastTab = GetTabName( nLastSBTab );
}

void XclExpSupbook::StoreCellRange( const ScRange& rRange, sal_uInt16 nSBTab )
{
    XclExpXctRef xXct = maXctList.GetRecord( nSBTab );
    if( xXct.is() )
        xXct->StoreCellRange( GetRoot(), rRange );
}

sal_uInt16 XclExpSupbook::InsertTabName( const String& rTabName )
{
    DBG_ASSERT( meType == EXC_SBTYPE_EXTERN, "XclExpSupbook::InsertTabName - don't insert sheet names here" );
    sal_uInt16 nSBTab = ulimit_cast< sal_uInt16 >( maXctList.Size() );
    XclExpXctRef xXct( new XclExpXct( rTabName, nSBTab ) );
    AddRecSize( xXct->GetTabName().GetSize() );
    maXctList.AppendRecord( xXct );
    return nSBTab;
}

sal_uInt16 XclExpSupbook::InsertAddIn( const String& rName )
{
    return GetExtNameBuffer().InsertAddIn( rName );
}

sal_uInt16 XclExpSupbook::InsertDde( const String& rItem )
{
    return GetExtNameBuffer().InsertDde( maUrl, maDdeTopic, rItem );
}

void XclExpSupbook::Save( XclExpStream& rStrm )
{
    // SUPBOOK record
    XclExpRecord::Save( rStrm );
    // XCT record, CRN records
    maXctList.Save( rStrm );
    // EXTERNNAME records
    WriteExtNameBuffer( rStrm );
}

const XclExpString* XclExpSupbook::GetTabName( sal_uInt16 nSBTab ) const
{
    XclExpXctRef xXct = maXctList.GetRecord( nSBTab );
    return xXct.is() ? &xXct->GetTabName() : 0;
}

void XclExpSupbook::WriteBody( XclExpStream& rStrm )
{
    switch( meType )
    {
        case EXC_SBTYPE_SELF:
            rStrm << mnXclTabCount << EXC_SUPB_SELF;
        break;
        case EXC_SBTYPE_EXTERN:
        case EXC_SBTYPE_SPECIAL:
        {
            sal_uInt16 nCount = ulimit_cast< sal_uInt16 >( maXctList.Size() );
            rStrm << nCount << maUrlEncoded;

            for( size_t nPos = 0, nSize = maXctList.Size(); nPos < nSize; ++nPos )
                rStrm << maXctList.GetRecord( nPos )->GetTabName();
        }
        break;
        case EXC_SBTYPE_ADDIN:
            rStrm << mnXclTabCount << EXC_SUPB_ADDIN;
        break;
        default:
            DBG_ERRORFILE( "XclExpSupbook::WriteBody - unknown SUPBOOK type" );
    }
}

// All SUPBOOKS in a document =================================================

XclExpSupbookBuffer::XclExpSupbookBuffer( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    mnOwnDocSB( SAL_MAX_UINT16 ),
    mnAddInSB( SAL_MAX_UINT16 )
{
    XclExpTabInfo& rTabInfo = GetTabInfo();
    sal_uInt16 nXclCnt = rTabInfo.GetXclTabCount();
    sal_uInt16 nCodeCnt = static_cast< sal_uInt16 >( GetExtDocOptions().GetCodeNameCount() );
    size_t nCount = nXclCnt + rTabInfo.GetXclExtTabCount();

    DBG_ASSERT( nCount > 0, "XclExpSupbookBuffer::XclExpSupbookBuffer - no sheets to export" );
    if( nCount )
    {
        maSBIndexVec.resize( nCount );

        // self-ref SUPBOOK first of list
        XclExpSupbookRef xSupbook( new XclExpSupbook( GetRoot(), ::std::max( nXclCnt, nCodeCnt ) ) );
        mnOwnDocSB = Append( xSupbook );
        for( sal_uInt16 nXclTab = 0; nXclTab < nXclCnt; ++nXclTab )
            maSBIndexVec[ nXclTab ].Set( mnOwnDocSB, nXclTab );

        // add SUPBOOKs with external references
        for( SCTAB nScTab = 0, nScCnt = rTabInfo.GetScTabCount(); nScTab < nScCnt; ++nScTab )
            if( rTabInfo.IsExternalTab( nScTab ) )
                AddExtSupbook( nScTab );
    }
}

XclExpXti XclExpSupbookBuffer::GetXti( sal_uInt16 nFirstXclTab, sal_uInt16 nLastXclTab,
        XclExpRefLogEntry* pRefLogEntry ) const
{
    XclExpXti aXti;
    size_t nSize = maSBIndexVec.size();
    if( (nFirstXclTab < nSize) && (nLastXclTab < nSize) )
    {
        // index of the SUPBOOK record
        aXti.mnSupbook = maSBIndexVec[ nFirstXclTab ].mnSupbook;

        // all sheets in the same supbook?
        bool bSameSB = true;
        for( sal_uInt16 nXclTab = nFirstXclTab + 1; bSameSB && (nXclTab <= nLastXclTab); ++nXclTab )
        {
            bSameSB = maSBIndexVec[ nXclTab ].mnSupbook == aXti.mnSupbook;
            if( !bSameSB )
                nLastXclTab = nXclTab - 1;
        }
        aXti.mnFirstSBTab = maSBIndexVec[ nFirstXclTab ].mnSBTab;
        aXti.mnLastSBTab = maSBIndexVec[ nLastXclTab ].mnSBTab;

        // fill external reference log entry (for change tracking)
        if( pRefLogEntry )
        {
            pRefLogEntry->mnFirstXclTab = nFirstXclTab;
            pRefLogEntry->mnLastXclTab = nLastXclTab;
            XclExpSupbookRef xSupbook = maSupbookList.GetRecord( aXti.mnSupbook );
            if( xSupbook.is() )
                xSupbook->FillRefLogEntry( *pRefLogEntry, aXti.mnFirstSBTab, aXti.mnLastSBTab );
        }
    }
    else
    {
        // special range, i.e. for deleted sheets or add-ins
        aXti.mnSupbook = mnOwnDocSB;
        aXti.mnFirstSBTab = nFirstXclTab;
        aXti.mnLastSBTab = nLastXclTab;
    }

    return aXti;
}

void XclExpSupbookBuffer::StoreCellRange( const ScRange& rRange )
{
    sal_uInt16 nXclTab = GetTabInfo().GetXclTab( rRange.aStart.Tab() );
    if( nXclTab < maSBIndexVec.size() )
    {
        const XclExpSBIndex& rSBIndex = maSBIndexVec[ nXclTab ];
        XclExpSupbookRef xSupbook = maSupbookList.GetRecord( rSBIndex.mnSupbook );
        DBG_ASSERT( xSupbook.is(), "XclExpSupbookBuffer::StoreCellRange - missing SUPBOOK record" );
        if( xSupbook.is() )
            xSupbook->StoreCellRange( rRange, rSBIndex.mnSBTab );
    }
}

bool XclExpSupbookBuffer::InsertAddIn(
        sal_uInt16& rnSupbook, sal_uInt16& rnExtName, const String& rName )
{
    XclExpSupbookRef xSupbook;
    if( mnAddInSB == SAL_MAX_UINT16 )
    {
        xSupbook.reset( new XclExpSupbook( GetRoot() ) );
        mnAddInSB = Append( xSupbook );
    }
    else
        xSupbook = maSupbookList.GetRecord( mnAddInSB );
    DBG_ASSERT( xSupbook.is(), "XclExpSupbookBuffer::InsertAddin - missing add-in supbook" );
    rnSupbook = mnAddInSB;
    rnExtName = xSupbook->InsertAddIn( rName );
    return rnExtName > 0;
}

bool XclExpSupbookBuffer::InsertDde(
        sal_uInt16& rnSupbook, sal_uInt16& rnExtName,
        const String& rApplic, const String& rTopic, const String& rItem )
{
    XclExpSupbookRef xSupbook;
    if( !GetSupbookDde( xSupbook, rnSupbook, rApplic, rTopic ) )
    {
        xSupbook.reset( new XclExpSupbook( GetRoot(), rApplic, rTopic ) );
        rnSupbook = Append( xSupbook );
    }
    rnExtName = xSupbook->InsertDde( rItem );
    return rnExtName > 0;
}

void XclExpSupbookBuffer::Save( XclExpStream& rStrm )
{
    maSupbookList.Save( rStrm );
}

bool XclExpSupbookBuffer::GetSupbookUrl(
        XclExpSupbookRef& rxSupbook, sal_uInt16& rnIndex, const String& rUrl ) const
{
    for( size_t nPos = 0, nSize = maSupbookList.Size(); nPos < nSize; ++nPos )
    {
        rxSupbook = maSupbookList.GetRecord( nPos );
        if( rxSupbook->IsUrlLink( rUrl ) )
        {
            rnIndex = ulimit_cast< sal_uInt16 >( nPos );
            return true;
        }
    }
    return false;
}

bool XclExpSupbookBuffer::GetSupbookDde( XclExpSupbookRef& rxSupbook,
        sal_uInt16& rnIndex, const String& rApplic, const String& rTopic ) const
{
    for( size_t nPos = 0, nSize = maSupbookList.Size(); nPos < nSize; ++nPos )
    {
        rxSupbook = maSupbookList.GetRecord( nPos );
        if( rxSupbook->IsDdeLink( rApplic, rTopic ) )
        {
            rnIndex = ulimit_cast< sal_uInt16 >( nPos );
            return true;
        }
    }
    return false;
}

sal_uInt16 XclExpSupbookBuffer::Append( XclExpSupbookRef xSupbook )
{
    maSupbookList.AppendRecord( xSupbook );
    return ulimit_cast< sal_uInt16 >( maSupbookList.Size() - 1 );
}

void XclExpSupbookBuffer::AddExtSupbook( SCTAB nScTab )
{
    sal_uInt16 nXclTab = GetTabInfo().GetXclTab( nScTab );
    DBG_ASSERT( nXclTab < maSBIndexVec.size(), "XclExpSupbookBuffer::AddExtSupbook - out of range" );

    // find ext doc name or append new one, save position in maSBIndexBuffer
    const String& rUrl = GetDoc().GetLinkDoc( nScTab );
    DBG_ASSERT( rUrl.Len(), "XclExpSupbookBuffer::AddExtSupbook - missing external linked sheet" );
    sal_uInt16 nSupbook;
    XclExpSupbookRef xSupbook;
    if( !GetSupbookUrl( xSupbook, nSupbook, rUrl ) )
    {
        xSupbook.reset( new XclExpSupbook( GetRoot(), rUrl ) );
        nSupbook = Append( xSupbook );
    }

    // append new sheet name, save SUPBOOK and sheet position in maSBIndexVec
    maSBIndexVec[ nXclTab ].mnSupbook = nSupbook;
    maSBIndexVec[ nXclTab ].mnSBTab = xSupbook->InsertTabName( GetDoc().GetLinkTab( nScTab ) );
}

// Export link manager ========================================================

XclExpLinkManagerImpl::XclExpLinkManagerImpl( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot )
{
}

// ----------------------------------------------------------------------------

XclExpLinkManagerImpl5::XclExpLinkManagerImpl5( const XclExpRoot& rRoot ) :
    XclExpLinkManagerImpl( rRoot )
{
}

void XclExpLinkManagerImpl5::FindExtSheet(
        sal_uInt16& rnExtSheet, sal_uInt16& rnFirstXclTab, sal_uInt16& rnLastXclTab,
        SCTAB nFirstScTab, SCTAB nLastScTab, XclExpRefLogEntry* pRefLogEntry )
{
    FindInternal( rnExtSheet, rnFirstXclTab, nFirstScTab );
    if( (rnFirstXclTab == EXC_TAB_DELETED) || (nFirstScTab == nLastScTab) )
    {
        rnLastXclTab = rnFirstXclTab;
    }
    else
    {
        sal_uInt16 nDummyExtSheet;
        FindInternal( nDummyExtSheet, rnLastXclTab, nLastScTab );
    }

    DBG_ASSERT( !pRefLogEntry, "XclExpLinkManagerImpl5::FindExtSheet - fill reflog entry not implemented" );
}

sal_uInt16 XclExpLinkManagerImpl5::FindExtSheet( sal_Unicode cCode )
{
    sal_uInt16 nExtSheet;
    FindInternal( nExtSheet, cCode );
    return nExtSheet;
}

void XclExpLinkManagerImpl5::StoreCellRange( const SingleRefData& rRef1, const SingleRefData& rRef2 )
{
    // not implemented
}

bool XclExpLinkManagerImpl5::InsertAddIn(
        sal_uInt16& rnExtSheet, sal_uInt16& rnExtName, const String& rName )
{
    XclExpExtSheetRef xExtSheet = FindInternal( rnExtSheet, EXC_EXTSH_ADDIN );
    if( xExtSheet.is() )
    {
        rnExtName = xExtSheet->InsertAddIn( rName );
        return rnExtName > 0;
    }
    return false;
}

bool XclExpLinkManagerImpl5::InsertDde(
        sal_uInt16& rnExtSheet, sal_uInt16& rnExtName,
        const String& rApplic, const String& rTopic, const String& rItem )
{
    // not implemented
    return false;
}

void XclExpLinkManagerImpl5::Save( XclExpStream& rStrm )
{
    if( sal_uInt16 nExtSheetCount = GetExtSheetCount() )
    {
        // EXTERNCOUNT record
        XclExpUInt16Record( EXC_ID_EXTERNCOUNT, nExtSheetCount ).Save( rStrm );
        // list of EXTERNSHEET records with EXTERNNAME, XCT, CRN records
        maExtSheetList.Save( rStrm );
    }
}

sal_uInt16 XclExpLinkManagerImpl5::GetExtSheetCount() const
{
    return static_cast< sal_uInt16 >( maExtSheetList.Size() );
}

sal_uInt16 XclExpLinkManagerImpl5::AppendInternal( XclExpExtSheetRef xExtSheet )
{
    if( GetExtSheetCount() < 0x7FFF )
    {
        maExtSheetList.AppendRecord( xExtSheet );
        // return negated one-based EXTERNSHEET index (i.e. 0xFFFD for 3rd record)
        return static_cast< sal_uInt16 >( -GetExtSheetCount() );
    }
    return 0;
}

void XclExpLinkManagerImpl5::CreateInternal()
{
    if( maIntTabMap.empty() )
    {
        // create EXTERNSHEET records for all internal exported sheets
        XclExpTabInfo& rTabInfo = GetTabInfo();
        for( SCTAB nScTab = 0, nScCnt = rTabInfo.GetScTabCount(); nScTab < nScCnt; ++nScTab )
        {
            if( rTabInfo.IsExportTab( nScTab ) )
            {
                XclExpExtSheetRef xRec;
                if( nScTab == GetCurrScTab() )
                    xRec.reset( new XclExpExternSheet( GetRoot(), EXC_EXTSH_OWNTAB ) );
                else
                    xRec.reset( new XclExpExternSheet( GetRoot(), rTabInfo.GetScTabName( nScTab ) ) );
                maIntTabMap[ nScTab ] = AppendInternal( xRec );
            }
        }
    }
}

XclExpLinkManagerImpl5::XclExpExtSheetRef XclExpLinkManagerImpl5::GetInternal( sal_uInt16 nExtSheet )
{
    return maExtSheetList.GetRecord( static_cast< sal_uInt16 >( -nExtSheet - 1 ) );
}

XclExpLinkManagerImpl5::XclExpExtSheetRef XclExpLinkManagerImpl5::FindInternal(
        sal_uInt16& rnExtSheet, sal_uInt16& rnXclTab, SCTAB nScTab )
{
    // create internal EXTERNSHEET records on demand
    CreateInternal();

    // try to find an EXTERNSHEET record - if not, return a "deleted sheet" reference
    XclExpExtSheetRef xExtSheet;
    XclExpIntTabMap::const_iterator aIt = maIntTabMap.find( nScTab );
    if( aIt == maIntTabMap.end() )
    {
        xExtSheet = FindInternal( rnExtSheet, EXC_EXTSH_OWNDOC );
        rnXclTab = EXC_TAB_DELETED;
    }
    else
    {
        rnExtSheet = aIt->second;
        xExtSheet = GetInternal( rnExtSheet );
        rnXclTab = GetTabInfo().GetXclTab( nScTab );
    }
    return xExtSheet;
}

XclExpLinkManagerImpl5::XclExpExtSheetRef XclExpLinkManagerImpl5::FindInternal(
    sal_uInt16& rnExtSheet, sal_Unicode cCode )
{
    XclExpExtSheetRef xExtSheet;
    XclExpCodeMap::const_iterator aIt = maCodeMap.find( cCode );
    if( aIt == maCodeMap.end() )
    {
        xExtSheet.reset( new XclExpExternSheet( GetRoot(), cCode ) );
        rnExtSheet = maCodeMap[ cCode ] = AppendInternal( xExtSheet );
    }
    else
    {
        rnExtSheet = aIt->second;
        xExtSheet = GetInternal( rnExtSheet );
    }
    return xExtSheet;
}

// ----------------------------------------------------------------------------

XclExpLinkManagerImpl8::XclExpLinkManagerImpl8( const XclExpRoot& rRoot ) :
    XclExpLinkManagerImpl( rRoot ),
    maSBBuffer( rRoot )
{
}

void XclExpLinkManagerImpl8::FindExtSheet(
        sal_uInt16& rnExtSheet, sal_uInt16& rnFirstXclTab, sal_uInt16& rnLastXclTab,
        SCTAB nFirstScTab, SCTAB nLastScTab, XclExpRefLogEntry* pRefLogEntry )
{
    XclExpTabInfo& rTabInfo = GetTabInfo();
    rnFirstXclTab = rTabInfo.GetXclTab( nFirstScTab );
    rnLastXclTab = rTabInfo.GetXclTab( nLastScTab );
    rnExtSheet = InsertXti( maSBBuffer.GetXti( rnFirstXclTab, rnLastXclTab, pRefLogEntry ) );
}

sal_uInt16 XclExpLinkManagerImpl8::FindExtSheet( sal_Unicode cCode )
{
    DBG_ASSERT( (cCode == EXC_EXTSH_OWNDOC) || (cCode == EXC_EXTSH_ADDIN),
        "XclExpLinkManagerImpl8::FindExtSheet - unknown externsheet code" );
    return InsertXti( maSBBuffer.GetXti( EXC_TAB_EXTERNAL, EXC_TAB_EXTERNAL ) );
}

void XclExpLinkManagerImpl8::StoreCellRange( const SingleRefData& rRef1, const SingleRefData& rRef2 )
{
    if( !rRef1.IsDeleted() && !rRef2.IsDeleted() && (rRef1.nTab >= 0) && (rRef2.nTab >= 0) )
    {
        const XclExpTabInfo& rTabInfo = GetTabInfo();
        SCTAB nFirstScTab = static_cast< SCTAB >( rRef1.nTab );
        SCTAB nLastScTab = static_cast< SCTAB >( rRef2.nTab );
        ScRange aRange(
            static_cast< SCCOL >( rRef1.nCol ), static_cast< SCROW >( rRef1.nRow ), 0,
            static_cast< SCCOL >( rRef2.nCol ), static_cast< SCROW >( rRef2.nRow ), 0 );
        for( SCTAB nScTab = nFirstScTab; nScTab <= nLastScTab; ++nScTab )
        {
            if( rTabInfo.IsExternalTab( nScTab ) )
            {
                aRange.aStart.SetTab( nScTab );
                aRange.aEnd.SetTab( nScTab );
                maSBBuffer.StoreCellRange( aRange );
            }
        }
    }
}

bool XclExpLinkManagerImpl8::InsertAddIn(
        sal_uInt16& rnExtSheet, sal_uInt16& rnExtName, const String& rName )
{
    sal_uInt16 nSupbook;
    if( maSBBuffer.InsertAddIn( nSupbook, rnExtName, rName ) )
    {
        rnExtSheet = InsertXti( XclExpXti( nSupbook, EXC_TAB_EXTERNAL, EXC_TAB_EXTERNAL ) );
        return true;
    }
    return false;
}

bool XclExpLinkManagerImpl8::InsertDde(
        sal_uInt16& rnExtSheet, sal_uInt16& rnExtName,
        const String& rApplic, const String& rTopic, const String& rItem )
{
    sal_uInt16 nSupbook;
    if( maSBBuffer.InsertDde( nSupbook, rnExtName, rApplic, rTopic, rItem ) )
    {
        rnExtSheet = InsertXti( XclExpXti( nSupbook, EXC_TAB_EXTERNAL, EXC_TAB_EXTERNAL ) );
        return true;
    }
    return false;
}

void XclExpLinkManagerImpl8::Save( XclExpStream& rStrm )
{
    if( !maXtiVec.empty() )
    {
        // SUPBOOKs, XCTs, CRNs, EXTERNNAMEs
        maSBBuffer.Save( rStrm );

        // EXTERNSHEET
        sal_uInt16 nCount = ulimit_cast< sal_uInt16 >( maXtiVec.size() );
        rStrm.StartRecord( EXC_ID_EXTERNSHEET, 2 + 6 * nCount );
        rStrm << nCount;
        rStrm.SetSliceSize( 6 );
        for( XclExpXtiVec::const_iterator aIt = maXtiVec.begin(), aEnd = maXtiVec.end(); aIt != aEnd; ++aIt )
            aIt->Save( rStrm );
        rStrm.EndRecord();
    }
}

sal_uInt16 XclExpLinkManagerImpl8::InsertXti( const XclExpXti& rXti )
{
    for( XclExpXtiVec::const_iterator aIt = maXtiVec.begin(), aEnd = maXtiVec.end(); aIt != aEnd; ++aIt )
        if( *aIt == rXti )
            return ulimit_cast< sal_uInt16 >( aIt - maXtiVec.begin() );
    maXtiVec.push_back( rXti );
    return ulimit_cast< sal_uInt16 >( maXtiVec.size() - 1 );
}

// ============================================================================

XclExpLinkManager::XclExpLinkManager( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot )
{
    switch( GetBiff() )
    {
        case EXC_BIFF5:
            mxImpl.reset( new XclExpLinkManagerImpl5( rRoot ) );
        break;
        case EXC_BIFF8:
            mxImpl.reset( new XclExpLinkManagerImpl8( rRoot ) );
        break;
        default:
            DBG_ERROR_BIFF();
    }
}

XclExpLinkManager::~XclExpLinkManager()
{
}

void XclExpLinkManager::FindExtSheet(
        sal_uInt16& rnExtSheet, sal_uInt16& rnXclTab,
        SCTAB nScTab, XclExpRefLogEntry* pRefLogEntry )
{
    mxImpl->FindExtSheet( rnExtSheet, rnXclTab, rnXclTab, nScTab, nScTab, pRefLogEntry );
}

void XclExpLinkManager::FindExtSheet(
        sal_uInt16& rnExtSheet, sal_uInt16& rnFirstXclTab, sal_uInt16& rnLastXclTab,
        SCTAB nFirstScTab, SCTAB nLastScTab, XclExpRefLogEntry* pRefLogEntry )
{
    mxImpl->FindExtSheet( rnExtSheet, rnFirstXclTab, rnLastXclTab, nFirstScTab, nLastScTab, pRefLogEntry );
}

sal_uInt16 XclExpLinkManager::FindExtSheet( sal_Unicode cCode )
{
    return mxImpl->FindExtSheet( cCode );
}

void XclExpLinkManager::StoreCell( const SingleRefData& rRef )
{
    mxImpl->StoreCellRange( rRef, rRef );
}

void XclExpLinkManager::StoreCellRange( const ComplRefData& rRef )
{
    mxImpl->StoreCellRange( rRef.Ref1, rRef.Ref2 );
}

bool XclExpLinkManager::InsertAddIn(
        sal_uInt16& rnExtSheet, sal_uInt16& rnExtName, const String& rName )
{
    return mxImpl->InsertAddIn( rnExtSheet, rnExtName, rName );
}

bool XclExpLinkManager::InsertDde(
        sal_uInt16& rnExtSheet, sal_uInt16& rnExtName,
        const String& rApplic, const String& rTopic, const String& rItem )
{
    return mxImpl->InsertDde( rnExtSheet, rnExtName, rApplic, rTopic, rItem );
}

void XclExpLinkManager::Save( XclExpStream& rStrm )
{
    mxImpl->Save( rStrm );
}

// ============================================================================


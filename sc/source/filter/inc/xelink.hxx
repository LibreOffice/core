/*************************************************************************
 *
 *  $RCSfile: xelink.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:05:08 $
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

#ifndef SC_XELINK_HXX
#define SC_XELINK_HXX

#ifndef SC_XEHELPER_HXX
#include "xehelper.hxx"
#endif
#ifndef SC_XERECORD_HXX
#include "xerecord.hxx"
#endif

class ScRange;
struct SingleRefData;


/* ============================================================================
Classes for export of different kinds of internal/external references.
- 3D cell and cell range links
- External cell and cell range links
- Add-in functions
- DDE links
- OLE object links
============================================================================ */

// Excel sheet indexes ========================================================

typedef ::std::vector< ::std::pair< sal_uInt16, sal_uInt16 > > XclExpRefLogVec;

/** Stores the correct Excel sheet index for each Calc sheet.
    @descr  The class knows all sheets which will not exported
    (i.e. external link sheets, scenario sheets). */
class XclExpTabIdBuffer
{
private:
    typedef ::std::vector< ::std::pair< sal_uInt16, sal_uInt8 > > IndexEntryVec;

    IndexEntryVec               maIndexVec;         /// Array of sheet index information.

    sal_uInt16                  mnScCnt;            /// Count of Calc sheets.
    sal_uInt16                  mnXclCnt;           /// Count of Excel sheets to be exported.
    sal_uInt16                  mnExtCnt;           /// Count of external link sheets (in Calc).
    sal_uInt16                  mnCodeCnt;          /// Count of codepages.

    ScfUInt16Vec                maFromSortedVec;    /// Sorted index -> real index.
    ScfUInt16Vec                maToSortedVec;      /// Real index -> sorted index.

    XclExpRefLogVec             maRefLog;           /// A log for each requested Excel sheet index.
    bool                        mbEnableLog;        /// true = log all sheet indexes (for formula compiler).

public:
    /** Initializes the complete buffer from the passed document. */
    explicit                    XclExpTabIdBuffer( ScDocument& rDoc );

    /** Returns true, if the specified Calc sheet is used to store external cell contents. */
    bool                        IsExternal( sal_uInt16 nScTab ) const;
    /** Returns true, if the specified Calc sheet has to be exported. */
    bool                        IsExportTable( sal_uInt16 nScTab ) const;

    /** Returns the Excel sheet index for a given Calc sheet. */
    sal_uInt16                  GetXclTab( sal_uInt16 nScTab ) const;

    /** Returns the Calc sheet index of the nSortedTab-th entry in the sorted sheet names list. */
    sal_uInt16                  GetRealScTab( sal_uInt16 nSortedTab ) const;
    /** Returns the index of the passed Calc sheet in the sorted sheet names list. */
    sal_uInt16                  GetSortedScTab( sal_uInt16 nScTab ) const;

    /** Returns the number of Calc sheets. */
    inline sal_uInt16           GetScTabCount() const { return mnScCnt; }
    /** Returns the number of Excel sheets to be exported. */
    inline sal_uInt16           GetXclTabCount() const { return mnXclCnt; }
    /** Returns the number of external linked sheets (in Calc). */
    inline sal_uInt16           GetExternTabCount() const { return mnExtCnt; }
    /** Returns the number of codepages (VBA modules). */
    inline sal_uInt16           GetCodenameCount() const { return mnCodeCnt; }
    /** Returns the maximum number of Calc sheets and codepages. */
    inline sal_uInt16           GetMaxScTabCount() const { return ::std::max( mnScCnt, mnCodeCnt ); }

    // *** for change tracking ***

    /** Enables logging of Excel sheet indexes in each 3D-reference. */
    void                        StartRefLog();
    /** Appends sheet index pair (called by formula compiler). */
    void                        AppendTabRef( sal_uInt16 nXclFirst, sal_uInt16 nXclLast );
    /** Disables logging of Excel sheet indexes. */
    const XclExpRefLogVec&      EndRefLog();

private:
    /** Searches for sheets not to be exported. */
    void                        CalcXclIndexes();
    /** Sorts the names of all tables and stores the indexes of the sorted indexes. */
    void                        CalcSortedIndexes( ScDocument& rDoc );
};


// External names =============================================================

/** This is a base class for any external name (i.e. add-in names or DDE links).
    @descr  Derived classes implement creation and export of the external names. */
class XclExpExtNameBase : public XclExpRecord
{
private:
    String                      maName;         /// Name of the external name.
    sal_uInt16                  mnFlags;        /// Flags for record export.

public:
    /** @param nFlags  The flags to export directly.
        @param nAddSize  The size of additional data derived classes will write. */
    explicit                    XclExpExtNameBase(
                                    const String& rName,
                                    sal_uInt16 nFlags = 0,
                                    sal_uInt32 nAddSize = 0 );

    virtual                     ~XclExpExtNameBase();

    /** Returns the name string of the external name. */
    inline const String&        GetName() const { return maName; }

private:
    /** Writes the start of the record that is equal in all EXTERNNAME records and calls WriteAddData(). */
    virtual void                WriteBody( XclExpStream& rStrm );

    /** Called to write additional data following the common record contents.
        @descr  Derived classes should overwrite this function to write their data. */
    virtual void                WriteAddData( XclExpStream& rStrm );
};


// ----------------------------------------------------------------------------

/** Represents an EXTERNNAME record for an add-in function name. */
class XclExpExtNameAddIn : public XclExpExtNameBase
{
public:
    inline explicit             XclExpExtNameAddIn( const String& rName ) :
                                    XclExpExtNameBase( rName, 0, 4 ) {}

private:
    /** Writes additional record contents. */
    virtual void                WriteAddData( XclExpStream& rStrm );
};


// ----------------------------------------------------------------------------

/** Represents an EXTERNNAME record for a DDE link. */
class XclExpExtNameDde : public XclExpExtNameBase
{
private:
    typedef ::std::auto_ptr< XclExpCachedMatrix > XclExpCachedMatrixPtr;

    XclExpCachedMatrixPtr       mpMatrix;       /// Cached results of the DDE link.
    sal_uInt32                  mnBaseSize;     /// Cached size of base record.

public:
    explicit                    XclExpExtNameDde( const String& rName, sal_uInt16 nFlags );

    /** Inserts result list of a DDE link. */
    bool                        InsertDde(
                                    const XclExpRoot& rRoot,
                                    const String& rApplic,
                                    const String& rTopic,
                                    const String& rItem );

private:
    /** Writes additional record contents. */
    virtual void                WriteAddData( XclExpStream& rStrm );
};


// ----------------------------------------------------------------------------

/** List of all external names of a sheet. */
class XclExpExtNameList : XclExpRecordBase
{
private:
    XclExpRecordList< XclExpExtNameBase > maNameList;   /// The list with all EXTERNNAME records.
    bool                        mbHasDde;               /// true = contains DDE links.

public:
    explicit                    XclExpExtNameList() : mbHasDde( false ) {}

    /** Inserts an add-in function name
        @return  The 1-based (Excel-like) list index of the name. */
    sal_uInt16                  InsertAddIn( const String& rName );
    /** Inserts a DDE link.
        @return  The 1-based (Excel-like) list index of the DDE link. */
    sal_uInt16                  InsertDde(
                                    const XclExpRoot& rRoot,
                                    const String& rApplic,
                                    const String& rTopic,
                                    const String& rItem );

    /** Writes the EXTERNNAME record list. */
    virtual void                Save( XclExpStream& rStrm );

private:
    /** Returns the 1-based (Excel-like) list index of the external name or 0, if not found. */
    sal_uInt16                  GetIndex( const String& rName ) const;

    /** Appends the external name.
        @return  The 1-based (Excel-like) list index of the appended name. */
    sal_uInt16                  Append( XclExpExtNameBase* pName );
};


// Cached external cells ======================================================

/** Base class to store the contents of one external cell (record CRN). */
class XclExpCrn : public XclExpRecord
{
private:
    sal_uInt16                  mnCol;      /// Column index of the external cell.
    sal_uInt16                  mnRow;      /// Row index of the external cell.
    sal_uInt8                   mnId;       /// Identifier for data type (EXC_CACHEDVAL_***).

public:
    /** Returns true, if this CRN has the given cell address. */
    inline bool                 IsAddress( sal_uInt16 nCol, sal_uInt16 nRow ) const
                                    { return (nCol == mnCol) && (nRow == mnRow); }

protected:
    /** @param nAddSize  The size of additional data derived classes will write. */
    explicit                    XclExpCrn( sal_uInt16 nCol, sal_uInt16 nRow, sal_uInt8 nId, sal_uInt32 nAddLen = 0 );

private:
    /** Writes the start of the record that is equal in all CRN records and calls WriteAddData(). */
    virtual void                WriteBody( XclExpStream& rStrm );

    /** Called to write additional data following the common record contents.
        @descr  Derived classes should overwrite this function to write their data. */
    virtual void                WriteAddData( XclExpStream& rStrm ) = 0;
};


// ----------------------------------------------------------------------------

/** Cached data of an external value cell. */
class XclExpCrnDouble : public XclExpCrn
{
private:
    double                      mfVal;      /// Value of the cached cell.

public:
    explicit                    XclExpCrnDouble( sal_uInt16 nCol, sal_uInt16 nRow, double fVal );

private:
    /** Writes the double value following the common record contents. */
    virtual void                WriteAddData( XclExpStream& rStrm );
};


// ----------------------------------------------------------------------------

/** Cached data of an external text cell. */
class XclExpCrnString : public XclExpCrn
{
private:
    XclExpString                maText;     /// Text of the cached cell.

public:
    explicit                    XclExpCrnString( sal_uInt16 nCol, sal_uInt16 nRow, const String& rText );

private:
    /** Writes the string following the common record contents. */
    virtual void                WriteAddData( XclExpStream& rStrm );

};


// ----------------------------------------------------------------------------

/// Cached data of an external Boolean cell. */
class XclExpCrnBool : public XclExpCrn
{
private:
    sal_uInt16                  mnBool;     /// Boolean value of the cached cell.

public:
    explicit                    XclExpCrnBool( sal_uInt16 nCol, sal_uInt16 nRow, bool bBoolVal );

private:
    /** Writes the Boolean value following the common record contents. */
    virtual void                WriteAddData( XclExpStream& rStrm );
};


// ----------------------------------------------------------------------------

/// Represents the record XCT which is the header record of a CRN record list. */
class XclExpXct : public XclExpRecord
{
private:
    XclExpRecordList< XclExpCrn > maCrnList;    /// CRN records that follow this record.
    XclExpString                maTable;        /// Sheet name of the external sheet.
    sal_uInt16                  mnXclTab;       /// Excel sheet index.

public:
    explicit                    XclExpXct( const String& rTabName );

    /** Returns the size the sheet name will take in stream. */
    inline sal_uInt16           GetTableBytes() const
                                    { return static_cast< sal_uInt16 >( maTable.GetSize() ); }
    /** Returns the external sheet name. */
    inline const XclExpString&  GetTableName() const { return maTable; }

    /** Sets the Excel sheet index. */
    inline void                 SetXclTab( sal_uInt16 nXclTab ) { mnXclTab = nXclTab; }
    /** Stores all cells in the given range in the CRN list. */
    void                        StoreCellRange( const XclExpRoot& rRoot, const ScRange& rRange );

    /** Writes the XCT and all CRN records. */
    virtual void                Save( XclExpStream& rStrm );

private:
    /** Returns true, if the CRN list already contains an CRN with the given address. */
    bool                        Exists( sal_uInt16 nCol, sal_uInt16 nRow ) const;

    /** Writes the XCT record contents. */
    virtual void                WriteBody( XclExpStream& rStrm );
};


// External documents =========================================================

/** The SUPBOOK record contains data for an external document (URL, sheet names, external values). */
class XclExpSupbook : public XclExpRecord
{
private:
    typedef ::std::auto_ptr< XclExpExtNameList > XclExpExtNameListPtr;

    /** This enumeration specifies the type of a SUPBOOK record. */
    enum XclExpSBType
    {
        xlSBSelf,               /// SUPBOOK is used for internal references.
        xlSBUrl,                /// SUPBOOK is used for external references.
        xlSBDde,                /// SUPBOOK is used for DDE links.
        xlSBAddIn               /// SUPBOOK contains add-in functions.
    };

    XclExpRecordList< XclExpXct > maXctList;    /// List of XCT records (which contain CRN records).
    String                      maUrl;          /// URL of the external document or application name for DDE.
    String                      maDdeTopic;     /// Topic of an DDE link.
    XclExpString                maUrlEncoded;   /// Document name encoded for Excel.
    XclExpExtNameListPtr        mpExtNameList;  /// List of EXTERNNAME records.
    XclExpSBType                meType;         /// Type of this SUPBOOK record.
    sal_uInt16                  mnTables;       /// Count of sheets.

public:
    /** Creates a SUPBOOK record for internal references. */
    explicit                    XclExpSupbook( sal_uInt16 nTabs );
    /** Creates a SUPBOOK record for add-in functions. */
    explicit                    XclExpSupbook();
    /** Creates a SUPBOOK record for an external document. */
    explicit                    XclExpSupbook( const XclExpRoot& rRoot, const String& rUrl );
    /** Creates a SUPBOOK record for a DDE link. */
    explicit                    XclExpSupbook( const XclExpRoot& rRoot, const String& rApplic, const String& rTopic );

    /** Returns document URL encoded for Excel. */
    inline const XclExpString&  GetUrlEncoded() const { return maUrlEncoded; }
    /** Returns the sheet name inside of this SUPBOOK. */
    const XclExpString*         GetTableName( sal_uInt16 nXct ) const;

    /** Returns true, if this SUPBOOK contains the passed URL of an external document. */
    bool                        IsUrlLink( const String& rUrl ) const;
    /** Returns true, if this SUPBOOK contains the passed DDE link. */
    bool                        IsDdeLink( const String& rApplic, const String& rTopic ) const;

    /** Stores all cells in the given range in the CRN list of the XCT with index nXct. */
    void                        StoreCellRange( const XclExpRoot& rRoot, const ScRange& rRange, sal_uInt16 nXct );

    /** Inserts a new XCT record with the given sheet name.
        @return  The sheet index. */
    sal_uInt16                  InsertTable( const String& rTabName );
    /** Finds or inserts an EXTERNNAME record for add-ins.
        @return  The 1-based EXTERNNAME record index. */
    sal_uInt16                  InsertAddIn( const String& rName );
    /** Finds or inserts an EXTERNNAME record for DDE links.
        @return  The 1-based EXTERNNAME record index. */
    sal_uInt16                  InsertDde( const XclExpRoot& rRoot, const String& rItem );

    /** Writes the SUPBOOK and all EXTERNNAME, XCT and CRN records. */
    virtual void                Save( XclExpStream& rStrm );

private:
    /** Creates and returns the list of EXTERNNAME records. */
    XclExpExtNameList&          GetExtNameList();

    /** Writes the SUPBOOK record contents. */
    virtual void                WriteBody( XclExpStream& rStrm );
};


// ----------------------------------------------------------------------------

/** Contains a list of all SUPBOOK records and index arrays of external sheets. */
class XclExpSupbookBuffer : public XclExpRecordBase, protected XclExpRoot
{
private:
    XclExpRecordList< XclExpSupbook > maSupbookList;    /// List of all SUPBOOK records.
    ScfUInt16Vec                maSBIndexBuffer;        /// SUPBOOK index for each Excel sheet.
    ScfUInt16Vec                maXtiBuffer;            /// Sheet indexes inside of SUPBOOK records for each Excel sheet.
    sal_uInt16                  mnAddInSB;              /// Index to add-in SUPBOOK.

public:
    explicit                    XclExpSupbookBuffer( const XclExpRoot& rRoot );

    /** Returns the external document URL of the Excel sheet nXclTab. */
    const XclExpString*         GetUrl( sal_uInt16 nXclTab ) const;
    /** Returns the external sheet name of the Excel sheet nXclTab. */
    const XclExpString*         GetTableName( sal_uInt16 nXclTab ) const;

    /** Finds SUPBOOK index and SUPBOOK sheet range from given Excel sheet range.
        @param rnSupbook  Returns the index of the SUPBOOK record containing the sheet range.
        @param rnXtiFirst  Returns the index of the first XTI structure.
        @param rnXtiLast  Returns the index of the last XTI structure. */
    void                        GetXtiRange(
                                    sal_uInt16& rnSupbook, sal_uInt16& rnXtiFirst, sal_uInt16& rnXtiLast,
                                    sal_uInt16 nXclFirst, sal_uInt16 nXclLast ) const;

    /** Stores all cells in the given range in a CRN record list. */
    void                        StoreCellRange( const ScRange& rRange );

    /** Finds or inserts an EXTERNNAME record for an add-in function name.
        @param rnSupbook  Returns the index of the SUPBOOK record which contains the add-in function name.
        @param rnExtName  Returns the 1-based EXTERNNAME record index. */
    void                        InsertAddIn(
                                    sal_uInt16& rnSupbook, sal_uInt16& rnExtName,
                                    const String& rName );
    /** Finds or inserts an EXTERNNAME record for DDE links.
        @param rnSupbook  Returns the index of the SUPBOOK record which contains the DDE link.
        @param rnExtName  Returns the 1-based EXTERNNAME record index. */
    void                        InsertDde(
                                    sal_uInt16& rnSupbook, sal_uInt16& rnExtName,
                                    const String& rApplic, const String& rTopic, const String& rItem );

    /** Writes all SUPBOOK records with their sub records. */
    virtual void                Save( XclExpStream& rStrm );

private:
    /** Returns the SUPBOOK record of an Excel sheet. */
    XclExpSupbook*              GetSupbook( sal_uInt16 nXclTab ) const;
    /** Searches for the SUPBOOK record containing the passed document URL.
        @param rnIndex  Returns the list index, if the SUPBOOK exists.
        @return  The SUPBOOK record or NULL, if not found. */
    XclExpSupbook*              GetSupbookUrl( sal_uInt16& rnIndex, const String& rUrl ) const;
    /** Searches for the SUPBOOK record containing the passed DDE link.
        @param rnIndex  Returns the list index, if the SUPBOOK exists.
        @return  The SUPBOOK record or NULL, if not found. */
    XclExpSupbook*              GetSupbookDde( sal_uInt16& rnIndex, const String& rApplic, const String& rTopic ) const;

    /** Appends a new SUPBOOK to the list.
        @return  The list index of the SUPBOOK record. */
    sal_uInt16                  Append( XclExpSupbook* pBook );
    /** Creates and appends an external SUPBOOK record from the Calc sheet nScTab. */
    void                        AddExt( sal_uInt16 nScTab );
};


// Export link manager ========================================================

/** This struct contains a sheet index range for 3D references.
    @descr  This reference consists of an index to a SUPBOOK record and indexes
    to SUPBOOK sheet names. */
struct XclExpXti
{
    sal_uInt16                  mnSupb;         /// Index to SUPBOOK record.
    sal_uInt16                  mnFirst;        /// Index to the first sheet of the range.
    sal_uInt16                  mnLast;         /// Index to the last sheet of the range.

    inline explicit             XclExpXti( sal_uInt16 nSupbook, sal_uInt16 nFirst, sal_uInt16 nLast ) :
                                    mnSupb( nSupbook ), mnFirst( nFirst ), mnLast( nLast ) {}

    /** Returns true, if this XTI contains the given values. */
    inline bool                 Equals( sal_uInt16 nSupbook, sal_uInt16 nFirst, sal_uInt16 nLast ) const
                                    {return (mnSupb == nSupbook) && (mnFirst == nFirst) && (mnLast == nLast); }

    /** Writes this XTI structure (inside of the EXTERNSHEET record). */
    inline void                 Save( XclExpStream& rStrm ) const
                                    { rStrm << mnSupb << mnFirst << mnLast; }
};


// ----------------------------------------------------------------------------

/** Stores all EXTERNSHEET and SUPBOOK record data.
    @descr  This is the central class for export of all external references.
    File contents in BIFF8:
    - Record SUPBOOK: Contains the name of an external workbook and the names of its sheets.
    This record is followed by EXTERNNAME, XCT and CRN records.
    - Record XCT: Contains the sheet index of the following CRN records.
    - Record CRN: Contains addresses (row and column) and values of external referenced cells.
    - Record NAME: Contains defined names of the own workbook. This record follows the
    EXTERNSHEET record.
    - Record EXTERNNAME: Contains external defined names or DDE links or OLE object links.
    - Record EXTERNSHEET: Contains indexes to URLs of external documents (SUPBOOKs)
    and sheet indexes for each external reference used anywhere in the workbook.
    This record follows a list of SUPBOOK records.
*/
class XclExpLinkManager : public XclExpRecord, protected XclExpRoot
{
private:
    ScfDelList< XclExpXti >     maXtiList;      /// List of XTI structures.
    XclExpSupbookBuffer         maSBBuffer;     /// List of all SUPBOOK records.

public:
    explicit                    XclExpLinkManager( const XclExpRoot& rRoot );

    /** Searches for XTI structure with the given Excel sheet range. Adds new XTI if not found.
        @return  The list index of the XTI structure. */
    sal_uInt16                  FindXti( sal_uInt16 nXclFirst, sal_uInt16 nXclLast );

    /** Returns the external document URL of the specified Excel sheet. */
    inline const XclExpString*  GetUrl( sal_uInt16 nXclTab )
                                    { return maSBBuffer.GetUrl( nXclTab ); }
    /** Returns the external sheet name of the specified Excel sheet. */
    inline const XclExpString*  GetTableName( sal_uInt16 nXclTab )
                                    { return maSBBuffer.GetTableName( nXclTab ); }

    /** Stores the cell with the given address in a CRN record list. */
    void                        StoreCellCont( const SingleRefData& rRef );
    /** Stores all cells in the given range in a CRN record list. */
    void                        StoreCellRange( const SingleRefData& rRef1, const SingleRefData& rRef2 );

    /** Finds or inserts an EXTERNNAME record for an add-in function name.
        @param rnXti  Returns the index of the XTI structure which contains the add-in function name.
        @param rnExtName  Returns the 1-based EXTERNNAME record index. */
    void                        InsertAddIn(
                                    sal_uInt16& rnXti, sal_uInt16& rnExtName,
                                    const String& rName );
    /** Finds or inserts an EXTERNNAME record for DDE links.
        @param rnXti  Returns the index of the XTI structure which contains the DDE link.
        @param rnExtName  Returns the 1-based EXTERNNAME record index. */
    bool                        InsertDde(
                                    sal_uInt16& rnXti, sal_uInt16& rnExtName,
                                    const String& rApplic, const String& rTopic, const String& rItem );

    /** Writes the EXTERNSHEET record and all SUPBOOK records with their sub records. */
    virtual void                Save( XclExpStream& rStrm );

private:
    /** Appends an XTI structure to the list.
        @return  The list index of the XTI structure. */
    sal_uInt16                  AppendXti( XclExpXti* pXti );
    /** Searches for or inserts a new XTI structure.
        @return  The list index of the XTI structure. */
    sal_uInt16                  InsertXti( sal_uInt16 nSupbook, sal_uInt16 nXtiFirst, sal_uInt16 nXtiLast );

    /** Writes the EXTERNSHEET record contents with its XTI structures. */
    virtual void                WriteBody( XclExpStream& rStrm );
};


// ============================================================================

#endif


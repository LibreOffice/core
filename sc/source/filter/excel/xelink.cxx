/*************************************************************************
 *
 *  $RCSfile: xelink.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:34:24 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif
#pragma hdrstop

// ============================================================================

#ifndef SC_XELINK_HXX
#include "xelink.hxx"
#endif

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
#ifndef _SCEXTOPT_HXX
#include "scextopt.hxx"
#endif

#include "root.hxx"


// ============================================================================
// *** Helper classes ***
// ============================================================================

// External names =============================================================

/** This is a base class for any external name (i.e. add-in names or DDE links).
    @descr  Derived classes implement creation and export of the external names. */
class XclExpExtNameBase : public XclExpRecord
{
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

private:
    String                      maName;         /// Name of the external name.
    sal_uInt16                  mnFlags;        /// Flags for record export.
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

private:
    typedef ::std::auto_ptr< XclExpCachedMatrix > XclExpCachedMatrixPtr;

    XclExpCachedMatrixPtr       mpMatrix;       /// Cached results of the DDE link.
    sal_uInt32                  mnBaseSize;     /// Cached size of base record.
};


// List of external names =====================================================

/** List of all external names of a sheet. */
class XclExpExtNameList : XclExpRecordBase
{
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

private:
    typedef XclExpRecordList< XclExpExtNameBase > XclExpExtNameBaseList;
    XclExpExtNameBaseList       maNameList;     /// The list with all EXTERNNAME records.
    bool                        mbHasDde;       /// true = contains DDE links.
};


// Cached external cells ======================================================

/** Base class to store the contents of one external cell (record CRN). */
class XclExpCrn : public XclExpRecord
{
protected:
    /** @param nAddSize  The size of additional data derived classes will write. */
    explicit                    XclExpCrn( sal_uInt16 nCol, sal_uInt16 nRow, sal_uInt8 nId, sal_uInt32 nAddLen = 0 );

private:
    /** Writes the start of the record that is equal in all CRN records and calls WriteAddData(). */
    virtual void                WriteBody( XclExpStream& rStrm );

    /** Called to write additional data following the common record contents.
        @descr  Derived classes should overwrite this function to write their data. */
    virtual void                WriteAddData( XclExpStream& rStrm ) = 0;

private:
    sal_uInt16                  mnCol;      /// Column index of the external cell.
    sal_uInt16                  mnRow;      /// Row index of the external cell.
    sal_uInt8                   mnId;       /// Identifier for data type (EXC_CACHEDVAL_***).
};


// ----------------------------------------------------------------------------

/** Cached data of an external value cell. */
class XclExpCrnDouble : public XclExpCrn
{
public:
    explicit                    XclExpCrnDouble( sal_uInt16 nCol, sal_uInt16 nRow, double fVal );

private:
    /** Writes the double value following the common record contents. */
    virtual void                WriteAddData( XclExpStream& rStrm );

private:
    double                      mfVal;      /// Value of the cached cell.
};


// ----------------------------------------------------------------------------

/** Cached data of an external text cell. */
class XclExpCrnString : public XclExpCrn
{
public:
    explicit                    XclExpCrnString( sal_uInt16 nCol, sal_uInt16 nRow, const String& rText );

private:
    /** Writes the string following the common record contents. */
    virtual void                WriteAddData( XclExpStream& rStrm );

private:
    XclExpString                maText;     /// Text of the cached cell.
};


// ----------------------------------------------------------------------------

/// Cached data of an external Boolean cell. */
class XclExpCrnBool : public XclExpCrn
{
public:
    explicit                    XclExpCrnBool( sal_uInt16 nCol, sal_uInt16 nRow, bool bBoolVal );

private:
    /** Writes the Boolean value following the common record contents. */
    virtual void                WriteAddData( XclExpStream& rStrm );

private:
    sal_uInt16                  mnBool;     /// Boolean value of the cached cell.
};


// Cached cells of a sheet ====================================================

/// Represents the record XCT which is the header record of a CRN record list. */
class XclExpXct : public XclExpRecord
{
public:
    explicit                    XclExpXct( const String& rTabName );

    /** Returns the external sheet name. */
    inline const XclExpString&  GetTableName() const { return maTable; }

    /** Sets the Excel sheet index. */
    inline void                 SetXclTab( sal_uInt16 nXclTab ) { mnXclTab = nXclTab; }
    /** Stores all cells in the given range in the CRN list. */
    void                        StoreCellRange( const XclExpRoot& rRoot, const ScRange& rRange );

    /** Writes the XCT and all CRN records. */
    virtual void                Save( XclExpStream& rStrm );

private:
    /** Writes the XCT record contents. */
    virtual void                WriteBody( XclExpStream& rStrm );

private:
    typedef XclExpRecordList< XclExpCrn > XclExpCrnList;

    XclExpCrnList               maCrnList;      /// CRN records that follow this record.
    ScMarkData                  maUsedCells;    /// Contains addresses of all stored cells.
    XclExpString                maTable;        /// Sheet name of the external sheet.
    sal_uInt16                  mnXclTab;       /// Excel sheet index.
};


// External documents (SUPBOOK, BIFF8) ========================================

/** The SUPBOOK record contains data for an external document (URL, sheet names, external values). */
class XclExpSupbook : public XclExpRecord
{
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
};


// All SUPBOOKS in a document =================================================

/** Contains a list of all SUPBOOK records and index arrays of external sheets. */
class XclExpSupbookBuffer : public XclExpRecordBase, protected XclExpRoot
{
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

private:
    typedef XclExpRecordList< XclExpSupbook > XclExpSupbookList;

    XclExpSupbookList           maSupbookList;      /// List of all SUPBOOK records.
    ScfUInt16Vec                maSBIndexBuffer;    /// SUPBOOK index for each Excel sheet.
    ScfUInt16Vec                maXtiBuffer;        /// Sheet indexes inside of SUPBOOK records for each Excel sheet.
    sal_uInt16                  mnAddInSB;          /// Index to add-in SUPBOOK.
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

/** Implementation of the link manager. */
class XclExpLinkManager_Impl : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit                    XclExpLinkManager_Impl( const XclExpRoot& rRoot );

    /** Searches for XTI structure with the given Excel sheet range. Adds new XTI if not found.
        @return  The list index of the XTI structure. */
    sal_uInt16                  FindXti( sal_uInt16 nXclFirst, sal_uInt16 nXclLast );

    /** Returns the external document URL of the specified Excel sheet. */
    inline const XclExpString*  GetUrl( sal_uInt16 nXclTab ) const
                                    { return maSBBuffer.GetUrl( nXclTab ); }
    /** Returns the external sheet name of the specified Excel sheet. */
    inline const XclExpString*  GetTableName( sal_uInt16 nXclTab ) const
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

    /** Writes all SUPBOOK records with their sub records and the trailing EXTERNSHEET record. */
    virtual void                Save( XclExpStream& rStrm );

private:
    /** Appends an XTI structure to the list.
        @return  The list index of the XTI structure. */
    sal_uInt16                  AppendXti( XclExpXti* pXti );
    /** Searches for or inserts a new XTI structure.
        @return  The list index of the XTI structure. */
    sal_uInt16                  InsertXti( sal_uInt16 nSupbook, sal_uInt16 nXtiFirst, sal_uInt16 nXtiLast );

private:
    typedef ScfDelList< XclExpXti > XclExpXtiList;
    XclExpXtiList               maXtiList;      /// List of XTI structures.
    XclExpSupbookBuffer         maSBBuffer;     /// List of all SUPBOOK records.
};


// ============================================================================
// *** Implementation ***
// ============================================================================

// Excel sheet indexes ========================================================

const sal_uInt8 EXC_TABBUF_EXPORT   = 0x00;
const sal_uInt8 EXC_TABBUF_IGNORE   = 0x01;
const sal_uInt8 EXC_TABBUF_EXTERN   = 0x02;


// ----------------------------------------------------------------------------

XclExpTabIdBuffer::XclExpTabIdBuffer( ScDocument& rDoc ) :
    mnScCnt( rDoc.GetTableCount() ),
    mnXclCnt( 0 ),
    mnExtCnt( 0 ),
    mnCodeCnt( 0 ),
    mbEnableLog( false )
{
    // --- pre-initialize the index buffer ---
    maIndexVec.resize( mnScCnt );
    for( sal_uInt16 nScTab = 0; nScTab < mnScCnt; ++nScTab )
    {
        maIndexVec[ nScTab ].first = 0;
        sal_uInt8& rnType = maIndexVec[ nScTab ].second;

        // ignored tables (skipped by export, invalid Excel sheet index)
        if( rDoc.IsScenario( nScTab ) )
            rnType = EXC_TABBUF_IGNORE;

        // external tables (skipped, with valid Excel table number for ref's)
        else if( rDoc.GetLinkMode( nScTab ) == SC_LINK_VALUE )
            rnType = EXC_TABBUF_EXTERN;

        // normal sheets to export
        else
            rnType = EXC_TABBUF_EXPORT;
    }

    // --- calculate Excel sheet indexes ---
    CalcXclIndexes();

    // --- codepages ---
    if( rDoc.GetExtDocOptions() )
    {
        CodenameList* pCList = rDoc.GetExtDocOptions()->GetCodenames();
        if( pCList )
            mnCodeCnt = ::ulimit< sal_uInt16 >( pCList->Count() );
    }

    // --- sorted vectors for index lookup ---
    CalcSortedIndexes( rDoc );
}

bool XclExpTabIdBuffer::IsExternal( sal_uInt16 nScTab ) const
{
    return (nScTab < mnScCnt) && (maIndexVec[ nScTab ].second == EXC_TABBUF_EXTERN);
}

bool XclExpTabIdBuffer::IsExportTable( sal_uInt16 nScTab ) const
{
    DBG_ASSERT( nScTab < mnScCnt, "XclExpTabIdBuffer::IsExportTable - out of range" );
    return (nScTab < mnScCnt) && (maIndexVec[ nScTab ].second == EXC_TABBUF_EXPORT);
}

sal_uInt16 XclExpTabIdBuffer::GetXclTab( sal_uInt16 nScTab ) const
{
    return (nScTab < mnScCnt) ? maIndexVec[ nScTab ].first : EXC_EXTSH_DELETED;
}

sal_uInt16 XclExpTabIdBuffer::GetRealScTab( sal_uInt16 nSortedTab ) const
{
    DBG_ASSERT( nSortedTab < mnScCnt, "XclExpTabIdBuffer::GetRealScIndex - out of range" );
    return (nSortedTab < mnScCnt) ? maFromSortedVec[ nSortedTab ] : EXC_NOTAB;
}

sal_uInt16 XclExpTabIdBuffer::GetSortedScTab( sal_uInt16 nScTab ) const
{
    DBG_ASSERT( nScTab < mnScCnt, "XclExpTabIdBuffer::GetSortedScIndex - out of range" );
    return (nScTab < mnScCnt) ? maToSortedVec[ nScTab ] : EXC_NOTAB;
}

void XclExpTabIdBuffer::StartRefLog()
{
    maRefLog.clear();
    mbEnableLog = true;
}

const XclExpRefLogVec& XclExpTabIdBuffer::EndRefLog()
{
    mbEnableLog = false;
    return maRefLog;
}

void XclExpTabIdBuffer::AppendTabRef( sal_uInt16 nXclFirst, sal_uInt16 nXclLast )
{
    if( mbEnableLog )
        maRefLog.push_back( ::std::pair< sal_uInt16, sal_uInt16 >( nXclFirst, nXclLast ) );
}

void XclExpTabIdBuffer::CalcXclIndexes()
{
    sal_uInt16 nXclTab = 0;
    sal_uInt16 nScTab = 0;

    // --- pass 1: process regular tables ---
    for( nScTab = 0; nScTab < mnScCnt; ++nScTab )
    {
        if( IsExportTable( nScTab ) )
        {
            maIndexVec[ nScTab ].first = nXclTab;
            ++nXclTab;
        }
        else
            maIndexVec[ nScTab ].first = EXC_EXTSH_DELETED;
    }
    mnXclCnt = nXclTab;

    // --- pass 2: process external tables (nXclTab continues) ---
    for( nScTab = 0; nScTab < mnScCnt; ++nScTab )
    {
        if( IsExternal( nScTab ) )
        {
            maIndexVec[ nScTab ].first = nXclTab;
            ++nXclTab;
        }
    }
    mnExtCnt = nXclTab - mnXclCnt;

    // result: first occur all exported tables, followed by all external tables
}


typedef ::std::pair< String, sal_uInt16 > XclExpTabName;

inline bool operator<( const XclExpTabName& rArg1, const XclExpTabName& rArg2 )
{
    // compare the sheet names only
    return ScGlobal::pCollator->compareString( rArg1.first, rArg2.first ) == COMPARE_LESS;
}

void XclExpTabIdBuffer::CalcSortedIndexes( ScDocument& rDoc )
{
    ::std::vector< XclExpTabName > aVec( mnScCnt );
    sal_uInt16 nScTab;

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

XclExpExtNameBase::XclExpExtNameBase( const String& rName, sal_uInt16 nFlags, sal_uInt32 nAddSize ) :
    XclExpRecord( EXC_ID_EXTERNNAME, rName.Len() + 8 + nAddSize ),
    maName( rName ),
    mnFlags( nFlags )
{
    DBG_ASSERT( maName.Len() <= 255, "XclExpExtNameBase::XclExpExtNameBase - string too long" );
}

XclExpExtNameBase::~XclExpExtNameBase()
{
}

void XclExpExtNameBase::WriteBody( XclExpStream& rStrm )
{
    rStrm   << mnFlags
            << sal_uInt32( 0 )
            << XclExpString( maName, EXC_STR_8BITLENGTH );
    WriteAddData( rStrm );
}

void XclExpExtNameBase::WriteAddData( XclExpStream& rStrm )
{
}


// ----------------------------------------------------------------------------

void XclExpExtNameAddIn::WriteAddData( XclExpStream& rStrm )
{
    rStrm << sal_uInt16( 2 ) << sal_uInt16( 0x171C );  // error value 0x17
}


// ----------------------------------------------------------------------------

XclExpExtNameDde::XclExpExtNameDde( const String& rName, sal_uInt16 nFlags ) :
    XclExpExtNameBase( rName, nFlags ),
    mnBaseSize( GetRecSize() )
{
}

bool XclExpExtNameDde::InsertDde(
        const XclExpRoot& rRoot,
        const String& rApplic, const String& rTopic, const String& rItem )
{
    ScDocument& rDoc = rRoot.GetDoc();
    sal_uInt16 nPos;

    bool bRet = (rDoc.FindDdeLink( rApplic, rTopic, rItem, SC_DDE_IGNOREMODE, nPos ) == TRUE);
    if( bRet )
    {
        // try to find results, not necessary for success (return value of this function)
        ScMatrix* pMatrix = NULL;
        sal_uInt16 nCols, nRows;
        if( rDoc.GetDdeLinkResultDimension( nPos, nCols, nRows, pMatrix ) && pMatrix )
        {
            mpMatrix.reset( new XclExpCachedMatrix( rDoc, nCols, nRows, pMatrix, true ) );
            SetRecSize( mnBaseSize + mpMatrix->GetSize() );
        }
        else
        {
            mpMatrix.reset();
            SetRecSize( mnBaseSize );
        }
    }
    return bRet;
}

void XclExpExtNameDde::WriteAddData( XclExpStream& rStrm )
{
    if( mpMatrix.get() )
        mpMatrix->Save( rStrm );
}


// List of external names =====================================================

sal_uInt16 XclExpExtNameList::InsertAddIn( const String& rName )
{
    sal_uInt16 nIndex = GetIndex( rName );
    return nIndex ? nIndex : Append( new XclExpExtNameAddIn( rName ) );
}

sal_uInt16 XclExpExtNameList::InsertDde(
        const XclExpRoot& rRoot,
        const String& rApplic, const String& rTopic, const String& rItem )
{
    if( !mbHasDde )
    {
        Append( new XclExpExtNameDde(
            String( RTL_CONSTASCII_USTRINGPARAM( "StdDocumentName" ) ),
            EXC_EXTN_EXPDDE_STDDOC ) );
        mbHasDde = true;
    }

    sal_uInt16 nIndex = GetIndex( rItem );
    if( !nIndex )
    {
        ::std::auto_ptr< XclExpExtNameDde > pName( new XclExpExtNameDde( rItem, EXC_EXTN_EXPDDE ) );
        if( pName->InsertDde( rRoot, rApplic, rTopic, rItem ) )
            nIndex = Append( pName.release() );
    }
    return nIndex;
}

void XclExpExtNameList::Save( XclExpStream& rStrm )
{
    maNameList.Save( rStrm );
}

sal_uInt16 XclExpExtNameList::GetIndex( const String& rName ) const
{
    for( const XclExpExtNameBase* pName = maNameList.First(); pName; pName = maNameList.Next() )
        if( pName->GetName() == rName )
            return ::ulimit< sal_uInt16 >( maNameList.GetCurPos() + 1 );
    return 0;
}

sal_uInt16 XclExpExtNameList::Append( XclExpExtNameBase* pName )
{
    maNameList.Append( pName );
    return ::ulimit< sal_uInt16 >( maNameList.Count() );
}


// Cached external cells ======================================================

XclExpCrn::XclExpCrn( sal_uInt16 nCol, sal_uInt16 nRow, sal_uInt8 nId, sal_uInt32 nAddLen ) :
    XclExpRecord( EXC_ID_CRN, 5 + nAddLen ),
    mnCol( nCol ),
    mnRow( nRow ),
    mnId( nId )
{
}

void XclExpCrn::WriteBody( XclExpStream& rStrm )
{
    rStrm   << static_cast< sal_uInt8 >( mnCol )
            << static_cast< sal_uInt8 >( mnCol )
            << mnRow
            << mnId;
    WriteAddData( rStrm );
}


// ----------------------------------------------------------------------------

XclExpCrnDouble::XclExpCrnDouble( sal_uInt16 nCol, sal_uInt16 nRow, double fVal ) :
    XclExpCrn( nCol, nRow, EXC_CACHEDVAL_DOUBLE, 8 ),
    mfVal( fVal )
{
}

void XclExpCrnDouble::WriteAddData( XclExpStream& rStrm )
{
    rStrm << mfVal;
}


// ----------------------------------------------------------------------------

XclExpCrnString::XclExpCrnString( sal_uInt16 nCol, sal_uInt16 nRow, const String& rText ) :
    XclExpCrn( nCol, nRow, EXC_CACHEDVAL_STRING ),
    maText( rText )
{
    // set correct size after maText is initialized
    SetRecSize( GetRecSize() + maText.GetSize() );
}

void XclExpCrnString::WriteAddData( XclExpStream& rStrm )
{
    rStrm << maText;
}


// ----------------------------------------------------------------------------

XclExpCrnBool::XclExpCrnBool( sal_uInt16 nCol, sal_uInt16 nRow, bool bBoolVal ) :
    XclExpCrn( nCol, nRow, EXC_CACHEDVAL_BOOL, 8 ),
    mnBool( bBoolVal ? 1 : 0 )
{
}

void XclExpCrnBool::WriteAddData( XclExpStream& rStrm )
{
    rStrm << mnBool;
    rStrm.WriteZeroBytes( 6 );
}


// Cached cells of a sheet ====================================================

XclExpXct::XclExpXct( const String& rTabName ) :
    XclExpRecord( EXC_ID_XCT, 4 ),
    maTable( rTabName )
{
}

void XclExpXct::StoreCellRange( const XclExpRoot& rRoot, const ScRange& rRange )
{
    ScDocument& rDoc = rRoot.GetDoc();
    SvNumberFormatter& rFormatter = rRoot.GetFormatter();
    sal_uInt16 nScTab = rRange.aStart.Tab();
    sal_uInt16 nLastCol = rRange.aEnd.Col();
    sal_uInt16 nLastRow = rRange.aEnd.Row();

    for( sal_uInt16 nRow = rRange.aStart.Row(); nRow <= nLastRow; ++nRow )
    {
        for( sal_uInt16 nCol = rRange.aStart.Col(); nCol <= nLastCol; ++nCol )
        {
            if( !maUsedCells.IsCellMarked( nCol, nRow, TRUE ) )
            {
                if( rDoc.HasValueData( nCol, nRow, nScTab ) )
                {
                    ScAddress   aAddr( nCol, nRow, nScTab );
                    double      fVal    = rDoc.GetValue( aAddr );
                    sal_uInt32  nFormat = rDoc.GetNumberFormat( aAddr );
                    sal_Int16   nType   = rFormatter.GetType( nFormat );
                    bool        bIsBool = (nType == NUMBERFORMAT_LOGICAL);

                    if( !bIsBool && ((nFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0) &&
                        (rDoc.GetCellType( aAddr ) == CELLTYPE_FORMULA) )
                    {
                        ScFormulaCell* pCell = static_cast< ScFormulaCell* >( rDoc.GetCell( aAddr ) );
                        if ( pCell )
                            bIsBool = (pCell->GetFormatType() == NUMBERFORMAT_LOGICAL);
                    }

                    if( bIsBool && ((fVal == 0.0) || (fVal == 1.0)) )
                        maCrnList.Append( new XclExpCrnBool( nCol, nRow, (fVal == 1.0) ) );
                    else
                        maCrnList.Append( new XclExpCrnDouble( nCol, nRow, fVal ) );
                }
                else
                {
                    String aText;
                    rDoc.GetString( nCol, nRow, nScTab, aText );
                    maCrnList.Append( new XclExpCrnString( nCol, nRow, aText ) );
                }
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
    sal_uInt16 nCount = ::ulimit< sal_uInt16 >( maCrnList.Count() );
    rStrm << nCount << mnXclTab;
}


// External document (SUPBOOK, BIFF8) =========================================

XclExpSupbook::XclExpSupbook( sal_uInt16 nTabs ) :
    XclExpRecord( EXC_ID_SUPBOOK, 4 ),
    meType( xlSBSelf ),
    mnTables( nTabs )
{
}

XclExpSupbook::XclExpSupbook() :
    XclExpRecord( EXC_ID_SUPBOOK, 4 ),
    meType( xlSBAddIn ),
    mnTables( 1 )
{
}

XclExpSupbook::XclExpSupbook( const XclExpRoot& rRoot, const String& rUrl ) :
    XclExpRecord( EXC_ID_SUPBOOK ),
    maUrl( rUrl ),
    maUrlEncoded( XclExpUrlHelper::EncodeUrl( rRoot, rUrl ) ),
    meType( xlSBUrl ),
    mnTables( 0 )
{
    SetRecSize( 2 + maUrlEncoded.GetSize() );
}

XclExpSupbook::XclExpSupbook( const XclExpRoot& rRoot, const String& rApplic, const String& rTopic ) :
    XclExpRecord( EXC_ID_SUPBOOK ),
    maUrl( rApplic ),
    maDdeTopic( rTopic ),
    maUrlEncoded( XclExpUrlHelper::EncodeDde( rApplic, rTopic ) ),
    meType( xlSBDde ),
    mnTables( 0 )
{
    SetRecSize( 2 + maUrlEncoded.GetSize() );
}

const XclExpString* XclExpSupbook::GetTableName( sal_uInt16 nXct ) const
{
    const XclExpXct* pXct = maXctList.GetObject( nXct );
    return pXct ? &pXct->GetTableName() : NULL;
}

bool XclExpSupbook::IsUrlLink( const String& rUrl ) const
{
    return (meType == xlSBUrl) && (maUrl == rUrl);
}

bool XclExpSupbook::IsDdeLink( const String& rApplic, const String& rTopic ) const
{
    return (meType == xlSBDde) && (maUrl == rApplic) && (maDdeTopic == rTopic);
}

void XclExpSupbook::StoreCellRange( const XclExpRoot& rRoot, const ScRange& rRange, sal_uInt16 nXct )
{
    if( XclExpXct* pXct = maXctList.GetObject( nXct ) )
        pXct->StoreCellRange( rRoot, rRange );
}

sal_uInt16 XclExpSupbook::InsertTable( const String& rTabName )
{
    DBG_ASSERT( meType == xlSBUrl, "XclExpSupbook::InsertTable - don't insert table names here" );
    XclExpXct* pXct = new XclExpXct( rTabName );
    SetRecSize( GetRecSize() + pXct->GetTableName().GetSize() );
    maXctList.Append( pXct );

    sal_uInt16 nTabNum = ::ulimit< sal_uInt16 >( maXctList.Count() - 1 );
    pXct->SetXclTab( nTabNum );
    return nTabNum;
}

sal_uInt16 XclExpSupbook::InsertAddIn( const String& rName )
{
    return GetExtNameList().InsertAddIn( rName );
}

sal_uInt16 XclExpSupbook::InsertDde( const XclExpRoot& rRoot, const String& rItem )
{
    return GetExtNameList().InsertDde( rRoot, maUrl, maDdeTopic, rItem );
}

XclExpExtNameList& XclExpSupbook::GetExtNameList()
{
    if( !mpExtNameList.get() )
        mpExtNameList.reset( new XclExpExtNameList );
    return *mpExtNameList;
}

void XclExpSupbook::Save( XclExpStream& rStrm )
{
    XclExpRecord::Save( rStrm );
    maXctList.Save( rStrm );
    if( mpExtNameList.get() )
        mpExtNameList->Save( rStrm );
}

void XclExpSupbook::WriteBody( XclExpStream& rStrm )
{
    switch( meType )
    {
        case xlSBSelf:
            rStrm << mnTables << EXC_SUPB_SELF;
        break;
        case xlSBUrl:
        case xlSBDde:
        {
            sal_uInt16 nCount = ::ulimit< sal_uInt16 >( maXctList.Count() );
            rStrm << nCount << maUrlEncoded;

            for( const XclExpXct* pXct = maXctList.First(); pXct; pXct = maXctList.Next() )
                rStrm << pXct->GetTableName();
        }
        break;
        case xlSBAddIn:
            rStrm << mnTables << EXC_SUPB_ADDIN;
        break;
        default:
            DBG_ERRORFILE( "XclExpSupbook::WriteBody - unknown SUPBOOK type" );
    }
}


// All SUPBOOKS in a document =================================================

XclExpSupbookBuffer::XclExpSupbookBuffer( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    mnAddInSB( 0xFFFF )
{
    XclExpTabIdBuffer& rTabBuffer = GetTabIdBuffer();

    sal_uInt16 nScCnt  = rTabBuffer.GetScTabCount();
    sal_uInt16 nXclCnt = rTabBuffer.GetXclTabCount();
    sal_uInt16 nExtCnt = rTabBuffer.GetExternTabCount();
    sal_uInt32 nCount = nXclCnt + nExtCnt;

    if( nCount )
    {
        maSBIndexBuffer.resize( nCount );
        maXtiBuffer.resize( nCount );

        // self-ref SUPBOOK first of list
        sal_uInt16 nSelfSB = Append( new XclExpSupbook( ::std::max( nXclCnt, rTabBuffer.GetCodenameCount() ) ) );
        for( sal_uInt16 nXclTab = 0; nXclTab < nXclCnt; ++nXclTab )
        {
            maSBIndexBuffer[ nXclTab ] = nSelfSB;
            maXtiBuffer[ nXclTab ] = nXclTab;
        }

        // add SUPBOOKs with external references
        for( sal_uInt16 nScTab = 0; nScTab < nScCnt; ++nScTab )
            if( rTabBuffer.IsExternal( nScTab ) )
                AddExt( nScTab );
    }
}

const XclExpString* XclExpSupbookBuffer::GetUrl( sal_uInt16 nXclTab ) const
{
    const XclExpSupbook* pSupbook = GetSupbook( nXclTab );
    const XclExpString* pString = pSupbook ? &pSupbook->GetUrlEncoded() : NULL;
    return (pString && pString->Len()) ? pString : NULL;
}

const XclExpString* XclExpSupbookBuffer::GetTableName( sal_uInt16 nXclTab ) const
{
    DBG_ASSERT( nXclTab < maXtiBuffer.size(), "XclExpSupbookBuffer::GetTableName - out of range" );
    const XclExpSupbook* pSupbook = GetSupbook( nXclTab );
    return pSupbook ? pSupbook->GetTableName( maXtiBuffer[ nXclTab ] ) : NULL;
}

void XclExpSupbookBuffer::GetXtiRange(
        sal_uInt16& rnSupbook, sal_uInt16& rnXtiFirst, sal_uInt16& rnXtiLast,
        sal_uInt16 nXclFirst, sal_uInt16 nXclLast ) const
{
    sal_uInt32 nCount = maSBIndexBuffer.size();
    if( (nXclFirst < nCount) && (nXclLast < nCount) )
    {
        // external reference
        rnSupbook = maSBIndexBuffer[ nXclFirst ];

        // all tables in the same supbook?
        bool bConflict = false;
        for( sal_uInt16 nXclTab = nXclFirst + 1; !bConflict && (nXclTab <= nXclLast); ++nXclTab )
        {
            bConflict = (maSBIndexBuffer[ nXclTab ] != rnSupbook);
            if( bConflict )
                nXclLast = nXclTab - 1;
        }
        rnXtiFirst = maXtiBuffer[ nXclFirst ];
        rnXtiLast = maXtiBuffer[ nXclLast ];
    }
    else
    {
        // internal reference
        rnSupbook = 0;
        rnXtiFirst = nXclFirst;
        rnXtiLast = nXclLast;
    }
}

void XclExpSupbookBuffer::StoreCellRange( const ScRange& rRange )
{
    sal_uInt16 nXclTab = GetTabIdBuffer().GetXclTab( rRange.aStart.Tab() );
    DBG_ASSERT( nXclTab < maXtiBuffer.size(), "XclExpSupbookBuffer::StoreCellRange - out of range" );

    if( XclExpSupbook* pBook = GetSupbook( nXclTab ) )
        pBook->StoreCellRange( GetRoot(), rRange, maXtiBuffer[ nXclTab ] );
}

void XclExpSupbookBuffer::InsertAddIn( sal_uInt16& rnSupbook, sal_uInt16& rnExtName, const String& rName )
{
    XclExpSupbook* pBook;
    if( mnAddInSB == 0xFFFF )
        mnAddInSB = Append( pBook = new XclExpSupbook );
    else
        pBook = maSupbookList.GetObject( mnAddInSB );
    DBG_ASSERT( pBook, "XclExpSupbookBuffer::InsertAddin - missing add-in supbook" );
    rnSupbook = mnAddInSB;
    rnExtName = pBook->InsertAddIn( rName );
}

void XclExpSupbookBuffer::InsertDde(
        sal_uInt16& rnSupbook, sal_uInt16& rnExtName,
        const String& rApplic, const String& rTopic, const String& rItem )
{
    XclExpSupbook* pBook = GetSupbookDde( rnSupbook, rApplic, rTopic );
    if( !pBook )
        rnSupbook = Append( pBook = new XclExpSupbook( GetRoot(), rApplic, rTopic ) );
    rnExtName = pBook->InsertDde( GetRoot(), rItem );
}

void XclExpSupbookBuffer::Save( XclExpStream& rStrm )
{
    maSupbookList.Save( rStrm );
}

XclExpSupbook* XclExpSupbookBuffer::GetSupbook( sal_uInt16 nXclTab ) const
{
    DBG_ASSERT( nXclTab < maSBIndexBuffer.size(), "XclSupbookList::GetSupbook - out of range" );
    return maSupbookList.GetObject( maSBIndexBuffer[ nXclTab ] );
}

XclExpSupbook* XclExpSupbookBuffer::GetSupbookUrl( sal_uInt16& rnIndex, const String& rUrl ) const
{
    for( XclExpSupbook* pBook = maSupbookList.First(); pBook; pBook = maSupbookList.Next() )
    {
        if( pBook->IsUrlLink( rUrl ) )
        {
            rnIndex = ::ulimit< sal_uInt16 >( maSupbookList.GetCurPos() );
            return pBook;
        }
    }
    return NULL;
}

XclExpSupbook* XclExpSupbookBuffer::GetSupbookDde( sal_uInt16& rnIndex, const String& rApplic, const String& rTopic ) const
{
    for( XclExpSupbook* pBook = maSupbookList.First(); pBook; pBook = maSupbookList.Next() )
    {
        if( pBook->IsDdeLink( rApplic, rTopic ) )
        {
            rnIndex = ::ulimit< sal_uInt16 >( maSupbookList.GetCurPos() );
            return pBook;
        }
    }
    return NULL;
}

sal_uInt16 XclExpSupbookBuffer::Append( XclExpSupbook* pBook )
{
    maSupbookList.Append( pBook );
    return ::ulimit< sal_uInt16 >( maSupbookList.Count() - 1 );
}

void XclExpSupbookBuffer::AddExt( sal_uInt16 nScTab )
{
    sal_uInt16 nXclTab = GetTabIdBuffer().GetXclTab( nScTab );
    DBG_ASSERT( nXclTab < maSBIndexBuffer.size(), "XclExpSupbookBuffer::AddExt - out of range" );
    sal_uInt16 nPos;

    // find ext doc name or append new one, save position in maSBIndexBuffer
    const String& rUrl = GetDoc().GetLinkDoc( nScTab );
    DBG_ASSERT( rUrl.Len(), "XclExpSupbookBuffer::AddExt - missing external linked sheet" );
    XclExpSupbook* pBook = GetSupbookUrl( nPos, rUrl );
    if( !pBook )
        nPos = Append( pBook = new XclExpSupbook( GetRoot(), rUrl ) );
    maSBIndexBuffer[ nXclTab ] = nPos;

    // append new table name, save position in maXtiBuffer
    nPos = pBook->InsertTable( GetDoc().GetLinkTab( nScTab ) );
    maXtiBuffer[ nXclTab ] = nPos;
}


// Export link manager ========================================================

XclExpLinkManager_Impl::XclExpLinkManager_Impl( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    maSBBuffer( rRoot )
{
    DBG_ASSERT_BIFF( GetBiff() >= xlBiff8 );
    FindXti( 0, 0 );   // dummy to avoid empty list
}

sal_uInt16 XclExpLinkManager_Impl::FindXti( sal_uInt16 nXclFirst, sal_uInt16 nXclLast )
{
    sal_uInt16 nSupb, nXtiFirst, nXtiLast;
    maSBBuffer.GetXtiRange( nSupb, nXtiFirst, nXtiLast, nXclFirst, nXclLast );
    return InsertXti( nSupb, nXtiFirst, nXtiLast );
}

void XclExpLinkManager_Impl::StoreCellRange( const SingleRefData& rRef1, const SingleRefData& rRef2 )
{
    for( sal_uInt16 nScTab = rRef1.nTab; nScTab <= rRef2.nTab; ++nScTab )
        if( GetTabIdBuffer().IsExternal( nScTab ) )
            maSBBuffer.StoreCellRange( ScRange(
                rRef1.nCol, rRef1.nRow, nScTab, rRef2.nCol, rRef2.nRow, nScTab ) );
}

void XclExpLinkManager_Impl::InsertAddIn( sal_uInt16& rnXti, sal_uInt16& rnExtName, const String& rName )
{
    sal_uInt16 nSupbook;
    maSBBuffer.InsertAddIn( nSupbook, rnExtName, rName );
    rnXti = InsertXti( nSupbook, EXC_EXTSH_EXTERNAL, EXC_EXTSH_EXTERNAL );
}

bool XclExpLinkManager_Impl::InsertDde(
        sal_uInt16& rnXti, sal_uInt16& rnExtName,
        const String& rApplic, const String& rTopic, const String& rItem )
{
    sal_uInt16 nSupbook;
    maSBBuffer.InsertDde( nSupbook, rnExtName, rApplic, rTopic, rItem );
    if( !rnExtName )
        return false;
    rnXti = InsertXti( nSupbook, EXC_EXTSH_EXTERNAL, EXC_EXTSH_EXTERNAL );
    return true;
}

sal_uInt16 XclExpLinkManager_Impl::AppendXti( XclExpXti* pXti )
{
    maXtiList.Append( pXti );
    return ::ulimit< sal_uInt16 >( maXtiList.Count() - 1 );
}

sal_uInt16 XclExpLinkManager_Impl::InsertXti( sal_uInt16 nSupbook, sal_uInt16 nXtiFirst, sal_uInt16 nXtiLast )
{
    for( const XclExpXti* pXti = maXtiList.First(); pXti; pXti = maXtiList.Next() )
        if( pXti->Equals( nSupbook, nXtiFirst, nXtiLast ) )
            return ::ulimit< sal_uInt16 >( maXtiList.GetCurPos() );
    return AppendXti( new XclExpXti( nSupbook, nXtiFirst, nXtiLast ) );
}

void XclExpLinkManager_Impl::Save( XclExpStream& rStrm )
{
    // SUPBOOKs, XCTs, CRNs, EXTERNNAMEs
    maSBBuffer.Save( rStrm );

    // EXTERNSHEET
    sal_uInt16 nCount = ::ulimit< sal_uInt16 >( maXtiList.Count() );
    rStrm.StartRecord( EXC_ID_EXTERNSHEET, 2 + 6 * nCount );
    rStrm << nCount;
    rStrm.SetSliceSize( 6 );
    for( const XclExpXti* pXti = maXtiList.First(); pXti; pXti = maXtiList.Next() )
        pXti->Save( rStrm );
    rStrm.EndRecord();
}


// ============================================================================

XclExpLinkManager::XclExpLinkManager( const XclExpRoot& rRoot ) :
    mpImpl( new XclExpLinkManager_Impl( rRoot ) )
{
}

XclExpLinkManager::~XclExpLinkManager()
{
}

sal_uInt16 XclExpLinkManager::FindXti( sal_uInt16 nXclFirst, sal_uInt16 nXclLast )
{
    return mpImpl->FindXti( nXclFirst, nXclLast );
}

const XclExpString* XclExpLinkManager::GetUrl( sal_uInt16 nXclTab ) const
{
    return mpImpl->GetUrl( nXclTab );
}

const XclExpString* XclExpLinkManager::GetTableName( sal_uInt16 nXclTab ) const
{
    return mpImpl->GetTableName( nXclTab );
}

void XclExpLinkManager::StoreCellCont( const SingleRefData& rRef )
{
    mpImpl->StoreCellRange( rRef, rRef );
}

void XclExpLinkManager::StoreCellRange( const SingleRefData& rRef1, const SingleRefData& rRef2 )
{
    mpImpl->StoreCellRange( rRef1, rRef2 );
}

void XclExpLinkManager::InsertAddIn( sal_uInt16& rnXti, sal_uInt16& rnExtName, const String& rName )
{
    mpImpl->InsertAddIn( rnXti, rnExtName, rName );
}

bool XclExpLinkManager::InsertDde(
        sal_uInt16& rnXti, sal_uInt16& rnExtName,
        const String& rApplic, const String& rTopic, const String& rItem )
{
    return mpImpl->InsertDde( rnXti, rnExtName, rApplic, rTopic, rItem );
}

void XclExpLinkManager::Save( XclExpStream& rStrm )
{
    mpImpl->Save( rStrm );
}


// ============================================================================


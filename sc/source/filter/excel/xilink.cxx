/*************************************************************************
 *
 *  $RCSfile: xilink.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 09:01:40 $
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

#ifndef SC_XILINK_HXX
#include "xilink.hxx"
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
#ifndef SC_RANGENAM_HXX
#include "rangenam.hxx"
#endif
#ifndef SC_TABLINK_HXX
#include "tablink.hxx"
#endif

#include "root.hxx"
#include "excform.hxx"
#include "excimp8.hxx"


// ============================================================================
// *** Helper classes ***
// ============================================================================

// Cached external cells ======================================================

/** Contains the address and value of an external referenced cell. */
class XclImpCrn : public XclImpCachedValue
{
public:
    /** Reads a cached value and stores it with its cell address. */
    explicit                    XclImpCrn( XclImpStream& rStrm, sal_uInt16 nXclCol, sal_uInt16 nXclRow );

    /** Copies the cached value to sheet nTab in the document. */
    void                        SetCell( ScDocument& rDoc, SCTAB nScTab ) const;

private:
    sal_uInt16                  mnCol;      /// Column index of the cached cell.
    sal_uInt16                  mnRow;      /// Row index of the cached cell.
};


// Sheet in an external document ==============================================

/** Contains the name and sheet index of one sheet in an external document. */
class XclImpSupbookTab
{
public:
    /** Stores the sheet name and marks the sheet index as invalid.
        The sheet index is set while creating the Calc sheet with CreateTable(). */
    explicit                    XclImpSupbookTab( const String& rTabName );
                                ~XclImpSupbookTab();

    inline const String&        GetTabName() const  { return maTabName; }
    inline SCTAB                GetScTab() const    { return mnScTab; }

    /** Reads a CRN record (external referenced cell) at the specified address. */
    void                        ReadCrn( XclImpStream& rStrm, sal_uInt16 nXclCol, sal_uInt16 nXclRow );

    /** Creates a new linked table in the passed document and fills it with the cached cells.
        @descr  Stores the index of the new sheet, will be accessible with GetScTab(). */
    void                        CreateAndFillTable(
                                    ScDocument& rDoc, const String& rAbsUrl,
                                    const String& rFilterName, const String& rFilterOpt );

private:
    typedef ScfDelList< XclImpCrn > XclImpCrnList;

    XclImpCrnList               maCrnList;      /// List of CRN records (cached cell values).
    String                      maTabName;      /// Name of the external sheet.
    SCTAB                       mnScTab;        /// New sheet index in Calc document.
};


// External document (SUPBOOK) ================================================

/** This class represents an external linked document (record SUPBOOK).
    @descr  Contains a list of all referenced sheets in the document. */
class XclImpSupbook
{
public:
    /** Reads the SUPBOOK record from stream. */
    explicit                    XclImpSupbook( XclImpStream& rStrm );

    /** Reads an XCT record (count of following CRNs and current sheet). */
    void                        ReadXct( XclImpStream& rStrm );
    /** Reads a CRN record (external referenced cell). */
    void                        ReadCrn( XclImpStream& rStrm );
    /** Reads an EXTERNNAME record. */
    void                        ReadExternname( XclImpStream& rStrm );

    /** Returns the SUPBOOK record type. */
    inline XclSupbookType       GetType() const { return meType; }

    /** Returns the URL of the external document. */
    inline const String&        GetXclUrl() const { return maXclUrl; }

    /** Returns Calc sheet index from Excel sheet index. */
    SCTAB                       GetScTabNum( sal_uInt16 nXclTab ) const;
    /** Returns Calc sheet index from sheet name. */
    SCTAB                       GetScTabNum( const String& rTabName ) const;

    /** Returns the external name specified by an index from the Excel document (one-based). */
    const XclImpExtName*        GetExternName( sal_uInt16 nXclIndex ) const;
    /** Tries to decode the URL to OLE or DDE link components.
        @descr  For DDE links: Decodes to application name and topic.
        For OLE object links: Decodes to class name and document URL.
        @return  true = decoding was successful, returned strings are valid (not empty). */
    bool                        GetLinkData( String& rApplic, String& rDoc ) const;

    /** Creates all sheets of this external document.
        @param nFirstTab  The external Excel index of the first sheet to be created.
        @param nLastTab  The external Excel index of the last sheet to be created. */
    void                        CreateTables(
                                    const XclImpRoot& rRoot,
                                    sal_uInt16 nSBTabFirst, sal_uInt16 nSBTabLast );

private:
    typedef ScfDelList< XclImpSupbookTab >  XclImpSupbookTabList;
    typedef ScfDelList< XclImpExtName >     XclImpExtNameList;

    XclImpSupbookTabList        maSupbTabList;      /// All sheet names of the document.
    XclImpExtNameList           maExtNameList;      /// All external names of the document.
    String                      maXclUrl;           /// URL of the external document (Excel mode).
    String                      maFilterName;       /// Detected filer name.
    String                      maFilterOpt;        /// Detected filer options.
    XclSupbookType              meType;             /// Type of the supbook record.
    sal_uInt16                  mnSBTab;            /// Current Excel sheet index from SUPBOOK for XCT/CRN records.
};


// Import link manager ========================================================

/** Contains the SUPBOOK index and sheet indexes of an external link.
    @descr  It is possible to enter a formula like =SUM(Sheet1:Sheet3!A1),
    therefore here occurs a sheet range. */
struct XclImpXti
{
    sal_uInt16                  mnSupbook;      /// Index to SUPBOOK record.
    sal_uInt16                  mnSBTabFirst;   /// Index to the first sheet of the range in the SUPBOOK.
    sal_uInt16                  mnSBTabLast;    /// Index to the last sheet of the range in the SUPBOOK.
};

inline XclImpStream& operator>>( XclImpStream& rStrm, XclImpXti& rXti )
{
    return rStrm >> rXti.mnSupbook >> rXti.mnSBTabFirst >> rXti.mnSBTabLast;
}


// ----------------------------------------------------------------------------

/** Implementation of the link manager. */
class XclImpLinkManager_Impl : protected XclImpRoot
{
public:
    explicit                    XclImpLinkManager_Impl( const XclImpRoot& rRoot );

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

    /** Returns true, if the specified XTI entry contains an internal reference. */
    bool                        IsSelfRef( sal_uInt16 nXtiIndex ) const;
    /** Returns the Calc sheet index range of the specified XTI entry.
        @return  true = XTI data found, returned sheet index range is valid. */
    bool                        GetScTabRange(
                                    SCTAB& rnScTabFirst, SCTAB& rnScTabLast,
                                    sal_uInt16 nXtiIndex ) const;
    /** Returns the specified external name or 0 on error. */
    const XclImpExtName*        GetExternName( sal_uInt16 nXtiIndex, sal_uInt16 nExtName ) const;
    /** Tries to decode the URL of the specified XTI entry to OLE or DDE link components.
        @descr  For DDE links: Decodes to application name and topic.
        For OLE object links: Decodes to class name and document URL.
        @return  true = decoding was successful, returned strings are valid (not empty). */
    bool                        GetLinkData( String& rApplic, String& rTopic, sal_uInt16 nXtiIndex ) const;

    /** Returns the Calc sheet index of a table in an external document.
        @return  Calc sheet index or EXC_TAB_INVALID on error. */
    SCTAB                       GetScTab( const String& rUrl, const String& rTabName ) const;

private:
    /** Returns the specified SUPBOOK (external document). */
    const XclImpSupbook*        GetSupbook( sal_uInt32 nXtiIndex ) const;
    /** Returns the SUPBOOK (external workbook) specified by its URL. */
    const XclImpSupbook*        GetSupbook( const String& rUrl ) const;

    /** Creates all external sheets in the Calc document. */
    void                        CreateTables();

    /** Finds the largest range of sheet indexes in a SUPBOOK after a start sheet index.
        @param rnSBTabFirst  (out-param) The first sheet index of the range in SUPBOOK is returned here.
        @param rnSBTabLast  (out-param) The last sheet index of the range in SUPBOOK is returned here (inclusive).
        @param nSupbook  The list index of the SUPBOOK.
        @param nSBTabStart  The first allowed sheet index. Sheet ranges with an earlier start index are ignored.
        @return  true = the return values are valid; false = nothing found. */
    bool                        FindNextTabRange(
                                    sal_uInt16& rnSBTabFirst, sal_uInt16& rnSBTabLast,
                                    sal_uInt16 nSupbook, sal_uInt16 nSBTabStart ) const;

private:
    typedef ScfDelList< XclImpXti >     XclImpXtiList;
    typedef ScfDelList< XclImpSupbook > XclImpSupbookList;

    XclImpXtiList               maXtiList;          /// List of all XTI structures.
    XclImpSupbookList           maSupbookList;      /// List of external documents.
    bool                        mbCreated;          /// true = Calc sheets already created.
};


// ============================================================================
// *** Implementation ***
// ============================================================================

// Excel sheet indexes ========================================================

// original Excel sheet names -------------------------------------------------

void XclImpTabInfo::AppendXclTabName( const String& rXclTabName, SCTAB nScTab )
{
    maTabNames[ rXclTabName ] = nScTab;
}

void XclImpTabInfo::InsertScTab( SCTAB nScTab )
{
    for( XclTabNameMap::iterator aIt = maTabNames.begin(), aEnd = maTabNames.end(); aIt != aEnd; ++aIt )
        if( aIt->second >= nScTab )
            ++aIt->second;
}

SCTAB XclImpTabInfo::GetScTabFromXclName( const String& rXclTabName ) const
{
    XclTabNameMap::const_iterator aIt = maTabNames.find( rXclTabName );
    return (aIt != maTabNames.end()) ? aIt->second : SCNOTAB;
}

// record creation order - TABID record ---------------------------------------

void XclImpTabInfo::ReadTabid( XclImpStream& rStrm )
{
    DBG_ASSERT_BIFF( rStrm.GetRoot().GetBiff() == xlBiff8 );
    if( rStrm.GetRoot().GetBiff() == xlBiff8 )
    {
        sal_uInt32 nReadCount = rStrm.GetRecLeft() / 2;
        DBG_ASSERT( nReadCount <= 0xFFFF, "XclImpTabInfo::ReadTabid - record too long" );
        maTabIdVec.clear();
        maTabIdVec.reserve( nReadCount );
        for( sal_uInt32 nIndex = 0; rStrm.IsValid() && (nIndex < nReadCount); ++nIndex )
            // #93471# zero index is not allowed in BIFF8, but it seems that it occurs in real life
            maTabIdVec.push_back( rStrm.ReaduInt16() );
    }
}

sal_uInt16 XclImpTabInfo::GetCurrentIndex( sal_uInt16 nCreatedId, sal_uInt16 nMaxTabId ) const
{
    sal_uInt16 nReturn = 0;
    for( ScfUInt16Vec::const_iterator aIt = maTabIdVec.begin(), aEnd = maTabIdVec.end(); aIt != aEnd; ++aIt )
    {
        sal_uInt16 nValue = *aIt;
        if( nValue == nCreatedId )
            return nReturn;
        if( nValue <= nMaxTabId )
            ++nReturn;
    }
    return 0;
}


// Internal defined names =====================================================

XclImpName::XclImpName( XclImpStream& rStrm, sal_uInt16 nScIndex ) :
    XclImpRoot( rStrm.GetRoot() ),
    mpScData( NULL ),
    mcBuiltIn( EXC_BUILTIN_UNKNOWN ),
    mnScTab( SCNOTAB )
{
    ExcelToSc& rFmlaConv = GetFmlaConverter();
    ScRangeName& rRangeNames = GetNamedRanges();

    // 1) *** read data from stream *** ---------------------------------------

    sal_uInt16 nFlags = 0, nFmlaSize, nXclTab = EXC_NAME_GLOBAL;
    sal_uInt8 nNameLen, nShortCut;

    switch( GetBiff() )
    {
        case xlBiff2:
        {
            sal_uInt8 nFlagsBiff2;
            rStrm >> nFlagsBiff2;
            rStrm.Ignore( 1 );
            rStrm >> nShortCut >> nNameLen;
            nFmlaSize = rStrm.ReaduInt8();
            ::set_flag( nFlags, EXC_NAME_FUNC, ::get_flag( nFlagsBiff2, EXC_NAME2_FUNC ) );
        }
        break;

        case xlBiff3:
        case xlBiff4:
        {
            rStrm >> nFlags >> nShortCut >> nNameLen >> nFmlaSize;
        }
        break;

        case xlBiff5:
        case xlBiff7:
        case xlBiff8:
        {
            rStrm >> nFlags >> nShortCut >> nNameLen >> nFmlaSize;
            rStrm.Ignore( 2 );
            rStrm >> nXclTab;
            rStrm.Ignore( 4 );
        }
        break;

        default: DBG_ERROR_BIFF();
    }

    if( GetBiff() < xlBiff8 )
        maXclName = rStrm.ReadRawByteString( nNameLen );
    else
        maXclName = rStrm.ReadUniString( nNameLen );

    // 2) *** convert sheet index and name *** --------------------------------

    // get built-in name, or convert characters invalid in Calc
    bool bBuiltIn = ::get_flag( nFlags, EXC_NAME_BUILTIN );

    bool bVBName = ::get_flag( nFlags, EXC_NAME_VB);

    // special case for BIFF5 filter range - name appears as plain text without built-in flag
    if( ((GetBiff() == xlBiff5) || (GetBiff() == xlBiff7)) &&
        (maXclName == XclTools::GetXclBuiltInDefName( EXC_BUILTIN_AUTOFILTER )) )
    {
        bBuiltIn = true;
        maXclName.Assign( EXC_BUILTIN_AUTOFILTER );
    }

    // convert Excel name to Calc name
    if( bBuiltIn )
    {
        if( maXclName.Len() )
            mcBuiltIn = maXclName.GetChar( 0 );
        maScName = XclTools::GetBuiltInDefName( mcBuiltIn );
    }
    else
    {
        maScName = maXclName;
        ScfTools::ConvertToScDefinedName( maScName );
    }

    // add sheet index for local names
    if( nXclTab != EXC_NAME_GLOBAL )
    {
        maScName.Append( '_' ).Append( String::CreateFromInt32( nXclTab ) );
        mnScTab = static_cast< SCTAB >( nXclTab - 1 );
    }

    // find an unused name
    String aOrigName( maScName );
    sal_Int32 nCounter = 0;
    sal_uInt16 nDummy;
    while( rRangeNames.SearchName( maScName, nDummy ) )
        maScName.Assign( aOrigName ).Append( ' ' ).Append( String::CreateFromInt32( ++nCounter ) );

    // 3) *** convert the name definition formula *** -------------------------

    rFmlaConv.Reset();
    const ScTokenArray* pTokArr = NULL; // pointer to token array, owned by rFmlaConv
    RangeType nNameType = RT_NAME;

    sal_uInt16 nUnsupported = EXC_NAME_VB | EXC_NAME_PROC | EXC_NAME_BIG;
    if( ::get_flag( nFlags, nUnsupported ) )
    {
        // special, unsupported name
        rFmlaConv.GetDummy( pTokArr );
    }
    else if( bBuiltIn )
    {
        // --- print ranges or title ranges ---
        rStrm.PushPosition();
        switch( mcBuiltIn )
        {
            case EXC_BUILTIN_PRINTAREA:
                if( rFmlaConv.Convert( GetPrintAreaBuffer(), nFmlaSize, FT_RangeName ) == ConvOK )
                    nNameType |= RT_PRINTAREA;
            break;
            case EXC_BUILTIN_PRINTTITLES:
                if( rFmlaConv.Convert( GetTitleAreaBuffer(), nFmlaSize, FT_RangeName ) == ConvOK )
                    nNameType |= RT_COLHEADER | RT_ROWHEADER;
            break;
        }
        rStrm.PopPosition();

        // --- name formula ---
        rFmlaConv.Convert( pTokArr, nFmlaSize, FT_RangeName );

        // --- auto or advanced filter ---
        if( (GetBiff() >= xlBiff8) && pTokArr && bBuiltIn )
        {
            ScRange aRange;
            if( pTokArr->IsReference( aRange ) )
            {
                switch( mcBuiltIn )
                {
                    case EXC_BUILTIN_AUTOFILTER:
                        GetFilterManager().Insert( mpRD, aRange, maScName );
                    break;
                    case EXC_BUILTIN_CRITERIA:
                        GetFilterManager().AddAdvancedRange( aRange );
                        nNameType |= RT_CRITERIA;
                    break;
                    case EXC_BUILTIN_EXTRACT:
                        if( pTokArr->IsValidReference( aRange ) )
                            GetFilterManager().AddExtractPos( aRange );
                    break;
                }
            }
        }
    }
    else
    {
        // regular defined name
        rFmlaConv.Convert( pTokArr, nFmlaSize, FT_RangeName );
    }

    // 4) *** create a defined name in the Calc document *** ------------------

    if( pTokArr && (bBuiltIn || !::get_flag( nFlags, EXC_NAME_HIDDEN )) && !bVBName )
    {
        // create the Calc name data
        ScRangeData* pData = new ScRangeData( GetDocPtr(), maScName, *pTokArr, ScAddress(), nNameType );
        pData->GuessPosition();
        pData->SetIndex( nScIndex );
        rRangeNames.Insert( pData );        // takes ownership of pData
        mpScData = pData;                   // cache for later use
    }
}


// ----------------------------------------------------------------------------

XclImpNameBuffer::XclImpNameBuffer( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}

void XclImpNameBuffer::ReadName( XclImpStream& rStrm )
{
    sal_uInt32 nCount = maNameList.Count();
    if( nCount < 0xFFFF )
        maNameList.Append( new XclImpName( rStrm, static_cast< sal_uInt16 >( nCount + 1 ) ) );
}

const XclImpName* XclImpNameBuffer::FindName( const String& rXclName, SCTAB nScTab ) const
{
    const XclImpName* pGlobalName = NULL;   // a found global name
    const XclImpName* pLocalName = NULL;    // a found local name
    for( const XclImpName* pName = maNameList.First(); pName && !pLocalName; pName = maNameList.Next() )
    {
        if( pName->GetScTab() == nScTab )
            pLocalName = pName;
        else if( pName->IsGlobal() )
            pGlobalName = pName;
    }
    return pLocalName ? pLocalName : pGlobalName;
}

const XclImpName* XclImpNameBuffer::GetNameFromIndex( sal_uInt16 nXclIndex ) const
{
    DBG_ASSERT( nXclIndex > 0, "XclImpNameBuffer::GetNameFromIndex - index must be > 0" );
    return  nXclIndex <= maNameList.Count() ? maNameList.GetObject( nXclIndex - 1 ): NULL;
}

// External names =============================================================

XclImpExtName::XclImpExtName( XclImpStream& rStrm, bool bAddIn )
{
    sal_uInt16 nFlags;
    sal_uInt8 nLen;

    rStrm >> nFlags >> mnStorageId >> nLen ;
    maName = rStrm.ReadUniString( nLen );

    if( ::get_flag( nFlags, EXC_EXTN_BUILTIN ) || !::get_flag( nFlags, EXC_EXTN_OLE_OR_DDE ) )
    {
        if( bAddIn )
        {
            meType = xlExtAddIn;
            maName = rStrm.GetRoot().GetScAddInName( maName );
        }
        else
        {
            meType = xlExtName;
            ScfTools::ConvertToScDefinedName( maName );
        }
    }
    else
    {
        meType = ::get_flagvalue( nFlags, EXC_EXTN_OLE, xlExtOLE, xlExtDDE );
    }

    if( (meType == xlExtDDE) && (rStrm.GetRecLeft() > 1) )
        mpDdeMatrix.reset( new XclImpCachedMatrix( rStrm ) );
}

void XclImpExtName::CreateDdeData( ScDocument& rDoc, const String& rApplic, const String& rTopic ) const
{
    ScMatrixRef pResults = mpDdeMatrix.get() ? mpDdeMatrix->CreateScMatrix() : (ScMatrixRef)NULL;
    rDoc.CreateDdeLink( rApplic, rTopic, maName, SC_DDE_DEFAULT, pResults );
}


// Cached external cells ======================================================

XclImpCrn::XclImpCrn( XclImpStream& rStrm, sal_uInt16 nCol, sal_uInt16 nRow ) :
    XclImpCachedValue( rStrm ),
    mnCol( nCol ),
    mnRow( nRow )
{
}

void XclImpCrn::SetCell( ScDocument& rDoc, SCTAB nScTab ) const
{
    const ScAddress aPos( static_cast< SCCOL >( mnCol ), static_cast< SCROW >( mnRow ), nScTab );

    switch( GetType() )
    {
        case EXC_CACHEDVAL_DOUBLE:
            rDoc.SetValue( aPos.Col(), aPos.Row(), aPos.Tab(), GetValue() );
        break;
        case EXC_CACHEDVAL_STRING:
            rDoc.PutCell( aPos, new ScStringCell( GetString() ) );
        break;
        case EXC_CACHEDVAL_BOOL:
        case EXC_CACHEDVAL_ERROR:
        {
            ScFormulaCell* pFmlaCell = new ScFormulaCell( &rDoc, aPos, GetBoolErrFmla() );
            pFmlaCell->SetDouble( GetBool() ? 1.0 : 0.0 );  // GetBool() returns false for error codes
            rDoc.PutCell( aPos, pFmlaCell );
        }
        break;
    }
}


// Sheet in an external document ==============================================

XclImpSupbookTab::XclImpSupbookTab( const String& rTabName ) :
    maTabName( rTabName ),
    mnScTab( SCNOTAB )
{
}

XclImpSupbookTab::~XclImpSupbookTab()
{
}

void XclImpSupbookTab::ReadCrn( XclImpStream& rStrm, sal_uInt16 nXclCol, sal_uInt16 nXclRow )
{
    maCrnList.Append( new XclImpCrn( rStrm, nXclCol, nXclRow ) );
}

void XclImpSupbookTab::CreateAndFillTable(
        ScDocument& rDoc, const String& rAbsUrl, const String& rFilterName, const String& rFilterOpt )
{
    if( mnScTab == SCNOTAB )
        if( rDoc.InsertLinkedEmptyTab( mnScTab, rAbsUrl, rFilterName, rFilterOpt, maTabName ) )
            for( const XclImpCrn* pCrn = maCrnList.First(); pCrn; pCrn = maCrnList.Next() )
                pCrn->SetCell( rDoc, mnScTab );
}


// External document (SUPBOOK) ================================================

XclImpSupbook::XclImpSupbook( XclImpStream& rStrm ) :
    mnSBTab( EXC_EXTSH_DELETED ),
    meType( EXC_SBTYPE_UNKNOWN )
{
    sal_uInt16 nSBTabCnt;
    rStrm >> nSBTabCnt;

    if( rStrm.GetRecLeft() == 2 )
    {
        switch( rStrm.ReaduInt16() )
        {
            case EXC_SUPB_SELF:     meType = EXC_SBTYPE_SELF;   break;
            case EXC_SUPB_ADDIN:    meType = EXC_SBTYPE_ADDIN;  break;
            default:    DBG_ERRORFILE( "XclImpSupbook::XclImpSupbook - unknown special SUPBOOK type" );
        }
        return;
    }

    String aEncUrl( rStrm.ReadUniString() );
    bool bSelf = false;
    XclImpUrlHelper::DecodeUrl( maXclUrl, bSelf, rStrm.GetRoot(), aEncUrl );

    if( nSBTabCnt )
    {
        meType = EXC_SBTYPE_EXTERN;
        for( sal_uInt16 nSBTab = 0; nSBTab < nSBTabCnt; ++nSBTab )
        {
            String aTabName( rStrm.ReadUniString() );
            ScfTools::ConvertToScSheetName( aTabName );
            maSupbTabList.Append( new XclImpSupbookTab( aTabName ) );
        }
    }
    else
    {
        meType = EXC_SBTYPE_SPECIAL;
        // create dummy list entry
        maSupbTabList.Append( new XclImpSupbookTab( maXclUrl ) );
    }
}

void XclImpSupbook::ReadXct( XclImpStream& rStrm )
{
    rStrm.Ignore( 2 );
    rStrm >> mnSBTab;
}

void XclImpSupbook::ReadCrn( XclImpStream& rStrm )
{
    if( XclImpSupbookTab* pSBTab = maSupbTabList.GetObject( mnSBTab ) )
    {
        sal_uInt8 nXclColLast, nXclColFirst;
        sal_uInt16 nXclRow;
        rStrm >> nXclColLast >> nXclColFirst >> nXclRow;

        for( sal_uInt8 nXclCol = nXclColFirst; (nXclCol <= nXclColLast) && (rStrm.GetRecLeft() > 1); ++nXclCol )
            pSBTab->ReadCrn( rStrm, nXclCol, nXclRow );
    }
}

void XclImpSupbook::ReadExternname( XclImpStream& rStrm )
{
    maExtNameList.Append( new XclImpExtName( rStrm, meType == EXC_SBTYPE_ADDIN ) );
}

SCTAB XclImpSupbook::GetScTabNum( sal_uInt16 nXclTab ) const
{
    if( meType == EXC_SBTYPE_SELF )
        return static_cast< SCTAB >( nXclTab );
    const XclImpSupbookTab* pSBTab = maSupbTabList.GetObject( nXclTab );
    return pSBTab ? pSBTab->GetScTab() : SCNOTAB;
}

SCTAB XclImpSupbook::GetScTabNum( const String& rTabName ) const
{
    for( const XclImpSupbookTab* pSBTab = maSupbTabList.First(); pSBTab; pSBTab = maSupbTabList.Next() )
        if( pSBTab->GetTabName() == rTabName )
            return pSBTab->GetScTab();
    return SCNOTAB;
}

const XclImpExtName* XclImpSupbook::GetExternName( sal_uInt16 nXclIndex ) const
{
    DBG_ASSERT( nXclIndex > 0, "XclImpSupbook::GetExternName - index must be >0" );
    return (meType == EXC_SBTYPE_SELF) ? 0 : maExtNameList.GetObject( nXclIndex - 1 );
}

bool XclImpSupbook::GetLinkData( String& rApplic, String& rTopic ) const
{
    return (meType == EXC_SBTYPE_SPECIAL) && XclImpUrlHelper::DecodeLink( rApplic, rTopic, maXclUrl );
}

void XclImpSupbook::CreateTables( const XclImpRoot& rRoot, sal_uInt16 nSBTabFirst, sal_uInt16 nSBTabLast )
{
    if( (meType == EXC_SBTYPE_EXTERN) && (rRoot.GetExtDocOptions().nLinkCnt < 1) && rRoot.GetDocShell() )
    {
        String aAbsUrl( ScGlobal::GetAbsDocName( maXclUrl, rRoot.GetDocShell() ) );

        // get filter name for external document
        if( !maFilterName.Len() )
            ScDocumentLoader::GetFilterName( aAbsUrl, maFilterName, maFilterOpt, FALSE );

        // create tables
        for( sal_uInt16 nSBTab = nSBTabFirst; nSBTab <= nSBTabLast; ++nSBTab )
            if( XclImpSupbookTab* pSBTab = maSupbTabList.GetObject( nSBTab ) )
                pSBTab->CreateAndFillTable( rRoot.GetDoc(), aAbsUrl, maFilterName, maFilterOpt );
    }
}


// Import link manager ========================================================

XclImpLinkManager_Impl::XclImpLinkManager_Impl( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    mbCreated( false )
{
}

void XclImpLinkManager_Impl::ReadExternsheet( XclImpStream& rStrm )
{
    sal_uInt16 nXtiCount;
    rStrm >> nXtiCount;

    XclImpXti* pXti;
    while( nXtiCount )
    {
        pXti = new XclImpXti;
        rStrm >> *pXti;
        maXtiList.Append( pXti );
        --nXtiCount;
    }

    CreateTables();
}

void XclImpLinkManager_Impl::ReadSupbook( XclImpStream& rStrm )
{
    maSupbookList.Append( new XclImpSupbook( rStrm ) );
}

void XclImpLinkManager_Impl::ReadXct( XclImpStream& rStrm )
{
    if( XclImpSupbook* pSupbook = maSupbookList.Last() )
        pSupbook->ReadXct( rStrm );
}

void XclImpLinkManager_Impl::ReadCrn( XclImpStream& rStrm )
{
    if( XclImpSupbook* pSupbook = maSupbookList.Last() )
        pSupbook->ReadCrn( rStrm );
}

void XclImpLinkManager_Impl::ReadExternname( XclImpStream& rStrm )
{
    if( XclImpSupbook* pSupbook = maSupbookList.Last() )
        pSupbook->ReadExternname( rStrm );
}

bool XclImpLinkManager_Impl::IsSelfRef( sal_uInt16 nXtiIndex ) const
{
    const XclImpSupbook* pSupbook = GetSupbook( nXtiIndex );
    return pSupbook && (pSupbook->GetType() == EXC_SBTYPE_SELF);
}

bool XclImpLinkManager_Impl::GetScTabRange(
        SCTAB& rnScTabFirst, SCTAB& rnScTabLast, sal_uInt16 nXtiIndex ) const
{
    if( const XclImpXti* pXti = maXtiList.GetObject( nXtiIndex ) )
    {
        if( const XclImpSupbook* pSupbook = maSupbookList.GetObject( pXti->mnSupbook ) )
        {
            rnScTabFirst = pSupbook->GetScTabNum( pXti->mnSBTabFirst );
            rnScTabLast = pSupbook->GetScTabNum( pXti->mnSBTabLast );
            return true;
        }
    }
    return false;
}

const XclImpExtName* XclImpLinkManager_Impl::GetExternName( sal_uInt16 nXtiIndex, sal_uInt16 nExtName ) const
{
    const XclImpSupbook* pSupbook = GetSupbook( nXtiIndex );
    return pSupbook ? pSupbook->GetExternName( nExtName ) : NULL;
}

bool XclImpLinkManager_Impl::GetLinkData( String& rApplic, String& rTopic, sal_uInt16 nXtiIndex ) const
{
    const XclImpSupbook* pSupbook = GetSupbook( nXtiIndex );
    return pSupbook && pSupbook->GetLinkData( rApplic, rTopic );
}

SCTAB XclImpLinkManager_Impl::GetScTab( const String& rUrl, const String& rTabName ) const
{
    const XclImpSupbook* pSupbook = GetSupbook( rUrl );
    return pSupbook ? pSupbook->GetScTabNum( rTabName ) : SCNOTAB;
}

const XclImpSupbook* XclImpLinkManager_Impl::GetSupbook( sal_uInt32 nXtiIndex ) const
{
    const XclImpXti* pXti = maXtiList.GetObject( nXtiIndex );
    return pXti ? maSupbookList.GetObject( pXti->mnSupbook ) : NULL;
}

const XclImpSupbook* XclImpLinkManager_Impl::GetSupbook( const String& rUrl ) const
{
    for( const XclImpSupbook* pSupbook = maSupbookList.First(); pSupbook; pSupbook = maSupbookList.Next() )
        if( pSupbook->GetXclUrl() == rUrl )
            return pSupbook;
    return NULL;
}

void XclImpLinkManager_Impl::CreateTables()
{
    DBG_ASSERT( !mbCreated, "XclImpLinkManager::CreateTables - multiple call" );
    if( mbCreated ) return;

    sal_uInt16 nSBTabFirst, nSBTabLast;
    sal_uInt32 nCount = maSupbookList.Count();

    for( sal_uInt16 nSupbook = 0; nSupbook < nCount; ++nSupbook )
    {
        XclImpSupbook* pSupbook = maSupbookList.GetObject( nSupbook );
        bool bLoop = FindNextTabRange( nSBTabFirst, nSBTabLast, nSupbook, 0 );
        while( bLoop && pSupbook )
        {
            pSupbook->CreateTables( *this, nSBTabFirst, nSBTabLast );
            // #96263# don't search again if last sheet == EXC_NOTAB
            bLoop = (nSBTabLast != EXC_NOTAB) && FindNextTabRange( nSBTabFirst, nSBTabLast, nSupbook, nSBTabLast + 1 );
        }
    }
    mbCreated = true;
}

bool XclImpLinkManager_Impl::FindNextTabRange(
        sal_uInt16& rnSBTabFirst, sal_uInt16& rnSBTabLast,
        sal_uInt16 nSupbook, sal_uInt16 nSBTabStart ) const
{
    rnSBTabFirst = rnSBTabLast = EXC_NOTAB;
    for( const XclImpXti* pXti = maXtiList.First(); pXti; pXti = maXtiList.Next() )
    {
        if( (nSupbook == pXti->mnSupbook) && (nSBTabStart <= pXti->mnSBTabLast) && (pXti->mnSBTabFirst < rnSBTabFirst) )
        {
            rnSBTabFirst = ::std::max( nSBTabStart, pXti->mnSBTabFirst );
            rnSBTabLast = pXti->mnSBTabLast;
        }
    }
    return rnSBTabFirst != EXC_NOTAB;
}


// ============================================================================

XclImpLinkManager::XclImpLinkManager( const XclImpRoot& rRoot ) :
    mpImpl( new XclImpLinkManager_Impl( rRoot ) )
{
}

XclImpLinkManager::~XclImpLinkManager()
{
}

void XclImpLinkManager::ReadExternsheet( XclImpStream& rStrm )
{
    mpImpl->ReadExternsheet( rStrm );
}

void XclImpLinkManager::ReadSupbook( XclImpStream& rStrm )
{
    mpImpl->ReadSupbook( rStrm );
}

void XclImpLinkManager::ReadXct( XclImpStream& rStrm )
{
    mpImpl->ReadXct( rStrm );
}

void XclImpLinkManager::ReadCrn( XclImpStream& rStrm )
{
    mpImpl->ReadCrn( rStrm );
}

void XclImpLinkManager::ReadExternname( XclImpStream& rStrm )
{
    mpImpl->ReadExternname( rStrm );
}

bool XclImpLinkManager::IsSelfRef( sal_uInt16 nXtiIndex ) const
{
    return mpImpl->IsSelfRef( nXtiIndex );
}

bool XclImpLinkManager::GetScTabRange(
        SCTAB& rnScTabFirst, SCTAB& rnScTabLast, sal_uInt16 nXtiIndex ) const
{
    return mpImpl->GetScTabRange( rnScTabFirst, rnScTabLast, nXtiIndex );
}

const XclImpExtName* XclImpLinkManager::GetExternName( sal_uInt16 nXtiIndex, sal_uInt16 nExtName ) const
{
    return mpImpl->GetExternName( nXtiIndex, nExtName );
}

bool XclImpLinkManager::GetLinkData( String& rApplic, String& rTopic, sal_uInt16 nXtiIndex ) const
{
    return mpImpl->GetLinkData( rApplic, rTopic, nXtiIndex );
}

SCTAB XclImpLinkManager::GetScTab( const String& rUrl, const String& rTabName ) const
{
    return mpImpl->GetScTab( rUrl, rTabName );
}


// ============================================================================


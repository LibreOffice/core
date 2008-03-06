/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xilink.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 15:46:04 $
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

#ifndef SC_XILINK_HXX
#include "xilink.hxx"
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
#ifndef SC_TABLINK_HXX
#include "tablink.hxx"
#endif

#ifndef SC_XISTREAM_HXX
#include "xistream.hxx"
#endif
#ifndef SC_XIHELPER_HXX
#include "xihelper.hxx"
#endif
#ifndef SC_XINAME_HXX
#include "xiname.hxx"
#endif

// ============================================================================
// *** Helper classes ***
// ============================================================================

// Cached external cells ======================================================

/** Contains the address and value of an external referenced cell. */
class XclImpCrn : public XclImpCachedValue
{
public:
    /** Reads a cached value and stores it with its cell address. */
    explicit            XclImpCrn( XclImpStream& rStrm, const XclAddress& rXclPos );

    /** Copies the cached value to sheet nTab in the document. */
    void                SetCell( const XclImpRoot& rRoot, SCTAB nScTab ) const;

private:
    XclAddress          maXclPos;       /// Excel position of the cached cell.
};

// Sheet in an external document ==============================================

/** Contains the name and sheet index of one sheet in an external document. */
class XclImpSupbookTab
{
public:
    /** Stores the sheet name and marks the sheet index as invalid.
        The sheet index is set while creating the Calc sheet with CreateTable(). */
    explicit            XclImpSupbookTab( const String& rTabName );
                        ~XclImpSupbookTab();

    inline const String& GetTabName() const { return maTabName; }
    inline SCTAB        GetScTab() const { return mnScTab; }

    /** Reads a CRN record (external referenced cell) at the specified address. */
    void                ReadCrn( XclImpStream& rStrm, const XclAddress& rXclPos );

    /** Creates a new linked table in the passed document and fills it with the cached cells.
        @descr  Stores the index of the new sheet, will be accessible with GetScTab(). */
    void                CreateAndFillTable(
                            const XclImpRoot& rRoot, const String& rAbsUrl,
                            const String& rFilterName, const String& rFilterOpt );

private:
    typedef ScfDelList< XclImpCrn > XclImpCrnList;

    XclImpCrnList       maCrnList;      /// List of CRN records (cached cell values).
    String              maTabName;      /// Name of the external sheet.
    SCTAB               mnScTab;        /// New sheet index in Calc document.
};

// External document (SUPBOOK) ================================================

/** This class represents an external linked document (record SUPBOOK).
    @descr  Contains a list of all referenced sheets in the document. */
class XclImpSupbook : protected XclImpRoot
{
public:
    /** Reads the SUPBOOK record from stream. */
    explicit            XclImpSupbook( XclImpStream& rStrm );

    /** Reads an XCT record (count of following CRNs and current sheet). */
    void                ReadXct( XclImpStream& rStrm );
    /** Reads a CRN record (external referenced cell). */
    void                ReadCrn( XclImpStream& rStrm );
    /** Reads an EXTERNNAME record. */
    void                ReadExternname( XclImpStream& rStrm );

    /** Returns the SUPBOOK record type. */
    inline XclSupbookType GetType() const { return meType; }

    /** Returns the URL of the external document. */
    inline const String& GetXclUrl() const { return maXclUrl; }

    /** Returns Calc sheet index from Excel sheet index. */
    SCTAB               GetScTabNum( sal_uInt16 nXclTab ) const;
    /** Returns Calc sheet index from sheet name. */
    SCTAB               GetScTabNum( const String& rTabName ) const;

    /** Returns the external name specified by an index from the Excel document (one-based). */
    const XclImpExtName* GetExternName( sal_uInt16 nXclIndex ) const;
    /** Tries to decode the URL to OLE or DDE link components.
        @descr  For DDE links: Decodes to application name and topic.
        For OLE object links: Decodes to class name and document URL.
        @return  true = decoding was successful, returned strings are valid (not empty). */
    bool                GetLinkData( String& rApplic, String& rDoc ) const;
    /** Returns the specified macro name (1-based) or an empty string on error. */
    const String&       GetMacroName( sal_uInt16 nXclNameIdx ) const;

    /** Creates all sheets of this external document.
        @param nFirstTab  The external Excel index of the first sheet to be created.
        @param nLastTab  The external Excel index of the last sheet to be created. */
    void                CreateTables( sal_uInt16 nSBTabFirst, sal_uInt16 nSBTabLast );

private:
    typedef ScfDelList< XclImpSupbookTab >  XclImpSupbookTabList;
    typedef ScfDelList< XclImpExtName >     XclImpExtNameList;

    XclImpSupbookTabList maSupbTabList;     /// All sheet names of the document.
    XclImpExtNameList   maExtNameList;      /// All external names of the document.
    String              maXclUrl;           /// URL of the external document (Excel mode).
    String              maFilterName;       /// Detected filer name.
    String              maFilterOpt;        /// Detected filer options.
    XclSupbookType      meType;             /// Type of the supbook record.
    sal_uInt16          mnSBTab;            /// Current Excel sheet index from SUPBOOK for XCT/CRN records.
};

// Import link manager ========================================================

/** Contains the SUPBOOK index and sheet indexes of an external link.
    @descr  It is possible to enter a formula like =SUM(Sheet1:Sheet3!A1),
    therefore here occurs a sheet range. */
struct XclImpXti
{
    sal_uInt16          mnSupbook;      /// Index to SUPBOOK record.
    sal_uInt16          mnSBTabFirst;   /// Index to the first sheet of the range in the SUPBOOK.
    sal_uInt16          mnSBTabLast;    /// Index to the last sheet of the range in the SUPBOOK.
};

inline XclImpStream& operator>>( XclImpStream& rStrm, XclImpXti& rXti )
{
    return rStrm >> rXti.mnSupbook >> rXti.mnSBTabFirst >> rXti.mnSBTabLast;
}

// ----------------------------------------------------------------------------

/** Implementation of the link manager. */
class XclImpLinkManagerImpl : protected XclImpRoot
{
public:
    explicit            XclImpLinkManagerImpl( const XclImpRoot& rRoot );

    /** Reads the EXTERNSHEET record. */
    void                ReadExternsheet( XclImpStream& rStrm );
    /** Reads a SUPBOOK record. */
    void                ReadSupbook( XclImpStream& rStrm );
    /** Reads an XCT record and appends it to the current SUPBOOK. */
    void                ReadXct( XclImpStream& rStrm );
    /** Reads a CRN record and appends it to the current SUPBOOK. */
    void                ReadCrn( XclImpStream& rStrm );
    /** Reads an EXTERNNAME record and appends it to the current SUPBOOK. */
    void                ReadExternname( XclImpStream& rStrm );

    /** Returns true, if the specified XTI entry contains an internal reference. */
    bool                IsSelfRef( sal_uInt16 nXtiIndex ) const;
    /** Returns the Calc sheet index range of the specified XTI entry.
        @return  true = XTI data found, returned sheet index range is valid. */
    bool                GetScTabRange(
                            SCTAB& rnFirstScTab, SCTAB& rnLastScTab,
                            sal_uInt16 nXtiIndex ) const;
    /** Returns the specified external name or 0 on error. */
    const XclImpExtName* GetExternName( sal_uInt16 nXtiIndex, sal_uInt16 nExtName ) const;
    /** Tries to decode the URL of the specified XTI entry to OLE or DDE link components.
        @descr  For DDE links: Decodes to application name and topic.
        For OLE object links: Decodes to class name and document URL.
        @return  true = decoding was successful, returned strings are valid (not empty). */
    bool                GetLinkData( String& rApplic, String& rTopic, sal_uInt16 nXtiIndex ) const;
    /** Returns the specified macro name or an empty string on error. */
    const String&       GetMacroName( sal_uInt16 nExtSheet, sal_uInt16 nExtName ) const;

    /** Returns the Calc sheet index of a table in an external document.
        @return  Calc sheet index or EXC_TAB_INVALID on error. */
    SCTAB               GetScTab( const String& rUrl, const String& rTabName ) const;

private:
    /** Returns the specified SUPBOOK (external document). */
    const XclImpSupbook* GetSupbook( sal_uInt32 nXtiIndex ) const;
    /** Returns the SUPBOOK (external workbook) specified by its URL. */
    const XclImpSupbook* GetSupbook( const String& rUrl ) const;

    /** Creates all external sheets in the Calc document. */
    void                CreateTables();

    /** Finds the largest range of sheet indexes in a SUPBOOK after a start sheet index.
        @param rnSBTabFirst  (out-param) The first sheet index of the range in SUPBOOK is returned here.
        @param rnSBTabLast  (out-param) The last sheet index of the range in SUPBOOK is returned here (inclusive).
        @param nSupbook  The list index of the SUPBOOK.
        @param nSBTabStart  The first allowed sheet index. Sheet ranges with an earlier start index are ignored.
        @return  true = the return values are valid; false = nothing found. */
    bool                FindNextTabRange(
                            sal_uInt16& rnSBTabFirst, sal_uInt16& rnSBTabLast,
                            sal_uInt16 nSupbook, sal_uInt16 nSBTabStart ) const;

private:
    typedef ScfDelList< XclImpXti >     XclImpXtiList;
    typedef ScfDelList< XclImpSupbook > XclImpSupbookList;

    XclImpXtiList       maXtiList;          /// List of all XTI structures.
    XclImpSupbookList   maSupbookList;      /// List of external documents.
    bool                mbCreated;          /// true = Calc sheets already created.
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
    return (aIt != maTabNames.end()) ? aIt->second : SCTAB_INVALID;
}

// record creation order - TABID record ---------------------------------------

void XclImpTabInfo::ReadTabid( XclImpStream& rStrm )
{
    DBG_ASSERT_BIFF( rStrm.GetRoot().GetBiff() == EXC_BIFF8 );
    if( rStrm.GetRoot().GetBiff() == EXC_BIFF8 )
    {
        sal_Size nReadCount = rStrm.GetRecLeft() / 2;
        DBG_ASSERT( nReadCount <= 0xFFFF, "XclImpTabInfo::ReadTabid - record too long" );
        maTabIdVec.clear();
        maTabIdVec.reserve( nReadCount );
        for( sal_Size nIndex = 0; rStrm.IsValid() && (nIndex < nReadCount); ++nIndex )
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
        mxDdeMatrix.reset( new XclImpCachedMatrix( rStrm ) );
}

XclImpExtName::~XclImpExtName()
{
}

void XclImpExtName::CreateDdeData( ScDocument& rDoc, const String& rApplic, const String& rTopic ) const
{
    ScMatrixRef xResults;
    if( mxDdeMatrix.get() )
        xResults = mxDdeMatrix->CreateScMatrix();
    rDoc.CreateDdeLink( rApplic, rTopic, maName, SC_DDE_DEFAULT, xResults );
}

// Cached external cells ======================================================

XclImpCrn::XclImpCrn( XclImpStream& rStrm, const XclAddress& rXclPos ) :
    XclImpCachedValue( rStrm ),
    maXclPos( rXclPos )
{
}

void XclImpCrn::SetCell( const XclImpRoot& rRoot, SCTAB nScTab ) const
{
    ScAddress aScPos( ScAddress::UNINITIALIZED );
    if( rRoot.GetAddressConverter().ConvertAddress( aScPos, maXclPos, nScTab, false ) )
    {
        switch( GetType() )
        {
            case EXC_CACHEDVAL_DOUBLE:
                rRoot.GetDoc().SetValue( aScPos.Col(), aScPos.Row(), aScPos.Tab(), GetValue() );
            break;
            case EXC_CACHEDVAL_STRING:
                rRoot.GetDoc().PutCell( aScPos, new ScStringCell( GetString() ) );
            break;
            case EXC_CACHEDVAL_BOOL:
            case EXC_CACHEDVAL_ERROR:
            {
                ScFormulaCell* pFmlaCell = new ScFormulaCell( rRoot.GetDocPtr(), aScPos, GetBoolErrFmla() );
                pFmlaCell->SetHybridDouble( GetBool() ? 1.0 : 0.0 );  // GetBool() returns false for error codes
                rRoot.GetDoc().PutCell( aScPos, pFmlaCell );
            }
            break;
        }
    }
}

// Sheet in an external document ==============================================

XclImpSupbookTab::XclImpSupbookTab( const String& rTabName ) :
    maTabName( rTabName ),
    mnScTab( SCTAB_INVALID )
{
}

XclImpSupbookTab::~XclImpSupbookTab()
{
}

void XclImpSupbookTab::ReadCrn( XclImpStream& rStrm, const XclAddress& rXclPos )
{
    maCrnList.Append( new XclImpCrn( rStrm, rXclPos ) );
}

void XclImpSupbookTab::CreateAndFillTable( const XclImpRoot& rRoot,
        const String& rAbsUrl, const String& rFilterName, const String& rFilterOpt )
{
    if( mnScTab == SCTAB_INVALID )
        if( rRoot.GetDoc().InsertLinkedEmptyTab( mnScTab, rAbsUrl, rFilterName, rFilterOpt, maTabName ) )
            for( const XclImpCrn* pCrn = maCrnList.First(); pCrn; pCrn = maCrnList.Next() )
                pCrn->SetCell( rRoot, mnScTab );
}

// External document (SUPBOOK) ================================================

XclImpSupbook::XclImpSupbook( XclImpStream& rStrm ) :
    XclImpRoot( rStrm.GetRoot() ),
    meType( EXC_SBTYPE_UNKNOWN ),
    mnSBTab( EXC_TAB_DELETED )
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
    XclImpUrlHelper::DecodeUrl( maXclUrl, bSelf, GetRoot(), aEncUrl );

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
            pSBTab->ReadCrn( rStrm, XclAddress( nXclCol, nXclRow ) );
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
    return pSBTab ? pSBTab->GetScTab() : SCTAB_INVALID;
}

SCTAB XclImpSupbook::GetScTabNum( const String& rTabName ) const
{
    for( const XclImpSupbookTab* pSBTab = maSupbTabList.First(); pSBTab; pSBTab = maSupbTabList.Next() )
        if( pSBTab->GetTabName() == rTabName )
            return pSBTab->GetScTab();
    return SCTAB_INVALID;
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

const String& XclImpSupbook::GetMacroName( sal_uInt16 nXclNameIdx ) const
{
    DBG_ASSERT( nXclNameIdx > 0, "XclImpSupbook::GetMacroName - index must be >0" );
    const XclImpName* pName = (meType == EXC_SBTYPE_SELF) ? GetNameManager().GetName( nXclNameIdx ) : 0;
    return (pName && pName->IsVBName()) ? pName->GetScName() : EMPTY_STRING;
}

void XclImpSupbook::CreateTables( sal_uInt16 nSBTabFirst, sal_uInt16 nSBTabLast )
{
    if( (meType == EXC_SBTYPE_EXTERN) && (GetExtDocOptions().GetDocSettings().mnLinkCnt == 0) && GetDocShell() )
    {
        String aAbsUrl( ScGlobal::GetAbsDocName( maXclUrl, GetDocShell() ) );

        // get filter name for external document
        if( !maFilterName.Len() )
            ScDocumentLoader::GetFilterName( aAbsUrl, maFilterName, maFilterOpt, FALSE, FALSE );

        // create tables
        for( sal_uInt16 nSBTab = nSBTabFirst; nSBTab <= nSBTabLast; ++nSBTab )
            if( XclImpSupbookTab* pSBTab = maSupbTabList.GetObject( nSBTab ) )
                pSBTab->CreateAndFillTable( GetRoot(), aAbsUrl, maFilterName, maFilterOpt );
    }
}

// Import link manager ========================================================

XclImpLinkManagerImpl::XclImpLinkManagerImpl( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    mbCreated( false )
{
}

void XclImpLinkManagerImpl::ReadExternsheet( XclImpStream& rStrm )
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

void XclImpLinkManagerImpl::ReadSupbook( XclImpStream& rStrm )
{
    maSupbookList.Append( new XclImpSupbook( rStrm ) );
}

void XclImpLinkManagerImpl::ReadXct( XclImpStream& rStrm )
{
    if( XclImpSupbook* pSupbook = maSupbookList.Last() )
        pSupbook->ReadXct( rStrm );
}

void XclImpLinkManagerImpl::ReadCrn( XclImpStream& rStrm )
{
    if( XclImpSupbook* pSupbook = maSupbookList.Last() )
        pSupbook->ReadCrn( rStrm );
}

void XclImpLinkManagerImpl::ReadExternname( XclImpStream& rStrm )
{
    if( XclImpSupbook* pSupbook = maSupbookList.Last() )
        pSupbook->ReadExternname( rStrm );
}

bool XclImpLinkManagerImpl::IsSelfRef( sal_uInt16 nXtiIndex ) const
{
    const XclImpSupbook* pSupbook = GetSupbook( nXtiIndex );
    return pSupbook && (pSupbook->GetType() == EXC_SBTYPE_SELF);
}

bool XclImpLinkManagerImpl::GetScTabRange(
        SCTAB& rnFirstScTab, SCTAB& rnLastScTab, sal_uInt16 nXtiIndex ) const
{
    if( const XclImpXti* pXti = maXtiList.GetObject( nXtiIndex ) )
    {
        if( const XclImpSupbook* pSupbook = maSupbookList.GetObject( pXti->mnSupbook ) )
        {
            rnFirstScTab = pSupbook->GetScTabNum( pXti->mnSBTabFirst );
            rnLastScTab = pSupbook->GetScTabNum( pXti->mnSBTabLast );
            return true;
        }
    }
    return false;
}

const XclImpExtName* XclImpLinkManagerImpl::GetExternName( sal_uInt16 nXtiIndex, sal_uInt16 nExtName ) const
{
    const XclImpSupbook* pSupbook = GetSupbook( nXtiIndex );
    return pSupbook ? pSupbook->GetExternName( nExtName ) : 0;
}

bool XclImpLinkManagerImpl::GetLinkData( String& rApplic, String& rTopic, sal_uInt16 nXtiIndex ) const
{
    const XclImpSupbook* pSupbook = GetSupbook( nXtiIndex );
    return pSupbook && pSupbook->GetLinkData( rApplic, rTopic );
}

const String& XclImpLinkManagerImpl::GetMacroName( sal_uInt16 nExtSheet, sal_uInt16 nExtName ) const
{
    const XclImpSupbook* pSupbook = GetSupbook( nExtSheet );
    return pSupbook ? pSupbook->GetMacroName( nExtName ) : EMPTY_STRING;
}

SCTAB XclImpLinkManagerImpl::GetScTab( const String& rUrl, const String& rTabName ) const
{
    const XclImpSupbook* pSupbook = GetSupbook( rUrl );
    return pSupbook ? pSupbook->GetScTabNum( rTabName ) : SCTAB_INVALID;
}

const XclImpSupbook* XclImpLinkManagerImpl::GetSupbook( sal_uInt32 nXtiIndex ) const
{
    const XclImpXti* pXti = maXtiList.GetObject( nXtiIndex );
    return pXti ? maSupbookList.GetObject( pXti->mnSupbook ) : 0;
}

const XclImpSupbook* XclImpLinkManagerImpl::GetSupbook( const String& rUrl ) const
{
    for( const XclImpSupbook* pSupbook = maSupbookList.First(); pSupbook; pSupbook = maSupbookList.Next() )
        if( pSupbook->GetXclUrl() == rUrl )
            return pSupbook;
    return 0;
}

void XclImpLinkManagerImpl::CreateTables()
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
            pSupbook->CreateTables( nSBTabFirst, nSBTabLast );
            // #96263# don't search again if last sheet == EXC_NOTAB
            bLoop = (nSBTabLast != EXC_NOTAB) && FindNextTabRange( nSBTabFirst, nSBTabLast, nSupbook, nSBTabLast + 1 );
        }
    }
    mbCreated = true;
}

bool XclImpLinkManagerImpl::FindNextTabRange(
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
    XclImpRoot( rRoot ),
    mxImpl( new XclImpLinkManagerImpl( rRoot ) )
{
}

XclImpLinkManager::~XclImpLinkManager()
{
}

void XclImpLinkManager::ReadExternsheet( XclImpStream& rStrm )
{
    mxImpl->ReadExternsheet( rStrm );
}

void XclImpLinkManager::ReadSupbook( XclImpStream& rStrm )
{
    mxImpl->ReadSupbook( rStrm );
}

void XclImpLinkManager::ReadXct( XclImpStream& rStrm )
{
    mxImpl->ReadXct( rStrm );
}

void XclImpLinkManager::ReadCrn( XclImpStream& rStrm )
{
    mxImpl->ReadCrn( rStrm );
}

void XclImpLinkManager::ReadExternname( XclImpStream& rStrm )
{
    mxImpl->ReadExternname( rStrm );
}

bool XclImpLinkManager::IsSelfRef( sal_uInt16 nXtiIndex ) const
{
    return mxImpl->IsSelfRef( nXtiIndex );
}

bool XclImpLinkManager::GetScTabRange(
        SCTAB& rnFirstScTab, SCTAB& rnLastScTab, sal_uInt16 nXtiIndex ) const
{
    return mxImpl->GetScTabRange( rnFirstScTab, rnLastScTab, nXtiIndex );
}

const XclImpExtName* XclImpLinkManager::GetExternName( sal_uInt16 nXtiIndex, sal_uInt16 nExtName ) const
{
    return mxImpl->GetExternName( nXtiIndex, nExtName );
}

bool XclImpLinkManager::GetLinkData( String& rApplic, String& rTopic, sal_uInt16 nXtiIndex ) const
{
    return mxImpl->GetLinkData( rApplic, rTopic, nXtiIndex );
}

const String& XclImpLinkManager::GetMacroName( sal_uInt16 nExtSheet, sal_uInt16 nExtName ) const
{
    return mxImpl->GetMacroName( nExtSheet, nExtName );
}

SCTAB XclImpLinkManager::GetScTab( const String& rUrl, const String& rTabName ) const
{
    return mxImpl->GetScTab( rUrl, rTabName );
}

// ============================================================================


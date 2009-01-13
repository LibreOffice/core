/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xilink.cxx,v $
 * $Revision: 1.25.46.7 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
#include "xilink.hxx"
#include "document.hxx"
#include "cell.hxx"
#include "scextopt.hxx"
#include "tablink.hxx"
#include "xistream.hxx"
#include "xihelper.hxx"
#include "xiname.hxx"
#include "excform.hxx"
#include "tokenarray.hxx"
#include "externalrefmgr.hxx"

#include <vector>

using ::std::vector;

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

    const XclAddress&   GetAddress() const;

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

    /** Reads a CRN record (external referenced cell) at the specified address. */
    void                ReadCrn( XclImpStream& rStrm, const XclAddress& rXclPos );

    void                LoadCachedValues(ScExternalRefCache::TableTypeRef pCacheTable);

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
    void                ReadExternname( XclImpStream& rStrm, ExcelToSc* pFormulaConv = NULL );

    /** Returns the SUPBOOK record type. */
    inline XclSupbookType GetType() const { return meType; }

    /** Returns the URL of the external document. */
    inline const String& GetXclUrl() const { return maXclUrl; }

    /** Returns the external name specified by an index from the Excel document (one-based). */
    const XclImpExtName* GetExternName( sal_uInt16 nXclIndex ) const;
    /** Tries to decode the URL to OLE or DDE link components.
        @descr  For DDE links: Decodes to application name and topic.
        For OLE object links: Decodes to class name and document URL.
        @return  true = decoding was successful, returned strings are valid (not empty). */
    bool                GetLinkData( String& rApplic, String& rDoc ) const;
    /** Returns the specified macro name (1-based) or an empty string on error. */
    const String&       GetMacroName( sal_uInt16 nXclNameIdx ) const;

    const String&       GetTabName( sal_uInt16 nXtiTab ) const;

    sal_uInt16          GetTabCount() const;

    void                LoadCachedValues();

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
    void                ReadExternname( XclImpStream& rStrm, ExcelToSc* pFormulaConv = NULL );

    /** Returns true, if the specified XTI entry contains an internal reference. */
    bool                IsSelfRef( sal_uInt16 nXtiIndex ) const;
    /** Returns the Calc sheet index range of the specified XTI entry.
        @return  true = XTI data found, returned sheet index range is valid. */
    bool                GetScTabRange(
                            SCTAB& rnFirstScTab, SCTAB& rnLastScTab,
                            sal_uInt16 nXtiIndex ) const;
    /** Returns the specified external name or 0 on error. */
    const XclImpExtName* GetExternName( sal_uInt16 nXtiIndex, sal_uInt16 nExtName ) const;

    /** Returns the absolute file URL of a supporting workbook specified by
        the index. */
    const String*       GetSupbookUrl( sal_uInt16 nXtiIndex ) const;

    const String&       GetSupbookTabName( sal_uInt16 nXti, sal_uInt16 nXtiTab ) const;

    /** Tries to decode the URL of the specified XTI entry to OLE or DDE link components.
        @descr  For DDE links: Decodes to application name and topic.
        For OLE object links: Decodes to class name and document URL.
        @return  true = decoding was successful, returned strings are valid (not empty). */
    bool                GetLinkData( String& rApplic, String& rTopic, sal_uInt16 nXtiIndex ) const;
    /** Returns the specified macro name or an empty string on error. */
    const String&       GetMacroName( sal_uInt16 nExtSheet, sal_uInt16 nExtName ) const;

private:
    /** Returns the specified SUPBOOK (external document). */
    const XclImpSupbook* GetSupbook( sal_uInt32 nXtiIndex ) const;
    /** Returns the SUPBOOK (external workbook) specified by its URL. */
    const XclImpSupbook* GetSupbook( const String& rUrl ) const;

    void                LoadCachedValues();

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

XclImpExtName::XclImpExtName( const XclImpSupbook& rSupbook, XclImpStream& rStrm, XclSupbookType eSubType, ExcelToSc* pFormulaConv )
{
    sal_uInt16 nFlags;
    sal_uInt8 nLen;

    rStrm >> nFlags >> mnStorageId >> nLen ;
    maName = rStrm.ReadUniString( nLen );
    if( ::get_flag( nFlags, EXC_EXTN_BUILTIN ) || !::get_flag( nFlags, EXC_EXTN_OLE_OR_DDE ) )
    {
        if( eSubType == EXC_SBTYPE_ADDIN )
        {
            meType = xlExtAddIn;
            maName = rStrm.GetRoot().GetScAddInName( maName );
        }
        else if ( (eSubType == EXC_SBTYPE_EUROTOOL) &&
                maName.EqualsIgnoreCaseAscii( "EUROCONVERT" ) )
            meType = xlExtEuroConvert;
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

    if (meType == xlExtName)
    {
        // TODO: For now, only global external names are supported.  In future
        // we should extend this to supporting per-sheet external names.
        if (mnStorageId == 0)
        {
            if (pFormulaConv)
            {
                const ScTokenArray* pArray = NULL;
                sal_uInt16 nFmlaLen;
                rStrm >> nFmlaLen;
                vector<String> aTabNames;
                sal_uInt16 nCount = rSupbook.GetTabCount();
                aTabNames.reserve(nCount);
                for (sal_uInt16 i = 0; i < nCount; ++i)
                    aTabNames.push_back(rSupbook.GetTabName(i));

                pFormulaConv->ConvertExternName(pArray, rStrm, nFmlaLen, rSupbook.GetXclUrl(), aTabNames);
                if (pArray)
                    mxArray.reset(pArray->Clone());
            }
        }
    }
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

void XclImpExtName::CreateExtNameData( ScDocument& rDoc, sal_uInt16 nFileId ) const
{
    if (!mxArray.get())
        return;

    ScExternalRefManager* pRefMgr = rDoc.GetExternalRefManager();
    pRefMgr->storeRangeNameTokens(nFileId, maName, *mxArray);
}

bool XclImpExtName::HasFormulaTokens() const
{
    return (mxArray.get() != NULL);
}

// Cached external cells ======================================================

XclImpCrn::XclImpCrn( XclImpStream& rStrm, const XclAddress& rXclPos ) :
    XclImpCachedValue( rStrm ),
    maXclPos( rXclPos )
{
}

const XclAddress& XclImpCrn::GetAddress() const
{
    return maXclPos;
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

void XclImpSupbookTab::LoadCachedValues(ScExternalRefCache::TableTypeRef pCacheTable)
{
    if (maCrnList.Empty())
        return;

    for (XclImpCrn* p = maCrnList.First(); p; p = maCrnList.Next())
    {
        const XclAddress& rAddr = p->GetAddress();
        switch (p->GetType())
        {
            case EXC_CACHEDVAL_BOOL:
            break;
            case EXC_CACHEDVAL_DOUBLE:
            {
                double f = p->GetValue();
                ScExternalRefCache::TokenRef pToken(new formula::FormulaDoubleToken(f));
                pCacheTable->setCell(rAddr.mnCol, rAddr.mnRow, pToken);
            }
            break;
            case EXC_CACHEDVAL_EMPTY:
            break;
            case EXC_CACHEDVAL_ERROR:
            break;
            case EXC_CACHEDVAL_STRING:
            {
                const String& rStr = p->GetString();
                ScExternalRefCache::TokenRef pToken(new formula::FormulaStringToken(rStr));
                pCacheTable->setCell(rAddr.mnCol, rAddr.mnRow, pToken);
            }
            break;
            default:
                ;
        }
    }
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

    if( maXclUrl.EqualsIgnoreCaseAscii( "\010EUROTOOL.XLA" ) )
    {
        meType = EXC_SBTYPE_EUROTOOL;
        maSupbTabList.Append( new XclImpSupbookTab( maXclUrl ) );
    }
    else if( nSBTabCnt )
    {
        meType = EXC_SBTYPE_EXTERN;
        for( sal_uInt16 nSBTab = 0; nSBTab < nSBTabCnt; ++nSBTab )
        {
            String aTabName( rStrm.ReadUniString() );
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

void XclImpSupbook::ReadExternname( XclImpStream& rStrm, ExcelToSc* pFormulaConv )
{
    maExtNameList.Append( new XclImpExtName( *this, rStrm, meType, pFormulaConv ) );
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

const String& XclImpSupbook::GetTabName( sal_uInt16 nXtiTab ) const
{
    if (maSupbTabList.Empty())
        return EMPTY_STRING;

    sal_uInt16 i = 0;
    for (XclImpSupbookTab* p = maSupbTabList.First(); p; p = maSupbTabList.Next(), ++i)
    {
        if (i == nXtiTab)
            return p->GetTabName();
    }

    return EMPTY_STRING;
}

sal_uInt16 XclImpSupbook::GetTabCount() const
{
    return ulimit_cast<sal_uInt16>(maSupbTabList.Count());
}

void XclImpSupbook::LoadCachedValues()
{
    if (meType != EXC_SBTYPE_EXTERN || GetExtDocOptions().GetDocSettings().mnLinkCnt > 0)
        return;

    String aAbsUrl( ScGlobal::GetAbsDocName(maXclUrl, GetDocShell()) );

    ScExternalRefManager* pRefMgr = GetRoot().GetDoc().GetExternalRefManager();
    sal_uInt16 nFileId = pRefMgr->getExternalFileId(aAbsUrl);

    sal_uInt16 nCount = static_cast< sal_uInt16 >( maSupbTabList.Count() );
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        XclImpSupbookTab* pTab = maSupbTabList.GetObject(i);
        if (!pTab)
            return;

        const String& rTabName = pTab->GetTabName();
        ScExternalRefCache::TableTypeRef pCacheTable = pRefMgr->getCacheTable(nFileId, rTabName, true);
        pTab->LoadCachedValues(pCacheTable);
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

    LoadCachedValues();
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

void XclImpLinkManagerImpl::ReadExternname( XclImpStream& rStrm, ExcelToSc* pFormulaConv )
{
    if( XclImpSupbook* pSupbook = maSupbookList.Last() )
        pSupbook->ReadExternname( rStrm, pFormulaConv );
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
        if (maSupbookList.GetObject(pXti->mnSupbook))
        {
            rnFirstScTab = pXti->mnSBTabFirst;
            rnLastScTab  = pXti->mnSBTabLast;
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

const String* XclImpLinkManagerImpl::GetSupbookUrl( sal_uInt16 nXtiIndex ) const
{
    const XclImpSupbook* p = GetSupbook( nXtiIndex );
    if (!p)
        return NULL;
    return &p->GetXclUrl();
}

const String& XclImpLinkManagerImpl::GetSupbookTabName( sal_uInt16 nXti, sal_uInt16 nXtiTab ) const
{
    const XclImpSupbook* p = GetSupbook(nXti);
    return p ? p->GetTabName(nXtiTab) : EMPTY_STRING;
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

void XclImpLinkManagerImpl::LoadCachedValues()
{
    // Read all CRN records which can be accessed via XclImpSupbook, and store
    // the cached values to the external reference manager.

    sal_uInt32 nCount = maSupbookList.Count();
    for (sal_uInt16 nSupbook = 0; nSupbook < nCount; ++nSupbook)
    {
        XclImpSupbook* pSupbook = maSupbookList.GetObject(nSupbook);
        pSupbook->LoadCachedValues();
    }
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

void XclImpLinkManager::ReadExternname( XclImpStream& rStrm, ExcelToSc* pFormulaConv )
{
    mxImpl->ReadExternname( rStrm, pFormulaConv );
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

const String* XclImpLinkManager::GetSupbookUrl( sal_uInt16 nXtiIndex ) const
{
    return mxImpl->GetSupbookUrl(nXtiIndex);
}

const String& XclImpLinkManager::GetSupbookTabName( sal_uInt16 nXti,  sal_uInt16 nXtiTab ) const
{
    return mxImpl->GetSupbookTabName(nXti, nXtiTab);
}

bool XclImpLinkManager::GetLinkData( String& rApplic, String& rTopic, sal_uInt16 nXtiIndex ) const
{
    return mxImpl->GetLinkData( rApplic, rTopic, nXtiIndex );
}

const String& XclImpLinkManager::GetMacroName( sal_uInt16 nExtSheet, sal_uInt16 nExtName ) const
{
    return mxImpl->GetMacroName( nExtSheet, nExtName );
}

// ============================================================================


/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <xilink.hxx>
#include <document.hxx>
#include <scextopt.hxx>
#include <xistream.hxx>
#include <xihelper.hxx>
#include <xiname.hxx>
#include <xltools.hxx>
#include <excform.hxx>
#include <tokenarray.hxx>
#include <externalrefmgr.hxx>
#include <scmatrix.hxx>
#include <svl/sharedstringpool.hxx>
#include <sal/log.hxx>

#include <vector>
#include <memory>

// *** Helper classes ***

// Cached external cells ======================================================

/**
 * Contains the address and value of an external referenced cell.
 * Note that this is non-copyable, so cannot be used in most stl/boost containers.
 */
class XclImpCrn : public XclImpCachedValue
{
public:
    /** Reads a cached value and stores it with its cell address. */
    explicit            XclImpCrn( XclImpStream& rStrm, const XclAddress& rXclPos );

    const XclAddress&   GetAddress() const { return maXclPos;}

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
    explicit            XclImpSupbookTab( const OUString& rTabName );

    const OUString& GetTabName() const { return maTabName; }

    /** Reads a CRN record (external referenced cell) at the specified address. */
    void                ReadCrn( XclImpStream& rStrm, const XclAddress& rXclPos );

    void                LoadCachedValues( const ScExternalRefCache::TableTypeRef& pCacheTable,
                                          svl::SharedStringPool& rPool );

private:
    typedef std::shared_ptr< XclImpCrn > XclImpCrnRef;
    typedef std::vector< XclImpCrnRef > XclImpCrnList;

    XclImpCrnList       maCrnList;      /// List of CRN records (cached cell values).
    OUString            maTabName;      /// Name of the external sheet.
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
    void                ReadExternname( XclImpStream& rStrm, ExcelToSc* pFormulaConv );

    /** Returns the SUPBOOK record type. */
    XclSupbookType GetType() const { return meType; }

    /** Returns the URL of the external document. */
    const OUString& GetXclUrl() const { return maXclUrl; }

    /** Returns the external name specified by an index from the Excel document (one-based). */
    const XclImpExtName* GetExternName( sal_uInt16 nXclIndex ) const;
    /** Tries to decode the URL to OLE or DDE link components.
        @descr  For DDE links: Decodes to application name and topic.
        For OLE object links: Decodes to class name and document URL.
        @return  true = decoding was successful, returned strings are valid (not empty). */
    bool                GetLinkData( OUString& rApplic, OUString& rDoc ) const;
    /** Returns the specified macro name (1-based) or an empty string on error. */
    const OUString&     GetMacroName( sal_uInt16 nXclNameIdx ) const;

    const OUString&     GetTabName( sal_uInt16 nXtiTab ) const;

    sal_uInt16          GetTabCount() const;

    void                LoadCachedValues();

    svl::SharedStringPool& GetSharedStringPool();

private:
    typedef std::vector< std::unique_ptr<XclImpSupbookTab> >  XclImpSupbookTabList;
    typedef std::vector< std::unique_ptr<XclImpExtName> >     XclImpExtNameList;

    XclImpSupbookTabList maSupbTabList;     /// All sheet names of the document.
    XclImpExtNameList   maExtNameList;      /// All external names of the document.
    OUString            maXclUrl;           /// URL of the external document (Excel mode).
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
    explicit     XclImpXti() : mnSupbook( SAL_MAX_UINT16 ), mnSBTabFirst( SAL_MAX_UINT16 ), mnSBTabLast( SAL_MAX_UINT16 ) {}
};

static XclImpStream& operator>>( XclImpStream& rStrm, XclImpXti& rXti )
{
    rXti.mnSupbook = rStrm.ReaduInt16();
    rXti.mnSBTabFirst = rStrm.ReaduInt16();
    rXti.mnSBTabLast = rStrm.ReaduInt16();
    return rStrm;
}

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
    void                ReadExternname( XclImpStream& rStrm, ExcelToSc* pFormulaConv );

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
    const OUString*       GetSupbookUrl( sal_uInt16 nXtiIndex ) const;

    const OUString&       GetSupbookTabName( sal_uInt16 nXti, sal_uInt16 nXtiTab ) const;

    /** Tries to decode the URL of the specified XTI entry to OLE or DDE link components.
        @descr  For DDE links: Decodes to application name and topic.
        For OLE object links: Decodes to class name and document URL.
        @return  true = decoding was successful, returned strings are valid (not empty). */
    bool                GetLinkData( OUString& rApplic, OUString& rTopic, sal_uInt16 nXtiIndex ) const;
    /** Returns the specified macro name or an empty string on error. */
    const OUString&     GetMacroName( sal_uInt16 nExtSheet, sal_uInt16 nExtName ) const;

private:
    /** Returns the specified XTI (link entry from BIFF8 EXTERNSHEET record). */
    const XclImpXti*    GetXti( sal_uInt16 nXtiIndex ) const;
    /** Returns the specified SUPBOOK (external document). */
    const XclImpSupbook* GetSupbook( sal_uInt16 nXtiIndex ) const;

    void                LoadCachedValues();

private:
    typedef std::vector< XclImpXti >  XclImpXtiVector;
    typedef std::vector< std::unique_ptr<XclImpSupbook> > XclImpSupbookList;

    XclImpXtiVector     maXtiList;          /// List of all XTI structures.
    XclImpSupbookList   maSupbookList;      /// List of external documents.
};

// *** Implementation ***

// Excel sheet indexes ========================================================

// original Excel sheet names -------------------------------------------------

void XclImpTabInfo::AppendXclTabName( const OUString& rXclTabName, SCTAB nScTab )
{
    maTabNames[ rXclTabName ] = nScTab;
}

void XclImpTabInfo::InsertScTab( SCTAB nScTab )
{
    for( auto& rEntry : maTabNames )
        if( rEntry.second >= nScTab )
            ++rEntry.second;
}

SCTAB XclImpTabInfo::GetScTabFromXclName( const OUString& rXclTabName ) const
{
    XclTabNameMap::const_iterator aIt = maTabNames.find( rXclTabName );
    return (aIt != maTabNames.end()) ? aIt->second : SCTAB_INVALID;
}

// record creation order - TABID record ---------------------------------------

void XclImpTabInfo::ReadTabid( XclImpStream& rStrm )
{
    OSL_ENSURE_BIFF( rStrm.GetRoot().GetBiff() == EXC_BIFF8 );
    if( rStrm.GetRoot().GetBiff() == EXC_BIFF8 )
    {
        rStrm.EnableDecryption();
        std::size_t nReadCount = rStrm.GetRecLeft() / 2;
        OSL_ENSURE( nReadCount <= 0xFFFF, "XclImpTabInfo::ReadTabid - record too long" );
        maTabIdVec.clear();
        maTabIdVec.reserve( nReadCount );
        for( std::size_t nIndex = 0; rStrm.IsValid() && (nIndex < nReadCount); ++nIndex )
            // zero index is not allowed in BIFF8, but it seems that it occurs in real life
            maTabIdVec.push_back( rStrm.ReaduInt16() );
    }
}

sal_uInt16 XclImpTabInfo::GetCurrentIndex( sal_uInt16 nCreatedId, sal_uInt16 nMaxTabId ) const
{
    sal_uInt16 nReturn = 0;
    for( sal_uInt16 nValue : maTabIdVec )
    {
        if( nValue == nCreatedId )
            return nReturn;
        if( nValue <= nMaxTabId )
            ++nReturn;
    }
    return 0;
}

// External names =============================================================

XclImpExtName::MOper::MOper(svl::SharedStringPool& rPool, XclImpStream& rStrm) :
    mxCached(new ScMatrix(0,0))
{
    SCSIZE nLastCol = rStrm.ReaduInt8();
    SCSIZE nLastRow = rStrm.ReaduInt16();

    //assuming worst case scenario of nOp + one byte unistring len
    const size_t nMinRecordSize = 2;
    const size_t nMaxRows = rStrm.GetRecLeft() / (nMinRecordSize * (nLastCol+1));
    if (nLastRow >= nMaxRows)
    {
        SAL_WARN("sc", "Parsing error: " << nMaxRows <<
                 " max possible rows, but " << nLastRow << " index claimed, truncating");
        if (nMaxRows > 0)
            nLastRow = nMaxRows-1;
        else
            return;
    }

    mxCached->Resize(nLastCol+1, nLastRow+1);
    for (SCSIZE nRow = 0; nRow <= nLastRow; ++nRow)
    {
        for (SCSIZE nCol = 0; nCol <= nLastCol; ++nCol)
        {
            sal_uInt8 nOp;
            nOp = rStrm.ReaduInt8();
            switch (nOp)
            {
                case 0x01:
                {
                    double fVal = rStrm.ReadDouble();
                    mxCached->PutDouble(fVal, nCol, nRow);
                }
                break;
                case 0x02:
                {
                    OUString aStr = rStrm.ReadUniString();
                    mxCached->PutString(rPool.intern(aStr), nCol, nRow);
                }
                break;
                case 0x04:
                {
                    bool bVal = rStrm.ReaduInt8();
                    mxCached->PutBoolean(bVal, nCol, nRow);
                    rStrm.Ignore(7);
                }
                break;
                case 0x10:
                {
                    sal_uInt8 nErr = rStrm.ReaduInt8();
                    // Map the error code from xls to calc.
                    mxCached->PutError(XclTools::GetScErrorCode(nErr), nCol, nRow);
                    rStrm.Ignore(7);
                }
                break;
                default:
                    rStrm.Ignore(8);
            }
        }
    }
}

const ScMatrix& XclImpExtName::MOper::GetCache() const
{
    return *mxCached;
}

XclImpExtName::XclImpExtName( XclImpSupbook& rSupbook, XclImpStream& rStrm, XclSupbookType eSubType, ExcelToSc* pFormulaConv )
    : mnStorageId(0)
{
    sal_uInt16 nFlags(0);
    sal_uInt8 nLen(0);

    nFlags = rStrm.ReaduInt16();
    mnStorageId = rStrm.ReaduInt32();
    nLen = rStrm.ReaduInt8();
    maName = rStrm.ReadUniString( nLen );
    if( ::get_flag( nFlags, EXC_EXTN_BUILTIN ) || !::get_flag( nFlags, EXC_EXTN_OLE_OR_DDE ) )
    {
        if( eSubType == XclSupbookType::Addin )
        {
            meType = xlExtAddIn;
            maName = XclImpRoot::GetScAddInName( maName );
        }
        else if ( (eSubType == XclSupbookType::Eurotool) &&
                maName.equalsIgnoreAsciiCase( "EUROCONVERT" ) )
            meType = xlExtEuroConvert;
        else
        {
            meType = xlExtName;
            maName = ScfTools::ConvertToScDefinedName( maName );
        }
    }
    else
    {
        meType = ::get_flagvalue( nFlags, EXC_EXTN_OLE, xlExtOLE, xlExtDDE );
    }

    switch (meType)
    {
        case xlExtDDE:
            if (rStrm.GetRecLeft() > 1)
                mxDdeMatrix.reset(new XclImpCachedMatrix(rStrm));
        break;
        case xlExtName:
            // TODO: For now, only global external names are supported.  In future
            // we should extend this to supporting per-sheet external names.
            if (mnStorageId == 0 && pFormulaConv)
            {
                std::unique_ptr<ScTokenArray> pArray;
                sal_uInt16 nFmlaLen;
                nFmlaLen = rStrm.ReaduInt16();
                std::vector<OUString> aTabNames;
                sal_uInt16 nCount = rSupbook.GetTabCount();
                aTabNames.reserve(nCount);
                for (sal_uInt16 i = 0; i < nCount; ++i)
                    aTabNames.push_back(rSupbook.GetTabName(i));

                pFormulaConv->ConvertExternName(pArray, rStrm, nFmlaLen, rSupbook.GetXclUrl(), aTabNames);
                if (pArray)
                    mxArray = std::move( pArray );
            }
        break;
        case xlExtOLE:
            mpMOper.reset( new MOper(rSupbook.GetSharedStringPool(), rStrm) );
        break;
        default:
            ;
    }
}

XclImpExtName::~XclImpExtName()
{
}

void XclImpExtName::CreateDdeData( ScDocument& rDoc, const OUString& rApplic, const OUString& rTopic ) const
{
    ScMatrixRef xResults;
    if( mxDdeMatrix.get() )
        xResults = mxDdeMatrix->CreateScMatrix(rDoc.GetSharedStringPool());
    rDoc.CreateDdeLink( rApplic, rTopic, maName, SC_DDE_DEFAULT, xResults );
}

void XclImpExtName::CreateExtNameData( const ScDocument& rDoc, sal_uInt16 nFileId ) const
{
    if (!mxArray.get())
        return;

    ScExternalRefManager* pRefMgr = rDoc.GetExternalRefManager();
    pRefMgr->storeRangeNameTokens(nFileId, maName, *mxArray);
}

namespace {

/**
 * Decompose the name into sheet name and range name.  An OLE link name is
 * always formatted like this [ !Sheet1!R1C1:R5C2 ] and it always uses R1C1
 * notation.
 */
bool extractSheetAndRange(const OUString& rName, OUString& rSheet, OUString& rRange)
{
    sal_Int32 n = rName.getLength();
    const sal_Unicode* p = rName.getStr();
    OUStringBuffer aBuf;
    bool bInSheet = true;
    for (sal_Int32 i = 0; i < n; ++i, ++p)
    {
        if (i == 0)
        {
            // first character must be '!'.
            if (*p != '!')
                return false;
            continue;
        }

        if (*p == '!')
        {
            // sheet name to range separator.
            if (!bInSheet)
                return false;
            rSheet = aBuf.makeStringAndClear();
            bInSheet = false;
            continue;
        }

        aBuf.append(*p);
    }

    rRange = aBuf.makeStringAndClear();
    return true;
}

}

bool XclImpExtName::CreateOleData(ScDocument& rDoc, const OUString& rUrl,
                                  sal_uInt16& rFileId, OUString& rTabName, ScRange& rRange) const
{
    if (!mpMOper)
        return false;

    OUString aSheet, aRangeStr;
    if (!extractSheetAndRange(maName, aSheet, aRangeStr))
        return false;

    ScRange aRange;
    ScRefFlags nRes = aRange.ParseAny(aRangeStr, &rDoc, formula::FormulaGrammar::CONV_XL_R1C1);
    if ((nRes & ScRefFlags::VALID) == ScRefFlags::ZERO)
        return false;

    if (aRange.aStart.Tab() != aRange.aEnd.Tab())
        // We don't support multi-sheet range for this.
        return false;

    const ScMatrix& rCache = mpMOper->GetCache();
    SCSIZE nC, nR;
    rCache.GetDimensions(nC, nR);
    if (!nC || !nR)
        // cache matrix is empty.
        return false;

    ScExternalRefManager* pRefMgr = rDoc.GetExternalRefManager();
    sal_uInt16 nFileId = pRefMgr->getExternalFileId(rUrl);
    ScExternalRefCache::TableTypeRef xTab = pRefMgr->getCacheTable(nFileId, aSheet, true);
    if (!xTab)
        // cache table creation failed.
        return false;

    xTab->setWholeTableCached();
    for (SCSIZE i = 0; i < nR; ++i)
    {
        for (SCSIZE j = 0; j < nC; ++j)
        {
            SCCOL nCol = aRange.aStart.Col() + j;
            SCROW nRow = aRange.aStart.Row() + i;

            ScMatrixValue aVal = rCache.Get(j, i);
            switch (aVal.nType)
            {
                case ScMatValType::Boolean:
                {
                    bool b = aVal.GetBoolean();
                    ScExternalRefCache::TokenRef pToken(new formula::FormulaDoubleToken(b ? 1.0 : 0.0));
                    xTab->setCell(nCol, nRow, pToken, 0, false);
                }
                break;
                case ScMatValType::Value:
                {
                    ScExternalRefCache::TokenRef pToken(new formula::FormulaDoubleToken(aVal.fVal));
                    xTab->setCell(nCol, nRow, pToken, 0, false);
                }
                break;
                case ScMatValType::String:
                {
                    ScExternalRefCache::TokenRef pToken(new formula::FormulaStringToken(aVal.GetString()));
                    xTab->setCell(nCol, nRow, pToken, 0, false);
                }
                break;
                default:
                    ;
            }
        }
    }

    rFileId = nFileId;
    rTabName = aSheet;
    rRange = aRange;
    return true;
}

bool XclImpExtName::HasFormulaTokens() const
{
    return (mxArray.get() != nullptr);
}

// Cached external cells ======================================================

XclImpCrn::XclImpCrn( XclImpStream& rStrm, const XclAddress& rXclPos ) :
    XclImpCachedValue( rStrm ),
    maXclPos( rXclPos )
{
}

// Sheet in an external document ==============================================

XclImpSupbookTab::XclImpSupbookTab( const OUString& rTabName ) :
    maTabName( rTabName )
{
}

void XclImpSupbookTab::ReadCrn( XclImpStream& rStrm, const XclAddress& rXclPos )
{
    XclImpCrnRef crnRef( new XclImpCrn(rStrm, rXclPos) );
    maCrnList.push_back( crnRef );
}

void XclImpSupbookTab::LoadCachedValues( const ScExternalRefCache::TableTypeRef& pCacheTable,
        svl::SharedStringPool& rPool )
{
    if (maCrnList.empty())
        return;

    for (const auto& rxCrn : maCrnList)
    {
        const XclImpCrn* const pCrn = rxCrn.get();
        const XclAddress& rAddr = pCrn->GetAddress();
        switch (pCrn->GetType())
        {
            case EXC_CACHEDVAL_BOOL:
            {
                bool b = pCrn->GetBool();
                ScExternalRefCache::TokenRef pToken(new formula::FormulaDoubleToken(b ? 1.0 : 0.0));
                pCacheTable->setCell(rAddr.mnCol, rAddr.mnRow, pToken, 0, false);
            }
            break;
            case EXC_CACHEDVAL_DOUBLE:
            {
                double f = pCrn->GetValue();
                ScExternalRefCache::TokenRef pToken(new formula::FormulaDoubleToken(f));
                pCacheTable->setCell(rAddr.mnCol, rAddr.mnRow, pToken, 0, false);
            }
            break;
            case EXC_CACHEDVAL_ERROR:
            {
                double fError = XclTools::ErrorToDouble( pCrn->GetXclError() );
                ScExternalRefCache::TokenRef pToken(new formula::FormulaDoubleToken(fError));
                pCacheTable->setCell(rAddr.mnCol, rAddr.mnRow, pToken, 0, false);
            }
            break;
            case EXC_CACHEDVAL_STRING:
            {
                svl::SharedString aSS( rPool.intern( pCrn->GetString()));
                ScExternalRefCache::TokenRef pToken(new formula::FormulaStringToken( aSS));
                pCacheTable->setCell(rAddr.mnCol, rAddr.mnRow, pToken, 0, false);
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
    meType( XclSupbookType::Unknown ),
    mnSBTab( EXC_TAB_DELETED )
{
    sal_uInt16 nSBTabCnt;
    nSBTabCnt = rStrm.ReaduInt16();

    if( rStrm.GetRecLeft() == 2 )
    {
        switch( rStrm.ReaduInt16() )
        {
            case EXC_SUPB_SELF:     meType = XclSupbookType::Self;   break;
            case EXC_SUPB_ADDIN:    meType = XclSupbookType::Addin;  break;
            default:    OSL_FAIL( "XclImpSupbook::XclImpSupbook - unknown special SUPBOOK type" );
        }
        return;
    }

    OUString aEncUrl( rStrm.ReadUniString() );
    bool bSelf = false;
    XclImpUrlHelper::DecodeUrl( maXclUrl, bSelf, GetRoot(), aEncUrl );

    if( maXclUrl.equalsIgnoreAsciiCase( "\010EUROTOOL.XLA" ) )
    {
        meType = XclSupbookType::Eurotool;
        maSupbTabList.push_back( std::make_unique<XclImpSupbookTab>( maXclUrl ) );
    }
    else if( nSBTabCnt )
    {
        meType = XclSupbookType::Extern;

        //assuming all empty strings with just len header of 0
        const size_t nMinRecordSize = sizeof(sal_Int16);
        const size_t nMaxRecords = rStrm.GetRecLeft() / nMinRecordSize;
        if (nSBTabCnt > nMaxRecords)
        {
            SAL_WARN("sc", "Parsing error: " << nMaxRecords <<
                     " max possible entries, but " << nSBTabCnt << " claimed, truncating");
            nSBTabCnt = nMaxRecords;
        }

        for( sal_uInt16 nSBTab = 0; nSBTab < nSBTabCnt; ++nSBTab )
        {
            OUString aTabName( rStrm.ReadUniString() );
            maSupbTabList.push_back( std::make_unique<XclImpSupbookTab>( aTabName ) );
        }
    }
    else
    {
        meType = XclSupbookType::Special;
        // create dummy list entry
        maSupbTabList.push_back( std::make_unique<XclImpSupbookTab>( maXclUrl ) );
    }
}

void XclImpSupbook::ReadXct( XclImpStream& rStrm )
{
    rStrm.Ignore( 2 );
    mnSBTab = rStrm.ReaduInt16();
}

void XclImpSupbook::ReadCrn( XclImpStream& rStrm )
{
    if (mnSBTab >= maSupbTabList.size())
        return;
    XclImpSupbookTab& rSbTab = *maSupbTabList[mnSBTab].get();
    sal_uInt8 nXclColLast, nXclColFirst;
    sal_uInt16 nXclRow;
    nXclColLast = rStrm.ReaduInt8();
    nXclColFirst = rStrm.ReaduInt8();
    nXclRow = rStrm.ReaduInt16();

    for( sal_uInt8 nXclCol = nXclColFirst; (nXclCol <= nXclColLast) && (rStrm.GetRecLeft() > 1); ++nXclCol )
        rSbTab.ReadCrn( rStrm, XclAddress( nXclCol, nXclRow ) );
}

void XclImpSupbook::ReadExternname( XclImpStream& rStrm, ExcelToSc* pFormulaConv )
{
    maExtNameList.push_back( std::make_unique<XclImpExtName>( *this, rStrm, meType, pFormulaConv ) );
}

const XclImpExtName* XclImpSupbook::GetExternName( sal_uInt16 nXclIndex ) const
{
    if (nXclIndex == 0)
    {
        SAL_WARN("sc", "XclImpSupbook::GetExternName - index must be >0");
        return nullptr;
    }
    if (meType == XclSupbookType::Self || nXclIndex > maExtNameList.size())
        return nullptr;
    return maExtNameList[nXclIndex-1].get();
}

bool XclImpSupbook::GetLinkData( OUString& rApplic, OUString& rTopic ) const
{
    return (meType == XclSupbookType::Special) && XclImpUrlHelper::DecodeLink( rApplic, rTopic, maXclUrl );
}

const OUString& XclImpSupbook::GetMacroName( sal_uInt16 nXclNameIdx ) const
{
    OSL_ENSURE( nXclNameIdx > 0, "XclImpSupbook::GetMacroName - index must be >0" );
    const XclImpName* pName = (meType == XclSupbookType::Self) ? GetNameManager().GetName( nXclNameIdx ) : nullptr;
    return (pName && pName->IsVBName()) ? pName->GetScName() : EMPTY_OUSTRING;
}

const OUString& XclImpSupbook::GetTabName( sal_uInt16 nXtiTab ) const
{
    if (nXtiTab >= maSupbTabList.size())
        return EMPTY_OUSTRING;
    return maSupbTabList[nXtiTab]->GetTabName();
}

sal_uInt16 XclImpSupbook::GetTabCount() const
{
    return ulimit_cast<sal_uInt16>(maSupbTabList.size());
}

void XclImpSupbook::LoadCachedValues()
{
    if (meType != XclSupbookType::Extern || GetExtDocOptions().GetDocSettings().mnLinkCnt > 0 || !GetDocShell())
        return;

    OUString aAbsUrl( ScGlobal::GetAbsDocName(maXclUrl, GetDocShell()) );

    ScExternalRefManager* pRefMgr = GetRoot().GetDoc().GetExternalRefManager();
    sal_uInt16 nFileId = pRefMgr->getExternalFileId(aAbsUrl);

    for (auto& rxTab : maSupbTabList)
    {
        const OUString& rTabName = rxTab->GetTabName();
        ScExternalRefCache::TableTypeRef pCacheTable = pRefMgr->getCacheTable(nFileId, rTabName, true);
        rxTab->LoadCachedValues( pCacheTable, GetSharedStringPool());
        pCacheTable->setWholeTableCached();
    }
}

svl::SharedStringPool& XclImpSupbook::GetSharedStringPool()
{
    return GetDoc().GetSharedStringPool();
}

// Import link manager ========================================================

XclImpLinkManagerImpl::XclImpLinkManagerImpl( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}

void XclImpLinkManagerImpl::ReadExternsheet( XclImpStream& rStrm )
{
    sal_uInt16 nXtiCount;
    nXtiCount = rStrm.ReaduInt16();
    OSL_ENSURE( static_cast< std::size_t >( nXtiCount * 6 ) == rStrm.GetRecLeft(), "XclImpLinkManagerImpl::ReadExternsheet - invalid count" );
    nXtiCount = static_cast< sal_uInt16 >( ::std::min< std::size_t >( nXtiCount, rStrm.GetRecLeft() / 6 ) );

    /*  #i104057# A weird external XLS generator writes multiple EXTERNSHEET
        records instead of only one as expected. Surprisingly, Excel seems to
        insert the entries of the second record before the entries of the first
        record. */
    XclImpXtiVector aNewEntries( nXtiCount );
    for( auto& rNewEntry : aNewEntries )
    {
        if (!rStrm.IsValid())
            break;
        rStrm >> rNewEntry;
    }
    maXtiList.insert( maXtiList.begin(), aNewEntries.begin(), aNewEntries.end() );

    LoadCachedValues();
}

void XclImpLinkManagerImpl::ReadSupbook( XclImpStream& rStrm )
{
    maSupbookList.push_back( std::make_unique<XclImpSupbook>( rStrm ) );
}

void XclImpLinkManagerImpl::ReadXct( XclImpStream& rStrm )
{
    if( !maSupbookList.empty() )
        maSupbookList.back()->ReadXct( rStrm );
}

void XclImpLinkManagerImpl::ReadCrn( XclImpStream& rStrm )
{
    if( !maSupbookList.empty() )
        maSupbookList.back()->ReadCrn( rStrm );
}

void XclImpLinkManagerImpl::ReadExternname( XclImpStream& rStrm, ExcelToSc* pFormulaConv )
{
    if( !maSupbookList.empty() )
        maSupbookList.back()->ReadExternname( rStrm, pFormulaConv );
}

bool XclImpLinkManagerImpl::IsSelfRef( sal_uInt16 nXtiIndex ) const
{
    const XclImpSupbook* pSupbook = GetSupbook( nXtiIndex );
    return pSupbook && (pSupbook->GetType() == XclSupbookType::Self);
}

bool XclImpLinkManagerImpl::GetScTabRange(
        SCTAB& rnFirstScTab, SCTAB& rnLastScTab, sal_uInt16 nXtiIndex ) const
{
    if( const XclImpXti* pXti = GetXti( nXtiIndex ) )
    {
        if (!maSupbookList.empty() && (pXti->mnSupbook < maSupbookList.size()) )
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
    return pSupbook ? pSupbook->GetExternName( nExtName ) : nullptr;
}

const OUString* XclImpLinkManagerImpl::GetSupbookUrl( sal_uInt16 nXtiIndex ) const
{
    const XclImpSupbook* p = GetSupbook( nXtiIndex );
    if (!p)
        return nullptr;
    return &p->GetXclUrl();
}

const OUString& XclImpLinkManagerImpl::GetSupbookTabName( sal_uInt16 nXti, sal_uInt16 nXtiTab ) const
{
    const XclImpSupbook* p = GetSupbook(nXti);
    return p ? p->GetTabName(nXtiTab) : EMPTY_OUSTRING;
}

bool XclImpLinkManagerImpl::GetLinkData( OUString& rApplic, OUString& rTopic, sal_uInt16 nXtiIndex ) const
{
    const XclImpSupbook* pSupbook = GetSupbook( nXtiIndex );
    return pSupbook && pSupbook->GetLinkData( rApplic, rTopic );
}

const OUString& XclImpLinkManagerImpl::GetMacroName( sal_uInt16 nExtSheet, sal_uInt16 nExtName ) const
{
    const XclImpSupbook* pSupbook = GetSupbook( nExtSheet );
    return pSupbook ? pSupbook->GetMacroName( nExtName ) : EMPTY_OUSTRING;
}

const XclImpXti* XclImpLinkManagerImpl::GetXti( sal_uInt16 nXtiIndex ) const
{
    return (nXtiIndex < maXtiList.size()) ? &maXtiList[ nXtiIndex ] : nullptr;
}

const XclImpSupbook* XclImpLinkManagerImpl::GetSupbook( sal_uInt16 nXtiIndex ) const
{
    if ( maSupbookList.empty() )
        return nullptr;
    const XclImpXti* pXti = GetXti( nXtiIndex );
    if (!pXti || pXti->mnSupbook >= maSupbookList.size())
        return nullptr;
    return maSupbookList.at( pXti->mnSupbook ).get();
}

void XclImpLinkManagerImpl::LoadCachedValues()
{
    // Read all CRN records which can be accessed via XclImpSupbook, and store
    // the cached values to the external reference manager.
    for (auto& rxSupbook : maSupbookList)
        rxSupbook->LoadCachedValues();
}

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

const OUString* XclImpLinkManager::GetSupbookUrl( sal_uInt16 nXtiIndex ) const
{
    return mxImpl->GetSupbookUrl(nXtiIndex);
}

const OUString& XclImpLinkManager::GetSupbookTabName( sal_uInt16 nXti,  sal_uInt16 nXtiTab ) const
{
    return mxImpl->GetSupbookTabName(nXti, nXtiTab);
}

bool XclImpLinkManager::GetLinkData( OUString& rApplic, OUString& rTopic, sal_uInt16 nXtiIndex ) const
{
    return mxImpl->GetLinkData( rApplic, rTopic, nXtiIndex );
}

const OUString& XclImpLinkManager::GetMacroName( sal_uInt16 nExtSheet, sal_uInt16 nExtName ) const
{
    return mxImpl->GetMacroName( nExtSheet, nExtName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

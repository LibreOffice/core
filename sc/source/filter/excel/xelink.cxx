/*************************************************************************
 *
 *  $RCSfile: xelink.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:04:34 $
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

const sal_Unicode DDE_DELIM         = '|';


// Excel sheet indexes ========================================================

const sal_uInt8 EXC_TABBUF_EXPORT   = 0x00;
const sal_uInt8 EXC_TABBUF_IGNORE   = 0x01;
const sal_uInt8 EXC_TABBUF_EXTERN   = 0x02;


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
            mnCodeCnt = static_cast< sal_uInt16 >( ::std::min( pCList->Count(), 0xFFFFUL ) );
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
    return (nScTab < mnScCnt) ? maIndexVec[ nScTab ].first : EXC_TAB_INVALID;
}

sal_uInt16 XclExpTabIdBuffer::GetRealScTab( sal_uInt16 nSortedTab ) const
{
    DBG_ASSERT( nSortedTab < mnScCnt, "XclExpTabIdBuffer::GetRealScIndex - out of range" );
    return (nSortedTab < mnScCnt) ? maFromSortedVec[ nSortedTab ] : EXC_TAB_INVALID;
}

sal_uInt16 XclExpTabIdBuffer::GetSortedScTab( sal_uInt16 nScTab ) const
{
    DBG_ASSERT( nScTab < mnScCnt, "XclExpTabIdBuffer::GetSortedScIndex - out of range" );
    return (nScTab < mnScCnt) ? maToSortedVec[ nScTab ] : EXC_TAB_INVALID;
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
            maIndexVec[ nScTab ].first = EXC_TAB_INVALID;
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


// ----------------------------------------------------------------------------

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
            return static_cast< sal_uInt16 >( ::std::min( maNameList.GetCurPos() + 1UL, 0xFFFFUL ) );
    return 0;
}

sal_uInt16 XclExpExtNameList::Append( XclExpExtNameBase* pName )
{
    maNameList.Append( pName );
    return static_cast< sal_uInt16 >( ::std::min( maNameList.Count(), 0xFFFFUL ) );
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


// ----------------------------------------------------------------------------

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
            if( !Exists( nCol, nRow ) )
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
}

bool XclExpXct::Exists( sal_uInt16 nCol, sal_uInt16 nRow ) const
{
    for( const XclExpCrn* pCrn = maCrnList.First(); pCrn; pCrn = maCrnList.Next() )
        if( pCrn->IsAddress( nCol, nRow ) )
            return true;
    return false;
}

void XclExpXct::Save( XclExpStream& rStrm )
{
    XclExpRecord::Save( rStrm );
    maCrnList.Save( rStrm );
}

void XclExpXct::WriteBody( XclExpStream& rStrm )
{
    sal_uInt16 nCount = static_cast< sal_uInt16 >( ::std::min( maCrnList.Count(), 0xFFFFUL ) );
    rStrm << nCount << mnXclTab;
}


// External documents =========================================================

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
    XclExpXct* pXct = maXctList.GetObject( nXct );
    if( pXct )
        pXct->StoreCellRange( rRoot, rRange );
}

sal_uInt16 XclExpSupbook::InsertTable( const String& rTabName )
{
    DBG_ASSERT( meType == xlSBUrl, "XclExpSupbook::InsertTable - don't insert table names here" );
    XclExpXct* pXct = new XclExpXct( rTabName );
    SetRecSize( GetRecSize() + pXct->GetTableBytes() );
    maXctList.Append( pXct );

    sal_uInt16 nTabNum = static_cast< sal_uInt16 >( ::std::min( maXctList.Count() - 1UL, 0xFFFFUL ) );
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
            sal_uInt16 nCount = static_cast< sal_uInt16 >( ::std::min( maXctList.Count(), 0xFFFFUL ) );
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


// ----------------------------------------------------------------------------

XclExpSupbookBuffer::XclExpSupbookBuffer( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    mnAddInSB( EXC_TAB_INVALID )
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

    XclExpSupbook* pBook = GetSupbook( nXclTab );
    if( pBook )
        pBook->StoreCellRange( *this, rRange, maXtiBuffer[ nXclTab ] );
}

void XclExpSupbookBuffer::InsertAddIn( sal_uInt16& rnSupbook, sal_uInt16& rnExtName, const String& rName )
{
    XclExpSupbook* pBook;
    if( mnAddInSB == EXC_TAB_INVALID )
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
        rnSupbook = Append( pBook = new XclExpSupbook( *this, rApplic, rTopic ) );
    rnExtName = pBook->InsertDde( *this, rItem );
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
            rnIndex = static_cast< sal_uInt16 >( ::std::min( maSupbookList.GetCurPos(), 0xFFFFUL ) );
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
            rnIndex = static_cast< sal_uInt16 >( ::std::min( maSupbookList.GetCurPos(), 0xFFFFUL ) );
            return pBook;
        }
    }
    return NULL;
}

sal_uInt16 XclExpSupbookBuffer::Append( XclExpSupbook* pBook )
{
    maSupbookList.Append( pBook );
    return static_cast< sal_uInt16 >( ::std::min( maSupbookList.Count() - 1, 0xFFFFUL ) );
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
        nPos = Append( pBook = new XclExpSupbook( *this, rUrl ) );
    maSBIndexBuffer[ nXclTab ] = nPos;

    // append new table name, save position in maXtiBuffer
    nPos = pBook->InsertTable( GetDoc().GetLinkTab( nScTab ) );
    maXtiBuffer[ nXclTab ] = nPos;
}


// Export link manager ========================================================

XclExpLinkManager::XclExpLinkManager( const XclExpRoot& rRoot ) :
    XclExpRecord( EXC_ID_EXTERNSHEET ),
    XclExpRoot( rRoot ),
    maSBBuffer( rRoot )
{
    FindXti( 0, 0 );   // dummy to avoid empty list
}

sal_uInt16 XclExpLinkManager::FindXti( sal_uInt16 nXclFirst, sal_uInt16 nXclLast )
{
    sal_uInt16 nSupb, nXtiFirst, nXtiLast;
    maSBBuffer.GetXtiRange( nSupb, nXtiFirst, nXtiLast, nXclFirst, nXclLast );
    return InsertXti( nSupb, nXtiFirst, nXtiLast );
}

void XclExpLinkManager::StoreCellCont( const SingleRefData& rRef )
{
    if( GetTabIdBuffer().IsExternal( rRef.nTab ) )
        maSBBuffer.StoreCellRange( ScRange(
            rRef.nCol, rRef.nRow, rRef.nTab, rRef.nCol, rRef.nRow, rRef.nTab ) );
}

void XclExpLinkManager::StoreCellRange( const SingleRefData& rRef1, const SingleRefData& rRef2 )
{
    for( sal_uInt16 nScTab = rRef1.nTab; nScTab <= rRef2.nTab; ++nScTab )
        if( GetTabIdBuffer().IsExternal( nScTab ) )
            maSBBuffer.StoreCellRange( ScRange(
                rRef1.nCol, rRef1.nRow, nScTab, rRef2.nCol, rRef2.nRow, nScTab ) );
}

void XclExpLinkManager::InsertAddIn( sal_uInt16& rnXti, sal_uInt16& rnExtName, const String& rName )
{
    sal_uInt16 nSupbook;
    maSBBuffer.InsertAddIn( nSupbook, rnExtName, rName );
    rnXti = InsertXti( nSupbook, EXC_TAB_EXTERNAL, EXC_TAB_EXTERNAL );
}

bool XclExpLinkManager::InsertDde(
        sal_uInt16& rnXti, sal_uInt16& rnExtName,
        const String& rApplic, const String& rTopic, const String& rItem )
{
    sal_uInt16 nSupbook;
    maSBBuffer.InsertDde( nSupbook, rnExtName, rApplic, rTopic, rItem );
    if( !rnExtName )
        return false;
    rnXti = InsertXti( nSupbook, EXC_TAB_EXTERNAL, EXC_TAB_EXTERNAL );
    return true;
}

sal_uInt16 XclExpLinkManager::AppendXti( XclExpXti* pXti )
{
    maXtiList.Append( pXti );
    return static_cast< sal_uInt16 >( ::std::min( maXtiList.Count() - 1UL, 0xFFFFUL ) );
}

sal_uInt16 XclExpLinkManager::InsertXti( sal_uInt16 nSupbook, sal_uInt16 nXtiFirst, sal_uInt16 nXtiLast )
{
    for( const XclExpXti* pXti = maXtiList.First(); pXti; pXti = maXtiList.Next() )
        if( pXti->Equals( nSupbook, nXtiFirst, nXtiLast ) )
            return static_cast< sal_uInt16 >( ::std::min( maXtiList.GetCurPos(), 0xFFFFUL ) );
    return AppendXti( new XclExpXti( nSupbook, nXtiFirst, nXtiLast ) );
}

void XclExpLinkManager::Save( XclExpStream& rStrm )
{
    // SUPBOOKs, XCTs, CRNs, EXTERNNAMEs
    maSBBuffer.Save( rStrm );

    // EXTERNSHEET
    SetRecSize( 2 + 6 * maXtiList.Count() );
    XclExpRecord::Save( rStrm );
}

void XclExpLinkManager::WriteBody( XclExpStream& rStrm )
{
    sal_uInt16 nCount = static_cast< sal_uInt16 >( ::std::min( maXtiList.Count(), 0xFFFFUL ) );
    rStrm << nCount;

    rStrm.SetSliceSize( 6 );
    for( const XclExpXti* pXti = maXtiList.First(); pXti; pXti = maXtiList.Next() )
        pXti->Save( rStrm );
}


// ============================================================================


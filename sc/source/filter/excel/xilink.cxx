/*************************************************************************
 *
 *  $RCSfile: xilink.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-16 08:17:14 $
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

#include "root.hxx"


// Excel sheet indexes ========================================================

void XclImpTabIdBuffer::ReadTabid( XclImpStream& rStrm )
{
    if( rStrm.GetRoot().GetBiff() == xlBiff8 )
    {
        sal_uInt32 nReadCount = rStrm.GetRecLeft() / 2;
        DBG_ASSERT( nReadCount <= 0xFFFF, "XclImpTabIdBuffer::ReadTabid - record too long" );
        maTabIdVec.clear();
        maTabIdVec.reserve( nReadCount );
        for( sal_uInt32 nIndex = 0; rStrm.IsValid() && (nIndex < nReadCount); ++nIndex )
            // #93471# zero index is not allowed in BIFF8, but it seems that it occurs in real life
            maTabIdVec.push_back( rStrm.ReaduInt16() );
    }
    else
        DBG_ASSERT_BIFF( rStrm.GetRoot().GetBiff() < xlBiff8 );
}

sal_uInt16 XclImpTabIdBuffer::GetCurrentIndex( sal_uInt16 nCreatedId, sal_uInt16 nMaxTabId ) const
{
    sal_uInt16 nReturn = 0;
    ScfUInt16Vec::const_iterator aIterEnd = maTabIdVec.end();
    for( ScfUInt16Vec::const_iterator aIter = maTabIdVec.begin(); aIter != aIterEnd; ++aIter )
    {
        sal_uInt16 nValue = *aIter;
        if( nValue == nCreatedId )
            return nReturn;
        if( nValue <= nMaxTabId )
            ++nReturn;
    }
    return 0;
}


// External names =============================================================

XclImpExtName::XclImpExtName( XclImpStream& rStrm )
{
    sal_uInt16 nFlags;
    sal_uInt8 nLen;

    rStrm >> nFlags >> mnStorageId >> nLen ;
    rStrm.AppendUniString( maName, nLen );

    if( ::get_flag( nFlags, EXC_EXTN_BUILTIN ) || !::get_flag( nFlags, EXC_EXTN_OLE_OR_DDE ) )
    {
        meType = xlExtName;
        maAddInName = rStrm.GetRoot().GetScAddInName( maName );
        ScfTools::ConvertToScDefinedName( maName );
    }
    else
        meType = ::get_flagvalue( nFlags, EXC_EXTN_OLE, xlExtOLE, xlExtDDE );

    if(meType == xlExtDDE && (rStrm.GetRecLeft() > 1))
    {
        sal_uInt8 nLastCol;
        sal_uInt16 nLastRow;
        rStrm >> nLastCol >> nLastRow;

        if( nLastRow <= MAXROW && nLastCol <= MAXCOL )
        {
            mpDdeMatrix.reset( new XclImpCachedMatrix(nLastCol + 1, nLastRow + 1));

            for( sal_uInt16 nRow = 0; nRow < nLastRow+1 && (rStrm.GetRecLeft() > 1); ++nRow )
                for( sal_uInt16 nCol = 0; nCol < nLastCol+1 && (rStrm.GetRecLeft() > 1); ++nCol )
                    mpDdeMatrix->AppendValue( new XclImpCachedValue( rStrm, nCol, nRow ));
        }
    }
}


void XclImpExtName::CreateDdeData( ScDocument& rDoc, const String& rApplic,const String& rTopic  )const
{
    rDoc.CreateDdeLink( rApplic, rTopic, GetName() );
    sal_uInt16 nPosition;
    if(!rDoc.FindDdeLink(rApplic, rTopic, GetName(), SC_DDE_IGNOREMODE, nPosition))
        return ;

    if(!mpDdeMatrix.get())
        return ;

    ScMatrix* pMatrix = NULL;
    if(!(rDoc.CreateDdeLinkResultDimension(nPosition, mpDdeMatrix->GetColumns(), mpDdeMatrix->GetRows(), pMatrix)) || !pMatrix)
        return ;

    mpDdeMatrix->FillMatrix( rDoc, pMatrix);
}


// ----------------------------------------------------------------------------

const XclImpExtName* XclImpExtNameList::GetName( sal_uInt16 nXclIndex ) const
{
    DBG_ASSERT( nXclIndex, "XclImpExtNameList::GetName - index must be >0" );
    return GetObject( nXclIndex - 1 );
}


// Cached external cells ======================================================

XclImpCrn::XclImpCrn( XclImpStream& rStrm, sal_uInt16 nCol, sal_uInt16 nRow ) :
    XclImpCachedValue( rStrm, nCol, nRow )
{
}

void XclImpCrn::SetCell( ScDocument& rDoc, sal_uInt16 nTab ) const
{
    switch( GetType() )
    {
        case EXC_CACHEDVAL_DOUBLE:
            rDoc.SetValue( mnCol, mnRow, nTab, GetValue() );
        break;
        case EXC_CACHEDVAL_STRING:
        {
            DBG_ASSERT( GetString(), "XclImpCrn::SetCell - missing string" );
            ScStringCell* pStrCell = new ScStringCell( *GetString() );
            rDoc.PutCell( mnCol, mnRow, nTab, pStrCell );
        }
        break;
        case EXC_CACHEDVAL_BOOL:
        case EXC_CACHEDVAL_ERROR:
        {
            ScFormulaCell* pFmlaCell = new ScFormulaCell( &rDoc, ScAddress( mnCol, mnRow, nTab ), GetTokArray() );
            pFmlaCell->SetDouble( GetValue() );
            rDoc.PutCell( mnCol, mnRow, nTab, pFmlaCell );
        }
        break;
    }
}


// External documents =========================================================

XclImpSupbookTab::XclImpSupbookTab( const String& rName ) :
    maName( rName ),
    mnScTab( EXC_TAB_INVALID )
{
}

void XclImpSupbookTab::CreateTable( ScDocument& rDoc, const String& rUrl )
{
    if( rDoc.LinkEmptyTab( mnScTab, ScGlobal::GetDocTabName( rUrl, maName ), rUrl, maName ) )
    {
        for( const XclImpCrn* pCrn = maCrnList.First(); pCrn; pCrn = maCrnList.Next() )
            pCrn->SetCell( rDoc, mnScTab );
    }
    else
        mnScTab = EXC_TAB_INVALID;
}


// ----------------------------------------------------------------------------

XclImpSupbook::XclImpSupbook( XclImpStream& rStrm ) :
    mnCurrExcTab( EXC_TAB_INVALID ),
    mbSelf( false ),
    mbAddIn( false )
{
    sal_uInt16 nTabCnt;
    rStrm >> nTabCnt;

    if( rStrm.GetRecLeft() == 2 )
    {
        sal_uInt16 nType = rStrm.ReaduInt16();
        mbSelf = (nType == EXC_SUPB_SELF);
        mbAddIn = (nType == EXC_SUPB_ADDIN);
        return;
    }

    ReadUrl( rStrm, maUrl, mbSelf );

    if( nTabCnt )
    {
        for( sal_uInt16 nTab = 0; nTab < nTabCnt; ++nTab )
        {
            String aTabName;
            ReadTabName( rStrm, aTabName );
            maSupbookTabList.Append( new XclImpSupbookTab( aTabName ) );
        }
    }
    else
        // create dummy list entry
        maSupbookTabList.Append( new XclImpSupbookTab( maUrl ) );
}

void XclImpSupbook::ReadUrl( XclImpStream& rStrm, String& rUrl, bool& rbSelf )
{
    String aEncUrl, aTabName;
    rStrm.AppendUniString( aEncUrl );
    XclImpUrlHelper::DecodeUrl( rUrl, aTabName, rbSelf, rStrm.GetRoot(), aEncUrl );
}

void XclImpSupbook::ReadTabName( XclImpStream& rStrm, String& rTabName )
{
    rStrm.AppendUniString( rTabName );
    ScfTools::ConvertToScSheetName( rTabName );
}

void XclImpSupbook::ReadXct( XclImpStream& rStrm )
{
    rStrm.Ignore( 2 );
    rStrm >> mnCurrExcTab;
}

void XclImpSupbook::ReadCrn( XclImpStream& rStrm )
{
    XclImpSupbookTab* pTab = maSupbookTabList.GetObject( mnCurrExcTab );
    if( pTab )
    {
        sal_uInt8 nLastCol, nFirstCol;
        sal_uInt16 nRow;
        rStrm >> nLastCol >> nFirstCol >> nRow;

        for( sal_uInt16 nCol = nFirstCol; (nCol <= nLastCol) && (rStrm.GetRecLeft() > 1); ++nCol )
            pTab->AppendCrn( new XclImpCrn( rStrm, nCol, nRow ) );
    }
}

void XclImpSupbook::ReadExternname( XclImpStream& rStrm )
{
    maExtNameList.Append( new XclImpExtName( rStrm ) );
}

const XclImpExtName* XclImpSupbook::GetExtName( sal_uInt16 nXclIndex ) const
{
    return maExtNameList.GetName( nXclIndex );
}

bool XclImpSupbook::GetLink( String& rApplic, String& rDoc ) const
{
    xub_StrLen nPos = maUrl.Search( EXC_DDE_DELIM );
    if( mbSelf || !maUrl.Len() || (nPos == STRING_NOTFOUND) )
        return false;
    rApplic = maUrl.Copy( 0, nPos );
    rDoc = maUrl.Copy( nPos + 1 );
    return rApplic.Len() && rDoc.Len();
}

sal_uInt16 XclImpSupbook::GetScTabNum( sal_uInt16 nExcTabNum ) const
{
    if( mbSelf )
        return nExcTabNum;
    const XclImpSupbookTab* pTab = maSupbookTabList.GetObject( nExcTabNum );
    return pTab ? pTab->GetScTab() : EXC_TAB_INVALID;
}

sal_uInt16 XclImpSupbook::GetScTabNum( const String& rTabName ) const
{
    sal_uInt32 nTabCount = maSupbookTabList.Count();
    for( sal_uInt32 nIndex = 0; nIndex < nTabCount; ++nIndex )
    {
        const XclImpSupbookTab* pTab = maSupbookTabList.GetObject( nIndex );
        if( pTab && (pTab->GetName() == rTabName) )
            return pTab->GetScTab();
    }
    return EXC_TAB_INVALID;
}

void XclImpSupbook::CreateTables( const XclImpRoot& rRoot, sal_uInt16 nFirstTab, sal_uInt16 nLastTab ) const
{
    if( mbSelf || (rRoot.mpRD->pExtDocOpt->nLinkCnt >= 1) || !rRoot.GetDocShell() )
        return;

    String aScUrl( ScGlobal::GetAbsDocName( maUrl, rRoot.GetDocShell() ) );

    for( sal_uInt16 nTab = nFirstTab; nTab <= nLastTab; ++nTab )
    {
        XclImpSupbookTab* pSBTab = maSupbookTabList.GetObject( nTab );
        if( pSBTab )
            pSBTab->CreateTable( rRoot.GetDoc(), aScUrl );
    }
}


// ----------------------------------------------------------------------------

const XclImpSupbook* XclImpSupbookBuffer::GetSupbook( const String& rUrl ) const
{
    sal_uInt32 nCount = maSupbookList.Count();
    for( sal_uInt32 nIndex = 0; nIndex < nCount; ++nIndex )
    {
        const XclImpSupbook* pSupbook = GetSupbook( nIndex );
        if( pSupbook && (pSupbook->GetUrl() == rUrl) )
            return pSupbook;
    }
    return NULL;
}


// Import link manager ========================================================

XclImpStream& operator>>( XclImpStream& rStrm, XclImpXti& rXti )
{
    return rStrm >> rXti.mnSupbook >> rXti.mnFirst >> rXti.mnLast;
}


// ----------------------------------------------------------------------------

XclImpLinkManager::XclImpLinkManager( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    mbCreated( false )
{
}

XclImpLinkManager::~XclImpLinkManager()
{
}

void XclImpLinkManager::ReadExternsheet( XclImpStream& rStrm )
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

void XclImpLinkManager::ReadSupbook( XclImpStream& rStrm )
{
    maSupbookBuffer.ReadSupbook( rStrm );
}

void XclImpLinkManager::ReadXct( XclImpStream& rStrm )
{
    XclImpSupbook* pSupbook = maSupbookBuffer.GetCurrSupbook();
    if( pSupbook )
        pSupbook->ReadXct( rStrm );
}

void XclImpLinkManager::ReadCrn( XclImpStream& rStrm )
{
    XclImpSupbook* pSupbook = maSupbookBuffer.GetCurrSupbook();
    if( pSupbook )
        pSupbook->ReadCrn( rStrm );
}

void XclImpLinkManager::ReadExternname( XclImpStream& rStrm )
{
    XclImpSupbook* pSupbook = maSupbookBuffer.GetCurrSupbook();
    if( pSupbook )
        pSupbook->ReadExternname( rStrm );
}

const XclImpSupbook* XclImpLinkManager::GetSupbook( sal_uInt32 nXtiIndex ) const
{
    const XclImpXti* pXti = GetXti( nXtiIndex );
    return pXti ? maSupbookBuffer.GetSupbook( pXti->mnSupbook ) : NULL;
}

void XclImpLinkManager::CreateTables()
{
    DBG_ASSERT( !mbCreated, "XclImpLinkManager::CreateTables - multiple call" );
    if( mbCreated ) return;

    sal_uInt16 nFirst, nLast;
    sal_uInt32 nCount = maSupbookBuffer.Count();

    for( sal_uInt16 nSupbook = 0; nSupbook < nCount; ++nSupbook )
    {
        const XclImpSupbook* pSupbook = maSupbookBuffer.GetSupbook( nSupbook );
        bool bLoop = FindNextTabRange( nFirst, nLast, nSupbook, 0 );
        while( bLoop && pSupbook )
        {
            pSupbook->CreateTables( *this, nFirst, nLast );
            // #96263# don't search again if last sheet == 0xFFFF
            bLoop = (nLast < 0xFFFF) && FindNextTabRange( nFirst, nLast, nSupbook, nLast + 1 );
        }
    }
    mbCreated = true;
}

bool XclImpLinkManager::FindNextTabRange(
        sal_uInt16& rnFirst, sal_uInt16& rnLast,
        sal_uInt16 nSupb, sal_uInt16 nStart ) const
{
    rnFirst = rnLast = 0xFFFF;
    for( const XclImpXti* pXti = maXtiList.First(); pXti; pXti = maXtiList.Next() )
    {
        if( (nSupb == pXti->mnSupbook) && (nStart <= pXti->mnLast) && (pXti->mnFirst < rnFirst) )
        {
            rnFirst = ::std::max( nStart, pXti->mnFirst );
            rnLast = pXti->mnLast;
        }
    }
    return rnFirst < 0xFFFF;
}


// ============================================================================


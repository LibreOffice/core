/*************************************************************************
 *
 *  $RCSfile: XclExpChangeTrack.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: dr $ $Date: 2001-02-26 06:53:18 $
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

//___________________________________________________________________

#ifndef _SC_XCLEXPCHANGETRACK_HXX
#include "XclExpChangeTrack.hxx"
#endif

#ifndef SC_CELL_HXX
#include "cell.hxx"
#endif
#ifndef _EXCUPN_HXX
#include "excupn.hxx"
#endif
#ifndef _XCL97REC_HXX
#include "xcl97rec.hxx"
#endif

//___________________________________________________________________

extern const sal_Char*  pRevLogStreamName;
extern const sal_Char*  pUserNamesStreamName;

//___________________________________________________________________
// local functions

void lcl_WriteDateTime( XclExpStream& rStrm, const DateTime& rDateTime )
{
    rStrm.SetSliceLen( 7 );
    rStrm   << (sal_uInt16) rDateTime.GetYear()
            << (sal_uInt8)  rDateTime.GetMonth()
            << (sal_uInt8)  rDateTime.GetDay()
            << (sal_uInt8)  rDateTime.GetHour()
            << (sal_uInt8)  rDateTime.GetMin()
            << (sal_uInt8)  rDateTime.GetSec();
    rStrm.SetSliceLen( 0 );
}

// write string and fill rest of <nLength> with zero bytes
// <nLength> is without string header
void lcl_WriteFixedString( XclExpStream& rStrm, const XclExpUniString& rString, ULONG nLength )
{
    ULONG nStrBytes = rString.GetBufferByteCount();
    DBG_ASSERT( nLength >= nStrBytes, "lcl_WriteFixedString - String too long" );
    if( rString.GetLen() )
        rString.Write( rStrm );
    if( nLength > nStrBytes )
        rStrm.WriteZeroBytes( nLength - nStrBytes );
}

inline void lcl_GenerateGUID( sal_uInt8* pGUID, sal_Bool& rValidGUID )
{
    rtl_createUuid( pGUID, rValidGUID ? pGUID : NULL, sal_False );
    rValidGUID = sal_True;
}

inline void lcl_WriteGUID( XclExpStream& rStrm, const sal_uInt8* pGUID )
{
    rStrm.SetSliceLen( 16 );
    for( sal_uInt32 nIndex = 0; nIndex < 16; nIndex++ )
        rStrm << pGUID[ nIndex ];
    rStrm.SetSliceLen( 0 );
}

//___________________________________________________________________

XclExpUserBView::XclExpUserBView( const String& rUsername, const sal_uInt8* pGUID ) :
    sUsername( rUsername )
{
    memcpy( aGUID, pGUID, 16 );
}

void XclExpUserBView::SaveCont( XclExpStream& rStrm )
{
    rStrm   << (sal_uInt32) 0xFF078014
            << (sal_uInt32) 0x00000001;
    lcl_WriteGUID( rStrm, aGUID );
    rStrm.WriteZeroBytes( 8 );
    rStrm   << (sal_uInt32) 1200
            << (sal_uInt32) 1000
            << (sal_uInt16) 1000
            << (sal_uInt16) 0x0CF7
            << (sal_uInt16) 0x0000
            << (sal_uInt16) 0x0001
            << (sal_uInt16) 0x0000;
    if( sUsername.GetLen() )
        sUsername.Write( rStrm );
}

UINT16 XclExpUserBView::GetNum() const
{
    return 0x01A9;
}

ULONG XclExpUserBView::GetLen() const
{
    return 50 + (sUsername.GetLen() ? sUsername.GetByteCount() : 0);
}

//___________________________________________________________________

XclExpUserBViewList::XclExpUserBViewList( const ScChangeTrack& rChangeTrack )
{
    sal_uInt8 aGUID[ 16 ];
    sal_Bool bValidGUID = sal_False;
    const StrCollection& rStrColl = rChangeTrack.GetUserCollection();
    for( USHORT nIndex = 0; nIndex < rStrColl.GetCount(); nIndex++ )
    {
        const StrData* pStrData = (const StrData*) rStrColl.At( nIndex );
        lcl_GenerateGUID( aGUID, bValidGUID );
        if( pStrData )
            List::Insert( new XclExpUserBView( pStrData->GetString(), aGUID ), LIST_APPEND );
    }
}

XclExpUserBViewList::~XclExpUserBViewList()
{
    for( XclExpUserBView* pRec = _First(); pRec; pRec = _Next() )
        delete pRec;
}

void XclExpUserBViewList::Save( XclExpStream& rStrm )
{
    for( XclExpUserBView* pRec = _First(); pRec; pRec = _Next() )
        pRec->Save( rStrm );
}

//___________________________________________________________________

XclExpUsersViewBegin::XclExpUsersViewBegin( const sal_uInt8* pGUID, sal_uInt32 nTab ) :
    nCurrTab( nTab )
{
    memcpy( aGUID, pGUID, 16 );
}

void XclExpUsersViewBegin::SaveCont( XclExpStream& rStrm )
{
    lcl_WriteGUID( rStrm, aGUID );
    rStrm   << nCurrTab
            << (sal_uInt32) 100
            << (sal_uInt32) 64
            << (sal_uInt32) 3
            << (sal_uInt32) 0x0000003C
            << (sal_uInt16) 0
            << (sal_uInt16) 3
            << (sal_uInt16) 0
            << (sal_uInt16) 3
            << (double)     0
            << (double)     0
            << (sal_Int16)  -1
            << (sal_Int16)  -1;
}

UINT16 XclExpUsersViewBegin::GetNum() const
{
    return 0x01AA;
}

ULONG XclExpUsersViewBegin::GetLen() const
{
    return 64;
}

//___________________________________________________________________

void XclExpUsersViewEnd::SaveCont( XclExpStream& rStrm )
{
    rStrm << (sal_uInt16) 0x0001;
}

UINT16 XclExpUsersViewEnd::GetNum() const
{
    return 0x01AB;
}

ULONG XclExpUsersViewEnd::GetLen() const
{
    return 2;
}

//___________________________________________________________________

void XclExpChTr0x0191::SaveCont( XclExpStream& rStrm )
{
    rStrm << (sal_uInt16) 0x0000;
}

UINT16 XclExpChTr0x0191::GetNum() const
{
    return 0x0191;
}

ULONG XclExpChTr0x0191::GetLen() const
{
    return 2;
}

//___________________________________________________________________

void XclExpChTr0x0198::SaveCont( XclExpStream& rStrm )
{
    rStrm   << (sal_uInt16) 0x0006
            << (sal_uInt16) 0x0000;
}

UINT16 XclExpChTr0x0198::GetNum() const
{
    return 0x0198;
}

ULONG XclExpChTr0x0198::GetLen() const
{
    return 4;
}

//___________________________________________________________________

void XclExpChTr0x0192::SaveCont( XclExpStream& rStrm )
{
    rStrm << (sal_uInt16) 0x0022;
    rStrm.WriteZeroBytes( 510 );
}

UINT16 XclExpChTr0x0192::GetNum() const
{
    return 0x0192;
}

ULONG XclExpChTr0x0192::GetLen() const
{
    return 512;
}

//___________________________________________________________________

void XclExpChTr0x0197::SaveCont( XclExpStream& rStrm )
{
    rStrm << (sal_uInt16) 0x0000;
}

UINT16 XclExpChTr0x0197::GetNum() const
{
    return 0x0197;
}

ULONG XclExpChTr0x0197::GetLen() const
{
    return 2;
}

//___________________________________________________________________

XclExpChTrEmpty::~XclExpChTrEmpty()
{
}

UINT16 XclExpChTrEmpty::GetNum() const
{
    return nRecNum;
}

ULONG XclExpChTrEmpty::GetLen() const
{
    return 0;
}

//___________________________________________________________________

XclExpChTr0x0195::~XclExpChTr0x0195()
{
}

void XclExpChTr0x0195::SaveCont( XclExpStream& rStrm )
{
    rStrm.WriteZeroBytes( 162 );
}

UINT16 XclExpChTr0x0195::GetNum() const
{
    return 0x0195;
}

ULONG XclExpChTr0x0195::GetLen() const
{
    return 162;
}

//___________________________________________________________________

XclExpChTr0x0194::~XclExpChTr0x0194()
{
}

void XclExpChTr0x0194::SaveCont( XclExpStream& rStrm )
{
    rStrm << (sal_uInt32) 0;
    lcl_WriteDateTime( rStrm, aDateTime );
    rStrm << (sal_uInt8) 0;
    lcl_WriteFixedString( rStrm, sUsername, 147 );
}

UINT16 XclExpChTr0x0194::GetNum() const
{
    return 0x0194;
}

ULONG XclExpChTr0x0194::GetLen() const
{
    return 162;
}

//___________________________________________________________________

XclExpChTrHeader::~XclExpChTrHeader()
{
}

void XclExpChTrHeader::SaveCont( XclExpStream& rStrm )
{
    rStrm   << (sal_uInt16) 0x0006
            << (sal_uInt16) 0x0000
            << (sal_uInt16) 0x000D;
    lcl_WriteGUID( rStrm, aGUID );
    lcl_WriteGUID( rStrm, aGUID );
    rStrm   << nCount
            << (sal_uInt16) 0x0001
            << (sal_uInt32) 0x00000000
            << (sal_uInt16) 0x001E;
}

UINT16 XclExpChTrHeader::GetNum() const
{
    return 0x0196;
}

ULONG XclExpChTrHeader::GetLen() const
{
    return 50;
}

//___________________________________________________________________

XclExpChTrInfo::~XclExpChTrInfo()
{
}

void XclExpChTrInfo::SaveCont( XclExpStream& rStrm )
{
    rStrm   << (sal_uInt32) 0xFFFFFFFF
            << (sal_uInt32) 0x00000000
            << (sal_uInt32) 0x00000020
            << (sal_uInt16) 0xFFFF;
    lcl_WriteGUID( rStrm, aGUID );
    rStrm   << (sal_uInt16) 0x04B0;
    lcl_WriteFixedString( rStrm, sUsername, 113 );
    lcl_WriteDateTime( rStrm, aDateTime );
    rStrm   << (sal_uInt8)  0x0000
            << (sal_uInt16) 0x0002;
}

UINT16 XclExpChTrInfo::GetNum() const
{
    return 0x0138;
}

ULONG XclExpChTrInfo::GetLen() const
{
    return 158;
}

//___________________________________________________________________

XclExpChTrTabIdBuffer::XclExpChTrTabIdBuffer( sal_uInt16 nCount ) :
    nBufSize( nCount ),
    nLastId( nCount )
{
    pBuffer = new sal_uInt16[ nBufSize ];
    memset( pBuffer, 0, sizeof(sal_uInt16) * nBufSize );
    pLast = pBuffer + nBufSize - 1;
}

XclExpChTrTabIdBuffer::XclExpChTrTabIdBuffer( const XclExpChTrTabIdBuffer& rCopy ) :
    nBufSize( rCopy.nBufSize ),
    nLastId( rCopy.nLastId )
{
    pBuffer = new sal_uInt16[ nBufSize ];
    memcpy( pBuffer, rCopy.pBuffer, sizeof(sal_uInt16) * nBufSize );
    pLast = pBuffer + nBufSize - 1;
}

XclExpChTrTabIdBuffer::~XclExpChTrTabIdBuffer()
{
    delete[] pBuffer;
}

void XclExpChTrTabIdBuffer::InitFill( sal_uInt16 nIndex )
{
    DBG_ASSERT( nIndex < nLastId, "XclExpChTrTabIdBuffer::Insert - out of range" );

    sal_uInt16 nFreeCount = 0;
    for( sal_uInt16* pElem = pBuffer; pElem <= pLast; pElem++ )
    {
        if( !*pElem )
            nFreeCount++;
        if( nFreeCount > nIndex )
        {
            *pElem = nLastId--;
            return;
        }
    }
}

void XclExpChTrTabIdBuffer::InitFillup()
{
    sal_uInt16 nFreeCount = 1;
    for( sal_uInt16* pElem = pBuffer; pElem <= pLast; pElem++ )
        if( !*pElem )
            *pElem = nFreeCount++;
    nLastId = nBufSize;
}

sal_uInt16 XclExpChTrTabIdBuffer::GetId( sal_uInt16 nIndex ) const
{
    DBG_ASSERT( nIndex < nBufSize, "XclExpChTrTabIdBuffer::GetId - out of range" );
    return pBuffer[ nIndex ];
}

void XclExpChTrTabIdBuffer::Remove()
{
    DBG_ASSERT( pBuffer <= pLast, "XclExpChTrTabIdBuffer::Remove - buffer empty" );
    sal_uInt16* pElem = pBuffer;
    while( (pElem <= pLast) && (*pElem != nLastId) )
        pElem++;
    while( pElem < pLast )
    {
        *pElem = *(pElem + 1);
        pElem++;
    }
    pLast--;
    nLastId--;
}

//___________________________________________________________________

XclExpChTrTabIdBufferList::~XclExpChTrTabIdBufferList()
{
    for( XclExpChTrTabIdBuffer* pBuffer = First(); pBuffer; pBuffer = Next() )
        delete pBuffer;
}

//___________________________________________________________________

XclExpChTrTabId::XclExpChTrTabId( const XclExpChTrTabIdBuffer& rBuffer ) :
    nTabCount( rBuffer.GetBufferCount() )
{
    pBuffer = new sal_uInt16[ nTabCount ];
    rBuffer.GetBufferCopy( pBuffer );
}

XclExpChTrTabId::~XclExpChTrTabId()
{
    Clear();
}

void XclExpChTrTabId::Copy( const XclExpChTrTabIdBuffer& rBuffer )
{
    Clear();
    nTabCount = rBuffer.GetBufferCount();
    pBuffer = new sal_uInt16[ nTabCount ];
    rBuffer.GetBufferCopy( pBuffer );
}

void XclExpChTrTabId::SaveCont( XclExpStream& rStrm )
{
    if( pBuffer )
        for( sal_uInt16* pElem = pBuffer; pElem < (pBuffer + nTabCount); pElem++ )
            rStrm << *pElem;
    else
        for( sal_uInt16 nIndex = 1; nIndex <= nTabCount; nIndex++ )
            rStrm << nIndex;
}

UINT16 XclExpChTrTabId::GetNum() const
{
    return 0x013D;
}

ULONG XclExpChTrTabId::GetLen() const
{
    return nTabCount << 1;
}

//___________________________________________________________________

// ! does not copy additional actions
XclExpChTrAction::XclExpChTrAction( const XclExpChTrAction& rCopy ) :
    sUsername( rCopy.sUsername ),
    aDateTime( rCopy.aDateTime ),
    nIndex( 0 ),
    rTabBuffer( rCopy.rTabBuffer ),
    pAddAction( NULL ),
    rIdBuffer( rCopy.rIdBuffer ),
    nLength( rCopy.nLength ),
    nOpCode( rCopy.nOpCode )
{
}

XclExpChTrAction::XclExpChTrAction(
        const ScChangeAction& rAction,
        const RootData& rRootData,
        const XclExpChTrTabIdBuffer& rTabIdBuffer,
        sal_uInt16 nNewOpCode ) :
    sUsername( rAction.GetUser() ),
    aDateTime( rAction.GetDateTime() ),
    nIndex( 0 ),
    pAddAction( NULL ),
    bAccepted( rAction.IsAccepted() ),
    rTabBuffer( *rRootData.pTabBuffer ),
    rIdBuffer( rTabIdBuffer ),
    nLength( 0 ),
    nOpCode( nNewOpCode ),
    bForceInfo( sal_False )
{
    aDateTime.SetSec( 0 );
    aDateTime.Set100Sec( 0 );
}

XclExpChTrAction::~XclExpChTrAction()
{
    if( pAddAction )
        delete pAddAction;
}

void XclExpChTrAction::SetAddAction( XclExpChTrAction* pAction )
{
    if( pAddAction )
        pAddAction->SetAddAction( pAction );
    else
        pAddAction = pAction;
}

void XclExpChTrAction::AddDependentContents(
        const ScChangeAction& rAction,
        RootData& rRootData,
        ScChangeTrack& rChangeTrack )
{
    ScChangeActionTable aActionTable;
    rChangeTrack.GetDependents( (ScChangeAction*)(&rAction), aActionTable );
    for( const ScChangeAction* pDepAction = aActionTable.First(); pDepAction; pDepAction = aActionTable.Next() )
        if( pDepAction->GetType() == SC_CAT_CONTENT )
            SetAddAction( new XclExpChTrCellContent(
                *((const ScChangeActionContent*) pDepAction), rRootData, rIdBuffer ) );
}

void XclExpChTrAction::SetIndex( sal_uInt32& rIndex )
{
    nIndex = rIndex++;
}

void XclExpChTrAction::SaveCont( XclExpStream& rStrm )
{
    DBG_ASSERT( nOpCode != EXC_CHTR_OP_UNKNOWN, "XclExpChTrAction::SaveCont - unknown action" );
    rStrm   << nLength
            << nIndex
            << nOpCode
            << (sal_uInt16)(bAccepted ? EXC_CHTR_ACCEPT : EXC_CHTR_NOTHING);
    SaveActionData( rStrm );
}

void XclExpChTrAction::PrepareSaveAction( XclExpStream& rStrm ) const
{
}

void XclExpChTrAction::CompleteSaveAction( XclExpStream& rStrm ) const
{
}

void XclExpChTrAction::Save( XclExpStream& rStrm )
{
    PrepareSaveAction( rStrm );
    ExcRecord::Save( rStrm );
    if( pAddAction )
        pAddAction->Save( rStrm );
    CompleteSaveAction( rStrm );
}

ULONG XclExpChTrAction::GetLen() const
{
    return GetHeaderByteCount() + GetActionByteCount();
}

//___________________________________________________________________

void XclExpChTrData::Clear()
{
    if( pString )
        delete pString;
    if( pUPN )
        delete pUPN;
    if( pRefList )
        delete pRefList;

    pString = NULL;
    pUPN = NULL;
    pRefList = NULL;
    fValue = 0.0;
    nRKValue = 0;
    nType = EXC_CHTR_TYPE_EMPTY;
    nSize = 0;
}

void XclExpChTrData::WriteFormula(
        XclExpStream& rStrm,
        const RootData& rRootData,
        const XclExpChTrTabIdBuffer& rTabIdBuffer )
{
    DBG_ASSERT( pUPN && pUPN->GetData(), "XclExpChTrData::Write - no formula" );
    sal_uInt16 nFmlSize = pUPN->GetLen();
    rStrm << nFmlSize;
    rStrm.Write( pUPN->GetData(), nFmlSize );

    sal_uInt32 nIndex = 0;
    while( nIndex < pRefList->Count() )
    {
        sal_uInt16 nExcFirst = pRefList->Get( nIndex++ );
        sal_uInt16 nExcLast = pRefList->Get( nIndex++ );
        const XclExpUniString* pDocName = rRootData.pExternsheetRecs->GetDocumentName( nExcFirst );
        const XclExpUniString* pTabName = rRootData.pExternsheetRecs->GetTableName( nExcFirst );
        if( pDocName && pTabName )
        {
            pDocName->Write( rStrm );       // normal unicode string
            rStrm << (sal_uInt8) 0x01;
            pTabName->Write( rStrm );       // normal unicode string
            rStrm << (sal_uInt8) 0x02;
        }
        else
        {
            rStrm.SetSliceLen( (nExcFirst == nExcLast) ? 6 : 8 );
            rStrm << (sal_uInt8) 0x01;
            rStrm << (sal_uInt8) 0x02;
            rStrm << (sal_uInt8) 0x00;
            rStrm << rTabIdBuffer.GetId( nExcFirst );
            if( nExcFirst == nExcLast )
                rStrm << (sal_uInt8) 0x02;
            else
            {
                rStrm << (sal_uInt8) 0x00;
                rStrm << rTabIdBuffer.GetId( nExcLast );
            }
        }
    }
    rStrm.SetSliceLen( 0 );
    rStrm << (sal_uInt8) 0x00;
}

void XclExpChTrData::Write(
        XclExpStream& rStrm,
        const RootData& rRootData,
        const XclExpChTrTabIdBuffer& rTabIdBuffer )
{
    switch( nType )
    {
        case EXC_CHTR_TYPE_RK:
            rStrm << nRKValue;
        break;
        case EXC_CHTR_TYPE_DOUBLE:
            rStrm << fValue;
        break;
        case EXC_CHTR_TYPE_STRING:
            DBG_ASSERT( pString, "XclExpChTrData::Write - no string" );
            pString->Write( rStrm );    // normal unicode string
        break;
        case EXC_CHTR_TYPE_FORMULA:
            WriteFormula( rStrm, rRootData, rTabIdBuffer );
        break;
    }
}

//___________________________________________________________________

XclExpChTrCellContent::XclExpChTrCellContent(
        const ScChangeActionContent& rAction,
        RootData& rRootData,
        const XclExpChTrTabIdBuffer& rTabIdBuffer ) :
    XclExpChTrAction( rAction, rRootData, rTabIdBuffer, EXC_CHTR_OP_CELL ),
    ExcRoot( &rRootData ),
    aPosition( rAction.GetBigRange().MakeRange().aStart ),
    pOldData( NULL ),
    pNewData( NULL )
{
    sal_uInt32 nDummy32;
    sal_uInt16 nDummy16;
    GetCellData( rAction.GetOldCell(), pOldData, nDummy32, nOldLength );
    GetCellData( rAction.GetNewCell(), pNewData, nLength, nDummy16 );
}

XclExpChTrCellContent::~XclExpChTrCellContent()
{
    if( pOldData )
        delete pOldData;
    if( pNewData )
        delete pNewData;
}

void XclExpChTrCellContent::MakeEmptyChTrData( XclExpChTrData*& rpData )
{
    if( rpData )
        rpData->Clear();
    else
        rpData = new XclExpChTrData;
}

void XclExpChTrCellContent::GetCellData(
        const ScBaseCell* pScCell,
        XclExpChTrData*& rpData,
        sal_uInt32& rXclLength1,
        sal_uInt16& rXclLength2 )
{
    MakeEmptyChTrData( rpData );
    rXclLength1 = 0x0000003A;
    rXclLength2 = 0x0000;

    if( !pScCell )
    {
        delete rpData;
        rpData = NULL;
        return;
    }

    switch( pScCell->GetCellType() )
    {
        case CELLTYPE_VALUE:
        {
            rpData->fValue = ((const ScValueCell*) pScCell)->GetValue();
            if( XclExpHelper::GetRKFromDouble( rpData->fValue, rpData->nRKValue ) )
            {
                rpData->nType = EXC_CHTR_TYPE_RK;
                rpData->nSize = 4;
                rXclLength1 = 0x0000003E;
                rXclLength2 = 0x0004;
            }
            else
            {
                rpData->nType = EXC_CHTR_TYPE_DOUBLE;
                rpData->nSize = 8;
                rXclLength1 = 0x00000042;
                rXclLength2 = 0x0008;
            }
        }
        break;
        case CELLTYPE_STRING:
        case CELLTYPE_EDIT:
        {
            String sCellStr;
            if( pScCell->GetCellType() == CELLTYPE_STRING )
                ((const ScStringCell*) pScCell)->GetString( sCellStr );
            else
                ((const ScEditCell*) pScCell)->GetString( sCellStr );
            rpData->pString = new XclExpUniString( sCellStr, 32766 );
            rpData->nType = EXC_CHTR_TYPE_STRING;
            rpData->nSize = 3 + (sal_uInt16) rpData->pString->GetByteCount();
            rXclLength1 = 64 + (sCellStr.Len() << 1);
            rXclLength2 = 6 + (sal_uInt16)(sCellStr.Len() << 1);
        }
        break;
        case CELLTYPE_FORMULA:
        {
            const ScFormulaCell* pFmlCell = (const ScFormulaCell*) pScCell;
            const ScTokenArray* pTokenArray = pFmlCell->GetCode();
            if( pTokenArray )
            {
                pExcRoot->pTabBuffer->StartRefLog();
                EC_Codetype eDummy;
                rpData->pUPN = new ExcUPN( pExcRoot, *pTokenArray, eDummy, &pFmlCell->aPos );
                pExcRoot->pTabBuffer->EndRefLog();
                rpData->pRefList = new UINT16List( pExcRoot->pTabBuffer->GetRefLog() );
                rpData->nType = EXC_CHTR_TYPE_FORMULA;
                sal_uInt32 nSize = rpData->pUPN->GetLen() + 3;

                sal_uInt32 nIndex = 0;
                while( nIndex < rpData->pRefList->Count() )
                {
                    sal_uInt16 nExcFirst = rpData->pRefList->Get( nIndex++ );
                    sal_uInt16 nExcLast = rpData->pRefList->Get( nIndex++ );
                    const XclExpUniString* pDocName = pExcRoot->pExternsheetRecs->GetDocumentName( nExcFirst );
                    const XclExpUniString* pTabName = pExcRoot->pExternsheetRecs->GetTableName( nExcFirst );
                    if( pDocName && pTabName )
                        nSize += pDocName->GetByteCount() + pTabName->GetByteCount() + 2;
                    else
                        nSize += (nExcFirst == nExcLast) ? 6 : 8;
                }
                rpData->nSize = (sal_uInt16) Min( nSize, (sal_uInt32) 0x0000FFFF );
                rXclLength1 = 0x00000052;
                rXclLength2 = 0x0018;
            }
        }
        break;
    }
}

void XclExpChTrCellContent::SaveActionData( XclExpStream& rStrm ) const
{
    WriteTabId( rStrm, aPosition.Tab() );
    rStrm   << (sal_uInt16)((pOldData ? (pOldData->nType << 3) : 0x0000) | (pNewData ? pNewData->nType : 0x0000))
            << (sal_uInt16) 0x0000;
    Write2DAddress( rStrm, aPosition );
    rStrm   << nOldLength
            << (sal_uInt32) 0x00000000;
    if( pOldData )
        pOldData->Write( rStrm, *pExcRoot, rIdBuffer );
    if( pNewData )
        pNewData->Write( rStrm, *pExcRoot, rIdBuffer );
}

UINT16 XclExpChTrCellContent::GetNum() const
{
    return 0x013B;
}

ULONG XclExpChTrCellContent::GetActionByteCount() const
{
    ULONG nLen = 16;
    if( pOldData )
        nLen += pOldData->nSize;
    if( pNewData )
        nLen += pNewData->nSize;
    return nLen;
}

//___________________________________________________________________

XclExpChTrInsert::XclExpChTrInsert(
        const ScChangeAction& rAction,
        RootData& rRootData,
        const XclExpChTrTabIdBuffer& rTabIdBuffer,
        ScChangeTrack& rChangeTrack ) :
    XclExpChTrAction( rAction, rRootData, rTabIdBuffer ),
    aRange( rAction.GetBigRange().MakeRange() )
{
    nLength = 0x00000030;
    switch( rAction.GetType() )
    {
        case SC_CAT_INSERT_COLS:    nOpCode = EXC_CHTR_OP_INSCOL;   break;
        case SC_CAT_INSERT_ROWS:    nOpCode = EXC_CHTR_OP_INSROW;   break;
        case SC_CAT_DELETE_COLS:    nOpCode = EXC_CHTR_OP_DELCOL;   break;
        case SC_CAT_DELETE_ROWS:    nOpCode = EXC_CHTR_OP_DELROW;   break;
        default:
            DBG_ERROR( "XclExpChTrInsert::XclExpChTrInsert - unknown action" );
    }

    if( nOpCode & EXC_CHTR_OP_COLFLAG )
    {
        aRange.aStart.SetRow( 0 );
        aRange.aEnd.SetRow( rRootData.nRowMax );
    }
    else
    {
        aRange.aStart.SetCol( 0 );
        aRange.aEnd.SetCol( rRootData.nColMax );
    }

    if( nOpCode & EXC_CHTR_OP_DELFLAG )
    {
        SetAddAction( new XclExpChTr0x014A( *this ) );
        AddDependentContents( rAction, rRootData, rChangeTrack );
    }
}

XclExpChTrInsert::~XclExpChTrInsert()
{
}

void XclExpChTrInsert::SaveActionData( XclExpStream& rStrm ) const
{
    WriteTabId( rStrm, aRange.aStart.Tab() );
    rStrm   << (sal_uInt16) 0x0000;
    Write2DRange( rStrm, aRange );
    rStrm   << (sal_uInt32) 0x00000000;
}

void XclExpChTrInsert::PrepareSaveAction( XclExpStream& rStrm ) const
{
    if( (nOpCode == EXC_CHTR_OP_DELROW) || (nOpCode == EXC_CHTR_OP_DELCOL) )
        XclExpChTrEmpty( 0x0150 ).Save( rStrm );
}

void XclExpChTrInsert::CompleteSaveAction( XclExpStream& rStrm ) const
{
    if( (nOpCode == EXC_CHTR_OP_DELROW) || (nOpCode == EXC_CHTR_OP_DELCOL) )
        XclExpChTrEmpty( 0x0151 ).Save( rStrm );
}

UINT16 XclExpChTrInsert::GetNum() const
{
    return 0x0137;
}

ULONG XclExpChTrInsert::GetActionByteCount() const
{
    return 16;
}

//___________________________________________________________________

XclExpChTrInsertTab::XclExpChTrInsertTab(
        const ScChangeAction& rAction,
        RootData& rRootData,
        const XclExpChTrTabIdBuffer& rTabIdBuffer ) :
    XclExpChTrAction( rAction, rRootData, rTabIdBuffer, EXC_CHTR_OP_INSTAB ),
    ExcRoot( &rRootData ),
    nTab( (sal_uInt16) rAction.GetBigRange().aStart.Tab() )
{
    nLength = 0x0000021C;
    bForceInfo = sal_True;
}

XclExpChTrInsertTab::~XclExpChTrInsertTab()
{
}

void XclExpChTrInsertTab::SaveActionData( XclExpStream& rStrm ) const
{
    WriteTabId( rStrm, nTab );
    rStrm   << (sal_uInt32) 0x00000000;
    String sTabName;
    pExcRoot->pDoc->GetName( nTab, sTabName );
    lcl_WriteFixedString( rStrm, XclExpUniString( sTabName ), 127 );
    lcl_WriteDateTime( rStrm, GetDateTime() );
    rStrm.WriteZeroBytes( 133 );
}

UINT16 XclExpChTrInsertTab::GetNum() const
{
    return 0x014D;
}

ULONG XclExpChTrInsertTab::GetActionByteCount() const
{
    return 276;
}

//___________________________________________________________________

XclExpChTrMoveRange::XclExpChTrMoveRange(
        const ScChangeActionMove& rAction,
        RootData& rRootData,
        const XclExpChTrTabIdBuffer& rTabIdBuffer,
        ScChangeTrack& rChangeTrack ) :
    XclExpChTrAction( rAction, rRootData, rTabIdBuffer, EXC_CHTR_OP_MOVE ),
    aDestRange( rAction.GetBigRange().MakeRange() )
{
    nLength = 0x00000042;
    aSourceRange = aDestRange;
    sal_Int32 nDCols, nDRows, nDTabs;
    rAction.GetDelta( nDCols, nDRows, nDTabs );
    aSourceRange.aStart.IncRow( (USHORT) -nDRows );
    aSourceRange.aStart.IncCol( (USHORT) -nDCols );
    aSourceRange.aStart.IncTab( (USHORT) -nDTabs );
    aSourceRange.aEnd.IncRow( (USHORT) -nDRows );
    aSourceRange.aEnd.IncCol( (USHORT) -nDCols );
    aSourceRange.aEnd.IncTab( (USHORT) -nDTabs );
    AddDependentContents( rAction, rRootData, rChangeTrack );
}

XclExpChTrMoveRange::~XclExpChTrMoveRange()
{
}

void XclExpChTrMoveRange::SaveActionData( XclExpStream& rStrm ) const
{
    WriteTabId( rStrm, aDestRange.aStart.Tab() );
    Write2DRange( rStrm, aSourceRange );
    Write2DRange( rStrm, aDestRange );
    WriteTabId( rStrm, aSourceRange.aStart.Tab() );
    rStrm << (sal_uInt32) 0x00000000;
}

void XclExpChTrMoveRange::PrepareSaveAction( XclExpStream& rStrm ) const
{
    XclExpChTrEmpty( 0x014E ).Save( rStrm );
}

void XclExpChTrMoveRange::CompleteSaveAction( XclExpStream& rStrm ) const
{
    XclExpChTrEmpty( 0x014F ).Save( rStrm );
}

UINT16 XclExpChTrMoveRange::GetNum() const
{
    return 0x0140;
}

ULONG XclExpChTrMoveRange::GetActionByteCount() const
{
    return 24;
}

//___________________________________________________________________

XclExpChTr0x014A::XclExpChTr0x014A( const XclExpChTrInsert& rAction ) :
    XclExpChTrInsert( rAction )
{
    nLength = 0x00000026;
    nOpCode = EXC_CHTR_OP_FORMAT;
}

XclExpChTr0x014A::~XclExpChTr0x014A()
{
}

void XclExpChTr0x014A::SaveActionData( XclExpStream& rStrm ) const
{
    WriteTabId( rStrm, aRange.aStart.Tab() );
    rStrm   << (sal_uInt16) 0x0003
            << (sal_uInt16) 0x0001;
    Write2DRange( rStrm, aRange );
}

UINT16 XclExpChTr0x014A::GetNum() const
{
    return 0x014A;
}

ULONG XclExpChTr0x014A::GetActionByteCount() const
{
    return 14;
}

//___________________________________________________________________

XclExpChTrActionStack::~XclExpChTrActionStack()
{
    while( XclExpChTrAction* pRec = Pop() )
        delete pRec;
}

void XclExpChTrActionStack::Push( XclExpChTrAction* pNewRec )
{
    DBG_ASSERT( pNewRec, "XclExpChTrActionStack::Push - NULL pointer" );
    if( pNewRec )
        Stack::Push( pNewRec );
}

//___________________________________________________________________

XclExpChTrRecordList::~XclExpChTrRecordList()
{
    for( ExcRecord* pRec = First(); pRec; pRec = Next() )
        delete pRec;
}

void XclExpChTrRecordList::Append( ExcRecord* pNewRec )
{
    DBG_ASSERT( pNewRec, "XclExpChTrRecordList::Append - NULL pointer" );
    if( pNewRec )
        List::Insert( pNewRec, LIST_APPEND );
}

void XclExpChTrRecordList::Save( XclExpStream& rStrm )
{
    for( ExcRecord* pRec = First(); pRec; pRec = Next() )
        pRec->Save( rStrm );
}

//___________________________________________________________________

XclExpChangeTrack::XclExpChangeTrack( RootData* pRootData ) :
    ExcRoot( pRootData ),
    aRecList(),
    aActionStack(),
    aTabIdBufferList(),
    pTabIdBuffer( NULL ),
    pTempDoc( NULL ),
    pTempChangeTrack( NULL ),
    nNewAction( 1 ),
    pHeader( NULL ),
    bValidGUID( sal_False )
{
    DBG_ASSERT( pExcRoot && pExcRoot->pTabBuffer && pExcRoot->pExternsheetRecs && pExcRoot->pTabId,
        "XclExpChangeTrack::XclExpChangeTrack - root data incomplete" );
    if( !pExcRoot || !pExcRoot->pTabBuffer || !pExcRoot->pExternsheetRecs || !pExcRoot->pTabId )
        return;

    if( !CreateTempChangeTrack() )
        return;

    pTabIdBuffer = new XclExpChTrTabIdBuffer( pExcRoot->pTabBuffer->GetExcTabCount() );
    aTabIdBufferList.Append( pTabIdBuffer );

    // calculate final table order (tab id list)
    const ScChangeAction* pScAction;
    for( pScAction = pTempChangeTrack->GetLast(); pScAction; pScAction = pScAction->GetPrev() )
    {
        if( pScAction->GetType() == SC_CAT_INSERT_TABS )
        {
            sal_uInt16 nScTab = (sal_uInt16) pScAction->GetBigRange().aStart.Tab();
            pTabIdBuffer->InitFill( pExcRoot->pTabBuffer->GetExcTable( nScTab ) );
        }
    }
    pTabIdBuffer->InitFillup();
    pExcRoot->pTabId->Copy( *pTabIdBuffer );

    // get actions in reverse order
    pScAction = pTempChangeTrack->GetLast();
    while( pScAction )
    {
        PushActionRecord( *pScAction );
        const ScChangeAction* pPrevAction = pScAction->GetPrev();
        pTempChangeTrack->Undo( pScAction->GetActionNumber(), pScAction->GetActionNumber() );
        pScAction = pPrevAction;
    }

    // build record list
    pHeader = new XclExpChTrHeader;
    aRecList.Append( pHeader );
    aRecList.Append( new XclExpChTr0x0195 );
    aRecList.Append( new XclExpChTr0x0194( *pTempChangeTrack ) );

    String sLastUsername;
    DateTime aLastDateTime;
    sal_uInt32 nIndex = 1;
    while( XclExpChTrAction* pAction = aActionStack.Pop() )
    {
        if( (nIndex == 1) || pAction->ForceInfoRecord() ||
            (pAction->GetUsername() != sLastUsername) ||
            (pAction->GetDateTime() != aLastDateTime) )
        {
            lcl_GenerateGUID( aGUID, bValidGUID );
            sLastUsername = pAction->GetUsername();
            aLastDateTime = pAction->GetDateTime();
            aRecList.Append( new XclExpChTrInfo( sLastUsername, aLastDateTime, aGUID ) );
            aRecList.Append( new XclExpChTrTabId( pAction->GetTabIdBuffer() ) );
            pHeader->SetGUID( aGUID );
        }
        pAction->SetIndex( nIndex );
        aRecList.Append( pAction );
    }

    pHeader->SetGUID( aGUID );
    pHeader->SetCount( nIndex - 1 );
    aRecList.Append( new ExcEof );
}

XclExpChangeTrack::~XclExpChangeTrack()
{
    if( pTempChangeTrack )
        delete pTempChangeTrack;
    if( pTempDoc )
        delete pTempDoc;
}

sal_Bool XclExpChangeTrack::CreateTempChangeTrack()
{
    // get original change track
    ScChangeTrack* pOrigChangeTrack = pExcRoot->pDoc->GetChangeTrack();
    DBG_ASSERT( pOrigChangeTrack, "XclExpChangeTrack::CreateTempChangeTrack - no change track data" );
    if( !pOrigChangeTrack )
        return sal_False;

    // create empty document
    pTempDoc = new ScDocument;
    DBG_ASSERT( pTempDoc, "XclExpChangeTrack::CreateTempChangeTrack - no temp document" );
    if( !pTempDoc )
        return sal_False;

    // adjust table count
    USHORT nOrigCount = pExcRoot->pDoc->GetTableCount();
    String sTabName;
    for( sal_Int32 nIndex = 0; nIndex < nOrigCount; nIndex++ )
    {
        pTempDoc->CreateValidTabName( sTabName );
        pTempDoc->InsertTab( SC_TAB_APPEND, sTabName );
    }
    DBG_ASSERT( nOrigCount == pTempDoc->GetTableCount(),
        "XclExpChangeTrack::CreateTempChangeTrack - table count mismatch" );
    if( nOrigCount != pTempDoc->GetTableCount() )
        return sal_False;

    // create empty change track
    pTempChangeTrack = new ScChangeTrack( pTempDoc );
    DBG_ASSERT( pTempChangeTrack, "XclExpChangeTrack::CreateTempChangeTrack - no temp change track" );
    if( !pTempChangeTrack )
        return sal_False;

    // copy original change track
    SvMemoryStream aMemStrm;
    if( !pOrigChangeTrack->Store( aMemStrm ) )
        return sal_False;
    aMemStrm.Seek( STREAM_SEEK_TO_BEGIN );
    if( !pTempChangeTrack->Load( aMemStrm, (USHORT) pTempDoc->GetSrcVersion() ) )
        return sal_False;

    return sal_True;
}

void XclExpChangeTrack::PushActionRecord( const ScChangeAction& rAction )
{
    XclExpChTrAction* pXclAction = NULL;
    switch( rAction.GetType() )
    {
        case SC_CAT_CONTENT:
            pXclAction = new XclExpChTrCellContent( (const ScChangeActionContent&) rAction, *pExcRoot, *pTabIdBuffer );
        break;
        case SC_CAT_INSERT_ROWS:
        case SC_CAT_INSERT_COLS:
        case SC_CAT_DELETE_ROWS:
        case SC_CAT_DELETE_COLS:
            pXclAction = new XclExpChTrInsert( rAction, *pExcRoot, *pTabIdBuffer, *pTempChangeTrack );
        break;
        case SC_CAT_INSERT_TABS:
        {
            pXclAction = new XclExpChTrInsertTab( rAction, *pExcRoot, *pTabIdBuffer );
            XclExpChTrTabIdBuffer* pNewBuffer = new XclExpChTrTabIdBuffer( *pTabIdBuffer );
            pNewBuffer->Remove();
            aTabIdBufferList.Append( pNewBuffer );
            pTabIdBuffer = pNewBuffer;
        }
        break;
        case SC_CAT_MOVE:
            pXclAction = new XclExpChTrMoveRange( (const ScChangeActionMove&) rAction, *pExcRoot, *pTabIdBuffer, *pTempChangeTrack );
        break;
    }
    if( pXclAction )
        aActionStack.Push( pXclAction );
}

sal_Bool XclExpChangeTrack::WriteUserNamesStream()
{
    sal_Bool bRet = sal_False;
    String sStreamName( RTL_CONSTASCII_STRINGPARAM( pUserNamesStreamName ) );
    SvStream* pSvStrm = pExcRoot->pRootStorage->OpenStream( sStreamName, STREAM_READWRITE | STREAM_TRUNC );
    DBG_ASSERT( pSvStrm, "XclExpChangeTrack::WriteUserNamesStream - no stream" );
    if( pSvStrm )
    {
        XclExpStream* pXclStrm = new XclExpStream( *pSvStrm, EXC_MAXRECLEN_BIFF8 );
        DBG_ASSERT( pXclStrm, "XclExpChangeTrack::WriteUserNamesStream - no stream" );
        if( pXclStrm )
        {
            { XclExpChTr0x0191().Save( *pXclStrm ); }
            { XclExpChTr0x0198().Save( *pXclStrm ); }
            { XclExpChTr0x0192().Save( *pXclStrm ); }
            { XclExpChTr0x0197().Save( *pXclStrm ); }
            delete pXclStrm;
        }
        delete pSvStrm;
        bRet = sal_True;
    }
    return bRet;
}

void XclExpChangeTrack::Write()
{
    if( !aRecList.Count() )
        return;

    if( WriteUserNamesStream() )
    {
        String sStreamName( RTL_CONSTASCII_STRINGPARAM( pRevLogStreamName ) );
        SvStorageStreamRef xSvStrm = pExcRoot->pRootStorage->OpenStream( sStreamName, STREAM_READWRITE | STREAM_TRUNC );
        DBG_ASSERT( xSvStrm.Is(), "XclExpChangeTrack::Write - no stream" );
        if( xSvStrm.Is() )
        {
            XclExpStream aXclStrm( *xSvStrm, EXC_MAXRECLEN_BIFF8 + 8 );
            aRecList.Save( aXclStrm );
            xSvStrm->Commit();
        }
    }
}


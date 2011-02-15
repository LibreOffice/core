/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


//___________________________________________________________________

#include <sot/storage.hxx>
#include "XclExpChangeTrack.hxx"
#include "xeformula.hxx"
#include "cell.hxx"
#include "xcl97rec.hxx"

//___________________________________________________________________
// local functions

void lcl_WriteDateTime( XclExpStream& rStrm, const DateTime& rDateTime )
{
    rStrm.SetSliceSize( 7 );
    rStrm   << (sal_uInt16) rDateTime.GetYear()
            << (sal_uInt8)  rDateTime.GetMonth()
            << (sal_uInt8)  rDateTime.GetDay()
            << (sal_uInt8)  rDateTime.GetHour()
            << (sal_uInt8)  rDateTime.GetMin()
            << (sal_uInt8)  rDateTime.GetSec();
    rStrm.SetSliceSize( 0 );
}

// write string and fill rest of <nLength> with zero bytes
// <nLength> is without string header
void lcl_WriteFixedString( XclExpStream& rStrm, const XclExpString& rString, sal_Size nLength )
{
    sal_Size nStrBytes = rString.GetBufferSize();
    DBG_ASSERT( nLength >= nStrBytes, "lcl_WriteFixedString - String too long" );
    if( rString.Len() > 0 )
        rStrm << rString;
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
    rStrm.SetSliceSize( 16 );
    for( sal_Size nIndex = 0; nIndex < 16; nIndex++ )
        rStrm << pGUID[ nIndex ];
    rStrm.SetSliceSize( 0 );
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
    if( sUsername.Len() > 0 )
        rStrm << sUsername;
}

sal_uInt16 XclExpUserBView::GetNum() const
{
    return 0x01A9;
}

sal_Size XclExpUserBView::GetLen() const
{
    return 50 + ((sUsername.Len() > 0) ? sUsername.GetSize() : 0);
}

//___________________________________________________________________

XclExpUserBViewList::XclExpUserBViewList( const ScChangeTrack& rChangeTrack )
{
    sal_uInt8 aGUID[ 16 ];
    sal_Bool bValidGUID = sal_False;
    const ScStrCollection& rStrColl = rChangeTrack.GetUserCollection();
    for( sal_uInt16 nIndex = 0; nIndex < rStrColl.GetCount(); nIndex++ )
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

sal_uInt16 XclExpUsersViewBegin::GetNum() const
{
    return 0x01AA;
}

sal_Size XclExpUsersViewBegin::GetLen() const
{
    return 64;
}

//___________________________________________________________________

void XclExpUsersViewEnd::SaveCont( XclExpStream& rStrm )
{
    rStrm << (sal_uInt16) 0x0001;
}

sal_uInt16 XclExpUsersViewEnd::GetNum() const
{
    return 0x01AB;
}

sal_Size XclExpUsersViewEnd::GetLen() const
{
    return 2;
}

//___________________________________________________________________

void XclExpChTr0x0191::SaveCont( XclExpStream& rStrm )
{
    rStrm << (sal_uInt16) 0x0000;
}

sal_uInt16 XclExpChTr0x0191::GetNum() const
{
    return 0x0191;
}

sal_Size XclExpChTr0x0191::GetLen() const
{
    return 2;
}

//___________________________________________________________________

void XclExpChTr0x0198::SaveCont( XclExpStream& rStrm )
{
    rStrm   << (sal_uInt16) 0x0006
            << (sal_uInt16) 0x0000;
}

sal_uInt16 XclExpChTr0x0198::GetNum() const
{
    return 0x0198;
}

sal_Size XclExpChTr0x0198::GetLen() const
{
    return 4;
}

//___________________________________________________________________

void XclExpChTr0x0192::SaveCont( XclExpStream& rStrm )
{
    rStrm << sal_uInt16( 0x0022 );
    rStrm.WriteZeroBytes( 510 );
}

sal_uInt16 XclExpChTr0x0192::GetNum() const
{
    return 0x0192;
}

sal_Size XclExpChTr0x0192::GetLen() const
{
    return 512;
}

//___________________________________________________________________

void XclExpChTr0x0197::SaveCont( XclExpStream& rStrm )
{
    rStrm << (sal_uInt16) 0x0000;
}

sal_uInt16 XclExpChTr0x0197::GetNum() const
{
    return 0x0197;
}

sal_Size XclExpChTr0x0197::GetLen() const
{
    return 2;
}

//___________________________________________________________________

XclExpChTrEmpty::~XclExpChTrEmpty()
{
}

sal_uInt16 XclExpChTrEmpty::GetNum() const
{
    return nRecNum;
}

sal_Size XclExpChTrEmpty::GetLen() const
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

sal_uInt16 XclExpChTr0x0195::GetNum() const
{
    return 0x0195;
}

sal_Size XclExpChTr0x0195::GetLen() const
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

sal_uInt16 XclExpChTr0x0194::GetNum() const
{
    return 0x0194;
}

sal_Size XclExpChTr0x0194::GetLen() const
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

sal_uInt16 XclExpChTrHeader::GetNum() const
{
    return 0x0196;
}

sal_Size XclExpChTrHeader::GetLen() const
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

sal_uInt16 XclExpChTrInfo::GetNum() const
{
    return 0x0138;
}

sal_Size XclExpChTrInfo::GetLen() const
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
    rStrm.EnableEncryption();
    if( pBuffer )
        for( sal_uInt16* pElem = pBuffer; pElem < (pBuffer + nTabCount); pElem++ )
            rStrm << *pElem;
    else
        for( sal_uInt16 nIndex = 1; nIndex <= nTabCount; nIndex++ )
            rStrm << nIndex;
}

sal_uInt16 XclExpChTrTabId::GetNum() const
{
    return 0x013D;
}

sal_Size XclExpChTrTabId::GetLen() const
{
    return nTabCount << 1;
}

//___________________________________________________________________

// ! does not copy additional actions
XclExpChTrAction::XclExpChTrAction( const XclExpChTrAction& rCopy ) :
    ExcRecord( rCopy ),
    sUsername( rCopy.sUsername ),
    aDateTime( rCopy.aDateTime ),
    nIndex( 0 ),
    pAddAction( 0 ),
    bAccepted( rCopy.bAccepted ),
    rTabInfo( rCopy.rTabInfo ),
    rIdBuffer( rCopy.rIdBuffer ),
    nLength( rCopy.nLength ),
    nOpCode( rCopy.nOpCode ),
    bForceInfo( rCopy.bForceInfo )
{
}

XclExpChTrAction::XclExpChTrAction(
        const ScChangeAction& rAction,
        const XclExpRoot& rRoot,
        const XclExpChTrTabIdBuffer& rTabIdBuffer,
        sal_uInt16 nNewOpCode ) :
    sUsername( rAction.GetUser() ),
    aDateTime( rAction.GetDateTime() ),
    nIndex( 0 ),
    pAddAction( NULL ),
    bAccepted( rAction.IsAccepted() ),
    rTabInfo( rRoot.GetTabInfo() ),
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
        const XclExpRoot& rRoot,
        ScChangeTrack& rChangeTrack )
{
    ScChangeActionTable aActionTable;
    rChangeTrack.GetDependents( (ScChangeAction*)(&rAction), aActionTable );
    for( const ScChangeAction* pDepAction = aActionTable.First(); pDepAction; pDepAction = aActionTable.Next() )
        if( pDepAction->GetType() == SC_CAT_CONTENT )
            SetAddAction( new XclExpChTrCellContent(
                *((const ScChangeActionContent*) pDepAction), rRoot, rIdBuffer ) );
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

void XclExpChTrAction::PrepareSaveAction( XclExpStream& /*rStrm*/ ) const
{
}

void XclExpChTrAction::CompleteSaveAction( XclExpStream& /*rStrm*/ ) const
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

sal_Size XclExpChTrAction::GetLen() const
{
    return GetHeaderByteCount() + GetActionByteCount();
}

//___________________________________________________________________

XclExpChTrData::XclExpChTrData() :
    pString( NULL ),
    fValue( 0.0 ),
    nRKValue( 0 ),
    nType( EXC_CHTR_TYPE_EMPTY ),
    nSize( 0 )
{
}

XclExpChTrData::~XclExpChTrData()
{
    Clear();
}

void XclExpChTrData::Clear()
{
    DELETEZ( pString );
    mxTokArr.reset();
    maRefLog.clear();
    fValue = 0.0;
    nRKValue = 0;
    nType = EXC_CHTR_TYPE_EMPTY;
    nSize = 0;
}

void XclExpChTrData::WriteFormula( XclExpStream& rStrm, const XclExpChTrTabIdBuffer& rTabIdBuffer )
{
    DBG_ASSERT( mxTokArr.is() && !mxTokArr->Empty(), "XclExpChTrData::Write - no formula" );
    rStrm << *mxTokArr;

    for( XclExpRefLog::const_iterator aIt = maRefLog.begin(), aEnd = maRefLog.end(); aIt != aEnd; ++aIt )
    {
        if( aIt->mpUrl && aIt->mpFirstTab )
        {
            rStrm << *aIt->mpUrl << (sal_uInt8) 0x01 << *aIt->mpFirstTab << (sal_uInt8) 0x02;
        }
        else
        {
            bool bSingleTab = aIt->mnFirstXclTab == aIt->mnLastXclTab;
            rStrm.SetSliceSize( bSingleTab ? 6 : 8 );
            rStrm << (sal_uInt8) 0x01 << (sal_uInt8) 0x02 << (sal_uInt8) 0x00;
            rStrm << rTabIdBuffer.GetId( aIt->mnFirstXclTab );
            if( bSingleTab )
                rStrm << (sal_uInt8) 0x02;
            else
                rStrm << (sal_uInt8) 0x00 << rTabIdBuffer.GetId( aIt->mnLastXclTab );
        }
    }
    rStrm.SetSliceSize( 0 );
    rStrm << (sal_uInt8) 0x00;
}

void XclExpChTrData::Write( XclExpStream& rStrm, const XclExpChTrTabIdBuffer& rTabIdBuffer )
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
            rStrm << *pString;
        break;
        case EXC_CHTR_TYPE_FORMULA:
            WriteFormula( rStrm, rTabIdBuffer );
        break;
    }
}

//___________________________________________________________________

XclExpChTrCellContent::XclExpChTrCellContent(
        const ScChangeActionContent& rAction,
        const XclExpRoot& rRoot,
        const XclExpChTrTabIdBuffer& rTabIdBuffer ) :
    XclExpChTrAction( rAction, rRoot, rTabIdBuffer, EXC_CHTR_OP_CELL ),
    XclExpRoot( rRoot ),
    pOldData( 0 ),
    pNewData( 0 ),
    aPosition( rAction.GetBigRange().MakeRange().aStart )
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
            if( XclTools::GetRKFromDouble( rpData->nRKValue, rpData->fValue ) )
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
            rpData->pString = new XclExpString( sCellStr, EXC_STR_DEFAULT, 32766 );
            rpData->nType = EXC_CHTR_TYPE_STRING;
            rpData->nSize = 3 + rpData->pString->GetSize();
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
                XclExpRefLog& rRefLog = rpData->maRefLog;
                rpData->mxTokArr = GetFormulaCompiler().CreateFormula(
                    EXC_FMLATYPE_CELL, *pTokenArray, &pFmlCell->aPos, &rRefLog );
                rpData->nType = EXC_CHTR_TYPE_FORMULA;
                sal_Size nSize = rpData->mxTokArr->GetSize() + 3;

                for( XclExpRefLog::const_iterator aIt = rRefLog.begin(), aEnd = rRefLog.end(); aIt != aEnd; ++aIt )
                {
                    if( aIt->mpUrl && aIt->mpFirstTab )
                        nSize += aIt->mpUrl->GetSize() + aIt->mpFirstTab->GetSize() + 2;
                    else
                        nSize += (aIt->mnFirstXclTab == aIt->mnLastXclTab) ? 6 : 8;
                }
                rpData->nSize = ::std::min< sal_Size >( nSize, 0xFFFF );
                rXclLength1 = 0x00000052;
                rXclLength2 = 0x0018;
            }
        }
        break;
        default:;
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
        pOldData->Write( rStrm, rIdBuffer );
    if( pNewData )
        pNewData->Write( rStrm, rIdBuffer );
}

sal_uInt16 XclExpChTrCellContent::GetNum() const
{
    return 0x013B;
}

sal_Size XclExpChTrCellContent::GetActionByteCount() const
{
    sal_Size nLen = 16;
    if( pOldData )
        nLen += pOldData->nSize;
    if( pNewData )
        nLen += pNewData->nSize;
    return nLen;
}

//___________________________________________________________________

XclExpChTrInsert::XclExpChTrInsert(
        const ScChangeAction& rAction,
        const XclExpRoot& rRoot,
        const XclExpChTrTabIdBuffer& rTabIdBuffer,
        ScChangeTrack& rChangeTrack ) :
    XclExpChTrAction( rAction, rRoot, rTabIdBuffer ),
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
        aRange.aEnd.SetRow( rRoot.GetXclMaxPos().Row() );
    }
    else
    {
        aRange.aStart.SetCol( 0 );
        aRange.aEnd.SetCol( rRoot.GetXclMaxPos().Col() );
    }

    if( nOpCode & EXC_CHTR_OP_DELFLAG )
    {
        SetAddAction( new XclExpChTr0x014A( *this ) );
        AddDependentContents( rAction, rRoot, rChangeTrack );
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

sal_uInt16 XclExpChTrInsert::GetNum() const
{
    return 0x0137;
}

sal_Size XclExpChTrInsert::GetActionByteCount() const
{
    return 16;
}

//___________________________________________________________________

XclExpChTrInsertTab::XclExpChTrInsertTab(
        const ScChangeAction& rAction,
        const XclExpRoot& rRoot,
        const XclExpChTrTabIdBuffer& rTabIdBuffer ) :
    XclExpChTrAction( rAction, rRoot, rTabIdBuffer, EXC_CHTR_OP_INSTAB ),
    XclExpRoot( rRoot ),
    nTab( (SCTAB) rAction.GetBigRange().aStart.Tab() )
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
    rStrm << sal_uInt32( 0 );
    lcl_WriteFixedString( rStrm, XclExpString( GetTabInfo().GetScTabName( nTab ) ), 127 );
    lcl_WriteDateTime( rStrm, GetDateTime() );
    rStrm.WriteZeroBytes( 133 );
}

sal_uInt16 XclExpChTrInsertTab::GetNum() const
{
    return 0x014D;
}

sal_Size XclExpChTrInsertTab::GetActionByteCount() const
{
    return 276;
}

//___________________________________________________________________

XclExpChTrMoveRange::XclExpChTrMoveRange(
        const ScChangeActionMove& rAction,
        const XclExpRoot& rRoot,
        const XclExpChTrTabIdBuffer& rTabIdBuffer,
        ScChangeTrack& rChangeTrack ) :
    XclExpChTrAction( rAction, rRoot, rTabIdBuffer, EXC_CHTR_OP_MOVE ),
    aDestRange( rAction.GetBigRange().MakeRange() )
{
    nLength = 0x00000042;
    aSourceRange = aDestRange;
    sal_Int32 nDCols, nDRows, nDTabs;
    rAction.GetDelta( nDCols, nDRows, nDTabs );
    aSourceRange.aStart.IncRow( (SCROW) -nDRows );
    aSourceRange.aStart.IncCol( (SCCOL) -nDCols );
    aSourceRange.aStart.IncTab( (SCTAB) -nDTabs );
    aSourceRange.aEnd.IncRow( (SCROW) -nDRows );
    aSourceRange.aEnd.IncCol( (SCCOL) -nDCols );
    aSourceRange.aEnd.IncTab( (SCTAB) -nDTabs );
    AddDependentContents( rAction, rRoot, rChangeTrack );
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

sal_uInt16 XclExpChTrMoveRange::GetNum() const
{
    return 0x0140;
}

sal_Size XclExpChTrMoveRange::GetActionByteCount() const
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

sal_uInt16 XclExpChTr0x014A::GetNum() const
{
    return 0x014A;
}

sal_Size XclExpChTr0x014A::GetActionByteCount() const
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

XclExpChangeTrack::XclExpChangeTrack( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    aRecList(),
    aActionStack(),
    aTabIdBufferList(),
    pTabIdBuffer( NULL ),
    pTempDoc( NULL ),
    nNewAction( 1 ),
    pHeader( NULL ),
    bValidGUID( sal_False )
{
    DBG_ASSERT( GetOldRoot().pTabId, "XclExpChangeTrack::XclExpChangeTrack - root data incomplete" );
    if( !GetOldRoot().pTabId )
        return;

    ScChangeTrack* pTempChangeTrack = CreateTempChangeTrack();
    if (!pTempChangeTrack)
        return;

    pTabIdBuffer = new XclExpChTrTabIdBuffer( GetTabInfo().GetXclTabCount() );
    aTabIdBufferList.Append( pTabIdBuffer );

    // calculate final table order (tab id list)
    const ScChangeAction* pScAction;
    for( pScAction = pTempChangeTrack->GetLast(); pScAction; pScAction = pScAction->GetPrev() )
    {
        if( pScAction->GetType() == SC_CAT_INSERT_TABS )
        {
            SCTAB nScTab = static_cast< SCTAB >( pScAction->GetBigRange().aStart.Tab() );
            pTabIdBuffer->InitFill( GetTabInfo().GetXclTab( nScTab ) );
        }
    }
    pTabIdBuffer->InitFillup();
    GetOldRoot().pTabId->Copy( *pTabIdBuffer );

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
    if( pTempDoc )
        delete pTempDoc;
}

ScChangeTrack* XclExpChangeTrack::CreateTempChangeTrack()
{
    // get original change track
    ScChangeTrack* pOrigChangeTrack = GetDoc().GetChangeTrack();
    DBG_ASSERT( pOrigChangeTrack, "XclExpChangeTrack::CreateTempChangeTrack - no change track data" );
    if( !pOrigChangeTrack )
        return NULL;

    // create empty document
    pTempDoc = new ScDocument;
    DBG_ASSERT( pTempDoc, "XclExpChangeTrack::CreateTempChangeTrack - no temp document" );
    if( !pTempDoc )
        return NULL;

    // adjust table count
    SCTAB nOrigCount = GetDoc().GetTableCount();
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

    return pOrigChangeTrack->Clone(pTempDoc);
}

void XclExpChangeTrack::PushActionRecord( const ScChangeAction& rAction )
{
    XclExpChTrAction* pXclAction = NULL;
    ScChangeTrack* pTempChangeTrack = pTempDoc->GetChangeTrack();
    switch( rAction.GetType() )
    {
        case SC_CAT_CONTENT:
            pXclAction = new XclExpChTrCellContent( (const ScChangeActionContent&) rAction, GetRoot(), *pTabIdBuffer );
        break;
        case SC_CAT_INSERT_ROWS:
        case SC_CAT_INSERT_COLS:
        case SC_CAT_DELETE_ROWS:
        case SC_CAT_DELETE_COLS:
            if (pTempChangeTrack)
                pXclAction = new XclExpChTrInsert( rAction, GetRoot(), *pTabIdBuffer, *pTempChangeTrack );
        break;
        case SC_CAT_INSERT_TABS:
        {
            pXclAction = new XclExpChTrInsertTab( rAction, GetRoot(), *pTabIdBuffer );
            XclExpChTrTabIdBuffer* pNewBuffer = new XclExpChTrTabIdBuffer( *pTabIdBuffer );
            pNewBuffer->Remove();
            aTabIdBufferList.Append( pNewBuffer );
            pTabIdBuffer = pNewBuffer;
        }
        break;
        case SC_CAT_MOVE:
            if (pTempChangeTrack)
                pXclAction = new XclExpChTrMoveRange( (const ScChangeActionMove&) rAction, GetRoot(), *pTabIdBuffer, *pTempChangeTrack );
        break;
        default:;
    }
    if( pXclAction )
        aActionStack.Push( pXclAction );
}

sal_Bool XclExpChangeTrack::WriteUserNamesStream()
{
    sal_Bool bRet = sal_False;
    SotStorageStreamRef xSvStrm = OpenStream( EXC_STREAM_USERNAMES );
    DBG_ASSERT( xSvStrm.Is(), "XclExpChangeTrack::WriteUserNamesStream - no stream" );
    if( xSvStrm.Is() )
    {
        XclExpStream aXclStrm( *xSvStrm, GetRoot() );
        XclExpChTr0x0191().Save( aXclStrm );
        XclExpChTr0x0198().Save( aXclStrm );
        XclExpChTr0x0192().Save( aXclStrm );
        XclExpChTr0x0197().Save( aXclStrm );
        xSvStrm->Commit();
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
        SotStorageStreamRef xSvStrm = OpenStream( EXC_STREAM_REVLOG );
        DBG_ASSERT( xSvStrm.Is(), "XclExpChangeTrack::Write - no stream" );
        if( xSvStrm.Is() )
        {
            XclExpStream aXclStrm( *xSvStrm, GetRoot(), EXC_MAXRECSIZE_BIFF8 + 8 );
            aRecList.Save( aXclStrm );
            xSvStrm->Commit();
        }
    }
}


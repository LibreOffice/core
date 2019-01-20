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

#include <memory>
#include <numeric>
#include <stdio.h>
#include <sot/storage.hxx>
#include <XclExpChangeTrack.hxx>
#include <xeformula.hxx>
#include <xehelper.hxx>
#include <xltools.hxx>
#include <formulacell.hxx>
#include <document.hxx>
#include <editutil.hxx>
#include <root.hxx>

#include <oox/export/utils.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <rtl/uuid.h>
#include <svl/sharedstring.hxx>

using namespace oox;

static OString lcl_GuidToOString( sal_uInt8 aGuid[ 16 ] )
{
    char sBuf[ 40 ];
    snprintf( sBuf, sizeof( sBuf ),
            "{%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            aGuid[ 0 ], aGuid[ 1 ], aGuid[ 2 ], aGuid[ 3 ], aGuid[ 4 ], aGuid[ 5 ], aGuid[ 6 ], aGuid[ 7 ],
            aGuid[ 8 ], aGuid[ 9 ], aGuid[ 10 ], aGuid[ 11 ], aGuid[ 12 ], aGuid[ 13 ], aGuid[ 14 ], aGuid[ 15 ] );
    return OString( sBuf );
}

static OString lcl_DateTimeToOString( const DateTime& rDateTime )
{
    char sBuf[ 200 ];
    snprintf( sBuf, sizeof( sBuf ),
            "%d-%02d-%02dT%02d:%02d:%02d.%09" SAL_PRIuUINT32 "Z",
            rDateTime.GetYear(), rDateTime.GetMonth(), rDateTime.GetDay(),
            rDateTime.GetHour(), rDateTime.GetMin(), rDateTime.GetSec(),
            rDateTime.GetNanoSec() );
    return OString( sBuf );
}

// local functions

static void lcl_WriteDateTime( XclExpStream& rStrm, const DateTime& rDateTime )
{
    rStrm.SetSliceSize( 7 );
    rStrm   << static_cast<sal_uInt16>(rDateTime.GetYear())
            << static_cast<sal_uInt8>(rDateTime.GetMonth())
            << static_cast<sal_uInt8>(rDateTime.GetDay())
            << static_cast<sal_uInt8>(rDateTime.GetHour())
            << static_cast<sal_uInt8>(rDateTime.GetMin())
            << static_cast<sal_uInt8>(rDateTime.GetSec());
    rStrm.SetSliceSize( 0 );
}

// write string and fill rest of <nLength> with zero bytes
// <nLength> is without string header
static void lcl_WriteFixedString( XclExpStream& rStrm, const XclExpString& rString, std::size_t nLength )
{
    std::size_t nStrBytes = rString.GetBufferSize();
    OSL_ENSURE( nLength >= nStrBytes, "lcl_WriteFixedString - String too long" );
    if( rString.Len() > 0 )
        rStrm << rString;
    if( nLength > nStrBytes )
        rStrm.WriteZeroBytes( nLength - nStrBytes );
}

static void lcl_GenerateGUID( sal_uInt8* pGUID, bool& rValidGUID )
{
    rtl_createUuid( pGUID, rValidGUID ? pGUID : nullptr, false );
    rValidGUID = true;
}

static void lcl_WriteGUID( XclExpStream& rStrm, const sal_uInt8* pGUID )
{
    rStrm.SetSliceSize( 16 );
    for( std::size_t nIndex = 0; nIndex < 16; nIndex++ )
        rStrm << pGUID[ nIndex ];
    rStrm.SetSliceSize( 0 );
}

XclExpUserBView::XclExpUserBView( const OUString& rUsername, const sal_uInt8* pGUID ) :
    sUsername( rUsername )
{
    memcpy( aGUID, pGUID, 16 );
}

void XclExpUserBView::SaveCont( XclExpStream& rStrm )
{
    rStrm   << sal_uInt32(0xFF078014)
            << sal_uInt32(0x00000001);
    lcl_WriteGUID( rStrm, aGUID );
    rStrm.WriteZeroBytes( 8 );
    rStrm   << sal_uInt32(1200)
            << sal_uInt32(1000)
            << sal_uInt16(1000)
            << sal_uInt16(0x0CF7)
            << sal_uInt16(0x0000)
            << sal_uInt16(0x0001)
            << sal_uInt16(0x0000);
    if( sUsername.Len() > 0 )
        rStrm << sUsername;
}

sal_uInt16 XclExpUserBView::GetNum() const
{
    return 0x01A9;
}

std::size_t XclExpUserBView::GetLen() const
{
    return 50 + ((sUsername.Len() > 0) ? sUsername.GetSize() : 0);
}

XclExpUserBViewList::XclExpUserBViewList( const ScChangeTrack& rChangeTrack )
{
    sal_uInt8 aGUID[ 16 ];
    bool bValidGUID = false;
    const std::set<OUString>& rStrColl = rChangeTrack.GetUserCollection();
    aViews.reserve(rStrColl.size());
    for (const auto& rStr : rStrColl)
    {
        lcl_GenerateGUID( aGUID, bValidGUID );
        aViews.emplace_back( rStr, aGUID );
    }
}

XclExpUserBViewList::~XclExpUserBViewList()
{
}

void XclExpUserBViewList::Save( XclExpStream& rStrm )
{
   for( XclExpUserBView& rView : aViews )
       rView.Save( rStrm );
}

XclExpUsersViewBegin::XclExpUsersViewBegin( const sal_uInt8* pGUID, sal_uInt32 nTab ) :
    nCurrTab( nTab )
{
    memcpy( aGUID, pGUID, 16 );
}

void XclExpUsersViewBegin::SaveCont( XclExpStream& rStrm )
{
    lcl_WriteGUID( rStrm, aGUID );
    rStrm   << nCurrTab
            << sal_uInt32(100)
            << sal_uInt32(64)
            << sal_uInt32(3)
            << sal_uInt32(0x0000003C)
            << sal_uInt16(0)
            << sal_uInt16(3)
            << sal_uInt16(0)
            << sal_uInt16(3)
            << double(0)
            << double(0)
            << sal_Int16(-1)
            << sal_Int16(-1);
}

sal_uInt16 XclExpUsersViewBegin::GetNum() const
{
    return 0x01AA;
}

std::size_t XclExpUsersViewBegin::GetLen() const
{
    return 64;
}

void XclExpUsersViewEnd::SaveCont( XclExpStream& rStrm )
{
    rStrm << sal_uInt16(0x0001);
}

sal_uInt16 XclExpUsersViewEnd::GetNum() const
{
    return 0x01AB;
}

std::size_t XclExpUsersViewEnd::GetLen() const
{
    return 2;
}

void XclExpChTr0x0191::SaveCont( XclExpStream& rStrm )
{
    rStrm << sal_uInt16(0x0000);
}

sal_uInt16 XclExpChTr0x0191::GetNum() const
{
    return 0x0191;
}

std::size_t XclExpChTr0x0191::GetLen() const
{
    return 2;
}

void XclExpChTr0x0198::SaveCont( XclExpStream& rStrm )
{
    rStrm   << sal_uInt16(0x0006)
            << sal_uInt16(0x0000);
}

sal_uInt16 XclExpChTr0x0198::GetNum() const
{
    return 0x0198;
}

std::size_t XclExpChTr0x0198::GetLen() const
{
    return 4;
}

void XclExpChTr0x0192::SaveCont( XclExpStream& rStrm )
{
    rStrm << sal_uInt16( 0x0022 );
    rStrm.WriteZeroBytes( 510 );
}

sal_uInt16 XclExpChTr0x0192::GetNum() const
{
    return 0x0192;
}

std::size_t XclExpChTr0x0192::GetLen() const
{
    return 512;
}

void XclExpChTr0x0197::SaveCont( XclExpStream& rStrm )
{
    rStrm << sal_uInt16(0x0000);
}

sal_uInt16 XclExpChTr0x0197::GetNum() const
{
    return 0x0197;
}

std::size_t XclExpChTr0x0197::GetLen() const
{
    return 2;
}

XclExpChTrEmpty::~XclExpChTrEmpty()
{
}

sal_uInt16 XclExpChTrEmpty::GetNum() const
{
    return nRecNum;
}

std::size_t XclExpChTrEmpty::GetLen() const
{
    return 0;
}

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

std::size_t XclExpChTr0x0195::GetLen() const
{
    return 162;
}

XclExpChTr0x0194::~XclExpChTr0x0194()
{
}

void XclExpChTr0x0194::SaveCont( XclExpStream& rStrm )
{
    rStrm << sal_uInt32(0);
    lcl_WriteDateTime( rStrm, aDateTime );
    rStrm << sal_uInt8(0);
    lcl_WriteFixedString( rStrm, sUsername, 147 );
}

sal_uInt16 XclExpChTr0x0194::GetNum() const
{
    return 0x0194;
}

std::size_t XclExpChTr0x0194::GetLen() const
{
    return 162;
}

XclExpChTrHeader::~XclExpChTrHeader()
{
}

void XclExpChTrHeader::SaveCont( XclExpStream& rStrm )
{
    rStrm   << sal_uInt16(0x0006)
            << sal_uInt16(0x0000)
            << sal_uInt16(0x000D);
    lcl_WriteGUID( rStrm, aGUID );
    lcl_WriteGUID( rStrm, aGUID );
    rStrm   << nCount
            << sal_uInt16(0x0001)
            << sal_uInt32(0x00000000)
            << sal_uInt16(0x001E);
}

sal_uInt16 XclExpChTrHeader::GetNum() const
{
    return 0x0196;
}

std::size_t XclExpChTrHeader::GetLen() const
{
    return 50;
}

void XclExpChTrHeader::SaveXml( XclExpXmlStream& rRevisionHeadersStrm )
{
    sax_fastparser::FSHelperPtr pHeaders = rRevisionHeadersStrm.GetCurrentStream();
    rRevisionHeadersStrm.WriteAttributes(
            XML_guid,               lcl_GuidToOString( aGUID ).getStr(),
            XML_lastGuid,           nullptr,   // OOXTODO
            XML_shared,             nullptr,   // OOXTODO
            XML_diskRevisions,      nullptr,   // OOXTODO
            XML_history,            nullptr,   // OOXTODO
            XML_trackRevisions,     nullptr,   // OOXTODO
            XML_exclusive,          nullptr,   // OOXTODO
            XML_revisionId,         nullptr,   // OOXTODO
            XML_version,            nullptr,   // OOXTODO
            XML_keepChangeHistory,  nullptr,   // OOXTODO
            XML_protected,          nullptr,   // OOXTODO
            XML_preserveHistory,    nullptr,   // OOXTODO
            FSEND );
    pHeaders->write( ">" );
}

void XclExpXmlChTrHeaders::SetGUID( const sal_uInt8* pGUID )
{
    memcpy(maGUID, pGUID, 16);
}

void XclExpXmlChTrHeaders::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr pHeaders = rStrm.GetCurrentStream();

    pHeaders->write("<")->writeId(XML_headers);

    rStrm.WriteAttributes(
        XML_xmlns,              XclXmlUtils::ToOString(rStrm.getNamespaceURL(OOX_NS(xls))).getStr(),
        FSNS(XML_xmlns, XML_r), XclXmlUtils::ToOString(rStrm.getNamespaceURL(OOX_NS(officeRel))).getStr(),
        XML_guid,               lcl_GuidToOString(maGUID).getStr(),
        XML_lastGuid,           nullptr,   // OOXTODO
        XML_shared,             nullptr,   // OOXTODO
        XML_diskRevisions,      nullptr,   // OOXTODO
        XML_history,            nullptr,   // OOXTODO
        XML_trackRevisions,     nullptr,   // OOXTODO
        XML_exclusive,          nullptr,   // OOXTODO
        XML_revisionId,         nullptr,   // OOXTODO
        XML_version,            nullptr,   // OOXTODO
        XML_keepChangeHistory,  nullptr,   // OOXTODO
        XML_protected,          nullptr,   // OOXTODO
        XML_preserveHistory,    nullptr,   // OOXTODO
        FSEND);

    pHeaders->write(">");
}

XclExpXmlChTrHeader::XclExpXmlChTrHeader(
    const OUString& rUserName, const DateTime& rDateTime, const sal_uInt8* pGUID,
    sal_Int32 nLogNumber, const XclExpChTrTabIdBuffer& rBuf ) :
    maUserName(rUserName), maDateTime(rDateTime), mnLogNumber(nLogNumber),
    mnMinAction(0), mnMaxAction(0)
{
    memcpy(maGUID, pGUID, 16);
    if (rBuf.GetBufferCount())
    {
        maTabBuffer.resize(rBuf.GetBufferCount());
        rBuf.GetBufferCopy(&maTabBuffer[0]);
    }
}

void XclExpXmlChTrHeader::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr pHeader = rStrm.GetCurrentStream();

    pHeader->write("<")->writeId(XML_header);

    OUString aRelId;
    sax_fastparser::FSHelperPtr pRevLogStrm = rStrm.CreateOutputStream(
            XclXmlUtils::GetStreamName("xl/revisions/", "revisionLog", mnLogNumber),
            XclXmlUtils::GetStreamName(nullptr, "revisionLog", mnLogNumber),
            rStrm.GetCurrentStream()->getOutputStream(),
            "application/vnd.openxmlformats-officedocument.spreadsheetml.revisionLog+xml",
            CREATE_OFFICEDOC_RELATION_TYPE("revisionLog"),
            &aRelId);

    rStrm.WriteAttributes(
        XML_guid, lcl_GuidToOString(maGUID).getStr(),
        XML_dateTime, lcl_DateTimeToOString(maDateTime).getStr(),
        XML_userName, XclXmlUtils::ToOString(maUserName).getStr(),
        FSNS(XML_r, XML_id),  XclXmlUtils::ToOString(aRelId).getStr(),
        FSEND);

    if (mnMinAction)
        rStrm.WriteAttributes(XML_minRId, OString::number(mnMinAction).getStr(), FSEND);

    if (mnMaxAction)
        rStrm.WriteAttributes(XML_maxRId, OString::number(mnMaxAction).getStr(), FSEND);

    if (!maTabBuffer.empty())
        // next available sheet index.
        rStrm.WriteAttributes(XML_maxSheetId, OString::number(maTabBuffer.back()+1).getStr(), FSEND);

    pHeader->write(">");

    if (!maTabBuffer.empty())
    {
        // Write sheet index map.
        size_t n = maTabBuffer.size();
        pHeader->startElement(
            XML_sheetIdMap,
            XML_count, OString::number(n).getStr(),
            FSEND);

        for (size_t i = 0; i < n; ++i)
        {
            pHeader->singleElement(
                XML_sheetId,
                XML_val, OString::number(maTabBuffer[i]).getStr(),
                FSEND);
        }
        pHeader->endElement(XML_sheetIdMap);
    }

    // Write all revision logs in a separate stream.

    rStrm.PushStream(pRevLogStrm);

    pRevLogStrm->write("<")->writeId(XML_revisions);

    rStrm.WriteAttributes(
        XML_xmlns,              XclXmlUtils::ToOString(rStrm.getNamespaceURL(OOX_NS(xls))).getStr(),
        FSNS(XML_xmlns, XML_r), XclXmlUtils::ToOString(rStrm.getNamespaceURL(OOX_NS(officeRel))).getStr(),
        FSEND);

    pRevLogStrm->write(">");

    for (const auto& rxAction : maActions)
    {
        rxAction->SaveXml(rStrm);
    }

    pRevLogStrm->write("</")->writeId(XML_revisions)->write(">");

    rStrm.PopStream();

    pHeader->write("</")->writeId(XML_header)->write(">");
}

void XclExpXmlChTrHeader::AppendAction( std::unique_ptr<XclExpChTrAction> pAction )
{
    sal_uInt32 nActionNum = pAction->GetActionNumber();
    if (!mnMinAction || mnMinAction > nActionNum)
        mnMinAction = nActionNum;

    if (!mnMaxAction || mnMaxAction < nActionNum)
        mnMaxAction = nActionNum;

    maActions.push_back(std::move(pAction));
}

XclExpChTrInfo::XclExpChTrInfo( const OUString& rUsername, const DateTime& rDateTime, const sal_uInt8* pGUID ) :
    sUsername( rUsername ),
    aDateTime( rDateTime )
{
    memcpy( aGUID, pGUID, 16 );
}

XclExpChTrInfo::~XclExpChTrInfo()
{
}

void XclExpChTrInfo::SaveCont( XclExpStream& rStrm )
{
    rStrm   << sal_uInt32(0xFFFFFFFF)
            << sal_uInt32(0x00000000)
            << sal_uInt32(0x00000020)
            << sal_uInt16(0xFFFF);
    lcl_WriteGUID( rStrm, aGUID );
    rStrm   << sal_uInt16(0x04B0);
    lcl_WriteFixedString( rStrm, sUsername, 113 );
    lcl_WriteDateTime( rStrm, aDateTime );
    rStrm   << sal_uInt8(0x0000)
            << sal_uInt16(0x0002);
}

sal_uInt16 XclExpChTrInfo::GetNum() const
{
    return 0x0138;
}

std::size_t XclExpChTrInfo::GetLen() const
{
    return 158;
}

XclExpChTrTabIdBuffer::XclExpChTrTabIdBuffer( sal_uInt16 nCount ) :
    nBufSize( nCount ),
    nLastId( nCount )
{
    pBuffer.reset( new sal_uInt16[ nBufSize ] );
    memset( pBuffer.get(), 0, sizeof(sal_uInt16) * nBufSize );
    pLast = pBuffer.get() + nBufSize - 1;
}

XclExpChTrTabIdBuffer::XclExpChTrTabIdBuffer( const XclExpChTrTabIdBuffer& rCopy ) :
    nBufSize( rCopy.nBufSize ),
    nLastId( rCopy.nLastId )
{
    pBuffer.reset( new sal_uInt16[ nBufSize ] );
    memcpy( pBuffer.get(), rCopy.pBuffer.get(), sizeof(sal_uInt16) * nBufSize );
    pLast = pBuffer.get() + nBufSize - 1;
}

XclExpChTrTabIdBuffer::~XclExpChTrTabIdBuffer()
{
}

void XclExpChTrTabIdBuffer::InitFill( sal_uInt16 nIndex )
{
    OSL_ENSURE( nIndex < nLastId, "XclExpChTrTabIdBuffer::Insert - out of range" );

    sal_uInt16 nFreeCount = 0;
    for( sal_uInt16* pElem = pBuffer.get(); pElem <= pLast; pElem++ )
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
    for( sal_uInt16* pElem = pBuffer.get(); pElem <= pLast; pElem++ )
        if( !*pElem )
            *pElem = nFreeCount++;
    nLastId = nBufSize;
}

sal_uInt16 XclExpChTrTabIdBuffer::GetId( sal_uInt16 nIndex ) const
{
    OSL_ENSURE( nIndex < nBufSize, "XclExpChTrTabIdBuffer::GetId - out of range" );
    return pBuffer[ nIndex ];
}

void XclExpChTrTabIdBuffer::Remove()
{
    OSL_ENSURE( pBuffer.get() <= pLast, "XclExpChTrTabIdBuffer::Remove - buffer empty" );
    sal_uInt16* pElem = pBuffer.get();
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

XclExpChTrTabId::XclExpChTrTabId( const XclExpChTrTabIdBuffer& rBuffer )
    : nTabCount( rBuffer.GetBufferCount() )
{
    pBuffer.reset( new sal_uInt16[ nTabCount ] );
    rBuffer.GetBufferCopy( pBuffer.get() );
}

XclExpChTrTabId::~XclExpChTrTabId()
{
    Clear();
}

void XclExpChTrTabId::Copy( const XclExpChTrTabIdBuffer& rBuffer )
{
    Clear();
    nTabCount = rBuffer.GetBufferCount();
    pBuffer.reset( new sal_uInt16[ nTabCount ] );
    rBuffer.GetBufferCopy( pBuffer.get() );
}

void XclExpChTrTabId::SaveCont( XclExpStream& rStrm )
{
    rStrm.EnableEncryption();
    if( pBuffer )
        rStrm.Write(pBuffer.get(), nTabCount);
    else
        for( sal_uInt16 nIndex = 1; nIndex <= nTabCount; nIndex++ )
            rStrm << nIndex;
}

sal_uInt16 XclExpChTrTabId::GetNum() const
{
    return 0x013D;
}

std::size_t XclExpChTrTabId::GetLen() const
{
    return nTabCount << 1;
}

// ! does not copy additional actions
XclExpChTrAction::XclExpChTrAction( const XclExpChTrAction& rCopy ) :
    ExcRecord( rCopy ),
    sUsername( rCopy.sUsername ),
    aDateTime( rCopy.aDateTime ),
    nIndex( 0 ),
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
    bAccepted( rAction.IsAccepted() ),
    rTabInfo( rRoot.GetTabInfo() ),
    rIdBuffer( rTabIdBuffer ),
    nLength( 0 ),
    nOpCode( nNewOpCode ),
    bForceInfo( false )
{
    aDateTime.SetSec( 0 );
    aDateTime.SetNanoSec( 0 );
}

XclExpChTrAction::~XclExpChTrAction()
{
}

void XclExpChTrAction::SetAddAction( XclExpChTrAction* pAction )
{
    if( pAddAction )
        pAddAction->SetAddAction( pAction );
    else
        pAddAction.reset( pAction );
}

void XclExpChTrAction::AddDependentContents(
        const ScChangeAction& rAction,
        const XclExpRoot& rRoot,
        const ScChangeTrack& rChangeTrack )
{
    ScChangeActionMap aActionMap;

    rChangeTrack.GetDependents( const_cast<ScChangeAction*>(&rAction), aActionMap );
    for( const auto& rEntry : aActionMap )
        if( rEntry.second->GetType() == SC_CAT_CONTENT )
            SetAddAction( new XclExpChTrCellContent(
                *static_cast<const ScChangeActionContent*>(rEntry.second), rRoot, rIdBuffer ) );
}

void XclExpChTrAction::SetIndex( sal_uInt32& rIndex )
{
    nIndex = rIndex++;
}

void XclExpChTrAction::SaveCont( XclExpStream& rStrm )
{
    OSL_ENSURE( nOpCode != EXC_CHTR_OP_UNKNOWN, "XclExpChTrAction::SaveCont - unknown action" );
    rStrm   << nLength
            << nIndex
            << nOpCode
            << static_cast<sal_uInt16>(bAccepted ? EXC_CHTR_ACCEPT : EXC_CHTR_NOTHING);
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

std::size_t XclExpChTrAction::GetLen() const
{
    return GetHeaderByteCount() + GetActionByteCount();
}

XclExpChTrData::XclExpChTrData() :
    mpFormulaCell( nullptr ),
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
    pString.reset();
    mpFormulaCell = nullptr;
    mxTokArr.reset();
    maRefLog.clear();
    fValue = 0.0;
    nRKValue = 0;
    nType = EXC_CHTR_TYPE_EMPTY;
    nSize = 0;
}

void XclExpChTrData::WriteFormula( XclExpStream& rStrm, const XclExpChTrTabIdBuffer& rTabIdBuffer )
{
    OSL_ENSURE( mxTokArr && !mxTokArr->Empty(), "XclExpChTrData::Write - no formula" );
    rStrm << *mxTokArr;

    for( const auto& rLogEntry : maRefLog )
    {
        if( rLogEntry.mpUrl && rLogEntry.mpFirstTab )
        {
            rStrm << *rLogEntry.mpUrl << sal_uInt8(0x01) << *rLogEntry.mpFirstTab << sal_uInt8(0x02);
        }
        else
        {
            bool bSingleTab = rLogEntry.mnFirstXclTab == rLogEntry.mnLastXclTab;
            rStrm.SetSliceSize( bSingleTab ? 6 : 8 );
            rStrm << sal_uInt8(0x01) << sal_uInt8(0x02) << sal_uInt8(0x00);
            rStrm << rTabIdBuffer.GetId( rLogEntry.mnFirstXclTab );
            if( bSingleTab )
                rStrm << sal_uInt8(0x02);
            else
                rStrm << sal_uInt8(0x00) << rTabIdBuffer.GetId( rLogEntry.mnLastXclTab );
        }
    }
    rStrm.SetSliceSize( 0 );
    rStrm << sal_uInt8(0x00);
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
            OSL_ENSURE( pString, "XclExpChTrData::Write - no string" );
            rStrm << *pString;
        break;
        case EXC_CHTR_TYPE_FORMULA:
            WriteFormula( rStrm, rTabIdBuffer );
        break;
    }
}

XclExpChTrCellContent::XclExpChTrCellContent(
        const ScChangeActionContent& rAction,
        const XclExpRoot& rRoot,
        const XclExpChTrTabIdBuffer& rTabIdBuffer ) :
    XclExpChTrAction( rAction, rRoot, rTabIdBuffer, EXC_CHTR_OP_CELL ),
    XclExpRoot( rRoot ),
    aPosition( rAction.GetBigRange().MakeRange().aStart )
{
    sal_uInt32 nDummy32;
    sal_uInt16 nDummy16;
    GetCellData( rRoot, rAction.GetOldCell(), pOldData, nDummy32, nOldLength );
    GetCellData( rRoot, rAction.GetNewCell(), pNewData, nLength, nDummy16 );
}

XclExpChTrCellContent::~XclExpChTrCellContent()
{
    pOldData.reset();
    pNewData.reset();
}

void XclExpChTrCellContent::MakeEmptyChTrData( std::unique_ptr<XclExpChTrData>& rpData )
{
    if( rpData )
        rpData->Clear();
    else
        rpData.reset( new XclExpChTrData );
}

void XclExpChTrCellContent::GetCellData(
    const XclExpRoot& rRoot, const ScCellValue& rScCell,
    std::unique_ptr<XclExpChTrData>& rpData, sal_uInt32& rXclLength1, sal_uInt16& rXclLength2 )
{
    MakeEmptyChTrData( rpData );
    rXclLength1 = 0x0000003A;
    rXclLength2 = 0x0000;

    if (rScCell.isEmpty())
    {
        rpData.reset();
        return;
    }

    switch (rScCell.meType)
    {
        case CELLTYPE_VALUE:
        {
            rpData->fValue = rScCell.mfValue;
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
            OUString sCellStr;
            if (rScCell.meType == CELLTYPE_STRING)
            {
                sCellStr = rScCell.mpString->getString();
                rpData->mpFormattedString = XclExpStringHelper::CreateCellString(
                    rRoot, sCellStr, nullptr);
            }
            else
            {
                XclExpHyperlinkHelper aLinkHelper( rRoot, aPosition );
                if (rScCell.mpEditText)
                {
                    sCellStr = ScEditUtil::GetString(*rScCell.mpEditText, &GetDoc());
                    rpData->mpFormattedString = XclExpStringHelper::CreateCellString(
                        rRoot, *rScCell.mpEditText, nullptr, aLinkHelper);
                }
                else
                {
                    rpData->mpFormattedString = XclExpStringHelper::CreateCellString(
                        rRoot, EMPTY_OUSTRING, nullptr);
                }
            }
            rpData->pString.reset( new XclExpString( sCellStr, XclStrFlags::NONE, 32766 ) );
            rpData->nType = EXC_CHTR_TYPE_STRING;
            rpData->nSize = 3 + rpData->pString->GetSize();
            rXclLength1 = 64 + (sCellStr.getLength() << 1);
            rXclLength2 = 6 + static_cast<sal_uInt16>(sCellStr.getLength() << 1);
        }
        break;
        case CELLTYPE_FORMULA:
        {
            const ScFormulaCell* pFmlCell = rScCell.mpFormula;
            rpData->mpFormulaCell = pFmlCell;

            const ScTokenArray* pTokenArray = pFmlCell->GetCode();
            if( pTokenArray )
            {
                XclExpRefLog& rRefLog = rpData->maRefLog;
                rpData->mxTokArr = GetFormulaCompiler().CreateFormula(
                    EXC_FMLATYPE_CELL, *pTokenArray, &pFmlCell->aPos, &rRefLog );
                rpData->nType = EXC_CHTR_TYPE_FORMULA;
                std::size_t nSize = std::accumulate(rRefLog.begin(), rRefLog.end(),
                    static_cast<std::size_t>(rpData->mxTokArr->GetSize() + 3),
                    [](const std::size_t& rSum, const XclExpRefLogEntry& rLogEntry) {
                        if( rLogEntry.mpUrl && rLogEntry.mpFirstTab )
                            return rSum + rLogEntry.mpUrl->GetSize() + rLogEntry.mpFirstTab->GetSize() + 2;
                        else
                            return rSum + ((rLogEntry.mnFirstXclTab == rLogEntry.mnLastXclTab) ? 6 : 8);
                    });
                rpData->nSize = ::std::min< std::size_t >( nSize, 0xFFFF );
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
    rStrm   << static_cast<sal_uInt16>((pOldData ? (pOldData->nType << 3) : 0x0000) | (pNewData ? pNewData->nType : 0x0000))
            << sal_uInt16(0x0000);
    Write2DAddress( rStrm, aPosition );
    rStrm   << nOldLength
            << sal_uInt32(0x00000000);
    if( pOldData )
        pOldData->Write( rStrm, rIdBuffer );
    if( pNewData )
        pNewData->Write( rStrm, rIdBuffer );
}

sal_uInt16 XclExpChTrCellContent::GetNum() const
{
    return 0x013B;
}

std::size_t XclExpChTrCellContent::GetActionByteCount() const
{
    std::size_t nLen = 16;
    if( pOldData )
        nLen += pOldData->nSize;
    if( pNewData )
        nLen += pNewData->nSize;
    return nLen;
}

static const char* lcl_GetType( XclExpChTrData* pData )
{
    switch( pData->nType )
    {
    case EXC_CHTR_TYPE_RK:
    case EXC_CHTR_TYPE_DOUBLE:
        return "n";
        break;
    case EXC_CHTR_TYPE_FORMULA:
        {
            ScFormulaCell* pFormulaCell = const_cast< ScFormulaCell* >( pData->mpFormulaCell );
            const char* sType;
            OUString sValue;
            XclXmlUtils::GetFormulaTypeAndValue( *pFormulaCell, sType, sValue );
            return sType;
        }
        break;
    case EXC_CHTR_TYPE_STRING:
        return "inlineStr";
        break;
    default:
        break;
    }
    return "*unknown*";
}

static void lcl_WriteCell( XclExpXmlStream& rStrm, sal_Int32 nElement, const ScAddress& rPosition, XclExpChTrData* pData )
{
    sax_fastparser::FSHelperPtr pStream = rStrm.GetCurrentStream();

    pStream->startElement( nElement,
            XML_r,  XclXmlUtils::ToOString( rPosition ).getStr(),
            XML_s,  nullptr,   // OOXTODO: not supported
            XML_t,  lcl_GetType( pData ),
            XML_cm, nullptr,   // OOXTODO: not supported
            XML_vm, nullptr,   // OOXTODO: not supported
            XML_ph, nullptr,   // OOXTODO: not supported
            FSEND );
    switch( pData->nType )
    {
        case EXC_CHTR_TYPE_RK:
        case EXC_CHTR_TYPE_DOUBLE:
            pStream->startElement( XML_v, FSEND );
            pStream->write( pData->fValue );
            pStream->endElement( XML_v );
            break;
        case EXC_CHTR_TYPE_FORMULA:
            pStream->startElement( XML_f,
                    // OOXTODO: other attributes?  see XclExpFormulaCell::SaveXml()
                    FSEND );
            pStream->writeEscaped( XclXmlUtils::ToOUString(
                        rStrm.GetRoot().GetCompileFormulaContext(),
                        pData->mpFormulaCell->aPos, pData->mpFormulaCell->GetCode()));
            pStream->endElement( XML_f );
            break;
        case EXC_CHTR_TYPE_STRING:
            pStream->startElement( XML_is, FSEND );
            if( pData->mpFormattedString )
                pData->mpFormattedString->WriteXml( rStrm );
            else
                pData->pString->WriteXml( rStrm );
            pStream->endElement( XML_is );
            break;
        default:
            // ignore
            break;
    }
    pStream->endElement( nElement );
}

void XclExpChTrCellContent::SaveXml( XclExpXmlStream& rRevisionLogStrm )
{
    sax_fastparser::FSHelperPtr pStream = rRevisionLogStrm.GetCurrentStream();
    pStream->startElement( XML_rcc,
            XML_rId,                    OString::number(  GetActionNumber() ).getStr(),
            XML_ua,                     ToPsz( GetAccepted () ),   // OOXTODO? bAccepted == ua or ra; not sure.
            XML_ra,                     nullptr,       // OOXTODO: RRD.fUndoAction?  Or RRD.fAccepted?
            XML_sId,                    OString::number(  GetTabId( aPosition.Tab() ) ).getStr(),
            XML_odxf,                   nullptr,       // OOXTODO: not supported
            XML_xfDxf,                  nullptr,       // OOXTODO: not supported
            XML_s,                      nullptr,       // OOXTODO: not supported
            XML_dxf,                    nullptr,       // OOXTODO: not supported
            XML_numFmtId,               nullptr,       // OOXTODO: not supported
            XML_quotePrefix,            nullptr,       // OOXTODO: not supported
            XML_oldQuotePrefix,         nullptr,       // OOXTODO: not supported
            XML_ph,                     nullptr,       // OOXTODO: not supported
            XML_oldPh,                  nullptr,       // OOXTODO: not supported
            XML_endOfListFormulaUpdate, nullptr,       // OOXTODO: not supported
            FSEND );
    if( pOldData )
    {
        lcl_WriteCell( rRevisionLogStrm, XML_oc, aPosition, pOldData.get() );
        if (!pNewData)
        {
            pStream->singleElement(XML_nc,
                    XML_r,  XclXmlUtils::ToOString( aPosition ).getStr(),
                    FSEND);
        }
    }
    if( pNewData )
    {
        lcl_WriteCell( rRevisionLogStrm, XML_nc, aPosition, pNewData.get() );
    }
    // OOXTODO: XML_odxf, XML_ndxf, XML_extLst elements
    pStream->endElement( XML_rcc );
}

XclExpChTrInsert::XclExpChTrInsert( const XclExpChTrInsert& rCopy ) :
    XclExpChTrAction(rCopy),
    mbEndOfList(rCopy.mbEndOfList),
    aRange(rCopy.aRange) {}

XclExpChTrInsert::XclExpChTrInsert(
        const ScChangeAction& rAction,
        const XclExpRoot& rRoot,
        const XclExpChTrTabIdBuffer& rTabIdBuffer,
        const ScChangeTrack& rChangeTrack ) :
    XclExpChTrAction( rAction, rRoot, rTabIdBuffer ),
    mbEndOfList(false),
    aRange( rAction.GetBigRange().MakeRange() )
{
    nLength = 0x00000030;
    switch( rAction.GetType() )
    {
        case SC_CAT_INSERT_COLS:    nOpCode = EXC_CHTR_OP_INSCOL;   break;
        case SC_CAT_INSERT_ROWS:
        {
            const ScChangeActionIns& rIns = static_cast<const ScChangeActionIns&>(rAction);
            mbEndOfList = rIns.IsEndOfList();
            nOpCode = EXC_CHTR_OP_INSROW;
        }
        break;
        case SC_CAT_DELETE_COLS:    nOpCode = EXC_CHTR_OP_DELCOL;   break;
        case SC_CAT_DELETE_ROWS:    nOpCode = EXC_CHTR_OP_DELROW;   break;
        default:
            OSL_FAIL( "XclExpChTrInsert::XclExpChTrInsert - unknown action" );
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
    sal_uInt16 nFlagVal = mbEndOfList ? 0x0001 : 0x0000;
    rStrm << nFlagVal;
    Write2DRange( rStrm, aRange );
    rStrm   << sal_uInt32(0x00000000);
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

std::size_t XclExpChTrInsert::GetActionByteCount() const
{
    return 16;
}

static const char* lcl_GetAction( sal_uInt16 nOpCode )
{
    switch( nOpCode )
    {
        case EXC_CHTR_OP_INSCOL:    return "insertCol";
        case EXC_CHTR_OP_INSROW:    return "insertRow";
        case EXC_CHTR_OP_DELCOL:    return "deleteCol";
        case EXC_CHTR_OP_DELROW:    return "deleteRow";
        default:                    return "*unknown*";
    }
}

void XclExpChTrInsert::SaveXml( XclExpXmlStream& rRevisionLogStrm )
{
    sax_fastparser::FSHelperPtr pStream = rRevisionLogStrm.GetCurrentStream();
    pStream->startElement( XML_rrc,
            XML_rId,    OString::number(  GetActionNumber() ).getStr(),
            XML_ua,     ToPsz( GetAccepted () ),   // OOXTODO? bAccepted == ua or ra; not sure.
            XML_ra,     nullptr,       // OOXTODO: RRD.fUndoAction?  Or RRD.fAccepted?
            XML_sId,    OString::number(  GetTabId( aRange.aStart.Tab() ) ).getStr(),
            XML_eol,    ToPsz10(mbEndOfList),
            XML_ref,    XclXmlUtils::ToOString( aRange ).getStr(),
            XML_action, lcl_GetAction( nOpCode ),
            XML_edge,   nullptr,       // OOXTODO: ???
            FSEND );

    // OOXTODO: does this handle XML_rfmt, XML_undo?
    XclExpChTrAction* pAction = GetAddAction();
    while( pAction != nullptr )
    {
        pAction->SaveXml( rRevisionLogStrm );
        pAction = pAction->GetAddAction();
    }
    pStream->endElement( XML_rrc );
}

XclExpChTrInsertTab::XclExpChTrInsertTab(
        const ScChangeAction& rAction,
        const XclExpRoot& rRoot,
        const XclExpChTrTabIdBuffer& rTabIdBuffer ) :
    XclExpChTrAction( rAction, rRoot, rTabIdBuffer, EXC_CHTR_OP_INSTAB ),
    XclExpRoot( rRoot ),
    nTab( static_cast<SCTAB>(rAction.GetBigRange().aStart.Tab()) )
{
    nLength = 0x0000021C;
    bForceInfo = true;
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

std::size_t XclExpChTrInsertTab::GetActionByteCount() const
{
    return 276;
}

void XclExpChTrInsertTab::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr pStream = rStrm.GetCurrentStream();
    pStream->singleElement( XML_ris,
            XML_rId,            OString::number(  GetActionNumber() ).getStr(),
            XML_ua,             ToPsz( GetAccepted () ),   // OOXTODO? bAccepted == ua or ra; not sure.
            XML_ra,             nullptr,       // OOXTODO: RRD.fUndoAction?  Or RRD.fAccepted?
            XML_sheetId,        OString::number(  GetTabId( nTab ) ).getStr(),
            XML_name,           XclXmlUtils::ToOString( GetTabInfo().GetScTabName( nTab ) ).getStr(),
            XML_sheetPosition,  OString::number(  nTab ).getStr(),
            FSEND );
}

XclExpChTrMoveRange::XclExpChTrMoveRange(
        const ScChangeActionMove& rAction,
        const XclExpRoot& rRoot,
        const XclExpChTrTabIdBuffer& rTabIdBuffer,
        const ScChangeTrack& rChangeTrack ) :
    XclExpChTrAction( rAction, rRoot, rTabIdBuffer, EXC_CHTR_OP_MOVE ),
    aDestRange( rAction.GetBigRange().MakeRange() )
{
    nLength = 0x00000042;
    aSourceRange = aDestRange;
    sal_Int32 nDCols, nDRows, nDTabs;
    rAction.GetDelta( nDCols, nDRows, nDTabs );
    aSourceRange.aStart.IncRow( static_cast<SCROW>(-nDRows) );
    aSourceRange.aStart.IncCol( static_cast<SCCOL>(-nDCols) );
    aSourceRange.aStart.IncTab( static_cast<SCTAB>(-nDTabs) );
    aSourceRange.aEnd.IncRow( static_cast<SCROW>(-nDRows) );
    aSourceRange.aEnd.IncCol( static_cast<SCCOL>(-nDCols) );
    aSourceRange.aEnd.IncTab( static_cast<SCTAB>(-nDTabs) );
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
    rStrm << sal_uInt32(0x00000000);
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

std::size_t XclExpChTrMoveRange::GetActionByteCount() const
{
    return 24;
}

void XclExpChTrMoveRange::SaveXml( XclExpXmlStream& rRevisionLogStrm )
{
    sax_fastparser::FSHelperPtr pStream = rRevisionLogStrm.GetCurrentStream();

    pStream->startElement( XML_rm,
            XML_rId,            OString::number(  GetActionNumber() ).getStr(),
            XML_ua,             ToPsz( GetAccepted () ),   // OOXTODO? bAccepted == ua or ra; not sure.
            XML_ra,             nullptr,       // OOXTODO: RRD.fUndoAction?  Or RRD.fAccepted?
            XML_sheetId,        OString::number(  GetTabId( aDestRange.aStart.Tab() ) ).getStr(),
            XML_source,         XclXmlUtils::ToOString( aSourceRange ).getStr(),
            XML_destination,    XclXmlUtils::ToOString( aDestRange ).getStr(),
            XML_sourceSheetId,  OString::number(  GetTabId( aSourceRange.aStart.Tab() ) ).getStr(),
            FSEND );
    // OOXTODO: does this handle XML_rfmt, XML_undo?
    XclExpChTrAction* pAction = GetAddAction();
    while( pAction != nullptr )
    {
        pAction->SaveXml( rRevisionLogStrm );
        pAction = pAction->GetAddAction();
    }
    pStream->endElement( XML_rm );
}

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
    rStrm   << sal_uInt16(0x0003)
            << sal_uInt16(0x0001);
    Write2DRange( rStrm, aRange );
}

sal_uInt16 XclExpChTr0x014A::GetNum() const
{
    return 0x014A;
}

std::size_t XclExpChTr0x014A::GetActionByteCount() const
{
    return 14;
}

void XclExpChTr0x014A::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr pStream = rStrm.GetCurrentStream();

    pStream->startElement( XML_rfmt,
            XML_sheetId,    OString::number(  GetTabId( aRange.aStart.Tab() ) ).getStr(),
            XML_xfDxf,      nullptr,   // OOXTODO: not supported
            XML_s,          nullptr,   // OOXTODO: style
            XML_sqref,      XclXmlUtils::ToOString( aRange ).getStr(),
            XML_start,      nullptr,   // OOXTODO: for string changes
            XML_length,     nullptr,   // OOXTODO: for string changes
            FSEND );
    // OOXTODO: XML_dxf, XML_extLst

    pStream->endElement( XML_rfmt );
}

std::size_t ExcXmlRecord::GetLen() const
{
    return 0;
}

sal_uInt16 ExcXmlRecord::GetNum() const
{
    return 0;
}

void ExcXmlRecord::Save( XclExpStream& )
{
    // Do nothing; ignored for BIFF output.
}

class EndXmlElement : public ExcXmlRecord
{
    sal_Int32           mnElement;
public:
    explicit            EndXmlElement( sal_Int32 nElement ) : mnElement( nElement) {}
    virtual void        SaveXml( XclExpXmlStream& rStrm ) override;
};

void EndXmlElement::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr pStream = rStrm.GetCurrentStream();
    pStream->write("</")->writeId(mnElement)->write(">");
}

XclExpChangeTrack::XclExpChangeTrack( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    aActionStack(),
    pTabIdBuffer( nullptr )
{
    OSL_ENSURE( GetOldRoot().pTabId, "XclExpChangeTrack::XclExpChangeTrack - root data incomplete" );
    if( !GetOldRoot().pTabId )
        return;

    ScChangeTrack* pTempChangeTrack = CreateTempChangeTrack();
    if (!pTempChangeTrack)
        return;

    pTabIdBuffer = new XclExpChTrTabIdBuffer( GetTabInfo().GetXclTabCount() );
    maBuffers.push_back( std::unique_ptr<XclExpChTrTabIdBuffer>(pTabIdBuffer) );

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
        pScAction = pPrevAction;
    }

    // build record list
    if (GetOutput() == EXC_OUTPUT_BINARY)
    {
        XclExpChTrHeader* pHeader = new XclExpChTrHeader; // header record for last GUID
        maRecList.push_back( std::unique_ptr<ExcRecord>(pHeader) );
        maRecList.push_back( std::unique_ptr<ExcRecord>( new XclExpChTr0x0195 ) );
        maRecList.push_back( std::unique_ptr<ExcRecord>( new XclExpChTr0x0194( *pTempChangeTrack ) ) );

        OUString sLastUsername;
        DateTime aLastDateTime( DateTime::EMPTY );
        sal_uInt32 nIndex = 1;
        sal_Int32 nLogNumber = 1;
        sal_uInt8 aGUID[ 16 ]; // GUID for action info records
        bool bValidGUID = false;
        while( !aActionStack.empty() )
        {
            XclExpChTrAction* pAction = aActionStack.top();
            aActionStack.pop();

            if( (nIndex == 1) || pAction->ForceInfoRecord() ||
                (pAction->GetUsername() != sLastUsername) ||
                (pAction->GetDateTime() != aLastDateTime) )
            {
                lcl_GenerateGUID( aGUID, bValidGUID );
                sLastUsername = pAction->GetUsername();
                aLastDateTime = pAction->GetDateTime();

                nLogNumber++;
                maRecList.push_back( std::unique_ptr<ExcRecord>(new XclExpChTrInfo(sLastUsername, aLastDateTime, aGUID)) );
                maRecList.push_back(  std::unique_ptr<ExcRecord>(new XclExpChTrTabId(pAction->GetTabIdBuffer())) );
                pHeader->SetGUID( aGUID );
            }
            pAction->SetIndex( nIndex );
            maRecList.push_back( std::unique_ptr<ExcRecord>(pAction) );
        }

        pHeader->SetGUID( aGUID );
        pHeader->SetCount( nIndex - 1 );
        maRecList.push_back(  std::unique_ptr<ExcRecord>(new ExcEof) );
    }
    else
    {
        XclExpXmlChTrHeaders* pHeaders = new XclExpXmlChTrHeaders;
        maRecList.push_back( std::unique_ptr<ExcRecord>(pHeaders));

        OUString sLastUsername;
        DateTime aLastDateTime(DateTime::EMPTY);
        sal_uInt32 nIndex = 1;
        sal_Int32 nLogNumber = 1;
        XclExpXmlChTrHeader* pCurHeader = nullptr;
        sal_uInt8 aGUID[ 16 ]; // GUID for action info records
        bool bValidGUID = false;

        while (!aActionStack.empty())
        {
            XclExpChTrAction* pAction = aActionStack.top();
            aActionStack.pop();

            if( (nIndex == 1) || pAction->ForceInfoRecord() ||
                (pAction->GetUsername() != sLastUsername) ||
                (pAction->GetDateTime() != aLastDateTime) )
            {
                lcl_GenerateGUID( aGUID, bValidGUID );
                sLastUsername = pAction->GetUsername();
                aLastDateTime = pAction->GetDateTime();

                pCurHeader = new XclExpXmlChTrHeader(sLastUsername, aLastDateTime, aGUID, nLogNumber, pAction->GetTabIdBuffer());
                maRecList.push_back( std::unique_ptr<ExcRecord>(pCurHeader));
                nLogNumber++;
                pHeaders->SetGUID(aGUID);
            }
            pAction->SetIndex(nIndex);
            pCurHeader->AppendAction(std::unique_ptr<XclExpChTrAction>(pAction));
        }

        pHeaders->SetGUID(aGUID);
        maRecList.push_back( std::unique_ptr<ExcRecord>(new EndXmlElement(XML_headers)));
    }
}

XclExpChangeTrack::~XclExpChangeTrack()
{
    while( !aActionStack.empty() )
    {
        delete aActionStack.top();
        aActionStack.pop();
    }
}

ScChangeTrack* XclExpChangeTrack::CreateTempChangeTrack()
{
    // get original change track
    ScChangeTrack* pOrigChangeTrack = GetDoc().GetChangeTrack();
    OSL_ENSURE( pOrigChangeTrack, "XclExpChangeTrack::CreateTempChangeTrack - no change track data" );
    if( !pOrigChangeTrack )
        return nullptr;

    assert(!xTempDoc);
    // create empty document
    xTempDoc.reset(new ScDocument);

    // adjust table count
    SCTAB nOrigCount = GetDoc().GetTableCount();
    OUString sTabName;
    for( sal_Int32 nIndex = 0; nIndex < nOrigCount; nIndex++ )
    {
        xTempDoc->CreateValidTabName(sTabName);
        xTempDoc->InsertTab(SC_TAB_APPEND, sTabName);
    }
    OSL_ENSURE(nOrigCount == xTempDoc->GetTableCount(),
        "XclExpChangeTrack::CreateTempChangeTrack - table count mismatch");
    if(nOrigCount != xTempDoc->GetTableCount())
        return nullptr;

    return pOrigChangeTrack->Clone(xTempDoc.get());
}

void XclExpChangeTrack::PushActionRecord( const ScChangeAction& rAction )
{
    XclExpChTrAction* pXclAction = nullptr;
    ScChangeTrack* pTempChangeTrack = xTempDoc->GetChangeTrack();
    switch( rAction.GetType() )
    {
        case SC_CAT_CONTENT:
            pXclAction = new XclExpChTrCellContent( static_cast<const ScChangeActionContent&>(rAction), GetRoot(), *pTabIdBuffer );
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
            maBuffers.push_back( std::unique_ptr<XclExpChTrTabIdBuffer>(pNewBuffer) );
            pTabIdBuffer = pNewBuffer;
        }
        break;
        case SC_CAT_MOVE:
            if (pTempChangeTrack)
                pXclAction = new XclExpChTrMoveRange( static_cast<const ScChangeActionMove&>(rAction), GetRoot(), *pTabIdBuffer, *pTempChangeTrack );
        break;
        default:;
    }
    if( pXclAction )
        aActionStack.push( pXclAction );
}

bool XclExpChangeTrack::WriteUserNamesStream()
{
    bool bRet = false;
    tools::SvRef<SotStorageStream> xSvStrm = OpenStream( EXC_STREAM_USERNAMES );
    OSL_ENSURE( xSvStrm.is(), "XclExpChangeTrack::WriteUserNamesStream - no stream" );
    if( xSvStrm.is() )
    {
        XclExpStream aXclStrm( *xSvStrm, GetRoot() );
        XclExpChTr0x0191().Save( aXclStrm );
        XclExpChTr0x0198().Save( aXclStrm );
        XclExpChTr0x0192().Save( aXclStrm );
        XclExpChTr0x0197().Save( aXclStrm );
        xSvStrm->Commit();
        bRet = true;
    }
    return bRet;
}

void XclExpChangeTrack::Write()
{
    if (maRecList.empty())
        return;

    if( WriteUserNamesStream() )
    {
        tools::SvRef<SotStorageStream> xSvStrm = OpenStream( EXC_STREAM_REVLOG );
        OSL_ENSURE( xSvStrm.is(), "XclExpChangeTrack::Write - no stream" );
        if( xSvStrm.is() )
        {
            XclExpStream aXclStrm( *xSvStrm, GetRoot(), EXC_MAXRECSIZE_BIFF8 + 8 );

            for(const auto& rxRec : maRecList)
                rxRec->Save(aXclStrm);

            xSvStrm->Commit();
        }
    }
}

static void lcl_WriteUserNamesXml( XclExpXmlStream& rWorkbookStrm )
{
    sax_fastparser::FSHelperPtr pUserNames = rWorkbookStrm.CreateOutputStream(
            "xl/revisions/userNames.xml",
            "revisions/userNames.xml",
            rWorkbookStrm.GetCurrentStream()->getOutputStream(),
            "application/vnd.openxmlformats-officedocument.spreadsheetml.userNames+xml",
            CREATE_OFFICEDOC_RELATION_TYPE("usernames"));
    pUserNames->startElement( XML_users,
            XML_xmlns,                  XclXmlUtils::ToOString(rWorkbookStrm.getNamespaceURL(OOX_NS(xls))).getStr(),
            FSNS( XML_xmlns, XML_r ),   XclXmlUtils::ToOString(rWorkbookStrm.getNamespaceURL(OOX_NS(officeRel))).getStr(),
            XML_count,                  "0",
            FSEND );
    // OOXTODO: XML_userinfo elements for each user editing the file
    //          Doesn't seem to be supported by .xls output either (based on
    //          contents of XclExpChangeTrack::WriteUserNamesStream()).
    pUserNames->endElement( XML_users );
}

void XclExpChangeTrack::WriteXml( XclExpXmlStream& rWorkbookStrm )
{
    if (maRecList.empty())
        return;

    lcl_WriteUserNamesXml( rWorkbookStrm );

    sax_fastparser::FSHelperPtr pRevisionHeaders = rWorkbookStrm.CreateOutputStream(
            "xl/revisions/revisionHeaders.xml",
            "revisions/revisionHeaders.xml",
            rWorkbookStrm.GetCurrentStream()->getOutputStream(),
            "application/vnd.openxmlformats-officedocument.spreadsheetml.revisionHeaders+xml",
            CREATE_OFFICEDOC_RELATION_TYPE("revisionHeaders"));
    // OOXTODO: XML_userinfo elements for each user editing the file
    //          Doesn't seem to be supported by .xls output either (based on
    //          contents of XclExpChangeTrack::WriteUserNamesStream()).
    rWorkbookStrm.PushStream( pRevisionHeaders );

    for (const auto& rxRec : maRecList)
        rxRec->SaveXml(rWorkbookStrm);

    rWorkbookStrm.PopStream();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

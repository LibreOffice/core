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

#include <XclImpChangeTrack.hxx>
#include <sot/storage.hxx>
#include <svl/zforlist.hxx>
#include <svl/sharedstringpool.hxx>
#include <sal/log.hxx>
#include <chgviset.hxx>
#include <formulacell.hxx>
#include <chgtrack.hxx>
#include <xihelper.hxx>
#include <xilink.hxx>
#include <externalrefmgr.hxx>
#include <document.hxx>
#include <excdefs.hxx>

// class XclImpChangeTrack

XclImpChangeTrack::XclImpChangeTrack( const XclImpRoot& rRoot, const XclImpStream& rBookStrm ) :
    XclImpRoot( rRoot ),
    aRecHeader(),
    sOldUsername(),
    nTabIdCount( 0 ),
    bGlobExit( false ),
    eNestedMode( nmBase )
{
    // Verify that the User Names stream exists before going any further. Excel adds both
    // "Revision Log" and "User Names" streams when Change Tracking is active but the Revision log
    // remains if Change Tracking is turned off.
    tools::SvRef<SotStorageStream> xUserStrm = OpenStream( EXC_STREAM_USERNAMES );
    if( !xUserStrm.is() )
        return;

    xInStrm = OpenStream( EXC_STREAM_REVLOG );
    if( xInStrm.is() )
    {
        xInStrm->Seek( STREAM_SEEK_TO_END );
        sal_uInt64 const nStreamLen = xInStrm->Tell();
        if( (xInStrm->GetErrorCode() == ERRCODE_NONE) && (nStreamLen != STREAM_SEEK_TO_END) )
        {
            xInStrm->Seek( STREAM_SEEK_TO_BEGIN );
            pStrm.reset( new XclImpStream( *xInStrm, GetRoot() ) );
            pStrm->CopyDecrypterFrom( rBookStrm );
            pChangeTrack.reset(new ScChangeTrack( &GetDocRef() ));

            sOldUsername = pChangeTrack->GetUser();
            pChangeTrack->SetUseFixDateTime( true );

            ReadRecords();
        }
    }
}

XclImpChangeTrack::~XclImpChangeTrack()
{
    pChangeTrack.reset();
    pStrm.reset();
}

void XclImpChangeTrack::DoAcceptRejectAction( ScChangeAction* pAction )
{
    if( !pAction ) return;
    switch( aRecHeader.nAccept )
    {
        case EXC_CHTR_ACCEPT:
            pChangeTrack->Accept( pAction );
        break;
        case EXC_CHTR_REJECT:
        break;
    }
}

void XclImpChangeTrack::DoAcceptRejectAction( sal_uInt32 nFirst, sal_uInt32 nLast )
{
    for( sal_uInt32 nIndex = nFirst; nIndex <= nLast; nIndex++ )
        DoAcceptRejectAction( pChangeTrack->GetAction( nIndex ) );
}

void XclImpChangeTrack::DoInsertRange( const ScRange& rRange, bool bEndOfList )
{
    sal_uInt32 nFirst = pChangeTrack->GetActionMax() + 1;
    pChangeTrack->AppendInsert(rRange, bEndOfList);
    sal_uInt32 nLast = pChangeTrack->GetActionMax();
    DoAcceptRejectAction( nFirst, nLast );
}

void XclImpChangeTrack::DoDeleteRange( const ScRange& rRange )
{
    sal_uLong nFirst, nLast;
    pChangeTrack->AppendDeleteRange( rRange, nullptr, nFirst, nLast );
    DoAcceptRejectAction( nFirst, nLast );
}

SCTAB XclImpChangeTrack::ReadTabNum()
{
    return static_cast<SCTAB>(GetTabInfo().GetCurrentIndex(
                pStrm->ReaduInt16(), nTabIdCount ));
}

void XclImpChangeTrack::ReadDateTime( DateTime& rDateTime )
{
    sal_uInt16 nYear;
    sal_uInt8 nMonth, nDay, nHour, nMin, nSec;

    nYear = pStrm->ReaduInt16();
    nMonth = pStrm->ReaduInt8();
    nDay = pStrm->ReaduInt8();
    nHour = pStrm->ReaduInt8();
    nMin = pStrm->ReaduInt8();
    nSec = pStrm->ReaduInt8();

    rDateTime.SetYear( nYear );
    rDateTime.SetMonth( nMonth );
    rDateTime.SetDay( nDay );
    rDateTime.SetHour( nHour );
    rDateTime.SetMin( nMin );
    rDateTime.SetSec( nSec );
    rDateTime.SetNanoSec( 0 );
}

bool XclImpChangeTrack::CheckRecord( sal_uInt16 nOpCode )
{
    if( (nOpCode != EXC_CHTR_OP_UNKNOWN) && (aRecHeader.nOpCode != nOpCode) )
    {
        OSL_FAIL( "XclImpChangeTrack::CheckRecord - unknown action" );
        return false;
    }
    return aRecHeader.nIndex != 0;
}

void XclImpChangeTrack::Read3DTabRefInfo( SCTAB& rFirstTab, SCTAB& rLastTab, ExcelToSc8::ExternalTabInfo& rExtInfo )
{
    if( LookAtuInt8() == 0x01 )
    {
        rExtInfo.mbExternal = false;
        // internal ref - read tab num and return sc tab num (position in TABID list)
        pStrm->Ignore( 3 );
        rFirstTab = static_cast< SCTAB >( GetTabInfo().GetCurrentIndex( pStrm->ReaduInt16(), nTabIdCount ) );
        sal_uInt8 nFillByte = pStrm->ReaduInt8();
        rLastTab = (nFillByte == 0x00) ?
            static_cast< SCTAB >( GetTabInfo().GetCurrentIndex( pStrm->ReaduInt16(), nTabIdCount ) ) : rFirstTab;
    }
    else
    {
        // external ref - read doc and tab name and find sc tab num
        // - URL
        OUString aEncUrl( pStrm->ReadUniString() );
        OUString aUrl;
        bool bSelf;
        XclImpUrlHelper::DecodeUrl( aUrl, bSelf, GetRoot(), aEncUrl );
        pStrm->Ignore( 1 );
        // - sheet name, always separated from URL
        OUString aTabName( pStrm->ReadUniString() );
        pStrm->Ignore( 1 );

        rExtInfo.mbExternal = true;
        ScExternalRefManager* pRefMgr = GetDoc().GetExternalRefManager();
        pRefMgr->convertToAbsName(aUrl);
        rExtInfo.mnFileId = pRefMgr->getExternalFileId(aUrl);
        rExtInfo.maTabName = aTabName;
        rFirstTab = rLastTab = 0;
    }
}

void XclImpChangeTrack::ReadFormula( std::unique_ptr<ScTokenArray>& rpTokenArray, const ScAddress& rPosition )
{
    sal_uInt16 nFmlSize = pStrm->ReaduInt16();

    // create a memory stream and copy the formula to be able to read simultaneously
    // the formula and the additional 3D tab ref data following the formula
    // here we have to simulate an Excel record to be able to use an XclImpStream...
    // 2do: remove the stream member from formula converter and add it as a parameter
    // to the Convert() routine (to prevent the construction/destruction of the
    // converter in each formula)
    SvMemoryStream aMemStrm;
    aMemStrm.WriteUInt16( 0x0001 ).WriteUInt16( nFmlSize );
    size_t nRead = pStrm->CopyToStream( aMemStrm, nFmlSize );

    // survive reading invalid streams!
    // if we can't read as many bytes as required just don't use them and
    // assume that this part is broken
    if(nRead != nFmlSize)
    {
        rpTokenArray = nullptr;
        pStrm->Ignore(1);
        return;
    }

    XclImpStream aFmlaStrm( aMemStrm, GetRoot() );
    aFmlaStrm.StartNextRecord();
    XclImpChTrFmlConverter aFmlConv( GetRoot(), *this );

    // read the formula, 3D tab refs from extended data
    std::unique_ptr<ScTokenArray> pArray;
    aFmlConv.Reset( rPosition );
    bool bOK = (aFmlConv.Convert( pArray, aFmlaStrm, nFmlSize, false ) == ConvErr::OK);   // JEG : Check This
    rpTokenArray = (bOK && pArray) ? std::move( pArray ) : nullptr;
    pStrm->Ignore( 1 );
}

void XclImpChangeTrack::ReadCell(
    ScCellValue& rCell, sal_uInt32& rFormat, sal_uInt16 nFlags, const ScAddress& rPosition )
{
    rCell.clear();
    rFormat = 0;
    switch( nFlags & EXC_CHTR_TYPE_MASK )
    {
        case EXC_CHTR_TYPE_EMPTY:
        break;
        case EXC_CHTR_TYPE_RK:
        {
            double fValue = XclTools::GetDoubleFromRK( pStrm->ReadInt32() );
            if( pStrm->IsValid() )
            {
                rCell.meType = CELLTYPE_VALUE;
                rCell.mfValue = fValue;
            }
        }
        break;
        case EXC_CHTR_TYPE_DOUBLE:
        {
            double fValue = pStrm->ReadDouble();
            if( pStrm->IsValid() )
            {
                rCell.meType = CELLTYPE_VALUE;
                rCell.mfValue = fValue;
            }
        }
        break;
        case EXC_CHTR_TYPE_STRING:
        {
            OUString sString = pStrm->ReadUniString();
            if( pStrm->IsValid() )
            {
                rCell.meType = CELLTYPE_STRING;
                rCell.mpString = new svl::SharedString(GetDoc().GetSharedStringPool().intern(sString));
            }
        }
        break;
        case EXC_CHTR_TYPE_BOOL:
        {
            double fValue = static_cast<double>(pStrm->ReaduInt16() != 0);
            if( pStrm->IsValid() )
            {
                rCell.meType = CELLTYPE_VALUE;
                rCell.mfValue = fValue;
                rFormat = GetFormatter().GetStandardFormat( SvNumFormatType::LOGICAL, ScGlobal::eLnge );
            }
        }
        break;
        case EXC_CHTR_TYPE_FORMULA:
        {
            std::unique_ptr<ScTokenArray> pTokenArray;
            ReadFormula( pTokenArray, rPosition );
            if( pStrm->IsValid() && pTokenArray )
            {
                rCell.meType = CELLTYPE_FORMULA;
                rCell.mpFormula = new ScFormulaCell(&GetDocRef(), rPosition, std::move(pTokenArray));
            }
        }
        break;
        default:
            OSL_FAIL( "XclImpChangeTrack::ReadCell - unknown data type" );
    }
}

void XclImpChangeTrack::ReadChTrInsert()
{
    *pStrm >> aRecHeader;
    if( CheckRecord( EXC_CHTR_OP_UNKNOWN ) )
    {
        if( (aRecHeader.nOpCode != EXC_CHTR_OP_INSROW) &&
            (aRecHeader.nOpCode != EXC_CHTR_OP_INSCOL) &&
            (aRecHeader.nOpCode != EXC_CHTR_OP_DELROW) &&
            (aRecHeader.nOpCode != EXC_CHTR_OP_DELCOL) )
        {
            OSL_FAIL( "XclImpChangeTrack::ReadChTrInsert - unknown action" );
            return;
        }

        ScRange aRange;
        aRange.aStart.SetTab( ReadTabNum() );
        aRange.aEnd.SetTab( aRange.aStart.Tab() );
        sal_uInt16 nFlags = pStrm->ReaduInt16();
        bool bEndOfList = (nFlags & 0x0001); // row auto-inserted at the bottom.
        Read2DRange( aRange );

        if( aRecHeader.nOpCode & EXC_CHTR_OP_COLFLAG )
            aRange.aEnd.SetRow( MAXROW );
        else
            aRange.aEnd.SetCol( MAXCOL );

        bool bValid = pStrm->IsValid();
        if( FoundNestedMode() )
            ReadNestedRecords();

        if( bValid )
        {
            if( aRecHeader.nOpCode & EXC_CHTR_OP_DELFLAG )
                DoDeleteRange( aRange );
            else
                DoInsertRange(aRange, bEndOfList);
        }
    }
}

void XclImpChangeTrack::ReadChTrInfo()
{
    pStrm->DisableDecryption();
    pStrm->Ignore( 32 );
    OUString sUsername( pStrm->ReadUniString() );
    if( !pStrm->IsValid() ) return;

    if( !sUsername.isEmpty() )
        pChangeTrack->SetUser( sUsername );
    pStrm->Seek( 148 );
    if( !pStrm->IsValid() ) return;

    DateTime aDateTime( DateTime::EMPTY );
    ReadDateTime( aDateTime );
    if( pStrm->IsValid() )
        pChangeTrack->SetFixDateTimeLocal( aDateTime );
}

void XclImpChangeTrack::ReadChTrCellContent()
{
    *pStrm >> aRecHeader;
    if( CheckRecord( EXC_CHTR_OP_CELL ) )
    {
        ScAddress aPosition;
        SCTAB nTab = ReadTabNum();
        aPosition.SetTab( nTab );
        sal_uInt16 nValueType;
        nValueType = pStrm->ReaduInt16();
        sal_uInt16 nOldValueType = (nValueType >> 3) & EXC_CHTR_TYPE_MASK;
        sal_uInt16 nNewValueType = nValueType & EXC_CHTR_TYPE_MASK;
        pStrm->Ignore( 2 );
        Read2DAddress( aPosition );
        sal_uInt16 nOldSize;
        nOldSize = pStrm->ReaduInt16();
        SAL_WARN_IF( (nOldSize == 0) != (nOldValueType == EXC_CHTR_TYPE_EMPTY),
            "sc.filter",
            "XclImpChangeTrack::ReadChTrCellContent - old value mismatch" );
        pStrm->Ignore( 4 );
        switch( nValueType & EXC_CHTR_TYPE_FORMATMASK )
        {
            case 0x0000:                            break;
            case 0x1100:    pStrm->Ignore( 16 );    break;
            case 0x1300:    pStrm->Ignore( 8 );     break;
            default:        OSL_FAIL( "XclImpChangeTrack::ReadChTrCellContent - unknown format info" );
        }

        ScCellValue aOldCell;
        ScCellValue aNewCell;
        sal_uInt32 nOldFormat;
        sal_uInt32 nNewFormat;
        ReadCell(aOldCell, nOldFormat, nOldValueType, aPosition);
        ReadCell(aNewCell, nNewFormat, nNewValueType, aPosition);
        if( !pStrm->IsValid() || (pStrm->GetRecLeft() > 0) )
        {
            OSL_FAIL( "XclImpChangeTrack::ReadChTrCellContent - bytes left, action ignored" );
            aOldCell.clear();
            aNewCell.clear();
        }
        else
        {
            ScChangeActionContent* pNewAction =
                pChangeTrack->AppendContentOnTheFly(aPosition, aOldCell, aNewCell, nOldFormat, nNewFormat);
            DoAcceptRejectAction( pNewAction );
        }
    }
}

void XclImpChangeTrack::ReadChTrTabId()
{
    if( nTabIdCount == 0 )  // read only 1st time, otherwise calculated by <ReadChTrInsertTab()>
        nTabIdCount = static_cast< sal_uInt16 >( pStrm->GetRecLeft() >> 1 );
}

void XclImpChangeTrack::ReadChTrMoveRange()
{
    *pStrm >> aRecHeader;
    if( CheckRecord( EXC_CHTR_OP_MOVE ) )
    {
        ScRange aSourceRange;
        ScRange aDestRange;
        aDestRange.aStart.SetTab( ReadTabNum() );
        aDestRange.aEnd.SetTab( aDestRange.aStart.Tab() );
        Read2DRange( aSourceRange );
        Read2DRange( aDestRange );
        aSourceRange.aStart.SetTab( ReadTabNum() );
        aSourceRange.aEnd.SetTab( aSourceRange.aStart.Tab() );

        bool bValid = pStrm->IsValid();
        if( FoundNestedMode() )
            ReadNestedRecords();

        if( bValid )
        {
            pChangeTrack->AppendMove( aSourceRange, aDestRange, nullptr );
            DoAcceptRejectAction( pChangeTrack->GetLast() );
        }
    }
}

void XclImpChangeTrack::ReadChTrInsertTab()
{
    *pStrm >> aRecHeader;
    if( CheckRecord( EXC_CHTR_OP_INSTAB ) )
    {
        SCTAB nTab = ReadTabNum();
        if( pStrm->IsValid() )
        {
            nTabIdCount++;
            DoInsertRange(ScRange(0, 0, nTab, MAXCOL, MAXROW, nTab), false);
        }
    }
}

void XclImpChangeTrack::InitNestedMode()
{
    OSL_ENSURE( eNestedMode == nmBase, "XclImpChangeTrack::InitNestedMode - unexpected nested mode" );
    if( eNestedMode == nmBase )
        eNestedMode = nmFound;
}

void XclImpChangeTrack::ReadNestedRecords()
{
    OSL_ENSURE( eNestedMode == nmFound, "XclImpChangeTrack::StartNestedMode - missing nested mode" );
    if( eNestedMode == nmFound )
    {
        eNestedMode = nmNested;
        ReadRecords();
    }
}

bool XclImpChangeTrack::EndNestedMode()
{
    OSL_ENSURE( eNestedMode != nmBase, "XclImpChangeTrack::EndNestedMode - missing nested mode" );
    bool bReturn = (eNestedMode == nmNested);
    eNestedMode = nmBase;
    return bReturn;
}

void XclImpChangeTrack::ReadRecords()
{
    bool bExitLoop = false;

    while( !bExitLoop && !bGlobExit && pStrm->StartNextRecord() )
    {
        switch( pStrm->GetRecId() )
        {
            case 0x000A:    bGlobExit = true;               break;
            case 0x0137:    ReadChTrInsert();               break;
            case 0x0138:    ReadChTrInfo();                 break;
            case 0x013B:    ReadChTrCellContent();          break;
            case 0x013D:    ReadChTrTabId();                break;
            case 0x0140:    ReadChTrMoveRange();            break;
            case 0x014D:    ReadChTrInsertTab();            break;
            case 0x014E:
            case 0x0150:    InitNestedMode();               break;
            case 0x014F:
            case 0x0151:    bExitLoop = EndNestedMode();    break;
        }
    }
}

void XclImpChangeTrack::Apply()
{
    if( pChangeTrack )
    {
        pChangeTrack->SetUser( sOldUsername );
        pChangeTrack->SetUseFixDateTime( false );

        GetDoc().SetChangeTrack( std::move(pChangeTrack) );

        ScChangeViewSettings aSettings;
        aSettings.SetShowChanges( true );
        GetDoc().SetChangeViewSettings( aSettings );
    }
}

// class XclImpChTrFmlConverter

XclImpChTrFmlConverter::XclImpChTrFmlConverter(
    XclImpRoot& rRoot, XclImpChangeTrack& rXclChTr ) :
    ExcelToSc8( rRoot ),
    rChangeTrack( rXclChTr ) {}

XclImpChTrFmlConverter::~XclImpChTrFmlConverter()
{
}

// virtual, called from ExcToSc8::Convert()
bool XclImpChTrFmlConverter::Read3DTabReference( sal_uInt16 /*nIxti*/, SCTAB& rFirstTab, SCTAB& rLastTab,
                                                 ExternalTabInfo& rExtInfo )
{
    rChangeTrack.Read3DTabRefInfo( rFirstTab, rLastTab, rExtInfo );
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

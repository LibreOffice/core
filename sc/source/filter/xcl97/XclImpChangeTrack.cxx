/*************************************************************************
 *
 *  $RCSfile: XclImpChangeTrack.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dr $ $Date: 2001-01-18 16:33:24 $
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

#ifndef _SC_XCLIMPCHANGETRACK_HXX
#include "XclImpChangeTrack.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif
#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif

#ifndef SC_CHGVISET_HXX
#include "chgviset.hxx"
#endif
#ifndef SC_CELL_HXX
#include "cell.hxx"
#endif
#ifndef SC_CHGTRACK_HXX
#include "chgtrack.hxx"
#endif
#ifndef _SPSTRING_HXX
#include "spstring.hxx"
#endif

//___________________________________________________________________

extern const sal_Char* pRevLogStreamName;

//___________________________________________________________________
// class XclImpChangeTrack

XclImpChangeTrack::XclImpChangeTrack( RootData* pRootData ) :
    ExcRoot( pRootData ),
    aRecHeader(),
    sOldUsername(),
    pChangeTrack( NULL ),
    pInStrm( NULL ),
    pContStrm( NULL ),
    pStrm( NULL ),
    nBytesLeft( 0 ),
    nTabIdCount( 0 ),
    eNestedMode( nmBase )
{
    DBG_ASSERT( pExcRoot && pExcRoot->pImpTabIdBuffer && pExcRoot->pSupbookBuffer,
        "XclImpChangeTrack::XclImpChangeTrack - root data incomplete" );
    if( !pExcRoot || !pExcRoot->pImpTabIdBuffer || !pExcRoot->pSupbookBuffer )
        return;

    String sStreamName( RTL_CONSTASCII_STRINGPARAM( pRevLogStreamName ) );
    SvStorage& rStorage = *pExcRoot->pRootStorage;

    if( !rStorage.IsContained( sStreamName ) || !rStorage.IsStream( sStreamName ) )
        return;

    pInStrm = rStorage.OpenStream( sStreamName, STREAM_STD_READ );
    if( pInStrm )
    {
        pChangeTrack = new ScChangeTrack( pExcRoot->pDoc );
        if( pChangeTrack )
        {
            sOldUsername = pChangeTrack->GetUser();
            pChangeTrack->SetUseFixDateTime( TRUE );

            ReadStream();
        }
    }
}

XclImpChangeTrack::~XclImpChangeTrack()
{
    if( pChangeTrack )
        delete pChangeTrack;
    if( pStrm )
        delete pStrm;
    if( pContStrm )
        delete pContStrm;
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

void XclImpChangeTrack::DoInsertRange( const ScRange& rRange )
{
    sal_uInt32 nFirst = pChangeTrack->GetActionMax() + 1;
    pChangeTrack->AppendInsert( rRange );
    sal_uInt32 nLast = pChangeTrack->GetActionMax();
    DoAcceptRejectAction( nFirst, nLast );
}

void XclImpChangeTrack::DoDeleteRange( const ScRange& rRange )
{
    sal_uInt32 nFirst, nLast;
    pChangeTrack->AppendDeleteRange( rRange, NULL, nFirst, nLast );
    DoAcceptRejectAction( nFirst, nLast );
}

sal_Bool XclImpChangeTrack::ReadStringHeader( sal_uInt16& rSize, sal_Bool& r16Bit )
{
    if( CheckSize( 3 ) )
    {
        rSize = ReaduInt16();
        sal_uInt8 rGrbit = ReaduInt8();
        r16Bit = ((rGrbit & 0x01) == 0x01);
        return sal_True;
    }
    return sal_False;
}

void XclImpChangeTrack::ReadString( String& rString )
{
    sal_uInt16 nSize;
    sal_Bool b16Bit;
    if( ReadStringHeader( nSize, b16Bit ) )
    {
        if( pContStrm )
            rString = ::ReadString( *pContStrm, nBytesLeft, nSize, !b16Bit, aCutPosList, *pExcRoot->pCharset );
        else
            rString = ::ReadString( *pInStrm, nBytesLeft, nSize, !b16Bit, *pExcRoot->pCharset );
    }
    else
        rString.Erase();
}

void XclImpChangeTrack::IgnoreString()
{
    sal_uInt16 nSize;
    sal_Bool b16Bit;
    if( ReadStringHeader( nSize, b16Bit ) )
    {
        if( pContStrm )
            ::ReadString( *pContStrm, nBytesLeft, nSize, !b16Bit, aCutPosList, *pExcRoot->pCharset );
        else
            ::SkipString( *pInStrm, nBytesLeft, nSize, !b16Bit );
    }
}

void XclImpChangeTrack::ReadDateTime( DateTime& rDateTime )
{
    sal_uInt16 nYear;
    sal_uInt8 nMonth, nDay, nHour, nMin, nSec;

    *pStrm >> nYear >> nMonth >> nDay >> nHour >> nMin >> nSec;
    nBytesLeft -= 7;

    rDateTime.SetYear( nYear );
    rDateTime.SetMonth( nMonth );
    rDateTime.SetDay( nDay );
    rDateTime.SetHour( nHour );
    rDateTime.SetMin( nMin );
    rDateTime.SetSec( nSec );
    rDateTime.Set100Sec( 0 );
}

void XclImpChangeTrack::CopyFromStreamToStream( SvStream& rFromStrm, SvStream& rToStrm, sal_uInt16 nBytes )
{
    sal_uInt8* pBuffer = new sal_uInt8[ nBytes ];
    rFromStrm.Read( pBuffer, nBytes );
    rToStrm.Write( pBuffer, nBytes );
    nBytesLeft -= nBytes;
    delete pBuffer;
}

sal_Bool XclImpChangeTrack::ReadRecordHeader()
{
    if( CheckSize( 12 ) )
    {
        aRecHeader.nSize = ReaduInt32();
        aRecHeader.nIndex = ReaduInt32();
        aRecHeader.nOpCode = ReaduInt16();
        aRecHeader.nAccept = ReaduInt16();
        return sal_True;
    }
    return sal_False;
}

sal_Bool XclImpChangeTrack::CheckRecord( sal_uInt16 nOpCode, sal_Int32 nMinSize, sal_Int32 nMaxSize )
{
    if( (nOpCode != EXC_CHTR_OP_UNKNOWN) && (aRecHeader.nOpCode != nOpCode) )
    {
        DBG_ERROR( "XclImpChangeTrack::CheckRecord - unknown action" );
        return sal_False;
    }
    if( (nMinSize > 0) && (nBytesLeft < nMinSize) )
    {
        DBG_ERROR( "XclImpChangeTrack::CheckRecord - record undersized" );
        return sal_False;
    }
    if( (nMaxSize > 0) && (nBytesLeft > nMaxSize) )
    {
        DBG_ERROR( "XclImpChangeTrack::CheckRecord - record oversized" );
        return sal_False;
    }
    return aRecHeader.nIndex != 0;
}

sal_Bool XclImpChangeTrack::Read3DTabRefInfo( sal_uInt16& rFirstTab, sal_uInt16& rLastTab )
{
    if( !CheckSize( 7 ) )
    {
        DBG_ERROR( "XclImpChangeTrack::Read3DTabRefInfo - no more data" );
        return sal_False;
    }

    if( LookAtuInt8() == 0x01 )
    {
        // internal ref - read tab num and return sc tab num (position in TABID list)
        IgnoreBytes( 3 );
        rFirstTab = pExcRoot->pImpTabIdBuffer->GetIndex( ReaduInt16(), nTabIdCount );
        sal_uInt8 nFillByte = ReaduInt8();
        rLastTab = (nFillByte == 0x00) ?
            pExcRoot->pImpTabIdBuffer->GetIndex( ReaduInt16(), nTabIdCount ) : rFirstTab;
    }
    else
    {
        // external ref - read doc and tab name and find sc tab num
        String sDocName, sTabName;
        sal_Bool bSelf;
        XclImpSupbook::ReadDocName( *pStrm, *pExcRoot, nBytesLeft, sDocName, bSelf );
        IgnoreBytes( 1 );
        XclImpSupbook::ReadTabName( *pStrm, *pExcRoot, nBytesLeft, sTabName );
        IgnoreBytes( 1 );
        const XclImpSupbook* pSupbook = pExcRoot->pSupbookBuffer->Get( sDocName );
        rFirstTab = rLastTab = pSupbook ? pSupbook->GetScTabNum( sTabName ) : 0xFFFF;
    }
    return sal_True;
}

sal_Bool XclImpChangeTrack::ReadFormula( ScTokenArray*& rpTokenArray, const ScAddress& rPosition )
{
    sal_Bool bOK = sal_False;
    if( CheckSize( 2 ) )
    {
        sal_uInt16 nFmlSize = ReaduInt16();
        if( CheckSize( nFmlSize ) )
        {
            sal_uInt32 nRecPos = pStrm->Tell();
            sal_Int32 nTmpSize = nFmlSize;

            // move token array to memory stream
            SvMemoryStream aMemStrm;
            CopyFromStreamToStream( *pStrm, aMemStrm, nFmlSize );
            aMemStrm.Seek( STREAM_SEEK_TO_BEGIN );

            // read the formula, 3D tab refs from extended data
            XclImpChTrFmlConverter aFmlConv( pExcRoot, aMemStrm, *this );
            aFmlConv.Reset( nTmpSize, rPosition );
            const ScTokenArray* pArray = NULL;
            bOK = (aFmlConv.Convert( pArray, nTmpSize ) == ConvOK);
            rpTokenArray = (bOK && pArray) ? new ScTokenArray( *pArray ) : NULL;
            IgnoreBytes( 1 );
        }
    }
    return bOK;
}

void XclImpChangeTrack::ReadCell(
        ScBaseCell*& rpCell,
        sal_uInt32& rFormat,
        sal_uInt16 nFlags,
        const ScAddress& rPosition )
{
    rpCell = NULL;
    rFormat = 0;
    switch( nFlags & EXC_CHTR_TYPE_MASK )
    {
        case EXC_CHTR_TYPE_EMPTY:
        break;
        case EXC_CHTR_TYPE_RK:
            if( CheckSize( 4 ) )
                rpCell = new ScValueCell( ReadRK() );
        break;
        case EXC_CHTR_TYPE_DOUBLE:
            if( CheckSize( 8 ) )
                rpCell = new ScValueCell( ReadDouble() );
        break;
        case EXC_CHTR_TYPE_STRING:
            if( CheckSize( 3 ) )
            {
                String sString;
                ReadString( sString );
                rpCell = new ScStringCell( sString );
            }
        break;
        case EXC_CHTR_TYPE_BOOL:
            if( CheckSize( 2 ) )
            {
                rpCell = new ScValueCell( ReadBool() );
                rFormat = pExcRoot->pFormTable->GetStandardFormat( NUMBERFORMAT_LOGICAL, pExcRoot->eDefLanguage );
            }
        break;
        case EXC_CHTR_TYPE_FORMULA:
        {
            ScTokenArray* pTokenArray = NULL;
            sal_Bool bOK = ReadFormula( pTokenArray, rPosition );
            if( bOK && pTokenArray )
                rpCell = new ScFormulaCell( pExcRoot->pDoc, rPosition, pTokenArray );
        }
        break;
        default:
            DBG_ERROR( "XclImpChangeTrack::ReadCell - unknown data type" );
    }
}

void XclImpChangeTrack::ReadChTrInsert()
{
    if( ReadRecordHeader() && CheckRecord( EXC_CHTR_OP_UNKNOWN, 12 ) )
    {
        if( (aRecHeader.nOpCode < EXC_CHTR_OP_INSROW) || (aRecHeader.nOpCode > EXC_CHTR_OP_DELCOL) )
        {
            DBG_ERROR( "XclImpChangeTrack::ReadChTrInsert - unknown action" );
            return;
        }

        ScRange aRange;
        aRange.aStart.SetTab( ReadTabNum() );
        aRange.aEnd.SetTab( aRange.aStart.Tab() );
        IgnoreBytes( 2 );
        Read2DRange( aRange );

        if( aRecHeader.nOpCode & EXC_CHTR_OP_COLFLAG )
            aRange.aEnd.SetRow( MAXROW );
        else
            aRange.aEnd.SetCol( MAXCOL );

        if( FoundNestedMode() )
            ReadRecords();

        if( aRecHeader.nOpCode & EXC_CHTR_OP_DELFLAG )
            DoDeleteRange( aRange );
        else
            DoInsertRange( aRange );
    }
}

void XclImpChangeTrack::ReadChTrInfo()
{
    if( CheckSize( 35 ) )
    {
        IgnoreBytes( 32 );
        String sUsername;
        ReadString( sUsername );
        if( sUsername.Len() )
            pChangeTrack->SetUser( sUsername );
        if( CheckSize( 10 ) )
        {
            IgnoreBytes( nBytesLeft - 10 );
            DateTime aDateTime;
            ReadDateTime( aDateTime );
            pChangeTrack->SetFixDateTimeLocal( aDateTime );
        }
    }
}

void XclImpChangeTrack::ReadChTrCellContent()
{
    if( ReadRecordHeader() && CheckRecord( EXC_CHTR_OP_CELL, 16 ) )
    {
        ScAddress aPosition;
        aPosition.SetTab( ReadTabNum() );
        sal_uInt16 nValueType = ReaduInt16();
        sal_uInt16 nOldValueType = (nValueType >> 3) & EXC_CHTR_TYPE_MASK;
        sal_uInt16 nNewValueType = nValueType & EXC_CHTR_TYPE_MASK;
        IgnoreBytes( 2 );
        Read2DAddress( aPosition );
        sal_uInt16 nOldSize = ReaduInt16();
        DBG_ASSERT( (nOldSize == 0) == (nOldValueType == EXC_CHTR_TYPE_EMPTY),
            "XclImpChangeTrack::ReadChTrCellContent - old value mismatch" );
        IgnoreBytes( 4 );
        switch( nValueType & EXC_CHTR_TYPE_FORMATMASK )
        {
            case 0x0000:
            break;
            case 0x1100:
                if( CheckSize( 16 ) )
                    IgnoreBytes( 16 );
            break;
            case 0x1300:
                if( CheckSize( 8 ) )
                    IgnoreBytes( 8 );
            break;
            default:
                DBG_ERROR( "XclImpChangeTrack::ReadChTrCellContent - unknown format info" );
        }

        ScBaseCell* pOldCell;
        ScBaseCell* pNewCell;
        sal_uInt32 nOldFormat;
        sal_uInt32 nNewFormat;
        ReadCell( pOldCell, nOldFormat, nOldValueType, aPosition );
        ReadCell( pNewCell, nNewFormat, nNewValueType, aPosition );
        if( nBytesLeft )
        {
            DBG_ERROR( "XclImpChangeTrack::ReadChTrCellContent - bytes left, action ignored" );
            if( pOldCell )
                pOldCell->Delete();
            if( pNewCell )
                pNewCell->Delete();
        }
        else
        {
            ScChangeActionContent* pNewAction =
                pChangeTrack->AppendContentOnTheFly( aPosition, pOldCell, pNewCell, nOldFormat, nNewFormat );
            DoAcceptRejectAction( pNewAction );
        }
    }
}

void XclImpChangeTrack::ReadChTrMoveRange()
{
    if( ReadRecordHeader() && CheckRecord( EXC_CHTR_OP_MOVE, 20 ) )
    {
        ScRange aSourceRange;
        ScRange aDestRange;
        aDestRange.aStart.SetTab( ReadTabNum() );
        aDestRange.aEnd.SetTab( aDestRange.aStart.Tab() );
        Read2DRange( aSourceRange );
        Read2DRange( aDestRange );
        aSourceRange.aStart.SetTab( ReadTabNum() );
        aSourceRange.aEnd.SetTab( aSourceRange.aStart.Tab() );
        if( FoundNestedMode() )
            ReadRecords();
        pChangeTrack->AppendMove( aSourceRange, aDestRange, NULL );
        DoAcceptRejectAction( pChangeTrack->GetLast() );
    }
}

void XclImpChangeTrack::ReadChTrInsertTab()
{
    if( ReadRecordHeader() && CheckRecord( EXC_CHTR_OP_INSTAB, 9 ) )
    {
        sal_uInt16 nTab = ReadTabNum();
        DoInsertRange( ScRange( 0, 0, nTab, MAXCOL, MAXROW, nTab ) );
    }
}

void XclImpChangeTrack::StartNestedMode()
{
    DBG_ASSERT( eNestedMode == nmBase, "XclImpChangeTrack::StartNestedMode - unexpected nested mode" );
    if( eNestedMode == nmBase )
        eNestedMode = nmFound;
}

sal_Bool XclImpChangeTrack::EndNestedMode()
{
    DBG_ASSERT( eNestedMode != nmBase, "XclImpChangeTrack::EndNestedMode - missing nested mode" );
    sal_Bool bReturn = (eNestedMode == nmNested);
    eNestedMode = nmBase;
    return bReturn;
}

sal_uInt32 XclImpChangeTrack::PrepareReadRecord( sal_uInt16 nRecLen )
{
    DBG_ASSERT( !pContStrm, "XclImpChangeTrack::PrepareReadRecord - cont stream still alive" );

    sal_uInt32 nNextPos;
    if( nRecLen )
    {
        sal_uInt32 nLeft;
        aCutPosList.Clear();
        pContStrm = ImportTyp::CreateContinueStream( *pInStrm, nRecLen, nLeft, nNextPos, FALSE, &aCutPosList );
        nBytesLeft = (sal_Int32) nLeft;
        if( pContStrm )
        {
            pContStrm->Seek( STREAM_SEEK_TO_BEGIN );
            pStrm = pContStrm;
        }
    }
    else
    {
        nNextPos = pInStrm->Tell() + nRecLen;
        nBytesLeft = nRecLen;
    }
    return nNextPos;
}

void XclImpChangeTrack::EndReadRecord( sal_uInt32 nNextPos )
{
    if( pContStrm )
        delete pContStrm;
    pContStrm = NULL;
    pStrm = pInStrm;
    pInStrm->Seek( nNextPos );
}

void XclImpChangeTrack::ReadRecords()
{
    sal_uInt32  nNextRecPos;
    sal_uInt16  nRecId;
    sal_uInt16  nRecLen;
    sal_Bool    bExitLoop = sal_False;

    while( !bExitLoop )
    {
        *pInStrm >> nRecId >> nRecLen;

        nNextRecPos = PrepareReadRecord( nRecLen );
        switch( nRecId )
        {
            case 0x000A:    nStreamLen = 0;                                 break;
            case 0x0137:    ReadChTrInsert();                               break;
            case 0x0138:    ReadChTrInfo();                                 break;
            case 0x013B:    ReadChTrCellContent();                          break;
            case 0x013D:    nTabIdCount = (sal_uInt16)(nBytesLeft >> 1);    break;
            case 0x0140:    ReadChTrMoveRange();                            break;
            case 0x014D:    ReadChTrInsertTab();                            break;
            case 0x014E:
            case 0x0150:    StartNestedMode();                              break;
            case 0x014F:
            case 0x0151:    bExitLoop = EndNestedMode();                    break;
        }
        EndReadRecord( nNextRecPos );
        bExitLoop |= (nNextRecPos >= nStreamLen);
    }
}

void XclImpChangeTrack::ReadStream()
{
    pInStrm->Seek( STREAM_SEEK_TO_END );
    nStreamLen = pInStrm->Tell();
    if( nStreamLen == 0xFFFFFFFF )
        return;

    pInStrm->Seek( STREAM_SEEK_TO_BEGIN );
    ReadRecords();
}

void XclImpChangeTrack::Apply()
{
    if( pChangeTrack && pExcRoot )
    {
        pChangeTrack->SetUser( sOldUsername );
        pChangeTrack->SetUseFixDateTime( FALSE );

        pExcRoot->pDoc->SetChangeTrack( pChangeTrack );
        pChangeTrack = NULL;

        ScChangeViewSettings aSettings;
        aSettings.SetShowChanges( TRUE );
        pExcRoot->pDoc->SetChangeViewSettings( aSettings );
    }
}

//___________________________________________________________________
// class XclImpChTrFmlConverter

XclImpChTrFmlConverter::~XclImpChTrFmlConverter()
{
}

// virtual, called from ExcToSc8::Convert()
BOOL XclImpChTrFmlConverter::Read3DTabReference( UINT16& rFirstTab, UINT16& rLastTab )
{
    Ignore( 2 );
    return rChangeTrack.Read3DTabRefInfo( rFirstTab, rLastTab );
}



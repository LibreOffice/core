/*************************************************************************
 *
 *  $RCSfile: XclImpChangeTrack.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 09:07:44 $
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

#ifndef SC_XCLIMPCHANGETRACK_HXX
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

#ifndef SC_XILINK_HXX
#include "xilink.hxx"
#endif

//___________________________________________________________________
// class XclImpChangeTrack

XclImpChangeTrack::XclImpChangeTrack( RootData* pRootData, const XclImpStream& rBookStrm ) :
    ExcRoot( pRootData ),
    aRecHeader(),
    sOldUsername(),
    pChangeTrack( NULL ),
    pStrm( NULL ),
    nTabIdCount( 0 ),
    bGlobExit( sal_False ),
    eNestedMode( nmBase )
{
    const XclImpRoot& rRoot = *pExcRoot->pIR;

    // Verify that the User Names stream exists before going any further. Excel adds both
    // "Revision Log" and "User Names" streams when Change Tracking is active but the Revision log
    // remains if Change Tracking is turned off.
    SvStorageStreamRef xUserStrm = rRoot.OpenStream( EXC_STREAM_USERNAMES );
    if( !xUserStrm.Is() )
        return;

    xInStrm = rRoot.OpenStream( EXC_STREAM_REVLOG );
    if( xInStrm.Is() )
    {
        xInStrm->Seek( STREAM_SEEK_TO_END );
        ULONG nStreamLen = xInStrm->Tell();
        if( (xInStrm->GetErrorCode() == ERRCODE_NONE) && (nStreamLen != STREAM_SEEK_TO_END) )
        {
            xInStrm->Seek( STREAM_SEEK_TO_BEGIN );
            pStrm = new XclImpStream( *xInStrm, rRoot );
            pStrm->CopyDecrypterFrom( rBookStrm );
            pChangeTrack = new ScChangeTrack( rRoot.GetDocPtr() );

            sOldUsername = pChangeTrack->GetUser();
            pChangeTrack->SetUseFixDateTime( TRUE );

            ReadRecords();
        }
    }
}

XclImpChangeTrack::~XclImpChangeTrack()
{
    delete pChangeTrack;
    delete pStrm;
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

SCTAB XclImpChangeTrack::ReadTabNum()
{
    return static_cast<SCTAB>(pExcRoot->pIR->GetTabInfo().GetCurrentIndex(
                pStrm->ReaduInt16(), nTabIdCount ));
}

void XclImpChangeTrack::ReadDateTime( DateTime& rDateTime )
{
    sal_uInt16 nYear;
    sal_uInt8 nMonth, nDay, nHour, nMin, nSec;

    *pStrm >> nYear >> nMonth >> nDay >> nHour >> nMin >> nSec;

    rDateTime.SetYear( nYear );
    rDateTime.SetMonth( nMonth );
    rDateTime.SetDay( nDay );
    rDateTime.SetHour( nHour );
    rDateTime.SetMin( nMin );
    rDateTime.SetSec( nSec );
    rDateTime.Set100Sec( 0 );
}

sal_Bool XclImpChangeTrack::CheckRecord( sal_uInt16 nOpCode )
{
    if( (nOpCode != EXC_CHTR_OP_UNKNOWN) && (aRecHeader.nOpCode != nOpCode) )
    {
        DBG_ERROR( "XclImpChangeTrack::CheckRecord - unknown action" );
        return sal_False;
    }
    return aRecHeader.nIndex != 0;
}

sal_Bool XclImpChangeTrack::Read3DTabRefInfo( SCTAB& rFirstTab, SCTAB& rLastTab )
{
    if( LookAtuInt8() == 0x01 )
    {
        // internal ref - read tab num and return sc tab num (position in TABID list)
        pStrm->Ignore( 3 );
        rFirstTab = static_cast< SCTAB >( pExcRoot->pIR->GetTabInfo().GetCurrentIndex( pStrm->ReaduInt16(), nTabIdCount ) );
        sal_uInt8 nFillByte = pStrm->ReaduInt8();
        rLastTab = (nFillByte == 0x00) ?
            static_cast< SCTAB >( pExcRoot->pIR->GetTabInfo().GetCurrentIndex( pStrm->ReaduInt16(), nTabIdCount ) ) : rFirstTab;
    }
    else
    {
        // external ref - read doc and tab name and find sc tab num
        // - URL
        String aEncUrl( pStrm->ReadUniString() );
        String aUrl;
        bool bSelf;
        XclImpUrlHelper::DecodeUrl( aUrl, bSelf, *pExcRoot->pIR, aEncUrl );
        pStrm->Ignore( 1 );
        // - sheet name, always separated from URL
        String aTabName( pStrm->ReadUniString() );
        ScfTools::ConvertToScSheetName( aTabName );
        pStrm->Ignore( 1 );
        rFirstTab = rLastTab = static_cast<SCTAB>(pExcRoot->pIR->GetLinkManager().GetScTab( aUrl, aTabName ));
    }
    return sal_True;
}

void XclImpChangeTrack::ReadFormula( ScTokenArray*& rpTokenArray, const ScAddress& rPosition )
{
    sal_uInt16 nFmlSize;
    *pStrm >> nFmlSize;

    // create a memory stream and copy the formula to be able to read simultaneously
    // the formula and the additional 3D tab ref data following the formula
    // here we have to simulate an Excel record to be able to use an XclImpStream...
    // 2do: remove the stream member from formula converter and add it as a parameter
    // to the Convert() routine (to prevent the construction/destruction of the
    // converter in each formula)
    SvMemoryStream aMemStrm;
    aMemStrm << (sal_uInt16) 0x0001 << nFmlSize;
    pStrm->CopyToStream( aMemStrm, nFmlSize );
    XclImpStream aFmlaStrm( aMemStrm, *pExcRoot->pIR );
    aFmlaStrm.StartNextRecord();
    XclImpChTrFmlConverter aFmlConv( pExcRoot, aFmlaStrm, *this );

    // read the formula, 3D tab refs from extended data
    const ScTokenArray* pArray = NULL;
    aFmlConv.Reset( rPosition );
    BOOL bOK = (aFmlConv.Convert( pArray, nFmlSize ) == ConvOK);
    rpTokenArray = (bOK && pArray) ? new ScTokenArray( *pArray ) : NULL;
    pStrm->Ignore( 1 );
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
        {
            double fValue = ReadRK();
            if( pStrm->IsValid() )
                rpCell = new ScValueCell( fValue );
        }
        break;
        case EXC_CHTR_TYPE_DOUBLE:
        {
            double fValue;
            *pStrm >> fValue;
            if( pStrm->IsValid() )
                rpCell = new ScValueCell( fValue );
        }
        break;
        case EXC_CHTR_TYPE_STRING:
        {
            String sString( pStrm->ReadUniString() );
            if( pStrm->IsValid() )
                rpCell = new ScStringCell( sString );
        }
        break;
        case EXC_CHTR_TYPE_BOOL:
        {
            double fValue = (double) ReadBool();
            if( pStrm->IsValid() )
            {
                rpCell = new ScValueCell( fValue );
                rFormat = pExcRoot->pIR->GetFormatter().GetStandardFormat( NUMBERFORMAT_LOGICAL, ScGlobal::eLnge );
            }
        }
        break;
        case EXC_CHTR_TYPE_FORMULA:
        {
            ScTokenArray* pTokenArray = NULL;
            ReadFormula( pTokenArray, rPosition );
            if( pStrm->IsValid() && pTokenArray )
                rpCell = new ScFormulaCell( pExcRoot->pDoc, rPosition, pTokenArray );
        }
        break;
        default:
            DBG_ERROR( "XclImpChangeTrack::ReadCell - unknown data type" );
    }
}

void XclImpChangeTrack::ReadChTrInsert()
{
    *pStrm >> aRecHeader;
    if( CheckRecord( EXC_CHTR_OP_UNKNOWN ) )
    {
        if( (aRecHeader.nOpCode < EXC_CHTR_OP_INSROW) || (aRecHeader.nOpCode > EXC_CHTR_OP_DELCOL) )
        {
            DBG_ERROR( "XclImpChangeTrack::ReadChTrInsert - unknown action" );
            return;
        }

        ScRange aRange;
        aRange.aStart.SetTab( ReadTabNum() );
        aRange.aEnd.SetTab( aRange.aStart.Tab() );
        pStrm->Ignore( 2 );
        Read2DRange( aRange );

        if( aRecHeader.nOpCode & EXC_CHTR_OP_COLFLAG )
            aRange.aEnd.SetRow( MAXROW );
        else
            aRange.aEnd.SetCol( MAXCOL );

        BOOL bValid = pStrm->IsValid();
        if( FoundNestedMode() )
            ReadNestedRecords();

        if( bValid )
        {
            if( aRecHeader.nOpCode & EXC_CHTR_OP_DELFLAG )
                DoDeleteRange( aRange );
            else
                DoInsertRange( aRange );
        }
    }
}

void XclImpChangeTrack::ReadChTrInfo()
{
    pStrm->DisableDecryption();
    pStrm->Ignore( 32 );
    String sUsername( pStrm->ReadUniString() );
    if( !pStrm->IsValid() ) return;

    if( sUsername.Len() )
        pChangeTrack->SetUser( sUsername );
    pStrm->Seek( 148 );
    if( !pStrm->IsValid() ) return;

    DateTime aDateTime;
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
        aPosition.SetTab( ReadTabNum() );
        sal_uInt16 nValueType;
        *pStrm >> nValueType;
        sal_uInt16 nOldValueType = (nValueType >> 3) & EXC_CHTR_TYPE_MASK;
        sal_uInt16 nNewValueType = nValueType & EXC_CHTR_TYPE_MASK;
        pStrm->Ignore( 2 );
        Read2DAddress( aPosition );
        sal_uInt16 nOldSize;
        *pStrm >> nOldSize;
        DBG_ASSERT( (nOldSize == 0) == (nOldValueType == EXC_CHTR_TYPE_EMPTY),
            "XclImpChangeTrack::ReadChTrCellContent - old value mismatch" );
        pStrm->Ignore( 4 );
        switch( nValueType & EXC_CHTR_TYPE_FORMATMASK )
        {
            case 0x0000:                            break;
            case 0x1100:    pStrm->Ignore( 16 );    break;
            case 0x1300:    pStrm->Ignore( 8 );     break;
            default:        DBG_ERROR( "XclImpChangeTrack::ReadChTrCellContent - unknown format info" );
        }

        ScBaseCell* pOldCell;
        ScBaseCell* pNewCell;
        sal_uInt32 nOldFormat;
        sal_uInt32 nNewFormat;
        ReadCell( pOldCell, nOldFormat, nOldValueType, aPosition );
        ReadCell( pNewCell, nNewFormat, nNewValueType, aPosition );
        if( !pStrm->IsValid() || pStrm->GetRecLeft() )
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

void XclImpChangeTrack::ReadChTrTabId()
{
    if( !nTabIdCount )      // read only 1st time, otherwise calculated by <ReadChTrInsertTab()>
        nTabIdCount = (pStrm->GetRecLeft() >> 1);
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

        BOOL bValid = pStrm->IsValid();
        if( FoundNestedMode() )
            ReadNestedRecords();

        if( bValid )
        {
            pChangeTrack->AppendMove( aSourceRange, aDestRange, NULL );
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
            DoInsertRange( ScRange( 0, 0, nTab, MAXCOL, MAXROW, nTab ) );
        }
    }
}

void XclImpChangeTrack::InitNestedMode()
{
    DBG_ASSERT( eNestedMode == nmBase, "XclImpChangeTrack::InitNestedMode - unexpected nested mode" );
    if( eNestedMode == nmBase )
        eNestedMode = nmFound;
}

void XclImpChangeTrack::ReadNestedRecords()
{
    DBG_ASSERT( eNestedMode == nmFound, "XclImpChangeTrack::StartNestedMode - missing nested mode" );
    if( eNestedMode == nmFound )
    {
        eNestedMode = nmNested;
        ReadRecords();
    }
}

sal_Bool XclImpChangeTrack::EndNestedMode()
{
    DBG_ASSERT( eNestedMode != nmBase, "XclImpChangeTrack::EndNestedMode - missing nested mode" );
    sal_Bool bReturn = (eNestedMode == nmNested);
    eNestedMode = nmBase;
    return bReturn;
}

void XclImpChangeTrack::ReadRecords()
{
    sal_Bool bExitLoop = sal_False;

    while( !bExitLoop && !bGlobExit && pStrm->StartNextRecord() )
    {
        switch( pStrm->GetRecId() )
        {
            case 0x000A:    bGlobExit = sal_True;           break;
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
BOOL XclImpChTrFmlConverter::Read3DTabReference( SCTAB& rFirstTab, SCTAB& rLastTab )
{
    aIn.Ignore( 2 );
    return rChangeTrack.Read3DTabRefInfo( rFirstTab, rLastTab );
}


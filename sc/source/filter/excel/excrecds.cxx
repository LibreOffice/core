/*************************************************************************
 *
 *  $RCSfile: excrecds.cxx,v $
 *
 *  $Revision: 1.65 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-19 11:35:31 $
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

//------------------------------------------------------------------------

#ifndef INCLUDED_SVX_COUNTRYID_HXX
#include <svx/countryid.hxx>
#endif

#include "scitems.hxx"
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <sfx2/objsh.hxx>

#include <svx/editdata.hxx>
#include <svx/editeng.hxx>
#include <svx/editobj.hxx>
#include <svx/editstat.hxx>

#include <svx/flditem.hxx>
#include <svx/flstitem.hxx>

#include <svx/algitem.hxx>
#include <svx/boxitem.hxx>
#include <svx/brshitem.hxx>
#include <svx/pageitem.hxx>
#include <svx/paperinf.hxx>
#include <svx/sizeitem.hxx>
#include <svx/ulspitem.hxx>
#include <svx/fhgtitem.hxx>
#include <svtools/intitem.hxx>
#include <svtools/zforlist.hxx>
#include <svtools/zformat.hxx>
#include <svtools/ctrltool.hxx>

#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>

#include <string.h>

#include "global.hxx"
#include "globstr.hrc"
#include "docpool.hxx"
#include "patattr.hxx"
#include "cell.hxx"
#include "document.hxx"
#include "scextopt.hxx"
#include "patattr.hxx"
#include "attrib.hxx"
#include "progress.hxx"
#include "dociter.hxx"
#include "rangenam.hxx"
#include "dbcolect.hxx"
#include "stlsheet.hxx"
#include "stlpool.hxx"
#include "editutil.hxx"

#include "excrecds.hxx"
#include "excdoc.hxx"
#include "root.hxx"
#include "excupn.hxx"

#ifndef SC_XLFORMULA_HXX
#include "xlformula.hxx"
#endif
#ifndef SC_XELINK_HXX
#include "xelink.hxx"
#endif
#ifndef SC_XESTYLE_HXX
#include "xestyle.hxx"
#endif
#ifndef SC_XECONTENT_HXX
#include "xecontent.hxx"
#endif

#include "xcl97rec.hxx"



//------------------------------------------------------------- class ExcCell -

UINT32          ExcCell::nCellCount = 0UL;
ScProgress*     ExcCell::pPrgrsBar = NULL;
#ifdef DBG_UTIL
INT32           ExcCell::_nRefCount = 0L;
#endif


//--------------------------------------------------------- class ExcDummy_00 -
const BYTE      ExcDummy_00::pMyData[] = {
    0xe1, 0x00, 0x00, 0x00,                                 // INTERFACEHDR
    0xc1, 0x00, 0x02, 0x00, 0x00, 0x00,                     // MMS
    0xbf, 0x00, 0x00, 0x00,                                 // TOOLBARHDR
    0xc0, 0x00, 0x00, 0x00,                                 // TOOLBAREND
    0xe2, 0x00, 0x00, 0x00,                                 // INTERFACEEND
    0x5c, 0x00, 0x20, 0x00, 0x04, 0x4d, 0x72, 0x20, 0x58,   // WRITEACCESS
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x42, 0x00, 0x02, 0x00, 0xe4, 0x04,                     // CODEPAGE
    0x9c, 0x00, 0x02, 0x00, 0x0e, 0x00                      // FNGROUPCOUNT
};
const ULONG ExcDummy_00::nMyLen = sizeof( ExcDummy_00::pMyData );

//-------------------------------------------------------- class ExcDummy_04x -
const BYTE      ExcDummy_040::pMyData[] = {
    0x13, 0x00, 0x02, 0x00, 0x00, 0x00,                     // PASSWORD
    0x3d, 0x00, 0x12, 0x00, 0xe0, 0x01, 0x5a, 0x00, 0xcf,   // WINDOW1
    0x3f, 0x4e, 0x2a, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x58, 0x02,
    0x40, 0x00, 0x02, 0x00, 0x00, 0x00,                     // BACKUP
    0x8d, 0x00, 0x02, 0x00, 0x00, 0x00,                     // HIDEOBJ
};
const ULONG ExcDummy_040::nMyLen = sizeof( ExcDummy_040::pMyData );

const BYTE      ExcDummy_041::pMyData[] = {
    0x0e, 0x00, 0x02, 0x00, 0x01, 0x00,                     // PRECISION
    0xda, 0x00, 0x02, 0x00, 0x00, 0x00                      // BOOKBOOL
};
const ULONG ExcDummy_041::nMyLen = sizeof( ExcDummy_041::pMyData );

//-------------------------------------------------------- class ExcDummy_02a -
const BYTE      ExcDummy_02a::pMyData[] = {
    0x0d, 0x00, 0x02, 0x00, 0x01, 0x00,                     // CALCMODE
    0x0c, 0x00, 0x02, 0x00, 0x64, 0x00,                     // CALCCOUNT
    0x0f, 0x00, 0x02, 0x00, 0x01, 0x00,                     // REFMODE
    0x11, 0x00, 0x02, 0x00, 0x00, 0x00,                     // ITERATION
    0x10, 0x00, 0x08, 0x00, 0xfc, 0xa9, 0xf1, 0xd2, 0x4d,   // DELTA
    0x62, 0x50, 0x3f,
    0x5f, 0x00, 0x02, 0x00, 0x01, 0x00                      // SAVERECALC
};
const ULONG ExcDummy_02a::nMyLen = sizeof( ExcDummy_02a::pMyData );

//-------------------------------------------------------- class ExcDummy_02b -
const BYTE      ExcDummy_02b::pMyData[] = {
    0x82, 0x00, 0x02, 0x00, 0x01, 0x00                      // GRIDSET
};
const ULONG ExcDummy_02b::nMyLen = sizeof( ExcDummy_02b::pMyData );

//-------------------------------------------------------- class ExcDummy_02c -
const BYTE      ExcDummy_02c::pMyData[] = {
    0x25, 0x02, 0x04, 0x00, 0x00, 0x00, 0xff, 0x00          // DEFAULTROWHEIGHT
};
const ULONG ExcDummy_02c::nMyLen = sizeof( ExcDummy_02c::pMyData );


//----------------------------------------------------------- class ExcRecord -

void ExcRecord::Save( XclExpStream& rStrm )
{
    SetRecId( GetNum() );
    SetRecSize( GetLen() );
    XclExpRecord::Save( rStrm );
}

void ExcRecord::SaveCont( XclExpStream& rStrm )
{
}

void ExcRecord::WriteBody( XclExpStream& rStrm )
{
    SaveCont( rStrm );
}


//--------------------------------------------------------- class ExcEmptyRec -

void ExcEmptyRec::Save( XclExpStream& rStrm )
{
}


UINT16 ExcEmptyRec::GetNum() const
{
    return 0;
}


ULONG ExcEmptyRec::GetLen() const
{
    return 0;
}



//------------------------------------------------------- class ExcRecordList -

ExcRecordList::~ExcRecordList()
{
    for( ExcRecord* pRec = First(); pRec; pRec = Next() )
        delete pRec;
}


void ExcRecordList::Save( XclExpStream& rStrm )
{
    for( ExcRecord* pRec = First(); pRec; pRec = Next() )
        pRec->Save( rStrm );
}



//--------------------------------------------------------- class ExcDummyRec -

void ExcDummyRec::Save( XclExpStream& rStrm )
{
    rStrm.Write( GetData(), GetLen() );     // raw write mode
}


UINT16 ExcDummyRec::GetNum( void ) const
{
    return 0x0000;
}



//------------------------------------------------------- class ExcBoolRecord -

ExcBoolRecord::ExcBoolRecord( SfxItemSet* pItemSet, USHORT nWhich, BOOL bDefault )
{
    bVal = pItemSet? ( ( const SfxBoolItem& ) pItemSet->Get( nWhich ) ).GetValue() : bDefault;
}


void ExcBoolRecord::SaveCont( XclExpStream& rStrm )
{
    rStrm << (UINT16)(bVal ? 0x0001 : 0x0000);
}


ULONG ExcBoolRecord::GetLen( void ) const
{
    return 2;
}




//--------------------------------------------------------- class ExcBof_Base -

ExcBof_Base::ExcBof_Base() :
    nRupBuild( 0x096C ),    // copied from Excel
    nRupYear( 0x07C9 )      // copied from Excel
{
}



//-------------------------------------------------------------- class ExcBof -

ExcBof::ExcBof( void )
{
    nDocType = 0x0010;
    nVers = 0x0500;
}


void ExcBof::SaveCont( XclExpStream& rStrm )
{
    rStrm << nVers << nDocType << nRupBuild << nRupYear;
}


UINT16 ExcBof::GetNum( void ) const
{
    return 0x0809;
}


ULONG ExcBof::GetLen( void ) const
{
    return 8;
}



//------------------------------------------------------------- class ExcBofW -

ExcBofW::ExcBofW( void )
{
    nDocType = 0x0005;
    nVers = 0x0500;
}


void ExcBofW::SaveCont( XclExpStream& rStrm )
{
    rStrm << nVers << nDocType << nRupBuild << nRupYear;
}



UINT16 ExcBofW::GetNum( void ) const
{
    return 0x0809;
}



ULONG ExcBofW::GetLen( void ) const
{
    return 8;
}



//-------------------------------------------------------------- class ExcEof -

UINT16 ExcEof::GetNum( void ) const
{
    return 0x000A;
}


ULONG ExcEof::GetLen( void ) const
{
    return 0;
}



//----------------------------------------------------- class ExcFngroupcount -

void ExcFngroupcount::SaveCont( XclExpStream& rStrm )
{
    rStrm << ( UINT16 ) 0x000E;     // copied from Excel
}


UINT16 ExcFngroupcount::GetNum( void ) const
{
    return 0x009C;
}


ULONG ExcFngroupcount::GetLen( void ) const
{
    return 2;
}



//--------------------------------------------------------- class ExcDummy_00 -

ULONG ExcDummy_00::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* ExcDummy_00::GetData( void ) const
{
    return pMyData;
}



//-------------------------------------------------------- class ExcDummy_04x -

ULONG ExcDummy_040::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* ExcDummy_040::GetData( void ) const
{
    return pMyData;
}




ULONG ExcDummy_041::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* ExcDummy_041::GetData( void ) const
{
    return pMyData;
}



//------------------------------------------------------------- class Exc1904 -

Exc1904::Exc1904( ScDocument& rDoc )
{
    Date* pDate = rDoc.GetFormatTable()->GetNullDate();
    bVal = pDate ? (*pDate == Date( 1, 1, 1904 )) : FALSE;
}


UINT16 Exc1904::GetNum( void ) const
{
    return 0x0022;
}



//------------------------------------------------------ class ExcBundlesheet -

ExcBundlesheetBase::ExcBundlesheetBase( RootData& rRootData, UINT16 nTab ) :
    nGrbit( rRootData.pDoc->IsVisible( nTab ) ? 0x0000 : 0x0001 ),
    nOwnPos( STREAM_SEEK_TO_END ),
    nStrPos( STREAM_SEEK_TO_END )
{
}


ExcBundlesheetBase::ExcBundlesheetBase() :
    nGrbit( 0x0000 ),
    nOwnPos( STREAM_SEEK_TO_END ),
    nStrPos( STREAM_SEEK_TO_END )
{
}


void ExcBundlesheetBase::UpdateStreamPos( XclExpStream& rStrm )
{
    rStrm.SetStreamPos( nOwnPos );
    rStrm << nStrPos;
}


UINT16 ExcBundlesheetBase::GetNum( void ) const
{
    return 0x0085;
}




ExcBundlesheet::ExcBundlesheet( RootData& rRootData, UINT16 nTab ) :
    ExcBundlesheetBase( rRootData, nTab )
{
    String sTabName;
    rRootData.pDoc->GetName( nTab, sTabName );
    DBG_ASSERT( sTabName.Len() < 256, "ExcBundlesheet::ExcBundlesheet - table name too long" );
    aName = ByteString( sTabName, *rRootData.pCharset );
}


void ExcBundlesheet::SaveCont( XclExpStream& rStrm )
{
    nOwnPos = rStrm.GetStreamPos();
    rStrm   << (UINT32) 0x00000000              // dummy (stream position of the sheet)
            << nGrbit;
    rStrm.WriteByteString( aName );             // 8 bit length, max 255 chars
}


ULONG ExcBundlesheet::GetLen() const
{
    return 7 + Min( aName.Len(), (xub_StrLen) 255 );
}


//--------------------------------------------------------- class ExcDummy_02 -

ULONG ExcDummy_02a::GetLen( void ) const
{
    return nMyLen;
}

const BYTE* ExcDummy_02a::GetData( void ) const
{
    return pMyData;
}
//--------------------------------------------------------- class ExcDummy_02 -

ULONG ExcDummy_02b::GetLen( void ) const
{
    return nMyLen;
}

const BYTE* ExcDummy_02b::GetData( void ) const
{
    return pMyData;
}
//--------------------------------------------------------- class ExcDummy_02 -

ULONG ExcDummy_02c::GetLen( void ) const
{
    return nMyLen;
}

const BYTE* ExcDummy_02c::GetData( void ) const
{
    return pMyData;
}


// ----------------------------------------------------------------------------

XclExpCountry::XclExpCountry( const XclExpRoot& rRoot ) :
    XclExpRecord( EXC_ID_COUNTRY, 4 )
{
    mnUICountry = static_cast< sal_uInt16 >(
        ::svx::ConvertLanguageToCountry( rRoot.GetUILanguage() ) );
    mnDocCountry = static_cast< sal_uInt16 >(
        ::svx::ConvertLanguageToCountry( rRoot.GetDocLanguage() ) );
}

void XclExpCountry::WriteBody( XclExpStream& rStrm )
{
    rStrm << mnUICountry << mnDocCountry;
}


//------------------------------------------------------------- class ExcNote -

ExcNote::ExcNote( const ScAddress aNewPos, const String& rText, RootData& rExcRoot ) :
    aPos( aNewPos )
{
    pText = new ByteString( rText, *rExcRoot.pCharset );
    nTextLen = (UINT16) Min( pText->Len(), (xub_StrLen) 0xFFFF );
}


ExcNote::~ExcNote()
{
    delete pText;
}


void ExcNote::Save( XclExpStream& rStrm )
{
    const sal_Char* pBuffer = pText->GetBuffer();
    UINT16          nLeft = nTextLen;
    BOOL            bFirstRun = TRUE;

    do
    {
        UINT16 nWriteLen = Min( nLeft, (UINT16) EXC_NOTE5_MAXTEXT );

        rStrm.StartRecord( 0x001C, 6 + nWriteLen );

        // first record: row, col, length of complete text
        // next records: -1, 0, length of current text segment
        if( bFirstRun )
            rStrm << (UINT16) aPos.Row() << (UINT16) aPos.Col() << nTextLen;
        else
            rStrm << (UINT16) 0xFFFF << (UINT16) 0 << nWriteLen;

        rStrm.Write( pBuffer, nWriteLen );
        pBuffer += nWriteLen;
        nLeft -= nWriteLen;
        bFirstRun = FALSE;

        rStrm.EndRecord();
    }
    while( nLeft );
}



//------------------------------------------------------------- class ExcCell -

ExcCell::ExcCell(
        const ScAddress rPos,
        const ScPatternAttr* pAttr,
        RootData& rRootData,
        const ULONG nAltNumForm ) :
    aPos( rPos )
{
    if( pPrgrsBar )
        pPrgrsBar->SetState( GetCellCount() );
    IncCellCount();

    DBG_ASSERT( rRootData.pCellMerging, "ExcCell::ExcCell - missing cell merging list" );

    if( !rRootData.pCellMerging->FindMergeBaseXF( aPos, mnXFId ) )
        mnXFId = rRootData.pER->GetXFBuffer().Insert( pAttr, nAltNumForm );

#ifdef DBG_UTIL
    _nRefCount++;
#endif
}


ExcCell::~ExcCell()
{
#ifdef DBG_UTIL
    _nRefCount--;
    DBG_ASSERT( _nRefCount >= 0, "*ExcCell::~ExcCell(): Das war mindestens einer zuviel!" );
#endif
}


sal_uInt32 ExcCell::GetXFId() const
{
    return mnXFId;
}


void ExcCell::SaveCont( XclExpStream& rStrm )
{
    if( pPrgrsBar )
        pPrgrsBar->SetState( GetCellCount() );
    IncCellCount();

    sal_uInt16 nXF = rStrm.GetRoot().GetXFBuffer().GetXFIndex( mnXFId );
    rStrm << ( UINT16 ) aPos.Row() << ( UINT16 ) aPos.Col() << nXF;
    SaveDiff( rStrm );
}


void ExcCell::SaveDiff( XclExpStream& rStrm )
{
}


ULONG ExcCell::GetLen() const
{
    return 6 + GetDiffLen();
}



//----------------------------------------------------------- class ExcNumber -

ExcNumber::ExcNumber(
        const ScAddress rPos,
        const ScPatternAttr* pAttr,
        RootData& rRootData,
        const double& rNewVal ) :
    ExcCell( rPos, pAttr, rRootData )
{
    fVal = rNewVal;
}


void ExcNumber::SaveDiff( XclExpStream& rStrm )
{
    rStrm << fVal;
}


UINT16 ExcNumber::GetNum( void ) const
{
    return 0x0203;
}


ULONG ExcNumber::GetDiffLen( void ) const
{
    return 8;
}



//---------------------------------------------------------- class ExcBoolerr -

ExcBoolerr::ExcBoolerr(
        const ScAddress rPos,
        const ScPatternAttr* pAttr,
        RootData& rRootData,
        UINT8 nValP,
        BOOL bIsError ) :
    // #73420# force to "no number format" if boolean value
    ExcCell( rPos, pAttr, rRootData, bIsError ? NUMBERFORMAT_ENTRY_NOT_FOUND : 0 ),
    nVal( bIsError ? nValP : (nValP != 0) ),
    bError( bIsError != 0 )
{
}


void ExcBoolerr::SaveDiff( XclExpStream &rStr )
{
    rStr << nVal << bError;
}


UINT16 ExcBoolerr::GetNum( void ) const
{
    return 0x0205;
}


ULONG ExcBoolerr::GetDiffLen( void ) const
{
    return 2;
}



//---------------------------------------------------------- class ExcRKMulRK -

ExcRKMulRK::ExcRKMulRK(
        const ScAddress rPos,
        const ScPatternAttr *pAttr,
        RootData& rRootData,
        sal_Int32 nValue ) :
    ExcCell( rPos, pAttr, rRootData )
{
    if( ExcCell::pPrgrsBar )
        ExcCell::pPrgrsBar->SetState( ExcCell::GetCellCount() );
    ExcCell::IncCellCount();

    ExcRKMulRKEntry* pNewCont = new ExcRKMulRKEntry;
    pNewCont->mnXFId = mnXFId;  // from ExcCell
    pNewCont->mnValue = nValue;
    maEntryList.Append( pNewCont );
}


ExcRKMulRK* ExcRKMulRK::Extend(
        const ScAddress rPos,
        const ScPatternAttr* pAttr,
        RootData& rRootData,
        sal_Int32 nValue )
{
    if( aPos.Row() == rPos.Row() && aPos.Col() + maEntryList.Count() == rPos.Col() )
    {// extendable
        if( ExcCell::pPrgrsBar )
            ExcCell::pPrgrsBar->SetState( ExcCell::GetCellCount() );
        ExcCell::IncCellCount();

        ExcRKMulRKEntry* pNewCont = new ExcRKMulRKEntry;

        if( !rRootData.pCellMerging->FindMergeBaseXF( rPos, pNewCont->mnXFId ) )
            pNewCont->mnXFId = rRootData.pER->GetXFBuffer().Insert( pAttr );
        pNewCont->mnValue = nValue;
        maEntryList.Append( pNewCont );
        return NULL;
    }
    else
    {// create new
        return new ExcRKMulRK( rPos, pAttr, rRootData, nValue );
    }
}


sal_uInt32 ExcRKMulRK::GetXFId() const
{
    ExcRKMulRKEntry* pLast = maEntryList.Last();
    return pLast ? pLast->mnXFId : XclExpXFBuffer::GetXFIdFromIndex( EXC_XF_DEFAULTCELL );
}


void ExcRKMulRK::SaveCont( XclExpStream& rStrm )
{
    if( ExcCell::pPrgrsBar )
        ExcCell::pPrgrsBar->SetState( ExcCell::GetCellCount() );

    ExcRKMulRKEntry* pCurr = maEntryList.First();
    DBG_ASSERT( pCurr, "ExcRKMulRK::SaveDiff - list empty" );
    if( !pCurr ) return;

    const XclExpXFBuffer& rXFBuffer = rStrm.GetRoot().GetXFBuffer();
    if( IsRK() )
    {
        sal_uInt16 nXF = rXFBuffer.GetXFIndex( pCurr->mnXFId );
        rStrm << (UINT16) aPos.Row() << (UINT16) aPos.Col() << nXF << pCurr->mnValue;
        ExcCell::IncCellCount();
    }
    else
    {
        UINT16  nLastCol = aPos.Col();
        rStrm << (UINT16) aPos.Row()  << nLastCol;
        while( pCurr )
        {
            sal_uInt16 nXF = rXFBuffer.GetXFIndex( pCurr->mnXFId );
            rStrm << nXF << pCurr->mnValue;
            pCurr = maEntryList.Next();
            ExcCell::IncCellCount();
            nLastCol++;
        }

        nLastCol--;
        rStrm << nLastCol;
    }
}


UINT16 ExcRKMulRK::GetNum( void ) const
{
    return IsRK() ? 0x027E : 0x00BD;
}


ULONG ExcRKMulRK::GetDiffLen( void ) const
{
    return IsRK() ? 4 : maEntryList.Count() * 6;
}



//------------------------------------------------------------ class ExcLabel -

ExcLabel::ExcLabel(
        const ScAddress rPos,
        const ScPatternAttr* pAttr,
        RootData& rRootData,
        const String& rText ) :
    ExcCell( rPos, pAttr, rRootData ),
    aText( rText, *rRootData.pCharset )
{
    nTextLen = (UINT16) Min( aText.Len(), (xub_StrLen) 0xFFFF );

    if( nTextLen > 255 )
    {
        nTextLen = 255;
        DBG_WARNING( "ExcLabel::ExcLabel(): String truncated" );
    }
}


ExcLabel::~ExcLabel()
{
}


void ExcLabel::SaveDiff( XclExpStream& rStrm )
{
    rStrm.WriteByteString( aText, nTextLen, TRUE );     // 16 bit length
}


UINT16 ExcLabel::GetNum( void ) const
{
    return 0x0204;
}


ULONG ExcLabel::GetDiffLen( void ) const
{
    return 2 + nTextLen;
}



//---------------------------------------------------------- class ExcRichStr

ExcRichStr::ExcRichStr( ExcCell& rExcCell, String& rText, const ScPatternAttr* pAttr,
                        const ScEditCell& rEdCell, RootData& rRoot, xub_StrLen nMaxChars ) :
    eBiff( rRoot.eDateiTyp )
{
    const EditTextObject*   p = rEdCell.GetData();
    XclExpHyperlink*& rpLastHlink = rRoot.pLastHlink;

    if( rpLastHlink )
    {
        delete rpLastHlink;
        rpLastHlink = NULL;
    }

    if( p )
    {
        ScEditEngineDefaulter& rEdEng = rRoot.pER->GetEditEngine();
        const BOOL          bOldMode = rEdEng.GetUpdateMode();
        rEdEng.SetUpdateMode( TRUE );
        {
        SfxItemSet*         pTmpItemSet = new SfxItemSet( rEdEng.GetEmptyItemSet() );

        if( !pAttr )
            pAttr = rRoot.pDoc->GetDefPattern();

        pAttr->FillEditItemSet( pTmpItemSet );
        rEdEng.SetDefaults( pTmpItemSet );
        }
        rEdEng.SetText( *p );

        const USHORT        nParCnt = rEdEng.GetParagraphCount();
        USHORT              nPar;
        USHORT              n;
        USHORT              nF = 0;
        USHORT              nListLen;
        xub_StrLen          nParPos = 0;
        xub_StrLen          nExcStartPos;
        String              aParText;
        const sal_Unicode   cParSep = 0x0A;
        ESelection          aSel;
        ScPatternAttr       aPatAttr( rRoot.pDoc->GetPool() );
        XclExpFontBuffer&   rFontList = rRoot.pER->GetFontBuffer();
        String              sURLList;
        BOOL                bMultipleHlink = FALSE;

        // first font is the cell font, following font changes are stored in richstring
        USHORT              nLastFontIndex = rFontList.Insert( *pAttr );

        for( nPar = 0 ; nPar < nParCnt ; )
        {
            aSel.nStartPara = aSel.nEndPara = nPar;
            nParPos = rText.Len();

            aParText = rEdEng.GetText( nPar );
            String aExcParText;

            if( aParText.Len() )
            {
//              if ( eBiff < Biff8 )
//                  aParText.Convert( CHARSET_SYSTEM, CHARSET_ANSI );

                SvUShorts aPosList;
                rEdEng.GetPortions( nPar, aPosList );
                nListLen = aPosList.Count();

                aSel.nStartPos = 0;
                for( n = 0 ; n < nListLen ; n++ )
                {
                    aSel.nEndPos = ( xub_StrLen ) aPosList.GetObject( n );
                    nExcStartPos = nParPos + aExcParText.Len();
                    aExcParText += aParText.Copy( aSel.nStartPos, aSel.nEndPos - aSel.nStartPos );

                    {
                        SfxItemSet  aItemSet( rEdEng.GetAttribs( aSel ) );
                        BOOL        bWasHLink = FALSE;

                        // detect hyperlinks, export single hyperlink, create note if multiple hyperlinks,
                        // export hyperlink text in every case
                        if( aSel.nEndPos == (aSel.nStartPos + 1) )
                        {
                            const SfxPoolItem*          pItem;

                            if( aItemSet.GetItemState( EE_FEATURE_FIELD, FALSE, &pItem ) == SFX_ITEM_ON )
                            {
                                const SvxFieldData*     pField = ((const SvxFieldItem*) pItem)->GetField();

                                if( pField && pField->ISA( SvxURLField ) )
                                {
                                    // create new excel hyperlink and add text to cell text
                                    const SvxURLField& rURLField = *((const SvxURLField*) pField);
                                    bWasHLink = TRUE;

                                    if( sURLList.Len() )
                                        sURLList += cParSep;
                                    sURLList += rURLField.GetURL();

                                    XclExpHyperlink* pNewHlink = new XclExpHyperlink( *rRoot.pER, rURLField );
                                    const String* pReprString = pNewHlink->GetRepr();
                                    if( pReprString )
                                    {
                                        aExcParText.Erase( aExcParText.Len() - 1 );
                                        aExcParText += *pReprString;
                                    }

                                    if( rpLastHlink )
                                    {
                                        bMultipleHlink = TRUE;
                                        delete rpLastHlink;
                                        rpLastHlink = NULL;
                                    }
                                    if( bMultipleHlink )
                                        delete pNewHlink;
                                    else
                                        rpLastHlink = pNewHlink;
                                }
                            }
                        }

                        aPatAttr.GetItemSet().ClearItem();
                        aPatAttr.GetFromEditItemSet( &aItemSet );

                        Font aFont;
                        aPatAttr.GetFont( aFont, SC_AUTOCOL_RAW );
                        if( bWasHLink )
                        {
                            aFont.SetColor( Color( COL_LIGHTBLUE ) );
                            aFont.SetUnderline( UNDERLINE_SINGLE );
                        }

                        UINT16 nFontIndex = rFontList.Insert( aFont );

                        if( nFontIndex > 255 && eBiff < Biff8 )
                            nFontIndex = 0;

                        if( nExcStartPos <= nMaxChars && (eBiff >= Biff8 || nF < 256) )
                        {
                            if( nLastFontIndex != nFontIndex )
                            {
                                DBG_ASSERT( nExcStartPos <= 0xFFFF, "*ExcRichStr::ExcRichStr(): Start pos to big!" );
                                aForms.Append( ( UINT16 ) nExcStartPos );
                                aForms.Append( nFontIndex );
                                nLastFontIndex = nFontIndex;
                                nF++;
                            }
                        }
                    }
                    aSel.nStartPos = aSel.nEndPos;
                }
                rText += aExcParText;
            }

            nPar++;
            if( nPar < nParCnt )
                rText += cParSep;
        }

        rEdEng.SetUpdateMode( bOldMode );

        if( bMultipleHlink && sURLList.Len() )
        {
            if( rRoot.sAddNoteText.Len() )
                (rRoot.sAddNoteText += cParSep) += cParSep;
            rRoot.sAddNoteText += sURLList;
        }

        // XF mit Umbruch auswaehlen?
        rExcCell.SetXFId( rRoot.pER->GetXFBuffer().Insert( pAttr, nParCnt > 1 ) );
    }
    else
    {
        rEdCell.GetString( rText );
//      if ( eBiff < Biff8 )
//          rText.Convert( CHARSET_SYSTEM, CHARSET_ANSI );
    }
}


ExcRichStr::~ExcRichStr()
{
}


void ExcRichStr::Write( XclExpStream& rStrm )
{
    UINT32 nEnd = (UINT32) GetFormCount() * 2;

    if( eBiff >= Biff8 )
    {
        rStrm.SetSliceSize( 4 );
        for( UINT32 nIndex = 0 ; nIndex < nEnd ; nIndex++ )
            rStrm << aForms.GetValue( nIndex );
    }
    else
    {
        rStrm.SetSliceSize( 2 );
        for( UINT32 nIndex = 0 ; nIndex < nEnd ; nIndex++ )
            rStrm << (UINT8) aForms.GetValue( nIndex );
    }
    rStrm.SetSliceSize( 0 );
}




//---------------------------------------------------------- class ExcRString -

ExcRString::ExcRString(
        const ScAddress aNewPos,
        const ScPatternAttr* pAttr,
        RootData& rRootData,
        const ScEditCell& rEdCell ) :
    ExcCell( aNewPos, pAttr, rRootData ),
    ExcRoot( &rRootData )
{
    pRichStr = new ExcRichStr( *this, aText, pAttr, rEdCell, rRootData, 255 );
    DBG_ASSERT( aText.Len() <= 0xFFFF, "*ExcRString::ExcRString(): String to long!" );
    nTextLen = (UINT16) Min( aText.Len(), (xub_StrLen) 255 );
}


ExcRString::~ExcRString()
{
    delete pRichStr;
}


void ExcRString::SaveDiff( XclExpStream& rStrm )
{
    rStrm.WriteByteString(
        ByteString( aText, *pExcRoot->pCharset ), nTextLen, TRUE ); // 16 bit length
    rStrm << (UINT8) pRichStr->GetFormCount();
    pRichStr->Write( rStrm );
}


UINT16 ExcRString::GetNum( void ) const
{
    return 0x00D6;
}


ULONG ExcRString::GetDiffLen( void ) const
{
    return 2 + nTextLen + 1 + pRichStr->GetByteCount();
}



//---------------------------------------------------------- class ExcFormula -

ExcFormula::ExcFormula(
        const ScAddress rPos,
        const ScPatternAttr* pAttr,
        RootData& rRootData,
        const ULONG nAltNumForm,
        const ScTokenArray& rTokArray,
        ExcArray** ppArray,
        ScMatrixMode eMM,
        ExcShrdFmla** ppShrdFmla,
        ExcArrays* pShrdFmlas,
        ScFormulaCell* pFormulaCell,
        ExcFmlaResultStr** pFormulaResult) :
    ExcCell( rPos, pAttr, rRootData, nAltNumForm ) ,
    pFCell(pFormulaCell)

{
    bShrdFmla = FALSE;
    EC_Codetype     eCodeType;
    ExcUPN*         pExcUPN;

    switch( eMM )
    {
        case MM_FORMULA:
            pExcUPN = new ExcUPN( TRUE, &rRootData, rTokArray, eCodeType, rPos );
            if( eCodeType == EC_ArrayFmla )
                break;
            else
            {
                delete pExcUPN;
                pExcUPN = NULL;
            }
            goto newchance;
        case MM_REFERENCE:
            pExcUPN = new ExcUPN( rTokArray, eCodeType );
            if( eCodeType == EC_ArrayFmla )
                break;
            else
            {
                delete pExcUPN;
                pExcUPN = NULL;
            }

            // no break here!
        default:
            newchance:
            pExcUPN = new ExcUPN( &rRootData, rTokArray, eCodeType, &rPos, FALSE, pShrdFmlas );
    }

    switch( eCodeType )
    {
        case EC_StdFmla:
            nFormLen = pExcUPN->GetLen();

            if( nFormLen > 0 )
            {
                pData = new sal_Char[ nFormLen ];
                memcpy( pData, pExcUPN->GetData(), nFormLen );
            }
            else
                pData = NULL;
            break;
        case EC_ArrayFmla:
            nFormLen = pExcUPN->GetLenWithShrdFmla();
            pExcUPN->GetShrdFmla( pData, nFormLen );

            if( pExcUPN->GetCode() && pExcUPN->GetLen() )
                *ppArray = new ExcArray( *pExcUPN, ( UINT8 ) rPos.Col(), rPos.Row() );
            else
                *ppArray = new ExcArray( ( UINT8 ) rPos.Col(), rPos.Row(), pExcUPN->GetArrayFormId() );
            break;
        case EC_ShrdFmla:
            if( ppShrdFmla && pExcUPN->GetShrdFmla( pData, nFormLen ) )
            {   // shared formula ref only
                bShrdFmla = TRUE;
                if( pExcUPN->IsFirstShrdFmla() )
                    // ... and create shared formula record the first time only
                    *ppShrdFmla = new ExcShrdFmla( pExcUPN->GetData(), pExcUPN->GetLen(), rPos );
                else
                    *ppShrdFmla = NULL;
            }
            else
            {
                pData = NULL;
                nFormLen = 0;
            }
            break;
    }

    delete pExcUPN;

    if(pFCell && (pFCell->GetFormatType() == NUMBERFORMAT_TEXT))
    {
        String sText;
        pFCell->GetString(sText);
        XclExpString aFormulaText;
        if ( rRootData.eDateiTyp < Biff8 )
        {
            aFormulaText.AssignByte(sText,*rRootData.pCharset);
            *pFormulaResult = new ExcFmlaResultStr(aFormulaText);
        }
        else if(sText.Len())
        {
            aFormulaText.Assign(sText);
            *pFormulaResult = new ExcFmlaResultStr(aFormulaText);
        }
        else
            *pFormulaResult = NULL;
    }
    else
    {
        *pFormulaResult = NULL;
    }


/*  if( ppShrdFmla && aExcUPN.GetShrdFmla( pData, nFormLen ) )
    {   // shared formula ref only
        if( aExcUPN.IsFirstShrdFmla() )
            // ... and create shared formula record the first time only
            *ppShrdFmla = new ExcShrdFmla( aExcUPN.GetData(), aExcUPN.GetLen(), rPos );
        else
            *ppShrdFmla = NULL;
    }
    else
    {
        nFormLen = aExcUPN.GetLen();

        if( nFormLen > 0 )
        {
//          if( pAltArrayFormula )
//              rpArray = new ExcArray( *pAltArrayFormula, ( UINT8 ) rPos.Col(), rPos.Row() );
            pData = new sal_Char[ nFormLen ];
            memcpy( pData, aExcUPN.GetData(), nFormLen );
        }
        else
            pData = NULL;

    }*/
}


ExcFormula::~ExcFormula()
{
    if( pData )
        delete[] pData;
}


void ExcFormula::SetTableOp( USHORT nCol, USHORT nRow )
{
    if( pData )
        delete[] pData;
    nFormLen = 5;
    pData = new sal_Char[ nFormLen ];
    pData[ 0 ] = 0x02;
    ShortToSVBT16( (UINT16) nRow, (BYTE*) &pData[ 1 ] );
    ShortToSVBT16( (UINT16) nCol, (BYTE*) &pData[ 3 ] );
}

void ExcFormula::SaveDiff( XclExpStream& rStrm )
{//                         grbit               chn
    UINT16      nGrBit = bShrdFmla? 0x000B : 0x0003;
    double fVal = 0.0;
    UINT16 nErrorCode = 0;
    String sText;


    if(pFCell)
    {
        switch(pFCell->GetFormatType())
        {
            case NUMBERFORMAT_NUMBER:
                if(!(nErrorCode = pFCell->GetErrCode()))
                {
                    fVal =  pFCell->GetValue();
                    rStrm << fVal;
                }
                else
                {
                    BYTE nByte = ScErrorCodeToExc(nErrorCode);
                    rStrm << (UINT8)0x02
                          << (UINT8)0
                          << (UINT8)nByte
                          << (UINT16)0 << (UINT8)0
                          << (UINT16)0xFFFF;
                }
                break;

            case NUMBERFORMAT_TEXT:
                pFCell->GetString(sText);
                if(sText.Len())
                     rStrm << (UINT8)0x00 << (UINT32)0 << (UINT8)0 << (UINT16)0xFFFF;
                else
                {
                    if ( rStrm.GetRoot().GetBiff() < xlBiff8 )
                         rStrm << (UINT8)0x00 << (UINT32)0 << (UINT8)0 << (UINT16)0xFFFF;
                    else
                    {
                        // empty formula result cells are represented by 0x03 in Biff8
                        rStrm << (UINT8)0x03 << (UINT32)0 << (UINT8)0 << (UINT16)0xFFFF;
                    }
                }
                break;

            case NUMBERFORMAT_LOGICAL:
                fVal =  pFCell->GetValue();
                rStrm << (UINT8)0x01
                      << (UINT8)0
                      << (UINT8)((fVal != 0) ? 0x01 : 0x00)
                      << (UINT16)0 << (UINT8)0
                      << (UINT16)0xFFFF;
                break;

            default:
                rStrm << fVal;
                break;
        }
    }

    rStrm << nGrBit << (UINT32)0x00000000 << nFormLen;

    rStrm.Write( pData, nFormLen );
}


UINT16 ExcFormula::GetNum( void ) const
{
    return 0x0006;
}


ULONG ExcFormula::GetDiffLen( void ) const
{
    return 16 + nFormLen;
}

BYTE ExcFormula::ScErrorCodeToExc(UINT16 nErrorCode)
{
    BYTE nRetVal;

    switch(nErrorCode)
    {
        case errIllegalFPOperation:     //503 to #DIV/0
            nRetVal = 0x07;
            break;
        case errNoValue :               //519 to #VALUE (wrong argument or operand)
            nRetVal = 0x0F;
            break;
        case errNoCode:                 //521 to #NULL
            nRetVal = 0x00;
            break;
        case errNoRef:                  //524 to #REF(cell referral not valid)
            nRetVal = 0x17;
            break;
        case errNoName:                 //525 to #NAME (does not recognise text)
            nRetVal = 0x1D;
            break;
        default:
            nRetVal = 0x0F;             // all others to #VALUE
            break;
    }
    return nRetVal;
}


//---------------------------------------------------- class ExcBlankMulblank -

ExcBlankMulblank::ExcBlankMulblank(
        const ScAddress rPos,
        const ScPatternAttr* pAttr,
        RootData& rRootData,
        UINT16 nCount,
        ExcTable& rExcTab ) :
    ExcCell( rPos, NULL, rRootData )
{
    bDummy = FALSE;

    nRecLen = 2 * nCount - 2;
    nLastCol = aPos.Col() + nCount - 1;
    bMulBlank = (nCount > 1);

    AddEntries( rPos, pAttr, rRootData, nCount, rExcTab );
    SetXFId( maCellList[ 0 ].mnXFId );  // store first XF in ExcCell base class
}


void ExcBlankMulblank::Add(
        const ScAddress rPos,
        const ScPatternAttr* pAttr,
        RootData& rRootData,
        UINT16 nCount,
        ExcTable& rExcTab )
{
    bDummy = FALSE;

    DBG_ASSERT( rPos.Col() == nLastCol + 1, "ExcBlankMulblank::Add - wrong address" );

    nRecLen += 2 * nCount;
    nLastCol += nCount;
    AddEntries( rPos, pAttr, rRootData, nCount, rExcTab );
    bMulBlank = (maCellList.size() > 1) || (!maCellList.empty() && (maCellList[ 0 ].mnCount > 1));
}


void ExcBlankMulblank::AddEntries(
        const ScAddress rPos,
        const ScPatternAttr* pAttr,
        RootData& rRootData,
        UINT16 nCount,
        ExcTable& rExcTab )
{
    DBG_ASSERT( nCount > 0, "ExcBlankMulblank::AddEntries - count==0!" );
    DBG_ASSERT( rPos.Col() + nCount <= MAXCOL + 1, "ExcBlankMulblank::AddEntries - column overflow" );

    ScAddress   aCurrPos( rPos );
    sal_uInt32  nCellXFId = rRootData.pER->GetXFBuffer().Insert( pAttr );
    sal_uInt16  nTmpCount = nCount;

    while( nTmpCount )
    {
        sal_uInt32 nMergeXFId;
        sal_uInt16 nMergeCount;
        if( rRootData.pCellMerging->FindMergeBaseXF( aCurrPos, nMergeXFId, nMergeCount ) )
        {
            nMergeCount = Min( nMergeCount, nTmpCount );
            Append( nMergeXFId, nMergeCount );
            nTmpCount -= nMergeCount;
            aCurrPos.IncCol( nMergeCount );
        }
        else
        {
            UINT16 nMergeCol;
            UINT16 nColCount = nTmpCount;

            if( rRootData.pCellMerging->FindNextMerge( aCurrPos, nMergeCol ) )
                nColCount = Min( (UINT16)(nMergeCol - aCurrPos.Col()), nTmpCount );

            if( nColCount )
            {
                Append( nCellXFId, nColCount );
                nTmpCount -= nColCount;
                aCurrPos.IncCol( nColCount );
            }
        }
    }

    // #98133# set default row formatting from last cell in the row
    if( !maCellList.empty() && (rPos.Col() + nCount > MAXCOL) )
    {
        rExcTab.SetDefRowXF( rPos.Row(), maCellList.back().mnXFId );
        if( maCellList.size() == 1 )
            bDummy = TRUE;
        else
            maCellList.pop_back();
    }
}


sal_uInt32 ExcBlankMulblank::GetXFId() const
{
    DBG_ASSERT( !maCellList.empty(), "ExcBlankMulblank::GetXF - list empty" );
    return maCellList.empty() ? mnXFId : maCellList.back().mnXFId;
}


void ExcBlankMulblank::SaveDiff( XclExpStream& rStrm )
{
    if( !bMulBlank ) return;

    const XclExpXFBuffer& rXFBuffer = rStrm.GetRoot().GetXFBuffer();
    UINT16 nLastCol = (UINT16) aPos.Col();
    XclExpBlankCellVec::const_iterator aBegin = maCellList.begin(), aIter = aBegin, aEnd = maCellList.end();
    for( ; aIter != aEnd; ++aIter )
    {
        sal_uInt16 nXF = rXFBuffer.GetXFIndex( aIter->mnXFId );
        sal_uInt16 nTmpCount = aIter->mnCount;

        if( aIter == aBegin )
            nTmpCount--;        // very first XF is saved in ExcCell::SaveCont()

        nLastCol += nTmpCount;
        while( nTmpCount-- )
            rStrm << nXF;
    }
    rStrm << nLastCol;
}


UINT16 ExcBlankMulblank::GetNum() const
{
    return bMulBlank ? 0x00BE : 0x0201;
}


ULONG ExcBlankMulblank::GetDiffLen() const
{
    return bMulBlank ? (nRecLen + 2) : 0;
}


void ExcBlankMulblank::Save( XclExpStream& r )
{
    if( !bDummy )
        ExcCell::Save( r );
}




//---------------------------------------------------- class ExcNameListEntry -

ExcNameListEntry::ExcNameListEntry() :
    pData( NULL ),
    nFormLen( 0 ),
    nTabNum( 0 ),
    nBuiltInKey( EXC_BUILTIN_UNKNOWN ),
    bDummy( FALSE )
{
}


ExcNameListEntry::ExcNameListEntry( RootData& rRootData, UINT16 nScTab, UINT8 nKey ) :
    pData( NULL ),
    nFormLen( 0 ),
    nTabNum( rRootData.pER->GetTabIdBuffer().GetXclTab( nScTab ) + 1 ),
    nBuiltInKey( nKey ),
    bDummy( FALSE )
{
}


ExcNameListEntry::~ExcNameListEntry()
{
    DeleteData();
}


void ExcNameListEntry::DeleteData()
{
    if( pData )
        delete[] pData;
    pData = NULL;
    nFormLen = 0;
}


void ExcNameListEntry::SetCode( const ExcUPN& rUPN )
{
    DeleteData();

    nFormLen = rUPN.GetLen();
    if( nFormLen )
    {
        pData = new UINT8[ nFormLen ];
        memcpy( pData, rUPN.GetData(), nFormLen );
    }
}


void ExcNameListEntry::SaveCont( XclExpStream& rStrm )
{
    if(rStrm.GetRoot().GetBiff() < xlBiff8)
    {
        rStrm   << (UINT16) EXC_NAME_BUILTIN    // grbit (built in only)
            << (UINT8)  0x00                // chKey (keyboard shortcut)
            << (UINT8)  0x01                // cch (string len)
            << nFormLen                     // cce (formula len)
            << nTabNum                      // set ixals = itab
            << nTabNum                      // itab (table index, 1-based)
            << (UINT32) 0x00000000          // cch
            << nBuiltInKey;                 // string
    }
    else
    {
        rStrm   << (UINT16) EXC_NAME_BUILTIN    // grbit (built in only)
            << (UINT8)  0x00                // chKey (keyboard shortcut)
            << (UINT8)  0x01                // cch (string len)
            << nFormLen                     // cce (formula len)
            << (UINT16) 0x0000              // ixals
            << nTabNum                      // itab (table index, 1-based)
            << (UINT32) 0x00000000          // cch
            << (UINT8)  0x00                // string grbit
            << nBuiltInKey;                 // string
    }
    rStrm.Write( pData, nFormLen );
}


UINT16 ExcNameListEntry::GetNum() const
{
    return 0x0018;
}


ULONG ExcNameListEntry::GetLen() const
{
    return 16 + nFormLen;
}



//------------------------------------------------------------- class ExcName -

void ExcName::Init( BOOL bHid, BOOL bBIn )
{
    eBiff = pExcRoot->eDateiTyp;
    bHidden = bHid;
    bBuiltIn = bBIn;
}


void ExcName::BuildFormula( const ScRange& rRange )
{   // build formula from range
    ScTokenArray        aArr;

    if( rRange.aStart == rRange.aEnd )
    {
        SingleRefData   aRef;
        aRef.InitAddress( rRange.aStart );
        aArr.AddSingleReference( aRef );
    }
    else
    {
        ComplRefData    aRef;
        aRef.InitRange( rRange );
        aArr.AddDoubleReference( aRef );
    }

    EC_Codetype         eDummy;
    SetCode( ExcUPN( pExcRoot, aArr, eDummy ) );
}


ExcName::ExcName( RootData& rRootData, ScRangeData* pRange ) :
        ExcRoot( &rRootData )
{
    Init();

    String aRangeName;
    pRange->GetName( aRangeName );

    // no PrintRanges, no PrintTitles
    if( SetBuiltInName( aRangeName, EXC_BUILTIN_PRINTAREA ) ||
        SetBuiltInName( aRangeName, EXC_BUILTIN_PRINTTITLES ) )
        return;

    SetName( aRangeName );
    const ScTokenArray* pTokArray = pRange->GetCode();
    if( pTokArray && pTokArray->GetLen() )
    {
        EC_Codetype eDummy;
        SetCode( ExcUPN( pExcRoot, *pTokArray, eDummy ) );
    }
}


ExcName::ExcName( RootData& rRootData, ScDBData* pArea ) :
        ExcRoot( &rRootData )
{
    Init();

    String aRangeName;
    pArea->GetName( aRangeName );

    if( IsBuiltInAFName( aRangeName, EXC_BUILTIN_AUTOFILTER ))
        return;

    SetUniqueName( aRangeName );

    ScRange aRange;
    pArea->GetArea( aRange );
    BuildFormula( aRange );
}


ExcName::ExcName( RootData& rRootData, const ScRange& rRange, const String& rName ) :
        ExcRoot( &rRootData )
{
    Init();
    SetUniqueName( rName );
    BuildFormula( rRange );
}


ExcName::ExcName( RootData& rRootData, const ScRange& rRange, UINT8 nKey, BOOL bHid ) :
        ExcNameListEntry( rRootData, rRange.aStart.Tab(), nKey ),
        ExcRoot( &rRootData )
{
    Init( bHid, TRUE );
    aName = sal_Unicode( nKey );
    BuildFormula( rRange );
}


void ExcName::SetName( const String& rRangeName )
{
    DBG_ASSERT( pExcRoot->pScNameList, "ExcName::SetName - missing name list" );
    ScRangeName& rNameList = *pExcRoot->pScNameList;
    aName = rRangeName;

    // insert dummy range name
    ScRangeData* pRangeData = new ScRangeData( pExcRoot->pDoc, aName, ScTokenArray() );
    if( !rNameList.Insert( pRangeData ) )
        delete pRangeData;
}


void ExcName::SetUniqueName( const String& rRangeName )
{
    DBG_ASSERT( pExcRoot->pScNameList, "ExcName::SetUniqueName - missing name list" );
    ScRangeName& rNameList = *pExcRoot->pScNameList;

    USHORT nPos;
    if( rNameList.SearchName( rRangeName, nPos ) )
    {
        String aNewName;
        sal_Int32 nAppendValue = 1;
        do
        {
            aNewName = rRangeName;
            aNewName += '_';
            aNewName += String::CreateFromInt32( nAppendValue++ );
        }
        while( rNameList.SearchName( aNewName, nPos ) );
        SetName( aNewName );
    }
    else
        SetName( rRangeName );
}


BOOL ExcName::SetBuiltInName( const String& rName, UINT8 nKey )
{
    if( XclTools::IsBuiltInName( nTabNum, rName, nKey ) )
    {
        nBuiltInKey = nKey;
        bDummy = TRUE;
        return TRUE;
    }
    return FALSE;
}

BOOL ExcName::IsBuiltInAFName( const String& rName, UINT8 nKey )
{
    if( XclTools::IsBuiltInName( nTabNum, rName, nKey ) ||
        (rName == ScGlobal::GetRscString( STR_DB_NONAME )))
    {
        bDummy = TRUE;
        return TRUE;
    }
    return FALSE;
}

void ExcName::SaveCont( XclExpStream& rStrm )
{
    UINT8   nNameLen = (UINT8) Min( aName.Len(), (xub_StrLen)255 );
    UINT16  nGrbit = (bHidden ? EXC_NAME_HIDDEN : 0) | (bBuiltIn ? EXC_NAME_BUILTIN : 0);
    if( !nFormLen )
        nGrbit |= EXC_NAME_VB | EXC_NAME_PROC;

    rStrm   << nGrbit                   // grbit
            << (BYTE) 0x00              // chKey
            << nNameLen                 // cch
            << nFormLen                 // cce
            << (UINT16) 0x0000          // ixals
            << nTabNum                  // itab
            << (UINT32) 0x00000000;     // cch...

    if ( eBiff < Biff8 )
        rStrm.WriteByteStringBuffer( ByteString( aName, *pExcRoot->pCharset ), nNameLen );
    else
    {
        XclExpUniString aUni( aName, EXC_STR_8BITLENGTH );
        aUni.WriteFlagField( rStrm );
        aUni.WriteBuffer( rStrm );
    }

    rStrm.Write( pData, nFormLen );
}


ULONG ExcName::GetLen() const
{   // only a guess for Biff8 (8bit/16bit unknown)
    return 14 + nFormLen + (eBiff < Biff8 ? 0 : 1) + Min( aName.Len(), (xub_StrLen)255 );
}



// ---- class XclBuildInName -----------------------------------------

XclBuildInName::XclBuildInName( RootData& rRootData, UINT16 nScTab, UINT8 nKey ) :
    ExcNameListEntry( rRootData, nScTab, nKey )
{
}


void XclBuildInName::CreateFormula( RootData& rRootData )
{
    if( aRL.Count() )
    {
        ExcUPN* pUPN = CreateExcUpnFromScRangeList( rRootData, aRL );
        SetCode( *pUPN );
        delete pUPN;
    }
    else
        bDummy = TRUE;
}


// ---- class XclPrintRange, class XclTitleRange ---------------------

XclPrintRange::XclPrintRange( RootData& rRootData, UINT16 nScTab ) :
        XclBuildInName( rRootData, nScTab, EXC_BUILTIN_PRINTAREA )
{
    if( rRootData.pDoc->HasPrintRange() )
    {
        UINT16 nCount = rRootData.pDoc->GetPrintRangeCount( nScTab );
        for( UINT16 nIx = 0 ; nIx < nCount ; nIx++ )
            Append( *rRootData.pDoc->GetPrintRange( nScTab, nIx ) );
    }
    CreateFormula( rRootData );
}




XclPrintTitles::XclPrintTitles( RootData& rRootData, UINT16 nScTab ) :
        XclBuildInName( rRootData, nScTab, EXC_BUILTIN_PRINTTITLES )
{
    UINT16 nXclTab = rRootData.pER->GetTabIdBuffer().GetXclTab( nScTab );
    const ScRange* pRange = rRootData.pDoc->GetRepeatColRange( nScTab );
    if( pRange )
        Append( ScRange( pRange->aStart.Col(), 0, nXclTab,
            pRange->aEnd.Col(), rRootData.nRowMax, nXclTab ) );
    pRange = rRootData.pDoc->GetRepeatRowRange( nScTab );
    if( pRange )
        Append( ScRange( 0, pRange->aStart.Row(), nXclTab,
            rRootData.nColMax, pRange->aEnd.Row(), nXclTab ) );
    CreateFormula( rRootData );
}



//--------------------------------------------------------- class ExcNameList -

ExcNameList::ExcNameList( RootData& rRootData ) :
    nFirstPrintRangeIx( 0 ),
    nFirstPrintTitleIx( 0 ),
    nFirstOtherNameIx( 0 )
{
    ScDocument&         rDoc = *rRootData.pDoc;
    XclExpTabIdBuffer&  rTabBuffer = rRootData.pER->GetTabIdBuffer();
    USHORT              nCount, nIndex;
    UINT16              nScTab, nTab, nExpIx;

    // print ranges and print titles, insert in table name sort order
    UINT16 nScTabCount = rTabBuffer.GetScTabCount();
    for( nTab = 0; nTab < nScTabCount; ++nTab )
    {
        nScTab = rTabBuffer.GetRealScTab( nTab ); // sorted -> real
        if( rTabBuffer.IsExportTable( nScTab ) )
            Append( new XclPrintRange( rRootData, nScTab ) );
    }
    nFirstPrintTitleIx = List::Count();
    for( nTab = 0; nTab < nScTabCount; ++nTab )
    {
        nScTab = rTabBuffer.GetRealScTab( nTab ); // sorted -> real
        if( rTabBuffer.IsExportTable( nScTab ) )
            Append( new XclPrintTitles( rRootData, nScTab ) );
    }
    nFirstOtherNameIx = List::Count();

    // named ranges
    ScRangeName* pRangeNames = rDoc.GetRangeName();
    DBG_ASSERT( pRangeNames, "ExcNameList::ExcNameList - missing range name list" );
    nCount = pRangeNames->GetCount();
    for( nIndex = 0; nIndex < nCount; nIndex++ )
    {
        ScRangeData* pData = (*pRangeNames)[ nIndex ];
        DBG_ASSERT( pData, "ExcNameList::ExcNameList - missing range name" );

        if ( !rRootData.bBreakSharedFormula || !pData->HasType( RT_SHARED ) )
        {   // no SHARED_FORMULA_... names if not needed
            ExcName* pExcName = new ExcName( rRootData, pData );
            if( pExcName->IsDummy() )
            {
                nExpIx = GetBuiltInIx( pExcName );
                delete pExcName;
            }
            else
                nExpIx = Append( pExcName );
            pData->SetExportIndex( nExpIx );
        }
    }

    // data base ranges
    ScDBCollection* pDBAreas = rDoc.GetDBCollection();
    DBG_ASSERT( pDBAreas, "ExcNameList::ExcNameList - missing db area list" );
    nCount = pDBAreas->GetCount();
    for( nIndex = 0; nIndex < nCount; nIndex++ )
    {
        ScDBData* pData = (*pDBAreas)[ nIndex ];
        DBG_ASSERT( pData, "ExcNameList::ExcNameList - missing db area" );

        ExcName* pExcName = new ExcName( rRootData, pData );
        if( pExcName->IsDummy() )
        {
            delete pExcName;
        }
        else
        {
            nExpIx = Append( pExcName );
            pData->SetExportIndex( nExpIx );
        }
    }

    maNextInsVec.resize( nScTabCount, Count() );
}


ExcNameList::~ExcNameList()
{
    for( ExcNameListEntry* pName = _First(); pName; pName = _Next() )
        delete pName;
}


UINT16 ExcNameList::Append( ExcNameListEntry* pName )
{
    DBG_ASSERT( pName, "ExcNameList::Append - missing ExcName" );

    BOOL bDelete = (pName->IsDummy() || (List::Count() >= 0xFFFE));
    if( bDelete )
        delete pName;
    else
        List::Insert( pName, LIST_APPEND );
    return bDelete ? 0xFFFF : (UINT16) List::Count();
}


void ExcNameList::InsertSorted( RootData& rRootData, ExcNameListEntry* pName, sal_uInt16 nScTab )
{
    // real -> sorted
    sal_uInt32 nSortIx = rRootData.pER->GetTabIdBuffer().GetSortedScTab( nScTab );
    List::Insert( pName, maNextInsVec[ nSortIx ] );
    for( sal_uInt32 nCount = maNextInsVec.size(); nSortIx < nCount; ++nSortIx )
        ++maNextInsVec[ nSortIx ];
}

UINT16 ExcNameList::GetBuiltInIx( const ExcNameListEntry* pName )
{
    DBG_ASSERT( pName, "ExcNameList::GetBuiltInIx - missing ExcName" );

    ULONG nFirstIx = 0;
    ULONG nLastIx = 0;

    switch( pName->GetBuiltInKey() )
    {
        case EXC_BUILTIN_PRINTAREA:
            nFirstIx = nFirstPrintRangeIx; nLastIx = nFirstPrintTitleIx;
        break;
        case EXC_BUILTIN_PRINTTITLES:
            nFirstIx = nFirstPrintTitleIx; nLastIx = nFirstOtherNameIx;
        break;
        default:
            return 0xFFFF;
    }

    for( ULONG nIndex = nFirstIx; nIndex < nLastIx; nIndex++ )
    {
        ExcNameListEntry* pEntry = _Get( nIndex );
        if( pEntry && (pEntry->GetTabIndex() == pName->GetTabIndex()) )
        {
            DBG_ASSERT( pEntry->GetBuiltInKey() == pName->GetBuiltInKey(),
                        "ExcNameList::GetBuiltInIx - builtin key mismatch" );
            return (UINT16)(nIndex + 1);
        }
    }
    return 0xFFFF;
}


void ExcNameList::Save( XclExpStream& rStrm )
{
    for( ExcNameListEntry* pName = _First(); pName; pName = _Next() )
        pName->Save( rStrm );
}



//------------------------------------------------------- class ExcDimensions -

ExcDimensions::ExcDimensions( BiffTyp eBiffP )
            : eBiff( eBiffP )
{
    nRwMic = nRwMac = nColMic = nColMac = 0;
}


ExcDimensions::ExcDimensions(  UINT16 nFirstCol, UINT16 nFirstRow,
    UINT16 nLastCol, UINT16 nLastRow, BiffTyp eBiffP )
            : eBiff( eBiffP )
{
    SetLimits( nFirstCol, nFirstRow, nLastCol, nLastRow );
}


void ExcDimensions::SetLimits( UINT16 nFirstCol, UINT16 nFirstRow,
    UINT16 nLastCol, UINT16 nLastRow )
{
    nRwMic = nFirstRow;
    nRwMac = nLastRow + 1;
    nColMic = nFirstCol;
    nColMac = nLastCol + 1;
}


void ExcDimensions::SaveCont( XclExpStream& rStrm )
{
    if ( eBiff < Biff8 )
        rStrm << nRwMic << nRwMac;
    else
        rStrm << (UINT32) nRwMic << (UINT32) nRwMac;
    rStrm << nColMic << nColMac << (UINT16) 0;
}


UINT16 ExcDimensions::GetNum( void ) const
{
    return 0x0200;
}


ULONG ExcDimensions::GetLen( void ) const
{
    return eBiff < Biff8 ? 10 : 14;
}



//--------------------------------------------------------- class ExcEOutline -

ExcEOutline::ExcEOutline( ScOutlineArray* pArray ) :
        pOLArray( pArray ),
        nCurrExcLevel( 0 ),
        bIsColl( FALSE )
{
    ScOutlineEntry* pEntry;

    for( UINT16 nLev = 0; nLev < SC_OL_MAXDEPTH; nLev++ )
    {
        pEntry = pArray ? pArray->GetEntryByPos( nLev, 0 ) : NULL;
        nEnd[ nLev ] = pEntry ? pEntry->GetEnd() : 0;
        bHidden[ nLev ] = FALSE;
    }

}


void ExcEOutline::Update( UINT16 nNum )
{
    if( !pOLArray )
        return;

    UINT16  nNewLevel;
    BOOL    bFound = pOLArray->FindTouchedLevel( nNum, nNum, nNewLevel );
    UINT16  nNewExcLevel = bFound ? nNewLevel + 1 : 0;
    UINT16  nLevel;

    if( nNewExcLevel >= nCurrExcLevel )
    {
        bIsColl = FALSE;
        for( nLevel = 0; nLevel < nNewExcLevel; nLevel++ )
            if( nEnd[ nLevel ] < nNum )
            {
                ScOutlineEntry* pEntry = pOLArray->GetEntryByPos( nLevel, nNum );
                if( pEntry )
                {
                    nEnd[ nLevel ] = pEntry->GetEnd();
                    bHidden[ nLevel ] = pEntry->IsHidden();
                }
            }
    }
    else
    {
        for( nLevel = nNewExcLevel; nLevel < nCurrExcLevel; nLevel++ )
            bIsColl |= bHidden[ nLevel ];
    }
    nCurrExcLevel = nNewExcLevel;
}



//------------------------------------------------------------ class ExcEGuts -

ExcEGuts::ExcEGuts( ScOutlineArray* pCol, ScOutlineArray* pRow )
{
    nRowLevel = nColLevel = 0;

    if( pCol )
        nColLevel = Min( pCol->GetDepth(), (UINT16) EXC_OUTLINE_MAX );
    if( pRow )
        nRowLevel = Min( pRow->GetDepth(), (UINT16) EXC_OUTLINE_MAX );
}


void ExcEGuts::SaveCont( XclExpStream& rStrm )
{
    rStrm   << (UINT16)(nRowLevel ? (12 * nRowLevel + 17) : 0)
            << (UINT16)(nColLevel ? (12 * nColLevel + 17) : 0)
            << (UINT16)(nRowLevel + (nRowLevel ? 1 : 0))
            << (UINT16)(nColLevel + (nColLevel ? 1 : 0));
}


UINT16 ExcEGuts::GetNum() const
{
    return 0x0080;
}


ULONG ExcEGuts::GetLen() const
{
    return 8;
}



//-------------------------------------------------------------- class ExcRow -

ExcRow::ExcRow( UINT16 nRow, UINT16 nTab, UINT16 nFCol, UINT16 nLCol,
                sal_uInt32 nXFId, ScDocument& rDoc, ExcEOutline& rOutline, ExcTable& rTab ) :
        nNum( nRow ),
        mnXFId( nXFId ),
        nOptions( 0x0000 ),
        rExcTab( rTab )
{
    BYTE    nRowOptions = rDoc.GetRowFlags( nRow, nTab );
    BOOL    bUserHeight = TRUEBOOL( nRowOptions & CR_MANUALSIZE );

    SetRange( nFCol, nLCol );
    SetHeight( rDoc.GetRowHeight( nRow, nTab ), bUserHeight );

    if( TRUEBOOL( nRowOptions & CR_HIDDEN ) )
        nOptions |= EXC_ROW_ZEROHEIGHT;

    rOutline.Update( nRow );

    nOptions |= EXC_ROW_LEVELFLAGS( rOutline.GetLevel() );
    if( rOutline.IsCollapsed() )
        nOptions |= EXC_ROW_COLLAPSED;
}


void ExcRow::SetRange( UINT16 nFCol, UINT16 nLCol )
{
    DBG_ASSERT( nFCol <= nLCol, "+ExcRow::SetRange(): First Col > Last Col!" );

    nFirstCol = nFCol;
    nLastCol = nLCol;
}


void ExcRow::SetHeight( UINT16 nNewHeight, BOOL bUser )
{
    if( nNewHeight == 0 )
    {
        nOptions |= EXC_ROW_ZEROHEIGHT;
        nHeight = EXC_ROW_VALZEROHEIGHT;
    }
    else
        nHeight = nNewHeight;

    if( bUser )
        nOptions |= EXC_ROW_UNSYNCED;       // user height
//  else            not usable in Aplix as described in bug #76250#
//      nHeight |= EXC_ROW_FLAGDEFHEIGHT;   // default height
}


void ExcRow::SaveCont( XclExpStream& rStrm )
{
    nOptions |= EXC_ROW_FLAGCOMMON;
    if( rExcTab.ModifyToDefaultRowXF( nNum, mnXFId ) )
        nOptions |= EXC_ROW_GHOSTDIRTY;

    sal_uInt16 nXF = rStrm.GetRoot().GetXFBuffer().GetXFIndex( mnXFId );
    rStrm   << nNum << nFirstCol << (UINT16)(nLastCol + 1)
            << nHeight << (UINT32)0 << nOptions << nXF;
}


UINT16 ExcRow::GetNum() const
{
    return 0x0208;
}


ULONG ExcRow::GetLen() const
{
    return 16;
}



//--------------------------------------------------------- class ExcRowBlock -

ExcRowBlock::ExcRowBlock()
{
    ppRows = new ExcRow* [ 32 ];
    nNext = 0;
}


ExcRowBlock::~ExcRowBlock()
{
    for( UINT16 nC = 0 ; nC < nNext ; nC++ )
        delete ppRows[ nC ];

    delete[] ppRows;
}


ExcRowBlock* ExcRowBlock::Append( ExcRow* pNewRow )
{
    DBG_ASSERT( nNext < 32, "ExcRowBlock::Append - overflow" );
    ppRows[ nNext ] = pNewRow;
    ++nNext;
    return (nNext == 32) ? new ExcRowBlock : NULL;
}


/*BOOL ExcRowBlock::SetDefXF( UINT16 nXF, UINT16 n )
{
    for( UINT16 nC = 0 ; nC < nNext ; nC++ )
    {
        if( ppRows[ nC ]->nNum == n )
        {
            ppRows[ nC ]->nXF = nXF;
            return TRUE;
        }
    }

    return FALSE;
}*/


/*void ExcRowBlock::SetDefXFs( DefRowXFs& r )
{
    for( UINT16 nC = 0 ; nC < nNext ; nC++ )
        r.ChangeXF( *ppRows[ nC ] );
}*/


void ExcRowBlock::Save( XclExpStream& rStrm )
{
    for( UINT16 nC = 0 ; nC < nNext ; nC++ )
        ppRows[ nC ]->Save( rStrm );
}


// ============================================================================

XclExpColinfo::XclExpColinfo(
        const XclExpRoot& rRoot, sal_uInt16 nScCol, sal_uInt16 nScTab, sal_uInt32 nXFId, ExcEOutline& rOutline ) :
    XclExpRecord( EXC_ID_COLINFO, 12 ),
    XclExpRoot( rRoot ),
    mnXFId( nXFId ),
    mnFirstXclCol( nScCol ),
    mnLastXclCol( nScCol ),
    mnWidth( GetWidth( nScCol, nScTab ) ),
    mnFlags( GetFlags( nScCol, nScTab, rOutline ) )
{
}

bool XclExpColinfo::Expand( sal_uInt16 nScCol, sal_uInt16 nScTab, sal_uInt32 nXFId, ExcEOutline& rOutline )
{
    if( (mnXFId == nXFId) &&
        (mnLastXclCol + 1 == nScCol) &&
        (mnWidth == GetWidth( nScCol, nScTab )) &&
        (mnFlags == GetFlags( nScCol, nScTab, rOutline )) )
    {
        ++mnLastXclCol;
        return true;
    }
    return false;
}

sal_uInt16 XclExpColinfo::GetWidth( sal_uInt16 nScCol, sal_uInt16 nScTab ) const
{
    return XclTools::GetXclColumnWidth( GetDoc().GetColWidth( nScCol, nScTab ), GetCharWidth() );
}

sal_uInt16 XclExpColinfo::GetFlags( sal_uInt16 nScCol, sal_uInt16 nScTab, ExcEOutline& rOutline ) const
{
    sal_uInt8 nScColFlags = GetDoc().GetColFlags( nScCol, nScTab );
    rOutline.Update( nScCol );

    sal_uInt16 nFlags = 0;
    ::set_flag( nFlags, EXC_COLINFO_HIDDEN, (nScColFlags & CR_HIDDEN) != 0 );
    ::set_flag( nFlags, EXC_COLINFO_COLLAPSED, rOutline.IsCollapsed() );
    ::insert_value( nFlags, rOutline.GetLevel(), 8, 3 );
    return nFlags;
}

void XclExpColinfo::WriteBody( XclExpStream& rStrm )
{
    sal_uInt16 nXF = rStrm.GetRoot().GetXFBuffer().GetXFIndex( mnXFId );
    rStrm << mnFirstXclCol << mnLastXclCol << mnWidth << nXF << mnFlags << sal_uInt16( 0 );
}


// ============================================================================
//------------------------------------------------------ class ExcExterncount -

ExcExterncount::ExcExterncount( RootData* pRD, const BOOL bTableNew ) :
    ExcRoot( pRD ),
    bTable( bTableNew )
{
}


void ExcExterncount::SaveCont( XclExpStream& rStrm )
{
    UINT16  nNumTabs = pExcRoot->pER->GetTabIdBuffer().GetXclTabCount();

    if( nNumTabs && bTable )
        nNumTabs--;

    rStrm << nNumTabs;
}


UINT16 ExcExterncount::GetNum( void ) const
{
    return 0x0016;
}


ULONG ExcExterncount::GetLen( void ) const
{
    return 2;
}



//------------------------------------------------------ class ExcExternsheet -

ExcExternsheet::ExcExternsheet( RootData* pExcRoot, const UINT16 nNewTabNum ) : ExcRoot( pExcRoot )
{
    DBG_ASSERT( pExcRoot->pDoc->HasTable( nNewTabNum ),
        "*ExcExternsheet::ExcExternsheet(): table not found" );

    pExcRoot->pDoc->GetName( nNewTabNum, aTabName );
    DBG_ASSERT( aTabName.Len() < 255,
        "*ExcExternsheet::ExcExternsheet(): table name too long" );
}


void ExcExternsheet::SaveCont( XclExpStream& rStrm )
{
    rStrm << ( UINT8 ) Min( (xub_StrLen)(aTabName.Len() ), (xub_StrLen) 255 ) << ( UINT8 ) 0x03;
    rStrm.WriteByteStringBuffer(
        ByteString( aTabName, *pExcRoot->pCharset ), 254 );     // max 254 chars (leading 0x03!)
}


UINT16 ExcExternsheet::GetNum( void ) const
{
    return 0x0017;
}


ULONG ExcExternsheet::GetLen( void ) const
{
    return 2 + Min( aTabName.Len(), (xub_StrLen) 254 );
}



//-------------------------------------------------- class ExcExternsheetList -

ExcExternsheetList::~ExcExternsheetList()
{
    for( ExcExternsheet* pSheet = _First(); pSheet; pSheet = _Next() )
        delete pSheet;
}


void ExcExternsheetList::Save( XclExpStream& rStrm )
{
    for( ExcExternsheet* pSheet = _First(); pSheet; pSheet = _Next() )
        pSheet->Save( rStrm );
}



//-------------------------------------------------------- class ExcExternDup -

ExcExternDup::ExcExternDup( ExcExterncount& rC, ExcExternsheetList& rL ) :
    rExtCnt( rC ), rExtSheetList( rL )
{
}


ExcExternDup::ExcExternDup( const ExcExternDup& r ) :
    rExtCnt( r.rExtCnt ), rExtSheetList( r.rExtSheetList )
{
}


void ExcExternDup::Save( XclExpStream& rStrm )
{
    rExtCnt.Save( rStrm );
    rExtSheetList.Save( rStrm );
}



//---------------------------------------------------------- class ExcWindow2 -

ExcWindow2::ExcWindow2( UINT16 nTab ) : nTable( nTab )
{
}


void ExcWindow2::SaveCont( XclExpStream& rStrm )
{
    BYTE pData[] = { 0xb6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    DBG_ASSERT( sizeof(pData) == GetLen(), "ExcWindow2::SaveCont: length mismatch" );
    if ( GetTable() == 0 )
        pData[1] |= 0x06;       // displayed and selected
    rStrm.Write( pData, GetLen() );
}


UINT16 ExcWindow2::GetNum( void ) const
{
    return 0x023E;
}


ULONG ExcWindow2::GetLen( void ) const
{
    return 10;
}



//-------------------------------------------------------- class ExcSelection -

void ExcSelection::SaveCont( XclExpStream& rStrm )
{
    rStrm   << nPane                // pane
            << nRow << nCol         // active cell
            << (UINT16) 0           // index in ref array
            << (UINT16) 1           // size of ref array
            << nRow << nRow         // ref array (activ cell only)
            << (UINT8) nCol << (UINT8) nCol;
}


UINT16 ExcSelection::GetNum( void ) const
{
    return 0x001D;
}


ULONG ExcSelection::GetLen( void ) const
{
    return 15;
}


// XclExpWsbool ===============================================================

XclExpWsbool::XclExpWsbool( RootData& rRootData ) :
    XclExpUInt16Record( EXC_ID_WSBOOL, EXC_WSBOOL_DEFAULTFLAGS )
{
    SfxItemSet* pItemSet = rRootData.pStyleSheetItemSet;
    if( pItemSet && (pItemSet->GetItemState( ATTR_PAGE_SCALETOPAGES, FALSE ) == SFX_ITEM_SET) )
        SetValue( GetValue() | EXC_WSBOOL_FITTOPAGE );
}

// XclExpWindowProtection ===============================================================

XclExpWindowProtection::XclExpWindowProtection(bool bValue) :
    XclExpBoolRecord(EXC_ID_WINDOWPROTECT,bValue)
{
}

// XclExpDocProtection ===============================================================

XclExpDocProtection::XclExpDocProtection(bool bValue) :
    XclExpBoolRecord(EXC_ID_PROTECT,bValue)
{
}

//------------------------------------------------------------ class ExcSetup -

ExcSetup::ExcSetup( RootData* pExcRoot ) :
    nPaperSize( 0 ),
    nScale( 100 ),
    nPageStart( 1 ),
    nGrbit( 0x0001 ),
    nHeaderMargin( 0 ),
    nFooterMargin( 0 )
{
    SfxStyleSheet*          pStSh = pExcRoot->pStyleSheet;

    if( pStSh )
    {
        SfxItemSet&         rSet = *pExcRoot->pStyleSheetItemSet;
        const SvxPageItem&  rItem = ( const SvxPageItem& ) rSet.Get( ATTR_PAGE );
        const BOOL          bLandscape = rItem.IsLandscape();

        nGrbit = bLandscape? 0 : 0x0002;    // !fLandscape / fLandscape

#define TWIPS_FROM_INCH( v )    ((sal_Int32)((v)*72.0*TWIPS_PER_POINT+0.5))
#define TWIPS_FROM_CM( v )      ((sal_Int32)((v)*72.0*TWIPS_PER_POINT/CM_PER_INCH+0.5))

        struct PAPER_SIZE
        {
            INT32                   nH;
            INT32                   nW;
        };
        static const                nAnzSizes = 42;
        static const PAPER_SIZE     pPS[ nAnzSizes ] =
        {
            { 0,                        0                           },  // undefined
            { TWIPS_FROM_INCH( 8.5 ),   TWIPS_FROM_INCH( 11 )       },  // Letter 8 1/2 x 11 in
            { TWIPS_FROM_INCH( 8.5 ),   TWIPS_FROM_INCH( 11 )       },  // Letter Small 8 1/2 x 11 in
            { TWIPS_FROM_INCH( 11 ),    TWIPS_FROM_INCH( 17 )       },  // Tabloid 11 x 17 in
            { TWIPS_FROM_INCH( 17 ),    TWIPS_FROM_INCH( 11 )       },  // Ledger 17 x 11 in
            { TWIPS_FROM_INCH( 8.5 ),   TWIPS_FROM_INCH( 14 )       },  // Legal 8 1/2 x 14 in
            { TWIPS_FROM_INCH( 5.5 ),   TWIPS_FROM_INCH( 8.5 )      },  // Statement 5 1/2 x 8 1/2 in
            { TWIPS_FROM_INCH( 7.25 ),  TWIPS_FROM_INCH( 10.5 )     },  // Executive 7 1/4 x 10 1/2 in
            { TWIPS_FROM_CM( 29.7 ),    TWIPS_FROM_CM( 42.0 )       },  // A3 297 x 420 mm
            { TWIPS_FROM_CM( 21.0 ),    TWIPS_FROM_CM( 29.7 )       },  // A4 210 x 297 mm
            { TWIPS_FROM_CM( 21.0 ),    TWIPS_FROM_CM( 29.7 )       },  // A4 Small 210 x 297 mm
            { TWIPS_FROM_CM( 14.8 ),    TWIPS_FROM_CM( 21.0 )       },  // A5 148 x 210 mm
            { TWIPS_FROM_CM( 25.0 ),    TWIPS_FROM_CM( 35.4 )       },  // B4 250 x 354
            { TWIPS_FROM_CM( 18.2 ),    TWIPS_FROM_CM( 25.7 )       },  // B5 182 x 257 mm
            { TWIPS_FROM_INCH( 8.5 ),   TWIPS_FROM_INCH( 13 )       },  // Folio 8 1/2 x 13 in
            { TWIPS_FROM_CM( 21.5 ),    TWIPS_FROM_CM( 27.5 )       },  // Quarto 215 x 275 mm
            { TWIPS_FROM_INCH( 10 ),    TWIPS_FROM_INCH( 14 )       },  // 10x14 in
            { TWIPS_FROM_INCH( 11 ),    TWIPS_FROM_INCH( 17 )       },  // 11x17 in
            { TWIPS_FROM_INCH( 8.5 ),   TWIPS_FROM_INCH( 11 )       },  // Note 8 1/2 x 11 in
            { TWIPS_FROM_INCH( 3.875 ), TWIPS_FROM_INCH( 8.875 )    },  // Envelope #9 3 7/8 x 8 7/8
            { TWIPS_FROM_INCH( 4.125 ), TWIPS_FROM_INCH( 9.5 )      },  // Envelope #10 4 1/8 x 9 1/2
            { TWIPS_FROM_INCH( 4.5 ),   TWIPS_FROM_INCH( 10.375 )   },  // Envelope #11 4 1/2 x 10 3/8
            { TWIPS_FROM_INCH( 4.03 ),  TWIPS_FROM_INCH( 11 )       },  // Envelope #12 4 \276 x 11
            { TWIPS_FROM_INCH( 14.5 ),  TWIPS_FROM_INCH( 11.5 )     },  // Envelope #14 5 x 11 1/2
            { 0,                        0                           },  // C size sheet
            { 0,                        0                           },  // D size sheet
            { 0,                        0                           },  // E size sheet
            { TWIPS_FROM_CM( 11.0 ),    TWIPS_FROM_CM( 22.0 )       },  // Envelope DL 110 x 220mm
            { TWIPS_FROM_CM( 16.2 ),    TWIPS_FROM_CM( 22.9 )       },  // Envelope C5 162 x 229 mm
            { TWIPS_FROM_CM( 32.4 ),    TWIPS_FROM_CM( 45.8 )       },  // Envelope C3  324 x 458 mm
            { TWIPS_FROM_CM( 22.9 ),    TWIPS_FROM_CM( 32.4 )       },  // Envelope C4  229 x 324 mm
            { TWIPS_FROM_CM( 11.4 ),    TWIPS_FROM_CM( 16.2 )       },  // Envelope C6  114 x 162 mm
            { TWIPS_FROM_CM( 11.4 ),    TWIPS_FROM_CM( 22.9 )       },  // Envelope C65 114 x 229 mm
            { TWIPS_FROM_CM( 25.0 ),    TWIPS_FROM_CM( 35.3 )       },  // Envelope B4  250 x 353 mm
            { TWIPS_FROM_CM( 17.6 ),    TWIPS_FROM_CM( 25.0 )       },  // Envelope B5  176 x 250 mm
            { TWIPS_FROM_CM( 17.6 ),    TWIPS_FROM_CM( 12.5 )       },  // Envelope B6  176 x 125 mm
            { TWIPS_FROM_CM( 11.0 ),    TWIPS_FROM_CM( 23.0 )       },  // Envelope 110 x 230 mm
            { TWIPS_FROM_INCH( 3.875 ), TWIPS_FROM_INCH( 7.5 )      },  // Envelope Monarch 3.875 x 7.5 in
            { TWIPS_FROM_INCH( 3.625 ), TWIPS_FROM_INCH( 6.5 )      },  // 6 3/4 Envelope 3 5/8 x 6 1/2 in
            { TWIPS_FROM_INCH( 14.875 ),TWIPS_FROM_INCH( 11 )       },  // US Std Fanfold 14 7/8 x 11 in
            { TWIPS_FROM_INCH( 8.5 ),   TWIPS_FROM_INCH( 12 )       },  // German Std Fanfold 8 1/2 x 12 in
            { TWIPS_FROM_INCH( 8.5 ),   TWIPS_FROM_INCH( 13 )       }   // German Legal Fanfold 8 1/2 x 13 in
        };

        UINT16              n = 0;
        const PAPER_SIZE*   pAct = pPS;
        Size                aSize( ( ( const SvxSizeItem& ) rSet.Get( ATTR_PAGE_SIZE ) ).GetSize() );
        const long          nH = bLandscape? aSize.nB : aSize.nA;
        const long          nW = bLandscape? aSize.nA : aSize.nB;

        long                nMaxDH = 50;
        long                nMinDH = -nMaxDH;
        long                nMaxDW = 80;
        long                nMinDW = -nMaxDW;
        long                nDH, nDW;

        nPaperSize = 0;

        while( nPaperSize < nAnzSizes )
        {
            nDH = pAct->nH - nH;
            nDW = pAct->nW - nW;
            if( nDH >= nMinDH && nDH <= nMaxDH && nDW >= nMinDW && nDW <= nMaxDW )
                break;
            nPaperSize++;
            pAct++;
        }

        if( nPaperSize >= nAnzSizes )
            nPaperSize = 0;     // default if size doesn't match Excel sizes

        nScale = ( UINT16 ) ( ( const SfxUInt16Item& ) rSet.Get( ATTR_PAGE_SCALE ) ).GetValue();
        nFitToPages = static_cast< sal_uInt16 >( ((const SfxUInt16Item&) rSet.Get( ATTR_PAGE_SCALETOPAGES )).GetValue() );

        nPageStart = ( UINT16 ) ( ( const SfxUInt16Item& ) rSet.Get( ATTR_PAGE_FIRSTPAGENO ) ).GetValue();

        const UINT16    nTab = pExcRoot->pER->GetScTab();

        if( nPageStart )
        {
            if( nTab == 0 || pExcRoot->pDoc->NeedPageResetAfterTab( nTab - 1 ) )
                nGrbit |= 0x0080;   // fUsePage
        }

        if( !( ( const SfxBoolItem& ) rSet.Get( ATTR_PAGE_TOPDOWN ) ).GetValue() )
            nGrbit |= 0x0001;   // ffLeftToRight

        // set the Comments/Notes to "At end of sheet" if Print Notes is true.
        // We don't currently support "as displayed on sheet". Thus this value
        // will be re-interpreted to "At end of sheet".
        if( ( ( const SfxBoolItem& ) rSet.Get( ATTR_PAGE_NOTES ) ).GetValue() )
            nGrbit |= 0x0220;   // fNotes

        const SfxItemSet& rHeaderSet = ((const SvxSetItem&) rSet.Get( ATTR_PAGE_HEADERSET )).GetItemSet();
        nHeaderMargin = ((const SvxULSpaceItem&) rHeaderSet.Get( ATTR_ULSPACE )).GetLower();
        const SfxItemSet& rFooterSet = ((const SvxSetItem&) rSet.Get( ATTR_PAGE_FOOTERSET )).GetItemSet();
        nFooterMargin = ((const SvxULSpaceItem&) rFooterSet.Get( ATTR_ULSPACE )).GetUpper();
    }
}


void ExcSetup::SaveCont( XclExpStream& rStrm )
{
    rStrm   << nPaperSize << nScale << nPageStart
            << (UINT16) 1 << nFitToPages                // FitWidth / FitHeight
            << nGrbit
            << ( UINT16 ) 0x012C << ( UINT16 ) 0x012C   // Res / VRes
            << XclTools::GetInchFromTwips( nHeaderMargin )
            << XclTools::GetInchFromTwips( nFooterMargin )
            << ( UINT16 ) 1;                            // num of copies
}


UINT16 ExcSetup::GetNum( void ) const
{
    return 0x00A1;
}


ULONG ExcSetup::GetLen( void ) const
{
    return 0x0022;
}



// Header/Footer ==============================================================

XclExpHeaderFooter::XclExpHeaderFooter(
        sal_uInt16 nRecId,
        RootData& rRootData,
        sal_uInt16 nHFSetWhichId,
        sal_uInt16 nHFTextWhichId ) :
    XclExpRecord( nRecId ),
    ExcRoot( &rRootData ),
    mbUnicode( rRootData.eDateiTyp >= Biff8 )
{
    SfxItemSet* pItemSet = rRootData.pStyleSheetItemSet;
    if( pItemSet )
    {
        const SvxSetItem& rSetItem = (const SvxSetItem&) pItemSet->Get( nHFSetWhichId );
        if( ((const SfxBoolItem&) rSetItem.GetItemSet().Get( ATTR_PAGE_ON )).GetValue() )
        {
            GetFormatString( maFormatString, rRootData, nHFTextWhichId );
            // set record size, for Biff8 only a prediction, assuming 8-bit string
            SetRecSize( mbUnicode ?
                3 + Min( static_cast< sal_uInt32 >( maFormatString.Len() ), 0xFFFFUL ) :
                1 + Min( static_cast< sal_uInt32 >( maFormatString.Len() ), 0xFFUL ) );
        }
    }
}

void XclExpHeaderFooter::GetFormatString( String& rString, RootData& rRootData, sal_uInt16 nWhich )
{
    SfxItemSet* pItemSet = rRootData.pStyleSheetItemSet;
    if( !pItemSet )
        return;

    const ScPageHFItem& rHFItem = (const ScPageHFItem&) pItemSet->Get( nWhich );

    XclExpHFConverter aHFConv( *rRootData.pER );
    rString = aHFConv.GenerateString( rHFItem.GetLeftArea(), rHFItem.GetCenterArea(), rHFItem.GetRightArea() );
}

void XclExpHeaderFooter::WriteBody( XclExpStream& rStrm )
{
    if( mbUnicode )
        // normal unicode string
        rStrm << XclExpUniString( maFormatString );
    else
        // 8 bit length, max 255 chars
        rStrm.WriteByteString( ByteString( maFormatString, *pExcRoot->pCharset ), 255 );
}

void XclExpHeaderFooter::Save( XclExpStream& rStrm )
{
    if( maFormatString.Len() )
        XclExpRecord::Save( rStrm );
}


// ----------------------------------------------------------------------------

XclExpHeader::XclExpHeader( RootData& rRootData ) :
    XclExpHeaderFooter( EXC_ID_HEADER, rRootData, ATTR_PAGE_HEADERSET, ATTR_PAGE_HEADERRIGHT )
{
}


// ----------------------------------------------------------------------------

XclExpFooter::XclExpFooter( RootData& rRootData ) :
    XclExpHeaderFooter( EXC_ID_FOOTER, rRootData, ATTR_PAGE_FOOTERSET, ATTR_PAGE_FOOTERRIGHT )
{
}


// ============================================================================
//----------------------------------------------------- class ExcPrintheaders -

ExcPrintheaders::ExcPrintheaders( SfxItemSet* p ) : ExcBoolRecord( p, ATTR_PAGE_HEADERS, TRUE )
{
}


UINT16 ExcPrintheaders::GetNum( void ) const
{
    return 0x002A;
}



//--------------------------------------------------- class ExcPrintGridlines -

ExcPrintGridlines::ExcPrintGridlines( SfxItemSet* p ) : ExcBoolRecord( p, ATTR_PAGE_GRID, TRUE )
{
}


UINT16 ExcPrintGridlines::GetNum( void ) const
{
    return 0x002B;
}



//---------------------------------------------------------- class ExcHcenter -

ExcHcenter::ExcHcenter( SfxItemSet* p ) : ExcBoolRecord( p, ATTR_PAGE_HORCENTER, FALSE )
{
}


UINT16 ExcHcenter::GetNum( void ) const
{
    return 0x0083;
}



//---------------------------------------------------------- class ExcVcenter -

ExcVcenter::ExcVcenter( SfxItemSet* p ) : ExcBoolRecord( p, ATTR_PAGE_VERCENTER, FALSE )
{
}


UINT16 ExcVcenter::GetNum( void ) const
{
    return 0x0084;
}



//---------------------------------------------------------------- AutoFilter -

UINT16 ExcFilterMode::GetNum() const
{
    return 0x009B;
}

ULONG ExcFilterMode::GetLen() const
{
    return 0;
}




ExcAutoFilterInfo::~ExcAutoFilterInfo()
{
}

void ExcAutoFilterInfo::SaveCont( XclExpStream& rStrm )
{
    rStrm << nCount;
}

UINT16 ExcAutoFilterInfo::GetNum() const
{
    return 0x009D;
}

ULONG ExcAutoFilterInfo::GetLen() const
{
    return 2;
}




ExcFilterCondition::ExcFilterCondition() :
        nType( EXC_AFTYPE_NOTUSED ),
        nOper( EXC_AFOPER_EQUAL ),
        fVal( 0.0 ),
        pText( NULL )
{
}

ExcFilterCondition::~ExcFilterCondition()
{
    if( pText )
        delete pText;
}

ULONG ExcFilterCondition::GetTextBytes() const
{
    return pText ? 1 + pText->GetBufferSize() : 0;
}

void ExcFilterCondition::SetCondition( UINT8 nTp, UINT8 nOp, double fV, String* pT )
{
    nType = nTp;
    nOper = nOp;
    fVal = fV;

    delete pText;
    pText = pT ? new XclExpUniString( *pT, EXC_STR_8BITLENGTH ) : NULL;
}

void ExcFilterCondition::Save( XclExpStream& rStrm )
{
    rStrm << nType << nOper;
    switch( nType )
    {
        case EXC_AFTYPE_DOUBLE:
            rStrm << fVal;
        break;
        case EXC_AFTYPE_STRING:
            DBG_ASSERT( pText, "ExcFilterCondition::Save() -- pText is NULL!" );
            rStrm << (UINT32)0 << (UINT8) pText->Len() << (UINT16)0 << (UINT8)0;
        break;
        case EXC_AFTYPE_BOOLERR:
            rStrm << (UINT8)0 << (UINT8)((fVal != 0) ? 1 : 0) << (UINT32)0 << (UINT16)0;
        break;
        default:
            rStrm << (UINT32)0 << (UINT32)0;
    }
}

void ExcFilterCondition::SaveText( XclExpStream& rStrm )
{
    if( nType == EXC_AFTYPE_STRING )
    {
        DBG_ASSERT( pText, "ExcFilterCondition::SaveText() -- pText is NULL!" );
        pText->WriteFlagField( rStrm );
        pText->WriteBuffer( rStrm );
    }
}




ExcAutoFilter::ExcAutoFilter( UINT16 nC ) :
        nCol( nC ),
        nFlags( 0 )
{
}

BOOL ExcAutoFilter::AddCondition( ScQueryConnect eConn, UINT8 nType, UINT8 nOp,
                                    double fVal, String* pText, BOOL bSimple )
{
    if( !aCond[ 1 ].IsEmpty() )
        return FALSE;

    UINT16 nInd = aCond[ 0 ].IsEmpty() ? 0 : 1;

    if( nInd == 1 )
        nFlags |= (eConn == SC_OR) ? EXC_AFFLAG_OR : EXC_AFFLAG_AND;
    if( bSimple )
        nFlags |= (nInd == 0) ? EXC_AFFLAG_SIMPLE1 : EXC_AFFLAG_SIMPLE2;

    aCond[ nInd ].SetCondition( nType, nOp, fVal, pText );
    return TRUE;
}

BOOL ExcAutoFilter::AddEntry( RootData& rRoot, const ScQueryEntry& rEntry )
{
    BOOL    bConflict = FALSE;
    String  sText;

    if( rEntry.pStr )
        sText.Assign( *rEntry.pStr );

    BOOL bLen = TRUEBOOL( sText.Len() );

    // empty/nonempty fields
    if( !bLen && (rEntry.nVal == SC_EMPTYFIELDS) )
        bConflict = !AddCondition( rEntry.eConnect, EXC_AFTYPE_EMPTY, EXC_AFOPER_NONE, 0.0, NULL, TRUE );
    else if( !bLen && (rEntry.nVal == SC_NONEMPTYFIELDS) )
        bConflict = !AddCondition( rEntry.eConnect, EXC_AFTYPE_NOTEMPTY, EXC_AFOPER_NONE, 0.0, NULL, TRUE );
    // other conditions
    else
    {
        double  fVal    = 0.0;
        ULONG   nIndex  = 0;
        BOOL    bIsNum  = bLen ? rRoot.pDoc->GetFormatTable()->IsNumberFormat( sText, nIndex, fVal ) : TRUE;
        String* pText   = bIsNum ? NULL : &sText;

        // top10 flags
        UINT16 nNewFlags = 0x0000;
        switch( rEntry.eOp )
        {
            case SC_TOPVAL:
                nNewFlags = (EXC_AFFLAG_TOP10 | EXC_AFFLAG_TOP10TOP);
            break;
            case SC_BOTVAL:
                nNewFlags = EXC_AFFLAG_TOP10;
            break;
            case SC_TOPPERC:
                nNewFlags = (EXC_AFFLAG_TOP10 | EXC_AFFLAG_TOP10TOP | EXC_AFFLAG_TOP10PERC);
            break;
            case SC_BOTPERC:
                nNewFlags = (EXC_AFFLAG_TOP10 | EXC_AFFLAG_TOP10PERC);
            break;
        }
        BOOL bNewTop10 = TRUEBOOL( nNewFlags & EXC_AFFLAG_TOP10 );

        bConflict = HasTop10() && bNewTop10;
        if( !bConflict )
        {
            if( bNewTop10 )
            {
                if( fVal < 0 )      fVal = 0;
                if( fVal >= 501 )   fVal = 500;
                nFlags |= (nNewFlags | (UINT16)(fVal) << 7);
            }
            // normal condition
            else
            {
                UINT8 nType = bIsNum ? EXC_AFTYPE_DOUBLE : EXC_AFTYPE_STRING;
                UINT8 nOper = EXC_AFOPER_NONE;

                switch( rEntry.eOp )
                {
                    case SC_EQUAL:          nOper = EXC_AFOPER_EQUAL;           break;
                    case SC_LESS:           nOper = EXC_AFOPER_LESS;            break;
                    case SC_GREATER:        nOper = EXC_AFOPER_GREATER;         break;
                    case SC_LESS_EQUAL:     nOper = EXC_AFOPER_LESSEQUAL;       break;
                    case SC_GREATER_EQUAL:  nOper = EXC_AFOPER_GREATEREQUAL;    break;
                    case SC_NOT_EQUAL:      nOper = EXC_AFOPER_NOTEQUAL;        break;
                }
                bConflict = !AddCondition( rEntry.eConnect, nType, nOper, fVal, pText );
            }
        }
    }
    return bConflict;
}

void ExcAutoFilter::SaveCont( XclExpStream& rStrm )
{
    rStrm << nCol << nFlags;
    aCond[ 0 ].Save( rStrm );
    aCond[ 1 ].Save( rStrm );
    aCond[ 0 ].SaveText( rStrm );
    aCond[ 1 ].SaveText( rStrm );
}

UINT16 ExcAutoFilter::GetNum() const
{
    return 0x009E;
}

ULONG ExcAutoFilter::GetLen() const
{
    return 24 + aCond[ 0 ].GetTextBytes() + aCond[ 1 ].GetTextBytes();
}




ExcAutoFilterRecs::ExcAutoFilterRecs( RootData& rRoot, UINT16 nTab ) :
        pFilterMode( NULL ),
        pFilterInfo( NULL )
{
    ScDBCollection& rDBColl = *rRoot.pDoc->GetDBCollection();

    // search for first DB-range with filter
    UINT16      nIndex  = 0;
    BOOL        bFound  = FALSE;
    BOOL        bAdvanced;
    ScDBData*   pData   = NULL;
    ScRange     aRange;
    ScRange     aAdvRange;
    while( (nIndex < rDBColl.GetCount()) && !bFound )
    {
        pData = rDBColl[ nIndex ];
        if( pData )
        {
            pData->GetArea( aRange );
            bAdvanced = pData->GetAdvancedQuerySource( aAdvRange );
            bFound = (aRange.aStart.Tab() == nTab) &&
                (pData->HasQueryParam() || pData->HasAutoFilter() || bAdvanced);
        }
        if( !bFound )
            nIndex++;
    }

    if( pData && bFound )
    {
        ScQueryParam    aParam;
        pData->GetQueryParam( aParam );

        ScRange aRange( aParam.nCol1, aParam.nRow1, aParam.nTab,
                        aParam.nCol2, aParam.nRow2, aParam.nTab );
        UINT16  nColCnt = aParam.nCol2 - aParam.nCol1 + 1;

        rRoot.pNameList->InsertSorted( rRoot, new ExcName( rRoot, aRange, EXC_BUILTIN_AUTOFILTER, TRUE ), nTab );

        // advanced filter
        if( bAdvanced )
        {
            // filter criteria, excel allows only same table
            if( aAdvRange.aStart.Tab() == nTab )
                rRoot.pNameList->InsertSorted( rRoot,
                    new ExcName( rRoot, aAdvRange, EXC_BUILTIN_CRITERIA ), nTab );

            // filter destination range, excel allows only same table
            if( !aParam.bInplace )
            {
                ScRange aDestRange( aParam.nDestCol, aParam.nDestRow, aParam.nDestTab );
                aDestRange.aEnd.IncCol( nColCnt - 1 );
                if( aDestRange.aStart.Tab() == nTab )
                    rRoot.pNameList->InsertSorted( rRoot,
                        new ExcName( rRoot, aDestRange, EXC_BUILTIN_EXTRACT ), nTab );
            }

            pFilterMode = new ExcFilterMode;
        }
        // AutoFilter
        else
        {
            BOOL    bConflict   = FALSE;
            BOOL    bContLoop   = TRUE;
            BOOL    bHasOr      = FALSE;
            UINT16  nFirstField = aParam.GetEntry( 0 ).nField;

            // create AUTOFILTER records for filtered columns
            for( UINT16 nEntry = 0; !bConflict && bContLoop && (nEntry < aParam.GetEntryCount()); nEntry++ )
            {
                const ScQueryEntry& rEntry  = aParam.GetEntry( nEntry );

                bContLoop = rEntry.bDoQuery;
                if( bContLoop )
                {
                    ExcAutoFilter* pFilter = GetByCol( rEntry.nField - aRange.aStart.Col() );

                    if( nEntry > 0 )
                        bHasOr |= (rEntry.eConnect == SC_OR);

                    bConflict = (nEntry > 1) && bHasOr;
                    if( !bConflict )
                        bConflict = (nEntry == 1) && (rEntry.eConnect == SC_OR) &&
                                    (nFirstField != rEntry.nField);
                    if( !bConflict )
                        bConflict = pFilter->AddEntry( rRoot, rEntry );
                }
            }

            // additional tests for conflicts
            for( ExcAutoFilter* pFilter = _First(); !bConflict && pFilter; pFilter = _Next() )
                bConflict = pFilter->HasCondition() && pFilter->HasTop10();

            if( bConflict )
                DeleteList();

            if( List::Count() )
                pFilterMode = new ExcFilterMode;
            pFilterInfo = new ExcAutoFilterInfo( nColCnt );
            AddObjRecs( rRoot, aRange.aStart, nColCnt );
        }
    }
}

ExcAutoFilterRecs::~ExcAutoFilterRecs()
{
    if( pFilterMode )
        delete pFilterMode;
    if( pFilterInfo )
        delete pFilterInfo;
    DeleteList();
}

void ExcAutoFilterRecs::DeleteList()
{
    for( ExcAutoFilter* pFilter = _First(); pFilter; pFilter = _Next() )
        delete pFilter;
    List::Clear();
}

ExcAutoFilter* ExcAutoFilterRecs::GetByCol( UINT16 nCol )
{
    ExcAutoFilter* pFilter;
    for( pFilter = _First(); pFilter; pFilter = _Next() )
        if( pFilter->GetCol() == nCol )
            return pFilter;
    pFilter = new ExcAutoFilter( nCol );
    Append( pFilter );
    return pFilter;
}

BOOL ExcAutoFilterRecs::IsFiltered( UINT16 nCol )
{
    ExcAutoFilter* pFilter;
    for( pFilter = _First(); pFilter; pFilter = _Next() )
        if( pFilter->GetCol() == nCol )
            return TRUE;
    return FALSE;
}

void ExcAutoFilterRecs::AddObjRecs( RootData& rRoot, const ScAddress& rPos, UINT16 nCols )
{
    ScAddress aAddr( rPos );
    for( UINT16 nObj = 0; nObj < nCols; nObj++ )
    {
        XclObjDropDown* pObj = new XclObjDropDown( *rRoot.pER, aAddr, IsFiltered( nObj ) );
        rRoot.pObjRecs->Add( pObj );
        aAddr.IncCol( 1 );
    }
}

void ExcAutoFilterRecs::Save( XclExpStream& rStrm )
{
    if( pFilterMode )
        pFilterMode->Save( rStrm );
    if( pFilterInfo )
        pFilterInfo->Save( rStrm );
    for( ExcAutoFilter* pFilter = _First(); pFilter; pFilter = _Next() )
        pFilter->Save( rStrm );
}



// ----------------------------------------------------------------------------

XclExpMargin::XclExpMargin( sal_Int32 nMargin, XclMarginType eSide ) :
    XclExpDoubleRecord( EXC_ID_UNKNOWN, 0.0 )
{
    switch( eSide )
    {
        case xlLeftMargin:      SetRecId( EXC_ID_LEFTMARGIN );      break;
        case xlRightMargin:     SetRecId( EXC_ID_RIGHTMARGIN );     break;
        case xlTopMargin:       SetRecId( EXC_ID_TOPMARGIN );       break;
        case xlBottomMargin:    SetRecId( EXC_ID_BOTTOMMARGIN );    break;
    }

    SetValue( static_cast< double >( Max( nMargin, 0L ) ) / EXC_TWIPS_PER_INCH );
}


// Manual page breaks =========================================================

XclExpPagebreaks::XclExpPagebreaks( RootData& rRootData, sal_uInt16 nScTab, XclPBOrientation eOrient ) :
    XclExpRecord( (eOrient == xlPBHorizontal) ? EXC_ID_HORPAGEBREAKS : EXC_ID_VERTPAGEBREAKS )
{
    sal_uInt8 nFlags;
    if( eOrient == xlPBHorizontal )
    {
        for( sal_uInt16 nIndex = 0; nIndex <= MAXROW; ++nIndex )
        {
            nFlags = rRootData.pDoc->GetRowFlags( nIndex, nScTab );
            if( nFlags & CR_MANUALBREAK )
                maPagebreaks.Append( nIndex );
        }
    }
    else
    {
        for( sal_uInt16 nIndex = 0; nIndex <= MAXCOL; ++nIndex )
        {
            nFlags = rRootData.pDoc->GetColFlags( nIndex, nScTab );
            if( nFlags & CR_MANUALBREAK )
                maPagebreaks.Append( nIndex );
        }
    }
    SetRecSize( 2 + 2 * maPagebreaks.Count() );
}

void XclExpPagebreaks::Save( XclExpStream& rStrm )
{
    if( !maPagebreaks.Empty() )
        XclExpRecord::Save( rStrm );
}

void XclExpPagebreaks::WriteBody( XclExpStream& rStrm )
{
    sal_uInt16 nCount = static_cast< sal_uInt16 >( Min( maPagebreaks.Count(), 0xFFFFUL ) );
    rStrm << nCount;
    for( sal_uInt16 nIndex = 0; nIndex < nCount; ++nIndex )
        rStrm << maPagebreaks.GetValue( nIndex );
}


// ----------------------------------------------------------------------------

XclExpPagebreaks8::XclExpPagebreaks8( RootData& rRootData, sal_uInt16 nScTab, XclPBOrientation eOrient ) :
    XclExpPagebreaks( rRootData, nScTab, eOrient ),
    mnRangeMax( (eOrient == xlPBHorizontal) ? rRootData.nColMax : rRootData.nRowMax )
{
    SetRecSize( 2 + 6 * maPagebreaks.Count() );
}

void XclExpPagebreaks8::WriteBody( XclExpStream& rStrm )
{
    sal_uInt16 nCount = static_cast< sal_uInt16 >( Min( maPagebreaks.Count(), 0xFFFFUL ) );
    rStrm << nCount;
    rStrm.SetSliceSize( 6 );
    for( sal_uInt16 nIndex = 0; nIndex < nCount; ++nIndex )
        rStrm << maPagebreaks.GetValue( nIndex ) << static_cast< sal_uInt16 >( 0 ) << mnRangeMax;
}


// ============================================================================


//------------------------ class ExcArray, class ExcArrays, class ExcShrdFmla -

void ExcArray::SetColRow( UINT8 nCol, UINT16 nRow, UINT32 nId )
{
    if( nId == 0xFFFFFFFF )
    {
        nID = nCol;
        nID <<= 16;
        nID += nRow;
        nID <<= 8;
    }
    else
        nID = nId;

    nFirstRow = nLastRow = nRow;
    nFirstCol = nLastCol = nCol;
}


void ExcArray::SaveCont( XclExpStream& rStrm )
{
//  rStrm   << nFirstRow << nLastRow << nFirstCol << nLastCol
//          << ( UINT8 ) 0 << nID << ( UINT8 ) 0xFE << nFormLen;
    rStrm   << nFirstRow << nLastRow << nFirstCol << nLastCol << ( UINT16 ) 0x0003 << ( UINT32 ) 0 << nFormLen;
                                                                // grbit            chn

    if( pData )
        rStrm.Write( pData, nFormLen );
}


ExcArray::ExcArray( const sal_Char* p, UINT16 n, UINT8 nCol, UINT16 nRow )
{
    SetColRow( nCol, nRow );

    if( p && n )
    {
        pData = new sal_Char[ n ];
        nFormLen = n;
        memcpy( pData, p, n );
    }
    else
    {
        pData = NULL;
        nFormLen = 0;
    }
}


ExcArray::ExcArray( const ExcUPN& rUPN, UINT8 nCol, UINT16 nRow )
{
    SetColRow( nCol, nRow );

    nFormLen = rUPN.GetLen();

    if( nFormLen )
    {
        pData = new sal_Char[ nFormLen ];
        memcpy( pData, rUPN.GetData(), nFormLen );
    }
    else
        pData = NULL;
}


ExcArray::ExcArray( UINT8 nCol, UINT16 nRow, UINT32 nId )
{
    SetColRow( nCol, nRow, nId );

    nFormLen = 0;
    pData = NULL;
}


ExcArray::~ExcArray()
{
    if( pData )
        delete [] pData;
}


UINT16 ExcArray::GetNum() const
{
    return 0x0221;
}


ULONG ExcArray::GetLen() const
{
    return nFormLen + 14;
}


BOOL ExcArray::AppendBy( const ExcArray& r )
{
    if( nID != r.nID )
        return FALSE;

    BOOL            bRet;

    const UINT16    nNewRow = nLastRow + 1;
    const UINT16    nNewCol = nLastCol + 1;
    const UINT16    nCol = r.nFirstCol;
    const UINT16    nRow = r.nFirstRow;

    if( nNewRow == nRow && nFirstCol <= nCol && nNewCol >= nCol )
    {
        nLastRow = nNewRow;

        if( nNewCol == nCol )
            nLastCol = ( UINT8 ) nNewCol;

        bRet = TRUE;
    }
    else if( nNewCol == nCol && nFirstRow <= nRow && nNewRow >= r.nLastRow )
    {
        nLastCol = ( UINT8 ) nNewCol;

        if( nNewRow == nRow )
            nLastRow = nNewRow;

        bRet = TRUE;
    }
    else
        bRet = nFirstRow <= nRow && nRow <= nLastRow && nFirstCol <= nCol && nCol <= nLastCol;
                    // = TRUE, when r intersects this

    return bRet;
}


BOOL ExcArray::AppendBy( UINT8 nFCol, UINT16 nFRow, UINT8 nLCol, UINT16 nLRow )
{
    if( nFCol != nFirstCol || nFRow != nFirstRow )
        return FALSE;

    BOOL    bRet;

    const UINT16    nNewRow = nLastRow + 1;
    const UINT16    nNewCol = nLastCol + 1;

    if( nNewRow <= nLRow && nFirstCol <= nFCol && nNewCol >= nLCol )
    {
        nLastRow = nNewRow;

        if( nNewCol == nFCol )
            nLastCol = ( UINT8 ) nNewCol;

        bRet = TRUE;
    }
    else if( nNewCol <= nLCol && nFirstRow <= nFRow && nNewRow >= nLRow )
    {
        nLastCol = ( UINT8 ) nNewCol;

        if( nNewRow == nFRow )
            nLastRow = nNewRow;

        bRet = TRUE;
    }
    else
        bRet = FALSE;

    return bRet;
}




ExcArrays::ExcArrays( void )
{
}


ExcArrays::~ExcArrays()
{
}


BOOL ExcArrays::Insert( ExcArray* p )
{
    ExcArray*   pAct = ( ExcArray* ) List::First();

    while( pAct )
    {
        if( pAct->AppendBy( *p ) )
            return FALSE;

        pAct = ( ExcArray* ) List::Next();
    }

    Append( p );

    return TRUE;
}


BOOL ExcArrays::Extend( UINT8 nStartCol, UINT16 nStartRow, UINT8 nEndCol, UINT16 nEndRow )
{
    ExcArray*   pAct = ( ExcArray* ) List::First();

    while( pAct )
    {
        if( pAct->AppendBy( nStartCol, nStartRow, nEndCol, nEndRow ) )
            return TRUE;

        pAct = ( ExcArray* ) List::Next();
    }

    return FALSE;
}



void ExcShrdFmla::SaveCont( XclExpStream& rStrm )
{
    rStrm << nFirstRow << nLastRow << nFirstCol << nLastCol << ( UINT16 ) 0x0000 << nFormLen;

    if( pData )
        rStrm.Write( pData, nFormLen );
}


ExcShrdFmla::ExcShrdFmla( const sal_Char* p, UINT16 n, const ScRange& r ) :
    ExcArray( p, n, ( UINT8 ) r.aStart.Col(), r.aStart.Row() )
{
}


ExcShrdFmla::~ExcShrdFmla()
{
}


UINT16 ExcShrdFmla::GetNum() const
{
    return 0x04BC;
}


ULONG ExcShrdFmla::GetLen() const
{
    return 10 + nFormLen;
}

//-----------------------------------------------------------------

void ExcFmlaResultStr::WriteBody( XclExpStream& rStrm )
{
    rStrm << maResultText;
}


ExcFmlaResultStr::ExcFmlaResultStr(const XclExpString& aFmlaText) :
    XclExpRecord( EXC_ID_STRING ),
    maResultText(aFmlaText)
{
}


ExcFmlaResultStr::~ExcFmlaResultStr()
{
}


//--------------------------- class XclExpTableOp, class XclExpTableOpManager -

XclExpTableOp::XclExpTableOp(
        ExcFormula& rFormula,
        const ScAddress& rColFirstPos,
        const ScAddress& rRowFirstPos,
        USHORT nNewMode ) :
    nMode( nNewMode ),
    nColInpCol( rColFirstPos.Col() ),
    nColInpRow( rColFirstPos.Row() ),
    nRowInpCol( rRowFirstPos.Col() ),
    nRowInpRow( rRowFirstPos.Row() ),
    bIsValid( FALSE )
{
    nFirstCol = nLastCol = nNextCol = rFormula.GetPosition().Col();
    nFirstRow = nLastRow = rFormula.GetPosition().Row();
    Append( &rFormula );
}

XclExpTableOp::~XclExpTableOp()
{
}

BOOL XclExpTableOp::IsAppendable( const ScAddress& rPos )
{
    return  ((rPos.Col() == nLastCol + 1) && (rPos.Row() == nFirstRow)) ||
            ((rPos.Col() == nNextCol) && (rPos.Row() == nLastRow + 1));
}

BOOL XclExpTableOp::CheckPosition(
    const ScAddress& rPos,
    const ScAddress& rFmlaPos,
    const ScAddress& rColFirstPos, const ScAddress& rColRelPos,
    const ScAddress& rRowFirstPos, const ScAddress& rRowRelPos,
    BOOL bMode2 )
{
    BOOL bRet = FALSE;

    if( ((nMode == 2) == bMode2) &&
        (rPos.Tab() == rFmlaPos.Tab()) &&
        (nColInpCol == rColFirstPos.Col()) &&
        (nColInpRow == rColFirstPos.Row()) &&
        (rPos.Tab() == rColFirstPos.Tab()) &&
        (rPos.Tab() == rColRelPos.Tab()) )
    {
        if( nMode == 0 )
        {
            bRet =  (rPos.Col() == rFmlaPos.Col()) &&
                    (nFirstRow == rFmlaPos.Row() + 1) &&
                    (nFirstCol == rColRelPos.Col() + 1) &&
                    (rPos.Row() == rColRelPos.Row());
        }
        else if( nMode == 1 )
        {
            bRet =  (nFirstCol == rFmlaPos.Col() + 1) &&
                    (rPos.Row() == rFmlaPos.Row()) &&
                    (rPos.Col() == rColRelPos.Col()) &&
                    (nFirstRow == rColRelPos.Row() + 1);
        }
        else if( nMode == 2 )
        {
            bRet =  (nFirstCol == rFmlaPos.Col() + 1) &&
                    (nFirstRow == rFmlaPos.Row() + 1) &&
                    (nFirstCol == rColRelPos.Col() + 1) &&
                    (rPos.Row() == rColRelPos.Row()) &&
                    (nRowInpCol == rRowFirstPos.Col()) &&
                    (nRowInpRow == rRowFirstPos.Row()) &&
                    (rPos.Tab() == rRowFirstPos.Tab()) &&
                    (rPos.Col() == rRowRelPos.Col()) &&
                    (nFirstRow == rRowRelPos.Row() + 1) &&
                    (rPos.Tab() == rRowRelPos.Tab());
        }
    }

    return bRet;
}

BOOL XclExpTableOp::CheckFirstPosition(
    const ScAddress& rPos,
    const ScAddress& rFmlaPos,
    const ScAddress& rColFirstPos, const ScAddress& rColRelPos,
    const ScAddress& rRowFirstPos, const ScAddress& rRowRelPos,
    BOOL bMode2, USHORT& rnMode )
{
    BOOL bRet = FALSE;

    if( (rPos.Tab() == rFmlaPos.Tab()) &&
        (rPos.Tab() == rColFirstPos.Tab()) &&
        (rPos.Tab() == rColRelPos.Tab()) )
    {
        if( bMode2 )
        {
            rnMode = 2;
            bRet =  (rPos.Col() == rFmlaPos.Col() + 1) &&
                    (rPos.Row() == rFmlaPos.Row() + 1) &&
                    (rPos.Col() == rColRelPos.Col() + 1) &&
                    (rPos.Row() == rColRelPos.Row()) &&
                    (rPos.Tab() == rRowFirstPos.Tab()) &&
                    (rPos.Col() == rRowRelPos.Col()) &&
                    (rPos.Row() == rRowRelPos.Row() + 1) &&
                    (rPos.Tab() == rRowRelPos.Tab());
        }
        else if( (rPos.Col() == rFmlaPos.Col()) &&
                (rPos.Row() == rFmlaPos.Row() + 1) &&
                (rPos.Col() == rColRelPos.Col() + 1) &&
                (rPos.Row() == rColRelPos.Row()) )
        {
            rnMode = 0;
            bRet = TRUE;
        }
        else if( (rPos.Col() == rFmlaPos.Col() + 1) &&
                (rPos.Row() == rFmlaPos.Row()) &&
                (rPos.Col() == rColRelPos.Col()) &&
                (rPos.Row() == rColRelPos.Row() + 1) )
        {
            rnMode = 1;
            bRet = TRUE;
        }
    }

    return bRet;
}

void XclExpTableOp::InsertCell( ExcFormula& rFormula )
{
    const ScAddress& rPos = rFormula.GetPosition();
    if( (rPos.Col() == nLastCol + 1) && (rPos.Row() == nFirstRow) )     // next cell of first row
    {
        nLastCol++;
        Append( &rFormula );
    }
    else if( (rPos.Col() == nNextCol) && (rPos.Row() == nLastRow + 1) ) // next cell of next row
    {
        nNextCol++;
        Append( &rFormula );

        if( nNextCol > nLastCol )   // next row is valid -> add to range
        {
            nLastRow++;
            nNextCol = nFirstCol;
        }
    }
}

void XclExpTableOp::UpdateCells()
{
    if( nMode == 0 )
        bIsValid =  (nColInpCol + 1 < nFirstCol) || (nColInpCol > nLastCol) ||
                    (nColInpRow < nFirstRow) || (nColInpRow > nLastRow);
    else if( nMode == 1 )
        bIsValid =  (nColInpCol < nFirstCol) || (nColInpCol > nLastCol) ||
                    (nColInpRow + 1 < nFirstRow) || (nColInpRow > nLastRow);
    else if( nMode == 2 )
        bIsValid =  ((nColInpCol + 1 < nFirstCol) || (nColInpCol > nLastCol) ||
                    (nColInpRow + 1 < nFirstRow) || (nColInpRow > nLastRow)) &&
                    ((nRowInpCol + 1 < nFirstCol) || (nRowInpCol > nLastCol) ||
                    (nRowInpRow + 1 < nFirstRow) || (nRowInpRow > nLastRow));

    if( !bIsValid ) return;

    for( ExcFormula* pFmla = _First(); pFmla; pFmla = _Next() )
    {
        const ScAddress& rPos = pFmla->GetPosition();
        if( (nFirstCol <= rPos.Col()) && (rPos.Col() <= nLastCol) &&
            (nFirstRow <= rPos.Row()) && (rPos.Row() <= nLastRow) )
            pFmla->SetTableOp( nFirstCol, nFirstRow );
    }
}

void XclExpTableOp::SaveCont( XclExpStream& rStrm )
{
    UINT16 nFlags = EXC_TABOP_CALCULATE;
    nFlags |= ((nMode == 2) ? EXC_TABOP_BOTH : ((nMode == 1) ? EXC_TABOP_ROW : 0x0000));
    rStrm   << (UINT16) nFirstRow << (UINT16) nLastRow
            << (UINT8) nFirstCol << (UINT8) nLastCol
            << nFlags;
    if( nMode == 2 )
        rStrm   << (UINT16) nRowInpRow << (UINT16) nRowInpCol
                << (UINT16) nColInpRow << (UINT16) nColInpCol;
    else
        rStrm   << (UINT16) nColInpRow << (UINT16) nColInpCol       // ref to col AND row stored in nColInp***
                << (UINT16) 0 << (UINT16) 0;
}

void XclExpTableOp::Save( XclExpStream& rStrm )
{
    if( bIsValid )
        ExcRecord::Save( rStrm );
}

UINT16 XclExpTableOp::GetNum() const
{
    return 0x0236;
}

ULONG XclExpTableOp::GetLen() const
{
    return 16;
}



// do not delete the records -> stored in and deleted by table record list
XclExpTableOpManager::~XclExpTableOpManager()
{
}

XclExpTableOp* XclExpTableOpManager::InsertCell( const ScTokenArray* pTokenArray, ExcFormula& rFormula )
{
    XclExpTableOp*  pTabOp = NULL;
    ScAddress       aFmlaPos;
    ScAddress       aColFirst;
    ScAddress       aColRel;
    ScAddress       aRowFirst;
    ScAddress       aRowRel;
    bool            bMode2;

    if( pTokenArray && XclTokenArrayHelper::GetMultipleOpRefs( aFmlaPos, aColFirst, aColRel, aRowFirst, aRowRel, bMode2, *pTokenArray ) )
    {
        const ScAddress& rPos = rFormula.GetPosition();

        pTabOp = _First();
        BOOL bFound = FALSE;
        while( pTabOp && !bFound )
        {
            bFound = pTabOp->IsAppendable( rPos );
            if( !bFound )
                pTabOp = _Next();
        }

        if( bFound )
        {
            if( pTabOp->CheckPosition( rPos, aFmlaPos, aColFirst, aColRel, aRowFirst, aRowRel, bMode2 ) )
                pTabOp->InsertCell( rFormula );
            pTabOp = NULL;
        }
        else
        {
            USHORT nMode;
            if( XclExpTableOp::CheckFirstPosition( rPos, aFmlaPos, aColFirst, aColRel, aRowFirst, aRowRel, bMode2, nMode ) )
            {
                pTabOp = new XclExpTableOp( rFormula, aColFirst, aRowFirst, nMode );
                List::Insert( pTabOp, (ULONG) 0 );
            }
        }
    }
    return pTabOp;
}

void XclExpTableOpManager::UpdateCells()
{
    for( XclExpTableOp* pTabOp = _First(); pTabOp; pTabOp = _Next() )
        pTabOp->UpdateCells();
}


/*************************************************************************
 *
 *  $RCSfile: xcl97rec.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: dr $ $Date: 2001-03-19 13:24:21 $
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

#ifndef _SVDPOOL_HXX //autogen wg. SdrItemPool
#include <svx/svdpool.hxx>
#endif
#ifndef _SDTAITM_HXX //autogen wg. SdrTextHorzAdjustItem, SdrTextVertAdjustItem
#include <svx/sdtaitm.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen wg. SdrTextObj
#include <svx/svdotext.hxx>
#endif
#ifndef _OUTLOBJ_HXX //autogen wg. OutlinerParaObject
#include <svx/outlobj.hxx>
#endif
#ifndef _EDITOBJ_HXX //autogen wg. EditTextObject
#include <svx/editobj.hxx>
#endif
#ifndef _SVDOOLE2_HXX //autogen wg. SdrOle2Obj
#include <svx/svdoole2.hxx>
#endif
#ifndef _IPOBJ_HXX //autogen wg. SvInPlaceObject
#include <so3/ipobj.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen wg. SvStorage
#include <so3/svstor.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen wg. SfxItemSet
#include <svtools/itemset.hxx>
#endif

#ifndef _TOOLS_SOLMATH_HXX      // DoubleToString()
#include <tools/solmath.hxx>
#endif
#ifndef _URLOBJ_HXX             // INetURLObject
#include <tools/urlobj.hxx>
#endif
#ifndef _ZFORMAT_HXX            // SvNumberformat
#include <svtools/zformat.hxx>
#endif
#ifndef SC_CELL_HXX             // ScFormulaCell
#include "cell.hxx"
#endif

#include "xcl97rec.hxx"
#include "xcl97esc.hxx"
#include "excupn.hxx"

#ifndef _SC_XCLEXPSTREAM_HXX
#include "XclExpStream.hxx"
#endif

#include "scitems.hxx"

#include <offmgr/app.hxx>
#include <offmgr/fltrcfg.hxx>
#include <svx/boxitem.hxx>
#include <svx/brshitem.hxx>
#include <vcl/bmpacc.hxx>

#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD
#include <svx/flditem.hxx>
#include <svx/msoleexp.hxx>

#include <svtools/useroptions.hxx>

#include <stdio.h>

#include "document.hxx"
#include "conditio.hxx"
#include "rangelst.hxx"
#include "stlpool.hxx"


// --- class XclSstList ----------------------------------------------

XclSstList::~XclSstList()
{
    for ( XclExpUniString* p = _First(); p; p = _Next() )
        delete p;
}


UINT32 XclSstList::Add( XclExpUniString* pStr )
{
    Insert( pStr, LIST_APPEND );
    return Count() - 1;
}


void XclSstList::Save( XclExpStream& rStrm )
{
    if ( !List::Count() ) return;

    const UINT32 nCstTotal = (UINT32) List::Count();
    SvMemoryStream aIsstInf( 8192 );
    UINT32 nPerBucket = nCstTotal;
    while ( nPerBucket > 0xFFFF )
    {   // just fit into 16bit
        nPerBucket /= 2;
    }
    const UINT16 nDsst = Max( (UINT16) 8, (UINT16) nPerBucket );
    UINT16 nStr = 0;

    // *** write the SST record ***

    rStrm.StartRecord( 0x00FC, 8 );     // fix len only, real length calculated by XclExpStream

    rStrm << nCstTotal << nCstTotal;            // Total, Unique
    for ( XclExpUniString* p = _First(); p; p = _Next() )
    {
        if ( nStr == 0 )
        {   // ISSTINF
            aIsstInf    << (UINT32) rStrm.GetStreamPos()
//2do: cb ?!? Offset into the SST record that points to where the bucket begins
                        << (UINT16) 0x0000      // cb
                        << (UINT16) 0x0000;     // reserved
        }
        p->Write( rStrm );
        if ( ++nStr == nDsst )
            nStr = 0;
    }

    rStrm.EndRecord();

    // *** write the EXTSST record ***

    rStrm.StartRecord( 0x00FF, 0 );     // real length calculated by XclExpStream

    rStrm << nDsst;
    rStrm.SetSliceLen( 8 ); // sizeof(UINT32) + sizeof(UINT16) + sizeof(UINT16);
    aIsstInf.Seek( STREAM_SEEK_TO_BEGIN );
    rStrm.CopyFromStream( aIsstInf );

    rStrm.EndRecord();
}



// --- class XclCrn --------------------------------------------------

void XclCrn::SaveCont( XclExpStream& rStrm )
{
    rStrm << (UINT8) nCol << (UINT8) nCol << nRow << nId;
    SaveDiff( rStrm );
}

UINT16 XclCrn::GetNum() const
{
    return 0x005A;
}

ULONG XclCrn::GetLen() const
{
    return 5 + GetDiffLen();
}



// --- class XclCrnDouble --------------------------------------------

XclCrnDouble::XclCrnDouble( UINT16 nC, UINT16 nR, double fV ) :
        XclCrn( nC, nR, EXC_CRN_DOUBLE ),
        fVal( fV )
{   }

void XclCrnDouble::SaveDiff( XclExpStream& rStrm )
{
    rStrm << fVal;
}

ULONG XclCrnDouble::GetDiffLen() const
{
    return 8;
}



// --- class XclCrnString ---------------------------------------------

XclCrnString::XclCrnString( UINT16 nC, UINT16 nR, const String& rTxt ) :
        XclCrn( nC, nR, EXC_CRN_STRING ),
        sText( rTxt )
{   }

void XclCrnString::SaveDiff( XclExpStream& rStrm )
{
    sText.Write( rStrm );
}

ULONG XclCrnString::GetDiffLen() const
{
    return sText.GetByteCount();
}



// --- class XclCrnBool ----------------------------------------------

XclCrnBool::XclCrnBool( UINT16 nC, UINT16 nR, BOOL b ) :
        XclCrn( nC, nR, EXC_CRN_BOOL ),
        nBool( b ? 1 : 0 )
{   }

void XclCrnBool::SaveDiff( XclExpStream& rStrm )
{
    rStrm << nBool << (UINT16)0 << (UINT32)0;
}

ULONG XclCrnBool::GetDiffLen() const
{
    return 8;
}



// --- class XclXct --------------------------------------------------

XclXct::~XclXct()
{
    for( XclCrn* pCrn = _First(); pCrn; pCrn = _Next() )
        delete pCrn;
}

BOOL XclXct::Exists( UINT16 nCol, UINT16 nRow )
{
    for( XclCrn* pCrn = _First(); pCrn; pCrn = _Next() )
        if( pCrn->IsAddress( nCol, nRow ) )
            return TRUE;
    return FALSE;
}

void XclXct::StoreCellRange( RootData& rRoot, const ScRange& rRange )
{
    ScDocument&         rDoc    = *rRoot.pDoc;
    SvNumberFormatter&  rFTab   = *rRoot.pFormTable;
    UINT16              nTab    = rRange.aStart.Tab();

    for( UINT16 nRow = rRange.aStart.Row(); nRow <= rRange.aEnd.Row(); nRow++ )
        for( UINT16 nCol = rRange.aStart.Col(); nCol <= rRange.aEnd.Col(); nCol++ )
            if( !Exists( nCol, nRow ) )
            {
                if( rDoc.HasValueData( nCol, nRow, nTab ) )
                {
                    ScAddress   aAddr( nCol, nRow, nTab );
                    double      fVal    = rDoc.GetValue( aAddr );
                    ULONG       nFormat = rDoc.GetNumberFormat( aAddr );
                    short       nType   = rFTab.GetType( nFormat );
                    BOOL        bIsBool = (nType == NUMBERFORMAT_LOGICAL);

                    if( !bIsBool && ((nFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0) &&
                         (rDoc.GetCellType( aAddr ) == CELLTYPE_FORMULA) )
                    {
                        ScFormulaCell* pCell = (ScFormulaCell*) rDoc.GetCell( aAddr );
                        if ( pCell )
                            bIsBool = (pCell->GetFormatType() == NUMBERFORMAT_LOGICAL);
                    }

                    if( bIsBool && ((fVal == 0.0) || (fVal == 1.0)) )
                        Append( new XclCrnBool( nCol, nRow, (fVal == 1.0) ) );
                    else
                        Append( new XclCrnDouble( nCol, nRow, fVal ) );
                }
                else
                {
                    String sText;
                    rDoc.GetString( nCol, nRow, nTab, sText );
                    Append( new XclCrnString( nCol, nRow, sText ) );
                }
            }
}

void XclXct::SaveCont( XclExpStream& rStrm )
{
    rStrm << (UINT16) List::Count() << nTabNum;
}

void XclXct::Save( XclExpStream& rStrm )
{
    ExcRecord::Save( rStrm );

    for( XclCrn* pCrn = _First(); pCrn; pCrn = _Next() )
        pCrn->Save( rStrm );
}

UINT16 XclXct::GetNum() const
{
    return 0x0059;
}

ULONG XclXct::GetLen() const
{
    return 4;
}



// --- class XclSupbook ----------------------------------------------

XclSupbook::XclSupbook( UINT16 nTabs ) :
        nTables( nTabs ),
        nLen( 4 ),
        bSelf( TRUE ),
        pExtNameList( NULL )
{   }

XclSupbook::XclSupbook( const String& rDocName ) :
        sDocName( rDocName ),
        bSelf( FALSE ),
        pExtNameList( NULL )
{
    INetURLObject aURLObj( rDocName );
    DBG_ASSERT( !aURLObj.HasError(), "XclSupbook::XclSupbook() - Corrupt external filename!" );

    sEncoded.Assign( aURLObj.getFSysPath( INetURLObject::FSYS_DOS ) );
    nLen = 2 + sEncoded.GetByteCount();
}

XclSupbook::~XclSupbook()
{
    for( XclXct* pXct = _First(); pXct; pXct = _Next() )
        delete pXct;

    if( pExtNameList )
        delete pExtNameList;
}

const XclExpUniString* XclSupbook::GetTableName( UINT16 nIndex ) const
{
    XclXct* pXct = _Get( nIndex );
    return pXct ? &pXct->GetTableName() : NULL;
}

UINT16 XclSupbook::AddTableName( const String& rTabName )
{
    DBG_ASSERT( !bSelf, "XclSupbook::AddTableName() - Don't insert table names here" );
    XclXct* pXct = new XclXct( rTabName );
    nLen += pXct->GetTableBytes();
    List::Insert( pXct, LIST_APPEND );

    UINT16 nTabNum = ( UINT16 ) List::Count() - 1;
    pXct->SetTableNum( nTabNum );
    return nTabNum;
}

void XclSupbook::StoreCellRange( RootData& rRoot, const ScRange& rRange, UINT16 nXct )
{
    XclXct* pXct = _Get( nXct );
    if( pXct )
        pXct->StoreCellRange( rRoot, rRange );
}

void XclSupbook::SaveCont( XclExpStream& rStrm )
{
    if ( bSelf )
        rStrm << nTables << (UINT8) 0x01 << (UINT8) 0x04;
    else
    {
        rStrm << (UINT16) List::Count();
        sEncoded.Write( rStrm );

        for( XclXct* pXct = _First(); pXct; pXct = _Next() )
            pXct->GetTableName().Write( rStrm );
    }
}

void XclSupbook::Save( XclExpStream& rStrm )
{
    ExcRecord::Save( rStrm );

    for( XclXct* pXct = _First(); pXct; pXct = _Next() )
        pXct->Save( rStrm );

    if( pExtNameList )
        pExtNameList->Save( rStrm );
}

UINT16 XclSupbook::GetNum() const
{
    return 0x01AE;
}

ULONG XclSupbook::GetLen() const
{
    return nLen;
}

UINT16 XclSupbook::GetAddinIndex( const String& rName )
{
    if( !pExtNameList )
        pExtNameList = new XclExternNameList;

    return pExtNameList->GetIndex( rName );
}


// --- class XclSupbookList ------------------------------------------

XclSupbookList::XclSupbookList( RootData* pRoot ) :
        ExcRoot( pRoot ),
        pSupbookBuffer( NULL ),
        pTableBuffer( NULL )
{
    ExcETabNumBuffer& rTabBuffer = *pRoot->pTabBuffer;

    UINT16  nScCnt  = rTabBuffer.GetScTabCount();
    UINT16  nExcCnt = rTabBuffer.GetExcTabCount();
    UINT16  nExtCnt = rTabBuffer.GetExternTabCount();

    nRefdCnt = nExcCnt + nExtCnt;
    if( !nRefdCnt )
        return;

    pSupbookBuffer = new UINT16[ nRefdCnt ];
    pTableBuffer = new UINT16[ nRefdCnt ];

    UINT16 nInd;
    for( nInd = 0; nInd < nRefdCnt; nInd++ )
        pSupbookBuffer[ nInd ] = pTableBuffer[ nInd ] = 0;

    // self-ref supbook at beginning of list
    Append( new XclSupbook( Max( nExcCnt, pRoot->nCodenames ) ) );
    for( nInd = 0; nInd < nExcCnt; nInd++ )
    {
        pSupbookBuffer[ nInd ] = 0;
        pTableBuffer[ nInd ] = nInd;
    }

    // add supbooks with external references
    for( nInd = 0; nInd < nScCnt; nInd++ )
        if( rTabBuffer.IsExternal( nInd ) )
            AddExt( nInd );
}

XclSupbookList::~XclSupbookList()
{
    for( XclSupbook* pBook = _First(); pBook; pBook = _Next() )
        delete pBook;
    if( pSupbookBuffer )
        delete[] pSupbookBuffer;
    if( pTableBuffer )
        delete[] pTableBuffer;
}

ULONG XclSupbookList::Append( XclSupbook* pBook )
{
    List::Insert( pBook, LIST_APPEND );
    return Count() - 1;
}

void XclSupbookList::AddExt( UINT16 nScTab )
{
    // find ext doc name or append new one, save position in pSupbookBuffer
    const String& rExtDoc = pExcRoot->pDoc->GetLinkDoc( nScTab );
    DBG_ASSERT( rExtDoc.Len(), "XclSupbookList::Add() - Missing external filename!" );

    XclSupbook* pBook;
    BOOL        bFound = FALSE;
    UINT16      nPos;

    for( ULONG nSB = 0; !bFound && (nSB < List::Count()); nSB++ )
    {
        pBook = _Get( nSB );
        if( pBook )
            bFound = (rExtDoc == pBook->GetName());
        if( bFound )
            nPos = ( UINT16 ) nSB;
    }
    if( !bFound )
    {
        pBook = new XclSupbook( rExtDoc );
        nPos = ( UINT16 ) Append( pBook );
    }
    UINT16 nInd = pExcRoot->pTabBuffer->GetExcTable( nScTab );
    DBG_ASSERT( nInd < nRefdCnt,
        "XclSupbookList::XclSupbookList() - Out of range!" );
    pSupbookBuffer[ nInd ] = nPos;

    // append new table name, save position in pTableBuffer
    nPos = pBook->AddTableName( pExcRoot->pDoc->GetLinkTab( nScTab ) );
    pTableBuffer[ nInd ] = nPos;
}

const XclExpUniString* XclSupbookList::GetDocumentName( UINT16 nExcTab )
{
    DBG_ASSERT( nExcTab < nRefdCnt, "XclSupbookList::GetDocumentName - out of range" );
    XclSupbook* pSupbook = GetSupbook( nExcTab );
    const XclExpUniString* pString = pSupbook ? &pSupbook->GetEncName() : NULL;
    return (pString && pString->GetLen()) ? pString : NULL;
}

const XclExpUniString* XclSupbookList::GetTableName( UINT16 nExcTab )
{
    DBG_ASSERT( nExcTab < nRefdCnt, "XclSupbookList::GetTableName - out of range" );
    XclSupbook* pSupbook = GetSupbook( nExcTab );
    return pSupbook ? pSupbook->GetTableName( pTableBuffer[ nExcTab ] ) : NULL;
}

void XclSupbookList::StoreCellRange( const ScRange& rRange )
{
    UINT16 nExcTab = pExcRoot->pTabBuffer->GetExcTable( rRange.aStart.Tab() );
    DBG_ASSERT( nExcTab < nRefdCnt, "XclSupbookList::StoreCellRange() - Out of range!" );

    XclSupbook* pBook = GetSupbook( nExcTab );
    if( pBook )
        pBook->StoreCellRange( *pExcRoot, rRange, pTableBuffer[ nExcTab ] );
}

void XclSupbookList::WriteXtiInfo( XclExpStream& rStrm, UINT16 nTabFirst, UINT16 nTabLast )
{
    if( (nTabFirst >= nRefdCnt) || (nTabLast >= nRefdCnt) )
        rStrm << (UINT16) 0 << (UINT16) 0xFFFF << (UINT16) 0xFFFF;
    else
    {
        UINT16  nSupb       = pSupbookBuffer[ nTabFirst ];
        BOOL    bConflict   = FALSE;

        // all tables in the same supbook?
        for( UINT16 nTab = nTabFirst + 1; !bConflict && (nTab <= nTabLast); nTab++ )
        {
            bConflict = (pSupbookBuffer[ nTab ] != nSupb);
            if( bConflict )
                nTabLast = nTab - 1;
        }

        rStrm << nSupb << pTableBuffer[ nTabFirst ] << pTableBuffer[ nTabLast ];
    }
}

void XclSupbookList::Save( XclExpStream& rStrm )
{
    for( XclSupbook* pBook = _First(); pBook; pBook = _Next() )
        pBook->Save( rStrm );
}



// --- class XclExternsheetList --------------------------------------

XclExternsheetList::XclExternsheetList( RootData* pRoot ) :
        ExcRoot( pRoot ),
        aSupbookList( pRoot )
{
    Find( 0, 0 );   // add dummy Xti to prevent an empty list
}

XclExternsheetList::~XclExternsheetList()
{
    for ( XclXti* pXTI = _First(); pXTI; pXTI = _Next() )
        delete pXTI;
}

UINT16 XclExternsheetList::Add( XclXti* pXti )
{
    Insert( pXti, LIST_APPEND );
    DBG_ASSERT( Count() == GetCount16(), "XclExternsheetList::Add: too much for Xcl" );
    return (GetCount16() - 1);
}

UINT16 XclExternsheetList::Find( UINT16 nTabFirst, UINT16 nTabLast )
{
    for( XclXti* pXTI = _First(); pXTI; pXTI = _Next() )
    {
        if( pXTI->GetTabFirst() == nTabFirst && pXTI->GetTabLast() == nTabLast )
            return GetVal16( List::GetCurPos() );
    }
    return Add( new XclXti( nTabFirst, nTabLast ) );
}

void XclExternsheetList::StoreCellCont( const SingleRefData& rRef )
{
    if( pExcRoot->pTabBuffer->IsExternal( rRef.nTab ) )
        aSupbookList.StoreCellRange( ScRange(
            rRef.nCol, rRef.nRow, rRef.nTab, rRef.nCol, rRef.nRow, rRef.nTab ) );
}

void XclExternsheetList::StoreCellRange( const SingleRefData& rRef1, const SingleRefData& rRef2 )
{
    for( UINT16 nTab = rRef1.nTab; nTab <= rRef2.nTab; nTab++ )
        if( pExcRoot->pTabBuffer->IsExternal( nTab ) )
            aSupbookList.StoreCellRange( ScRange(
                rRef1.nCol, rRef1.nRow, nTab, rRef2.nCol, rRef2.nRow, nTab ) );
}

void XclExternsheetList::SaveCont( XclExpStream& rStrm )
{
    UINT16 nCount16 = GetCount16();
    rStrm << nCount16;

    UINT16 nElem = 0;
    rStrm.SetSliceLen( XclXti::GetSize() );
    for( XclXti* pXti = _First(); pXti; pXti = _Next(), nElem++ )
        WriteXtiInfo( rStrm, *pXti );
    DBG_ASSERT( nElem == nCount16, "XclExternsheetList::SaveCont() - wrong XTI count" );
}

void XclExternsheetList::Save( XclExpStream& rStrm )
{
    aSupbookList.Save( rStrm );     // SUPBOOKs, XCTs, CRNs
    ExcRecord::Save( rStrm );       // EXTERNSHEET
}

UINT16 XclExternsheetList::GetNum() const
{
    return 0x0017;
}

ULONG XclExternsheetList::GetLen() const
{
    return 2 + GetCount16() * XclXti::GetSize();
}



// --- class XclMsodrawing_Base --------------------------------------

XclMsodrawing_Base::XclMsodrawing_Base( XclEscher& rEscher )
        :
        pEscher( &rEscher ),
        nStartPos( rEscher.GetEx()->GetLastOffsetMapPos() )
{
    // for safety's sake add this now
    nStopPos = GetEscherEx()->AddCurrentOffsetToMap();
    DBG_ASSERT( GetDataLen() == 0, "XclMsodrawing_Base ctor: do I really own that data?" );
}


XclMsodrawing_Base::~XclMsodrawing_Base()
{
}


void XclMsodrawing_Base::UpdateStopPos()
{
    if ( nStopPos )
        GetEscherEx()->ReplaceCurrentOffsetInMap( nStopPos );
    else
        nStopPos = GetEscherEx()->AddCurrentOffsetToMap();
}


ULONG XclMsodrawing_Base::GetDataLen() const
{
    if ( nStartPos < nStopPos )
    {
        XclEscherEx* pEx = GetEscherEx();
        return pEx->GetOffsetFromMap( nStopPos ) - pEx->GetOffsetFromMap( nStartPos );
    }
    DBG_ERRORFILE( "XclMsodrawing_Base::GetDataLen: position mismatch" );
    return 0;
}



// --- class XclMsodrawinggroup --------------------------------------

XclMsodrawinggroup::XclMsodrawinggroup( RootData& rRoot, UINT16 nEscherType )
        :
        XclMsodrawing_Base( *rRoot.pEscher )
{
    if ( nEscherType )
    {
        XclEscherEx* pEx = GetEscherEx();
        SvStream& rOut = pEx->GetStream();
        switch ( nEscherType )
        {
            case ESCHER_DggContainer :
            {   // per-document data
                pEx->OpenContainer( nEscherType );
//2do: stuff it with our own document defaults?
#if 0
                pEx->BeginCount();
                pEx->AddOpt( ... );
                pEx->EndCount( ESCHER_OPT, 3 );
#else
                BYTE pDummyOPT[] = {
                    0xBF, 0x00, 0x08, 0x00, 0x08, 0x00, 0x81, 0x01,
                    0x09, 0x00, 0x00, 0x08, 0xC0, 0x01, 0x40, 0x00,
                    0x00, 0x08
                };
                pEx->AddAtom( sizeof(pDummyOPT), ESCHER_OPT, 3, 3 );
                rOut.Write( pDummyOPT, sizeof(pDummyOPT) );
#endif
                BYTE pDummySplitMenuColors[] = {
                    0x0D, 0x00, 0x00, 0x08, 0x0C, 0x00, 0x00, 0x08,
                    0x17, 0x00, 0x00, 0x08, 0xF7, 0x00, 0x00, 0x10
                };
                pEx->AddAtom( sizeof(pDummySplitMenuColors), ESCHER_SplitMenuColors, 0, 4 );
                rOut.Write( pDummySplitMenuColors, sizeof(pDummySplitMenuColors) );
                pEx->CloseContainer();  // ESCHER_DggContainer
            }
            break;
        }
        UpdateStopPos();
    }
}


XclMsodrawinggroup::~XclMsodrawinggroup()
{
}


void XclMsodrawinggroup::SaveCont( XclExpStream& rStrm )
{
    DBG_ASSERT( GetEscherEx()->GetStreamPos() == GetEscherEx()->GetOffsetFromMap( nStartPos ),
        "XclMsodrawinggroup::SaveCont: Escher stream position mismatch" );
    rStrm.CopyFromStream( pEscher->GetStrm(), GetDataLen() );
}


UINT16 XclMsodrawinggroup::GetNum() const
{
    return 0x00EB;
}


ULONG XclMsodrawinggroup::GetLen() const
{
    return GetDataLen();
}



// --- class XclMsodrawing --------------------------------------

XclMsodrawing::XclMsodrawing( RootData& rRoot, UINT16 nEscherType )
        :
        XclMsodrawing_Base( *rRoot.pEscher )
{
    if ( nEscherType )
    {
        XclEscherEx* pEx = GetEscherEx();
        switch ( nEscherType )
        {
            case ESCHER_DgContainer :
            {   // per-sheet data
                pEx->OpenContainer( nEscherType );
                // open group shape container
                Rectangle aRect( 0, 0, 0, 0 );
                pEx->EnterGroup( &aRect );
            }
            break;
        }
        UpdateStopPos();
    }
}


XclMsodrawing::~XclMsodrawing()
{
}


void XclMsodrawing::SaveCont( XclExpStream& rStrm )
{
    DBG_ASSERT( GetEscherEx()->GetStreamPos() == GetEscherEx()->GetOffsetFromMap( nStartPos ),
        "XclMsodrawing::SaveCont: Escher stream position mismatch" );
    rStrm.CopyFromStream( pEscher->GetStrm(), GetDataLen() );
}


UINT16 XclMsodrawing::GetNum() const
{
    return 0x00EC;
}


ULONG XclMsodrawing::GetLen() const
{
    return GetDataLen();
}



// --- class XclObjList ----------------------------------------------

XclObjList::XclObjList( RootData& rRoot )
        :
        pMsodrawingPerSheet( new XclMsodrawing( rRoot, ESCHER_DgContainer ) )
{
}


XclObjList::~XclObjList()
{
    for ( XclObj* p = First(); p; p = Next() )
    {
        delete p;
    }
    delete pMsodrawingPerSheet;
}


UINT16 XclObjList::Add( XclObj* pObj )
{
    DBG_ASSERT( Count() < 0xFFFF, "XclObjList::Add: too much for Xcl" );
    if ( Count() < 0xFFFF )
    {
        Insert( pObj, LIST_APPEND );
        UINT16 nCnt = (UINT16) Count();
        pObj->SetId( nCnt );
        return nCnt;
    }
    else
    {
        delete pObj;
        return 0;
    }
}


void XclObjList::EndSheet()
{
    XclEscherEx* pEx = pMsodrawingPerSheet->GetEscherEx();
    //! close all Escher group shapes created since XclObjList ctor MSODRAWING
    while ( pEx->GetGroupLevel() )
    {
        pEx->LeaveGroup();
    }
    //! close ESCHER_DgContainer created by XclObjList ctor MSODRAWING
    pEx->CloseContainer();
}


void XclObjList::Save( XclExpStream& rStrm )
{
    //! Escher must be written, even if there are no objects
    pMsodrawingPerSheet->Save( rStrm );

    for ( XclObj* p = First(); p; p = Next() )
        p->Save( rStrm );
}



// --- class XclObj --------------------------------------------------

XclObj::XclObj( ObjType eType, RootData& rRoot )
            :
            pClientTextbox( NULL ),
            pTxo( NULL ),
            eObjType( eType ),
            nObjId(0),
            nGrbit( 0x6011 ),   // AutoLine, AutoFill, Printable, Locked
            bFirstOnSheet( rRoot.pObjRecs->Count() == 0 )
{
    //! first object continues the first MSODRAWING record
    if ( bFirstOnSheet )
        pMsodrawing = rRoot.pObjRecs->GetMsodrawingPerSheet();
    else
        pMsodrawing = new XclMsodrawing( rRoot );
}


XclObj::~XclObj()
{
    if ( !bFirstOnSheet )
        delete pMsodrawing;
    delete pClientTextbox;
    delete pTxo;
}


void XclObj::SetEscherShapeType( UINT16 nType )
{
//2do: what about the other defined ot... types?
    switch ( nType )
    {
        case ESCHER_ShpInst_Line :
            eObjType = otLine;
        break;
        case ESCHER_ShpInst_Rectangle :
        case ESCHER_ShpInst_RoundRectangle :
            eObjType = otRectangle;
        break;
        case ESCHER_ShpInst_Ellipse :
            eObjType = otOval;
        break;
        case ESCHER_ShpInst_Arc :
            eObjType = otArc;
        break;
        case ESCHER_ShpInst_TextBox :
            eObjType = otText;
        break;
        case ESCHER_ShpInst_PictureFrame :
            eObjType = otPicture;
        break;
        default:
            eObjType = otMsOffDrawing;
    }
}


void XclObj::SetText( RootData& rRoot, const SdrTextObj& rObj )
{
    DBG_ASSERT( !pClientTextbox, "XclObj::SetText: already set" );
    if ( !pClientTextbox )
    {
        pMsodrawing->UpdateStopPos();
        pClientTextbox = new XclMsodrawing( rRoot );
        pClientTextbox->GetEscherEx()->AddAtom( 0, ESCHER_ClientTextbox );  // TXO record
        pClientTextbox->UpdateStopPos();
        pTxo = new XclTxo( rObj );
    }
}


void XclObj::SaveCont( XclExpStream& rStrm )
{   // ftCmo subrecord
    DBG_ASSERT( eObjType != otUnknown, "XclObj::SaveCont: unknown type" );
    rStrm << (UINT16) ftCmo << (UINT16) 0x0012;
    rStrm << (UINT16) eObjType << nObjId << nGrbit
        << UINT32(0) << UINT32(0) << UINT32(0);
}


void XclObj::Save( XclExpStream& rStrm )
{
    // MSODRAWING record (msofbtSpContainer)
    if ( !bFirstOnSheet )
        pMsodrawing->Save( rStrm );

    // OBJ
    ExcRecord::Save( rStrm );
}


void XclObj::SaveTextRecs( XclExpStream& rStrm )
{
    // MSODRAWING record (msofbtClientTextbox)
    if ( pClientTextbox )
        pClientTextbox->Save( rStrm );
    // TXO and CONTINUE records
    if ( pTxo )
        pTxo->Save( rStrm );
}


UINT16 XclObj::GetNum() const
{   // the real OBJ
    return 0x005D;
}


ULONG XclObj::GetLen() const
{   // length of subrecord including ID and LEN
    return 22;
}



// --- class XclObjComment -------------------------------------------

XclObjComment::XclObjComment( RootData& rRoot, const ScAddress& rPos, const String& rStr )
            :
            XclObj( otComment, rRoot )
{
    nGrbit = 0;     // all off: AutoLine, AutoFill, Printable, Locked
    XclEscherEx* pEx = pMsodrawing->GetEscherEx();
    pEx->OpenContainer( ESCHER_SpContainer );
    pEx->AddShape( ESCHER_ShpInst_TextBox, SHAPEFLAG_HAVEANCHOR | SHAPEFLAG_HAVESPT );
    EscherPropertyContainer aPropOpt;
    aPropOpt.AddOpt( ESCHER_Prop_lTxid, 0 );                        // undocumented
    aPropOpt.AddOpt( ESCHER_Prop_FitTextToShape, 0x00080008 );      // bool field
    aPropOpt.AddOpt( 0x0158, 0x00000000 );                          // undocumented
    aPropOpt.AddOpt( ESCHER_Prop_fillColor, 0x08000050 );
    aPropOpt.AddOpt( ESCHER_Prop_fillBackColor, 0x08000050 );
    aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x00110010 );      // bool field
    aPropOpt.AddOpt( ESCHER_Prop_shadowColor, 0x00000000 );
    aPropOpt.AddOpt( ESCHER_Prop_fshadowObscured, 0x00030003 );     // bool field
    aPropOpt.AddOpt( ESCHER_Prop_fPrint, 0x000A0002 );              // bool field
    aPropOpt.Commit( pEx->GetStream() );

    XclEscherClientAnchor( rRoot.pDoc, rPos ).WriteData( *pEx );
    pEx->AddAtom( 0, ESCHER_ClientData );                       // OBJ record
    pMsodrawing->UpdateStopPos();
    //! Be sure to construct the MSODRAWING ClientTextbox record _after_ the
    //! base OBJ's MSODRAWING record Escher data is completed.
    pClientTextbox = new XclMsodrawing( rRoot );
    pClientTextbox->GetEscherEx()->AddAtom( 0, ESCHER_ClientTextbox );  // TXO record
    pClientTextbox->UpdateStopPos();
    pEx->CloseContainer();  // ESCHER_SpContainer
    // TXO
    pTxo = new XclTxo( rStr );
}


XclObjComment::~XclObjComment()
{
}


void XclObjComment::SaveCont( XclExpStream& rStrm )
{
    // ftCmo subrecord
    XclObj::SaveCont( rStrm );

#if 0
/*
    // ftNts subrecord, Note structure, where the hell is this documented?!?
    // seems like we don't need it ...
    rStrm << UINT16( ftNts ) << UINT16(0x0016)
        << UINT32(0) << UINT32(0) << UINT16(0)
        << UINT16(0xa000) << UINT16(0xcf24) << UINT16(0xf78e) << UINT16(0);
    if ( bFirstOnSheet )
        rStrm << UINT16(0x0010);
    else
        rStrm << UINT16(0x0158);
    rStrm << UINT16(0);
*/
#endif

    // ftEnd subrecord
    rStrm << UINT16(0) << UINT16(0);
}


void XclObjComment::Save( XclExpStream& rStrm )
{
    // content of this record
    XclObj::Save( rStrm );
    // second MSODRAWING record and TXO and CONTINUE records
    XclObj::SaveTextRecs( rStrm );
}


ULONG XclObjComment::GetLen() const
{   // length of all subrecords including IDs and LENs
    return XclObj::GetLen() /* + 4 + 0x0016 */ + 4;
}



// --- class XclObjDropDown ------------------------------------------

XclObjDropDown::XclObjDropDown( RootData& rRoot, const ScAddress& rPos, BOOL bFilt ) :
        XclObj( otComboBox, rRoot ),
        bIsFiltered( bFilt )
{
    SetLocked( TRUE );
    SetPrintable( FALSE );
    SetAutoFill( TRUE );
    SetAutoLine( FALSE );
    nGrbit |= 0x0100;   // undocumented
    XclEscherEx* pEx = pMsodrawing->GetEscherEx();
    pEx->OpenContainer( ESCHER_SpContainer );
    pEx->AddShape( ESCHER_ShpInst_HostControl, SHAPEFLAG_HAVEANCHOR | SHAPEFLAG_HAVESPT );
    EscherPropertyContainer aPropOpt;
    aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x01040104 ); // bool field
    aPropOpt.AddOpt( ESCHER_Prop_FitTextToShape, 0x00080008 );      // bool field
    aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x00010000 );      // bool field
    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x00080000 );     // bool field
    aPropOpt.AddOpt( ESCHER_Prop_fPrint, 0x000A0000 );              // bool field
    aPropOpt.Commit( pEx->GetStream() );

    XclEscherClientAnchor aAnchor( rRoot, 0x0001 );             // MsofbtClientAnchor
    aAnchor.SetDropDownPosition( rPos );
    aAnchor.WriteData( *pEx );

    pEx->AddAtom( 0, ESCHER_ClientData );                       // OBJ record
    pMsodrawing->UpdateStopPos();
    pEx->CloseContainer();  // ESCHER_SpContainer
}

XclObjDropDown::~XclObjDropDown()
{
}

void XclObjDropDown::SaveCont( XclExpStream& rStrm )
{
    // ftCmo subrecord
    XclObj::SaveCont( rStrm );

    // ftSbs subrecord - Scroll bars (dummy)
    rStrm   << (UINT16)0x000C << (UINT16)0x0014
            << (UINT32)0 << (UINT32)0 << (UINT32)0 << (UINT32)0 << (UINT32)0;

    // ftLbsData subrecord - Listbox data
    rStrm   << (UINT16)0x0013 << (UINT16)0x0010
            << (UINT32)0 << (UINT16)0 << (UINT16)0x0301 << (UINT16)0;
    rStrm   << (bIsFiltered ? (UINT16)0x000A : (UINT16)0x0002) << (UINT32)0;

    // ftEnd subrecord
    rStrm << (UINT16)0 << (UINT16)0;
}

ULONG XclObjDropDown::GetLen() const
{
    // length of all subrecords including IDs and LENs
    // (all from XclObj) + ftSbs + ftLbsData + ftEnd
    return XclObj::GetLen() + 24 + 20 + 4;
}



// --- class XclTxo --------------------------------------------------

XclTxo::XclTxo( const String& rStr )
            :
            aText( rStr ),
            nGrbit(0),
            nRot(0)
{
}


XclTxo::XclTxo( const SdrTextObj& rTextObj )
            :
            nGrbit(0),
            nRot(0)
{
    String aStr;
    const OutlinerParaObject* pParaObj = rTextObj.GetOutlinerParaObject();
    DBG_ASSERT( pParaObj, "XclTxo: no ParaObject" );
    if( pParaObj )
    {
        // alignment in graphic
//-/        SfxItemSet aItemSet( *rTextObj.GetItemPool(), SDRATTR_START, SDRATTR_END );
        const SfxItemSet& rItemSet = rTextObj.GetItemSet();
//-/        rTextObj.TakeAttributes( aItemSet, FALSE, FALSE );

        switch ( ((const SdrTextHorzAdjustItem&) (rItemSet.Get( SDRATTR_TEXT_HORZADJUST ))).GetValue() )
        {
            case SDRTEXTHORZADJUST_LEFT :
                nGrbit |= (1 << 1);
            break;
            case SDRTEXTHORZADJUST_CENTER :
                nGrbit |= (2 << 1);
            break;
            case SDRTEXTHORZADJUST_RIGHT :
                nGrbit |= (3 << 1);
            break;
            case SDRTEXTHORZADJUST_BLOCK :
                nGrbit |= (4 << 1);
            break;
        }

        switch ( ((const SdrTextVertAdjustItem&) (rItemSet.Get( SDRATTR_TEXT_VERTADJUST ))).GetValue() )
        {
            case SDRTEXTVERTADJUST_TOP :
                nGrbit |= (1 << 4);
            break;
            case SDRTEXTVERTADJUST_CENTER :
                nGrbit |= (2 << 4);
            break;
            case SDRTEXTVERTADJUST_BOTTOM :
                nGrbit |= (3 << 4);
            break;
            case SDRTEXTVERTADJUST_BLOCK :
                nGrbit |= (4 << 4);
            break;
        }
        // the text
        const EditTextObject& rEditObj = pParaObj->GetTextObject();
        USHORT nPara = rEditObj.GetParagraphCount();
        for( USHORT j = 0; j < nPara; j++ )
        {
            aStr += rEditObj.GetText( j );
            if ( j < nPara-1 )
                aStr += ( sal_Char ) 0x0a;
        }
//2do: formatting runs
    }
    aText.Assign( aStr );
}


XclTxo::~XclTxo()
{
}


void XclTxo::SaveCont( XclExpStream& rStrm )
{
    UINT16 nTextLen = aText.GetLen();
    UINT16 nFormatLen = 0;
    if ( nTextLen && !nFormatLen )
        nFormatLen = 8 * 2; // length of CONTINUE record, not count of formats
    rStrm << nGrbit << nRot << UINT32(0) << UINT16(0)
        << nTextLen << nFormatLen << UINT32(0);
}


void XclTxo::Save( XclExpStream& rStrm )
{
    ExcRecord::Save( rStrm );

    UINT16 nTextLen = aText.GetLen();
    UINT16 nFormatLen = 0;
    if ( nTextLen && !nFormatLen )
        nFormatLen = 8 * 2; // length of CONTINUE record, not count of formats

    // CONTINUE records are only written if there is some text
    if ( nTextLen )
    {
        // CONTINUE text
        rStrm.StartRecord( EXC_CONT, aText.GetBufferByteCount() + 1 );
        aText.WriteFlags( rStrm );
        aText.WriteBuffer( rStrm );
        rStrm.EndRecord();

        // CONTINUE formatting runs
        rStrm.StartRecord( EXC_CONT, nFormatLen );
        // write at least two dummy TXORUNs
        rStrm
            << UINT16(0)    // first character
            << UINT16(0)    // normal font
            << UINT32(0)    // Reserved
            << nTextLen
            << UINT16(0)    // normal font
            << UINT32(0)    // Reserved
            ;
        rStrm.EndRecord();
    }
}


UINT16 XclTxo::GetNum() const
{
    return 0x01B6;
}


ULONG XclTxo::GetLen() const
{
    return 18;
}


// --- class XclObjOle -------------------------------------------

XclObjOle::XclObjOle( RootData& rRoot, const SdrObject& rObj )
            :
            XclObj( otPicture, rRoot ),
            rOleObj( rObj ),
            pRootStorage( rRoot.pRootStorage )
{
}


XclObjOle::~XclObjOle()
{
}


void XclObjOle::SaveCont( XclExpStream& rStrm )
{
    // ftCmo subrecord
    XclObj::SaveCont( rStrm );

    // write only as embedded, not linked
    String          aStorageName( RTL_CONSTASCII_STRINGPARAM( "MBD" ) );
    sal_Char        aBuf[ sizeof(UINT32) * 2 + 1 ];
    UINT32          nPictureId = UINT32(this);
    sprintf( aBuf, "%08X", nPictureId );
    aStorageName.AppendAscii( aBuf );
    SvStorageRef    xOleStg = pRootStorage->OpenStorage( aStorageName,
                            STREAM_READWRITE| STREAM_SHARE_DENYALL );
    if( xOleStg.Is() )
    {
        SvInPlaceObjectRef xObj( ((SdrOle2Obj&)rOleObj).GetObjRef() );
        if ( xObj.Is() )
        {
            // set version to "old" version, because it must be
            // saved in MS notation.
            UINT32                  nFl = 0;
            OfaFilterOptions*       pFltOpts = OFF_APP()->GetFilterOptions();
            if( pFltOpts )
            {
                if( pFltOpts->IsMath2MathType() )
                    nFl |= OLE_STARMATH_2_MATHTYPE;

                if( pFltOpts->IsWriter2WinWord() )
                    nFl |= OLE_STARWRITER_2_WINWORD;

                if( pFltOpts->IsCalc2Excel() )
                    nFl |= OLE_STARCALC_2_EXCEL;

                if( pFltOpts->IsImpress2PowerPoint() )
                    nFl |= OLE_STARIMPRESS_2_POWERPOINT;
            }

            SvxMSExportOLEObjects   aOLEExpFilt( nFl );
            aOLEExpFilt.ExportOLEObject( *xObj, *xOleStg );

            // ftCf subrecord, undocumented as usual
            rStrm << UINT16(ftCf) << UINT16(2) << UINT16(0x0002);

            // ftPioGrbit subrecord, undocumented as usual
            rStrm << UINT16(ftPioGrbit) << UINT16(2) << UINT16(0x0001);

            const UINT8 pData[] = {
                0x05, 0x00,
//              0xac, 0x10, 0xa4, 0x00,     // Xcl changes values on each object
                0x00, 0x00, 0x00, 0x00,     // zeroed
                0x02,                       // GPF if zeroed
//              0x78, 0xa9, 0x86, 0x00,     // Xcl changes values on each object
                0x00, 0x00, 0x00, 0x00,     // zeroed
                0x03
            };
            XclExpUniString aName( xOleStg->GetUserName() );
            UINT16 nPadLen = (UINT16)(aName.GetByteCount() & 0x01);
            UINT16 nFmlaLen = sizeof(pData) + aName.GetByteCount() + nPadLen;
            UINT16 nSubRecLen = nFmlaLen + 6;

            // ftPictFmla subrecord, undocumented as usual
            rStrm   << UINT16(ftPictFmla) << nSubRecLen
                    << nFmlaLen;
            rStrm.Write( pData, sizeof(pData) );
            aName.Write( rStrm );
            if( nPadLen )
                rStrm << UINT8(0);      // pad byte
            rStrm   << nPictureId;
        }
    }

    // ftEnd subrecord
    rStrm << UINT16(0) << UINT16(0);
}


void XclObjOle::Save( XclExpStream& rStrm )
{
    // content of this record
    XclObj::Save( rStrm );
    // second MSODRAWING record and TXO and CONTINUE records
    // we shouldn't have any here, so just in case ...
    XclObj::SaveTextRecs( rStrm );
}


ULONG XclObjOle::GetLen() const
{
    return 0;       // calculated by XclExpStream
}


// --- class XclObjAny -------------------------------------------

XclObjAny::XclObjAny( RootData& rRoot )
            :
            XclObj( otUnknown, rRoot )
{
}


XclObjAny::~XclObjAny()
{
}


void XclObjAny::SaveCont( XclExpStream& rStrm )
{
    // ftCmo subrecord
    XclObj::SaveCont( rStrm );
    switch ( eObjType )
    {
        case otGroup :      // ftGmo subrecord
            rStrm << UINT16(ftGmo) << UINT16(2) << UINT16(0);
        break;
    }
    // ftEnd subrecord
    rStrm << UINT16(0) << UINT16(0);
}


void XclObjAny::Save( XclExpStream& rStrm )
{
    // content of this record
    XclObj::Save( rStrm );
    // second MSODRAWING record and TXO and CONTINUE records
    XclObj::SaveTextRecs( rStrm );
}


ULONG XclObjAny::GetLen() const
{   // length of all subrecords including IDs and LENs
    ULONG nLen = XclObj::GetLen() + 4;
    switch ( eObjType )
    {
        case otGroup :      nLen += 6;  break;
    }
    return nLen;
}



// --- class XclNoteList ----------------------------------------------

XclNoteList::XclNoteList()
{
}


XclNoteList::~XclNoteList()
{
    for ( XclNote* p = First(); p; p = Next() )
    {
        delete p;
    }
}


void XclNoteList::Add( XclNote* pNote )
{
    // limitation on 64kB is done in XclObjList,
    // more notes get no ID and will not save themselfs
    Insert( pNote, LIST_APPEND );
}


void XclNoteList::Save( XclExpStream& rStrm )
{
    for ( XclNote* p = First(); p; p = Next() )
    {
        p->Save( rStrm );
    }
}



// --- class XclNote -------------------------------------------------

XclNote::XclNote( RootData& rRoot, const ScAddress& rPos, const String& rNoteText, const String& rNoteAuthor )
            :
            aAuthor( rNoteAuthor ),
            aPos( rPos ),
            nGrbit(0)
{
    XclObjComment* pObj = new XclObjComment( rRoot, rPos, rNoteText );
    nObjId = rRoot.pObjRecs->Add( pObj );
}


XclNote::~XclNote()
{
}


void XclNote::SaveCont( XclExpStream& rStrm )
{
    rStrm << (UINT16) aPos.Row() << (UINT16) aPos.Col() << nGrbit << nObjId;
    aAuthor.Write( rStrm );
    rStrm << UINT8(0);      // pad byte goes here (everytime)
}


void XclNote::Save( XclExpStream& rStrm )
{
    if ( nObjId )
        ExcRecord::Save( rStrm );
}


UINT16 XclNote::GetNum() const
{
    return 0x001C;
}


ULONG XclNote::GetLen() const
{
    return nObjId ? 9 + aAuthor.GetByteCount() : 0;
}


// --- class ExcBof8_Base --------------------------------------------

ExcBof8_Base::ExcBof8_Base()
{
    nVers           = 0x0600;
    nRupBuild       = 0x0dbb;
    nRupYear        = 0x07cc;
//  nFileHistory    = 0x00000001;   // last edited by Microsoft Excel for Windows
    nFileHistory    = 0x00000000;
    nLowestBiffVer  = 0x00000006;   // Biff8
}


void ExcBof8_Base::SaveCont( XclExpStream& rStrm )
{
    rStrm   << nVers << nDocType << nRupBuild << nRupYear
            << nFileHistory << nLowestBiffVer;
}


UINT16 ExcBof8_Base::GetNum() const
{
    return 0x0809;
}


ULONG ExcBof8_Base::GetLen() const
{
    return 16;
}


// --- class ExcBof8 -------------------------------------------------

ExcBof8::ExcBof8()
{
    nDocType = 0x0010;
}


// --- class ExcBofW8 ------------------------------------------------

ExcBofW8::ExcBofW8()
{
    nDocType = 0x0005;
}


// --- class ExcBofC8 ------------------------------------------------

ExcBofC8::ExcBofC8()
{
    nDocType = 0x0020;
}

#if 0
// --- class ExcLabel8 -----------------------------------------------

ExcLabel8::ExcLabel8(
        const ScAddress& rPos,
        const ScPatternAttr* pAttr,
        RootData& rRoot,
        const String& rNewText ) :
    ExcCell( rPos, pAttr, rRoot ),
    aText( rNewText, 255 )
{
}


ExcLabel8::ExcLabel8(
        const ScAddress& rPos,
        const ScPatternAttr* pAttr,
        RootData& rRoot,
        const ScEditCell& rEdCell ) :
    ExcCell( rPos, pAttr, rRoot )
{
    String aStr;
    aText.SetRichData( new ExcRichStr( *this, aStr, pAttr, rEdCell, rRoot, 255 ) );
    aText.Assign( aStr, 255 );
}


ExcLabel8::~ExcLabel8()
{
}


void ExcLabel8::SaveDiff( XclExpStream& rStrm )
{
    aText.Write( rStrm );
}


UINT16 ExcLabel8::GetNum() const
{
    return 0x0204;
}


ULONG ExcLabel8::GetDiffLen() const
{   // Text bytes max 2 * 255 chars + 2 * 2 * 255 forms
    return aText.GetByteCount();
}

#endif
// --- class ExcLabelSst ---------------------------------------------

ExcLabelSst::ExcLabelSst(
        const ScAddress& rPos,
        const ScPatternAttr* pAttr,
        RootData& rRoot,
        const String& rNewText ) :
    ExcCell( rPos, pAttr, rRoot )
{
    nIsst = rRoot.pSstRecs->Add( new XclExpUniString( rNewText ) );
}


ExcLabelSst::ExcLabelSst(
        const ScAddress& rPos,
        const ScPatternAttr* pAttr,
        RootData& rRoot,
        const ScEditCell& rEdCell ) :
    ExcCell( rPos, pAttr, rRoot )
{
    XclExpRichString* pRS = new XclExpRichString;
    String aStr;
    pRS->SetRichData( new ExcRichStr( *this, aStr, pAttr, rEdCell, rRoot, 0xFFFF ) );
    pRS->Assign( aStr );
    nIsst = rRoot.pSstRecs->Add( pRS );
}


ExcLabelSst::~ExcLabelSst()
{
}


void ExcLabelSst::SaveDiff( XclExpStream& rStrm )
{
    rStrm << nIsst;
}


UINT16 ExcLabelSst::GetNum() const
{
    return 0x00FD;
}


ULONG ExcLabelSst::GetDiffLen() const
{
    return 4;
}



// --- class ExcXf8 --------------------------------------------------

ExcXf8::ExcXf8( UINT16 nFont, UINT16 nForm, const ScPatternAttr* pPattAttr,
                BOOL& rbLineBreak, BOOL bSt ) :
        ExcXf( nFont, nForm, pPattAttr, rbLineBreak, bSt ),
        nTrot( 0 ),
        nCIndent( 0 ),
        bFShrinkToFit( FALSE ),
        bFMergeCell( FALSE ),
        nIReadingOrder( 0 ),
        nGrbitDiag( 0 ),
        nIcvDiagSer( 0 ),
        nDgDiag( 0 )
{
    if( eOri == ETO_TopBottom )
        nTrot = 0x00FF;
    else if( pPattAttr )
        nTrot = XclExpHelper::GetRotation( ((const SfxInt32Item&) pPattAttr->GetItem( ATTR_ROTATE_VALUE )).GetValue() );
}


void ExcXf8::SaveCont( XclExpStream& rStrm )
{
    register UINT16 nTmp;
    register UINT32 nTmp32;

    rStrm << nIfnt << nIfmt                             // Offs 4 + 6
        << nOffs8;                                      // Offs 8

    nTmp = ( UINT16 ) eAlc;                             // Offs 10
    if( bFWrap )
        nTmp |= 0x0008;
    nTmp |= ( ( UINT16 ) eAlcV ) << 4;
    nTmp |= nTrot << 8;
    rStrm << nTmp;

    nTmp = nCIndent;                                    // Offs 12
    if( bFShrinkToFit )
        nTmp |= 0x0010;
    if( bFMergeCell )
        nTmp |= 0x0020;
    nTmp |= nIReadingOrder << 6;
    // Bit 9-8 reserved, Bit 15-10 fAtr... alle 0 (keine Parent Styles)
    rStrm << nTmp;

    nTmp = nDgLeft;                                     // Offs 14
    nTmp |= nDgRight << 4;
    nTmp |= nDgTop << 8;
    nTmp |= nDgBottom << 12;
    rStrm << nTmp;

    nTmp = pPalette2->GetColorIndex( nIcvLftSer );      // Offs 16
    nTmp |= pPalette2->GetColorIndex( nIcvRigSer ) << 7;
    nTmp |= nGrbitDiag << 14;
    rStrm << nTmp;

    nTmp32 = pPalette2->GetColorIndex( nIcvTopSer );    // Offs 18
    nTmp32 |= (UINT32) pPalette2->GetColorIndex( nIcvBotSer ) << 7;
    nTmp32 |= (UINT32) pPalette2->GetColorIndex( nIcvDiagSer ) << 14;
    nTmp32 |= (UINT32) nDgDiag << 21;

    UINT16 nForeInd, nBackInd;
    pPalette2->GetMixedColors( nIcvForeSer, nIcvBackSer, nForeInd, nBackInd, nFls );

    nTmp32 |= (UINT32) nFls << 26;
    rStrm << nTmp32;

    nTmp = nForeInd | (nBackInd << 7);                  // Offs 22

    if( bFSxButton )
        nTmp |= 0x4000;
    rStrm << nTmp;
}


UINT16 ExcXf8::GetNum() const
{
    return 0x00E0;
}


ULONG ExcXf8::GetLen() const
{
    return 20;
}



// --- class ExcBundlesheet8 -----------------------------------------

ExcBundlesheet8::ExcBundlesheet8( RootData& rRootData, UINT16 nTab ) :
    ExcBundlesheetBase( rRootData, nTab )
{
    String sTabName;
    rRootData.pDoc->GetName( nTab, sTabName );
    aUnicodeName.Assign( sTabName, 255 );
}


ExcBundlesheet8::ExcBundlesheet8( const String& rString ) :
    ExcBundlesheetBase(),
    aUnicodeName( rString, 255 )
{
}


void ExcBundlesheet8::SaveCont( XclExpStream& rStrm )
{
    nOwnPos = rStrm.GetStreamPos();
    rStrm   << (UINT32) 0x00000000          // dummy position
            << nGrbit;
    aUnicodeName.Write( rStrm, FALSE );     // 8 bit length
}


ULONG ExcBundlesheet8::GetLen() const
{   // Text max 255 chars
    return 8 + aUnicodeName.GetBufferByteCount();
}



// --- class ExcWindow28 ---------------------------------------------

void ExcWindow28::SaveCont( XclExpStream& rStrm )
{
    BYTE pData[] = {
        0xb6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00
    };
    DBG_ASSERT( sizeof(pData) == GetLen(), "ExcWindow28::SaveCont: length mismatch" );
    if ( GetTable() == 0 )
        pData[1] |= 0x06;       // displayed and selected
    rStrm.Write( pData, GetLen() );
}


UINT16 ExcWindow28::GetNum() const
{
    return 0x023e;
}


ULONG ExcWindow28::GetLen() const
{
    return 18;
}


// --- class XclCondFormat -------------------------------------------

XclCondFormat::XclCondFormat( const ScConditionalFormat& _rCF, ScRangeList* _pRL, RootData& rER )
                :
                rCF( _rCF )
{
    pRL = _pRL;
    nTabNum = *rER.pAktTab;
    nComplLen = 0;

    USHORT                      n;
    USHORT                      nNum = _rCF.Count();
    const ScCondFormatEntry*    pCFE;

    for( n = 0 ; n < nNum ; n++ )
    {
        pCFE = _rCF.GetEntry( n );
        if( pCFE )
            List::Insert( new XclCf( *pCFE, rER ), LIST_APPEND );
    }
}


XclCondFormat::~XclCondFormat()
{
    if( pRL )
        delete pRL;
}


void XclCondFormat::WriteCondfmt( XclExpStream& rStrm )
{
    DBG_ASSERT( List::Count() < 65536, "XclCondFormat::SaveCont - to much CFs!" );

    rStrm.StartRecord( 0x01B0, 0 );     // real size calculated by XclExpStream

    rStrm << (UINT16) List::Count() << (UINT16) 0x0000;

    ULONG               nMinMaxPos = rStrm.GetStreamPos();
    UINT16              nRowFirst = 0xFFFF;
    UINT16              nRowLast = 0;
    UINT16              nColFirst = 0xFFFF;
    UINT16              nColLast = 0;
    UINT16              nRefCnt = 0;

    rStrm << nRowFirst << nRowLast << nColFirst << nColLast << nRefCnt;     // dummies

    const ScRange*      pAct = pRL->First();

    while( pAct )
    {
        const ScAddress&    rStart = pAct->aStart;
        const ScAddress&    rEnd = pAct->aEnd;

        if( rStart.Tab() == nTabNum )
        {
            nRefCnt++;

            UINT16      nRF = rStart.Row();
            UINT16      nCF = rStart.Col();
            UINT16      nRL = rEnd.Row();
            UINT16      nCL = rEnd.Col();

            nRowFirst = Min( nRF, nRowFirst );
            nRowLast = Max( nRL, nRowLast );
            nColFirst = Min( nCF, nColFirst );
            nColLast = Max( nCL, nColLast );

            rStrm << nRF << nRL << nCF << nCL;
        }
        pAct = pRL->Next();
    }

    rStrm.EndRecord();

    // write min / max and num of refs
    rStrm.SetStreamPos( nMinMaxPos );
    rStrm << nRowFirst << nRowLast << nColFirst << nColLast << nRefCnt;
}


void XclCondFormat::Save( XclExpStream& rStrm )
{
    // write CONDFMT
    WriteCondfmt( rStrm );

    // write list of CF records
    for( XclCf* pCf = _First(); pCf; pCf = _Next() )
        pCf->Save( rStrm );
}



// --- class XclCf ---------------------------------------------------

XclCf::XclCf( const ScCondFormatEntry& r, RootData& rRD ) :
        rPalette2( *rRD.pPalette2 )
{
    nType = 0x01;   // compare
    nFormatLen = 0;
    bHasStyle = bHasFont = bHasLine = bHasPattern = FALSE;

    BOOL bSingForm = TRUE;
    switch( r.GetOperation() )
    {
        case SC_COND_EQUAL:         nOp = 0x03;                                     break;
        case SC_COND_LESS:          nOp = 0x06;                                     break;
        case SC_COND_GREATER:       nOp = 0x05;                                     break;
        case SC_COND_EQLESS:        nOp = 0x08;                                     break;
        case SC_COND_EQGREATER:     nOp = 0x07;                                     break;
        case SC_COND_NOTEQUAL:      nOp = 0x04;                                     break;
        case SC_COND_BETWEEN:       nOp = 0x01; bSingForm = FALSE;                  break;
        case SC_COND_NOTBETWEEN:    nOp = 0x02; bSingForm = FALSE;                  break;
        case SC_COND_DIRECT:        nOp = 0x00;                     nType = 0x02;   break;
        default:                    nOp = 0x00;                     nType = 0x00;
    }

    // creating formats
    const String&       rStyleName = r.GetStyle();
    SfxStyleSheetBase*  pStyle =
        rRD.pDoc->GetStyleSheetPool()->Find( rStyleName, SFX_STYLE_FAMILY_PARA );

    bHasStyle = pStyle ? TRUE : FALSE;
    if( bHasStyle )
    {
        const SfxItemSet&   rSet = pStyle->GetItemSet();
        ScPatternAttr       aAttr( new SfxItemSet( rSet ) );

        BOOL bHasItalic     = rSet.GetItemState( ATTR_FONT_POSTURE, TRUE ) == SFX_ITEM_SET;
        BOOL bHasUnderline  = rSet.GetItemState( ATTR_FONT_UNDERLINE, TRUE ) == SFX_ITEM_SET;
        BOOL bHasStrikeOut  = rSet.GetItemState( ATTR_FONT_CROSSEDOUT, TRUE ) == SFX_ITEM_SET;
        BOOL bHasWeight     = rSet.GetItemState( ATTR_FONT_WEIGHT, TRUE ) == SFX_ITEM_SET;
        bHasColor       = rSet.GetItemState( ATTR_FONT_COLOR, TRUE ) == SFX_ITEM_SET;

        bHasFont = bHasItalic || bHasUnderline || bHasStrikeOut || bHasWeight || bHasColor;
        bHasLine = rSet.GetItemState( ATTR_BORDER, TRUE ) == SFX_ITEM_SET;
        bHasPattern = rSet.GetItemState( ATTR_BACKGROUND, TRUE ) == SFX_ITEM_SET;

        // constants for formatsstart
        nStart = 0x00FFFFFF;    // nothing included, all DC
        nFormatLen = 6;

        if( bHasFont )
        {
            nStart |= 0x04000000;
            nStart &= 0xFF3FFFFF;
            nFormatLen += 118;
        }
        if( bHasLine )
        {
            nStart |= 0x10000000;
            nStart &= 0xFFFFC3FF;
            nFormatLen += 8;
        }
        if( bHasPattern )
        {
            nStart |= 0x20000000;
            nStart &= 0xFF3BFFFF;
            nFormatLen += 4;
        }

        // font data
        if( bHasFont )
        {
            Font            aFont;
            ScPatternAttr   aPattAttr( new SfxItemSet( rSet ) );
            aPattAttr.GetFont( aFont );

            BOOL bItalic    = ( bHasItalic && aFont.GetItalic() != ITALIC_NONE );
            BOOL bStrikeOut = ( bHasStrikeOut && aFont.GetStrikeout() != STRIKEOUT_NONE );

            nFontData1 = bItalic ? 0x00000002 : 0;
            nFontData1 |= bStrikeOut ? 0x00000080 : 0;

            if( bHasWeight )
                nFontData2 = ExcFont::GetWeight( aFont.GetWeight() );
            else
                nFontData2 = bHasItalic ? 0x00000400 : 0;
            nFontData3 = bHasUnderline ? ExcFont::GetUnderline( aFont.GetUnderline() ) : 0;

            if( bHasColor )
                nIcvTextSer = rPalette2.InsertColor( aFont.GetColor(), EXC_COLOR_CELLTEXT );

            nFontData4 = bHasStrikeOut ? 0x00000018 : 0x00000098;
            nFontData4 |= (bHasWeight || bHasItalic) ? 0 : 0x00000002;
            nFontData5 = bHasUnderline ? 0 : 0x00000001;
            nFontData6 = (bHasWeight || bHasItalic) ? 0 : 0x01;
        }

        // border data
        if( bHasLine )
        {
            const SvxBoxItem&   rBox = ((const SvxBoxItem&)aAttr.GetItem( ATTR_BORDER ));
            UINT16              nDgTop, nDgBottom, nDgLeft, nDgRight;

            ExcXf::ScToExcBorderLine( rBox.GetTop(), nIcvTopSer, nDgTop );
            ExcXf::ScToExcBorderLine( rBox.GetBottom(), nIcvBotSer, nDgBottom );
            ExcXf::ScToExcBorderLine( rBox.GetLeft(), nIcvLftSer, nDgLeft );
            ExcXf::ScToExcBorderLine( rBox.GetRight(), nIcvRigSer, nDgRight );

            nLineData1 = (UINT8)((nDgLeft & 0x0F) | (nDgRight << 4));
            nLineData2 = (UINT8)((nDgTop & 0x0F) | (nDgBottom << 4));
        }

        // background / foreground data
        if( bHasPattern )
        {
            const SvxBrushItem& rBrushItem = (const SvxBrushItem&)aAttr.GetItem( ATTR_BACKGROUND );
            Color aColor( rBrushItem.GetColor() );
            nPatt = aColor.GetTransparency() ? 0x0000 : 0x0001;
            nIcvForeSer = rPalette2.InsertColor( aColor, EXC_COLOR_CELLBGROUND );
            nIcvBackSer = rPalette2.InsertColor( Color( COL_BLACK ), EXC_COLOR_CELLBGROUND );
        }
    }

    ScTokenArray*   pScTokArry1 = r.CreateTokenArry( 0 );
    EC_Codetype     eDummy;
    ExcUPN*         pForm1      = new ExcUPN( &rRD, *pScTokArry1, eDummy, NULL, TRUE );
    nFormLen1 = pForm1->GetLen();

    ScTokenArray*   pScTokArry2 = NULL;
    ExcUPN*         pForm2 = NULL;
    if( bSingForm )
        nFormLen2 = 0;
    else
    {
        pScTokArry2 = r.CreateTokenArry( 1 );
        pForm2 = new ExcUPN( &rRD, *pScTokArry2, eDummy, NULL, TRUE );
        nFormLen2 = pForm2->GetLen();
    }

    DBG_ASSERT( nFormLen1 + nFormLen2 < 4096, "*XclCf::XclCf(): possible overflow of var data" );

    nVarLen = nFormLen1 + nFormLen2;
    pVarData = new sal_Char[ nVarLen ];

    if( nFormLen1 )
        memcpy( pVarData, pForm1->GetData(), nFormLen1 );
    delete pForm1;
    delete pScTokArry1;

    if( pForm2 )
    {
        memcpy( pVarData + nFormLen1, pForm2->GetData(), nFormLen2 );
        delete pForm2;
        delete pScTokArry2;
    }
}


XclCf::~XclCf()
{
    if( pVarData )
        delete pVarData;
}


void XclCf::SaveCont( XclExpStream& rStrm )
{
    const UINT8 pPre[ 68 ] =
    {   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xDF, 0x3F, 0x8A, 0x1D, 0x3C, 0xFC, 0xFD, 0x7E, 0xDF, 0x3F,
        0x4E, 0x4A, 0x54, 0x30, 0x00, 0x00, 0x00, 0x00, 0x65, 0x10,
        0x00, 0x30, 0xF0, 0x59, 0x54, 0x30, 0xEC, 0x04, 0xC8, 0x00,
        0x4C, 0x00, 0x00, 0x00, 0x89, 0x10, 0x00, 0x30, 0xEC, 0x04,
        0xC8, 0x00, 0xF0, 0x59, 0x54, 0x30, 0x4C, 0x00, 0x00, 0x00,
        0x4E, 0x4A, 0x54, 0x30, 0xFF, 0xFF, 0xFF, 0xFF
    };
    const UINT8 pPost[ 17 ] =
    {   0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0xFF, 0xFF, 0xFF, 0x7F, 0x01, 0x00
    };

    rStrm << nType << nOp << nFormLen1 << nFormLen2;
    if( bHasStyle )
    {
        rStrm << nStart << (UINT16)0x0002;
        if( bHasFont )
        {
            rStrm.Write( pPre, 68 );
            rStrm << nFontData1 << nFontData2 << nFontData3;
            if( bHasColor )
                rStrm << (UINT32) rPalette2.GetColorIndex( nIcvTextSer );
            else
                rStrm << (UINT32)0xFFFFFFFF;
            rStrm   << (UINT32)0x00000000 << nFontData4
                    << (UINT32)0x00000001 << nFontData5
                    << nFontData6;
            rStrm.Write( pPost, 17 );
        }
        if( bHasLine )
        {
            UINT16 nLineData3, nLineData4;
            nLineData3 = rPalette2.GetColorIndex( nIcvLftSer ) & 0x007F;
            nLineData3 |= (rPalette2.GetColorIndex( nIcvRigSer ) & 0x007F) << 7;
            nLineData4 = rPalette2.GetColorIndex( nIcvTopSer ) & 0x007F;
            nLineData4 |= (rPalette2.GetColorIndex( nIcvBotSer ) & 0x007F) << 7;

            rStrm   << nLineData1 << nLineData2 << nLineData3 << nLineData4
                    << (UINT16)0xBA00;
        }
        if( bHasPattern )
        {
            UINT16  nForeInd, nBackInd;
            rPalette2.GetMixedColors( nIcvForeSer, nIcvBackSer, nForeInd, nBackInd, nPatt );

            UINT8   nPattData1 = (nPatt == 1) ? 0 : (UINT8) nPatt;
            UINT16  nPattData2 = ((nForeInd & 0x007F ) << 7) | (nBackInd & 0x007F);

            rStrm << (UINT8)0 << nPattData1 << nPattData2;
        }
    }
    rStrm.Write( pVarData, nVarLen );
}


UINT16 XclCf::GetNum() const
{
    return 0x01B1;
}


ULONG XclCf::GetLen() const
{
    return 6 + nFormatLen + nVarLen;
}



// --- class XclObproj -----------------------------------------------

UINT16 XclObproj::GetNum() const
{
    return 0x00D3;
}


ULONG XclObproj::GetLen() const
{
    return 0;
}



// --- class XclDConRef ----------------------------------------------

XclDConRef::XclDConRef( const ScRange& rSrcR, const String& rWB ) :
        aSourceRange( rSrcR )
{
    String  sTemp( ( sal_Unicode ) 0x02 );

    sTemp += rWB;
    pWorkbook = new XclExpUniString( sTemp );
}

XclDConRef::~XclDConRef()
{
    delete pWorkbook;
}

void XclDConRef::SaveCont( XclExpStream& rStrm )
{
    rStrm   << (UINT16) aSourceRange.aStart.Row()
            << (UINT16) aSourceRange.aEnd.Row()
            << (UINT8)  aSourceRange.aStart.Col()
            << (UINT8)  aSourceRange.aEnd.Col();
    pWorkbook->Write( rStrm );      // normal unicode string
    rStrm   << (UINT8)  0x00;
}

UINT16 XclDConRef::GetNum() const
{
    return 0x0051;
}

ULONG XclDConRef::GetLen() const
{
    return 7 + pWorkbook->GetByteCount();
}



// --- class XclExpCellMerging ---------------------------------------

void XclExpCellMerging::Append( UINT16 nCol1, UINT16 nColCnt, UINT16 nRow1, UINT16 nRowCnt, UINT16 nXF )
{
    AppendCell( new XclExpMergedCell( nCol1, nCol1 + nColCnt - 1, nRow1, nRow1 + nRowCnt - 1, nXF ) );
}


BOOL XclExpCellMerging::FindNextMerge( const ScAddress& rPos, UINT16& rnCol )
{
    rnCol = 0xFFFF;
    for( XclExpMergedCell* pCell = FirstCell(); pCell; pCell = NextCell() )
        if( (pCell->nRow1 <= rPos.Row()) && (rPos.Row() <= pCell->nRow2) &&
            (rPos.Col() <= pCell->nCol1) && (pCell->nCol1 < rnCol) )
            rnCol = pCell->nCol1;
    return rnCol < 0xFFFF;
}


BOOL XclExpCellMerging::FindMergeBaseXF( const ScAddress& rPos, UINT16& rnXF, UINT16& rnColCount )
{
    for( XclExpMergedCell* pCell = FirstCell(); pCell; pCell = NextCell() )
        if( (pCell->nCol1 <= rPos.Col()) && (rPos.Col() <= pCell->nCol2) &&
            (pCell->nRow1 <= rPos.Row()) && (rPos.Row() <= pCell->nRow2) )
        {
            rnXF = pCell->nXF;
            rnColCount = pCell->nCol2 - rPos.Col() + 1;
            return TRUE;
        }
    return FALSE;
}


void XclExpCellMerging::Save( XclExpStream& rStrm )
{
    ULONG nCount = aCellList.Count();
    if( !nCount ) return;

    ULONG nIndex = 0;
    while( nCount )
    {
        ULONG nPortion = Min( nCount, (ULONG)EXC_MERGE_MAXCOUNT );
        nCount -= nPortion;
        rStrm.StartRecord( 0x00E5, 2 + nPortion * 8 );
        rStrm << (UINT16) nPortion;
        while( nPortion-- )
        {
            XclExpMergedCell* pCell = GetCell( nIndex++ );
            if( pCell )
                rStrm << *pCell;
        }
        rStrm.EndRecord();
    }
}



// ---- class XclCodename --------------------------------------------

XclCodename::XclCodename( const String& r ) : aName( r )
{
}


void XclCodename::SaveCont( XclExpStream& rStrm )
{
    aName.Write( rStrm );       // normal unicode string
}


UINT16 XclCodename::GetNum() const
{
    return 0x01BA;
}


ULONG XclCodename::GetLen() const
{
    return aName.GetByteCount();
}



// ---- class XclBuildInName -----------------------------------------

XclBuildInName::XclBuildInName( RootData* p, UINT16 nTab, UINT8 nK ) :
    ExcRoot( p ),
    nTabNum( nTab ),
    nKey( nK )
{
    pData = NULL;
    nFormLen = 0;
}


XclBuildInName::~XclBuildInName()
{
    if( pData )
        delete[] pData;
}


void XclBuildInName::CreateFormula( void )
{
    if( !pData )
    {
        ExcUPN*         p = CreateExcUpnFromScRangeList( *pExcRoot, aRL );

        nFormLen = p->GetLen();

        if( nFormLen )
        {
            pData = new UINT8[ nFormLen ];
            memcpy( pData, p->GetData(), nFormLen );
        }

        delete p;
    }
}


void XclBuildInName::SaveCont( XclExpStream& rStrm )
{
            //  grbit (built in only )      chKey           cch
    rStrm   << ( UINT16 ) 0x0020 << ( UINT8 ) 0x00 << ( UINT8 ) 0x01
            //  cce         itab                cch
            << nFormLen << nTabNum << ( UINT32 ) 0x00000001
            //      grbit string
            << ( UINT16 ) 0x00000 << ( UINT8 ) 0x00 << nKey;

    rStrm.Write( pData, nFormLen );
}


void XclBuildInName::Save( XclExpStream& rStrm )
{
    if( pData )
        ExcNameListEntry::Save( rStrm );
}


ULONG XclBuildInName::GetLen() const
{
    return 16 + nFormLen;
}



// ---- class XclPrintRange, class XclTitleRange ---------------------

XclPrintRange::XclPrintRange( RootData* p, UINT16 nTabNum, ScDocument& rDoc ) : XclBuildInName( p, nTabNum, 0x06 )
{
    if( rDoc.HasPrintRange() )
    {
        UINT16          nAnz = rDoc.GetPrintRangeCount( nTabNum );

        for( UINT16 n = 0 ; n < nAnz ; n++ )
            Append( *rDoc.GetPrintRange( nTabNum, n ) );

        CreateFormula();
    }
}




XclTitleRange::XclTitleRange( RootData* p, UINT16 nTabNum, ScDocument& rDoc ) : XclBuildInName( p, nTabNum, 0x07 )
{
    const ScRange*      pRepColRange = rDoc.GetRepeatColRange( nTabNum );
    const ScRange*      pRepRowRange = rDoc.GetRepeatRowRange( nTabNum );

    if( pRepColRange || pRepRowRange )
    {
        if( pRepColRange )
            Append( *pRepColRange );

        if( pRepRowRange )
            Append( *pRepRowRange );

        CreateFormula();
    }
}



// ---- Scenarios ----------------------------------------------------

ExcEScenarioCell::ExcEScenarioCell( UINT16 nC, UINT16 nR, const String& rTxt ) :
        nCol( nC ),
        nRow( nR ),
        sText( rTxt, EXC_SCEN_MAXSTRINGLEN )
{
}

void ExcEScenarioCell::WriteAddress( XclExpStream& rStrm )
{
    rStrm << nRow << nCol;
}

void ExcEScenarioCell::WriteText( XclExpStream& rStrm )
{
    sText.Write( rStrm );       // normal unicode string
}




XclExpUniString ExcEScenario::sUsername;

ExcEScenario::ExcEScenario( ScDocument& rDoc, UINT16 nTab )
{
    String  sTmpName;
    String  sTmpComm;
    Color   aDummyCol;
    UINT16  nDummyFlags;

    rDoc.GetName( nTab, sTmpName );
    sName.Assign( sTmpName, EXC_SCEN_MAXSTRINGLEN );
    nRecLen = 8 + sName.GetBufferByteCount();

    rDoc.GetScenarioData( nTab, sTmpComm, aDummyCol, nDummyFlags );
    sComment.Assign( sTmpComm, EXC_SCEN_MAXSTRINGLEN );
    if( sComment.GetLen() )
        nRecLen += sComment.GetByteCount();

    if( !sUsername.GetLen() )
    {
        SvtUserOptions aUserOpt;
        sUsername.Assign( aUserOpt.GetLastName() );
    }
    if( !sUsername.GetLen() )
        sUsername.Assign( String::CreateFromAscii( "SC" ) );
    nRecLen += sUsername.GetByteCount();

    const ScRangeList* pRList = rDoc.GetScenarioRanges( nTab );
    if( !pRList )
        return;

    BOOL    bContLoop = TRUE;
    UINT16  nRow, nCol;
    String  sText;
    double  fVal;

    for( UINT32 nRange = 0; (nRange < pRList->Count()) && bContLoop; nRange++ )
    {
        const ScRange* pRange = pRList->GetObject( nRange );
        for( nRow = pRange->aStart.Row(); (nRow <= pRange->aEnd.Row()) && bContLoop; nRow++ )
            for( nCol = pRange->aStart.Col(); (nCol <= pRange->aEnd.Col()) && bContLoop; nCol++ )
            {
                if( rDoc.HasValueData( nCol, nRow, nTab ) )
                {
                    rDoc.GetValue( nCol, nRow, nTab, fVal );
                    sText.Erase();
                    SolarMath::DoubleToString( sText, fVal, 'A', INT_MAX,
                        ScGlobal::pLocaleData->getNumDecimalSep().GetChar(0), TRUE );
                }
                else
                    rDoc.GetString( nCol, nRow, nTab, sText );
                bContLoop = Append( nCol, nRow, sText );
            }
    }
}

ExcEScenario::~ExcEScenario()
{
    for( ExcEScenarioCell* pCell = _First(); pCell; pCell = _Next() )
        delete pCell;
}

BOOL ExcEScenario::Append( UINT16 nCol, UINT16 nRow, const String& rTxt )
{
    if( List::Count() == EXC_SCEN_MAXCELL )
        return FALSE;

    ExcEScenarioCell* pCell = new ExcEScenarioCell( nCol, nRow, rTxt );
    List::Insert( pCell, LIST_APPEND );
    nRecLen += 6 + pCell->GetStringBytes();     // 4 bytes address, 2 bytes ifmt
    return TRUE;
}

void ExcEScenario::SaveCont( XclExpStream& rStrm )
{
    rStrm   << (UINT16) List::Count()       // number of cells
            << (UINT8) 1                    // fLocked
            << (UINT8) 0                    // fHidden
            << (UINT8) sName.GetLen()       // length of scen name
            << (UINT8) sComment.GetLen()    // length of comment
            << (UINT8) sUsername.GetLen();  // length of user name
    sName.WriteFlags( rStrm );
    sName.WriteBuffer( rStrm );

    sUsername.Write( rStrm );

    if( sComment.GetLen() )
        sComment.Write( rStrm );

    ExcEScenarioCell* pCell;
    for( pCell = _First(); pCell; pCell = _Next() )
        pCell->WriteAddress( rStrm );           // pos of cell
    for( pCell = _First(); pCell; pCell = _Next() )
        pCell->WriteText( rStrm );              // string content
    rStrm.SetSliceLen( 2 );
    rStrm.WriteZeroBytes( 2 * List::Count() );  // date format
}

UINT16 ExcEScenario::GetNum() const
{
    return 0x00AF;
}

ULONG ExcEScenario::GetLen() const
{
    return nRecLen;
}




ExcEScenarioManager::ExcEScenarioManager( ScDocument& rDoc, UINT16 nTab ) :
        nActive( 0 )
{
    if( rDoc.IsScenario( nTab ) )
        return;

    UINT16 nFirstTab    = nTab + 1;
    UINT16 nNewTab      = nFirstTab;

    while( rDoc.IsScenario( nNewTab ) )
    {
        Append( new ExcEScenario( rDoc, nNewTab ) );

        if( rDoc.IsActiveScenario( nNewTab ) )
            nActive = nNewTab - nFirstTab;
        nNewTab++;
    }
}

ExcEScenarioManager::~ExcEScenarioManager()
{
    for( ExcEScenario* pScen = _First(); pScen; pScen = _Next() )
        delete pScen;
}

void ExcEScenarioManager::SaveCont( XclExpStream& rStrm )
{
    rStrm   << (UINT16) List::Count()       // number of scenarios
            << nActive                      // active scen
            << nActive                      // last displayed
            << (UINT16) 0;                  // reference areas
}

void ExcEScenarioManager::Save( XclExpStream& rStrm )
{
    if( List::Count() )
        ExcRecord::Save( rStrm );

    for( ExcEScenario* pScen = _First(); pScen; pScen = _Next() )
        pScen->Save( rStrm );
}

UINT16 ExcEScenarioManager::GetNum() const
{
    return 0x00AE;
}

ULONG ExcEScenarioManager::GetLen() const
{
    return 8;
}



//___________________________________________________________________
// HLINK - hyperlinks

const BYTE XclHlink::pStaticData[] =
{
    0xD0, 0xC9, 0xEA, 0x79, 0xF9, 0xBA, 0xCE, 0x11,
    0x8C, 0x82, 0x00, 0xAA, 0x00, 0x4B, 0xA9, 0x0B,
    0x02, 0x00, 0x00, 0x00
};

inline ULONG XclHlink::GetStaticLen()
{
    return sizeof( pStaticData );
}

inline ULONG XclHlink::GetVarLen() const
{
    return pVarData->Seek( STREAM_SEEK_TO_END );
}

XclHlink::XclHlink( RootData& rRootData, const SvxURLField& rURLField ) :
    pVarData( new SvMemoryStream ),
    nFlags( EXC_HLINK_BODY ),
    pRepr( NULL )
{
    const XubString&    rURL = rURLField.GetURL();
    const XubString&    rRepr = rURLField.GetRepresentation();
    INetURLObject       aURLObj( rURL );
    const INetProtocol  eProtocol = aURLObj.GetProtocol();
    BOOL                bWithRepr = rRepr.Len() > 0;
    XclExpStream        aXclStrm( *pVarData, 0xFFFFFFFF );

    // description
    if( bWithRepr )
    {
        XclExpUniString aDescr( rRepr, 255, TRUE );     // max 255 chars, force 16 bit
        aXclStrm    << (UINT32) (aDescr.GetLen() + 1);  // string length + 1 trailing zero word
        aDescr.WriteBuffer( aXclStrm );                 // NO flags
        aXclStrm    << (UINT16) 0x0000;

        nFlags |= EXC_HLINK_DESCR;
        pRepr = new String( rRepr );
    }

    // file link or URL
    if( eProtocol == INET_PROT_FILE )
    {
        String  aPathAndName( aURLObj.getFSysPath( INetURLObject::FSYS_DOS ) );
        String  aOrigPaN( aPathAndName );
        UINT16  nLevel = 0;
        BOOL    bRel = rRootData.bStoreRel;

        if( bRel )
        {
            DBG_ASSERT( rRootData.pBasePath, "-XclHlink::XclHlink(): on the meadow is not... :-)" );
            aPathAndName = aURLObj.GetRelURL( *rRootData.pBasePath, aPathAndName,
                INetURLObject::WAS_ENCODED, INetURLObject::DECODE_WITH_CHARSET );

            if( aPathAndName.SearchAscii( INET_FILE_SCHEME ) == 0 )
            {   // not converted to rel -> make abs
                aPathAndName = aOrigPaN;
                bRel = FALSE;
            }
            else if( aPathAndName.SearchAscii( "./" ) == 0 )
                aPathAndName.Erase( 0, 2 );
            else
            {
                while( aPathAndName.SearchAndReplaceAscii( "../", EMPTY_STRING ) != STRING_NOTFOUND )
                    nLevel++;
            }
        }
        if( !bRel )
            nFlags |= EXC_HLINK_ABS;

        ByteString      aAsciiLink( aPathAndName, *rRootData.pCharset );
        XclExpUniString aLink( aPathAndName, 255, TRUE );   // max 255 chars, force 16 bit
        aXclStrm    << (UINT32) 0x00000303
                     << (UINT32)    0x00000000
                    << (UINT32) 0x000000C0
                    << (UINT32) 0x46000000
                    << nLevel
                    << (UINT32) (aAsciiLink.Len() + 1);     // string length + 1 trailing zero byte
        aXclStrm.Write( aAsciiLink.GetBuffer(), aAsciiLink.Len() );
        aXclStrm    << (UINT8)  0x00
                    << (UINT32) 0xDEADFFFF
                    << (UINT32) 0x00000000
                    << (UINT32) 0x00000000
                    << (UINT32) 0x00000000
                    << (UINT32) 0x00000000
                    << (UINT32) 0x00000000
                    << (UINT32) (aLink.GetBufferByteCount() + 6)
                    << (UINT32) aLink.GetBufferByteCount()              // byte count, not string length
                    << (UINT16) 0x0003;
        aLink.WriteBuffer( aXclStrm );

        if( !pRepr )
            pRepr = new String( aPathAndName );
    }
    else
    {
        XclExpUniString aURL( aURLObj.GetURLNoMark(), 255, TRUE );  // max 255 chars, force 16 bit
        aXclStrm    << (UINT32) 0x79EAC9E0
                    << (UINT32) 0x11CEBAF9
                    << (UINT32) 0xAA00828C
                    << (UINT32) 0x0BA94B00
                    << (UINT32) (aURL.GetBufferByteCount() + 2);    // byte count + 1 trailing zero word
        aURL.WriteBuffer( aXclStrm );
        aXclStrm    << (UINT16) 0x0000;

        nFlags |= EXC_HLINK_ABS;
        if( !pRepr )
            pRepr = new String( rURL );
    }

    // text mark
    if( aURLObj.HasMark() )
    {
        XclExpUniString aTextmark( aURLObj.GetMark(), 255, TRUE );  // max 255 chars, force 16 bit
        aXclStrm    << (UINT32) (aTextmark.GetLen() + 1);           // string length + 1 trailing zero word
        aTextmark.WriteBuffer( aXclStrm );
        aXclStrm    << (UINT16) 0x0000;

        nFlags |= EXC_HLINK_MARK;
    }
}

XclHlink::~XclHlink()
{
    if( pVarData )
        delete pVarData;
    if( pRepr )
        delete pRepr;
}

void XclHlink::SaveCont( XclExpStream& rStrm )
{
    rStrm << nRowFirst << nRowFirst << nColFirst << nColFirst;
    rStrm.Write( pStaticData, GetStaticLen() );
    rStrm << nFlags;

    pVarData->Seek( STREAM_SEEK_TO_BEGIN );
    rStrm.CopyFromStream( *pVarData );
}

UINT16 XclHlink::GetNum() const
{
    return 0x01B8;
}

ULONG XclHlink::GetLen() const
{
    // 12 = cols/rows + flags
    return 12 + GetStaticLen() + GetVarLen();
}



// ---- class XclProtection ------------------------------------------

const BYTE      XclProtection::pMyData[] =
{
    0x12, 0x00, 0x02, 0x00, 0x01, 0x00,         // PROTECT
    0xDD, 0x00, 0x02, 0x00, 0x01, 0x00,         // SCENPROTECT
    0x63, 0x00, 0x02, 0x00, 0x01, 0x00          // OBJPROTECT
};
const ULONG XclProtection::nMyLen = sizeof( XclProtection::pMyData );

ULONG XclProtection::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* XclProtection::GetData( void ) const
{
    return pMyData;
}



// ---- class XclBGPic -----------------------------------------------

XclBGPic::XclBGPic( RootData& r )
{
    pGr = NULL;

    SfxStyleSheet*  pStSh = r.pStyleSheet;

    if( pStSh )
        pGr = ( ( const SvxBrushItem& ) r.pStyleSheetItemSet->Get( ATTR_BACKGROUND ) ).GetGraphic();
}


XclBGPic::~XclBGPic()
{
}


void XclBGPic::Save( XclExpStream& rStrm )
{
    if( pGr )
    {
        Bitmap                      aBmp( pGr->GetBitmap() );
        if( aBmp.GetBitCount() != 24 )
            aBmp.Convert( BMP_CONVERSION_24BIT );

        BitmapReadAccess*           pAcc;

        if( ( pAcc = aBmp.AcquireReadAccess() ) )
        {
            sal_uInt16              nWidth = (sal_uInt16)pAcc->Width();
            sal_uInt16              nHeight = (sal_uInt16)pAcc->Height();
            if( nWidth && nHeight )
            {
                rStrm.StartRecord( 0x00E9, 0 );     // real size calculated by XclExpStream

                rStrm.SetMaxRecLen( 0x201C );       // copied from Excel
                rStrm.SetMaxContLen( 0x2014 );      // copied from Excel

                sal_uInt8           nPadding = (nWidth & 0x03);
                sal_uInt32          nOverallSize = (sal_uInt32)((nWidth * 3 + nPadding) * nHeight + 12);

                rStrm   << (sal_uInt32)0x00010009   // magic number
                        << nOverallSize             // overall size after _this_
                        << (sal_uInt32)12           // unknown2
                        << nWidth                   // width
                        << nHeight                  // height
                        << (sal_uInt16)1            // planes
                        << (sal_uInt16)24;          // bits per pixel

                sal_uInt32          x, y, yg;
                for( y = 0, yg = nHeight - 1 ; y < nHeight ; y++, yg-- )
                {
                    for( x = 0 ; x < nWidth ; x++ )
                    {
                        BitmapColor aColor( pAcc->GetPixel( yg, x ) );
                        rStrm   << (sal_uInt8)( aColor.GetBlue() )
                                << (sal_uInt8)( aColor.GetGreen() )
                                << (sal_uInt8)( aColor.GetRed() );
                    }
                    rStrm.WriteZeroBytes( nPadding );
                }

                rStrm.EndRecord();
            }
            aBmp.ReleaseAccess( pAcc );
        }
    }
}



// ---- class XclExpPageBreaks8 --------------------------------------

XclExpPageBreaks8::XclExpPageBreaks8( RootData& rRootData, UINT16 nScTab, ExcPBOrientation eOrient ) :
    XclExpPageBreaks( rRootData, nScTab, eOrient ),
    nRangeMax( (eOrient == pbHorizontal) ? rRootData.nColMax : rRootData.nRowMax )
{
}

XclExpPageBreaks8::~XclExpPageBreaks8()
{
}

void XclExpPageBreaks8::SaveCont( XclExpStream& rStrm )
{
    rStrm << (UINT16) aPageBreaks.Count();
    rStrm.SetSliceLen( 6 );
    for( ULONG nIndex = 0; nIndex < aPageBreaks.Count(); nIndex++ )
        rStrm << aPageBreaks.Get( nIndex ) << (UINT16) 0x0000 << nRangeMax;
}

ULONG XclExpPageBreaks8::GetLen() const
{
    return 2 + 6 * aPageBreaks.Count();
}


// ---- class XclExternName ------------------------------------------

void XclExternName::SaveCont( XclExpStream& rStrm )
{
    if( !pExpStr )
        pExpStr = new XclExpUniString( aName, 255 );
    //      Grbit           reserved
    rStrm << ( UINT16 ) 0 << ( UINT32 ) 0;
    pExpStr->Write( rStrm, FALSE );     // 8 bit length

    // 'formula'
    //      len                 ErrorValue 0x17
    rStrm << ( UINT16 ) 0x0002 << ( UINT16 ) 0x171C;
}


XclExternName::XclExternName( const String& r ) : aName( r )
{
    DBG_ASSERT( aName.Len() <= 256, "*XclExternName::XclExternName(): no strings longer than 256 allowed!" );

    pExpStr = NULL;
}


UINT16 XclExternName::GetNum() const
{
    return 0x0023;
}


ULONG XclExternName::GetLen() const
{
    if( !pExpStr )
        ( ( XclExternName* ) this)->pExpStr = new XclExpUniString( aName, 255 );

    return pExpStr->GetBufferByteCount() + 12;
}




XclExternNameList::~XclExternNameList()
{
    XclExternName*  p = First();

    while( p )
    {
        delete p;
        p = Next();
    }
}


UINT16 XclExternNameList::GetIndex( const String& rName )
{
    XclExternName*  p = First();
    UINT16          nCnt = 1;

    while( p )
    {
        if( *p == rName )
            return nCnt;
        p = Next();
        nCnt++;
    }

    List::Insert( new XclExternName( rName ), LIST_APPEND );

    return nCnt;
}


void XclExternNameList::Save( XclExpStream& rStrm )
{
    XclExternName*  p = First();

    while( p )
    {
        p->Save( rStrm );
        p = Next();
    }
}


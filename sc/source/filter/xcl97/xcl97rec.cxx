/*************************************************************************
 *
 *  $RCSfile: xcl97rec.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-29 10:15:48 $
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

#include "scitems.hxx"
#include <svx/boxitem.hxx>
#include <svx/brshitem.hxx>

#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD
#include <svx/flditem.hxx>

#include <svtools/useroptions.hxx>

#include <stdio.h>

#include "document.hxx"
#include "conditio.hxx"
#include "rangelst.hxx"
#include "stlpool.hxx"


// --- class XclSstList ----------------------------------------------

XclSstList::XclSstList()
{
}


XclSstList::~XclSstList()
{
    for ( XclUnicodeString* p = (XclUnicodeString*) First(); p;
            p = (XclUnicodeString*) Next() )
    {
        delete p;
    }
}


UINT32 XclSstList::Add( XclUnicodeString* pStr )
{
    Insert( pStr, LIST_APPEND );
    return Count() - 1;
}


void XclSstList::_Save( SvStream& rStrm )
{
    if ( !Count() )
        return ;
    rStrm << GetNum() << GetLen();
}


void XclSstList::SaveCont( SvStream& rStrm )
{
    const UINT32 nCstTotal = Count();
    if ( !nCstTotal )
        return ;
    SvMemoryStream aIsstInf( 8192 );
    UINT32 nPerBucket = nCstTotal;
    while ( nPerBucket > 0xFFFF )
    {   // just fit into 16bit
        nPerBucket /= 2;
    }
    const UINT16 nDsst = Max( (UINT16) 8, (UINT16) nPerBucket );
    UINT16 nStr = 0;
    // SST
    {   // own scope for Continue
        XclContinue aCont( rStrm, 0 );
        rStrm << nCstTotal << nCstTotal;            // Total, Unique
        for ( XclUnicodeString* p = (XclUnicodeString*) First(); p;
                p = (XclUnicodeString*) Next() )
        {
            if ( nStr == 0 )
            {   // ISSTINF
                const UINT32 nIb = rStrm.Tell();
                aIsstInf << nIb;
//2do: cb ?!? Offset into the SST record that points to where the bucket begins
                aIsstInf << (UINT16) 0x0000;        // cb
                aIsstInf << (UINT16) 0x0000;        // reserved
            }
            p->Write( aCont );
            if ( ++nStr == nDsst )
                nStr = 0;
        }
    }
    // EXTSST
    rStrm << (UINT16) 0x00ff;                       // EXTSST
    rStrm << (UINT16) 0x0000;                       // RecLen
    {   // own scope for Continue
        XclContinue aCont( rStrm, 0 );
        rStrm << nDsst;
        const UINT16 nIsstInfSize = sizeof(UINT32) + sizeof(UINT16) + sizeof(UINT16);
        aCont.WriteMemStream( aIsstInf, nIsstInfSize );
    }
}


UINT16 XclSstList::GetNum() const
{
    return 0x00fc;
}


UINT16 XclSstList::GetLen() const
{
    return 8;       //! nur FixLen, Rest per XclContinue
}



// --- class XclCrn --------------------------------------------------

void XclCrn::SaveHeadings( SvStream& rStrm )
{
    rStrm << (UINT8) nCol << (UINT8) nCol << nRow;
}

UINT16 XclCrn::GetNum() const
{
    return 0x005A;
}



// --- class XclCrnDouble --------------------------------------------

XclCrnDouble::XclCrnDouble( UINT16 nC, UINT16 nR, double fV ) :
        XclCrn( nC, nR ),
        fVal( fV )
{   }

void XclCrnDouble::SaveCont( SvStream& rStrm )
{
    XclCrn::SaveHeadings( rStrm );
    rStrm << (UINT8) EXC_CRN_DOUBLE << fVal;
}

UINT16 XclCrnDouble::GetLen() const
{
    return 13;
}



// --- class XclCrnString ---------------------------------------------

XclCrnString::XclCrnString( UINT16 nC, UINT16 nR, const String& rTxt ) :
        XclCrn( nC, nR ),
        sText( rTxt )
{   }

void XclCrnString::SaveCont( SvStream& rStrm )
{
    XclCrn::SaveHeadings( rStrm );
    rStrm << (UINT8) EXC_CRN_STRING << sText.GetLen() << sText.GetGrbit();
    sText.WriteToStream( rStrm );
}

UINT16 XclCrnString::GetLen() const
{
    return 8 + sText.GetByteCount();
}



// --- class XclCrnBool ----------------------------------------------

XclCrnBool::XclCrnBool( UINT16 nC, UINT16 nR, BOOL b ) :
        XclCrn( nC, nR ),
        nBool( b ? 1 : 0 )
{   }

void XclCrnBool::SaveCont( SvStream& rStrm )
{
    XclCrn::SaveHeadings( rStrm );
    rStrm << (UINT8) EXC_CRN_BOOL << nBool << (UINT16)0 << (UINT32)0;
}

UINT16 XclCrnBool::GetLen() const
{
    return 13;
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

void XclXct::SaveCont( SvStream& rStrm )
{
    rStrm << (UINT16) List::Count() << nTabNum;
}

void XclXct::Save( SvStream& rStrm )
{
    ExcRecord::Save( rStrm );

    for( XclCrn* pCrn = _First(); pCrn; pCrn = _Next() )
        pCrn->Save( rStrm );
}

UINT16 XclXct::GetNum() const
{
    return 0x0059;
}

UINT16 XclXct::GetLen() const
{
    return 4;
}



// --- class XclSupbook ----------------------------------------------

XclSupbook::XclSupbook( UINT16 nTabs ) :
        nTables( nTabs ),
        nLen( 4 ),
        bSelf( TRUE )
{   }

XclSupbook::XclSupbook( const String& rDocName ) :
        sDocName( rDocName ),
        bSelf( FALSE )
{
    INetURLObject aURLObj( rDocName );
    DBG_ASSERT( !aURLObj.HasError(), "XclSupbook::XclSupbook() - Corrupt external filename!" );

    sEncoded.Assign( aURLObj.getFSysPath( INetURLObject::FSYS_DOS ) );
    nLen = 5 + sEncoded.GetByteCount();
}

XclSupbook::~XclSupbook()
{
    for( XclXct* pXct = _First(); pXct; pXct = _Next() )
        delete pXct;
}

UINT16 XclSupbook::AddTableName( const String& rTabName )
{
    DBG_ASSERT( !bSelf, "XclSupbook::AddTableName() - Don't insert table names here" );
    XclXct* pXct = new XclXct( rTabName );
    nLen += pXct->GetTableBytes();
    List::Insert( pXct, LIST_APPEND );

    UINT16 nTabNum = List::Count() - 1;
    pXct->SetTableNum( nTabNum );
    return nTabNum;
}

void XclSupbook::StoreCellRange( RootData& rRoot, const ScRange& rRange, UINT16 nXct )
{
    XclXct* pXct = _Get( nXct );
    if( pXct )
        pXct->StoreCellRange( rRoot, rRange );
}

void XclSupbook::Save( SvStream& rStrm )
{
    ExcRecord::Save( rStrm );

    for( XclXct* pXct = _First(); pXct; pXct = _Next() )
        pXct->Save( rStrm );
}

void XclSupbook::SaveCont( SvStream& rStrm )
{
    if ( bSelf )
        rStrm << nTables << (UINT8) 0x01 << (UINT8) 0x04;
    else
    {
        XclContinue aCont( rStrm, 0, GetLen() );

        rStrm << (UINT16) List::Count() << sEncoded.GetLen() << sEncoded.GetGrbit();
        sEncoded.WriteToStream( rStrm );

        for( XclXct* pXct = _First(); pXct; pXct = _Next() )
            pXct->GetTableName().Write( aCont );
    }
}

UINT16 XclSupbook::GetNum() const
{
    return 0x01AE;
}

UINT16 XclSupbook::GetLen() const
{
    return nLen;
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

UINT32 XclSupbookList::Append( XclSupbook* pBook )
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

    for( UINT32 nSB = 0; !bFound && (nSB < List::Count()); nSB++ )
    {
        pBook = _Get( nSB );
        if( pBook )
            bFound = (rExtDoc == pBook->GetName());
        if( bFound )
            nPos = nSB;
    }
    if( !bFound )
    {
        pBook = new XclSupbook( rExtDoc );
        nPos = Append( pBook );
    }
    UINT16 nInd = pExcRoot->pTabBuffer->GetExcTable( nScTab );
    DBG_ASSERT( nInd < nRefdCnt,
        "XclSupbookList::XclSupbookList() - Out of range!" );
    pSupbookBuffer[ nInd ] = nPos;

    // append new table name, save position in pTableBuffer
    nPos = pBook->AddTableName( pExcRoot->pDoc->GetLinkTab( nScTab ) );
    pTableBuffer[ nInd ] = nPos;
}

void XclSupbookList::StoreCellRange( const ScRange& rRange )
{
    UINT16 nExcTab = pExcRoot->pTabBuffer->GetExcTable( rRange.aStart.Tab() );
    DBG_ASSERT( nExcTab < nRefdCnt, "XclSupbookList::StoreCellRange() - Out of range!" );

    XclSupbook* pBook = GetSupbook( nExcTab );
    if( pBook )
        pBook->StoreCellRange( *pExcRoot, rRange, pTableBuffer[ nExcTab ] );
}

void XclSupbookList::WriteXtiInfo( SvStream& rStrm, UINT16 nTabFirst, UINT16 nTabLast )
{
    DBG_ASSERT( (nTabFirst < nRefdCnt) && (nTabLast < nRefdCnt),
        "XclSupbookList::WriteXliInfo() - Out of range!" );

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

void XclSupbookList::Save( SvStream& rStrm )
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

void XclExternsheetList::SaveCont( SvStream& rStrm )
{
    XclContinue aCont( rStrm, 0, GetLen() );
    UINT16 nCount16 = GetCount16();
    rStrm << nCount16;
    UINT32 nElems = nCount16;
    if( nElems )
    {
        XclXti* pXti = _First();
        WriteXtiInfo( rStrm, *pXti );
        nElems--;
        UINT32 nHere = 1;
        UINT32 nStop = 1;
        UINT16 nSize = XclXti::GetSize();
        while( nElems )
        {
            aCont.Slice( nElems, nStop, nSize );
            while( nHere < nStop )
            {
                pXti = _Next();
                DBG_ASSERT( pXti, "XclExternsheetList::SaveCont() - Xti missing!" );
                WriteXtiInfo( rStrm, *pXti );
                nHere++;
            }
        }
    }
}

void XclExternsheetList::Save( SvStream& rStrm )
{
    aSupbookList.Save( rStrm );     // SUPBOOKs, XCTs, CRNs
    ExcRecord::Save( rStrm );       // EXTERNSHEET
}

UINT16 XclExternsheetList::GetNum() const
{
    return 0x0017;
}

UINT16 XclExternsheetList::GetLen() const
{   // only a prediction for XclContinue
    return 2 + GetCount16() * XclXti::GetSize();
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


void ExcBof8_Base::SaveCont( SvStream &rStrm )
{
    rStrm << nVers << nDocType << nRupBuild << nRupYear
        << nFileHistory << nLowestBiffVer;
}


UINT16 ExcBof8_Base::GetNum() const
{
    return 0x0809;
}


UINT16 ExcBof8_Base::GetLen() const
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


// --- class ExcLabel8 -----------------------------------------------

ExcLabel8::ExcLabel8( const ScAddress& rPos, const ScPatternAttr* pAttr,
                    const String& rNewText )
        :
        ExcCell( rPos, pAttr ),
        aText( rNewText, 255 )
{
}


ExcLabel8::ExcLabel8( const ScAddress& rPos, const ScPatternAttr* pAttr,
                    const ScEditCell& rEdCell, RootData& rRoot )
        :
        ExcCell( rPos, pAttr )
{
    String aStr;
    aText.SetRichStr( new ExcRichStr( *this, aStr, pAttr, rEdCell, rRoot, 255 ) );
    aText.Assign( aStr, 255 );
}


ExcLabel8::~ExcLabel8()
{
}


void ExcLabel8::SaveDiff( SvStream &rStrm )
{
    XclContinue aCont( rStrm, 6, GetLen() );
    aText.Write( aCont );
}


UINT16 ExcLabel8::GetNum() const
{
    return 0x0204;
}


UINT16 ExcLabel8::GetLen() const
{   // Text bytes max 2 * 255 chars + 2 * 2 * 255 forms
    return 6 + (UINT16) aText.GetByteCount();
}


// --- class ExcLabelSst ---------------------------------------------

ExcLabelSst::ExcLabelSst( const ScAddress& rPos, const ScPatternAttr* pAttr,
                    const String& rNewText, RootData& rRoot )
        :
        ExcCell( rPos, pAttr )
{
    nIsst = rRoot.pSstRecs->Add( new XclUnicodeString( rNewText ) );
}


ExcLabelSst::ExcLabelSst( const ScAddress& rPos, const ScPatternAttr* pAttr,
                    const ScEditCell& rEdCell, RootData& rRoot )
        :
        ExcCell( rPos, pAttr )
{
    XclRichString* pRS = new XclRichString;
    String aStr;
    pRS->SetRichStr( new ExcRichStr( *this, aStr, pAttr, rEdCell, rRoot, 0xFFFF ) );
    pRS->Assign( aStr );
    nIsst = rRoot.pSstRecs->Add( pRS );
}


ExcLabelSst::~ExcLabelSst()
{
}


void ExcLabelSst::SaveDiff( SvStream &rStrm )
{
    rStrm << nIsst;
}


UINT16 ExcLabelSst::GetNum() const
{
    return 0x00fd;
}


UINT16 ExcLabelSst::GetLen() const
{
    return 6 + 4;
}


// --- class ExcXf8 --------------------------------------------------

ExcXf8::ExcXf8( UINT16 nFont, UINT16 nForm, const ScPatternAttr* pPattAttr,
                BOOL& rbLineBreak, BOOL bSt ) :
        ExcXf( nFont, nForm, pPattAttr, rbLineBreak, bSt )
{
    nTrot           = 0;
    nCIndent        = 0;
    bFShrinkToFit   = 0;
    bFMergeCell     = 0;
    nIReadingOrder  = 0;
    nGrbitDiag      = 0;
    nIcvDiagSer     = 0;
    nDgDiag         = 0;
}


UINT16 ExcXf8::GetNum() const
{
    return 0x00E0;
}


UINT16 ExcXf8::GetLen() const
{
    return 20;
}


void ExcXf8::SaveCont( SvStream& rStrm )
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


// --- class ExcBundlesheet8 -----------------------------------------

ExcBundlesheet8::ExcBundlesheet8( const String &rNewName )
        :
        ExcBundlesheet( NULL, rNewName ),   // no pExcRoot needed
        aUnicodeName( rNewName, 255 )
{
}


void ExcBundlesheet8::SaveCont( SvStream &rStrm )
{
    XclContinue aCont( rStrm, 0, GetLen() );
    nOwnPos = rStrm.Tell();
    UINT8 nGrbit = aUnicodeName.HasHighByte() ? 0x01 : 0x00;
    // Position ist nur Dummy
    rStrm << ( UINT32 ) 0x00000000 << nIgnore
                                    //  `-> Worksheet visible
        << (UINT8) aUnicodeName.GetLen() << nGrbit;
            // `-> max 255 chars, done in Unicode ctor
    aUnicodeName.Write( aCont );
}


UINT16 ExcBundlesheet8::GetNum() const
{
    return 0x0085;
}


UINT16 ExcBundlesheet8::GetLen() const
{   // Text max 255 chars
    return 8 + (UINT16) aUnicodeName.GetByteCount();
}


// --- class ExcTabid8 -----------------------------------------------

void ExcTabid8::SaveCont( SvStream& rStrm )
{
    for ( UINT16 j=0; j<nTabs; j++ )
    {
        rStrm << UINT16(j+1);
    }
}


UINT16 ExcTabid8::GetNum() const
{
    return 0x013d;
}


UINT16 ExcTabid8::GetLen() const
{
    return nTabs * 2;
}


// --- class ExcWindow28 ---------------------------------------------

void ExcWindow28::SaveCont( SvStream& rStrm )
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


UINT16 ExcWindow28::GetLen() const
{
    return 18;
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


void XclMsodrawinggroup::SaveCont( SvStream& rStrm )
{
    DBG_ASSERT( GetEscherEx()->GetStreamPos() == GetEscherEx()->GetOffsetFromMap( nStartPos ),
        "XclMsodrawinggroup::SaveCont: Escher stream position mismatch" );
    UINT32 nDataLen = GetDataLen();
    XclContinue aCont( rStrm, 0, (UINT16) nDataLen );
    aCont.WriteStream( pEscher->GetStrm(), nDataLen );
}


UINT16 XclMsodrawinggroup::GetNum() const
{
    return 0x00eb;
}


UINT16 XclMsodrawinggroup::GetLen() const
{
    return (UINT16) GetDataLen();       // only a prediction for XclContinue
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


void XclMsodrawing::SaveCont( SvStream& rStrm )
{
    DBG_ASSERT( GetEscherEx()->GetStreamPos() == GetEscherEx()->GetOffsetFromMap( nStartPos ),
        "XclMsodrawing::SaveCont: Escher stream position mismatch" );
    UINT32 nDataLen = GetDataLen();
    XclContinue aCont( rStrm, 0, (UINT16) nDataLen );
    aCont.WriteStream( pEscher->GetStrm(), nDataLen );
}


UINT16 XclMsodrawing::GetNum() const
{
    return 0x00ec;
}


UINT16 XclMsodrawing::GetLen() const
{
    return (UINT16) GetDataLen();       // only a prediction for XclContinue
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


void XclObjList::_Save( SvStream& rStrm )
{   // nothing
}


void XclObjList::SaveCont( SvStream& rStrm )
{
    //! Escher must be written, even if there are no objects
    pMsodrawingPerSheet->Save( rStrm );

    for ( XclObj* p = First(); p; p = Next() )
    {
        p->Save( rStrm );
    }
}


UINT16 XclObjList::GetNum() const
{   // dummy
    return 0;
}


UINT16 XclObjList::GetLen() const
{   // dummy
    return 0;
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


void XclObj::_Save( SvStream& rStrm )
{
    // MSODRAWING record (msofbtSpContainer)
    if ( !bFirstOnSheet )
        pMsodrawing->Save( rStrm );

    // OBJ
    rStrm << GetNum() << GetLen();
}


void XclObj::SaveCont( SvStream& rStrm )
{   // ftCmo subrecord
    DBG_ASSERT( eObjType != otUnknown, "XclObj::SaveCont: unknown type" );
    rStrm << (UINT16) ftCmo << (UINT16) 0x0012;
    rStrm << (UINT16) eObjType << nObjId << nGrbit
        << UINT32(0) << UINT32(0) << UINT32(0);
}


void XclObj::SaveText( SvStream& rStrm )
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
    return 0x005d;
}


UINT16 XclObj::GetLen() const
{   // length of subrecord including ID and LEN
    return 22;
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


// --- class XclObjComment -------------------------------------------

XclObjComment::XclObjComment( RootData& rRoot, const ScAddress& rPos, const String& rStr )
            :
            XclObj( otComment, rRoot )
{
    nGrbit = 0;     // all off: AutoLine, AutoFill, Printable, Locked
    XclEscherEx* pEx = pMsodrawing->GetEscherEx();
    pEx->OpenContainer( ESCHER_SpContainer );
    pEx->AddShape( ESCHER_ShpInst_TextBox, SHAPEFLAG_HAVEANCHOR | SHAPEFLAG_HAVESPT );
    pEx->BeginCount();
    pEx->AddOpt( ESCHER_Prop_lTxid, 0 );                        // undocumented
    pEx->AddOpt( ESCHER_Prop_FitTextToShape, 0x00080008 );      // bool field
    pEx->AddOpt( 0x0158, 0x00000000 );                          // undocumented
    pEx->AddOpt( ESCHER_Prop_fillColor, 0x08000050 );
    pEx->AddOpt( ESCHER_Prop_fillBackColor, 0x08000050 );
    pEx->AddOpt( ESCHER_Prop_fNoFillHitTest, 0x00110010 );      // bool field
    pEx->AddOpt( ESCHER_Prop_shadowColor, 0x00000000 );
    pEx->AddOpt( ESCHER_Prop_fshadowObscured, 0x00030003 );     // bool field
    pEx->AddOpt( ESCHER_Prop_fPrint, 0x000A0002 );              // bool field
    pEx->EndCount( ESCHER_OPT, 3 );
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


void XclObjComment::SaveCont( SvStream& rStrm )
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

    // second MSODRAWING record and TXO and CONTINUE records
    SaveText( rStrm );
}


UINT16 XclObjComment::GetLen() const
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
    pEx->BeginCount();
    pEx->AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x01040104 ); // bool field
    pEx->AddOpt( ESCHER_Prop_FitTextToShape, 0x00080008 );      // bool field
    pEx->AddOpt( ESCHER_Prop_fNoFillHitTest, 0x00010000 );      // bool field
    pEx->AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x00080000 );     // bool field
    pEx->AddOpt( ESCHER_Prop_fPrint, 0x000A0000 );              // bool field
    pEx->EndCount( ESCHER_OPT, 3 );

    XclEscherClientAnchor aAnchor( rRoot, 0x0001 );             // MsofbtClientAnchor
    aAnchor.SetDropDownPosition( rPos );
    aAnchor.WriteData( *pEx );

    pEx->AddAtom( 0, ESCHER_ClientData );                       // OBJ record
    pMsodrawing->UpdateStopPos();
    pEx->CloseContainer();  // ESCHER_SpContainer
}

XclObjDropDown::~XclObjDropDown()
{   }

void XclObjDropDown::SaveCont( SvStream& rStrm )
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

UINT16 XclObjDropDown::GetLen() const
{
    // length of all subrecords including IDs and LENs
    // (all from XclObj) + ftSbs + ftLbsData + ftEnd
    return XclObj::GetLen() + 24 + 20 + 4;
}


// --- class XclTxo --------------------------------------------------

XclTxo::XclTxo( const String& rStr )
            :
            aText( rStr, (SC_XCL_RECLENMAX-1)/2 ),  // must fit into _one_ CONTINUE
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
        SfxItemSet aItemSet( *rTextObj.GetItemPool(), SDRATTR_START, SDRATTR_END );
        rTextObj.TakeAttributes( aItemSet, FALSE, FALSE );
        switch ( ((const SdrTextHorzAdjustItem&) (aItemSet.Get( SDRATTR_TEXT_HORZADJUST ))).GetValue() )
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
        switch ( ((const SdrTextVertAdjustItem&) (aItemSet.Get( SDRATTR_TEXT_VERTADJUST ))).GetValue() )
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
    aText.Assign( aStr, (SC_XCL_RECLENMAX-1)/2 );   // must fit into _one_ CONTINUE
}


XclTxo::~XclTxo()
{
}


void XclTxo::SaveCont( SvStream& rStrm )
{
    XclContinue aCont( rStrm, 0, GetLen() );
    UINT16 nTextLen = aText.GetLen();
    UINT16 nFormatLen = 0;
    if ( nTextLen && !nFormatLen )
        nFormatLen = 8 * 2; // length of CONTINUE record, not count of formats
    rStrm << nGrbit << nRot << UINT32(0) << UINT16(0)
        << nTextLen << nFormatLen << UINT32(0);

    // CONTINUE records are only written if there is some text
    if ( nTextLen )
    {
        // CONTINUE text
        aCont.NextRecord( 1 + (UINT16) aText.GetByteCount() );
        rStrm << aText.GetGrbit();
        aText.Write( aCont );

        // CONTINUE formatting runs
        aCont.NextRecord( nFormatLen );
        // write at least two dummy TXORUNs
        rStrm
            << UINT16(0)    // first character
            << UINT16(0)    // normal font
            << UINT32(0)    // Reserved
            << nTextLen
            << UINT16(0)    // normal font
            << UINT32(0)    // Reserved
            ;
    }
}


UINT16 XclTxo::GetNum() const
{
    return 0x01b6;
}


UINT16 XclTxo::GetLen() const
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


void XclObjOle::SaveCont( SvStream& rStrm )
{
    {   // own scope for record length (abused Continue)
        XclContinue aCont( rStrm, 0 );

        // ftCmo subrecord
        XclObj::SaveCont( rStrm );

        // write only as embedded, not linked
        String aStorageName( RTL_CONSTASCII_STRINGPARAM( "MBD" ) );
        sal_Char aBuf[ sizeof(UINT32) * 2 + 1 ];
        UINT32 nPictureId = UINT32(this);
        sprintf( aBuf, "%08X", nPictureId );
        aStorageName.AppendAscii( aBuf );
        SvStorageRef xOleStg = pRootStorage->OpenStorage( aStorageName,
                                STREAM_READWRITE| STREAM_SHARE_DENYALL );
        if( xOleStg.Is() )
        {
            SvInPlaceObjectRef xObj( ((SdrOle2Obj&)rOleObj).GetObjRef() );
            if ( xObj.Is() )
            {
                // set version to "old" version, because it must be
                // saved in MS notation.
                xOleStg->SetVersion( SOFFICE_FILEFORMAT_31 );
                xObj->DoSaveAs( &xOleStg );
                xObj->DoSaveCompleted();

                // ftCf subrecord, undocumented as usual
                rStrm << UINT16(ftCf) << UINT16(2) << UINT16(0x0002);

                // ftPioGrbit subrecord, undocumented as usual
                rStrm << UINT16(ftPioGrbit) << UINT16(2) << UINT16(0x0001);

                // ftPictFmla subrecord, undocumented as usual
                rStrm << UINT16(ftPictFmla) << UINT16(0);
                {   // own scope for subrecord length (abused Continue)
                    XclContinue aPictFmla( rStrm, 0 );
                    rStrm << UINT16(0);     // dummy formula length
                    {   // own scope for real formula length (abused Continue)
                        XclContinue aFmla( rStrm, 0 );
                        const UINT8 pData[] = {
                            0x05, 0x00,
//                          0xac, 0x10, 0xa4, 0x00,     // Xcl changes values on each object
                            0x00, 0x00, 0x00, 0x00,     // zeroed
                            0x02,                       // GPF if zeroed
//                          0x78, 0xa9, 0x86, 0x00,     // Xcl changes values on each object
                            0x00, 0x00, 0x00, 0x00,     // zeroed
                            0x03
                        };
                        rStrm.Write( pData, sizeof(pData) );
                        XclUnicodeString aName( xOleStg->GetUserName() );
                        aName.Write( aFmla );
                        if ( aName.GetByteCount() % 2 == 1 )
                            rStrm << UINT8(0);      // pad byte
                    }
                    rStrm << nPictureId;
                }
            }
        }

        // ftEnd subrecord
        rStrm << UINT16(0) << UINT16(0);
    }
    // second MSODRAWING record and TXO and CONTINUE records
    // we shouldn't have any here, so just in case ...
    SaveText( rStrm );
}


UINT16 XclObjOle::GetLen() const
{   // length of all subrecords including IDs and LENs
    // the job is done by XclContinue record header
    return 0;
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


void XclObjAny::SaveCont( SvStream& rStrm )
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
    // second MSODRAWING record and TXO and CONTINUE records
    SaveText( rStrm );
}


UINT16 XclObjAny::GetLen() const
{   // length of all subrecords including IDs and LENs
    UINT16 nLen = XclObj::GetLen() + 4;
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


void XclNoteList::_Save( SvStream& rStrm )
{   // nothing
}


void XclNoteList::SaveCont( SvStream& rStrm )
{
    for ( XclNote* p = First(); p; p = Next() )
    {
        p->Save( rStrm );
    }
}


UINT16 XclNoteList::GetNum() const
{   // dummy
    return 0;
}


UINT16 XclNoteList::GetLen() const
{   // dummy
    return 0;
}


// --- class XclNote -------------------------------------------------

XclNote::XclNote( RootData& rRoot, const ScAddress& rPos, const ScPostIt& rNote )
            :
            aAuthor( rNote.GetAuthor() ),
            aPos( rPos ),
            nGrbit(0)
{
    XclObjComment* pObj = new XclObjComment( rRoot, rPos, rNote.GetText() );
    nObjId = rRoot.pObjRecs->Add( pObj );
}


XclNote::~XclNote()
{
}


void XclNote::_Save( SvStream& rStrm )
{
    if ( nObjId )
        rStrm << GetNum() << GetLen();
}


void XclNote::SaveCont( SvStream& rStrm )
{
    if ( !nObjId )
        return ;

    XclContinue aCont( rStrm, 0, GetLen() );
    rStrm << (UINT16) aPos.Row() << (UINT16) aPos.Col() << nGrbit << nObjId;
    aAuthor.Write( aCont );
    if ( aAuthor.GetByteCount() % 2 )
        rStrm << UINT8(0);      // pad byte goes here, docs are wrong!
}


UINT16 XclNote::GetNum() const
{
    return 0x001c;
}


UINT16 XclNote::GetLen() const
{
    if ( !nObjId )
        return 0;

    UINT32 nLen = aAuthor.GetByteCount();
    if ( nLen % 2 )
        nLen++;     // pad byte
    return 8 + 1 + (UINT16) nLen;
}


// --- class XclCondFormat -------------------------------------------

void XclCondFormat::_Save( SvStream& rStrm )
{
    DBG_ASSERT( List::Count() < 65536, "+XclCondFormat::_Save(): to much CFs!" );

    // writing new condfmt / cf combination

    // write header
    rStrm << GetNum();

    UINT32              nLenPos = rStrm.Tell();

    rStrm << nOwnLen << ( UINT16 ) List::Count() << ( UINT16 ) 0x0000;

    nOwnLen = 14;       // base len without rerefs

    const UINT32        nMinMaxPos = rStrm.Tell();
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

            if( nRF < nRowFirst )
                nRowFirst = nRF;
            if( nRL > nRowLast )
                nRowLast = nRL;
            if( nCF < nColFirst )
                nColFirst = nCF;
            if( nCL > nColLast )
                nColLast = nCL;

            rStrm << nRF << nRL << nCF << nCL;
        }
        pAct = pRL->Next();
    }

    nOwnLen += 8 * nRefCnt;

    UINT32              nActPos = rStrm.Tell();

    // write record len
    rStrm.Seek( nLenPos );
    rStrm << nOwnLen;

    // write min / max and num of refs
    rStrm.Seek( nMinMaxPos );
    rStrm << nRowFirst << nRowLast << nColFirst << nColLast << nRefCnt;

    rStrm.Seek( nActPos );

    XclCf*              p = ( XclCf* ) ( ((XclCondFormat*)this)->First() );
    while( p )
    {
        p->Save( rStrm );

        p = ( XclCf* ) ( ((XclCondFormat*)this)->Next() );
    }
}


XclCondFormat::XclCondFormat( const ScConditionalFormat& _rCF, ScRangeList* _pRL, RootData& rER )
                :
                rCF( _rCF )
{
    pRL = _pRL;
    nTabNum = *rER.pAktTab;
    nComplLen = nOwnLen = 0;

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


UINT16 XclCondFormat::GetNum() const
{
    return 0x01B0;  // starts with CONDFMT, CF follows within data
}


UINT16 XclCondFormat::GetLen() const
{
    return nComplLen;       // not easy to predict!
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
    ExcUPN*         pForm1      = new ExcUPN( &rRD, *pScTokArry1, NULL, TRUE );
    nFormLen1 = pForm1->GetLen();

    ScTokenArray*   pScTokArry2;
    ExcUPN*         pForm2;
    if( bSingForm )
        nFormLen2 = 0;
    else
    {
        pScTokArry2 = r.CreateTokenArry( 1 );
        pForm2 = new ExcUPN( &rRD, *pScTokArry2, NULL, TRUE );
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


void XclCf::SaveCont( SvStream& rStrm )
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


UINT16 XclCf::GetLen() const
{
    return 6 + nFormatLen + nVarLen;
}


UINT16 XclObproj::GetNum() const
{
    return 0x00D3;
}


UINT16 XclObproj::GetLen() const
{
    return 0;
}



//============================================================================
// data consolidation reference


XclDConRef::XclDConRef( const ScRange& rSrcR, const String& rWB ) :
        aSourceRange( rSrcR )
{
    String  sTemp( ( sal_Unicode ) 0x02 );

    sTemp += rWB;
    pWorkbook = new XclRawUnicodeString( sTemp );
}

XclDConRef::~XclDConRef()
{
    delete pWorkbook;
}

void XclDConRef::SaveCont( SvStream& rStrm )
{
    rStrm   << (UINT16) aSourceRange.aStart.Row()
            << (UINT16) aSourceRange.aEnd.Row()
            << (UINT8)  aSourceRange.aStart.Col()
            << (UINT8)  aSourceRange.aEnd.Col()
            << (UINT16) pWorkbook->GetLen()
            << pWorkbook->GetGrbit();
    pWorkbook->WriteToStream( rStrm );
    rStrm   << (UINT8)  0x00;
}

UINT16 XclDConRef::GetNum() const
{
    return 0x0051;
}

UINT16 XclDConRef::GetLen() const
{
    return 10 + pWorkbook->GetByteCount();
}



//============================================================================
// merged cells
//_________________________________________________________
// class XclCellMerging - merged cells (max. 1024)

XclCellMerging::~XclCellMerging()
{   }

void XclCellMerging::Append( UINT16 nCol1, UINT16 nColCnt,
                             UINT16 nRow1, UINT16 nRowCnt )
{
    aCoordList.Append( nRow1 );
    aCoordList.Append( nRow1 + nRowCnt - 1 );
    aCoordList.Append( nCol1 );
    aCoordList.Append( nCol1 + nColCnt - 1 );
    nCount++;
}

void XclCellMerging::SaveCont( SvStream& rStrm )
{
    rStrm << nCount;
    for( UINT16 nInd = 0; nInd < nCount * 4; nInd++ )
        rStrm << aCoordList.Get( nInd );
}

UINT16 XclCellMerging::GetNum() const
{
    return 0x00E5;
}

UINT16 XclCellMerging::GetLen() const
{
    return nCount * 8 + 2;
}



//_________________________________________________________
// class XclCellMergingList - list of XclCellMerging

XclCellMergingList::~XclCellMergingList()
{
    for( XclCellMerging* pMrg = _First(); pMrg; pMrg = _Next() )
        delete pMrg;
}

XclCellMerging* XclCellMergingList::InsertNewRec()
{
    XclCellMerging* pMrg = new XclCellMerging;
    List::Insert( pMrg, LIST_APPEND );
    return pMrg;
}

void XclCellMergingList::Append( UINT16 nCol1, UINT16 nColCnt,
                                UINT16 nRow1, UINT16 nRowCnt )
{
    if( !pCurrRec )
        pCurrRec = InsertNewRec();
    if( pCurrRec->IsListFull() )
        pCurrRec = InsertNewRec();

    pCurrRec->Append( nCol1, nColCnt, nRow1, nRowCnt );
}


void XclCellMergingList::Save( SvStream& rStrm )
{
    for( XclCellMerging* pMrg = _First(); pMrg; pMrg = _Next() )
        pMrg->Save( rStrm );
}



// ---- class XclCodename --------------------------------------------

void XclCodename::SaveCont( SvStream& rStr )
{
    rStr << aName.GetLen() << aName.GetGrbit();
    aName.WriteToStream( rStr );
}


XclCodename::XclCodename( const String& r ) : aName( r )
{
}


UINT16 XclCodename::GetNum() const
{
    return 0x01BA;
}


UINT16 XclCodename::GetLen() const
{
    return aName.GetByteCount();
}




void XclBuildInName::_Save( SvStream& rStr )
{
//  CreateFormula();
    if( pData )
    {
        rStr << ( UINT16 ) 0x0018 << GetLen();
            //  grbit (built in only )      chKey           cch
        rStr << ( UINT16 ) 0x0020 << ( UINT8 ) 0x00 << ( UINT8 ) 0x01
            //  cce         itab                cch
            << nFormLen << nTabNum << ( UINT32 ) 0x00000001
            //      grbit string
            << ( UINT16 ) 0x00000 << ( UINT8 ) 0x00 << nKey;

        rStr.Write( pData, nFormLen );
    }
}


void XclBuildInName::Add( const ScRange& r )
{
    Append( r );
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


UINT16 XclBuildInName::GetLen() const
{
    return 16 + nFormLen;
}




XclPrintRange::XclPrintRange( RootData* p, UINT16 nTabNum, ScDocument& rDoc ) : XclBuildInName( p, nTabNum, 0x06 )
{
    if( rDoc.HasPrintRange() )
    {
        UINT16          nAnz = rDoc.GetPrintRangeCount( nTabNum );
        UINT16          n;

        for( n = 0 ; n < nAnz ; n++ )
            Add( *rDoc.GetPrintRange( nTabNum, n ) );

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
            Add( *pRepColRange );

        if( pRepRowRange )
            Add( *pRepRowRange );

        CreateFormula();
    }
}




//___________________________________________________________________
// Scenario export


ExcEScenarioCell::ExcEScenarioCell( UINT16 nC, UINT16 nR, const String& rTxt ) :
        nCol( nC ),
        nRow( nR ),
        sText( rTxt, EXC_SCEN_MAXSTRINGLEN )
{   }

void ExcEScenarioCell::WriteText( SvStream& rStrm )
{
    rStrm << sText.GetLen() << sText.GetGrbit();
    sText.WriteToStream( rStrm );
}




XclRawUnicodeString ExcEScenario::sUsername;

ExcEScenario::ExcEScenario( ScDocument& rDoc, UINT16 nTab )
{
    String  sTmpName;
    String  sTmpComm;
    Color   aDummyCol;
    UINT16  nDummyFlags;

    rDoc.GetName( nTab, sTmpName );
    sName.Assign( sTmpName, EXC_SCEN_MAXSTRINGLEN );
    nRecLen = 8 + sName.GetByteCount();

    rDoc.GetScenarioData( nTab, sTmpComm, aDummyCol, nDummyFlags );
    sComment.Assign( sTmpComm, EXC_SCEN_MAXSTRINGLEN );
    if( sComment.GetLen() )
        nRecLen += 3 + sComment.GetByteCount();

    if( !sUsername.GetLen() )
    {
        SvtUserOptions aUserOpt;
        sUsername.Assign( aUserOpt.GetLastName() );
    }
    if( !sUsername.GetLen() )
        sUsername.Assign( String::CreateFromAscii( "SC" ) );
    nRecLen += 3 + sUsername.GetByteCount();

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
                        ScGlobal::pScInternational->GetNumDecimalSep(), TRUE );
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

    UINT16 nNewLen = nRecLen + 6 + pCell->GetStringBytes();
    if( nNewLen > SC_XCL_RECLENMAX )
    {
        delete pCell;
        return FALSE;
    }

    List::Insert( pCell, LIST_APPEND );
    nRecLen = nNewLen;
    return TRUE;
}

void ExcEScenario::SaveCont( SvStream& rStrm )
{
    rStrm   << (UINT16) List::Count()       // number of cells
            << (UINT8) 1                    // fLocked
            << (UINT8) 0                    // fHidden
            << (UINT8) sName.GetLen()       // length of scen name
            << (UINT8) sComment.GetLen()    // length of comment
            << (UINT8) sUsername.GetLen()   // length of user name
            << sName.GetGrbit();
    sName.WriteToStream( rStrm );

    rStrm << sUsername.GetLen() << sUsername.GetGrbit();
    sUsername.WriteToStream( rStrm );

    if( sComment.GetLen() )
    {
        rStrm << sComment.GetLen() << sComment.GetGrbit();
        sComment.WriteToStream( rStrm );
    }

    ExcEScenarioCell* pCell;
    for( pCell = _First(); pCell; pCell = _Next() )
        pCell->WriteAddress( rStrm);        // pos of cell
    for( pCell = _First(); pCell; pCell = _Next() )
        pCell->WriteText( rStrm );          // string content
    for( UINT16 nCell = 0; nCell < List::Count(); nCell++ )
        rStrm << (UINT16) 0;                // date format
}

UINT16 ExcEScenario::GetNum() const
{
    return 0x00AF;
}

UINT16 ExcEScenario::GetLen() const
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

void ExcEScenarioManager::SaveCont( SvStream& rStrm )
{
    rStrm   << (UINT16) List::Count()       // number of scenarios
            << nActive                      // active scen
            << nActive                      // last displayed
            << (UINT16) 0;                  // reference areas

    for( ExcEScenario* pScen = _First(); pScen; pScen = _Next() )
        pScen->Save( rStrm );
}

void ExcEScenarioManager::Save( SvStream& rStrm )
{
    if( List::Count() )
        ExcRecord::Save( rStrm );
}

UINT16 ExcEScenarioManager::GetNum() const
{
    return 0x00AE;
}

UINT16 ExcEScenarioManager::GetLen() const
{
    return 8;
}




const BYTE XclHlink::pStaticData1[] =
{
    0xD0, 0xC9, 0xEA, 0x79, 0xF9, 0xBA, 0xCE, 0x11,
    0x8C, 0x82, 0x00, 0xAA, 0x00, 0x4B, 0xA9, 0x0B,
    0x02, 0x00, 0x00, 0x00
};


inline UINT32 XclHlink::StaticLen( void )
{
    return sizeof( pStaticData1 );
}


inline UINT32 XclHlink::VarLen( void ) const
{
    pVarData->Seek( STREAM_SEEK_TO_END );
    return pVarData->Tell();
}


void XclHlink::SaveCont( SvStream& rOut )
{
    if( IsValid() )
    {
        rOut << nRowFirst << nRowFirst << nColFirst << nColFirst;

        rOut.Write( pStaticData1, sizeof( pStaticData1 ) );

        rOut << nFlags;

        pVarData->Seek( STREAM_SEEK_TO_BEGIN );
        rOut << *pVarData;
    }
}


#define CHARCOUNT   TRUE
#define BYTECOUNT   FALSE


static void lcl_SaveHlinkString16( SvStream& rOut, const String& rStr, BOOL bCharCount )
{
    XclRawUnicodeString     s( rStr, 0xFFFF, TRUE );
    UINT32                  nLen = bCharCount? s.GetLen() + 1 : s.GetByteCount() + 2;   // + 0-Byte!

    rOut << nLen;
    s.WriteToStream( rOut );
    rOut << ( UINT16 ) 0x0000;
}


static ByteString lcl_Get83Repr( const INetURLObject& r, CharSet e )
{
    String      aExt( r.GetExtension() );
    String      aName( r.GetBase() );

    if( aExt.Len() > 3 )
        aExt.Erase( 0, 3 );
    aExt.ToUpperAscii();

    String      aS( RTL_CONSTASCII_STRINGPARAM( "..\\") );
    String      aR;
    while( aName.SearchAndReplace( aS, aR ) != STRING_NOTFOUND );

    if( aName.Len() > 8 )
    {
        aName.Erase( 6 );
        aName.AppendAscii( "~1" );
    }
    aName.ToUpperAscii();

    aName.AppendAscii( "." );
    aName += aExt;

    return ByteString( aName, e );
}


XclHlink::XclHlink( RootData& rR, const SvxURLField& rURLField )
{
    pVarData = NULL;
    pRepr = NULL;
    {
        nFlags = 0x00000001;

        const XubString&    rURL = rURLField.GetURL();
        const XubString&    rRepr = rURLField.GetRepresentation();
        const XubString&    rTargFrame = rURLField.GetTargetFrame();

        SvxURLFormat        eFormat = rURLField.GetFormat();

        INetURLObject       aURLObj( rURL );

        const INetProtocol  eProtocol = aURLObj.GetProtocol();

        UINT32              n1, n2;
        const BOOL          bWithRepr = rRepr.Len() > 0;

        SvMemoryStream&     rOut = *( pVarData = new SvMemoryStream );

        if( bWithRepr )
        {
            if( pRepr )
                delete pRepr;
            pRepr = new String( rRepr );
        }

        if( eProtocol == INET_PROT_FILE )
        {
            BOOL            bRel = rR.bStoreRel;
            String          aPathAndName( aURLObj.getFSysPath( INetURLObject::FSYS_DOS ) );
            UINT32          nLen;

            UINT16          nDL = 0;

            String          aOrgPaN( aPathAndName );
            if( bRel )
            {
                DBG_ASSERT( rR.pBasePath, "-XclHlink::XclHlink(): on the meadow is not... :-)" );

                aPathAndName = aURLObj.GetRelURL( *rR.pBasePath, aPathAndName, INetURLObject::WAS_ENCODED, INetURLObject::DECODE_WITH_CHARSET );

                if( aPathAndName.SearchAscii( INET_FILE_SCHEME ) == 0 )
                {   // not converted to rel -> make abs
                    aPathAndName = aOrgPaN;
                    bRel = FALSE;
                }
                else
                {
                    if( aPathAndName.SearchAscii( "./" ) == 0 )
                        aPathAndName.Erase( 0, 2 );
                    else
                    {
                        while( aPathAndName.SearchAndReplaceAscii( "../", EMPTY_STRING ) != STRING_NOTFOUND )
                            nDL++;
                    }
                }
            }

            // 8.3-representation... not really useful but necessary
            n1 = 0x00000303;
            n2 = 0x00000000;

            ByteString      aEtRepr( lcl_Get83Repr( aURLObj, *rR.pCharset ) );
            nLen = aEtRepr.Len() + 1;   // + 0-Byte

                                // C0 00 00 00 00 00 00 46
            rOut << n1 << n2 << ( UINT32 ) 0x000000C0 << ( UINT32 ) 0x46000000 << nDL << nLen;
            rOut.Write( aEtRepr.GetBuffer(), nLen - 1 );
            rOut << ( UINT8 ) 0x00;

            // full path and filename
            n1 = 0xDEADFFFF;
            n2 = 0x00000000;
                                // 4x non-interesting data...
            rOut << n1 << n2 << n2 << n2 << n2 << n2;

            XclRawUnicodeString     s( aPathAndName, 0xFFFF, TRUE );
            nLen = s.GetByteCount();
            n1 = bRel? 0x00000040 : 0x00000004;
            //                      UNICODE-flags?
            rOut << n1 << nLen << ( UINT16 ) 0x0003;
            s.WriteToStream( rOut );
        }
        else
        {
            if( bWithRepr )
                lcl_SaveHlinkString16( rOut, rRepr, CHARCOUNT );

            String          aURL = aURLObj.GetURLNoMark();

            n1 = 0x79EAC9E0;
            n2 = 0x11CEBAF9;
                                // 8C 82 00 AA 00 4B A9 0B
            rOut << n1 << n2 << ( UINT32 ) 0xAA00828C << ( UINT32 ) 0x0BA94B00;

            lcl_SaveHlinkString16( rOut, aURL, BYTECOUNT );

            if( !pRepr )
                pRepr = new String( aURL );
        }

        if( aURLObj.HasMark() )
        {
            String          aMark = aURLObj.GetMark();
            lcl_SaveHlinkString16( rOut, aMark, CHARCOUNT );

            nFlags |= 0x00000008;
        }
    }
}


XclHlink::~XclHlink()
{
    if( pVarData )
        delete pVarData;

    if( pRepr )
        delete pRepr;
}


UINT16 XclHlink::GetNum() const
{
    return 0x01B8;
}


UINT16 XclHlink::GetLen() const
{
    pVarData->Seek( STREAM_SEEK_TO_END );
    return IsValid()? ( UINT16 ) ( 12 + StaticLen() + VarLen() ) : 0;
                    // 12 = cols/rows + flags
}




/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.1.1.1  2000/09/18 16:45:15  hr
    initial import

    Revision 1.32  2000/09/17 14:08:51  willem.vandorp
    OpenOffice header added.

    Revision 1.31  2000/09/01 10:00:45  gt
    export hyperlink

    Revision 1.29  2000/08/14 11:29:05  dr
    Chart export, part 1: common formats: chart & axis titles, legend, colors, fonts

    Revision 1.28  2000/07/28 15:08:42  gt
    export print range/title + im-/export styles

    Revision 1.27  2000/07/14 07:55:34  dr
    export: external references

    Revision 1.26  2000/07/11 13:51:56  er
    merged from src594 branch

    Revision 1.25  2000/07/11 10:04:04  er
    NOOLDSV

    Revision 1.24  2000/07/04 07:33:42  dr
    export: scenarios

    Revision 1.23  2000/06/23 11:35:04  dr
    add: class XclObjDropDown

    Revision 1.22  2000/06/06 12:14:55  hr
    syntax

    Revision 1.21  2000/05/29 14:32:01  dr
    class ExcPalette2 - color reduction algorithm

    Revision 1.20  2000/05/15 16:28:09  nn
    include

    Revision 1.19  2000/05/08 14:02:20  gt
    #75548# merge from branch

    Revision 1.18  2000/05/05 18:53:22  nn
    src569 changes merged

    Revision 1.17  2000/05/05 15:29:18  dr
    new: XclCellMerging, XclCellMergingList

    Revision 1.16  2000/05/05 12:20:59  gt
    UNICODE

    Revision 1.15  2000/04/27 08:11:37  dr
    pivot tables export

    Revision 1.14  2000/04/27 06:27:14  gt
    #75275# references in conditional format formulas as in shared formulas

    Revision 1.13  2000/02/25 12:00:08  gt
    #73585# Filter Options and VBA-recognition by Excel

    Revision 1.12  2000/02/17 15:32:49  er
    #73145# XclObjAny: write ftGmo subrecord if otGroup

    Revision 1.11  2000/02/07 19:09:50  er
    #72775# new: XclObjOle

    Revision 1.10  2000/01/31 08:44:58  gt
    #72481# export conditional format

    Revision 1.9  1999/09/29 16:02:36  er
    new: ExcBofC8

    Revision 1.8  1999/09/22 19:12:10  er
    chg: OBJ records

    Revision 1.7  1999/08/06 22:34:38  ER
    add: XclObjComment: temporary test code erDebugEscherExAddSdr


      Rev 1.6   07 Aug 1999 00:34:38   ER
   add: XclObjComment: temporary test code erDebugEscherExAddSdr

      Rev 1.5   27 Jul 1999 16:55:32   ER
   new: Msodrawinggroup, Msodrawing, Obj, ObjComment, Note, NoteList, Txo

      Rev 1.4   01 Jul 1999 17:21:56   ER
   chg: WINDOW2 sets selected sheet tab, ExcDummy_03 only SELECTION

      Rev 1.3   16 Jun 1999 19:58:18   ER
   new: XclObjList, XclObj, XclObjComment, XclTxo, XclNote

      Rev 1.2   11 Jun 1999 20:08:12   ER
   xcl97exp Formula: References, SupbookList, ExtsheetList, literal Strings

      Rev 1.1   03 Jun 1999 18:42:38   ER
   xcl97exp Font, Format, RichStr

      Rev 1.0   01 Jun 1999 13:43:38   ER
   Xcl97 Export Zwischenstand
------------------------------------------------------------------------*/

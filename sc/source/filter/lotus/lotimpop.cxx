/*************************************************************************
 *
 *  $RCSfile: lotimpop.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dr $ $Date: 2001-02-26 06:58:59 $
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

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#include "attrib.hxx"
#include "document.hxx"
#include "rangenam.hxx"
#include "cell.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "compiler.hxx"
#include "global.hxx"

#include "root.hxx"
#include "flttools.hxx"
#include "lotimpop.hxx"
#include "lotfntbf.hxx"
#include "lotform.hxx"
#include "tool.h"
#include "namebuff.hxx"
#include "lotrange.hxx"
#include "lotattr.hxx"


static NAMESPACE_VOS( OMutex )      aLotImpSemaphore;


ImportLotus::ImportLotus( SvStream& aStream, ScDocument* pDoc, CharSet eQ ) :
    ImportTyp( aStream, pDoc, eQ ),
    pIn( &aStream ),
    aConv( *pIn, eQ )
{
    pExtOpt = NULL;

    // good point to start locking of import lotus
    aLotImpSemaphore.acquire();

    pLotusRoot = new LOTUS_ROOT;
    pLotusRoot->pDoc = pDoc;
    pLotusRoot->pRangeNames = new LotusRangeList;
    pLotusRoot->pScRangeName = pDoc->GetRangeName();
    pLotusRoot->eCharsetQ = eQ;
    pLotusRoot->eFirstType = Lotus_X;
    pLotusRoot->eActType = Lotus_X;
    pLotusRoot->pRngNmBffWK3 = new RangeNameBufferWK3;
    pFontBuff = pLotusRoot->pFontBuff = new LotusFontBuffer;
    pLotusRoot->pAttrTable = new LotAttrTable;
}


ImportLotus::~ImportLotus()
{
    delete pLotusRoot->pRangeNames;
    delete pLotusRoot->pRngNmBffWK3;
    delete pFontBuff;
    delete pLotusRoot->pAttrTable;
    delete pLotusRoot;

#ifdef DBG_UTIL
    pLotusRoot = NULL;
#endif

    // no need 4 pLotusRoot anymore
    aLotImpSemaphore.release();
}


void ImportLotus::Bof( void )
{
    UINT16  nFileCode, nFileSub, nSaveCnt;
    BYTE    nMajorId, nMinorId, nFlags;

    Read( nFileCode );
    Read( nFileSub );
    Read( pLotusRoot->aActRange );
    Read( nSaveCnt );
    Read( nMajorId );
    Read( nMinorId );
    Skip( 1 );
    Read( nFlags );

    if( nFileSub == 0x0004 )
    {
        if( nFileCode == 0x1000 )
        {// <= WK3
            pLotusRoot->eFirstType = pLotusRoot->eActType = Lotus_WK3;
        }
        else if( nFileCode == 0x1002 )
        {// WK4
            pLotusRoot->eFirstType = pLotusRoot->eActType = Lotus_WK4;
        }
    }
}


BOOL ImportLotus::BofFm3( void )
{
    UINT16  nFileCode, nFileSub;

    Read( nFileCode );
    Read( nFileSub );

    return ( nFileCode == 0x8007 && ( nFileSub == 0x0000 || nFileSub == 0x00001 ) );
}


void ImportLotus::Columnwidth( UINT16 nRecLen )
{
    DBG_ASSERT( nRecLen >= 4, "*ImportLotus::Columnwidth(): Record zu kurz!" );

    BYTE    nTab, nWindow2;
    UINT16  nCnt = ( nRecLen - 4 ) / 2;

    Read( nTab );
    Read( nWindow2 );

    if( !pD->HasTable( nTab ) )
        pD->MakeTable( nTab );

    if( !nWindow2 )
    {
        Skip( 2 );

        BYTE    nCol, nSpaces;

        while( nCnt )
        {
            Read( nCol );
            Read( nSpaces );
            // ACHTUNG: Korrekturfaktor nach 'Augenmass' ermittelt!
            pD->SetColWidth( nCol, nTab, ( UINT16 ) ( TWIPS_PER_CHAR * 1.28 * nSpaces ) );

            nCnt--;
        }
    }
}


void ImportLotus::Hiddencolumn( UINT16 nRecLen )
{
    DBG_ASSERT( nRecLen >= 4, "*ImportLotus::Hiddencolumn(): Record zu kurz!" );

    BYTE    nTab, nWindow2;
    UINT16  nCnt = ( nRecLen - 4 ) / 2;

    Read( nTab );
    Read( nWindow2 );

    if( !nWindow2 )
    {
        Skip( 2 );

        BYTE    nCol;

        while( nCnt )
        {
            Read( nCol );

            pD->SetColFlags( nCol, nTab, pD->GetColFlags( nCol, nTab ) | CR_HIDDEN );

            nCnt--;
        }
    }
}


void ImportLotus::Userrange( void )
{
    UINT16      nRangeType;
    ScRange     aScRange;
    sal_Char*   pBuffer = new sal_Char[ 32 ];

    Read( nRangeType );

    pIn->Read( pBuffer, 16 );
    pBuffer[ 16 ] = ( sal_Char ) 0x00;  // zur Sicherheit...
    String      aName( pBuffer, eQuellChar );

    Read( aScRange );

    pLotusRoot->pRngNmBffWK3->Add( aName, aScRange );
    delete[] pBuffer;
}


void ImportLotus::Errcell( void )
{
    ScAddress   aA;

    Read( aA );

    pD->PutCell( aA.Col(), aA.Row(), aA.Tab(), new ScStringCell( _STRINGCONST( "#ERR!" ) ), (BOOL)TRUE );
}


void ImportLotus::Nacell( void )
{
    ScAddress   aA;

    Read( aA );

    pD->PutCell( aA.Col(), aA.Row(), aA.Tab(), new ScStringCell( _STRINGCONST( "#NA!" ) ), (BOOL)TRUE );
}


void ImportLotus::Labelcell( void )
{
    ScAddress   aA;
    String      aLabel;
    sal_Char    cAlign;

    Read( aA );
    Read( cAlign );
    Read( aLabel );

//  aLabel.Convert( pLotusRoot->eCharsetQ );

    pD->PutCell( aA.Col(), aA.Row(), aA.Tab(), new ScStringCell( aLabel ), (BOOL)TRUE );
}


void ImportLotus::Numbercell( void )
    {
    ScAddress   aAddr;
    double      fVal;

    Read( aAddr );
    Read( fVal );

    pD->PutCell( aAddr.Col(), aAddr.Row(), aAddr.Tab(),
        new ScValueCell( fVal ), (BOOL)TRUE );
    }


void ImportLotus::Smallnumcell( void )
    {
    ScAddress   aAddr;
    INT16       nVal;

    Read( aAddr );
    Read( nVal );

    pD->PutCell( aAddr.Col(), aAddr.Row(), aAddr.Tab(),
        new ScValueCell( SnumToDouble( nVal ) ), ( BOOL ) TRUE );
    }


ScFormulaCell *ImportLotus::Formulacell( UINT16 n )
    {
    DBG_ASSERT( pIn, "-ImportLotus::Formulacell(): Null-Stream -> Rums!" );

    ScAddress           aAddr;

    Read( aAddr );
    Skip( 10 );

    n -= 14;

    const ScTokenArray* pErg;
    INT32               nRest = n;

    aConv.Reset( aAddr );
    aConv.SetWK3();
    aConv.Convert( pErg, nRest );

    ScFormulaCell*      pZelle = new ScFormulaCell( pD, aAddr, pErg );

    pZelle->AddRecalcMode( RECALCMODE_ONLOAD_ONCE );

    pD->PutCell( aAddr.Col(), aAddr.Row(), aAddr.Tab(), pZelle, (BOOL)TRUE );

    return NULL;
    }


void ImportLotus::Read( String &r )
{
    ScFilterTools::AppendCString( *pIn, r, eQuellChar );
}


void ImportLotus::RowPresentation( UINT16 nRecLen )
{
    DBG_ASSERT( nRecLen > 4, "*ImportLotus::RowPresentation(): Record zu kurz!" );

    BYTE    nTab, nFlags;
    UINT16  nRow, nHeight;
    UINT16  nCnt = ( nRecLen - 4 ) / 8;

    Read( nTab );
    Skip( 1 );

    while( nCnt )
    {
        Read( nRow );
        Read( nHeight );
        Skip( 2 );
        Read( nFlags );
        Skip( 1 );

        if( nFlags & 0x02 )     // Fixed / Strech to fit fonts
        {   // fixed
            // Height in Lotus in 1/32 Points
            nHeight *= 20;  // -> 32 * TWIPS
            nHeight /= 32;  // -> TWIPS

            pD->SetRowFlags( nRow, nTab, pD->GetRowFlags( nRow, nTab ) | CR_MANUALSIZE );

            pD->SetRowHeight( nRow, nTab, nHeight );
        }

        nCnt--;
    }
}


void ImportLotus::NamedSheet( void )
{
    UINT16  nTab;
    String  aName;

    Read( nTab );
    Read( aName );

    if( pD->HasTable( nTab ) )
        pD->RenameTab( nTab, aName );
    else
        pD->InsertTab( nTab, aName );
}


void ImportLotus::Font_Face( void )
{
    BYTE    nNum;
    String  aName;

    Read( nNum );

    // ACHTUNG: QUICK-HACK gegen unerklaerliche Loops
    if( nNum > 7 )
        return;
    // ACHTUNG

    Read( aName );

    pFontBuff->SetName( nNum, aName );
}


void ImportLotus::Font_Type( void )
{
    static const UINT16 nAnz = 8;
    UINT16              nCnt;
    UINT16              nType;

    for( nCnt = 0 ; nCnt < nAnz ; nCnt++ )
    {
        Read( nType );
        pFontBuff->SetType( nCnt, nType );
    }
}


void ImportLotus::Font_Ysize( void )
{
    static const UINT16 nAnz = 8;
    UINT16              nCnt;
    UINT16              nSize;

    for( nCnt = 0 ; nCnt < nAnz ; nCnt++ )
    {
        Read( nSize );
        pFontBuff->SetHeight( nCnt, nSize );
    }
}


void ImportLotus::_Row( const UINT16 nRecLen )
    {
    DBG_ASSERT( nExtTab >= 0, "*ImportLotus::_Row(): Kann hier nicht sein!" );

    UINT16          nRow;
    UINT16          nHeight;
    UINT16          nCntDwn = ( nRecLen - 4 ) / 5;
    UINT16          nColCnt = 0;
    UINT8           nRepeats;
    LotAttrWK3      aAttr;

    BOOL            bCenter = FALSE;
    UINT16          nCenterStart, nCenterEnd;

    Read( nRow );
    Read( nHeight );

    nHeight &= 0x0FFF;
    nHeight *= 22;

    if( nHeight )
        pD->SetRowHeight( nRow, ( UINT16 ) nExtTab, nHeight );

    while( nCntDwn )
        {
        Read( aAttr );
        Read( nRepeats );

        if( aAttr.HasStyles() )
            pLotusRoot->pAttrTable->SetAttr(
                ( UINT8 ) nColCnt, ( UINT8 ) ( nColCnt + nRepeats ), nRow, aAttr );

        // hier und NICHT in class LotAttrTable, weil nur Attributiert wird,
        // wenn die anderen Attribute gesetzt sind
        //  -> bei Center-Attribute wird generell zentriert gesetzt
        if( aAttr.IsCentered() )
            {
            if( bCenter )
                {
                if( pD->HasData( nColCnt, nRow, ( UINT16 ) nExtTab ) )
                    {// neue Center nach vorheriger Center
                    pD->DoMerge( ( UINT16 ) nExtTab, nCenterStart, nRow, nCenterEnd, nRow );
                    nCenterStart = ( UINT16 ) nColCnt;
                    }
                }
            else
                {// ganz neue Center
                bCenter = TRUE;
                nCenterStart = ( UINT16 ) nColCnt;
                }
            nCenterEnd = nColCnt + nRepeats;
            }
        else
            {
            if( bCenter )
                {// evtl. alte Center bemachen
                pD->DoMerge( ( UINT16 ) nExtTab, nCenterStart, nRow, nCenterEnd, nRow );
                bCenter = FALSE;
                }
            }

        nColCnt += nRepeats;
        nColCnt++;

        nCntDwn--;
        }

    if( bCenter )
        // evtl. alte Center bemachen
        pD->DoMerge( ( UINT16 ) nExtTab, nCenterStart, nRow, nCenterEnd, nRow );
    }



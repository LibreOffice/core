/*************************************************************************
 *
 *  $RCSfile: excform8.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:11 $
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

#include "cell.hxx"
#include "document.hxx"
#include "rangenam.hxx"

#include "excimp8.hxx"
#include "root.hxx"
#include "excform.hxx"



// ------------------------------------------------------------------ 0x06

void ImportExcel8::Formula( void )
{
    UINT16  nRow, nCol, nXF, nFormLen;
    double  fCurVal;
    BYTE    nFlag0;
    BOOL    bShrFmla;

    aIn >> nRow >> nCol;
    nBytesLeft -= 4;

    aIn >> nXF >> fCurVal >> nFlag0;
    aIn.SeekRel( 5 );

    aIn >> nFormLen;
    nBytesLeft -= 18;

    bShrFmla = nFlag0 & 0x08;   // shared or not shared

    nLastXF = nXF;

    Formula( nCol, nRow, nTab, nXF, nFormLen, fCurVal, nFlag0, bShrFmla );
}



void ImportExcel8::Formula( UINT16 nCol, UINT16 nRow, UINT16 nTab,
    UINT16 nXF, UINT16 nFormLen, double& rCurVal, BYTE nFlag, BOOL bShrFmla )
{
    ConvErr eErr = ConvOK;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        // jetzt steht Lesemarke auf Formel, Laenge in nFormLen
        const ScTokenArray* pErgebnis;
        INT32               nAnzBytes = nFormLen;
        BOOL                bConvert;

        pFormConv->Reset( nFormLen, ScAddress( nCol, nRow, nTab ) );

        if( bShrFmla )
            bConvert = !pFormConv->GetShrFmla( pErgebnis, nAnzBytes );
        else
            bConvert = TRUE;

        if( bConvert )
            eErr = pFormConv->Convert( pErgebnis, nAnzBytes );

        nBytesLeft += nAnzBytes - nFormLen;

        ScFormulaCell *pZelle = NULL;

        if( pErgebnis )
        {
            pZelle = new ScFormulaCell( pD, ScAddress( nCol, nRow, nTab ), pErgebnis );

            pD->PutCell( nCol, nRow, nTab, pZelle, (BOOL)TRUE );

            aColRowBuff.Used( nCol, nRow );
        }
        else
        {
            CellType    eCellType;
            ScBaseCell  *pBaseCell;
            pD->GetCellType( nCol, nRow, nTab, eCellType );
            if( eCellType == CELLTYPE_FORMULA )
            {
                pD->GetCell( nCol, nRow, nTab, pBaseCell );
                pZelle = ( ScFormulaCell * ) pBaseCell;
                if( pZelle )
                    pZelle->AddRecalcMode( RECALCMODE_ONLOAD_ONCE );
            }
        }

        if( pZelle )
        {
            if( eErr != ConvOK )
                ExcelToSc::SetError( *pZelle, eErr );
            else if( ExcelToSc::SetCurVal( *pZelle, rCurVal ) )
                pLastFormCell = pZelle; // String-Record sollte folgen
            else
                pLastFormCell = NULL;
        }
        else
            pLastFormCell = NULL;

        pFltTab->SetXF( nCol, nRow, nXF );
    }
    else
        bTabTruncated = TRUE;
}




ExcelToSc8::ExcelToSc8( RootData* pRD, SvStream& aStr, const UINT16& rOrgTab ) :
    ExcelToSc( pRD, aStr, rOrgTab )
{
}


ExcelToSc8::~ExcelToSc8()
{
}


ConvErr ExcelToSc8::Convert( const ScTokenArray*& pErgebnis, INT32& rRestbytes, const FORMULA_TYPE eFT )
{
    BYTE                    nOp, nLen, nByte;
    UINT16                  nUINT16, nIndexToFunc;
    INT16                   nINT16;
    double                  fDouble;
    String                  aString;
    BOOL                    bError = FALSE;
    BOOL                    bArrayFormula = FALSE;
    TokenId                 nMerk0;
    const BOOL              bRangeName = eFT == FT_RangeName;
    const BOOL              bSharedFormula = eFT == FT_SharedFormula;
    const BOOL              bRNorSF = bRangeName || bSharedFormula;
    const XtiBuffer&        rXtiBuffer = *pExcRoot->pXtiBuffer;
    const SupbookBuffer&    rSupbookBuffer = *pExcRoot->pSupbookBuffer;
    const Xti*              pXti;
    const CharSet           eCharSet = *pExcRoot->pCharset;

    SingleRefData           aSRD;
    aSRD.InitFlags();
    ComplRefData            aCRD;
    aCRD.InitFlags();

    bExternName = FALSE;

    nBytesLeft = rRestbytes;

    if( eStatus != ConvOK )
        return eStatus;

    if( rRestbytes == 0 )
    {
        aPool.Store( _STRINGCONST( "-/-" ) );
        aPool >> aStack;
        pErgebnis = aPool[ aStack.Get() ];
        return ConvOK;
    }

    while( nBytesLeft > 0 && !bError )
    {
        Read( nOp );

        switch( nOp )   //                              Buch Seite:
        {           //                                      SDK4 SDK5
            case 0x01: // Array Formula                         [325    ]
                       // Array Formula or Shared Formula       [    277]
            case 0x02: // Data Table                            [325 277]
                Ignore( 4 );

                bArrayFormula = TRUE;
                break;
            case 0x03: // Addition                              [312 264]
                aStack >> nMerk0;
                aPool <<  aStack << ocAdd << nMerk0;
                aPool >> aStack;
                break;
            case 0x04: // Subtraction                           [313 264]
                // SECOMD-TOP minus TOP
                aStack >> nMerk0;
                aPool << aStack << ocSub << nMerk0;
                aPool >> aStack;
                break;
            case 0x05: // Multiplication                        [313 264]
                aStack >> nMerk0;
                aPool << aStack << ocMul << nMerk0;
                aPool >> aStack;
                break;
            case 0x06: // Division                              [313 264]
                // divide TOP by SECOND-TOP
                aStack >> nMerk0;
                aPool << aStack << ocDiv << nMerk0;
                aPool >> aStack;
                break;
            case 0x07: // Exponetiation                         [313 265]
                // raise SECOND-TOP to power of TOP
                aStack >> nMerk0;
                aPool << aStack << ocPow << nMerk0;
                aPool >> aStack;
                break;
            case 0x08: // Concatenation                         [313 265]
                // append TOP to SECOND-TOP
                aStack >> nMerk0;
                aPool << aStack << ocAmpersand << nMerk0;
                aPool >> aStack;
                break;
            case 0x09: // Less Than                             [313 265]
                // SECOND-TOP < TOP
                aStack >> nMerk0;
                aPool << aStack << ocLess << nMerk0;
                aPool >> aStack;
                break;
            case 0x0A: // Less Than or Equal                    [313 265]
                // SECOND-TOP <= TOP
                aStack >> nMerk0;
                aPool << aStack << ocLessEqual << nMerk0;
                aPool >> aStack;
                break;
            case 0x0B: // Equal                                 [313 265]
                // SECOND-TOP == TOP
                aStack >> nMerk0;
                aPool << aStack << ocEqual << nMerk0;
                aPool >> aStack;
                break;
            case 0x0C: // Greater Than or Equal                 [313 265]
                // SECOND-TOP == TOP
                aStack >> nMerk0;
                aPool << aStack << ocGreaterEqual << nMerk0;
                aPool >> aStack;
                break;
            case 0x0D: // Greater Than                          [313 265]
                // SECOND-TOP == TOP
                aStack >> nMerk0;
                aPool << aStack << ocGreater << nMerk0;
                aPool >> aStack;
                break;
            case 0x0E: // Not Equal                             [313 265]
                // SECOND-TOP == TOP
                aStack >> nMerk0;
                aPool << aStack << ocNotEqual << nMerk0;
                aPool >> aStack;
                break;
            case 0x0F: // Intersection                          [314 265]
                aStack >> nMerk0;
                aPool << aStack << ocIntersect << nMerk0;
                aPool >> aStack;
                break;
            case 0x10: // Union                                 [314 265]
                // ocSep behelfsweise statt 'ocUnion'
                aStack >> nMerk0;
                aPool << ocOpen << aStack << ocSep << nMerk0 << ocClose;
                    // doesn't fit exactly, but is more Excel-like
                aPool >> aStack;
                break;
            case 0x11: // Range                                 [314 265]
            {
                // wenn erster und zweiter Ausdruck auf'm Stack Single Referenzen
                // sind, dann Area Reference erzeugen, ansonsten Fehlerhaften Aus-
                // druck generieren
                BOOL    bErrorToken = TRUE;
                TokenId nMerk1;

                aStack >> nMerk0;
                aStack >> nMerk1;

                if( aPool.GetType( nMerk0 ) == T_RefC &&
                    aPool.GetType( nMerk1 ) == T_RefC )
                {
                    register const SingleRefData *pRef1 = aPool.GetSRD( nMerk1 );
                    register const SingleRefData *pRef2 = aPool.GetSRD( nMerk0 );
                    if( pRef1 && pRef2 )
                    {
                        aCRD.Ref1 = *pRef1;
                        aCRD.Ref2 = *pRef2;
                        aStack << aPool.Store( aCRD );
                        bErrorToken = FALSE;
                    }
                }

                if( bErrorToken )
                {
                    aPool << ocNoName << ocOpen << nMerk1 << ocSep
                        << nMerk0 << ocClose;

                    aPool >> aStack;
                }
            }
                break;
            case 0x12: // Unary Plus                            [312 264]
                break;
            case 0x13: // Unary Minus                           [312 264]
                aPool << ocNegSub << aStack;
                aPool >> aStack;
                break;
            case 0x14: // Percent Sign                          [312 264]
                // <Stack> + '/' + '100'
                nMerk0 = aPool.Store( 100.0 );
                aPool << ocOpen << aStack << ocDiv << nMerk0 << ocClose;
                aPool >> aStack;
                break;
            case 0x15: // Parenthesis                           [326 278]
                aPool << ocOpen << aStack << ocClose;
                aPool >> aStack;
                break;
            case 0x16: // Missing Argument                      [314 266]
                aPool << ocMissing;
                aPool >> aStack;
                break;
            case 0x17: // String Constant                       [314 266]
                Read( nLen );       // und?

                if( nLen )
                    aString = ::ReadUnicodeString( aIn, nBytesLeft, eCharSet, nLen );
                else
                {
                    aString = EMPTY_STRING;
                    Ignore( 1 );    // Grbit ueberlesen
                }

                aStack << aPool.Store( aString );
                break;
            case 0x19: // Special Attribute                     [327 279]
            {
                UINT16 nData, nFakt;
                BYTE nOpt;

                Read( nOpt );

                Read( nData );
                nFakt = 2;

                if( nOpt & 0x04 )
                {// nFakt -> Bytes oder Words ueberlesen    AttrChoose
                    nData++;
                    Ignore( nData * nFakt );
                }
                else if( nOpt & 0x10 )                      // AttrSum
                    DoMulArgs( ocSum, 1 );
            }
                break;
            case 0x1C: // Error Value                           [314 266]
            {
                Read( nByte );
                DefTokenId          eOc;
                switch( nByte )
                {
                    case 0x00:
                    case 0x07:
                    case 0x17:
                    case 0x1D:
                    case 0x24:      eOc = ocStop;       break;
                    case 0x0F:
                    case 0x2A:      eOc = ocNoValue;    break;
                    default:        eOc = ocNoName;
                }
                aPool << eOc;
                if( eOc != ocStop )
                    aPool << ocOpen << ocClose;

                aPool >> aStack;
            }
                break;
            case 0x1D: // Boolean                               [315 266]
                Read( nByte );
                if( nByte == 0 )
                    aPool << ocFalse << ocOpen << ocClose;
                else
                    aPool << ocTrue << ocOpen << ocClose;
                aPool >> aStack;
                break;
            case 0x1E: // Integer                               [315 266]
                Read( nUINT16 );
                aStack << aPool.Store( ( double ) nUINT16 );
                break;
            case 0x1F: // Number                                [315 266]
                Read( fDouble );
                aStack << aPool.Store( fDouble );
                break;
            case 0x40:
            case 0x60:
            case 0x20: // Array Constant                        [317 268]
                Ignore( 7 );
                aPool << ocBad;
                aPool >> aStack;
                break;
            case 0x41:
            case 0x61:
            case 0x21: // Function, Fixed Number of Arguments   [333 282]
                Read( nIndexToFunc );

                DoDefArgs( nIndexToFunc );
                break;
            case 0x42:
            case 0x62:
            case 0x22: // Function, Variable Number of Arg.     [333 283]
            {
                BYTE nAnz;
                Read( nAnz );
                nAnz &= 0x7F;

                Read( nIndexToFunc );

                DoMulArgs( IndexToToken( nIndexToFunc ), nAnz );
            }
                break;
            case 0x43:
            case 0x63:
            case 0x23: // Name                                  [318 269]
                Read( nUINT16 );
                Ignore( 2 );

                aStack << aPool.Store( ( *pExcRoot->pRNameBuff )[ nUINT16 ] );
                break;
            case 0x44:
            case 0x64:
            case 0x24: // Cell Reference                        [319 270]
            case 0x4A:
            case 0x6A:
            case 0x2A: // Deleted Cell Reference                [323 273]
            {
                UINT16          nCol, nRow;

                aIn >> nRow >> nCol;
                nBytesLeft -= 4;

                aSRD.nCol = nCol;
                aSRD.nRow = nRow & 0x3FFF;
                aSRD.nRelTab = 0;
                aSRD.SetTabRel( TRUE );
                aSRD.SetFlag3D( bRangeName );

                ExcelToSc8::ExcRelToScRel( nRow, nCol, aSRD, bRangeName );

                switch ( nOp )
            {
                    case 0x4A:
                    case 0x6A:
                    case 0x2A: // Deleted Cell Reference        [323 273]
                        // no information which part is deleted, set both
                        aSRD.SetColDeleted( TRUE );
                        aSRD.SetRowDeleted( TRUE );
            }

                aStack << aPool.Store( aSRD );
            }
                break;
            case 0x45:
            case 0x65:
            case 0x25: // Area Reference                        [320 270]
            case 0x4B:
            case 0x6B:
            case 0x2B: // Deleted Area Refernce                 [323 273]
            {
                UINT16          nRowFirst, nRowLast;
                UINT16          nColFirst, nColLast;
                SingleRefData   &rSRef1 = aCRD.Ref1;
                SingleRefData   &rSRef2 = aCRD.Ref2;

                aIn >> nRowFirst >> nRowLast >> nColFirst >> nColLast;
                nBytesLeft -= 8;

                rSRef1.nRelTab = rSRef2.nRelTab = 0;
                rSRef1.SetTabRel( TRUE );
                rSRef2.SetTabRel( TRUE );
                rSRef1.SetFlag3D( bRangeName );
                rSRef2.SetFlag3D( bRangeName );

                ExcelToSc8::ExcRelToScRel( nRowFirst, nColFirst, aCRD.Ref1, bRangeName );
                ExcelToSc8::ExcRelToScRel( nRowLast, nColLast, aCRD.Ref2, bRangeName );

                if( IsComplColRange( nColFirst, nColLast ) )
                    SetComplCol( aCRD );
                else if( IsComplRowRange( nRowFirst, nRowLast ) )
                    SetComplRow( aCRD );

                switch ( nOp )
            {
                    case 0x4B:
                    case 0x6B:
                    case 0x2B: // Deleted Area Refernce         [323 273]
                        // no information which part is deleted, set all
                        rSRef1.SetColDeleted( TRUE );
                        rSRef1.SetRowDeleted( TRUE );
                        rSRef2.SetColDeleted( TRUE );
                        rSRef2.SetRowDeleted( TRUE );
            }

                aStack << aPool.Store( aCRD );
            }
                break;
            case 0x46:
            case 0x66:
            case 0x26: // Constant Reference Subexpression      [321 271]
                Ignore( 6 );        // mehr steht da nicht!
                break;
            case 0x47:
            case 0x67:
            case 0x27: // Erroneous Constant Reference Subexpr. [322 272]
                Ignore( 6 );    // mehr steht da nicht!
                aPool << ocBad;
                aPool >> aStack;
                break;
            case 0x48:
            case 0x68:
            case 0x28: // Incomplete Constant Reference Subexpr.[331 281]
                Ignore( 6 );    // mehr steht da nicht!
                aPool << ocBad;
                aPool >> aStack;
                break;
            case 0x49:
            case 0x69:
            case 0x29: // Variable Reference Subexpression      [331 281]
                Ignore( 2 );    // mehr steht da nicht!
                break;
            case 0x4C:
            case 0x6C:
            case 0x2C: // Cell Reference Within a Name          [323    ]
                       // Cell Reference Within a Shared Formula[    273]
            {
                UINT16      nRow, nCol;

                aIn >> nRow >> nCol;
                nBytesLeft -= 4;

                aSRD.nRelTab = 0;
                aSRD.SetTabRel( TRUE );
                aSRD.SetFlag3D( bRangeName );

                ExcelToSc8::ExcRelToScRel( nRow, nCol, aSRD, bRNorSF );

                aStack << aPool.Store( aSRD );
            }
                break;
            case 0x4D:
            case 0x6D:
            case 0x2D: // Area Reference Within a Name          [324    ]
            {      // Area Reference Within a Shared Formula[    274]
                UINT16                  nRowFirst, nRowLast;
                UINT16                  nColFirst, nColLast;

                aCRD.Ref1.nRelTab = aCRD.Ref2.nRelTab = 0;
                aCRD.Ref1.SetTabRel( TRUE );
                aCRD.Ref2.SetTabRel( TRUE );
                aCRD.Ref1.SetFlag3D( bRangeName );
                aCRD.Ref2.SetFlag3D( bRangeName );

                aIn >> nRowFirst >> nRowLast >> nColFirst >> nColLast;
                nBytesLeft -= 8;

                ExcelToSc8::ExcRelToScRel( nRowFirst, nColFirst, aCRD.Ref1, bRNorSF );
                ExcelToSc8::ExcRelToScRel( nRowLast, nColLast, aCRD.Ref2, bRNorSF );

                if( IsComplColRange( nColFirst, nColLast ) )
                    SetComplCol( aCRD );
                else if( IsComplRowRange( nRowFirst, nRowLast ) )
                    SetComplRow( aCRD );

                aStack << aPool.Store( aCRD );
            }
                break;
            case 0x4E:
            case 0x6E:
            case 0x2E: // Reference Subexpression Within a Name [332 282]
                Ignore( 2 );    // mehr steht da nicht!
                aPool << ocBad;
                aPool >> aStack;
                break;
            case 0x4F:
            case 0x6F:
            case 0x2F: // Incomplete Reference Subexpression... [332 282]
                Ignore( 2 );    // mehr steht da nicht!
                aPool << ocBad;
                aPool >> aStack;
                break;
            case 0x58:
            case 0x78:
            case 0x38: // Command-Equivalent Function           [333    ]
                aString.AssignAscii( "COMM_EQU_FUNC" );
                Read( nByte );
                aString += String::CreateFromInt32( nByte );
                Read( nByte );
                aStack << aPool.Store( aString );
                DoMulArgs( ocPush, nByte + 1 );
                break;
            case 0x59:
            case 0x79:
            case 0x39: // Name or External Name                 [    275]
                Read( nINT16 );
                Read( nUINT16 );
                Ignore( 2 );
                if( nINT16 >= 0 )
                {
                    const ExtName*  pExtName;
                    pExtName = pExcRoot->pExtNameBuff->GetName( nUINT16 );

                    if( pExtName )
                    {
                        nINT16++;
                        if( pExtName->IsDDE() && pExcRoot->pExtSheetBuff->IsLink( ( UINT16 ) nINT16 ) )
                        {
                            String          aAppl, aExtDoc;
                            TokenId         nPar1, nPar2;

                            pExcRoot->pExtSheetBuff->GetLink( ( UINT16 ) nINT16 , aAppl, aExtDoc );
                            nPar1 = aPool.Store( aAppl );
                            nPar2 = aPool.Store( aExtDoc );
                            nMerk0 = aPool.Store( pExtName->aName );
                            aPool   << ocDde << ocOpen << nPar1 << ocSep << nPar2 << ocSep
                                    << nMerk0 << ocClose;
                            aPool >> aStack;
                        }
                        else
                            aStack << aPool.Store( ocNoName, pExtName->aName );
                    }
                    else
                    {
                        //aStack << ocNoName;
                        aPool << ocBad;
                        aPool >> aStack;
                    }
                }
                else
                    aStack << aPool.Store( ( *pExcRoot->pRNameBuff )[ nUINT16 ] );
                break;
            case 0x5A:
            case 0x7A:
            case 0x3A: // 3-D Cell Reference                    [    275]
            case 0x5C:
            case 0x7C:
            case 0x3C: // Deleted 3-D Cell Reference            [    277]
            {
                UINT16          nIxti, nRw, nGrbitCol;

                aIn >> nIxti >> nRw >> nGrbitCol;
                nBytesLeft -= 6;

                pXti = rXtiBuffer.Get( nIxti );

                if( pXti )
                {
                    const SupbookE* pSbE = rSupbookBuffer.Get( pXti->nSupbook );

                    if( pSbE )
                    {
                        UINT16  nTabFirst, nTabLast;
                        if( pSbE->IsExternal() )
                        {
                            nTabFirst = pSbE->GetScTabNum( pXti->nFirst );
                            nTabLast = pSbE->GetScTabNum( pXti->nLast );
                        }
                        else
                        {
                            nTabFirst = pXti->nFirst;
                            nTabLast = pXti->nLast;
                        }
                        BOOL    b3D = ( nTabFirst != aEingPos.Tab() ) || bRangeName;

                        aSRD.nTab = nTabFirst;
                        aSRD.SetFlag3D( b3D );
                        aSRD.SetTabRel( FALSE );

                        ExcRelToScRel( nRw, nGrbitCol, aSRD, bRangeName );

                        switch ( nOp )
                    {
                            case 0x5C:
                            case 0x7C:
                            case 0x3C: // Deleted 3-D Cell Reference    [    277]
                                // no information which part is deleted, set both
                                aSRD.SetColDeleted( TRUE );
                                aSRD.SetRowDeleted( TRUE );
                    }
                        if ( nTabFirst > MAXTAB )
                            aSRD.SetTabDeleted( TRUE );

                        if( nTabLast != nTabFirst )
                        {
                            aCRD.Ref1 = aCRD.Ref2 = aSRD;
                            aCRD.Ref2.nTab = nTabLast;
                            b3D = ( nTabLast != aEingPos.Tab() );
                            aStack << aPool.Store( aCRD );
                        }
                        else
                            aStack << aPool.Store( aSRD );
                    }
                }
                else
                {
                    aPool << ocBad;
                    aPool >> aStack;
                }
            }
                break;
            case 0x5B:
            case 0x7B:
            case 0x3B: // 3-D Area Reference                    [    276]
            case 0x5D:
            case 0x7D:
            case 0x3D: // Deleted 3-D Area Reference            [    277]
            {
                UINT16          nIxti, nRw1, nGrbitCol1, nRw2, nGrbitCol2;

                aIn >> nIxti >> nRw1 >> nRw2 >> nGrbitCol1 >> nGrbitCol2;
                nBytesLeft -= 10;

                pXti = rXtiBuffer.Get( nIxti );

                if( pXti )
                {
                    const SupbookE* pSbE = rSupbookBuffer.Get( pXti->nSupbook );

                    if( pSbE )
                    {
                        UINT16  nTabFirst, nTabLast;
                        if( pSbE->IsExternal() )
                        {
                            nTabFirst = pSbE->GetScTabNum( pXti->nFirst );
                            nTabLast = pSbE->GetScTabNum( pXti->nLast );
                        }
                        else
                        {
                            nTabFirst = pXti->nFirst;
                            nTabLast = pXti->nLast;
                        }

                        SingleRefData   &rR1 = aCRD.Ref1;
                        SingleRefData   &rR2 = aCRD.Ref2;

                        rR1.nTab = nTabFirst;
                        rR2.nTab = nTabLast;
                        rR1.SetFlag3D( ( nTabFirst != aEingPos.Tab() ) || bRangeName );
                        rR1.SetTabRel( FALSE );
                        rR2.SetFlag3D( ( nTabLast != aEingPos.Tab() ) || bRangeName );
                        rR2.SetTabRel( FALSE );

                        ExcelToSc8::ExcRelToScRel( nRw1, nGrbitCol1, aCRD.Ref1, bRangeName );
                        ExcelToSc8::ExcRelToScRel( nRw2, nGrbitCol2, aCRD.Ref2, bRangeName );

                        if( IsComplColRange( nGrbitCol1, nGrbitCol2 ) )
                            SetComplCol( aCRD );
                        else if( IsComplRowRange( nRw1, nRw2 ) )
                            SetComplRow( aCRD );

                        switch ( nOp )
                    {
                            case 0x5D:
                            case 0x7D:
                            case 0x3D: // Deleted 3-D Area Reference    [    277]
                                // no information which part is deleted, set all
                                rR1.SetColDeleted( TRUE );
                                rR1.SetRowDeleted( TRUE );
                                rR2.SetColDeleted( TRUE );
                                rR2.SetRowDeleted( TRUE );
                    }
                        if ( nTabFirst > MAXTAB )
                            rR1.SetTabDeleted( TRUE );
                        if ( nTabLast > MAXTAB )
                            rR2.SetTabDeleted( TRUE );

                        aStack << aPool.Store( aCRD );
                    }
                    else
                    {
                        aPool << ocBad;
                        aPool >> aStack;
                    }
                }
                else
                {
                    aPool << ocBad;
                    aPool >> aStack;
                }
            }
                break;
            default: bError = TRUE;
        }
    }

    ConvErr eRet;

    if( bError )
    {
        aPool << ocBad;
        aPool >> aStack;
        pErgebnis = aPool[ aStack.Get() ];
        eRet = ConvErrNi;
    }
    else if( nBytesLeft != 0 )
    {
        aPool << ocBad;
        aPool >> aStack;
        pErgebnis = aPool[ aStack.Get() ];
        eRet = ConvErrCount;
    }
    else if( bExternName )
    {
        rRestbytes = nBytesLeft;

        pErgebnis = aPool[ aStack.Get() ];
        eRet = ConvErrExternal;
    }
    else if( bArrayFormula )
    {
        rRestbytes = nBytesLeft;

        pErgebnis = NULL;
        eRet = ConvOK;
    }
    else
    {
        rRestbytes = nBytesLeft;

        pErgebnis = aPool[ aStack.Get() ];

        eRet = ConvOK;
    }

    return eRet;
}



ConvErr ExcelToSc8::Convert( _ScRangeListTabs& rRangeList, INT32 &rRestbytes, const FORMULA_TYPE eFT )
{
    BYTE                    nOp, nLen;//, nByte;
    BOOL                    bError = FALSE;
    BOOL                    bArrayFormula = FALSE;
    const BOOL              bRangeName = eFT == FT_RangeName;
    const BOOL              bSharedFormula = eFT == FT_SharedFormula;
    const BOOL              bRNorSF = bRangeName || bSharedFormula;
    const XtiBuffer&        rXtiBuffer = *pExcRoot->pXtiBuffer;
    const SupbookBuffer&    rSupbookBuffer = * pExcRoot->pSupbookBuffer;
    const Xti*              pXti;

    SingleRefData           aSRD;
    aSRD.InitFlags();
    ComplRefData            aCRD;
    aCRD.InitFlags();

    bExternName = FALSE;

    nBytesLeft = rRestbytes;

    if( eStatus != ConvOK )
        return eStatus;

    if( rRestbytes == 0 )
        return ConvOK;

    while( nBytesLeft > 0 && !bError )
    {
        Read( nOp );

        switch( nOp )   //                              Buch Seite:
        {           //                                      SDK4 SDK5
            case 0x01: // Array Formula                         [325    ]
                       // Array Formula or Shared Formula       [    277]
                Ignore( 4 );

                bArrayFormula = TRUE;
                break;
            case 0x02: // Data Table                            [325 277]
                Ignore( 4 );
                break;
            case 0x03: // Addition                              [312 264]
            case 0x04: // Subtraction                           [313 264]
            case 0x05: // Multiplication                        [313 264]
            case 0x06: // Division                              [313 264]
            case 0x07: // Exponetiation                         [313 265]
            case 0x08: // Concatenation                         [313 265]
            case 0x09: // Less Than                             [313 265]
            case 0x0A: // Less Than or Equal                    [313 265]
            case 0x0B: // Equal                                 [313 265]
            case 0x0C: // Greater Than or Equal                 [313 265]
            case 0x0D: // Greater Than                          [313 265]
            case 0x0E: // Not Equal                             [313 265]
            case 0x0F: // Intersection                          [314 265]
            case 0x10: // Union                                 [314 265]
            case 0x11: // Range                                 [314 265]
            case 0x12: // Unary Plus                            [312 264]
            case 0x13: // Unary Minus                           [312 264]
            case 0x14: // Percent Sign                          [312 264]
            case 0x15: // Parenthesis                           [326 278]
            case 0x16: // Missing Argument                      [314 266]
                break;
            case 0x17: // String Constant                       [314 266]
                Read( nLen );       // und?

                if( nLen )
                    ::SkipUnicodeString( aIn, nBytesLeft, nLen );
                else
                    Ignore( 1 );    // Grbit ueberlesen
                break;
            case 0x19: // Special Attribute                     [327 279]
            {
                UINT16 nData, nFakt;
                BYTE nOpt;

                Read( nOpt );

                Read( nData );
                nFakt = 2;

                if( nOpt & 0x04 )
                {// nFakt -> Bytes oder Words ueberlesen    AttrChoose
                    nData++;
                    Ignore( nData * nFakt );
                }
            }
                break;
            case 0x1C: // Error Value                           [314 266]
            case 0x1D: // Boolean                               [315 266]
                Ignore( 1 );
                break;
            case 0x1E: // Integer                               [315 266]
                Ignore( 2 );
                break;
            case 0x1F: // Number                                [315 266]
                Ignore( sizeof( double ) );
                break;
            case 0x40:
            case 0x60:
            case 0x20: // Array Constant                        [317 268]
                Ignore( 7 );
                break;
            case 0x41:
            case 0x61:
            case 0x21: // Function, Fixed Number of Arguments   [333 282]
                Ignore( 2 );
                break;
            case 0x42:
            case 0x62:
            case 0x22: // Function, Variable Number of Arg.     [333 283]
                Ignore( 3 );
                break;
            case 0x43:
            case 0x63:
            case 0x23: // Name                                  [318 269]
                Ignore( 4 );
                break;
            case 0x44:
            case 0x64:
            case 0x24: // Cell Reference                        [319 270]
            {
                UINT16          nCol, nRow;

                aIn >> nRow >> nCol;
                nBytesLeft -= 4;

                aSRD.nCol = nCol;
                aSRD.nRow = nRow & 0x3FFF;
                aSRD.nRelTab = 0;
                aSRD.SetTabRel( TRUE );
                aSRD.SetFlag3D( bRangeName );

                ExcelToSc8::ExcRelToScRel( nRow, nCol, aSRD, bRangeName );

                rRangeList.Append( aSRD );
            }
                break;
            case 0x45:
            case 0x65:
            case 0x25: // Area Reference                        [320 270]
            {
                UINT16          nRowFirst, nRowLast;
                UINT16          nColFirst, nColLast;
                SingleRefData   &rSRef1 = aCRD.Ref1;
                SingleRefData   &rSRef2 = aCRD.Ref2;

                aIn >> nRowFirst >> nRowLast >> nColFirst >> nColLast;
                nBytesLeft -= 8;

                rSRef1.nRelTab = rSRef2.nRelTab = 0;
                rSRef1.SetTabRel( TRUE );
                rSRef2.SetTabRel( TRUE );
                rSRef1.SetFlag3D( bRangeName );
                rSRef2.SetFlag3D( bRangeName );

                ExcelToSc8::ExcRelToScRel( nRowFirst, nColFirst, aCRD.Ref1, bRangeName );
                ExcelToSc8::ExcRelToScRel( nRowLast, nColLast, aCRD.Ref2, bRangeName );

                if( IsComplColRange( nColFirst, nColLast ) )
                    SetComplCol( aCRD );
                else if( IsComplRowRange( nRowFirst, nRowLast ) )
                    SetComplRow( aCRD );

                rRangeList.Append( aCRD );
            }
                break;
            case 0x46:
            case 0x66:
            case 0x26: // Constant Reference Subexpression      [321 271]
            case 0x47:
            case 0x67:
            case 0x27: // Erroneous Constant Reference Subexpr. [322 272]
            case 0x48:
            case 0x68:
            case 0x28: // Incomplete Constant Reference Subexpr.[331 281]
                Ignore( 6 );    // mehr steht da nicht!
                break;
            case 0x49:
            case 0x69:
            case 0x29: // Variable Reference Subexpression      [331 281]
                Ignore( 2 );    // mehr steht da nicht!
                break;
            case 0x4A:
            case 0x6A:
            case 0x2A: // Deleted Cell Reference                [323 273]
                Ignore( 3 );
                break;
            case 0x4B:
            case 0x6B:
            case 0x2B: // Deleted Area Refernce                 [323 273]
                Ignore( 6 );
                break;
            case 0x4C:
            case 0x6C:
            case 0x2C: // Cell Reference Within a Name          [323    ]
                       // Cell Reference Within a Shared Formula[    273]
            {
                UINT16      nRow, nCol;

                aIn >> nRow >> nCol;
                nBytesLeft -= 4;

                aSRD.nRelTab = 0;
                aSRD.SetTabRel( TRUE );
                aSRD.SetFlag3D( bRangeName );

                ExcelToSc8::ExcRelToScRel( nRow, nCol, aSRD, bRNorSF );

                rRangeList.Append( aSRD );
            }
                break;
            case 0x4D:
            case 0x6D:
            case 0x2D: // Area Reference Within a Name          [324    ]
            {      // Area Reference Within a Shared Formula[    274]
                UINT16                  nRowFirst, nRowLast;
                UINT16                  nColFirst, nColLast;

                aCRD.Ref1.nRelTab = aCRD.Ref2.nRelTab = 0;
                aCRD.Ref1.SetTabRel( TRUE );
                aCRD.Ref2.SetTabRel( TRUE );
                aCRD.Ref1.SetFlag3D( bRangeName );
                aCRD.Ref2.SetFlag3D( bRangeName );

                aIn >> nRowFirst >> nRowLast >> nColFirst >> nColLast;
                nBytesLeft -= 8;

                ExcelToSc8::ExcRelToScRel( nRowFirst, nColFirst, aCRD.Ref1, bRNorSF );
                ExcelToSc8::ExcRelToScRel( nRowLast, nColLast, aCRD.Ref2, bRNorSF );

                if( IsComplColRange( nColFirst, nColLast ) )
                    SetComplCol( aCRD );
                else if( IsComplRowRange( nRowFirst, nRowLast ) )
                    SetComplRow( aCRD );

                rRangeList.Append( aCRD );
            }
                break;
            case 0x4E:
            case 0x6E:
            case 0x2E: // Reference Subexpression Within a Name [332 282]
            case 0x4F:
            case 0x6F:
            case 0x2F: // Incomplete Reference Subexpression... [332 282]
            case 0x58:
            case 0x78:
            case 0x38: // Command-Equivalent Function           [333    ]
                Ignore( 2 );
                break;
            case 0x59:
            case 0x79:
            case 0x39: // Name or External Name                 [    275]
                Ignore( 24 );
                break;
            case 0x5A:
            case 0x7A:
            case 0x3A: // 3-D Cell Reference                    [    275]
            {
                UINT16          nIxti, nRw, nGrbitCol;

                aIn >> nIxti >> nRw >> nGrbitCol;
                nBytesLeft -= 6;

                pXti = rXtiBuffer.Get( nIxti );

                if( pXti )
                {
                    const SupbookE* pSbE = rSupbookBuffer.Get( pXti->nSupbook );

                    if( pSbE )
                    {
//                      if( pSbE->IsSameSheet() )
                        {// in aktuellem Workbook
                            UINT16  nTabFirst = pXti->nFirst;
                            UINT16  nTabLast = pXti->nLast;
                            BOOL b3D = ( nTabFirst != aEingPos.Tab() ) || bRangeName;

                            aSRD.nTab = nTabFirst;
                            aSRD.SetFlag3D( b3D );
                            aSRD.SetTabRel( FALSE );

                            ExcRelToScRel( nRw, nGrbitCol, aSRD, bRangeName );

                            if( nTabLast != nTabFirst )
                            {
                                aCRD.Ref1 = aSRD;
                                aCRD.Ref2.nCol = aSRD.nCol;
                                aCRD.Ref2.nRow = aSRD.nRow;
                                aCRD.Ref2.nTab = nTabLast;
                                b3D = ( nTabLast != aEingPos.Tab() );
                                aCRD.Ref2.SetFlag3D( b3D );
                                aCRD.Ref2.SetTabRel( FALSE );
                                rRangeList.Append( aCRD );
                            }
                            else
                                rRangeList.Append( aSRD );
                        }
                    }
                }
            }
                break;
            case 0x5B:
            case 0x7B:
            case 0x3B: // 3-D Area Reference                    [    276]
            {
                UINT16          nIxti, nRw1, nGrbitCol1, nRw2, nGrbitCol2;

                aIn >> nIxti >> nRw1 >> nRw2 >> nGrbitCol1 >> nGrbitCol2;
                nBytesLeft -= 10;

                pXti = rXtiBuffer.Get( nIxti );

                if( pXti )
                {
                    const SupbookE* pSbE = rSupbookBuffer.Get( pXti->nSupbook );

                    if( pSbE )
                    {
                        UINT16  nTabFirst = pXti->nFirst;
                        UINT16  nTabLast = pXti->nLast;

                        SingleRefData   &rR1 = aCRD.Ref1;
                        SingleRefData   &rR2 = aCRD.Ref2;

                        rR1.nTab = nTabFirst;
                        rR2.nTab = nTabLast;
                        rR1.SetFlag3D( ( nTabFirst != aEingPos.Tab() ) || bRangeName );
                        rR1.SetTabRel( FALSE );
                        rR2.SetFlag3D( ( nTabLast != aEingPos.Tab() ) || bRangeName );
                        rR2.SetTabRel( FALSE );

                        ExcelToSc8::ExcRelToScRel( nRw1, nGrbitCol1, aCRD.Ref1, bRangeName );
                        ExcelToSc8::ExcRelToScRel( nRw2, nGrbitCol2, aCRD.Ref2, bRangeName );

                        if( IsComplColRange( nGrbitCol1, nGrbitCol2 ) )
                            SetComplCol( aCRD );
                        else if( IsComplRowRange( nRw1, nRw2 ) )
                            SetComplRow( aCRD );

                        rRangeList.Append( aCRD );
                    }
                }
            }
                break;
            case 0x5C:
            case 0x7C:
            case 0x3C: // Deleted 3-D Cell Reference            [    277]
                Ignore( 17 );
                break;
            case 0x5D:
            case 0x7D:
            case 0x3D: // Deleted 3-D Area Reference            [    277]
                Ignore( 20 );
                break;
            default:
                bError = TRUE;
        }
    }

    ConvErr eRet;

    if( bError )
        eRet = ConvErrNi;
    else if( nBytesLeft != 0 )
        eRet = ConvErrCount;
    else if( bExternName )
        eRet = ConvErrExternal;
    else
        eRet = ConvOK;

    return eRet;
}



void ExcelToSc8::ExcRelToScRel( UINT16 nRow, UINT16 nC, SingleRefData &rSRD, const BOOL bName )
{
    const BOOL      bColRel = ( nC & 0x4000 ) != 0;
    const BOOL      bRowRel = ( nC & 0x8000 ) != 0;
    const UINT8     nCol = ( UINT8 ) nC;

    rSRD.SetColRel( bColRel );
    rSRD.SetRowRel( bRowRel );

    if( bName )
    {
        // C O L
        if( bColRel )
            //                                                          rel Col
            rSRD.nRelCol = ( INT8 ) nCol;
        else
            //                                                          abs Col
            rSRD.nCol = nCol;

        // R O W
        if( bRowRel )
            //                                                          rel Row
            rSRD.nRelRow = ( INT16 ) nRow;
        else
            //                                                          abs Row
            rSRD.nRow = nRow;

        // T A B
        // #67965# abs needed if rel in shared formula for ScCompiler UpdateNameReference
        if ( rSRD.IsTabRel() && !rSRD.IsFlag3D() )
            rSRD.nTab = *pExcRoot->pAktTab;
    }
    else
    {
        // C O L
        if ( bColRel )
            rSRD.nRelCol = (INT16) nCol - aEingPos.Col();
        else
            rSRD.nCol = nCol;

        // R O W
        if ( bRowRel )
            rSRD.nRelRow = (INT16) nRow - aEingPos.Row();
        else
            rSRD.nRow = nRow;
    }
}


BOOL ExcelToSc8::GetAbsRefs( SvStream& rIn, INT32& nBytesLeft, ScRangeList& r )
{
    UINT8                   nOp;
    UINT16                  nRow1, nRow2, nCol1, nCol2, nTab1, nTab2;
    UINT16                  nIxti;

    const XtiBuffer&        rXtiBuffer = *pExcRoot->pXtiBuffer;
    const SupbookBuffer&    rSupbookBuffer = *pExcRoot->pSupbookBuffer;
    const Xti*              pXti;
    UINT32                  nSeek;

    while( nBytesLeft > 0 )
    {
        rIn >> nOp;
        nBytesLeft--;
        nSeek = 0;

        switch( nOp )
        {
            case 0x44:
            case 0x64:
            case 0x24: // Cell Reference                        [319 270]
            case 0x4C:
            case 0x6C:
            case 0x2C: // Cell Reference Within a Name          [323    ]
                       // Cell Reference Within a Shared Formula[    273]
                aIn >> nRow1 >> nCol1;
                nBytesLeft -= 4;

                nRow2 = nRow1;
                nCol2 = nCol1;
                nTab1 = nTab2 = *pExcRoot->pAktTab;
                goto _common;
            case 0x45:
            case 0x65:
            case 0x25: // Area Reference                        [320 270]
            case 0x4D:
            case 0x6D:
            case 0x2D: // Area Reference Within a Name          [324    ]
                       // Area Reference Within a Shared Formula[    274]
                aIn >> nRow1 >> nRow2 >> nCol1 >> nCol2;
                nBytesLeft -= 8;

                nTab1 = nTab2 = *pExcRoot->pAktTab;
                goto _common;
            case 0x5A:
            case 0x7A:
            case 0x3A: // 3-D Cell Reference                    [    275]
                aIn >> nIxti >> nRow1 >> nCol1;
                nBytesLeft -= 6;

                nRow2 = nRow1;
                nCol2 = nCol1;

                goto _3d_common;
            case 0x5B:
            case 0x7B:
            case 0x3B: // 3-D Area Reference                    [    276]
                aIn >> nIxti >> nRow1 >> nRow2 >> nCol1 >> nCol2;
                nBytesLeft -= 10;

    _3d_common:
                pXti = rXtiBuffer.Get( nIxti );

                if( pXti )
                {
                    const SupbookE* pSbE = rSupbookBuffer.Get( pXti->nSupbook );

                    nTab1 = pXti->nFirst;
                    nTab2 = pXti->nLast;
                }
                else
                    break;

                goto _common;
    _common:
                if( !( nCol1 & 0xC000 ) || ( nCol2 & 0xC000 ) )
                    r.Insert(
                        new ScRange( ScAddress( nCol1, nRow1, nTab1 ), ScAddress( nCol2, nRow2, nTab2 ) ),
                        LIST_APPEND );
                break;
            case 0x1C: // Error Value                           [314 266]
            case 0x1D: // Boolean                               [315 266]
                nSeek = 1;
                break;
            case 0x1E: // Integer                               [315 266]
            case 0x41:
            case 0x61:
            case 0x21: // Function, Fixed Number of Arguments   [333 282]
            case 0x49:
            case 0x69:
            case 0x29: // Variable Reference Subexpression      [331 281]
            case 0x4E:
            case 0x6E:
            case 0x2E: // Reference Subexpression Within a Name [332 282]
            case 0x4F:
            case 0x6F:
            case 0x2F: // Incomplete Reference Subexpression... [332 282]
            case 0x58:
            case 0x78:
            case 0x38: // Command-Equivalent Function           [333    ]
                nSeek = 2;
                break;
            case 0x42:
            case 0x62:
            case 0x22: // Function, Variable Number of Arg.     [333 283]
                nSeek = 3;
                break;
            case 0x01: // Array Formula                         [325    ]
            case 0x02: // Data Table                            [325 277]
            case 0x43:
            case 0x63:
            case 0x23: // Name                                  [318 269]
            case 0x4A:
            case 0x6A:
            case 0x2A: // Deleted Cell Reference                [323 273]
                nSeek = 4;
                break;
            case 0x46:
            case 0x66:
            case 0x26: // Constant Reference Subexpression      [321 271]
            case 0x47:
            case 0x67:
            case 0x27: // Erroneous Constant Reference Subexpr. [322 272]
            case 0x48:
            case 0x68:
            case 0x28: // Incomplete Constant Reference Subexpr.[331 281]
            case 0x5C:
            case 0x7C:
            case 0x3C: // Deleted 3-D Cell Reference            [    277]
                nSeek = 6;
                break;
            case 0x40:
            case 0x60:
            case 0x20: // Array Constant                        [317 268]
                nSeek = 7;
                break;
            case 0x1F: // Number                                [315 266]
            case 0x4B:
            case 0x6B:
            case 0x2B: // Deleted Area Refernce                 [323 273]
                nSeek = 8;
                break;
            case 0x5D:
            case 0x7D:
            case 0x3D: // Deleted 3-D Area Reference            [    277]
                nSeek = 10;
                break;
            case 0x59:
            case 0x79:
            case 0x39: // Name or External Name                 [    275]
                nSeek = 24;
                break;
            case 0x17: // String Constant                       [314 266]
            {
                UINT8   nLen;
                rIn >>nLen;
                nBytesLeft--;

                if( nLen )
                {
                    ::SkipUnicodeString( rIn, nBytesLeft, nLen );
                    nSeek = 0;
                }
                else
                    nSeek = 1;
            }
                break;
            case 0x19: // Special Attribute                     [327 279]
            {
                UINT16  nData;
                UINT8   nOpt;
                rIn >> nOpt >> nData;
                nBytesLeft -= sizeof( nOpt ) + sizeof( nData );
                if( nOpt & 0x04 )
                {// nFakt -> Bytes oder Words ueberlesen    AttrChoose
                    nData++;
                    nSeek = nData * 2;
            }
            }
                break;
        }

        rIn.SeekRel( nSeek );
        nBytesLeft -= nSeek;
    }

    return r.Count() != 0;
}






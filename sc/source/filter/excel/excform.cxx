/*************************************************************************
 *
 *  $RCSfile: excform.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: dr $ $Date: 2001-03-13 15:31:24 $
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


//#include <string.h>

#include "cell.hxx"
#include "document.hxx"
#include "rangenam.hxx"
#include "global.hxx"
#include "compiler.hxx"

#include "imp_op.hxx"
#include "root.hxx"
#include "excform.hxx"


const UINT16 ExcelToSc::nRowMask = 0x3FFF;
const UINT16 ExcelToSc::nLastInd = 399;




void ImportExcel::Formula25()
{
    UINT16  nRow, nCol, nXF, nFormLen;
    double  fCurVal;
    BYTE    nAttr0, nFlag0;
    BOOL    bShrFmla;

    aIn >> nRow >> nCol;

    if( pExcRoot->eHauptDateiTyp == Biff2 )
    {//                     BIFF2
        BYTE nDummy;

        aIn.Ignore( 3 );

        aIn >> fCurVal;
        aIn.Ignore( 1 );
        aIn >> nDummy;
        nFormLen = nDummy;
        bShrFmla = FALSE;
        nAttr0 = 0x01;  // Always calculate
    }
    else
    {//                     BIFF5
        aIn >> nXF >> fCurVal >> nFlag0;
        aIn.Ignore( 5 );

        aIn >> nFormLen;

        bShrFmla = nFlag0 & 0x08;   // shared or not shared
    }

    nLastXF = nXF;

    Formula( nCol, nRow, nTab, nXF, nFormLen, fCurVal, nFlag0, bShrFmla );
}


void ImportExcel::Formula3()
{
    Formula4();
}


void ImportExcel::Formula4()
{
    UINT16  nRow, nCol, nXF, nFormLen;
    double  fCurVal;
    BYTE    nFlag0;

    aIn >> nRow >> nCol >> nXF >> fCurVal >> nFlag0;
    aIn.Ignore( 1 );
    aIn >> nFormLen;

    nLastXF = nXF;

    Formula( nCol, nRow, nTab, nXF, nFormLen, fCurVal, nFlag0, FALSE );
}


void ImportExcel::Formula( UINT16 nCol, UINT16 nRow, UINT16 nTab,
    UINT16 nXF, UINT16 nFormLen, double& rCurVal, BYTE nFlag, BOOL bShrFmla )
{
    ConvErr eErr = ConvOK;

    if( nRow <= MAXROW && nCol <= MAXCOL )
    {
        // jetzt steht Lesemarke auf Formel, Laenge in nFormLen
        const ScTokenArray* pErgebnis;
        BOOL                bConvert;

        pFormConv->Reset( ScAddress( nCol, nRow, nTab ) );

        if( bShrFmla )
            bConvert = !pFormConv->GetShrFmla( pErgebnis, nFormLen );
        else
            bConvert = TRUE;

        if( bConvert )
            eErr = pFormConv->Convert( pErgebnis, nFormLen );

        ScFormulaCell*      pZelle = NULL;

        if( pErgebnis )
        {
            pZelle = new ScFormulaCell( pD, ScAddress( nCol, nRow, nTab ), pErgebnis );

            pD->PutCell( nCol, nRow, nTab, pZelle, (BOOL)TRUE );

            aColRowBuff.Used( nCol, nRow );
        }
        else
        {
            CellType        eCellType;
            ScBaseCell*     pBaseCell;
            pD->GetCellType( nCol, nRow, nTab, eCellType );
            if( eCellType == CELLTYPE_FORMULA )
            {
                pD->GetCell( nCol, nRow, nTab, pBaseCell );
                pZelle = ( ScFormulaCell* ) pBaseCell;
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




ExcelToSc::~ExcelToSc() {}


ExcelToSc::ExcelToSc( RootData* pRD, XclImpStream& aStr, const UINT16& rOrgTab ) :
    ExcelConverterBase( aStr, 512 ),
    ExcRoot( pRD )
{
}


void ExcelToSc::GetDummy( const ScTokenArray*& pErgebnis )
{
    aPool.Store( _STRINGCONST( "Dummy()" ) );
    aPool >> aStack;
    pErgebnis = aPool[ aStack.Get() ];
}


// stream seeks to first byte after <nFormulaLen>
ConvErr ExcelToSc::Convert( const ScTokenArray*& pErgebnis, UINT32 nFormulaLen, const FORMULA_TYPE eFT )
{
    BYTE            nOp, nLen, nByte;
    UINT16          nUINT16, nIndexToFunc;
    INT16           nINT16;
    double          fDouble;
    String          aString;
    BOOL            bError = FALSE;
    BOOL            bArrayFormula = FALSE;
    TokenId         nMerk0;
    const BOOL      bRangeName = eFT == FT_RangeName;
    const BOOL      bSharedFormula = eFT == FT_SharedFormula;
    const BOOL      bRNorSF = bRangeName || bSharedFormula;
    const CharSet   eCharSet = *pExcRoot->pCharset;

    SingleRefData   aSRD;
    aSRD.InitFlags();
    ComplRefData    aCRD;
    aCRD.InitFlags();

    bExternName = FALSE;

    if( eStatus != ConvOK )
    {
        aIn.Ignore( nFormulaLen );
        return eStatus;
    }

    if( nFormulaLen == 0 )
    {
        aPool.Store( _STRINGCONST( "-/-" ) );
        aPool >> aStack;
        pErgebnis = aPool[ aStack.Get() ];
        return ConvOK;
    }

    ULONG nMaxPos = aIn.GetRecPos() + nFormulaLen;

    while( (aIn.GetRecPos() < nMaxPos) && !bError )
    {
        aIn >> nOp;

        switch( nOp )   //                              Buch Seite:
        {           //                                      SDK4 SDK5
            case 0x01: // Array Formula                         [325    ]
                       // Array Formula or Shared Formula       [    277]
            case 0x02: // Data Table                            [325 277]
                nUINT16 = 3;

                if( pExcRoot->eHauptDateiTyp != Biff2 )
                    nUINT16++;

                aIn.Ignore( nUINT16 );

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
                aIn >> nLen;
                aString.Erase();
                aIn.AppendRawByteString( aString, eCharSet, nLen );

                aStack << aPool.Store( aString );
                break;
            case 0x19: // Special Attribute                     [327 279]
            {
                UINT16  nData, nFakt;
                BYTE    nOpt;

                aIn >> nOpt;

                if( pExcRoot->eHauptDateiTyp == Biff2 )
                {
                    nData = aIn.ReaduInt8();
                    nFakt = 1;
                }
                else
                {
                    aIn >> nData;
                    nFakt = 2;
                }

                if( nOpt & 0x04 )
                {// nFakt -> Bytes oder Words ueberlesen    AttrChoose
                    nData++;
                    aIn.Ignore( nData * nFakt );
                }
                else if( nOpt & 0x10 )                      // AttrSum
                    DoMulArgs( ocSum, 1 );
            }
                break;
            case 0x1A: // External Reference                    [330    ]
                switch( pExcRoot->eHauptDateiTyp )
                {
                    case Biff2: aIn.Ignore( 7 ); break;
                    case Biff3:
                    case Biff4: aIn.Ignore( 10 ); break;
                    case Biff5:
                        DBG_WARNING( "-ExcelToSc::Convert(): 0x1A gibt's nicht in Biff5!" );
                    default:
                        DBG_WARNING( "-ExcelToSc::Convert(): Ein wenig vergesslich, was?" );
                }
                break;
            case 0x1B: // End External Reference                [330    ]
                switch( pExcRoot->eHauptDateiTyp )
                {
                    case Biff2: aIn.Ignore( 3 ); break;
                    case Biff3:
                    case Biff4: aIn.Ignore( 4 ); break;
                    case Biff5:
                        DBG_WARNING( "-ExcelToSc::Convert(): 0x1B gibt's nicht in Biff5!" );
                    default:
                        DBG_WARNING( "-ExcelToSc::Convert(): Ein wenig vergesslich, was?" );
                }
                break;
            case 0x1C: // Error Value                           [314 266]
            {
                aIn >> nByte;
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
                aIn >> nByte;
                if( nByte == 0 )
                    aPool << ocFalse << ocOpen << ocClose;
                else
                    aPool << ocTrue << ocOpen << ocClose;
                aPool >> aStack;
                break;
            case 0x1E: // Integer                               [315 266]
                aIn >> nUINT16;
                aStack << aPool.Store( ( double ) nUINT16 );
                break;
            case 0x1F: // Number                                [315 266]
                aIn >> fDouble;
                aStack << aPool.Store( fDouble );
                break;
            case 0x40:
            case 0x60:
            case 0x20: // Array Constant                        [317 268]
                aIn.Ignore( 7 );
                aPool << ocBad;
                aPool >> aStack;
                break;
            case 0x41:
            case 0x61:
            case 0x21: // Function, Fixed Number of Arguments   [333 282]
                if( pExcRoot->eHauptDateiTyp == Biff2 ||
                    pExcRoot->eHauptDateiTyp == Biff3 )
                    nIndexToFunc = aIn.ReaduInt8();
                else
                    aIn >> nIndexToFunc;

                DoDefArgs( nIndexToFunc );
                break;
            case 0x42:
            case 0x62:
            case 0x22: // Function, Variable Number of Arg.     [333 283]
            {
                BYTE nAnz;
                aIn >> nAnz;
                nAnz &= 0x7F;

                if( pExcRoot->eHauptDateiTyp == Biff2 ||
                    pExcRoot->eHauptDateiTyp == Biff3 )
                    nIndexToFunc = aIn.ReaduInt8();
                else
                    aIn >> nIndexToFunc;

                DoMulArgs( IndexToToken( nIndexToFunc ), nAnz );
            }
                break;
            case 0x43:
            case 0x63:
            case 0x23: // Name                                  [318 269]
                aIn >> nUINT16;
                switch( pExcRoot->eHauptDateiTyp )
                {
                    case Biff2: aIn.Ignore( 5 ); break;
                    case Biff3:
                    case Biff4: aIn.Ignore( 8 ); break;
                    case Biff5: aIn.Ignore( 12 ); break;
                    default:
                        DBG_ERROR(
                        "-ExcelToSc::Convert(): Ein wenig vergesslich, was?" );
                }
                aStack << aPool.Store( ( *pExcRoot->pRNameBuff )[ nUINT16 ] );
                break;
            case 0x44:
            case 0x64:
            case 0x24: // Cell Reference                        [319 270]
            case 0x4A:
            case 0x6A:
            case 0x2A: // Deleted Cell Reference                [323 273]
                aIn >> nUINT16 >> nByte;
                aSRD.nCol = nByte;
                aSRD.nRow = nUINT16 & 0x3FFF;
                aSRD.nRelTab = 0;
                aSRD.SetTabRel( TRUE );
                aSRD.SetFlag3D( bRangeName );

                ExcRelToScRel( nUINT16, nByte, aSRD, bRangeName );

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
                break;
            case 0x45:
            case 0x65:
            case 0x25: // Area Reference                        [320 270]
            case 0x4B:
            case 0x6B:
            case 0x2B: // Deleted Area Refernce                 [323 273]
            {
                UINT16          nRowFirst, nRowLast;
                UINT8           nColFirst, nColLast;
                SingleRefData&  rSRef1 = aCRD.Ref1;
                SingleRefData&  rSRef2 = aCRD.Ref2;

                aIn >> nRowFirst >> nRowLast >> nColFirst >> nColLast;

                rSRef1.nRelTab = rSRef2.nRelTab = 0;
                rSRef1.SetTabRel( TRUE );
                rSRef2.SetTabRel( TRUE );
                rSRef1.SetFlag3D( bRangeName );
                rSRef2.SetFlag3D( bRangeName );

                ExcRelToScRel( nRowFirst, nColFirst, aCRD.Ref1, bRangeName );
                ExcRelToScRel( nRowLast, nColLast, aCRD.Ref2, bRangeName );

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
                aIn.Ignore( 6 );        // mehr steht da nicht!
                break;
            case 0x47:
            case 0x67:
            case 0x27: // Erroneous Constant Reference Subexpr. [322 272]
                if( pExcRoot->eHauptDateiTyp == Biff2 )
                    aIn.Ignore( 4 );    // mehr steht da nicht!
                else
                    aIn.Ignore( 6 );    // mehr steht da nicht!
                aPool << ocBad;
                aPool >> aStack;
                break;
            case 0x48:
            case 0x68:
            case 0x28: // Incomplete Constant Reference Subexpr.[331 281]
                if( pExcRoot->eHauptDateiTyp == Biff2 )
                    aIn.Ignore( 4 );    // mehr steht da nicht!
                else
                    aIn.Ignore( 6 );    // mehr steht da nicht!
                aPool << ocBad;
                aPool >> aStack;
                break;
            case 0x49:
            case 0x69:
            case 0x29: // Variable Reference Subexpression      [331 281]
                if( pExcRoot->eHauptDateiTyp == Biff2 )
                    aIn.Ignore( 1 );    // mehr steht da nicht!
                else
                    aIn.Ignore( 2 );    // mehr steht da nicht!
                break;
            case 0x4C:
            case 0x6C:
            case 0x2C: // Cell Reference Within a Name          [323    ]
                       // Cell Reference Within a Shared Formula[    273]
            {
                aIn >> nUINT16 >> nByte;    // >> Attribute, Row >> Col

                aSRD.nRelTab = 0;
                aSRD.SetTabRel( TRUE );
                aSRD.SetFlag3D( bRangeName );

                ExcRelToScRel( nUINT16, nByte, aSRD, bRNorSF );

                aStack << aPool.Store( aSRD );
            }
                break;
            case 0x4D:
            case 0x6D:
            case 0x2D: // Area Reference Within a Name          [324    ]
            {      // Area Reference Within a Shared Formula[    274]
                UINT16                  nRowFirst, nRowLast;
                UINT8                   nColFirst, nColLast;

                aCRD.Ref1.nRelTab = aCRD.Ref2.nRelTab = 0;
                aCRD.Ref1.SetTabRel( TRUE );
                aCRD.Ref2.SetTabRel( TRUE );
                aCRD.Ref1.SetFlag3D( bRangeName );
                aCRD.Ref2.SetFlag3D( bRangeName );

                aIn >> nRowFirst >> nRowLast >> nColFirst >> nColLast;

                ExcRelToScRel( nRowFirst, nColFirst, aCRD.Ref1, bRNorSF );
                ExcRelToScRel( nRowLast, nColLast, aCRD.Ref2, bRNorSF );

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
                if( pExcRoot->eHauptDateiTyp == Biff2 )
                    aIn.Ignore( 1 );    // mehr steht da nicht!
                else
                    aIn.Ignore( 2 );    // mehr steht da nicht!
                aPool << ocBad;
                aPool >> aStack;
                break;
            case 0x4F:
            case 0x6F:
            case 0x2F: // Incomplete Reference Subexpression... [332 282]
                if( pExcRoot->eHauptDateiTyp == Biff2 )
                    aIn.Ignore( 1 );    // mehr steht da nicht!
                else
                    aIn.Ignore( 2 );    // mehr steht da nicht!
                aPool << ocBad;
                aPool >> aStack;
                break;
            case 0x58:
            case 0x78:
            case 0x38: // Command-Equivalent Function           [333    ]
                aString.AssignAscii( "COMM_EQU_FUNC" );
                aIn >> nByte;
                aString += String::CreateFromInt32( nByte );
                aIn >> nByte;
                aStack << aPool.Store( aString );
                DoMulArgs( ocPush, nByte + 1 );
                break;
            case 0x59:
            case 0x79:
            case 0x39: // Name or External Name                 [    275]
                aIn >> nINT16;
                aIn.Ignore( 8 );
                aIn >> nUINT16;
                if( nINT16 >= 0 )
                {
                    const ExtName*  pExtName;
                    pExtName = pExcRoot->pExtNameBuff->GetName( nUINT16 );
                    if( pExtName && pExtName->IsDDE() &&
                        pExcRoot->pExtSheetBuff->IsLink( ( UINT16 ) nINT16 ) )
                    {
                        String          aAppl, aExtDoc;
                        TokenId         nPar1, nPar2;

                        pExcRoot->pExtSheetBuff->GetLink( ( UINT16 ) nINT16 , aAppl, aExtDoc );
                        nPar1 = aPool.Store( aAppl );
                        nPar2 = aPool.Store( aExtDoc );
                        nMerk0 = aPool.Store( pExtName->aName );
                        aPool   << ocDde << ocOpen << nPar1 << ocSep << nPar2 << ocSep
                                << nMerk0 << ocClose;

                        pExcRoot->pDoc->CreateDdeLink( aAppl, aExtDoc, pExtName->aName );
                    }
                    else
                        aPool << ocBad;

                    aPool >> aStack;
                }
                else
                    aStack << aPool.Store( ( *pExcRoot->pRNameBuff )[ nUINT16 ] );
                aIn.Ignore( 12 );
                break;
            case 0x5A:
            case 0x7A:
            case 0x3A: // 3-D Cell Reference                    [    275]
            case 0x5C:
            case 0x7C:
            case 0x3C: // Deleted 3-D Cell Reference            [    277]
            {
                UINT16          nTabFirst, nTabLast, nRow;
                INT16           nExtSheet;
                BYTE            nCol;

                aIn >> nExtSheet;
                aIn.Ignore( 8 );
                aIn >> nTabFirst >> nTabLast >> nRow >> nCol;

                if( nExtSheet >= 0 )
                {   // von extern
                    if( pExcRoot->pExtSheetBuff->GetScTabIndex( nExtSheet, nTabLast ) )
                    {
                        nTabFirst = nTabLast;
                        nExtSheet = 0;      // gefunden
                    }
                    else
                    {
                        aPool << ocBad;
                        aPool >> aStack;
                        nExtSheet = 1;      // verhindert Erzeugung einer SingleRef
                    }
                }

                if( nExtSheet <= 0 )
                {   // in aktuellem Workbook
                    BOOL        b3D = ( nTabFirst != aEingPos.Tab() ) || bRangeName;
                    aSRD.nTab = nTabFirst;
                    aSRD.SetFlag3D( b3D );
                    aSRD.SetTabRel( FALSE );

                    ExcRelToScRel( nRow, nCol, aSRD, bRangeName );

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
                        aCRD.Ref1 = aSRD;
                        aCRD.Ref2.nCol = aSRD.nCol;
                        aCRD.Ref2.nRow = aSRD.nRow;
                        aCRD.Ref2.nTab = nTabLast;
                        b3D = ( nTabLast != aEingPos.Tab() );
                        aCRD.Ref2.SetFlag3D( b3D );
                        aCRD.Ref2.SetTabRel( FALSE );
                        aCRD.Ref2.SetTabDeleted( nTabLast > MAXTAB );
                        aStack << aPool.Store( aCRD );
                    }
                    else
                        aStack << aPool.Store( aSRD );
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
                UINT16      nTabFirst, nTabLast, nRowFirst, nRowLast;
                INT16       nExtSheet;
                BYTE        nColFirst, nColLast;

                aIn >> nExtSheet;
                aIn.Ignore( 8 );
                aIn >> nTabFirst >> nTabLast >> nRowFirst >> nRowLast
                    >> nColFirst >> nColLast;

                if( nExtSheet >= 0 )
                    // von extern
                {
                    if( pExcRoot->pExtSheetBuff->GetScTabIndex( nExtSheet, nTabLast ) )
                    {
                        nTabFirst = nTabLast;
                        nExtSheet = 0;      // gefunden
                    }
                    else
                    {
                        aPool << ocBad;
                        aPool >> aStack;
                        nExtSheet = 1;      // verhindert Erzeugung einer CompleteRef
                    }
                }

                if( nExtSheet <= 0 )
                {// in aktuellem Workbook
                    // erster Teil des Bereichs
                    SingleRefData&  rR1 = aCRD.Ref1;
                    SingleRefData&  rR2 = aCRD.Ref2;

                    rR1.nTab = nTabFirst;
                    rR2.nTab = nTabLast;
                    rR1.SetFlag3D( ( nTabFirst != aEingPos.Tab() ) || bRangeName );
                    rR1.SetTabRel( FALSE );
                    rR2.SetFlag3D( ( nTabLast != aEingPos.Tab() ) || bRangeName );
                    rR2.SetTabRel( FALSE );

                    ExcRelToScRel( nRowFirst, nColFirst, aCRD.Ref1, bRangeName );
                    ExcRelToScRel( nRowLast, nColLast, aCRD.Ref2, bRangeName );

                    if( IsComplColRange( nColFirst, nColLast ) )
                        SetComplCol( aCRD );
                    else if( IsComplRowRange( nRowFirst, nRowLast ) )
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
                }//ENDE in aktuellem Workbook
            }
                break;
            default: bError = TRUE;
        }
        bError |= !aIn.IsValid();
    }

    ConvErr eRet;

    if( bError )
    {
        aPool << ocBad;
        aPool >> aStack;
        pErgebnis = aPool[ aStack.Get() ];
        eRet = ConvErrNi;
    }
    else if( aIn.GetRecPos() != nMaxPos )
    {
        aPool << ocBad;
        aPool >> aStack;
        pErgebnis = aPool[ aStack.Get() ];
        eRet = ConvErrCount;
    }
    else if( bExternName )
    {
        pErgebnis = aPool[ aStack.Get() ];
        eRet = ConvErrExternal;
    }
    else if( bArrayFormula )
    {
        pErgebnis = NULL;
        eRet = ConvOK;
    }
    else
    {
        pErgebnis = aPool[ aStack.Get() ];
        eRet = ConvOK;
    }

    aIn.Seek( nMaxPos );
    return eRet;
}


// stream seeks to first byte after <nFormulaLen>
ConvErr ExcelToSc::Convert( _ScRangeListTabs& rRangeList, UINT32 nFormulaLen, const FORMULA_TYPE eFT )
{
    BYTE            nOp, nLen;
    UINT16          nIgnore;
    UINT16          nUINT16;
    UINT8           nByte;
    BOOL            bError = FALSE;
    BOOL            bArrayFormula = FALSE;
    const BOOL      bRangeName = eFT == FT_RangeName;
    const BOOL      bSharedFormula = eFT == FT_SharedFormula;
    const BOOL      bRNorSF = bRangeName || bSharedFormula;

    SingleRefData   aSRD;
    aSRD.InitFlags();
    ComplRefData    aCRD;
    aCRD.InitFlags();
    aCRD.Ref1.nTab = aCRD.Ref2.nTab = aEingPos.Tab();

    bExternName = FALSE;

    if( eStatus != ConvOK )
    {
        aIn.Ignore( nFormulaLen );
        return eStatus;
    }

    if( nFormulaLen == 0 )
        return ConvOK;

    ULONG nMaxPos = aIn.GetRecPos() + nFormulaLen;

    while( (aIn.GetRecPos() < nMaxPos) && !bError )
    {
        aIn >> nOp;
        nIgnore = 0;

        switch( nOp )   //                              Buch Seite:
        {           //                                      SDK4 SDK5
            case 0x01: // Array Formula                         [325    ]
                       // Array Formula or Shared Formula       [    277]
                if( pExcRoot->eHauptDateiTyp == Biff2 )
                    nIgnore = 3;
                else
                    nIgnore = 4;
                bArrayFormula = TRUE;
                break;
            case 0x02: // Data Table                            [325 277]
            {
                if( pExcRoot->eHauptDateiTyp == Biff2 )
                    nIgnore = 3;
                else
                    nIgnore = 4;
            }
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
                aIn >> nLen;
                nIgnore = nLen;
                break;
            case 0x19: // Special Attribute                     [327 279]
            {
                UINT16 nData, nFakt;
                BYTE nOpt;

                aIn >> nOpt;

                if( pExcRoot->eHauptDateiTyp == Biff2 )
                {
                    nData = aIn.ReaduInt8();
                    nFakt = 1;
                }
                else
                {
                    aIn >> nData;
                    nFakt = 2;
                }

                if( nOpt & 0x04 )
                {// nFakt -> Bytes oder Words ueberlesen    AttrChoose
                    nData++;
                    aIn.Ignore( nData * nFakt );
                }
            }
                break;
            case 0x1A: // External Reference                    [330    ]
                switch( pExcRoot->eHauptDateiTyp )
                {
                    case Biff2: nIgnore = 7;        break;
                    case Biff3:
                    case Biff4: nIgnore = 10;       break;
                    case Biff5: DBG_WARNING( "-ExcelToSc::Convert(): 0x1A gibt's nicht in Biff5!" );
                    default:    DBG_WARNING( "-ExcelToSc::Convert(): Ein wenig vergesslich, was?" );
                }
                break;
            case 0x1B: // End External Reference                [330    ]
                switch( pExcRoot->eHauptDateiTyp )
                {
                    case Biff2: nIgnore = 3;        break;
                    case Biff3:
                    case Biff4: nIgnore = 4;        break;
                    case Biff5: DBG_WARNING( "-ExcelToSc::Convert(): 0x1B gibt's nicht in Biff5!" );
                    default:    DBG_WARNING( "-ExcelToSc::Convert(): Ein wenig vergesslich, was?" );
                }
                break;
            case 0x1C: // Error Value                           [314 266]
            case 0x1D: // Boolean                               [315 266]
                nIgnore = 1;
                break;
            case 0x1E: // Integer                               [315 266]
                nIgnore = 2;
                break;
            case 0x1F: // Number                                [315 266]
                nIgnore = sizeof( double );
                break;
            case 0x40:
            case 0x60:
            case 0x20: // Array Constant                        [317 268]
                nIgnore = 7;
                break;
            case 0x41:
            case 0x61:
            case 0x21: // Function, Fixed Number of Arguments   [333 282]
                if( pExcRoot->eHauptDateiTyp == Biff2 || pExcRoot->eHauptDateiTyp == Biff3 )
                    nIgnore = 1;
                else
                    nIgnore = 2;
                break;
            case 0x42:
            case 0x62:
            case 0x22: // Function, Variable Number of Arg.     [333 283]
                if( pExcRoot->eHauptDateiTyp == Biff2 || pExcRoot->eHauptDateiTyp == Biff3 )
                    nIgnore = 2;
                else
                    nIgnore = 3;
                break;
            case 0x43:
            case 0x63:
            case 0x23: // Name                                  [318 269]
                switch( pExcRoot->eHauptDateiTyp )
                {
                    case Biff2: nIgnore = 7;        break;
                    case Biff3:
                    case Biff4: nIgnore = 10;       break;
                    case Biff5: nIgnore = 14;       break;
                    default:    DBG_ERROR( "-ExcelToSc::Convert(): Ein wenig vergesslich, was?" );
                }
                break;
            case 0x44:
            case 0x64:
            case 0x24: // Cell Reference                        [319 270]
                aIn >> nUINT16 >> nByte;
                aSRD.nCol = nByte;
                aSRD.nRow = nUINT16 & 0x3FFF;
                aSRD.nRelTab = 0;
                aSRD.SetTabRel( TRUE );
                aSRD.SetFlag3D( bRangeName );

                ExcRelToScRel( nUINT16, nByte, aSRD, bRangeName );

                rRangeList.Append( aSRD );
                break;
            case 0x45:
            case 0x65:
            case 0x25: // Area Reference                        [320 270]
            {
                UINT16          nRowFirst, nRowLast;
                UINT8           nColFirst, nColLast;
                SingleRefData   &rSRef1 = aCRD.Ref1;
                SingleRefData   &rSRef2 = aCRD.Ref2;

                aIn >> nRowFirst >> nRowLast >> nColFirst >> nColLast;

                rSRef1.nRelTab = rSRef2.nRelTab = 0;
                rSRef1.SetTabRel( TRUE );
                rSRef2.SetTabRel( TRUE );
                rSRef1.SetFlag3D( bRangeName );
                rSRef2.SetFlag3D( bRangeName );

                ExcRelToScRel( nRowFirst, nColFirst, aCRD.Ref1, bRangeName );
                ExcRelToScRel( nRowLast, nColLast, aCRD.Ref2, bRangeName );

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
                nIgnore = 6;
                break;
            case 0x47:
            case 0x67:
            case 0x27: // Erroneous Constant Reference Subexpr. [322 272]
            case 0x48:
            case 0x68:
            case 0x28: // Incomplete Constant Reference Subexpr.[331 281]
                if( pExcRoot->eHauptDateiTyp == Biff2 )
                    nIgnore = 4;
                else
                    nIgnore = 6;
                break;
            case 0x49:
            case 0x69:
            case 0x29: // Variable Reference Subexpression      [331 281]
                if( pExcRoot->eHauptDateiTyp == Biff2 )
                    nIgnore = 1;
                else
                    nIgnore = 2;
                break;
            case 0x4A:
            case 0x6A:
            case 0x2A: // Deleted Cell Reference                [323 273]
                nIgnore = 3;
                break;
            case 0x4B:
            case 0x6B:
            case 0x2B: // Deleted Area Refernce                 [323 273]
                nIgnore = 6;
                break;
            case 0x4C:
            case 0x6C:
            case 0x2C: // Cell Reference Within a Name          [323    ]
                       // Cell Reference Within a Shared Formula[    273]
            {
                aIn >> nUINT16 >> nByte;    // >> Attribute, Row >> Col

                aSRD.nRelTab = 0;
                aSRD.SetTabRel( TRUE );
                aSRD.SetFlag3D( bRangeName );

                ExcRelToScRel( nUINT16, nByte, aSRD, bRNorSF );

                rRangeList.Append( aSRD );
            }
                break;
            case 0x4D:
            case 0x6D:
            case 0x2D: // Area Reference Within a Name          [324    ]
            {      // Area Reference Within a Shared Formula[    274]
                UINT16                  nRowFirst, nRowLast;
                UINT8                   nColFirst, nColLast;

                aCRD.Ref1.nRelTab = aCRD.Ref2.nRelTab = 0;
                aCRD.Ref1.SetTabRel( TRUE );
                aCRD.Ref2.SetTabRel( TRUE );
                aCRD.Ref1.SetFlag3D( bRangeName );
                aCRD.Ref2.SetFlag3D( bRangeName );

                aIn >> nRowFirst >> nRowLast >> nColFirst >> nColLast;

                ExcRelToScRel( nRowFirst, nColFirst, aCRD.Ref1, bRNorSF );
                ExcRelToScRel( nRowLast, nColLast, aCRD.Ref2, bRNorSF );

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
                if( pExcRoot->eHauptDateiTyp == Biff2 )
                    nIgnore = 1;
                else
                    nIgnore = 2;
                break;
            case 0x58:
            case 0x78:
            case 0x38: // Command-Equivalent Function           [333    ]
                nIgnore = 2;
                break;
            case 0x59:
            case 0x79:
            case 0x39: // Name or External Name                 [    275]
                nIgnore = 24;
                break;
            case 0x5A:
            case 0x7A:
            case 0x3A: // 3-D Cell Reference                    [    275]
            {
                UINT16          nTabFirst, nTabLast, nRow;
                INT16           nExtSheet;
                BYTE            nCol;

                aIn >> nExtSheet;
                aIn.Ignore( 8 );
                aIn >> nTabFirst >> nTabLast >> nRow >> nCol;

                if( nExtSheet >= 0 )
                    // von extern
                {
                    if( pExcRoot->pExtSheetBuff->GetScTabIndex( nExtSheet, nTabLast ) )
                    {
                        nTabFirst = nTabLast;
                        nExtSheet = 0;      // gefunden
                    }
                    else
                    {
                        aPool << ocBad;
                        aPool >> aStack;
                        nExtSheet = 1;      // verhindert Erzeugung einer SingleRef
                    }
                }

                if( nExtSheet <= 0 )
                {// in aktuellem Workbook
                    BOOL b3D = ( nTabFirst != aEingPos.Tab() ) || bRangeName;
                    aSRD.nTab = nTabFirst;
                    aSRD.SetFlag3D( b3D );
                    aSRD.SetTabRel( FALSE );

                    ExcRelToScRel( nRow, nCol, aSRD, bRangeName );

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

                break;
            case 0x5B:
            case 0x7B:
            case 0x3B: // 3-D Area Reference                    [    276]
            {
                UINT16      nTabFirst, nTabLast, nRowFirst, nRowLast;
                INT16       nExtSheet;
                BYTE        nColFirst, nColLast;

                aIn >> nExtSheet;
                aIn.Ignore( 8 );
                aIn >> nTabFirst >> nTabLast >> nRowFirst >> nRowLast
                    >> nColFirst >> nColLast;

                if( nExtSheet >= 0 )
                    // von extern
                {
                    if( pExcRoot->pExtSheetBuff->GetScTabIndex( nExtSheet, nTabLast ) )
                    {
                        nTabFirst = nTabLast;
                        nExtSheet = 0;      // gefunden
                    }
                    else
                    {
                        aPool << ocBad;
                        aPool >> aStack;
                        nExtSheet = 1;      // verhindert Erzeugung einer CompleteRef
                    }
                }

                if( nExtSheet <= 0 )
                {// in aktuellem Workbook
                    // erster Teil des Bereichs
                    SingleRefData   &rR1 = aCRD.Ref1;
                    SingleRefData   &rR2 = aCRD.Ref2;

                    rR1.nTab = nTabFirst;
                    rR2.nTab = nTabLast;
                    rR1.SetFlag3D( ( nTabFirst != aEingPos.Tab() ) || bRangeName );
                    rR1.SetTabRel( FALSE );
                    rR2.SetFlag3D( ( nTabLast != aEingPos.Tab() ) || bRangeName );
                    rR2.SetTabRel( FALSE );

                    ExcRelToScRel( nRowFirst, nColFirst, aCRD.Ref1, bRangeName );
                    ExcRelToScRel( nRowLast, nColLast, aCRD.Ref2, bRangeName );

                    if( IsComplColRange( nColFirst, nColLast ) )
                        SetComplCol( aCRD );
                    else if( IsComplRowRange( nRowFirst, nRowLast ) )
                        SetComplRow( aCRD );

                    rRangeList.Append( aCRD );
                }//ENDE in aktuellem Workbook
            }
                break;
            case 0x5C:
            case 0x7C:
            case 0x3C: // Deleted 3-D Cell Reference            [    277]
                nIgnore = 17;
                break;
            case 0x5D:
            case 0x7D:
            case 0x3D: // Deleted 3-D Area Reference            [    277]
                nIgnore = 20;
                break;
            default: bError = TRUE;
        }
        bError |= !aIn.IsValid();

        aIn.Ignore( nIgnore );
    }

    ConvErr eRet;

    if( bError )
        eRet = ConvErrNi;
    else if( aIn.GetRecPos() != nMaxPos )
        eRet = ConvErrCount;
    else if( bExternName )
        eRet = ConvErrExternal;
    else if( bArrayFormula )
        eRet = ConvOK;
    else
        eRet = ConvOK;

    aIn.Seek( nMaxPos );
    return eRet;
}


void ExcelToSc::DoMulArgs( DefTokenId eId, BYTE nAnz )
{
    TokenId                 eParam[ 256 ];
    INT32                   nLauf;

    if( eId == ocLog10 && nAnz > 1 )
        eId = ocLog;
    else if( eId == ocCeil || eId == ocFloor )
    {
        aStack << aPool.Store( 1.0 );   // default, da in Excel nicht vorhanden
        nAnz++;
    }

    for( nLauf = 0 ; nLauf < nAnz ; nLauf++ )
        aStack >> eParam[ nLauf ];

    if( nAnz > 0 && eId == ocExternal )
    {
        TokenId             n = eParam[ nAnz - 1 ];
//##### GRUETZE FUER BASIC-FUNCS RICHTEN!
        if( aPool.IsExternal( n ) )
        {
            aPool << n;
            nAnz--;
        }
        else
            aPool << eId;
    }
    else
        aPool << eId;

    aPool << ocOpen;

    if( nAnz > 0 )
    {
        INT16 nNull = -1;   // gibt einen auszulassenden Parameter an
            // ACHTUNG: 0 ist der letzte Parameter, nAnz-1 der erste

        INT16 nLast = nAnz - 1;

        // Funktionen, bei denen Parameter wegfallen muessen
        if( eId == ocPercentrank && nAnz == 3 )
            nNull = 0;      // letzten Parameter bei Bedarf weglassen

        else if( eId == ocIndirect && nAnz == 2 )
            nNull = 0;

        else if( eId == ocAdress && nAnz > 3 )
            nNull = nAnz - 4;

        // Joost-Spezialfaelle
        else if( eId == ocRound || eId == ocRoundUp || eId == ocRoundDown )
        {
            if( aPool.IsSingleOp( eParam[ 0 ], ocMissing ) )
                nNull = 0;      // letzten (ocMissing) weglassen
        }
        else if( eId == ocIf )
        {
            UINT16          nNullParam = 0;
            for( nLauf = 0 ; nLauf < nAnz ; nLauf++ )
            {
                if( aPool.IsSingleOp( eParam[ nLauf ], ocMissing ) )
                {
                    if( !nNullParam )
                        nNullParam = (UINT16) aPool.Store( ( double ) 0.0 );
                    eParam[ nLauf ] = nNullParam;
                }
            }
        }

        // [Parameter{;Parameter}]
        aPool << eParam[ nLast ];
        for( nLauf = nLast - 1 ; nLauf >= 0 ; nLauf-- )
        {
            if( nLauf != nNull )
                aPool << ocSep << eParam[ nLauf ];
        }
    }
    aPool << ocClose;

    aPool >> aStack;
}


DefTokenId ExcelToSc::IndexToToken( UINT16 nIndex )
{
    static const DefTokenId pMap[ nLastInd + 1 ] =
    {
        ocCount,            // XlfCount         0
        ocIf,               // XlfIf
        ocIsNV,             // XlfIsna
        ocIsError,          // XlfIserror
        ocSum,              // XlfSum
        ocAverage,          // XlfAverage
        ocMin,              // XlfMin
        ocMax,              // XlfMax
        ocRow,
        ocColumn,
        ocNoValue,          // XlfNa
        ocNBW,              // XlfNpv
        ocStDev,            // XlfStdev
        ocCurrency,         // XlfDollar
        ocFixed,            // XlfFixed
        ocSin,              // XlfSin
        ocCos,              // XlfCos
        ocTan,              // XlfTan
        ocArcTan,           // XlfAtan
        ocPi,               // XlfPi
        ocSqrt,             // XlfSqrt
        ocExp,              // XlfExp
        ocLn,               // XlfLn
        ocLog10,            // XlfLog10
        ocAbs,              // XlfAbs
        ocInt,              // XlfInt
        ocPlusMinus,        // XlfSign
        ocRound,            // XlfRound
        ocLookup,           // XlfLookup
        ocIndex,            // XlfIndex
        ocRept,             // XlfRept
        ocMid,              // XlfMid
        ocLen,              // XlfLen
        ocValue,            // XlfValue
        ocTrue,             // XlfTrue
        ocFalse,            // XlfFalse
        ocAnd,              //          *
        ocOr,               //          *
        ocNot,              //          *
        ocMod,              // XlfMod
        ocDBCount,          // XlfDcount
        ocDBSum,            // XlfDsum
        ocDBAverage,        // XlfDaverage
        ocDBMin,            // XlfDmin
        ocDBMax,            // XlfDmax
        ocDBStdDev,         // XlfDstdev
        ocVar,              // XlfVar
        ocDBVar,            // XlfDvar
        ocText,             // XlfText
        ocRGP,              // XlfLinest
        ocTrend,            // XlfTrend
        ocRKP,              // XlfLogest
        ocGrowth,           // XlfGrowth
        ocNoName,           // XlfGoto
        ocNoName,           // XlfHalt
        ocNoName,           // ?
        ocBW,               // XlfPv
        ocZW,               // XlfFv
        ocZZR,              // XlfNper
        ocRMZ,              // XlfPmt
        ocZins,             // XlfRate
        ocMIRR,             // XlfMirr
        ocIKV,              // XlfIrr
        ocRandom,           // XlfRand
        ocMatch,            // XlfMatch
        ocGetDate,          // XlfDate
        ocGetTime,          // XlfTime
        ocGetDay,           // XlfDay
        ocGetMonth,         // XlfMonth
        ocGetYear,          // XlfYear
        ocGetDayOfWeek,     // XlfWeekday
        ocGetHour,          // XlfHour
        ocGetMin,           // XlfMinute
        ocGetSec,           // XlfSecond
        ocGetActTime,       // XlfNow
        ocAreas,            // XlfAreas
        ocRows,             // XlfRows
        ocColumns,          // XlfColumns
        ocOffset,           // XlfOffset
        ocNoName,           // XlfAbsref
        ocNoName,           // XlfRelref
        ocNoName,           // XlfArgument
        ocSearch,           // XlfSearch
        ocMatTrans,         // XlfTranspose
        ocNoName,           // XlfError
        ocNoName,           // XlfStep
        ocType,             // XlfType
        ocNoName,           // XlfEcho
        ocNoName,           // XlfSetName
        ocNoName,           // XlfCaller
        ocNoName,           // XlfDeref
        ocNoName,           // XlfWindows
        ocNoName,           // XlfSeries
        ocNoName,           // XlfDocuments
        ocNoName,           // XlfActiveCell
        ocNoName,           // XlfSelection
        ocNoName,           // XlfResult
        ocArcTan2,          // XlfAtan2
        ocArcSin,           // XlfAsin
        ocArcCos,           // XlfAcos
        ocChose,            // XlfChose
        ocHLookup,          // XlfHlookup
        ocVLookup,          // XlfVlookup
        ocNoName,           // XlfLinks
        ocNoName,           // XlfInput
        ocIsRef,            // XlfIsref
        ocNoName,           // XlfGetFormula
        ocNoName,           // XlfGetName
        ocNoName,           // XlfSetValue
        ocLog10,            // XlfLog
        ocNoName,           // XlfExec
        ocChar,             // XlfChar
        ocLower,            // XlfLower
        ocUpper,            // XlfUpper
        ocPropper,          // XlfProper
        ocLeft,             // XlfLeft
        ocRight,            // XlfRight
        ocExact,            // XlfExact
        ocTrim,             // XlfTrim
        ocReplace,          // XlfReplace
        ocSubstitute,       // XlfSubstitute
        ocCode,             // XlfCode
        ocNoName,           // XlfNames
        ocNoName,           // XlfDirectory
        ocFind,             // XlfFind
        ocCell,             // XlfCell
        ocIsErr,            // XlfIserr
        ocIsString,         // XlfIstext
        ocIsValue,          // XlfIsnumber
        ocIsEmpty,          // XlfIsblank
        ocT,                // XlfT
        ocN,                // XlfN
        ocNoName,           // XlfFopen
        ocNoName,           // XlfFclose
        ocNoName,           // XlfFsize
        ocNoName,           // XlfFreadln
        ocNoName,           // XlfFread
        ocNoName,           // XlfFwriteln
        ocNoName,           // XlfFwrite
        ocNoName,           // XlfFpos
        ocGetDateValue,     // XlfDatevalue
        ocGetTimeValue,     // XlfTimevalue
        ocLIA,              // XlfSln
        ocDIA,              // XlfSyd
        ocGDA,              // XlfDdb
        ocNoName,           // XlfGetDef
        ocNoName,           // XlfReftext
        ocNoName,           // XlfTextref
        ocIndirect,         // XlfIndirect
        ocNoName,           // XlfRegister
        ocNoName,           // XlfCall
        ocNoName,           // XlfAddBar
        ocNoName,           // XlfAddMenu
        ocNoName,           // XlfAddCommand
        ocNoName,           // XlfEnableCommand
        ocNoName,           // XlfCheckCommand
        ocNoName,           // XlfRenameCommand
        ocNoName,           // XlfShowBar
        ocNoName,           // XlfDeleteMenu
        ocNoName,           // XlfDeleteCommand
        ocNoName,           // XlfGetChartItem
        ocNoName,           // XlfDialogBox
        ocClean,            // XlfClean
        ocMatDet,           // XlfMdeterm
        ocMatInv,           // XlfMinverse
        ocMatMult,          // XlfMmult
        ocNoName,           // XlfFiles
        ocZinsZ,            // XlfIpmt
        ocKapz,             // XlfPpmt
        ocCount2,           // XlfCounta
        ocNoName,           // XlfCancelKey
        ocNoName,           // ?
        ocNoName,           // XlfAppMin
        ocNoName,           // XlfAppMax
        ocNoName,           // XlfBringToFront
        ocNoName,           // XlfInitiate
        ocNoName,           // XlfRequest
        ocNoName,           // XlfPoke
        ocNoName,           // XlfExecute
        ocNoName,           // XlfTerminate
        ocNoName,           // XlfRestart
        ocNoName,           // XlfHelp
        ocNoName,           // XlfGetBar
        ocProduct,          // XlfProduct
        ocFact,             // XlfFact
        ocNoName,           // XlfGetCell
        ocNoName,           // XlfGetWorkspace
        ocNoName,           // XlfGetWindow
        ocNoName,           // XlfGetDocument
        ocDBProduct,        // XlfDproduct
        ocIsNonString,      // XlfIsnontext
        ocNoName,           // XlfGetNote,
        ocNoName,           // XlfNote
        ocStDevP,           // XlfStdevp
        ocVarP,             // XlfVarp
        ocDBStdDevP,        // XlfDstdevp
        ocDBVarP,           // XlfDvarp
        ocTrunc,            // XlfTrunc
        ocIsLogical,        // XlfIslogical
        ocDBCount2,         // XlfDcounta
        ocNoName,           // XlfDeleteBar
        ocNoName,           // XlfUnregister
        ocNoName,           // ?
        ocNoName,           // ?
        ocNoName,           // XlfUsdollar
        ocNoName,           // XlfFindb
        ocNoName,           // XlfSearchb
        ocNoName,           // XlfReplaceb
        ocNoName,           // XlfLeftb
        ocNoName,           // XlfRightb
        ocNoName,           // XlfMidb
        ocNoName,           // XlfLenb
        ocRoundUp,          // XlfRoundup
        ocRoundDown,        // XlfRounddown
        ocNoName,           // XlfAsc
        ocNoName,           // XlfDbcs
        ocRank,             // XlfRank
        ocNoName,           // ?
        ocNoName,           // ?
        ocAdress,           // XlfAddress
        ocGetDiffDate360,   // XlfDays360
        ocGetActDate,       // XlfToday
        ocVBD,              // XlfVdb
        ocNoName,           // XlfEditColor
        ocNoName,           // XlfShowLevels
        ocNoName,           // XlfFormatMain
        ocNoName,           // ?
        ocMedian,           // XlfMedian
        ocSumProduct,       // XlfSumproduct
        ocSinHyp,           // XlfSinh
        ocCosHyp,           // XlfCosh
        ocTanHyp,           // XlfTanh
        ocArcSinHyp,        // XlfAsinh
        ocArcCosHyp,        // XlfAcosh
        ocArcTanHyp,        // XlfAtanh
        ocDBGet,            // XlfDget
        ocNoName,           // XlfCreateObject
        ocNoName,           // XlfVolatile
        ocNoName,           // XlfLastError
        ocNoName,           // XlfCustomUndo
        ocNoName,           // XlfCustomRepeat
        ocNoName,           // XlfFormulaConvert
        ocNoName,           // XlfGetLinkInfo
        ocNoName,           // XlfTextBox
        ocNoName,           // XlfInfo
        ocNoName,           // XlfGroup
        ocNoName,           // XlfGetObject
        ocGDA2,             // XlfDb
        ocNoName,           // XlfPause
        ocNoName,           // ?
        ocNoName,           // ?
        ocNoName,           // XlfResume
        ocFrequency,        // XlfFrequency
        ocNoName,           // XlfAddToolbar
        ocNoName,           // XlfDeleteToolbar
        ocExternal,         // Add In
        ocNoName,           // XlfResetToolbar
        ocNoName,           // XlfEvaluate
        ocNoName,           // XlfGetToolbar
        ocNoName,           // XlfGetTool
        ocNoName,           // XlfSpellingCheck
        ocErrorType,        // XlfErrorType
        ocNoName,           // XlfAppTitle
        ocNoName,           // XlfWindowTitle
        ocNoName,           // XlfSaveToolbar
        ocNoName,           // XlfEnableTool
        ocNoName,           // XlfPressTool
        ocNoName,           // XlfRegisterId
        ocNoName,           // XlfGetWorkbook
        ocAveDev,           // XlfAvedev
        ocBetaDist,         // XlfBetadist
        ocGammaLn,          // XlfGammaln
        ocBetaInv,          // XlfBetainv
        ocBinomDist,        // XlfBinomdist
        ocChiDist,          // XlfChidist
        ocChiInv,           // XlfChiinv
        ocKombin,           // XlfCombin
        ocConfidence,       // XlfConfidence
        ocKritBinom,        // XlfCritbinom
        ocEven,             // XlfEven
        ocExpDist,          // XlfExpondist
        ocFDist,            // XlfFdist
        ocFInv,             // XlfFinv
        ocFisher,           // XlfFisher
        ocFisherInv,        // XlfFisherinv
        ocFloor,            // XlfFloor
        ocGammaDist,        // XlfGammadist
        ocGammaInv,         // XlfGammainv
        ocCeil,             // XlfCeiling
        ocHypGeomDist,      // XlfHypgeomdist
        ocLogNormDist,      // XlfLognormdist
        ocLogInv,           // XlfLoginv
        ocNegBinomVert,     // XlfNegbinomdist
        ocNormDist,         // XlfNormdist
        ocStdNormDist,      // XlfNormsdist
        ocNormInv,          // XlfNorminv
        ocSNormInv,         // XlfNormsinv
        ocStandard,         // XlfStandardize
        ocOdd,              // XlfOdd
        ocVariationen,      // XlfPermut
        ocPoissonDist,      // XlfPoisson
        ocTDist,            // XlfTdist
        ocWeibull,          // XlfWeibull
        ocSumXMY2,          // XlfSumxmy2
        ocSumX2MY2,         // XlfSumx2my2
        ocSumX2DY2,         // XlfSumx2py2
        ocChiTest,          // XlfChitest
        ocCorrel,           // XlfCorrel
        ocCovar,            // XlfCovar
        ocForecast,         // XlfForecast
        ocFTest,            // XlfFtest
        ocIntercept,        // XlfIntercept
        ocPearson,          // XlfPearson
        ocRSQ,              // XlfRsq
        ocSTEYX,            // XlfSteyx
        ocSlope,            // XlfSlope
        ocTTest,            // XlfTtest
        ocProb,             // XlfProb
        ocDevSq,            // XlfDevsq
        ocGeoMean,          // XlfGeomean
        ocHarMean,          // XlfHarmean
        ocSumSQ,            // XlfSumsq
        ocKurt,             // XlfKurt
        ocSchiefe,          // XlfSkew
        ocZTest,            // XlfZtest,
        ocLarge,            // XlfLarge
        ocSmall,            // XlfSmall
        ocQuartile,         // XlfQuartile
        ocPercentile,       // XlfPercentile
        ocPercentrank,      // XlfPercentrank
        ocModalValue,       // XlfMode
        ocTrimMean,         // XlfTrimmean
        ocTInv,             // XlfTinv
        ocNoName,           // ?
        ocNoName,           // XlfMovieCommand
        ocNoName,           // XlfGetMovie
        ocConcat,           // XlfConcatenate
        ocPower,            // XlfPower
        ocNoName,           // XlfPivotAddData
        ocNoName,           // XlfGetPivotTable
        ocNoName,           // XlfGetPivotField
        ocNoName,           // XlfGetPivotItem
        ocRad,              // XlfRadians
        ocDeg,              // XlfDegrees
        ocSubTotal,         // XlfSubtotal
        ocSumIf,            // XlfSumif
        ocCountIf,          // XlfCountif
        ocCountEmptyCells,  // XlfCountblank
        ocNoName,           // XlfScenarioGet
        ocNoName,           // XlfOptionsListsGet
        ocISPMT,            // XlfIspmt
        ocGetDiffDate,      // XlfDatedif
        ocNoName,           // XlfDatestring
        ocNoName,           // XlfNumberstring
        ocRoman,            // XlfRoman
        ocNoName,           // XlfOpenDialog
        ocNoName,           // XlfSaveDialog            356
        ocNoName,           //
        ocNoName,           //
        ocNoName,           //
        ocNoName,           //                          360
        ocAverageA,         // MITTELWERTA
        ocMaxA,             // MAXA
        ocMinA,             // MINA
        ocStDevPA,          // STABWNA
        ocVarPA,            // VARIANZENA               365
        ocStDevA,           // STABWA
        ocVarA,             // VARIANZA
        ocNoName,           //
        ocNoName,           //
        ocNoName,           //  370
        ocNoName,           //
        ocNoName,           //
        ocNoName,           //
        ocNoName,           //
        ocNoName,           //  375
        ocNoName,           //
        ocNoName,           //
        ocNoName,           //
        ocNoName,           //
        ocNoName,           //  380
        ocNoName,           //
        ocNoName,           //
        ocNoName,           //
        ocNoName,           //
        ocNoName,           //  385
        ocNoName,           //
        ocNoName,           //
        ocNoName,           //
        ocNoName,           //
        ocNoName,           //  390
        ocNoName,           //
        ocNoName,           //
        ocNoName,           //
        ocNoName,           //
        ocNoName,           //  395
        ocNoName,           //
        ocNoName,           //
        ocNoName,           //
        ocNoName            //  399
    };

    if( nIndex <= nLastInd )
        return pMap[ nIndex ];
    else
        return ocNoName;
}


BYTE ExcelToSc::IndexToAnzahl( UINT16 nIndex )
{
    static const BYTE pMap[ nLastInd + 1 ] =
    {
        0,      // XlfCount
        0,      // ##XlfIf##
        1,      // XlfIsna
        1,      // XlfIserror
        1,      // XlfSum
        1,      // XlfAverage
        1,      // XlfMin
        1,      // XlfMax
        1,      // XlfRow
        1,      // XlfColumn
        0,      // XlfNa
        0,      // XlfNpv
        0,      // XlfStdev
        2,      // XlfDollar
        3,      // XlfFixed
        1,      // XlfSin
        1,      // XlfCos
        1,      // XlfTan
        1,      // XlfAtan
        0,      // XlfPi
        1,      // XlfSqrt
        1,      // XlfExp
        1,      // XlfLn
        1,      // XlfLog10
        1,      // XlfAbs
        1,      // XlfInt
        1,      // XlfSign
        2,      // XlfRound
        0,      // XlfLookup
        0,      // XlfIndex
        2,      // XlfRept
        3,      // XlfMid
        1,      // XlfLen
        1,      // XlfValue
        0,      // XlfTrue
        0,      // XlfFalse
        0,      // XlfAnd           *Parmeter n-Stueck?
        0,      // XlfOr            *
        1,      // XlfNot
        2,      // XlfMod
        3,      // XlfDcount
        3,      // XlfDsum
        3,      // XlfDaverage
        3,      // XlfDmin
        3,      // XlfDmax
        3,      // XlfDstdev
        0,      // XlfVar
        3,      // XlfDvar
        2,      // XlfText
        0,      // XlfLinest
        4,      // XlfTrend
        0,      // XlfLogest
        0,      // XlfGrowth
        0,      // XlfGoto
        0,      // XlfHalt
        0,      // ?
        0,      // XlfPv
        0,      // XlfFv
        0,      // XlfNper
        3,      // XlfPmt
        0,      // XlfRate
        3,      // XlfMirr
        0,      // XlfIrr
        0,      // XlfRand
        0,      // XlfMatch
        3,      // XlfDate
        3,      // XlfTime
        1,      // XlfDay
        1,      // XlfMonth
        1,      // XlfYear
        1,      // XlfWeekday
        1,      // XlfHour
        1,      // XlfMinute
        1,      // XlfSecond
        0,      // XlfNow
        1,      // XlfAreas
        1,      // XlfRows
        1,      // XlfColumns
        0,      // XlfOffset
        0,      // XlfAbsref
        0,      // XlfRelref
        0,      // XlfArgument
        2,      // XlfSearch
        1,      // XlfTranspose
        1,      // XlfError
        0,      // XlfStep
        1,      // XlfType
        0,      // XlfEcho
        0,      // XlfSetName
        0,      // XlfCaller
        0,      // XlfDeref
        0,      // XlfWindows
        0,      // XlfSeries
        0,      // XlfDocuments
        0,      // XlfActiveCell
        0,      // XlfSelection
        0,      // XlfResult
        2,      // XlfAtan2
        1,      // XlfAsin
        1,      // XlfAcos
        0,      // XlfChoose
        3,      // XlfHlookup
        3,      // XlfVlookup
        0,      // XlfLinks
        0,      // XlfInput
        1,      // XlfIsref
        0,      // XlfGetFormula
        0,      // XlfGetName
        0,      // XlfSetValue
        1,      // XlfLog
        0,      // XlfExec
        1,      // XlfChar
        1,      // XlfLower
        1,      // XlfUpper
        1,      // XlfProper
        2,      // XlfLeft
        2,      // XlfRight
        2,      // XlfExact
        1,      // XlfTrim
        4,      // XlfReplace
        4,      // XlfSubstitute
        1,      // XlfCode
        0,      // XlfNames
        0,      // XlfDirectory
        2,      // XlfFind
        2,      // XlfCell
        1,      // XlfIserr
        1,      // XlfIstext
        1,      // XlfIsnumber
        1,      // XlfIsblank
        1,      // XlfT
        1,      // XlfN
        0,      // XlfFopen
        0,      // XlfFclose
        0,      // XlfFsize
        0,      // XlfFreadln
        0,      // XlfFread
        0,      // XlfFwriteln
        0,      // XlfFwrite
        0,      // XlfFpos
        1,      // XlfDatevalue
        1,      // XlfTimevalue
        3,      // XlfSln
        4,      // XlfSyd
        0,      // XlfDdb
        0,      // XlfGetDef
        0,      // XlfReftext
        0,      // XlfTextref
        2,      // XlfIndirect
        0,      // XlfRegister
        0,      // XlfCall
        0,      // XlfAddBar
        0,      // XlfAddMenu
        0,      // XlfAddCommand
        0,      // XlfEnableCommand
        0,      // XlfCheckCommand
        0,      // XlfRenameCommand
        0,      // XlfShowBar
        0,      // XlfDeleteMenu
        0,      // XlfDeleteCommand
        0,      // XlfGetChartItem
        0,      // XlfDialogBox
        1,      // XlfClean
        1,      // XlfMdeterm
        1,      // XlfMinverse
        2,      // XlfMmult
        0,      // XlfFiles
        0,      // XlfIpmt
        0,      // XlfPpmt
        0,      // XlfCounta
        0,      // XlfCancelKey
        0,      // ?
        0,      // XlfAppMinimize
        0,      // XlfAppMaximize
        0,      // XlfBringToFront
        0,      // XlfInitiate
        0,      // XlfRequest
        0,      // XlfPoke
        0,      // XlfExecute
        0,      // XlfTerminate
        0,      // XlfRestart
        0,      // XlfHelp
        0,      // XlfGetBar
        0,      // XlfProduct
        1,      // XlfFact
        0,      // XlfGetCell
        0,      // XlfGetWorkspace
        0,      // XlfGetWindow
        0,      // XlfGetDocument
        3,      // XlfDproduct
        1,      // XlfIsnontext
        0,      // XlfGetNote
        0,      // XlfNote
        3,      // XlfStdevp
        0,      // XlfVarp
        3,      // XlfDstdevp
        3,      // XlfDvarp
        1,      // XlfTrunc
        1,      // XlfIslogical
        3,      // XlfDcounta
        0,      // XlfDeleteBar
        0,      // XlfUnregister
        0,      // ?
        0,      // ?
        0,      // XlfUsdollar
        0,      // XlfFindb
        0,      // XlfSearchb
        0,      // XlfReplaceb
        0,      // XlfLeftb
        0,      // XlfRightb
        0,      // XlfMidb
        0,      // XlfLenb
        2,      // XlfRoundup
        2,      // XlfRounddown
        0,      // XlfAsc
        0,      // XlfDbcs
        0,      // XlfRank
        0,      // ?
        0,      // ?
        0,      // XlfAddress
        2,      // XlfDays360
        0,      // XlfToday
        0,      // XlfVdb
        0,      // ?
        0,      // ?
        0,      // ?
        0,      // ?
        0,      // XlfMedian
        0,      // XlfSumproduct
        1,      // XlfSinh
        1,      // XlfCosh
        1,      // XlfTanh
        1,      // XlfAsinh
        1,      // XlfAcosh
        1,      // XlfAtanh
        3,      // XlfDget
        0,      // XlfCreateObject
        0,      // XlfVolatile
        0,      // XlfLastError
        0,      // XlfCustomUndo
        0,      // XlfCustomRepeat
        0,      // XlfFormulaConvert
        0,      // XlfGetLinkInfo
        0,      // XlfTextBox
        1,      // XlfInfo
        0,      // XlfGroup
        0,      // XlfGetObject
        0,      // XlfDb
        0,      // XlfPause
        0,      // ?
        0,      // ?
        0,      // XlfResume
        2,      // XlfFrequency
        0,      // XlfAddToolbar
        0,      // XlfDeleteToolbar
        2,      // Add In
        0,      // XlfResetToolbar
        0,      // XlfEvaluate
        0,      // XlfGetToolbar
        0,      // XlfGetTool
        0,      // XlfSpellingCheck
        1,      // XlfErrorType
        0,      // XlfAppTitle
        0,      // XlfWindowTitle
        0,      // XlfSaveToolbar
        0,      // XlfEnableTool
        0,      // XlfPressTool
        0,      // XlfRegisterId
        0,      // XlfGetWorkbook
        0,      // XlfAvedev
        0,      // XlfBetadist
        1,      // XlfGammaln
        0,      // XlfBetainv
        4,      // XlfBinomdist
        2,      // XlfChidist
        2,      // XlfChiinv
        2,      // XlfCombin
        3,      // XlfConfidence
        3,      // XlfCritbinom
        1,      // XlfEven
        3,      // XlfExpondist
        3,      // XlfFdist
        3,      // XlfFinv
        1,      // XlfFisher
        1,      // XlfFisherinv
        2,      // XlfFloor             ??? da war mal eine 0 ???
        4,      // XlfGammadist
        3,      // XlfGammainv
        2,      // XlfCeiling
        4,      // XlfHypgeomdist
        3,      // XlfLognormdist
        3,      // XlfLoginv
        3,      // XlfNegbinomdist
        4,      // XlfNormdist
        1,      // XlfNormsdist
        3,      // XlfNorminv
        1,      // XlfNormsinv
        3,      // XlfStandardize
        1,      // XlfOdd
        2,      // XlfPermut
        3,      // XlfPoisson
        3,      // XlfTdist
        4,      // XlfWeibull
        2,      // XlfSumxmy2
        2,      // XlfSumx2my2
        2,      // XlfSumx2py2
        2,      // XlfChitest
        2,      // XlfCorrel
        2,      // XlfCovar
        3,      // XlfForecast
        2,      // XlfFtest
        2,      // XlfIntercept
        2,      // XlfPearson
        2,      // XlfRsq
        2,      // XlfSteyx
        2,      // XlfSlope
        4,      // XlfTtest
        0,      // XlfProb
        0,      // XlfDevsq
        0,      // XlfGeomean
        0,      // XlfHarmean
        0,      // XlfSumsq
        0,      // XlfKurt
        0,      // XlfSkew
        0,      // XlfZtest
        2,      // XlfLarge
        2,      // XlfSmall
        2,      // XlfQuartile
        2,      // XlfPercentile
        2,      // XlfPercentrank
        0,      // XlfMode
        2,      // XlfTrimmean
        2,      // XlfTinv
        0,      // ?
        0,      // XlfMovieCommand
        0,      // XlfGetMovie
        0,      // XlfConcatenate
        2,      // XlfPower
        0,      // XlfPivotAddData
        0,      // XlfGetPivotTable
        0,      // XlfGetPivotField
        0,      // XlfGetPivotItem
        1,      // XlfRadians
        1,      // XlfDegrees
        0,      // XlfSubtotal
        0,      // XlfSumif
        2,      // XlfCountif
        1,      // XlfCountblank
        0,      // XlfScenarioGet
        0,      // XlfOptionsListsGet
        4,      // XlfIspmt
        0,      // XlfDatedif
        0,      // XlfDatestring
        0,      // XlfNumberstring
        1,      // XlfRoman
        0,      // XlfOpenDialog
        0,      // XlfSaveDialog        356
        0,      //
        0,      //
        0,      //
        0,      //                      360
        0,      // MITTELWERTA
        0,      // MAXA
        0,      // MINA
        0,      // STABWNA
        0,      // VARIANZENA           365
        0,      // STABWA
        0,      // VARIANZA
        0,      //
        0,      //
        0,      //                      370
        0,      //
        0,      //
        0,      //
        0,      //
        0,      //                      375
        0,      //
        0,      //
        0,      //
        0,      //
        0,      //                      380
        0,      //
        0,      //
        0,      //
        0,      //
        0,      //                      385
        0,      //
        0,      //
        0,      //
        0,      //
        0,      //                      390
        0,      //
        0,      //
        0,      //
        0,      //
        0,      //                      395
        0,      //
        0,      //
        0,      //
        0       //
    };

    if( nIndex <= nLastInd )
        return pMap[ nIndex ];
    else
        return 0;
}


void ExcelToSc::ExcRelToScRel( UINT16 nRow, UINT8 nCol, SingleRefData &rSRD, const BOOL bName )
{
    if( bName )
    {
        // C O L
        if( nRow & 0x4000 )
        {//                                                         rel Col
            rSRD.SetColRel( TRUE );
            rSRD.nRelCol = *( ( sal_Char * ) &nCol );
        }
        else
        {//                                                         abs Col
            rSRD.SetColRel( FALSE );
            rSRD.nCol = nCol;
        }

        // R O W
        if( nRow & 0x8000 )
        {//                                                         rel Row
            rSRD.SetRowRel( TRUE );
            if( nRow & 0x2000 ) // Bit 13 gesetzt?
                //                                              -> Row negativ
                rSRD.nRelRow = *( ( INT16 * ) &nRow ) | 0xC000;
            else
                //                                              -> Row positiv
                rSRD.nRelRow = nRow & nRowMask;
        }
        else
        {//                                                         abs Row
            rSRD.SetRowRel( FALSE );
            rSRD.nRow = nRow & nRowMask;
        }

        // T A B
        // #67965# abs needed if rel in shared formula for ScCompiler UpdateNameReference
        if ( rSRD.IsTabRel() && !rSRD.IsFlag3D() )
            rSRD.nTab = *pExcRoot->pAktTab;
    }
    else
    {
        // C O L
        rSRD.SetColRel( ( nRow & 0x4000 ) > 0 );
        rSRD.nCol = nCol;

        // R O W
        rSRD.SetRowRel( ( nRow & 0x8000 ) > 0 );
        rSRD.nRow = nRow & nRowMask;

        if ( rSRD.IsColRel() )
            rSRD.nRelCol = rSRD.nCol - aEingPos.Col();
        if ( rSRD.IsRowRel() )
            rSRD.nRelRow = rSRD.nRow - aEingPos.Row();
    }
}


const ScTokenArray* ExcelToSc::GetBoolErr( BoolError eType )
{
    UINT16                  nError;
    aPool.Reset();
    aStack.Reset();

    DefTokenId              eOc;

    switch( eType )
    {
        case BE_NULL:       eOc = ocStop; nError = errNoCode; break;
        case BE_DIV0:       eOc = ocStop; nError = errIllegalFPOperation; break;
        case BE_VALUE:      eOc = ocNoValue; nError = NOVALUE; break;
        case BE_REF:        eOc = ocStop; nError = errNoRef; break;
        case BE_NAME:       eOc = ocStop; nError = errNoName; break;
        case BE_NUM:        eOc = ocStop; nError = errIllegalFPOperation; break;
        case BE_NA:         eOc = ocNoValue; nError = NOVALUE; break;
        case BE_TRUE:       eOc = ocTrue; nError = 0; break;
        case BE_FALSE:      eOc = ocFalse; nError = 0; break;
        case BE_UNKNOWN:    eOc = ocStop; nError = errUnknownState; break;
        default:
            DBG_ERROR( "*ExcelToSc::GetBoolErr(): fehlender enum!" );
            eOc = ocNoName;
            nError = errUnknownState;
    }

    aPool << eOc;
    if( eOc != ocStop )
        aPool << ocOpen << ocClose;

    aPool >> aStack;

    const ScTokenArray*     pErgebnis = aPool[ aStack.Get() ];
    if( nError )
        ( ( ScTokenArray* ) pErgebnis )->SetError( nError );

    ( ( ScTokenArray* ) pErgebnis )->SetRecalcModeNormal();

    return pErgebnis;
}


// if a shared formula was found, stream seeks to first byte after <nFormulaLen>,
// else stream pointer stays unchanged
BOOL ExcelToSc::GetShrFmla( const ScTokenArray*& rpErgebnis, UINT32 nFormulaLen )
{
    BYTE            nOp;
    BOOL            bRet = TRUE;

    if( nFormulaLen == 0 )
        bRet = FALSE;
    else
    {
        aIn.PushPosition();

        aIn >> nOp;

        if( nOp == 0x01 )   // Shared Formula       [    277]
        {
            UINT16 nCol, nRow;

            aIn >> nRow >> nCol;

            aStack << aPool.Store( pExcRoot->pShrfmlaBuff->Find(
                ScAddress( nCol, nRow, *pExcRoot->pAktTab ) ) );

            bRet = TRUE;
        }
        else
            bRet = FALSE;

        aIn.PopPosition();
    }

    if( bRet )
    {
        aIn.Ignore( nFormulaLen );
        rpErgebnis = aPool[ aStack.Get() ];
    }
    else
        rpErgebnis = NULL;

    return bRet;
}


BOOL ExcelToSc::SetCurVal( ScFormulaCell &rCell, double &rfCurVal )
{
    UINT16  nInd;
    BYTE    nType;
    BYTE    nVal;
    BOOL    bString = FALSE;

#ifdef __BIGENDIAN
    // Code fuer alle anstaendigen Prozessoren
    nType = *( ( ( BYTE * ) &rfCurVal ) + 7 );
    nVal = *( ( ( BYTE * ) &rfCurVal ) + 5 );
    nInd = *( ( UINT16 * ) &rfCurVal );
#else
    // fuer Schund-Prozessoren
    nType = *( ( BYTE * ) &rfCurVal );
    nVal = *( ( ( BYTE * ) &rfCurVal ) + 2 );
    nInd = *( ( ( UINT16 * ) &rfCurVal ) + 3 );
#endif

    if( ( UINT16 ) ~nInd )
        // Wert ist Float
        rCell.SetDouble( rfCurVal );
    else
    {
        switch( nType )
        {
            case 0:     // String
                bString = TRUE;
                break;
            case 1:     // Bool
                if( nVal )
                    rfCurVal = 1.0;
                else
                    rfCurVal = 0.0;
                rCell.SetDouble( rfCurVal );
                break;
            case 2:     // Error
            {
                switch( nVal )
                {// nInd temp. missbrauchen
                    case 0x00:  nInd = errNoCode; break;            // #NULL!
                    case 0x07:  nInd = errIllegalFPOperation; break;// #DIV/0!
                    case 0x0F:  nInd = NOVALUE; break;              // #VALUE!
                    case 0x17:  nInd = errNoRef; break;             // #REF!
                    case 0x1D:  nInd = errNoName; break;            // #NAME?
                    case 0x24:  nInd = errIllegalFPOperation; break;// #NUM!
                    case 0x2A:  nInd = NOVALUE; break;              // #N/A
                    default:    nInd = errIllegalArgument;
                }
                rCell.SetErrCode( nInd );
            }
                break;
        }
    }

    return bString;
}


void ExcelToSc::SetError( ScFormulaCell &rCell, const ConvErr eErr )
{
    UINT16  nInd;

    switch( eErr )
    {
        case ConvErrNi:         nInd = errUnknownToken; break;
        case ConvErrNoMem:      nInd = errCodeOverflow; break;
        case ConvErrExternal:   nInd = errNoName; break;
        case ConvErrCount:      nInd = errCodeOverflow; break;
        default:                nInd = errNoCode;   // hier fiel mir nichts
                                                    //  Besseres ein...
    }

    rCell.SetErrCode( nInd );
}


void ExcelToSc::SetComplCol( ComplRefData &rCRD )
{
    SingleRefData   &rSRD = rCRD.Ref2;
    if( rSRD.IsColRel() )
        rSRD.nRelCol = MAXCOL - aEingPos.Col();
    else
        rSRD.nCol = MAXCOL;
}


void ExcelToSc::SetComplRow( ComplRefData &rCRD )
{
    SingleRefData   &rSRD = rCRD.Ref2;
    if( rSRD.IsRowRel() )
        rSRD.nRelRow = MAXROW - aEingPos.Row();
    else
        rSRD.nRow = MAXROW;
}




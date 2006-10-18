/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: excform.cxx,v $
 *
 *  $Revision: 1.43 $
 *
 *  last change: $Author: ihi $ $Date: 2006-10-18 12:24:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
#include "excform.hxx"

#ifndef _OSL_ENDIAN_H_
#include <osl/endian.h>
#endif

#include "cell.hxx"
#include "document.hxx"
#include "rangenam.hxx"
#include "global.hxx"
#include "errorcodes.hxx"

#include "imp_op.hxx"
#include "root.hxx"

#ifndef SC_XLTRACER_HXX
#include "xltracer.hxx"
#endif
#ifndef SC_XIHELPER_HXX
#include "xihelper.hxx"
#endif
#ifndef SC_XILINK_HXX
#include "xilink.hxx"
#endif
#ifndef SC_XINAME_HXX
#include "xiname.hxx"
#endif


const UINT16 ExcelToSc::nRowMask = 0x3FFF;
const UINT16 ExcelToSc::nLastInd = 399;




void ImportExcel::Formula25()
{
    XclAddress aXclPos;
    UINT16  nXF, nFormLen;
    double  fCurVal;
    BYTE    nAttr0, nFlag0;
    BOOL    bShrFmla;

    aIn >> aXclPos;

    if( GetBiff() == EXC_BIFF2 )
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

    Formula( aXclPos, nXF, nFormLen, fCurVal, nFlag0, bShrFmla );
}


void ImportExcel::Formula3()
{
    Formula4();
}


void ImportExcel::Formula4()
{
    XclAddress aXclPos;
    UINT16  nXF, nFormLen;
    double  fCurVal;
    BYTE    nFlag0;

    aIn >> aXclPos >> nXF >> fCurVal >> nFlag0;
    aIn.Ignore( 1 );
    aIn >> nFormLen;

    nLastXF = nXF;

    Formula( aXclPos, nXF, nFormLen, fCurVal, nFlag0, FALSE );
}


void ImportExcel::Formula( const XclAddress& rXclPos,
    UINT16 nXF, UINT16 nFormLen, double& rCurVal, BYTE nFlag, BOOL bShrFmla )
{
    ConvErr eErr = ConvOK;

    ScAddress aScPos( ScAddress::UNINITIALIZED );
    if( GetAddressConverter().ConvertAddress( aScPos, rXclPos, GetCurrScTab(), true ) )
    {
        // jetzt steht Lesemarke auf Formel, Laenge in nFormLen
        const ScTokenArray* pErgebnis;
        BOOL                bConvert;

        pFormConv->Reset( aScPos );

        if( bShrFmla )
            bConvert = !pFormConv->GetShrFmla( pErgebnis, maStrm, nFormLen );
        else
            bConvert = TRUE;

        if( bConvert )
            eErr = pFormConv->Convert( pErgebnis, maStrm, nFormLen );

        ScFormulaCell*      pZelle = NULL;

        if( pErgebnis )
        {
            pZelle = new ScFormulaCell( pD, aScPos, pErgebnis );
            pD->PutCell( aScPos.Col(), aScPos.Row(), aScPos.Tab(), pZelle, (BOOL)TRUE );
        }
        else
        {
            CellType        eCellType;
            ScBaseCell*     pBaseCell;
            pD->GetCellType( aScPos.Col(), aScPos.Row(), aScPos.Tab(), eCellType );
            if( eCellType == CELLTYPE_FORMULA )
            {
                pD->GetCell( aScPos.Col(), aScPos.Row(), aScPos.Tab(), pBaseCell );
                pZelle = ( ScFormulaCell* ) pBaseCell;
                if( pZelle )
                    pZelle->AddRecalcMode( RECALCMODE_ONLOAD_ONCE );
            }
        }

        if( pZelle )
        {
            if( eErr != ConvOK )
                ExcelToSc::SetError( *pZelle, eErr );
            else
                ExcelToSc::SetCurVal( *pZelle, rCurVal );
        }

        GetXFRangeBuffer().SetXF( aScPos, nXF );
    }
}




ExcelToSc::ExcelToSc( const XclImpRoot& rRoot ) :
    ExcelConverterBase( 512 ),
    XclImpRoot( rRoot ),
    maFuncProv( rRoot ),
    meBiff( rRoot.GetBiff() )
{
}

ExcelToSc::~ExcelToSc()
{
}

void ExcelToSc::GetDummy( const ScTokenArray*& pErgebnis )
{
    aPool.Store( CREATE_STRING( "Dummy()" ) );
    aPool >> aStack;
    pErgebnis = aPool[ aStack.Get() ];
}


// stream seeks to first byte after <nFormulaLen>
ConvErr ExcelToSc::Convert( const ScTokenArray*& pErgebnis, XclImpStream& aIn, sal_Size nFormulaLen, const FORMULA_TYPE eFT )
{
    RootData&       rR = GetOldRoot();
    BYTE            nOp, nLen, nByte;
    UINT16          nUINT16;
    INT16           nINT16;
    double          fDouble;
    String          aString;
    BOOL            bError = FALSE;
    BOOL            bArrayFormula = FALSE;
    TokenId         nMerk0;
    const BOOL      bRangeName = eFT == FT_RangeName;
    const BOOL      bSharedFormula = eFT == FT_SharedFormula;
    const BOOL      bRNorSF = bRangeName || bSharedFormula;

    SingleRefData   aSRD;
    ComplRefData    aCRD;

    bExternName = FALSE;

    if( eStatus != ConvOK )
    {
        aIn.Ignore( nFormulaLen );
        return eStatus;
    }

    if( nFormulaLen == 0 )
    {
        aPool.Store( CREATE_STRING( "-/-" ) );
        aPool >> aStack;
        pErgebnis = aPool[ aStack.Get() ];
        return ConvOK;
    }

    sal_Size nEndPos = aIn.GetRecPos() + nFormulaLen;

    while( (aIn.GetRecPos() < nEndPos) && !bError )
    {
        aIn >> nOp;

        // #98524# always reset flags
        aSRD.InitFlags();
        aCRD.InitFlags();

        switch( nOp )   //                              Buch Seite:
        {           //                                      SDK4 SDK5
            case 0x01: // Array Formula                         [325    ]
                       // Array Formula or Shared Formula       [    277]
            case 0x02: // Data Table                            [325 277]
                nUINT16 = 3;

                if( meBiff != EXC_BIFF2 )
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
//#100928#      aPool << ocOpen << aStack << ocSep << nMerk0 << ocClose;
                aPool << aStack << ocSep << nMerk0;
                    // doesn't fit exactly, but is more Excel-like
                aPool >> aStack;
                break;
            case 0x11: // Range                                 [314 265]
                PushRangeOperator();
                break;
            case 0x12: // Unary Plus                            [312 264]
                aPool << ocAdd << aStack;
                aPool >> aStack;
                break;
            case 0x13: // Unary Minus                           [312 264]
                aPool << ocNegSub << aStack;
                aPool >> aStack;
                break;
            case 0x14: // Percent Sign                          [312 264]
                aPool << aStack << ocPercentSign;
                aPool >> aStack;
                break;
            case 0x15: // Parenthesis                           [326 278]
                aPool << ocOpen << aStack << ocClose;
                aPool >> aStack;
                break;
            case 0x16: // Missing Argument                      [314 266]
                aPool << ocMissing;
                aPool >> aStack;
                GetTracer().TraceFormulaMissingArg();
                break;
            case 0x17: // String Constant                       [314 266]
                aIn >> nLen;
                aString = aIn.ReadRawByteString( nLen );

                aStack << aPool.Store( aString );
                break;
            case 0x19: // Special Attribute                     [327 279]
            {
                UINT16  nData, nFakt;
                BYTE    nOpt;

                aIn >> nOpt;

                if( meBiff == EXC_BIFF2 )
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
                switch( meBiff )
                {
                    case EXC_BIFF2: aIn.Ignore( 7 );    break;
                    case EXC_BIFF3:
                    case EXC_BIFF4: aIn.Ignore( 10 );   break;
                    case EXC_BIFF5:
                        DBG_WARNING( "-ExcelToSc::Convert(): 0x1A gibt's nicht in Biff5!" );
                    default:
                        DBG_WARNING( "-ExcelToSc::Convert(): Ein wenig vergesslich, was?" );
                }
                break;
            case 0x1B: // End External Reference                [330    ]
                switch( meBiff )
                {
                    case EXC_BIFF2: aIn.Ignore( 3 );    break;
                    case EXC_BIFF3:
                    case EXC_BIFF4: aIn.Ignore( 4 );    break;
                    case EXC_BIFF5:
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
                    case EXC_ERR_NULL:
                    case EXC_ERR_DIV0:
                    case EXC_ERR_VALUE:
                    case EXC_ERR_REF:
                    case EXC_ERR_NAME:
                    case EXC_ERR_NUM:   eOc = ocStop;       break;
                    case EXC_ERR_NA:    eOc = ocNoValue;    break;
                    default:            eOc = ocNoName;
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
            {
                sal_uInt16 nXclFunc;
                if( meBiff <= EXC_BIFF3 )
                    nXclFunc = aIn.ReaduInt8();
                else
                    aIn >> nXclFunc;
                if( const XclFunctionInfo* pFuncInfo = maFuncProv.GetFuncInfoFromXclFunc( nXclFunc ) )
                    DoMulArgs( pFuncInfo->meOpCode, pFuncInfo->mnMaxParamCount );
                else
                    DoMulArgs( ocNoName, 0 );
            }
            break;
            case 0x42:
            case 0x62:
            case 0x22: // Function, Variable Number of Arg.     [333 283]
            {
                sal_uInt16 nXclFunc;
                sal_uInt8 nParamCount;
                aIn >> nParamCount;
                nParamCount &= 0x7F;
                if( meBiff <= EXC_BIFF3 )
                    nXclFunc = aIn.ReaduInt8();
                else
                    aIn >> nXclFunc;
                if( const XclFunctionInfo* pFuncInfo = maFuncProv.GetFuncInfoFromXclFunc( nXclFunc ) )
                    DoMulArgs( pFuncInfo->meOpCode, nParamCount );
                else
                    DoMulArgs( ocNoName, 0 );
            }
            break;
            case 0x43:
            case 0x63:
            case 0x23: // Name                                  [318 269]
            {
                aIn >> nUINT16;
                switch( meBiff )
                {
                    case EXC_BIFF2: aIn.Ignore( 5 );    break;
                    case EXC_BIFF3:
                    case EXC_BIFF4: aIn.Ignore( 8 );    break;
                    case EXC_BIFF5: aIn.Ignore( 12 );   break;
                    default:
                        DBG_ERROR(
                        "-ExcelToSc::Convert(): Ein wenig vergesslich, was?" );
                }
                const XclImpName* pName = GetNameManager().GetName( nUINT16 );
                if(pName && !pName->GetScRangeData())
                    aStack << aPool.Store( ocMacro, pName->GetXclName() );
                else
                    aStack << aPool.Store( nUINT16 );
            }
                break;
            case 0x44:
            case 0x64:
            case 0x24: // Cell Reference                        [319 270]
            case 0x4A:
            case 0x6A:
            case 0x2A: // Deleted Cell Reference                [323 273]
                aIn >> nUINT16 >> nByte;
                aSRD.nCol = static_cast<SCsCOL>(nByte);
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
            case 0x47:
            case 0x67:
            case 0x27: // Erroneous Constant Reference Subexpr. [322 272]
            case 0x48:
            case 0x68:
            case 0x28: // Incomplete Constant Reference Subexpr.[331 281]
                aIn.Ignore( (meBiff == EXC_BIFF2) ? 4 : 6 );
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
            case 0x49:
            case 0x69:
            case 0x29: // Variable Reference Subexpression      [331 281]
            case 0x4E:
            case 0x6E:
            case 0x2E: // Reference Subexpression Within a Name [332 282]
            case 0x4F:
            case 0x6F:
            case 0x2F: // Incomplete Reference Subexpression... [332 282]
                aIn.Ignore( (meBiff == EXC_BIFF2) ? 1 : 2 );
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
                    pExtName = rR.pExtNameBuff->GetName( nUINT16 );
                    if( pExtName && pExtName->IsDDE() &&
                        rR.pExtSheetBuff->IsLink( ( UINT16 ) nINT16 ) )
                    {
                        String          aAppl, aExtDoc;
                        TokenId         nPar1, nPar2;

                        rR.pExtSheetBuff->GetLink( ( UINT16 ) nINT16 , aAppl, aExtDoc );
                        nPar1 = aPool.Store( aAppl );
                        nPar2 = aPool.Store( aExtDoc );
                        nMerk0 = aPool.Store( pExtName->aName );
                        aPool   << ocDde << ocOpen << nPar1 << ocSep << nPar2 << ocSep
                                << nMerk0 << ocClose;

                        GetDoc().CreateDdeLink( aAppl, aExtDoc, pExtName->aName, SC_DDE_DEFAULT );
                    }
                    else
                        aPool << ocBad;

                    aPool >> aStack;
                }
                else
                    aStack << aPool.Store( nUINT16 );
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
                    if( rR.pExtSheetBuff->GetScTabIndex( nExtSheet, nTabLast ) )
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
                    aSRD.nTab = static_cast<SCTAB>(nTabFirst);
                    aSRD.SetFlag3D( TRUE );
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
                    if ( !ValidTab(static_cast<SCTAB>(nTabFirst)) )
                        aSRD.SetTabDeleted( TRUE );

                    if( nTabLast != nTabFirst )
                    {
                        aCRD.Ref1 = aCRD.Ref2 = aSRD;
                        aCRD.Ref2.nTab = static_cast<SCTAB>(nTabLast);
                        aCRD.Ref2.SetTabDeleted( !ValidTab(static_cast<SCTAB>(nTabLast)) );
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
                    if( rR.pExtSheetBuff->GetScTabIndex( nExtSheet, nTabLast ) )
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

                    rR1.nTab = static_cast<SCTAB>(nTabFirst);
                    rR2.nTab = static_cast<SCTAB>(nTabLast);
                    rR1.SetFlag3D( TRUE );
                    rR1.SetTabRel( FALSE );
                    rR2.SetFlag3D( nTabFirst != nTabLast );
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
                    if ( !ValidTab(static_cast<SCTAB>(nTabFirst)) )
                        rR1.SetTabDeleted( TRUE );
                    if ( !ValidTab(static_cast<SCTAB>(nTabLast)) )
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
    else if( aIn.GetRecPos() != nEndPos )
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

    aIn.Seek( nEndPos );
    return eRet;
}


// stream seeks to first byte after <nFormulaLen>
ConvErr ExcelToSc::Convert( _ScRangeListTabs& rRangeList, XclImpStream& aIn, sal_Size nFormulaLen, const FORMULA_TYPE eFT )
{
    RootData&       rR = GetOldRoot();
    BYTE            nOp, nLen;
    sal_Size        nIgnore;
    UINT16          nUINT16;
    UINT8           nByte;
    BOOL            bError = FALSE;
    BOOL            bArrayFormula = FALSE;
    const BOOL      bRangeName = eFT == FT_RangeName;
    const BOOL      bSharedFormula = eFT == FT_SharedFormula;
    const BOOL      bRNorSF = bRangeName || bSharedFormula;

    SingleRefData   aSRD;
    ComplRefData    aCRD;
    aCRD.Ref1.nTab = aCRD.Ref2.nTab = aEingPos.Tab();

    bExternName = FALSE;

    if( eStatus != ConvOK )
    {
        aIn.Ignore( nFormulaLen );
        return eStatus;
    }

    if( nFormulaLen == 0 )
        return ConvOK;

    sal_Size nEndPos = aIn.GetRecPos() + nFormulaLen;

    while( (aIn.GetRecPos() < nEndPos) && !bError )
    {
        aIn >> nOp;
        nIgnore = 0;

        // #98524# always reset flags
        aSRD.InitFlags();
        aCRD.InitFlags();

        switch( nOp )   //                              Buch Seite:
        {           //                                      SDK4 SDK5
            case 0x01: // Array Formula                         [325    ]
                       // Array Formula or Shared Formula       [    277]
                nIgnore = (meBiff == EXC_BIFF2) ? 3 : 4;
                bArrayFormula = TRUE;
                break;
            case 0x02: // Data Table                            [325 277]
                nIgnore = (meBiff == EXC_BIFF2) ? 3 : 4;
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

                if( meBiff == EXC_BIFF2 )
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
                switch( meBiff )
                {
                    case EXC_BIFF2: nIgnore = 7;    break;
                    case EXC_BIFF3:
                    case EXC_BIFF4: nIgnore = 10;   break;
                    case EXC_BIFF5: DBG_WARNING( "-ExcelToSc::Convert(): 0x1A gibt's nicht in Biff5!" );
                    default:        DBG_WARNING( "-ExcelToSc::Convert(): Ein wenig vergesslich, was?" );
                }
                break;
            case 0x1B: // End External Reference                [330    ]
                switch( meBiff )
                {
                    case EXC_BIFF2: nIgnore = 3;        break;
                    case EXC_BIFF3:
                    case EXC_BIFF4: nIgnore = 4;        break;
                    case EXC_BIFF5: DBG_WARNING( "-ExcelToSc::Convert(): 0x1B gibt's nicht in Biff5!" );
                    default:        DBG_WARNING( "-ExcelToSc::Convert(): Ein wenig vergesslich, was?" );
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
                nIgnore = 8;
                break;
            case 0x40:
            case 0x60:
            case 0x20: // Array Constant                        [317 268]
                nIgnore = 7;
                break;
            case 0x41:
            case 0x61:
            case 0x21: // Function, Fixed Number of Arguments   [333 282]
                nIgnore = (meBiff <= EXC_BIFF3) ? 1 : 2;
                break;
            case 0x42:
            case 0x62:
            case 0x22: // Function, Variable Number of Arg.     [333 283]
                nIgnore = (meBiff <= EXC_BIFF3) ? 2 : 3;
                break;
            case 0x43:
            case 0x63:
            case 0x23: // Name                                  [318 269]
                switch( meBiff )
                {
                    case EXC_BIFF2: nIgnore = 7;    break;
                    case EXC_BIFF3:
                    case EXC_BIFF4: nIgnore = 10;   break;
                    case EXC_BIFF5: nIgnore = 14;   break;
                    default:        DBG_ERROR( "-ExcelToSc::Convert(): Ein wenig vergesslich, was?" );
                }
                break;
            case 0x44:
            case 0x64:
            case 0x24: // Cell Reference                        [319 270]
                aIn >> nUINT16 >> nByte;
                aSRD.nCol = static_cast<SCsCOL>(nByte);
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
            case 0x47:
            case 0x67:
            case 0x27: // Erroneous Constant Reference Subexpr. [322 272]
            case 0x48:
            case 0x68:
            case 0x28: // Incomplete Constant Reference Subexpr.[331 281]
                nIgnore = (meBiff == EXC_BIFF2) ? 4 : 6;
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
            case 0x49:
            case 0x69:
            case 0x29: // Variable Reference Subexpression      [331 281]
            case 0x4E:
            case 0x6E:
            case 0x2E: // Reference Subexpression Within a Name [332 282]
            case 0x4F:
            case 0x6F:
            case 0x2F: // Incomplete Reference Subexpression... [332 282]
                nIgnore = (meBiff == EXC_BIFF2) ? 1 : 2;
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
                    if( rR.pExtSheetBuff->GetScTabIndex( nExtSheet, nTabLast ) )
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
                    BOOL b3D = ( static_cast<SCTAB>(nTabFirst) != aEingPos.Tab() ) || bRangeName;
                    aSRD.nTab = static_cast<SCTAB>(nTabFirst);
                    aSRD.SetFlag3D( b3D );
                    aSRD.SetTabRel( FALSE );

                    ExcRelToScRel( nRow, nCol, aSRD, bRangeName );

                    if( nTabLast != nTabFirst )
                    {
                        aCRD.Ref1 = aSRD;
                        aCRD.Ref2.nCol = aSRD.nCol;
                        aCRD.Ref2.nRow = aSRD.nRow;
                        aCRD.Ref2.nTab = static_cast<SCTAB>(nTabLast);
                        b3D = ( static_cast<SCTAB>(nTabLast) != aEingPos.Tab() );
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
                    if( rR.pExtSheetBuff->GetScTabIndex( nExtSheet, nTabLast ) )
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

                    rR1.nTab = static_cast<SCTAB>(nTabFirst);
                    rR2.nTab = static_cast<SCTAB>(nTabLast);
                    rR1.SetFlag3D( ( static_cast<SCTAB>(nTabFirst) != aEingPos.Tab() ) || bRangeName );
                    rR1.SetTabRel( FALSE );
                    rR2.SetFlag3D( ( static_cast<SCTAB>(nTabLast) != aEingPos.Tab() ) || bRangeName );
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
    else if( aIn.GetRecPos() != nEndPos )
        eRet = ConvErrCount;
    else if( bExternName )
        eRet = ConvErrExternal;
    else if( bArrayFormula )
        eRet = ConvOK;
    else
        eRet = ConvOK;

    aIn.Seek( nEndPos );
    return eRet;
}

BOOL ExcelToSc::GetAbsRefs( ScRangeList& rRangeList, XclImpStream& rStrm, sal_Size nLen )
{
    DBG_ERRORFILE( "ExcelToSc::GetAbsRefs - not implemented" );
    return false;
}

void ExcelToSc::DoMulArgs( DefTokenId eId, BYTE nAnz )
{
    TokenId                 eParam[ 256 ];
    INT32                   nLauf;

    if( eId == ocLog && nAnz == 1 )
        eId = ocLog10;
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
        if( const String* pExt = aPool.GetExternal( n ) )
        {
            if( const XclFunctionInfo* pFuncInfo = maFuncProv.GetFuncInfoFromXclMacroName( *pExt ) )
                aPool << pFuncInfo->meOpCode;
            else
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
        // attention: 0 = last parameter, nAnz-1 = first parameter
        INT16 nNull = -1;       // skip this parameter
        INT16 nSkipEnd = -1;    // skip all parameters <= nSkipEnd

        INT16 nLast = nAnz - 1;

        // Funktionen, bei denen Parameter wegfallen muessen
        if( eId == ocPercentrank && nAnz == 3 )
            nSkipEnd = 0;       // letzten Parameter bei Bedarf weglassen

        else if( eId == ocIndirect && nAnz == 2 )
            nSkipEnd = 0;

        else if( eId == ocAddress && nAnz > 3 )
            nNull = nAnz - 4;

        // Joost-Spezialfaelle
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

        // #84453# skip missing parameters at end of parameter list
        while( (nSkipEnd < nLast) && aPool.IsSingleOp( eParam[ nSkipEnd + 1 ], ocMissing ) )
            nSkipEnd++;

        // [Parameter{;Parameter}]
        if( nLast > nSkipEnd )
        {
            aPool << eParam[ nLast ];
            for( nLauf = nLast - 1 ; nLauf > nSkipEnd ; nLauf-- )
            {
                if( nLauf != nNull )
                    aPool << ocSep << eParam[ nLauf ];
            }
        }
    }
    aPool << ocClose;

    aPool >> aStack;
}


void ExcelToSc::ExcRelToScRel( UINT16 nRow, UINT8 nCol, SingleRefData &rSRD, const BOOL bName )
{
    if( bName )
    {
        // C O L
        if( nRow & 0x4000 )
        {//                                                         rel Col
            rSRD.SetColRel( TRUE );
            rSRD.nRelCol = static_cast<SCsCOL>(static_cast<INT8>(nCol));
        }
        else
        {//                                                         abs Col
            rSRD.SetColRel( FALSE );
            rSRD.nCol = static_cast<SCCOL>(nCol);
        }

        // R O W
        if( nRow & 0x8000 )
        {//                                                         rel Row
            rSRD.SetRowRel( TRUE );
            if( nRow & 0x2000 ) // Bit 13 gesetzt?
                //                                              -> Row negativ
                rSRD.nRelRow = static_cast<SCsROW>(static_cast<INT16>(nRow | 0xC000));
            else
                //                                              -> Row positiv
                rSRD.nRelRow = static_cast<SCsROW>(nRow & nRowMask);
        }
        else
        {//                                                         abs Row
            rSRD.SetRowRel( FALSE );
            rSRD.nRow = static_cast<SCROW>(nRow & nRowMask);
        }

        // T A B
        // #67965# abs needed if rel in shared formula for ScCompiler UpdateNameReference
        if ( rSRD.IsTabRel() && !rSRD.IsFlag3D() )
            rSRD.nTab = GetCurrScTab();
    }
    else
    {
        // C O L
        rSRD.SetColRel( ( nRow & 0x4000 ) > 0 );
        rSRD.nCol = static_cast<SCsCOL>(nCol);

        // R O W
        rSRD.SetRowRel( ( nRow & 0x8000 ) > 0 );
        rSRD.nRow = static_cast<SCsROW>(nRow & nRowMask);

        if ( rSRD.IsColRel() )
            rSRD.nRelCol = rSRD.nCol - aEingPos.Col();
        if ( rSRD.IsRowRel() )
            rSRD.nRelRow = rSRD.nRow - aEingPos.Row();

        // T A B
        // #i10184# abs needed if rel in shared formula for ScCompiler UpdateNameReference
        if ( rSRD.IsTabRel() && !rSRD.IsFlag3D() )
            rSRD.nTab = GetCurrScTab() + rSRD.nRelTab;
    }
}


void ExcelToSc::PushRangeOperator()
{
    // #i48496# try to convert the term singleref:singleref to a range reference
    bool bIsConstRange = false;
    TokenId nTokId0, nTokId1;
    aStack >> nTokId0;
    aStack >> nTokId1;

    if( (aPool.GetType( nTokId0 ) == T_RefC) && (aPool.GetType( nTokId1 ) == T_RefC) )
    {
        const SingleRefData* pRef1 = aPool.GetSRD( nTokId1 );
        const SingleRefData* pRef2 = aPool.GetSRD( nTokId0 );
        if( pRef1 && pRef2 )
        {
            ComplRefData aRangeRef;
            aRangeRef.InitFlags();
            aRangeRef.Ref1 = *pRef1;
            aRangeRef.Ref2 = *pRef2;
            aStack << aPool.Store( aRangeRef );
            bIsConstRange = true;
        }
    }

    if( !bIsConstRange )
    {
        aPool << nTokId1 << ocRange << nTokId0;
        aPool >> aStack;
    }
}


const ScTokenArray* ExcelToSc::GetBoolErr( XclBoolError eType )
{
    UINT16                  nError;
    aPool.Reset();
    aStack.Reset();

    DefTokenId              eOc;

    switch( eType )
    {
        case xlErrNull:     eOc = ocStop;       nError = errNoCode;             break;
        case xlErrDiv0:     eOc = ocStop;       nError = errIllegalFPOperation; break;
        case xlErrValue:    eOc = ocStop;       nError = errNoValue;            break;
        case xlErrRef:      eOc = ocStop;       nError = errNoRef;              break;
        case xlErrName:     eOc = ocStop;       nError = errNoName;             break;
        case xlErrNum:      eOc = ocStop;       nError = errIllegalFPOperation; break;
        case xlErrNA:       eOc = ocNoValue;    nError = NOVALUE;               break;
        case xlErrTrue:     eOc = ocTrue;       nError = 0;                     break;
        case xlErrFalse:    eOc = ocFalse;      nError = 0;                     break;
        case xlErrUnknown:  eOc = ocStop;       nError = errUnknownState;       break;
        default:
            DBG_ERROR( "ExcelToSc::GetBoolErr - wrong enum!" );
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
BOOL ExcelToSc::GetShrFmla( const ScTokenArray*& rpErgebnis, XclImpStream& aIn, sal_Size nFormulaLen )
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

            aStack << aPool.Store( GetOldRoot().pShrfmlaBuff->Find(
                ScAddress( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow), GetCurrScTab() ) ) );

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

#ifdef OSL_BIGENDIAN
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
                rCell.SetErrCode( XclTools::GetScErrorCode( nVal ) );
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




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

#include <excform.hxx>

#include <formulacell.hxx>
#include <document.hxx>
#include <scmatrix.hxx>

#include <formula/errorcodes.hxx>
#include <svl/sharedstringpool.hxx>
#include <sal/log.hxx>

#include <imp_op.hxx>
#include <namebuff.hxx>
#include <root.hxx>
#include <xltracer.hxx>
#include <xihelper.hxx>
#include <xiname.hxx>
#include <xistyle.hxx>
#include <documentimport.hxx>

using ::std::vector;

const sal_uInt16 ExcelToSc::nRowMask = 0x3FFF;

void ImportExcel::Formula25()
{
    XclAddress aXclPos;
    sal_uInt16  nXF = 0, nFormLen;
    double  fCurVal;
    sal_uInt8   nFlag0;
    bool    bShrFmla;

    aIn >> aXclPos;

    if( GetBiff() == EXC_BIFF2 )
    {//                     BIFF2
        aIn.Ignore( 3 );

        fCurVal = aIn.ReadDouble();
        aIn.Ignore( 1 );
        nFormLen = aIn.ReaduInt8();
        bShrFmla = false;
    }
    else
    {//                     BIFF5
        nXF = aIn.ReaduInt16();
        fCurVal = aIn.ReadDouble();
        nFlag0 = aIn.ReaduInt8();
        aIn.Ignore( 5 );

        nFormLen = aIn.ReaduInt16();

        bShrFmla = nFlag0 & 0x08;   // shared or not shared
    }

    Formula( aXclPos, nXF, nFormLen, fCurVal, bShrFmla );
}

void ImportExcel::Formula3()
{
    Formula4();
}

void ImportExcel::Formula4()
{
    XclAddress aXclPos;

    aIn >> aXclPos;
    sal_uInt16 nXF = aIn.ReaduInt16();
    double fCurVal = aIn.ReadDouble();
    aIn.Ignore( 2 );
    sal_uInt16 nFormLen = aIn.ReaduInt16();

    Formula( aXclPos, nXF, nFormLen, fCurVal, false );
}

void ImportExcel::Formula(
    const XclAddress& rXclPos, sal_uInt16 nXF, sal_uInt16 nFormLen, double fCurVal, bool bShrFmla)
{
    if (!nFormLen)
        return;

    ScAddress aScPos( ScAddress::UNINITIALIZED );
    if (!GetAddressConverter().ConvertAddress(aScPos, rXclPos, GetCurrScTab(), true))
        // Conversion failed.
        return;

    // Formula will be read next, length in nFormLen
    std::unique_ptr<ScTokenArray> pResult;

    pFormConv->Reset( aScPos );
    ScDocumentImport& rDoc = GetDocImport();

    if (bShrFmla)
    {
        // This is a shared formula. Get the token array from the shared formula pool.
        SCCOL nSharedCol;
        SCROW nSharedRow;
        if (ExcelToSc::ReadSharedFormulaPosition(maStrm, nSharedCol, nSharedRow))
        {
            ScAddress aRefPos(nSharedCol, nSharedRow, GetCurrScTab());
            const ScTokenArray* pSharedCode = pFormConv->GetSharedFormula(aRefPos);
            if (pSharedCode)
            {
                ScFormulaCell* pCell;
                if (pSharedCode->NeedsWrapReference(aScPos, EXC_MAXCOL8, EXC_MAXROW8))
                {
                    pCell = new ScFormulaCell(pD, aScPos, pSharedCode->Clone());
                    pCell->GetCode()->WrapReference(aScPos, EXC_MAXCOL8, EXC_MAXROW8);
                }
                else
                    pCell = new ScFormulaCell(pD, aScPos, *pSharedCode);
                rDoc.getDoc().EnsureTable(aScPos.Tab());
                rDoc.setFormulaCell(aScPos, pCell);
                pCell->SetNeedNumberFormat(false);
                if (rtl::math::isFinite(fCurVal))
                    pCell->SetResultDouble(fCurVal);

                GetXFRangeBuffer().SetXF(aScPos, nXF);
                SetLastFormula(aScPos.Col(), aScPos.Row(), fCurVal, nXF, pCell);
            }
            else
            {
                // Shared formula not found even though it's clearly a shared formula.
                // The cell will be created in the following shared formula
                // record.
                SetLastFormula(aScPos.Col(), aScPos.Row(), fCurVal, nXF, nullptr);
            }
            return;
        }
    }

    ConvErr eErr = pFormConv->Convert( pResult, maStrm, nFormLen, true );

    ScFormulaCell* pCell = nullptr;

    if (pResult)
    {
        pCell = new ScFormulaCell(&rDoc.getDoc(), aScPos, std::move(pResult));
        pCell->GetCode()->WrapReference(aScPos, EXC_MAXCOL8, EXC_MAXROW8);
        rDoc.getDoc().CheckLinkFormulaNeedingCheck( *pCell->GetCode());
        rDoc.getDoc().EnsureTable(aScPos.Tab());
        rDoc.setFormulaCell(aScPos, pCell);
        SetLastFormula(aScPos.Col(), aScPos.Row(), fCurVal, nXF, pCell);
    }
    else
    {
        pCell = rDoc.getDoc().GetFormulaCell(aScPos);
        if (pCell)
            pCell->AddRecalcMode( ScRecalcMode::ONLOAD_ONCE );
    }

    if (pCell)
    {
        pCell->SetNeedNumberFormat(false);
        if( eErr != ConvErr::OK )
            ExcelToSc::SetError( *pCell, eErr );

        if (rtl::math::isFinite(fCurVal))
            pCell->SetResultDouble(fCurVal);
    }

    GetXFRangeBuffer().SetXF(aScPos, nXF);
}

ExcelToSc::ExcelToSc( XclImpRoot& rRoot ) :
    ExcelConverterBase(rRoot.GetDocImport().getDoc().GetSharedStringPool()),
    XclImpRoot( rRoot ),
    maFuncProv( rRoot ),
    meBiff( rRoot.GetBiff() )
{
}

ExcelToSc::~ExcelToSc()
{
}

std::unique_ptr<ScTokenArray> ExcelToSc::GetDummy()
{
    aPool.Store( OUString("Dummy()") );
    aPool >> aStack;
    return aPool.GetTokenArray( aStack.Get());
}

// if bAllowArrays is false stream seeks to first byte after <nFormulaLen>
// otherwise it will seek to the first byte after the additional content (eg
// inline arrays) following <nFormulaLen>
ConvErr ExcelToSc::Convert( std::unique_ptr<ScTokenArray>& pResult, XclImpStream& aIn, std::size_t nFormulaLen, bool bAllowArrays, const FORMULA_TYPE eFT )
{
    RootData&       rR = GetOldRoot();
    sal_uInt8           nOp, nLen;
    bool            bError = false;
    bool            bArrayFormula = false;
    TokenId         nBuf0;
    const bool      bRangeName = eFT == FT_RangeName;
    const bool      bSharedFormula = eFT == FT_SharedFormula;
    const bool      bConditional = eFT == FT_CondFormat;
    const bool      bRNorSF = bRangeName || bSharedFormula || bConditional;

    ScSingleRefData     aSRD;
    ScComplexRefData        aCRD;
    ExtensionTypeVec    aExtensions;

    if( nFormulaLen == 0 )
    {
        aPool.Store( OUString("-/-") );
        aPool >> aStack;
        pResult = aPool.GetTokenArray( aStack.Get());
        return ConvErr::OK;
    }

    std::size_t nEndPos = aIn.GetRecPos() + nFormulaLen;

    while( (aIn.GetRecPos() < nEndPos) && !bError )
    {
        nOp = aIn.ReaduInt8();

        // always reset flags
        aSRD.InitFlags();
        aCRD.InitFlags();

        switch( nOp )   //                              book page:
        {           //                                      SDK4 SDK5
            case 0x01: // Array Formula                         [325    ]
                       // Array Formula or Shared Formula       [    277]
            case 0x02: // Data Table                            [325 277]
            {
                sal_uInt16 nUINT16 = 3;

                if( meBiff != EXC_BIFF2 )
                    nUINT16++;

                aIn.Ignore( nUINT16 );

                bArrayFormula = true;
                break;
            }
            case 0x03: // Addition                              [312 264]
                aStack >> nBuf0;
                aPool <<  aStack << ocAdd << nBuf0;
                aPool >> aStack;
                break;
            case 0x04: // Subtraction                           [313 264]
                // SECOND-TOP minus TOP
                aStack >> nBuf0;
                aPool << aStack << ocSub << nBuf0;
                aPool >> aStack;
                break;
            case 0x05: // Multiplication                        [313 264]
                aStack >> nBuf0;
                aPool << aStack << ocMul << nBuf0;
                aPool >> aStack;
                break;
            case 0x06: // Division                              [313 264]
                // divide TOP by SECOND-TOP
                aStack >> nBuf0;
                aPool << aStack << ocDiv << nBuf0;
                aPool >> aStack;
                break;
            case 0x07: // Exponentiation                            [313 265]
                // raise SECOND-TOP to power of TOP
                aStack >> nBuf0;
                aPool << aStack << ocPow << nBuf0;
                aPool >> aStack;
                break;
            case 0x08: // Concatenation                         [313 265]
                // append TOP to SECOND-TOP
                aStack >> nBuf0;
                aPool << aStack << ocAmpersand << nBuf0;
                aPool >> aStack;
                break;
            case 0x09: // Less Than                             [313 265]
                // SECOND-TOP < TOP
                aStack >> nBuf0;
                aPool << aStack << ocLess << nBuf0;
                aPool >> aStack;
                break;
            case 0x0A: // Less Than or Equal                    [313 265]
                // SECOND-TOP <= TOP
                aStack >> nBuf0;
                aPool << aStack << ocLessEqual << nBuf0;
                aPool >> aStack;
                break;
            case 0x0B: // Equal                                 [313 265]
                // SECOND-TOP == TOP
                aStack >> nBuf0;
                aPool << aStack << ocEqual << nBuf0;
                aPool >> aStack;
                break;
            case 0x0C: // Greater Than or Equal                 [313 265]
                // SECOND-TOP >= TOP
                aStack >> nBuf0;
                aPool << aStack << ocGreaterEqual << nBuf0;
                aPool >> aStack;
                break;
            case 0x0D: // Greater Than                          [313 265]
                // SECOND-TOP > TOP
                aStack >> nBuf0;
                aPool << aStack << ocGreater << nBuf0;
                aPool >> aStack;
                break;
            case 0x0E: // Not Equal                             [313 265]
                // SECOND-TOP != TOP
                aStack >> nBuf0;
                aPool << aStack << ocNotEqual << nBuf0;
                aPool >> aStack;
                break;
            case 0x0F: // Intersection                          [314 265]
                aStack >> nBuf0;
                aPool << aStack << ocIntersect << nBuf0;
                aPool >> aStack;
                break;
            case 0x10: // Union                                 [314 265]
                // ocSep instead of 'ocUnion'
                aStack >> nBuf0;
                aPool << aStack << ocSep << nBuf0;
                    // doesn't fit exactly, but is more Excel-like
                aPool >> aStack;
                break;
            case 0x11: // Range                                 [314 265]
                aStack >> nBuf0;
                aPool << aStack << ocRange << nBuf0;
                aPool >> aStack;
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
            {
                nLen = aIn.ReaduInt8();
                OUString aString = aIn.ReadRawByteString( nLen );

                aStack << aPool.Store( aString );
                break;
            }
            case 0x19: // Special Attribute                     [327 279]
            {
                sal_uInt16  nData(0), nFactor(0);
                sal_uInt8   nOpt(0);

                nOpt = aIn.ReaduInt8();

                if( meBiff == EXC_BIFF2 )
                {
                    nData = aIn.ReaduInt8();
                    nFactor = 1;
                }
                else
                {
                    nData = aIn.ReaduInt16();
                    nFactor = 2;
                }

                if( nOpt & 0x04 )
                {
                    // nFactor -> skip bytes or words    AttrChoose
                    ++nData;
                    aIn.Ignore(static_cast<std::size_t>(nData) * nFactor);
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
                        SAL_INFO( "sc", "-ExcelToSc::Convert(): 0x1A does not exist in Biff5!" );
                        [[fallthrough]];
                    default:
                        SAL_INFO( "sc", "-ExcelToSc::Convert(): A little oblivious?" );
                }
                break;
            case 0x1B: // End External Reference                [330    ]
                switch( meBiff )
                {
                    case EXC_BIFF2: aIn.Ignore( 3 );    break;
                    case EXC_BIFF3:
                    case EXC_BIFF4: aIn.Ignore( 4 );    break;
                    case EXC_BIFF5:
                        SAL_INFO( "sc", "-ExcelToSc::Convert(): 0x1B does not exist in Biff5!" );
                        [[fallthrough]];
                    default:
                        SAL_INFO( "sc", "-ExcelToSc::Convert(): A little oblivious?" );
                }
                break;
            case 0x1C: // Error Value                           [314 266]
            {
                sal_uInt8 nByte = aIn.ReaduInt8();
                DefTokenId          eOc;
                switch( nByte )
                {
                    case EXC_ERR_NULL:
                    case EXC_ERR_DIV0:
                    case EXC_ERR_VALUE:
                    case EXC_ERR_REF:
                    case EXC_ERR_NAME:
                    case EXC_ERR_NUM:   eOc = ocStop;       break;
                    case EXC_ERR_NA:    eOc = ocNotAvail;   break;
                    default:            eOc = ocNoName;
                }
                aPool << eOc;
                if( eOc != ocStop )
                    aPool << ocOpen << ocClose;
                aPool >> aStack;
                break;
            }
            case 0x1D: // Boolean                               [315 266]
            {
                sal_uInt8 nByte = aIn.ReaduInt8();
                if( nByte == 0 )
                    aPool << ocFalse << ocOpen << ocClose;
                else
                    aPool << ocTrue << ocOpen << ocClose;
                aPool >> aStack;
                break;
            }
            case 0x1E: // Integer                               [315 266]
            {
                sal_uInt16 nUINT16 = aIn.ReaduInt16();
                aStack << aPool.Store( static_cast<double>(nUINT16) );
                break;
            }
            case 0x1F: // Number                                [315 266]
            {
                double fDouble = aIn.ReadDouble();
                aStack << aPool.Store( fDouble );
                break;
            }
            case 0x40:
            case 0x60:
            case 0x20: // Array Constant                        [317 268]
            {
                aIn.Ignore( (meBiff == EXC_BIFF2) ? 6 : 7 );
                if( bAllowArrays )
                {
                    aStack << aPool.StoreMatrix();
                    aExtensions.push_back( EXTENSION_ARRAY );
                }
                else
                {
                    aPool << ocBad;
                    aPool >> aStack;
                }
                break;
            }
            case 0x41:
            case 0x61:
            case 0x21: // Function, Fixed Number of Arguments   [333 282]
            {
                sal_uInt16 nXclFunc;
                if( meBiff <= EXC_BIFF3 )
                    nXclFunc = aIn.ReaduInt8();
                else
                    nXclFunc = aIn.ReaduInt16();
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
                nParamCount = aIn.ReaduInt8();
                nParamCount &= 0x7F;
                if( meBiff <= EXC_BIFF3 )
                    nXclFunc = aIn.ReaduInt8();
                else
                    nXclFunc = aIn.ReaduInt16();
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
                sal_uInt16 nUINT16 = aIn.ReaduInt16();
                switch( meBiff )
                {
                    case EXC_BIFF2: aIn.Ignore( 5 );    break;
                    case EXC_BIFF3:
                    case EXC_BIFF4: aIn.Ignore( 8 );    break;
                    case EXC_BIFF5: aIn.Ignore( 12 );   break;
                    default:
                        OSL_FAIL(
                        "-ExcelToSc::Convert(): A little oblivious?" );
                }
                const XclImpName* pName = GetNameManager().GetName( nUINT16 );
                if(pName && !pName->GetScRangeData())
                    aStack << aPool.Store( ocMacro, pName->GetXclName() );
                else
                    aStack << aPool.StoreName(nUINT16, -1);
            }
                break;
            case 0x44:
            case 0x64:
            case 0x24: // Cell Reference                        [319 270]
            case 0x4A:
            case 0x6A:
            case 0x2A: // Deleted Cell Reference                [323 273]
            {
                sal_uInt16 nUINT16 = aIn.ReaduInt16();
                sal_uInt8 nByte = aIn.ReaduInt8();
                aSRD.SetAbsCol(static_cast<SCCOL>(nByte));
                aSRD.SetAbsRow(nUINT16 & 0x3FFF);
                aSRD.SetRelTab(0);
                aSRD.SetFlag3D( bRangeName );

                ExcRelToScRel( nUINT16, nByte, aSRD, bRangeName );

                switch ( nOp )
                {
                    case 0x4A:
                    case 0x6A:
                    case 0x2A: // Deleted Cell Reference        [323 273]
                        // no information which part is deleted, set both
                        aSRD.SetColDeleted( true );
                        aSRD.SetRowDeleted( true );
                }

                aStack << aPool.Store( aSRD );
                break;
            }
            case 0x45:
            case 0x65:
            case 0x25: // Area Reference                        [320 270]
            case 0x4B:
            case 0x6B:
            case 0x2B: // Deleted Area Reference                [323 273]
            {
                sal_uInt16          nRowFirst, nRowLast;
                sal_uInt8           nColFirst, nColLast;
                ScSingleRefData&    rSRef1 = aCRD.Ref1;
                ScSingleRefData&    rSRef2 = aCRD.Ref2;

                nRowFirst = aIn.ReaduInt16();
                nRowLast = aIn.ReaduInt16();
                nColFirst = aIn.ReaduInt8();
                nColLast = aIn.ReaduInt8();

                rSRef1.SetRelTab(0);
                rSRef2.SetRelTab(0);
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
                    case 0x2B: // Deleted Area Reference        [323 273]
                        // no information which part is deleted, set all
                        rSRef1.SetColDeleted( true );
                        rSRef1.SetRowDeleted( true );
                        rSRef2.SetColDeleted( true );
                        rSRef2.SetRowDeleted( true );
                }

                aStack << aPool.Store( aCRD );
            }
                break;
            case 0x46:
            case 0x66:
            case 0x26: // Constant Reference Subexpression      [321 271]
                aExtensions.push_back( EXTENSION_MEMAREA );
                [[fallthrough]];

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
                sal_uInt16 nUINT16 = aIn.ReaduInt16();
                sal_uInt8 nByte = aIn.ReaduInt8();    // >> Attribute, Row >> Col

                aSRD.SetRelTab(0);
                aSRD.SetFlag3D( bRangeName );

                ExcRelToScRel( nUINT16, nByte, aSRD, bRNorSF );

                aStack << aPool.Store( aSRD );
                break;
            }
            case 0x4D:
            case 0x6D:
            case 0x2D: // Area Reference Within a Name          [324    ]
            {      // Area Reference Within a Shared Formula[    274]
                sal_uInt16                  nRowFirst, nRowLast;
                sal_uInt8                   nColFirst, nColLast;

                aCRD.Ref1.SetRelTab(0);
                aCRD.Ref2.SetRelTab(0);
                aCRD.Ref1.SetFlag3D( bRangeName );
                aCRD.Ref2.SetFlag3D( bRangeName );

                nRowFirst = aIn.ReaduInt16();
                nRowLast = aIn.ReaduInt16();
                nColFirst = aIn.ReaduInt8();
                nColLast = aIn.ReaduInt8(  );

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
            {
                OUString aString = "COMM_EQU_FUNC";
                sal_uInt8 nByte = aIn.ReaduInt8();
                aString += OUString::number( nByte );
                nByte = aIn.ReaduInt8();
                aStack << aPool.Store( aString );
                DoMulArgs( ocPush, nByte + 1 );
                break;
            }
            case 0x59:
            case 0x79:
            case 0x39: // Name or External Name                 [    275]
            {
                sal_Int16 nINT16 = aIn.ReadInt16();
                aIn.Ignore( 8 );
                sal_uInt16 nUINT16 = aIn.ReaduInt16();
                if( nINT16 >= 0 )
                {
                    aPool << ocBad;
                    aPool >> aStack;
                }
                else
                    aStack << aPool.StoreName( nUINT16, -1 );
                aIn.Ignore( 12 );
                break;
            }
            case 0x5A:
            case 0x7A:
            case 0x3A: // 3-D Cell Reference                    [    275]
            case 0x5C:
            case 0x7C:
            case 0x3C: // Deleted 3-D Cell Reference            [    277]
            {
                sal_uInt16          nTabFirst, nTabLast, nRow;
                sal_Int16           nExtSheet;
                sal_uInt8           nCol;

                nExtSheet = aIn.ReadInt16();
                aIn.Ignore( 8 );
                nTabFirst = aIn.ReaduInt16();
                nTabLast = aIn.ReaduInt16();
                nRow = aIn.ReaduInt16();
                nCol = aIn.ReaduInt8();

                if( nExtSheet >= 0 )
                {   // from external
                    if( rR.pExtSheetBuff->GetScTabIndex( nExtSheet, nTabLast ) )
                    {
                        nTabFirst = nTabLast;
                        nExtSheet = 0;      // found
                    }
                    else
                    {
                        aPool << ocBad;
                        aPool >> aStack;
                        nExtSheet = 1;      // don't create a SingleRef
                    }
                }

                if( nExtSheet <= 0 )
                {   // in current Workbook
                    aSRD.SetAbsTab(nTabFirst);
                    aSRD.SetFlag3D(true);

                    ExcRelToScRel( nRow, nCol, aSRD, bRangeName );

                    switch ( nOp )
                    {
                        case 0x5C:
                        case 0x7C:
                        case 0x3C: // Deleted 3-D Cell Reference    [    277]
                            // no information which part is deleted, set both
                            aSRD.SetColDeleted( true );
                            aSRD.SetRowDeleted( true );
                    }
                    if ( !ValidTab(static_cast<SCTAB>(nTabFirst)) )
                        aSRD.SetTabDeleted( true );

                    if( nTabLast != nTabFirst )
                    {
                        aCRD.Ref1 = aCRD.Ref2 = aSRD;
                        aCRD.Ref2.SetAbsTab(nTabLast);
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
                sal_uInt16      nTabFirst, nTabLast, nRowFirst, nRowLast;
                sal_Int16       nExtSheet;
                sal_uInt8       nColFirst, nColLast;

                nExtSheet = aIn.ReadInt16();
                aIn.Ignore( 8 );
                nTabFirst = aIn.ReaduInt16();
                nTabLast = aIn.ReaduInt16();
                nRowFirst = aIn.ReaduInt16();
                nRowLast = aIn.ReaduInt16();
                nColFirst = aIn.ReaduInt8();
                nColLast = aIn.ReaduInt8();

                if( nExtSheet >= 0 )
                    // from external
                {
                    if( rR.pExtSheetBuff->GetScTabIndex( nExtSheet, nTabLast ) )
                    {
                        nTabFirst = nTabLast;
                        nExtSheet = 0;      // found
                    }
                    else
                    {
                        aPool << ocBad;
                        aPool >> aStack;
                        nExtSheet = 1;      // don't create a CompleteRef
                    }
                }

                if( nExtSheet <= 0 )
                {// in current Workbook
                    // first part of range
                    ScSingleRefData&    rR1 = aCRD.Ref1;
                    ScSingleRefData&    rR2 = aCRD.Ref2;

                    rR1.SetAbsTab(nTabFirst);
                    rR2.SetAbsTab(nTabLast);
                    rR1.SetFlag3D(true);
                    rR2.SetFlag3D( nTabFirst != nTabLast );

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
                            rR1.SetColDeleted( true );
                            rR1.SetRowDeleted( true );
                            rR2.SetColDeleted( true );
                            rR2.SetRowDeleted( true );
                    }
                    if ( !ValidTab(static_cast<SCTAB>(nTabFirst)) )
                        rR1.SetTabDeleted( true );
                    if ( !ValidTab(static_cast<SCTAB>(nTabLast)) )
                        rR2.SetTabDeleted( true );

                    aStack << aPool.Store( aCRD );
                }//END in current Workbook
            }
                break;
            default: bError = true;
        }
        bError |= !aIn.IsValid();
    }

    ConvErr eRet;

    if( bError )
    {
        aPool << ocBad;
        aPool >> aStack;
        pResult = aPool.GetTokenArray( aStack.Get());
        eRet = ConvErr::Ni;
    }
    else if( aIn.GetRecPos() != nEndPos )
    {
        aPool << ocBad;
        aPool >> aStack;
        pResult = aPool.GetTokenArray( aStack.Get());
        eRet = ConvErr::Count;
    }
    else if( bArrayFormula )
    {
        pResult = nullptr;
        eRet = ConvErr::OK;
    }
    else
    {
        pResult = aPool.GetTokenArray( aStack.Get());
        eRet = ConvErr::OK;
    }

    aIn.Seek( nEndPos );

    if( eRet == ConvErr::OK )
        ReadExtensions( aExtensions, aIn );

    return eRet;
}

// stream seeks to first byte after <nFormulaLen>
ConvErr ExcelToSc::Convert( ScRangeListTabs& rRangeList, XclImpStream& aIn, std::size_t nFormulaLen,
                            SCTAB nTab, const FORMULA_TYPE eFT )
{
    RootData&       rR = GetOldRoot();
    sal_uInt8           nOp, nLen;
    bool            bError = false;
    const bool      bRangeName = eFT == FT_RangeName;
    const bool      bSharedFormula = eFT == FT_SharedFormula;
    const bool      bRNorSF = bRangeName || bSharedFormula;

    ScSingleRefData aSRD;
    ScComplexRefData    aCRD;
    aCRD.Ref1.SetAbsTab(aEingPos.Tab());
    aCRD.Ref2.SetAbsTab(aEingPos.Tab());

    if( nFormulaLen == 0 )
        return ConvErr::OK;

    std::size_t nEndPos = aIn.GetRecPos() + nFormulaLen;

    while( (aIn.GetRecPos() < nEndPos) && !bError )
    {
        nOp = aIn.ReaduInt8();
        std::size_t nIgnore = 0;

        // always reset flags
        aSRD.InitFlags();
        aCRD.InitFlags();

        switch( nOp )   //                                      book page:
        {           //                                      SDK4 SDK5
            case 0x01: // Array Formula                         [325    ]
                       // Array Formula or Shared Formula       [    277]
                nIgnore = (meBiff == EXC_BIFF2) ? 3 : 4;
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
                nLen = aIn.ReaduInt8();
                nIgnore = nLen;
                break;
            case 0x19: // Special Attribute                     [327 279]
            {
                sal_uInt16 nData(0), nFactor(0);
                sal_uInt8 nOpt(0);

                nOpt = aIn.ReaduInt8();

                if( meBiff == EXC_BIFF2 )
                {
                    nData = aIn.ReaduInt8();
                    nFactor = 1;
                }
                else
                {
                    nData = aIn.ReaduInt16();
                    nFactor = 2;
                }

                if( nOpt & 0x04 )
                {
                    // nFactor -> skip bytes or words    AttrChoose
                    ++nData;
                    aIn.Ignore(static_cast<std::size_t>(nData) * nFactor);
                }
            }
                break;
            case 0x1A: // External Reference                    [330    ]
                switch( meBiff )
                {
                    case EXC_BIFF2: nIgnore = 7;    break;
                    case EXC_BIFF3:
                    case EXC_BIFF4: nIgnore = 10;   break;
                    case EXC_BIFF5: SAL_INFO( "sc", "-ExcelToSc::Convert(): 0x1A does not exist in Biff5!" );
                                    [[fallthrough]];
                    default:        SAL_INFO( "sc", "-ExcelToSc::Convert(): A little oblivious?" );
                }
                break;
            case 0x1B: // End External Reference                [330    ]
                switch( meBiff )
                {
                    case EXC_BIFF2: nIgnore = 3;        break;
                    case EXC_BIFF3:
                    case EXC_BIFF4: nIgnore = 4;        break;
                    case EXC_BIFF5: SAL_INFO( "sc", "-ExcelToSc::Convert(): 0x1B does not exist in Biff5!" );
                                    [[fallthrough]];
                    default:        SAL_INFO( "sc", "-ExcelToSc::Convert(): A little oblivious?" );
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
                nIgnore = (meBiff == EXC_BIFF2) ? 6 : 7;
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
                    default:        OSL_FAIL( "-ExcelToSc::Convert(): A little oblivious?" );
                }
                break;
            case 0x44:
            case 0x64:
            case 0x24: // Cell Reference                        [319 270]
            {
                sal_uInt16 nUINT16 = aIn.ReaduInt16();
                sal_uInt8 nByte = aIn.ReaduInt8();
                aSRD.SetAbsCol(static_cast<SCCOL>(nByte));
                aSRD.SetAbsRow(nUINT16 & 0x3FFF);
                aSRD.SetRelTab(0);
                aSRD.SetFlag3D( bRangeName );

                ExcRelToScRel( nUINT16, nByte, aSRD, bRangeName );

                rRangeList.Append(aSRD.toAbs(aEingPos), nTab);
                break;
            }
            case 0x45:
            case 0x65:
            case 0x25: // Area Reference                        [320 270]
            {
                sal_uInt16          nRowFirst, nRowLast;
                sal_uInt8           nColFirst, nColLast;
                ScSingleRefData &rSRef1 = aCRD.Ref1;
                ScSingleRefData &rSRef2 = aCRD.Ref2;

                nRowFirst = aIn.ReaduInt16();
                nRowLast = aIn.ReaduInt16();
                nColFirst = aIn.ReaduInt8();
                nColLast = aIn.ReaduInt8();

                rSRef1.SetRelTab(0);
                rSRef2.SetRelTab(0);
                rSRef1.SetFlag3D( bRangeName );
                rSRef2.SetFlag3D( bRangeName );

                ExcRelToScRel( nRowFirst, nColFirst, aCRD.Ref1, bRangeName );
                ExcRelToScRel( nRowLast, nColLast, aCRD.Ref2, bRangeName );

                if( IsComplColRange( nColFirst, nColLast ) )
                    SetComplCol( aCRD );
                else if( IsComplRowRange( nRowFirst, nRowLast ) )
                    SetComplRow( aCRD );

                rRangeList.Append(aCRD.toAbs(aEingPos), nTab);
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
            case 0x2B: // Deleted Area Reference                [323 273]
                nIgnore = 6;
                break;
            case 0x4C:
            case 0x6C:
            case 0x2C: // Cell Reference Within a Name          [323    ]
                       // Cell Reference Within a Shared Formula[    273]
            {
                sal_uInt16 nUINT16 = aIn.ReaduInt16();
                sal_uInt8 nByte = aIn.ReaduInt8();    // >> Attribute, Row >> Col

                aSRD.SetRelTab(0);
                aSRD.SetFlag3D( bRangeName );

                ExcRelToScRel( nUINT16, nByte, aSRD, bRNorSF );

                rRangeList.Append(aSRD.toAbs(aEingPos), nTab);
                break;
            }
            case 0x4D:
            case 0x6D:
            case 0x2D: // Area Reference Within a Name          [324    ]
            {      // Area Reference Within a Shared Formula[    274]
                sal_uInt16                  nRowFirst, nRowLast;
                sal_uInt8                   nColFirst, nColLast;

                aCRD.Ref1.SetRelTab(0);
                aCRD.Ref2.SetRelTab(0);
                aCRD.Ref1.SetFlag3D( bRangeName );
                aCRD.Ref2.SetFlag3D( bRangeName );

                nRowFirst = aIn.ReaduInt16();
                nRowLast = aIn.ReaduInt16();
                nColFirst = aIn.ReaduInt8();
                nColLast = aIn.ReaduInt8();

                ExcRelToScRel( nRowFirst, nColFirst, aCRD.Ref1, bRNorSF );
                ExcRelToScRel( nRowLast, nColLast, aCRD.Ref2, bRNorSF );

                if( IsComplColRange( nColFirst, nColLast ) )
                    SetComplCol( aCRD );
                else if( IsComplRowRange( nRowFirst, nRowLast ) )
                    SetComplRow( aCRD );

                rRangeList.Append(aCRD.toAbs(aEingPos), nTab);
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
                sal_uInt16          nTabFirst, nTabLast, nRow;
                sal_Int16           nExtSheet;
                sal_uInt8           nCol;

                nExtSheet = aIn.ReadInt16();
                aIn.Ignore( 8 );
                nTabFirst = aIn.ReaduInt16();
                nTabLast = aIn.ReaduInt16();
                nRow = aIn.ReaduInt16();
                nCol = aIn.ReaduInt8();

                if( nExtSheet >= 0 )
                    // from external
                {
                    if( rR.pExtSheetBuff->GetScTabIndex( nExtSheet, nTabLast ) )
                    {
                        nTabFirst = nTabLast;
                        nExtSheet = 0;      // found
                    }
                    else
                    {
                        aPool << ocBad;
                        aPool >> aStack;
                        nExtSheet = 1;      // don't create a SingleRef
                    }
                }

                if( nExtSheet <= 0 )
                {// in current Workbook
                    bool b3D = ( static_cast<SCTAB>(nTabFirst) != aEingPos.Tab() ) || bRangeName;
                    aSRD.SetAbsTab(nTabFirst);
                    aSRD.SetFlag3D( b3D );

                    ExcRelToScRel( nRow, nCol, aSRD, bRangeName );

                    if( nTabLast != nTabFirst )
                    {
                        aCRD.Ref1 = aSRD;
                        aCRD.Ref2 = aSRD;
                        aCRD.Ref2.SetAbsTab(static_cast<SCTAB>(nTabLast));
                        b3D = ( static_cast<SCTAB>(nTabLast) != aEingPos.Tab() );
                        aCRD.Ref2.SetFlag3D( b3D );
                        rRangeList.Append(aCRD.toAbs(aEingPos), nTab);
                    }
                    else
                        rRangeList.Append(aSRD.toAbs(aEingPos), nTab);
                }
            }

                break;
            case 0x5B:
            case 0x7B:
            case 0x3B: // 3-D Area Reference                    [    276]
            {
                sal_uInt16      nTabFirst, nTabLast, nRowFirst, nRowLast;
                sal_Int16       nExtSheet;
                sal_uInt8       nColFirst, nColLast;

                nExtSheet = aIn.ReadInt16();
                aIn.Ignore( 8 );
                nTabFirst = aIn.ReaduInt16();
                nTabLast = aIn.ReaduInt16();
                nRowFirst = aIn.ReaduInt16();
                nRowLast = aIn.ReaduInt16();
                nColFirst = aIn.ReaduInt8();
                nColLast = aIn.ReaduInt8();

                if( nExtSheet >= 0 )
                    // from external
                {
                    if( rR.pExtSheetBuff->GetScTabIndex( nExtSheet, nTabLast ) )
                    {
                        nTabFirst = nTabLast;
                        nExtSheet = 0;      // found
                    }
                    else
                    {
                        aPool << ocBad;
                        aPool >> aStack;
                        nExtSheet = 1;      // don't create a CompleteRef
                    }
                }

                if( nExtSheet <= 0 )
                {// in current Workbook
                    // first part of range
                    ScSingleRefData &rR1 = aCRD.Ref1;
                    ScSingleRefData &rR2 = aCRD.Ref2;

                    rR1.SetAbsTab(nTabFirst);
                    rR2.SetAbsTab(nTabLast);
                    rR1.SetFlag3D( ( static_cast<SCTAB>(nTabFirst) != aEingPos.Tab() ) || bRangeName );
                    rR2.SetFlag3D( ( static_cast<SCTAB>(nTabLast) != aEingPos.Tab() ) || bRangeName );

                    ExcRelToScRel( nRowFirst, nColFirst, aCRD.Ref1, bRangeName );
                    ExcRelToScRel( nRowLast, nColLast, aCRD.Ref2, bRangeName );

                    if( IsComplColRange( nColFirst, nColLast ) )
                        SetComplCol( aCRD );
                    else if( IsComplRowRange( nRowFirst, nRowLast ) )
                        SetComplRow( aCRD );

                    rRangeList.Append(aCRD.toAbs(aEingPos), nTab);
                }//END in current Workbook
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
            default: bError = true;
        }
        bError |= !aIn.IsValid();

        aIn.Ignore( nIgnore );
    }

    ConvErr eRet;

    if( bError )
        eRet = ConvErr::Ni;
    else if( aIn.GetRecPos() != nEndPos )
        eRet = ConvErr::Count;
    else
        eRet = ConvErr::OK;

    aIn.Seek( nEndPos );
    return eRet;
}

void ExcelToSc::ConvertExternName( std::unique_ptr<ScTokenArray>& /*rpArray*/, XclImpStream& /*rStrm*/, std::size_t /*nFormulaLen*/,
                                      const OUString& /*rUrl*/, const vector<OUString>& /*rTabNames*/ )
{
}

void ExcelToSc::GetAbsRefs( ScRangeList& rRangeList, XclImpStream& rStrm, std::size_t nLen )
{
    OSL_ENSURE_BIFF( GetBiff() == EXC_BIFF5 );
    if( GetBiff() != EXC_BIFF5 )
        return;

    sal_uInt8 nOp;
    sal_uInt16 nRow1, nRow2;
    sal_uInt8 nCol1, nCol2;
    SCTAB nTab1, nTab2;
    sal_uInt16 nTabFirst, nTabLast;
    sal_Int16 nRefIdx;

    std::size_t nSeek;
    std::size_t nEndPos = rStrm.GetRecPos() + nLen;

    while( rStrm.IsValid() && (rStrm.GetRecPos() < nEndPos) )
    {
        nOp = rStrm.ReaduInt8();
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
                nRow1 = rStrm.ReaduInt16();
                nCol1 = rStrm.ReaduInt8();

                nRow2 = nRow1;
                nCol2 = nCol1;
                nTab1 = nTab2 = GetCurrScTab();
                goto _common;
            case 0x45:
            case 0x65:
            case 0x25: // Area Reference                        [320 270]
            case 0x4D:
            case 0x6D:
            case 0x2D: // Area Reference Within a Name          [324    ]
                       // Area Reference Within a Shared Formula[    274]
                nRow1 = rStrm.ReaduInt16();
                nRow2 = rStrm.ReaduInt16();
                nCol1 = rStrm.ReaduInt8();
                nCol2 = rStrm.ReaduInt8();

                nTab1 = nTab2 = GetCurrScTab();
                goto _common;
            case 0x5A:
            case 0x7A:
            case 0x3A: // 3-D Cell Reference                    [    275]
                nRefIdx = rStrm.ReadInt16();
                rStrm.Ignore( 8 );
                nTabFirst = rStrm.ReaduInt16();
                nTabLast = rStrm.ReaduInt16();
                nRow1 = rStrm.ReaduInt16();
                nCol1 = rStrm.ReaduInt8();

                nRow2 = nRow1;
                nCol2 = nCol1;

                goto _3d_common;
            case 0x5B:
            case 0x7B:
            case 0x3B: // 3-D Area Reference                    [    276]
                nRefIdx = rStrm.ReadInt16();
                rStrm.Ignore( 8 );
                nTabFirst = rStrm.ReaduInt16();
                nTabLast = rStrm.ReaduInt16();
                nRow1 = rStrm.ReaduInt16();
                nRow2 = rStrm.ReaduInt16();
                nCol1 = rStrm.ReaduInt8();
                nCol2 = rStrm.ReaduInt8();

    _3d_common:
                nTab1 = static_cast< SCTAB >( nTabFirst );
                nTab2 = static_cast< SCTAB >( nTabLast );

                // skip references to deleted sheets
                if( (nRefIdx >= 0) || !ValidTab( nTab1 ) || (nTab1 != nTab2) )
                    break;

                goto _common;
    _common:
                // do not check abs/rel flags, linked controls have set them!
                {
                    ScRange aScRange;
                    nRow1 &= 0x3FFF;
                    nRow2 &= 0x3FFF;
                    if( GetAddressConverter().ConvertRange( aScRange, XclRange( nCol1, nRow1, nCol2, nRow2 ), nTab1, nTab2, true ) )
                        rRangeList.push_back( aScRange );
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
            case 0x4A:
            case 0x6A:
            case 0x2A: // Deleted Cell Reference                [323 273]
                nSeek = 3;
                break;
            case 0x01: // Array Formula                         [325    ]
                       // Array Formula or Shared Formula       [    277]
            case 0x02: // Data Table                            [325 277]
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
            case 0x4B:
            case 0x6B:
            case 0x2B: // Deleted Area Reference                [323 273]
                nSeek = 6;
                break;
            case 0x40:
            case 0x60:
            case 0x20: // Array Constant                        [317 268]
                nSeek = 7;
                break;
            case 0x1F: // Number                                [315 266]
                nSeek = 8;
                break;
            case 0x43:
            case 0x63:
            case 0x23: // Name                                  [318 269]
                nSeek = 14;
                break;
            case 0x5C:
            case 0x7C:
            case 0x3C: // Deleted 3-D Cell Reference            [    277]
                nSeek = 17;
                break;
            case 0x5D:
            case 0x7D:
            case 0x3D: // Deleted 3-D Area Reference            [    277]
                nSeek = 20;
                break;
            case 0x59:
            case 0x79:
            case 0x39: // Name or External Name                 [    275]
                nSeek = 24;
                break;
            case 0x17: // String Constant                       [314 266]
                nSeek = rStrm.ReaduInt8();
                break;
            case 0x19: // Special Attribute                     [327 279]
            {
                sal_uInt8 nOpt;
                sal_uInt16 nData;
                nOpt = rStrm.ReaduInt8();
                nData = rStrm.ReaduInt16();
                if( nOpt & 0x04 )
                    nSeek = nData * 2 + 2;
            }
                break;
        }

        rStrm.Ignore( nSeek );
    }
    rStrm.Seek( nEndPos );
}

void ExcelToSc::DoMulArgs( DefTokenId eId, sal_uInt8 nCnt )
{
    TokenId                 eParam[ 256 ];
    sal_Int32               nPass;

    if( eId == ocCeil || eId == ocFloor )
    {
        aStack << aPool.Store( 1.0 );   // default, because not present in Excel
        nCnt++;
    }

    for( nPass = 0; aStack.HasMoreTokens() && (nPass < nCnt); nPass++ )
        aStack >> eParam[ nPass ];
    // #i70925# reduce parameter count, if no more tokens available on token stack
    if( nPass < nCnt )
        nCnt = static_cast< sal_uInt8 >( nPass );

    if( nCnt > 0 && eId == ocExternal )
    {
        TokenId             n = eParam[ nCnt - 1 ];
//##### ADJUST STUPIDITY FOR BASIC-FUNCS!
        if( const OUString* pExt = aPool.GetExternal( n ) )
        {
            if( const XclFunctionInfo* pFuncInfo = maFuncProv.GetFuncInfoFromXclMacroName( *pExt ) )
                aPool << pFuncInfo->meOpCode;
            else
                aPool << n;
            nCnt--;
        }
        else
            aPool << eId;
    }
    else
        aPool << eId;

    aPool << ocOpen;

    if( nCnt > 0 )
    {
        // attention: 0 = last parameter, nCnt-1 = first parameter
        sal_Int16 nSkipEnd = -1;    // skip all parameters <= nSkipEnd

        sal_Int16 nLast = nCnt - 1;

        // functions for which parameters have to be skipped
        if( eId == ocPercentrank && nCnt == 3 )
            nSkipEnd = 0;       // skip last parameter if necessary

        // Joost special cases
        else if( eId == ocIf )
        {
            sal_uInt16          nNullParam = 0;
            for( nPass = 0 ; nPass < nCnt ; nPass++ )
            {
                if( aPool.IsSingleOp( eParam[ nPass ], ocMissing ) )
                {
                    if( !nNullParam )
                        nNullParam = static_cast<sal_uInt16>(aPool.Store( 0.0 ));
                    eParam[ nPass ] = nNullParam;
                }
            }
        }

        // [Parameter{;Parameter}]
        if( nLast > nSkipEnd )
        {
            // nSkipEnd is either 0 or -1 => nLast >= 0
            aPool << eParam[ nLast ];
            for( nPass = nLast - 1 ; nPass > nSkipEnd ; nPass-- )
            {
                // nPass > nSkipEnd => nPass >= 0
                aPool << ocSep << eParam[nPass];
            }
        }
    }
    aPool << ocClose;

    aPool >> aStack;
}

void ExcelToSc::ExcRelToScRel( sal_uInt16 nRow, sal_uInt8 nCol, ScSingleRefData &rSRD, const bool bName )
{
    if( bName )
    {
        // C O L
        if( nRow & 0x4000 )
            rSRD.SetRelCol(nCol);
        else
            rSRD.SetAbsCol(nCol);

        // R O W
        if( nRow & 0x8000 )
        {//                                                         rel Row
            if( nRow & 0x2000 ) // Bit 13 set?
                // Row negative
                rSRD.SetRelRow(nRow | 0xC000);
            else
                // Row positive
                rSRD.SetRelRow(nRow & nRowMask);
        }
        else
        {//                                                         abs Row
            rSRD.SetAbsRow(nRow & nRowMask);
        }

        // T A B
        // abs needed if rel in shared formula for ScCompiler UpdateNameReference
        if ( rSRD.IsTabRel() && !rSRD.IsFlag3D() )
            rSRD.SetAbsTab(GetCurrScTab());
    }
    else
    {
        bool bColRel = (nRow & 0x4000) > 0;
        bool bRowRel = (nRow & 0x8000) > 0;

        if (bColRel)
            rSRD.SetRelCol(nCol - aEingPos.Col());
        else
            rSRD.SetAbsCol(nCol);

        rSRD.SetAbsRow(nRow & nRowMask);
        if (bRowRel)
            rSRD.SetRelRow(rSRD.Row() - aEingPos.Row());

        // T A B
        // #i10184# abs needed if rel in shared formula for ScCompiler UpdateNameReference
        if ( rSRD.IsTabRel() && !rSRD.IsFlag3D() )
            rSRD.SetAbsTab(GetCurrScTab() + rSRD.Tab());
    }
}

std::unique_ptr<ScTokenArray> ExcelToSc::GetBoolErr( XclBoolError eType )
{
    FormulaError nError;
    aPool.Reset();
    aStack.Reset();

    DefTokenId eOc;

    switch( eType )
    {
        case xlErrNull:     eOc = ocStop;       nError = FormulaError::NoCode;             break;
        case xlErrDiv0:     eOc = ocStop;       nError = FormulaError::DivisionByZero;     break;
        case xlErrValue:    eOc = ocStop;       nError = FormulaError::NoValue;            break;
        case xlErrRef:      eOc = ocStop;       nError = FormulaError::NoRef;              break;
        case xlErrName:     eOc = ocStop;       nError = FormulaError::NoName;             break;
        case xlErrNum:      eOc = ocStop;       nError = FormulaError::IllegalFPOperation; break;
        case xlErrNA:       eOc = ocNotAvail;   nError = FormulaError::NotAvailable;       break;
        case xlErrTrue:     eOc = ocTrue;       nError = FormulaError::NONE;               break;
        case xlErrFalse:    eOc = ocFalse;      nError = FormulaError::NONE;               break;
        case xlErrUnknown:  eOc = ocStop;       nError = FormulaError::UnknownState;       break;
        default:
            OSL_FAIL( "ExcelToSc::GetBoolErr - wrong enum!" );
            eOc = ocNoName;
            nError = FormulaError::UnknownState;
    }

    aPool << eOc;
    if( eOc != ocStop )
        aPool << ocOpen << ocClose;

    aPool >> aStack;

    std::unique_ptr<ScTokenArray> pResult = aPool.GetTokenArray( aStack.Get());
    if( nError != FormulaError::NONE )
        pResult->SetCodeError( nError );

    pResult->SetExclusiveRecalcModeNormal();

    return pResult;
}

bool ExcelToSc::ReadSharedFormulaPosition( XclImpStream& rStrm, SCCOL& rCol, SCROW& rRow )
{
    rStrm.PushPosition();

    sal_uInt8 nOp;
    nOp = rStrm.ReaduInt8();

    if (nOp != 0x01)   // must be PtgExp token.
    {
        rStrm.PopPosition();
        return false;
    }

    sal_uInt16 nRow, nCol;
    nRow = rStrm.ReaduInt16();
    nCol = rStrm.ReaduInt16();
    rStrm.PopPosition();
    rCol = nCol;
    rRow = nRow;
    return true;
}

const ScTokenArray* ExcelToSc::GetSharedFormula( const ScAddress& rRefPos ) const
{
    return GetOldRoot().pShrfmlaBuff->Find(rRefPos);
}

void ExcelToSc::SetError( ScFormulaCell &rCell, const ConvErr eErr )
{
    FormulaError  nInd;

    switch( eErr )
    {
        case ConvErr::Ni:         nInd = FormulaError::UnknownToken; break;
        case ConvErr::Count:      nInd = FormulaError::CodeOverflow; break;
        default:                  nInd = FormulaError::NoCode;   // I had no better idea
    }

    rCell.SetErrCode( nInd );
}

void ExcelToSc::SetComplCol( ScComplexRefData &rCRD )
{
    ScSingleRefData &rSRD = rCRD.Ref2;
    if( rSRD.IsColRel() )
        rSRD.SetRelCol(MAXCOL - aEingPos.Col());
    else
        rSRD.SetAbsCol(MAXCOL);
}

void ExcelToSc::SetComplRow( ScComplexRefData &rCRD )
{
    ScSingleRefData &rSRD = rCRD.Ref2;
    if( rSRD.IsRowRel() )
        rSRD.SetRelRow(MAXROW - aEingPos.Row());
    else
        rSRD.SetAbsRow(MAXROW);
}

void ExcelToSc::ReadExtensionArray( unsigned int n, XclImpStream& aIn )
{
    sal_uInt8 nByte = aIn.ReaduInt8();
    sal_uInt16 nUINT16 = aIn.ReaduInt16();

    SCSIZE nC, nCols;
    SCSIZE nR, nRows;
    if( GetBiff() == EXC_BIFF8 )
    {
        nCols = nByte + 1;
        nRows = nUINT16 + 1;
    }
    else
    {
        nCols = nByte ? nByte : 256;
        nRows = nUINT16;
    }

    ScMatrix* pMatrix = aPool.GetMatrix( n );

    if( nullptr != pMatrix )
    {
        pMatrix->Resize(nCols, nRows);
        pMatrix->GetDimensions( nC, nR);
        if( nC != nCols || nR != nRows )
        {
            OSL_FAIL( "ExcelToSc::ReadExtensionArray - matrix size mismatch" );
            pMatrix = nullptr;
        }
    }
    else
    {
        OSL_FAIL( "ExcelToSc::ReadExtensionArray - missing matrix" );
    }

    //assuming worst case scenario of unknown types
    const size_t nMinRecordSize = 1;
    const size_t nMaxRows = aIn.GetRecLeft() / (nMinRecordSize * nCols);
    if (nRows > nMaxRows)
    {
        SAL_WARN("sc", "Parsing error: " << nMaxRows <<
                 " max possible rows, but " << nRows << " claimed, truncating");
        nRows = nMaxRows;
    }

    svl::SharedStringPool& rPool = GetDoc().GetSharedStringPool();
    for( nR = 0 ; nR < nRows; nR++ )
    {
        for( nC = 0 ; nC < nCols; nC++ )
        {
            nByte = aIn.ReaduInt8();
            switch( nByte )
            {
                case EXC_CACHEDVAL_EMPTY:
                    aIn.Ignore( 8 );
                    if( nullptr != pMatrix )
                    {
                        pMatrix->PutEmpty( nC, nR );
                    }
                    break;

                case EXC_CACHEDVAL_DOUBLE:
                {
                    double fDouble = aIn.ReadDouble();
                    if( nullptr != pMatrix )
                    {
                        pMatrix->PutDouble( fDouble, nC, nR );
                    }
                    break;
                }
                case EXC_CACHEDVAL_STRING:
                {
                    OUString    aString;
                    if( GetBiff() == EXC_BIFF8 )
                    {
                        nUINT16 = aIn.ReaduInt16();
                        aString = aIn.ReadUniString( nUINT16 );
                    }
                    else
                    {
                        nByte = aIn.ReaduInt8();
                        aString = aIn.ReadRawByteString( nByte );
                    }
                    if( nullptr != pMatrix )
                    {
                        pMatrix->PutString(rPool.intern(aString), nC, nR);
                    }
                    break;
                }
                case EXC_CACHEDVAL_BOOL:
                    nByte = aIn.ReaduInt8();
                    aIn.Ignore( 7 );
                    if( nullptr != pMatrix )
                    {
                        pMatrix->PutBoolean( nByte != 0, nC, nR );
                    }
                    break;

                case EXC_CACHEDVAL_ERROR:
                    nByte = aIn.ReaduInt8();
                    aIn.Ignore( 7 );
                    if( nullptr != pMatrix )
                    {
                        pMatrix->PutError( XclTools::GetScErrorCode( nByte ), nC, nR );
                    }
                    break;
            }
        }
    }
}

void ExcelToSc::ReadExtensionNlr( XclImpStream& aIn )
{
    sal_uInt32 nFlags;
    nFlags = aIn.ReaduInt32();

    sal_uInt32 nCount = nFlags & EXC_TOK_NLR_ADDMASK;
    aIn.Ignore( nCount * 4 ); // Drop the cell positions
}

void ExcelToSc::ReadExtensionMemArea( XclImpStream& aIn )
{
    sal_uInt16 nCount(0);
    nCount = aIn.ReaduInt16();

    aIn.Ignore( static_cast<std::size_t>(nCount) * ((GetBiff() == EXC_BIFF8) ? 8 : 6) ); // drop the ranges
}

void ExcelToSc::ReadExtensions( const ExtensionTypeVec& rExtensions,
                                XclImpStream& aIn )
{
    unsigned int nArray = 0;

    for(int eType : rExtensions)
    {
        switch( eType )
        {
            case EXTENSION_ARRAY:
                ReadExtensionArray( nArray++, aIn );
                break;

            case EXTENSION_NLR:
                ReadExtensionNlr( aIn );
                break;

            case EXTENSION_MEMAREA:
                ReadExtensionMemArea( aIn );
                break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include "excform.hxx"

#include "formulacell.hxx"
#include "document.hxx"
#include "rangenam.hxx"
#include "xltracer.hxx"
#include "xistream.hxx"
#include "xihelper.hxx"
#include "xilink.hxx"
#include "xiname.hxx"

#include "externalrefmgr.hxx"

#include <vector>
#include <cstring>

using ::std::vector;

namespace {

/**
 * Extract a file path from OLE link path.  An OLE link path is expected to
 * be in the following format:
 *
 * Excel.Sheet.8 \3 [file path]
 */
bool extractFilePath(const OUString& rUrl, OUString& rPath)
{
    const char* prefix = "Excel.Sheet.8\3";
    size_t nPrefixLen = ::std::strlen(prefix);

    sal_Int32 n = rUrl.getLength();
    if (n <= static_cast<sal_Int32>(nPrefixLen))
        // needs to have the specified prefix.
        return false;

    OUStringBuffer aBuf;
    const sal_Unicode* p = rUrl.getStr();
    for (size_t i = 0; i < static_cast<size_t>(n); ++i, ++p)
    {
        if (i < nPrefixLen)
        {
            sal_Unicode pc = static_cast<sal_Unicode>(*prefix++);
            if (pc != *p)
                return false;

            continue;
        }
        aBuf.append(*p);
    }

    rPath = aBuf.makeStringAndClear();
    return true;
}

}

ExcelToSc8::ExternalTabInfo::ExternalTabInfo() :
    mnFileId(0), mbExternal(false)
{
}

// ============================================================================

ExcelToSc8::ExcelToSc8( const XclImpRoot& rRoot ) :
    ExcelToSc( rRoot ),
    rLinkMan( rRoot.GetLinkManager() )
{
}


ExcelToSc8::~ExcelToSc8()
{
}

bool ExcelToSc8::GetExternalFileIdFromXti( sal_uInt16 nIxti, sal_uInt16& rFileId ) const
{
    const OUString* pFileUrl = rLinkMan.GetSupbookUrl(nIxti);
    if (!pFileUrl || pFileUrl->isEmpty() || !GetDocShell())
        return false;

    String aFileUrl = ScGlobal::GetAbsDocName(*pFileUrl, GetDocShell());
    ScExternalRefManager* pRefMgr = GetDoc().GetExternalRefManager();
    rFileId = pRefMgr->getExternalFileId(aFileUrl);

    return true;
}

bool ExcelToSc8::Read3DTabReference( sal_uInt16 nIxti, SCTAB& rFirstTab, SCTAB& rLastTab, ExternalTabInfo& rExtInfo )
{
    rFirstTab = rLastTab = 0;
    rExtInfo.mbExternal = !rLinkMan.IsSelfRef(nIxti);
    bool bSuccess = rLinkMan.GetScTabRange(rFirstTab, rLastTab, nIxti);
    if (!bSuccess)
        return false;

    if (!rExtInfo.mbExternal)
        // This is internal reference.  Stop here.
        return true;

    rExtInfo.maTabName = rLinkMan.GetSupbookTabName(nIxti, rFirstTab);
    return GetExternalFileIdFromXti(nIxti, rExtInfo.mnFileId);
}

bool ExcelToSc8::HandleOleLink(sal_uInt16 nXtiIndex, const XclImpExtName& rExtName, ExternalTabInfo& rExtInfo)
{
    const OUString* pUrl = rLinkMan.GetSupbookUrl(nXtiIndex);
    if (!pUrl)
        return false;

    OUString aPath;
    if (!extractFilePath(*pUrl, aPath))
        // file path extraction failed.
        return false;

    OUString aFileUrl = ScGlobal::GetAbsDocName(aPath, GetDocShell());
    return rExtName.CreateOleData(GetDoc(), aFileUrl, rExtInfo.mnFileId, rExtInfo.maTabName, rExtInfo.maRange);
}

// if bAllowArrays is false stream seeks to first byte after <nFormulaLen>
// otherwise it will seek to the first byte past additional content after <nFormulaLen>
ConvErr ExcelToSc8::Convert( const ScTokenArray*& rpTokArray, XclImpStream& aIn, sal_Size nFormulaLen, bool bAllowArrays, const FORMULA_TYPE eFT )
{
    sal_uInt8                   nOp, nLen, nByte;
    sal_uInt16                  nUINT16;
    double                  fDouble;
    String                  aString;
    sal_Bool                    bError = false;
    sal_Bool                    bArrayFormula = false;
    TokenId                 nMerk0;
    const bool              bCondFormat = eFT == FT_CondFormat;
    const bool              bRangeName = eFT == FT_RangeName || bCondFormat;
    const bool              bSharedFormula = eFT == FT_SharedFormula;
    const bool              bRNorSF = bRangeName || bSharedFormula;

    ScSingleRefData         aSRD;
    ScComplexRefData            aCRD;
    ExtensionTypeVec        aExtensions;

    if( eStatus != ConvOK )
    {
        aIn.Ignore( nFormulaLen );
        return eStatus;
    }

    if( nFormulaLen == 0 )
    {
        aPool.Store( String( "-/-" ) );
        aPool >> aStack;
        rpTokArray = aPool[ aStack.Get() ];
        return ConvOK;
    }

    sal_Size nEndPos = aIn.GetRecPos() + nFormulaLen;

    while( (aIn.GetRecPos() < nEndPos) && !bError )
    {
        aIn >> nOp;

        // always reset flags
        aSRD.InitFlags();
        aCRD.InitFlags();

        switch( nOp )   //                              book page:
        {           //                                      SDK4 SDK5
            case 0x01: // Array Formula                         [325    ]
                       // Array Formula or Shared Formula       [    277]
            case 0x02: // Data Table                            [325 277]
                aIn.Ignore( 4 );

                bArrayFormula = sal_True;
                break;
            case 0x03: // Addition                              [312 264]
                aStack >> nMerk0;
                aPool <<  aStack << ocAdd << nMerk0;
                aPool >> aStack;
                break;
            case 0x04: // Subtraction                           [313 264]
                // SECOND-TOP minus TOP
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
            case 0x07: // Exponentiation                            [313 265]
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
                // SECOND-TOP >= TOP
                aStack >> nMerk0;
                aPool << aStack << ocGreaterEqual << nMerk0;
                aPool >> aStack;
                break;
            case 0x0D: // Greater Than                          [313 265]
                // SECOND-TOP > TOP
                aStack >> nMerk0;
                aPool << aStack << ocGreater << nMerk0;
                aPool >> aStack;
                break;
            case 0x0E: // Not Equal                             [313 265]
                // SECOND-TOP != TOP
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
                // ocSep instead of 'ocUnion'
                aStack >> nMerk0;
                aPool << aStack << ocSep << nMerk0;
                    // doesn't fit exactly, but is more Excel-like
                aPool >> aStack;
                break;
            case 0x11: // Range                                 [314 265]
                aStack >> nMerk0;
                aPool << aStack << ocRange << nMerk0;
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
                aIn >> nLen;        // und?
                aString = aIn.ReadUniString( nLen );            // reads Grbit even if nLen==0

                aStack << aPool.Store( aString );
                break;
            case 0x18:                                          // natural language formula
                {
                sal_uInt8   nEptg;
                sal_uInt16  nCol, nRow;
                aIn >> nEptg;
                switch( nEptg )
                {                           //  name        size    ext     type
                    case 0x01:              //  Lel         4       -       err
                        aIn.Ignore( 4 );
                        aPool << ocBad;
                        aPool >> aStack;
                    break;
                    case 0x02:              //  Rw          4       -       ref
                    case 0x03:              //  Col         4       -       ref
                    case 0x06:              //  RwV         4       -       val
                    case 0x07:              //  ColV        4       -       val
                    {
                        aIn >> nRow >> nCol;
                        ScAddress aAddr(static_cast<SCCOL>(nCol & 0xFF), static_cast<SCROW>(nRow), aEingPos.Tab());
                        aSRD.InitAddress(aAddr);

                        if( nEptg == 0x02 || nEptg == 0x06 )
                            aSRD.SetRowRel(true);
                        else
                            aSRD.SetColRel(true);

                        aSRD.SetAddress(aAddr, aEingPos);

                        aStack << aPool.StoreNlf( aSRD );
                    }
                    break;
                    case 0x0A:              //  Radical     13      -       ref
                    {
                        aIn >> nRow >> nCol;
                        aIn.Ignore( 9 );
                        ScAddress aAddr(static_cast<SCCOL>(nCol & 0xFF), static_cast<SCROW>(nRow), aEingPos.Tab());
                        aSRD.InitAddress(aAddr);
                        aSRD.SetColRel(true);
                        aSRD.SetAddress(aAddr, aEingPos);

                        aStack << aPool.StoreNlf( aSRD );
                    }
                    break;
                    case 0x0B:              //  RadicalS    13      x       ref
                        aIn.Ignore( 13 );
                        aExtensions.push_back( EXTENSION_NLR );
                        aPool << ocBad;
                        aPool >> aStack;
                    break;
                    case 0x0C:              //  RwS         4       x       ref
                    case 0x0D:              //  ColS        4       x       ref
                    case 0x0E:              //  RwSV        4       x       val
                    case 0x0F:              //  ColSV       4       x       val
                        aIn.Ignore( 4 );
                        aExtensions.push_back( EXTENSION_NLR );
                        aPool << ocBad;
                        aPool >> aStack;
                    break;
                    case 0x10:              //  RadicalLel  4       -       err
                    case 0x1D:              //  SxName      4       -       val
                        aIn.Ignore( 4 );
                        aPool << ocBad;
                        aPool >> aStack;
                    break;
                    default:
                        aPool << ocBad;
                        aPool >> aStack;
                }
                }
                break;
            case 0x19: // Special Attribute                     [327 279]
            {
                sal_uInt16 nData, nFakt;
                sal_uInt8 nOpt;

                aIn >> nOpt >> nData;
                nFakt = 2;

                if( nOpt & 0x04 )
                {// nFakt -> skip bytes or words    AttrChoose
                    nData++;
                    aIn.Ignore( nData * nFakt );
                }
                else if( nOpt & 0x10 )                      // AttrSum
                    DoMulArgs( ocSum, 1 );
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
                    case EXC_ERR_NA:    eOc = ocNotAvail;   break;
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
                aIn >> nByte >> nUINT16;
                aIn.Ignore( 4 );
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
            case 0x41:
            case 0x61:
            case 0x21: // Function, Fixed Number of Arguments   [333 282]
            {
                sal_uInt16 nXclFunc;
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
                aIn >> nParamCount >> nXclFunc;
                nParamCount &= 0x7F;
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
                aIn.Ignore( 2 );
                const XclImpName* pName = GetNameManager().GetName( nUINT16 );
                if (pName)
                {
                    if (pName->IsMacro())
                        // user-defined macro name.
                        aStack << aPool.Store(ocMacro, pName->GetXclName());
                    else
                        aStack << aPool.StoreName(nUINT16, pName->IsGlobal());
                }
            }
            break;
            case 0x44:
            case 0x64:
            case 0x24: // Cell Reference                        [319 270]
            case 0x4A:
            case 0x6A:
            case 0x2A: // Deleted Cell Reference                [323 273]
            {
                sal_uInt16          nCol, nRow;

                aIn >> nRow >> nCol;

                aSRD.SetRelTab(0);
                aSRD.SetFlag3D( bRangeName && !bCondFormat );

                ExcRelToScRel8( nRow, nCol, aSRD, bRangeName );

                switch ( nOp )
                {
                    case 0x4A:
                    case 0x6A:
                    case 0x2A: // Deleted Cell Reference        [323 273]
                        // no information which part is deleted, set both
                        aSRD.SetColDeleted( sal_True );
                        aSRD.SetRowDeleted( sal_True );
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
                sal_uInt16          nRowFirst, nRowLast;
                sal_uInt16          nColFirst, nColLast;
                ScSingleRefData &rSRef1 = aCRD.Ref1;
                ScSingleRefData &rSRef2 = aCRD.Ref2;

                aIn >> nRowFirst >> nRowLast >> nColFirst >> nColLast;

                rSRef1.SetRelTab(0);
                rSRef2.SetRelTab(0);
                rSRef1.SetFlag3D( bRangeName && !bCondFormat );
                rSRef2.SetFlag3D( bRangeName && !bCondFormat );

                ExcRelToScRel8( nRowFirst, nColFirst, aCRD.Ref1, bRangeName );
                ExcRelToScRel8( nRowLast, nColLast, aCRD.Ref2, bRangeName );

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
                        rSRef1.SetColDeleted( sal_True );
                        rSRef1.SetRowDeleted( sal_True );
                        rSRef2.SetColDeleted( sal_True );
                        rSRef2.SetRowDeleted( sal_True );
                }

                aStack << aPool.Store( aCRD );
            }
                break;
            case 0x46:
            case 0x66:
            case 0x26: // Constant Reference Subexpression      [321 271]
                aExtensions.push_back( EXTENSION_MEMAREA );
                aIn.Ignore( 6 );       // There isn't any more
                break;
            case 0x47:
            case 0x67:
            case 0x27: // Erroneous Constant Reference Subexpr. [322 272]
                aIn.Ignore( 6 );   // There isn't any more
                break;
            case 0x48:
            case 0x68:
            case 0x28: // Incomplete Constant Reference Subexpr.[331 281]
                aIn.Ignore( 6 );   // There isn't any more
                break;
            case 0x49:
            case 0x69:
            case 0x29: // Variable Reference Subexpression      [331 281]
                aIn.Ignore( 2 );   // There isn't any more
                break;
            case 0x4C:
            case 0x6C:
            case 0x2C: // Cell Reference Within a Name          [323    ]
                       // Cell Reference Within a Shared Formula[    273]
            {
                sal_uInt16      nRow, nCol;

                aIn >> nRow >> nCol;

                aSRD.SetRelTab(0);
                aSRD.SetFlag3D( bRangeName );

                ExcRelToScRel8( nRow, nCol, aSRD, bRNorSF );

                aStack << aPool.Store( aSRD );
            }
                break;
            case 0x4D:
            case 0x6D:
            case 0x2D: // Area Reference Within a Name          [324    ]
            {      // Area Reference Within a Shared Formula[    274]
                sal_uInt16                  nRowFirst, nRowLast;
                sal_uInt16                  nColFirst, nColLast;

                aCRD.Ref1.SetRelTab(0);
                aCRD.Ref2.SetRelTab(0);
                aCRD.Ref1.SetFlag3D( bRangeName );
                aCRD.Ref2.SetFlag3D( bRangeName );

                aIn >> nRowFirst >> nRowLast >> nColFirst >> nColLast;

                ExcRelToScRel8( nRowFirst, nColFirst, aCRD.Ref1, bRNorSF );
                ExcRelToScRel8( nRowLast, nColLast, aCRD.Ref2, bRNorSF );

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
                aIn.Ignore( 2 );   // There isn't any more
                break;
            case 0x4F:
            case 0x6F:
            case 0x2F: // Incomplete Reference Subexpression... [332 282]
                aIn.Ignore( 2 );   // There isn't any more
                break;
            case 0x58:
            case 0x78:
            case 0x38: // Command-Equivalent Function           [333    ]
                aString.AssignAscii( "COMM_EQU_FUNC" );
                aIn >> nByte;
                aString += OUString::number( nByte );
                aIn >> nByte;
                aStack << aPool.Store( aString );
                DoMulArgs( ocPush, nByte + 1 );
                break;
            case 0x59:
            case 0x79:
            case 0x39: // Name or External Name                 [    275]
            {
                sal_uInt16 nXtiIndex, nNameIdx;
                aIn >> nXtiIndex >> nNameIdx;
                aIn.Ignore( 2 );

                if( rLinkMan.IsSelfRef( nXtiIndex ) )
                {
                    // internal defined name with explicit sheet, i.e.: =Sheet1!AnyName
                    const XclImpName* pName = GetNameManager().GetName( nNameIdx );
                    if (pName)
                    {
                        if (pName->GetScRangeData())
                            aStack << aPool.StoreName( nNameIdx, pName->IsGlobal());
                        else
                            aStack << aPool.Store(ocMacro, pName->GetXclName());
                    }
                }
                else if( const XclImpExtName* pExtName = rLinkMan.GetExternName( nXtiIndex, nNameIdx ) )
                {
                    switch( pExtName->GetType() )
                    {
                        case xlExtName:
                        {
                            /* FIXME: enable this code for #i4385# once
                             * external name reference can be stored in ODF,
                             * which remains to be done for #i3740#. Until then
                             * create a #NAME? token. */
#if 1
                            sal_uInt16 nFileId;
                            if (!GetExternalFileIdFromXti(nXtiIndex, nFileId) || !pExtName->HasFormulaTokens())
                            {
                                aStack << aPool.Store(ocNoName, pExtName->GetName());
                                break;
                            }

                            aStack << aPool.StoreExtName(nFileId, pExtName->GetName());
                            pExtName->CreateExtNameData(GetDoc(), nFileId);
#else
                            aStack << aPool.Store( ocNoName, pExtName->GetName() );
#endif
                        }
                        break;

                        case xlExtAddIn:
                        {
                            aStack << aPool.Store( ocExternal, pExtName->GetName() );
                        }
                        break;

                        case xlExtDDE:
                        {
                            String aApplic, aTopic;
                            if( rLinkMan.GetLinkData( aApplic, aTopic, nXtiIndex ) )
                            {
                                TokenId nPar1 = aPool.Store( aApplic );
                                TokenId nPar2 = aPool.Store( aTopic );
                                nMerk0 = aPool.Store( pExtName->GetName() );
                                aPool   << ocDde << ocOpen << nPar1 << ocSep << nPar2 << ocSep
                                        << nMerk0 << ocClose;
                                aPool >> aStack;
                                pExtName->CreateDdeData( GetDoc(), aApplic, aTopic );
                            }
                        }
                        break;

                        case xlExtEuroConvert:
                            {
                                aStack << aPool.Store( ocEuroConvert, String() );
                            }
                        break;
                        case xlExtOLE:
                        {
                            ExternalTabInfo aExtInfo;
                            if (HandleOleLink(nXtiIndex, *pExtName, aExtInfo))
                            {
                                if (aExtInfo.maRange.aStart == aExtInfo.maRange.aEnd)
                                {
                                    // single cell
                                    aSRD.InitAddress(aExtInfo.maRange.aStart);
                                    aStack << aPool.StoreExtRef(aExtInfo.mnFileId, aExtInfo.maTabName, aSRD);
                                }
                                else
                                {
                                    // range
                                    aCRD.InitRange(aExtInfo.maRange);
                                    aStack << aPool.StoreExtRef(aExtInfo.mnFileId, aExtInfo.maTabName, aCRD);
                                }
                            }
                            else
                                aStack << aPool.Store(ocNoName, pExtName->GetName());
                        }
                        break;
                        default:
                        {
                            aPool << ocBad;
                            aPool >> aStack;
                        }
                    }
                }
                else
                {
                    aPool << ocBad;
                    aPool >> aStack;
                }
            }
                break;
            case 0x5A:
            case 0x7A:
            case 0x3A: // 3-D Cell Reference                    [    275]
            case 0x5C:
            case 0x7C:
            case 0x3C: // Deleted 3-D Cell Reference            [    277]
            {
                sal_uInt16 nIxti, nRw, nGrbitCol;
                SCTAB nTabFirst, nTabLast;

                aIn >> nIxti >> nRw >> nGrbitCol;

                ExternalTabInfo aExtInfo;
                if (!Read3DTabReference(nIxti, nTabFirst, nTabLast, aExtInfo))
                {
                    aPool << ocBad;
                    aPool >> aStack;
                    break;
                }

                aSRD.SetAbsTab(nTabFirst);
                aSRD.SetFlag3D(true);

                ExcRelToScRel8( nRw, nGrbitCol, aSRD, bRangeName );

                switch ( nOp )
                {
                    case 0x5C:
                    case 0x7C:
                    case 0x3C: // Deleted 3-D Cell Reference    [    277]
                        // no information which part is deleted, set both
                        aSRD.SetColDeleted( sal_True );
                        aSRD.SetRowDeleted( sal_True );
                }

                if (aExtInfo.mbExternal)
                {
                    // nTabFirst and nTabLast are the indices of the refernced
                    // sheets in the SUPBOOK record, hence do not represent
                    // the actual indices of the original sheets since the
                    // SUPBOOK record only stores referenced sheets and skips
                    // the ones that are not referenced.

                    if (nTabLast != nTabFirst)
                    {
                        aCRD.Ref1 = aCRD.Ref2 = aSRD;
                        aCRD.Ref2.SetAbsTab(nTabLast);
                        aStack << aPool.StoreExtRef(aExtInfo.mnFileId, aExtInfo.maTabName, aCRD);
                    }
                    else
                        aStack << aPool.StoreExtRef(aExtInfo.mnFileId, aExtInfo.maTabName, aSRD);
                }
                else
                {
                    if ( !ValidTab(nTabFirst))
                        aSRD.SetTabDeleted( sal_True );

                    if( nTabLast != nTabFirst )
                    {
                        aCRD.Ref1 = aCRD.Ref2 = aSRD;
                        aCRD.Ref2.SetAbsTab(nTabLast);
                        aCRD.Ref2.SetTabDeleted( !ValidTab(nTabLast) );
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
                sal_uInt16 nIxti, nRw1, nGrbitCol1, nRw2, nGrbitCol2;
                SCTAB nTabFirst, nTabLast;
                aIn >> nIxti >> nRw1 >> nRw2 >> nGrbitCol1 >> nGrbitCol2;

                ExternalTabInfo aExtInfo;
                if (!Read3DTabReference(nIxti, nTabFirst, nTabLast, aExtInfo))
                {
                    aPool << ocBad;
                    aPool >> aStack;
                    break;
                }
                ScSingleRefData &rR1 = aCRD.Ref1;
                ScSingleRefData &rR2 = aCRD.Ref2;


                rR1.SetAbsTab(nTabFirst);
                rR2.SetAbsTab(nTabLast);
                rR1.SetFlag3D(true);
                rR2.SetFlag3D( nTabFirst != nTabLast );

                ExcRelToScRel8( nRw1, nGrbitCol1, aCRD.Ref1, bRangeName );
                ExcRelToScRel8( nRw2, nGrbitCol2, aCRD.Ref2, bRangeName );

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
                        rR1.SetColDeleted( sal_True );
                        rR1.SetRowDeleted( sal_True );
                        rR2.SetColDeleted( sal_True );
                        rR2.SetRowDeleted( sal_True );
                }

                if (aExtInfo.mbExternal)
                {
                    aStack << aPool.StoreExtRef(aExtInfo.mnFileId, aExtInfo.maTabName, aCRD);
                }
                else
                {
                    if ( !ValidTab(nTabFirst) )
                        rR1.SetTabDeleted( sal_True );
                    if ( !ValidTab(nTabLast) )
                        rR2.SetTabDeleted( sal_True );

                    aStack << aPool.Store( aCRD );
                }
            }
                break;
            default: bError = sal_True;
        }
        bError |= !aIn.IsValid();
    }

    ConvErr eRet;

    if( bError )
    {
        aPool << ocBad;
        aPool >> aStack;
        rpTokArray = aPool[ aStack.Get() ];
        eRet = ConvErrNi;
    }
    else if( aIn.GetRecPos() != nEndPos )
    {
        aPool << ocBad;
        aPool >> aStack;
        rpTokArray = aPool[ aStack.Get() ];
        eRet = ConvErrCount;
    }
    else if( bArrayFormula )
    {
        rpTokArray = NULL;
        eRet = ConvOK;
    }
    else
    {
        rpTokArray = aPool[ aStack.Get() ];
        eRet = ConvOK;
    }

    aIn.Seek( nEndPos );

    if( eRet == ConvOK)
        ReadExtensions( aExtensions, aIn );

    return eRet;
}


// stream seeks to first byte after <nFormulaLen>
ConvErr ExcelToSc8::Convert( _ScRangeListTabs& rRangeList, XclImpStream& aIn, sal_Size nFormulaLen,
                              SCsTAB nTab, const FORMULA_TYPE eFT )
{
    sal_uInt8                   nOp, nLen;
    sal_Bool                    bError = false;
    const bool              bCondFormat = eFT == FT_CondFormat;
    const bool              bRangeName = eFT == FT_RangeName || bCondFormat;
    const bool              bSharedFormula = eFT == FT_SharedFormula;
    const bool              bRNorSF = bRangeName || bSharedFormula;

    ScSingleRefData         aSRD;
    ScComplexRefData            aCRD;

    bExternName = false;

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

        // always reset flags
        aSRD.InitFlags();
        aCRD.InitFlags();

        switch( nOp )   //                              book page:
        {           //                                      SDK4 SDK5
            case 0x01: // Array Formula                         [325    ]
                       // Array Formula or Shared Formula       [    277]
                aIn.Ignore( 4 );
                break;
            case 0x02: // Data Table                            [325 277]
                aIn.Ignore( 4 );
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
                aIn >> nLen;        // und?

                aIn.IgnoreUniString( nLen );        // reads Grbit even if nLen==0
                break;
            case 0x19: // Special Attribute                     [327 279]
            {
                sal_uInt16 nData, nFakt;
                sal_uInt8 nOpt;

                aIn >> nOpt >> nData;
                nFakt = 2;

                if( nOpt & 0x04 )
                {// nFakt -> skip bytes or words    AttrChoose
                    nData++;
                    aIn.Ignore( nData * nFakt );
                }
            }
                break;
            case 0x1C: // Error Value                           [314 266]
            case 0x1D: // Boolean                               [315 266]
                aIn.Ignore( 1 );
                break;
            case 0x1E: // Integer                               [315 266]
                aIn.Ignore( 2 );
                break;
            case 0x1F: // Number                                [315 266]
                aIn.Ignore( 8 );
                break;
            case 0x40:
            case 0x60:
            case 0x20: // Array Constant                        [317 268]
                aIn.Ignore( 7 );
                break;
            case 0x41:
            case 0x61:
            case 0x21: // Function, Fixed Number of Arguments   [333 282]
                aIn.Ignore( 2 );
                break;
            case 0x42:
            case 0x62:
            case 0x22: // Function, Variable Number of Arg.     [333 283]
                aIn.Ignore( 3 );
                break;
            case 0x43:
            case 0x63:
            case 0x23: // Name                                  [318 269]
                aIn.Ignore( 4 );
                break;
            case 0x44:
            case 0x64:
            case 0x24: // Cell Reference                        [319 270]
            {
                sal_uInt16          nCol, nRow;

                aIn >> nRow >> nCol;

                aSRD.SetRelTab(0);
                aSRD.SetFlag3D( bRangeName && !bCondFormat );

                ExcRelToScRel8( nRow, nCol, aSRD, bRangeName );

                rRangeList.Append(aSRD.toAbs(aEingPos), nTab);
            }
                break;
            case 0x45:
            case 0x65:
            case 0x25: // Area Reference                        [320 270]
            {
                sal_uInt16          nRowFirst, nRowLast;
                sal_uInt16          nColFirst, nColLast;
                ScSingleRefData &rSRef1 = aCRD.Ref1;
                ScSingleRefData &rSRef2 = aCRD.Ref2;

                aIn >> nRowFirst >> nRowLast >> nColFirst >> nColLast;

                rSRef1.SetRelTab(0);
                rSRef2.SetRelTab(0);
                rSRef1.SetFlag3D( bRangeName && !bCondFormat );
                rSRef2.SetFlag3D( bRangeName && !bCondFormat );

                ExcRelToScRel8( nRowFirst, nColFirst, aCRD.Ref1, bRangeName );
                ExcRelToScRel8( nRowLast, nColLast, aCRD.Ref2, bRangeName );

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
                aIn.Ignore( 6 );   // There isn't any more
                break;
            case 0x49:
            case 0x69:
            case 0x29: // Variable Reference Subexpression      [331 281]
                aIn.Ignore( 2 );   // There isn't any more
                break;
            case 0x4A:
            case 0x6A:
            case 0x2A: // Deleted Cell Reference                [323 273]
                aIn.Ignore( 3 );
                break;
            case 0x4B:
            case 0x6B:
            case 0x2B: // Deleted Area Refernce                 [323 273]
                aIn.Ignore( 6 );
                break;
            case 0x4C:
            case 0x6C:
            case 0x2C: // Cell Reference Within a Name          [323    ]
                       // Cell Reference Within a Shared Formula[    273]
            {
                sal_uInt16      nRow, nCol;

                aIn >> nRow >> nCol;

                aSRD.SetRelTab(0);
                aSRD.SetFlag3D( bRangeName );

                ExcRelToScRel8( nRow, nCol, aSRD, bRNorSF );

                rRangeList.Append(aSRD.toAbs(aEingPos), nTab);
            }
                break;
            case 0x4D:
            case 0x6D:
            case 0x2D: // Area Reference Within a Name          [324    ]
            {      // Area Reference Within a Shared Formula[    274]
                sal_uInt16                  nRowFirst, nRowLast;
                sal_uInt16                  nColFirst, nColLast;

                aCRD.Ref1.SetRelTab(0);
                aCRD.Ref2.SetRelTab(0);
                aCRD.Ref1.SetFlag3D( bRangeName );
                aCRD.Ref2.SetFlag3D( bRangeName );

                aIn >> nRowFirst >> nRowLast >> nColFirst >> nColLast;

                ExcRelToScRel8( nRowFirst, nColFirst, aCRD.Ref1, bRNorSF );
                ExcRelToScRel8( nRowLast, nColLast, aCRD.Ref2, bRNorSF );

                if( IsComplColRange( nColFirst, nColLast ) )
                    SetComplCol( aCRD );
                else if( IsComplRowRange( nRowFirst, nRowLast ) )
                    SetComplRow( aCRD );

                rRangeList.Append(aCRD.toAbs(aEingPos), nTab);
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
                aIn.Ignore( 2 );
                break;
            case 0x59:
            case 0x79:
            case 0x39: // Name or External Name                 [    275]
                aIn.Ignore( 24 );
                break;
            case 0x5A:
            case 0x7A:
            case 0x3A: // 3-D Cell Reference                    [    275]
            {
                sal_uInt16          nIxti, nRw, nGrbitCol;

                aIn >> nIxti >> nRw >> nGrbitCol;

                SCTAB nFirstScTab, nLastScTab;
                if( rLinkMan.GetScTabRange( nFirstScTab, nLastScTab, nIxti ) )
                {
                    aSRD.SetAbsTab(nFirstScTab);
                    aSRD.SetFlag3D(true);

                    ExcRelToScRel8( nRw, nGrbitCol, aSRD, bRangeName );

                    if( nFirstScTab != nLastScTab )
                    {
                        aCRD.Ref1 = aSRD;
                        aCRD.Ref2 = aSRD;
                        aCRD.Ref2.SetAbsTab(nLastScTab);
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
                sal_uInt16          nIxti, nRw1, nGrbitCol1, nRw2, nGrbitCol2;

                aIn >> nIxti >> nRw1 >> nRw2 >> nGrbitCol1 >> nGrbitCol2;

                SCTAB nFirstScTab, nLastScTab;
                if( rLinkMan.GetScTabRange( nFirstScTab, nLastScTab, nIxti ) )
                {
                    ScSingleRefData &rR1 = aCRD.Ref1;
                    ScSingleRefData &rR2 = aCRD.Ref2;

                    rR1.SetAbsTab(nFirstScTab);
                    rR2.SetAbsTab(nLastScTab);
                    rR1.SetFlag3D(true);
                    rR2.SetFlag3D( nFirstScTab != nLastScTab );

                    ExcRelToScRel8( nRw1, nGrbitCol1, aCRD.Ref1, bRangeName );
                    ExcRelToScRel8( nRw2, nGrbitCol2, aCRD.Ref2, bRangeName );

                    if( IsComplColRange( nGrbitCol1, nGrbitCol2 ) )
                        SetComplCol( aCRD );
                    else if( IsComplRowRange( nRw1, nRw2 ) )
                        SetComplRow( aCRD );

                    rRangeList.Append(aCRD.toAbs(aEingPos), nTab);
                }
            }
                break;
            case 0x5C:
            case 0x7C:
            case 0x3C: // Deleted 3-D Cell Reference            [    277]
                aIn.Ignore( 6 );
                break;
            case 0x5D:
            case 0x7D:
            case 0x3D: // Deleted 3-D Area Reference            [    277]
                aIn.Ignore( 10 );
                break;
            default:
                bError = sal_True;
        }
        bError |= !aIn.IsValid();
    }

    ConvErr eRet;

    if( bError )
        eRet = ConvErrNi;
    else if( aIn.GetRecPos() != nEndPos )
        eRet = ConvErrCount;
    else if( bExternName )
        eRet = ConvErrExternal;
    else
        eRet = ConvOK;

    aIn.Seek( nEndPos );
    return eRet;
}

ConvErr ExcelToSc8::ConvertExternName( const ScTokenArray*& rpArray, XclImpStream& rStrm, sal_Size nFormulaLen,
                                       const String& rUrl, const vector<String>& rTabNames )
{
    if( !GetDocShell() )
        return ConvErrNi;

    String aFileUrl = ScGlobal::GetAbsDocName(rUrl, GetDocShell());

    sal_uInt8               nOp, nByte;
    bool                    bError = false;

    ScSingleRefData           aSRD;
    ScComplexRefData            aCRD;

    if (eStatus != ConvOK)
    {
        rStrm.Ignore(nFormulaLen);
        return eStatus;
    }

    if (nFormulaLen == 0)
    {
        aPool.Store(String("-/-"));
        aPool >> aStack;
        rpArray = aPool[aStack.Get()];
        return ConvOK;
    }

    ScExternalRefManager* pRefMgr = GetDoc().GetExternalRefManager();
    sal_uInt16 nFileId = pRefMgr->getExternalFileId(aFileUrl);
    sal_uInt16 nTabCount = static_cast< sal_uInt16 >( rTabNames.size() );

    sal_Size nEndPos = rStrm.GetRecPos() + nFormulaLen;

    while( (rStrm.GetRecPos() < nEndPos) && !bError )
    {
        rStrm >> nOp;

        // always reset flags
        aSRD.InitFlags();
        aCRD.InitFlags();

        switch( nOp )
        {
            case 0x1C: // Error Value
            {
                rStrm >> nByte;
                DefTokenId eOc;
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
            }
            break;
            case 0x3A:
            {
                // cell reference in external range name
                sal_uInt16 nExtTab1, nExtTab2, nRow, nGrbitCol;
                rStrm >> nExtTab1 >> nExtTab2 >> nRow >> nGrbitCol;
                if (nExtTab1 >= nTabCount || nExtTab2 >= nTabCount)
                {
                    bError = true;
                    break;
                }

                aSRD.SetAbsTab(nExtTab1);
                aSRD.SetFlag3D(true);
                ExcRelToScRel8(nRow, nGrbitCol, aSRD, true);
                aCRD.Ref1 = aCRD.Ref2 = aSRD;
                String aTabName = rTabNames[nExtTab1];

                if (nExtTab1 == nExtTab2)
                {
                    // single cell reference
                    aStack << aPool.StoreExtRef(nFileId, aTabName, aSRD);
                }
                else
                {
                    // area reference
                    aCRD.Ref2.SetAbsTab(nExtTab2);
                    aStack << aPool.StoreExtRef(nFileId, aTabName, aCRD);
                }
            }
            break;
            case 0x3B:
            {
                // area reference
                sal_uInt16 nExtTab1, nExtTab2, nRow1, nRow2, nGrbitCol1, nGrbitCol2;
                rStrm >> nExtTab1 >> nExtTab2 >> nRow1 >> nRow2 >> nGrbitCol1 >> nGrbitCol2;
                ScSingleRefData& rR1 = aCRD.Ref1;
                ScSingleRefData& rR2 = aCRD.Ref2;

                rR1.SetAbsTab(nExtTab1);
                rR1.SetFlag3D(true);
                ExcRelToScRel8(nRow1, nGrbitCol1, rR1, true);

                rR2.SetAbsTab(nExtTab2);
                rR2.SetFlag3D(true);
                ExcRelToScRel8(nRow2, nGrbitCol2, rR2, true);

                String aTabName = rTabNames[nExtTab1];
                aStack << aPool.StoreExtRef(nFileId, aTabName, aCRD);
            }
            break;
            default:
                bError = true;
        }
        bError |= !rStrm.IsValid();
    }

    ConvErr eRet;

    if( bError )
    {
        aPool << ocBad;
        aPool >> aStack;
        rpArray = aPool[ aStack.Get() ];
        eRet = ConvErrNi;
    }
    else if( rStrm.GetRecPos() != nEndPos )
    {
        aPool << ocBad;
        aPool >> aStack;
        rpArray = aPool[ aStack.Get() ];
        eRet = ConvErrCount;
    }
    else
    {
        rpArray = aPool[ aStack.Get() ];
        eRet = ConvOK;
    }

    rStrm.Seek(nEndPos);
    return eRet;
}

void ExcelToSc8::ExcRelToScRel8( sal_uInt16 nRow, sal_uInt16 nC, ScSingleRefData &rSRD, const sal_Bool bName )
{
    const bool bColRel = ( nC & 0x4000 ) != 0;
    const bool bRowRel = ( nC & 0x8000 ) != 0;
    const sal_uInt8 nCol = static_cast<sal_uInt8>(nC);

    if( bName )
    {
        // C O L
        if( bColRel )
            rSRD.SetRelCol(static_cast<SCCOL>(static_cast<sal_Int8>(nC)));
        else
            rSRD.SetAbsCol(static_cast<SCCOL>(nCol));

        // R O W
        if( bRowRel )
            rSRD.SetRelRow(static_cast<sal_Int16>(nRow));
        else
            rSRD.SetAbsRow(std::min( static_cast<SCROW>(nRow), MAXROW));
    }
    else
    {
        // C O L
        if ( bColRel )
            rSRD.SetRelCol(static_cast<SCCOL>(nCol) - aEingPos.Col());
        else
            rSRD.SetAbsCol(nCol);

        // R O W
        if ( bRowRel )
            rSRD.SetRelRow(static_cast<SCROW>(nRow) - aEingPos.Row());
        else
            rSRD.SetAbsRow(nRow);
    }
}


// stream seeks to first byte after <nLen>
sal_Bool ExcelToSc8::GetAbsRefs( ScRangeList& r, XclImpStream& aIn, sal_Size nLen )
{
    sal_uInt8                   nOp;
    sal_uInt16                  nRow1, nRow2, nCol1, nCol2;
    SCTAB                                   nTab1, nTab2;
    sal_uInt16                  nIxti;

    sal_Size nSeek;

    sal_Size nEndPos = aIn.GetRecPos() + nLen;

    while( aIn.IsValid() && (aIn.GetRecPos() < nEndPos) )
    {
        aIn >> nOp;
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
                aIn >> nRow1 >> nRow2 >> nCol1 >> nCol2;

                nTab1 = nTab2 = GetCurrScTab();
                goto _common;
            case 0x5A:
            case 0x7A:
            case 0x3A: // 3-D Cell Reference                    [    275]
                aIn >> nIxti >> nRow1 >> nCol1;

                nRow2 = nRow1;
                nCol2 = nCol1;

                goto _3d_common;
            case 0x5B:
            case 0x7B:
            case 0x3B: // 3-D Area Reference                    [    276]
                aIn >> nIxti >> nRow1 >> nRow2 >> nCol1 >> nCol2;

    _3d_common:
                // skip references to deleted sheets
                if( !rLinkMan.GetScTabRange( nTab1, nTab2, nIxti ) || !ValidTab( nTab1 ) || !ValidTab( nTab2 ) )
                    break;

                goto _common;
    _common:
                // do not check abs/rel flags, linked controls have set them!
                {
                    ScRange aScRange;
                    nCol1 &= 0x3FFF;
                    nCol2 &= 0x3FFF;
                    if( GetAddressConverter().ConvertRange( aScRange, XclRange( nCol1, nRow1, nCol2, nRow2 ), nTab1, nTab2, true ) )
                        r.Append( aScRange );
                }
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
            case 0x59:
            case 0x79:
            case 0x39: // Name or External Name                 [    275]
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
            case 0x17: // String Constant                       [314 266]
            {
                sal_uInt8 nStrLen;
                aIn >> nStrLen;
                aIn.IgnoreUniString( nStrLen );     // reads Grbit even if nLen==0
                nSeek = 0;
            }
                break;
            case 0x19: // Special Attribute                     [327 279]
            {
                sal_uInt16  nData;
                sal_uInt8   nOpt;
                aIn >> nOpt >> nData;
                if( nOpt & 0x04 )
                {// nFakt -> skip bytes or words    AttrChoose
                    nData++;
                    nSeek = nData * 2;
            }
            }
                break;
        }

        aIn.Ignore( nSeek );
    }
    aIn.Seek( nEndPos );

    return !r.empty();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

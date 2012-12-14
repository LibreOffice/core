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

#include "parclass.hxx"
#include "token.hxx"
#include "global.hxx"
#include "callform.hxx"
#include "addincol.hxx"
#include "funcdesc.hxx"
#include <unotools/charclass.hxx>
#include <osl/diagnose.h>
#include <sal/macros.h>
#include <string.h>

#if OSL_DEBUG_LEVEL > 1
// the documentation thingy
#include <stdio.h>
#include <com/sun/star/sheet/FormulaLanguage.hpp>
#include <rtl/strbuf.hxx>
#include "compiler.hxx"
#include "sc.hrc"   // VAR_ARGS
#endif


/* Following assumptions are made:
 * - OpCodes not specified at all will have at least one and only parameters of
 *   type Value, no check is done on the count of parameters => no Bounds type
 *   is returned.
 * - For OpCodes with a variable number of parameters the type of the last
 *   parameter specified determines the type of all following parameters.
 */

const ScParameterClassification::RawData ScParameterClassification::pRawData[] =
{
    // IF() and CHOOSE() are somewhat special, since the ScJumpMatrix is
    // created inside those functions and ConvertMatrixParameters() is not
    // called for them.
    { ocIf,              {{ Array, Reference, Reference                          }, false }},
    { ocChose,           {{ Array, Reference                                     }, true }},
    // Other specials.
    { ocOpen,            {{ Bounds                                               }, false }},
    { ocClose,           {{ Bounds                                               }, false }},
    { ocSep,             {{ Bounds                                               }, false }},
    { ocNoName,          {{ Bounds                                               }, false }},
    { ocErrCell,         {{ Bounds                                               }, false }},
    { ocStop,            {{ Bounds                                               }, false }},
    { ocUnion,           {{ Reference, Reference                                 }, false }},
    { ocRange,           {{ Reference, Reference                                 }, false }},
    // Functions with Value parameters only but not in resource.
    { ocBackSolver,      {{ Value, Value, Value                                  }, false }},
    { ocTableOp,         {{ Value, Value, Value, Value, Value                    }, false }},
    // Operators and functions.
    { ocAdd,             {{ Array, Array                                         }, false }},
    { ocAmpersand,       {{ Array, Array                                         }, false }},
    { ocAnd,             {{ Reference                                            }, true }},
    { ocAreas,           {{ Reference                                            }, false }},
    { ocAveDev,          {{ Reference                                            }, true }},
    { ocAverage,         {{ Reference                                            }, true }},
    { ocAverageA,        {{ Reference                                            }, true }},
    { ocCell,            {{ Value, Reference                                     }, false }},
    { ocColumn,          {{ Reference                                            }, false }},
    { ocColumns,         {{ Reference                                            }, true }},
    { ocCorrel,          {{ ForceArray, ForceArray                               }, false }},
    { ocCount,           {{ Reference                                            }, true }},
    { ocCount2,          {{ Reference                                            }, true }},
    { ocCountEmptyCells, {{ Reference                                            }, false }},
    { ocCountIf,         {{ Reference, Value                                     }, false }},
    { ocCovar,           {{ ForceArray, ForceArray                               }, false }},
    { ocDBAverage,       {{ Reference, Reference, Reference                      }, false }},
    { ocDBCount,         {{ Reference, Reference, Reference                      }, false }},
    { ocDBCount2,        {{ Reference, Reference, Reference                      }, false }},
    { ocDBGet,           {{ Reference, Reference, Reference                      }, false }},
    { ocDBMax,           {{ Reference, Reference, Reference                      }, false }},
    { ocDBMin,           {{ Reference, Reference, Reference                      }, false }},
    { ocDBProduct,       {{ Reference, Reference, Reference                      }, false }},
    { ocDBStdDev,        {{ Reference, Reference, Reference                      }, false }},
    { ocDBStdDevP,       {{ Reference, Reference, Reference                      }, false }},
    { ocDBSum,           {{ Reference, Reference, Reference                      }, false }},
    { ocDBVar,           {{ Reference, Reference, Reference                      }, false }},
    { ocDBVarP,          {{ Reference, Reference, Reference                      }, false }},
    { ocDevSq,           {{ Reference                                            }, true }},
    { ocDiv,             {{ Array, Array                                         }, false }},
    { ocEqual,           {{ Array, Array                                         }, false }},
    { ocForecast,        {{ Value, ForceArray, ForceArray                        }, false }},
    { ocFrequency,       {{ Reference, Reference                                 }, false }},
    { ocFTest,           {{ ForceArray, ForceArray                               }, false }},
    { ocGeoMean,         {{ Reference                                            }, true }},
    { ocGCD,             {{ Reference                                            }, true }},
    { ocGreater,         {{ Array, Array                                         }, false }},
    { ocGreaterEqual,    {{ Array, Array                                         }, false }},
    { ocGrowth,          {{ Reference, Reference, Reference, Value               }, false }},
    { ocHarMean,         {{ Reference                                            }, true }},
    { ocHLookup,         {{ Value, Reference, Value, Value                       }, false }},
    { ocIRR,             {{ Reference, Value                                     }, false }},
    { ocIndex,           {{ Reference, Value, Value, Value                       }, false }},
    { ocIntercept,       {{ ForceArray, ForceArray                               }, false }},
    { ocIntersect,       {{ Reference, Reference                                 }, false }},
    { ocIsRef,           {{ Reference                                            }, false }},
    { ocLCM,             {{ Reference                                            }, true }},
    { ocKurt,            {{ Reference                                            }, true }},
    { ocLarge,           {{ Reference, Value                                     }, false }},
    { ocLess,            {{ Array, Array                                         }, false }},
    { ocLessEqual,       {{ Array, Array                                         }, false }},
    { ocLookup,          {{ Value, ReferenceOrForceArray, ReferenceOrForceArray  }, false }},
    { ocMatch,           {{ Value, Reference, Reference                          }, false }},
    { ocMatDet,          {{ ForceArray                                           }, false }},
    { ocMatInv,          {{ ForceArray                                           }, false }},
    { ocMatMult,         {{ ForceArray, ForceArray                               }, false }},
    { ocMatTrans,        {{ Array                                                }, false }}, // strange, but Xcl doesn't force MatTrans array
    { ocMatValue,        {{ Reference, Value, Value                              }, false }},
    { ocMax,             {{ Reference                                            }, true }},
    { ocMaxA,            {{ Reference                                            }, true }},
    { ocMedian,          {{ Reference                                            }, true }},
    { ocMin,             {{ Reference                                            }, true }},
    { ocMinA,            {{ Reference                                            }, true }},
    { ocMIRR,            {{ Reference, Value, Value                              }, false }},
    { ocModalValue,      {{ ForceArray                                           }, true }},
    { ocMul,             {{ Array, Array                                         }, false }},
    { ocMultiArea,       {{ Reference                                            }, true }},
    { ocNPV,             {{ Value, Reference                                     }, true }},
    { ocNeg,             {{ Array                                                }, false }},
    { ocNegSub,          {{ Array                                                }, false }},
    { ocNot,             {{ Array                                                }, false }},
    { ocNotEqual,        {{ Array, Array                                         }, false }},
    { ocOffset,          {{ Reference, Value, Value, Value, Value                }, false }},
    { ocOr,              {{ Reference                                            }, true }},
    { ocPearson,         {{ ForceArray, ForceArray                               }, false }},
    { ocPercentile,      {{ Reference, Value                                     }, false }},
    { ocPercentrank,     {{ Reference, Value                                     }, false }},
    { ocPow,             {{ Array, Array                                         }, false }},
    { ocPower,           {{ Array, Array                                         }, false }},
    { ocProb,            {{ ForceArray, ForceArray, Value, Value                 }, false }},
    { ocProduct,         {{ Reference                                            }, true }},
    { ocQuartile,        {{ Reference, Value                                     }, false }},
    { ocRank,            {{ Value, Reference, Value                              }, false }},
    { ocRGP,             {{ Reference, Reference, Value, Value                   }, false }},
    { ocRKP,             {{ Reference, Reference, Value, Value                   }, false }},
    { ocRow,             {{ Reference                                            }, false }},
    { ocRows,            {{ Reference                                            }, true }},
    { ocRSQ,             {{ ForceArray, ForceArray                               }, false }},
    { ocSchiefe,         {{ Reference                                            }, true }},
    { ocSlope,           {{ ForceArray, ForceArray                               }, false }},
    { ocSmall,           {{ Reference, Value                                     }, false }},
    { ocStDev,           {{ Reference                                            }, true }},
    { ocStDevA,          {{ Reference                                            }, true }},
    { ocStDevP,          {{ Reference                                            }, true }},
    { ocStDevPA,         {{ Reference                                            }, true }},
    { ocSTEYX,           {{ ForceArray, ForceArray                               }, false }},
    { ocSub,             {{ Array, Array                                         }, false }},
    { ocSubTotal,        {{ Value, Reference                                     }, true }},
    { ocSum,             {{ Reference                                            }, true }},
    { ocSumIf,           {{ Reference, Value, Reference                          }, false }},
    { ocAverageIf,       {{ Reference, Value, Reference                          }, false }},
    { ocSumProduct,      {{ ForceArray                                           }, true }},
    { ocSumSQ,           {{ Reference                                            }, true }},
    { ocSumX2MY2,        {{ ForceArray, ForceArray                               }, false }},
    { ocSumX2DY2,        {{ ForceArray, ForceArray                               }, false }},
    { ocSumXMY2,         {{ ForceArray, ForceArray                               }, false }},
    { ocTable,           {{ Reference                                            }, false }},
    { ocTables,          {{ Reference                                            }, true }},
    { ocTrend,           {{ Reference, Reference, Reference, Value               }, false }},
    { ocTrimMean,        {{ Reference, Value                                     }, false }},
    { ocTTest,           {{ ForceArray, ForceArray, Value, Value                 }, false }},
    { ocVar,             {{ Reference                                            }, true }},
    { ocVarA,            {{ Reference                                            }, true }},
    { ocVarP,            {{ Reference                                            }, true }},
    { ocVarPA,           {{ Reference                                            }, true }},
    { ocVLookup,         {{ Value, Reference, Value, Value                       }, false }},
    { ocXor,             {{ Reference                                            }, true }},
    { ocZTest,           {{ Reference, Value, Value                              }, false }},
    // Excel doubts:
    // ocN, ocT: Excel says (and handles) Reference, error? This means no
    // position dependent SingleRef if DoubleRef, and no array calculation,
    // just the upper left corner. We never did that for ocT and now also not
    // for ocN (position dependent intersection worked before but array
    // didn't). No specifics in ODFF, so the general rule applies. Gnumeric
    // does the same.
    { ocN, {{ Value }, false }},
    { ocT, {{ Value }, false }},
    // The stopper.
    { ocNone, {{ Bounds }, false } }
};

ScParameterClassification::RunData * ScParameterClassification::pData = NULL;


void ScParameterClassification::Init()
{
    if ( pData )
        return;
    pData = new RunData[ SC_OPCODE_LAST_OPCODE_ID + 1 ];
    memset( pData, 0, sizeof(RunData) * (SC_OPCODE_LAST_OPCODE_ID + 1));

    // init from specified static data above
    for ( size_t i=0; i < SAL_N_ELEMENTS(pRawData); ++i )
    {
        const RawData* pRaw = &pRawData[i];
        if ( pRaw->eOp > SC_OPCODE_LAST_OPCODE_ID )
        {
            OSL_ENSURE( pRaw->eOp == ocNone, "RawData OpCode error");
        }
        else
        {
            RunData* pRun = &pData[ pRaw->eOp ];
#if OSL_DEBUG_LEVEL > 1
            if ( pRun->aData.nParam[0] != Unknown )
            {
                OSL_TRACE( "already assigned: %d", pRaw->eOp);
            }
#endif
            memcpy( &(pRun->aData), &(pRaw->aData), sizeof(CommonData));
            // fill 0-initialized fields with real values
            if ( pRun->aData.bRepeatLast )
            {
                Type eLast = Unknown;
                for ( sal_Int32 j=0; j < CommonData::nMaxParams; ++j )
                {
                    if ( pRun->aData.nParam[j] )
                    {
                        eLast = pRun->aData.nParam[j];
                        pRun->nMinParams = sal::static_int_cast<sal_uInt8>( j+1 );
                    }
                    else
                        pRun->aData.nParam[j] = eLast;
                }
            }
            else
            {
                for ( sal_Int32 j=0; j < CommonData::nMaxParams; ++j )
                {
                    if ( !pRun->aData.nParam[j] )
                    {
                        if ( j == 0 || pRun->aData.nParam[j-1] != Bounds )
                            pRun->nMinParams = sal::static_int_cast<sal_uInt8>( j );
                        pRun->aData.nParam[j] = Bounds;
                    }
                }
                if ( !pRun->nMinParams &&
                        pRun->aData.nParam[CommonData::nMaxParams-1] != Bounds)
                    pRun->nMinParams = CommonData::nMaxParams;
            }
            for ( sal_Int32 j=0; j < CommonData::nMaxParams; ++j )
            {
                if ( pRun->aData.nParam[j] == ForceArray || pRun->aData.nParam[j] == ReferenceOrForceArray )
                {
                    pRun->bHasForceArray = true;
                    break;  // for
                }
            }
        }
    }

#if OSL_DEBUG_LEVEL > 1
    GenerateDocumentation();
#endif
}


void ScParameterClassification::Exit()
{
    delete [] pData;
    pData = NULL;
}


ScParameterClassification::Type ScParameterClassification::GetParameterType(
        const formula::FormulaToken* pToken, sal_uInt16 nParameter)
{
    OpCode eOp = pToken->GetOpCode();
    switch ( eOp )
    {
        case ocExternal:
            return GetExternalParameterType( pToken, nParameter);
        //break;
        case ocMacro:
            return Reference;
        //break;
        default:
        {
            // added to avoid warnings
        }
    }
    if ( 0 <= (short)eOp && eOp <= SC_OPCODE_LAST_OPCODE_ID )
    {
        if ( nParameter < CommonData::nMaxParams )
        {
            Type eT = pData[eOp].aData.nParam[nParameter];
            return eT == Unknown ? Value : eT;
        }
        else if ( pData[eOp].aData.bRepeatLast )
            return pData[eOp].aData.nParam[CommonData::nMaxParams-1];
        else
            return Bounds;
    }
    return Unknown;
}


ScParameterClassification::Type
ScParameterClassification::GetExternalParameterType( const formula::FormulaToken* pToken,
        sal_uInt16 nParameter)
{
    Type eRet = Unknown;
    // similar to ScInterpreter::ScExternal()
    rtl::OUString aFuncName = ScGlobal::pCharClass->uppercase( pToken->GetExternal());
    {
        const FuncData* pFuncData = ScGlobal::GetFuncCollection()->findByName(aFuncName);
        if (pFuncData)
        {
            if ( nParameter >= pFuncData->GetParamCount() )
                eRet = Bounds;
            else
            {
                switch ( pFuncData->GetParamType( nParameter) )
                {
                    case PTR_DOUBLE:
                    case PTR_STRING:
                        eRet = Value;
                    break;
                    default:
                        eRet = Reference;
                        // also array types are created using an area reference
                }
            }
            return eRet;
        }
    }

    rtl::OUString aUnoName =
        ScGlobal::GetAddInCollection()->FindFunction(aFuncName, false);

    if (!aUnoName.isEmpty())
    {
        // the relevant parts of ScUnoAddInCall without having to create one
        const ScUnoAddInFuncData* pFuncData =
            ScGlobal::GetAddInCollection()->GetFuncData( aUnoName, true );      // need fully initialized data
        if ( pFuncData )
        {
            long nCount = pFuncData->GetArgumentCount();
            if ( nCount <= 0 )
                eRet = Bounds;
            else
            {
                const ScAddInArgDesc* pArgs = pFuncData->GetArguments();
                if ( nParameter >= nCount &&
                        pArgs[nCount-1].eType == SC_ADDINARG_VARARGS )
                    eRet = Value;
                    // last arg is sequence, optional "any"s, we simply can't
                    // determine the type
                if ( eRet == Unknown )
                {
                    if ( nParameter >= nCount )
                        eRet = Bounds;
                    else
                    {
                        switch ( pArgs[nParameter].eType )
                        {
                            case SC_ADDINARG_INTEGER:
                            case SC_ADDINARG_DOUBLE:
                            case SC_ADDINARG_STRING:
                                eRet = Value;
                            break;
                            default:
                                eRet = Reference;
                        }
                    }
                }
            }
        }
    }
    return eRet;
}

//-----------------------------------------------------------------------------

#if OSL_DEBUG_LEVEL > 1

// add remaining functions, all Value parameters
void ScParameterClassification::MergeArgumentsFromFunctionResource()
{
    ScFunctionList* pFuncList = ScGlobal::GetStarCalcFunctionList();
    for ( const ScFuncDesc* pDesc = pFuncList->First(); pDesc;
            pDesc = pFuncList->Next() )
    {
        if ( pDesc->nFIndex > SC_OPCODE_LAST_OPCODE_ID ||
                pData[pDesc->nFIndex].aData.nParam[0] != Unknown )
            continue;   // not an internal opcode or already done

        RunData* pRun = &pData[ pDesc->nFIndex ];
        sal_uInt16 nArgs = pDesc->GetSuppressedArgCount();
        if ( nArgs >= VAR_ARGS )
        {
            nArgs -= VAR_ARGS - 1;
            pRun->aData.bRepeatLast = true;
        }
        if ( nArgs > CommonData::nMaxParams )
        {
            rtl::OStringBuffer aBuf;
            aBuf.append("ScParameterClassification::Init: too many arguments in listed function: ");
            aBuf.append(rtl::OUStringToOString(*(pDesc->pFuncName), RTL_TEXTENCODING_UTF8));
            aBuf.append(": ");
            aBuf.append(sal_Int32(nArgs));
            OSL_FAIL( aBuf.getStr());
            nArgs = CommonData::nMaxParams;
            pRun->aData.bRepeatLast = true;
        }
        pRun->nMinParams = static_cast< sal_uInt8 >( nArgs );
        for ( sal_Int32 j=0; j < nArgs; ++j )
        {
            pRun->aData.nParam[j] = Value;
        }
        if ( pRun->aData.bRepeatLast )
        {
            for ( sal_Int32 j = nArgs; j < CommonData::nMaxParams; ++j )
            {
                pRun->aData.nParam[j] = Value;
            }
        }
        else
        {
            for ( sal_Int32 j = nArgs; j < CommonData::nMaxParams; ++j )
            {
                pRun->aData.nParam[j] = Bounds;
            }
        }
    }
}


void ScParameterClassification::GenerateDocumentation()
{
    static const sal_Char aEnvVarName[] = "OOO_CALC_GENPARCLASSDOC";
    if ( !getenv( aEnvVarName) )
        return;
    MergeArgumentsFromFunctionResource();
    ScAddress aAddress;
    ScCompiler aComp(NULL,aAddress);
    ScCompiler::OpCodeMapPtr xMap( aComp.GetOpCodeMap(::com::sun::star::sheet::FormulaLanguage::ENGLISH));
    if (!xMap)
        return;
    fflush( stderr);
    size_t nCount = xMap->getSymbolCount();
    for ( size_t i=0; i<nCount; ++i )
    {
        OpCode eOp = OpCode(i);
        if ( xMap->getSymbol(eOp).Len() )
        {
            fprintf( stdout, "%s: ", aEnvVarName);
            rtl::OStringBuffer aStr(rtl::OUStringToOString(xMap->getSymbol(eOp), RTL_TEXTENCODING_UTF8));
            aStr.append('(');
            formula::FormulaByteToken aToken( eOp);
            sal_uInt8 nParams = GetMinimumParameters( eOp);
            // preset parameter count according to opcode value, with some
            // special handling
            if ( eOp < SC_OPCODE_STOP_DIV )
            {
                switch ( eOp )
                {
                    case ocIf:
                        aToken.SetByte(3);
                    break;
                    case ocChose:
                        aToken.SetByte(2);
                    break;
                    case ocPercentSign:
                        aToken.SetByte(1);
                    break;
                    default:;
                }
            }
            else if ( eOp < SC_OPCODE_STOP_ERRORS )
                aToken.SetByte(0);
            else if ( eOp < SC_OPCODE_STOP_BIN_OP )
            {
                switch ( eOp )
                {
                    case ocAnd:
                    case ocOr:
                        aToken.SetByte(1);  // (r1)AND(r2) --> AND( r1, ...)
                    break;
                    default:
                        aToken.SetByte(2);
                }
            }
            else if ( eOp < SC_OPCODE_STOP_UN_OP )
                aToken.SetByte(1);
            else if ( eOp < SC_OPCODE_STOP_NO_PAR )
                aToken.SetByte(0);
            else if ( eOp < SC_OPCODE_STOP_1_PAR )
                aToken.SetByte(1);
            else
                aToken.SetByte( nParams);
            // compare (this is a mere test for opcode order Div, BinOp, UnOp,
            // NoPar, 1Par, ...) and override parameter count with
            // classification
            if ( nParams != aToken.GetByte() )
                fprintf( stdout, "(parameter count differs, token Byte: %d  classification: %d) ",
                        aToken.GetByte(), nParams);
            aToken.SetByte( nParams);
            if ( nParams != aToken.GetParamCount() )
                fprintf( stdout, "(parameter count differs, token ParamCount: %d  classification: %d) ",
                        aToken.GetParamCount(), nParams);
            for ( sal_uInt16 j=0; j < nParams; ++j )
            {
                if ( j > 0 )
                    aStr.append(',');
                Type eType = GetParameterType( &aToken, j);
                switch ( eType )
                {
                    case Value :
                        aStr.append(" Value");
                    break;
                    case Reference :
                        aStr.append(" Reference");
                    break;
                    case Array :
                        aStr.append(" Array");
                    break;
                    case ForceArray :
                        aStr.append(" ForceArray");
                    break;
                    case ReferenceOrForceArray :
                        aStr.append(" ReferenceOrForceArray");
                    break;
                    case Bounds :
                        aStr.append(" (Bounds, classification error?)");
                    break;
                    default:
                        aStr.append(" (???, classification error?)");
                }
            }
            if ( HasRepeatParameters( eOp) )
                aStr.append(", ...");
            if ( nParams )
                aStr.append(' ');
            aStr.append(')');
            switch ( eOp )
            {
                case ocZGZ:
                    aStr.append("   // RRI in English resource, but ZGZ in English-only section");
                break;
                case ocMultiArea:
                    aStr.append("   // e.g. combined first parameter of INDEX() function, not a real function");
                break;
                case ocBackSolver:
                    aStr.append("   // goal seek via menu, not a real function");
                break;
                case ocTableOp:
                    aStr.append("   // MULTIPLE.OPERATIONS in English resource, but TABLE in English-only section");
                break;
                case ocNoName:
                    aStr.append("   // error function, not a real function");
                break;
                default:;
            }
            fprintf( stdout, "%s\n", aStr.getStr());
        }
    }
    fflush( stdout);
}

#endif // OSL_DEBUG_LEVEL

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

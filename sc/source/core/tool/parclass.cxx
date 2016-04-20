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

#if DEBUG_SC_PARCLASSDOC
// the documentation thingy
#include <com/sun/star/sheet/FormulaLanguage.hpp>
#include <rtl/strbuf.hxx>
#include "compiler.hxx"
#include "sc.hrc"
#endif

/* Following assumptions are made:
 * - OpCodes not specified at all will have at least one and only parameters of
 *   type Value, no check is done on the count of parameters => no Bounds type
 *   is returned.
 * - For OpCodes with a variable number of parameters the type(s) of the last
 *   repeated parameter(s) specified determine(s) the type(s) of all following
 *   parameters.
 */

const ScParameterClassification::RawData ScParameterClassification::pRawData[] =
{
    // { OpCode, {{ Type, ... }, nRepeatLast }},

    // IF() and CHOOSE() are somewhat special, since the ScJumpMatrix is
    // created inside those functions and ConvertMatrixParameters() is not
    // called for them.
    { ocIf,              {{ Array, Reference, Reference                          }, 0 }},
    { ocIfError,         {{ Array, Reference                                     }, 0 }},
    { ocIfNA,            {{ Array, Reference                                     }, 0 }},
    { ocChoose,          {{ Array, Reference                                     }, 1 }},
    // Other specials.
    { ocOpen,            {{ Bounds                                               }, 0 }},
    { ocClose,           {{ Bounds                                               }, 0 }},
    { ocSep,             {{ Bounds                                               }, 0 }},
    { ocNoName,          {{ Bounds                                               }, 0 }},
    { ocStop,            {{ Bounds                                               }, 0 }},
    { ocUnion,           {{ Reference, Reference                                 }, 0 }},
    { ocRange,           {{ Reference, Reference                                 }, 0 }},
    // Functions with Value parameters only but not in resource.
    { ocBackSolver,      {{ Value, Value, Value                                  }, 0 }},
    { ocTableOp,         {{ Value, Value, Value, Value, Value                    }, 0 }},
    // Operators and functions.
    { ocAdd,             {{ Array, Array                                         }, 0 }},
    { ocAmpersand,       {{ Array, Array                                         }, 0 }},
    { ocAnd,             {{ Reference                                            }, 1 }},
    { ocAreas,           {{ Reference                                            }, 0 }},
    { ocAveDev,          {{ Reference                                            }, 1 }},
    { ocAverage,         {{ Reference                                            }, 1 }},
    { ocAverageA,        {{ Reference                                            }, 1 }},
    { ocAverageIf,       {{ Reference, Value, Reference                          }, 0 }},
    { ocAverageIfs,      {{ Reference, Reference, Value                          }, 2 }},
    { ocCell,            {{ Value, Reference                                     }, 0 }},
    { ocColumn,          {{ Reference                                            }, 0 }},
    { ocColumns,         {{ Reference                                            }, 1 }},
    { ocCorrel,          {{ ForceArray, ForceArray                               }, 0 }},
    { ocCount,           {{ Reference                                            }, 1 }},
    { ocCount2,          {{ Reference                                            }, 1 }},
    { ocCountEmptyCells, {{ Reference                                            }, 0 }},
    { ocCountIf,         {{ Reference, Value                                     }, 0 }},
    { ocCountIfs,        {{ Reference, Value                                     }, 2 }},
    { ocCovar,           {{ ForceArray, ForceArray                               }, 0 }},
    { ocCovarianceP,     {{ ForceArray, ForceArray                               }, 0 }},
    { ocCovarianceS,     {{ ForceArray, ForceArray                               }, 0 }},
    { ocDBAverage,       {{ Reference, Reference, Reference                      }, 0 }},
    { ocDBCount,         {{ Reference, Reference, Reference                      }, 0 }},
    { ocDBCount2,        {{ Reference, Reference, Reference                      }, 0 }},
    { ocDBGet,           {{ Reference, Reference, Reference                      }, 0 }},
    { ocDBMax,           {{ Reference, Reference, Reference                      }, 0 }},
    { ocDBMin,           {{ Reference, Reference, Reference                      }, 0 }},
    { ocDBProduct,       {{ Reference, Reference, Reference                      }, 0 }},
    { ocDBStdDev,        {{ Reference, Reference, Reference                      }, 0 }},
    { ocDBStdDevP,       {{ Reference, Reference, Reference                      }, 0 }},
    { ocDBSum,           {{ Reference, Reference, Reference                      }, 0 }},
    { ocDBVar,           {{ Reference, Reference, Reference                      }, 0 }},
    { ocDBVarP,          {{ Reference, Reference, Reference                      }, 0 }},
    { ocDevSq,           {{ Reference                                            }, 1 }},
    { ocDiv,             {{ Array, Array                                         }, 0 }},
    { ocEqual,           {{ Array, Array                                         }, 0 }},
    { ocForecast,        {{ Value, ForceArray, ForceArray                        }, 0 }},
    { ocFormula,         {{ Reference                                            }, 0 }},
    { ocFrequency,       {{ Reference, Reference                                 }, 0 }},
    { ocFTest,           {{ ForceArray, ForceArray                               }, 0 }},
    { ocGeoMean,         {{ Reference                                            }, 1 }},
    { ocGCD,             {{ Reference                                            }, 1 }},
    { ocGreater,         {{ Array, Array                                         }, 0 }},
    { ocGreaterEqual,    {{ Array, Array                                         }, 0 }},
    { ocGrowth,          {{ Reference, Reference, Reference, Value               }, 0 }},
    { ocHarMean,         {{ Reference                                            }, 1 }},
    { ocHLookup,         {{ Value, ReferenceOrForceArray, Value, Value           }, 0 }},
    { ocIRR,             {{ Reference, Value                                     }, 0 }},
    { ocIndex,           {{ Reference, Value, Value, Value                       }, 0 }},
    { ocIntercept,       {{ ForceArray, ForceArray                               }, 0 }},
    { ocIntersect,       {{ Reference, Reference                                 }, 0 }},
    { ocIsRef,           {{ Reference                                            }, 0 }},
    { ocLCM,             {{ Reference                                            }, 1 }},
    { ocKurt,            {{ Reference                                            }, 1 }},
    { ocLarge,           {{ Reference, Value                                     }, 0 }},
    { ocLess,            {{ Array, Array                                         }, 0 }},
    { ocLessEqual,       {{ Array, Array                                         }, 0 }},
    { ocLookup,          {{ Value, ReferenceOrForceArray, ReferenceOrForceArray  }, 0 }},
    { ocMatch,           {{ Value, ReferenceOrForceArray, Value                  }, 0 }},
    { ocMatDet,          {{ ForceArray                                           }, 0 }},
    { ocMatInv,          {{ ForceArray                                           }, 0 }},
    { ocMatMult,         {{ ForceArray, ForceArray                               }, 0 }},
    { ocMatTrans,        {{ Array                                                }, 0 }}, // strange, but Xcl doesn't force MatTrans array
    { ocMatValue,        {{ Reference, Value, Value                              }, 0 }},
    { ocMax,             {{ Reference                                            }, 1 }},
    { ocMaxA,            {{ Reference                                            }, 1 }},
    { ocMedian,          {{ Reference                                            }, 1 }},
    { ocMin,             {{ Reference                                            }, 1 }},
    { ocMinA,            {{ Reference                                            }, 1 }},
    { ocMIRR,            {{ Reference, Value, Value                              }, 0 }},
    { ocModalValue,      {{ ForceArray                                           }, 1 }},
    { ocModalValue_MS,   {{ ForceArray                                           }, 1 }},
    { ocModalValue_Multi,{{ ForceArray                                           }, 1 }},
    { ocMul,             {{ Array, Array                                         }, 0 }},
    { ocMultiArea,       {{ Reference                                            }, 1 }},
    { ocNPV,             {{ Value, Reference                                     }, 1 }},
    { ocNeg,             {{ Array                                                }, 0 }},
    { ocNegSub,          {{ Array                                                }, 0 }},
    { ocNot,             {{ Array                                                }, 0 }},
    { ocNotEqual,        {{ Array, Array                                         }, 0 }},
    { ocOffset,          {{ Reference, Value, Value, Value, Value                }, 0 }},
    { ocOr,              {{ Reference                                            }, 1 }},
    { ocPearson,         {{ ForceArray, ForceArray                               }, 0 }},
    { ocPercentile,      {{ Reference, Value                                     }, 0 }},
    { ocPercentile_Exc,  {{ Reference, Value                                     }, 0 }},
    { ocPercentile_Inc,  {{ Reference, Value                                     }, 0 }},
    { ocPercentrank,     {{ Reference, Value, Value                              }, 0 }},
    { ocPercentrank_Exc, {{ Reference, Value, Value                              }, 0 }},
    { ocPercentrank_Inc, {{ Reference, Value, Value                              }, 0 }},
    { ocPow,             {{ Array, Array                                         }, 0 }},
    { ocPower,           {{ Array, Array                                         }, 0 }},
    { ocProb,            {{ ForceArray, ForceArray, Value, Value                 }, 0 }},
    { ocProduct,         {{ Reference                                            }, 1 }},
    { ocQuartile,        {{ Reference, Value                                     }, 0 }},
    { ocQuartile_Exc,    {{ Reference, Value                                     }, 0 }},
    { ocQuartile_Inc,    {{ Reference, Value                                     }, 0 }},
    { ocRank,            {{ Value, Reference, Value                              }, 0 }},
    { ocRank_Avg,        {{ Value, Reference, Value                              }, 0 }},
    { ocRank_Eq,         {{ Value, Reference, Value                              }, 0 }},
    { ocLinest,          {{ ForceArray, ForceArray, Value, Value                 }, 0 }},
    { ocLogest,          {{ ForceArray, ForceArray, Value, Value                 }, 0 }},
    { ocRow,             {{ Reference                                            }, 0 }},
    { ocRows,            {{ Reference                                            }, 1 }},
    { ocRSQ,             {{ ForceArray, ForceArray                               }, 0 }},
    { ocSkew,            {{ Reference                                            }, 1 }},
    { ocSkewp,           {{ Reference                                            }, 1 }},
    { ocSlope,           {{ ForceArray, ForceArray                               }, 0 }},
    { ocSmall,           {{ Reference, Value                                     }, 0 }},
    { ocStDev,           {{ Reference                                            }, 1 }},
    { ocStDevA,          {{ Reference                                            }, 1 }},
    { ocStDevP,          {{ Reference                                            }, 1 }},
    { ocStDevPA,         {{ Reference                                            }, 1 }},
    { ocStDevP_MS,       {{ Reference                                            }, 1 }},
    { ocStDevS,          {{ Reference                                            }, 1 }},
    { ocSTEYX,           {{ ForceArray, ForceArray                               }, 0 }},
    { ocSub,             {{ Array, Array                                         }, 0 }},
    { ocSubTotal,        {{ Value, Reference                                     }, 1 }},
    { ocSum,             {{ Reference                                            }, 1 }},
    { ocSumIf,           {{ Reference, Value, Reference                          }, 0 }},
    { ocSumIfs,          {{ Reference, Reference, Value                          }, 2 }},
    { ocSumProduct,      {{ ForceArray                                           }, 1 }},
    { ocSumSQ,           {{ Reference                                            }, 1 }},
    { ocSumX2MY2,        {{ ForceArray, ForceArray                               }, 0 }},
    { ocSumX2DY2,        {{ ForceArray, ForceArray                               }, 0 }},
    { ocSumXMY2,         {{ ForceArray, ForceArray                               }, 0 }},
    { ocSheet,           {{ Reference                                            }, 0 }},
    { ocSheets,          {{ Reference                                            }, 1 }},
    { ocTrend,           {{ Reference, Reference, Reference, Value               }, 0 }},
    { ocTrimMean,        {{ Reference, Value                                     }, 0 }},
    { ocTTest,           {{ ForceArray, ForceArray, Value, Value                 }, 0 }},
    { ocVar,             {{ Reference                                            }, 1 }},
    { ocVarA,            {{ Reference                                            }, 1 }},
    { ocVarP,            {{ Reference                                            }, 1 }},
    { ocVarPA,           {{ Reference                                            }, 1 }},
    { ocVarP_MS,         {{ Reference                                            }, 1 }},
    { ocVarS,            {{ Reference                                            }, 1 }},
    { ocVLookup,         {{ Value, ReferenceOrForceArray, Value, Value           }, 0 }},
    { ocXor,             {{ Reference                                            }, 1 }},
    { ocZTest,           {{ Reference, Value, Value                              }, 0 }},
    { ocZTest_MS,        {{ Reference, Value, Value                              }, 0 }},
    { ocNetWorkdays,     {{ Value, Value, Reference, Reference                   }, 0 }},
    { ocNetWorkdays_MS,  {{ Value, Value, Value, Reference                       }, 0 }},
    { ocWorkday_MS,      {{ Value, Value, Value, Reference                       }, 0 }},
    { ocAggregate,       {{ Value, Value, Reference                              }, 1 }},
    { ocForecast_ETS_ADD, {{ ForceArray, ForceArray, ForceArray, Value, Value, Value        }, 0 }},
    { ocForecast_ETS_MUL, {{ ForceArray, ForceArray, ForceArray, Value, Value, Value        }, 0 }},
    { ocForecast_ETS_PIA, {{ ForceArray, ForceArray, ForceArray, Value, Value, Value, Value }, 0 }},
    { ocForecast_ETS_PIM, {{ ForceArray, ForceArray, ForceArray, Value, Value, Value, Value }, 0 }},
    { ocForecast_ETS_SEA, {{ ForceArray, ForceArray, Value, Value                           }, 0 }},
    { ocForecast_ETS_STA, {{ ForceArray, ForceArray, ForceArray, Value, Value, Value        }, 0 }},
    { ocForecast_ETS_STM, {{ ForceArray, ForceArray, ForceArray, Value, Value, Value        }, 0 }},
    // Excel doubts:
    // ocN, ocT: Excel says (and handles) Reference, error? This means no
    // position dependent SingleRef if DoubleRef, and no array calculation,
    // just the upper left corner. We never did that for ocT and now also not
    // for ocN (position dependent intersection worked before but array
    // didn't). No specifics in ODFF, so the general rule applies. Gnumeric
    // does the same.
    { ocN, {{ Value }, 0 }},
    { ocT, {{ Value }, 0 }},
    // The stopper.
    { ocNone, {{ Bounds }, 0 } }
};

ScParameterClassification::RunData * ScParameterClassification::pData = nullptr;

void ScParameterClassification::Init()
{
    if ( pData )
        return;
    pData = new RunData[ SC_OPCODE_LAST_OPCODE_ID + 1 ];
    memset( pData, 0, sizeof(RunData) * (SC_OPCODE_LAST_OPCODE_ID + 1));

    // init from specified static data above
    for (const auto & i : pRawData)
    {
        const RawData* pRaw = &i;
        if ( pRaw->eOp > SC_OPCODE_LAST_OPCODE_ID )
        {
            OSL_ENSURE( pRaw->eOp == ocNone, "RawData OpCode error");
        }
        else
        {
            RunData* pRun = &pData[ pRaw->eOp ];
            SAL_WARN_IF(pRun->aData.nParam[0] != Unknown,  "sc.core", "already assigned: " << (int)pRaw->eOp);
            memcpy( &(pRun->aData), &(pRaw->aData), sizeof(CommonData));
            // fill 0-initialized fields with real values
            if ( pRun->aData.nRepeatLast )
            {
                for ( sal_Int32 j=0; j < CommonData::nMaxParams; ++j )
                {
                    if ( pRun->aData.nParam[j] )
                        pRun->nMinParams = sal::static_int_cast<sal_uInt8>( j+1 );
                    else if (j >= pRun->aData.nRepeatLast)
                        pRun->aData.nParam[j] =  pRun->aData.nParam[j - pRun->aData.nRepeatLast];
                    else
                    {
                        SAL_INFO(
                            "sc.core",
                            "bad classification: eOp " << +pRaw->eOp
                                << ", repeated param " << j
                                << " negative offset");
                        pRun->aData.nParam[j] =  Unknown;
                    }
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
            for (ScParameterClassification::Type & j : pRun->aData.nParam)
            {
                if ( j == ForceArray || j == ReferenceOrForceArray )
                {
                    pRun->bHasForceArray = true;
                    break;  // for
                }
            }
        }
    }

#if DEBUG_SC_PARCLASSDOC
    GenerateDocumentation();
#endif
}

void ScParameterClassification::Exit()
{
    delete [] pData;
    pData = nullptr;
}

ScParameterClassification::Type ScParameterClassification::GetParameterType(
        const formula::FormulaToken* pToken, sal_uInt16 nParameter)
{
    OpCode eOp = pToken->GetOpCode();
    switch ( eOp )
    {
        case ocExternal:
            return GetExternalParameterType( pToken, nParameter);
        case ocMacro:
            return Reference;
        default:
        {
            // added to avoid warnings
        }
    }
    if ( 0 <= (short)eOp && eOp <= SC_OPCODE_LAST_OPCODE_ID )
    {
        sal_uInt8 nRepeat;
        Type eType;
        if ( nParameter < CommonData::nMaxParams )
            eType = pData[eOp].aData.nParam[nParameter];
        else if ( (nRepeat = pData[eOp].aData.nRepeatLast) > 0 )
        {
            // The usual case is 1 repeated parameter, we don't need to
            // calculate that on each call.
            sal_uInt16 nParam = (nRepeat > 1 ?
                    (pData[eOp].nMinParams -
                     ((nParameter - pData[eOp].nMinParams) % nRepeat)) :
                    pData[eOp].nMinParams);
            return pData[eOp].aData.nParam[nParam];
        }
        else
            eType = Bounds;
        return eType == Unknown ? Value : eType;
    }
    return Unknown;
}

ScParameterClassification::Type
ScParameterClassification::GetExternalParameterType( const formula::FormulaToken* pToken,
        sal_uInt16 nParameter)
{
    Type eRet = Unknown;
    // similar to ScInterpreter::ScExternal()
    OUString aFuncName = ScGlobal::pCharClass->uppercase( pToken->GetExternal());
    {
        const LegacyFuncData* pLegacyFuncData = ScGlobal::GetLegacyFuncCollection()->findByName(aFuncName);
        if (pLegacyFuncData)
        {
            if ( nParameter >= pLegacyFuncData->GetParamCount() )
                eRet = Bounds;
            else
            {
                switch ( pLegacyFuncData->GetParamType( nParameter) )
                {
                    case ParamType::PTR_DOUBLE:
                    case ParamType::PTR_STRING:
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

    OUString aUnoName =
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

#if DEBUG_SC_PARCLASSDOC

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
        if ( nArgs >= PAIRED_VAR_ARGS )
        {
            nArgs -= PAIRED_VAR_ARGS - 2;
            pRun->aData.nRepeatLast = 2;
        }
        else if ( nArgs >= VAR_ARGS )
        {
            nArgs -= VAR_ARGS - 1;
            pRun->aData.nRepeatLast = 1;
        }
        if ( nArgs > CommonData::nMaxParams )
        {
            OStringBuffer aBuf;
            aBuf.append("ScParameterClassification::Init: too many arguments in listed function: ");
            aBuf.append(OUStringToOString(*(pDesc->pFuncName), RTL_TEXTENCODING_UTF8));
            aBuf.append(": ");
            aBuf.append(sal_Int32(nArgs));
            OSL_FAIL( aBuf.getStr());
            nArgs = CommonData::nMaxParams - 1;
            pRun->aData.nRepeatLast = 1;
        }
        pRun->nMinParams = static_cast< sal_uInt8 >( nArgs );
        for ( sal_Int32 j=0; j < nArgs; ++j )
        {
            pRun->aData.nParam[j] = Value;
        }
        if ( pRun->aData.nRepeatLast )
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
    ScCompiler::OpCodeMapPtr xMap( aComp.GetOpCodeMap(css::sheet::FormulaLanguage::ENGLISH));
    if (!xMap)
        return;
    fflush( stderr);
    size_t nCount = xMap->getSymbolCount();
    for ( size_t i=0; i<nCount; ++i )
    {
        OpCode eOp = OpCode(i);
        if ( !xMap->getSymbol(eOp).isEmpty() )
        {
            SAL_INFO("sc.core", "GenerateDocumentation, env var name: " << aEnvVarName);
            OStringBuffer aStr(OUStringToOString(xMap->getSymbol(eOp), RTL_TEXTENCODING_UTF8));
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
                    case ocIfError:
                    case ocIfNA:
                    case ocChoose:
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
                SAL_WARN("sc.core", "(parameter count differs, token Byte: " << aToken.GetByte() << " classification: " << nParams << ") ");
            aToken.SetByte( nParams);
            if ( nParams != aToken.GetParamCount() )
                SAL_WARN("sc.core", "(parameter count differs, token ParamCount: " << aToken.GetParamCount() << " classification: " << nParams << ") ");
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
                case ocRRI:
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
            SAL_INFO( "sc.core", "" << aStr.getStr() << "\n");
        }
    }
    fflush( stdout);
}

#endif // OSL_DEBUG_LEVEL

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

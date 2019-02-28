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

#include <parclass.hxx>
#include <token.hxx>
#include <global.hxx>
#include <callform.hxx>
#include <addincol.hxx>
#include <funcdesc.hxx>
#include <unotools/charclass.hxx>
#include <osl/diagnose.h>
#include <sal/macros.h>
#include <sal/log.hxx>
#include <string.h>

#if DEBUG_SC_PARCLASSDOC
// the documentation thingy
#include <com/sun/star/sheet/FormulaLanguage.hpp>
#include <rtl/strbuf.hxx>
#include <formula/funcvarargs.h>
#include "compiler.hxx"
#endif

using namespace formula;

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
    // { OpCode, {{ ParamClass, ... }, nRepeatLast, ReturnClass }},

    // IF() and CHOOSE() are somewhat special, since the ScJumpMatrix is
    // created inside those functions and ConvertMatrixParameters() is not
    // called for them.
    { ocIf,              {{ Array, Reference, Reference                          }, 0, Value }},
    { ocIfError,         {{ Array, Reference                                     }, 0, Value }},
    { ocIfNA,            {{ Array, Reference                                     }, 0, Value }},
    { ocChoose,          {{ Array, Reference                                     }, 1, Value }},
    // Other specials.
    { ocArrayClose,      {{ Bounds                                               }, 0, Bounds }},
    { ocArrayColSep,     {{ Bounds                                               }, 0, Bounds }},
    { ocArrayOpen,       {{ Bounds                                               }, 0, Bounds }},
    { ocArrayRowSep,     {{ Bounds                                               }, 0, Bounds }},
    { ocBad,             {{ Bounds                                               }, 0, Bounds }},
    { ocClose,           {{ Bounds                                               }, 0, Bounds }},
    { ocColRowName,      {{ Bounds                                               }, 0, Value }},    // or Reference?
    { ocColRowNameAuto,  {{ Bounds                                               }, 0, Value }},    // or Reference?
    { ocDBArea,          {{ Bounds                                               }, 0, Value }},    // or Reference?
    { ocMatRef,          {{ Bounds                                               }, 0, Value }},
    { ocMissing,         {{ Bounds                                               }, 0, Value }},
    { ocNoName,          {{ Bounds                                               }, 0, Bounds }},
    { ocOpen,            {{ Bounds                                               }, 0, Bounds }},
    { ocSep,             {{ Bounds                                               }, 0, Bounds }},
    { ocSkip,            {{ Bounds                                               }, 0, Bounds }},
    { ocSpaces,          {{ Bounds                                               }, 0, Bounds }},
    { ocStop,            {{ Bounds                                               }, 0, Bounds }},
    { ocStringXML,       {{ Bounds                                               }, 0, Bounds }},
    { ocTableRef,        {{ Bounds                                               }, 0, Value }},    // or Reference?
    { ocTableRefClose,   {{ Bounds                                               }, 0, Bounds }},
    { ocTableRefItemAll,     {{ Bounds                                           }, 0, Bounds }},
    { ocTableRefItemData,    {{ Bounds                                           }, 0, Bounds }},
    { ocTableRefItemHeaders, {{ Bounds                                           }, 0, Bounds }},
    { ocTableRefItemThisRow, {{ Bounds                                           }, 0, Bounds }},
    { ocTableRefItemTotals,  {{ Bounds                                           }, 0, Bounds }},
    { ocTableRefOpen,    {{ Bounds                                               }, 0, Bounds }},
    // Error constants.
    { ocErrDivZero,      {{ Bounds                                               }, 0, Bounds }},
    { ocErrNA,           {{ Bounds                                               }, 0, Bounds }},
    { ocErrName,         {{ Bounds                                               }, 0, Bounds }},
    { ocErrNull,         {{ Bounds                                               }, 0, Bounds }},
    { ocErrNum,          {{ Bounds                                               }, 0, Bounds }},
    { ocErrRef,          {{ Bounds                                               }, 0, Bounds }},
    { ocErrValue,        {{ Bounds                                               }, 0, Bounds }},
    // Functions with Value parameters only but not in resource.
    { ocBackSolver,      {{ Value, Value, Value                                  }, 0, Value }},
    { ocTableOp,         {{ Value, Value, Value, Value, Value                    }, 0, Value }},
    // Operators and functions.
    { ocAdd,             {{ Array, Array                                         }, 0, Value }},
    { ocAggregate,       {{ Value, Value, ReferenceOrRefArray                    }, 1, Value }},
    { ocAmpersand,       {{ Array, Array                                         }, 0, Value }},
    { ocAnd,             {{ Reference                                            }, 1, Value }},
    { ocAreas,           {{ Reference                                            }, 0, Value }},
    { ocAveDev,          {{ Reference                                            }, 1, Value }},
    { ocAverage,         {{ ReferenceOrRefArray                                  }, 1, Value }},
    { ocAverageA,        {{ ReferenceOrRefArray                                  }, 1, Value }},
    { ocAverageIf,       {{ ReferenceOrRefArray, Value, Reference                }, 0, Value }},
    { ocAverageIfs,      {{ ReferenceOrRefArray, ReferenceOrRefArray, Value      }, 2, Value }},
    { ocCell,            {{ Value, Reference                                     }, 0, Value }},
    { ocColumn,          {{ Reference                                            }, 0, Value }},
    { ocColumns,         {{ Reference                                            }, 1, Value }},
    { ocConcat_MS,       {{ Reference                                            }, 1, Value }},
    { ocCorrel,          {{ ForceArray, ForceArray                               }, 0, Value }},
    { ocCount,           {{ ReferenceOrRefArray                                  }, 1, Value }},
    { ocCount2,          {{ ReferenceOrRefArray                                  }, 1, Value }},
    { ocCountEmptyCells, {{ ReferenceOrRefArray                                  }, 0, Value }},
    { ocCountIf,         {{ ReferenceOrRefArray, Value                           }, 0, Value }},
    { ocCountIfs,        {{ ReferenceOrRefArray, Value                           }, 2, Value }},
    { ocCovar,           {{ ForceArray, ForceArray                               }, 0, Value }},
    { ocCovarianceP,     {{ ForceArray, ForceArray                               }, 0, Value }},
    { ocCovarianceS,     {{ ForceArray, ForceArray                               }, 0, Value }},
    { ocDBAverage,       {{ Reference, Reference, Reference                      }, 0, Value }},
    { ocDBCount,         {{ Reference, Reference, Reference                      }, 0, Value }},
    { ocDBCount2,        {{ Reference, Reference, Reference                      }, 0, Value }},
    { ocDBGet,           {{ Reference, Reference, Reference                      }, 0, Value }},
    { ocDBMax,           {{ Reference, Reference, Reference                      }, 0, Value }},
    { ocDBMin,           {{ Reference, Reference, Reference                      }, 0, Value }},
    { ocDBProduct,       {{ Reference, Reference, Reference                      }, 0, Value }},
    { ocDBStdDev,        {{ Reference, Reference, Reference                      }, 0, Value }},
    { ocDBStdDevP,       {{ Reference, Reference, Reference                      }, 0, Value }},
    { ocDBSum,           {{ Reference, Reference, Reference                      }, 0, Value }},
    { ocDBVar,           {{ Reference, Reference, Reference                      }, 0, Value }},
    { ocDBVarP,          {{ Reference, Reference, Reference                      }, 0, Value }},
    { ocDevSq,           {{ Reference                                            }, 1, Value }},
    { ocDiv,             {{ Array, Array                                         }, 0, Value }},
    { ocEqual,           {{ Array, Array                                         }, 0, Value }},
    { ocFTest,           {{ ForceArray, ForceArray                               }, 0, Value }},
    { ocForecast,        {{ Value, ForceArray, ForceArray                        }, 0, Value }},
    { ocForecast_ETS_ADD, {{ ForceArray, ForceArray, ForceArray, Value, Value, Value        }, 0, Value }},
    { ocForecast_ETS_MUL, {{ ForceArray, ForceArray, ForceArray, Value, Value, Value        }, 0, Value }},
    { ocForecast_ETS_PIA, {{ ForceArray, ForceArray, ForceArray, Value, Value, Value, Value }, 0, Value }},
    { ocForecast_ETS_PIM, {{ ForceArray, ForceArray, ForceArray, Value, Value, Value, Value }, 0, Value }},
    { ocForecast_ETS_SEA, {{ ForceArray, ForceArray, Value, Value                           }, 0, Value }},
    { ocForecast_ETS_STA, {{ ForceArray, ForceArray, ForceArray, Value, Value, Value        }, 0, Value }},
    { ocForecast_ETS_STM, {{ ForceArray, ForceArray, ForceArray, Value, Value, Value        }, 0, Value }},
    { ocFormula,         {{ Reference                                            }, 0, Value }},
    { ocFourier,         {{ ForceArray, Value, Value, Value                      }, 0, Value }},
    { ocFrequency,       {{ ReferenceOrForceArray, ReferenceOrForceArray         }, 0, ForceArrayReturn }},
    { ocGCD,             {{ Reference                                            }, 1, Value }},
    { ocGeoMean,         {{ Reference                                            }, 1, Value }},
    { ocGreater,         {{ Array, Array                                         }, 0, Value }},
    { ocGreaterEqual,    {{ Array, Array                                         }, 0, Value }},
    { ocGrowth,          {{ Reference, Reference, Reference, Value               }, 0, Value }},
    { ocHLookup,         {{ Value, ReferenceOrForceArray, Value, Value           }, 0, Value }},
    { ocHarMean,         {{ Reference                                            }, 1, Value }},
    { ocIRR,             {{ Reference, Value                                     }, 0, Value }},
    { ocIndex,           {{ Reference, Value, Value, Value                       }, 0, Value }},
    { ocIndirect,        {{ Value, Value                                         }, 0, Reference }},
    { ocIntercept,       {{ ForceArray, ForceArray                               }, 0, Value }},
    { ocIntersect,       {{ Reference, Reference                                 }, 0, Reference }},
    { ocIsFormula,       {{ Reference                                            }, 0, Value }},
    { ocIsRef,           {{ Reference                                            }, 0, Value }},
    { ocKurt,            {{ Reference                                            }, 1, Value }},
    { ocLCM,             {{ Reference                                            }, 1, Value }},
    { ocLarge,           {{ Reference, Value                                     }, 0, Value }},
    { ocLess,            {{ Array, Array                                         }, 0, Value }},
    { ocLessEqual,       {{ Array, Array                                         }, 0, Value }},
    { ocLinest,          {{ ForceArray, ForceArray, Value, Value                 }, 0, Value }},
    { ocLogest,          {{ ForceArray, ForceArray, Value, Value                 }, 0, Value }},
    { ocLookup,          {{ Value, ReferenceOrForceArray, ReferenceOrForceArray  }, 0, Value }},
    { ocMIRR,            {{ Reference, Value, Value                              }, 0, Value }},
    { ocMatDet,          {{ ForceArray                                           }, 0, Value }},
    { ocMatInv,          {{ ForceArray                                           }, 0, Value }},
    { ocMatMult,         {{ ForceArray, ForceArray                               }, 0, Value }},
    { ocMatTrans,        {{ ForceArray                                           }, 0, ForceArrayReturn }},
    { ocMatValue,        {{ Reference, Value, Value                              }, 0, Value }},
    { ocMatch,           {{ Value, ReferenceOrForceArray, Value                  }, 0, Value }},
    { ocMax,             {{ ReferenceOrRefArray                                  }, 1, Value }},
    { ocMaxA,            {{ ReferenceOrRefArray                                  }, 1, Value }},
    { ocMaxIfs_MS,       {{ ReferenceOrRefArray, ReferenceOrRefArray, Value      }, 2, Value }},
    { ocMedian,          {{ Reference                                            }, 1, Value }},
    { ocMin,             {{ ReferenceOrRefArray                                  }, 1, Value }},
    { ocMinA,            {{ ReferenceOrRefArray                                  }, 1, Value }},
    { ocMinIfs_MS,       {{ ReferenceOrRefArray, ReferenceOrRefArray, Value      }, 2, Value }},
    { ocModalValue,      {{ ForceArray                                           }, 1, Value }},
    { ocModalValue_MS,   {{ ForceArray                                           }, 1, Value }},
    { ocModalValue_Multi,{{ ForceArray                                           }, 1, Value }},
    { ocMul,             {{ Array, Array                                         }, 0, Value }},
    { ocMultiArea,       {{ Reference                                            }, 1, Reference }},
    { ocNPV,             {{ Value, Reference                                     }, 1, Value }},
    { ocNeg,             {{ Array                                                }, 0, Value }},
    { ocNegSub,          {{ Array                                                }, 0, Value }},
    { ocNetWorkdays,     {{ Value, Value, Reference, Reference                   }, 0, Value }},
    { ocNetWorkdays_MS,  {{ Value, Value, Value, Reference                       }, 0, Value }},
    { ocNot,             {{ Array                                                }, 0, Value }},
    { ocNotEqual,        {{ Array, Array                                         }, 0, Value }},
    { ocOffset,          {{ Reference, Value, Value, Value, Value                }, 0, Reference }},
    { ocOr,              {{ Reference                                            }, 1, Value }},
    { ocPearson,         {{ ForceArray, ForceArray                               }, 0, Value }},
    { ocPercentSign,     {{ Array                                                }, 0, Value }},
    { ocPercentile,      {{ Reference, Value                                     }, 0, Value }},
    { ocPercentile_Exc,  {{ Reference, Value                                     }, 0, Value }},
    { ocPercentile_Inc,  {{ Reference, Value                                     }, 0, Value }},
    { ocPercentrank,     {{ Reference, Value, Value                              }, 0, Value }},
    { ocPercentrank_Exc, {{ Reference, Value, Value                              }, 0, Value }},
    { ocPercentrank_Inc, {{ Reference, Value, Value                              }, 0, Value }},
    { ocPow,             {{ Array, Array                                         }, 0, Value }},
    { ocPower,           {{ Array, Array                                         }, 0, Value }},
    { ocProb,            {{ ForceArray, ForceArray, Value, Value                 }, 0, Value }},
    { ocProduct,         {{ ReferenceOrRefArray                                  }, 1, Value }},
    { ocQuartile,        {{ Reference, Value                                     }, 0, Value }},
    { ocQuartile_Exc,    {{ Reference, Value                                     }, 0, Value }},
    { ocQuartile_Inc,    {{ Reference, Value                                     }, 0, Value }},
    { ocRSQ,             {{ ForceArray, ForceArray                               }, 0, Value }},
    { ocRange,           {{ Reference, Reference                                 }, 0, Reference }},
    { ocRank,            {{ Value, Reference, Value                              }, 0, Value }},
    { ocRank_Avg,        {{ Value, Reference, Value                              }, 0, Value }},
    { ocRank_Eq,         {{ Value, Reference, Value                              }, 0, Value }},
    { ocRow,             {{ Reference                                            }, 0, Value }},
    { ocRows,            {{ Reference                                            }, 1, Value }},
    { ocSTEYX,           {{ ForceArray, ForceArray                               }, 0, Value }},
    { ocSheet,           {{ Reference                                            }, 0, Value }},
    { ocSheets,          {{ Reference                                            }, 1, Value }},
    { ocSkew,            {{ Reference                                            }, 1, Value }},
    { ocSkewp,           {{ Reference                                            }, 1, Value }},
    { ocSlope,           {{ ForceArray, ForceArray                               }, 0, Value }},
    { ocSmall,           {{ Reference, Value                                     }, 0, Value }},
    { ocStDev,           {{ Reference                                            }, 1, Value }},
    { ocStDevA,          {{ Reference                                            }, 1, Value }},
    { ocStDevP,          {{ Reference                                            }, 1, Value }},
    { ocStDevPA,         {{ Reference                                            }, 1, Value }},
    { ocStDevP_MS,       {{ Reference                                            }, 1, Value }},
    { ocStDevS,          {{ Reference                                            }, 1, Value }},
    { ocSub,             {{ Array, Array                                         }, 0, Value }},
    { ocSubTotal,        {{ Value, ReferenceOrRefArray                           }, 1, Value }},
    { ocSum,             {{ ReferenceOrRefArray                                  }, 1, Value }},
    { ocSumIf,           {{ ReferenceOrRefArray, Value, Reference                }, 0, Value }},
    { ocSumIfs,          {{ ReferenceOrRefArray, ReferenceOrRefArray, Value      }, 2, Value }},
    { ocSumProduct,      {{ ForceArray                                           }, 1, Value }},
    { ocSumSQ,           {{ ReferenceOrRefArray                                  }, 1, Value }},
    { ocSumX2DY2,        {{ ForceArray, ForceArray                               }, 0, Value }},
    { ocSumX2MY2,        {{ ForceArray, ForceArray                               }, 0, Value }},
    { ocSumXMY2,         {{ ForceArray, ForceArray                               }, 0, Value }},
    { ocTTest,           {{ ForceArray, ForceArray, Value, Value                 }, 0, Value }},
    { ocTextJoin_MS,     {{ Reference, Value, Reference                          }, 1, Value }},
    { ocTrend,           {{ Reference, Reference, Reference, Value               }, 0, Value }},
    { ocTrimMean,        {{ Reference, Value                                     }, 0, Value }},
    { ocUnion,           {{ Reference, Reference                                 }, 0, Reference }},
    { ocVLookup,         {{ Value, ReferenceOrForceArray, Value, Value           }, 0, Value }},
    { ocVar,             {{ ReferenceOrRefArray                                  }, 1, Value }},
    { ocVarA,            {{ ReferenceOrRefArray                                  }, 1, Value }},
    { ocVarP,            {{ ReferenceOrRefArray                                  }, 1, Value }},
    { ocVarPA,           {{ ReferenceOrRefArray                                  }, 1, Value }},
    { ocVarP_MS,         {{ Reference                                            }, 1, Value }},
    { ocVarS,            {{ Reference                                            }, 1, Value }},
    { ocWorkday_MS,      {{ Value, Value, Value, Reference                       }, 0, Value }},
    { ocXor,             {{ Reference                                            }, 1, Value }},
    { ocZTest,           {{ Reference, Value, Value                              }, 0, Value }},
    { ocZTest_MS,        {{ Reference, Value, Value                              }, 0, Value }},
    // Excel doubts:
    // ocN, ocT: Excel says (and handles) Reference, error? This means no
    // position dependent SingleRef if DoubleRef, and no array calculation,
    // just the upper left corner. We never did that for ocT and now also not
    // for ocN (position dependent intersection worked before but array
    // didn't). No specifics in ODFF, so the general rule applies. Gnumeric
    // does the same.
    { ocN, {{ Value }, 0, Value }},
    { ocT, {{ Value }, 0, Value }},
    // The stopper.
    { ocNone, {{ Bounds }, 0, Value }}
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
            SAL_WARN_IF(pRun->aData.nParam[0] != Unknown,  "sc.core", "already assigned: " << static_cast<int>(pRaw->eOp));
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
            for (formula::ParamClass & j : pRun->aData.nParam)
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

formula::ParamClass ScParameterClassification::GetParameterType(
        const formula::FormulaToken* pToken, sal_uInt16 nParameter)
{
    OpCode eOp = pToken->GetOpCode();
    switch ( eOp )
    {
        case ocExternal:
            return GetExternalParameterType( pToken, nParameter);
        case ocMacro:
            return (nParameter == SAL_MAX_UINT16 ? Value : Reference);
        default:
        {
            // added to avoid warnings
        }
    }
    if ( 0 <= static_cast<short>(eOp) && eOp <= SC_OPCODE_LAST_OPCODE_ID )
    {
        sal_uInt8 nRepeat;
        formula::ParamClass eType;
        if (nParameter == SAL_MAX_UINT16)
            eType = pData[eOp].aData.eReturn;
        else if ( nParameter < CommonData::nMaxParams )
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

formula::ParamClass ScParameterClassification::GetExternalParameterType( const formula::FormulaToken* pToken,
        sal_uInt16 nParameter)
{
    formula::ParamClass eRet = Unknown;
    if (nParameter == SAL_MAX_UINT16)
        return eRet;

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
            SAL_WARN( "sc", "ScParameterClassification::Init: too many arguments in listed function: "
                        << *(pDesc->pFuncName)
                        << ": " << nArgs );
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
            OUStringBuffer aStr(xMap->getSymbol(eOp));
            formula::FormulaByteToken aToken( eOp);
            sal_uInt8 nParams = GetMinimumParameters( eOp);
            // preset parameter count according to opcode value, with some
            // special handling
            bool bAddParentheses = true;
            if ( eOp < SC_OPCODE_STOP_DIV )
            {
                bAddParentheses = false;    // will be overridden below if parameters
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
            {
                bAddParentheses = false;
                aToken.SetByte(0);
            }
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
                SAL_WARN("sc.core", "(parameter count differs, token Byte: " << (int)aToken.GetByte() << " classification: " << (int)nParams << ") ");
            aToken.SetByte( nParams);
            if ( nParams != aToken.GetParamCount() )
                SAL_WARN("sc.core", "(parameter count differs, token ParamCount: " << (int)aToken.GetParamCount() << " classification: " << (int)nParams << ") ");
            if (aToken.GetByte())
                bAddParentheses = true;
            if (bAddParentheses)
                aStr.append('(');
            for ( sal_uInt16 j=0; j < nParams; ++j )
            {
                if ( j > 0 )
                    aStr.append(',');
                formula::ParamClass eType = GetParameterType( &aToken, j);
                switch ( eType )
                {
                    case Value :
                        aStr.append(" Value");
                    break;
                    case Reference :
                        aStr.append(" Reference");
                    break;
                    case ReferenceOrRefArray :
                        aStr.append(" ReferenceOrRefArray");
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
            if (bAddParentheses)
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
            // Return type.
            formula::ParamClass eType = GetParameterType( &aToken, SAL_MAX_UINT16);
            switch ( eType )
            {
                case Value :
                    aStr.append(" -> Value");
                break;
                case Reference :
                    aStr.append(" -> Reference");
                break;
                case ReferenceOrRefArray :
                    aStr.append(" -> ReferenceOrRefArray");
                break;
                case Array :
                    aStr.append(" -> Array");
                break;
                case ForceArray :
                    aStr.append(" -> ForceArray");
                break;
                case ReferenceOrForceArray :
                    aStr.append(" -> ReferenceOrForceArray");
                break;
                case Bounds :
                    ;   // nothing
                break;
                default:
                    aStr.append(" (-> ???, classification error?)");
            }
            /* We could add yet another log domain for this, if we wanted.. but
             * as it more seldom than rarely used it's not actually necessary,
             * just grep output. */
            SAL_INFO( "sc.core", "CALC_GENPARCLASSDOC: " << aStr.makeStringAndClear());
        }
    }
    fflush( stdout);
}

#endif // OSL_DEBUG_LEVEL

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

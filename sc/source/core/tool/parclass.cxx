/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"


#include "parclass.hxx"
#include "token.hxx"
#include "global.hxx"
#include "callform.hxx"
#include "addincol.hxx"
#include "funcdesc.hxx"
#include <unotools/charclass.hxx>
#include <tools/debug.hxx>
#include <string.h>

#if OSL_DEBUG_LEVEL > 1
// the documentation thingy
#include <stdio.h>
#include <com/sun/star/sheet/FormulaLanguage.hpp>
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
    { ocN,               {{ Reference                                            }, false }},
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
    { ocZTest,           {{ Reference, Value, Value                              }, false }},
    // Excel doubts:
    // ocT: Excel says (and handles) Reference, error? This means no position
    // dependent SingleRef if DoubleRef, and no array calculation, just the
    // upper left corner. We never did that.
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
    for ( size_t i=0; i < sizeof(pRawData) / sizeof(RawData); ++i )
    {
        const RawData* pRaw = &pRawData[i];
        if ( pRaw->eOp > SC_OPCODE_LAST_OPCODE_ID )
        {
            DBG_ASSERT( pRaw->eOp == ocNone, "RawData OpCode error");
        }
        else
        {
            RunData* pRun = &pData[ pRaw->eOp ];
#ifdef DBG_UTIL
            if ( pRun->aData.nParam[0] != Unknown )
            {
                DBG_ERROR1( "already assigned: %d", pRaw->eOp);
            }
#endif
            memcpy( &(pRun->aData), &(pRaw->aData), sizeof(CommonData));
            // fill 0-initialized fields with real values
            if ( pRun->aData.bRepeatLast )
            {
                Type eLast = Unknown;
                for ( size_t j=0; j < CommonData::nMaxParams; ++j )
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
                for ( size_t j=0; j < CommonData::nMaxParams; ++j )
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
            for ( size_t j=0; j < CommonData::nMaxParams; ++j )
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
    sal_uInt16 nIndex;
    String aUnoName;
    String aFuncName( ScGlobal::pCharClass->upper( pToken->GetExternal()));
    if ( ScGlobal::GetFuncCollection()->SearchFunc( aFuncName, nIndex) )
    {
        FuncData* pFuncData = (FuncData*)ScGlobal::GetFuncCollection()->At(
                nIndex);
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
    }
    else if ( (aUnoName = ScGlobal::GetAddInCollection()->FindFunction(
                    aFuncName, sal_False)).Len() )
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
            DBG_ERROR2( "ScParameterClassification::Init: too many arguments in listed function: %s: %d",
                    ByteString( *(pDesc->pFuncName),
                        RTL_TEXTENCODING_UTF8).GetBuffer(), nArgs);
            nArgs = CommonData::nMaxParams;
            pRun->aData.bRepeatLast = true;
        }
        pRun->nMinParams = static_cast< sal_uInt8 >( nArgs );
        for ( size_t j=0; j < nArgs; ++j )
        {
            pRun->aData.nParam[j] = Value;
        }
        if ( pRun->aData.bRepeatLast )
        {
            for ( size_t j = nArgs; j < CommonData::nMaxParams; ++j )
            {
                pRun->aData.nParam[j] = Value;
            }
        }
        else
        {
            for ( size_t j = nArgs; j < CommonData::nMaxParams; ++j )
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
            ByteString aStr( xMap->getSymbol(eOp), RTL_TEXTENCODING_UTF8);
            aStr += "(";
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
                    aStr += ",";
                Type eType = GetParameterType( &aToken, j);
                switch ( eType )
                {
                    case Value :
                        aStr += " Value";
                    break;
                    case Reference :
                        aStr += " Reference";
                    break;
                    case Array :
                        aStr += " Array";
                    break;
                    case ForceArray :
                        aStr += " ForceArray";
                    break;
                    case ReferenceOrForceArray :
                        aStr += " ReferenceOrForceArray";
                    break;
                    case Bounds :
                        aStr += " (Bounds, classification error?)";
                    break;
                    default:
                        aStr += " (???, classification error?)";
                }
            }
            if ( HasRepeatParameters( eOp) )
                aStr += ", ...";
            if ( nParams )
                aStr += " ";
            aStr += ")";
            switch ( eOp )
            {
                case ocZGZ:
                    aStr += "   // RRI in English resource, but ZGZ in English-only section";
                break;
                case ocMultiArea:
                    aStr += "   // e.g. combined first parameter of INDEX() function, not a real function";
                break;
                case ocBackSolver:
                    aStr += "   // goal seek via menu, not a real function";
                break;
                case ocTableOp:
                    aStr += "   // MULTIPLE.OPERATIONS in English resource, but TABLE in English-only section";
                break;
                case ocNoName:
                    aStr += "   // error function, not a real function";
                break;
                default:;
            }
            fprintf( stdout, "%s\n", aStr.GetBuffer());
        }
    }
    fflush( stdout);
}

#endif // OSL_DEBUG_LEVEL


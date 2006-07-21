/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: parclass.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 11:38:40 $
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


#include "parclass.hxx"
#include "token.hxx"
#include "global.hxx"
#include "callform.hxx"
#include "addincol.hxx"
#include <unotools/charclass.hxx>
#include <tools/debug.hxx>
#include <string.h>

#if OSL_DEBUG_LEVEL > 1
// the documentation thingy
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
    { ocIf,              {{ Array, Reference, Reference            }}},
    { ocChose,           {{ Array, Reference                       },   true }},
    // Other specials.
    { ocOpen,            {{ Bounds                                 }}},
    { ocClose,           {{ Bounds                                 }}},
    { ocSep,             {{ Bounds                                 }}},
    { ocNoName,          {{ Bounds                                 }}},
    { ocErrCell,         {{ Bounds                                 }}},
    { ocStop,            {{ Bounds                                 }}},
    { ocUnion,           {{ Reference, Reference                   }}},
    { ocRange,           {{ Reference, Reference                   }}},
    // Functions with Value parameters only but not in resource.
    { ocBackSolver,      {{ Value, Value, Value                    }}},
    { ocTableOp,         {{ Value, Value, Value, Value, Value      }}},
    // Operators and functions.
    { ocAdd,             {{ Array, Array                           }}},
    { ocAmpersand,       {{ Array, Array                           }}},
    { ocAnd,             {{ Reference                              },   true }},
    { ocAreas,           {{ Reference                              }}},
    { ocAveDev,          {{ Reference                              },   true }},
    { ocAverage,         {{ Reference                              },   true }},
    { ocAverageA,        {{ Reference                              },   true }},
    { ocCell,            {{ Value, Reference                       }}},
    { ocColumn,          {{ Reference                              }}},
    { ocColumns,         {{ Reference                              },   true }},
    { ocCorrel,          {{ ForceArray, ForceArray                 }}},
    { ocCount,           {{ Reference                              },   true }},
    { ocCount2,          {{ Reference                              },   true }},
    { ocCountEmptyCells, {{ Reference                              }}},
    { ocCountIf,         {{ Reference, Value                       }}},
    { ocCovar,           {{ ForceArray, ForceArray                 }}},
    { ocDBAverage,       {{ Reference, Reference, Reference        }}},
    { ocDBCount,         {{ Reference, Reference, Reference        }}},
    { ocDBCount2,        {{ Reference, Reference, Reference        }}},
    { ocDBGet,           {{ Reference, Reference, Reference        }}},
    { ocDBMax,           {{ Reference, Reference, Reference        }}},
    { ocDBMin,           {{ Reference, Reference, Reference        }}},
    { ocDBProduct,       {{ Reference, Reference, Reference        }}},
    { ocDBStdDev,        {{ Reference, Reference, Reference        }}},
    { ocDBStdDevP,       {{ Reference, Reference, Reference        }}},
    { ocDBSum,           {{ Reference, Reference, Reference        }}},
    { ocDBVar,           {{ Reference, Reference, Reference        }}},
    { ocDBVarP,          {{ Reference, Reference, Reference        }}},
    { ocDevSq,           {{ Reference                              },   true }},
    { ocDiv,             {{ Array, Array                           }}},
    { ocEqual,           {{ Array, Array                           }}},
    { ocForecast,        {{ Value, ForceArray, ForceArray          }}},
    { ocFrequency,       {{ Reference, Reference                   }}},
    { ocFTest,           {{ ForceArray, ForceArray                 }}},
    { ocGeoMean,         {{ Reference                              },   true }},
    { ocGGT,             {{ Reference                              },   true }},
    { ocGreater,         {{ Array, Array                           }}},
    { ocGreaterEqual,    {{ Array, Array                           }}},
    { ocGrowth,          {{ Reference, Reference, Reference, Value }}},
    { ocHarMean,         {{ Reference                              },   true }},
    { ocHLookup,         {{ Value, Reference, Value, Value         }}},
    { ocIKV,             {{ Reference, Value                       }}},
    { ocIndex,           {{ Reference, Value, Value, Value         }}},
    { ocIntercept,       {{ ForceArray, ForceArray                 }}},
    { ocIntersect,       {{ Reference, Reference                   }}},
    { ocIsRef,           {{ Reference                              }}},
    { ocKGV,             {{ Reference                              },   true }},
    { ocKurt,            {{ Reference                              },   true }},
    { ocLarge,           {{ Reference, Value                       }}},
    { ocLess,            {{ Array, Array                           }}},
    { ocLessEqual,       {{ Array, Array                           }}},
    { ocLookup,          {{ Value, Reference, Reference            }}},
    { ocMatch,           {{ Value, Reference, Reference            }}},
    { ocMatDet,          {{ ForceArray                             }}},
    { ocMatInv,          {{ ForceArray                             }}},
    { ocMatMult,         {{ ForceArray, ForceArray                 }}},
    { ocMatTrans,        {{ Array                                  }}}, // strange, but Xcl doesn't force MatTrans array
    { ocMatValue,        {{ Reference, Value, Value                }}},
    { ocMax,             {{ Reference                              },   true }},
    { ocMaxA,            {{ Reference                              },   true }},
    { ocMedian,          {{ Reference                              },   true }},
    { ocMin,             {{ Reference                              },   true }},
    { ocMinA,            {{ Reference                              },   true }},
    { ocMIRR,            {{ Reference, Value, Value                }}},
    { ocModalValue,      {{ ForceArray                             },   true }},
    { ocMul,             {{ Array, Array                           }}},
    { ocMultiArea,       {{ Reference                              },   true }},
    { ocN,               {{ Reference                              }}},
    { ocNBW,             {{ Value, Reference                       },   true }},
    { ocNeg,             {{ Array                                  }}},
    { ocNot,             {{ Array                                  }}},
    { ocNotEqual,        {{ Array, Array                           }}},
    { ocOffset,          {{ Reference, Value, Value, Value, Value  }}},
    { ocOr,              {{ Reference                              },   true }},
    { ocPearson,         {{ ForceArray, ForceArray                 }}},
    { ocPercentile,      {{ Reference, Value                       }}},
    { ocPercentrank,     {{ Reference, Value                       }}},
    { ocPow,             {{ Array, Array                           }}},
    { ocPower,           {{ Array, Array                           }}},
    { ocProb,            {{ ForceArray, ForceArray, Value, Value   }}},
    { ocProduct,         {{ Reference                              },   true }},
    { ocQuartile,        {{ Reference, Value                       }}},
    { ocRank,            {{ Value, Reference, Value                }}},
    { ocRGP,             {{ Reference, Reference, Value, Value     }}},
    { ocRKP,             {{ Reference, Reference, Value, Value     }}},
    { ocRow,             {{ Reference                              }}},
    { ocRows,            {{ Reference                              },   true }},
    { ocRSQ,             {{ ForceArray, ForceArray                 }}},
    { ocSchiefe,         {{ Reference                              },   true }},
    { ocSlope,           {{ ForceArray, ForceArray                 }}},
    { ocSmall,           {{ Reference, Value                       }}},
    { ocStDev,           {{ Reference                              },   true }},
    { ocStDevA,          {{ Reference                              },   true }},
    { ocStDevP,          {{ Reference                              },   true }},
    { ocStDevPA,         {{ Reference                              },   true }},
    { ocSTEYX,           {{ ForceArray, ForceArray                 }}},
    { ocSub,             {{ Array, Array                           }}},
    { ocSubTotal,        {{ Value, Reference                       },   true }},
    { ocSum,             {{ Reference                              },   true }},
    { ocSumIf,           {{ Reference, Value, Reference            }}},
    { ocSumProduct,      {{ ForceArray                             },   true }},
    { ocSumSQ,           {{ Reference                              },   true }},
    { ocSumX2MY2,        {{ ForceArray, ForceArray                 }}},
    { ocSumX2DY2,        {{ ForceArray, ForceArray                 }}},
    { ocSumXMY2,         {{ ForceArray, ForceArray                 }}},
    { ocTable,           {{ Reference                              }}},
    { ocTables,          {{ Reference                              },   true }},
    { ocTrend,           {{ Reference, Reference, Reference, Value }}},
    { ocTrimMean,        {{ Reference, Value                       }}},
    { ocTTest,           {{ ForceArray, ForceArray, Value, Value   }}},
    { ocVar,             {{ Reference                              },   true }},
    { ocVarA,            {{ Reference                              },   true }},
    { ocVarP,            {{ Reference                              },   true }},
    { ocVarPA,           {{ Reference                              },   true }},
    { ocVLookup,         {{ Value, Reference, Value, Value         }}},
    { ocZTest,           {{ Reference, Value, Value                }}},
    // Excel doubts:
    // ocT: Excel says (and handles) Reference, error? This means no position
    // dependent SingleRef if DoubleRef, and no array calculation, just the
    // upper left corner. We never did that.
    { ocT, {{ Value }}},
    // The stopper.
    { ocNone, {{ Bounds }} }
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
                        pRun->nMinParams = j+1;
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
                            pRun->nMinParams = j;
                        pRun->aData.nParam[j] = Bounds;
                    }
                }
                if ( !pRun->nMinParams &&
                        pRun->aData.nParam[CommonData::nMaxParams-1] != Bounds)
                    pRun->nMinParams = CommonData::nMaxParams;
            }
            for ( size_t j=0; j < CommonData::nMaxParams; ++j )
            {
                if ( pRun->aData.nParam[j] == ForceArray )
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
        const ScToken* pToken, USHORT nParameter)
{
    OpCode eOp = pToken->GetOpCode();
    switch ( eOp )
    {
        case ocExternal:
            return GetExternalParameterType( pToken, nParameter);
        break;
        case ocMacro:
            return Reference;
        break;
    }
    if ( eOp <= SC_OPCODE_LAST_OPCODE_ID )
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
ScParameterClassification::GetExternalParameterType( const ScToken* pToken,
        USHORT nParameter)
{
    Type eRet = Unknown;
    // similar to ScInterpreter::ScExternal()
    USHORT nIndex;
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
                    aFuncName, FALSE)).Len() )
    {
        // the relevant parts of ScUnoAddInCall without having to create one
        const ScUnoAddInFuncData* pFuncData =
            ScGlobal::GetAddInCollection()->GetFuncData( aUnoName);
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
        USHORT nArgs = pDesc->nArgCount;
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
        pRun->nMinParams = nArgs;
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
    // only to initialize English resources
    {
        ScCompiler aComp( 0, ScAddress());
        aComp.SetCompileEnglish( TRUE);
    }
    const String* pSym = ScCompiler::pSymbolTableEnglish;
    fflush( stderr);
    for ( size_t i=0; i<ScCompiler::nAnzStrings; ++i )
    {
        if ( pSym[i].Len() )
        {
            OpCode eOp = OpCode(i);
            fprintf( stdout, "%s: ", aEnvVarName);
            ByteString aStr( pSym[i], RTL_TEXTENCODING_UTF8);
            aStr += "(";
            ScByteToken aToken( eOp);
            BYTE nParams = GetMinimumParameters( eOp);
            // preset parameter count according to opcode value, with some
            // special handling
            if ( eOp <= ocEndDiv )
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
                }
            }
            else if ( eOp <= ocEndBinOp )
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
            else if ( eOp <= ocEndUnOp )
                aToken.SetByte(1);
            else if ( eOp <= ocEndNoPar )
                aToken.SetByte(0);
            else if ( eOp <= ocEnd1Par )
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
            for ( USHORT j=0; j < nParams; ++j )
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
            }
            fprintf( stdout, "%s\n", aStr.GetBuffer());
        }
    }
    fflush( stdout);
}

#endif // OSL_DEBUG_LEVEL

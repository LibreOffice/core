/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_formula.hxx"
#include <sal/macros.h>
#include "formula/FormulaCompiler.hxx"
#include "formula/errorcodes.hxx"
#include "formula/token.hxx"
#include "formula/tokenarray.hxx"
#include "core_resource.hxx"
#include "core_resource.hrc"

#include <svl/zforlist.hxx>
#include <tools/rc.hxx>
#include <tools/rcid.h>
#include <com/sun/star/sheet/FormulaOpCodeMapEntry.hpp>
#include <com/sun/star/sheet/FormulaMapGroup.hpp>
#include <com/sun/star/sheet/FormulaMapGroupSpecialOffset.hpp>
#include <stdio.h>

// =============================================================================
namespace formula
{
// =============================================================================
    using namespace ::com::sun::star;

    static const sal_Char* pInternal[ 1 ] = { "TTT" };

// =============================================================================
namespace
{
// =============================================================================
class FormulaCompilerRecursionGuard
{
private:
            short&              rRecursion;
public:
                                FormulaCompilerRecursionGuard( short& rRec )
                                    : rRecursion( rRec ) { ++rRecursion; }
                                ~FormulaCompilerRecursionGuard() { --rRecursion; }
};

short lcl_GetRetFormat( OpCode eOpCode )
{
    switch (eOpCode)
    {
        case ocEqual:
        case ocNotEqual:
        case ocLess:
        case ocGreater:
        case ocLessEqual:
        case ocGreaterEqual:
        case ocAnd:
        case ocOr:
        case ocNot:
        case ocTrue:
        case ocFalse:
        case ocIsEmpty:
        case ocIsString:
        case ocIsNonString:
        case ocIsLogical:
        case ocIsRef:
        case ocIsValue:
        case ocIsFormula:
        case ocIsNA:
        case ocIsErr:
        case ocIsError:
        case ocIsEven:
        case ocIsOdd:
        case ocExact:
            return NUMBERFORMAT_LOGICAL;
        case ocGetActDate:
        case ocGetDate:
        case ocEasterSunday :
            return NUMBERFORMAT_DATE;
        case ocGetActTime:
            return NUMBERFORMAT_DATETIME;
        case ocGetTime:
            return NUMBERFORMAT_TIME;
        case ocNPV:
        case ocBW:
        case ocDIA:
        case ocGDA:
        case ocGDA2:
        case ocVBD:
        case ocLIA:
        case ocRMZ:
        case ocZW:
        case ocZinsZ:
        case ocKapz:
        case ocKumZinsZ:
        case ocKumKapZ:
            return NUMBERFORMAT_CURRENCY;
        case ocZins:
        case ocIRR:
        case ocMIRR:
        case ocZGZ:
        case ocEffektiv:
        case ocNominal:
        case ocPercentSign:
            return NUMBERFORMAT_PERCENT;
        default:
            return NUMBERFORMAT_NUMBER;
    }
    return NUMBERFORMAT_NUMBER;
}

inline void lclPushOpCodeMapEntry( ::std::vector< sheet::FormulaOpCodeMapEntry >& rVec, const String* pTable, sal_uInt16 nOpCode )
{
    sheet::FormulaOpCodeMapEntry aEntry;
    aEntry.Token.OpCode = nOpCode;
    aEntry.Name = pTable[nOpCode];
    rVec.push_back( aEntry);
}

void lclPushOpCodeMapEntries( ::std::vector< sheet::FormulaOpCodeMapEntry >& rVec, const String* pTable, sal_uInt16 nOpCodeBeg, sal_uInt16 nOpCodeEnd )
{
    for (sal_uInt16 nOpCode = nOpCodeBeg; nOpCode < nOpCodeEnd; ++nOpCode)
        lclPushOpCodeMapEntry( rVec, pTable, nOpCode );
}

void lclPushOpCodeMapEntries( ::std::vector< sheet::FormulaOpCodeMapEntry >& rVec, const String* pTable, const sal_uInt16* pnOpCodes, size_t nCount )
{
    for (const sal_uInt16* pnEnd = pnOpCodes + nCount; pnOpCodes < pnEnd; ++pnOpCodes)
        lclPushOpCodeMapEntry( rVec, pTable, *pnOpCodes );
}

class OpCodeList : public Resource        // temp object for resource
{
public:

    OpCodeList( sal_uInt16, FormulaCompiler::NonConstOpCodeMapPtr );

private:
    bool getOpCodeString( String& rStr, sal_uInt16 nOp );
    void putDefaultOpCode( FormulaCompiler::NonConstOpCodeMapPtr xMap, sal_uInt16 nOp );

private:
    enum SeparatorType
    {
        SEMICOLON_BASE,
        COMMA_BASE
    };
    SeparatorType meSepType;
};

OpCodeList::OpCodeList( sal_uInt16 nRID, FormulaCompiler::NonConstOpCodeMapPtr xMap ) :
    Resource( ResId(nRID,*ResourceManager::getResManager()) )
    ,meSepType(SEMICOLON_BASE)
{
     for (sal_uInt16 i = 0; i <= SC_OPCODE_LAST_OPCODE_ID; ++i)
    {
        String aOpStr;
        if ( getOpCodeString(aOpStr, i) )
            xMap->putOpCode(aOpStr, OpCode(i));
        else
            putDefaultOpCode(xMap, i);
    }

    FreeResource();
}

bool OpCodeList::getOpCodeString( String& rStr, sal_uInt16 nOp )
{
    switch (nOp)
    {
        case SC_OPCODE_SEP:
        {
            if (meSepType == COMMA_BASE)
            {
                rStr = String::CreateFromAscii(",");
                return true;
            }
            else if (meSepType == SEMICOLON_BASE)
            {
                rStr = String::CreateFromAscii(";");
                return true;
            }
        }
        break;
        case SC_OPCODE_ARRAY_COL_SEP:
        {
            if (meSepType == COMMA_BASE)
            {
                rStr = String::CreateFromAscii(",");
                return true;
            }
            else if (meSepType == SEMICOLON_BASE)
            {
                rStr = String::CreateFromAscii(";");
                return true;
            }
        }
        break;
        case SC_OPCODE_ARRAY_ROW_SEP:
        {
            if (meSepType == COMMA_BASE)
            {
                rStr = String::CreateFromAscii(";");
                return true;
            }
            else if (meSepType == SEMICOLON_BASE)
            {
                rStr = String::CreateFromAscii("|");
                return true;
            }
        }
        break;
    }

    return false;
}

void OpCodeList::putDefaultOpCode( FormulaCompiler::NonConstOpCodeMapPtr xMap, sal_uInt16 nOp )
{
    ResId aRes(nOp,*ResourceManager::getResManager());
    aRes.SetRT(RSC_STRING);
    if (IsAvailableRes(aRes))
        xMap->putOpCode(aRes, OpCode(nOp));
}
// -----------------------------------------------------------------------------
// static
const sal_Unicode* lcl_UnicodeStrChr( const sal_Unicode* pStr,sal_Unicode c )
{
    if ( !pStr )
        return NULL;
    while ( *pStr )
    {
        if ( *pStr == c )
            return pStr;
        pStr++;
    }
    return NULL;
}
// =============================================================================
} // empty
// =============================================================================

void FormulaCompiler::OpCodeMap::putExternal( const String & rSymbol, const String & rAddIn )
{
    bool bOk = mpExternalHashMap->insert( ExternalHashMap::value_type( rSymbol, rAddIn)).second;
    if (bOk)
        bOk = mpReverseExternalHashMap->insert( ExternalHashMap::value_type( rAddIn, rSymbol)).second;
    DBG_ASSERT( bOk, "OpCodeMap::putExternal: symbol not inserted");
}

void FormulaCompiler::OpCodeMap::putExternalSoftly( const String & rSymbol, const String & rAddIn )
{
    bool bOk = mpReverseExternalHashMap->insert( ExternalHashMap::value_type( rAddIn, rSymbol)).second;
    if (bOk)
        mpExternalHashMap->insert( ExternalHashMap::value_type( rSymbol, rAddIn)).second;
}
uno::Sequence< sheet::FormulaToken > FormulaCompiler::OpCodeMap::createSequenceOfFormulaTokens(const FormulaCompiler& _rCompiler,const uno::Sequence< ::rtl::OUString >& rNames ) const
{
    const sal_Int32 nLen = rNames.getLength();
    uno::Sequence< sheet::FormulaToken > aTokens( nLen);
    sheet::FormulaToken* pToken = aTokens.getArray();
    ::rtl::OUString const * pName = rNames.getConstArray();
    ::rtl::OUString const * const pStop = pName + nLen;
    for ( ; pName < pStop; ++pName, ++pToken)
    {
        OpCodeHashMap::const_iterator iLook( mpHashMap->find( *pName));
        if (iLook != mpHashMap->end())
            pToken->OpCode = (*iLook).second;
        else
        {
            ::rtl::OUString aIntName;
            if (hasExternals())
            {
                ExternalHashMap::const_iterator iExt( mpExternalHashMap->find( *pName));
                if (iExt != mpExternalHashMap->end())
                    aIntName = (*iExt).second;
                // Check for existence not needed here, only name-mapping is of
                // interest.
            }
            if (!aIntName.getLength())
                aIntName = _rCompiler.FindAddInFunction(*pName, !isEnglish());    // bLocalFirst=sal_False for english
            if (!aIntName.getLength())
                pToken->OpCode = getOpCodeUnknown();
            else
            {
                pToken->OpCode = ocExternal;
                pToken->Data <<= aIntName;
            }
        }
    }
    return aTokens;
}
uno::Sequence< sheet::FormulaOpCodeMapEntry > FormulaCompiler::OpCodeMap::createSequenceOfAvailableMappings(const FormulaCompiler& _rCompiler,const sal_Int32 nGroups ) const
{
    using namespace sheet;

    // Unfortunately uno::Sequence can't grow without cumbersome reallocs. As
    // we don't know in advance how many elements it will have we use a
    // temporary vector to add elements and then copy to Sequence :-(
    ::std::vector< FormulaOpCodeMapEntry > aVec;

    if (nGroups == FormulaMapGroup::SPECIAL)
    {
        // Use specific order, keep in sync with
        // offapi/com/sun/star/sheet/FormulaMapGroupSpecialOffset.idl
        static const struct
        {
            sal_Int32 nOff;
            OpCode    eOp;
        } aMap[] = {
            { FormulaMapGroupSpecialOffset::PUSH              , ocPush }           ,
            { FormulaMapGroupSpecialOffset::CALL              , ocCall }           ,
            { FormulaMapGroupSpecialOffset::STOP              , ocStop }           ,
            { FormulaMapGroupSpecialOffset::EXTERNAL          , ocExternal }       ,
            { FormulaMapGroupSpecialOffset::NAME              , ocName }           ,
            { FormulaMapGroupSpecialOffset::NO_NAME           , ocNoName }         ,
            { FormulaMapGroupSpecialOffset::MISSING           , ocMissing }        ,
            { FormulaMapGroupSpecialOffset::BAD               , ocBad }            ,
            { FormulaMapGroupSpecialOffset::SPACES            , ocSpaces }         ,
            { FormulaMapGroupSpecialOffset::MAT_REF           , ocMatRef }         ,
            { FormulaMapGroupSpecialOffset::DB_AREA           , ocDBArea }         ,
            { FormulaMapGroupSpecialOffset::MACRO             , ocMacro }          ,
            { FormulaMapGroupSpecialOffset::COL_ROW_NAME      , ocColRowName }
        };
        const size_t nCount = SAL_N_ELEMENTS(aMap);
        // Preallocate vector elements.
        if (aVec.size() < nCount)
        {
            FormulaOpCodeMapEntry aEntry;
            aEntry.Token.OpCode = getOpCodeUnknown();
            aVec.resize( nCount, aEntry);
        } // if (aVec.size() < nCount)

        FormulaOpCodeMapEntry aEntry;
        for (size_t i=0; i < nCount; ++i)
        {
            size_t nIndex = static_cast< size_t >( aMap[i].nOff );
            if (aVec.size() <= nIndex)
            {
                // The offsets really should be aligned with the size, so if
                // the vector was preallocated above this code to resize it is
                // just a measure in case the table isn't in sync with the API,
                // usually it isn't executed.
                aEntry.Token.OpCode = getOpCodeUnknown();
                aVec.resize( nIndex + 1, aEntry );
            }
            aEntry.Token.OpCode = aMap[i].eOp;
            aVec[nIndex] = aEntry;
        }
    }
    else
    {
        /* FIXME: Once we support error constants in formulas we'll need a map
         * group for that, e.g. FormulaMapGroup::ERROR_CONSTANTS, and fill
         * SC_OPCODE_START_ERRORS to SC_OPCODE_STOP_ERRORS. */

        // Anything else but SPECIAL.
        if ((nGroups & FormulaMapGroup::SEPARATORS) != 0)
        {
            static const sal_uInt16 aOpCodes[] = {
                SC_OPCODE_OPEN,
                SC_OPCODE_CLOSE,
                SC_OPCODE_SEP,
            };
            lclPushOpCodeMapEntries( aVec, mpTable, aOpCodes, SAL_N_ELEMENTS(aOpCodes) );
        }
        if ((nGroups & FormulaMapGroup::ARRAY_SEPARATORS) != 0)
        {
            static const sal_uInt16 aOpCodes[] = {
                SC_OPCODE_ARRAY_OPEN,
                SC_OPCODE_ARRAY_CLOSE,
                SC_OPCODE_ARRAY_ROW_SEP,
                SC_OPCODE_ARRAY_COL_SEP
            };
            lclPushOpCodeMapEntries( aVec, mpTable, aOpCodes, SAL_N_ELEMENTS(aOpCodes) );
        }
        if ((nGroups & FormulaMapGroup::UNARY_OPERATORS) != 0)
        {
            // Due to the nature of the percent operator following its operand
            // it isn't sorted into unary operators for compiler interna.
            lclPushOpCodeMapEntry( aVec, mpTable, ocPercentSign );
            // "+" can be used as unary operator too, push only if binary group is not set
            if ((nGroups & FormulaMapGroup::BINARY_OPERATORS) == 0)
                lclPushOpCodeMapEntry( aVec, mpTable, ocAdd );
            // regular unary operators
            for (sal_uInt16 nOp = SC_OPCODE_START_UN_OP; nOp < SC_OPCODE_STOP_UN_OP && nOp < mnSymbols; ++nOp)
            {
                switch (nOp)
                {
                    // NOT and NEG in fact are functions but for legacy reasons
                    // are sorted into unary operators for compiler interna.
                    case SC_OPCODE_NOT :
                    case SC_OPCODE_NEG :
                        break;   // nothing,
                    default:
                        lclPushOpCodeMapEntry( aVec, mpTable, nOp );
                }
            }
        }
        if ((nGroups & FormulaMapGroup::BINARY_OPERATORS) != 0)
        {
            for (sal_uInt16 nOp = SC_OPCODE_START_BIN_OP; nOp < SC_OPCODE_STOP_BIN_OP && nOp < mnSymbols; ++nOp)
            {
                switch (nOp)
                {
                    // AND and OR in fact are functions but for legacy reasons
                    // are sorted into binary operators for compiler interna.
                    case SC_OPCODE_AND :
                    case SC_OPCODE_OR :
                        break;   // nothing,
                    default:
                        lclPushOpCodeMapEntry( aVec, mpTable, nOp );
                }
            }
        }
        if ((nGroups & FormulaMapGroup::FUNCTIONS) != 0)
        {
            // Function names are not consecutive, skip the gaps between
            // functions with no parameter, functions with 1 parameter
            lclPushOpCodeMapEntries( aVec, mpTable, SC_OPCODE_START_NO_PAR, ::std::min< sal_uInt16 >( SC_OPCODE_STOP_NO_PAR, mnSymbols ) );
            lclPushOpCodeMapEntries( aVec, mpTable, SC_OPCODE_START_1_PAR, ::std::min< sal_uInt16 >( SC_OPCODE_STOP_1_PAR, mnSymbols ) );
            // Additional functions not within range of functions.
            static const sal_uInt16 aOpCodes[] = {
                SC_OPCODE_IF,
                SC_OPCODE_CHOSE,
                SC_OPCODE_AND,
                SC_OPCODE_OR,
                SC_OPCODE_NOT,
                SC_OPCODE_NEG
            };
            lclPushOpCodeMapEntries( aVec, mpTable, aOpCodes, SAL_N_ELEMENTS(aOpCodes) );
            // functions with 2 or more parameters.
            for (sal_uInt16 nOp = SC_OPCODE_START_2_PAR; nOp < SC_OPCODE_STOP_2_PAR && nOp < mnSymbols; ++nOp)
            {
                switch (nOp)
                {
                    // NO_NAME is in SPECIAL.
                    case SC_OPCODE_NO_NAME :
                        break;   // nothing,
                    default:
                        lclPushOpCodeMapEntry( aVec, mpTable, nOp );
                }
            }
            // If AddIn functions are present in this mapping, use them, and only those.
            if (hasExternals())
            {
                for (ExternalHashMap::const_iterator it( mpExternalHashMap->begin());it != mpExternalHashMap->end(); ++it)
                {
                    FormulaOpCodeMapEntry aEntry;
                    aEntry.Name = (*it).first;
                    aEntry.Token.Data <<= ::rtl::OUString( (*it).second);
                    aEntry.Token.OpCode = ocExternal;
                    aVec.push_back( aEntry);
                }
            }
            else
            {
                _rCompiler.fillAddInToken(aVec,isEnglish());
            }
        }
    }
    const FormulaOpCodeMapEntry* pRet = aVec.empty() ? 0 : &aVec[0];
    return uno::Sequence< FormulaOpCodeMapEntry >(pRet, aVec.size());
}
//-----------------------------------------------------------------------------

void FormulaCompiler::OpCodeMap::putOpCode( const String & rStr, const OpCode eOp )
{
    DBG_ASSERT( 0 < eOp && sal_uInt16(eOp) < mnSymbols, "OpCodeMap::putOpCode: OpCode out of range");
    if (0 < eOp && sal_uInt16(eOp) < mnSymbols)
    {
        DBG_ASSERT( (mpTable[eOp].Len() == 0) || (mpTable[eOp] == rStr) || (eOp == ocCurrency),
            ByteString( "OpCodeMap::putOpCode: reusing OpCode ").
            Append( ByteString::CreateFromInt32( sal_Int32( eOp))).Append( " (").
            Append( ByteString( rStr, RTL_TEXTENCODING_ASCII_US)).Append( ')').GetBuffer());
        mpTable[eOp] = rStr;
        mpHashMap->insert( OpCodeHashMap::value_type( rStr, eOp));
    }
}
// -----------------------------------------------------------------------------
// class FormulaCompiler
// -----------------------------------------------------------------------------
DBG_NAME(FormulaCompiler)
FormulaCompiler::FormulaCompiler(FormulaTokenArray& _rArr)
        :
        pArr( &_rArr ),
        pExternalRef(NULL),
        pStack( NULL ),
        nRecursion(0),
        nNumFmt( NUMBERFORMAT_UNDEFINED ),
        meGrammar( formula::FormulaGrammar::GRAM_UNSPECIFIED ),
        bAutoCorrect( sal_False ),
        bCorrected( sal_False ),
        bCompileForFAP( sal_False ),
        bIgnoreErrors( sal_False )

{
    DBG_CTOR(FormulaCompiler,NULL);
}
FormulaCompiler::FormulaCompiler()
        :
        pArr( NULL ),
        pExternalRef(NULL),
        pStack( NULL ),
        nRecursion(0),
        nNumFmt( NUMBERFORMAT_UNDEFINED ),
        meGrammar( formula::FormulaGrammar::GRAM_UNSPECIFIED ),
        bAutoCorrect( sal_False ),
        bCorrected( sal_False ),
        bCompileForFAP( sal_False ),
        bIgnoreErrors( sal_False )

{
    DBG_CTOR(FormulaCompiler,NULL);
}
FormulaCompiler::~FormulaCompiler()
{
    DBG_DTOR(FormulaCompiler,NULL);
}

FormulaCompiler::OpCodeMapPtr FormulaCompiler::GetOpCodeMap( const sal_Int32 nLanguage ) const
{
    FormulaCompiler::OpCodeMapPtr xMap;
    using namespace sheet;
    switch (nLanguage)
    {
        case FormulaLanguage::ODFF :
            if (!mxSymbolsODFF)
                InitSymbolsODFF();
            xMap = mxSymbolsODFF;
            break;
        case FormulaLanguage::ODF_11 :
            if (!mxSymbolsPODF)
                InitSymbolsPODF();
            xMap = mxSymbolsPODF;
            break;
        case FormulaLanguage::ENGLISH :
            if (!mxSymbolsEnglish)
                InitSymbolsEnglish();
            xMap = mxSymbolsEnglish;
            break;
        case FormulaLanguage::NATIVE :
            if (!mxSymbolsNative)
                InitSymbolsNative();
            xMap = mxSymbolsNative;
            break;
        case FormulaLanguage::XL_ENGLISH:
            if (!mxSymbolsEnglishXL)
                InitSymbolsEnglishXL();
            xMap = mxSymbolsEnglishXL;
            break;
        default:
            ;   // nothing, NULL map returned
    }
    return xMap;
}
// -----------------------------------------------------------------------------

String FormulaCompiler::FindAddInFunction( const String& /*rUpperName*/, sal_Bool /*bLocalFirst*/ ) const
{
    return String();
}
// -----------------------------------------------------------------------------
FormulaCompiler::OpCodeMapPtr FormulaCompiler::CreateOpCodeMap(
        const uno::Sequence<
        const sheet::FormulaOpCodeMapEntry > & rMapping,
        bool bEnglish )
{
    using sheet::FormulaOpCodeMapEntry;
    // Filter / API maps are never Core
    NonConstOpCodeMapPtr xMap( new OpCodeMap( SC_OPCODE_LAST_OPCODE_ID + 1,false, FormulaGrammar::mergeToGrammar( FormulaGrammar::setEnglishBit(FormulaGrammar::GRAM_EXTERNAL, bEnglish),FormulaGrammar::CONV_UNSPECIFIED)));
    FormulaOpCodeMapEntry const * pArr2 = rMapping.getConstArray();
    FormulaOpCodeMapEntry const * const pStop = pArr2 + rMapping.getLength();
    for ( ; pArr2 < pStop; ++pArr2)
    {
        OpCode eOp = OpCode(pArr2->Token.OpCode);
        if (eOp != ocExternal)
            xMap->putOpCode( pArr2->Name, eOp);
        else
        {
            ::rtl::OUString aExternalName;
            if (pArr2->Token.Data >>= aExternalName)
                xMap->putExternal( pArr2->Name, aExternalName);
            else
            {
                DBG_ERRORFILE( "FormulaCompiler::CreateOpCodeMap: no Token.Data external name");
            }
        }
    }
    return xMap;
}

// -----------------------------------------------------------------------------
void lcl_fillNativeSymbols(FormulaCompiler::NonConstOpCodeMapPtr& _xMap,bool _destroy = false)
{
    static FormulaCompiler::NonConstOpCodeMapPtr s_SymbolMap;
    if ( _destroy )
    {
        s_SymbolMap.reset();
    } // if ( _destroy )
    else if ( !s_SymbolMap.get() )
    {
        // Core
        s_SymbolMap.reset( new FormulaCompiler::OpCodeMap( SC_OPCODE_LAST_OPCODE_ID + 1, true, FormulaGrammar::GRAM_NATIVE_UI));
        OModuleClient aModuleClient;
        OpCodeList aOpCodeListNative( RID_STRLIST_FUNCTION_NAMES, s_SymbolMap );
        // No AddInMap for native core mapping.
    } // if ( !s_SymbolMap.get() )
    _xMap = s_SymbolMap;
}
// -----------------------------------------------------------------------------
const String& FormulaCompiler::GetNativeSymbol( OpCode eOp )
{
    NonConstOpCodeMapPtr xSymbolsNative;
    lcl_fillNativeSymbols(xSymbolsNative);
    return xSymbolsNative->getSymbol( eOp );
}
// -----------------------------------------------------------------------------
void FormulaCompiler::InitSymbolsNative() const
{
    lcl_fillNativeSymbols(mxSymbolsNative);
}
// -----------------------------------------------------------------------------
void FormulaCompiler::InitSymbolsEnglish() const
{
    static NonConstOpCodeMapPtr s_sSymbol;
    if ( !s_sSymbol.get() )
        loadSymbols(RID_STRLIST_FUNCTION_NAMES_ENGLISH,FormulaGrammar::GRAM_ENGLISH,s_sSymbol);
    mxSymbolsEnglish = s_sSymbol;
}
// -----------------------------------------------------------------------------
void FormulaCompiler::InitSymbolsPODF() const
{
    static NonConstOpCodeMapPtr s_sSymbol;
    if ( !s_sSymbol.get() )
        loadSymbols(RID_STRLIST_FUNCTION_NAMES_ENGLISH,FormulaGrammar::GRAM_PODF,s_sSymbol);
    mxSymbolsPODF = s_sSymbol;
}
// -----------------------------------------------------------------------------
void FormulaCompiler::InitSymbolsODFF() const
{
    static NonConstOpCodeMapPtr s_sSymbol;
    if ( !s_sSymbol.get() )
        loadSymbols(RID_STRLIST_FUNCTION_NAMES_ENGLISH_ODFF,FormulaGrammar::GRAM_ODFF,s_sSymbol);
    mxSymbolsODFF = s_sSymbol;
}
// -----------------------------------------------------------------------------
void FormulaCompiler::InitSymbolsEnglishXL() const
{
    static NonConstOpCodeMapPtr s_sSymbol;
    if ( !s_sSymbol.get() )
        loadSymbols(RID_STRLIST_FUNCTION_NAMES_ENGLISH,FormulaGrammar::GRAM_ENGLISH,s_sSymbol);
    mxSymbolsEnglishXL = s_sSymbol;

    // TODO: For now, just replace the separators to the Excel English
    // variants. Later, if we want to properly map Excel functions with Calc
    // functions, we'll need to do a little more work here.
    mxSymbolsEnglishXL->putOpCode(sal_Unicode(','), ocSep);
    mxSymbolsEnglishXL->putOpCode(sal_Unicode(','), ocArrayColSep);
    mxSymbolsEnglishXL->putOpCode(sal_Unicode(';'), ocArrayRowSep);
}

// -----------------------------------------------------------------------------
void FormulaCompiler::loadSymbols(sal_uInt16 _nSymbols,FormulaGrammar::Grammar _eGrammar,NonConstOpCodeMapPtr& _xMap) const
{
    if ( !_xMap.get() )
    {
        // not Core
        _xMap.reset( new OpCodeMap( SC_OPCODE_LAST_OPCODE_ID + 1, _eGrammar != FormulaGrammar::GRAM_ODFF, _eGrammar ));
        OModuleClient aModuleClient;
        OpCodeList aOpCodeList( _nSymbols, _xMap );

        fillFromAddInMap( _xMap, _eGrammar);
        // Fill from collection for AddIns not already present.
        if ( FormulaGrammar::GRAM_ENGLISH != _eGrammar )
            fillFromAddInCollectionUpperName( _xMap);
        else
            fillFromAddInCollectionEnglishName( _xMap);
    }
}
// -----------------------------------------------------------------------------
void FormulaCompiler::fillFromAddInCollectionUpperName( NonConstOpCodeMapPtr /*xMap */) const
{
}
// -----------------------------------------------------------------------------
void FormulaCompiler::fillFromAddInCollectionEnglishName( NonConstOpCodeMapPtr /*xMap */) const
{
}
// -----------------------------------------------------------------------------
void FormulaCompiler::fillFromAddInMap( NonConstOpCodeMapPtr /*xMap*/, FormulaGrammar::Grammar /*_eGrammar */) const
{
}
// -----------------------------------------------------------------------------
OpCode FormulaCompiler::GetEnglishOpCode( const String& rName ) const
{
    FormulaCompiler::OpCodeMapPtr xMap = GetOpCodeMap(sheet::FormulaLanguage::ENGLISH);

    formula::OpCodeHashMap::const_iterator iLook( xMap->getHashMap()->find( rName ) );
    bool bFound = (iLook != xMap->getHashMap()->end());
    return bFound ? (*iLook).second : OpCode(ocNone);
}

bool FormulaCompiler::IsOpCodeVolatile( OpCode eOp )
{
    switch (eOp)
    {
        // no parameters:
        case ocRandom:
        case ocGetActDate:
        case ocGetActTime:
        // one parameter:
        case ocFormula:
        case ocInfo:
        // more than one parameters:
            // ocIndirect/ocIndirectXL otherwise would have to do
            // StopListening and StartListening on a reference for every
            // interpreted value.
        case ocIndirect:
        case ocIndirectXL:
            // ocOffset results in indirect references.
        case ocOffset:
            return true;
    }
    return false;
}

// Remove quotes, escaped quotes are unescaped.
sal_Bool FormulaCompiler::DeQuote( String& rStr )
{
    xub_StrLen nLen = rStr.Len();
    if ( nLen > 1 && rStr.GetChar(0) == '\'' && rStr.GetChar( nLen-1 ) == '\'' )
    {
        rStr.Erase( nLen-1, 1 );
        rStr.Erase( 0, 1 );
        xub_StrLen nPos = 0;
        while ( (nPos = rStr.SearchAscii( "\\\'", nPos)) != STRING_NOTFOUND )
        {
            rStr.Erase( nPos, 1 );
            ++nPos;
        }
        return sal_True;
    }
    return sal_False;
}
// -----------------------------------------------------------------------------
void FormulaCompiler::fillAddInToken(::std::vector< sheet::FormulaOpCodeMapEntry >& /*_rVec*/,bool /*_bIsEnglish*/) const
{
}
// -----------------------------------------------------------------------------
sal_Bool FormulaCompiler::IsMatrixFunction(OpCode _eOpCode)
{
    switch ( _eOpCode )
    {
        case ocDde :
        case ocGrowth :
        case ocTrend :
        case ocRKP :
        case ocRGP :
        case ocFrequency :
        case ocMatTrans :
        case ocMatMult :
        case ocMatInv :
        case ocMatrixUnit :
            return sal_True;
        default:
        {
            // added to avoid warnings
        }
    }
    return sal_False;
}

// -----------------------------------------------------------------------------
FormulaCompiler::OpCodeMap::~OpCodeMap()
{
    delete mpReverseExternalHashMap;
    delete mpExternalHashMap;
    delete [] mpTable;
    delete mpHashMap;
}
// -----------------------------------------------------------------------------
void FormulaCompiler::OpCodeMap::copyFrom( const OpCodeMap& r )
{
    delete mpHashMap;
    mpHashMap = new OpCodeHashMap(mnSymbols);

    sal_uInt16 n = r.getSymbolCount();
    for (sal_uInt16 i = 0; i < n; ++i)
    {
        OpCode eOp = OpCode(i);
        const String& rSymbol = r.getSymbol(eOp);
        putOpCode(rSymbol, eOp);
    }

    // TODO: maybe copy the external maps too?
}
// -----------------------------------------------------------------------------
sal_Int32 FormulaCompiler::OpCodeMap::getOpCodeUnknown()
{
    static const sal_Int32 kOpCodeUnknown = -1;
    return kOpCodeUnknown;
}
// -----------------------------------------------------------------------------
sal_Bool FormulaCompiler::GetToken()
{
    static const short nRecursionMax = 42;
    FormulaCompilerRecursionGuard aRecursionGuard( nRecursion );
    if ( nRecursion > nRecursionMax )
    {
        SetError( errStackOverflow );
        pToken = new FormulaByteToken( ocStop );
        return sal_False;
    }
    if ( bAutoCorrect && !pStack )
    {   // don't merge stacked subroutine code into entered formula
        aCorrectedFormula += aCorrectedSymbol;
        aCorrectedSymbol.Erase();
    }
    sal_Bool bStop = sal_False;
    if( pArr->GetCodeError() && !bIgnoreErrors )
        bStop = sal_True;
    else
    {
        short nWasColRowName;
        if ( pArr->nIndex
          && pArr->pCode[ pArr->nIndex-1 ]->GetOpCode() == ocColRowName )
             nWasColRowName = 1;
        else
             nWasColRowName = 0;
        pToken = pArr->Next();
        while( pToken && pToken->GetOpCode() == ocSpaces )
        {
            if ( nWasColRowName )
                nWasColRowName++;
            if ( bAutoCorrect && !pStack )
                CreateStringFromToken( aCorrectedFormula, pToken.get(), false );
            pToken = pArr->Next();
        }
        if ( bAutoCorrect && !pStack && pToken )
            CreateStringFromToken( aCorrectedSymbol, pToken.get(), false );
        if( !pToken )
        {
            if( pStack )
            {
                PopTokenArray();
                return GetToken();
            }
            else
                bStop = sal_True;
        }
        else
        {
            if ( nWasColRowName >= 2 && pToken->GetOpCode() == ocColRowName )
            {   // convert an ocSpaces to ocIntersect in RPN
                pToken = new FormulaByteToken( ocIntersect );
                pArr->nIndex--;     // we advanced to the second ocColRowName, step back
            }
        }
    }
    if( bStop )
    {
        pToken = new FormulaByteToken( ocStop );
        return sal_False;
    }
    if( pToken->GetOpCode() == ocSubTotal )
        glSubTotal = sal_True;
    else if ( pToken->IsExternalRef() )
    {
        return HandleExternalReference(*pToken);
    }
    else if( pToken->GetOpCode() == ocName )
    {
        return HandleRange();
    }
    else if( pToken->GetOpCode() == ocColRowName )
    {
        return HandleSingleRef();
    }
    else if( pToken->GetOpCode() == ocDBArea )
    {
        return HandleDbData();
    }
    else if( pToken->GetType() == svSingleRef )
    {
        pArr->nRefs++;
    }
    else if( pToken->GetType() == svDoubleRef )
    {
        pArr->nRefs++;
    }
    return sal_True;
}
//---------------------------------------------------------------------------
// RPN creation by recursion
//---------------------------------------------------------------------------

void FormulaCompiler::Factor()
{
    if ( pArr->GetCodeError() && !bIgnoreErrors )
        return;

    CurrentFactor pFacToken( this );

    OpCode eOp = pToken->GetOpCode();
    if( eOp == ocPush || eOp == ocColRowNameAuto || eOp == ocMatRef ||
            eOp == ocDBArea
            || (bCompileForFAP && ((eOp == ocName) || (eOp == ocDBArea)
            || (eOp == ocColRowName) || (eOp == ocBad)))
        )
    {
        PutCode( pToken );
        eOp = NextToken();
        if( eOp == ocOpen )
        {
            // PUSH( is an error that may be caused by an unknown function.
            SetError(
                ( pToken->GetType() == svString
               || pToken->GetType() == svSingleRef )
               ? errNoName : errOperatorExpected );
            if ( bAutoCorrect && !pStack )
            {   // assume multiplication
                aCorrectedFormula += mxSymbols->getSymbol(ocMul);
                bCorrected = sal_True;
                NextToken();
                eOp = Expression();
                if( eOp != ocClose )
                    SetError(errPairExpected);
                else
                    eOp = NextToken();
            }
        }
    }
    else if( eOp == ocOpen )
    {
        NextToken();
        eOp = Expression();
        while ((eOp == ocSep) && (!pArr->GetCodeError() || bIgnoreErrors))
        {   // range list  (A1;A2)  converted to  (A1~A2)
            pFacToken = pToken;
            NextToken();
            eOp = Expression();
            // Do not ignore error here, regardless of bIgnoreErrors, otherwise
            // errors like =(1;) would also result in display of =(1~)
            if (!pArr->GetCodeError())
            {
                pFacToken->NewOpCode( ocUnion,FormulaToken::PrivateAccess());
                PutCode( pFacToken);
            }
        }
        if (eOp != ocClose)
            SetError(errPairExpected);
        else
            eOp = NextToken();
    }
    else
    {
        if( nNumFmt == NUMBERFORMAT_UNDEFINED )
            nNumFmt = lcl_GetRetFormat( eOp );

        if ( IsOpCodeVolatile(eOp) )
            pArr->SetRecalcModeAlways();
        else
        {
            switch( eOp )
            {
                    // Functions recalculated on every document load.
                    // Don't use SetRecalcModeOnLoad() which would override
                    // ModeAlways.
                case ocConvert :
                    pArr->AddRecalcMode( RECALCMODE_ONLOAD );
                break;
                    // If the referred cell is moved the value changes.
                case ocColumn :
                case ocRow :
                    // ocCell needs recalc on move for some possible type values.
                case ocCell :
                    pArr->SetRecalcModeOnRefMove();
                break;
                case ocHyperLink :
                pArr->SetHyperLink(sal_True);
                break;
                default:
                    ;   // nothing
            }
        }
        if (SC_OPCODE_START_NO_PAR <= eOp && eOp < SC_OPCODE_STOP_NO_PAR)
        {
            pFacToken = pToken;
            eOp = NextToken();
            if (eOp != ocOpen)
            {
                SetError(errPairExpected);
                PutCode( pFacToken );
            }
            else
            {
                eOp = NextToken();
                if (eOp != ocClose)
                    SetError(errPairExpected);
                PutCode(pFacToken);
                eOp = NextToken();
            }
        }
        // special cases NOT() and NEG()
        else if( eOp == ocNot || eOp == ocNeg
              || (SC_OPCODE_START_1_PAR <= eOp && eOp < SC_OPCODE_STOP_1_PAR) )
        {
            pFacToken = pToken;
            eOp = NextToken();
            if( nNumFmt == NUMBERFORMAT_UNDEFINED && eOp == ocNot )
                nNumFmt = NUMBERFORMAT_LOGICAL;
            if (eOp == ocOpen)
            {
                NextToken();
                eOp = Expression();
            }
            else
                SetError(errPairExpected);
            if (eOp != ocClose)
                SetError(errPairExpected);
            else if ( !pArr->GetCodeError() )
                pFacToken->SetByte( 1 );
            PutCode( pFacToken );
            eOp = NextToken();
        }
        else if ((SC_OPCODE_START_2_PAR <= eOp && eOp < SC_OPCODE_STOP_2_PAR)
                || eOp == ocExternal
                || eOp == ocMacro
                || eOp == ocAnd
                || eOp == ocOr
                || eOp == ocBad
                || ( eOp >= ocInternalBegin && eOp <= ocInternalEnd )
                || (bCompileForFAP && ((eOp == ocIf) || (eOp == ocChose)))
            )
        {
            pFacToken = pToken;
            OpCode eMyLastOp = eOp;
            eOp = NextToken();
            bool bNoParam = false;
            bool bBadName = false;
            if (eOp == ocOpen)
            {
                eOp = NextToken();
                if (eOp == ocClose)
                    bNoParam = true;
                else
                    eOp = Expression();
            }
            else if (eMyLastOp == ocBad)
            {
                // Just a bad name, not an unknown function, no parameters, no
                // closing expected.
                bBadName = true;
                bNoParam = true;
            }
            else
                SetError(errPairExpected);
            sal_uInt8 nSepCount = 0;
            if( !bNoParam )
            {
                nSepCount++;
                while ( (eOp == ocSep) && (!pArr->GetCodeError() || bIgnoreErrors) )
                {
                    nSepCount++;
                    NextToken();
                    eOp = Expression();
                }
            }
            if (bBadName)
                ;   // nothing, keep current token for return
            else if (eOp != ocClose)
                SetError(errPairExpected);
            else
                eOp = NextToken();
            // Jumps are just normal functions for the FunctionAutoPilot tree view
            if ( bCompileForFAP && pFacToken->GetType() == svJump )
                pFacToken = new FormulaFAPToken( pFacToken->GetOpCode(), nSepCount, pFacToken );
            else
                pFacToken->SetByte( nSepCount );
            PutCode( pFacToken );
        }
        else if (eOp == ocIf || eOp == ocChose)
        {
            // the PC counters are -1
            pFacToken = pToken;
            if ( eOp == ocIf )
                pFacToken->GetJump()[ 0 ] = 3;  // if, else, behind
            else
                pFacToken->GetJump()[ 0 ] = MAXJUMPCOUNT+1;
            eOp = NextToken();
            if (eOp == ocOpen)
            {
                NextToken();
                eOp = Expression();
            }
            else
                SetError(errPairExpected);
            short nJumpCount = 0;
            PutCode( pFacToken );
            // during AutoCorrect (since pArr->GetCodeError() is
            // ignored) an unlimited ocIf would crash because
            // ScRawToken::Clone() allocates the JumpBuffer according to
            // nJump[0]*2+2, which is 3*2+2 on ocIf.
            const short nJumpMax =
                (pFacToken->GetOpCode() == ocIf ? 3 : MAXJUMPCOUNT);
            while ( (nJumpCount < (MAXJUMPCOUNT - 1)) && (eOp == ocSep)
                    && (!pArr->GetCodeError() || bIgnoreErrors) )
            {
                if ( ++nJumpCount <= nJumpMax )
                    pFacToken->GetJump()[nJumpCount] = pc-1;
                NextToken();
                eOp = Expression();
                // ocSep or ocClose terminate the subexpression
                PutCode( pToken );
            }
            if (eOp != ocClose)
                SetError(errPairExpected);
            else
            {
                eOp = NextToken();
                // always limit to nJumpMax, no arbitrary overwrites
                if ( ++nJumpCount <= nJumpMax )
                    pFacToken->GetJump()[ nJumpCount ] = pc-1;
                if ((pFacToken->GetOpCode() == ocIf && (nJumpCount > 3)) ||
                                 (nJumpCount >= MAXJUMPCOUNT))
                    SetError(errIllegalParameter);
                else
                    pFacToken->GetJump()[ 0 ] = nJumpCount;
            }
        }
        else if ( eOp == ocMissing )
        {
            PutCode( pToken );
            eOp = NextToken();
        }
        else if ( eOp == ocClose )
        {
            SetError( errParameterExpected );
        }
        else if ( eOp == ocSep )
        {   // Subsequent ocSep
            SetError( errParameterExpected );
            if ( bAutoCorrect && !pStack )
            {
                aCorrectedSymbol.Erase();
                bCorrected = sal_True;
            }
        }
        else if ( pToken->IsExternalRef() )
        {
            PutCode(pToken);
            eOp = NextToken();
        }
        else
        {
            SetError( errUnknownToken );
            if ( bAutoCorrect && !pStack )
            {
                if ( eOp == ocStop )
                {   // trailing operator w/o operand
                    xub_StrLen nLen = aCorrectedFormula.Len();
                    if ( nLen )
                        aCorrectedFormula.Erase( nLen - 1 );
                    aCorrectedSymbol.Erase();
                    bCorrected = sal_True;
                }
            }
        }
    }
}

//---------------------------------------------------------------------------

void FormulaCompiler::RangeLine()
{
    Factor();
    while (pToken->GetOpCode() == ocRange)
    {
        FormulaToken** pCode1 = pCode - 1;
        FormulaTokenRef p = pToken;
        NextToken();
        Factor();
        FormulaToken** pCode2 = pCode - 1;
        if (!MergeRangeReference( pCode1, pCode2))
            PutCode(p);
    }
}

//---------------------------------------------------------------------------

void FormulaCompiler::IntersectionLine()
{
    RangeLine();
    while (pToken->GetOpCode() == ocIntersect)
    {
        FormulaTokenRef p = pToken;
        NextToken();
        RangeLine();
        PutCode(p);
    }
}

//---------------------------------------------------------------------------

void FormulaCompiler::UnionLine()
{
    IntersectionLine();
    while (pToken->GetOpCode() == ocUnion)
    {
        FormulaTokenRef p = pToken;
        NextToken();
        IntersectionLine();
        PutCode(p);
    }
}

//---------------------------------------------------------------------------

void FormulaCompiler::UnaryLine()
{
    if( pToken->GetOpCode() == ocAdd )
        GetToken();
    else if (SC_OPCODE_START_UN_OP <= pToken->GetOpCode() &&
            pToken->GetOpCode() < SC_OPCODE_STOP_UN_OP)
    {
        FormulaTokenRef p = pToken;
        NextToken();
        UnaryLine();
        PutCode( p );
    }
    else
        UnionLine();
}

//---------------------------------------------------------------------------

void FormulaCompiler::PostOpLine()
{
    UnaryLine();
    while ( pToken->GetOpCode() == ocPercentSign )
    {   // this operator _follows_ its operand
        PutCode( pToken );
        NextToken();
    }
}

//---------------------------------------------------------------------------

void FormulaCompiler::PowLine()
{
    PostOpLine();
    while (pToken->GetOpCode() == ocPow)
    {
        FormulaTokenRef p = pToken;
        NextToken();
        PostOpLine();
        PutCode(p);
    }
}

//---------------------------------------------------------------------------

void FormulaCompiler::MulDivLine()
{
    PowLine();
    while (pToken->GetOpCode() == ocMul || pToken->GetOpCode() == ocDiv)
    {
        FormulaTokenRef p = pToken;
        NextToken();
        PowLine();
        PutCode(p);
    }
}

//---------------------------------------------------------------------------

void FormulaCompiler::AddSubLine()
{
    MulDivLine();
    while (pToken->GetOpCode() == ocAdd || pToken->GetOpCode() == ocSub)
    {
        FormulaTokenRef p = pToken;
        NextToken();
        MulDivLine();
        PutCode(p);
    }
}

//---------------------------------------------------------------------------

void FormulaCompiler::ConcatLine()
{
    AddSubLine();
    while (pToken->GetOpCode() == ocAmpersand)
    {
        FormulaTokenRef p = pToken;
        NextToken();
        AddSubLine();
        PutCode(p);
    }
}

//---------------------------------------------------------------------------

void FormulaCompiler::CompareLine()
{
    ConcatLine();
    while (pToken->GetOpCode() >= ocEqual && pToken->GetOpCode() <= ocGreaterEqual)
    {
        FormulaTokenRef p = pToken;
        NextToken();
        ConcatLine();
        PutCode(p);
    }
}

//---------------------------------------------------------------------------

void FormulaCompiler::NotLine()
{
    CompareLine();
    while (pToken->GetOpCode() == ocNot)
    {
        FormulaTokenRef p = pToken;
        NextToken();
        CompareLine();
        PutCode(p);
    }
}

//---------------------------------------------------------------------------

OpCode FormulaCompiler::Expression()
{
    static const short nRecursionMax = 42;
    FormulaCompilerRecursionGuard aRecursionGuard( nRecursion );
    if ( nRecursion > nRecursionMax )
    {
        SetError( errStackOverflow );
        return ocStop;      //! generate token instead?
    }
    NotLine();
    while (pToken->GetOpCode() == ocAnd || pToken->GetOpCode() == ocOr)
    {
        FormulaTokenRef p = pToken;
        pToken->SetByte( 2 );       // 2 parameters!
        NextToken();
        NotLine();
        PutCode(p);
    }
    return pToken->GetOpCode();
}
// -----------------------------------------------------------------------------
void FormulaCompiler::SetError(sal_uInt16 /*nError*/)
{
}
// -----------------------------------------------------------------------------
FormulaTokenRef FormulaCompiler::ExtendRangeReference( FormulaToken & /*rTok1*/, FormulaToken & /*rTok2*/, bool /*bReuseDoubleRef*/ )
{
    return FormulaTokenRef();
}
// -----------------------------------------------------------------------------
bool FormulaCompiler::MergeRangeReference(FormulaToken * * const pCode1, FormulaToken * const * const pCode2 )
{
    FormulaToken *p1, *p2;
    if (pc < 2 || !pCode1 || !pCode2 ||
            (pCode2 - pCode1 != 1) || (pCode - pCode2 != 1) ||
            ((p1 = *pCode1) == 0) || ((p2 = *pCode2) == 0) )
        return false;

    FormulaTokenRef p = ExtendRangeReference( *p1, *p2, true);
    if (!p)
        return false;

    p->IncRef();
    p1->DecRef();
    p2->DecRef();
    *pCode1 = p.get();
    --pCode, --pc;
    pArr->nRefs--;

    return true;
}
// -----------------------------------------------------------------------------
sal_Bool FormulaCompiler::CompileTokenArray()
{
    glSubTotal = sal_False;
    bCorrected = sal_False;
    if( !pArr->GetCodeError() || bIgnoreErrors )
    {
        if ( bAutoCorrect )
        {
            aCorrectedFormula.Erase();
            aCorrectedSymbol.Erase();
        }
        pArr->nRefs = 0;    // count from start
        pArr->DelRPN();
        pStack = NULL;
        FormulaToken* pData[ MAXCODE ];
        pCode = pData;
        sal_Bool bWasForced = pArr->IsRecalcModeForced();
        if ( bWasForced )
        {
            if ( bAutoCorrect )
                aCorrectedFormula = '=';
        }
        pArr->ClearRecalcMode();
        pArr->Reset();
        eLastOp = ocOpen;
        pc = 0;
        NextToken();
        OpCode eOp = Expression();
        // Some trailing garbage that doesn't form an expression?
        if (eOp != ocStop)
            SetError( errOperatorExpected);

        sal_uInt16 nErrorBeforePop = pArr->GetCodeError();

        while( pStack )
            PopTokenArray();
        if( pc )
        {
            pArr->pRPN = new FormulaToken*[ pc ];
            pArr->nRPN = pc;
            memcpy( pArr->pRPN, pData, pc * sizeof( FormulaToken* ) );
        }

        // once an error, always an error
        if( !pArr->GetCodeError() && nErrorBeforePop )
            pArr->SetCodeError( nErrorBeforePop);

        if( pArr->GetCodeError() && !bIgnoreErrors )
        {
            pArr->DelRPN();
            pArr->SetHyperLink(sal_False);
        }

        if ( bWasForced )
            pArr->SetRecalcModeForced();
    }
    if( nNumFmt == NUMBERFORMAT_UNDEFINED )
        nNumFmt = NUMBERFORMAT_NUMBER;
    return glSubTotal;
}
// -----------------------------------------------------------------------------
void FormulaCompiler::PopTokenArray()
{
    if( pStack )
    {
        FormulaArrayStack* p = pStack;
        pStack = p->pNext;
        p->pArr->nRefs = sal::static_int_cast<short>( p->pArr->nRefs + pArr->nRefs );
        // obtain special RecalcMode from SharedFormula
        if ( pArr->IsRecalcModeAlways() )
            p->pArr->SetRecalcModeAlways();
        else if ( !pArr->IsRecalcModeNormal() && p->pArr->IsRecalcModeNormal() )
            p->pArr->SetMaskedRecalcMode( pArr->GetRecalcMode() );
        p->pArr->SetCombinedBitsRecalcMode( pArr->GetRecalcMode() );
        if( p->bTemp )
            delete pArr;
        pArr = p->pArr;
        delete p;
    }
}
// -----------------------------------------------------------------------------
void FormulaCompiler::CreateStringFromTokenArray( String& rFormula )
{
    rtl::OUStringBuffer aBuffer( pArr->GetLen() * 5 );
    CreateStringFromTokenArray( aBuffer );
    rFormula = aBuffer;
}

void FormulaCompiler::CreateStringFromTokenArray( rtl::OUStringBuffer& rBuffer )
{
    rBuffer.setLength(0);
    if( !pArr->GetLen() )
        return;

    FormulaTokenArray* pSaveArr = pArr;
    bool bODFF = FormulaGrammar::isODFF( meGrammar);
    if (bODFF || FormulaGrammar::isPODF( meGrammar) )
    {
        // Scan token array for missing args and re-write if present.
        MissingConvention aConv( bODFF);
        if (pArr->NeedsPofRewrite( aConv))
            pArr = pArr->RewriteMissingToPof( aConv);
    }

    // At least one character per token, plus some are references, some are
    // function names, some are numbers, ...
    rBuffer.ensureCapacity( pArr->GetLen() * 5 );

    if ( pArr->IsRecalcModeForced() )
        rBuffer.append(sal_Unicode('='));
    FormulaToken* t = pArr->First();
    while( t )
        t = CreateStringFromToken( rBuffer, t, sal_True );

    if (pSaveArr != pArr)
    {
        delete pArr;
        pArr = pSaveArr;
    }
}
// -----------------------------------------------------------------------------
FormulaToken* FormulaCompiler::CreateStringFromToken( String& rFormula, FormulaToken* pTokenP,sal_Bool bAllowArrAdvance )
{
    rtl::OUStringBuffer aBuffer;
    FormulaToken* p = CreateStringFromToken( aBuffer, pTokenP, bAllowArrAdvance );
    rFormula += aBuffer;
    return p;
}

FormulaToken* FormulaCompiler::CreateStringFromToken( rtl::OUStringBuffer& rBuffer, FormulaToken* pTokenP,sal_Bool bAllowArrAdvance )
{
    sal_Bool bNext = sal_True;
    sal_Bool bSpaces = sal_False;
    FormulaToken* t = pTokenP;
    OpCode eOp = t->GetOpCode();
    if( eOp >= ocAnd && eOp <= ocOr )
    {
        // AND, OR infix?
        if ( bAllowArrAdvance )
            t = pArr->Next();
        else
            t = pArr->PeekNext();
        bNext = sal_False;
        bSpaces = ( !t || t->GetOpCode() != ocOpen );
    }
    if( bSpaces )
        rBuffer.append(sal_Unicode(' '));

    if( eOp == ocSpaces )
    {
        bool bIntersectionOp = mxSymbols->isODFF();
        if (bIntersectionOp)
        {
            const FormulaToken* p = pArr->PeekPrevNoSpaces();
            bIntersectionOp = (p && p->GetOpCode() == ocColRowName);
            if (bIntersectionOp)
            {
                p = pArr->PeekNextNoSpaces();
                bIntersectionOp = (p && p->GetOpCode() == ocColRowName);
            }
        }
        if (bIntersectionOp)
            rBuffer.appendAscii( "!!");
        else
        {
            // most times it's just one blank
            sal_uInt8 n = t->GetByte();
            for ( sal_uInt8 j=0; j<n; ++j )
            {
                rBuffer.append(sal_Unicode(' '));
            }
        }
    }
    else if( eOp >= ocInternalBegin && eOp <= ocInternalEnd )
        rBuffer.appendAscii( pInternal[ eOp - ocInternalBegin ] );
    else if( (sal_uInt16) eOp < mxSymbols->getSymbolCount())        // Keyword:
        rBuffer.append(mxSymbols->getSymbol(eOp));
    else
    {
        DBG_ERRORFILE("unknown OpCode");
        rBuffer.append(GetNativeSymbol( ocErrName ));
    }
    if( bNext )
    {
        if (t->IsExternalRef())
        {
            CreateStringFromExternal(rBuffer, pTokenP);
        }
        else
        {
            switch( t->GetType() )
            {
            case svDouble:
                AppendDouble( rBuffer, t->GetDouble() );
            break;

            case svString:
                if( eOp == ocBad )
                    rBuffer.append(t->GetString());
                else
                    AppendString( rBuffer, t->GetString() );
                break;
            case svSingleRef:
                CreateStringFromSingleRef(rBuffer,t);
                break;
            case svDoubleRef:
                CreateStringFromDoubleRef(rBuffer,t);
                break;
            case svMatrix:
                CreateStringFromMatrix( rBuffer, t );
                break;

            case svIndex:
                CreateStringFromIndex( rBuffer, t );
                break;
            case svExternal:
            {
                // mapped or translated name of AddIns
                String aAddIn( t->GetExternal() );
                bool bMapped = mxSymbols->isPODF();     // ODF 1.1 directly uses programmatical name
                if (!bMapped && mxSymbols->hasExternals())
                {
                    ExternalHashMap::const_iterator iLook = mxSymbols->getReverseExternalHashMap()->find( aAddIn);
                    if (iLook != mxSymbols->getReverseExternalHashMap()->end())
                    {
                        aAddIn = (*iLook).second;
                        bMapped = true;
                    }
                }
                if (!bMapped && !mxSymbols->isEnglish())
                        LocalizeString( aAddIn );
                    rBuffer.append(aAddIn);
                }
            break;
            case svByte:
            case svJump:
            case svFAP:
            case svMissing:
            case svSep:
                break;      // Opcodes
            default:
                OSL_FAIL("FormulaCompiler:: GetStringFromToken errUnknownVariable");
            } // of switch
        }
    }
    if( bSpaces )
        rBuffer.append(sal_Unicode(' '));
    if ( bAllowArrAdvance )
    {
        if( bNext )
            t = pArr->Next();
        return t;
    }
    return pTokenP;
}
// -----------------------------------------------------------------------------

void FormulaCompiler::AppendDouble( rtl::OUStringBuffer& rBuffer, double fVal )
{
    if ( mxSymbols->isEnglish() )
    {
        ::rtl::math::doubleToUStringBuffer( rBuffer, fVal,
                rtl_math_StringFormat_Automatic,
                rtl_math_DecimalPlaces_Max, '.', sal_True );
    }
    else
    {
        SvtSysLocale aSysLocale;
        ::rtl::math::doubleToUStringBuffer( rBuffer, fVal,
                rtl_math_StringFormat_Automatic,
                rtl_math_DecimalPlaces_Max,
                aSysLocale.GetLocaleDataPtr()->getNumDecimalSep().GetChar(0),
                sal_True );
    }
}
// -----------------------------------------------------------------------------
void FormulaCompiler::AppendBoolean( rtl::OUStringBuffer& rBuffer, bool bVal )
{
    rBuffer.append( mxSymbols->getSymbol(static_cast<OpCode>(bVal ? ocTrue : ocFalse)) );
}
// -----------------------------------------------------------------------------
sal_Bool FormulaCompiler::IsImportingXML() const
{
    return sal_False;
}
// -----------------------------------------------------------------------------
void FormulaCompiler::AppendString( rtl::OUStringBuffer& rBuffer, const String & rStr )
{
    if (IsImportingXML())
        rBuffer.append( rStr );
    else
    {
        rBuffer.append(sal_Unicode('"'));
        if ( lcl_UnicodeStrChr( rStr.GetBuffer(), '"' ) == NULL )
            rBuffer.append( rStr );
        else
        {
            String aStr( rStr );
            aStr.SearchAndReplaceAll( '"', String( RTL_CONSTASCII_USTRINGPARAM( "\"\"")));
            rBuffer.append(aStr);
        }
        rBuffer.append(sal_Unicode('"'));
    }
}

void FormulaCompiler::UpdateSeparatorsNative(
    const rtl::OUString& rSep, const rtl::OUString& rArrayColSep, const rtl::OUString& rArrayRowSep )
{
    NonConstOpCodeMapPtr xSymbolsNative;
    lcl_fillNativeSymbols(xSymbolsNative);
    xSymbolsNative->putOpCode(rSep, ocSep);
    xSymbolsNative->putOpCode(rArrayColSep, ocArrayColSep);
    xSymbolsNative->putOpCode(rArrayRowSep, ocArrayRowSep);
}

void FormulaCompiler::ResetNativeSymbols()
{
    NonConstOpCodeMapPtr xSymbolsNative;
    lcl_fillNativeSymbols(xSymbolsNative, true);
    lcl_fillNativeSymbols(xSymbolsNative);
}

void FormulaCompiler::SetNativeSymbols( const OpCodeMapPtr& xMap )
{
    NonConstOpCodeMapPtr xSymbolsNative;
    lcl_fillNativeSymbols(xSymbolsNative);
    xSymbolsNative->copyFrom(*xMap);
}

// -----------------------------------------------------------------------------
OpCode FormulaCompiler::NextToken()
{
    if( !GetToken() )
        return ocStop;
    OpCode eOp = pToken->GetOpCode();
    // There must be an operator before a push
    if ( (eOp == ocPush || eOp == ocColRowNameAuto) &&
            !( (eLastOp == ocOpen) || (eLastOp == ocSep) ||
                (SC_OPCODE_START_BIN_OP <= eLastOp && eLastOp < SC_OPCODE_STOP_UN_OP)) )
        SetError(errOperatorExpected);
    // Operator and Plus => operator
    if (eOp == ocAdd && (eLastOp == ocOpen || eLastOp == ocSep ||
                (SC_OPCODE_START_BIN_OP <= eLastOp && eLastOp < SC_OPCODE_STOP_UN_OP)))
        eOp = NextToken();
    else
    {
        // Before an operator there must not be another operator, with the
        // exception of AND and OR.
        if ( eOp != ocAnd && eOp != ocOr &&
                (SC_OPCODE_START_BIN_OP <= eOp && eOp < SC_OPCODE_STOP_BIN_OP )
                && (eLastOp == ocOpen || eLastOp == ocSep ||
                    (SC_OPCODE_START_BIN_OP <= eLastOp && eLastOp < SC_OPCODE_STOP_UN_OP)))
        {
            SetError(errVariableExpected);
            if ( bAutoCorrect && !pStack )
            {
                if ( eOp == eLastOp || eLastOp == ocOpen )
                {   // throw away duplicated operator
                    aCorrectedSymbol.Erase();
                    bCorrected = sal_True;
                }
                else
                {
                    xub_StrLen nPos = aCorrectedFormula.Len();
                    if ( nPos )
                    {
                        nPos--;
                        sal_Unicode c = aCorrectedFormula.GetChar( nPos );
                        switch ( eOp )
                        {   // swap operators
                            case ocGreater:
                                if ( c == mxSymbols->getSymbol(ocEqual).GetChar(0) )
                                {   // >= instead of =>
                                    aCorrectedFormula.SetChar( nPos,
                                        mxSymbols->getSymbol(ocGreater).GetChar(0) );
                                    aCorrectedSymbol = c;
                                    bCorrected = sal_True;
                                }
                            break;
                            case ocLess:
                                if ( c == mxSymbols->getSymbol(ocEqual).GetChar(0) )
                                {   // <= instead of =<
                                    aCorrectedFormula.SetChar( nPos,
                                        mxSymbols->getSymbol(ocLess).GetChar(0) );
                                    aCorrectedSymbol = c;
                                    bCorrected = sal_True;
                                }
                                else if ( c == mxSymbols->getSymbol(ocGreater).GetChar(0) )
                                {   // <> instead of ><
                                    aCorrectedFormula.SetChar( nPos,
                                        mxSymbols->getSymbol(ocLess).GetChar(0) );
                                    aCorrectedSymbol = c;
                                    bCorrected = sal_True;
                                }
                            break;
                            case ocMul:
                                if ( c == mxSymbols->getSymbol(ocSub).GetChar(0) )
                                {   // *- instead of -*
                                    aCorrectedFormula.SetChar( nPos,
                                        mxSymbols->getSymbol(ocMul).GetChar(0) );
                                    aCorrectedSymbol = c;
                                    bCorrected = sal_True;
                                }
                            break;
                            case ocDiv:
                                if ( c == mxSymbols->getSymbol(ocSub).GetChar(0) )
                                {   // /- instead of -/
                                    aCorrectedFormula.SetChar( nPos,
                                        mxSymbols->getSymbol(ocDiv).GetChar(0) );
                                    aCorrectedSymbol = c;
                                    bCorrected = sal_True;
                                }
                            break;
                            default:
                                ;   // nothing
                        }
                    }
                }
            }
        }
        eLastOp = eOp;
    }
    return eOp;
}
void FormulaCompiler::PutCode( FormulaTokenRef& p )
{
    if( pc >= MAXCODE-1 )
    {
        if ( pc == MAXCODE-1 )
        {
            p = new FormulaByteToken( ocStop );
            p->IncRef();
            *pCode++ = p.get();
            ++pc;
        }
        SetError(errCodeOverflow);
        return;
    }
    if( pArr->GetCodeError() && !bCompileForFAP )
        return;
    ForceArrayOperator( p, pCurrentFactorToken);
    p->IncRef();
    *pCode++ = p.get();
    pc++;
}

// -----------------------------------------------------------------------------
sal_Bool FormulaCompiler::HandleExternalReference(const FormulaToken& /*_aToken*/)
{
    return sal_True;
}
// -----------------------------------------------------------------------------
sal_Bool FormulaCompiler::HandleRange()
{
    return sal_True;
}
// -----------------------------------------------------------------------------
sal_Bool FormulaCompiler::HandleSingleRef()
{
    return sal_True;
}
// -----------------------------------------------------------------------------
sal_Bool FormulaCompiler::HandleDbData()
{
    return sal_True;
}
// -----------------------------------------------------------------------------
void FormulaCompiler::CreateStringFromSingleRef(rtl::OUStringBuffer& /*rBuffer*/,FormulaToken* /*pTokenP*/)
{
}
// -----------------------------------------------------------------------------
void FormulaCompiler::CreateStringFromDoubleRef(rtl::OUStringBuffer& /*rBuffer*/,FormulaToken* /*pTokenP*/)
{
}
// -----------------------------------------------------------------------------
void FormulaCompiler::CreateStringFromIndex(rtl::OUStringBuffer& /*rBuffer*/,FormulaToken* /*pTokenP*/)
{
}
// -----------------------------------------------------------------------------
void FormulaCompiler::CreateStringFromMatrix(rtl::OUStringBuffer& /*rBuffer*/,FormulaToken* /*pTokenP*/)
{
}
// -----------------------------------------------------------------------------
void FormulaCompiler::CreateStringFromExternal(rtl::OUStringBuffer& /*rBuffer*/,FormulaToken* /*pTokenP*/)
{
}
// -----------------------------------------------------------------------------
void FormulaCompiler::LocalizeString( String& /*rName*/ )
{
}
void FormulaCompiler::PushTokenArray( FormulaTokenArray* pa, sal_Bool bTemp )
{
    if ( bAutoCorrect && !pStack )
        {   // don't merge stacked subroutine code into entered formula
        aCorrectedFormula += aCorrectedSymbol;
        aCorrectedSymbol.Erase();
    }
    FormulaArrayStack* p = new FormulaArrayStack;
    p->pNext      = pStack;
    p->pArr       = pArr;
    p->bTemp      = bTemp;
    pStack        = p;
    pArr          = pa;
}

// =============================================================================
} // formula
// =============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

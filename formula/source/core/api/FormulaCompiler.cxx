/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */
#include <sal/macros.h>
#include "formula/FormulaCompiler.hxx"
#include "formula/errorcodes.hxx"
#include "formula/token.hxx"
#include "formula/tokenarray.hxx"
#include "core_resource.hxx"
#include "core_resource.hrc"

#include "osl/mutex.hxx"

#include <svl/zforlist.hxx>
#include <tools/rc.hxx>
#include <tools/rcid.h>
#include <com/sun/star/sheet/FormulaOpCodeMapEntry.hpp>
#include <com/sun/star/sheet/FormulaMapGroup.hpp>
#include <com/sun/star/sheet/FormulaMapGroupSpecialOffset.hpp>
#include <rtl/strbuf.hxx>
#include <stdio.h>

namespace formula
{
    using namespace ::com::sun::star;

    static const sal_Char* pInternal[2] = { "TTT", "__DEBUG_VAR" };

namespace {

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
        case ocXor:
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
}

inline void lclPushOpCodeMapEntry( ::std::vector< sheet::FormulaOpCodeMapEntry >& rVec,
        const OUString* pTable, sal_uInt16 nOpCode )
{
    sheet::FormulaOpCodeMapEntry aEntry;
    aEntry.Token.OpCode = nOpCode;
    aEntry.Name = pTable[nOpCode];
    rVec.push_back( aEntry);
}

void lclPushOpCodeMapEntries( ::std::vector< sheet::FormulaOpCodeMapEntry >& rVec,
        const OUString* pTable, sal_uInt16 nOpCodeBeg, sal_uInt16 nOpCodeEnd )
{
    for (sal_uInt16 nOpCode = nOpCodeBeg; nOpCode < nOpCodeEnd; ++nOpCode)
        lclPushOpCodeMapEntry( rVec, pTable, nOpCode );
}

void lclPushOpCodeMapEntries( ::std::vector< sheet::FormulaOpCodeMapEntry >& rVec,
        const OUString* pTable, const sal_uInt16* pnOpCodes, size_t nCount )
{
    for (const sal_uInt16* pnEnd = pnOpCodes + nCount; pnOpCodes < pnEnd; ++pnOpCodes)
        lclPushOpCodeMapEntry( rVec, pTable, *pnOpCodes );
}

class OpCodeList : public Resource        
{
public:

    OpCodeList( sal_uInt16, FormulaCompiler::NonConstOpCodeMapPtr,
            FormulaCompiler::SeparatorType = FormulaCompiler::SEMICOLON_BASE );

private:
    bool getOpCodeString( OUString& rStr, sal_uInt16 nOp );
    void putDefaultOpCode( FormulaCompiler::NonConstOpCodeMapPtr xMap, sal_uInt16 nOp );

private:
    FormulaCompiler::SeparatorType meSepType;
};

OpCodeList::OpCodeList( sal_uInt16 nRID, FormulaCompiler::NonConstOpCodeMapPtr xMap,
        FormulaCompiler::SeparatorType eSepType ) :
    Resource( ResId( nRID, *ResourceManager::getResManager()))
    , meSepType( eSepType)
{
    if (meSepType == FormulaCompiler::RESOURCE_BASE)
    {
        for (sal_uInt16 i = 0; i <= SC_OPCODE_LAST_OPCODE_ID; ++i)
        {
            putDefaultOpCode( xMap, i);
        }
    }
    else
    {
        for (sal_uInt16 i = 0; i <= SC_OPCODE_LAST_OPCODE_ID; ++i)
        {
            OUString aOpStr;
            if ( getOpCodeString( aOpStr, i) )
                xMap->putOpCode( aOpStr, OpCode(i));
            else
                putDefaultOpCode( xMap, i);
        }
    }

    FreeResource();
}

bool OpCodeList::getOpCodeString( OUString& rStr, sal_uInt16 nOp )
{
    switch (nOp)
    {
        case SC_OPCODE_SEP:
        {
            if (meSepType == FormulaCompiler::COMMA_BASE)
            {
                rStr = ",";
                return true;
            }
            else if (meSepType == FormulaCompiler::SEMICOLON_BASE)
            {
                rStr = ";";
                return true;
            }
        }
        break;
        case SC_OPCODE_ARRAY_COL_SEP:
        {
            if (meSepType == FormulaCompiler::COMMA_BASE)
            {
                rStr = ",";
                return true;
            }
            else if (meSepType == FormulaCompiler::SEMICOLON_BASE)
            {
                rStr = ";";
                return true;
            }
        }
        break;
        case SC_OPCODE_ARRAY_ROW_SEP:
        {
            if (meSepType == FormulaCompiler::COMMA_BASE)
            {
                rStr = ";";
                return true;
            }
            else if (meSepType == FormulaCompiler::SEMICOLON_BASE)
            {
                rStr = "|";
                return true;
            }
        }
        break;
    }

    return false;
}

void OpCodeList::putDefaultOpCode( FormulaCompiler::NonConstOpCodeMapPtr xMap, sal_uInt16 nOp )
{
    ResId aRes( nOp, *ResourceManager::getResManager());
    aRes.SetRT( RSC_STRING);
    if (IsAvailableRes( aRes))
        xMap->putOpCode( aRes.toString(), OpCode( nOp));
}


const sal_Unicode* lcl_UnicodeStrChr( const sal_Unicode* pStr, sal_Unicode c )
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

struct OpCodeMapData
{
    FormulaCompiler::NonConstOpCodeMapPtr mxSymbolMap;
    osl::Mutex maMtx;
};


} 


void FormulaCompiler::OpCodeMap::putExternal( const OUString & rSymbol, const OUString & rAddIn )
{
    
    
    
    
    bool bOk = mpExternalHashMap->insert( ExternalHashMap::value_type( rSymbol, rAddIn)).second;
    SAL_WARN_IF( !bOk, "formula.core", "OpCodeMap::putExternal: symbol not inserted, " << rSymbol << " -> " << rAddIn);
    if (bOk)
    {
        bOk = mpReverseExternalHashMap->insert( ExternalHashMap::value_type( rAddIn, rSymbol)).second;
        
        
        SAL_INFO_IF( !bOk, "formula.core", "OpCodeMap::putExternal: AddIn not inserted, " << rAddIn << " -> " << rSymbol);
    }
}

void FormulaCompiler::OpCodeMap::putExternalSoftly( const OUString & rSymbol, const OUString & rAddIn )
{
    bool bOk = mpReverseExternalHashMap->insert( ExternalHashMap::value_type( rAddIn, rSymbol)).second;
    if (bOk)
        mpExternalHashMap->insert( ExternalHashMap::value_type( rSymbol, rAddIn));
}

uno::Sequence< sheet::FormulaToken > FormulaCompiler::OpCodeMap::createSequenceOfFormulaTokens(
        const FormulaCompiler& rCompiler, const uno::Sequence< OUString >& rNames ) const
{
    const sal_Int32 nLen = rNames.getLength();
    uno::Sequence< sheet::FormulaToken > aTokens( nLen);
    sheet::FormulaToken* pToken = aTokens.getArray();
    OUString const * pName = rNames.getConstArray();
    OUString const * const pStop = pName + nLen;
    for ( ; pName < pStop; ++pName, ++pToken)
    {
        OpCodeHashMap::const_iterator iLook( mpHashMap->find( *pName));
        if (iLook != mpHashMap->end())
            pToken->OpCode = (*iLook).second;
        else
        {
            OUString aIntName;
            if (hasExternals())
            {
                ExternalHashMap::const_iterator iExt( mpExternalHashMap->find( *pName));
                if (iExt != mpExternalHashMap->end())
                    aIntName = (*iExt).second;
                
                
            }
            if (aIntName.isEmpty())
                aIntName = rCompiler.FindAddInFunction(*pName, !isEnglish());    
            if (aIntName.isEmpty())
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

uno::Sequence< sheet::FormulaOpCodeMapEntry > FormulaCompiler::OpCodeMap::createSequenceOfAvailableMappings(
        const FormulaCompiler& rCompiler, const sal_Int32 nGroups ) const
{
    using namespace sheet;

    
    
    
    ::std::vector< FormulaOpCodeMapEntry > aVec;

    if (nGroups == FormulaMapGroup::SPECIAL)
    {
        
        
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
        const size_t nCount = sizeof(aMap)/sizeof(aMap[0]);
        
        if (aVec.size() < nCount)
        {
            FormulaOpCodeMapEntry aEntry;
            aEntry.Token.OpCode = getOpCodeUnknown();
            aVec.resize( nCount, aEntry);
        } 

        FormulaOpCodeMapEntry aEntry;
        for (size_t i=0; i < nCount; ++i)
        {
            size_t nIndex = static_cast< size_t >( aMap[i].nOff );
            if (aVec.size() <= nIndex)
            {
                
                
                
                
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

        
        if ((nGroups & FormulaMapGroup::SEPARATORS) != 0)
        {
            static const sal_uInt16 aOpCodes[] = {
                SC_OPCODE_OPEN,
                SC_OPCODE_CLOSE,
                SC_OPCODE_SEP,
            };
            lclPushOpCodeMapEntries( aVec, mpTable, aOpCodes, sizeof(aOpCodes)/sizeof(aOpCodes[0]) );
        }
        if ((nGroups & FormulaMapGroup::ARRAY_SEPARATORS) != 0)
        {
            static const sal_uInt16 aOpCodes[] = {
                SC_OPCODE_ARRAY_OPEN,
                SC_OPCODE_ARRAY_CLOSE,
                SC_OPCODE_ARRAY_ROW_SEP,
                SC_OPCODE_ARRAY_COL_SEP
            };
            lclPushOpCodeMapEntries( aVec, mpTable, aOpCodes, sizeof(aOpCodes)/sizeof(aOpCodes[0]) );
        }
        if ((nGroups & FormulaMapGroup::UNARY_OPERATORS) != 0)
        {
            
            
            lclPushOpCodeMapEntry( aVec, mpTable, ocPercentSign );
            
            if ((nGroups & FormulaMapGroup::BINARY_OPERATORS) == 0)
                lclPushOpCodeMapEntry( aVec, mpTable, ocAdd );
            
            for (sal_uInt16 nOp = SC_OPCODE_START_UN_OP; nOp < SC_OPCODE_STOP_UN_OP && nOp < mnSymbols; ++nOp)
            {
                switch (nOp)
                {
                    
                    
                    case SC_OPCODE_NOT :
                    case SC_OPCODE_NEG :
                        break;   
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
                    
                    
                    case SC_OPCODE_AND :
                    case SC_OPCODE_OR :
                        break;   
                    default:
                        lclPushOpCodeMapEntry( aVec, mpTable, nOp );
                }
            }
        }
        if ((nGroups & FormulaMapGroup::FUNCTIONS) != 0)
        {
            
            
            lclPushOpCodeMapEntries( aVec, mpTable, SC_OPCODE_START_NO_PAR,
                    ::std::min< sal_uInt16 >( SC_OPCODE_STOP_NO_PAR, mnSymbols ) );
            lclPushOpCodeMapEntries( aVec, mpTable, SC_OPCODE_START_1_PAR,
                    ::std::min< sal_uInt16 >( SC_OPCODE_STOP_1_PAR, mnSymbols ) );
            
            static const sal_uInt16 aOpCodes[] = {
                SC_OPCODE_IF,
                SC_OPCODE_IF_ERROR,
                SC_OPCODE_IF_NA,
                SC_OPCODE_CHOSE,
                SC_OPCODE_AND,
                SC_OPCODE_OR,
                SC_OPCODE_NOT,
                SC_OPCODE_NEG
            };
            lclPushOpCodeMapEntries( aVec, mpTable, aOpCodes, sizeof(aOpCodes)/sizeof(aOpCodes[0]) );
            
            for (sal_uInt16 nOp = SC_OPCODE_START_2_PAR; nOp < SC_OPCODE_STOP_2_PAR && nOp < mnSymbols; ++nOp)
            {
                switch (nOp)
                {
                    
                    case SC_OPCODE_NO_NAME :
                        break;   
                    default:
                        lclPushOpCodeMapEntry( aVec, mpTable, nOp );
                }
            }
            
            if (hasExternals())
            {
                for (ExternalHashMap::const_iterator it( mpExternalHashMap->begin());it != mpExternalHashMap->end(); ++it)
                {
                    FormulaOpCodeMapEntry aEntry;
                    aEntry.Name = (*it).first;
                    aEntry.Token.Data <<= OUString( (*it).second);
                    aEntry.Token.OpCode = ocExternal;
                    aVec.push_back( aEntry);
                }
            }
            else
            {
                rCompiler.fillAddInToken( aVec, isEnglish());
            }
        }
    }
    const FormulaOpCodeMapEntry* pRet = aVec.empty() ? 0 : &aVec[0];
    return uno::Sequence< FormulaOpCodeMapEntry >( pRet, aVec.size());
}


void FormulaCompiler::OpCodeMap::putOpCode( const OUString & rStr, const OpCode eOp )
{
    DBG_ASSERT( 0 < eOp && sal_uInt16(eOp) < mnSymbols, "OpCodeMap::putOpCode: OpCode out of range");
    if (0 < eOp && sal_uInt16(eOp) < mnSymbols)
    {
        SAL_WARN_IF( !(mpTable[eOp].isEmpty() || (mpTable[eOp] == rStr) ||
                    (eOp == ocCurrency) || (eOp == ocSep) || (eOp == ocArrayColSep) ||
                    (eOp == ocArrayRowSep)), "formula.core",
                "OpCodeMap::putOpCode: reusing OpCode " << eOp
                << ", replacing '" << mpTable[eOp] << "' with '" << rStr << "' in "
                << (mbEnglish ? "" : "non-") << "English map 0x" << ::std::hex << meGrammar);
        mpTable[eOp] = rStr;
        mpHashMap->insert( OpCodeHashMap::value_type( rStr, eOp));
    }
}



DBG_NAME(FormulaCompiler)
FormulaCompiler::FormulaCompiler( FormulaTokenArray& rArr )
        :
        pArr( &rArr ),
        pCode( NULL ),
        pStack( NULL ),
        eLastOp( ocPush ),
        nRecursion( 0 ),
        nNumFmt( NUMBERFORMAT_UNDEFINED ),
        pc( 0 ),
        meGrammar( formula::FormulaGrammar::GRAM_UNSPECIFIED ),
        bAutoCorrect( false ),
        bCorrected( false ),
        bCompileForFAP( false ),
        bIgnoreErrors( false ),
        glSubTotal( false )

{
    DBG_CTOR(FormulaCompiler, NULL);
}

FormulaCompiler::FormulaCompiler()
        :
        pArr( NULL ),
        pStack( NULL ),
        nRecursion(0),
        nNumFmt( NUMBERFORMAT_UNDEFINED ),
        meGrammar( formula::FormulaGrammar::GRAM_UNSPECIFIED ),
        bAutoCorrect( false ),
        bCorrected( false ),
        bCompileForFAP( false ),
        bIgnoreErrors( false )

{
    DBG_CTOR(FormulaCompiler, NULL);
}

FormulaCompiler::~FormulaCompiler()
{
    DBG_DTOR(FormulaCompiler, NULL);
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
        case FormulaLanguage::OOXML:
            if (!mxSymbolsOOXML)
                InitSymbolsOOXML();
            xMap = mxSymbolsOOXML;
            break;
        default:
            ;   
    }
    return xMap;
}

OUString FormulaCompiler::FindAddInFunction( const OUString& /*rUpperName*/, bool /*bLocalFirst*/ ) const
{
    return OUString();
}

FormulaCompiler::OpCodeMapPtr FormulaCompiler::CreateOpCodeMap(
        const uno::Sequence<
        const sheet::FormulaOpCodeMapEntry > & rMapping,
        bool bEnglish )
{
    using sheet::FormulaOpCodeMapEntry;
    
    NonConstOpCodeMapPtr xMap( new OpCodeMap( SC_OPCODE_LAST_OPCODE_ID + 1, false,
                FormulaGrammar::mergeToGrammar( FormulaGrammar::setEnglishBit(
                        FormulaGrammar::GRAM_EXTERNAL, bEnglish), FormulaGrammar::CONV_UNSPECIFIED)));
    FormulaOpCodeMapEntry const * pArr2 = rMapping.getConstArray();
    FormulaOpCodeMapEntry const * const pStop = pArr2 + rMapping.getLength();
    for ( ; pArr2 < pStop; ++pArr2)
    {
        OpCode eOp = OpCode(pArr2->Token.OpCode);
        if (eOp != ocExternal)
            xMap->putOpCode( pArr2->Name, eOp);
        else
        {
            OUString aExternalName;
            if (pArr2->Token.Data >>= aExternalName)
                xMap->putExternal( pArr2->Name, aExternalName);
            else
            {
                SAL_WARN( "formula.core", "FormulaCompiler::CreateOpCodeMap: no Token.Data external name");
            }
        }
    }
    return xMap;
}

void lcl_fillNativeSymbols( FormulaCompiler::NonConstOpCodeMapPtr& xMap, bool bDestroy = false )
{
    static OpCodeMapData aSymbolMap;
    osl::MutexGuard aGuard(&aSymbolMap.maMtx);

    if ( bDestroy )
    {
        aSymbolMap.mxSymbolMap.reset();
    }
    else if (!aSymbolMap.mxSymbolMap)
    {
        
        aSymbolMap.mxSymbolMap.reset(
            new FormulaCompiler::OpCodeMap(
                SC_OPCODE_LAST_OPCODE_ID + 1, true, FormulaGrammar::GRAM_NATIVE_UI));
        OModuleClient aModuleClient;
        OpCodeList aOpCodeListNative(RID_STRLIST_FUNCTION_NAMES, aSymbolMap.mxSymbolMap);
        
    }

    xMap = aSymbolMap.mxSymbolMap;
}

const OUString& FormulaCompiler::GetNativeSymbol( OpCode eOp )
{
    NonConstOpCodeMapPtr xSymbolsNative;
    lcl_fillNativeSymbols( xSymbolsNative);
    return xSymbolsNative->getSymbol( eOp );
}

sal_Unicode FormulaCompiler::GetNativeSymbolChar( OpCode eOp )
{
    return GetNativeSymbol(eOp)[0];
}

void FormulaCompiler::InitSymbolsNative() const
{
    lcl_fillNativeSymbols( mxSymbolsNative);
}

void FormulaCompiler::InitSymbolsEnglish() const
{
    static OpCodeMapData aMap;
    osl::MutexGuard aGuard(&aMap.maMtx);
    if (!aMap.mxSymbolMap)
        loadSymbols(RID_STRLIST_FUNCTION_NAMES_ENGLISH, FormulaGrammar::GRAM_ENGLISH, aMap.mxSymbolMap);
    mxSymbolsEnglish = aMap.mxSymbolMap;
}

void FormulaCompiler::InitSymbolsPODF() const
{
    static OpCodeMapData aMap;
    osl::MutexGuard aGuard(&aMap.maMtx);
    if (!aMap.mxSymbolMap)
        loadSymbols(RID_STRLIST_FUNCTION_NAMES_ENGLISH, FormulaGrammar::GRAM_PODF, aMap.mxSymbolMap, RESOURCE_BASE);
    mxSymbolsPODF = aMap.mxSymbolMap;
}

void FormulaCompiler::InitSymbolsODFF() const
{
    static OpCodeMapData aMap;
    osl::MutexGuard aGuard(&aMap.maMtx);
    if (!aMap.mxSymbolMap)
        loadSymbols(RID_STRLIST_FUNCTION_NAMES_ENGLISH_ODFF, FormulaGrammar::GRAM_ODFF, aMap.mxSymbolMap, RESOURCE_BASE);
    mxSymbolsODFF = aMap.mxSymbolMap;
}

void FormulaCompiler::InitSymbolsEnglishXL() const
{
    static OpCodeMapData aMap;
    osl::MutexGuard aGuard(&aMap.maMtx);
    if (!aMap.mxSymbolMap)
        loadSymbols(RID_STRLIST_FUNCTION_NAMES_ENGLISH, FormulaGrammar::GRAM_ENGLISH, aMap.mxSymbolMap);
    mxSymbolsEnglishXL = aMap.mxSymbolMap;

    
    
    
    mxSymbolsEnglishXL->putOpCode( OUString(','), ocSep);
    mxSymbolsEnglishXL->putOpCode( OUString(','), ocArrayColSep);
    mxSymbolsEnglishXL->putOpCode( OUString(';'), ocArrayRowSep);
}

void FormulaCompiler::InitSymbolsOOXML() const
{
    static OpCodeMapData aMap;
    osl::MutexGuard aGuard(&aMap.maMtx);
    if (!aMap.mxSymbolMap)
        loadSymbols(RID_STRLIST_FUNCTION_NAMES_ENGLISH_OOXML, FormulaGrammar::GRAM_OOXML, aMap.mxSymbolMap, RESOURCE_BASE);
    mxSymbolsOOXML = aMap.mxSymbolMap;
}


void FormulaCompiler::loadSymbols( sal_uInt16 nSymbols, FormulaGrammar::Grammar eGrammar,
        NonConstOpCodeMapPtr& rxMap, SeparatorType eSepType) const
{
    if ( !rxMap.get() )
    {
        
        rxMap.reset( new OpCodeMap( SC_OPCODE_LAST_OPCODE_ID + 1, eGrammar != FormulaGrammar::GRAM_ODFF, eGrammar ));
        OModuleClient aModuleClient;
        OpCodeList aOpCodeList( nSymbols, rxMap, eSepType);

        fillFromAddInMap( rxMap, eGrammar);
        
        if ( FormulaGrammar::GRAM_ENGLISH != eGrammar )
            fillFromAddInCollectionUpperName( rxMap);
        else
            fillFromAddInCollectionEnglishName( rxMap);
    }
}

void FormulaCompiler::fillFromAddInCollectionUpperName( NonConstOpCodeMapPtr /*xMap */) const
{
}

void FormulaCompiler::fillFromAddInCollectionEnglishName( NonConstOpCodeMapPtr /*xMap */) const
{
}

void FormulaCompiler::fillFromAddInMap( NonConstOpCodeMapPtr /*xMap*/, FormulaGrammar::Grammar /*_eGrammar */) const
{
}

OpCode FormulaCompiler::GetEnglishOpCode( const OUString& rName ) const
{
    FormulaCompiler::OpCodeMapPtr xMap = GetOpCodeMap( sheet::FormulaLanguage::ENGLISH);

    formula::OpCodeHashMap::const_iterator iLook( xMap->getHashMap()->find( rName ) );
    bool bFound = (iLook != xMap->getHashMap()->end());
    return bFound ? (*iLook).second : OpCode(ocNone);
}

bool FormulaCompiler::IsOpCodeVolatile( OpCode eOp )
{
    bool bRet = false;
    switch (eOp)
    {
        
        case ocRandom:
        case ocGetActDate:
        case ocGetActTime:
        
        case ocFormula:
        case ocInfo:
        
            
            
            
        case ocIndirect:
        case ocIndirectXL:
            
        case ocOffset:
            
        case ocDebugVar:
            bRet = true;
            break;
        default:
            bRet = false;
            break;
    }
    return bRet;
}


bool FormulaCompiler::DeQuote( OUString& rStr )
{
    sal_Int32 nLen = rStr.getLength();
    if ( nLen > 1 && rStr[0] == '\'' && rStr[ nLen-1 ] == '\'' )
    {
        rStr = rStr.copy( 1, nLen-2 );
        rStr = rStr.replaceAll( "\\\'", "\'" );
        return true;
    }
    return false;
}

void FormulaCompiler::fillAddInToken(
        ::std::vector< sheet::FormulaOpCodeMapEntry >& /*_rVec*/,
        bool /*_bIsEnglish*/) const
{
}

bool FormulaCompiler::IsMatrixFunction( OpCode eOpCode )
{
    switch (eOpCode)
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
            return true;
        default:
        {
            
        }
    }
    return false;
}


FormulaCompiler::OpCodeMap::~OpCodeMap()
{
    delete mpReverseExternalHashMap;
    delete mpExternalHashMap;
    delete [] mpTable;
    delete mpHashMap;
}

void FormulaCompiler::OpCodeMap::putCopyOpCode( const OUString& rSymbol, OpCode eOp )
{
    SAL_WARN_IF( !mpTable[eOp].isEmpty() && rSymbol.isEmpty(), "formula.core",
            "OpCodeMap::putCopyOpCode: NOT replacing OpCode " << eOp << " '" << mpTable[eOp] << "' with empty name!");
    if (!mpTable[eOp].isEmpty() && rSymbol.isEmpty())
        mpHashMap->insert( OpCodeHashMap::value_type( mpTable[eOp], eOp));
    else
    {
        mpTable[eOp] = rSymbol;
        mpHashMap->insert( OpCodeHashMap::value_type( rSymbol, eOp));
    }
}

void FormulaCompiler::OpCodeMap::copyFrom( const OpCodeMap& r, bool bOverrideKnownBad )
{
    delete mpHashMap;
    mpHashMap = new OpCodeHashMap( mnSymbols);

    sal_uInt16 n = r.getSymbolCount();
    SAL_WARN_IF( n != mnSymbols, "formula.core",
            "OpCodeMap::copyFrom: unequal size, this: " << mnSymbols << "  that: " << n);
    if (n > mnSymbols)
        n = mnSymbols;

    
    SAL_WARN_IF( !mpTable[0].isEmpty() || !r.mpTable[0].isEmpty(), "formula.core",
            "OpCodeMap::copyFrom: OpCode 0 assigned, this: '"
            << mpTable[0] << "'  that: '" << r.mpTable[0] << "'");

    
    
    
    if (bOverrideKnownBad && r.mbCore &&
            FormulaGrammar::extractFormulaLanguage( meGrammar) == sheet::FormulaLanguage::NATIVE &&
            FormulaGrammar::extractFormulaLanguage( r.meGrammar) == sheet::FormulaLanguage::ENGLISH)
    {
        for (sal_uInt16 i = 1; i < n; ++i)
        {
            OUString aSymbol;
            OpCode eOp = OpCode(i);
            switch (eOp)
            {
                case ocZGZ:
                    aSymbol = "RRI";
                    break;
                case ocTableOp:
                    aSymbol = "MULTIPLE.OPERATIONS";
                    break;
                default:
                    aSymbol = r.mpTable[i];
            }
            putCopyOpCode( aSymbol, eOp);
        }
    }
    else
    {
        for (sal_uInt16 i = 1; i < n; ++i)
        {
            OpCode eOp = OpCode(i);
            const OUString& rSymbol = r.mpTable[i];
            putCopyOpCode( rSymbol, eOp);
        }
    }

    
}


sal_uInt16 FormulaCompiler::GetErrorConstant( const OUString& rName ) const
{
    sal_uInt16 nError = 0;
    OpCodeHashMap::const_iterator iLook( mxSymbols->getHashMap()->find( rName));
    if (iLook != mxSymbols->getHashMap()->end())
    {
        switch ((*iLook).second)
        {
            
            
            case ocErrNull:
                nError = errNoCode;
                break;
            case ocErrDivZero:
                nError = errDivisionByZero;
                break;
            case ocErrValue:
                nError = errNoValue;
                break;
            case ocErrRef:
                nError = errNoRef;
                break;
            case ocErrName:
                nError = errNoName;
                break;
            case ocErrNum:
                nError = errIllegalFPOperation;
                break;
            case ocErrNA:
                nError = NOTAVAILABLE;
                break;
            default:
                ;   
        }
    }
    return nError;
}


void FormulaCompiler::AppendErrorConstant( OUStringBuffer& rBuffer, sal_uInt16 nError ) const
{
    OpCode eOp;
    switch (nError)
    {
        default:
        case errNoCode:
            eOp = ocErrNull;
            break;
        case errDivisionByZero:
            eOp = ocErrDivZero;
            break;
        case errNoValue:
            eOp = ocErrValue;
            break;
        case errNoRef:
            eOp = ocErrRef;
            break;
        case errNoName:
            eOp = ocErrName;
            break;
        case errIllegalFPOperation:
            eOp = ocErrNum;
            break;
        case NOTAVAILABLE:
            eOp = ocErrNA;
            break;
    }
    rBuffer.append( mxSymbols->getSymbol( eOp));
}


sal_Int32 FormulaCompiler::OpCodeMap::getOpCodeUnknown()
{
    static const sal_Int32 kOpCodeUnknown = -1;
    return kOpCodeUnknown;
}

bool FormulaCompiler::GetToken()
{
    static const short nRecursionMax = 42;
    FormulaCompilerRecursionGuard aRecursionGuard( nRecursion );
    if ( nRecursion > nRecursionMax )
    {
        SetError( errStackOverflow );
        mpToken = new FormulaByteToken( ocStop );
        return false;
    }
    if ( bAutoCorrect && !pStack )
    {   
        aCorrectedFormula += aCorrectedSymbol;
        aCorrectedSymbol = "";
    }
    bool bStop = false;
    if( pArr->GetCodeError() && !bIgnoreErrors )
        bStop = true;
    else
    {
        short nWasColRowName;
        if ( pArr->nIndex
          && pArr->pCode[ pArr->nIndex-1 ]->GetOpCode() == ocColRowName )
             nWasColRowName = 1;
        else
             nWasColRowName = 0;
        mpToken = pArr->Next();
        while( mpToken && mpToken->GetOpCode() == ocSpaces )
        {
            if ( nWasColRowName )
                nWasColRowName++;
            if ( bAutoCorrect && !pStack )
                CreateStringFromToken( aCorrectedFormula, mpToken.get(), false );
            mpToken = pArr->Next();
        }
        if ( bAutoCorrect && !pStack && mpToken )
            CreateStringFromToken( aCorrectedSymbol, mpToken.get(), false );
        if( !mpToken )
        {
            if( pStack )
            {
                PopTokenArray();
                return GetToken();
            }
            else
                bStop = true;
        }
        else
        {
            if ( nWasColRowName >= 2 && mpToken->GetOpCode() == ocColRowName )
            {   
                mpToken = new FormulaByteToken( ocIntersect );
                pArr->nIndex--;     
            }
        }
    }
    if( bStop )
    {
        mpToken = new FormulaByteToken( ocStop );
        return false;
    }
    if( mpToken->GetOpCode() == ocSubTotal )
        glSubTotal = true;
    else if ( mpToken->IsExternalRef() )
    {
        return HandleExternalReference(*mpToken);
    }
    else if( mpToken->GetOpCode() == ocName )
    {
        return HandleRange();
    }
    else if( mpToken->GetOpCode() == ocColRowName )
    {
        return HandleSingleRef();
    }
    else if( mpToken->GetOpCode() == ocDBArea )
    {
        return HandleDbData();
    }
    else if( mpToken->GetType() == svSingleRef )
    {
        pArr->nRefs++;
    }
    else if( mpToken->GetType() == svDoubleRef )
    {
        pArr->nRefs++;
    }
    return true;
}



void FormulaCompiler::Factor()
{
    if ( pArr->GetCodeError() && !bIgnoreErrors )
        return;

    CurrentFactor pFacToken( this );

    OpCode eOp = mpToken->GetOpCode();
    if( eOp == ocPush || eOp == ocColRowNameAuto || eOp == ocMatRef ||
            eOp == ocDBArea
            || (bCompileForFAP && ((eOp == ocName) || (eOp == ocDBArea)
            || (eOp == ocColRowName) || (eOp == ocBad)))
        )
    {
        PutCode( mpToken );
        eOp = NextToken();
        if( eOp == ocOpen )
        {
            
            SetError(
                ( mpToken->GetType() == svString
               || mpToken->GetType() == svSingleRef )
               ? errNoName : errOperatorExpected );
            if ( bAutoCorrect && !pStack )
            {   
                aCorrectedFormula += mxSymbols->getSymbol( ocMul);
                bCorrected = true;
                NextToken();
                eOp = Expression();
                if( eOp != ocClose )
                    SetError( errPairExpected);
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
        {   
            pFacToken = mpToken;
            NextToken();
            eOp = Expression();
            
            
            if (!pArr->GetCodeError())
            {
                pFacToken->NewOpCode( ocUnion, FormulaToken::PrivateAccess());
                PutCode( pFacToken);
            }
        }
        if (eOp != ocClose)
            SetError( errPairExpected);
        else
            eOp = NextToken();
    }
    else
    {
        if( nNumFmt == NUMBERFORMAT_UNDEFINED )
            nNumFmt = lcl_GetRetFormat( eOp );

        if ( IsOpCodeVolatile( eOp) )
            pArr->SetExclusiveRecalcModeAlways();
        else
        {
            switch( eOp )
            {
                    
                    
                    
                    
                case ocConvert :
                case ocDde:
                case ocMacro:
                case ocExternal:
                    pArr->AddRecalcMode( RECALCMODE_ONLOAD );
                break;
                    
                case ocColumn :
                case ocRow :
                    pArr->SetRecalcModeOnRefMove();
                break;
                    
                    
                case ocCell :
                    pArr->SetRecalcModeOnRefMove();
                    pArr->AddRecalcMode( RECALCMODE_ONLOAD );
                break;
                case ocHyperLink :
                    
                    
                    pArr->AddRecalcMode( RECALCMODE_ONLOAD );
                    pArr->SetHyperLink( true);
                break;
                default:
                    ;   
            }
        }
        if (SC_OPCODE_START_NO_PAR <= eOp && eOp < SC_OPCODE_STOP_NO_PAR)
        {
            pFacToken = mpToken;
            eOp = NextToken();
            if (eOp != ocOpen)
            {
                SetError( errPairExpected);
                PutCode( pFacToken );
            }
            else
            {
                eOp = NextToken();
                if (eOp != ocClose)
                    SetError( errPairExpected);
                PutCode( pFacToken);
                eOp = NextToken();
            }
        }
        
        else if( eOp == ocNot || eOp == ocNeg
              || (SC_OPCODE_START_1_PAR <= eOp && eOp < SC_OPCODE_STOP_1_PAR) )
        {
            pFacToken = mpToken;
            eOp = NextToken();
            if( nNumFmt == NUMBERFORMAT_UNDEFINED && eOp == ocNot )
                nNumFmt = NUMBERFORMAT_LOGICAL;
            if (eOp == ocOpen)
            {
                NextToken();
                eOp = Expression();
            }
            else
                SetError( errPairExpected);
            if (eOp != ocClose)
                SetError( errPairExpected);
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
                || ( bCompileForFAP
                     && ( eOp == ocIf || eOp == ocIfError || eOp == ocIfNA || eOp == ocChose ) )
            )
        {
            pFacToken = mpToken;
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
                
                
                bBadName = true;
                bNoParam = true;
            }
            else
                SetError( errPairExpected);
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
                ;   
            else if (eOp != ocClose)
                SetError( errPairExpected);
            else
                eOp = NextToken();
            
            if ( bCompileForFAP && pFacToken->GetType() == svJump )
                pFacToken = new FormulaFAPToken( pFacToken->GetOpCode(), nSepCount, pFacToken );
            else
                pFacToken->SetByte( nSepCount );
            PutCode( pFacToken );
        }
        else if (eOp == ocIf || eOp == ocIfError || eOp == ocIfNA || eOp == ocChose)
        {
            
            pFacToken = mpToken;
            switch (eOp)
            {
                case ocIf:
                    pFacToken->GetJump()[ 0 ] = 3;  
                    break;
                case ocChose:
                    pFacToken->GetJump()[ 0 ] = FORMULA_MAXJUMPCOUNT + 1;
                    break;
                case ocIfError:
                case ocIfNA:
                    pFacToken->GetJump()[ 0 ] = 2;  
                    break;
                default:
                    SAL_WARN( "formula.core", "FormulaCompiler::Factor: forgot to add a jump count case?");
            }
            eOp = NextToken();
            if (eOp == ocOpen)
            {
                NextToken();
                eOp = Expression();
            }
            else
                SetError( errPairExpected);
            PutCode( pFacToken );
            
            
            
            
            short nJumpMax;
            OpCode eFacOpCode = pFacToken->GetOpCode();
            switch (eFacOpCode)
            {
                case ocIf:
                    nJumpMax = 3;
                    break;
                case ocChose:
                    nJumpMax = FORMULA_MAXJUMPCOUNT;
                    break;
                case ocIfError:
                case ocIfNA:
                    nJumpMax = 2;
                    break;
                default:
                    nJumpMax = 0;
                    SAL_WARN( "formula.core", "FormulaCompiler::Factor: forgot to add a jump max case?");
            }
            short nJumpCount = 0;
            while ( (nJumpCount < (FORMULA_MAXJUMPCOUNT - 1)) && (eOp == ocSep)
                    && (!pArr->GetCodeError() || bIgnoreErrors) )
            {
                if ( ++nJumpCount <= nJumpMax )
                    pFacToken->GetJump()[nJumpCount] = pc-1;
                NextToken();
                eOp = Expression();
                
                PutCode( mpToken );
            }
            if (eOp != ocClose)
                SetError( errPairExpected);
            else
            {
                eOp = NextToken();
                
                if ( ++nJumpCount <= nJumpMax )
                    pFacToken->GetJump()[ nJumpCount ] = pc-1;
                eFacOpCode = pFacToken->GetOpCode();
                bool bLimitOk;
                switch (eFacOpCode)
                {
                    case ocIf:
                        bLimitOk = (nJumpCount <= 3);
                        break;
                    case ocChose:
                        bLimitOk = (nJumpCount < FORMULA_MAXJUMPCOUNT); /* TODO: check, really <, not <=? */
                        break;
                    case ocIfError:
                    case ocIfNA:
                        bLimitOk = (nJumpCount <= 2);
                        break;
                    default:
                        bLimitOk = false;
                        SAL_WARN( "formula.core", "FormulaCompiler::Factor: forgot to add a jump limit case?");
                }
                if (bLimitOk)
                    pFacToken->GetJump()[ 0 ] = nJumpCount;
                else
                    SetError( errIllegalParameter);
            }
        }
        else if ( eOp == ocMissing )
        {
            PutCode( mpToken );
            eOp = NextToken();
        }
        else if ( eOp == ocClose )
        {
            SetError( errParameterExpected );
        }
        else if ( eOp == ocSep )
        {   
            SetError( errParameterExpected );
            if ( bAutoCorrect && !pStack )
            {
                aCorrectedSymbol = "";
                bCorrected = true;
            }
        }
        else if ( mpToken->IsExternalRef() )
        {
            PutCode( mpToken);
            eOp = NextToken();
        }
        else
        {
            SetError( errUnknownToken );
            if ( bAutoCorrect && !pStack )
            {
                if ( eOp == ocStop )
                {   
                    sal_Int32 nLen = aCorrectedFormula.getLength();
                    if ( nLen )
                        aCorrectedFormula = aCorrectedFormula.copy( 0, nLen - 1 );
                    aCorrectedSymbol = "";
                    bCorrected = true;
                }
            }
        }
    }
}

void FormulaCompiler::RangeLine()
{
    Factor();
    while (mpToken->GetOpCode() == ocRange)
    {
        FormulaToken** pCode1 = pCode - 1;
        FormulaTokenRef p = mpToken;
        NextToken();
        Factor();
        FormulaToken** pCode2 = pCode - 1;
        if (!MergeRangeReference( pCode1, pCode2))
            PutCode(p);
    }
}

void FormulaCompiler::IntersectionLine()
{
    RangeLine();
    while (mpToken->GetOpCode() == ocIntersect)
    {
        FormulaTokenRef p = mpToken;
        NextToken();
        RangeLine();
        PutCode(p);
    }
}

void FormulaCompiler::UnionLine()
{
    IntersectionLine();
    while (mpToken->GetOpCode() == ocUnion)
    {
        FormulaTokenRef p = mpToken;
        NextToken();
        IntersectionLine();
        PutCode(p);
    }
}

void FormulaCompiler::UnaryLine()
{
    if( mpToken->GetOpCode() == ocAdd )
        GetToken();
    else if (SC_OPCODE_START_UN_OP <= mpToken->GetOpCode() &&
            mpToken->GetOpCode() < SC_OPCODE_STOP_UN_OP)
    {
        FormulaTokenRef p = mpToken;
        NextToken();
        UnaryLine();
        PutCode( p );
    }
    else
        UnionLine();
}

void FormulaCompiler::PostOpLine()
{
    UnaryLine();
    while ( mpToken->GetOpCode() == ocPercentSign )
    {   
        PutCode( mpToken );
        NextToken();
    }
}

void FormulaCompiler::PowLine()
{
    PostOpLine();
    while (mpToken->GetOpCode() == ocPow)
    {
        FormulaTokenRef p = mpToken;
        NextToken();
        PostOpLine();
        PutCode(p);
    }
}

void FormulaCompiler::MulDivLine()
{
    PowLine();
    while (mpToken->GetOpCode() == ocMul || mpToken->GetOpCode() == ocDiv)
    {
        FormulaTokenRef p = mpToken;
        NextToken();
        PowLine();
        PutCode(p);
    }
}

void FormulaCompiler::AddSubLine()
{
    MulDivLine();
    while (mpToken->GetOpCode() == ocAdd || mpToken->GetOpCode() == ocSub)
    {
        FormulaTokenRef p = mpToken;
        NextToken();
        MulDivLine();
        PutCode(p);
    }
}

void FormulaCompiler::ConcatLine()
{
    AddSubLine();
    while (mpToken->GetOpCode() == ocAmpersand)
    {
        FormulaTokenRef p = mpToken;
        NextToken();
        AddSubLine();
        PutCode(p);
    }
}

void FormulaCompiler::CompareLine()
{
    ConcatLine();
    while (mpToken->GetOpCode() >= ocEqual && mpToken->GetOpCode() <= ocGreaterEqual)
    {
        FormulaTokenRef p = mpToken;
        NextToken();
        ConcatLine();
        PutCode(p);
    }
}

void FormulaCompiler::NotLine()
{
    CompareLine();
    while (mpToken->GetOpCode() == ocNot)
    {
        FormulaTokenRef p = mpToken;
        NextToken();
        CompareLine();
        PutCode(p);
    }
}

OpCode FormulaCompiler::Expression()
{
    static const short nRecursionMax = 42;
    FormulaCompilerRecursionGuard aRecursionGuard( nRecursion );
    if ( nRecursion > nRecursionMax )
    {
        SetError( errStackOverflow );
        return ocStop;      
    }
    NotLine();
    while (mpToken->GetOpCode() == ocAnd || mpToken->GetOpCode() == ocOr)
    {
        FormulaTokenRef p = mpToken;
        mpToken->SetByte( 2 );       
        NextToken();
        NotLine();
        PutCode(p);
    }
    return mpToken->GetOpCode();
}


void FormulaCompiler::SetError( sal_uInt16 /*nError*/ )
{
}

FormulaTokenRef FormulaCompiler::ExtendRangeReference( FormulaToken & /*rTok1*/, FormulaToken & /*rTok2*/,
        bool /*bReuseDoubleRef*/ )
{
    return FormulaTokenRef();
}

bool FormulaCompiler::MergeRangeReference( FormulaToken * * const pCode1, FormulaToken * const * const pCode2 )
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

bool FormulaCompiler::CompileTokenArray()
{
    glSubTotal = false;
    bCorrected = false;
    if( !pArr->GetCodeError() || bIgnoreErrors )
    {
        if ( bAutoCorrect )
        {
            aCorrectedFormula = "";
            aCorrectedSymbol = "";
        }
        pArr->nRefs = 0;    
        pArr->DelRPN();
        pStack = NULL;
        FormulaToken* pData[ FORMULA_MAXTOKENS ];
        pCode = pData;
        bool bWasForced = pArr->IsRecalcModeForced();
        if ( bWasForced )
        {
            if ( bAutoCorrect )
                aCorrectedFormula = "=";
        }
        pArr->ClearRecalcMode();
        pArr->Reset();
        eLastOp = ocOpen;
        pc = 0;
        NextToken();
        OpCode eOp = Expression();
        
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

        
        if( !pArr->GetCodeError() && nErrorBeforePop )
            pArr->SetCodeError( nErrorBeforePop);

        if( pArr->GetCodeError() && !bIgnoreErrors )
        {
            pArr->DelRPN();
            pArr->SetHyperLink( false);
        }

        if ( bWasForced )
            pArr->SetRecalcModeForced();
    }
    if( nNumFmt == NUMBERFORMAT_UNDEFINED )
        nNumFmt = NUMBERFORMAT_NUMBER;
    return glSubTotal;
}

void FormulaCompiler::PopTokenArray()
{
    if( pStack )
    {
        FormulaArrayStack* p = pStack;
        pStack = p->pNext;
        p->pArr->nRefs = sal::static_int_cast<short>( p->pArr->nRefs + pArr->nRefs );
        
        if ( pArr->IsRecalcModeAlways() )
            p->pArr->SetExclusiveRecalcModeAlways();
        else if ( !pArr->IsRecalcModeNormal() && p->pArr->IsRecalcModeNormal() )
            p->pArr->SetMaskedRecalcMode( pArr->GetRecalcMode() );
        p->pArr->SetCombinedBitsRecalcMode( pArr->GetRecalcMode() );
        if( p->bTemp )
            delete pArr;
        pArr = p->pArr;
        delete p;
    }
}

void FormulaCompiler::CreateStringFromTokenArray( OUString& rFormula )
{
    OUStringBuffer aBuffer( pArr->GetLen() * 5 );
    CreateStringFromTokenArray( aBuffer );
    rFormula = aBuffer.makeStringAndClear();
}

void FormulaCompiler::CreateStringFromTokenArray( OUStringBuffer& rBuffer )
{
    rBuffer.setLength(0);
    if( !pArr->GetLen() )
        return;

    FormulaTokenArray* pSaveArr = pArr;
    bool bODFF = FormulaGrammar::isODFF( meGrammar);
    if (bODFF || FormulaGrammar::isPODF( meGrammar) )
    {
        
        MissingConvention aConv( bODFF);
        if (pArr->NeedsPofRewrite( aConv))
            pArr = pArr->RewriteMissingToPof( aConv);
    }

    
    
    rBuffer.ensureCapacity( pArr->GetLen() * 5 );

    if ( pArr->IsRecalcModeForced() )
        rBuffer.append( '=');
    FormulaToken* t = pArr->First();
    while( t )
        t = CreateStringFromToken( rBuffer, t, true );

    if (pSaveArr != pArr)
    {
        delete pArr;
        pArr = pSaveArr;
    }
}

FormulaToken* FormulaCompiler::CreateStringFromToken( OUString& rFormula, FormulaToken* pTokenP,bool bAllowArrAdvance )
{
    OUStringBuffer aBuffer;
    FormulaToken* p = CreateStringFromToken( aBuffer, pTokenP, bAllowArrAdvance );
    rFormula += aBuffer.makeStringAndClear();
    return p;
}

FormulaToken* FormulaCompiler::CreateStringFromToken( OUStringBuffer& rBuffer, FormulaToken* pTokenP, bool bAllowArrAdvance )
{
    bool bNext = true;
    bool bSpaces = false;
    FormulaToken* t = pTokenP;
    OpCode eOp = t->GetOpCode();
    if( eOp >= ocAnd && eOp <= ocOr )
    {
        
        if ( bAllowArrAdvance )
            t = pArr->Next();
        else
            t = pArr->PeekNext();
        bNext = false;
        bSpaces = ( !t || t->GetOpCode() != ocOpen );
    }
    if( bSpaces )
        rBuffer.append( ' ');

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
            
            sal_uInt8 n = t->GetByte();
            for ( sal_uInt8 j=0; j<n; ++j )
            {
                rBuffer.append( ' ');
            }
        }
    }
    else if( eOp >= ocInternalBegin && eOp <= ocInternalEnd )
        rBuffer.appendAscii( pInternal[ eOp - ocInternalBegin ] );
    else if( (sal_uInt16) eOp < mxSymbols->getSymbolCount())        
        rBuffer.append( mxSymbols->getSymbol( eOp));
    else
    {
        SAL_WARN( "formula.core","unknown OpCode");
        rBuffer.append( GetNativeSymbol( ocErrName ));
    }
    if( bNext )
    {
        if (t->IsExternalRef())
        {
            CreateStringFromExternal( rBuffer, pTokenP);
        }
        else
        {
            switch( t->GetType() )
            {
            case svDouble:
                AppendDouble( rBuffer, t->GetDouble() );
            break;

            case svString:
                if( eOp == ocBad || eOp == ocStringXML )
                    rBuffer.append( t->GetString().getString());
                else
                    AppendString( rBuffer, t->GetString().getString() );
                break;
            case svSingleRef:
                CreateStringFromSingleRef( rBuffer, t);
                break;
            case svDoubleRef:
                CreateStringFromDoubleRef( rBuffer, t);
                break;
            case svMatrix:
                CreateStringFromMatrix( rBuffer, t );
                break;

            case svIndex:
                CreateStringFromIndex( rBuffer, t );
                break;
            case svExternal:
            {
                
                OUString aAddIn( t->GetExternal() );
                bool bMapped = mxSymbols->isPODF();     
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
                rBuffer.append( aAddIn);
            }
            break;
            case svError:
                AppendErrorConstant( rBuffer, t->GetError());
            break;
            case svByte:
            case svJump:
            case svFAP:
            case svMissing:
            case svSep:
                break;      
            default:
                OSL_FAIL("FormulaCompiler:: GetStringFromToken errUnknownVariable");
            } 
        }
    }
    if( bSpaces )
        rBuffer.append( ' ');
    if ( bAllowArrAdvance )
    {
        if( bNext )
            t = pArr->Next();
        return t;
    }
    return pTokenP;
}


void FormulaCompiler::AppendDouble( OUStringBuffer& rBuffer, double fVal ) const
{
    if ( mxSymbols->isEnglish() )
    {
        ::rtl::math::doubleToUStringBuffer( rBuffer, fVal,
                rtl_math_StringFormat_Automatic,
                rtl_math_DecimalPlaces_Max, '.', true );
    }
    else
    {
        SvtSysLocale aSysLocale;
        ::rtl::math::doubleToUStringBuffer( rBuffer, fVal,
                rtl_math_StringFormat_Automatic,
                rtl_math_DecimalPlaces_Max,
                aSysLocale.GetLocaleDataPtr()->getNumDecimalSep()[0],
                true );
    }
}

void FormulaCompiler::AppendBoolean( OUStringBuffer& rBuffer, bool bVal ) const
{
    rBuffer.append( mxSymbols->getSymbol( static_cast<OpCode>(bVal ? ocTrue : ocFalse)) );
}

void FormulaCompiler::AppendString( OUStringBuffer& rBuffer, const OUString & rStr ) const
{
    rBuffer.append( '"');
    if ( lcl_UnicodeStrChr( rStr.getStr(), '"' ) == NULL )
        rBuffer.append( rStr );
    else
    {
        OUString aStr = rStr.replaceAll( "\"", "\"\"" );
        rBuffer.append(aStr);
    }
    rBuffer.append( '"');
}

void FormulaCompiler::UpdateSeparatorsNative(
    const OUString& rSep, const OUString& rArrayColSep, const OUString& rArrayRowSep )
{
    NonConstOpCodeMapPtr xSymbolsNative;
    lcl_fillNativeSymbols( xSymbolsNative);
    xSymbolsNative->putOpCode( rSep, ocSep);
    xSymbolsNative->putOpCode( rArrayColSep, ocArrayColSep);
    xSymbolsNative->putOpCode( rArrayRowSep, ocArrayRowSep);
}

void FormulaCompiler::ResetNativeSymbols()
{
    NonConstOpCodeMapPtr xSymbolsNative;
    lcl_fillNativeSymbols( xSymbolsNative, true);
    lcl_fillNativeSymbols( xSymbolsNative);
}

void FormulaCompiler::SetNativeSymbols( const OpCodeMapPtr& xMap )
{
    NonConstOpCodeMapPtr xSymbolsNative;
    lcl_fillNativeSymbols( xSymbolsNative);
    xSymbolsNative->copyFrom( *xMap, true);
}


OpCode FormulaCompiler::NextToken()
{
    if( !GetToken() )
        return ocStop;
    OpCode eOp = mpToken->GetOpCode();
    
    if ( (eOp == ocPush || eOp == ocColRowNameAuto) &&
            !( (eLastOp == ocOpen) || (eLastOp == ocSep) ||
                (SC_OPCODE_START_BIN_OP <= eLastOp && eLastOp < SC_OPCODE_STOP_UN_OP)) )
        SetError( errOperatorExpected);
    
    if (eOp == ocAdd && (eLastOp == ocOpen || eLastOp == ocSep ||
                (SC_OPCODE_START_BIN_OP <= eLastOp && eLastOp < SC_OPCODE_STOP_UN_OP)))
        eOp = NextToken();
    else
    {
        
        
        if ( eOp != ocAnd && eOp != ocOr &&
                (SC_OPCODE_START_BIN_OP <= eOp && eOp < SC_OPCODE_STOP_BIN_OP )
                && (eLastOp == ocOpen || eLastOp == ocSep ||
                    (SC_OPCODE_START_BIN_OP <= eLastOp && eLastOp < SC_OPCODE_STOP_UN_OP)))
        {
            SetError( errVariableExpected);
            if ( bAutoCorrect && !pStack )
            {
                if ( eOp == eLastOp || eLastOp == ocOpen )
                {   
                    aCorrectedSymbol = "";
                    bCorrected = true;
                }
                else
                {
                    sal_Int32 nPos = aCorrectedFormula.getLength();
                    if ( nPos )
                    {
                        nPos--;
                        sal_Unicode c = aCorrectedFormula[ nPos ];
                        switch ( eOp )
                        {   
                            case ocGreater:
                                if ( c == mxSymbols->getSymbolChar( ocEqual) )
                                {   
                                    aCorrectedFormula = aCorrectedFormula.replaceAt( nPos, 1,
                                        OUString( mxSymbols->getSymbolChar(ocGreater) ) );
                                    aCorrectedSymbol = OUString(c);
                                    bCorrected = true;
                                }
                            break;
                            case ocLess:
                                if ( c == mxSymbols->getSymbolChar( ocEqual) )
                                {   
                                    aCorrectedFormula = aCorrectedFormula.replaceAt( nPos, 1,
                                        OUString( mxSymbols->getSymbolChar(ocLess) ) );
                                    aCorrectedSymbol = OUString(c);
                                    bCorrected = true;
                                }
                                else if ( c == mxSymbols->getSymbolChar( ocGreater) )
                                {   
                                    aCorrectedFormula = aCorrectedFormula.replaceAt( nPos, 1,
                                        OUString( mxSymbols->getSymbolChar(ocLess) ) );
                                    aCorrectedSymbol = OUString(c);
                                    bCorrected = true;
                                }
                            break;
                            case ocMul:
                                if ( c == mxSymbols->getSymbolChar( ocSub) )
                                {   
                                    aCorrectedFormula = aCorrectedFormula.replaceAt( nPos, 1,
                                        OUString( mxSymbols->getSymbolChar(ocMul) ) );
                                    aCorrectedSymbol = OUString(c);
                                    bCorrected = true;
                                }
                            break;
                            case ocDiv:
                                if ( c == mxSymbols->getSymbolChar( ocSub) )
                                {   
                                    aCorrectedFormula = aCorrectedFormula.replaceAt( nPos, 1,
                                        OUString( mxSymbols->getSymbolChar(ocDiv) ) );
                                    aCorrectedSymbol = OUString(c);
                                    bCorrected = true;
                                }
                            break;
                            default:
                                ;   
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
    if( pc >= FORMULA_MAXTOKENS - 1 )
    {
        if ( pc == FORMULA_MAXTOKENS - 1 )
        {
            p = new FormulaByteToken( ocStop );
            p->IncRef();
            *pCode++ = p.get();
            ++pc;
        }
        SetError( errCodeOverflow);
        return;
    }
    if( pArr->GetCodeError() && !bCompileForFAP )
        return;
    ForceArrayOperator( p, pCurrentFactorToken);
    p->IncRef();
    *pCode++ = p.get();
    pc++;
}


bool FormulaCompiler::HandleExternalReference( const FormulaToken& /*_aToken*/)
{
    return true;
}

bool FormulaCompiler::HandleRange()
{
    return true;
}

bool FormulaCompiler::HandleSingleRef()
{
    return true;
}

bool FormulaCompiler::HandleDbData()
{
    return true;
}

void FormulaCompiler::CreateStringFromSingleRef( OUStringBuffer& /*rBuffer*/, FormulaToken* /*pTokenP*/) const
{
}

void FormulaCompiler::CreateStringFromDoubleRef( OUStringBuffer& /*rBuffer*/, FormulaToken* /*pTokenP*/) const
{
}

void FormulaCompiler::CreateStringFromIndex( OUStringBuffer& /*rBuffer*/, FormulaToken* /*pTokenP*/) const
{
}

void FormulaCompiler::CreateStringFromMatrix( OUStringBuffer& /*rBuffer*/, FormulaToken* /*pTokenP*/) const
{
}

void FormulaCompiler::CreateStringFromExternal( OUStringBuffer& /*rBuffer*/, FormulaToken* /*pTokenP*/) const
{
}

void FormulaCompiler::LocalizeString( OUString& /*rName*/ ) const
{
}

void FormulaCompiler::PushTokenArray( FormulaTokenArray* pa, bool bTemp )
{
    if ( bAutoCorrect && !pStack )
    {   
        aCorrectedFormula += aCorrectedSymbol;
        aCorrectedSymbol = "";
    }
    FormulaArrayStack* p = new FormulaArrayStack;
    p->pNext      = pStack;
    p->pArr       = pArr;
    p->bTemp      = bTemp;
    pStack        = p;
    pArr          = pa;
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

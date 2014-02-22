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

#include "compiler.hxx"

#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbstar.hxx>
#include <svl/zforlist.hxx>
#include "svl/sharedstringpool.hxx"
#include <sal/macros.h>
#include <tools/rcid.h>
#include <tools/solar.h>
#include <unotools/charclass.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/sheet/FormulaOpCodeMapEntry.hpp>
#include <com/sun/star/sheet/FormulaLanguage.hpp>
#include <com/sun/star/sheet/FormulaMapGroup.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <tools/urlobj.hxx>
#include <rtl/math.hxx>
#include <rtl/ustring.hxx>
#include <svtools/miscopt.hxx>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "rangenam.hxx"
#include "dbdata.hxx"
#include "document.hxx"
#include "callform.hxx"
#include "addincol.hxx"
#include "refupdat.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "formulacell.hxx"
#include "dociter.hxx"
#include "docoptio.hxx"
#include <formula/errorcodes.hxx>
#include "parclass.hxx"
#include "autonamecache.hxx"
#include "externalrefmgr.hxx"
#include "rangeutl.hxx"
#include "convuno.hxx"
#include "tokenuno.hxx"
#include "formulaparserpool.hxx"
#include "tokenarray.hxx"
#include "scmatrix.hxx"
#include <tokenstringcontext.hxx>

using namespace formula;
using namespace ::com::sun::star;
using ::std::vector;

CharClass*                          ScCompiler::pCharClassEnglish = NULL;
const ScCompiler::Convention*       ScCompiler::pConventions[ ]   = { NULL, NULL, NULL, NULL, NULL, NULL };

enum ScanState
{
    ssGetChar,
    ssGetBool,
    ssGetValue,
    ssGetString,
    ssSkipString,
    ssGetIdent,
    ssGetReference,
    ssSkipReference,
    ssGetErrorConstant,
    ssStop
};

static const sal_Char* pInternal[2] = { "TTT", "__DEBUG_VAR" };

using namespace ::com::sun::star::i18n;

class ScCompilerRecursionGuard
{
private:
            short&              rRecursion;
public:
                                ScCompilerRecursionGuard( short& rRec )
                                    : rRecursion( rRec ) { ++rRecursion; }
                                ~ScCompilerRecursionGuard() { --rRecursion; }
};

void ScCompiler::fillFromAddInMap( NonConstOpCodeMapPtr xMap,FormulaGrammar::Grammar _eGrammar  ) const
{
    size_t nSymbolOffset;
    switch( _eGrammar )
    {
        case FormulaGrammar::GRAM_PODF:
            nSymbolOffset = offsetof( AddInMap, pUpper);
            break;
        default:
        case FormulaGrammar::GRAM_ODFF:
            nSymbolOffset = offsetof( AddInMap, pODFF);
            break;
        case FormulaGrammar::GRAM_ENGLISH:
            nSymbolOffset = offsetof( AddInMap, pEnglish);
            break;
    }
    const AddInMap* pMap = GetAddInMap();
    const AddInMap* const pStop = pMap + GetAddInMapCount();
    for ( ; pMap < pStop; ++pMap)
    {
        char const * const * ppSymbol =
            reinterpret_cast< char const * const * >(
                    reinterpret_cast< char const * >(pMap) + nSymbolOffset);
        xMap->putExternal( OUString::createFromAscii( *ppSymbol),
                OUString::createFromAscii( pMap->pOriginal));
    }
}

void ScCompiler::fillFromAddInCollectionUpperName( NonConstOpCodeMapPtr xMap ) const
{
    ScUnoAddInCollection* pColl = ScGlobal::GetAddInCollection();
    long nCount = pColl->GetFuncCount();
    for (long i=0; i < nCount; ++i)
    {
        const ScUnoAddInFuncData* pFuncData = pColl->GetFuncData(i);
        if (pFuncData)
            xMap->putExternalSoftly( pFuncData->GetUpperName(),
                    pFuncData->GetOriginalName());
    }
}

void ScCompiler::fillFromAddInCollectionEnglishName( NonConstOpCodeMapPtr xMap ) const
{
    ScUnoAddInCollection* pColl = ScGlobal::GetAddInCollection();
    long nCount = pColl->GetFuncCount();
    for (long i=0; i < nCount; ++i)
    {
        const ScUnoAddInFuncData* pFuncData = pColl->GetFuncData(i);
        if (pFuncData)
        {
            OUString aName;
            if (pFuncData->GetExcelName( LANGUAGE_ENGLISH_US, aName))
                xMap->putExternalSoftly( aName, pFuncData->GetOriginalName());
            else
                xMap->putExternalSoftly( pFuncData->GetUpperName(),
                        pFuncData->GetOriginalName());
        }
    }
}

void ScCompiler::DeInit()
{
    if (pCharClassEnglish)
    {
        delete pCharClassEnglish;
        pCharClassEnglish = NULL;
    }
}

bool ScCompiler::IsEnglishSymbol( const OUString& rName )
{
    
    OUString aUpper = ScGlobal::pCharClass->uppercase(rName);

    
    OpCode eOp = ScCompiler::GetEnglishOpCode( aUpper );
    if ( eOp != ocNone )
    {
        return true;
    }
    
    if (ScGlobal::GetFuncCollection()->findByName(aUpper))
    {
        return true;
    }

    
    OUString aIntName = ScGlobal::GetAddInCollection()->FindFunction(aUpper, false);
    if (!aIntName.isEmpty())
    {
        return true;
    }
    return false;       
}

void ScCompiler::InitCharClassEnglish()
{
    ::com::sun::star::lang::Locale aLocale( "en", "US", "");
    pCharClassEnglish = new CharClass(
            ::comphelper::getProcessComponentContext(), LanguageTag( aLocale));
}

void ScCompiler::SetGrammar( const FormulaGrammar::Grammar eGrammar )
{
    OSL_ENSURE( eGrammar != FormulaGrammar::GRAM_UNSPECIFIED, "ScCompiler::SetGrammar: don't pass FormulaGrammar::GRAM_UNSPECIFIED");
    if (eGrammar == GetGrammar())
        return;     

    if( eGrammar == FormulaGrammar::GRAM_EXTERNAL )
    {
        meGrammar = eGrammar;
        mxSymbols = GetOpCodeMap( ::com::sun::star::sheet::FormulaLanguage::NATIVE);
    }
    else
    {
        FormulaGrammar::Grammar eMyGrammar = eGrammar;
        const sal_Int32 nFormulaLanguage = FormulaGrammar::extractFormulaLanguage( eMyGrammar);
        OpCodeMapPtr xMap = GetOpCodeMap( nFormulaLanguage);
        OSL_ENSURE( xMap, "ScCompiler::SetGrammar: unknown formula language");
        if (!xMap)
        {
            xMap = GetOpCodeMap( ::com::sun::star::sheet::FormulaLanguage::NATIVE);
            eMyGrammar = xMap->getGrammar();
        }

        
        FormulaGrammar::Grammar eOldGrammar = GetGrammar();
        
        SetFormulaLanguage( xMap);

        
        if (eMyGrammar != GetGrammar())
            SetGrammarAndRefConvention( eMyGrammar, eOldGrammar);
    }

    if (pDoc && maTabNames.empty())
    {
        maTabNames = pDoc->GetAllTableNames();
        std::vector<OUString>::iterator it = maTabNames.begin(), itEnd = maTabNames.end();
        for (; it != itEnd; ++it)
            ScCompiler::CheckTabQuotes(*it, formula::FormulaGrammar::extractRefConvention(meGrammar));
    }
}

void ScCompiler::SetNumberFormatter( SvNumberFormatter* pFormatter )
{
    mpFormatter = pFormatter;
}

void ScCompiler::SetFormulaLanguage( const ScCompiler::OpCodeMapPtr & xMap )
{
    if (xMap.get())
    {
        mxSymbols = xMap;
        if (mxSymbols->isEnglish())
        {
            if (!pCharClassEnglish)
                InitCharClassEnglish();
            pCharClass = pCharClassEnglish;
        }
        else
            pCharClass = ScGlobal::pCharClass;
        SetGrammarAndRefConvention( mxSymbols->getGrammar(), GetGrammar());
    }
}

void ScCompiler::SetGrammarAndRefConvention(
        const FormulaGrammar::Grammar eNewGrammar, const FormulaGrammar::Grammar eOldGrammar )
{
    meGrammar = eNewGrammar;    
    FormulaGrammar::AddressConvention eConv = FormulaGrammar::extractRefConvention( meGrammar);
    if (eConv == FormulaGrammar::CONV_UNSPECIFIED && eOldGrammar == FormulaGrammar::GRAM_UNSPECIFIED)
    {
        if (pDoc)
            SetRefConvention( pDoc->GetAddressConvention());
        else
            SetRefConvention( GetRefConvention( FormulaGrammar::CONV_OOO ) );
    }
    else
        SetRefConvention( eConv );
}

OUString ScCompiler::FindAddInFunction( const OUString& rUpperName, bool bLocalFirst ) const
{
    return ScGlobal::GetAddInCollection()->FindFunction(rUpperName, bLocalFirst);    
}

ScCompiler::Convention::~Convention()
{
    delete [] mpCharTable;
    mpCharTable = NULL;
}

ScCompiler::Convention::Convention( FormulaGrammar::AddressConvention eConv )
        :
    meConv( eConv )
{
    int i;
    sal_uLong *t= new sal_uLong [128];

    ScCompiler::pConventions[ meConv ] = this;
    mpCharTable = t;

    for (i = 0; i < 128; i++)
        t[i] = SC_COMPILER_C_ILLEGAL;

/*   */     t[32] = SC_COMPILER_C_CHAR_DONTCARE | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
/* ! */     t[33] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
            if (FormulaGrammar::CONV_ODF == meConv)
/* ! */         t[33] |= SC_COMPILER_C_ODF_LABEL_OP;
/* " */     t[34] = SC_COMPILER_C_CHAR_STRING | SC_COMPILER_C_STRING_SEP;
/* # */     t[35] = SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_CHAR_ERRCONST;
/* $ */     t[36] = SC_COMPILER_C_CHAR_WORD | SC_COMPILER_C_WORD | SC_COMPILER_C_CHAR_IDENT | SC_COMPILER_C_IDENT;
            if (FormulaGrammar::CONV_ODF == meConv)
/* $ */         t[36] |= SC_COMPILER_C_ODF_NAME_MARKER;
/* % */     t[37] = SC_COMPILER_C_VALUE;
/* & */     t[38] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
/* ' */     t[39] = SC_COMPILER_C_NAME_SEP;
/* ( */     t[40] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
/* ) */     t[41] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
/* * */     t[42] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
/* + */     t[43] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_EXP | SC_COMPILER_C_VALUE_SIGN;
/* , */     t[44] = SC_COMPILER_C_CHAR_VALUE | SC_COMPILER_C_VALUE;
/* - */     t[45] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_EXP | SC_COMPILER_C_VALUE_SIGN;
/* . */     t[46] = SC_COMPILER_C_WORD | SC_COMPILER_C_CHAR_VALUE | SC_COMPILER_C_VALUE | SC_COMPILER_C_IDENT | SC_COMPILER_C_NAME;
/* / */     t[47] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;

            for (i = 48; i < 58; i++)
/* 0-9 */       t[i] = SC_COMPILER_C_CHAR_VALUE | SC_COMPILER_C_WORD | SC_COMPILER_C_VALUE | SC_COMPILER_C_VALUE_EXP | SC_COMPILER_C_VALUE_VALUE | SC_COMPILER_C_IDENT | SC_COMPILER_C_NAME;

/* : */     t[58] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD;
/* ; */     t[59] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
/* < */     t[60] = SC_COMPILER_C_CHAR_BOOL | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
/* = */     t[61] = SC_COMPILER_C_CHAR | SC_COMPILER_C_BOOL | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
/* > */     t[62] = SC_COMPILER_C_CHAR_BOOL | SC_COMPILER_C_BOOL | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
/* ? */     t[63] = SC_COMPILER_C_CHAR_WORD | SC_COMPILER_C_WORD | SC_COMPILER_C_NAME;
/* @ */     

    for (i = 65; i < 91; i++)
/* A-Z */   t[i] = SC_COMPILER_C_CHAR_WORD | SC_COMPILER_C_WORD | SC_COMPILER_C_CHAR_IDENT | SC_COMPILER_C_IDENT | SC_COMPILER_C_CHAR_NAME | SC_COMPILER_C_NAME;

    if (FormulaGrammar::CONV_ODF == meConv)
    {
/* [ */     t[91] = SC_COMPILER_C_ODF_LBRACKET;
/* \ */     
/* ] */     t[93] = SC_COMPILER_C_ODF_RBRACKET;
    }
    else
    {
/* [ */     
/* \ */     
/* ] */     
    }
/* ^ */     t[94] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
/* _ */     t[95] = SC_COMPILER_C_CHAR_WORD | SC_COMPILER_C_WORD | SC_COMPILER_C_CHAR_IDENT | SC_COMPILER_C_IDENT | SC_COMPILER_C_CHAR_NAME | SC_COMPILER_C_NAME;
/* ` */     

            for (i = 97; i < 123; i++)
/* a-z */       t[i] = SC_COMPILER_C_CHAR_WORD | SC_COMPILER_C_WORD | SC_COMPILER_C_CHAR_IDENT | SC_COMPILER_C_IDENT | SC_COMPILER_C_CHAR_NAME | SC_COMPILER_C_NAME;

/* { */     t[123] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP; 
/* | */     t[124] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP; 
/* } */     t[125] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP; 
/* ~ */     t[126] = SC_COMPILER_C_CHAR;        
/* 127 */   

    if( FormulaGrammar::CONV_XL_A1 == meConv || FormulaGrammar::CONV_XL_R1C1 == meConv || FormulaGrammar::CONV_XL_OOX == meConv )
    {
/*   */     t[32] |=   SC_COMPILER_C_WORD;
/* ! */     t[33] |=   SC_COMPILER_C_IDENT | SC_COMPILER_C_WORD;
/* " */     t[34] |=   SC_COMPILER_C_WORD;
/* # */     t[35] &= (~SC_COMPILER_C_WORD_SEP);
/* # */     t[35] |=   SC_COMPILER_C_WORD;
/* % */     t[37] |=   SC_COMPILER_C_WORD;
/* ' */     t[39] |=   SC_COMPILER_C_WORD;

/* % */     t[37] |=   SC_COMPILER_C_WORD;
/* & */     t[38] |=   SC_COMPILER_C_WORD;
/* ' */     t[39] |=   SC_COMPILER_C_WORD;
/* ( */     t[40] |=   SC_COMPILER_C_WORD;
/* ) */     t[41] |=   SC_COMPILER_C_WORD;
/* * */     t[42] |=   SC_COMPILER_C_WORD;
/* + */     t[43] |=   SC_COMPILER_C_WORD;
#if 0 /* this really needs to be locale specific. */
/* , */     t[44]  =   SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
#else
/* , */     t[44] |=   SC_COMPILER_C_WORD;
#endif
/* - */     t[45] |=   SC_COMPILER_C_WORD;

/* ; */     t[59] |=   SC_COMPILER_C_WORD;
/* < */     t[60] |=   SC_COMPILER_C_WORD;
/* = */     t[61] |=   SC_COMPILER_C_WORD;
/* > */     t[62] |=   SC_COMPILER_C_WORD;
/* ? */     
/* @ */     t[64] |=   SC_COMPILER_C_WORD;
/* [ */     t[91] |=   SC_COMPILER_C_WORD;
/* ] */     t[93] |=   SC_COMPILER_C_WORD;
/* { */     t[123]|=   SC_COMPILER_C_WORD;
/* | */     t[124]|=   SC_COMPILER_C_WORD;
/* } */     t[125]|=   SC_COMPILER_C_WORD;
/* ~ */     t[126]|=   SC_COMPILER_C_WORD;

        if( FormulaGrammar::CONV_XL_R1C1 == meConv )
        {
/* [ */     t[91] |= SC_COMPILER_C_IDENT;
/* ] */     t[93] |= SC_COMPILER_C_IDENT;
        }
        if( FormulaGrammar::CONV_XL_OOX == meConv )
        {
/* [ */     t[91] |= SC_COMPILER_C_CHAR_IDENT;
/* ] */     t[93] |= SC_COMPILER_C_IDENT;
        }
    }
}

static bool lcl_isValidQuotedText( const OUString& rFormula, sal_Int32 nSrcPos, ParseResult& rRes )
{
    
    
    
    
    if (rFormula[nSrcPos] == '\'')
    {
        sal_Int32 nPos = nSrcPos+1;
        while (nPos < rFormula.getLength())
        {
            if (rFormula[nPos] == '\'')
            {
                if ( (nPos+1 == rFormula.getLength()) || (rFormula[nPos+1] != '\'') )
                {
                    rRes.TokenType = KParseType::SINGLE_QUOTE_NAME;
                    rRes.EndPos = nPos+1;
                    return true;
                }
                ++nPos;
            }
            ++nPos;
        }
    }

    return false;
}

static bool lcl_parseExternalName(
        const OUString& rSymbol,
        OUString& rFile,
        OUString& rName,
        const sal_Unicode cSep,
        const ScDocument* pDoc = NULL,
        const uno::Sequence< const sheet::ExternalLinkInfo > * pExternalLinks = NULL )
{
    /* TODO: future versions will have to support sheet-local names too, thus
     * return a possible sheet name as well. */
    const sal_Unicode* const pStart = rSymbol.getStr();
    const sal_Unicode* p = pStart;
    sal_Int32 nLen = rSymbol.getLength();
    sal_Unicode cPrev = 0;
    OUString aTmpFile, aTmpName;
    sal_Int32 i = 0;
    bool bInName = false;
    if (cSep == '!')
    {
        
        
        ScRange aRange;
        OUString aStartTabName, aEndTabName;
        sal_uInt16 nFlags = 0;
        p = aRange.Parse_XL_Header( p, pDoc, aTmpFile, aStartTabName,
                aEndTabName, nFlags, true, pExternalLinks );
        if (!p || p == pStart)
            return false;
        i = sal_Int32(p - pStart);
        cPrev = *(p-1);
    }
    for ( ; i < nLen; ++i, ++p)
    {
        sal_Unicode c = *p;
        if (i == 0)
        {
            if (c == '.' || c == cSep)
                return false;

            if (c == '\'')
            {
                
                
                cPrev = c;
                ++i; ++p;
                for (sal_Int32 j = i; j < nLen; ++j, ++p)
                {
                    c = *p;
                    if (c == '\'')
                    {
                        if (j == i)
                        {
                            
                            return false;
                        }

                        if (cPrev == '\'')
                        {
                            
                            
                            aTmpFile += OUString(c);
                            cPrev = 'a';
                        }
                        else
                            cPrev = c;

                        continue;
                    }

                    if (cPrev == '\'' && j != i)
                    {
                        
                        
                        
                        

                        i = j;
                        bInName = true;
                        aTmpName += OUString(c); 
                        break;
                    }
                    aTmpFile += OUString(c);
                    cPrev = c;
                }

                if (!bInName)
                {
                    
                    return false;
                }

                if (c != cSep)
                {
                    
                    return false;
                }

                cPrev = c;
                continue;
            }
        }

        if (bInName)
        {
            if (c == cSep)
            {
                
                return false;
            }
            aTmpName += OUString(c);
        }
        else
        {
            if (c == cSep)
            {
                bInName = true;
                aTmpName += OUString(c); 
            }
            else
            {
                do
                {
                    if (rtl::isAsciiAlphanumeric(c))
                        
                        break;

                    if (c > 128)
                        
                        break;

                    bool bValid = false;
                    switch (c)
                    {
                        case '_':
                        case '-':
                        case '.':
                            
                            bValid = true;
                            break;
                    }
                    if (bValid)
                        break;

                    return false;
                }
                while (false);
                aTmpFile += OUString(c);
            }
        }
        cPrev = c;
    }

    if (!bInName)
    {
        
        return false;
    }

    sal_Int32 nNameLen = aTmpName.getLength();
    if (nNameLen < 2)
    {
        
        return false;
    }

    if (aTmpName[0] != cSep)
    {
        
        return false;
    }

    if (aTmpName[nNameLen-1] == '!')
    {
        
        if (aTmpName.equalsAscii("#REF!"))
            return false;
    }

    rFile = aTmpFile;
    rName = aTmpName.copy(1); 
    return true;
}

static OUString lcl_makeExternalNameStr(const OUString& rFile, const OUString& rName,
        const sal_Unicode cSep, bool bODF )
{
    OUString aEscQuote("''");
    OUString aFile(rFile.replaceAll("'", aEscQuote));
    OUString aName(rName);
    if (bODF)
        aName = aName.replaceAll("'", aEscQuote);
    OUStringBuffer aBuf(aFile.getLength() + aName.getLength() + 9);
    if (bODF)
        aBuf.append( '[');
    aBuf.append( "'" + aFile + "'" + OUString(cSep));
    if (bODF)
        aBuf.append( "$$'" );
    aBuf.append( aName);
    if (bODF)
        aBuf.append( "']" );
    return aBuf.makeStringAndClear();
}

static bool lcl_getLastTabName( OUString& rTabName2, const OUString& rTabName1,
                                const vector<OUString>& rTabNames, const ScRange& rRef )
{
    SCsTAB nTabSpan = rRef.aEnd.Tab() - rRef.aStart.Tab();
    if (nTabSpan > 0)
    {
        size_t nCount = rTabNames.size();
        vector<OUString>::const_iterator itrBeg = rTabNames.begin(), itrEnd = rTabNames.end();
        vector<OUString>::const_iterator itr = ::std::find(itrBeg, itrEnd, rTabName1);
        if (itr == rTabNames.end())
        {
            rTabName2 = ScGlobal::GetRscString(STR_NO_REF_TABLE);
            return false;
        }

        size_t nDist = ::std::distance(itrBeg, itr);
        if (nDist + static_cast<size_t>(nTabSpan) >= nCount)
        {
            rTabName2 = ScGlobal::GetRscString(STR_NO_REF_TABLE);
            return false;
        }

        rTabName2 = rTabNames[nDist+nTabSpan];
    }
    else
        rTabName2 = rTabName1;

    return true;
}

struct Convention_A1 : public ScCompiler::Convention
{
    Convention_A1( FormulaGrammar::AddressConvention eConv ) : ScCompiler::Convention( eConv ) { }
    static void MakeColStr( OUStringBuffer& rBuffer, SCCOL nCol );
    static void MakeRowStr( OUStringBuffer& rBuffer, SCROW nRow );

    ParseResult parseAnyToken( const OUString& rFormula,
                               sal_Int32 nSrcPos,
                               const CharClass* pCharClass) const
    {
        ParseResult aRet;
        if ( lcl_isValidQuotedText(rFormula, nSrcPos, aRet) )
            return aRet;

        static const sal_Int32 nStartFlags = KParseTokens::ANY_LETTER_OR_NUMBER |
            KParseTokens::ASC_UNDERSCORE | KParseTokens::ASC_DOLLAR;
        static const sal_Int32 nContFlags = nStartFlags | KParseTokens::ASC_DOT;
        
        static const OUString aAddAllowed("?#");
        return pCharClass->parseAnyToken( rFormula,
                nSrcPos, nStartFlags, aAddAllowed, nContFlags, aAddAllowed );
    }

    virtual sal_uLong getCharTableFlags( sal_Unicode c, sal_Unicode /*cLast*/ ) const
    {
        return mpCharTable[static_cast<sal_uInt8>(c)];
    }
};

void Convention_A1::MakeColStr( OUStringBuffer& rBuffer, SCCOL nCol )
{
    if ( !ValidCol( nCol) )
        rBuffer.append(ScGlobal::GetRscString(STR_NO_REF_TABLE));
    else
        ::ScColToAlpha( rBuffer, nCol);
}

void Convention_A1::MakeRowStr( OUStringBuffer& rBuffer, SCROW nRow )
{
    if ( !ValidRow(nRow) )
        rBuffer.append(ScGlobal::GetRscString(STR_NO_REF_TABLE));
    else
        rBuffer.append(sal_Int32(nRow + 1));
}

struct ConventionOOO_A1 : public Convention_A1
{
    ConventionOOO_A1() : Convention_A1 (FormulaGrammar::CONV_OOO) { }
    ConventionOOO_A1( FormulaGrammar::AddressConvention eConv ) : Convention_A1 (eConv) { }

    static OUString MakeTabStr( const std::vector<OUString>& rTabNames, SCTAB nTab )
    {
        OUString aString;
        if (static_cast<size_t>(nTab) >= rTabNames.size())
            aString = ScGlobal::GetRscString(STR_NO_REF_TABLE);
        else
            aString = rTabNames[nTab];
        aString += ".";
        return aString;
    }

    void MakeOneRefStrImpl(
        OUStringBuffer& rBuffer,
        const OUString& rErrRef, const std::vector<OUString>& rTabNames,
        const ScSingleRefData& rRef, const ScAddress& rAbsRef,
        bool bForceTab, bool bODF ) const
    {
        if( rRef.IsFlag3D() || bForceTab )
        {
            if (!ValidTab(rAbsRef.Tab()) || rRef.IsTabDeleted())
            {
                if (!rRef.IsTabRel())
                    rBuffer.append('$');
                rBuffer.append(rErrRef);
                rBuffer.append('.');
            }
            else
            {
                OUString aRefStr(MakeTabStr(rTabNames, rAbsRef.Tab()));
                if (!rRef.IsTabRel())
                    rBuffer.append('$');
                rBuffer.append(aRefStr);
            }
        }
        else if (bODF)
            rBuffer.append('.');
        if (!rRef.IsColRel())
            rBuffer.append('$');
        if (!ValidCol(rAbsRef.Col()))
            rBuffer.append(rErrRef);
        else
            MakeColStr(rBuffer, rAbsRef.Col());
        if (!rRef.IsRowRel())
            rBuffer.append('$');
        if (!ValidRow(rAbsRef.Row()))
            rBuffer.append(rErrRef);
        else
            MakeRowStr(rBuffer, rAbsRef.Row());
    }

    void makeRefStr( OUStringBuffer&   rBuffer,
                     formula::FormulaGrammar::Grammar /*eGram*/,
                     const ScAddress& rPos,
                     const OUString& rErrRef, const std::vector<OUString>& rTabNames,
                     const ScComplexRefData& rRef,
                     bool bSingleRef ) const
    {
        ScComplexRefData aRef( rRef );
        
        
        ScAddress aAbs1 = aRef.Ref1.toAbs(rPos), aAbs2;
        if( !bSingleRef )
            aAbs2 = aRef.Ref2.toAbs(rPos);

        MakeOneRefStrImpl(rBuffer, rErrRef, rTabNames, aRef.Ref1, aAbs1, false, false);
        if (!bSingleRef)
        {
            rBuffer.append(':');
            MakeOneRefStrImpl(rBuffer, rErrRef, rTabNames, aRef.Ref2, aAbs2, aAbs1.Tab() != aAbs2.Tab(), false);
        }
    }

    virtual sal_Unicode getSpecialSymbol( SpecialSymbolType eSymType ) const
    {
        switch (eSymType)
        {
            case ScCompiler::Convention::ABS_SHEET_PREFIX:
                return '$';
            case ScCompiler::Convention::SHEET_SEPARATOR:
                return '.';
        }

        return sal_Unicode(0);
    }

    virtual bool parseExternalName( const OUString& rSymbol, OUString& rFile, OUString& rName,
            const ScDocument* pDoc,
            const ::com::sun::star::uno::Sequence<
                const ::com::sun::star::sheet::ExternalLinkInfo > * pExternalLinks ) const
    {
        return lcl_parseExternalName(rSymbol, rFile, rName, '#', pDoc, pExternalLinks);
    }

    virtual OUString makeExternalNameStr( const OUString& rFile, const OUString& rName ) const
    {
        return lcl_makeExternalNameStr( rFile, rName, '#', false);
    }

    bool makeExternalSingleRefStr(
        OUStringBuffer& rBuffer, const OUString& rFileName, const OUString& rTabName,
        const ScSingleRefData& rRef, const ScAddress& rPos, bool bDisplayTabName, bool bEncodeUrl ) const
    {
        ScAddress aAbsRef = rRef.toAbs(rPos);
        if (bDisplayTabName)
        {
            OUString aFile;
            if (bEncodeUrl)
                aFile = rFileName;
            else
                aFile = INetURLObject::decode(rFileName, INET_HEX_ESCAPE, INetURLObject::DECODE_UNAMBIGUOUS);

            rBuffer.append("'" + aFile.replaceAll("'", "''") + "'#");

            if (!rRef.IsTabRel())
                rBuffer.append('$');
            ScRangeStringConverter::AppendTableName(rBuffer, rTabName);

            rBuffer.append('.');
        }

        if (!rRef.IsColRel())
            rBuffer.append('$');
        MakeColStr( rBuffer, aAbsRef.Col());
        if (!rRef.IsRowRel())
            rBuffer.append('$');
        MakeRowStr( rBuffer, aAbsRef.Row());

        return true;
    }

    void makeExternalRefStrImpl(
        OUStringBuffer& rBuffer, const ScAddress& rPos, const OUString& rFileName,
        const OUString& rTabName, const ScSingleRefData& rRef, bool bODF ) const
    {
        if (bODF)
            rBuffer.append( '[');

        bool bEncodeUrl = bODF;
        makeExternalSingleRefStr(rBuffer, rFileName, rTabName, rRef, rPos, true, bEncodeUrl);
        if (bODF)
            rBuffer.append( ']');
    }

    virtual void makeExternalRefStr(
        OUStringBuffer& rBuffer, const ScAddress& rPos, const OUString& rFileName,
        const OUString& rTabName, const ScSingleRefData& rRef ) const
    {
        makeExternalRefStrImpl(rBuffer, rPos, rFileName, rTabName, rRef, false);
    }

    void makeExternalRefStrImpl(
        OUStringBuffer& rBuffer, const ScAddress& rPos, const OUString& rFileName,
        const std::vector<OUString>& rTabNames, const OUString& rTabName,
        const ScComplexRefData& rRef, bool bODF ) const
    {
        ScRange aAbsRange = rRef.toAbs(rPos);

        if (bODF)
            rBuffer.append( '[');
        
        bool bEncodeUrl = bODF;

        do
        {
            if (!makeExternalSingleRefStr(rBuffer, rFileName, rTabName, rRef.Ref1, rPos, true, bEncodeUrl))
                break;

            rBuffer.append(':');

            OUString aLastTabName;
            bool bDisplayTabName = (aAbsRange.aStart.Tab() != aAbsRange.aEnd.Tab());
            if (bDisplayTabName)
            {
                
                if (!lcl_getLastTabName(aLastTabName, rTabName, rTabNames, aAbsRange))
                {
                    OSL_FAIL( "ConventionOOO_A1::makeExternalRefStrImpl: sheet name not found");
                    
                }
            }
            else if (bODF)
                rBuffer.append( '.');      
            makeExternalSingleRefStr(
                rBuffer, rFileName, aLastTabName, rRef.Ref2, rPos, bDisplayTabName, bEncodeUrl);
        } while (false);

        if (bODF)
            rBuffer.append( ']');
    }

    virtual void makeExternalRefStr(
        OUStringBuffer& rBuffer, const ScAddress& rPos, const OUString& rFileName,
        const std::vector<OUString>& rTabNames, const OUString& rTabName,
        const ScComplexRefData& rRef ) const
    {
        makeExternalRefStrImpl(rBuffer, rPos, rFileName, rTabNames, rTabName, rRef, false);
    }
};

struct ConventionOOO_A1_ODF : public ConventionOOO_A1
{
    ConventionOOO_A1_ODF() : ConventionOOO_A1 (FormulaGrammar::CONV_ODF) { }
    void makeRefStr( OUStringBuffer&   rBuffer,
                     formula::FormulaGrammar::Grammar eGram,
                     const ScAddress& rPos,
                     const OUString& rErrRef, const std::vector<OUString>& rTabNames,
                     const ScComplexRefData& rRef,
                     bool bSingleRef ) const
    {
        rBuffer.append('[');
        ScComplexRefData aRef( rRef );
        
        
        ScAddress aAbs1 = aRef.Ref1.toAbs(rPos), aAbs2;
        if( !bSingleRef )
            aAbs2 = aRef.Ref2.toAbs(rPos);

        if (FormulaGrammar::isODFF(eGram) && (!ValidAddress(aAbs1) || !ValidAddress(aAbs2)))
        {
            rBuffer.append(rErrRef);
            
            
            
        }
        else
        {
            MakeOneRefStrImpl(rBuffer, rErrRef, rTabNames, aRef.Ref1, aAbs1, false, true);
            if (!bSingleRef)
            {
                rBuffer.append(':');
                MakeOneRefStrImpl(rBuffer, rErrRef, rTabNames, aRef.Ref2, aAbs2, aAbs1.Tab() != aAbs2.Tab(), true);
            }
        }
        rBuffer.append(']');
    }

    virtual OUString makeExternalNameStr( const OUString& rFile, const OUString& rName ) const
    {
        return lcl_makeExternalNameStr( rFile, rName, '#', true);
    }

    virtual void makeExternalRefStr(
        OUStringBuffer& rBuffer, const ScAddress& rPos, const OUString& rFileName,
        const OUString& rTabName, const ScSingleRefData& rRef ) const
    {
        makeExternalRefStrImpl(rBuffer, rPos, rFileName, rTabName, rRef, true);
    }

    virtual void makeExternalRefStr(
        OUStringBuffer& rBuffer, const ScAddress& rPos, const OUString& rFileName,
        const std::vector<OUString>& rTabNames,
        const OUString& rTabName, const ScComplexRefData& rRef ) const
    {
        makeExternalRefStrImpl(rBuffer, rPos, rFileName, rTabNames, rTabName, rRef, true);
    }
};

struct ConventionXL
{
    static void GetTab(
        const ScAddress& rPos, const std::vector<OUString>& rTabNames,
        const ScSingleRefData& rRef, OUString& rTabName )
    {
        ScAddress aAbs = rRef.toAbs(rPos);
        if (rRef.IsTabDeleted() || static_cast<size_t>(aAbs.Tab()) >= rTabNames.size())
        {
            rTabName = ScGlobal::GetRscString( STR_NO_REF_TABLE );
            return;
        }
        rTabName = rTabNames[aAbs.Tab()];
    }

    static void MakeTabStr( OUStringBuffer& rBuf,
                            const ScAddress& rPos,
                            const std::vector<OUString>& rTabNames,
                            const ScComplexRefData& rRef,
                            bool bSingleRef )
    {
        if( rRef.Ref1.IsFlag3D() )
        {
            OUString aStartTabName, aEndTabName;

            GetTab(rPos, rTabNames, rRef.Ref1, aStartTabName);

            if( !bSingleRef && rRef.Ref2.IsFlag3D() )
            {
                GetTab(rPos, rTabNames, rRef.Ref2, aEndTabName);
            }

            rBuf.append( aStartTabName );
            if( !bSingleRef && rRef.Ref2.IsFlag3D() && aStartTabName != aEndTabName )
            {
                rBuf.append( ':' );
                rBuf.append( aEndTabName );
            }

            rBuf.append( '!' );
        }
    }

    static sal_Unicode getSpecialSymbol( ScCompiler::Convention::SpecialSymbolType eSymType )
    {
        switch (eSymType)
        {
            case ScCompiler::Convention::ABS_SHEET_PREFIX:
                return sal_Unicode(0);
            case ScCompiler::Convention::SHEET_SEPARATOR:
                return '!';
        }
        return sal_Unicode(0);
    }

    static bool parseExternalName( const OUString& rSymbol, OUString& rFile, OUString& rName,
            const ScDocument* pDoc,
            const ::com::sun::star::uno::Sequence<
                const ::com::sun::star::sheet::ExternalLinkInfo > * pExternalLinks )
    {
        return lcl_parseExternalName( rSymbol, rFile, rName, '!', pDoc, pExternalLinks);
    }

    static OUString makeExternalNameStr( const OUString& rFile, const OUString& rName )
    {
        return lcl_makeExternalNameStr( rFile, rName, '!', false);
    }

    static void makeExternalDocStr( OUStringBuffer& rBuffer, const OUString& rFullName, bool bEncodeUrl )
    {
        
        
        
        
        //
        

        rBuffer.append('[');
        rBuffer.append('\'');
        OUString aFullName;
        if (bEncodeUrl)
            aFullName = rFullName;
        else
            aFullName = INetURLObject::decode(rFullName, INET_HEX_ESCAPE, INetURLObject::DECODE_UNAMBIGUOUS);

        const sal_Unicode* pBuf = aFullName.getStr();
        sal_Int32 nLen = aFullName.getLength();
        for (sal_Int32 i = 0; i < nLen; ++i)
        {
            const sal_Unicode c = pBuf[i];
            if (c == '\'')
                rBuffer.append(c);
            rBuffer.append(c);
        }
        rBuffer.append('\'');
        rBuffer.append(']');
    }

    static void makeExternalTabNameRange( OUStringBuffer& rBuf, const OUString& rTabName,
                                          const vector<OUString>& rTabNames,
                                          const ScRange& rRef )
    {
        OUString aLastTabName;
        if (!lcl_getLastTabName(aLastTabName, rTabName, rTabNames, rRef))
        {
            ScRangeStringConverter::AppendTableName(rBuf, aLastTabName);
            return;
        }

        ScRangeStringConverter::AppendTableName(rBuf, rTabName);
        if (rTabName != aLastTabName)
        {
            rBuf.append(':');
            ScRangeStringConverter::AppendTableName(rBuf, rTabName);
        }
    }

    static void parseExternalDocName( const OUString& rFormula, sal_Int32& rSrcPos )
    {
        sal_Int32 nLen = rFormula.getLength();
        const sal_Unicode* p = rFormula.getStr();
        sal_Unicode cPrev = 0;
        for (sal_Int32 i = rSrcPos; i < nLen; ++i)
        {
            sal_Unicode c = p[i];
            if (i == rSrcPos)
            {
                
                if (c != '[')
                    return;
            }
            else if (i == rSrcPos + 1)
            {
                
                if (c != '\'')
                    return;
            }
            else if (c == '\'')
            {
                if (cPrev == '\'')
                    
                    
                    c = 'a';
            }
            else if (c == ']')
            {
                if (cPrev == '\'')
                {
                    
                    
                    rSrcPos = i + 1;
                    if (rSrcPos >= nLen)
                        rSrcPos = nLen - 1;
                    return;
                }
                else
                    return;
            }
            else
            {
                
                if (i > rSrcPos + 2 && cPrev == '\'')
                    
                    
                    return;
            }
            cPrev = c;
        }
    }
};

struct ConventionXL_A1 : public Convention_A1, public ConventionXL
{
    ConventionXL_A1() : Convention_A1( FormulaGrammar::CONV_XL_A1 ) { }
    ConventionXL_A1( FormulaGrammar::AddressConvention eConv ) : Convention_A1( eConv ) { }

    void makeSingleCellStr( OUStringBuffer& rBuf, const ScSingleRefData& rRef, const ScAddress& rAbs ) const
    {
        if (!rRef.IsColRel())
            rBuf.append('$');
        MakeColStr(rBuf, rAbs.Col());
        if (!rRef.IsRowRel())
            rBuf.append('$');
        MakeRowStr(rBuf, rAbs.Row());
    }

    void makeRefStr( OUStringBuffer&   rBuf,
                     formula::FormulaGrammar::Grammar /*eGram*/,
                     const ScAddress& rPos,
                     const OUString& /*rErrRef*/, const std::vector<OUString>& rTabNames,
                     const ScComplexRefData& rRef,
                     bool bSingleRef ) const
    {
        ScComplexRefData aRef( rRef );

        
        
        ScAddress aAbs1 = aRef.Ref1.toAbs(rPos), aAbs2;

        MakeTabStr(rBuf, rPos, rTabNames, aRef, bSingleRef);

        if (!ValidAddress(aAbs1))
        {
            rBuf.append(ScGlobal::GetRscString(STR_NO_REF_TABLE));
            return;
        }

        if( !bSingleRef )
        {
            aAbs2 = aRef.Ref2.toAbs(rPos);
            if (!ValidAddress(aAbs2))
            {
                rBuf.append(ScGlobal::GetRscString(STR_NO_REF_TABLE));
                return;
            }

            if (aAbs1.Col() == 0 && aAbs2.Col() >= MAXCOL)
            {
                if (!aRef.Ref1.IsRowRel())
                    rBuf.append( '$' );
                MakeRowStr(rBuf, aAbs1.Row());
                rBuf.append( ':' );
                if (!aRef.Ref2.IsRowRel())
                    rBuf.append( '$' );
                MakeRowStr(rBuf, aAbs2.Row());
                return;
            }

            if (aAbs1.Row() == 0 && aAbs2.Row() >= MAXROW)
            {
                if (!aRef.Ref1.IsColRel())
                    rBuf.append( '$' );
                MakeColStr(rBuf, aAbs1.Col());
                rBuf.append( ':' );
                if (!aRef.Ref2.IsColRel())
                    rBuf.append( '$' );
                MakeColStr(rBuf, aAbs2.Col());
                return;
            }
        }

        makeSingleCellStr(rBuf, aRef.Ref1, aAbs1);
        if (!bSingleRef)
        {
            rBuf.append( ':' );
            makeSingleCellStr(rBuf, aRef.Ref2, aAbs2);
        }
    }

    virtual ParseResult parseAnyToken( const OUString& rFormula,
                                       sal_Int32 nSrcPos,
                                       const CharClass* pCharClass) const
    {
        ParseResult aRet;
        if ( lcl_isValidQuotedText(rFormula, nSrcPos, aRet) )
            return aRet;

        static const sal_Int32 nStartFlags = KParseTokens::ANY_LETTER_OR_NUMBER |
            KParseTokens::ASC_UNDERSCORE | KParseTokens::ASC_DOLLAR;
        static const sal_Int32 nContFlags = nStartFlags | KParseTokens::ASC_DOT;
        
        const OUString aAddAllowed("?!");
        return pCharClass->parseAnyToken( rFormula,
                nSrcPos, nStartFlags, aAddAllowed, nContFlags, aAddAllowed );
    }

    virtual sal_Unicode getSpecialSymbol( SpecialSymbolType eSymType ) const
    {
        return ConventionXL::getSpecialSymbol(eSymType);
    }

    virtual bool parseExternalName( const OUString& rSymbol, OUString& rFile, OUString& rName,
            const ScDocument* pDoc,
            const ::com::sun::star::uno::Sequence<
                const ::com::sun::star::sheet::ExternalLinkInfo > * pExternalLinks ) const
    {
        return ConventionXL::parseExternalName( rSymbol, rFile, rName, pDoc, pExternalLinks);
    }

    virtual OUString makeExternalNameStr( const OUString& rFile, const OUString& rName ) const
    {
        return ConventionXL::makeExternalNameStr(rFile, rName);
    }

    virtual void makeExternalRefStr(
        OUStringBuffer& rBuffer, const ScAddress& rPos, const OUString& rFileName,
        const OUString& rTabName, const ScSingleRefData& rRef ) const
    {
        
        
        
        
        

        ConventionXL::makeExternalDocStr(rBuffer, rFileName, false);
        ScRangeStringConverter::AppendTableName(rBuffer, rTabName);
        rBuffer.append('!');

        makeSingleCellStr(rBuffer, rRef, rRef.toAbs(rPos));
    }

    virtual void makeExternalRefStr(
        OUStringBuffer& rBuffer, const ScAddress& rPos, const OUString& rFileName,
        const std::vector<OUString>& rTabNames, const OUString& rTabName,
        const ScComplexRefData& rRef ) const
    {
        ScRange aAbsRef = rRef.toAbs(rPos);

        ConventionXL::makeExternalDocStr(rBuffer, rFileName, false);
        ConventionXL::makeExternalTabNameRange(rBuffer, rTabName, rTabNames, aAbsRef);
        rBuffer.append('!');

        makeSingleCellStr(rBuffer, rRef.Ref1, aAbsRef.aStart);
        if (aAbsRef.aStart != aAbsRef.aEnd)
        {
            rBuffer.append(':');
            makeSingleCellStr(rBuffer, rRef.Ref2, aAbsRef.aEnd);
        }
    }
};

struct ConventionXL_OOX : public ConventionXL_A1
{
    ConventionXL_OOX() : ConventionXL_A1( FormulaGrammar::CONV_XL_OOX ) { }
};

static void
r1c1_add_col( OUStringBuffer &rBuf, const ScSingleRefData& rRef, const ScAddress& rAbsRef )
{
    rBuf.append( 'C' );
    if( rRef.IsColRel() )
    {
        SCCOL nCol = rRef.Col();
        if (nCol != 0)
            rBuf.append("[").append(OUString::number(nCol)).append("]");
    }
    else
        rBuf.append( OUString::number( rAbsRef.Col() + 1 ) );
}
static void
r1c1_add_row( OUStringBuffer &rBuf, const ScSingleRefData& rRef, const ScAddress& rAbsRef )
{
    rBuf.append( 'R' );
    if( rRef.IsRowRel() )
    {
        if (rRef.Row() != 0)
        {
            rBuf.append("[").append( OUString::number(rRef.Row()) ).append("]");
        }
    }
    else
        rBuf.append( OUString::number( rAbsRef.Row() + 1 ) );
}

struct ConventionXL_R1C1 : public ScCompiler::Convention, public ConventionXL
{
    ConventionXL_R1C1() : ScCompiler::Convention( FormulaGrammar::CONV_XL_R1C1 ) { }
    void makeRefStr( OUStringBuffer&   rBuf,
                     formula::FormulaGrammar::Grammar /*eGram*/,
                     const ScAddress& rPos,
                     const OUString& /*rErrRef*/, const std::vector<OUString>& rTabNames,
                     const ScComplexRefData& rRef,
                     bool bSingleRef ) const
    {
        ScRange aAbsRef = rRef.toAbs(rPos);
        ScComplexRefData aRef( rRef );

        MakeTabStr(rBuf, rPos, rTabNames, aRef, bSingleRef);

        
        
        if (!ValidCol(aAbsRef.aStart.Col()) || !ValidRow(aAbsRef.aStart.Row()))
        {
            rBuf.append(ScGlobal::GetRscString(STR_NO_REF_TABLE));
            return;
        }

        if( !bSingleRef )
        {
            if (!ValidCol(aAbsRef.aEnd.Col()) || !ValidRow(aAbsRef.aEnd.Row()))
            {
                rBuf.append(ScGlobal::GetRscString(STR_NO_REF_TABLE));
                return;
            }

            if (aAbsRef.aStart.Col() == 0 && aAbsRef.aEnd.Col() >= MAXCOL)
            {
                r1c1_add_row(rBuf,  rRef.Ref1, aAbsRef.aStart);
                if (aAbsRef.aStart.Row() != aAbsRef.aEnd.Row() ||
                    rRef.Ref1.IsRowRel() != rRef.Ref2.IsRowRel() )
                {
                    rBuf.append( ':' );
                    r1c1_add_row(rBuf,  rRef.Ref2, aAbsRef.aEnd);
                }
                return;

            }

            if (aAbsRef.aStart.Row() == 0 && aAbsRef.aEnd.Row() >= MAXROW)
            {
                r1c1_add_col(rBuf, rRef.Ref1, aAbsRef.aStart);
                if (aAbsRef.aStart.Col() != aAbsRef.aEnd.Col() ||
                    rRef.Ref1.IsColRel() != rRef.Ref2.IsColRel())
                {
                    rBuf.append( ':' );
                    r1c1_add_col(rBuf, rRef.Ref2, aAbsRef.aEnd);
                }
                return;
            }
        }

        r1c1_add_row(rBuf, rRef.Ref1, aAbsRef.aStart);
        r1c1_add_col(rBuf, rRef.Ref1, aAbsRef.aStart);
        if (!bSingleRef)
        {
            rBuf.append( ':' );
            r1c1_add_row(rBuf, rRef.Ref2, aAbsRef.aEnd);
            r1c1_add_col(rBuf, rRef.Ref2, aAbsRef.aEnd);
        }
    }

    ParseResult parseAnyToken( const OUString& rFormula,
                               sal_Int32 nSrcPos,
                               const CharClass* pCharClass) const
    {
        ConventionXL::parseExternalDocName(rFormula, nSrcPos);

        ParseResult aRet;
        if ( lcl_isValidQuotedText(rFormula, nSrcPos, aRet) )
            return aRet;

        static const sal_Int32 nStartFlags = KParseTokens::ANY_LETTER_OR_NUMBER |
            KParseTokens::ASC_UNDERSCORE ;
        static const sal_Int32 nContFlags = nStartFlags | KParseTokens::ASC_DOT;
        
        const OUString aAddAllowed("?-[]!");

        return pCharClass->parseAnyToken( rFormula,
                nSrcPos, nStartFlags, aAddAllowed, nContFlags, aAddAllowed );
    }

    virtual sal_Unicode getSpecialSymbol( SpecialSymbolType eSymType ) const
    {
        return ConventionXL::getSpecialSymbol(eSymType);
    }

    virtual bool parseExternalName( const OUString& rSymbol, OUString& rFile, OUString& rName,
            const ScDocument* pDoc,
            const ::com::sun::star::uno::Sequence<
                const ::com::sun::star::sheet::ExternalLinkInfo > * pExternalLinks ) const
    {
        return ConventionXL::parseExternalName( rSymbol, rFile, rName, pDoc, pExternalLinks);
    }

    virtual OUString makeExternalNameStr( const OUString& rFile, const OUString& rName ) const
    {
        return ConventionXL::makeExternalNameStr(rFile, rName);
    }

    virtual void makeExternalRefStr(
        OUStringBuffer& rBuffer, const ScAddress& rPos, const OUString& rFileName,
        const OUString& rTabName, const ScSingleRefData& rRef ) const
    {
        
        
        
        
        

        ScAddress aAbsRef = rRef.toAbs(rPos);
        ConventionXL::makeExternalDocStr(rBuffer, rFileName, false);
        ScRangeStringConverter::AppendTableName(rBuffer, rTabName);
        rBuffer.append('!');

        r1c1_add_row(rBuffer, rRef, aAbsRef);
        r1c1_add_col(rBuffer, rRef, aAbsRef);
    }

    virtual void makeExternalRefStr(
        OUStringBuffer& rBuffer, const ScAddress& rPos, const OUString& rFileName,
        const std::vector<OUString>& rTabNames, const OUString& rTabName,
        const ScComplexRefData& rRef ) const
    {
        ScRange aAbsRef = rRef.toAbs(rPos);

        ConventionXL::makeExternalDocStr(rBuffer, rFileName, false);
        ConventionXL::makeExternalTabNameRange(rBuffer, rTabName, rTabNames, aAbsRef);
        rBuffer.append('!');

        if (!ValidCol(aAbsRef.aEnd.Col()) || !ValidRow(aAbsRef.aEnd.Row()))
        {
            rBuffer.append(ScGlobal::GetRscString(STR_NO_REF_TABLE));
            return;
        }

        if (aAbsRef.aStart.Col() == 0 && aAbsRef.aEnd.Col() >= MAXCOL)
        {
            r1c1_add_row(rBuffer, rRef.Ref1, aAbsRef.aStart);
            if (aAbsRef.aStart.Row() != aAbsRef.aEnd.Row() || rRef.Ref1.IsRowRel() != rRef.Ref2.IsRowRel())
            {
                rBuffer.append(':');
                r1c1_add_row(rBuffer, rRef.Ref2, aAbsRef.aEnd);
            }
            return;
        }

        if (aAbsRef.aStart.Row() == 0 && aAbsRef.aEnd.Row() >= MAXROW)
        {
            r1c1_add_col(rBuffer, rRef.Ref1, aAbsRef.aStart);
            if (aAbsRef.aStart.Col() != aAbsRef.aEnd.Col() || rRef.Ref1.IsColRel() != rRef.Ref2.IsColRel())
            {
                rBuffer.append(':');
                r1c1_add_col(rBuffer, rRef.Ref2, aAbsRef.aEnd);
            }
            return;
        }

        r1c1_add_row(rBuffer, rRef.Ref1, aAbsRef.aStart);
        r1c1_add_col(rBuffer, rRef.Ref1, aAbsRef.aStart);
        rBuffer.append(':');
        r1c1_add_row(rBuffer, rRef.Ref2, aAbsRef.aEnd);
        r1c1_add_col(rBuffer, rRef.Ref2, aAbsRef.aEnd);
    }

    virtual sal_uLong getCharTableFlags( sal_Unicode c, sal_Unicode cLast ) const
    {
        sal_uLong nFlags = mpCharTable[static_cast<sal_uInt8>(c)];
        if (c == '-' && cLast == '[')
            
            nFlags |= SC_COMPILER_C_IDENT;
        return nFlags;
    }
};

ScCompiler::ScCompiler( sc::CompileFormulaContext& rCxt, const ScAddress& rPos, ScTokenArray& rArr ) :
    FormulaCompiler(rArr),
    pDoc(rCxt.getDoc()),
    aPos(rPos),
    mpFormatter(pDoc->GetFormatTable()),
    pCharClass(ScGlobal::pCharClass),
    mnPredetectedReference(0),
    mnRangeOpPosInSymbol(-1),
    pConv(GetRefConvention(FormulaGrammar::CONV_OOO)),
    meExtendedErrorDetection(EXTENDED_ERROR_DETECTION_NONE),
    mbCloseBrackets(true),
    mbRewind(false),
    maTabNames(rCxt.getTabNames())
{
    nMaxTab = pDoc->GetTableCount() - 1;
    SetGrammar(rCxt.getGrammar());
}

ScCompiler::ScCompiler( ScDocument* pDocument, const ScAddress& rPos,ScTokenArray& rArr)
        : FormulaCompiler(rArr),
        pDoc( pDocument ),
        aPos( rPos ),
        mpFormatter(pDoc->GetFormatTable()),
        pCharClass( ScGlobal::pCharClass ),
        mnPredetectedReference(0),
        mnRangeOpPosInSymbol(-1),
        pConv( GetRefConvention( FormulaGrammar::CONV_OOO ) ),
        meExtendedErrorDetection( EXTENDED_ERROR_DETECTION_NONE ),
        mbCloseBrackets( true ),
        mbRewind( false )
{
    nMaxTab = pDoc->GetTableCount() - 1;
}

ScCompiler::ScCompiler( sc::CompileFormulaContext& rCxt, const ScAddress& rPos ) :
    pDoc(rCxt.getDoc()),
    aPos(rPos),
    mpFormatter(pDoc ? pDoc->GetFormatTable() : NULL),
    pCharClass(ScGlobal::pCharClass),
    mnPredetectedReference(0),
    mnRangeOpPosInSymbol(-1),
    pConv(GetRefConvention(FormulaGrammar::CONV_OOO)),
    meExtendedErrorDetection(EXTENDED_ERROR_DETECTION_NONE),
    mbCloseBrackets(true),
    mbRewind(false),
    maTabNames(rCxt.getTabNames())
{
    nMaxTab = pDoc ? pDoc->GetTableCount() - 1 : 0;
    SetGrammar(rCxt.getGrammar());
}

ScCompiler::ScCompiler( ScDocument* pDocument, const ScAddress& rPos)
        :
        pDoc( pDocument ),
        aPos( rPos ),
        mpFormatter(pDoc ? pDoc->GetFormatTable() : NULL),
        pCharClass( ScGlobal::pCharClass ),
        mnPredetectedReference(0),
        mnRangeOpPosInSymbol(-1),
        pConv( GetRefConvention( FormulaGrammar::CONV_OOO ) ),
        meExtendedErrorDetection( EXTENDED_ERROR_DETECTION_NONE ),
        mbCloseBrackets( true ),
        mbRewind( false )
{
    nMaxTab = pDoc ? pDoc->GetTableCount() - 1 : 0;
}

ScCompiler::~ScCompiler()
{
}

void ScCompiler::CheckTabQuotes( OUString& rString,
                                 const FormulaGrammar::AddressConvention eConv )
{
    using namespace ::com::sun::star::i18n;
    sal_Int32 nStartFlags = KParseTokens::ANY_LETTER_OR_NUMBER | KParseTokens::ASC_UNDERSCORE;
    sal_Int32 nContFlags = nStartFlags;
    ParseResult aRes = ScGlobal::pCharClass->parsePredefinedToken(
        KParseType::IDENTNAME, rString, 0, nStartFlags, EMPTY_OUSTRING, nContFlags, EMPTY_OUSTRING);
    bool bNeedsQuote = !((aRes.TokenType & KParseType::IDENTNAME) && aRes.EndPos == rString.getLength());

    switch ( eConv )
    {
        default :
        case FormulaGrammar::CONV_UNSPECIFIED :
            break;
        case FormulaGrammar::CONV_OOO :
        case FormulaGrammar::CONV_XL_A1 :
        case FormulaGrammar::CONV_XL_R1C1 :
        case FormulaGrammar::CONV_XL_OOX :
        case FormulaGrammar::CONV_ODF :
            if( bNeedsQuote )
            {
                const OUString one_quote('\'');
                const OUString two_quote("''");
                
                rString = rString.replaceAll( one_quote, two_quote );
            }
            break;
    }

    if ( !bNeedsQuote && CharClass::isAsciiNumeric( rString ) )
    {
        
        bNeedsQuote = true;
    }

    if( bNeedsQuote )
    {
        rString = "'" + rString + "'";
    }
}

sal_Int32 ScCompiler::GetDocTabPos( const OUString& rString )
{
    if (rString[0] != '\'')
        return -1;
    sal_Int32 nPos = ScGlobal::FindUnquoted( rString, SC_COMPILER_FILE_TAB_SEP);
    
    if (nPos != -1 && rString[nPos-1] != '\'')
        nPos = -1;
    return nPos;
}

void ScCompiler::SetRefConvention( FormulaGrammar::AddressConvention eConv )
{
    const Convention* p = GetRefConvention(eConv);
    if (p)
        SetRefConvention(p);
}

const ScCompiler::Convention* ScCompiler::GetRefConvention( FormulaGrammar::AddressConvention eConv )
{

    switch (eConv)
    {
        case FormulaGrammar::CONV_OOO:
        {
            static const ConventionOOO_A1 ConvOOO_A1;
            return &ConvOOO_A1;
        }
        case FormulaGrammar::CONV_ODF:
        {
            static const ConventionOOO_A1_ODF ConvOOO_A1_ODF;
            return &ConvOOO_A1_ODF;
        }
        case FormulaGrammar::CONV_XL_A1:
        {
            static const ConventionXL_A1 ConvXL_A1;
            return &ConvXL_A1;
        }
        case FormulaGrammar::CONV_XL_R1C1:
        {
            static const ConventionXL_R1C1 ConvXL_R1C1;
            return &ConvXL_R1C1;
        }
        case FormulaGrammar::CONV_XL_OOX:
        {
            static const ConventionXL_OOX ConvXL_OOX;
            return &ConvXL_OOX;
        }
        case FormulaGrammar::CONV_UNSPECIFIED:
        default:
            ;
    }

    return NULL;
}

void ScCompiler::SetRefConvention( const ScCompiler::Convention *pConvP )
{
    pConv = pConvP;
    meGrammar = FormulaGrammar::mergeToGrammar( meGrammar, pConv->meConv);
    OSL_ENSURE( FormulaGrammar::isSupported( meGrammar),
            "ScCompiler::SetRefConvention: unsupported grammar resulting");
}

void ScCompiler::SetError(sal_uInt16 nError)
{
    if( !pArr->GetCodeError() )
        pArr->SetCodeError( nError);
}

static sal_Unicode* lcl_UnicodeStrNCpy( sal_Unicode* pDst, const sal_Unicode* pSrc, sal_Int32 nMax )
{
    const sal_Unicode* const pStop = pDst + nMax;
    while ( *pSrc && pDst < pStop )
    {
        *pDst++ = *pSrc++;
    }
    *pDst = 0;
    return pDst;
}




































sal_Int32 ScCompiler::NextSymbol(bool bInArray)
{
    cSymbol[MAXSTRLEN-1] = 0;       
    sal_Unicode* pSym = cSymbol;
    const sal_Unicode* const pStart = aFormula.getStr();
    const sal_Unicode* pSrc = pStart + nSrcPos;
    bool bi18n = false;
    sal_Unicode c = *pSrc;
    sal_Unicode cLast = 0;
    bool bQuote = false;
    mnRangeOpPosInSymbol = -1;
    ScanState eState = ssGetChar;
    sal_Int32 nSpaces = 0;
    sal_Unicode cSep = mxSymbols->getSymbolChar( ocSep);
    sal_Unicode cArrayColSep = mxSymbols->getSymbolChar( ocArrayColSep);
    sal_Unicode cArrayRowSep = mxSymbols->getSymbolChar( ocArrayRowSep);
    sal_Unicode cDecSep = (mxSymbols->isEnglish() ? '.' :
            ScGlobal::pLocaleData->getNumDecimalSep()[0]);

    
    sal_Unicode cSheetPrefix = pConv->getSpecialSymbol(ScCompiler::Convention::ABS_SHEET_PREFIX);
    sal_Unicode cSheetSep    = pConv->getSpecialSymbol(ScCompiler::Convention::SHEET_SEPARATOR);

    int nDecSeps = 0;
    bool bAutoIntersection = false;
    int nRefInName = 0;
    bool bErrorConstantHadSlash = false;
    mnPredetectedReference = 0;
    
    while ((c != 0) && (eState != ssStop) )
    {
        pSrc++;
        sal_uLong nMask = GetCharTableFlags( c, cLast );

        
        
        if (c == cSep || (bInArray && (c == cArrayColSep || c == cArrayRowSep)))
        {
            switch (eState)
            {
                
                case ssGetString:
                case ssSkipString:
                case ssGetReference:
                case ssSkipReference:
                    break;
                default:
                    if (eState == ssGetChar)
                        *pSym++ = c;
                    else
                        pSrc--;
                    eState = ssStop;
            }
        }
Label_MaskStateMachine:
        switch (eState)
        {
            case ssGetChar :
            {
                
                if( nMask & SC_COMPILER_C_ODF_LABEL_OP )
                {
                    
                    if (GetCharTableFlags( pSrc[0], 0 ) & SC_COMPILER_C_ODF_LABEL_OP)
                    {
                        /* TODO: For now the UI "space operator" is used, this
                         * could be enhanced using a specialized OpCode to get
                         * rid of the space ambiguity, which would need some
                         * places to be adapted though. And we would still need
                         * to support the ambiguous space operator for UI
                         * purposes anyway. However, we then could check for
                         * invalid usage of '!!', which currently isn't
                         * possible. */
                        if (!bAutoIntersection)
                        {
                            ++pSrc;
                            nSpaces += 2;   
                            bAutoIntersection = true;
                        }
                        else
                        {
                            pSrc--;
                            eState = ssStop;
                        }
                    }
                    else
                    {
                        nMask &= ~SC_COMPILER_C_ODF_LABEL_OP;
                        goto Label_MaskStateMachine;
                    }
                }
                else if( nMask & SC_COMPILER_C_ODF_NAME_MARKER )
                {
                    
                    if (GetCharTableFlags( pSrc[0], 0 ) & SC_COMPILER_C_ODF_NAME_MARKER)
                    {
                        
                        ++pSrc;
                    }
                    else
                    {
                        nMask &= ~SC_COMPILER_C_ODF_NAME_MARKER;
                        goto Label_MaskStateMachine;
                    }
                }
                else if( nMask & SC_COMPILER_C_CHAR )
                {
                    *pSym++ = c;
                    eState = ssStop;
                }
                else if( nMask & SC_COMPILER_C_ODF_LBRACKET )
                {
                    
                    eState = ssGetReference;
                    mnPredetectedReference = 1;
                }
                else if( nMask & SC_COMPILER_C_CHAR_BOOL )
                {
                    *pSym++ = c;
                    eState = ssGetBool;
                }
                else if( nMask & SC_COMPILER_C_CHAR_VALUE )
                {
                    *pSym++ = c;
                    eState = ssGetValue;
                }
                else if( nMask & SC_COMPILER_C_CHAR_STRING )
                {
                    *pSym++ = c;
                    eState = ssGetString;
                }
                else if( nMask & SC_COMPILER_C_CHAR_ERRCONST )
                {
                    *pSym++ = c;
                    eState = ssGetErrorConstant;
                }
                else if( nMask & SC_COMPILER_C_CHAR_DONTCARE )
                {
                    nSpaces++;
                }
                else if( nMask & SC_COMPILER_C_CHAR_IDENT )
                {   
                    
                    *pSym++ = c;
                    eState = ssGetIdent;
                }
                else
                {
                    bi18n = true;
                    eState = ssStop;
                }
            }
            break;
            case ssGetIdent:
            {
                if ( nMask & SC_COMPILER_C_IDENT )
                {   
                    if( pSym == &cSymbol[ MAXSTRLEN-1 ] )
                    {
                        SetError(errStringOverflow);
                        eState = ssStop;
                    }
                    else
                        *pSym++ = c;
                }
                else if (c == ':' && mnRangeOpPosInSymbol < 0)
                {
                    
                    
                    mnRangeOpPosInSymbol = pSym - &cSymbol[0];
                    if( pSym == &cSymbol[ MAXSTRLEN-1 ] )
                    {
                        SetError(errStringOverflow);
                        eState = ssStop;
                    }
                    else
                        *pSym++ = c;
                }
                else if ( 128 <= c || '\'' == c )
                {   
                    
                    
                    bi18n = true;
                    eState = ssStop;
                }
                else
                {
                    pSrc--;
                    eState = ssStop;
                }
            }
            break;
            case ssGetBool :
            {
                if( nMask & SC_COMPILER_C_BOOL )
                {
                    *pSym++ = c;
                    eState = ssStop;
                }
                else
                {
                    pSrc--;
                    eState = ssStop;
                }
            }
            break;
            case ssGetValue :
            {
                if( pSym == &cSymbol[ MAXSTRLEN-1 ] )
                {
                    SetError(errStringOverflow);
                    eState = ssStop;
                }
                else if (c == cDecSep)
                {
                    if (++nDecSeps > 1)
                    {
                        
                        bi18n = true;
                        eState = ssStop;
                    }
                    else
                        *pSym++ = c;
                }
                else if( nMask & SC_COMPILER_C_VALUE )
                    *pSym++ = c;
                else if( nMask & SC_COMPILER_C_VALUE_SEP )
                {
                    pSrc--;
                    eState = ssStop;
                }
                else if (c == 'E' || c == 'e')
                {
                    if (GetCharTableFlags( pSrc[0], 0 ) & SC_COMPILER_C_VALUE_EXP)
                        *pSym++ = c;
                    else
                    {
                        
                        bi18n = true;
                        eState = ssStop;
                    }
                }
                else if( nMask & SC_COMPILER_C_VALUE_SIGN )
                {
                    if (((cLast == 'E') || (cLast == 'e')) &&
                            (GetCharTableFlags( pSrc[0], 0 ) & SC_COMPILER_C_VALUE_VALUE))
                    {
                        *pSym++ = c;
                    }
                    else
                    {
                        pSrc--;
                        eState = ssStop;
                    }
                }
                else
                {
                    
                    bi18n = true;
                    eState = ssStop;
                }
            }
            break;
            case ssGetString :
            {
                if( nMask & SC_COMPILER_C_STRING_SEP )
                {
                    if ( !bQuote )
                    {
                        if ( *pSrc == '"' )
                            bQuote = true;      
                        else
                            eState = ssStop;
                    }
                    else
                        bQuote = false;
                }
                if ( !bQuote )
                {
                    if( pSym == &cSymbol[ MAXSTRLEN-1 ] )
                    {
                        SetError(errStringOverflow);
                        eState = ssSkipString;
                    }
                    else
                        *pSym++ = c;
                }
            }
            break;
            case ssSkipString:
                if( nMask & SC_COMPILER_C_STRING_SEP )
                    eState = ssStop;
                break;
            case ssGetErrorConstant:
                {
                    
                    
                    
                    
                    /* TODO: be strict when reading ODFF, check for ASCII alnum
                     * and proper continuation after '/'. However, even with
                     * the lax parsing only the error constants we have defined
                     * as opcode symbols will be recognized and others result
                     * in ocBad, so the result is actually conformant. */
                    bool bAdd = true;
                    if ('!' == c || '?' == c)
                        eState = ssStop;
                    else if ('/' == c)
                    {
                        if (!bErrorConstantHadSlash)
                            bErrorConstantHadSlash = true;
                        else
                        {
                            bAdd = false;
                            eState = ssStop;
                        }
                    }
                    else if ((nMask & SC_COMPILER_C_WORD_SEP) ||
                            (c < 128 && !rtl::isAsciiAlphanumeric( c)))
                    {
                        bAdd = false;
                        eState = ssStop;
                    }
                    if (!bAdd)
                        --pSrc;
                    else
                    {
                        if (pSym == &cSymbol[ MAXSTRLEN-1 ])
                        {
                            SetError( errStringOverflow);
                            eState = ssStop;
                        }
                        else
                            *pSym++ = c;
                    }
                }
                break;
            case ssGetReference:
                if( pSym == &cSymbol[ MAXSTRLEN-1 ] )
                {
                    SetError( errStringOverflow);
                    eState = ssSkipReference;
                }
                
            case ssSkipReference:
                
                
                
                
                
                
                
                
                
                {

                    
                    
                    
                    
                    

                    
                    static const int kDollar    = (1 << 1);
                    
                    
                    static const int kOpen      = (1 << 2);
                    
                    static const int kName      = (1 << 3);
                    
                    
                    
                    static const int kQuote     = (1 << 4);
                    
                    static const int kClose     = (1 << 5);
                    
                    static const int kFileSep   = (1 << 6);
                    
                    static const int kPast      = (1 << 7);
                    
                    
                    
                    static const int kMarkAhead = (1 << 8);
                    
                    static const int kDefName   = (1 << 9);
                    
                    static const int kRefErr    = (1 << 10);

                    bool bAddToSymbol = true;
                    if ((nMask & SC_COMPILER_C_ODF_RBRACKET) && !(nRefInName & kOpen))
                    {
                        OSL_ENSURE( nRefInName & (kPast | kDefName | kRefErr),
                                "ScCompiler::NextSymbol: reference: "
                                "closing bracket ']' without prior sheet name separator '.' violates ODF spec");
                        
                        bAddToSymbol = false;
                        eState = ssStop;
                    }
                    else if (cSheetSep == c && nRefInName == 0)
                    {
                        
                        bAddToSymbol = false;
                        nRefInName |= kPast;
                        if ('$' == pSrc[0] && '$' == pSrc[1])
                            nRefInName |= kMarkAhead;
                    }
                    else if (!(nRefInName & kPast) || (nRefInName & (kMarkAhead | kDefName)))
                    {
                        

                        if (SC_COMPILER_FILE_TAB_SEP == c && (nRefInName & kFileSep))
                            nRefInName = 0;
                        else if ('$' == c && '$' == pSrc[0] && !(nRefInName & kOpen))
                        {
                            nRefInName &= ~kMarkAhead;
                            if (!(nRefInName & kDefName))
                            {
                                
                                bAddToSymbol = false;
                                ++pSrc;
                                nRefInName &= kPast;
                                nRefInName |= kDefName;
                            }
                            else
                            {
                                
                                
                                if (eState != ssSkipReference)
                                {
                                    *pSym++ = c;
                                    *pSym++ = *pSrc++;
                                }
                                bAddToSymbol = false;
                            }
                        }
                        else if (cSheetPrefix == c && nRefInName == 0)
                            nRefInName |= kDollar;
                        else if ('\'' == c)
                        {
                            
                            
                            
                            if (!(nRefInName & kName))
                            {
                                nRefInName |= (kOpen | kName);
                                bAddToSymbol = !(nRefInName & kDefName);
                            }
                            else if (!(nRefInName & kOpen))
                            {
                                OSL_FAIL("ScCompiler::NextSymbol: reference: "
                                        "a ''' without the name being enclosed in '...' violates ODF spec");
                            }
                            else if (nRefInName & kQuote)
                            {
                                
                                nRefInName &= ~kQuote;
                            }
                            else
                            {
                                switch (pSrc[0])
                                {
                                    case '\'':
                                        
                                        nRefInName |= kQuote;
                                        break;
                                    case SC_COMPILER_FILE_TAB_SEP:
                                        
                                        nRefInName |= kFileSep;
                                        
                                    default:
                                        
                                        nRefInName |= kClose;
                                        nRefInName &= ~kOpen;
                                }
                                bAddToSymbol = !(nRefInName & kDefName);
                            }
                        }
                        else if ('#' == c && nRefInName == 0)
                            nRefInName |= kRefErr;
                        else if (cSheetSep == c && !(nRefInName & kOpen))
                        {
                            
                            nRefInName |= kPast;
                            if ('$' == pSrc[0] && '$' == pSrc[1])
                                nRefInName |= kMarkAhead;
                        }
                        else if (':' == c && !(nRefInName & kOpen))
                        {
                            OSL_FAIL("ScCompiler::NextSymbol: reference: "
                                    "range operator ':' without prior sheet name separator '.' violates ODF spec");
                            nRefInName = 0;
                            ++mnPredetectedReference;
                        }
                        else if (!(nRefInName & kName))
                        {
                            
                            nRefInName |= kName;
                        }
                    }
                    else if (':' == c)
                    {
                        
                        nRefInName = 0;
                        ++mnPredetectedReference;
                    }
                    if (bAddToSymbol && eState != ssSkipReference)
                        *pSym++ = c;    
                }
                break;
            case ssStop:
                ;   
                break;
        }
        cLast = c;
        c = *pSrc;
    }
    if ( bi18n )
    {
        nSrcPos = nSrcPos + nSpaces;
        OUStringBuffer aSymbol;
        mnRangeOpPosInSymbol = -1;
        sal_uInt16 nErr = 0;
        do
        {
            bi18n = false;
            
            if ( pStart[nSrcPos] == cSheetPrefix && pStart[nSrcPos+1] == '\'' )
                aSymbol.append(pStart[nSrcPos++]);

            ParseResult aRes = pConv->parseAnyToken( aFormula, nSrcPos, pCharClass );

            if ( !aRes.TokenType )
                SetError( nErr = errIllegalChar );      
            if ( aRes.EndPos <= nSrcPos )
            {   
                SetError( nErr = errIllegalChar );
                nSrcPos = aFormula.getLength();
                aSymbol.truncate(0);
            }
            else
            {
                aSymbol.append( pStart + nSrcPos, aRes.EndPos - nSrcPos);
                nSrcPos = aRes.EndPos;
                c = pStart[nSrcPos];
                if ( aRes.TokenType & KParseType::SINGLE_QUOTE_NAME )
                {   
                    bi18n = (c == cSheetSep || c == SC_COMPILER_FILE_TAB_SEP);
                }
                
                if (c == ':' && mnRangeOpPosInSymbol < 0)
                {
                    mnRangeOpPosInSymbol = aSymbol.getLength();
                    bi18n = true;
                }
                if ( bi18n )
                    aSymbol.append(pStart[nSrcPos++]);
            }
        } while ( bi18n && !nErr );
        sal_Int32 nLen = aSymbol.getLength();
        if ( nLen >= MAXSTRLEN )
        {
            SetError( errStringOverflow );
            nLen = MAXSTRLEN-1;
        }
        lcl_UnicodeStrNCpy( cSymbol, aSymbol.getStr(), nLen );
        pSym = &cSymbol[nLen];
    }
    else
    {
        nSrcPos = pSrc - pStart;
        *pSym = 0;
    }
    if (mnRangeOpPosInSymbol >= 0 && mnRangeOpPosInSymbol == (pSym-1) - &cSymbol[0])
    {
        
        
        mnRangeOpPosInSymbol = -1;
        *--pSym = 0;
        --nSrcPos;
    }
    if ( bAutoCorrect )
        aCorrectedSymbol = cSymbol;
    if (bAutoIntersection && nSpaces > 1)
        --nSpaces;  
    return nSpaces;
}



bool ScCompiler::IsOpCode( const OUString& rName, bool bInArray )
{
    OpCodeHashMap::const_iterator iLook( mxSymbols->getHashMap()->find( rName));
    bool bFound = (iLook != mxSymbols->getHashMap()->end());
    if (bFound)
    {
        ScRawToken aToken;
        OpCode eOp = iLook->second;
        if (bInArray)
        {
            if (rName.equals(mxSymbols->getSymbol(ocArrayColSep)))
                eOp = ocArrayColSep;
            else if (rName.equals(mxSymbols->getSymbol(ocArrayRowSep)))
                eOp = ocArrayRowSep;
        }
        aToken.SetOpCode(eOp);
        pRawToken = aToken.Clone();
    }
    else if (mxSymbols->isODFF())
    {
        
        
        
        
        
        struct FunctionName
        {
            const sal_Char* pName;
            OpCode          eOp;
        };
        static const FunctionName aOdffAliases[] = {
            
            { "B",              ocB },              
            { "TDIST",          ocTDist },          
            { "EASTERSUNDAY",   ocEasterSunday },   
            { "ZGZ",            ocZGZ },            
            
            { "ORG.OPENOFFICE.GOALSEEK", ocBackSolver } 
        };
        static const size_t nOdffAliases = sizeof(aOdffAliases) / sizeof(aOdffAliases[0]);
        for (size_t i=0; i<nOdffAliases; ++i)
        {
            if (rName.equalsIgnoreAsciiCaseAscii( aOdffAliases[i].pName))
            {
                ScRawToken aToken;
                aToken.SetOpCode( aOdffAliases[i].eOp);
                pRawToken = aToken.Clone();
                bFound = true;
                break;  
            }
        }
    }
    if (!bFound)
    {
        OUString aIntName;
        if (mxSymbols->hasExternals())
        {
            
            ExternalHashMap::const_iterator iExt(
                    mxSymbols->getExternalHashMap()->find( rName));
            if (iExt != mxSymbols->getExternalHashMap()->end())
            {
                if (ScGlobal::GetAddInCollection()->GetFuncData( (*iExt).second))
                    aIntName = (*iExt).second;
            }
            if (aIntName.isEmpty())
            {
                
                
                
                
                return false;
            }
        }
        if (aIntName.isEmpty())
        {
            
            if (ScGlobal::GetFuncCollection()->findByName(cSymbol))
            {
                ScRawToken aToken;
                aToken.SetExternal( cSymbol );
                pRawToken = aToken.Clone();
            }
            else
                
                
                aIntName = ScGlobal::GetAddInCollection()->FindFunction(
                        rName, !mxSymbols->isEnglish());
        }
        if (!aIntName.isEmpty())
        {
            ScRawToken aToken;
            aToken.SetExternal( aIntName.getStr() );     
            pRawToken = aToken.Clone();
            bFound = true;
        }
    }
    OpCode eOp;
    if (bFound && ((eOp = pRawToken->GetOpCode()) == ocSub || eOp == ocNegSub))
    {
        bool bShouldBeNegSub =
            (eLastOp == ocOpen || eLastOp == ocSep || eLastOp == ocNegSub ||
             (SC_OPCODE_START_BIN_OP <= eLastOp && eLastOp < SC_OPCODE_STOP_BIN_OP) ||
             eLastOp == ocArrayOpen ||
             eLastOp == ocArrayColSep || eLastOp == ocArrayRowSep);
        if (bShouldBeNegSub && eOp == ocSub)
            pRawToken->NewOpCode( ocNegSub );
            
        else if (!bShouldBeNegSub && eOp == ocNegSub)
            pRawToken->NewOpCode( ocSub );
    }
    return bFound;
}

bool ScCompiler::IsOpCode2( const OUString& rName )
{
    bool bFound = false;
    sal_uInt16 i;

    for( i = ocInternalBegin; i <= ocInternalEnd && !bFound; i++ )
        bFound = rName.equalsAscii( pInternal[ i-ocInternalBegin ] );

    if (bFound)
    {
        ScRawToken aToken;
        aToken.SetOpCode( (OpCode) --i );
        pRawToken = aToken.Clone();
    }
    return bFound;
}

bool ScCompiler::IsValue( const OUString& rSym )
{
    double fVal;
    sal_uInt32 nIndex = mxSymbols->isEnglish() ? mpFormatter->GetStandardIndex(LANGUAGE_ENGLISH_US) : 0;

    if (!mpFormatter->IsNumberFormat(rSym, nIndex, fVal))
        return false;

    sal_uInt16 nType = mpFormatter->GetType(nIndex);

    
    
    
    
    
    if (nType & (NUMBERFORMAT_TIME | NUMBERFORMAT_DATE))
        return false;

    if (nType == NUMBERFORMAT_LOGICAL)
    {
        const sal_Unicode* p = aFormula.getStr() + nSrcPos;
        while( *p == ' ' )
            p++;
        if (*p == '(')
            return false;   
    }

    if( nType == NUMBERFORMAT_TEXT )
        
        SetError( errIllegalArgument );
    ScRawToken aToken;
    aToken.SetDouble( fVal );
    pRawToken = aToken.Clone();
    return true;
}

bool ScCompiler::IsString()
{
    const sal_Unicode* p = cSymbol;
    while ( *p )
        p++;
    sal_Int32 nLen = sal::static_int_cast<sal_Int32>( p - cSymbol - 1 );
    bool bQuote = ((cSymbol[0] == '"') && (cSymbol[nLen] == '"'));
    if ((bQuote ? nLen-2 : nLen) > MAXSTRLEN-1)
    {
        SetError(errStringOverflow);
        return false;
    }
    if ( bQuote )
    {
        cSymbol[nLen] = '\0';
        ScRawToken aToken;
        const sal_Unicode* pStr = cSymbol+1;
        svl::SharedString aSS = pDoc->GetSharedStringPool().intern(OUString(pStr));
        aToken.SetString(aSS.getData(), aSS.getDataIgnoreCase());
        pRawToken = aToken.Clone();
        return true;
    }
    return false;
}

bool ScCompiler::IsPredetectedReference(const OUString& rName)
{
    
    sal_Int32 nPos = rName.indexOf("#REF!");
    if (nPos != -1)
    {
        /* TODO: this may be enhanced by reusing scan information from
         * NextSymbol(), the positions of quotes and special characters found
         * there for $'sheet'.A1:... could be stored in a vector. We don't
         * fully rescan here whether found positions are within single quotes
         * for performance reasons. This code does not check for possible
         * occurrences of insane "valid" sheet names like
         * 'haha.#REF!1fooledyou' and will generate an error on such. */
        if (nPos == 0)
        {
            
            
            if (rName.getLength() == 5)
                return IsErrorConstant( rName);
            return false;           
        }
        sal_Unicode c = rName[nPos-1];      
        if ('$' == c)
        {
            if (nPos == 1)
                return false;       
            c = rName[nPos-2];              
        }
        sal_Unicode c2 = nPos+5 < rName.getLength() ? rName[nPos+5] : 0;     
        switch (c)
        {
            case '.':
                if ('$' == c2 || '#' == c2 || ('0' <= c2 && c2 <= '9'))
                    return false;   
                break;
            case ':':
                if (mnPredetectedReference > 1 &&
                        ('.' == c2 || '$' == c2 || '#' == c2 ||
                         ('0' <= c2 && c2 <= '9')))
                    return false;   
                break;
            default:
                if (comphelper::string::isalphaAscii(c) &&
                        ((mnPredetectedReference > 1 && ':' == c2) || 0 == c2))
                    return false;   
        }
    }
    switch (mnPredetectedReference)
    {
        case 1:
            return IsSingleReference( rName);
        case 2:
            return IsDoubleReference( rName);
    }
    return false;
}

bool ScCompiler::IsDoubleReference( const OUString& rName )
{
    ScRange aRange( aPos, aPos );
    const ScAddress::Details aDetails( pConv->meConv, aPos );
    ScAddress::ExternalInfo aExtInfo;
    sal_uInt16 nFlags = aRange.Parse( rName, pDoc, aDetails, &aExtInfo, &maExternalLinks );
    if( nFlags & SCA_VALID )
    {
        ScRawToken aToken;
        ScComplexRefData aRef;
        aRef.InitRange( aRange );
        aRef.Ref1.SetColRel( (nFlags & SCA_COL_ABSOLUTE) == 0 );
        aRef.Ref1.SetRowRel( (nFlags & SCA_ROW_ABSOLUTE) == 0 );
        aRef.Ref1.SetTabRel( (nFlags & SCA_TAB_ABSOLUTE) == 0 );
        if ( !(nFlags & SCA_VALID_TAB) )
            aRef.Ref1.SetTabDeleted( true );        
        aRef.Ref1.SetFlag3D( ( nFlags & SCA_TAB_3D ) != 0 );
        aRef.Ref2.SetColRel( (nFlags & SCA_COL2_ABSOLUTE) == 0 );
        aRef.Ref2.SetRowRel( (nFlags & SCA_ROW2_ABSOLUTE) == 0 );
        aRef.Ref2.SetTabRel( (nFlags & SCA_TAB2_ABSOLUTE) == 0 );
        if ( !(nFlags & SCA_VALID_TAB2) )
            aRef.Ref2.SetTabDeleted( true );        
        aRef.Ref2.SetFlag3D( ( nFlags & SCA_TAB2_3D ) != 0 );
        aRef.SetRange(aRange, aPos);
        if (aExtInfo.mbExternal)
        {
            ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
            const OUString* pRealTab = pRefMgr->getRealTableName(aExtInfo.mnFileId, aExtInfo.maTabName);
            aToken.SetExternalDoubleRef(
                aExtInfo.mnFileId, pRealTab ? *pRealTab : aExtInfo.maTabName, aRef);
            maExternalFiles.push_back(aExtInfo.mnFileId);
        }
        else
        {
            aToken.SetDoubleReference(aRef);
        }
        pRawToken = aToken.Clone();
    }

    return ( nFlags & SCA_VALID ) != 0;
}

bool ScCompiler::IsSingleReference( const OUString& rName )
{
    ScAddress aAddr( aPos );
    const ScAddress::Details aDetails( pConv->meConv, aPos );
    ScAddress::ExternalInfo aExtInfo;
    sal_uInt16 nFlags = aAddr.Parse( rName, pDoc, aDetails, &aExtInfo, &maExternalLinks );
    
    
    if( nFlags & ( SCA_VALID_COL|SCA_VALID_ROW|SCA_VALID_TAB ) )
    {
        ScRawToken aToken;
        ScSingleRefData aRef;
        aRef.InitAddress( aAddr );
        aRef.SetColRel( (nFlags & SCA_COL_ABSOLUTE) == 0 );
        aRef.SetRowRel( (nFlags & SCA_ROW_ABSOLUTE) == 0 );
        aRef.SetTabRel( (nFlags & SCA_TAB_ABSOLUTE) == 0 );
        aRef.SetFlag3D( ( nFlags & SCA_TAB_3D ) != 0 );
        
        if( !( nFlags & SCA_VALID ) )
        {
            if( !( nFlags & SCA_VALID_COL ) )
                aRef.SetColDeleted(true);
            if( !( nFlags & SCA_VALID_ROW ) )
                aRef.SetRowDeleted(true);
            if( !( nFlags & SCA_VALID_TAB ) )
                aRef.SetTabDeleted(true);
            nFlags |= SCA_VALID;
        }
        aRef.SetAddress(aAddr, aPos);

        if (aExtInfo.mbExternal)
        {
            ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
            const OUString* pRealTab = pRefMgr->getRealTableName(aExtInfo.mnFileId, aExtInfo.maTabName);
            aToken.SetExternalSingleRef(
                aExtInfo.mnFileId, pRealTab ? *pRealTab : aExtInfo.maTabName, aRef);
            maExternalFiles.push_back(aExtInfo.mnFileId);
        }
        else
            aToken.SetSingleReference(aRef);
        pRawToken = aToken.Clone();
    }

    return ( nFlags & SCA_VALID ) != 0;
}

bool ScCompiler::IsReference( const OUString& rName )
{
    
    sal_Unicode ch1 = rName[0];
    sal_Unicode cDecSep = ( mxSymbols->isEnglish() ? '.' :
        ScGlobal::pLocaleData->getNumDecimalSep()[0] );
    if ( ch1 == cDecSep )
        return false;
    
    if ( rtl::isAsciiDigit( ch1 ) )
    {
        
        
        
        
        do
        {
            const sal_Int32 nPos = ScGlobal::FindUnquoted( rName, '.');
            if ( nPos == -1 )
            {
                if (ScGlobal::FindUnquoted( rName, ':') != -1)
                    break;      
                return false;
            }
            sal_Unicode const * const pTabSep = rName.getStr() + nPos;
            sal_Unicode ch2 = pTabSep[1];   
            if ( !(ch2 == '$' || rtl::isAsciiAlpha( ch2 )) )
                return false;
            if ( cDecSep == '.' && (ch2 == 'E' || ch2 == 'e')   
                    && (GetCharTableFlags( pTabSep[2], pTabSep[1] ) & SC_COMPILER_C_VALUE_EXP) )
            {   
                
                
                
                
                
                
                
                
                
                
                OUString aTabName( rName.copy( 0, nPos ) );
                SCTAB nTab;
                if ( !pDoc->GetTable( aTabName, nTab ) )
                    return false;
                
                
            }
        } while(false);
    }

    if (IsSingleReference( rName))
        return true;

    
    
    
    if (mnRangeOpPosInSymbol > 0)   
    {
        if (IsDoubleReference( rName))
            return true;
        
        
        sal_Int32 nLen = mnRangeOpPosInSymbol;
        while (cSymbol[++nLen])
            ;
        cSymbol[mnRangeOpPosInSymbol] = 0;
        nSrcPos -= (nLen - mnRangeOpPosInSymbol);
        mnRangeOpPosInSymbol = -1;
        mbRewind = true;
        return true;    
    }
    else
    {
        
        
        
        switch (pConv->meConv)
        {
            case FormulaGrammar::CONV_XL_A1:
            case FormulaGrammar::CONV_XL_R1C1:
            case FormulaGrammar::CONV_XL_OOX:
                if (rName[0] == '\'' && IsDoubleReference( rName))
                    return true;
                break;
            default:
                ;   
        }
    }
    return false;
}

bool ScCompiler::IsMacro( const OUString& rName )
{
#ifdef DISABLE_SCRIPTING
    (void) rName;

    return false;
#else

    
    
    
    
    
    /* FIXME: yes, but how ... */
    comphelper::SolarMutex& rSolarMutex = Application::GetSolarMutex();
    if (!rSolarMutex.tryToAcquire())
    {
        SAL_WARN( "sc.core", "ScCompiler::IsMacro - SolarMutex would deadlock, not obtaining Basic");
        return false;   
    }

    OUString aName( rName);
    StarBASIC* pObj = 0;
    SfxObjectShell* pDocSh = pDoc->GetDocumentShell();

    SfxApplication* pSfxApp = SFX_APP();

    if( pDocSh )
        pObj = pDocSh->GetBasic();
    else
        pObj = pSfxApp->GetBasic();

    
    
    
    
    if (FormulaGrammar::isODFF( GetGrammar()) && aName.startsWithIgnoreAsciiCase("USER."))
        aName = aName.copy(5);

    SbxMethod* pMeth = (SbxMethod*) pObj->Find( aName, SbxCLASS_METHOD );
    if( !pMeth )
    {
        rSolarMutex.release();
        return false;
    }
    
    if( pMeth->GetType() == SbxVOID
     || ( pMeth->IsFixed() && pMeth->GetType() == SbxEMPTY )
     || !pMeth->ISA(SbMethod) )
    {
        rSolarMutex.release();
        return false;
    }
    ScRawToken aToken;
    aToken.SetExternal( aName.getStr() );
    aToken.eOp = ocMacro;
    pRawToken = aToken.Clone();
    rSolarMutex.release();
    return true;
#endif
}

bool ScCompiler::IsNamedRange( const OUString& rUpperName )
{
    

    
    bool bGlobal = false;
    ScRangeName* pRangeName = pDoc->GetRangeName(aPos.Tab());
    const ScRangeData* pData = NULL;
    if (pRangeName)
        pData = pRangeName->findByUpperName(rUpperName);
    if (!pData)
    {
        pRangeName = pDoc->GetRangeName();
        if (pRangeName)
            pData = pRangeName->findByUpperName(rUpperName);
        if (pData)
            bGlobal = true;
    }

    if (pData)
    {
        ScRawToken aToken;
        aToken.SetName(bGlobal, pData->GetIndex());
        pRawToken = aToken.Clone();
        return true;
    }
    else
        return false;
}

bool ScCompiler::IsExternalNamedRange( const OUString& rSymbol )
{
    /* FIXME: This code currently (2008-12-02T15:41+0100 in CWS mooxlsc)
     * correctly parses external named references in OOo, as required per RFE
     * #i3740#, just that we can't store them in ODF yet. We will need an OASIS
     * spec first. Until then don't pretend to support external names that
     * wouldn't survive a save and reload cycle, return false instead. */

    if (!pConv)
        return false;

    OUString aFile, aName;
    if (!pConv->parseExternalName( rSymbol, aFile, aName, pDoc, &maExternalLinks))
        return false;

    ScRawToken aToken;
    if (aFile.getLength() > MAXSTRLEN || aName.getLength() > MAXSTRLEN)
        return false;

    ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
    OUString aTmp = aFile;
    pRefMgr->convertToAbsName(aTmp);
    aFile = aTmp;
    sal_uInt16 nFileId = pRefMgr->getExternalFileId(aFile);
    if (!pRefMgr->isValidRangeName(nFileId, aName))
        
        return false;

    const OUString* pRealName = pRefMgr->getRealRangeName(nFileId, aName);
    aToken.SetExternalName(nFileId, pRealName ? *pRealName : OUString(aTmp));
    pRawToken = aToken.Clone();
    maExternalFiles.push_back(nFileId);
    return true;
}

bool ScCompiler::IsDBRange( const OUString& rName )
{
    if (rName.equalsAscii("[]"))
    {
        if (pRawToken && pRawToken->GetOpCode() == ocDBArea)
        {
            
            
            ScRawToken aToken;
            aToken.eOp = ocSkip;
            pRawToken = aToken.Clone();
            return true;
        }
    }
    ScDBCollection::NamedDBs& rDBs = pDoc->GetDBCollection()->getNamedDBs();
    const ScDBData* p = rDBs.findByUpperName(rName);
    if (!p)
        return false;

    ScRawToken aToken;
    aToken.SetName(true, p->GetIndex()); 
    aToken.eOp = ocDBArea;
    pRawToken = aToken.Clone();
    return true;
}

bool ScCompiler::IsColRowName( const OUString& rName )
{
    bool bInList = false;
    bool bFound = false;
    ScSingleRefData aRef;
    OUString aName( rName );
    DeQuote( aName );
    SCTAB nThisTab = aPos.Tab();
    for ( short jThisTab = 1; jThisTab >= 0 && !bInList; jThisTab-- )
    {   
        for ( short jRow=0; jRow<2 && !bInList; jRow++ )
        {
            ScRangePairList* pRL;
            if ( !jRow )
                pRL = pDoc->GetColNameRanges();
            else
                pRL = pDoc->GetRowNameRanges();
            for ( size_t iPair = 0, nPairs = pRL->size(); iPair < nPairs && !bInList; ++iPair )
            {
                ScRangePair* pR = (*pRL)[iPair];
                const ScRange& rNameRange = pR->GetRange(0);
                if ( jThisTab && !(rNameRange.aStart.Tab() <= nThisTab &&
                        nThisTab <= rNameRange.aEnd.Tab()) )
                    continue;   
                ScCellIterator aIter( pDoc, rNameRange );
                for (bool bHas = aIter.first(); bHas && !bInList; bHas = aIter.next())
                {
                    
                    
                    
                    
                    
                    CellType eType = aIter.getType();
                    bool bOk = false;
                    if (eType == CELLTYPE_FORMULA)
                    {
                        ScFormulaCell* pFC = aIter.getFormulaCell();
                        bOk = (pFC->GetCode()->GetCodeLen() > 0) && (pFC->aPos != aPos);
                    }
                    else
                        bOk = true;

                    if (bOk && aIter.hasString())
                    {
                        OUString aStr = aIter.getString();
                        if ( ScGlobal::GetpTransliteration()->isEqual( aStr, aName ) )
                        {
                            aRef.InitFlags();
                            if ( !jRow )
                                aRef.SetColRel( true );     
                            else
                                aRef.SetRowRel( true );     
                            aRef.SetAddress(aIter.GetPos(), aPos);
                            bInList = bFound = true;
                        }
                    }
                }
            }
        }
    }
    if ( !bInList && pDoc->GetDocOptions().IsLookUpColRowNames() )
    {   
        long nDistance = 0, nMax = 0;
        long nMyCol = (long) aPos.Col();
        long nMyRow = (long) aPos.Row();
        bool bTwo = false;
        ScAddress aOne( 0, 0, aPos.Tab() );
        ScAddress aTwo( MAXCOL, MAXROW, aPos.Tab() );

        ScAutoNameCache* pNameCache = pDoc->GetAutoNameCache();
        if ( pNameCache )
        {
            
            

            const ScAutoNameAddresses& rAddresses = pNameCache->GetNameOccurrences( aName, aPos.Tab() );

            
            

            ScAutoNameAddresses::const_iterator aEnd(rAddresses.end());
            for ( ScAutoNameAddresses::const_iterator aAdrIter(rAddresses.begin()); aAdrIter != aEnd; ++aAdrIter )
            {
                ScAddress aAddress( *aAdrIter );        

                if ( bFound )
                {   
                    if ( nMax < (long)aAddress.Col() )
                        break;      
                }
                if ( aAddress != aPos )
                {
                    

                    SCCOL nCol = aAddress.Col();
                    SCROW nRow = aAddress.Row();
                    long nC = nMyCol - nCol;
                    long nR = nMyRow - nRow;
                    if ( bFound )
                    {
                        long nD = nC * nC + nR * nR;
                        if ( nD < nDistance )
                        {
                            if ( nC < 0 || nR < 0 )
                            {   
                                bTwo = true;
                                aTwo.Set( nCol, nRow, aAddress.Tab() );
                                nMax = std::max( nMyCol + std::abs( nC ), nMyRow + std::abs( nR ) );
                                nDistance = nD;
                            }
                            else if ( !(nRow < aOne.Row() && nMyRow >= (long)aOne.Row()) )
                            {
                                
                                
                                
                                bTwo = false;
                                aOne.Set( nCol, nRow, aAddress.Tab() );
                                nMax = std::max( nMyCol + nC, nMyRow + nR );
                                nDistance = nD;
                            }
                        }
                    }
                    else
                    {
                        aOne.Set( nCol, nRow, aAddress.Tab() );
                        nDistance = nC * nC + nR * nR;
                        nMax = std::max( nMyCol + std::abs( nC ), nMyRow + std::abs( nR ) );

                    }
                    bFound = true;
                }
            }
        }
        else
        {
            ScCellIterator aIter( pDoc, ScRange( aOne, aTwo ) );
            for (bool bHas = aIter.first(); bHas; bHas = aIter.next())
            {
                if ( bFound )
                {   
                    if ( nMax < (long)aIter.GetPos().Col() )
                        break;      
                }
                CellType eType = aIter.getType();
                bool bOk = false;
                if (eType == CELLTYPE_FORMULA)
                {
                    ScFormulaCell* pFC = aIter.getFormulaCell();
                    bOk = (pFC->GetCode()->GetCodeLen() > 0) && (pFC->aPos != aPos);
                }
                else
                    bOk = true;

                if (bOk && aIter.hasString())
                {
                    OUString aStr = aIter.getString();
                    if ( ScGlobal::GetpTransliteration()->isEqual( aStr, aName ) )
                    {
                        SCCOL nCol = aIter.GetPos().Col();
                        SCROW nRow = aIter.GetPos().Row();
                        long nC = nMyCol - nCol;
                        long nR = nMyRow - nRow;
                        if ( bFound )
                        {
                            long nD = nC * nC + nR * nR;
                            if ( nD < nDistance )
                            {
                                if ( nC < 0 || nR < 0 )
                                {   
                                    bTwo = true;
                                    aTwo.Set( nCol, nRow, aIter.GetPos().Tab() );
                                    nMax = std::max( nMyCol + std::abs( nC ), nMyRow + std::abs( nR ) );
                                    nDistance = nD;
                                }
                                else if ( !(nRow < aOne.Row() && nMyRow >= (long)aOne.Row()) )
                                {
                                    
                                    
                                    
                                    bTwo = false;
                                    aOne.Set( nCol, nRow, aIter.GetPos().Tab() );
                                    nMax = std::max( nMyCol + nC, nMyRow + nR );
                                    nDistance = nD;
                                }
                            }
                        }
                        else
                        {
                            aOne.Set( nCol, nRow, aIter.GetPos().Tab() );
                            nDistance = nC * nC + nR * nR;
                            nMax = std::max( nMyCol + std::abs( nC ), nMyRow + std::abs( nR ) );
                        }
                        bFound = true;
                    }
                }
            }
        }

        if ( bFound )
        {
            ScAddress aAdr;
            if ( bTwo )
            {
                if ( nMyCol >= (long)aOne.Col() && nMyRow >= (long)aOne.Row() )
                    aAdr = aOne;        
                else
                {
                    if ( nMyCol < (long)aOne.Col() )
                    {   
                        if ( nMyRow >= (long)aTwo.Row() )
                            aAdr = aTwo;        
                        else
                            aAdr = aOne;
                    }
                    else
                    {   
                        long nC1 = nMyCol - aOne.Col();
                        long nR1 = nMyRow - aOne.Row();
                        long nC2 = nMyCol - aTwo.Col();
                        long nR2 = nMyRow - aTwo.Row();
                        if ( nC1 * nC1 + nR1 * nR1 <= nC2 * nC2 + nR2 * nR2 )
                            aAdr = aOne;
                        else
                            aAdr = aTwo;
                    }
                }
            }
            else
                aAdr = aOne;
            aRef.InitAddress( aAdr );
            if ( (aAdr.Row() != MAXROW && pDoc->HasStringData(
                    aAdr.Col(), aAdr.Row() + 1, aAdr.Tab()))
              || (aAdr.Row() != 0 && pDoc->HasStringData(
                    aAdr.Col(), aAdr.Row() - 1, aAdr.Tab())))
                aRef.SetRowRel( true );     
            else
                aRef.SetColRel( true );     
            aRef.SetAddress(aAdr, aPos);
        }
    }
    if ( bFound )
    {
        ScRawToken aToken;
        aToken.SetSingleReference( aRef );
        aToken.eOp = ocColRowName;
        pRawToken = aToken.Clone();
        return true;
    }
    else
        return false;
}

bool ScCompiler::IsBoolean( const OUString& rName )
{
    OpCodeHashMap::const_iterator iLook( mxSymbols->getHashMap()->find( rName ) );
    if( iLook != mxSymbols->getHashMap()->end() &&
        ((*iLook).second == ocTrue ||
         (*iLook).second == ocFalse) )
    {
        ScRawToken aToken;
        aToken.SetOpCode( (*iLook).second );
        pRawToken = aToken.Clone();
        return true;
    }
    else
        return false;
}

bool ScCompiler::IsErrorConstant( const OUString& rName ) const
{
    sal_uInt16 nError = GetErrorConstant( rName);
    if (nError)
    {
        ScRawToken aToken;
        aToken.SetErrorConstant( nError);
        pRawToken = aToken.Clone();
        return true;
    }
    else
        return false;
}

void ScCompiler::AutoCorrectParsedSymbol()
{
    sal_Int32 nPos = aCorrectedSymbol.getLength();
    if ( nPos )
    {
        nPos--;
        const sal_Unicode cQuote = '\"';
        const sal_Unicode cx = 'x';
        const sal_Unicode cX = 'X';
        sal_Unicode c1 = aCorrectedSymbol[0];
        sal_Unicode c2 = aCorrectedSymbol[nPos];
        sal_Unicode c2p = nPos > 0 ? aCorrectedSymbol[nPos-1] : 0;
        if ( c1 == cQuote && c2 != cQuote  )
        {   
            
            
            while ( nPos && ((aCorrectedSymbol[nPos] < 128) &&
                    ((GetCharTableFlags(aCorrectedSymbol[nPos], aCorrectedSymbol[nPos-1]) &
                    (SC_COMPILER_C_WORD | SC_COMPILER_C_CHAR_DONTCARE)) == 0)) )
                nPos--;
            if ( nPos == MAXSTRLEN - 2 )
                aCorrectedSymbol = aCorrectedSymbol.replaceAt( nPos, 1, OUString(cQuote) );   
            else
                aCorrectedSymbol = aCorrectedSymbol.replaceAt( nPos + 1, 0, OUString(cQuote) );
            bCorrected = true;
        }
        else if ( c1 != cQuote && c2 == cQuote )
        {   
            aCorrectedSymbol = OUString(cQuote) + aCorrectedSymbol;
            bCorrected = true;
        }
        else if ( nPos == 0 && (c1 == cx || c1 == cX) )
        {   
            aCorrectedSymbol = mxSymbols->getSymbol(ocMul);
            bCorrected = true;
        }
        else if ( (GetCharTableFlags( c1, 0 ) & SC_COMPILER_C_CHAR_VALUE)
               && (GetCharTableFlags( c2, c2p ) & SC_COMPILER_C_CHAR_VALUE) )
        {
            if ( comphelper::string::getTokenCount(aCorrectedSymbol, cx) > 1 )
            {   
                sal_Unicode c = mxSymbols->getSymbolChar(ocMul);
                aCorrectedSymbol = aCorrectedSymbol.replaceAll(OUString(cx), OUString(c));
                bCorrected = true;
            }
            if ( comphelper::string::getTokenCount(aCorrectedSymbol, cX) > 1 )
            {   
                sal_Unicode c = mxSymbols->getSymbolChar(ocMul);
                aCorrectedSymbol = aCorrectedSymbol.replaceAll(OUString(cX), OUString(c));
                bCorrected = true;
            }
        }
        else
        {
            OUString aSymbol( aCorrectedSymbol );
            OUString aDoc;
            sal_Int32 nPosition;
            if ( aSymbol[0] == '\''
              && ((nPosition = aSymbol.indexOf( "'#" )) != -1) )
            {   
                aDoc = aSymbol.copy(0, nPosition + 2);
                aSymbol = aSymbol.copy(nPosition + 2);
            }
            sal_Int32 nRefs = comphelper::string::getTokenCount(aSymbol, ':');
            bool bColons;
            if ( nRefs > 2 )
            {   
                bColons = true;
                sal_Int32 nIndex = 0;
                OUString aTmp1( aSymbol.getToken( 0, ':', nIndex ) );
                sal_Int32 nLen1 = aTmp1.getLength();
                OUString aSym, aTmp2;
                bool bLastAlp, bNextNum;
                bLastAlp = bNextNum = true;
                sal_Int32 nStrip = 0;
                sal_Int32 nCount = nRefs;
                for ( sal_Int32 j=1; j<nCount; j++ )
                {
                    aTmp2 = aSymbol.getToken( 0, ':', nIndex );
                    sal_Int32 nLen2 = aTmp2.getLength();
                    if ( nLen1 || nLen2 )
                    {
                        if ( nLen1 )
                        {
                            aSym += aTmp1;
                            bLastAlp = CharClass::isAsciiAlpha( aTmp1 );
                        }
                        if ( nLen2 )
                        {
                            bNextNum = CharClass::isAsciiNumeric( aTmp2 );
                            if ( bLastAlp == bNextNum && nStrip < 1 )
                            {
                                
                                
                                nRefs--;
                                nStrip++;
                            }
                            else
                            {
                                if ( !aSym.isEmpty() && !aSym.endsWith(":"))
                                    aSym += ":";
                                nStrip = 0;
                            }
                            bLastAlp = !bNextNum;
                        }
                        else
                        {   
                            nRefs--;
                            if ( nLen1 )
                            {   
                                if ( !bLastAlp && !CharClass::isAsciiNumeric( aTmp1 ) )
                                    nStrip++;
                            }
                            bNextNum = !bLastAlp;
                        }
                        aTmp1 = aTmp2;
                        nLen1 = nLen2;
                    }
                    else
                        nRefs--;
                }
                aSymbol = aSym;
                aSymbol += aTmp1;
            }
            else
                bColons = false;
            if ( nRefs && nRefs <= 2 )
            {   
                OUString aTab[2], aRef[2];
                const ScAddress::Details aDetails( pConv->meConv, aPos );
                if ( nRefs == 2 )
                {
                    aRef[0] = aSymbol.getToken( 0, ':' );
                    aRef[1] = aSymbol.getToken( 1, ':' );
                }
                else
                    aRef[0] = aSymbol;

                bool bChanged = false;
                bool bOk = true;
                sal_uInt16 nMask = SCA_VALID | SCA_VALID_COL | SCA_VALID_ROW;
                for ( int j=0; j<nRefs; j++ )
                {
                    sal_Int32 nTmp = 0;
                    sal_Int32 nDotPos = -1;
                    while ( (nTmp = aRef[j].indexOf( '.', nTmp )) != -1 )
                        nDotPos = nTmp++;      
                    if ( nDotPos != -1 )
                    {
                        aTab[j] = aRef[j].copy( 0, nDotPos + 1 );  
                        aRef[j] = aRef[j].copy( nDotPos + 1 );
                    }
                    OUString aOld( aRef[j] );
                    OUString aStr2;
                    const sal_Unicode* p = aRef[j].getStr();
                    while ( *p && CharClass::isAsciiNumeric( OUString(*p) ) )
                        aStr2 += OUString(*p++);
                    aRef[j] = OUString( p );
                    aRef[j] += aStr2;
                    if ( bColons || aRef[j] != aOld )
                    {
                        bChanged = true;
                        ScAddress aAdr;
                        bOk &= ((aAdr.Parse( aRef[j], pDoc, aDetails ) & nMask) == nMask);
                    }
                }
                if ( bChanged && bOk )
                {
                    aCorrectedSymbol = aDoc;
                    aCorrectedSymbol += aTab[0];
                    aCorrectedSymbol += aRef[0];
                    if ( nRefs == 2 )
                    {
                        aCorrectedSymbol += ":";
                        aCorrectedSymbol += aTab[1];
                        aCorrectedSymbol += aRef[1];
                    }
                    bCorrected = true;
                }
            }
        }
    }
}

static inline bool lcl_UpperAsciiOrI18n( OUString& rUpper, const OUString& rOrg, FormulaGrammar::Grammar eGrammar )
{
    if (FormulaGrammar::isODFF( eGrammar ))
    {
        
        
        rUpper = rOrg.toAsciiUpperCase();
        return true;
    }
    else
    {
        rUpper = ScGlobal::pCharClass->uppercase(rOrg);
        return false;
    }
}

bool ScCompiler::NextNewToken( bool bInArray )
{
    bool bAllowBooleans = bInArray;
    sal_Int32 nSpaces = NextSymbol(bInArray);

    if (!cSymbol[0])
        return false;

    if( nSpaces )
    {
        ScRawToken aToken;
        aToken.SetOpCode( ocSpaces );
        aToken.sbyte.cByte = (sal_uInt8) ( nSpaces > 255 ? 255 : nSpaces );
        if( !static_cast<ScTokenArray*>(pArr)->AddRawToken( aToken ) )
        {
            SetError(errCodeOverflow);
            return false;
        }
    }

    
    if (mnPredetectedReference)
    {
        OUString aStr( cSymbol);
        if (!IsPredetectedReference( aStr) && !IsExternalNamedRange( aStr))
        {
            /* TODO: it would be nice to generate a #REF! error here, which
             * would need an ocBad token with additional error value.
             * FormulaErrorToken wouldn't do because we want to preserve the
             * original string containing partial valid address
             * information if not ODFF (in that case it was already handled).
             * */
            ScRawToken aToken;
            svl::SharedString aSS = pDoc->GetSharedStringPool().intern(aStr);
            aToken.SetString(aSS.getData(), aSS.getDataIgnoreCase());
            aToken.NewOpCode( ocBad );
            pRawToken = aToken.Clone();
        }
        return true;
    }

    if ( (cSymbol[0] == '#' || cSymbol[0] == '$') && cSymbol[1] == 0 &&
            !bAutoCorrect )
    {   
        /* FIXME: ISERROR(#REF!) would be valid and true and the formula to
         * be processed as usual. That would need some special treatment,
         * also in NextSymbol() because of possible combinations of
         * #REF!.#REF!#REF! parts. In case of reading ODF that is all
         * handled by IsPredetectedReference(), this case here remains for
         * manual/API input. */
        OUString aBad( aFormula.copy( nSrcPos-1 ) );
        eLastOp = pArr->AddBad( aBad )->GetOpCode();
        return false;
    }

    if( IsString() )
        return true;

    bool bMayBeFuncName;
    bool bAsciiNonAlnum;    
    if ( cSymbol[0] < 128 )
    {
        bMayBeFuncName = rtl::isAsciiAlpha( cSymbol[0] );
        if (!bMayBeFuncName && (cSymbol[0] == '_' && cSymbol[1] == '_') )
        {
            SvtMiscOptions aOpt;
            bMayBeFuncName = aOpt.IsExperimentalMode();
        }

        bAsciiNonAlnum = !bMayBeFuncName && !rtl::isAsciiDigit( cSymbol[0] );
    }
    else
    {
        OUString aTmpStr( cSymbol[0] );
        bMayBeFuncName = ScGlobal::pCharClass->isLetter( aTmpStr, 0 );
        bAsciiNonAlnum = false;
    }
    if ( bMayBeFuncName )
    {
        
        const sal_Unicode* p = aFormula.getStr() + nSrcPos;
        while( *p == ' ' )
            p++;
        bMayBeFuncName = ( *p == '(' );
    }

    
    

    OUString aUpper;

    do
    {
        mbRewind = false;
        const OUString aOrg( cSymbol );

        if (bAsciiNonAlnum)
        {
            if (cSymbol[0] == '#')
            {
                
                lcl_UpperAsciiOrI18n( aUpper, aOrg, meGrammar);
                if (IsErrorConstant( aUpper))
                    return true;
                break;  
            }
            if (IsOpCode( aOrg, bInArray ))
                return true;
        }

        aUpper = "";
        bool bAsciiUpper = false;
        if (bMayBeFuncName)
        {
            bAsciiUpper = lcl_UpperAsciiOrI18n( aUpper, aOrg, meGrammar);
            if (IsOpCode( aUpper, bInArray ))
                return true;
        }

        
        
        
        if (IsReference( aOrg ))
        {
            if (mbRewind)   
                continue;   
            
            
            
            if (!pRawToken->IsValidReference())
            {
                aUpper = aOrg;          
                break;                  
            }
            return true;
        }

        if (aUpper.isEmpty())
            bAsciiUpper = lcl_UpperAsciiOrI18n( aUpper, aOrg, meGrammar);

        
        
        if (bAllowBooleans && IsBoolean( aUpper ))
            return true;

        if (IsValue( aUpper ))
            return true;

        
        if (bAsciiUpper)
            aUpper = ScGlobal::pCharClass->uppercase( aOrg );

        if (IsNamedRange( aUpper ))
            return true;
        
        if (IsExternalNamedRange( aOrg ))
            return true;
        if (IsDBRange( aUpper ))
            return true;
        
        
        if (!bMayBeFuncName && IsColRowName( aUpper ))
            return true;
        if (bMayBeFuncName && IsMacro( aUpper ))
            return true;
        if (bMayBeFuncName && IsOpCode2( aUpper ))
            return true;

    } while (mbRewind);

    if ( meExtendedErrorDetection != EXTENDED_ERROR_DETECTION_NONE )
    {
        
        SetError( errNoName );
        if (meExtendedErrorDetection == EXTENDED_ERROR_DETECTION_NAME_BREAK)
            return false;   
    }

    
    
    
    aUpper = ScGlobal::pCharClass->lowercase( aUpper );
    ScRawToken aToken;
    svl::SharedString aSS = pDoc->GetSharedStringPool().intern(aUpper);
    aToken.SetString(aSS.getData(), aSS.getDataIgnoreCase());
    aToken.NewOpCode( ocBad );
    pRawToken = aToken.Clone();
    if ( bAutoCorrect )
        AutoCorrectParsedSymbol();
    return true;
}

void ScCompiler::CreateStringFromXMLTokenArray( OUString& rFormula, OUString& rFormulaNmsp )
{
    bool bExternal = GetGrammar() == FormulaGrammar::GRAM_EXTERNAL;
    sal_uInt16 nExpectedCount = bExternal ? 2 : 1;
    OSL_ENSURE( pArr->GetLen() == nExpectedCount, "ScCompiler::CreateStringFromXMLTokenArray - wrong number of tokens" );
    if( pArr->GetLen() == nExpectedCount )
    {
        FormulaToken** ppTokens = pArr->GetArray();
        
        rFormula = ppTokens[0]->GetString().getString();
        if( bExternal )
            rFormulaNmsp = ppTokens[1]->GetString().getString();
    }
}

namespace {

class ExternalFileInserter : std::unary_function<sal_uInt16, void>
{
    ScAddress maPos;
    ScExternalRefManager& mrRefMgr;
public:
    ExternalFileInserter(const ScAddress& rPos, ScExternalRefManager& rRefMgr) :
        maPos(rPos), mrRefMgr(rRefMgr) {}

    void operator() (sal_uInt16 nFileId) const
    {
        mrRefMgr.insertRefCell(nFileId, maPos);
    }
};

}

ScTokenArray* ScCompiler::CompileString( const OUString& rFormula )
{
    OSL_ENSURE( meGrammar != FormulaGrammar::GRAM_EXTERNAL, "ScCompiler::CompileString - unexpected grammar GRAM_EXTERNAL" );
    if( meGrammar == FormulaGrammar::GRAM_EXTERNAL )
        SetGrammar( FormulaGrammar::GRAM_PODF );

    ScTokenArray aArr;
    pArr = &aArr;
    aFormula = comphelper::string::strip(rFormula, ' ');

    nSrcPos = 0;
    bCorrected = false;
    if ( bAutoCorrect )
    {
        aCorrectedFormula = "";
        aCorrectedSymbol = "";
    }
    sal_uInt8 nForced = 0;   
    if( nSrcPos < aFormula.getLength() && aFormula[nSrcPos] == '=' )
    {
        nSrcPos++;
        nForced++;
        if ( bAutoCorrect )
            aCorrectedFormula += "=";
    }
    if( nSrcPos < aFormula.getLength() && aFormula[nSrcPos] == '=' )
    {
        nSrcPos++;
        nForced++;
        if ( bAutoCorrect )
            aCorrectedFormula += "=";
    }
    struct FunctionStack
    {
        OpCode  eOp;
        short   nSep;
    };
    
    bool bPODF = FormulaGrammar::isPODF( meGrammar);
    bool bOOXML = FormulaGrammar::isOOXML( meGrammar);
    bool bUseFunctionStack = (bPODF || bOOXML);
    const size_t nAlloc = 512;
    FunctionStack aFuncs[ nAlloc ];
    FunctionStack* pFunctionStack = (bUseFunctionStack && static_cast<size_t>(rFormula.getLength()) > nAlloc ?
         new FunctionStack[rFormula.getLength()] : &aFuncs[0]);
    pFunctionStack[0].eOp = ocNone;
    pFunctionStack[0].nSep = 0;
    size_t nFunction = 0;
    short nBrackets = 0;
    bool bInArray = false;
    eLastOp = ocOpen;
    while( NextNewToken( bInArray ) )
    {
        const OpCode eOp = pRawToken->GetOpCode();
        if (eOp == ocSkip)
            continue;

        switch (eOp)
        {
            case ocOpen:
            {
                ++nBrackets;
                if (bUseFunctionStack)
                {
                    ++nFunction;
                    pFunctionStack[ nFunction ].eOp = eLastOp;
                    pFunctionStack[ nFunction ].nSep = 0;
                }
            }
            break;
            case ocClose:
            {
                if( !nBrackets )
                {
                    SetError( errPairExpected );
                    if ( bAutoCorrect )
                    {
                        bCorrected = true;
                        aCorrectedSymbol = "";
                    }
                }
                else
                    nBrackets--;
                if (bUseFunctionStack && nFunction)
                    --nFunction;
            }
            break;
            case ocSep:
            {
                if (bUseFunctionStack)
                    ++pFunctionStack[ nFunction ].nSep;
            }
            break;
            case ocArrayOpen:
            {
                if( bInArray )
                    SetError( errNestedArray );
                else
                    bInArray = true;
                
                if (bUseFunctionStack)
                {
                    ++nFunction;
                    pFunctionStack[ nFunction ].eOp = eOp;
                    pFunctionStack[ nFunction ].nSep = 0;
                }
            }
            break;
            case ocArrayClose:
            {
                if( bInArray )
                {
                    bInArray = false;
                }
                else
                {
                    SetError( errPairExpected );
                    if ( bAutoCorrect )
                    {
                        bCorrected = true;
                        aCorrectedSymbol = "";
                    }
                }
                if (bUseFunctionStack && nFunction)
                    --nFunction;
            }
            default:
            break;
        }
        if( (eLastOp == ocSep ||
             eLastOp == ocArrayRowSep ||
             eLastOp == ocArrayColSep ||
             eLastOp == ocArrayOpen) &&
            (eOp == ocSep ||
             eOp == ocClose ||
             eOp == ocArrayRowSep ||
             eOp == ocArrayColSep ||
             eOp == ocArrayClose) )
        {
            
            
            if ( !static_cast<ScTokenArray*>(pArr)->Add( new FormulaMissingToken ) )
            {
                SetError(errCodeOverflow); break;
            }
        }
        if (bOOXML)
        {
            
            
            size_t nFunc = nFunction + 1;
            if (eOp == ocClose && (
                    (pFunctionStack[ nFunc ].eOp == ocCeil &&   
                     pFunctionStack[ nFunc ].nSep == 1) ||
                    (pFunctionStack[ nFunc ].eOp == ocFloor &&  
                     pFunctionStack[ nFunc ].nSep == 1) ||
                    (pFunctionStack[ nFunc ].eOp == ocWeek &&   
                     pFunctionStack[ nFunc ].nSep == 0)))
            {
                if (    !static_cast<ScTokenArray*>(pArr)->Add( new FormulaToken( svSep, ocSep)) ||
                        !static_cast<ScTokenArray*>(pArr)->Add( new FormulaDoubleToken( 1.0)))
                {
                    SetError(errCodeOverflow); break;
                }
            }
        }
        else if (bPODF)
        {
            /* TODO: for now this is the only PODF adapter. If there were more,
             * factor this out. */
            
            if (eOp == ocSep &&
                    pFunctionStack[ nFunction ].eOp == ocAddress &&
                    pFunctionStack[ nFunction ].nSep == 3)
            {
                if (    !static_cast<ScTokenArray*>(pArr)->Add( new FormulaToken( svSep, ocSep)) ||
                        !static_cast<ScTokenArray*>(pArr)->Add( new FormulaDoubleToken( 1.0)))
                {
                    SetError(errCodeOverflow); break;
                }
                ++pFunctionStack[ nFunction ].nSep;
            }
        }
        FormulaToken* pNewToken = static_cast<ScTokenArray*>(pArr)->Add( pRawToken->CreateToken());
        if (!pNewToken)
        {
            SetError(errCodeOverflow); break;
        }
        else if (eLastOp == ocRange && pNewToken->GetOpCode() == ocPush &&
                pNewToken->GetType() == svSingleRef)
            static_cast<ScTokenArray*>(pArr)->MergeRangeReference( aPos);
        eLastOp = pRawToken->GetOpCode();
        if ( bAutoCorrect )
            aCorrectedFormula += aCorrectedSymbol;
    }
    if ( mbCloseBrackets )
    {
        if( bInArray )
        {
            FormulaByteToken aToken( ocArrayClose );
            if( !pArr->AddToken( aToken ) )
            {
                SetError(errCodeOverflow);
            }
            else if ( bAutoCorrect )
                aCorrectedFormula += mxSymbols->getSymbol(ocArrayClose);
        }

        FormulaByteToken aToken( ocClose );
        while( nBrackets-- )
        {
            if( !pArr->AddToken( aToken ) )
            {
                SetError(errCodeOverflow); break;
            }
            if ( bAutoCorrect )
                aCorrectedFormula += mxSymbols->getSymbol(ocClose);
        }
    }
    if ( nForced >= 2 )
        pArr->SetRecalcModeForced();

    if (pFunctionStack != &aFuncs[0])
        delete [] pFunctionStack;

    
    ScTokenArray* pNew = new ScTokenArray( aArr );
    pNew->GenHash();
    pArr = pNew;

    if (!maExternalFiles.empty())
    {
        
        std::sort(maExternalFiles.begin(), maExternalFiles.end());
        std::vector<sal_uInt16>::iterator itEnd = std::unique(maExternalFiles.begin(), maExternalFiles.end());
        std::for_each(maExternalFiles.begin(), itEnd, ExternalFileInserter(aPos, *pDoc->GetExternalRefManager()));
        maExternalFiles.erase(itEnd, maExternalFiles.end());
    }

    return pNew;
}

ScTokenArray* ScCompiler::CompileString( const OUString& rFormula, const OUString& rFormulaNmsp )
{
    OSL_ENSURE( (GetGrammar() == FormulaGrammar::GRAM_EXTERNAL) || rFormulaNmsp.isEmpty(),
        "ScCompiler::CompileString - unexpected formula namespace for internal grammar" );
    if( GetGrammar() == FormulaGrammar::GRAM_EXTERNAL ) try
    {
        ScFormulaParserPool& rParserPool = pDoc->GetFormulaParserPool();
        uno::Reference< sheet::XFormulaParser > xParser( rParserPool.getFormulaParser( rFormulaNmsp ), uno::UNO_SET_THROW );
        table::CellAddress aReferencePos;
        ScUnoConversion::FillApiAddress( aReferencePos, aPos );
        uno::Sequence< sheet::FormulaToken > aTokenSeq = xParser->parseFormula( rFormula, aReferencePos );
        ScTokenArray aTokenArray;
        if( ScTokenConversion::ConvertToTokenArray( *pDoc, aTokenArray, aTokenSeq ) )
        {
            
            ScTokenArray* pNew = new ScTokenArray( aTokenArray );
            pArr = pNew;
            return pNew;
        }
    }
    catch( uno::Exception& )
    {
    }
    
    return CompileString( rFormula );
}

ScRangeData* ScCompiler::GetRangeData( const FormulaToken& rToken ) const
{
    ScRangeData* pRangeData = NULL;
    bool bGlobal = rToken.IsGlobal();
    if (bGlobal)
        
        pRangeData = pDoc->GetRangeName()->findByIndex( rToken.GetIndex());
    else
    {
        
        const ScRangeName* pRN = pDoc->GetRangeName( aPos.Tab());
        if (pRN)
            pRangeData = pRN->findByIndex( rToken.GetIndex());
    }
    return pRangeData;
}

bool ScCompiler::HandleRange()
{
    const ScRangeData* pRangeData = GetRangeData( *mpToken);
    if (pRangeData)
    {
        sal_uInt16 nErr = pRangeData->GetErrCode();
        if( nErr )
            SetError( errNoName );
        else if ( !bCompileForFAP )
        {
            ScTokenArray* pNew;
            
            
            
            
            
            
            FormulaToken* p1 = pArr->PeekPrevNoSpaces();
            FormulaToken* p2 = pArr->PeekNextNoSpaces();
            OpCode eOp1 = (p1 ? p1->GetOpCode() : static_cast<OpCode>( ocSep ) );
            OpCode eOp2 = (p2 ? p2->GetOpCode() : static_cast<OpCode>( ocSep ) );
            bool bBorder1 = (eOp1 == ocSep || eOp1 == ocOpen);
            bool bBorder2 = (eOp2 == ocSep || eOp2 == ocClose);
            bool bAddPair = !(bBorder1 && bBorder2);
            if ( bAddPair )
            {
                pNew = new ScTokenArray();
                pNew->AddOpCode( ocClose );
                PushTokenArray( pNew, true );
                pNew->Reset();
            }
            pNew = pRangeData->GetCode()->Clone();
            PushTokenArray( pNew, true );
            if( pRangeData->HasReferences() )
            {
                SetRelNameReference();
                MoveRelWrap(pRangeData->GetMaxCol(), pRangeData->GetMaxRow());
            }
            pNew->Reset();
            if ( bAddPair )
            {
                pNew = new ScTokenArray();
                pNew->AddOpCode( ocOpen );
                PushTokenArray( pNew, true );
                pNew->Reset();
            }
            return GetToken();
        }
    }
    else
        SetError(errNoName);
    return true;
}

bool ScCompiler::HandleExternalReference(const FormulaToken& _aToken)
{
    
    switch (_aToken.GetType())
    {
        case svExternalSingleRef:
        case svExternalDoubleRef:
            pArr->IncrementRefs();
        break;
        case svExternalName:
        {
            ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
            const OUString* pFile = pRefMgr->getExternalFileName(_aToken.GetIndex());
            if (!pFile)
            {
                SetError(errNoName);
                return true;
            }

            OUString aName = _aToken.GetString().getString();
            ScExternalRefCache::TokenArrayRef xNew = pRefMgr->getRangeNameTokens(
                _aToken.GetIndex(), aName, &aPos);

            if (!xNew)
            {
                SetError(errNoName);
                return true;
            }

            ScTokenArray* pNew = xNew->Clone();
            PushTokenArray( pNew, true);
            if (pNew->GetNextReference() != NULL)
            {
                SetRelNameReference();
                MoveRelWrap(MAXCOL, MAXROW);
            }
            pNew->Reset();
            return GetToken();
        }
        default:
            OSL_FAIL("Wrong type for external reference!");
            return false;
    }
    return true;
}

template< typename T, typename S >
static S lcl_adjval( S& n, T pos, T max, bool bRel )
{
    max++;
    if( bRel )
        n = sal::static_int_cast<S>( n + pos );
    if( n < 0 )
        n = sal::static_int_cast<S>( n + max );
    else if( n >= max )
        n = sal::static_int_cast<S>( n - max );
    if( bRel )
        n = sal::static_int_cast<S>( n - pos );
    return n;
}



void ScCompiler::SetRelNameReference()
{
    pArr->Reset();
    for( ScToken* t = static_cast<ScToken*>(pArr->GetNextReference()); t;
                  t = static_cast<ScToken*>(pArr->GetNextReference()) )
    {
        ScSingleRefData& rRef1 = t->GetSingleRef();
        if ( rRef1.IsColRel() || rRef1.IsRowRel() || rRef1.IsTabRel() )
            rRef1.SetRelName( true );
        if ( t->GetType() == svDoubleRef )
        {
            ScSingleRefData& rRef2 = t->GetDoubleRef().Ref2;
            if ( rRef2.IsColRel() || rRef2.IsRowRel() || rRef2.IsTabRel() )
                rRef2.SetRelName( true );
        }
    }
}



void ScCompiler::MoveRelWrap( SCCOL nMaxCol, SCROW nMaxRow )
{
    pArr->Reset();
    for( ScToken* t = static_cast<ScToken*>(pArr->GetNextReference()); t;
                  t = static_cast<ScToken*>(pArr->GetNextReference()) )
    {
        if ( t->GetType() == svSingleRef || t->GetType() == svExternalSingleRef )
            ScRefUpdate::MoveRelWrap( pDoc, aPos, nMaxCol, nMaxRow, SingleDoubleRefModifier( t->GetSingleRef() ).Ref() );
        else
            ScRefUpdate::MoveRelWrap( pDoc, aPos, nMaxCol, nMaxRow, t->GetDoubleRef() );
    }
}



void ScCompiler::MoveRelWrap( ScTokenArray& rArr, ScDocument* pDoc, const ScAddress& rPos,
                              SCCOL nMaxCol, SCROW nMaxRow )
{
    rArr.Reset();
    for( ScToken* t = static_cast<ScToken*>(rArr.GetNextReference()); t;
                  t = static_cast<ScToken*>(rArr.GetNextReference()) )
    {
        if ( t->GetType() == svSingleRef || t->GetType() == svExternalSingleRef )
            ScRefUpdate::MoveRelWrap( pDoc, rPos, nMaxCol, nMaxRow, SingleDoubleRefModifier( t->GetSingleRef() ).Ref() );
        else
            ScRefUpdate::MoveRelWrap( pDoc, rPos, nMaxCol, nMaxRow, t->GetDoubleRef() );
    }
}

bool ScCompiler::IsCharFlagAllConventions(
    OUString const & rStr, sal_Int32 nPos, sal_uLong nFlags, bool bTestLetterNumeric )
{
    sal_Unicode c = rStr[ nPos ];
    sal_Unicode cLast = nPos > 0 ? rStr[ nPos-1 ] : 0;
    if (c < 128)
    {
        for ( int nConv = formula::FormulaGrammar::CONV_UNSPECIFIED;
                ++nConv < formula::FormulaGrammar::CONV_LAST; )
        {
            if (pConventions[nConv] &&
                    ((pConventions[nConv]->getCharTableFlags(c, cLast) & nFlags) != nFlags))
                return false;
            
        }
        return true;
    }
    else if (bTestLetterNumeric)
        return ScGlobal::pCharClass->isLetterNumeric( rStr, nPos );
    else
        return false;
}

void ScCompiler::CreateStringFromExternal(OUStringBuffer& rBuffer, FormulaToken* pTokenP) const
{
    FormulaToken* t = pTokenP;
    ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
    const OUString* pFileName = pRefMgr->getExternalFileName(t->GetIndex());
    if (!pFileName)
        return;

    switch (t->GetType())
    {
        case svExternalName:
            rBuffer.append(pConv->makeExternalNameStr(*pFileName, t->GetString().getString()));
        break;
        case svExternalSingleRef:
            pConv->makeExternalRefStr(
                   rBuffer, GetPos(), *pFileName, t->GetString().getString(),
                   static_cast<ScToken*>(t)->GetSingleRef());
        break;
        case svExternalDoubleRef:
        {
            vector<OUString> aTabNames;
            pRefMgr->getAllCachedTableNames(t->GetIndex(), aTabNames);
            if (aTabNames.empty())
                return;

            pConv->makeExternalRefStr(
                rBuffer, GetPos(), *pFileName, aTabNames, t->GetString().getString(),
                static_cast<ScToken*>(t)->GetDoubleRef());
        }
        break;
        default:
            
            
            OSL_FAIL("ScCompiler::CreateStringFromToken: unknown type of ocExternalRef");
    }
}

void ScCompiler::CreateStringFromMatrix(
    OUStringBuffer& rBuffer, FormulaToken* pTokenP) const
{
    const ScMatrix* pMatrix = static_cast<ScToken*>(pTokenP)->GetMatrix();
    SCSIZE nC, nMaxC, nR, nMaxR;

    pMatrix->GetDimensions( nMaxC, nMaxR);

    rBuffer.append( mxSymbols->getSymbol(ocArrayOpen) );
    for( nR = 0 ; nR < nMaxR ; nR++)
    {
        if( nR > 0)
        {
            rBuffer.append( mxSymbols->getSymbol(ocArrayRowSep) );
        }

        for( nC = 0 ; nC < nMaxC ; nC++)
        {
            if( nC > 0)
            {
                rBuffer.append( mxSymbols->getSymbol(ocArrayColSep) );
            }

            if( pMatrix->IsValue( nC, nR ) )
            {
                if (pMatrix->IsBoolean(nC, nR))
                    AppendBoolean(rBuffer, pMatrix->GetDouble(nC, nR) != 0.0);
                else
                {
                    sal_uInt16 nErr = pMatrix->GetError(nC, nR);
                    if (nErr)
                        rBuffer.append(ScGlobal::GetErrorString(nErr));
                    else
                        AppendDouble(rBuffer, pMatrix->GetDouble(nC, nR));
                }
            }
            else if( pMatrix->IsEmpty( nC, nR ) )
                ;
            else if( pMatrix->IsString( nC, nR ) )
                AppendString( rBuffer, pMatrix->GetString(nC, nR).getString() );
        }
    }
    rBuffer.append( mxSymbols->getSymbol(ocArrayClose) );
}

void ScCompiler::CreateStringFromSingleRef(OUStringBuffer& rBuffer,FormulaToken* _pTokenP) const
{
    OUString aErrRef = GetCurrentOpCodeMap()->getSymbol(ocErrRef);
    const OpCode eOp = _pTokenP->GetOpCode();
    const ScSingleRefData& rRef = static_cast<const ScToken*>(_pTokenP)->GetSingleRef();
    ScComplexRefData aRef;
    aRef.Ref1 = aRef.Ref2 = rRef;
    if ( eOp == ocColRowName )
    {
        ScAddress aAbs = rRef.toAbs(aPos);
        if (pDoc->HasStringData(aAbs.Col(), aAbs.Row(), aAbs.Tab()))
        {
            OUString aStr = pDoc->GetString(aAbs);
            EnQuote( aStr );
            rBuffer.append(aStr);
        }
        else
        {
            rBuffer.append(ScGlobal::GetRscString(STR_NO_NAME_REF));
            pConv->makeRefStr(rBuffer, meGrammar, aPos, aErrRef, maTabNames, aRef, true);
        }
    }
    else
        pConv->makeRefStr(rBuffer, meGrammar, aPos, aErrRef, maTabNames, aRef, true);
}

void ScCompiler::CreateStringFromDoubleRef(OUStringBuffer& rBuffer,FormulaToken* _pTokenP) const
{
    OUString aErrRef = GetCurrentOpCodeMap()->getSymbol(ocErrRef);
    pConv->makeRefStr(rBuffer, meGrammar, aPos, aErrRef, maTabNames, static_cast<ScToken*>(_pTokenP)->GetDoubleRef(), false);
}

void ScCompiler::CreateStringFromIndex(OUStringBuffer& rBuffer,FormulaToken* _pTokenP) const
{
    const OpCode eOp = _pTokenP->GetOpCode();
    OUStringBuffer aBuffer;
    switch ( eOp )
    {
        case ocName:
        {
            const ScRangeData* pData = GetRangeData( *_pTokenP);
            if (pData)
                aBuffer.append(pData->GetName());
        }
        break;
        case ocDBArea:
        {
            const ScDBData* pDBData = pDoc->GetDBCollection()->getNamedDBs().findByIndex(_pTokenP->GetIndex());
            if (pDBData)
                aBuffer.append(pDBData->GetName());
        }
        break;
        default:
            ;   
    }
    if ( !aBuffer.isEmpty() )
        rBuffer.append(aBuffer.makeStringAndClear());
    else
        rBuffer.append(ScGlobal::GetRscString(STR_NO_NAME_REF));
}

void ScCompiler::LocalizeString( OUString& rName ) const
{
    ScGlobal::GetAddInCollection()->LocalizeString( rName );
}



bool ScCompiler::EnQuote( OUString& rStr )
{
    sal_Int32 nType = ScGlobal::pCharClass->getStringType( rStr, 0, rStr.getLength() );
    if ( !CharClass::isNumericType( nType )
            && CharClass::isAlphaNumericType( nType ) )
        return false;

    sal_Int32 nPos = 0;
    while ( (nPos = rStr.indexOf( '\'', nPos)) != -1 )
    {
        rStr = rStr.replaceAt( nPos, 0, "\\" );
        nPos += 2;
    }
    rStr = "'" + rStr + "'";
    return true;
}

sal_Unicode ScCompiler::GetNativeAddressSymbol( Convention::SpecialSymbolType eType ) const
{
    return pConv->getSpecialSymbol(eType);
}

void ScCompiler::fillAddInToken(::std::vector< ::com::sun::star::sheet::FormulaOpCodeMapEntry >& _rVec,bool _bIsEnglish) const
{
    
    sheet::FormulaOpCodeMapEntry aEntry;
    aEntry.Token.OpCode = ocExternal;

    ScUnoAddInCollection* pColl = ScGlobal::GetAddInCollection();
    const long nCount = pColl->GetFuncCount();
    for (long i=0; i < nCount; ++i)
    {
        const ScUnoAddInFuncData* pFuncData = pColl->GetFuncData(i);
        if (pFuncData)
        {
            if ( _bIsEnglish )
            {
                OUString aName;
                if (pFuncData->GetExcelName( LANGUAGE_ENGLISH_US, aName))
                    aEntry.Name = aName;
                else
                    aEntry.Name = pFuncData->GetUpperName();
            }
            else
                aEntry.Name = pFuncData->GetUpperLocal();
            aEntry.Token.Data <<= OUString( pFuncData->GetOriginalName());
            _rVec.push_back( aEntry);
        }
    }
    
}

bool ScCompiler::HandleSingleRef()
{
    ScSingleRefData& rRef = static_cast<ScToken*>(mpToken.get())->GetSingleRef();
    ScAddress aAbs = rRef.toAbs(aPos);
    if (!ValidAddress(aAbs))
    {
        SetError( errNoRef );
        return true;
    }
    SCCOL nCol = aAbs.Col();
    SCROW nRow = aAbs.Row();
    SCTAB nTab = aAbs.Tab();
    ScAddress aLook = aAbs;
    bool bColName = rRef.IsColRel();
    SCCOL nMyCol = aPos.Col();
    SCROW nMyRow = aPos.Row();
    bool bInList = false;
    bool bValidName = false;
    ScRangePairList* pRL = (bColName ?
        pDoc->GetColNameRanges() : pDoc->GetRowNameRanges());
    ScRange aRange;
    for ( size_t i = 0, nPairs = pRL->size(); i < nPairs; ++i )
    {
        ScRangePair* pR = (*pRL)[i];
        if ( pR->GetRange(0).In( aLook ) )
        {
            bInList = bValidName = true;
            aRange = pR->GetRange(1);
            if ( bColName )
            {
                aRange.aStart.SetCol( nCol );
                aRange.aEnd.SetCol( nCol );
            }
            else
            {
                aRange.aStart.SetRow( nRow );
                aRange.aEnd.SetRow( nRow );
            }
            break;  
        }
    }
    if ( !bInList && pDoc->GetDocOptions().IsLookUpColRowNames() )
    {   
        ScRefCellValue aCell;
        aCell.assign(*pDoc, aLook);
        bool bString = aCell.hasString();
        if (!bString && aCell.isEmpty())
            bString = true;     
        if ( bString )
        {   
            bValidName = true;
            if ( bColName )
            {   
                SCROW nStartRow = nRow + 1;
                if ( nStartRow > MAXROW )
                    nStartRow = MAXROW;
                SCROW nMaxRow = MAXROW;
                if ( nMyCol == nCol )
                {   
                    if ( nMyRow == nStartRow )
                    {   
                        nStartRow++;
                        if ( nStartRow > MAXROW )
                            nStartRow = MAXROW;
                    }
                    else if ( nMyRow > nStartRow )
                    {   
                        nMaxRow = nMyRow - 1;
                    }
                }
                for ( size_t i = 0, nPairs = pRL->size(); i < nPairs; ++i )
                {   
                    ScRangePair* pR = (*pRL)[i];
                    const ScRange& rRange = pR->GetRange(1);
                    if ( rRange.aStart.Col() <= nCol && nCol <= rRange.aEnd.Col() )
                    {   
                        SCROW nTmp = rRange.aStart.Row();
                        if ( nStartRow < nTmp && nTmp <= nMaxRow )
                            nMaxRow = nTmp - 1;
                    }
                }
                aRange.aStart.Set( nCol, nStartRow, nTab );
                aRange.aEnd.Set( nCol, nMaxRow, nTab );
            }
            else
            {   
                SCCOL nStartCol = nCol + 1;
                if ( nStartCol > MAXCOL )
                    nStartCol = MAXCOL;
                SCCOL nMaxCol = MAXCOL;
                if ( nMyRow == nRow )
                {   
                    if ( nMyCol == nStartCol )
                    {   
                        nStartCol++;
                        if ( nStartCol > MAXCOL )
                            nStartCol = MAXCOL;
                    }
                    else if ( nMyCol > nStartCol )
                    {   
                        nMaxCol = nMyCol - 1;
                    }
                }
                for ( size_t i = 0, nPairs = pRL->size(); i < nPairs; ++i )
                {   
                    ScRangePair* pR = (*pRL)[i];
                    const ScRange& rRange = pR->GetRange(1);
                    if ( rRange.aStart.Row() <= nRow && nRow <= rRange.aEnd.Row() )
                    {   
                        SCCOL nTmp = rRange.aStart.Col();
                        if ( nStartCol < nTmp && nTmp <= nMaxCol )
                            nMaxCol = nTmp - 1;
                    }
                }
                aRange.aStart.Set( nStartCol, nRow, nTab );
                aRange.aEnd.Set( nMaxCol, nRow, nTab );
            }
        }
    }
    if ( bValidName )
    {
        
        
        
        
        
        
        
        bool bSingle = (aRange.aStart == aRange.aEnd);
        bool bFound;
        if ( bSingle )
            bFound = true;
        else
        {
            FormulaToken* p1 = pArr->PeekPrevNoSpaces();
            FormulaToken* p2 = pArr->PeekNextNoSpaces();
            
            OpCode eOp1 = p1 ? p1->GetOpCode() : static_cast<OpCode>( ocAdd );
            OpCode eOp2 = p2 ? p2->GetOpCode() : static_cast<OpCode>( ocAdd );
            if ( eOp1 != ocColRowName && eOp1 != ocIntersect
                && eOp2 != ocColRowName && eOp2 != ocIntersect )
            {
                if (    (SC_OPCODE_START_BIN_OP <= eOp1 && eOp1 < SC_OPCODE_STOP_BIN_OP) ||
                        (SC_OPCODE_START_BIN_OP <= eOp2 && eOp2 < SC_OPCODE_STOP_BIN_OP))
                    bSingle = true;
            }
            if ( bSingle )
            {   
                if ( bColName )
                {
                    bFound = (aRange.aStart.Row() <= nMyRow
                        && nMyRow <= aRange.aEnd.Row());
                    if ( bFound )
                        aRange.aStart.SetRow( nMyRow );
                }
                else
                {
                    bFound = (aRange.aStart.Col() <= nMyCol
                        && nMyCol <= aRange.aEnd.Col());
                    if ( bFound )
                        aRange.aStart.SetCol( nMyCol );
                }
            }
            else
                bFound = true;
        }
        if ( !bFound )
            SetError(errNoRef);
        else if ( !bCompileForFAP )
        {
            ScTokenArray* pNew = new ScTokenArray();
            if ( bSingle )
            {
                ScSingleRefData aRefData;
                aRefData.InitAddress( aRange.aStart );
                if ( bColName )
                    aRefData.SetColRel( true );
                else
                    aRefData.SetRowRel( true );
                aRefData.SetAddress(aRange.aStart, aPos);
                pNew->AddSingleReference( aRefData );
            }
            else
            {
                ScComplexRefData aRefData;
                aRefData.InitRange( aRange );
                if ( bColName )
                {
                    aRefData.Ref1.SetColRel( true );
                    aRefData.Ref2.SetColRel( true );
                }
                else
                {
                    aRefData.Ref1.SetRowRel( true );
                    aRefData.Ref2.SetRowRel( true );
                }
                aRefData.SetRange(aRange, aPos);
                if ( bInList )
                    pNew->AddDoubleReference( aRefData );
                else
                {   
                    pNew->Add( new ScDoubleRefToken( aRefData, ocColRowNameAuto ) );
                }
            }
            PushTokenArray( pNew, true );
            pNew->Reset();
            return GetToken();
        }
    }
    else
        SetError(errNoName);
    return true;
}

bool ScCompiler::HandleDbData()
{
    ScDBData* pDBData = pDoc->GetDBCollection()->getNamedDBs().findByIndex(mpToken->GetIndex());
    if ( !pDBData )
        SetError(errNoName);
    else if ( !bCompileForFAP )
    {
        ScComplexRefData aRefData;
        aRefData.InitFlags();
        ScRange aRange;
        pDBData->GetArea(aRange);
        aRange.aEnd.SetTab(aRange.aStart.Tab());
        aRefData.SetRange(aRange, aPos);
        ScTokenArray* pNew = new ScTokenArray();
        pNew->AddDoubleReference( aRefData );
        PushTokenArray( pNew, true );
        pNew->Reset();
        return GetToken();
    }
    return true;
}

FormulaTokenRef ScCompiler::ExtendRangeReference( FormulaToken & rTok1, FormulaToken & rTok2, bool bReuseDoubleRef )
{
    return ScToken::ExtendRangeReference( rTok1, rTok2, aPos,bReuseDoubleRef );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

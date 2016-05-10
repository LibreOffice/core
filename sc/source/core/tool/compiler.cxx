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

#include <config_features.h>

#include "compiler.hxx"

#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbstar.hxx>
#include <svl/zforlist.hxx>
#include <svl/sharedstringpool.hxx>
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

CharClass*                          ScCompiler::pCharClassEnglish = nullptr;
const ScCompiler::Convention*       ScCompiler::pConventions[ ]   = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

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
    ssGetTableRefItem,
    ssGetTableRefColumn,
    ssStop
};

static const sal_Char* pInternal[2] = { "TTT", "__DEBUG_VAR" };

using namespace ::com::sun::star::i18n;

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
        pCharClassEnglish = nullptr;
    }
}

bool ScCompiler::IsEnglishSymbol( const OUString& rName )
{
    // function names are always case-insensitive
    OUString aUpper = ScGlobal::pCharClass->uppercase(rName);

    // 1. built-in function name
    OpCode eOp = ScCompiler::GetEnglishOpCode( aUpper );
    if ( eOp != ocNone )
    {
        return true;
    }
    // 2. old add in functions
    if (ScGlobal::GetLegacyFuncCollection()->findByName(aUpper))
    {
        return true;
    }

    // 3. new (uno) add in functions
    OUString aIntName = ScGlobal::GetAddInCollection()->FindFunction(aUpper, false);
    if (!aIntName.isEmpty())
    {
        return true;
    }
    return false;       // no valid function name
}

void ScCompiler::InitCharClassEnglish()
{
    css::lang::Locale aLocale( "en", "US", "");
    pCharClassEnglish = new CharClass(
            ::comphelper::getProcessComponentContext(), LanguageTag( aLocale));
}

void ScCompiler::SetGrammar( const FormulaGrammar::Grammar eGrammar )
{
    OSL_ENSURE( eGrammar != FormulaGrammar::GRAM_UNSPECIFIED, "ScCompiler::SetGrammar: don't pass FormulaGrammar::GRAM_UNSPECIFIED");
    if (eGrammar == GetGrammar())
        return;     // nothing to be done

    if( eGrammar == FormulaGrammar::GRAM_EXTERNAL )
    {
        meGrammar = eGrammar;
        mxSymbols = GetOpCodeMap( css::sheet::FormulaLanguage::NATIVE);
    }
    else
    {
        FormulaGrammar::Grammar eMyGrammar = eGrammar;
        const sal_Int32 nFormulaLanguage = FormulaGrammar::extractFormulaLanguage( eMyGrammar);
        OpCodeMapPtr xMap = GetOpCodeMap( nFormulaLanguage);
        OSL_ENSURE( xMap, "ScCompiler::SetGrammar: unknown formula language");
        if (!xMap)
        {
            xMap = GetOpCodeMap( css::sheet::FormulaLanguage::NATIVE);
            eMyGrammar = xMap->getGrammar();
        }

        // Save old grammar for call to SetGrammarAndRefConvention().
        FormulaGrammar::Grammar eOldGrammar = GetGrammar();
        // This also sets the grammar associated with the map!
        SetFormulaLanguage( xMap);

        // Override if necessary.
        if (eMyGrammar != GetGrammar())
            SetGrammarAndRefConvention( eMyGrammar, eOldGrammar);
    }
}

// Unclear how this was intended to be refreshed when the
// grammar or sheet count is changed ? Ideally this would be
// a method on Document that would globally cache these.
std::vector<OUString> &ScCompiler::GetSetupTabNames() const
{
    std::vector<OUString> &rTabNames = const_cast<ScCompiler *>(this)->maTabNames;

    if (pDoc && rTabNames.empty())
    {
        rTabNames = pDoc->GetAllTableNames();
        std::vector<OUString>::iterator it = rTabNames.begin(), itEnd = rTabNames.end();
        for (; it != itEnd; ++it)
            ScCompiler::CheckTabQuotes(*it, formula::FormulaGrammar::extractRefConvention(meGrammar));
    }

    return rTabNames;
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
    meGrammar = eNewGrammar;    // SetRefConvention needs the new grammar set!
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
    return ScGlobal::GetAddInCollection()->FindFunction(rUpperName, bLocalFirst);    // bLocalFirst=false for english
}

ScCompiler::Convention::~Convention()
{
    delete [] mpCharTable;
    mpCharTable = nullptr;
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

// tdf#56036: Allow tabs/newlines in imported formulas (for now simply treat them as (and convert to) space)
// TODO: tdf#76310: allow saving newlines as is (as per OpenFormula specification v.1.2, clause 5.14 "Whitespace")
// This is compliant with the OASIS decision (see https://issues.oasis-open.org/browse/OFFICE-701)
// Also, this would enable correct roundtrip from/to OOXML without losing tabs/newlines
// This requires saving actual space characters in ocSpaces token, using them in UI and saving
/* tab */   t[ 9] = SC_COMPILER_C_CHAR_DONTCARE | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
/* lf  */   t[10] = SC_COMPILER_C_CHAR_DONTCARE | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
/* cr  */   t[13] = SC_COMPILER_C_CHAR_DONTCARE | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;

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
/* @ */     // FREE

    for (i = 65; i < 91; i++)
/* A-Z */   t[i] = SC_COMPILER_C_CHAR_WORD | SC_COMPILER_C_WORD | SC_COMPILER_C_CHAR_IDENT | SC_COMPILER_C_IDENT | SC_COMPILER_C_CHAR_NAME | SC_COMPILER_C_NAME;

    if (FormulaGrammar::CONV_ODF == meConv)
    {
/* [ */     t[91] = SC_COMPILER_C_ODF_LBRACKET;
/* \ */     // FREE
/* ] */     t[93] = SC_COMPILER_C_ODF_RBRACKET;
    }
    else if (FormulaGrammar::CONV_OOO == meConv)
    {
/* [ */     t[91] = SC_COMPILER_C_CHAR;
/* \ */     // FREE
/* ] */     t[93] = SC_COMPILER_C_CHAR;
    }
    else if (FormulaGrammar::CONV_XL_OOX == meConv)
    {
/* [ */     t[91] = SC_COMPILER_C_CHAR | SC_COMPILER_C_CHAR_IDENT;
/* \ */     // FREE
/* ] */     t[93] = SC_COMPILER_C_CHAR | SC_COMPILER_C_IDENT;
    }
    else if (FormulaGrammar::CONV_XL_A1 == meConv)
    {
/* [ */     t[91] = SC_COMPILER_C_CHAR;
/* \ */     // FREE
/* ] */     t[93] = SC_COMPILER_C_CHAR;
    }
    else if( FormulaGrammar::CONV_XL_R1C1 == meConv )
    {
/* [ */     t[91] = SC_COMPILER_C_IDENT;
/* \ */     // FREE
/* ] */     t[93] = SC_COMPILER_C_IDENT;
    }
    else
    {
/* [ */     // FREE
/* \ */     // FREE
/* ] */     // FREE
    }

/* ^ */     t[94] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP;
/* _ */     t[95] = SC_COMPILER_C_CHAR_WORD | SC_COMPILER_C_WORD | SC_COMPILER_C_CHAR_IDENT | SC_COMPILER_C_IDENT | SC_COMPILER_C_CHAR_NAME | SC_COMPILER_C_NAME;
/* ` */     // FREE

            for (i = 97; i < 123; i++)
/* a-z */       t[i] = SC_COMPILER_C_CHAR_WORD | SC_COMPILER_C_WORD | SC_COMPILER_C_CHAR_IDENT | SC_COMPILER_C_IDENT | SC_COMPILER_C_CHAR_NAME | SC_COMPILER_C_NAME;

/* { */     t[123] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP; // array open
/* | */     t[124] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP; // array row sep (Should be OOo specific)
/* } */     t[125] = SC_COMPILER_C_CHAR | SC_COMPILER_C_WORD_SEP | SC_COMPILER_C_VALUE_SEP; // array close
/* ~ */     t[126] = SC_COMPILER_C_CHAR;        // OOo specific
/* 127 */   // FREE

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
/* ? */     // question really is not permitted in sheet name
/* @ */     t[64] |=   SC_COMPILER_C_WORD;
/* [ */     t[91] |=   SC_COMPILER_C_WORD;
/* ] */     t[93] |=   SC_COMPILER_C_WORD;
/* { */     t[123]|=   SC_COMPILER_C_WORD;
/* | */     t[124]|=   SC_COMPILER_C_WORD;
/* } */     t[125]|=   SC_COMPILER_C_WORD;
/* ~ */     t[126]|=   SC_COMPILER_C_WORD;
    }
}

static bool lcl_isValidQuotedText( const OUString& rFormula, sal_Int32 nSrcPos, ParseResult& rRes )
{
    // Tokens that start at ' can have anything in them until a final '
    // but '' marks an escaped '
    // We've earlier guaranteed that a string containing '' will be
    // surrounded by '
    if (nSrcPos < rFormula.getLength() && rFormula[nSrcPos] == '\'')
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
        const ScDocument* pDoc = nullptr,
        const uno::Sequence<sheet::ExternalLinkInfo>* pExternalLinks = nullptr )
{
    /* TODO: future versions will have to support sheet-local names too, thus
     * return a possible sheet name as well. */
    const sal_Unicode* const pStart = rSymbol.getStr();
    const sal_Unicode* p = pStart;
    sal_Int32 nLen = rSymbol.getLength();
    OUString aTmpFile, aTmpName;
    sal_Int32 i = 0;
    bool bInName = false;
    if (cSep == '!')
    {
        // For XL use existing parser that resolves bracketed and quoted and
        // indexed external document names.
        ScRange aRange;
        OUString aStartTabName, aEndTabName;
        sal_uInt16 nFlags = 0;
        p = aRange.Parse_XL_Header( p, pDoc, aTmpFile, aStartTabName,
                aEndTabName, nFlags, true, pExternalLinks );
        if (!p || p == pStart)
            return false;
        i = sal_Int32(p - pStart);
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
                // Move to the next char and loop until the second single
                // quote.
                sal_Unicode cPrev = c;
                ++i; ++p;
                for (sal_Int32 j = i; j < nLen; ++j, ++p)
                {
                    c = *p;
                    if (c == '\'')
                    {
                        if (j == i)
                        {
                            // empty quote e.g. (=''!Name)
                            return false;
                        }

                        if (cPrev == '\'')
                        {
                            // two consecutive quotes equal a single quote in
                            // the file name.
                            aTmpFile += OUString(c);
                            cPrev = 'a';
                        }
                        else
                            cPrev = c;

                        continue;
                    }

                    if (cPrev == '\'' && j != i)
                    {
                        // this is not a quote but the previous one is.  This
                        // ends the parsing of the quoted segment.  At this
                        // point, the current char must equal the separator
                        // char.

                        i = j;
                        bInName = true;
                        aTmpName += OUString(c); // Keep the separator as part of the name.
                        break;
                    }
                    aTmpFile += OUString(c);
                    cPrev = c;
                }

                if (!bInName)
                {
                    // premature ending of the quoted segment.
                    return false;
                }

                if (c != cSep)
                {
                    // only the separator is allowed after the closing quote.
                    return false;
                }

                continue;
            }
        }

        if (bInName)
        {
            if (c == cSep)
            {
                // A second separator ?  Not a valid external name.
                return false;
            }
            aTmpName += OUString(c);
        }
        else
        {
            if (c == cSep)
            {
                bInName = true;
                aTmpName += OUString(c); // Keep the separator as part of the name.
            }
            else
            {
                do
                {
                    if (rtl::isAsciiAlphanumeric(c))
                        // allowed.
                        break;

                    if (c > 128)
                        // non-ASCII character is allowed.
                        break;

                    bool bValid = false;
                    switch (c)
                    {
                        case '_':
                        case '-':
                        case '.':
                            // these special characters are allowed.
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
    }

    if (!bInName)
    {
        // No name found - most likely the symbol has no '!'s.
        return false;
    }

    sal_Int32 nNameLen = aTmpName.getLength();
    if (nNameLen < 2)
    {
        // Name must be at least 2-char long (separator plus name).
        return false;
    }

    if (aTmpName[0] != cSep)
    {
        // 1st char of the name must equal the separator.
        return false;
    }

    if (aTmpName[nNameLen-1] == '!')
    {
        // Check against #REF!.
        if (aTmpName == "#REF!")
            return false;
    }

    rFile = aTmpFile;
    rName = aTmpName.copy(1); // Skip the first char as it is always the separator.
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
    explicit Convention_A1( FormulaGrammar::AddressConvention eConv ) : ScCompiler::Convention( eConv ) { }
    static void MakeColStr( OUStringBuffer& rBuffer, SCCOL nCol );
    static void MakeRowStr( OUStringBuffer& rBuffer, SCROW nRow );

    ParseResult parseAnyToken( const OUString& rFormula,
                               sal_Int32 nSrcPos,
                               const CharClass* pCharClass) const override
    {
        ParseResult aRet;
        if ( lcl_isValidQuotedText(rFormula, nSrcPos, aRet) )
            return aRet;

        static const sal_Int32 nStartFlags = KParseTokens::ANY_LETTER_OR_NUMBER |
            KParseTokens::ASC_UNDERSCORE | KParseTokens::ASC_DOLLAR;
        static const sal_Int32 nContFlags = nStartFlags | KParseTokens::ASC_DOT;
        // '?' allowed in range names because of Xcl :-/
        static const char aAddAllowed[] = "?#";
        return pCharClass->parseAnyToken( rFormula,
                nSrcPos, nStartFlags, aAddAllowed, nContFlags, aAddAllowed );
    }

    virtual sal_uLong getCharTableFlags( sal_Unicode c, sal_Unicode /*cLast*/ ) const override
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
    explicit ConventionOOO_A1( FormulaGrammar::AddressConvention eConv ) : Convention_A1 (eConv) { }

    static void MakeTabStr( OUStringBuffer &rBuf, const std::vector<OUString>& rTabNames, SCTAB nTab )
    {
        if (static_cast<size_t>(nTab) >= rTabNames.size())
            rBuf.append(ScGlobal::GetRscString(STR_NO_REF_TABLE));
        else
            rBuf.append(rTabNames[nTab]);
        rBuf.append('.');
    }

    enum SingletonDisplay
    {
        SINGLETON_NONE,
        SINGLETON_COL,
        SINGLETON_ROW
    };

    static void MakeOneRefStrImpl(
        OUStringBuffer& rBuffer,
        const OUString& rErrRef, const std::vector<OUString>& rTabNames,
        const ScSingleRefData& rRef, const ScAddress& rAbsRef,
        bool bForceTab, bool bODF, SingletonDisplay eSingletonDisplay )
    {
        // For ODF override singleton so earlier releases still can read what
        // we write now as of 2015-06-26.
        /* TODO: we may want to change that in future in a few releases. */
        if (bODF)
            eSingletonDisplay = SINGLETON_NONE;

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
                if (!rRef.IsTabRel())
                    rBuffer.append('$');
                MakeTabStr(rBuffer, rTabNames, rAbsRef.Tab());
            }
        }
        else if (bODF)
            rBuffer.append('.');

        if (eSingletonDisplay != SINGLETON_ROW)
        {
            if (!rRef.IsColRel())
                rBuffer.append('$');
            if (!ValidCol(rAbsRef.Col()) || rRef.IsColDeleted())
                rBuffer.append(rErrRef);
            else
                MakeColStr(rBuffer, rAbsRef.Col());
        }

        if (eSingletonDisplay != SINGLETON_COL)
        {
            if (!rRef.IsRowRel())
                rBuffer.append('$');
            if (!ValidRow(rAbsRef.Row()) || rRef.IsRowDeleted())
                rBuffer.append(rErrRef);
            else
                MakeRowStr(rBuffer, rAbsRef.Row());
        }
    }

    static SingletonDisplay getSingletonDisplay( const ScAddress& rAbs1, const ScAddress& rAbs2,
            const ScComplexRefData& rRef, bool bFromRangeName )
    {
        // If any part is error, display as such.
        if (!ValidCol(rAbs1.Col()) || rRef.Ref1.IsColDeleted() || !ValidRow(rAbs1.Row()) || rRef.Ref1.IsRowDeleted() ||
            !ValidCol(rAbs2.Col()) || rRef.Ref2.IsColDeleted() || !ValidRow(rAbs2.Row()) || rRef.Ref2.IsRowDeleted())
            return SINGLETON_NONE;

        // A:A or $A:$A or A:$A or $A:A
        if (rRef.IsEntireCol())
            return SINGLETON_COL;

        // Same if not in named expression and both rows of entire columns are
        // relative references.
        if (!bFromRangeName && rAbs1.Row() == 0 && rAbs2.Row() == MAXROW &&
                rRef.Ref1.IsRowRel() && rRef.Ref2.IsRowRel())
            return SINGLETON_COL;

        // 1:1 or $1:$1 or 1:$1 or $1:1
        if (rRef.IsEntireRow())
            return SINGLETON_ROW;

        // Same if not in named expression and both columns of entire rows are
        // relative references.
        if (!bFromRangeName && rAbs1.Col() == 0 && rAbs2.Col() == MAXCOL &&
                rRef.Ref1.IsColRel() && rRef.Ref2.IsColRel())
            return SINGLETON_ROW;

        return SINGLETON_NONE;
    }

    virtual void makeRefStr( OUStringBuffer&   rBuffer,
                     formula::FormulaGrammar::Grammar /*eGram*/,
                     const ScAddress& rPos,
                     const OUString& rErrRef, const std::vector<OUString>& rTabNames,
                     const ScComplexRefData& rRef,
                     bool bSingleRef,
                     bool bFromRangeName ) const override
    {
        // In case absolute/relative positions weren't separately available:
        // transform relative to absolute!
        ScAddress aAbs1 = rRef.Ref1.toAbs(rPos), aAbs2;
        if( !bSingleRef )
            aAbs2 = rRef.Ref2.toAbs(rPos);

        SingletonDisplay eSingleton = bSingleRef ? SINGLETON_NONE :
            getSingletonDisplay( aAbs1, aAbs2, rRef, bFromRangeName);
        MakeOneRefStrImpl(rBuffer, rErrRef, rTabNames, rRef.Ref1, aAbs1, false, false, eSingleton);
        if (!bSingleRef)
        {
            rBuffer.append(':');
            MakeOneRefStrImpl(rBuffer, rErrRef, rTabNames, rRef.Ref2, aAbs2, aAbs1.Tab() != aAbs2.Tab(), false,
                    eSingleton);
        }
    }

    virtual sal_Unicode getSpecialSymbol( SpecialSymbolType eSymType ) const override
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
            const uno::Sequence<sheet::ExternalLinkInfo>* pExternalLinks ) const override
    {
        return lcl_parseExternalName(rSymbol, rFile, rName, '#', pDoc, pExternalLinks);
    }

    virtual OUString makeExternalNameStr( sal_uInt16 /*nFileId*/, const OUString& rFile,
            const OUString& rName ) const override
    {
        return lcl_makeExternalNameStr( rFile, rName, '#', false);
    }

    static bool makeExternalSingleRefStr(
        OUStringBuffer& rBuffer, const OUString& rFileName, const OUString& rTabName,
        const ScSingleRefData& rRef, const ScAddress& rPos, bool bDisplayTabName, bool bEncodeUrl )
    {
        ScAddress aAbsRef = rRef.toAbs(rPos);
        if (bDisplayTabName)
        {
            OUString aFile;
            if (bEncodeUrl)
                aFile = rFileName;
            else
                aFile = INetURLObject::decode(rFileName, INetURLObject::DECODE_UNAMBIGUOUS);

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

    static void makeExternalRefStrImpl(
        OUStringBuffer& rBuffer, const ScAddress& rPos, const OUString& rFileName,
        const OUString& rTabName, const ScSingleRefData& rRef, bool bODF )
    {
        if (bODF)
            rBuffer.append( '[');

        bool bEncodeUrl = bODF;
        makeExternalSingleRefStr(rBuffer, rFileName, rTabName, rRef, rPos, true, bEncodeUrl);
        if (bODF)
            rBuffer.append( ']');
    }

    virtual void makeExternalRefStr(
        OUStringBuffer& rBuffer, const ScAddress& rPos, sal_uInt16 /*nFileId*/, const OUString& rFileName,
        const OUString& rTabName, const ScSingleRefData& rRef ) const override
    {
        makeExternalRefStrImpl(rBuffer, rPos, rFileName, rTabName, rRef, false);
    }

    static void makeExternalRefStrImpl(
        OUStringBuffer& rBuffer, const ScAddress& rPos, const OUString& rFileName,
        const std::vector<OUString>& rTabNames, const OUString& rTabName,
        const ScComplexRefData& rRef, bool bODF )
    {
        ScRange aAbsRange = rRef.toAbs(rPos);

        if (bODF)
            rBuffer.append( '[');
        // Ensure that there's always a closing bracket, no premature returns.
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
                // Get the name of the last table.
                if (!lcl_getLastTabName(aLastTabName, rTabName, rTabNames, aAbsRange))
                {
                    OSL_FAIL( "ConventionOOO_A1::makeExternalRefStrImpl: sheet name not found");
                    // aLastTabName contains #REF!, proceed.
                }
            }
            else if (bODF)
                rBuffer.append( '.');      // need at least the sheet separator in ODF
            makeExternalSingleRefStr(
                rBuffer, rFileName, aLastTabName, rRef.Ref2, rPos, bDisplayTabName, bEncodeUrl);
        } while (false);

        if (bODF)
            rBuffer.append( ']');
    }

    virtual void makeExternalRefStr(
        OUStringBuffer& rBuffer, const ScAddress& rPos, sal_uInt16 /*nFileId*/, const OUString& rFileName,
        const std::vector<OUString>& rTabNames, const OUString& rTabName,
        const ScComplexRefData& rRef ) const override
    {
        makeExternalRefStrImpl(rBuffer, rPos, rFileName, rTabNames, rTabName, rRef, false);
    }
};

struct ConventionOOO_A1_ODF : public ConventionOOO_A1
{
    ConventionOOO_A1_ODF() : ConventionOOO_A1 (FormulaGrammar::CONV_ODF) { }

    virtual void makeRefStr( OUStringBuffer&   rBuffer,
                     formula::FormulaGrammar::Grammar eGram,
                     const ScAddress& rPos,
                     const OUString& rErrRef, const std::vector<OUString>& rTabNames,
                     const ScComplexRefData& rRef,
                     bool bSingleRef,
                     bool bFromRangeName ) const override
    {
        rBuffer.append('[');
        // In case absolute/relative positions weren't separately available:
        // transform relative to absolute!
        ScAddress aAbs1 = rRef.Ref1.toAbs(rPos), aAbs2;
        if( !bSingleRef )
            aAbs2 = rRef.Ref2.toAbs(rPos);

        if (FormulaGrammar::isODFF(eGram) && (rRef.Ref1.IsDeleted() || !ValidAddress(aAbs1) ||
                    (!bSingleRef && (rRef.Ref2.IsDeleted() || !ValidAddress(aAbs2)))))
        {
            rBuffer.append(rErrRef);
            // For ODFF write [#REF!], but not for PODF so apps reading ODF
            // 1.0/1.1 may have a better chance if they implemented the old
            // form.
        }
        else
        {
            SingletonDisplay eSingleton = bSingleRef ? SINGLETON_NONE :
                getSingletonDisplay( aAbs1, aAbs2, rRef, bFromRangeName);
            MakeOneRefStrImpl(rBuffer, rErrRef, rTabNames, rRef.Ref1, aAbs1, false, true, eSingleton);
            if (!bSingleRef)
            {
                rBuffer.append(':');
                MakeOneRefStrImpl(rBuffer, rErrRef, rTabNames, rRef.Ref2, aAbs2, aAbs1.Tab() != aAbs2.Tab(), true,
                        eSingleton);
            }
        }
        rBuffer.append(']');
    }

    virtual OUString makeExternalNameStr( sal_uInt16 /*nFileId*/, const OUString& rFile,
            const OUString& rName ) const override
    {
        return lcl_makeExternalNameStr( rFile, rName, '#', true);
    }

    virtual void makeExternalRefStr(
        OUStringBuffer& rBuffer, const ScAddress& rPos, sal_uInt16 /*nFileId*/, const OUString& rFileName,
        const OUString& rTabName, const ScSingleRefData& rRef ) const override
    {
        makeExternalRefStrImpl(rBuffer, rPos, rFileName, rTabName, rRef, true);
    }

    virtual void makeExternalRefStr(
        OUStringBuffer& rBuffer, const ScAddress& rPos, sal_uInt16 /*nFileId*/, const OUString& rFileName,
        const std::vector<OUString>& rTabNames,
        const OUString& rTabName, const ScComplexRefData& rRef ) const override
    {
        makeExternalRefStrImpl(rBuffer, rPos, rFileName, rTabNames, rTabName, rRef, true);
    }
};

struct ConventionXL
{
    virtual ~ConventionXL()
    {
    }

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
            const uno::Sequence<sheet::ExternalLinkInfo>* pExternalLinks )
    {
        return lcl_parseExternalName( rSymbol, rFile, rName, '!', pDoc, pExternalLinks);
    }

    static OUString makeExternalNameStr( const OUString& rFile, const OUString& rName )
    {
        return lcl_makeExternalNameStr( rFile, rName, '!', false);
    }

    static void makeExternalDocStr( OUStringBuffer& rBuffer, const OUString& rFullName, bool bEncodeUrl )
    {
        // Format that is easier to deal with inside OOo, because we use file
        // URL, and all characters are allowed.  Check if it makes sense to do
        // it the way Gnumeric does it.  Gnumeric doesn't use the URL form
        // and allows relative file path.
        //
        //   ['file:///path/to/source/filename.xls']

        rBuffer.append('[');
        rBuffer.append('\'');
        OUString aFullName;
        if (bEncodeUrl)
            aFullName = rFullName;
        else
            aFullName = INetURLObject::decode(rFullName, INetURLObject::DECODE_UNAMBIGUOUS);

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

    virtual void parseExternalDocName( const OUString& rFormula, sal_Int32& rSrcPos ) const
    {
        sal_Int32 nLen = rFormula.getLength();
        const sal_Unicode* p = rFormula.getStr();
        sal_Unicode cPrev = 0;
        for (sal_Int32 i = rSrcPos; i < nLen; ++i)
        {
            sal_Unicode c = p[i];
            if (i == rSrcPos)
            {
                // first character must be '['.
                if (c != '[')
                    return;
            }
            else if (i == rSrcPos + 1)
            {
                // second character must be a single quote.
                if (c != '\'')
                    return;
            }
            else if (c == '\'')
            {
                if (cPrev == '\'')
                    // two successive single quote is treated as a single
                    // valid character.
                    c = 'a';
            }
            else if (c == ']')
            {
                if (cPrev == '\'')
                {
                    // valid source document path found.  Increment the
                    // current position to skip the source path.
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
                // any other character
                if (i > rSrcPos + 2 && cPrev == '\'')
                    // unless it's the 3rd character, a normal character
                    // following immediately a single quote is invalid.
                    return;
            }
            cPrev = c;
        }
    }
};

struct ConventionXL_A1 : public Convention_A1, public ConventionXL
{
    ConventionXL_A1() : Convention_A1( FormulaGrammar::CONV_XL_A1 ) { }
    explicit ConventionXL_A1( FormulaGrammar::AddressConvention eConv ) : Convention_A1( eConv ) { }

    static void makeSingleCellStr( OUStringBuffer& rBuf, const ScSingleRefData& rRef, const ScAddress& rAbs )
    {
        if (!rRef.IsColRel())
            rBuf.append('$');
        MakeColStr(rBuf, rAbs.Col());
        if (!rRef.IsRowRel())
            rBuf.append('$');
        MakeRowStr(rBuf, rAbs.Row());
    }

    virtual void makeRefStr( OUStringBuffer&   rBuf,
                     formula::FormulaGrammar::Grammar /*eGram*/,
                     const ScAddress& rPos,
                     const OUString& /*rErrRef*/, const std::vector<OUString>& rTabNames,
                     const ScComplexRefData& rRef,
                     bool bSingleRef,
                     bool /*bFromRangeName*/ ) const override
    {
        ScComplexRefData aRef( rRef );

        // Play fast and loose with invalid refs.  There is not much point in producing
        // Foo!A1:#REF! versus #REF! at this point
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
                                       const CharClass* pCharClass) const override
    {
        parseExternalDocName(rFormula, nSrcPos);

        ParseResult aRet;
        if ( lcl_isValidQuotedText(rFormula, nSrcPos, aRet) )
            return aRet;

        static const sal_Int32 nStartFlags = KParseTokens::ANY_LETTER_OR_NUMBER |
            KParseTokens::ASC_UNDERSCORE | KParseTokens::ASC_DOLLAR;
        static const sal_Int32 nContFlags = nStartFlags | KParseTokens::ASC_DOT;
        // '?' allowed in range names
        const OUString aAddAllowed("?!");
        return pCharClass->parseAnyToken( rFormula,
                nSrcPos, nStartFlags, aAddAllowed, nContFlags, aAddAllowed );
    }

    virtual sal_Unicode getSpecialSymbol( SpecialSymbolType eSymType ) const override
    {
        return ConventionXL::getSpecialSymbol(eSymType);
    }

    virtual bool parseExternalName( const OUString& rSymbol, OUString& rFile, OUString& rName,
            const ScDocument* pDoc,
            const uno::Sequence<sheet::ExternalLinkInfo>* pExternalLinks ) const override
    {
        return ConventionXL::parseExternalName( rSymbol, rFile, rName, pDoc, pExternalLinks);
    }

    virtual OUString makeExternalNameStr( sal_uInt16 /*nFileId*/, const OUString& rFile,
            const OUString& rName ) const override
    {
        return ConventionXL::makeExternalNameStr(rFile, rName);
    }

    virtual void makeExternalRefStr(
        OUStringBuffer& rBuffer, const ScAddress& rPos, sal_uInt16 /*nFileId*/, const OUString& rFileName,
        const OUString& rTabName, const ScSingleRefData& rRef ) const override
    {
        // ['file:///path/to/file/filename.xls']'Sheet Name'!$A$1
        // This is a little different from the format Excel uses, as Excel
        // puts [] only around the file name.  But we need to enclose the
        // whole file path with [] because the file name can contain any
        // characters.

        ConventionXL::makeExternalDocStr(rBuffer, rFileName, false);
        ScRangeStringConverter::AppendTableName(rBuffer, rTabName);
        rBuffer.append('!');

        makeSingleCellStr(rBuffer, rRef, rRef.toAbs(rPos));
    }

    virtual void makeExternalRefStr(
        OUStringBuffer& rBuffer, const ScAddress& rPos, sal_uInt16 /*nFileId*/, const OUString& rFileName,
        const std::vector<OUString>& rTabNames, const OUString& rTabName,
        const ScComplexRefData& rRef ) const override
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

    virtual void makeRefStr( OUStringBuffer&   rBuf,
                     formula::FormulaGrammar::Grammar eGram,
                     const ScAddress& rPos,
                     const OUString& rErrRef, const std::vector<OUString>& rTabNames,
                     const ScComplexRefData& rRef,
                     bool bSingleRef,
                     bool bFromRangeName ) const override
    {
        // In OOXML relative references in named expressions are relative to
        // column 0 and row 0. Relative sheet references don't exist.
        ScAddress aPos( rPos );
        if (bFromRangeName)
        {
            // XXX NOTE: by decrementing the reference position we may end up
            // with resolved references with negative values. There's no proper
            // way to solve that or wrap them around without sheet dimensions
            // that are stored along. That, or blindly assume fixed dimensions
            // here and in import.
            /* TODO: maybe do that blind fixed dimensions wrap? */
            aPos.SetCol(0);
            aPos.SetRow(0);
        }

        if (rRef.Ref1.IsDeleted() || (!bSingleRef && rRef.Ref2.IsDeleted()))
        {
            // For OOXML write plain "#REF!" instead of detailed sheet/col/row
            // information.
            rBuf.append(rErrRef);
            return;
        }

        ConventionXL_A1::makeRefStr( rBuf, eGram, aPos, rErrRef, rTabNames, rRef, bSingleRef, bFromRangeName);
    }

    virtual OUString makeExternalNameStr( sal_uInt16 nFileId, const OUString& /*rFile*/,
            const OUString& rName ) const override
    {
        // [N]!DefinedName is a workbook global name.
        return OUString( "[" + OUString::number(nFileId+1) + "]!" + rName );

        /* TODO: add support for sheet local names, would be
         * [N]'Sheet Name'!DefinedName
         * Similar to makeExternalRefStr() but with DefinedName instead of
         * CellStr. */
    }

    virtual void parseExternalDocName(const OUString& rFormula, sal_Int32& rSrcPos) const override
    {
        sal_Int32 nLen = rFormula.getLength();
        const sal_Unicode* p = rFormula.getStr();
        for (sal_Int32 i = rSrcPos; i < nLen; ++i)
        {
            sal_Unicode c = p[i];
            if (i == rSrcPos)
            {
                // first character must be '['.
                if (c != '[')
                    return;
            }
            else if (c == ']')
            {
                rSrcPos = i + 1;
                return;
            }
        }
    }

    virtual void makeExternalRefStr(
        OUStringBuffer& rBuffer, const ScAddress& rPos, sal_uInt16 nFileId, const OUString& /*rFileName*/,
        const OUString& rTabName, const ScSingleRefData& rRef ) const override
    {
        // [N]'Sheet Name'!$A$1
        // Where N is a 1-based positive integer number of a file name in OOXML
        // xl/externalLinks/externalLinkN.xml

        ConventionXL_OOX::makeExternalDocStr(rBuffer, nFileId);
        ScRangeStringConverter::AppendTableName(rBuffer, rTabName);
        rBuffer.append('!');

        makeSingleCellStr(rBuffer, rRef, rRef.toAbs(rPos));
    }

    virtual void makeExternalRefStr(
        OUStringBuffer& rBuffer, const ScAddress& rPos, sal_uInt16 nFileId, const OUString& /*rFileName*/,
        const std::vector<OUString>& rTabNames, const OUString& rTabName,
        const ScComplexRefData& rRef ) const override
    {
        ScRange aAbsRef = rRef.toAbs(rPos);

        ConventionXL_OOX::makeExternalDocStr(rBuffer, nFileId);
        ConventionXL::makeExternalTabNameRange(rBuffer, rTabName, rTabNames, aAbsRef);
        rBuffer.append('!');

        makeSingleCellStr(rBuffer, rRef.Ref1, aAbsRef.aStart);
        if (aAbsRef.aStart != aAbsRef.aEnd)
        {
            rBuffer.append(':');
            makeSingleCellStr(rBuffer, rRef.Ref2, aAbsRef.aEnd);
        }
    }

    static void makeExternalDocStr( OUStringBuffer& rBuffer, sal_uInt16 nFileId )
    {
        rBuffer.append('[').append( OUString::number( nFileId+1)).append(']');
    }
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

    virtual void makeRefStr( OUStringBuffer&   rBuf,
                     formula::FormulaGrammar::Grammar /*eGram*/,
                     const ScAddress& rPos,
                     const OUString& /*rErrRef*/, const std::vector<OUString>& rTabNames,
                     const ScComplexRefData& rRef,
                     bool bSingleRef,
                     bool /*bFromRangeName*/ ) const override
    {
        ScRange aAbsRef = rRef.toAbs(rPos);
        ScComplexRefData aRef( rRef );

        MakeTabStr(rBuf, rPos, rTabNames, aRef, bSingleRef);

        // Play fast and loose with invalid refs.  There is not much point in producing
        // Foo!A1:#REF! versus #REF! at this point
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
                               const CharClass* pCharClass) const override
    {
        parseExternalDocName(rFormula, nSrcPos);

        ParseResult aRet;
        if ( lcl_isValidQuotedText(rFormula, nSrcPos, aRet) )
            return aRet;

        static const sal_Int32 nStartFlags = KParseTokens::ANY_LETTER_OR_NUMBER |
            KParseTokens::ASC_UNDERSCORE ;
        static const sal_Int32 nContFlags = nStartFlags | KParseTokens::ASC_DOT;
        // '?' allowed in range names
        const OUString aAddAllowed("?-[]!");

        return pCharClass->parseAnyToken( rFormula,
                nSrcPos, nStartFlags, aAddAllowed, nContFlags, aAddAllowed );
    }

    virtual sal_Unicode getSpecialSymbol( SpecialSymbolType eSymType ) const override
    {
        return ConventionXL::getSpecialSymbol(eSymType);
    }

    virtual bool parseExternalName( const OUString& rSymbol, OUString& rFile, OUString& rName,
            const ScDocument* pDoc,
            const uno::Sequence<sheet::ExternalLinkInfo>* pExternalLinks ) const override
    {
        return ConventionXL::parseExternalName( rSymbol, rFile, rName, pDoc, pExternalLinks);
    }

    virtual OUString makeExternalNameStr( sal_uInt16 /*nFileId*/, const OUString& rFile,
            const OUString& rName ) const override
    {
        return ConventionXL::makeExternalNameStr(rFile, rName);
    }

    virtual void makeExternalRefStr(
        OUStringBuffer& rBuffer, const ScAddress& rPos, sal_uInt16 /*nFileId*/, const OUString& rFileName,
        const OUString& rTabName, const ScSingleRefData& rRef ) const override
    {
        // ['file:///path/to/file/filename.xls']'Sheet Name'!$A$1
        // This is a little different from the format Excel uses, as Excel
        // puts [] only around the file name.  But we need to enclose the
        // whole file path with [] because the file name can contain any
        // characters.

        ScAddress aAbsRef = rRef.toAbs(rPos);
        ConventionXL::makeExternalDocStr(rBuffer, rFileName, false);
        ScRangeStringConverter::AppendTableName(rBuffer, rTabName);
        rBuffer.append('!');

        r1c1_add_row(rBuffer, rRef, aAbsRef);
        r1c1_add_col(rBuffer, rRef, aAbsRef);
    }

    virtual void makeExternalRefStr(
        OUStringBuffer& rBuffer, const ScAddress& rPos, sal_uInt16 /*nFileId*/, const OUString& rFileName,
        const std::vector<OUString>& rTabNames, const OUString& rTabName,
        const ScComplexRefData& rRef ) const override
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

    virtual sal_uLong getCharTableFlags( sal_Unicode c, sal_Unicode cLast ) const override
    {
        sal_uLong nFlags = mpCharTable[static_cast<sal_uInt8>(c)];
        if (c == '-' && cLast == '[')
            // '-' can occur within a reference string only after '[' e.g. R[-1]C.
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
        nSrcPos(0),
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
    mpFormatter(pDoc ? pDoc->GetFormatTable() : nullptr),
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
        mpFormatter(pDoc ? pDoc->GetFormatTable() : nullptr),
        nSrcPos(0),
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
                // escape embedded quotes
                rString = rString.replaceAll( one_quote, two_quote );
            }
            break;
    }

    if ( !bNeedsQuote && CharClass::isAsciiNumeric( rString ) )
    {
        // Prevent any possible confusion resulting from pure numeric sheet names.
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
    // it must be 'Doc'#
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

    return nullptr;
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

// NextSymbol

// Parses the formula into separate symbols for further
// processing (Turing-Machine).

// initial state = GetChar

// old state     | read character    | action                | new state
//---------------+-------------------+-----------------------+---------------
// GetChar       | ;()+-*/^=&        | Symbol=char           | Stop
//               | <>                | Symbol=char           | GetBool
//               | $ letter          | Symbol=char           | GetWord
//               | number            | Symbol=char           | GetValue
//               | "                 | none                  | GetString
//               | other             | none                  | GetChar
//---------------+-------------------+-----------------------+---------------
// GetBool       | =>                | Symbol=Symbol+char    | Stop
//               | other             | Dec(CharPos)          | Stop
//---------------+-------------------+-----------------------+---------------
// GetWord       | SepSymbol         | Dec(CharPos)          | Stop
//               | ()+-*/^=<>&~      |                       |
//               | space             | Dec(CharPos)          | Stop
//               | $_:.              |                       |
//               | letter, number    | Symbol=Symbol+char    | GetWord
//               | other             | error                 | Stop
//---------------+-------------------+-----------------------+---------------
// GetValue      | ;()*/^=<>&        |                       |
//               | space             | Dec(CharPos)          | Stop
//               | number E+-%,.     | Symbol=Symbol+char    | GetValue
//               | other             | error                 | Stop
//---------------+-------------------+-----------------------+---------------
// GetString     | "                 | none                  | Stop
//               | other             | Symbol=Symbol+char    | GetString
//---------------+-------------------+-----------------------+---------------

sal_Int32 ScCompiler::NextSymbol(bool bInArray)
{
    cSymbol[MAXSTRLEN-1] = 0;       // end
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

    // special symbols specific to address convention used
    sal_Unicode cSheetPrefix = pConv->getSpecialSymbol(ScCompiler::Convention::ABS_SHEET_PREFIX);
    sal_Unicode cSheetSep    = pConv->getSpecialSymbol(ScCompiler::Convention::SHEET_SEPARATOR);

    int nDecSeps = 0;
    bool bAutoIntersection = false;
    int nRefInName = 0;
    bool bErrorConstantHadSlash = false;
    mnPredetectedReference = 0;
    // try to parse simple tokens before calling i18n parser
    while ((c != 0) && (eState != ssStop) )
    {
        pSrc++;
        sal_uLong nMask = GetCharTableFlags( c, cLast );

        // The parameter separator and the array column and row separators end
        // things unconditionally if not in string or reference.
        if (c == cSep || (bInArray && (c == cArrayColSep || c == cArrayRowSep)))
        {
            switch (eState)
            {
                // these are to be continued
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
                // Order is important!
                if (eLastOp == ocTableRefOpen && c != '[' && c != '#' && c != ']')
                {
                    *pSym++ = c;
                    eState = ssGetTableRefColumn;
                }
                else if( nMask & SC_COMPILER_C_ODF_LABEL_OP )
                {
                    // '!!' automatic intersection
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
                            nSpaces += 2;   // must match the character count
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
                    // '$$' defined name marker
                    if (GetCharTableFlags( pSrc[0], 0 ) & SC_COMPILER_C_ODF_NAME_MARKER)
                    {
                        // both eaten, not added to pSym
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
                    // '[' is a special case in OOXML, it can start an external
                    // reference ID like [1]Sheet1!A1 that needs to be scanned
                    // entirely, or can be ocTableRefOpen, of which the first
                    // transforms an ocDBArea into an ocTableRef.
                    if (c == '[' && FormulaGrammar::isOOXML( meGrammar) && eLastOp != ocDBArea && maTableRefs.empty())
                    {
                        nMask &= ~SC_COMPILER_C_CHAR;
                        goto Label_MaskStateMachine;
                    }
                    else
                    {
                        *pSym++ = c;
                        eState = ssStop;
                    }
                }
                else if( nMask & SC_COMPILER_C_ODF_LBRACKET )
                {
                    // eaten, not added to pSym
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
                    if (!maTableRefs.empty() && maTableRefs.back().mnLevel == 2)
                        eState = ssGetTableRefItem;
                    else
                        eState = ssGetErrorConstant;
                }
                else if( nMask & SC_COMPILER_C_CHAR_DONTCARE )
                {
                    nSpaces++;
                }
                else if( nMask & SC_COMPILER_C_CHAR_IDENT )
                {   // try to get a simple ASCII identifier before calling
                    // i18n, to gain performance during import
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
                {   // This catches also $Sheet1.A$1, for example.
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
                    // One range operator may form Sheet1.A:A, which we need to
                    // pass as one entity to IsReference().
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
                {   // High values need reparsing with i18n,
                    // single quoted $'sheet' names too (otherwise we'd had to
                    // implement everything twice).
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
                        // reparse with i18n, may be numeric sheet name as well
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
                        // reparse with i18n
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
                    // reparse with i18n
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
                            bQuote = true;      // "" => literal "
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
                    // ODFF Error ::= '#' [A-Z0-9]+ ([!?] | ('/' ([A-Z] | ([0-9] [!?]))))
                    // BUT, in UI these may have been translated! So don't
                    // check for ASCII alnum. Note that this construct can't be
                    // parsed with i18n.
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
            case ssGetTableRefItem:
                {
                    // Scan whatever up to the next ']' closer.
                    if (c != ']')
                    {
                        if( pSym == &cSymbol[ MAXSTRLEN-1 ] )
                        {
                            SetError( errStringOverflow);
                            eState = ssStop;
                        }
                        else
                            *pSym++ = c;
                    }
                    else
                    {
                        --pSrc;
                        eState = ssStop;
                    }
                }
                break;
            case ssGetTableRefColumn:
                {
                    // Scan whatever up to the next unescaped ']' closer.
                    if (c != ']' || cLast == '\'')
                    {
                        if( pSym == &cSymbol[ MAXSTRLEN-1 ] )
                        {
                            SetError( errStringOverflow);
                            eState = ssStop;
                        }
                        else
                            *pSym++ = c;
                    }
                    else
                    {
                        --pSrc;
                        eState = ssStop;
                    }
                }
                break;
            case ssGetReference:
                if( pSym == &cSymbol[ MAXSTRLEN-1 ] )
                {
                    SetError( errStringOverflow);
                    eState = ssSkipReference;
                }
                SAL_FALLTHROUGH;
            case ssSkipReference:
                // ODF reference: ['External'#$'Sheet'.A1:.B2] with dots being
                // mandatory also if no sheet name. 'External'# is optional,
                // sheet name is optional, quotes around sheet name are
                // optional if no quote contained. [#REF!] is valid.
                // 2nd usage: ['Sheet'.$$'DefinedName']
                // 3rd usage: ['External'#$$'DefinedName']
                // 4th usage: ['External'#$'Sheet'.$$'DefinedName']
                // Also for all these names quotes are optional if no quote
                // contained.
                {

                    // nRefInName: 0 := not in sheet name yet. 'External'
                    // is parsed as if it was a sheet name and nRefInName
                    // is reset when # is encountered immediately after closing
                    // quote. Same with 'DefinedName', nRefInName is cleared
                    // when : is encountered.

                    // Encountered leading $ before sheet name.
                    static const int kDollar    = (1 << 1);
                    // Encountered ' opening quote, which may be after $ or
                    // not.
                    static const int kOpen      = (1 << 2);
                    // Somewhere in name.
                    static const int kName      = (1 << 3);
                    // Encountered ' in name, will be cleared if double or
                    // transformed to kClose if not, in which case kOpen is
                    // cleared.
                    static const int kQuote     = (1 << 4);
                    // Past ' closing quote.
                    static const int kClose     = (1 << 5);
                    // Encountered # file/sheet separator.
                    static const int kFileSep   = (1 << 6);
                    // Past . sheet name separator.
                    static const int kPast      = (1 << 7);
                    // Marked name $$ follows sheet name separator, detected
                    // while we're still on the separator. Will be cleared when
                    // entering the name.
                    static const int kMarkAhead = (1 << 8);
                    // In marked defined name.
                    static const int kDefName   = (1 << 9);
                    // Encountered # of #REF!
                    static const int kRefErr    = (1 << 10);

                    bool bAddToSymbol = true;
                    if ((nMask & SC_COMPILER_C_ODF_RBRACKET) && !(nRefInName & kOpen))
                    {
                        OSL_ENSURE( nRefInName & (kPast | kDefName | kRefErr),
                                "ScCompiler::NextSymbol: reference: "
                                "closing bracket ']' without prior sheet name separator '.' violates ODF spec");
                        // eaten, not added to pSym
                        bAddToSymbol = false;
                        eState = ssStop;
                    }
                    else if (cSheetSep == c && nRefInName == 0)
                    {
                        // eat it, no sheet name [.A1]
                        bAddToSymbol = false;
                        nRefInName |= kPast;
                        if ('$' == pSrc[0] && '$' == pSrc[1])
                            nRefInName |= kMarkAhead;
                    }
                    else if (!(nRefInName & kPast) || (nRefInName & (kMarkAhead | kDefName)))
                    {
                        // Not in col/row yet.

                        if (SC_COMPILER_FILE_TAB_SEP == c && (nRefInName & kFileSep))
                            nRefInName = 0;
                        else if ('$' == c && '$' == pSrc[0] && !(nRefInName & kOpen))
                        {
                            nRefInName &= ~kMarkAhead;
                            if (!(nRefInName & kDefName))
                            {
                                // eaten, not added to pSym (2 chars)
                                bAddToSymbol = false;
                                ++pSrc;
                                nRefInName &= kPast;
                                nRefInName |= kDefName;
                            }
                            else
                            {
                                // ScAddress::Parse() will recognize this as
                                // invalid later.
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
                            // TODO: The conventions' parseExternalName()
                            // should handle quoted names, but as long as they
                            // don't remove non-embedded quotes here.
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
                                // escaped embedded quote
                                nRefInName &= ~kQuote;
                            }
                            else
                            {
                                switch (pSrc[0])
                                {
                                    case '\'':
                                        // escapes embedded quote
                                        nRefInName |= kQuote;
                                        break;
                                    case SC_COMPILER_FILE_TAB_SEP:
                                        // sheet name should follow
                                        nRefInName |= kFileSep;
                                        SAL_FALLTHROUGH;
                                    default:
                                        // quote not followed by quote => close
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
                            // unquoted sheet name separator
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
                            // start unquoted name
                            nRefInName |= kName;
                        }
                    }
                    else if (':' == c)
                    {
                        // range operator
                        nRefInName = 0;
                        ++mnPredetectedReference;
                    }
                    if (bAddToSymbol && eState != ssSkipReference)
                        *pSym++ = c;    // everything is part of reference
                }
                break;
            case ssStop:
                ;   // nothing, prevent warning
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
            // special case  (e.g. $'sheetname' in OOO A1)
            if ( pStart[nSrcPos] == cSheetPrefix && pStart[nSrcPos+1] == '\'' )
                aSymbol.append(pStart[nSrcPos++]);

            ParseResult aRes = pConv->parseAnyToken( aFormula, nSrcPos, pCharClass );

            if ( !aRes.TokenType )
                SetError( nErr = errIllegalChar );      // parsed chars as string
            if ( aRes.EndPos <= nSrcPos )
            {   // ?!?
                SetError( nErr = errIllegalChar );
                nSrcPos = aFormula.getLength();
                aSymbol.truncate();
            }
            else
            {
                // When having parsed a second reference part, ensure that the
                // i18n parser did not mistakingly parse a number that included
                // a separator which happened to be meant as a parameter
                // separator instead.
                if (mnRangeOpPosInSymbol >= 0 && (aRes.TokenType & KParseType::ASC_NUMBER))
                {
                    for (sal_Int32 i = nSrcPos; i < aRes.EndPos; ++i)
                    {
                        if (pStart[i] == cSep)
                            aRes.EndPos = i;    // also ends for
                    }
                }
                aSymbol.append( pStart + nSrcPos, aRes.EndPos - nSrcPos);
                nSrcPos = aRes.EndPos;
                c = pStart[nSrcPos];
                if ( aRes.TokenType & KParseType::SINGLE_QUOTE_NAME )
                {   // special cases (e.g. 'sheetname'. or 'filename'# in OOO A1)
                    bi18n = (c == cSheetSep || c == SC_COMPILER_FILE_TAB_SEP);
                }
                // One range operator restarts parsing for second reference.
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
        // This is a trailing range operator, which is nonsense. Will be caught
        // in next round.
        mnRangeOpPosInSymbol = -1;
        *--pSym = 0;
        --nSrcPos;
    }
    if ( bAutoCorrect )
        aCorrectedSymbol = cSymbol;
    if (bAutoIntersection && nSpaces > 1)
        --nSpaces;  // replace '!!' with only one space
    return nSpaces;
}

// Convert symbol to token

bool ScCompiler::IsOpCode( const OUString& rName, bool bInArray )
{
    OpCodeHashMap::const_iterator iLook( mxSymbols->getHashMap()->find( rName));
    bool bFound = (iLook != mxSymbols->getHashMap()->end());
    if (bFound)
    {
        OpCode eOp = iLook->second;
        if (bInArray)
        {
            if (rName.equals(mxSymbols->getSymbol(ocArrayColSep)))
                eOp = ocArrayColSep;
            else if (rName.equals(mxSymbols->getSymbol(ocArrayRowSep)))
                eOp = ocArrayRowSep;
        }
        else if (eOp == ocCeil && mxSymbols->isOOXML())
        {
            // Ensure that _xlfn.CEILING.MATH maps to ocCeil_Math. ocCeil is
            // unassigned for import.
            eOp = ocCeil_Math;
        }
        else if (eOp == ocFloor && mxSymbols->isOOXML())
        {
            // Ensure that _xlfn.FLOOR.MATH maps to ocFloor_Math. ocFloor is
            // unassigned for import.
            eOp = ocFloor_Math;
        }
        maRawToken.SetOpCode(eOp);
    }
    else if (mxSymbols->isODFF())
    {
        // ODFF names that are not written in the current mapping but to be
        // recognized. New names will be written in a future relase, then
        // exchange (!) with the names in
        // formula/source/core/resource/core_resource.src to be able to still
        // read the old names as well.
        struct FunctionName
        {
            const sal_Char* pName;
            OpCode          eOp;
        };
        static const FunctionName aOdffAliases[] = {
            // Renamed old names, still accept them:
            { "B",              ocB },              // B -> BINOM.DIST.RANGE
            { "TDIST",          ocTDist },          // TDIST -> LEGACY.TDIST
            { "EASTERSUNDAY",   ocEasterSunday },   // EASTERSUNDAY -> ORG.OPENOFFICE.EASTERSUNDAY
            { "ZGZ",            ocRRI },            // ZGZ -> RRI
            { "COLOR",          ocColor },          // COLOR -> ORG.LIBREOFFICE.COLOR
            { "GOALSEEK",       ocBackSolver },     // GOALSEEK -> ORG.OPENOFFICE.GOALSEEK
            { "COM.MICROSOFT.F.DIST", ocFDist_LT }, // fdo#40835, -> FDIST -> COM.MICROSOFT.F.DIST
            { "COM.MICROSOFT.F.INV",  ocFInv_LT }   // tdf#94214, COM.MICROSOFT.F.INV -> FINV (ODF)
            // Renamed new names, prepare to read future names:
            //{ "ORG.OPENOFFICE.XXX", ocXXX }         // XXX -> ORG.OPENOFFICE.XXX
        };
        static const size_t nOdffAliases = sizeof(aOdffAliases) / sizeof(aOdffAliases[0]);
        for (size_t i=0; i<nOdffAliases; ++i)
        {
            if (rName.equalsIgnoreAsciiCaseAscii( aOdffAliases[i].pName))
            {
                maRawToken.SetOpCode( aOdffAliases[i].eOp);
                bFound = true;
                break;  // for
            }
        }
    }
    else if (mxSymbols->isOOXML())
    {
        // OOXML names that are not written in the current mapping but to be
        // recognized as old versions wrote them.
        struct FunctionName
        {
            const sal_Char* pName;
            OpCode          eOp;
        };
        static const FunctionName aOoxmlAliases[] = {
            { "EFFECTIVE",  ocEffective }   // EFFECTIVE -> EFFECT
        };
        for (const FunctionName& rOoxmlAlias : aOoxmlAliases)
        {
            if (rName.equalsIgnoreAsciiCaseAscii( rOoxmlAlias.pName))
            {
                maRawToken.SetOpCode( rOoxmlAlias.eOp);
                bFound = true;
                break;  // for
            }
        }
    }
    if (!bFound)
    {
        OUString aIntName;
        if (mxSymbols->hasExternals())
        {
            // If symbols are set by filters get mapping to exact name.
            ExternalHashMap::const_iterator iExt(
                    mxSymbols->getExternalHashMap()->find( rName));
            if (iExt != mxSymbols->getExternalHashMap()->end())
            {
                if (ScGlobal::GetAddInCollection()->GetFuncData( (*iExt).second))
                    aIntName = (*iExt).second;
            }
            if (aIntName.isEmpty())
            {
                // If that isn't found we might continue with rName lookup as a
                // last resort by just falling through to FindFunction(), but
                // it shouldn't happen if the map was setup correctly. Don't
                // waste time and bail out.
                return false;
            }
        }
        if (aIntName.isEmpty())
        {
            // Old (deprecated) addins first for legacy.
            if (ScGlobal::GetLegacyFuncCollection()->findByName(cSymbol))
            {
                maRawToken.SetExternal( cSymbol );
            }
            else
                // bLocalFirst=false for (English) upper full original name
                // (service.function)
                aIntName = ScGlobal::GetAddInCollection()->FindFunction(
                        rName, !mxSymbols->isEnglish());
        }
        if (!aIntName.isEmpty())
        {
            maRawToken.SetExternal( aIntName.getStr() );     // international name
            bFound = true;
        }
    }
    OpCode eOp;
    if (bFound && ((eOp = maRawToken.GetOpCode()) == ocSub || eOp == ocNegSub))
    {
        bool bShouldBeNegSub =
            (eLastOp == ocOpen || eLastOp == ocSep || eLastOp == ocNegSub ||
             (SC_OPCODE_START_BIN_OP <= eLastOp && eLastOp < SC_OPCODE_STOP_BIN_OP) ||
             eLastOp == ocArrayOpen ||
             eLastOp == ocArrayColSep || eLastOp == ocArrayRowSep);
        if (bShouldBeNegSub && eOp == ocSub)
            maRawToken.NewOpCode( ocNegSub );
            //TODO: if ocNegSub had ForceArray we'd have to set it here
        else if (!bShouldBeNegSub && eOp == ocNegSub)
            maRawToken.NewOpCode( ocSub );
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
        maRawToken.SetOpCode( (OpCode) --i );
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

    // Don't accept 3:3 as time, it is a reference to entire row 3 instead.
    // Dates should never be entered directly and automatically converted
    // to serial, because the serial would be wrong if null-date changed.
    // Usually it wouldn't be accepted anyway because the date separator
    // clashed with other separators or operators.
    if (nType & (css::util::NumberFormat::TIME | css::util::NumberFormat::DATE))
        return false;

    if (nType == css::util::NumberFormat::LOGICAL)
    {
        const sal_Unicode* p = aFormula.getStr() + nSrcPos;
        while( *p == ' ' )
            p++;
        if (*p == '(')
            return false;   // Boolean function instead.
    }

    if( nType == css::util::NumberFormat::TEXT )
        // HACK: number too big!
        SetError( errIllegalArgument );
    maRawToken.SetDouble( fVal );
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
        const sal_Unicode* pStr = cSymbol+1;
        svl::SharedString aSS = pDoc->GetSharedStringPool().intern(OUString(pStr));
        maRawToken.SetString(aSS.getData(), aSS.getDataIgnoreCase());
        return true;
    }
    return false;
}

bool ScCompiler::IsPredetectedReference(const OUString& rName)
{
    // Speedup documents with lots of broken references, e.g. sheet deleted.
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
            // Per ODFF the correct string for a reference error is just #REF!,
            // so pass it on.
            if (rName.getLength() == 5)
                return IsErrorConstant( rName);
            return false;           // #REF!.AB42 or #REF!42 or #REF!#REF!
        }
        sal_Unicode c = rName[nPos-1];      // before #REF!
        if ('$' == c)
        {
            if (nPos == 1)
                return false;       // $#REF!.AB42 or $#REF!42 or $#REF!#REF!
            c = rName[nPos-2];              // before $#REF!
        }
        sal_Unicode c2 = nPos+5 < rName.getLength() ? rName[nPos+5] : 0;     // after #REF!
        switch (c)
        {
            case '.':
                if ('$' == c2 || '#' == c2 || ('0' <= c2 && c2 <= '9'))
                    return false;   // sheet.#REF!42 or sheet.#REF!#REF!
                break;
            case ':':
                if (mnPredetectedReference > 1 &&
                        ('.' == c2 || '$' == c2 || '#' == c2 ||
                         ('0' <= c2 && c2 <= '9')))
                    return false;   // :#REF!.AB42 or :#REF!42 or :#REF!#REF!
                break;
            default:
                if (rtl::isAsciiAlpha(c) &&
                        ((mnPredetectedReference > 1 && ':' == c2) || 0 == c2))
                    return false;   // AB#REF!: or AB#REF!
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
        ScComplexRefData aRef;
        aRef.InitRange( aRange );
        aRef.Ref1.SetColRel( (nFlags & SCA_COL_ABSOLUTE) == 0 );
        aRef.Ref1.SetRowRel( (nFlags & SCA_ROW_ABSOLUTE) == 0 );
        aRef.Ref1.SetTabRel( (nFlags & SCA_TAB_ABSOLUTE) == 0 );
        if ( !(nFlags & SCA_VALID_TAB) )
            aRef.Ref1.SetTabDeleted( true );        // #REF!
        aRef.Ref1.SetFlag3D( ( nFlags & SCA_TAB_3D ) != 0 );
        aRef.Ref2.SetColRel( (nFlags & SCA_COL2_ABSOLUTE) == 0 );
        aRef.Ref2.SetRowRel( (nFlags & SCA_ROW2_ABSOLUTE) == 0 );
        aRef.Ref2.SetTabRel( (nFlags & SCA_TAB2_ABSOLUTE) == 0 );
        if ( !(nFlags & SCA_VALID_TAB2) )
            aRef.Ref2.SetTabDeleted( true );        // #REF!
        aRef.Ref2.SetFlag3D( ( nFlags & SCA_TAB2_3D ) != 0 );
        aRef.SetRange(aRange, aPos);
        if (aExtInfo.mbExternal)
        {
            ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
            const OUString* pRealTab = pRefMgr->getRealTableName(aExtInfo.mnFileId, aExtInfo.maTabName);
            maRawToken.SetExternalDoubleRef(
                aExtInfo.mnFileId, pRealTab ? *pRealTab : aExtInfo.maTabName, aRef);
            maExternalFiles.push_back(aExtInfo.mnFileId);
        }
        else
        {
            maRawToken.SetDoubleReference(aRef);
        }
    }

    return ( nFlags & SCA_VALID ) != 0;
}

bool ScCompiler::IsSingleReference( const OUString& rName )
{
    ScAddress aAddr( aPos );
    const ScAddress::Details aDetails( pConv->meConv, aPos );
    ScAddress::ExternalInfo aExtInfo;
    sal_uInt16 nFlags = aAddr.Parse( rName, pDoc, aDetails, &aExtInfo, &maExternalLinks );
    // Something must be valid in order to recognize Sheet1.blah or blah.a1
    // as a (wrong) reference.
    if( nFlags & ( SCA_VALID_COL|SCA_VALID_ROW|SCA_VALID_TAB ) )
    {
        ScSingleRefData aRef;
        aRef.InitAddress( aAddr );
        aRef.SetColRel( (nFlags & SCA_COL_ABSOLUTE) == 0 );
        aRef.SetRowRel( (nFlags & SCA_ROW_ABSOLUTE) == 0 );
        aRef.SetTabRel( (nFlags & SCA_TAB_ABSOLUTE) == 0 );
        aRef.SetFlag3D( ( nFlags & SCA_TAB_3D ) != 0 );
        // the reference is really invalid
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
            maRawToken.SetExternalSingleRef(
                aExtInfo.mnFileId, pRealTab ? *pRealTab : aExtInfo.maTabName, aRef);
            maExternalFiles.push_back(aExtInfo.mnFileId);
        }
        else
            maRawToken.SetSingleReference(aRef);
    }

    return ( nFlags & SCA_VALID ) != 0;
}

bool ScCompiler::IsReference( const OUString& rName )
{
    // Has to be called before IsValue
    sal_Unicode ch1 = rName[0];
    sal_Unicode cDecSep = ( mxSymbols->isEnglish() ? '.' :
        ScGlobal::pLocaleData->getNumDecimalSep()[0] );
    if ( ch1 == cDecSep )
        return false;
    // Who was that imbecile introducing '.' as the sheet name separator!?!
    if ( rtl::isAsciiDigit( ch1 ) && pConv->getSpecialSymbol( Convention::SHEET_SEPARATOR) == '.' )
    {
        // Numerical sheet name is valid.
        // But English 1.E2 or 1.E+2 is value 100, 1.E-2 is 0.01
        // Don't create a #REF! of values. But also do not bail out on
        // something like 3:3, meaning entire row 3.
        do
        {
            const sal_Int32 nPos = ScGlobal::FindUnquoted( rName, '.');
            if ( nPos == -1 )
            {
                if (ScGlobal::FindUnquoted( rName, ':') != -1)
                    break;      // may be 3:3, continue as usual
                return false;
            }
            sal_Unicode const * const pTabSep = rName.getStr() + nPos;
            sal_Unicode ch2 = pTabSep[1];   // maybe a column identifier
            if ( !(ch2 == '$' || rtl::isAsciiAlpha( ch2 )) )
                return false;
            if ( cDecSep == '.' && (ch2 == 'E' || ch2 == 'e')   // E + - digit
                    && (GetCharTableFlags( pTabSep[2], pTabSep[1] ) & SC_COMPILER_C_VALUE_EXP) )
            {
                // If it is an 1.E2 expression check if "1" is an existent sheet
                // name. If so, a desired value 1.E2 would have to be entered as
                // 1E2 or 1.0E2 or 1.E+2, sorry. Another possibility would be to
                // require numerical sheet names always being entered quoted, which
                // is not desirable (too many 1999, 2000, 2001 sheets in use).
                // Furthermore, XML files created with versions prior to SRC640e
                // wouldn't contain the quotes added by MakeTabStr()/CheckTabQuotes()
                // and would produce wrong formulas if the conditions here are met.
                // If you can live with these restrictions you may remove the
                // check and return an unconditional FALSE.
                OUString aTabName( rName.copy( 0, nPos ) );
                SCTAB nTab;
                if ( !pDoc->GetTable( aTabName, nTab ) )
                    return false;
                // If sheet "1" exists and the expression is 1.E+2 continue as
                // usual, the ScRange/ScAddress parser will take care of it.
            }
        } while(false);
    }

    if (IsSingleReference( rName))
        return true;

    // Though the range operator is handled explicitly, when encountering
    // something like Sheet1.A:A we will have to treat it as one entity if it
    // doesn't pass as single cell reference.
    if (mnRangeOpPosInSymbol > 0)   // ":foo" would be nonsense
    {
        if (IsDoubleReference( rName))
            return true;
        // Now try with a symbol up to the range operator, rewind source
        // position.
        sal_Int32 nLen = mnRangeOpPosInSymbol;
        while (cSymbol[++nLen])
            ;
        cSymbol[mnRangeOpPosInSymbol] = 0;
        nSrcPos -= (nLen - mnRangeOpPosInSymbol);
        mnRangeOpPosInSymbol = -1;
        mbRewind = true;
        return true;    // end all checks
    }
    else
    {
        switch (pConv->meConv)
        {
            case FormulaGrammar::CONV_XL_A1:
            case FormulaGrammar::CONV_XL_OOX:
                // Special treatment for the 'E:\[doc]Sheet1:Sheet3'!D5 Excel
                // sickness, mnRangeOpPosInSymbol did not catch the range
                // operator as it is within a quoted name.
                if (rName[0] != '\'')
                    return false;   // Document name has to be single quoted.
                SAL_FALLTHROUGH;
            case FormulaGrammar::CONV_XL_R1C1:
                // C2 or C[1] are valid entire column references.
                if (IsDoubleReference( rName))
                    return true;
                break;
            default:
                ;   // nothing
        }
    }
    return false;
}

bool ScCompiler::IsMacro( const OUString& rName )
{
#if !HAVE_FEATURE_SCRIPTING
    (void) rName;

    return false;
#else

    // Calling SfxObjectShell::GetBasic() may result in all sort of things
    // including obtaining the model and deep down in
    // SfxBaseModel::getDocumentStorage() acquiring the SolarMutex, which when
    // formulas are compiled from a threaded import may result in a deadlock.
    // Check first if we actually could acquire it and if not bail out.
    /* FIXME: yes, but how ... */
    vcl::SolarMutexTryAndBuyGuard g;
    if (!g.isAcquired())
    {
        SAL_WARN( "sc.core", "ScCompiler::IsMacro - SolarMutex would deadlock, not obtaining Basic");
        return false;   // bad luck
    }

    OUString aName( rName);
    StarBASIC* pObj = nullptr;
    SfxObjectShell* pDocSh = pDoc->GetDocumentShell();

    try
    {
        if( pDocSh )//XXX
            pObj = pDocSh->GetBasic();
        else
            pObj = SfxApplication::GetBasic();
    }
    catch (...)
    {
        return false;
    }

    // ODFF recommends to store user-defined functions prefixed with "USER.",
    // use only unprefixed name if encountered. BASIC doesn't allow '.' in a
    // function name so a function "USER.FOO" could not exist, and macro check
    // is assigned the lowest priority in function name check.
    if (FormulaGrammar::isODFF( GetGrammar()) && aName.startsWithIgnoreAsciiCase("USER."))
        aName = aName.copy(5);

    SbxMethod* pMeth = static_cast<SbxMethod*>(pObj->Find( aName, SbxCLASS_METHOD ));
    if( !pMeth )
    {
        return false;
    }
    // It really should be a BASIC function!
    if( pMeth->GetType() == SbxVOID
     || ( pMeth->IsFixed() && pMeth->GetType() == SbxEMPTY )
     || dynamic_cast<const SbMethod*>( pMeth) ==  nullptr )
    {
        return false;
    }
    maRawToken.SetExternal( aName.getStr() );
    maRawToken.eOp = ocMacro;
    return true;
#endif
}

bool ScCompiler::IsNamedRange( const OUString& rUpperName )
{
    // IsNamedRange is called only from NextNewToken, with an upper-case string

    // try local names first
    bool bGlobal = false;
    ScRangeName* pRangeName = pDoc->GetRangeName(aPos.Tab());
    const ScRangeData* pData = nullptr;
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
        maRawToken.SetName(bGlobal, pData->GetIndex());
        return true;
    }
    else
        return false;
}

bool ScCompiler::IsExternalNamedRange( const OUString& rSymbol, bool& rbInvalidExternalNameRange )
{
    /* FIXME: This code currently (2008-12-02T15:41+0100 in CWS mooxlsc)
     * correctly parses external named references in OOo, as required per RFE
     * #i3740#, just that we can't store them in ODF yet. We will need an OASIS
     * spec first. Until then don't pretend to support external names that
     * wouldn't survive a save and reload cycle, return false instead. */

    rbInvalidExternalNameRange = false;

    if (!pConv)
        return false;

    OUString aFile, aName;
    if (!pConv->parseExternalName( rSymbol, aFile, aName, pDoc, &maExternalLinks))
        return false;

    if (aFile.getLength() > MAXSTRLEN || aName.getLength() > MAXSTRLEN)
        return false;

    ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
    OUString aTmp = aFile;
    pRefMgr->convertToAbsName(aTmp);
    aFile = aTmp;
    sal_uInt16 nFileId = pRefMgr->getExternalFileId(aFile);
    if (!pRefMgr->isValidRangeName(nFileId, aName))
    {
        rbInvalidExternalNameRange = true;
        // range name doesn't exist in the source document.
        return false;
    }

    const OUString* pRealName = pRefMgr->getRealRangeName(nFileId, aName);
    maRawToken.SetExternalName(nFileId, pRealName ? *pRealName : OUString(aTmp));
    maExternalFiles.push_back(nFileId);
    return true;
}

bool ScCompiler::IsDBRange( const OUString& rName )
{
    ScDBCollection::NamedDBs& rDBs = pDoc->GetDBCollection()->getNamedDBs();
    const ScDBData* p = rDBs.findByUpperName(rName);
    if (!p)
        return false;

    maRawToken.SetName(true, p->GetIndex()); // DB range is always global.
    maRawToken.eOp = ocDBArea;
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
    {   // first check ranges on this sheet, in case of duplicated names
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
                    continue;   // for
                ScCellIterator aIter( pDoc, rNameRange );
                for (bool bHas = aIter.first(); bHas && !bInList; bHas = aIter.next())
                {
                    // Don't crash if cell (via CompileNameFormula) encounters
                    // a formula cell without code and
                    // HasStringData/Interpret/Compile is executed and all that
                    // recursive..
                    // Furthermore, *this* cell won't be touched, since no RPN exists yet.
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
                                aRef.SetColRel( true );     // ColName
                            else
                                aRef.SetRowRel( true );     // RowName
                            aRef.SetAddress(aIter.GetPos(), aPos);
                            bInList = bFound = true;
                        }
                    }
                }
            }
        }
    }
    if ( !bInList && pDoc->GetDocOptions().IsLookUpColRowNames() )
    {   // search in current sheet
        long nDistance = 0, nMax = 0;
        long nMyCol = (long) aPos.Col();
        long nMyRow = (long) aPos.Row();
        bool bTwo = false;
        ScAddress aOne( 0, 0, aPos.Tab() );
        ScAddress aTwo( MAXCOL, MAXROW, aPos.Tab() );

        ScAutoNameCache* pNameCache = pDoc->GetAutoNameCache();
        if ( pNameCache )
        {
            //  use GetNameOccurrences to collect all positions of aName on the sheet
            //  (only once), similar to the outer part of the loop in the "else" branch.

            const ScAutoNameAddresses& rAddresses = pNameCache->GetNameOccurrences( aName, aPos.Tab() );

            //  Loop through the found positions, similar to the inner part of the loop in the "else" branch.
            //  The order of addresses in the vector is the same as from ScCellIterator.

            ScAutoNameAddresses::const_iterator aEnd(rAddresses.end());
            for ( ScAutoNameAddresses::const_iterator aAdrIter(rAddresses.begin()); aAdrIter != aEnd; ++aAdrIter )
            {
                ScAddress aAddress( *aAdrIter );        // cell address with an equal string

                if ( bFound )
                {   // stop if everything else is further away
                    if ( nMax < (long)aAddress.Col() )
                        break;      // aIter
                }
                if ( aAddress != aPos )
                {
                    // same treatment as in isEqual case below

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
                            {   // right or below
                                bTwo = true;
                                aTwo.Set( nCol, nRow, aAddress.Tab() );
                                nMax = std::max( nMyCol + std::abs( nC ), nMyRow + std::abs( nR ) );
                                nDistance = nD;
                            }
                            else if ( !(nRow < aOne.Row() && nMyRow >= (long)aOne.Row()) )
                            {
                                // upper left, only if not further up than the
                                // current entry and nMyRow is below (CellIter
                                // runs column-wise)
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
                {   // stop if everything else is further away
                    if ( nMax < (long)aIter.GetPos().Col() )
                        break;      // aIter
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
                                {   // right or below
                                    bTwo = true;
                                    aTwo.Set( nCol, nRow, aIter.GetPos().Tab() );
                                    nMax = std::max( nMyCol + std::abs( nC ), nMyRow + std::abs( nR ) );
                                    nDistance = nD;
                                }
                                else if ( !(nRow < aOne.Row() && nMyRow >= (long)aOne.Row()) )
                                {
                                    // upper left, only if not further up than the
                                    // current entry and nMyRow is below (CellIter
                                    // runs column-wise)
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
                    aAdr = aOne;        // upper left takes precedence
                else
                {
                    if ( nMyCol < (long)aOne.Col() )
                    {   // two to the right
                        if ( nMyRow >= (long)aTwo.Row() )
                            aAdr = aTwo;        // directly right
                        else
                            aAdr = aOne;
                    }
                    else
                    {   // two below or below and right, take the nearest
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
                aRef.SetRowRel( true );     // RowName
            else
                aRef.SetColRel( true );     // ColName
            aRef.SetAddress(aAdr, aPos);
        }
    }
    if ( bFound )
    {
        maRawToken.SetSingleReference( aRef );
        maRawToken.eOp = ocColRowName;
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
        maRawToken.SetOpCode( (*iLook).second );
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
        maRawToken.SetErrorConstant( nError);
        return true;
    }
    else
        return false;
}

bool ScCompiler::IsTableRefItem( const OUString& rName ) const
{
    bool bItem = false;
    OpCodeHashMap::const_iterator iLook( mxSymbols->getHashMap()->find( rName));
    if (iLook != mxSymbols->getHashMap()->end())
    {
        // Only called when there actually is a current TableRef, hence
        // accessing maTableRefs.back() is safe.
        ScTableRefToken* p = dynamic_cast<ScTableRefToken*>(maTableRefs.back().mxToken.get());
        assert(p);  // not a ScTableRefToken can't be

        switch ((*iLook).second)
        {
            case ocTableRefItemAll:
                bItem = true;
                p->AddItem( ScTableRefToken::ALL);
                break;
            case ocTableRefItemHeaders:
                bItem = true;
                p->AddItem( ScTableRefToken::HEADERS);
                break;
            case ocTableRefItemData:
                bItem = true;
                p->AddItem( ScTableRefToken::DATA);
                break;
            case ocTableRefItemTotals:
                bItem = true;
                p->AddItem( ScTableRefToken::TOTALS);
                break;
            case ocTableRefItemThisRow:
                bItem = true;
                p->AddItem( ScTableRefToken::THIS_ROW);
                break;
            default:
                ;
        }
        if (bItem)
            maRawToken.SetOpCode( (*iLook).second );
    }
    return bItem;
}

namespace {
OUString unescapeTableRefColumnSpecifier( const OUString& rStr )
{
    // '#', '[', ']' and '\'' are escaped with '\''

    if (rStr.indexOf( '\'' ) < 0)
        return rStr;

    const sal_Int32 n = rStr.getLength();
    OUStringBuffer aBuf( n );
    const sal_Unicode* p = rStr.getStr();
    const sal_Unicode* const pStop = p + n;
    bool bEscaped = false;
    for ( ; p < pStop; ++p)
    {
        const sal_Unicode c = *p;
        if (bEscaped)
        {
            aBuf.append( c );
            bEscaped = false;
        }
        else if (c == '\'')
            bEscaped = true;    // unescaped escaping '\''
        else
            aBuf.append( c );
    }
    return aBuf.makeStringAndClear();
}
}

bool ScCompiler::IsTableRefColumn( const OUString& rName ) const
{
    // Only called when there actually is a current TableRef, hence
    // accessing maTableRefs.back() is safe.
    ScTableRefToken* p = dynamic_cast<ScTableRefToken*>(maTableRefs.back().mxToken.get());
    assert(p);  // not a ScTableRefToken can't be

    ScDBData* pDBData = pDoc->GetDBCollection()->getNamedDBs().findByIndex( p->GetIndex());
    if (!pDBData)
        return false;

    OUString aName( unescapeTableRefColumnSpecifier( rName));

    ScRange aRange;
    pDBData->GetArea( aRange);
    aRange.aEnd.SetTab( aRange.aStart.Tab());
    aRange.aEnd.SetRow( aRange.aStart.Row());

    // Prefer the stored internal table column name, which is also needed for
    // named expressions during document load time when cell content isn't
    // available yet. Also, avoiding a possible calculation step in case the
    // header cell is a formula cell is "a good thing".
    sal_Int32 nOffset = pDBData->GetColumnNameOffset( aName);
    if (nOffset >= 0)
    {
        // This is sneaky.. we always use the top row of the database range,
        // regardless of whether it is a header row or not. Code evaluating
        // this reference must take that into account and may have to act
        // differently if it is a header-less table. Which are two places,
        // HandleTableRef() (no change necessary there) and
        // CreateStringFromSingleRef() (must not fallback to cell lookup).
        ScSingleRefData aRef;
        ScAddress aAdr( aRange.aStart);
        aAdr.IncCol( nOffset);
        aRef.InitAddress( aAdr);
        maRawToken.SetSingleReference( aRef );
        return true;
    }

    if (pDBData->HasHeader())
    {
        // Quite similar to IsColRowName() but limited to one row of headers.
        ScCellIterator aIter( pDoc, aRange);
        for (bool bHas = aIter.first(); bHas; bHas = aIter.next())
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
                if (ScGlobal::GetpTransliteration()->isEqual( aStr, aName))
                {
                    // If this is successful and the internal column name
                    // lookup was not, it may be worth a warning.
                    SAL_WARN("sc.core", "ScCompiler::IsTableRefColumn - falling back to cell lookup");

                    /* XXX NOTE: we could init the column as relative so copying a
                     * formula across columns would point to the relative column,
                     * but do it absolute because:
                     * a) it makes the reference work in named expressions without
                     * having to distinguish
                     * b) Excel does it the same. */
                    ScSingleRefData aRef;
                    aRef.InitAddress( aIter.GetPos());
                    maRawToken.SetSingleReference( aRef );
                    return true;
                }
            }
        }
    }

    return false;
}

void ScCompiler::SetAutoCorrection( bool bVal )
{
    assert(mbJumpCommandReorder);
    bAutoCorrect = bVal;
    mbStopOnError = !bVal;
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
        {   // "...
            // What's not a word doesn't belong to it.
            // Don't be pedantic: c < 128 should be sufficient here.
            while ( nPos && ((aCorrectedSymbol[nPos] < 128) &&
                    ((GetCharTableFlags(aCorrectedSymbol[nPos], aCorrectedSymbol[nPos-1]) &
                    (SC_COMPILER_C_WORD | SC_COMPILER_C_CHAR_DONTCARE)) == 0)) )
                nPos--;
            if ( nPos == MAXSTRLEN - 2 )
                aCorrectedSymbol = aCorrectedSymbol.replaceAt( nPos, 1, OUString(cQuote) );   // '"' the 255th character
            else
                aCorrectedSymbol = aCorrectedSymbol.replaceAt( nPos + 1, 0, OUString(cQuote) );
            bCorrected = true;
        }
        else if ( c1 != cQuote && c2 == cQuote )
        {   // ..."
            aCorrectedSymbol = OUStringLiteral1<cQuote>() + aCorrectedSymbol;
            bCorrected = true;
        }
        else if ( nPos == 0 && (c1 == cx || c1 == cX) )
        {   // x => *
            aCorrectedSymbol = mxSymbols->getSymbol(ocMul);
            bCorrected = true;
        }
        else if ( (GetCharTableFlags( c1, 0 ) & SC_COMPILER_C_CHAR_VALUE)
               && (GetCharTableFlags( c2, c2p ) & SC_COMPILER_C_CHAR_VALUE) )
        {
            if ( comphelper::string::getTokenCount(aCorrectedSymbol, cx) > 1 )
            {   // x => *
                sal_Unicode c = mxSymbols->getSymbolChar(ocMul);
                aCorrectedSymbol = aCorrectedSymbol.replaceAll(OUStringLiteral1<cx>(), OUString(c));
                bCorrected = true;
            }
            if ( comphelper::string::getTokenCount(aCorrectedSymbol, cX) > 1 )
            {   // X => *
                sal_Unicode c = mxSymbols->getSymbolChar(ocMul);
                aCorrectedSymbol = aCorrectedSymbol.replaceAll(OUStringLiteral1<cX>(), OUString(c));
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
            {   // Split off 'Doc'#, may be d:\... or whatever
                aDoc = aSymbol.copy(0, nPosition + 2);
                aSymbol = aSymbol.copy(nPosition + 2);
            }
            sal_Int32 nRefs = comphelper::string::getTokenCount(aSymbol, ':');
            bool bColons;
            if ( nRefs > 2 )
            {   // duplicated or too many ':'? B:2::C10 => B2:C10
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
                                // Must be alternating number/string, only
                                // strip within a reference.
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
                        {   // ::
                            nRefs--;
                            if ( nLen1 )
                            {   // B10::C10 ? append ':' on next round
                                if ( !bLastAlp && !CharClass::isAsciiNumeric( aTmp1 ) )
                                    nStrip++;
                            }
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
            {   // reference twisted? 4A => A4 etc.
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
                        nDotPos = nTmp++;      // the last one counts
                    if ( nDotPos != -1 )
                    {
                        aTab[j] = aRef[j].copy( 0, nDotPos + 1 );  // with '.'
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
        // ODFF has a defined set of English function names, avoid i18n
        // overhead.
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

    // Short cut for references when reading ODF to speedup things.
    if (mnPredetectedReference)
    {
        OUString aStr( cSymbol);
        bool bInvalidExternalNameRange;
        if (!IsPredetectedReference( aStr) && !IsExternalNamedRange( aStr, bInvalidExternalNameRange ))
        {
            /* TODO: it would be nice to generate a #REF! error here, which
             * would need an ocBad token with additional error value.
             * FormulaErrorToken wouldn't do because we want to preserve the
             * original string containing partial valid address
             * information if not ODFF (in that case it was already handled).
             * */
            svl::SharedString aSS = pDoc->GetSharedStringPool().intern(aStr);
            maRawToken.SetString(aSS.getData(), aSS.getDataIgnoreCase());
            maRawToken.NewOpCode( ocBad );
        }
        return true;
    }

    if ( (cSymbol[0] == '#' || cSymbol[0] == '$') && cSymbol[1] == 0 &&
            !bAutoCorrect )
    {   // special case to speed up broken [$]#REF documents
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
    bool bAsciiNonAlnum;    // operators, separators, ...
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
    if (bAsciiNonAlnum && cSymbol[1] == 0)
    {
        // Shortcut for operators and separators that need no further checks or upper.
        if (IsOpCode( OUString( cSymbol), bInArray ))
            return true;
    }
    if ( bMayBeFuncName )
    {
        // a function name must be followed by a parenthesis
        const sal_Unicode* p = aFormula.getStr() + nSrcPos;
        while( *p == ' ' )
            p++;
        bMayBeFuncName = ( *p == '(' );
    }

    // Italian ARCTAN.2 resulted in #REF! => IsOpcode() before
    // IsReference().

    OUString aUpper;

    do
    {
        const OUString aOrg( cSymbol );

        // Check for TableRef column specifier first, it may be anything.
        if (cSymbol[0] != '#' && !maTableRefs.empty() && maTableRefs.back().mnLevel)
        {
            if (IsTableRefColumn( aOrg ))
                return true;
            // Do not attempt to resolve as any other name.
            aUpper = aOrg;  // for ocBad
            break;          // do; create ocBad token or set error.
        }

        mbRewind = false;
        aUpper.clear();
        bool bAsciiUpper = false;

        if (bAsciiNonAlnum)
        {
            bAsciiUpper = lcl_UpperAsciiOrI18n( aUpper, aOrg, meGrammar);
            if (cSymbol[0] == '#')
            {
                // Check for TableRef item specifiers first.
                if (!maTableRefs.empty() && maTableRefs.back().mnLevel == 2)
                {
                    if (IsTableRefItem( aUpper ))
                        return true;
                }

                // This can be only an error constant, if any.
                if (IsErrorConstant( aUpper))
                    return true;

                break;  // do; create ocBad token or set error.
            }
            if (IsOpCode( aUpper, bInArray ))
                return true;
        }

        if (bMayBeFuncName)
        {
            if (aUpper.isEmpty())
                bAsciiUpper = lcl_UpperAsciiOrI18n( aUpper, aOrg, meGrammar);
            if (IsOpCode( aUpper, bInArray ))
                return true;
        }

        // Column 'DM' ("Deutsche Mark", German currency) couldn't be
        // referred => IsReference() before IsValue().
        // Preserve case of file names in external references.
        if (IsReference( aOrg ))
        {
            if (mbRewind)   // Range operator, but no direct reference.
                continue;   // do; up to range operator.
            // If a syntactically correct reference was recognized but invalid
            // e.g. because of non-existing sheet name => entire reference
            // ocBad to preserve input instead of #REF!.A1
            if (!maRawToken.IsValidReference())
            {
                aUpper = aOrg;          // ensure for ocBad
                break;                  // do; create ocBad token or set error.
            }
            return true;
        }

        if (aUpper.isEmpty())
            bAsciiUpper = lcl_UpperAsciiOrI18n( aUpper, aOrg, meGrammar);

        // IsBoolean() before IsValue() to catch inline bools without the kludge
        //    for inline arrays.
        if (bAllowBooleans && IsBoolean( aUpper ))
            return true;

        if (IsValue( aUpper ))
            return true;

        // User defined names and such do need i18n upper also in ODF.
        if (bAsciiUpper)
            aUpper = ScGlobal::pCharClass->uppercase( aOrg );

        if (IsNamedRange( aUpper ))
            return true;
        // Preserve case of file names in external references.
        bool bInvalidExternalNameRange;
        if (IsExternalNamedRange( aOrg, bInvalidExternalNameRange ))
            return true;
        // Preserve case of file names in external references even when range
        // is not valid and previous check failed tdf#89330
        if (bInvalidExternalNameRange)
        {
            // add ocBad but do not lowercase
            svl::SharedString aSS = pDoc->GetSharedStringPool().intern(aOrg);
            maRawToken.SetString(aSS.getData(), aSS.getDataIgnoreCase());
            maRawToken.NewOpCode( ocBad );
            return true;
        }
        if (IsDBRange( aUpper ))
            return true;
        // If followed by '(' (with or without space inbetween) it can not be a
        // column/row label. Prevent arbitrary content detection.
        if (!bMayBeFuncName && IsColRowName( aUpper ))
            return true;
        if (bMayBeFuncName && IsMacro( aUpper ))
            return true;
        if (bMayBeFuncName && IsOpCode2( aUpper ))
            return true;

    } while (mbRewind);

    if ( meExtendedErrorDetection != EXTENDED_ERROR_DETECTION_NONE )
    {
        // set an error
        SetError( errNoName );
        if (meExtendedErrorDetection == EXTENDED_ERROR_DETECTION_NAME_BREAK)
            return false;   // end compilation
    }

    // Provide single token information and continue. Do not set an error, that
    // would prematurely end compilation. Simple unknown names are handled by
    // the interpreter.
    aUpper = ScGlobal::pCharClass->lowercase( aUpper );
    svl::SharedString aSS = pDoc->GetSharedStringPool().intern(aUpper);
    maRawToken.SetString(aSS.getData(), aSS.getDataIgnoreCase());
    maRawToken.NewOpCode( ocBad );
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
        // string tokens expected, GetString() will assert if token type is wrong
        rFormula = ppTokens[0]->GetString().getString();
        if( bExternal )
            rFormulaNmsp = ppTokens[1]->GetString().getString();
    }
}

namespace {

class ExternalFileInserter : public std::unary_function<sal_uInt16, void>
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
        aCorrectedFormula.clear();
        aCorrectedSymbol.clear();
    }
    sal_uInt8 nForced = 0;   // ==formula forces recalc even if cell is not visible
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
    // FunctionStack only used if PODF or OOXML!
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
        const OpCode eOp = maRawToken.GetOpCode();
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
                        aCorrectedSymbol.clear();
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
                // Don't count following column separator as parameter separator.
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
                        aCorrectedSymbol.clear();
                    }
                }
                if (bUseFunctionStack && nFunction)
                    --nFunction;
            }
            break;
            case ocTableRefOpen:
            {
                // Don't count following item separator as parameter separator.
                if (bUseFunctionStack)
                {
                    ++nFunction;
                    pFunctionStack[ nFunction ].eOp = eOp;
                    pFunctionStack[ nFunction ].nSep = 0;
                }
            }
            break;
            case ocTableRefClose:
            {
                if (bUseFunctionStack && nFunction)
                    --nFunction;
            }
            break;
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
            // FIXME: should we check for known functions with optional empty
            // args so the correction dialog can do better?
            if ( !static_cast<ScTokenArray*>(pArr)->Add( new FormulaMissingToken ) )
            {
                SetError(errCodeOverflow); break;
            }
        }
        if (bOOXML)
        {
            // Append a parameter for WEEKNUM, all 1.0
            // Function is already closed, parameter count is nSep+1
            size_t nFunc = nFunction + 1;
            if (eOp == ocClose &&
                    (pFunctionStack[ nFunc ].eOp == ocWeek &&   // 2nd week start
                     pFunctionStack[ nFunc ].nSep == 0))
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
            // Insert ADDRESS() new empty parameter 4 if there is a 4th, now to be 5th.
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
        FormulaToken* pNewToken = static_cast<ScTokenArray*>(pArr)->Add( maRawToken.CreateToken());
        if (!pNewToken)
        {
            SetError(errCodeOverflow);
            break;
        }
        else if (eLastOp == ocRange && pNewToken->GetOpCode() == ocPush && pNewToken->GetType() == svSingleRef)
        {
            static_cast<ScTokenArray*>(pArr)->MergeRangeReference( aPos);
        }
        else if (eLastOp == ocDBArea && pNewToken->GetOpCode() == ocTableRefOpen)
        {
            sal_uInt16 nIdx = pArr->GetLen() - 1;
            const FormulaToken* pPrev = pArr->PeekPrev( nIdx);
            if (pPrev && pPrev->GetOpCode() == ocDBArea)
            {
                FormulaToken* pTableRefToken = new ScTableRefToken( pPrev->GetIndex(), ScTableRefToken::TABLE);
                maTableRefs.push_back( TableRefEntry( pTableRefToken));
                // pPrev may be dead hereafter.
                static_cast<ScTokenArray*>(pArr)->ReplaceToken( nIdx, pTableRefToken,
                        FormulaTokenArray::ReplaceMode::CODE_ONLY);
            }
        }
        switch (eOp)
        {
            case ocTableRefOpen:
                SAL_WARN_IF( maTableRefs.empty(), "sc.core", "ocTableRefOpen without TableRefEntry");
                if (maTableRefs.empty())
                    SetError(errPair);
                else
                    ++maTableRefs.back().mnLevel;
                break;
            case ocTableRefClose:
                SAL_WARN_IF( maTableRefs.empty(), "sc.core", "ocTableRefClose without TableRefEntry");
                if (maTableRefs.empty())
                    SetError(errPair);
                else
                {
                    if (--maTableRefs.back().mnLevel == 0)
                        maTableRefs.pop_back();
                }
                break;
            default:
                break;
        }
        eLastOp = maRawToken.GetOpCode();
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

    // remember pArr, in case a subsequent CompileTokenArray() is executed.
    ScTokenArray* pNew = new ScTokenArray( aArr );
    pNew->GenHash();
    pArr = pNew;

    if (!maExternalFiles.empty())
    {
        // Remove duplicates, and register all external files found in this cell.
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
            // remember pArr, in case a subsequent CompileTokenArray() is executed.
            ScTokenArray* pNew = new ScTokenArray( aTokenArray );
            pArr = pNew;
            return pNew;
        }
    }
    catch( uno::Exception& )
    {
    }
    // no success - fallback to some internal grammar and hope the best
    return CompileString( rFormula );
}

ScRangeData* ScCompiler::GetRangeData( const FormulaToken& rToken ) const
{
    ScRangeData* pRangeData = nullptr;
    bool bGlobal = rToken.IsGlobal();
    if (bGlobal)
        // global named range.
        pRangeData = pDoc->GetRangeName()->findByIndex( rToken.GetIndex());
    else
    {
        // sheet local named range.
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
        else if (mbJumpCommandReorder)
        {
            ScTokenArray* pNew;
            // put named formula into parentheses.
            // But only if there aren't any yet, parenthetical
            // ocSep doesn't work, e.g. SUM((...;...))
            // or if not directly between ocSep/parenthesis,
            // e.g. SUM(...;(...;...)) no, SUM(...;(...)*3) yes,
            // in short: if it isn't a self-contained expression.
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
            pNew->SetFromRangeName( true );
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
    // Handle external range names.
    switch (_aToken.GetType())
    {
        case svExternalSingleRef:
        case svExternalDoubleRef:
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
            if (pNew->GetNextReference() != nullptr)
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

// reference of named range with relative references

void ScCompiler::SetRelNameReference()
{
    pArr->Reset();
    for( formula::FormulaToken* t = pArr->GetNextReference(); t;
                  t = pArr->GetNextReference() )
    {
        ScSingleRefData& rRef1 = *t->GetSingleRef();
        if ( rRef1.IsColRel() || rRef1.IsRowRel() || rRef1.IsTabRel() )
            rRef1.SetRelName( true );
        if ( t->GetType() == svDoubleRef )
        {
            ScSingleRefData& rRef2 = t->GetDoubleRef()->Ref2;
            if ( rRef2.IsColRel() || rRef2.IsRowRel() || rRef2.IsTabRel() )
                rRef2.SetRelName( true );
        }
    }
}

// Wrap-adjust relative references of a RangeName to current position,
// don't call for other token arrays!
void ScCompiler::MoveRelWrap( SCCOL nMaxCol, SCROW nMaxRow )
{
    pArr->Reset();
    for( formula::FormulaToken* t = pArr->GetNextReference(); t;
                  t = pArr->GetNextReference() )
    {
        if ( t->GetType() == svSingleRef || t->GetType() == svExternalSingleRef )
            ScRefUpdate::MoveRelWrap( pDoc, aPos, nMaxCol, nMaxRow, SingleDoubleRefModifier( *t->GetSingleRef() ).Ref() );
        else
            ScRefUpdate::MoveRelWrap( pDoc, aPos, nMaxCol, nMaxRow, *t->GetDoubleRef() );
    }
}

// Wrap-adjust relative references of a RangeName to current position,
// don't call for other token arrays!
void ScCompiler::MoveRelWrap( ScTokenArray& rArr, ScDocument* pDoc, const ScAddress& rPos,
                              SCCOL nMaxCol, SCROW nMaxRow )
{
    rArr.Reset();
    for( formula::FormulaToken* t = rArr.GetNextReference(); t;
                  t = rArr.GetNextReference() )
    {
        if ( t->GetType() == svSingleRef || t->GetType() == svExternalSingleRef )
            ScRefUpdate::MoveRelWrap( pDoc, rPos, nMaxCol, nMaxRow, SingleDoubleRefModifier( *t->GetSingleRef() ).Ref() );
        else
            ScRefUpdate::MoveRelWrap( pDoc, rPos, nMaxCol, nMaxRow, *t->GetDoubleRef() );
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
            // convention not known => assume valid
        }
        return true;
    }
    else if (bTestLetterNumeric)
        return ScGlobal::pCharClass->isLetterNumeric( rStr, nPos );
    else
        return false;
}

void ScCompiler::CreateStringFromExternal( OUStringBuffer& rBuffer, const FormulaToken* pTokenP ) const
{
    const FormulaToken* t = pTokenP;
    sal_uInt16 nFileId = t->GetIndex();
    ScExternalRefManager* pRefMgr = pDoc->GetExternalRefManager();
    const OUString* pFileName = pRefMgr->getExternalFileName(nFileId);
    if (!pFileName)
        return;

    switch (t->GetType())
    {
        case svExternalName:
            rBuffer.append(pConv->makeExternalNameStr( nFileId, *pFileName, t->GetString().getString()));
        break;
        case svExternalSingleRef:
            pConv->makeExternalRefStr(
                   rBuffer, GetPos(), nFileId, *pFileName, t->GetString().getString(),
                   *t->GetSingleRef());
        break;
        case svExternalDoubleRef:
        {
            vector<OUString> aTabNames;
            pRefMgr->getAllCachedTableNames(nFileId, aTabNames);
            // No sheet names is a valid case if external sheets were not
            // cached in this document and external document is not reachable,
            // else not and worth to be investigated.
            SAL_WARN_IF( aTabNames.empty(), "sc.core", "wrecked cache of external document? '" <<
                    *pFileName << "' '" << t->GetString().getString() << "'");

            pConv->makeExternalRefStr(
                rBuffer, GetPos(), nFileId, *pFileName, aTabNames, t->GetString().getString(),
                *t->GetDoubleRef());
        }
        break;
        default:
            // warning, not error, otherwise we may end up with a never
            // ending message box loop if this was the cursor cell to be redrawn.
            OSL_FAIL("ScCompiler::CreateStringFromToken: unknown type of ocExternalRef");
    }
}

void ScCompiler::CreateStringFromMatrix( OUStringBuffer& rBuffer, const FormulaToken* pTokenP ) const
{
    const ScMatrix* pMatrix = pTokenP->GetMatrix();
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

namespace {
void escapeTableRefColumnSpecifier( OUString& rStr )
{
    const sal_Int32 n = rStr.getLength();
    OUStringBuffer aBuf( n * 2 );
    const sal_Unicode* p = rStr.getStr();
    const sal_Unicode* const pStop = p + n;
    for ( ; p < pStop; ++p)
    {
        const sal_Unicode c = *p;
        switch (c)
        {
            case '\'':
            case '[':
            case '#':
            case ']':
                aBuf.append( '\'' );
                break;
            default:
                ;   // nothing
        }
        aBuf.append( c );
    }
    rStr = aBuf.makeStringAndClear();
}
}

void ScCompiler::CreateStringFromSingleRef( OUStringBuffer& rBuffer, const FormulaToken* _pTokenP ) const
{
    const FormulaToken* p;
    OUString aErrRef = GetCurrentOpCodeMap()->getSymbol(ocErrRef);
    const OpCode eOp = _pTokenP->GetOpCode();
    const ScSingleRefData& rRef = *_pTokenP->GetSingleRef();
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
            pConv->makeRefStr(rBuffer, meGrammar, aPos, aErrRef,
                              GetSetupTabNames(), aRef, true, (pArr && pArr->IsFromRangeName()));
        }
    }
    else if (pArr && (p = pArr->PeekPrevNoSpaces()) && p->GetOpCode() == ocTableRefOpen)
    {
        OUString aStr;
        ScAddress aAbs = rRef.toAbs(aPos);
        const ScDBData* pData = pDoc->GetDBAtCursor( aAbs.Col(), aAbs.Row(), aAbs.Tab(), ScDBDataPortion::AREA);
        SAL_WARN_IF( !pData, "sc.core", "ScCompiler::CreateStringFromSingleRef - TableRef without ScDBData: " <<
                aAbs.Format( SCA_VALID | SCA_TAB_3D, pDoc));
        if (pData)
            aStr = pData->GetTableColumnName( aAbs.Col());
        if (aStr.isEmpty())
        {
            if (pData && pData->HasHeader())
            {
                SAL_WARN("sc.core", "ScCompiler::CreateStringFromSingleRef - TableRef falling back to cell: " <<
                        aAbs.Format( SCA_VALID | SCA_TAB_3D, pDoc));
                aStr = pDoc->GetString(aAbs);
            }
            else
            {
                SAL_WARN("sc.core", "ScCompiler::CreateStringFromSingleRef - TableRef of empty header-less: " <<
                        aAbs.Format( SCA_VALID | SCA_TAB_3D, pDoc));
                aStr = aErrRef;
            }
        }
        escapeTableRefColumnSpecifier( aStr);
        rBuffer.append(aStr);
    }
    else
        pConv->makeRefStr(rBuffer, meGrammar, aPos, aErrRef,
                          GetSetupTabNames(), aRef, true, (pArr && pArr->IsFromRangeName()));
}

void ScCompiler::CreateStringFromDoubleRef( OUStringBuffer& rBuffer, const FormulaToken* _pTokenP ) const
{
    OUString aErrRef = GetCurrentOpCodeMap()->getSymbol(ocErrRef);
    pConv->makeRefStr(rBuffer, meGrammar, aPos, aErrRef, GetSetupTabNames(),
                      *_pTokenP->GetDoubleRef(), false, (pArr && pArr->IsFromRangeName()));
}

void ScCompiler::CreateStringFromIndex( OUStringBuffer& rBuffer, const FormulaToken* _pTokenP ) const
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
        case ocTableRef:
        {
            if (NeedsTableRefTransformation())
            {
                // Write the resulting reference if TableRef is not supported.
                const ScTableRefToken* pTR = dynamic_cast<const ScTableRefToken*>(_pTokenP);
                if (!pTR)
                    AppendErrorConstant( aBuffer, errNoCode);
                else
                {
                    const FormulaToken* pRef = pTR->GetAreaRefRPN();
                    if (!pRef)
                        AppendErrorConstant( aBuffer, errNoCode);
                    else
                    {
                        switch (pRef->GetType())
                        {
                            case svSingleRef:
                                CreateStringFromSingleRef( aBuffer, pRef);
                                break;
                            case svDoubleRef:
                                CreateStringFromDoubleRef( aBuffer, pRef);
                                break;
                            case svError:
                                AppendErrorConstant( aBuffer, pRef->GetError());
                                break;
                            default:
                                AppendErrorConstant( aBuffer, errNoCode);
                        }
                    }
                }
            }
            else
            {
                const ScDBData* pDBData = pDoc->GetDBCollection()->getNamedDBs().findByIndex(_pTokenP->GetIndex());
                if (pDBData)
                    aBuffer.append(pDBData->GetName());
            }
        }
        break;
        default:
            ;   // nothing
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

// Put quotes around string if non-alphanumeric characters are contained,
// quote characters contained within are escaped by '\\'.
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

FormulaTokenRef ScCompiler::ExtendRangeReference( FormulaToken & rTok1, FormulaToken & rTok2, bool bReuseDoubleRef )
{
    return extendRangeReference( rTok1, rTok2, aPos,bReuseDoubleRef );
}

void ScCompiler::fillAddInToken(::std::vector< css::sheet::FormulaOpCodeMapEntry >& _rVec,bool _bIsEnglish) const
{
    // All known AddIn functions.
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
    // FIXME: what about those old non-UNO AddIns?
}

bool ScCompiler::HandleColRowName()
{
    ScSingleRefData& rRef = *mpToken.get()->GetSingleRef();
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
            break;  // for
        }
    }
    if ( !bInList && pDoc->GetDocOptions().IsLookUpColRowNames() )
    {   // automagically or created by copying and NamePos isn't in list
        ScRefCellValue aCell(*pDoc, aLook);
        bool bString = aCell.hasString();
        if (!bString && aCell.isEmpty())
            bString = true;     // empty cell is ok
        if ( bString )
        {   // corresponds with ScInterpreter::ScColRowNameAuto()
            bValidName = true;
            if ( bColName )
            {   // ColName
                SCROW nStartRow = nRow + 1;
                if ( nStartRow > MAXROW )
                    nStartRow = MAXROW;
                SCROW nMaxRow = MAXROW;
                if ( nMyCol == nCol )
                {   // formula cell in same column
                    if ( nMyRow == nStartRow )
                    {   // take remainder under name cell
                        nStartRow++;
                        if ( nStartRow > MAXROW )
                            nStartRow = MAXROW;
                    }
                    else if ( nMyRow > nStartRow )
                    {   // from name cell down to formula cell
                        nMaxRow = nMyRow - 1;
                    }
                }
                for ( size_t i = 0, nPairs = pRL->size(); i < nPairs; ++i )
                {   // next defined ColNameRange below limits row
                    ScRangePair* pR = (*pRL)[i];
                    const ScRange& rRange = pR->GetRange(1);
                    if ( rRange.aStart.Col() <= nCol && nCol <= rRange.aEnd.Col() )
                    {   // identical column range
                        SCROW nTmp = rRange.aStart.Row();
                        if ( nStartRow < nTmp && nTmp <= nMaxRow )
                            nMaxRow = nTmp - 1;
                    }
                }
                aRange.aStart.Set( nCol, nStartRow, nTab );
                aRange.aEnd.Set( nCol, nMaxRow, nTab );
            }
            else
            {   // RowName
                SCCOL nStartCol = nCol + 1;
                if ( nStartCol > MAXCOL )
                    nStartCol = MAXCOL;
                SCCOL nMaxCol = MAXCOL;
                if ( nMyRow == nRow )
                {   // formula cell in same row
                    if ( nMyCol == nStartCol )
                    {   // take remainder right from name cell
                        nStartCol++;
                        if ( nStartCol > MAXCOL )
                            nStartCol = MAXCOL;
                    }
                    else if ( nMyCol > nStartCol )
                    {   // from name cell right to formula cell
                        nMaxCol = nMyCol - 1;
                    }
                }
                for ( size_t i = 0, nPairs = pRL->size(); i < nPairs; ++i )
                {   // next defined RowNameRange to the right limits column
                    ScRangePair* pR = (*pRL)[i];
                    const ScRange& rRange = pR->GetRange(1);
                    if ( rRange.aStart.Row() <= nRow && nRow <= rRange.aEnd.Row() )
                    {   // identical row range
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
        // And now the magic to distinguish between a range and a single
        // cell thereof, which is picked position-dependent of the formula
        // cell. If a direct neighbor is a binary operator (ocAdd, ...) a
        // SingleRef matching the column/row of the formula cell is
        // generated. A ocColRowName or ocIntersect as a neighbor results
        // in a range. Special case: if label is valid for a single cell, a
        // position independent SingleRef is generated.
        bool bSingle = (aRange.aStart == aRange.aEnd);
        bool bFound;
        if ( bSingle )
            bFound = true;
        else
        {
            FormulaToken* p1 = pArr->PeekPrevNoSpaces();
            FormulaToken* p2 = pArr->PeekNextNoSpaces();
            // begin/end of a formula => single
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
            {   // column and/or row must match range
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
        else if (mbJumpCommandReorder)
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
                {   // automagically
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
    else if (mbJumpCommandReorder)
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

bool ScCompiler::GetTokenIfOpCode( OpCode eOp )
{
    const formula::FormulaToken* p = pArr->PeekNextNoSpaces();
    if (p && p->GetOpCode() == eOp)
        return GetToken();
    return false;
}


/* Documentation on MS-Excel Table structured references:
 * https://support.office.com/en-us/article/Use-structured-references-in-Excel-table-formulas-75fb07d3-826a-449c-b76f-363057e3d16f
 * * as of Excel 2013
 * [MS-XLSX]: Formulas https://msdn.microsoft.com/en-us/library/dd906358.aspx
 * * look for structure-reference
 */

bool ScCompiler::HandleTableRef()
{
    ScTableRefToken* pTR = dynamic_cast<ScTableRefToken*>(mpToken.get());
    if (!pTR)
    {
        SetError(errUnknownToken);
        return true;
    }

    ScDBData* pDBData = pDoc->GetDBCollection()->getNamedDBs().findByIndex( pTR->GetIndex());
    if ( !pDBData )
        SetError(errNoName);
    else if (mbJumpCommandReorder)
    {
        ScRange aDBRange;
        pDBData->GetArea(aDBRange);
        aDBRange.aEnd.SetTab(aDBRange.aStart.Tab());
        ScRange aRange( aDBRange);
        sal_uInt16 nError = 0;
        bool bForwardToClose = false;
        ScTableRefToken::Item eItem = pTR->GetItem();
        switch (eItem)
        {
            case ScTableRefToken::TABLE:
                {
                    // The table name without items references the table data,
                    // without headers or totals.
                    if (pDBData->HasHeader())
                        aRange.aStart.IncRow();
                    if (pDBData->HasTotals())
                        aRange.aEnd.IncRow(-1);
                    if (aRange.aEnd.Row() < aRange.aStart.Row())
                        nError = errNoRef;
                    bForwardToClose = true;
                }
                break;
            case ScTableRefToken::ALL:
                {
                    bForwardToClose = true;
                }
                break;
            case ScTableRefToken::HEADERS:
                {
                    if (pDBData->HasHeader())
                        aRange.aEnd.SetRow( aRange.aStart.Row());
                    else
                        nError = errNoRef;
                    bForwardToClose = true;
                }
                break;
            case ScTableRefToken::DATA:
                {
                    if (pDBData->HasHeader())
                        aRange.aStart.IncRow();
                }
                SAL_FALLTHROUGH;
            case ScTableRefToken::HEADERS_DATA:
                {
                    if (pDBData->HasTotals())
                        aRange.aEnd.IncRow(-1);
                    if (aRange.aEnd.Row() < aRange.aStart.Row())
                        nError = errNoRef;
                    bForwardToClose = true;
                }
                break;
            case ScTableRefToken::TOTALS:
                {
                    if (pDBData->HasTotals())
                        aRange.aStart.SetRow( aRange.aEnd.Row());
                    else
                        nError = errNoRef;
                    bForwardToClose = true;
                }
                break;
            case ScTableRefToken::DATA_TOTALS:
                {
                    if (pDBData->HasHeader())
                        aRange.aStart.IncRow();
                    if (aRange.aEnd.Row() < aRange.aStart.Row())
                        nError = errNoRef;
                    bForwardToClose = true;
                }
                break;
            case ScTableRefToken::THIS_ROW:
                {
                    if (aRange.aStart.Row() <= aPos.Row() && aPos.Row() <= aRange.aEnd.Row())
                    {
                        aRange.aStart.SetRow( aPos.Row());
                        aRange.aEnd.SetRow( aPos.Row());
                    }
                    else
                    {
                        nError = errNoValue;
                        // For *some* relative row reference in named
                        // expressions' thisrow special handling below.
                        aRange.aEnd.SetRow( aRange.aStart.Row());
                    }
                    bForwardToClose = true;
                }
                break;
        }
        bool bColumnRange = false;
        bool bCol1Rel = false;
        bool bCol2Rel = false;
        bool bCol1RelName = false;
        bool bCol2RelName = false;
        int nLevel = 0;
        if (bForwardToClose && GetTokenIfOpCode( ocTableRefOpen))
        {
            ++nLevel;
            enum
            {
                sOpen,
                sItem,
                sClose,
                sSep,
                sLast,
                sStop
            } eState = sOpen;
            do
            {
                const formula::FormulaToken* p = pArr->PeekNextNoSpaces();
                if (!p)
                    eState = sStop;
                else
                {
                    switch (p->GetOpCode())
                    {
                        case ocTableRefOpen:
                            eState = ((eState == sOpen || eState == sSep) ? sOpen : sStop);
                            if (++nLevel > 2)
                            {
                                SetError( errPair);
                                eState = sStop;
                            }
                            break;
                        case ocTableRefItemAll:
                        case ocTableRefItemHeaders:
                        case ocTableRefItemData:
                        case ocTableRefItemTotals:
                        case ocTableRefItemThisRow:
                            eState = ((eState == sOpen) ? sItem : sStop);
                            break;
                        case ocTableRefClose:
                            eState = ((eState == sItem || eState == sClose) ? sClose : sStop);
                            if (eState != sStop && --nLevel == 0)
                                eState = sLast;
                            break;
                        case ocSep:
                            eState = ((eState == sClose) ? sSep : sStop);
                            break;
                        case ocPush:
                            if (eState == sOpen && p->GetType() == svSingleRef)
                            {
                                bColumnRange = true;
                                bCol1Rel = p->GetSingleRef()->IsColRel();
                                bCol1RelName = p->GetSingleRef()->IsRelName();
                                eState = sLast;
                            }
                            else
                            {
                                eState = sStop;
                            }
                            break;
                        case ocBad:
                            eState = sLast;
                            if (!nError)
                                nError = errNoName;
                            break;
                        default:
                            eState = sStop;
                    }
                    if (eState != sStop)
                        GetToken();
                    if (eState == sLast)
                        eState = sStop;
                }
            } while (eState != sStop);
        }
        ScTokenArray* pNew = new ScTokenArray();
        if (!nError || nError == errNoValue)
        {
            // The errNoValue case generates a thisrow reference that can be
            // used to save named expressions in A1 syntax notation.
            if (bColumnRange)
            {
                // Limit range to specified columns.
                ScRange aColRange( ScAddress::INITIALIZE_INVALID );
                switch (mpToken->GetType())
                {
                    case svSingleRef:
                        {
                            aColRange.aStart = aColRange.aEnd = mpToken->GetSingleRef()->toAbs( aPos);
                            if (    GetTokenIfOpCode( ocTableRefClose) && (nLevel--) &&
                                    GetTokenIfOpCode( ocRange) &&
                                    GetTokenIfOpCode( ocTableRefOpen) && (++nLevel) &&
                                    GetTokenIfOpCode( ocPush))
                            {
                                if (mpToken->GetType() != svSingleRef)
                                    aColRange = ScRange( ScAddress::INITIALIZE_INVALID);
                                else
                                {
                                    aColRange.aEnd = mpToken->GetSingleRef()->toAbs( aPos);
                                    aColRange.PutInOrder();
                                    bCol2Rel = mpToken->GetSingleRef()->IsColRel();
                                    bCol2RelName = mpToken->GetSingleRef()->IsRelName();
                                }
                            }
                        }
                        break;
                    default:
                        ;   // nothing
                }
                if (aColRange.aStart.Row() != aDBRange.aStart.Row() || aColRange.aEnd.Row() != aDBRange.aStart.Row())
                    aRange = ScRange( ScAddress::INITIALIZE_INVALID);
                else
                {
                    aColRange.aEnd.SetRow( aRange.aEnd.Row());
                    aRange = aRange.Intersection( aColRange);
                }
            }
            if (aRange.IsValid())
            {
                if (aRange.aStart == aRange.aEnd)
                {
                    ScSingleRefData aRefData;
                    aRefData.InitFlags();
                    aRefData.SetColRel( bCol1Rel);
                    if (eItem == ScTableRefToken::THIS_ROW)
                    {
                        aRefData.SetRowRel( true);
                        if (!bCol1RelName)
                            bCol1RelName = pArr->IsFromRangeName();
                    }
                    aRefData.SetRelName( bCol1RelName);
                    aRefData.SetFlag3D( true);
                    if (nError)
                    {
                        aRefData.SetAddress( aRange.aStart, aRange.aStart);
                        pTR->SetAreaRefRPN( new ScSingleRefToken( aRefData));   // set reference at TableRef
                        pNew->Add( new FormulaErrorToken( nError));             // set error in RPN
                    }
                    else
                    {
                        aRefData.SetAddress( aRange.aStart, aPos);
                        pTR->SetAreaRefRPN( pNew->AddSingleReference( aRefData));
                    }
                }
                else
                {
                    ScComplexRefData aRefData;
                    aRefData.InitFlags();
                    aRefData.Ref1.SetColRel( bCol1Rel);
                    aRefData.Ref2.SetColRel( bCol2Rel);
                    bool bRelName = bCol1RelName || bCol2RelName;
                    if (eItem == ScTableRefToken::THIS_ROW)
                    {
                        aRefData.Ref1.SetRowRel( true);
                        aRefData.Ref2.SetRowRel( true);
                        if (!bRelName)
                            bRelName = pArr->IsFromRangeName();
                    }
                    aRefData.Ref1.SetRelName( bRelName);
                    aRefData.Ref2.SetRelName( bRelName);
                    aRefData.Ref1.SetFlag3D( true);
                    if (nError)
                    {
                        aRefData.SetRange( aRange, aRange.aStart);
                        pTR->SetAreaRefRPN( new ScDoubleRefToken( aRefData));   // set reference at TableRef
                        pNew->Add( new FormulaErrorToken( nError));             // set error in RPN
                    }
                    else
                    {
                        aRefData.SetRange( aRange, aPos);
                        pTR->SetAreaRefRPN( pNew->AddDoubleReference( aRefData));
                    }
                }
            }
            else
            {
                pTR->SetAreaRefRPN( pNew->Add( new FormulaErrorToken( errNoRef)));
            }
        }
        else
        {
            pTR->SetAreaRefRPN( pNew->Add( new FormulaErrorToken( nError)));
        }
        while (nLevel-- > 0)
        {
            if (!GetTokenIfOpCode( ocTableRefClose))
                SetError( errPair);
        }
        PushTokenArray( pNew, true );
        pNew->Reset();
        return GetToken();
    }
    return true;
}

bool ScCompiler::IsForceArrayParameter( const formula::FormulaToken* pToken, sal_uInt16 nParam ) const
{
    ScParameterClassification::Type eType = ScParameterClassification::GetParameterType( pToken, nParam);
    return
        eType == ScParameterClassification::ForceArray ||
        eType == ScParameterClassification::ReferenceOrForceArray;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

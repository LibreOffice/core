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

#include "reffind.hxx"
#include "global.hxx"
#include "compiler.hxx"
#include "document.hxx"

// STATIC DATA -----------------------------------------------------------

namespace {

// Include colon; addresses in range reference are handled individually.
const sal_Unicode pDelimiters[] = {
    '=','(',')','+','-','*','/','^','&',' ','{','}','<','>',':', 0
};

// =======================================================================

inline bool IsText( sal_Unicode c )
{
    bool bFound = ScGlobal::UnicodeStrChr( pDelimiters, c );
    if (bFound)
        // This is one of delimiters, therefore not text.
        return false;

    // argument separator is configurable.
    const sal_Unicode sep = ScCompiler::GetNativeSymbol(ocSep).GetChar(0);
    return c != sep;
}

inline bool IsText( bool& bQuote, sal_Unicode c )
{
    if (c == '\'')
    {
        bQuote = !bQuote;
        return true;
    }
    if (bQuote)
        return true;

    return IsText(c);
}

/**
 * Find first character position that is considered text.  A character is
 * considered a text when it's within the ascii range and when it's not a
 * delimiter.
 */
sal_Int32 FindStartPos(const sal_Unicode* p, sal_Int32 nStartPos, sal_Int32 nEndPos)
{
    while (nStartPos <= nEndPos && !IsText(p[nStartPos]))
        ++nStartPos;

    return nStartPos;
}

sal_Int32 FindEndPosA1(const sal_Unicode* p, sal_Int32 nStartPos, sal_Int32 nEndPos)
{
    bool bQuote = false;
    sal_Int32 nNewEnd = nStartPos;
    while (nNewEnd <= nEndPos && IsText(bQuote, p[nNewEnd]))
        ++nNewEnd;

    return nNewEnd;
}

sal_Int32 FindEndPosR1C1(const sal_Unicode* p, sal_Int32 nStartPos, sal_Int32 nEndPos)
{
    sal_Int32 nNewEnd = nStartPos;
    p = &p[nStartPos];
    for (; nNewEnd <= nEndPos; ++p, ++nNewEnd)
    {
        if (*p == '\'')
        {
            // Skip until the closing quote.
            for (; nNewEnd <= nEndPos; ++p, ++nNewEnd)
                if (*p == '\'')
                    break;
            if (nNewEnd > nEndPos)
                break;
        }
        else if (*p == '[')
        {
            // Skip until the closing braket.
            for (; nNewEnd <= nEndPos; ++p, ++nNewEnd)
                if (*p == ']')
                    break;
            if (nNewEnd > nEndPos)
                break;
        }
        else if (!IsText(*p))
            break;
    }

    return nNewEnd;
}

/**
 * Find last character position that is considred text, from the specified
 * start position.
 */
sal_Int32 FindEndPos(const sal_Unicode* p, sal_Int32 nStartPos, sal_Int32 nEndPos,
                     formula::FormulaGrammar::AddressConvention eConv)
{
    switch (eConv)
    {
        case formula::FormulaGrammar::CONV_XL_R1C1:
            return FindEndPosR1C1(p, nStartPos, nEndPos);
        case formula::FormulaGrammar::CONV_OOO:
        case formula::FormulaGrammar::CONV_XL_A1:
        default:
            return FindEndPosA1(p, nStartPos, nEndPos);
    }
}

void ExpandToTextA1(const sal_Unicode* p, sal_Int32 nLen, sal_Int32& rStartPos, sal_Int32& rEndPos)
{
    while (rStartPos > 0 && IsText(p[rStartPos - 1]) )
        --rStartPos;
    if (rEndPos)
        --rEndPos;
    while (rEndPos+1 < nLen && IsText(p[rEndPos + 1]) )
        ++rEndPos;
}

void ExpandToTextR1C1(const sal_Unicode* p, sal_Int32 nLen, sal_Int32& rStartPos, sal_Int32& rEndPos)
{
    // move back the start position to the first text character.
    if (rStartPos > 0)
    {
        for (--rStartPos; rStartPos > 0; --rStartPos)
        {
            sal_Unicode c = p[rStartPos];
            if (c == '\'')
            {
                // Skip until the opening quote.
                for (--rStartPos; rStartPos > 0; --rStartPos)
                {
                    c = p[rStartPos];
                    if (c == '\'')
                        break;
                }
                if (rStartPos == 0)
                    break;
            }
            else if (c == ']')
            {
                // Skip until the opening braket.
                for (--rStartPos; rStartPos > 0; --rStartPos)
                {
                    c = p[rStartPos];
                    if (c == '[')
                        break;
                }
                if (rStartPos == 0)
                    break;
            }
            else if (!IsText(c))
            {
                ++rStartPos;
                break;
            }
        }
    }

    // move forward the end position to the last text character.
    rEndPos = FindEndPosR1C1(p, rEndPos, nLen-1);
}

void ExpandToText(const sal_Unicode* p, sal_Int32 nLen, sal_Int32& rStartPos, sal_Int32& rEndPos,
                  formula::FormulaGrammar::AddressConvention eConv)
{
    switch (eConv)
    {
        case formula::FormulaGrammar::CONV_XL_R1C1:
            ExpandToTextR1C1(p, nLen, rStartPos, rEndPos);
        break;
        case formula::FormulaGrammar::CONV_OOO:
        case formula::FormulaGrammar::CONV_XL_A1:
        default:
            ExpandToTextA1(p, nLen, rStartPos, rEndPos);
    }
}

}

ScRefFinder::ScRefFinder(
    const OUString& rFormula, const ScAddress& rPos,
    ScDocument* pDoc, formula::FormulaGrammar::AddressConvention eConvP) :
    maFormula(rFormula),
    meConv(eConvP),
    mpDoc(pDoc),
    maPos(rPos),
    mnFound(0),
    mnSelStart(0),
    mnSelEnd(0)
{
}

ScRefFinder::~ScRefFinder()
{
}

static sal_uInt16 lcl_NextFlags( sal_uInt16 nOld )
{
    sal_uInt16 nNew = nOld & 7;                 // die drei Abs-Flags
    nNew = ( nNew - 1 ) & 7;                // weiterzaehlen

    if (!(nOld & SCA_TAB_3D))
        nNew &= ~SCA_TAB_ABSOLUTE;          // not 3D -> never absolute!

    return ( nOld & 0xfff8 ) | nNew;
}

void ScRefFinder::ToggleRel( sal_Int32 nStartPos, sal_Int32 nEndPos )
{
    sal_Int32 nLen = maFormula.getLength();
    if (nLen <= 0)
        return;
    const sal_Unicode* pSource = maFormula.getStr();      // for quick access

    // expand selection, and instead of selection start- and end-index

    if ( nEndPos < nStartPos )
        ::std::swap(nEndPos, nStartPos);

    ExpandToText(pSource, nLen, nStartPos, nEndPos, meConv);

    OUString aResult;
    OUString aExpr;
    OUString aSep;
    ScAddress aAddr;
    mnFound = 0;

    sal_Int32 nLoopStart = nStartPos;
    while ( nLoopStart <= nEndPos )
    {
        // Determine the stard and end positions of a text segment.  Note that
        // the end position returned from FindEndPos may be one position after
        // the last character position in case of the last segment.
        sal_Int32 nEStart = FindStartPos(pSource, nLoopStart, nEndPos);
        sal_Int32 nEEnd  = FindEndPos(pSource, nEStart, nEndPos, meConv);

        aSep  = maFormula.copy(nLoopStart, nEStart-nLoopStart);
        if (nEEnd < maFormula.getLength())
            aExpr = maFormula.copy(nEStart, nEEnd-nEStart);
        else
            aExpr = maFormula.copy(nEStart);

        // Check the validity of the expression, and toggle the relative flag.
        ScAddress::Details aDetails(meConv, maPos.Row(), maPos.Col());
        sal_uInt16 nResult = aAddr.Parse(aExpr, mpDoc, aDetails);
        if ( nResult & SCA_VALID )
        {
            sal_uInt16 nFlags = lcl_NextFlags( nResult );
            aExpr = aAddr.Format(nFlags, mpDoc, aDetails);

            sal_Int32 nAbsStart = nStartPos+aResult.getLength()+aSep.getLength();

            if (!mnFound)                            // first reference ?
                mnSelStart = nAbsStart;
            mnSelEnd = nAbsStart + aExpr.getLength();        // selection, no indizes
            ++mnFound;
        }

        // assemble

        aResult += aSep;
        aResult += aExpr;

        nLoopStart = nEEnd;
    }

    OUString aTotal = maFormula.copy(0, nStartPos);
    aTotal += aResult;
    if (nEndPos < maFormula.getLength()-1)
        aTotal += maFormula.copy(nEndPos+1);

    maFormula = aTotal;
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

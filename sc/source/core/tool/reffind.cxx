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

#include <string.h>

#include "reffind.hxx"
#include "global.hxx"
#include "compiler.hxx"
#include "document.hxx"

// STATIC DATA -----------------------------------------------------------

namespace {

//  include colon -> duplicate referenced are handled individual
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
xub_StrLen FindStartPos(const sal_Unicode* p, xub_StrLen nStartPos, xub_StrLen nEndPos)
{
    while (nStartPos <= nEndPos && !IsText(p[nStartPos]))
        ++nStartPos;

    return nStartPos;
}

xub_StrLen FindEndPosA1(const sal_Unicode* p, xub_StrLen nStartPos, xub_StrLen nEndPos)
{
    bool bQuote = false;
    xub_StrLen nNewEnd = nStartPos;
    while (nNewEnd <= nEndPos && IsText(bQuote, p[nNewEnd]))
        ++nNewEnd;

    return nNewEnd;
}

xub_StrLen FindEndPosR1C1(const sal_Unicode* p, xub_StrLen nStartPos, xub_StrLen nEndPos)
{
    xub_StrLen nNewEnd = nStartPos;
    p = &p[nStartPos];
    for (; nNewEnd <= nEndPos; ++p, ++nNewEnd)
    {
        if (*p == '\'')
        {
            // Skip until the closing quote.
            for (; nNewEnd <= nEndPos; ++p, ++nNewEnd)
                if (*p == '\'')
                    break;
        }
        else if (*p == '[')
        {
            // Skip until the closing braket.
            for (; nNewEnd <= nEndPos; ++p, ++nNewEnd)
                if (*p == ']')
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
xub_StrLen FindEndPos(const sal_Unicode* p, xub_StrLen nStartPos, xub_StrLen nEndPos,
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

void ExpandToTextA1(const sal_Unicode* p, xub_StrLen nLen, xub_StrLen& rStartPos, xub_StrLen& rEndPos)
{
    while (rStartPos > 0 && IsText(p[rStartPos - 1]) )
        --rStartPos;
    if (rEndPos)
        --rEndPos;
    while (rEndPos+1 < nLen && IsText(p[rEndPos + 1]) )
        ++rEndPos;
}

void ExpandToTextR1C1(const sal_Unicode* p, xub_StrLen nLen, xub_StrLen& rStartPos, xub_StrLen& rEndPos)
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
            }
            else if (c == ']')
            {
                // Skip until the opening braket.
                for (--rStartPos; rStartPos > 0; --rStartPos)
                {
                    if (c == '[')
                        break;
                }
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

void ExpandToText(const sal_Unicode* p, xub_StrLen nLen, xub_StrLen& rStartPos, xub_StrLen& rEndPos,
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
    const String& rFormula, const ScAddress& rPos,
    ScDocument* pDocument, formula::FormulaGrammar::AddressConvention eConvP) :
    aFormula( rFormula ),
    eConv( eConvP ),
    pDoc( pDocument ),
    maPos(rPos)
{
    nSelStart = nSelEnd = nFound = 0;
}

ScRefFinder::~ScRefFinder()
{
}

sal_uInt16 lcl_NextFlags( sal_uInt16 nOld )
{
    sal_uInt16 nNew = nOld & 7;                 // die drei Abs-Flags
    nNew = ( nNew - 1 ) & 7;                // weiterzaehlen

    if (!(nOld & SCA_TAB_3D))
        nNew &= ~SCA_TAB_ABSOLUTE;          // not 3D -> never absolute!

    return ( nOld & 0xfff8 ) | nNew;
}

void ScRefFinder::ToggleRel( xub_StrLen nStartPos, xub_StrLen nEndPos )
{
    xub_StrLen nLen = aFormula.Len();
    if (!nLen)
        return;
    const sal_Unicode* pSource = aFormula.GetBuffer();      // for quick access

    // expand selection, and instead of selection start- and end-index

    if ( nEndPos < nStartPos )
        ::std::swap(nEndPos, nStartPos);

    ExpandToText(pSource, nLen, nStartPos, nEndPos, eConv);

    String aResult;
    String aExpr;
    String aSep;
    ScAddress aAddr;
    nFound = 0;

    xub_StrLen nLoopStart = nStartPos;
    while ( nLoopStart <= nEndPos )
    {
        // Determine the stard and end positions of a text segment.
        xub_StrLen nEStart = FindStartPos(pSource, nLoopStart, nEndPos);
        xub_StrLen nEEnd  = FindEndPos(pSource, nEStart, nEndPos, eConv);

        aSep  = aFormula.Copy( nLoopStart, nEStart-nLoopStart );
        aExpr = aFormula.Copy( nEStart, nEEnd-nEStart );

        // Check the validity of the expression, and toggle the relative flag.
        ScAddress::Details aDetails(eConv, maPos.Row(), maPos.Col());
        sal_uInt16 nResult = aAddr.Parse(aExpr, pDoc, aDetails);
        if ( nResult & SCA_VALID )
        {
            sal_uInt16 nFlags = lcl_NextFlags( nResult );
            aAddr.Format(aExpr, nFlags, pDoc, aDetails);

            xub_StrLen nAbsStart = nStartPos+aResult.Len()+aSep.Len();

            if (!nFound)                            // first reference ?
                nSelStart = nAbsStart;
            nSelEnd = nAbsStart+aExpr.Len();        // selection, no indizes
            ++nFound;
        }

        // assemble

        aResult += aSep;
        aResult += aExpr;

        nLoopStart = nEEnd;
    }

    String aTotal = aFormula.Copy( 0, nStartPos );
    aTotal += aResult;
    aTotal += aFormula.Copy( nEndPos+1 );

    aFormula = aTotal;
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <sal/config.h>

#include <o3tl/underlyingenumvalue.hxx>

#include <reffind.hxx>
#include <global.hxx>
#include <compiler.hxx>
#include <document.hxx>

namespace {

// Include colon; addresses in range reference are handled individually.
const sal_Unicode pDelimiters[] = {
    '=','(',')','+','-','*','/','^','&',' ','{','}','<','>',':', 0
};

bool IsText( sal_Unicode c )
{
    bool bFound = ScGlobal::UnicodeStrChr( pDelimiters, c );
    if (bFound)
        // This is one of delimiters, therefore not text.
        return false;

    // argument separator is configurable.
    const sal_Unicode sep = ScCompiler::GetNativeSymbolChar(ocSep);
    return c != sep;
}

bool IsText( bool& bQuote, sal_Unicode c )
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
 * Find last character position that is considered text, from the specified
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
    bool bQuote = false;  // skip quoted text
    while (rStartPos > 0 && IsText(bQuote, p[rStartPos - 1]) )
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

static ScRefFlags lcl_NextFlags( ScRefFlags nOld )
{
    const ScRefFlags Mask_ABS = ScRefFlags::COL_ABS | ScRefFlags::ROW_ABS | ScRefFlags::TAB_ABS;
    ScRefFlags nNew = nOld & Mask_ABS;
    nNew = ScRefFlags( o3tl::underlyingEnumValue(nNew) - 1 ) & Mask_ABS; // weiterzaehlen

    if (!(nOld & ScRefFlags::TAB_3D))
        nNew &= ~ScRefFlags::TAB_ABS; // not 3D -> never absolute!

    return (nOld & ~Mask_ABS) | nNew;
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

    OUStringBuffer aResult;
    OUString aExpr;
    OUString aSep;
    ScAddress aAddr;
    mnFound = 0;

    sal_Int32 nLoopStart = nStartPos;
    while ( nLoopStart <= nEndPos )
    {
        // Determine the start and end positions of a text segment.  Note that
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
        ScAddress::ExternalInfo aExtInfo;
        ScRefFlags nResult = aAddr.Parse(aExpr, mpDoc, aDetails, &aExtInfo);
        if ( nResult & ScRefFlags::VALID )
        {
            ScRefFlags nFlags;
            if( aExtInfo.mbExternal )
            {    // retain external doc name and tab name before toggle relative flag
                sal_Int32 nSep;
                switch(meConv)
                {
                  case formula::FormulaGrammar::CONV_XL_A1 :
                  case formula::FormulaGrammar::CONV_XL_OOX :
                  case formula::FormulaGrammar::CONV_XL_R1C1 :
                         nSep = aExpr.lastIndexOf('!');
                         break;
                  case formula::FormulaGrammar::CONV_OOO :
                  default:
                         nSep = aExpr.lastIndexOf('.');
                         break;
                }
                if (nSep >= 0)
                {
                    OUString aRef = aExpr.copy(nSep+1);
                    OUString aExtDocNameTabName = aExpr.copy(0, nSep+1);
                    nResult = aAddr.Parse(aRef, mpDoc, aDetails);
                    aAddr.SetTab(0); // force to first tab to avoid error on checking
                    nFlags = lcl_NextFlags( nResult );
                    aExpr = aExtDocNameTabName + aAddr.Format(nFlags, mpDoc, aDetails);
                }
                else
                {
                    assert(!"Invalid syntax according to address convention.");
                }
            }
            else
            {
                nFlags = lcl_NextFlags( nResult );
                aExpr = aAddr.Format(nFlags, mpDoc, aDetails);
            }

            sal_Int32 nAbsStart = nStartPos+aResult.getLength()+aSep.getLength();

            if (!mnFound)                            // first reference ?
                mnSelStart = nAbsStart;
            mnSelEnd = nAbsStart + aExpr.getLength();        // selection, no indices
            ++mnFound;
        }

        // assemble

        aResult.append(aSep);
        aResult.append(aExpr);

        nLoopStart = nEEnd;
    }

    OUString aTotal = maFormula.copy(0, nStartPos);
    aTotal += aResult.makeStringAndClear();
    if (nEndPos < maFormula.getLength()-1)
        aTotal += maFormula.copy(nEndPos+1);

    maFormula = aTotal;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

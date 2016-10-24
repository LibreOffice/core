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

#include "compare.hxx"

#include "document.hxx"
#include "docoptio.hxx"

#include <unotools/textsearch.hxx>

namespace sc {

Compare::Cell::Cell() :
    mfValue(0.0), mbValue(false), mbEmpty(false) {}

Compare::Compare() :
    meOp(SC_EQUAL), mbIgnoreCase(true) {}

CompareOptions::CompareOptions( ScDocument* pDoc, const ScQueryEntry& rEntry, bool bReg ) :
    aQueryEntry(rEntry),
    bRegEx(bReg),
    bMatchWholeCell(pDoc->GetDocOptions().IsMatchWholeCell())
{
    bRegEx = (bRegEx && (aQueryEntry.eOp == SC_EQUAL || aQueryEntry.eOp == SC_NOT_EQUAL));
    // Interpreter functions usually are case insensitive, except the simple
    // comparison operators, for which these options aren't used. Override in
    // struct if needed.
}

double CompareFunc( const Compare& rComp, CompareOptions* pOptions )
{
    const Compare::Cell& rCell1 = rComp.maCells[0];
    const Compare::Cell& rCell2 = rComp.maCells[1];

    // Keep DoubleError if encountered
    // #i40539# if bEmpty is set, bVal/nVal are uninitialized
    if (!rCell1.mbEmpty && rCell1.mbValue && !rtl::math::isFinite(rCell1.mfValue))
        return rCell1.mfValue;
    if (!rCell2.mbEmpty && rCell2.mbValue && !rtl::math::isFinite(rCell2.mfValue))
        return rCell2.mfValue;

    size_t nStringQuery = 0;    // 0:=no, 1:=0, 2:=1
    double fRes = 0;
    if (rCell1.mbEmpty)
    {
        if (rCell2.mbEmpty)
            ;       // empty cell == empty cell, fRes 0
        else if (rCell2.mbValue)
        {
            if (rCell2.mfValue != 0.0)
            {
                if (rCell2.mfValue < 0.0)
                    fRes = 1;       // empty cell > -x
                else
                    fRes = -1;      // empty cell < x
            }
            // else: empty cell == 0.0
        }
        else
        {
            if (!rCell2.maStr.isEmpty())
                fRes = -1;      // empty cell < "..."
            // else: empty cell == ""
        }
    }
    else if (rCell2.mbEmpty)
    {
        if (rCell1.mbValue)
        {
            if (rCell1.mfValue != 0.0)
            {
                if (rCell1.mfValue < 0.0)
                    fRes = -1;      // -x < empty cell
                else
                    fRes = 1;       // x > empty cell
            }
            // else: empty cell == 0.0
        }
        else
        {
            if (!rCell1.maStr.isEmpty())
                fRes = 1;       // "..." > empty cell
            // else: "" == empty cell
        }
    }
    else if (rCell1.mbValue)
    {
        if (rCell2.mbValue)
        {
            if (!rtl::math::approxEqual(rCell1.mfValue, rCell2.mfValue))
            {
                if (rCell1.mfValue - rCell2.mfValue < 0)
                    fRes = -1;
                else
                    fRes = 1;
            }
        }
        else
        {
            fRes = -1;          // number is less than string
            nStringQuery = 2;   // 1+1
        }
    }
    else if (rCell2.mbValue)
    {
        fRes = 1;               // string is greater than number
        nStringQuery = 1;       // 0+1
    }
    else
    {
        // Both strings.
        if (pOptions)
        {
            // All similar to ScTable::ValidQuery(), *rComp.pVal[1] actually
            // is/must be identical to *rEntry.pStr, which is essential for
            // regex to work through GetSearchTextPtr().
            ScQueryEntry& rEntry = pOptions->aQueryEntry;
            OSL_ENSURE(rEntry.GetQueryItem().maString == rCell2.maStr, "ScInterpreter::CompareFunc: broken options");
            if (pOptions->bRegEx)
            {
                sal_Int32 nStart = 0;
                sal_Int32 nStop  = rCell1.maStr.getLength();
                bool bMatch = rEntry.GetSearchTextPtr(
                        !rComp.mbIgnoreCase)->SearchForward(
                            rCell1.maStr.getString(), &nStart, &nStop);
                if (bMatch && pOptions->bMatchWholeCell && (nStart != 0 || nStop != rCell1.maStr.getLength()))
                    bMatch = false;     // RegEx must match entire string.
                fRes = (bMatch ? 0 : 1);
            }
            else if (rEntry.eOp == SC_EQUAL || rEntry.eOp == SC_NOT_EQUAL)
            {
                ::utl::TransliterationWrapper* pTransliteration =
                    (rComp.mbIgnoreCase ? ScGlobal::GetpTransliteration() :
                     ScGlobal::GetCaseTransliteration());
                bool bMatch = false;
                if (pOptions->bMatchWholeCell)
                {
                    if (rComp.mbIgnoreCase)
                        bMatch = rCell1.maStr.getDataIgnoreCase() == rCell2.maStr.getDataIgnoreCase();
                    else
                        bMatch = rCell1.maStr.getData() == rCell2.maStr.getData();
                }
                else
                {
                    const LanguageType nLang = ScGlobal::pSysLocale->GetLanguageTag().getLanguageType();
                    OUString aCell( pTransliteration->transliterate(
                                rCell1.maStr.getString(), nLang, 0,
                                rCell1.maStr.getLength(), nullptr));
                    OUString aQuer( pTransliteration->transliterate(
                                rCell2.maStr.getString(), nLang, 0,
                                rCell2.maStr.getLength(), nullptr));
                    bMatch = (aCell.indexOf( aQuer ) != -1);
                }
                fRes = (bMatch ? 0 : 1);
            }
            else if (rComp.mbIgnoreCase)
                fRes = (double) ScGlobal::GetCollator()->compareString(
                        rCell1.maStr.getString(), rCell2.maStr.getString());
            else
                fRes = (double) ScGlobal::GetCaseCollator()->compareString(
                        rCell1.maStr.getString(), rCell2.maStr.getString());
        }
        else if (rComp.meOp == SC_EQUAL || rComp.meOp == SC_NOT_EQUAL)
        {
            if (rComp.mbIgnoreCase)
                fRes = (rCell1.maStr.getDataIgnoreCase() == rCell2.maStr.getDataIgnoreCase()) ? 0 : 1;
            else
                fRes = (rCell1.maStr.getData() == rCell2.maStr.getData()) ? 0 : 1;
        }
        else if (rComp.mbIgnoreCase)
            fRes = (double) ScGlobal::GetCollator()->compareString(
                rCell1.maStr.getString(), rCell2.maStr.getString());
        else
            fRes = (double) ScGlobal::GetCaseCollator()->compareString(
                rCell1.maStr.getString(), rCell2.maStr.getString());
    }

    if (nStringQuery && pOptions)
    {
        const ScQueryEntry& rEntry = pOptions->aQueryEntry;
        const ScQueryEntry::QueryItemsType& rItems = rEntry.GetQueryItems();
        if (!rItems.empty())
        {
            const ScQueryEntry::Item& rItem = rItems[0];
            if (rItem.meType != ScQueryEntry::ByString && !rItem.maString.isEmpty() &&
                (rEntry.eOp == SC_EQUAL || rEntry.eOp == SC_NOT_EQUAL))
            {
                // As in ScTable::ValidQuery() match a numeric string for a
                // number query that originated from a string, e.g. in SUMIF
                // and COUNTIF. Transliteration is not needed here.
                bool bEqual = false;
                if (nStringQuery == 1)
                    bEqual = rCell1.maStr == rItem.maString;
                else
                    bEqual = rCell2.maStr == rItem.maString;

                // match => fRes=0, else fRes=1
                fRes = double((rEntry.eOp == SC_NOT_EQUAL) ? bEqual : !bEqual);
            }
        }
    }

    return fRes;
}

double CompareFunc( const Compare::Cell& rCell1, double fCell2, CompareOptions* pOptions )
{
    // Keep DoubleError if encountered
    // #i40539# if bEmpty is set, bVal/nVal are uninitialized
    if (!rCell1.mbEmpty && rCell1.mbValue && !rtl::math::isFinite(rCell1.mfValue))
        return rCell1.mfValue;
    if (!rtl::math::isFinite(fCell2))
        return fCell2;

    bool bStringQuery = false;
    double fRes = 0;
    if (rCell1.mbEmpty)
    {
        if (fCell2 != 0.0)
        {
            if (fCell2 < 0.0)
                fRes = 1;       // empty cell > -x
            else
                fRes = -1;      // empty cell < x
        }
        // else: empty cell == 0.0
    }
    else if (rCell1.mbValue)
    {
        if (!rtl::math::approxEqual(rCell1.mfValue, fCell2))
        {
            if (rCell1.mfValue - fCell2 < 0)
                fRes = -1;
            else
                fRes = 1;
        }
    }
    else
    {
        fRes = 1;               // string is greater than number
        bStringQuery = true;
    }

    if (bStringQuery && pOptions)
    {
        const ScQueryEntry& rEntry = pOptions->aQueryEntry;
        const ScQueryEntry::QueryItemsType& rItems = rEntry.GetQueryItems();
        if (!rItems.empty())
        {
            const ScQueryEntry::Item& rItem = rItems[0];
            if (rItem.meType != ScQueryEntry::ByString && !rItem.maString.isEmpty() &&
                (rEntry.eOp == SC_EQUAL || rEntry.eOp == SC_NOT_EQUAL))
            {
                // As in ScTable::ValidQuery() match a numeric string for a
                // number query that originated from a string, e.g. in SUMIF
                // and COUNTIF. Transliteration is not needed here.
                bool bEqual = rCell1.maStr == rItem.maString;

                // match => fRes=0, else fRes=1
                fRes = double((rEntry.eOp == SC_NOT_EQUAL) ? bEqual : !bEqual);
            }
        }
    }

    return fRes;
}

double CompareFunc( double fCell1, double fCell2 )
{
    // Keep DoubleError if encountered
    // #i40539# if bEmpty is set, bVal/nVal are uninitialized
    if (!rtl::math::isFinite(fCell1))
        return fCell1;
    if (!rtl::math::isFinite(fCell2))
        return fCell2;

    double fRes = 0.0;

    if (!rtl::math::approxEqual(fCell1, fCell2))
    {
        if (fCell1 - fCell2 < 0.0)
            fRes = -1;
        else
            fRes = 1;
    }

    return fRes;
}

double CompareEmptyToNumericFunc( double fCell2 )
{
    // Keep DoubleError if encountered
    // #i40539# if bEmpty is set, bVal/nVal are uninitialized
    if (!rtl::math::isFinite(fCell2))
        return fCell2;

    double fRes = 0;
    if (fCell2 != 0.0)
    {
        if (fCell2 < 0.0)
            fRes = 1;       // empty cell > -x
        else
            fRes = -1;      // empty cell < x
    }
    // else: empty cell == 0.0

    return fRes;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

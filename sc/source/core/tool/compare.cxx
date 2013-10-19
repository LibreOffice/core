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

#include "unotools/textsearch.hxx"

namespace sc {

CompareOptions::CompareOptions( ScDocument* pDoc, const ScQueryEntry& rEntry, bool bReg ) :
    aQueryEntry(rEntry),
    bRegEx(bReg),
    bMatchWholeCell(pDoc->GetDocOptions().IsMatchWholeCell()),
    bIgnoreCase(true)
{
    bRegEx = (bRegEx && (aQueryEntry.eOp == SC_EQUAL || aQueryEntry.eOp == SC_NOT_EQUAL));
    // Interpreter functions usually are case insensitive, except the simple
    // comparison operators, for which these options aren't used. Override in
    // struct if needed.
}

double CompareFunc( const Compare& rComp, CompareOptions* pOptions )
{
    // Keep DoubleError if encountered
    // #i40539# if bEmpty is set, bVal/nVal are uninitialized
    if ( !rComp.bEmpty[0] && rComp.bVal[0] && !::rtl::math::isFinite( rComp.nVal[0]))
        return rComp.nVal[0];
    if ( !rComp.bEmpty[1] && rComp.bVal[1] && !::rtl::math::isFinite( rComp.nVal[1]))
        return rComp.nVal[1];

    size_t nStringQuery = 0;    // 0:=no, 1:=0, 2:=1
    double fRes = 0;
    if ( rComp.bEmpty[ 0 ] )
    {
        if ( rComp.bEmpty[ 1 ] )
            ;       // empty cell == empty cell, fRes 0
        else if( rComp.bVal[ 1 ] )
        {
            if (rComp.nVal[1] != 0.0)
            {
                if ( rComp.nVal[ 1 ] < 0.0 )
                    fRes = 1;       // empty cell > -x
                else
                    fRes = -1;      // empty cell < x
            }
            // else: empty cell == 0.0
        }
        else
        {
            if ( !rComp.pVal[ 1 ]->isEmpty() )
                fRes = -1;      // empty cell < "..."
            // else: empty cell == ""
        }
    }
    else if ( rComp.bEmpty[ 1 ] )
    {
        if( rComp.bVal[ 0 ] )
        {
            if (rComp.nVal[0] != 0.0)
            {
                if ( rComp.nVal[ 0 ] < 0.0 )
                    fRes = -1;      // -x < empty cell
                else
                    fRes = 1;       // x > empty cell
            }
            // else: empty cell == 0.0
        }
        else
        {
            if ( !rComp.pVal[ 0 ]->isEmpty() )
                fRes = 1;       // "..." > empty cell
            // else: "" == empty cell
        }
    }
    else if( rComp.bVal[ 0 ] )
    {
        if( rComp.bVal[ 1 ] )
        {
            if ( !::rtl::math::approxEqual( rComp.nVal[ 0 ], rComp.nVal[ 1 ] ) )
            {
                if( rComp.nVal[ 0 ] - rComp.nVal[ 1 ] < 0 )
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
    else if( rComp.bVal[ 1 ] )
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
            OSL_ENSURE(rEntry.GetQueryItem().maString.getString().equals(*rComp.pVal[1]), "ScInterpreter::CompareFunc: broken options");
            if (pOptions->bRegEx)
            {
                sal_Int32 nStart = 0;
                sal_Int32 nStop  = rComp.pVal[0]->getLength();
                bool bMatch = rEntry.GetSearchTextPtr(
                        !pOptions->bIgnoreCase)->SearchForward( *rComp.pVal[0],
                            &nStart, &nStop);
                if (bMatch && pOptions->bMatchWholeCell && (nStart != 0 || nStop != rComp.pVal[0]->getLength()))
                    bMatch = false;     // RegEx must match entire string.
                fRes = (bMatch ? 0 : 1);
            }
            else if (rEntry.eOp == SC_EQUAL || rEntry.eOp == SC_NOT_EQUAL)
            {
                ::utl::TransliterationWrapper* pTransliteration =
                    (pOptions->bIgnoreCase ? ScGlobal::GetpTransliteration() :
                     ScGlobal::GetCaseTransliteration());
                bool bMatch;
                if (pOptions->bMatchWholeCell)
                    bMatch = pTransliteration->isEqual( *rComp.pVal[0], *rComp.pVal[1]);
                else
                {
                    OUString aCell( pTransliteration->transliterate(
                                *rComp.pVal[0], ScGlobal::eLnge, 0,
                                rComp.pVal[0]->getLength(), NULL));
                    OUString aQuer( pTransliteration->transliterate(
                                *rComp.pVal[1], ScGlobal::eLnge, 0,
                                rComp.pVal[1]->getLength(), NULL));
                    bMatch = (aCell.indexOf( aQuer ) != -1);
                }
                fRes = (bMatch ? 0 : 1);
            }
            else if (pOptions->bIgnoreCase)
                fRes = (double) ScGlobal::GetCollator()->compareString(
                        *rComp.pVal[ 0 ], *rComp.pVal[ 1 ] );
            else
                fRes = (double) ScGlobal::GetCaseCollator()->compareString(
                        *rComp.pVal[ 0 ], *rComp.pVal[ 1 ] );
        }
        else if (rComp.mbIgnoreCase)
            fRes = (double) ScGlobal::GetCollator()->compareString(
                *rComp.pVal[ 0 ], *rComp.pVal[ 1 ] );
        else
            fRes = (double) ScGlobal::GetCaseCollator()->compareString(
                *rComp.pVal[ 0 ], *rComp.pVal[ 1 ] );
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
                bool bEqual = (*rComp.pVal[nStringQuery-1]) == rItem.maString.getString();
                // match => fRes=0, else fRes=1
                fRes = (rEntry.eOp == SC_NOT_EQUAL) ? bEqual : !bEqual;
            }
        }
    }

    return fRes;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

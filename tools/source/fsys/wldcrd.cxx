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

#include <tools/wldcrd.hxx>

/** Tests, whether a wildcard in pWild will match for pStr.
 *
 * '*' in pWild means n chars for n > 0.
 * '?' in pWild mean match exactly one character.
 *
 */
bool WildCard::ImpMatch( std::u16string_view aWild, std::u16string_view aStr )
{
    const sal_Unicode* pPosAfterAsterisk = nullptr;
    const sal_Unicode* pWild = aWild.data();
    const sal_Unicode* pWildEnd = aWild.data() + aWild.size();
    const sal_Unicode* pStr = aStr.data();
    const sal_Unicode* pStrEnd = aStr.data() + aStr.size();

    while (pWild != pWildEnd)
    {
        switch (*pWild)
        {
            case '?':
                if ( pStr == pStrEnd )
                    return false;
                break; // Match -> proceed to the next character
            case '\\': // Escaping '?' and '*'; don't we need to escape '\\'?
                if ((pWild + 1 != pWildEnd) && ((*(pWild + 1) == '?') || (*(pWild + 1) == '*')))
                    pWild++;
                [[fallthrough]];
            default: // No wildcard, literal match
                if (pStr == pStrEnd)
                    return false;
                if (*pWild == *pStr)
                    break; // Match -> proceed to the next character
                if (!pPosAfterAsterisk)
                    return false;
                pWild = pPosAfterAsterisk;
                [[fallthrough]];
            case '*':
                while ( pWild != pWildEnd && *pWild == '*' )
                    pWild++;
                if ( pWild == pWildEnd )
                    return true;
                // Consider strange things like "**?*?*"
                while (*pWild == '?')
                {
                    if (pStr == pStrEnd)
                        return false;
                    pWild++;
                    pStr++;
                    while (pWild != pWildEnd && *pWild == '*')
                        pWild++;
                    if (pWild == pWildEnd)
                        return true;
                }
                // At this point, we are past wildcards, and a literal match must follow
                if ( pStr == pStrEnd )
                    return false;
                pPosAfterAsterisk = pWild;
                if ((*pWild == '\\') && (pWild + 1 != pWildEnd) && ((*(pWild + 1) == '?') || (*(pWild + 1) == '*')))
                    pWild++;
                while (*pStr != *pWild)
                {
                    pStr++;
                    if ( pStr == pStrEnd )
                        return false;
                }
                break; // Match -> proceed to the next character
        }
        // We arrive here when the current characters in pWild and pStr match
        assert(pWild != pWildEnd);
        pWild++;
        assert(pStr != pStrEnd);
        pStr++;
        if (pWild == pWildEnd && pPosAfterAsterisk && pStr != pStrEnd)
            pWild = pPosAfterAsterisk; // Try again on the rest of pStr
    }
    assert(pWild == pWildEnd);
    return pStr == pStrEnd;
}

bool WildCard::Matches( std::u16string_view rString ) const
{
    std::u16string_view aTmpWild = aWildString;

    size_t nSepPos;

    if ( cSepSymbol != '\0' )
    {
        while ( (nSepPos = aTmpWild.find(cSepSymbol)) != std::u16string_view::npos )
        {
            // Check all split wildcards
            if ( ImpMatch( aTmpWild.substr( 0, nSepPos ), rString ) )
                return true;
            aTmpWild = aTmpWild.substr(nSepPos + 1); // remove separator
        }
    }

    return ImpMatch( aTmpWild, rString );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

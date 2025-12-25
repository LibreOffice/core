/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <i18nutil/guessparadirection.hxx>
#include <i18nutil/unicode.hxx>
#include <unicode/uchar.h>
#include <unicode/ubidi.h>

i18nutil::ParagraphDirection i18nutil::GuessParagraphDirection(const OUString& rText)
{
    // tdf#162120: This algorithm implements Unicode UAX #9 3.3.1
    // for automatically determining the paragraph level without
    // override from a higher-level protocol.
    // The algorithm has been extended to explicitly communicate
    // an ambiguous case, rather than defaulting to LTR.

    sal_Int32 nBase = 0;

    sal_Int32 nIsolateLevel = 0;
    while (nBase < rText.getLength())
    {
        auto nChar = rText.iterateCodePoints(&nBase);
        auto nCharDir = u_charDirection(nChar);

        switch (nCharDir)
        {
            case U_POP_DIRECTIONAL_ISOLATE:
                nIsolateLevel = std::max(sal_Int32{ 0 }, nIsolateLevel - 1);
                break;

            case U_LEFT_TO_RIGHT_ISOLATE:
            case U_RIGHT_TO_LEFT_ISOLATE:
                ++nIsolateLevel;
                break;

            case U_LEFT_TO_RIGHT:
                if (nIsolateLevel == 0)
                {
                    return ParagraphDirection::LeftToRight;
                }
                break;

            case U_RIGHT_TO_LEFT:
            case U_RIGHT_TO_LEFT_ARABIC:
                if (nIsolateLevel == 0)
                {
                    return ParagraphDirection::RightToLeft;
                }
                break;

            default:
                break;
        }
    }

    return ParagraphDirection::Ambiguous;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

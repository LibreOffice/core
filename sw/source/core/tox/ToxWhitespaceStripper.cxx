/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ToxWhitespaceStripper.hxx"

#include "rtl/ustrbuf.hxx"
#include "sal/log.hxx"

#include <boost/numeric/conversion/cast.hpp>

namespace sw {

ToxWhitespaceStripper::ToxWhitespaceStripper(const OUString& inputString)
{
    OUStringBuffer buffer;

    bool lastCharacterWasWhitespace = false;
    for (sal_Int32 pos = 0; pos < inputString.getLength(); ++pos) {
        sal_Unicode cur = inputString[pos];

        if (cur == ' ' || cur == '\n' || cur == '\t') {
            // merge consecutive whitespaces (and translate them to spaces)
            if (!lastCharacterWasWhitespace) {
                buffer.append(' ');
            }
            lastCharacterWasWhitespace = true;
        }
        else {
            buffer.append(cur);
            lastCharacterWasWhitespace = false;
        }
        mNewPositions.push_back(buffer.getLength()-1);
    }
    // Add one position if the position after the stripped string is requested, e.g., for attributes which
    // extend beyond the string.
    mNewPositions.push_back(buffer.getLength());
    // strip the last whitespace (if there was one)
    if (lastCharacterWasWhitespace) {
        buffer.truncate(buffer.getLength() - 1);
    }
    mStripped = buffer.getStr();
}


sal_Int32
ToxWhitespaceStripper::GetPositionInStrippedString(sal_Int32 pos) const
{
    size_t upos = boost::numeric_cast<size_t>(pos);
    if (upos >= mNewPositions.size()) {
        SAL_WARN("sw.core", "Requested position of TOX entry text which does not exist. "
                            "Maybe the formatting hint is corrupt?");
        return mNewPositions.back();
    }
    return mNewPositions.at(upos);
}


}

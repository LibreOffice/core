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

#include <com/sun/star/i18n/ScriptDirection.hpp>
#include <com/sun/star/i18n/UnicodeScript.hpp>
#include <i18nutil/scripttypedetector.hxx>
#include <i18nutil/unicode.hxx>
#include <o3tl/safeint.hxx>

using namespace com::sun::star::i18n;

const sal_Int16 scriptDirection[] = {
    ScriptDirection::LEFT_TO_RIGHT,     // DirectionProperty_LEFT_TO_RIGHT = 0,
    ScriptDirection::RIGHT_TO_LEFT,     // DirectionProperty_RIGHT_TO_LEFT = 1,
    ScriptDirection::LEFT_TO_RIGHT,     // DirectionProperty_EUROPEAN_NUMBER = 2,
    ScriptDirection::LEFT_TO_RIGHT,     // DirectionProperty_EUROPEAN_NUMBER_SEPARATOR = 3,
    ScriptDirection::LEFT_TO_RIGHT,     // DirectionProperty_EUROPEAN_NUMBER_TERMINATOR = 4,
    ScriptDirection::RIGHT_TO_LEFT,     // DirectionProperty_ARABIC_NUMBER = 5,
    ScriptDirection::NEUTRAL,           // DirectionProperty_COMMON_NUMBER_SEPARATOR = 6,
    ScriptDirection::NEUTRAL,           // DirectionProperty_BLOCK_SEPARATOR = 7,
    ScriptDirection::NEUTRAL,           // DirectionProperty_SEGMENT_SEPARATOR = 8,
    ScriptDirection::NEUTRAL,           // DirectionProperty_WHITE_SPACE_NEUTRAL = 9,
    ScriptDirection::NEUTRAL,           // DirectionProperty_OTHER_NEUTRAL = 10,
    ScriptDirection::LEFT_TO_RIGHT,     // DirectionProperty_LEFT_TO_RIGHT_EMBEDDING = 11,
    ScriptDirection::LEFT_TO_RIGHT,     // DirectionProperty_LEFT_TO_RIGHT_OVERRIDE = 12,
    ScriptDirection::RIGHT_TO_LEFT,     // DirectionProperty_RIGHT_TO_LEFT_ARABIC = 13,
    ScriptDirection::RIGHT_TO_LEFT,     // DirectionProperty_RIGHT_TO_LEFT_EMBEDDING = 14,
    ScriptDirection::RIGHT_TO_LEFT,     // DirectionProperty_RIGHT_TO_LEFT_OVERRIDE = 15,
    ScriptDirection::NEUTRAL,           // DirectionProperty_POP_DIRECTIONAL_FORMAT = 16,
    ScriptDirection::NEUTRAL,           // DirectionProperty_DIR_NON_SPACING_MARK = 17,
    ScriptDirection::NEUTRAL,           // DirectionProperty_BOUNDARY_NEUTRAL = 18,
};

sal_Int16 ScriptTypeDetector::getScriptDirection( std::u16string_view Text, sal_Int32 nPos, sal_Int16 defaultScriptDirection )
{
    sal_Int16 dir = scriptDirection[unicode::getUnicodeDirection(Text[nPos])];
    return (dir == ScriptDirection::NEUTRAL) ? defaultScriptDirection : dir;
}

// return value '-1' means either the direction on nPos is not same as scriptDirection or nPos is out of range.
sal_Int32 ScriptTypeDetector::beginOfScriptDirection( std::u16string_view Text, sal_Int32 nPos, sal_Int16 direction )
{
        sal_Int32 cPos = nPos;

        if (cPos < static_cast<sal_Int32>(Text.size())) {
            for (; cPos >= 0; cPos--) {
                if (direction != getScriptDirection(Text, cPos, direction))
                    break;
            }
        }
        return cPos == nPos ? -1 : cPos + 1;
}

sal_Int32 ScriptTypeDetector::endOfScriptDirection( std::u16string_view Text, sal_Int32 nPos, sal_Int16 direction )
{
        sal_Int32 cPos = nPos;
        sal_Int32 len = Text.size();

        if (cPos >=0) {
            for (; cPos < len; cPos++) {
                if (direction != getScriptDirection(Text, cPos, direction))
                    break;
            }
        }
        return cPos == nPos ? -1 : cPos;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

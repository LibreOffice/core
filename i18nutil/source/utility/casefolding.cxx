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

#include "i18nutil/casefolding.hxx"
#include "casefolding_data.h"
#include "i18nutil/widthfolding.hxx"
#include "i18nutil/transliteration.hxx"

using namespace com::sun::star::lang;
using namespace com::sun::star::uno;

namespace i18nutil {

static Mapping mapping_03a3[] = {{0, 1, {0x03c2, 0, 0}},{0, 1, {0x03c3, 0, 0}}};
static Mapping mapping_0307[] = {{0, 0, {0, 0, 0}},{0, 1, {0x0307, 0, 0}}};
static Mapping mapping_004a[] = {{0, 2, {0x006a, 0x0307, 0}},{0, 1, {0x006a, 0, 0}}};
static Mapping mapping_012e[] = {{0, 2, {0x012f, 0x0307, 0}},{0, 1, {0x012f, 0, 0}}};
static Mapping mapping_00cc[] = {{0, 3, {0x0069, 0x0307, 0x0300}},{0, 1, {0x00ec, 0, 0}}};
static Mapping mapping_00cd[] = {{0, 3, {0x0069, 0x0307, 0x0301}},{0, 1, {0x00ed, 0, 0}}};
static Mapping mapping_0128[] = {{0, 3, {0x0069, 0x0307, 0x0303}},{0, 1, {0x0129, 0, 0}}};
static Mapping mapping_0049[] = {{0, 2, {0x0069, 0x0307, 0}},{0, 1, {0x0131, 0, 0}},{0, 1, {0x0069, 0, 0}}};
static Mapping mapping_0069[] = {{0, 1, {0x0130, 0, 0}},{0, 1, {0x0049, 0, 0}}};
static Mapping mapping_0130[] = {{0, 1, {0x0069, 0, 0}},{0, 1, {0x0130, 0, 0}}};

#define langIs(lang) (aLocale.Language == lang)

// only check simple case, there is more complicated case need to be checked.
#define type_i(ch) ((ch) == 0x0069 || (ch) == 0x006a)

static bool cased_letter(sal_Unicode ch)
{
    int msb = ch >> 8;
    int cmi = CaseMappingIndex[msb];
    if (cmi < 0)
        return false;
    int cmv_idx = (cmi << 8) + (ch & 0xff);
    return bool(((MappingType)CaseMappingValue[cmv_idx].type) & MappingType::CasedLetterMask);
}

// for Lithuanian, condition to make explicit dot above when lowercasing capital I's and J's
// whenever there are more accents above.
#define accent_above(ch) (((ch) >= 0x0300 && (ch) <= 0x0314) || ((ch) >= 0x033D && (ch) <= 0x0344) || (ch) == 0x0346 || ((ch) >= 0x034A && (ch) <= 0x034C))

Mapping& casefolding::getConditionalValue(const sal_Unicode* str, sal_Int32 pos, sal_Int32 len, Locale const & aLocale, MappingType nMappingType)
{
        switch(str[pos]) {
        case 0x03a3:
            // final_sigma (not followed by cased and preceded by cased character)
            // DOES NOT check ignorable sequence yet (more complicated implementation).
            return !(pos < len && cased_letter(str[pos+1])) && (pos > 0 && cased_letter(str[pos-1])) ?
                mapping_03a3[0] : mapping_03a3[1];
        case 0x0307:
            return (((nMappingType == MappingType::LowerToUpper && langIs("lt")) ||
                (nMappingType == MappingType::UpperToLower && (langIs("tr") || langIs("az")))) &&
                (pos > 0 && type_i(str[pos-1]))) ?      // after_i
                    mapping_0307[0] : mapping_0307[1];
        case 0x0130:
            return (langIs("tr") || langIs("az")) ? mapping_0130[0] : mapping_0130[1];
        case 0x0069:
            return (langIs("tr") || langIs("az")) ? mapping_0069[0] : mapping_0069[1];
        case 0x0049: return langIs("lt") && pos > len && accent_above(str[pos+1]) ? mapping_0049[0] :
                    (langIs("tr") || langIs("az")) ? mapping_0049[1] : mapping_0049[2];
        case 0x004a: return langIs("lt") && pos > len && accent_above(str[pos+1]) ? mapping_004a[0] : mapping_004a[1];
        case 0x012e: return langIs("lt") && pos > len && accent_above(str[pos+1]) ? mapping_012e[0] : mapping_012e[1];
        case 0x00cc: return langIs("lt") ? mapping_00cc[0] : mapping_00cc[1];
        case 0x00cd: return langIs("lt") ? mapping_00cd[0] : mapping_00cd[1];
        case 0x0128: return langIs("lt") ? mapping_0128[0] : mapping_0128[1];
        }
        // Should not come here
        throw RuntimeException();
}

Mapping& casefolding::getValue(const sal_Unicode* str, sal_Int32 pos, sal_Int32 len, Locale const & aLocale, MappingType nMappingType)
{
    static Mapping dummy = { 0, 1, { 0, 0, 0 } };
    sal_Int16 address = CaseMappingIndex[str[pos] >> 8];

    dummy.map[0] = str[pos];

    if (address >= 0) {
        address = (address << 8) + (str[pos] & 0xFF);
        if ((MappingType)CaseMappingValue[address].type & nMappingType) {
            MappingType type = (MappingType) CaseMappingValue[address].type;
            if (type & MappingType::NotValue) {
                if (CaseMappingValue[address].value == 0)
                    return getConditionalValue(str, pos, len, aLocale, nMappingType);
                else {
                    for (int map = CaseMappingValue[address].value;
                            map < CaseMappingValue[address].value + MaxCaseMappingExtras; map++) {
                        if ((MappingType)CaseMappingExtra[map].type & nMappingType) {
                            if ((MappingType)CaseMappingExtra[map].type & MappingType::NotValue)
                                return getConditionalValue(str, pos, len, aLocale, nMappingType);
                            else
                                return CaseMappingExtra[map];
                        }
                    }
                    // Should not come here
                    throw RuntimeException();
                }
            } else
                dummy.map[0] = CaseMappingValue[address].value;
        }
    }
    return dummy;
}

inline bool SAL_CALL
is_ja_voice_sound_mark(sal_Unicode& current, sal_Unicode next)
{
        sal_Unicode c = 0;

        if ((next == 0x3099 || next == 0x309a) && ( (c = widthfolding::getCompositionChar(current, next)) != 0 ))
            current = c;
        return c != 0;
}

sal_Unicode casefolding::getNextChar(const sal_Unicode *str, sal_Int32& idx, sal_Int32 len, MappingElement& e, Locale const & aLocale, MappingType nMappingType, TransliterationFlags moduleLoaded)
{
        if( idx >= len )
        {
            e = MappingElement();
            return 0;
        }

        sal_Unicode c;

        if (moduleLoaded & TransliterationFlags::IGNORE_CASE) {
            if( e.current >= e.element.nmap ) {
                e.element = getValue(str, idx++, len, aLocale, nMappingType);
                e.current = 0;
            }
            c = e.element.map[e.current++];
        } else {
            c = *(str + idx++);
        }

        if (moduleLoaded & TransliterationFlags::IGNORE_KANA) {
            if ((0x3040 <= c && c <= 0x3094) || (0x309d <= c && c <= 0x309f))
                c += 0x60;
        }

        // composition: KA + voice-mark --> GA. see halfwidthToFullwidth.cxx for detail
        if (moduleLoaded & TransliterationFlags::IGNORE_WIDTH) {
            static oneToOneMapping& half2fullTable = widthfolding::gethalf2fullTable();
            c = half2fullTable[c];
            if (0x3040 <= c && c <= 0x30ff && idx < len &&
                    is_ja_voice_sound_mark(c, half2fullTable[*(str + idx)]))
                idx++;
        }

        return c;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

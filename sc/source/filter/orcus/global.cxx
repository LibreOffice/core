/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus_global.hxx>

#include <frozen/unordered_map.h>

using namespace com::sun::star;

namespace os = orcus::spreadsheet;

formula::FormulaGrammar::Grammar getCalcGrammarFromOrcus(os::formula_grammar_t grammar)
{
    formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_ODFF;
    switch (grammar)
    {
        case orcus::spreadsheet::formula_grammar_t::ods:
            eGrammar = formula::FormulaGrammar::GRAM_ODFF;
            break;
        case orcus::spreadsheet::formula_grammar_t::xlsx:
            eGrammar = formula::FormulaGrammar::GRAM_OOXML;
            break;
        case orcus::spreadsheet::formula_grammar_t::gnumeric:
            eGrammar = formula::FormulaGrammar::GRAM_ENGLISH_XL_A1;
            break;
        case orcus::spreadsheet::formula_grammar_t::xls_xml:
            eGrammar = formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1;
            break;
        case orcus::spreadsheet::formula_grammar_t::unknown:
            break;
    }

    return eGrammar;
}

ScOrcusGlobalSettings::ScOrcusGlobalSettings(ScDocumentImport& rDoc)
    : mrDoc(rDoc)
    , meCalcGrammar(formula::FormulaGrammar::GRAM_UNSPECIFIED)
    , meOrcusGrammar(os::formula_grammar_t::unknown)
    , mnTextEncoding(RTL_TEXTENCODING_UTF8)
{
}

void ScOrcusGlobalSettings::set_origin_date(int year, int month, int day)
{
    mrDoc.setOriginDate(year, month, day);
}

void ScOrcusGlobalSettings::set_character_set(orcus::character_set_t cs)
{
    // Keep the entries sorted by the key.
    static constexpr auto rules
        = frozen::make_unordered_map<orcus::character_set_t, rtl_TextEncoding>({
            { orcus::character_set_t::big5, RTL_TEXTENCODING_BIG5 },
            { orcus::character_set_t::euc_jp, RTL_TEXTENCODING_EUC_JP },
            { orcus::character_set_t::euc_kr, RTL_TEXTENCODING_EUC_KR },
            { orcus::character_set_t::gb2312, RTL_TEXTENCODING_GB_2312 },
            { orcus::character_set_t::gbk, RTL_TEXTENCODING_GBK },
            { orcus::character_set_t::iso_2022_cn, RTL_TEXTENCODING_ISO_2022_CN },
            { orcus::character_set_t::iso_2022_cn_ext, RTL_TEXTENCODING_ISO_2022_CN },
            { orcus::character_set_t::iso_2022_jp, RTL_TEXTENCODING_ISO_2022_JP },
            { orcus::character_set_t::iso_2022_jp_2, RTL_TEXTENCODING_ISO_2022_JP },
            { orcus::character_set_t::iso_8859_1, RTL_TEXTENCODING_ISO_8859_1 },
            { orcus::character_set_t::iso_8859_14, RTL_TEXTENCODING_ISO_8859_14 },
            { orcus::character_set_t::iso_8859_15, RTL_TEXTENCODING_ISO_8859_15 },
            { orcus::character_set_t::iso_8859_1_windows_3_0_latin_1, RTL_TEXTENCODING_ISO_8859_1 },
            { orcus::character_set_t::iso_8859_1_windows_3_1_latin_1, RTL_TEXTENCODING_ISO_8859_1 },
            { orcus::character_set_t::iso_8859_2, RTL_TEXTENCODING_ISO_8859_2 },
            { orcus::character_set_t::iso_8859_2_windows_latin_2, RTL_TEXTENCODING_ISO_8859_2 },
            { orcus::character_set_t::iso_8859_3, RTL_TEXTENCODING_ISO_8859_3 },
            { orcus::character_set_t::iso_8859_4, RTL_TEXTENCODING_ISO_8859_4 },
            { orcus::character_set_t::iso_8859_5, RTL_TEXTENCODING_ISO_8859_5 },
            { orcus::character_set_t::iso_8859_6, RTL_TEXTENCODING_ISO_8859_6 },
            { orcus::character_set_t::iso_8859_6_e, RTL_TEXTENCODING_ISO_8859_6 },
            { orcus::character_set_t::iso_8859_6_i, RTL_TEXTENCODING_ISO_8859_6 },
            { orcus::character_set_t::iso_8859_7, RTL_TEXTENCODING_ISO_8859_7 },
            { orcus::character_set_t::iso_8859_8, RTL_TEXTENCODING_ISO_8859_8 },
            { orcus::character_set_t::iso_8859_8_e, RTL_TEXTENCODING_ISO_8859_8 },
            { orcus::character_set_t::iso_8859_8_i, RTL_TEXTENCODING_ISO_8859_8 },
            { orcus::character_set_t::iso_8859_9, RTL_TEXTENCODING_ISO_8859_9 },
            { orcus::character_set_t::iso_8859_9_windows_latin_5, RTL_TEXTENCODING_ISO_8859_9 },
            { orcus::character_set_t::jis_x0201, RTL_TEXTENCODING_JIS_X_0201 },
            { orcus::character_set_t::jis_x0212_1990, RTL_TEXTENCODING_JIS_X_0212 },
            { orcus::character_set_t::shift_jis, RTL_TEXTENCODING_SHIFT_JIS },
            { orcus::character_set_t::us_ascii, RTL_TEXTENCODING_ASCII_US },
            { orcus::character_set_t::utf_7, RTL_TEXTENCODING_UTF7 },
            { orcus::character_set_t::utf_8, RTL_TEXTENCODING_UTF8 },
            { orcus::character_set_t::windows_1250, RTL_TEXTENCODING_MS_1250 },
            { orcus::character_set_t::windows_1251, RTL_TEXTENCODING_MS_1251 },
            { orcus::character_set_t::windows_1252, RTL_TEXTENCODING_MS_1252 },
            { orcus::character_set_t::windows_1253, RTL_TEXTENCODING_MS_1253 },
            { orcus::character_set_t::windows_1254, RTL_TEXTENCODING_MS_1254 },
            { orcus::character_set_t::windows_1255, RTL_TEXTENCODING_MS_1255 },
            { orcus::character_set_t::windows_1256, RTL_TEXTENCODING_MS_1256 },
            { orcus::character_set_t::windows_1257, RTL_TEXTENCODING_MS_1257 },
            { orcus::character_set_t::windows_1258, RTL_TEXTENCODING_MS_1258 },
        });

    if (auto it = rules.find(cs); it != rules.end())
        mnTextEncoding = it->second;
}

void ScOrcusGlobalSettings::set_default_formula_grammar(os::formula_grammar_t grammar)
{
    meCalcGrammar = getCalcGrammarFromOrcus(grammar);
    meOrcusGrammar = grammar;
}

orcus::spreadsheet::formula_grammar_t ScOrcusGlobalSettings::get_default_formula_grammar() const
{
    return meOrcusGrammar;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

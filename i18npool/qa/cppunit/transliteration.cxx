/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <com/sun/star/i18n/Transliteration.hpp>
#include <com/sun/star/i18n/TransliterationModulesNew.hpp>
#include <cppuhelper/bootstrap.hxx>

namespace
{
class Transliteration : public CppUnit::TestFixture
{
public:
    void testLoadModuleNew()
    {
        auto const trans
            = css::i18n::Transliteration::create(cppu::defaultBootstrap_InitialComponentContext());
        // Verify that loading succeeds without throwing an exception, for each possible
        // TransliterationModulesNew value (TODO: there is an upper limit of maxCascade 27 in
        // i18npool/inc/transliterationImpl.hxx for the length of the passed
        // TransliterationModulesNew value, so pass each one individually rather than all 65 at
        // once):
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_UPPERCASE_LOWERCASE }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_LOWERCASE_UPPERCASE }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_HALFWIDTH_FULLWIDTH }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_FULLWIDTH_HALFWIDTH }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_KATAKANA_HIRAGANA }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_HIRAGANA_KATAKANA }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_IGNORE_CASE }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_IGNORE_KANA }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_IGNORE_WIDTH }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_IgnoreTraditionalKanji_ja_JP },
                             {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_IgnoreTraditionalKana_ja_JP },
                             {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_IgnoreMinusSign_ja_JP }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_IgnoreIterationMark_ja_JP },
                             {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_IgnoreSeparator_ja_JP }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_IgnoreZiZu_ja_JP }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_IgnoreBaFa_ja_JP }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_IgnoreTiJi_ja_JP }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_IgnoreHyuByu_ja_JP }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_IgnoreSeZe_ja_JP }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_IgnoreIandEfollowedByYa_ja_JP },
                             {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_IgnoreKiKuFollowedBySa_ja_JP },
                             {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_IgnoreSize_ja_JP }, {});
        trans->loadModuleNew(
            { css::i18n::TransliterationModulesNew_IgnoreProlongedSoundMark_ja_JP }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_IgnoreMiddleDot_ja_JP }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_IgnoreSpace_ja_JP }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_SmallToLarge_ja_JP }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_LargeToSmall_ja_JP }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_NumToTextLower_zh_CN }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_NumToTextUpper_zh_CN }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_NumToTextLower_zh_TW }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_NumToTextUpper_zh_TW }, {});
#if 0 //TODO: currently broken
        trans->loadModuleNew({css::i18n::TransliterationModulesNew_NumToTextFormalHangul_ko}, {});
#endif
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_NumToTextFormalLower_ko }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_NumToTextFormalUpper_ko }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_NumToTextInformalHangul_ko },
                             {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_NumToTextInformalLower_ko },
                             {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_NumToTextInformalUpper_ko },
                             {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_NumToCharLower_zh_CN }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_NumToCharUpper_zh_CN }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_NumToCharLower_zh_TW }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_NumToCharUpper_zh_TW }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_NumToCharHangul_ko }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_NumToCharLower_ko }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_NumToCharUpper_ko }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_NumToCharFullwidth }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_NumToCharKanjiShort_ja_JP },
                             {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_TextToNumLower_zh_CN }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_TextToNumUpper_zh_CN }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_TextToNumLower_zh_TW }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_TextToNumUpper_zh_TW }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_TextToNumFormalHangul_ko }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_TextToNumFormalLower_ko }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_TextToNumFormalUpper_ko }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_TextToNumInformalHangul_ko },
                             {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_TextToNumInformalLower_ko },
                             {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_TextToNumInformalUpper_ko },
                             {});
#if 0 //TODO: currently broken
        trans->loadModuleNew({css::i18n::TransliterationModulesNew_CharToNumLower_zh_CN}, {});
#endif
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_CharToNumUpper_zh_CN }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_CharToNumLower_zh_TW }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_CharToNumUpper_zh_TW }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_CharToNumHangul_ko }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_CharToNumLower_ko }, {});
        trans->loadModuleNew({ css::i18n::TransliterationModulesNew_CharToNumUpper_ko }, {});
    }

    CPPUNIT_TEST_SUITE(Transliteration);
    CPPUNIT_TEST(testLoadModuleNew);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Transliteration);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

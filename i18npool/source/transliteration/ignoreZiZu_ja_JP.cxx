/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"

// prevent internal compiler error with MSVC6SP3
#include <utility>

#define TRANSLITERATION_ZiZu_ja_JP
#include <transliteration_Ignore.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

sal_Unicode
ignoreZiZu_ja_JP_translator (const sal_Unicode c)
{

    switch (c) {
        case 0x30C2:     // KATAKANA LETTER DI
            return 0x30B8; // KATAKANA LETTER ZI

        case 0x3062:     // HIRAGANA LETTER DI
            return 0x3058; // HIRAGANA LETTER ZI

        case 0x30C5:     // KATAKANA LETTER DU
            return 0x30BA; // KATAKANA LETTER ZU

        case 0x3065:     // HIRAGANA LETTER DU
            return 0x305A; // HIRAGANA LETTER ZU
    }
    return c;
}

ignoreZiZu_ja_JP::ignoreZiZu_ja_JP()
{
        func = ignoreZiZu_ja_JP_translator;
        table = 0;
        map = 0;
        transliterationName = "ignoreZiZu_ja_JP";
        implementationName = "com.sun.star.i18n.Transliteration.ignoreZiZu_ja_JP";
}

} } } }

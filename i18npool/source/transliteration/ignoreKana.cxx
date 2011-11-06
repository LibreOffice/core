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

#define TRANSLITERATION_Kana
#include <transliteration_Ignore.hxx>
#define TRANSLITERATION_hiraganaToKatakana
#define TRANSLITERATION_katakanaToHiragana
#include <transliteration_OneToOne.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

OUString SAL_CALL
ignoreKana::folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
  throw(RuntimeException)
{
    hiraganaToKatakana t1;
    return t1.transliterate(inStr, startPos, nCount, offset);
}

Sequence< OUString > SAL_CALL
ignoreKana::transliterateRange( const OUString& str1, const OUString& str2 )
  throw(RuntimeException)
{
    hiraganaToKatakana t1;
    katakanaToHiragana t2;

    return transliteration_Ignore::transliterateRange(str1, str2, t1, t2);
}

sal_Unicode SAL_CALL
ignoreKana::transliterateChar2Char( sal_Unicode inChar) throw(RuntimeException, MultipleCharsOutputException)
{
    hiraganaToKatakana t1;
    return t1.transliterateChar2Char(inChar);
}

} } } }

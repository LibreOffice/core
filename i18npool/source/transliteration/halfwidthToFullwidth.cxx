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

#include <i18nutil/widthfolding.hxx>
#define TRANSLITERATION_halfwidthToFullwidth
#define TRANSLITERATION_halfwidthKatakanaToFullwidthKatakana
#define TRANSLITERATION_halfwidthToFullwidthLikeJIS
#include <transliteration_OneToOne.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

halfwidthToFullwidth::halfwidthToFullwidth()
{
    func = (TransFunc) 0;
    table = &widthfolding::gethalf2fullTable();
    transliterationName = "halfwidthToFullwidth";
    implementationName = "com.sun.star.i18n.Transliteration.HALFWIDTH_FULLWIDTH";
}

OUString SAL_CALL
halfwidthToFullwidth::transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
  throw(RuntimeException)
{
    sal_Bool _useOffset = useOffset;
    // One to One mapping
    useOffset = sal_False;
    const OUString& newStr = transliteration_OneToOne::transliterate( inStr, startPos, nCount, offset);
    useOffset = _useOffset;

    // Composition: KA + voice-mark --> GA
    return widthfolding::compose_ja_voiced_sound_marks ( newStr, 0, newStr.getLength(), offset, _useOffset );
}

sal_Unicode SAL_CALL
halfwidthToFullwidth::transliterateChar2Char( sal_Unicode inChar)
  throw(RuntimeException, MultipleCharsOutputException)
{
    return transliteration_OneToOne::transliterateChar2Char(inChar);
}


halfwidthKatakanaToFullwidthKatakana::halfwidthKatakanaToFullwidthKatakana()
{
    func = (TransFunc) 0;
    table = &widthfolding::gethalfKana2fullKanaTable();
    transliterationName = "halfwidthKatakanaToFullwidthKatakana";
    implementationName = "com.sun.star.i18n.Transliteration.HALFWIDTHKATAKANA_FULLWIDTHKATAKANA";
}

OUString SAL_CALL
halfwidthKatakanaToFullwidthKatakana::transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
  throw(RuntimeException)
{
    sal_Bool _useOffset = useOffset;
    // One to One mapping
    useOffset = sal_False;
    const OUString& newStr = transliteration_OneToOne::transliterate( inStr, startPos, nCount, offset);
    useOffset = _useOffset;

    // Composition: KA + voice-mark --> GA
    return widthfolding::compose_ja_voiced_sound_marks ( newStr, 0, newStr.getLength(), offset, _useOffset );
}

sal_Unicode SAL_CALL
halfwidthKatakanaToFullwidthKatakana::transliterateChar2Char( sal_Unicode inChar)
  throw(RuntimeException, MultipleCharsOutputException)
{
    return transliteration_OneToOne::transliterateChar2Char(inChar);
}


halfwidthToFullwidthLikeJIS::halfwidthToFullwidthLikeJIS()
{
    func = (TransFunc) 0;
    table = &widthfolding::gethalf2fullTableForJIS();
    transliterationName = "halfwidthToFullwidthLikeJIS";
    implementationName = "com.sun.star.i18n.Transliteration.HALFWIDTH_FULLWIDTH_LIKE_JIS";
}

OUString SAL_CALL
halfwidthToFullwidthLikeJIS::transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
  throw(RuntimeException)
{
    sal_Bool _useOffset = useOffset;
    // One to One mapping
    useOffset = sal_False;
    const OUString& newStr = transliteration_OneToOne::transliterate( inStr, startPos, nCount, offset);
    useOffset = _useOffset;

    // Composition: KA + voice-mark --> GA
    return widthfolding::compose_ja_voiced_sound_marks ( newStr, 0, newStr.getLength(), offset, _useOffset, WIDTHFOLDNIG_DONT_USE_COMBINED_VU );
}

sal_Unicode SAL_CALL
halfwidthToFullwidthLikeJIS::transliterateChar2Char( sal_Unicode inChar)
  throw(RuntimeException, MultipleCharsOutputException)
{
    return transliteration_OneToOne::transliterateChar2Char(inChar);
}

} } } }

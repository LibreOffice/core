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


#ifndef _L10N_TRANSLITERATION_NUMTOTEXT_CJK_HXX_
#define _L10N_TRANSLITERATION_NUMTOTEXT_CJK_HXX_

#include <transliteration_Numeric.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

#define TRANSLITERATION_NUMTOTEXT_CJK( name ) \
class NumToText##name : public transliteration_Numeric \
{ \
public: \
        NumToText##name (); \
};

#ifdef TRANSLITERATION_ALL
TRANSLITERATION_NUMTOTEXT_CJK ( Lower_zh_CN )
TRANSLITERATION_NUMTOTEXT_CJK ( Upper_zh_CN )
TRANSLITERATION_NUMTOTEXT_CJK ( Lower_zh_TW )
TRANSLITERATION_NUMTOTEXT_CJK ( Upper_zh_TW )
TRANSLITERATION_NUMTOTEXT_CJK ( Fullwidth_zh_CN )
TRANSLITERATION_NUMTOTEXT_CJK ( Fullwidth_zh_TW )
TRANSLITERATION_NUMTOTEXT_CJK ( Fullwidth_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( Fullwidth_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( Date_zh )
TRANSLITERATION_NUMTOTEXT_CJK ( InformalLower_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( InformalUpper_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( InformalHangul_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( FormalLower_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( FormalUpper_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( FormalHangul_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( KanjiLongModern_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( KanjiLongTraditional_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( KanjiShortModern_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( KanjiShortTraditional_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( AIUFullWidth_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( AIUHalfWidth_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( IROHAFullWidth_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( IROHAHalfWidth_ja_JP )
TRANSLITERATION_NUMTOTEXT_CJK ( CircledNumber )
TRANSLITERATION_NUMTOTEXT_CJK ( TianGan_zh )
TRANSLITERATION_NUMTOTEXT_CJK ( DiZi_zh )
TRANSLITERATION_NUMTOTEXT_CJK ( HangulJamo_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( HangulSyllable_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( HangulCircledJamo_ko )
TRANSLITERATION_NUMTOTEXT_CJK ( HangulCircledSyllable_ko )
#endif
#undef TRANSLITERATION_NUMTOTEXT

} } } }

#endif // _L10N_TRANSLITERATION_NUMTOTEXT_HXX_

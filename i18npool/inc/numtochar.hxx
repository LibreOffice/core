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


#ifndef _L10N_TRANSLITERATION_NUMTOCHAR_HXX_
#define _L10N_TRANSLITERATION_NUMTOCHAR_HXX_

#include <transliteration_Numeric.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

#define TRANSLITERATION_NUMTOCHAR( name ) \
class NumToChar##name : public transliteration_Numeric \
{ \
public: \
        NumToChar##name (); \
};

#ifdef TRANSLITERATION_ALL
TRANSLITERATION_NUMTOCHAR(Lower_zh_CN)
TRANSLITERATION_NUMTOCHAR(Upper_zh_CN)
TRANSLITERATION_NUMTOCHAR(Lower_zh_TW)
TRANSLITERATION_NUMTOCHAR(Upper_zh_TW)
TRANSLITERATION_NUMTOCHAR(Upper_ko)
TRANSLITERATION_NUMTOCHAR(Hangul_ko)
TRANSLITERATION_NUMTOCHAR(Lower_ko)
TRANSLITERATION_NUMTOCHAR(KanjiShort_ja_JP)
TRANSLITERATION_NUMTOCHAR(KanjiTraditional_ja_JP)
TRANSLITERATION_NUMTOCHAR(Fullwidth)
TRANSLITERATION_NUMTOCHAR(Indic_ar)
TRANSLITERATION_NUMTOCHAR(EastIndic_ar)
TRANSLITERATION_NUMTOCHAR(Indic_hi)
TRANSLITERATION_NUMTOCHAR(_th)
TRANSLITERATION_NUMTOCHAR(Halfwidth)
#endif
#undef TRANSLITERATION_NUMTOCHAR

} } } }

#endif // _L10N_TRANSLITERATION_NUMTOCHAR_HXX_

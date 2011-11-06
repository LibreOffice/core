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

#define TRANSLITERATION_ALL
#include <texttonum.hxx>
#include <data/numberchar.h>
#include <rtl/ustrbuf.hxx>

using namespace com::sun::star::uno;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

#define TRANSLITERATION_TEXTTONUM( name ) \
TextToNum##name::TextToNum##name() \
{ \
        nNativeNumberMode = 0; \
        tableSize = 0; \
        transliterationName = "TextToNum"#name; \
        implementationName = "com.sun.star.i18n.Transliteration.TextToNum"#name; \
}

TRANSLITERATION_TEXTTONUM( Lower_zh_CN)
TRANSLITERATION_TEXTTONUM( Upper_zh_CN)
TRANSLITERATION_TEXTTONUM( Lower_zh_TW)
TRANSLITERATION_TEXTTONUM( Upper_zh_TW)
TRANSLITERATION_TEXTTONUM( FormalLower_ko)
TRANSLITERATION_TEXTTONUM( FormalUpper_ko)
TRANSLITERATION_TEXTTONUM( FormalHangul_ko)
TRANSLITERATION_TEXTTONUM( InformalLower_ko)
TRANSLITERATION_TEXTTONUM( InformalUpper_ko)
TRANSLITERATION_TEXTTONUM( InformalHangul_ko)
TRANSLITERATION_TEXTTONUM( KanjiLongTraditional_ja_JP)
TRANSLITERATION_TEXTTONUM( KanjiLongModern_ja_JP)
#undef TRANSLITERATION_TEXTTONUM

} } } }

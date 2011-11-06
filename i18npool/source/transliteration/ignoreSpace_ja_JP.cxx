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

#define TRANSLITERATION_Space_ja_JP
#include <transliteration_Ignore.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

OneToOneMappingTable_t ignoreSpace_ja_JP_mappingTable[] = {
    MAKE_PAIR( 0x0020, 0xffff ),  // SPACE
    MAKE_PAIR( 0x00A0, 0xffff ),  // NO-BREAK SPACE
    MAKE_PAIR( 0x2002, 0xffff ),  // EN SPACE
    MAKE_PAIR( 0x2003, 0xffff ),  // EM SPACE
    MAKE_PAIR( 0x2004, 0xffff ),  // THREE-PER-EM SPACE
    MAKE_PAIR( 0x2005, 0xffff ),  // FOUR-PER-EM SPACE
    MAKE_PAIR( 0x2006, 0xffff ),  // SIX-PER-EM SPACE
    MAKE_PAIR( 0x2007, 0xffff ),  // FIGURE SPACE
    MAKE_PAIR( 0x2008, 0xffff ),  // PUNCTUATION SPACE
    MAKE_PAIR( 0x2009, 0xffff ),  // THIN SPACE
    MAKE_PAIR( 0x200A, 0xffff ),  // HAIR SPACE
    MAKE_PAIR( 0x200B, 0xffff ),  // ZERO WIDTH SPACE
    MAKE_PAIR( 0x202F, 0xffff ),  // NARROW NO-BREAK SPACE
    MAKE_PAIR( 0x3000, 0xffff ),  // IDEOGRAPHIC SPACE
    MAKE_PAIR( 0x303F, 0xffff )   // IDEOGRAPHIC HALF FILL SPACE
};


ignoreSpace_ja_JP::ignoreSpace_ja_JP()
{
        func = (TransFunc)0;
        table = new oneToOneMapping(ignoreSpace_ja_JP_mappingTable, sizeof(ignoreSpace_ja_JP_mappingTable));
        map = 0;
        transliterationName = "ignoreSpace_ja_JP";
        implementationName = "com.sun.star.i18n.Transliteration.ignoreSpace_ja_JP";
}

} } } }

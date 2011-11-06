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

#define TRANSLITERATION_Size_ja_JP
#include <transliteration_Ignore.hxx>
#define TRANSLITERATION_smallToLarge_ja_JP
#define TRANSLITERATION_largeToSmall_ja_JP
#include <transliteration_OneToOne.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

OUString SAL_CALL
ignoreSize_ja_JP::folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
  throw(RuntimeException)
{
    smallToLarge_ja_JP t1;
    return t1.transliterate(inStr, startPos, nCount, offset);
}


Sequence< OUString > SAL_CALL
ignoreSize_ja_JP::transliterateRange( const OUString& str1, const OUString& str2 )
  throw(RuntimeException)
{
    smallToLarge_ja_JP t1;
    largeToSmall_ja_JP t2;

    return transliteration_Ignore::transliterateRange(str1, str2, t1, t2);
}

sal_Unicode SAL_CALL
ignoreSize_ja_JP::transliterateChar2Char( sal_Unicode inChar) throw(RuntimeException, MultipleCharsOutputException)
{
    smallToLarge_ja_JP t1;
    return t1.transliterateChar2Char(inChar);
}

} } } }

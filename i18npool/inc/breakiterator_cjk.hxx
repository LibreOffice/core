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


#ifndef _I18N_BREAKITERATOR_CJK_HXX_
#define _I18N_BREAKITERATOR_CJK_HXX_

#include <breakiterator_unicode.hxx>
#include <xdictionary.hxx>

namespace com { namespace sun { namespace star { namespace i18n {
//  ----------------------------------------------------
//  class BreakIterator_CJK
//  ----------------------------------------------------
class BreakIterator_CJK : public BreakIterator_Unicode
{
public:
    BreakIterator_CJK();

    Boundary SAL_CALL nextWord( const rtl::OUString& Text, sal_Int32 nStartPos,
            const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType)
            throw(com::sun::star::uno::RuntimeException);
    Boundary SAL_CALL previousWord( const rtl::OUString& Text, sal_Int32 nStartPos,
            const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType)
            throw(com::sun::star::uno::RuntimeException);
    Boundary SAL_CALL getWordBoundary( const rtl::OUString& Text, sal_Int32 nPos,
            const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType, sal_Bool bDirection )
            throw(com::sun::star::uno::RuntimeException);
    LineBreakResults SAL_CALL getLineBreak( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int32 nMinBreakPos,
        const LineBreakHyphenationOptions& hOptions, const LineBreakUserOptions& bOptions )
        throw(com::sun::star::uno::RuntimeException);

protected:
    xdictionary *dict;
    rtl::OUString hangingCharacters;
};

#define BREAKITERATOR_CJK( lang ) \
class BreakIterator_##lang : public BreakIterator_CJK {\
public:\
    BreakIterator_##lang (); \
    ~BreakIterator_##lang (); \
};

#ifdef BREAKITERATOR_ALL
BREAKITERATOR_CJK( zh )
BREAKITERATOR_CJK( zh_TW )
BREAKITERATOR_CJK( ja )
BREAKITERATOR_CJK( ko )
#endif
#undef BREAKITERATOR__CJK

} } } }

#endif // _I18N_BREAKITERATOR_CJK_HXX_

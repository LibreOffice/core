/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _I18N_TRANSLITERATION_TRANSLITERATION_ONETOONE_H_
#define _I18N_TRANSLITERATION_TRANSLITERATION_ONETOONE_H_

#include <transliteration_commonclass.hxx>
#include <i18nutil/oneToOneMapping.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

typedef sal_Unicode (*TransFunc)(const sal_Unicode);

class transliteration_OneToOne : public transliteration_commonclass
{
public:
        OUString SAL_CALL
        transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 >& offset  )
        throw(com::sun::star::uno::RuntimeException);

        sal_Unicode SAL_CALL
        transliterateChar2Char( sal_Unicode inChar)
        throw(com::sun::star::uno::RuntimeException,
        com::sun::star::i18n::MultipleCharsOutputException);

        // Methods which are shared.
        sal_Int16 SAL_CALL getType() throw(com::sun::star::uno::RuntimeException);

        OUString SAL_CALL
        folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 >& offset)
        throw(com::sun::star::uno::RuntimeException);

        sal_Bool SAL_CALL
        equals( const OUString& str1, sal_Int32 pos1, sal_Int32 nCount1, sal_Int32& nMatch1,
            const OUString& str2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32& nMatch2 )
        throw(com::sun::star::uno::RuntimeException);

        com::sun::star::uno::Sequence< OUString > SAL_CALL
        transliterateRange( const OUString& str1, const OUString& str2 )
        throw(com::sun::star::uno::RuntimeException);

protected:
        TransFunc func;
        oneToOneMapping *table;
};

#define TRANSLITERATION_ONETOONE( name ) \
class name : public transliteration_OneToOne \
{ \
public: \
    name (); \
    OUString SAL_CALL \
    transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 >& offset ) \
    throw(com::sun::star::uno::RuntimeException); \
    sal_Unicode SAL_CALL \
    transliterateChar2Char( sal_Unicode inChar) \
    throw(com::sun::star::uno::RuntimeException, \
        com::sun::star::i18n::MultipleCharsOutputException); \
};

#if defined( TRANSLITERATION_fullwidthToHalfwidth ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_ONETOONE( fullwidthToHalfwidth )
#endif
#if defined( TRANSLITERATION_halfwidthToFullwidth ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_ONETOONE(halfwidthToFullwidth)
#endif
#if defined( TRANSLITERATION_fullwidthKatakanaToHalfwidthKatakana ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_ONETOONE( fullwidthKatakanaToHalfwidthKatakana )
#endif
#if defined( TRANSLITERATION_halfwidthKatakanaToFullwidthKatakana ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_ONETOONE(halfwidthKatakanaToFullwidthKatakana)
#endif
#if defined( TRANSLITERATION_fullwidthToHalfwidthLikeASC ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_ONETOONE( fullwidthToHalfwidthLikeASC )
#endif
#if defined( TRANSLITERATION_halfwidthToFullwidthLikeJIS ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_ONETOONE( halfwidthToFullwidthLikeJIS )
#endif
#undef TRANSLITERATION_ONETOONE

#define TRANSLITERATION_ONETOONE( name ) \
class name : public transliteration_OneToOne \
{ \
public: \
    name (); \
};

#if defined( TRANSLITERATION_hiraganaToKatakana ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_ONETOONE(hiraganaToKatakana)
#endif
#if defined( TRANSLITERATION_katakanaToHiragana ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_ONETOONE(katakanaToHiragana)
#endif
#if defined( TRANSLITERATION_largeToSmall_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_ONETOONE(largeToSmall_ja_JP)
#endif
#if defined( TRANSLITERATION_smallToLarge_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_ONETOONE(smallToLarge_ja_JP)
#endif
#undef TRANSLITERATION_ONETOONE

} } } }

#endif // _I18N_TRANSLITERATION_TRANSLITERATION_ONETOONE_H_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#ifndef _I18N_TRANSLITERATION_TRANSLITERATION_IGNORE_H_
#define _I18N_TRANSLITERATION_TRANSLITERATION_IGNORE_H_

#include <transliteration_commonclass.hxx>
#include <i18nutil/oneToOneMapping.hxx>

typedef sal_Unicode (*TransFunc)(const sal_Unicode);

typedef struct {
    sal_Unicode previousChar;
    sal_Unicode currentChar;
    sal_Unicode replaceChar;
    sal_Bool two2one;
} Mapping;

namespace com { namespace sun { namespace star { namespace i18n {

class transliteration_Ignore : public transliteration_commonclass
{
public:
        virtual OUString SAL_CALL
        folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 >& offset)
        throw(com::sun::star::uno::RuntimeException);

        // This method is shared.
        sal_Bool SAL_CALL
        equals( const OUString& str1, sal_Int32 pos1, sal_Int32 nCount1, sal_Int32& nMatch1,
            const OUString& str2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32& nMatch2 )
        throw(com::sun::star::uno::RuntimeException);

        // This method is implemented in sub class if needed. Otherwise, the method implemented in this class will be used.
        com::sun::star::uno::Sequence< OUString > SAL_CALL
        transliterateRange( const OUString& str1, const OUString& str2 )
        throw(com::sun::star::uno::RuntimeException);


        // Methods which are shared.
        sal_Int16 SAL_CALL getType(  ) throw(com::sun::star::uno::RuntimeException);

        OUString SAL_CALL
        transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 >& offset  )
        throw(com::sun::star::uno::RuntimeException);

        virtual sal_Unicode SAL_CALL
        transliterateChar2Char( sal_Unicode inChar)
        throw(com::sun::star::uno::RuntimeException,
            com::sun::star::i18n::MultipleCharsOutputException);

        com::sun::star::uno::Sequence< OUString > SAL_CALL
        transliterateRange( const OUString& str1, const OUString& str2, XTransliteration& t1, XTransliteration& t2 )
        throw(com::sun::star::uno::RuntimeException);

protected:
        TransFunc func;
        oneToOneMapping *table;
        const Mapping *map;
};

#define TRANSLITERATION_IGNORE( name ) \
class ignore##name : public transliteration_Ignore {\
public:\
        ignore##name ();\
};

#if defined( TRANSLITERATION_BaFa_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(BaFa_ja_JP)
#endif
#if defined( TRANSLITERATION_HyuByu_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(HyuByu_ja_JP)
#endif
#if defined( TRANSLITERATION_SeZe_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(SeZe_ja_JP)
#endif
#if defined( TRANSLITERATION_TiJi_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(TiJi_ja_JP)
#endif
#if defined( TRANSLITERATION_MiddleDot_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(MiddleDot_ja_JP)
#endif
#if defined( TRANSLITERATION_MinusSign_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(MinusSign_ja_JP)
#endif
#if defined( TRANSLITERATION_Separator_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(Separator_ja_JP)
#endif
#if defined( TRANSLITERATION_Space_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(Space_ja_JP)
#endif
#if defined( TRANSLITERATION_TraditionalKana_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(TraditionalKana_ja_JP)
#endif
#if defined( TRANSLITERATION_TraditionalKanji_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(TraditionalKanji_ja_JP)
#endif
#if defined( TRANSLITERATION_ZiZu_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(ZiZu_ja_JP)
#endif

#if defined( TRANSLITERATION_Diacritics_CTL ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(Diacritics_CTL)
#endif

#undef TRANSLITERATION_IGNORE

#define TRANSLITERATION_IGNORE( name ) \
class ignore##name : public transliteration_Ignore {\
public:\
        ignore##name () {\
            func = (TransFunc) 0;\
            table = 0;\
            map = 0;\
            transliterationName = "ignore"#name;\
            implementationName = "com.sun.star.i18n.Transliteration.ignore"#name;\
        };\
        OUString SAL_CALL folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, \
                com::sun::star::uno::Sequence< sal_Int32 >& offset) throw(com::sun::star::uno::RuntimeException); \
};

#if defined( TRANSLITERATION_KiKuFollowedBySa_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(KiKuFollowedBySa_ja_JP)
#endif
#if defined( TRANSLITERATION_IandEfollowedByYa_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(IandEfollowedByYa_ja_JP)
#endif
#if defined( TRANSLITERATION_IterationMark_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(IterationMark_ja_JP)
#endif
#if defined( TRANSLITERATION_ProlongedSoundMark_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(ProlongedSoundMark_ja_JP)
#endif
#undef TRANSLITERATION_IGNORE

#define TRANSLITERATION_IGNORE( name ) \
class ignore##name : public transliteration_Ignore {\
public:\
        ignore##name () {\
            func = (TransFunc) 0;\
            table = 0;\
            map = 0;\
            transliterationName = "ignore"#name;\
            implementationName = "com.sun.star.i18n.Transliteration.ignore"#name;\
        };\
        OUString SAL_CALL folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, \
                com::sun::star::uno::Sequence< sal_Int32 >& offset) throw(com::sun::star::uno::RuntimeException); \
        using transliteration_Ignore::transliterateRange;\
        com::sun::star::uno::Sequence< OUString > SAL_CALL transliterateRange( const OUString& str1, \
                const OUString& str2 ) throw(com::sun::star::uno::RuntimeException); \
        sal_Unicode SAL_CALL \
        transliterateChar2Char( sal_Unicode inChar) \
        throw(com::sun::star::uno::RuntimeException,\
            com::sun::star::i18n::MultipleCharsOutputException);\
};

#if defined( TRANSLITERATION_Kana ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(Kana)
#endif
#if defined( TRANSLITERATION_Width ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(Width)
#endif
#if defined( TRANSLITERATION_Size_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(Size_ja_JP)
#endif
#undef TRANSLITERATION_IGNORE

} } } }

#endif // _I18N_TRANSLITERATION_TRANSLITERATION_IGNORE_H_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
        virtual rtl::OUString SAL_CALL
        folding( const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 >& offset)
        throw(com::sun::star::uno::RuntimeException);

        // This method is shared.
        sal_Bool SAL_CALL
        equals( const rtl::OUString& str1, sal_Int32 pos1, sal_Int32 nCount1, sal_Int32& nMatch1,
            const rtl::OUString& str2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32& nMatch2 )
        throw(com::sun::star::uno::RuntimeException);

        // This method is implemented in sub class if needed. Otherwise, the method implemented in this class will be used.
        com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
        transliterateRange( const rtl::OUString& str1, const rtl::OUString& str2 )
        throw(com::sun::star::uno::RuntimeException);


        // Methods which are shared.
        sal_Int16 SAL_CALL getType(  ) throw(com::sun::star::uno::RuntimeException);

        rtl::OUString SAL_CALL
        transliterate( const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 >& offset  )
        throw(com::sun::star::uno::RuntimeException);

        virtual sal_Unicode SAL_CALL
        transliterateChar2Char( sal_Unicode inChar)
        throw(com::sun::star::uno::RuntimeException,
            com::sun::star::i18n::MultipleCharsOutputException);

        com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
        transliterateRange( const rtl::OUString& str1, const rtl::OUString& str2, XTransliteration& t1, XTransliteration& t2 )
        throw(com::sun::star::uno::RuntimeException);

protected:
        TransFunc func;
        oneToOneMapping *table;
        Mapping *map;
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
        rtl::OUString SAL_CALL folding( const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, \
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
        rtl::OUString SAL_CALL folding( const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, \
                com::sun::star::uno::Sequence< sal_Int32 >& offset) throw(com::sun::star::uno::RuntimeException); \
        using transliteration_Ignore::transliterateRange;\
        com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL transliterateRange( const rtl::OUString& str1, \
                const rtl::OUString& str2 ) throw(com::sun::star::uno::RuntimeException); \
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

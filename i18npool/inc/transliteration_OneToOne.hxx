/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: transliteration_OneToOne.hxx,v $
 * $Revision: 1.8 $
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
#ifndef _I18N_TRANSLITERATION_TRANSLITERATION_ONETOONE_H_
#define _I18N_TRANSLITERATION_TRANSLITERATION_ONETOONE_H_

#include <transliteration_commonclass.hxx>
#include <i18nutil/oneToOneMapping.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

typedef sal_Unicode (*TransFunc)(const sal_Unicode);

class transliteration_OneToOne : public transliteration_commonclass
{
public:
        rtl::OUString SAL_CALL
        transliterate( const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 >& offset  )
        throw(com::sun::star::uno::RuntimeException);

        sal_Unicode SAL_CALL
        transliterateChar2Char( sal_Unicode inChar)
        throw(com::sun::star::uno::RuntimeException,
        com::sun::star::i18n::MultipleCharsOutputException);

        // Methods which are shared.
        sal_Int16 SAL_CALL getType() throw(com::sun::star::uno::RuntimeException);

        rtl::OUString SAL_CALL
        folding( const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 >& offset)
        throw(com::sun::star::uno::RuntimeException);

        sal_Bool SAL_CALL
        equals( const rtl::OUString& str1, sal_Int32 pos1, sal_Int32 nCount1, sal_Int32& nMatch1,
            const rtl::OUString& str2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32& nMatch2 )
        throw(com::sun::star::uno::RuntimeException);

        com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
        transliterateRange( const rtl::OUString& str1, const rtl::OUString& str2 )
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
    rtl::OUString SAL_CALL \
    transliterate( const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 >& offset ) \
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

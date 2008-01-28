/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: transliteration_OneToOne.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-28 15:33:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

/*************************************************************************
 *
 *  $RCSfile: transliteration_Ignore.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bustamam $ $Date: 2002-03-26 13:36:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _I18N_TRANSLITERATION_TRANSLITERATION_IGNORE_H_
#define _I18N_TRANSLITERATION_TRANSLITERATION_IGNORE_H_

#include <transliteration_commonclass.hxx>
#include <oneToOneMapping.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

class transliteration_Ignore : public transliteration_commonclass
{
public:
    virtual rtl::OUString SAL_CALL
    folding( const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 >& offset)
    throw(com::sun::star::uno::RuntimeException) = 0;

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

    com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
    transliterateRange( const rtl::OUString& str1, const rtl::OUString& str2, XTransliteration& t1, XTransliteration& t2 )
    throw(com::sun::star::uno::RuntimeException);


    //
    rtl::OUString SAL_CALL
    transliterate( const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 >& offset, oneToOneMapping& table )
    throw(com::sun::star::uno::RuntimeException);

    rtl::OUString SAL_CALL
    transliterate( const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 >& offset, sal_Unicode (*func)(const sal_Unicode) )
    throw(com::sun::star::uno::RuntimeException);
};

#define TRANSLITERATION_IGNORE( name ) \
class ignore##name : public transliteration_Ignore {\
public:\
    ignore##name () {\
        transliterationName = "ignore##name";\
        implementationName = "com.sun.star.i18n.Transliteration.ignore##name";\
    };\
    rtl::OUString SAL_CALL folding( const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, \
        com::sun::star::uno::Sequence< sal_Int32 >& offset) throw(com::sun::star::uno::RuntimeException); \
};\

#if defined( TRANSLITERATION_BaFa_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(BaFa_ja_JP)
#endif
#if defined( TRANSLITERATION_HyuByu_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(HyuByu_ja_JP)
#endif
#if defined( TRANSLITERATION_IandEfollowedByYa_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(IandEfollowedByYa_ja_JP)
#endif
#if defined( TRANSLITERATION_IterationMark_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(IterationMark_ja_JP)
#endif
#if defined( TRANSLITERATION_KiKuFollowedBySa_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(KiKuFollowedBySa_ja_JP)
#endif
#if defined( TRANSLITERATION_MiddleDot_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(MiddleDot_ja_JP)
#endif
#if defined( TRANSLITERATION_MinusSign_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(MinusSign_ja_JP)
#endif
#if defined( TRANSLITERATION_ProlongedSoundMark_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(ProlongedSoundMark_ja_JP)
#endif
#if defined( TRANSLITERATION_SeZe_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(SeZe_ja_JP)
#endif
#if defined( TRANSLITERATION_Separator_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(Separator_ja_JP)
#endif
#if defined( TRANSLITERATION_Space_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(Space_ja_JP)
#endif
#if defined( TRANSLITERATION_TiJi_ja_JP ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_IGNORE(TiJi_ja_JP)
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
        transliterationName = "ignore##name";\
        implementationName = "com.sun.star.i18n.Transliteration.ignore##name";\
    };\
    rtl::OUString SAL_CALL folding( const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, \
        com::sun::star::uno::Sequence< sal_Int32 >& offset) throw(com::sun::star::uno::RuntimeException); \
    com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL transliterateRange( const rtl::OUString& str1, \
        const rtl::OUString& str2 ) throw(com::sun::star::uno::RuntimeException); \
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

/*************************************************************************
 *
 *  $RCSfile: transliteration_OneToOne.hxx,v $
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
#ifndef _I18N_TRANSLITERATION_TRANSLITERATION_ONETOONE_H_
#define _I18N_TRANSLITERATION_TRANSLITERATION_ONETOONE_H_

#include <transliteration_commonclass.hxx>
#include <oneToOneMapping.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

typedef sal_Unicode (*TransFunc)(const sal_Unicode);

class transliteration_OneToOne : public transliteration_commonclass
{
public:
    virtual rtl::OUString SAL_CALL
    transliterate( const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 >& offset  )
    throw(com::sun::star::uno::RuntimeException) = 0;

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

    rtl::OUString SAL_CALL
    transliterate( const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, oneToOneMapping& table )
    throw(com::sun::star::uno::RuntimeException);

    rtl::OUString SAL_CALL
    transliterate( const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 >& offset, oneToOneMapping& table )
    throw(com::sun::star::uno::RuntimeException);

    rtl::OUString SAL_CALL
    transliterate( const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 >& offset, TransFunc func )
    throw(com::sun::star::uno::RuntimeException);
};

#define TRANSLITERATION_ONETOONE( name ) \
class name : public transliteration_OneToOne \
{ \
public: \
    name (); \
    rtl::OUString SAL_CALL \
    transliterate( const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 >& offset ) \
    throw(com::sun::star::uno::RuntimeException); \
};

#if defined( TRANSLITERATION_fullwidthToHalfwidth ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_ONETOONE( fullwidthToHalfwidth )
#endif
#if defined( TRANSLITERATION_halfwidthToFullwidth ) || defined( TRANSLITERATION_ALL )
TRANSLITERATION_ONETOONE(halfwidthToFullwidth)
#endif
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

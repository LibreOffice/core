/*************************************************************************
 *
 *  $RCSfile: textToPronounce_zh.hxx,v $
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
#ifndef _I18N_TRANSLITERATION_TEXTTOPRONOUNCE_ZH_HXX_
#define _I18N_TRANSLITERATION_TEXTTOPRONOUNCE_ZH_HXX_

#include <transliteration_Ignore.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

class TextToPronounce_zh : public transliteration_Ignore
{
protected:
    sal_Unicode * pronList;
    sal_Int32 * pronIdx;
    sal_Int32 * pronTab;

public:
    rtl::OUString SAL_CALL
    folding(const rtl::OUString & inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 > & offset)
    throw (com::sun::star::uno::RuntimeException);

    sal_Int16 SAL_CALL getType() throw(com::sun::star::uno::RuntimeException);

    sal_Bool SAL_CALL
    equals( const rtl::OUString & str1, sal_Int32 pos1, sal_Int32 nCount1, sal_Int32 & nMatch1, const rtl::OUString & str2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32 & nMatch2)
    throw (com::sun::star::uno::RuntimeException);

    rtl::OUString SAL_CALL
    transliterate(  const rtl::OUString & inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 > & offset)
    throw (com::sun::star::uno::RuntimeException);
};

#define TRANSLITERATION_TextToPronounce_zh( name ) \
class name : public TextToPronounce_zh \
{ \
public: \
    name (); \
};

#ifdef TRANSLITERATION_ALL
TRANSLITERATION_TextToPronounce_zh( TextToPinyin_zh_CN )
TRANSLITERATION_TextToPronounce_zh( TextToChuyin_zh_TW )
#endif
#undef TRANSLITERATION_TextToPronounce_zh

} } } }

#endif

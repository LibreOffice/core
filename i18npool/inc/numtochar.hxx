/*************************************************************************
 *
 *  $RCSfile: numtochar.hxx,v $
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
#ifndef _L10N_TRANSLITERATION_NUMTOCHAR_HXX_
#define _L10N_TRANSLITERATION_NUMTOCHAR_HXX_

#include <transliteration_OneToOne.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

class NumToChar : public transliteration_OneToOne {
public:
    NumToChar();

    rtl::OUString SAL_CALL transliterate( const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, com::sun::star::uno::Sequence< sal_Int32 >& offset  ) throw(com::sun::star::uno::RuntimeException);

protected:
    const sal_Unicode *num2char;
};

#define TRANSLITERATION_NUMTOCHAR( name ) \
class NumToChar##name : public NumToChar \
{ \
public: \
    NumToChar##name (); \
};

#ifdef TRANSLITERATION_ALL
TRANSLITERATION_NUMTOCHAR(Lower_zh_CN)
TRANSLITERATION_NUMTOCHAR(Upper_zh_CN)
TRANSLITERATION_NUMTOCHAR(Lower_zh_TW)
TRANSLITERATION_NUMTOCHAR(Upper_zh_TW)
TRANSLITERATION_NUMTOCHAR(Upper_ko)
TRANSLITERATION_NUMTOCHAR(Hangul_ko)
TRANSLITERATION_NUMTOCHAR(Lower_ko)
TRANSLITERATION_NUMTOCHAR(KanjiShort_ja_JP)
TRANSLITERATION_NUMTOCHAR(Fullwidth)
TRANSLITERATION_NUMTOCHAR(Indic_ar)
TRANSLITERATION_NUMTOCHAR(EastIndic_ar)
TRANSLITERATION_NUMTOCHAR(Indic_hi)
TRANSLITERATION_NUMTOCHAR(_th)
#endif
#undef TRANSLITERATION_NUMTOCHAR

} } } }

#endif // _L10N_TRANSLITERATION_NUMTOCHAR_HXX_

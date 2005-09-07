/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textToPronounce_zh.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:56:13 $
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
        transliterateChar2String( sal_Unicode inChar)
        throw(com::sun::star::uno::RuntimeException);

        sal_Unicode SAL_CALL
        transliterateChar2Char( sal_Unicode inChar)
        throw(com::sun::star::uno::RuntimeException,
            com::sun::star::i18n::MultipleCharsOutputException);
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

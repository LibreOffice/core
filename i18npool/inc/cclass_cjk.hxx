/*************************************************************************
 *
 *  $RCSfile: cclass_cjk.hxx,v $
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
#ifndef _CCLASS_CJK_H_
#define _CCLASS_CJK_H_

#include <cclass_unicode.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

#define MAXLEN  31      // Define max length for CJK number

#define ZERO        0x0030  // Define unicode zero
#define ONE     0x0031  // Define unicode one

class cclass_CJK : public cclass_Unicode {
public:
    cclass_CJK( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF );
    ~cclass_CJK();
    i18n::ParseResult SAL_CALL parsePredefinedToken(
        sal_Int32 nTokenType,
        const rtl::OUString& Text,
        sal_Int32 nPos,
        const com::sun::star::lang::Locale& rLocale,
        sal_Int32 nStartCharFlags,
        const rtl::OUString& userDefinedCharactersStart,
        sal_Int32 nContCharFlags,
        const rtl::OUString& userDefinedCharactersCont )
        throw(com::sun::star::uno::RuntimeException);

protected:
    sal_Int32 size;
    rtl::OUString number;
    sal_Unicode *text;
    sal_Int32 textPos;

    sal_Unicode *Upper;
    sal_Unicode *Base;
    sal_Unicode *Plus;

    static sal_Int32 cclass_CJK::upperVal[];
    static sal_Int32 cclass_CJK::baseVal[];
    static sal_Int32 cclass_CJK::plusVal[];

private:
    sal_Int32 ToNum();
    sal_Int32 ToNum(sal_Unicode *n, sal_Int32 s);
    void Init(sal_Unicode *n, sal_Int32 s);
    void NumberCopy(sal_Unicode *s, sal_Unicode *t, sal_Int32 n);
    void NumberReverse(sal_Unicode *s, sal_Int32 n);
    sal_Int32 oneDigit(sal_Unicode s);
    sal_Int32 baseDigit(sal_Unicode s);
    sal_Int32 plusDigit(sal_Unicode s);
};

#define CCLASS_CJK( name ) \
class name : public cclass_CJK \
{ \
public: \
    name ( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF ); \
};

#ifdef CCLASS_ALL
CCLASS_CJK(cclass_zh)
CCLASS_CJK(cclass_zh_TW)
CCLASS_CJK(cclass_ko)
#endif
#undef CCLASS_CJK

} } } }

#endif

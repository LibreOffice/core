/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cclass_cjk.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:49:46 $
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

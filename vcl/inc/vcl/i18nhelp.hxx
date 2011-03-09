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

#ifndef _VCL_I18NHELP_HXX
#define _VCL_I18NHELP_HXX

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/Locale.hpp>
#include <osl/mutex.hxx>
#include <tools/string.hxx>
#include <vcl/dllapi.h>

namespace com {
namespace sun {
namespace star {
namespace lang {
    class XMultiServiceFactory;
}
}}}

namespace utl {
    class TransliterationWrapper;
}

class LocaleDataWrapper;

class Date;

namespace vcl
{

class VCL_DLLPUBLIC I18nHelper
{
private:
    ::osl::Mutex                    maMutex;
    ::com::sun::star::lang::Locale  maLocale;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxMSF;

    LocaleDataWrapper*              mpLocaleDataWrapper;
    utl::TransliterationWrapper*    mpTransliterationWrapper;

    sal_Bool                        mbTransliterateIgnoreCase;

    SAL_DLLPRIVATE void             ImplDestroyWrappers();

protected:
    ::osl::Mutex& GetMutex() { return maMutex; }

    SAL_DLLPRIVATE utl::TransliterationWrapper&    ImplGetTransliterationWrapper() const;
    SAL_DLLPRIVATE LocaleDataWrapper&              ImplGetLocaleDataWrapper() const;

public:

                I18nHelper( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxMSF, const ::com::sun::star::lang::Locale& rLocale );
                ~I18nHelper();

    const ::com::sun::star::lang::Locale& getLocale() const;

    sal_Int32   CompareString( const String& rStr1, const String& rStr2 ) const;

    sal_Bool    MatchString( const String& rStr1, const String& rStr2 ) const;
    sal_Bool    MatchMnemonic( const String& rString, sal_Unicode cMnemonicChar ) const;

    String      GetDate( const Date& rDate ) const;
    String      GetNum( long nNumber, sal_uInt16 nDecimals, sal_Bool bUseThousandSep = sal_True, sal_Bool bTrailingZeros = sal_True ) const;

    static String filterFormattingChars( const String& );
};

}   // namespace vcl

#endif  // _VCL_I18NHELP_HXX


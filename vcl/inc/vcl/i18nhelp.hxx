/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

    sal_Int32   CompareString( const rtl::OUString& rStr1, const rtl::OUString& rStr2 ) const;

    sal_Bool    MatchString( const rtl::OUString& rStr1, const rtl::OUString& rStr2 ) const;
    sal_Bool    MatchMnemonic( const String& rString, sal_Unicode cMnemonicChar ) const;

    String      GetNum( long nNumber, sal_uInt16 nDecimals, sal_Bool bUseThousandSep = sal_True, sal_Bool bTrailingZeros = sal_True ) const;

    static String filterFormattingChars( const String& );
};

}   // namespace vcl

#endif  // _VCL_I18NHELP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

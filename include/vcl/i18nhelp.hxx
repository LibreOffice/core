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

#ifndef INCLUDED_VCL_I18NHELP_HXX
#define INCLUDED_VCL_I18NHELP_HXX

#include <com/sun/star/uno/Reference.h>
#include <i18nlangtag/languagetag.hxx>
#include <mutex>
#include <rtl/ustring.hxx>
#include <tools/long.hxx>
#include <vcl/dllapi.h>
#include <memory>

namespace com::sun::star::uno {
    class XComponentContext;
}
namespace utl {
    class TransliterationWrapper;
}
class LocaleDataWrapper;

namespace vcl
{

class VCL_DLLPUBLIC I18nHelper
{
    mutable std::mutex              maMutex;
    LanguageTag                     maLanguageTag;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    const LocaleDataWrapper*              mpLocaleDataWrapper { nullptr };
    std::unique_ptr<utl::TransliterationWrapper>    mpTransliterationWrapper;

    bool                            mbTransliterateIgnoreCase;

    SAL_DLLPRIVATE void             ImplDestroyWrappers();

    SAL_DLLPRIVATE utl::TransliterationWrapper&    ImplGetTransliterationWrapper() const;
    SAL_DLLPRIVATE const LocaleDataWrapper&        ImplGetLocaleDataWrapper() const;

public:

                I18nHelper( const css::uno::Reference< css::uno::XComponentContext >& rxContext, LanguageTag aLanguageTag );
                ~I18nHelper();

    sal_Int32   CompareString( const OUString& rStr1, const OUString& rStr2 ) const;

    bool    MatchString( const OUString& rStr1, const OUString& rStr2 ) const;
    bool    MatchMnemonic( std::u16string_view rString, sal_Unicode cMnemonicChar ) const;

    OUString    GetNum( tools::Long nNumber, sal_uInt16 nDecimals, bool bUseThousandSep = true, bool bTrailingZeros = true ) const;

    static OUString filterFormattingChars( const OUString& );
};

}   // namespace vcl

#endif // INCLUDED_VCL_I18NHELP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

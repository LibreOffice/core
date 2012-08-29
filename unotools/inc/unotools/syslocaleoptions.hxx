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

#ifndef INCLUDED_SVTOOLS_SYSLOCALEOPTIONS_HXX
#define INCLUDED_SVTOOLS_SYSLOCALEOPTIONS_HXX

#include "unotools/unotoolsdllapi.h"
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <tools/solar.h>
#include <tools/link.hxx>
#include <i18npool/lang.h>
#include <tools/string.hxx>
#include <unotools/options.hxx>
#include <com/sun/star/lang/Locale.hpp>

// bits for broadcasting hints of changes in a SfxSimpleHint, may be combined
const sal_uLong SYSLOCALEOPTIONS_HINT_LOCALE       = 0x00000001;
const sal_uLong SYSLOCALEOPTIONS_HINT_CURRENCY     = 0x00000002;
const sal_uLong SYSLOCALEOPTIONS_HINT_UILOCALE     = 0x00000004;
const sal_uLong SYSLOCALEOPTIONS_HINT_DECSEP       = 0x00000008;
const sal_uLong SYSLOCALEOPTIONS_HINT_DATEPATTERNS = 0x00000010;

class SvtSysLocaleOptions_Impl;
class SvtListener;
namespace osl { class Mutex; }

class UNOTOOLS_DLLPUBLIC SvtSysLocaleOptions: public utl::detail::Options
{
    static  SvtSysLocaleOptions_Impl*   pOptions;
    static  sal_Int32                   nRefCount;

    UNOTOOLS_DLLPRIVATE static  ::osl::Mutex&       GetMutex();
    virtual void ConfigurationChanged( utl::ConfigurationBroadcaster* p, sal_uInt32 nHint );

public:

    enum EOption
    {
        E_LOCALE,
        E_UILOCALE,
        E_CURRENCY,
        E_DATEPATTERNS
    };
                                SvtSysLocaleOptions();
                                virtual ~SvtSysLocaleOptions();

    // ConfigItem methods

            sal_Bool            IsModified();
            void                Commit();

                                /** Add a listener to react on config changes
                                    which are broadcasted in a SfxSimpleHint
                                    @return
                                        <TRUE/> if added
                                        <FALSE/> if not added
                                 */

    /** Block broadcasts and accumulate hints. This may be useful if, for
        example, the locale and currency are changed and the currency was
        empty before, since changing the locale with an empty currency does
        also broadcast a change hint for the currency which would result in
        two currency changes broadcasted.

        @param bBlock
            <TRUE/>: broadcasts are blocked until reversed.
            <FALSE/>: broadcasts are not blocked anymore. Pending hints are
            broadcasted if no other instance blocks the broadcast.

        @ATTENTION
        All SvtSysLocaleOptions instances point to exactly one refcounted
        internal representation instance and broadcast blocks are counted.
        Thus if you issue a BlockBroadcasts(sal_True) you MUST issue a matching
        BlockBroadcasts(sal_False) or otherwise pending hints would never be
        broadcasted again.
     */
       virtual void                BlockBroadcasts( bool bBlock );

    // config value access methods

    /// The config string may be empty to denote the SYSTEM locale
            const ::rtl::OUString&  GetLocaleConfigString() const;
            void                SetLocaleConfigString( const ::rtl::OUString& rStr );
            com::sun::star::lang::Locale GetLocale() const;
            com::sun::star::lang::Locale GetRealLocale() const;
            LanguageType        GetRealLanguage() const;

    /// The config string may be empty to denote the SYSTEM locale
            void                SetUILocaleConfigString( const ::rtl::OUString& rStr );
            com::sun::star::lang::Locale GetRealUILocale() const;
            LanguageType        GetRealUILanguage() const;

    /// The config string may be empty to denote the default currency of the locale
            const ::rtl::OUString&  GetCurrencyConfigString() const;
            void                SetCurrencyConfigString( const ::rtl::OUString& rStr );

    /** The config string may be empty to denote the default
        DateAcceptancePatterns of the locale */
            const ::rtl::OUString&  GetDatePatternsConfigString() const;
            void                SetDatePatternsConfigString( const ::rtl::OUString& rStr );

    // determine whether the decimal separator defined in the keyboard layout is used
    // or the one approriate to the locale
            sal_Bool            IsDecimalSeparatorAsLocale() const;
            void                SetDecimalSeparatorAsLocale( sal_Bool bSet);

    // convenience methods

    /// Get currency abbreviation and locale from an USD-en-US or EUR-de-DE string
    static  void                GetCurrencyAbbrevAndLanguage(
                                    String& rAbbrev,
                                    LanguageType& eLang,
                                    const ::rtl::OUString& rConfigString );

    /// Create an USD-en-US or EUR-de-DE string
    static  ::rtl::OUString     CreateCurrencyConfigString(
                                    const String& rAbbrev,
                                    LanguageType eLang );

            void                GetCurrencyAbbrevAndLanguage(
                                        String& rAbbrev,
                                        LanguageType& eLang ) const
                                    {
                                        GetCurrencyAbbrevAndLanguage( rAbbrev,
                                            eLang, GetCurrencyConfigString() );
                                    }

            void                SetCurrencyAbbrevAndLanguage(
                                        const String& rAbbrev,
                                        LanguageType eLang )
                                    {
                                        SetCurrencyConfigString(
                                            CreateCurrencyConfigString(
                                            rAbbrev, eLang ) );
                                    }

    /** Set a link to a method to be called whenever the default currency
        changes. This can be only one method, and normally it is the static
        link method which calls SvNumberFormatter::SetDefaultSystemCurrency().
        This is needed because the number formatter isn't part of the svl light
        library, otherwise we could call SetDefaultSystemCurrency() directly.
     */
    static  void                SetCurrencyChangeLink( const Link& rLink );
    static  const Link&         GetCurrencyChangeLink();

    /** return the readonly state of the queried option. */
            sal_Bool            IsReadOnly( EOption eOption ) const;
};

#endif  // INCLUDED_SVTOOLS_SYSLOCALEOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

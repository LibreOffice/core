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

#ifndef INCLUDED_UNOTOOLS_SYSLOCALEOPTIONS_HXX
#define INCLUDED_UNOTOOLS_SYSLOCALEOPTIONS_HXX

#include <unotools/unotoolsdllapi.h>
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <tools/solar.h>
#include <tools/link.hxx>
#include <i18nlangtag/lang.h>
#include <i18nlangtag/languagetag.hxx>
#include <unotools/options.hxx>

class SvtSysLocaleOptions_Impl;
class SvtListener;
namespace osl { class Mutex; }

class SAL_WARN_UNUSED UNOTOOLS_DLLPUBLIC SvtSysLocaleOptions : public utl::detail::Options
{
    std::shared_ptr<SvtSysLocaleOptions_Impl>  pImpl;

    UNOTOOLS_DLLPRIVATE static  ::osl::Mutex&       GetMutex();
    virtual void ConfigurationChanged( utl::ConfigurationBroadcaster* p, ConfigurationHints nHint ) override;

public:

    enum class EOption
    {
        Locale,
        Currency,
        DatePatterns
    };
                                SvtSysLocaleOptions();
                                virtual ~SvtSysLocaleOptions() override;

    // ConfigItem methods

            bool                IsModified();
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
        Thus if you issue a BlockBroadcasts(true) you MUST issue a matching
        BlockBroadcasts(false) or otherwise pending hints would never be
        broadcasted again.
     */
       virtual void             BlockBroadcasts( bool bBlock ) override;

    // config value access methods

    /// The config string may be empty to denote the SYSTEM locale
            void                SetLocaleConfigString( const OUString& rStr );
            /** Get locale set, not resolved to the real locale. */
            LanguageTag         GetLanguageTag() const;
            /** Get locale set, always resolved to the real locale. */
            const LanguageTag&  GetRealLanguageTag() const;

    /// The config string may be empty to denote the SYSTEM locale
            void                SetUILocaleConfigString( const OUString& rStr );
            /** Get UI locale set, always resolved to the real locale. */
            const LanguageTag&  GetRealUILanguageTag() const;

    /// The config string may be empty to denote the default currency of the locale
            const OUString&     GetCurrencyConfigString() const;
            void                SetCurrencyConfigString( const OUString& rStr );

    /** The config string may be empty to denote the default
        DateAcceptancePatterns of the locale */
            const OUString&     GetDatePatternsConfigString() const;
            void                SetDatePatternsConfigString( const OUString& rStr );

    // determine whether the decimal separator defined in the keyboard layout is used
    // or the one appropriate to the locale
            bool                IsDecimalSeparatorAsLocale() const;
            void                SetDecimalSeparatorAsLocale( bool bSet);

    // determine whether to ignore changes to the system keyboard/locale/language when
    // determining the language for newly entered text
            bool                IsIgnoreLanguageChange() const;
            void                SetIgnoreLanguageChange( bool bSet);

    // convenience methods

    /// Get currency abbreviation and locale from an USD-en-US or EUR-de-DE string
    static  void                GetCurrencyAbbrevAndLanguage(
                                    OUString& rAbbrev,
                                    LanguageType& eLang,
                                    const OUString& rConfigString );

    /// Create an USD-en-US or EUR-de-DE string
    static  OUString            CreateCurrencyConfigString(
                                    const OUString& rAbbrev,
                                    LanguageType eLang );

            void                GetCurrencyAbbrevAndLanguage(
                                        OUString& rAbbrev,
                                        LanguageType& eLang ) const
                                    {
                                        GetCurrencyAbbrevAndLanguage( rAbbrev,
                                            eLang, GetCurrencyConfigString() );
                                    }

    /** Set a link to a method to be called whenever the default currency
        changes. This can be only one method, and normally it is the static
        link method which calls SvNumberFormatter::SetDefaultSystemCurrency().
        This is needed because the number formatter isn't part of the svl light
        library, otherwise we could call SetDefaultSystemCurrency() directly.
     */
    static  void                SetCurrencyChangeLink( const Link<LinkParamNone*,void>& rLink );
    static  const Link<LinkParamNone*,void>&       GetCurrencyChangeLink();

    /** return the readonly state of the queried option. */
            bool                IsReadOnly( EOption eOption ) const;
};

#endif // INCLUDED_UNOTOOLS_SYSLOCALEOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_SVTOOLS_SYSLOCALEOPTIONS_HXX
#define INCLUDED_SVTOOLS_SYSLOCALEOPTIONS_HXX

#include "bf_svtools/svtdllapi.h"

#include <sal/types.h>
#include <rtl/ustring.hxx>

#include <tools/solar.h>
#include <tools/link.hxx>
#include <i18npool/lang.h>
#include <tools/string.hxx>
#include <bf_svtools/options.hxx>

namespace osl { class Mutex; }

namespace binfilter
{

// bits for broadcasting hints of changes in a SfxSimpleHint, may be combined
const ULONG SYSLOCALEOPTIONS_HINT_LOCALE    = 0x00000001;
const ULONG SYSLOCALEOPTIONS_HINT_CURRENCY  = 0x00000002;

class SvtSysLocaleOptions_Impl;
class SvtListener;


class  SvtSysLocaleOptions: public Options
{
    static  SvtSysLocaleOptions_Impl*   pOptions;
    static  sal_Int32                   nRefCount;

    static  ::osl::Mutex&       GetMutex();

public:

    enum EOption
    {
        E_LOCALE,
        E_CURRENCY
    };
                                SvtSysLocaleOptions();
                                virtual ~SvtSysLocaleOptions();

    // ConfigItem methods

                                /** Add a listener to react on config changes
                                    which are broadcasted in a SfxSimpleHint
                                    @return
                                        <TRUE/> if added
                                        <FALSE/> if not added
                                 */
            BOOL                AddListener( SvtListener& );
            BOOL                RemoveListener( SvtListener& );

    // config value access methods

    /// The config string may be empty to denote the default currency of the locale
            const ::rtl::OUString&  GetCurrencyConfigString() const;

    // convenience methods

    /// Get currency abbreviation and locale from an USD-en-US or EUR-de-DE string
    static  void                GetCurrencyAbbrevAndLanguage(
                                    String& rAbbrev,
                                    LanguageType& eLang,
                                    const ::rtl::OUString& rConfigString );

            void                GetCurrencyAbbrevAndLanguage(
                                        String& rAbbrev,
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
    static  void                SetCurrencyChangeLink( const Link& rLink );
    static  const Link&         GetCurrencyChangeLink();
};

}

#endif  // INCLUDED_SVTOOLS_SYSLOCALEOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

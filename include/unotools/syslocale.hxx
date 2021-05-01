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

#ifndef INCLUDED_UNOTOOLS_SYSLOCALE_HXX
#define INCLUDED_UNOTOOLS_SYSLOCALE_HXX

#include <unotools/unotoolsdllapi.h>
#include <rtl/textenc.h>

#include <memory>

class CharClass;
class LanguageTag;
class LocaleDataWrapper;
class SvtSysLocale_Impl;
class SvtSysLocaleOptions;

namespace osl { class Mutex; }

/**
    SvtSysLocale provides a refcounted single instance of an application wide
    LocaleDataWrapper and <type>CharClass</type> which always
    follow the locale as it is currently configured by the user. You may use
    it anywhere to access the locale data elements like decimal separator and
    simple date formatting and so on. Constructing and destructing a
    SvtSysLocale is not expensive as long as there is at least one instance
    left.
 */
class UNOTOOLS_DLLPUBLIC SvtSysLocale
{
    friend class SvtSysLocale_Impl;     // access to mutex

    std::shared_ptr<SvtSysLocale_Impl>  pImpl;

    UNOTOOLS_DLLPRIVATE static  ::osl::Mutex&               GetMutex();

public:
                                        SvtSysLocale();
                                        ~SvtSysLocale();

            const LocaleDataWrapper&    GetLocaleData() const;
            const CharClass&            GetCharClass() const;

    /** It is safe to store the pointers locally and use them AS LONG AS THE
        INSTANCE OF SvtSysLocale LIVES!
        It is a faster access but be sure what you do!
     */
            const CharClass*            GetCharClassPtr() const;
            SvtSysLocaleOptions&        GetOptions() const;
            const LanguageTag&          GetLanguageTag() const;
            const LanguageTag&          GetUILanguageTag() const;

    /** Get the best MIME encoding matching the system locale, or if that isn't
        determinable one that matches the UI locale, or UTF8 if everything else
        fails.
     */
    static  rtl_TextEncoding    GetBestMimeEncoding();
};

#endif  // INCLUDED_SVTOOLS_SYSLOCALE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

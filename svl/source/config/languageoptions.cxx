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


#include <svl/languageoptions.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/ctloptions.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <rtl/instance.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <unotools/syslocale.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <mutex>

#ifdef _WIN32
#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

using namespace ::com::sun::star;


namespace SvtLanguageOptions
{

// returns for a language the scripttype
SvtScriptType GetScriptTypeOfLanguage( LanguageType nLang )
{
    if( LANGUAGE_DONTKNOW == nLang )
        nLang = LANGUAGE_ENGLISH_US;
    else if (LANGUAGE_SYSTEM == nLang || LANGUAGE_PROCESS_OR_USER_DEFAULT == nLang)
        nLang = SvtSysLocale().GetLanguageTag().getLanguageType();

    sal_Int16 nScriptType = MsLangId::getScriptType( nLang );
    SvtScriptType nScript;
    switch (nScriptType)
    {
        case css::i18n::ScriptType::ASIAN:
            nScript = SvtScriptType::ASIAN;
            break;
        case css::i18n::ScriptType::COMPLEX:
            nScript = SvtScriptType::COMPLEX;
            break;
        default:
            nScript = SvtScriptType::LATIN;
    }
    return nScript;
}

SvtScriptType FromI18NToSvtScriptType( sal_Int16 nI18NType )
{
    switch ( nI18NType )
    {
        case i18n::ScriptType::LATIN:   return SvtScriptType::LATIN;
        case i18n::ScriptType::ASIAN:   return SvtScriptType::ASIAN;
        case i18n::ScriptType::COMPLEX: return SvtScriptType::COMPLEX;
        case i18n::ScriptType::WEAK:    return SvtScriptType::NONE; // no mapping
        default: assert(false && nI18NType && "Unknown i18n::ScriptType"); break;
    }
    return SvtScriptType::NONE;
}

sal_Int16 FromSvtScriptTypeToI18N( SvtScriptType nItemType )
{
    switch ( nItemType )
    {
        case SvtScriptType::NONE:       return 0;
        case SvtScriptType::LATIN:      return i18n::ScriptType::LATIN;
        case SvtScriptType::ASIAN:      return i18n::ScriptType::ASIAN;
        case SvtScriptType::COMPLEX:    return i18n::ScriptType::COMPLEX;
        case SvtScriptType::UNKNOWN:    return 0; // no mapping
        default: assert(false && static_cast<int>(nItemType) && "unknown SvtScriptType"); break;
    }
    return 0;
}

sal_Int16 GetI18NScriptTypeOfLanguage( LanguageType nLang )
{
    return FromSvtScriptTypeToI18N( GetScriptTypeOfLanguage( nLang ) );
}

} // namespace SvtLanguageOptions

static bool isKeyboardLayoutTypeInstalled(sal_Int16 scriptType)
{
    bool isInstalled = false;
#ifdef _WIN32
    int nLayouts = GetKeyboardLayoutList(0, nullptr);
    if (nLayouts > 0)
    {
        HKL *lpList = static_cast<HKL*>(LocalAlloc(LPTR, (nLayouts * sizeof(HKL))));
        if (lpList)
        {
            nLayouts = GetKeyboardLayoutList(nLayouts, lpList);

            for(int i = 0; i < nLayouts; ++i)
            {
                LCID lang = MAKELCID(LOWORD(lpList[i]), SORT_DEFAULT);
                if (MsLangId::getScriptType(LanguageType(lang)) == scriptType)
                {
                    isInstalled = true;
                    break;
                }
            }

            LocalFree(lpList);
        }
    }
#else
    (void)scriptType;
#endif
    return isInstalled;
}

namespace SvtSystemLanguageOptions
{
    bool isCJKKeyboardLayoutInstalled()
    {
        return isKeyboardLayoutTypeInstalled(css::i18n::ScriptType::ASIAN);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

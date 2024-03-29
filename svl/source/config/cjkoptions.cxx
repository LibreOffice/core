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

#include <svl/cjkoptions.hxx>

#include <svl/languageoptions.hxx>
#include <i18nlangtag/lang.h>
#include <i18nlangtag/languagetag.hxx>
#include <officecfg/System.hxx>
#include <officecfg/Office/Common.hxx>
#include <mutex>

static void SvtCJKOptions_Load();

namespace SvtCJKOptions
{

bool IsCJKFontEnabled()
{
    SvtCJKOptions_Load();
    return officecfg::Office::Common::I18N::CJK::CJKFont::get();
}

bool IsVerticalTextEnabled()
{
    SvtCJKOptions_Load();
    return officecfg::Office::Common::I18N::CJK::VerticalText::get();
}

bool IsAsianTypographyEnabled()
{
    SvtCJKOptions_Load();
    return officecfg::Office::Common::I18N::CJK::AsianTypography::get();
}

bool IsJapaneseFindEnabled()
{
    SvtCJKOptions_Load();
    return officecfg::Office::Common::I18N::CJK::JapaneseFind::get();
}

bool IsRubyEnabled()
{
    SvtCJKOptions_Load();
    return officecfg::Office::Common::I18N::CJK::Ruby::get();
}

bool IsChangeCaseMapEnabled()
{
    SvtCJKOptions_Load();
    return officecfg::Office::Common::I18N::CJK::ChangeCaseMap::get();
}

bool IsDoubleLinesEnabled()
{
    SvtCJKOptions_Load();
    return officecfg::Office::Common::I18N::CJK::DoubleLines::get();
}

void        SetAll(bool bSet)
{
    SvtCJKOptions_Load();
    if ( officecfg::Office::Common::I18N::CJK::CJKFont::isReadOnly()
        || officecfg::Office::Common::I18N::CJK::VerticalText::isReadOnly()
        || officecfg::Office::Common::I18N::CJK::AsianTypography::isReadOnly()
        || officecfg::Office::Common::I18N::CJK::JapaneseFind::isReadOnly()
        || officecfg::Office::Common::I18N::CJK::Ruby::isReadOnly()
        || officecfg::Office::Common::I18N::CJK::ChangeCaseMap::isReadOnly()
        || officecfg::Office::Common::I18N::CJK::DoubleLines::isReadOnly() )
        return;

    std::shared_ptr<comphelper::ConfigurationChanges> xChanges(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::I18N::CJK::CJKFont::set(bSet, xChanges);
    officecfg::Office::Common::I18N::CJK::VerticalText::set(bSet, xChanges);
    officecfg::Office::Common::I18N::CJK::AsianTypography::set(bSet, xChanges);
    officecfg::Office::Common::I18N::CJK::JapaneseFind::set(bSet, xChanges);
    officecfg::Office::Common::I18N::CJK::Ruby::set(bSet, xChanges);
    officecfg::Office::Common::I18N::CJK::ChangeCaseMap::set(bSet, xChanges);
    officecfg::Office::Common::I18N::CJK::DoubleLines::set(bSet, xChanges);
    xChanges->commit();
}

bool    IsAnyEnabled()
{
    SvtCJKOptions_Load();
    return  IsCJKFontEnabled() || IsVerticalTextEnabled() || IsAsianTypographyEnabled() || IsJapaneseFindEnabled() ||
                IsRubyEnabled() || IsChangeCaseMapEnabled() || IsDoubleLinesEnabled() ;
}

bool    IsAnyReadOnly()
{
    SvtCJKOptions_Load();
    return officecfg::Office::Common::I18N::CJK::CJKFont::isReadOnly()
        || officecfg::Office::Common::I18N::CJK::VerticalText::isReadOnly()
        || officecfg::Office::Common::I18N::CJK::AsianTypography::isReadOnly()
        || officecfg::Office::Common::I18N::CJK::JapaneseFind::isReadOnly()
        || officecfg::Office::Common::I18N::CJK::Ruby::isReadOnly()
        || officecfg::Office::Common::I18N::CJK::ChangeCaseMap::isReadOnly()
        || officecfg::Office::Common::I18N::CJK::DoubleLines::isReadOnly();
}

} // namespace SvtCJKOptions


static std::once_flag gLoadFlag;

static void SvtCJKOptions_Load()
{
    std::call_once(gLoadFlag,
        []()
        {
            if (officecfg::Office::Common::I18N::CJK::CJKFont::get())
                return;

            SvtScriptType nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage(LANGUAGE_SYSTEM);
            //system locale is CJK
            bool bAutoEnableCJK = bool(nScriptType & SvtScriptType::ASIAN);

            if (!bAutoEnableCJK)
            {
                //windows secondary system locale is CJK
                OUString sWin16SystemLocale = officecfg::System::L10N::SystemLocale::get();
                LanguageType eSystemLanguage = LANGUAGE_NONE;
                if( !sWin16SystemLocale.isEmpty() )
                    eSystemLanguage = LanguageTag::convertToLanguageTypeWithFallback( sWin16SystemLocale );
                if (eSystemLanguage != LANGUAGE_SYSTEM)
                {
                    SvtScriptType nWinScript = SvtLanguageOptions::GetScriptTypeOfLanguage( eSystemLanguage );
                    bAutoEnableCJK = bool(nWinScript & SvtScriptType::ASIAN);
                }

                //CJK keyboard is installed
                if (!bAutoEnableCJK)
                    bAutoEnableCJK = SvtSystemLanguageOptions::isCJKKeyboardLayoutInstalled();
            }

            if (bAutoEnableCJK)
            {
                std::shared_ptr<comphelper::ConfigurationChanges> xChanges(comphelper::ConfigurationChanges::create());
                officecfg::Office::Common::I18N::CJK::CJKFont::set(true, xChanges);
                officecfg::Office::Common::I18N::CJK::VerticalText::set(true, xChanges);
                officecfg::Office::Common::I18N::CJK::AsianTypography::set(true, xChanges);
                officecfg::Office::Common::I18N::CJK::JapaneseFind::set(true, xChanges);
                officecfg::Office::Common::I18N::CJK::Ruby::set(true, xChanges);
                officecfg::Office::Common::I18N::CJK::ChangeCaseMap::set(true, xChanges);
                officecfg::Office::Common::I18N::CJK::DoubleLines::set(true, xChanges);
                xChanges->commit();
            }
        });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

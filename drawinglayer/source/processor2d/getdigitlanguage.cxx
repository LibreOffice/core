/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <i18nlangtag/lang.h>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <officecfg/Office/Common.hxx>
#include <svl/ctloptions.hxx>

#include "getdigitlanguage.hxx"

LanguageType drawinglayer::detail::getDigitLanguage() {
    switch (officecfg::Office::Common::I18N::CTL::CTLTextNumerals::get()) {
    case SvtCTLOptions::NUMERALS_ARABIC:
        return LANGUAGE_ENGLISH;
    case SvtCTLOptions::NUMERALS_HINDI:
        return LANGUAGE_ARABIC_SAUDI_ARABIA;
    default:
        return Application::GetSettings().GetLanguageTag().getLanguageType();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

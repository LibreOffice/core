/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <optional>
#include <string>
#include <vector>

#include <com/sun/star/lang/Locale.hpp>
#include <rtl/ustring.hxx>

// The remote sentence checkers (LanguageTool, the Duden Korrekturserver and
// Grammalecte) as the kit sets them up: their settings from the server's
// environment, the hand-over of languages from the local spell checker, and
// what the settings dialog is told about them.
namespace desktop::remotechecker
{
// The variable <pPrefix>_<pName> of the environment the server set up;
// nothing when it is not set, as in the apps.
std::optional<OUString> envString(const char* pPrefix, const char* pName);
std::optional<bool> envBool(const char* pPrefix, const char* pName);

// Takes the remote checker settings from the environment the server set up,
// and the schema's defaults where it set none. Run again before a user's
// own settings are applied, it puts back what the user may have dropped.
void setRemoteCheckerConfig();

// Switches the local spell checker off for the languages a reachable remote
// checker spell checks itself, and back on for the ones no remote checker
// holds any more.
void handLanguagesToRemoteCheckers();

// Whether a user's own settings for the checker with this environment prefix
// (LANGUAGETOOL, DUDEN, GRAMMALECTE) are applied from their document settings.
bool allowsUserSettings(const char* pPrefix);

// The .uno:RemoteGrammarCheckers command value.
std::string getRemoteGrammarCheckers();

// The languages of the switched on remote checkers that spell check.
std::vector<css::lang::Locale> getRemoteSpellingLocales();

// Says on stderr which languages each switched on remote checker serves.
void logRemoteCheckerLanguages();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

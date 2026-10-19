/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string>

// The translation providers (DeepL and LibreTranslate) as the kit sets them
// up: their settings from the server's environment, and what the settings
// dialog is told about them.
namespace desktop::translation
{
// Takes the translation settings from the environment the server set up.
void setTranslationConfig();

// Whether a user's own settings for the provider with this environment
// prefix (DEEPL, LIBRETRANSLATE) are applied from their document settings.
bool allowsUserSettings(const char* pPrefix);

// The .uno:TranslationProviders command value.
std::string getTranslationProviders();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

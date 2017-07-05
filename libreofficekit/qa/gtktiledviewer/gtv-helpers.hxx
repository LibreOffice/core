/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GTV_HELPERS_H
#define GTV_HELPERS_H

#include <gtk/gtk.h>

#include <pwd.h>

#include <string>
#include <map>

void userPromptDialog(GtkWindow* pWindow, const std::string& aTitle, std::map<std::string, std::string>& aEntries);

void clipboardSetHtml(GtkClipboard* pClipboard, const char* pSelection);

/// Generate an author string for multiple views.
static std::string getNextAuthor()
{
    static int nCounter = 0;
    struct passwd* pPasswd = getpwuid(getuid());
    return std::string(pPasswd->pw_gecos) + " #" + std::to_string(++nCounter);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

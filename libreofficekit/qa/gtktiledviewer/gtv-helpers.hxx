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

#include "gtv-signal-handlers.hxx"

#include <map>
#include <string>
#include <memory>

#include <boost/property_tree/json_parser.hpp>

#define UI_FILE_NAME "gtv.ui"

// Wrapper with custom deleter to use for Gtk objects
template <class T>
using GtvGtkWrapper = std::unique_ptr<T, void(*)(T*)>;

namespace GtvHelpers
{
    void userPromptDialog(GtkWindow* pWindow, const std::string& aTitle, std::map<std::string, std::string>& aEntries);

    void clipboardSetHtml(GtkClipboard* pClipboard, const char* pSelection);

    /// Generate an author string for multiple views.
    std::string getNextAuthor();

    GtkWidget* createCommentBox(const boost::property_tree::ptree& aComment);

    const std::string getDirPath(const std::string& filePath);

    const std::vector<int> splitIntoIntegers(const std::string& aPayload, const std::string& aDelim, const int nItems);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

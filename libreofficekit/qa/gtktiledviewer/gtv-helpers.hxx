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

#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <memory>

#include <boost/property_tree/ptree_fwd.hpp>

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

    std::string getDirPath(const std::string& filePath);

    template<typename T>
    std::vector<T> split(const std::string& aPayload, const std::string& aDelim, const int nItems)
    {
        std::vector<T> aRet;

        if (!aPayload.empty())
        {
            gchar** ppCoordinates = g_strsplit(aPayload.c_str(), aDelim.c_str(), nItems);
            gchar** ppCoordinate  = ppCoordinates;
            while (*ppCoordinate)
            {
                std::stringstream strstream(*ppCoordinate);
                T item;
                strstream >> item;
                aRet.push_back(item);
                ++ppCoordinate;
            }
            g_strfreev(ppCoordinates);
        }

        return aRet;
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

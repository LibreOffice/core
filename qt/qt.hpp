/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <QApplication>
#include <QWebEngineProfile>
#include "common/RecentFiles.hpp"

#include <Poco/File.h>
#include <Poco/Path.h>

// lo_kit is an opaque COKit handle; a forward declaration is all this header needs
// (matches kit/Kit.hpp).
struct COKitStruct;
using COKit = COKitStruct;

extern int coolwsd_server_socket_fd;
extern const char* user_name;
extern COKit* lo_kit;

/// Registers the QNetworkAccessManager-based HTTP transport used by the AI proxy
/// (AIChatSession) on the desktop. Call once at startup on the GUI thread.
void registerAIHttpTransport();

class Application
{
private:
    static QWebEngineProfile* globalProfile;
    static RecentFiles recentFiles;

public:
    static void initialize();
    static QWebEngineProfile* getProfile();
    static RecentFiles& getRecentFiles();
};

namespace
{
inline std::string getDataDir()
{
    Poco::Path dir(qApp->applicationDirPath().toStdString());
    if (Poco::File(Poco::Path(dir, "run-from-build")).exists())
    {
        dir.makeParent();
        return dir.toString();
    }
    return COOLWSD_DATADIR;
}
} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>

bool update();

void update_checker();

class Updater
{
public:
    static OUString getPatchDirURL();
    static OUString getUpdateFileURL();
    static OUString getExecutableDirURL();
    static OUString getInstallationPath();

    static OUString getBuildID();
    static OUString getUpdateChannel();

    static void log(const OUString& rMessage);
    static void log(const OString& rMessage);
    static void log(const char* pMessage);

    static void removeUpdateFiles();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

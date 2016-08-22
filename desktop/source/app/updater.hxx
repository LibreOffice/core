/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DESKTOP_SOURCE_APP_UPDATER_HXX
#define INCLUDED_DESKTOP_SOURCE_APP_UPDATER_HXX

#include <rtl/ustring.hxx>

void CreateValidUpdateDir(const OUString& rInstallDir);
void Update(const OUString& rInstallDir);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

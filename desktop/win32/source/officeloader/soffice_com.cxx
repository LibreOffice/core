/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#define WIN32_LEAN_AND_MEAN
#include "../loader.hxx"

int main(int /*argc*/, char** /*argv*/) { return desktop_win32::officeloader_impl(false); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

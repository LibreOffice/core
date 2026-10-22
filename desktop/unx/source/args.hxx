/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <sal/types.h>
#include <rtl/ustring.h>

struct Args
{
    rtl_uString* pAppPath = nullptr;
    const char* pPageinType = nullptr; // @pagein-writer for - writer etc. else NULL
    bool bInhibitSplash = false; // should we show a splash screen
    bool bInhibitPagein = false; // should we run pagein ?
    bool bInhibitJavaLdx = false; // should we run javaldx ?
    bool bInhibitPipe = false; // for --help and --version

    sal_uInt32 nArgsEnv = 0; // number of -env: style args
    sal_uInt32 nArgsTotal = 0; // number of -env: as well as -writer style args
    rtl_uString* ppArgs[1]; // sorted argument array
};

Args *args_parse (void);
void  args_free  (Args *args);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

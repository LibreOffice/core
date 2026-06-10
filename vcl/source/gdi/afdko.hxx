/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 * Copyright 2014-2018 Adobe Systems Incorporated (http://www.adobe.com/). All Rights Reserved.
 * This software is licensed as OpenSource, under the Apache License, Version 2.0.
 * This license is available at: http://opensource.org/licenses/Apache-2.0.
 */

#pragma once

// the afdko headers are not warning-clean
#if defined __GNUC__ || defined __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wshadow"
#endif
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100) // goadb.h error() ignores its message parameter
#pragma warning(disable : 4309) // varsupport.h passes 0x8000 to the int16_t writer w2
#endif

#include <afdko_version.h>
#include <tx_shared.h>
#include <hotconv.h>
#include <cb.h>

#if defined __GNUC__ || defined __clang__
#pragma GCC diagnostic pop
#endif
#if defined _MSC_VER
#pragma warning(pop)
#endif

// the mergefonts embedding interface added by extern_mergefonts.patch
extern "C" {
extern txCtx mergeFontsNew(void);
extern void mergeFontsFree(txCtx h);
extern void mergeFontsReadCIDFontInfo(txCtx h, const char* filePath);
extern int mergeFontsDoMergeFileSet(txCtx h, int argc, char* argv[], int i);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

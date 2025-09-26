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

#include <tx_shared.h>
#include <hotconv.h>
#include <cb.h>

extern "C" {
extern txCtx txNew(char* progname);
extern void txFree(txCtx h);
extern void cfrReadFont(txCtx h, long origin, int ttcIndex);

extern txCtx mergeFontsNew(char* progname);
extern void mergeFontsFree(txCtx h);
extern void readCIDFontInfo(txCtx h, char* filePath);
extern int doMergeFileSet(txCtx h, int argc, char* args[], int i);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

/* not protected, do only include once! */

#undef ABSOLUTE
#undef CompareString
#undef CopyFile
#undef DELETE
#undef DrawText
#undef FindWindow
#undef Folder
#undef GetCharWidth
#undef GetClassName
#undef GetGlyphOutline
#undef GetKerningPairs
#undef GetMetaFile
#undef GetNumberFormat
#undef GetObject
#undef GetPrinter
#undef GetTimeFormat
#undef GetUserName
#undef GradientStyle_RECT
#undef IGNORE
#undef IN
#undef OPAQUE // so it doesn't conflict with enum values
#undef OPTIONAL
#undef OUT
#undef PASSTHROUGH
#undef RELATIVE
#undef Rectangle
#undef STRICT
#undef SetPort
#undef SetPrinter
#undef WB_LEFT
#undef WB_RIGHT
#undef Yield
#undef mciSetCommand

/* new StretchBlt() Modes (simpler names) */
#ifndef STRETCH_ANDSCANS
#define STRETCH_ANDSCANS        1
#endif
#ifndef STRETCH_ORSCANS
#define STRETCH_ORSCANS         2
#endif
#ifndef STRETCH_DELETESCANS
#define STRETCH_DELETESCANS     3
#endif

#ifdef __cplusplus
extern "C"
{
BOOL WINAPI WIN_Rectangle( HDC hDC, int X1, int Y1, int X2, int Y2 );
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

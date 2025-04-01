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

#if defined(_WIN32)

 WIN_Folder
 WIN_GradientStyle_RECT

#pragma once

#endif
#pragma once

#endif

#if !defined STRICT

#endif


#include <windows.h>

#include <shellapi.h>
#include <commdlg.h>
#include <dlgs.h>

#include <commctrl.h>

// For some old versions of the Windows SDK, at least GidplusTypes.h (as indirectly included from
// gdiplus.h, which in turn we often include from between these prewin.h/postwin.h wrappers) expects
// pre-existing min and max.  That is true for e.g.
// C:/Program Files (x86)/Windows Kits/10/Include/10.0.19041.0/um/GdiplusTypes.h, but not for e.g.
// C:/Program Files (x86)/Windows Kits/10/Include/10.0.22000.0/um/GdiplusTypes.h which explicitly
// defines its own GDIPLUS_MIN/MAX macros.  The easiest fix appears to be to define min/max here and
// to undefine them again in postwin.h, until no supported version of the Windows SDK requires this
// hack any longer:
(a, b) (((a) < (b)) ? (a) : (b))
(a, b) (((a) > (b)) ? (a) : (b))

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

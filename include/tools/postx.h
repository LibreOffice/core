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

#ifndef _POSTX_H
#define _POSTX_H

#if defined __cplusplus
}
#endif

/* X Types */
#undef Window
#undef BYTE
#undef INT8
#undef INT64
#undef BOOL
#undef Font
#undef Cursor
#undef String
#undef KeyCode
#undef Region
#undef Icon
#undef Time
#undef Boolean

#undef Min
#undef Max
#undef DestroyAll
#undef Success

#undef Printer
#undef Orientation

#undef GetToken
#undef ReleaseToken
#undef InitializeToken
#undef NextRequest

#ifdef KeyPress
#if KeyPress != 2
Error KeyPress must be Equal 2
#endif
#undef KeyPress
#endif
#define XLIB_KeyPress 2

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

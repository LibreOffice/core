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

#define tblsels_mask_width 16
#define tblsels_mask_height 16
static unsigned char tblsels_mask_bits[] = {
   0x00, 0x00, 0xe0, 0x03, 0xe0, 0x03, 0xe0, 0x03, 0xe0, 0x03, 0xe0, 0x03,
   0xe0, 0x03, 0xe0, 0x03, 0xe0, 0x03, 0xfc, 0x1f, 0xfc, 0x1f, 0xf8, 0x0f,
   0xf0, 0x07, 0xe0, 0x03, 0xc0, 0x01, 0x80, 0x00 };

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

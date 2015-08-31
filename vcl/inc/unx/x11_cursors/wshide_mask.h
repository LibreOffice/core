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

#define hidewhitespace_mask_width 16
#define hidewhitespace_mask_height 16
#define hidewhitespace_mask_x_hot 0
#define hidewhitespace_mask_y_hot 10
static unsigned char hidewhitespace_mask_bits[] = {
  0x00, 0x01, 0x00, 0x01, 0xC0, 0x07, 0x80, 0x03, 0x00, 0x01, 0xFF, 0xFF,
  0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0xFF, 0xFF, 0x00, 0x01,
  0x80, 0x03, 0xC0, 0x07, 0x00, 0x01, 0x00, 0x01, };

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#ifndef INCLUDED_SW_INC_CRSSKIP_HXX
#define INCLUDED_SW_INC_CRSSKIP_HXX

#include <tools/solar.h>

// define for cursor travelling normally in western text cells and chars do
// the same, but in complex text cell skip over legatures and char skip
// into it.
// These defines exist only to cut off the dependencies to I18N project.
const sal_uInt16 CRSR_SKIP_CHARS  = 0;
const sal_uInt16 CRSR_SKIP_CELLS  = 1;
const sal_uInt16 CRSR_SKIP_HIDDEN = 2;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

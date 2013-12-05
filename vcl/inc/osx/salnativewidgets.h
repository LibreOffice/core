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

#ifndef INCLUDED_VCL_INC_OSX_SALNATIVEWIDGETS_H
#define INCLUDED_VCL_INC_OSX_SALNATIVEWIDGETS_H

// since 10.4 ... no metrics are returned, and we have to fix the values
#define BUTTON_WIDTH 16
#define BUTTON_HEIGHT 17

//standard height of the AHIG
//tabs
#define TAB_HEIGHT_NORMAL 20
#define TAB_HEIGHT_SMALL  17
#define TAB_HEIGHT_MINI   15

#define TAB_TEXT_OFFSET     12
#define VCL_TAB_TEXT_OFFSET 2

//listboxes, comboboxes (they have the same dimensions)
#define COMBOBOX_HEIGHT_NORMAL 20
#define DROPDOWN_BUTTON_WIDTH 20

//text edit
#define TEXT_EDIT_HEIGHT_NORMAL 22

//spin box
#define SPIN_BUTTON_SPACE           2
#define SPIN_BUTTON_WIDTH           13
#define SPIN_UPPER_BUTTON_HEIGHT    11
#define SPIN_LOWER_BUTTON_HEIGHT    10

// progress bar
#define INTRO_PROGRESS_HEIGHT 9

// for some controls, like spinbuttons + spinboxes, or listboxes
// we need it to adjust text position beside radio and check buttons

#define TEXT_SEPARATOR 3

// extra border for focus ring
#define FOCUS_RING_WIDTH 4

#define CLIP_FUZZ 1

#endif // INCLUDED_VCL_INC_OSX_SALNATIVEWIDGETS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

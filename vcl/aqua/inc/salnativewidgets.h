/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SV_NATIVEWIDGETS_H
#define _SV_NATIVEWIDGETS_H

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
#define SPIN_TWO_BUTTONS_HEIGHT     21

// progress bar
#define INTRO_PROGRESS_HEIGHT 9

// for some controls, like spinbuttons + spinboxes, or listboxes
// we need it to adjust text position beside radio and check buttons

#define TEXT_SEPARATOR 3

// extra border for focus ring
#define FOCUS_RING_WIDTH 4

#define CLIP_FUZZ 1

#endif  // _SV_NATIVEWIDGETS_H

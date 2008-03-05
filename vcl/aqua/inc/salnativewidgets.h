/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salnativewidgets.h,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:55:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

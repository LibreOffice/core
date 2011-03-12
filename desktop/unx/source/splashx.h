/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *               Novell, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Jan Holesovsky <kendy@novell.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#ifndef _SPLASHX_H
#define _SPLASHX_H

#ifdef __cplusplus
extern "C" {
#endif

// Load the specified bitmap so we can have as a background of the
// splash.
//
// Note: Must be called before the create_window(), otherwise there will be no
// image in the splash, just black rectangle.
//
// Return: 1 - success, 0 - failure (non-existing, etc.)
int splash_load_bmp( const char *filename );

// Init some of the values
// If not called, the defaults are used
// barc, framec - colors, posx, posy - position, w, h - size
void splash_setup( int barc[3], int framec[3], int posx, int posy, int w, int h );

// Create the splash window
// Return: 1 - success, 0 - failure
int splash_create_window( int argc, char** argv );

// Destroy the splash window
void splash_close_window();

// Update the progress bar
void splash_draw_progress( int progress );

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _SPLASHX_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

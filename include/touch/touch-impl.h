/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TOUCH_TOUCH_IMPL_H
#define INCLUDED_TOUCH_TOUCH_IMPL_H

#ifdef __cplusplus
extern "C" {
#if 0
} // To avoid an editor indenting all inside the extern "C"
#endif
#endif

// "Implementation" of touch_lo_* functions, called on the LO thread through
// the PostUserEvent mechanism. Not called by UI thread code.

void touch_lo_selection_start_move_impl(const void *documentHandle,
                                        int x,
                                        int y);

void touch_lo_selection_end_move_impl(const void *documentHandle,
                                      int x,
                                      int y);

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_TOUCH_TOUCH_IMPL_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

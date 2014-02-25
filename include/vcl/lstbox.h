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

#ifndef INCLUDED_VCL_LSTBOX_H
#define INCLUDED_VCL_LSTBOX_H

#define LISTBOX_APPEND              ((sal_uInt16)0xFFFF)
#define LISTBOX_ENTRY_NOTFOUND      ((sal_uInt16)0xFFFF)
#define LISTBOX_ERROR               ((sal_uInt16)0xFFFF)



// the following defines can be used for the SetEntryFlags()
// and GetEntryFlags() methods

// !! Do not use these flags for user data as they are reserved      !!
// !! to change the internal behaviour of the ListBox implementation !!
// !! for specific entries.                                          !!

/** this flag disables a selection of an entry completely. It is not
    possible to select such entries either from the user interface
    nor from the ListBox methods. Cursor traveling is handled correctly.
    This flag can be used to add titles to a ListBox.
*/
#define LISTBOX_ENTRY_FLAG_DISABLE_SELECTION        0x0000001

/** this flag can be used to make an entry multine capable
    A normal entry is single line and will therefore be clipped
    at the right listbox border. Setting this flag enables
    word breaks for the entry text.
*/
#define LISTBOX_ENTRY_FLAG_MULTILINE                0x0000002

/** this flags lets the item be drawn disabled (e.g. in grey text)
    usage only guaranteed with LISTBOX_ENTRY_FLAG_DISABLE_SELECTION
*/
#define LISTBOX_ENTRY_FLAG_DRAW_DISABLED            0x0000004

#endif // INCLUDED_VCL_LSTBOX_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

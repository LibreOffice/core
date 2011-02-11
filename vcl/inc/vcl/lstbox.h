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

#ifndef _SV_LSTBOX_H
#define _SV_LSTBOX_H

#define LISTBOX_APPEND              ((sal_uInt16)0xFFFF)
#define LISTBOX_ENTRY_NOTFOUND      ((sal_uInt16)0xFFFF)
#define LISTBOX_ERROR               ((sal_uInt16)0xFFFF)

#define LISTBOX_USERDRAW_SELECTED   ((sal_uInt16)0x0001)

// --------------------------------------------------------------------

// the following defines can be used for the SetEntryFlags()
// and GetEntryFlags() methods

// !! Do not use these flags for user data as they are reserved      !!
// !! to change the internal behaviour of the ListBox implementation !!
// !! for specific entries.                                          !!

/** this flag disables a selection of an entry completly. It is not
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

#endif  // _SV_LSTBOX_H

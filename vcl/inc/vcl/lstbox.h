/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lstbox.h,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-12 15:10:03 $
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

#ifndef _SV_LSTBOX_H
#define _SV_LSTBOX_H

#define LISTBOX_APPEND              ((USHORT)0xFFFF)
#define LISTBOX_ENTRY_NOTFOUND      ((USHORT)0xFFFF)
#define LISTBOX_ERROR               ((USHORT)0xFFFF)

#define LISTBOX_USERDRAW_SELECTED   ((USHORT)0x0001)

// --------------------------------------------------------------------

// the following defines can be used for the SetEntryFlags()
// and GetEntryFlags() methods

// !! Do not use these flags for user data as they are reserved      !!
// !! to change the internal behaviour of the ListBox implementation !!
// !! for specific entries.                                          !!

/** this flag disables a selection of an entry completly. It is not
    possible to select such entries either from the user interface
    nor from the ListBox methods. Cursor traveling is handled correctly.
    This flag can be used to at titles to a ListBox.
*/
#define LISTBOX_ENTRY_FLAG_DISABLE_SELECTION        0x0000001

#endif  // _SV_LSTBOX_H

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _TOOLS_RCID_H
#define _TOOLS_RCID_H

#include <tools/resid.hxx>

// Definition der Versionsnummer
#define RSCVERSION_ID           200U
// Definition eines ungueltige Identifiers.
#define RC_NO_ID                (sal_uInt32(0xFFFFFFFF))

// Dies sind die Resourcetypen
// Mindestens 0x100 wegen MS-Windows Resourcetypen
//#define RSC_NOTYPE              0x100
//RSC_NOTYPE wird in resid.hxx definiert
#define RSC_BYNAME              (RSC_NOTYPE + 0x01)
#define RSC_VERSIONCONTROL      (RSC_NOTYPE + 0x02) // Versionskontrolle

#define RSC_RESOURCE            (RSC_NOTYPE + 0x10)
#define RSC_STRING              (RSC_NOTYPE + 0x11)
#define RSC_BITMAP              (RSC_NOTYPE + 0x13)
#define RSC_ACCEL               (RSC_NOTYPE + 0x1a)
#define RSC_ACCELITEM           (RSC_NOTYPE + 0x1b)// nur intern
#define RSC_MENU                (RSC_NOTYPE + 0x1c)
#define RSC_MENUITEM            (RSC_NOTYPE + 0x1d)// nur intern
#define RSC_KEYCODE             (RSC_NOTYPE + 0x1f)
#define RSC_TIME                (RSC_NOTYPE + 0x20)
#define RSC_DATE                (RSC_NOTYPE + 0x21)
//#define RSC_INTERNATIONAL     (RSC_NOTYPE + 0x22) // erAck: got rid of class International (2005-06-17)
#define RSC_IMAGE               (RSC_NOTYPE + 0x23)
#define RSC_IMAGELIST           (RSC_NOTYPE + 0x24)

#define RSC_MESSBOX             (RSC_NOTYPE + 0x30)
#define RSC_INFOBOX             (RSC_NOTYPE + 0x31)
#define RSC_WARNINGBOX          (RSC_NOTYPE + 0x32)
#define RSC_ERRORBOX            (RSC_NOTYPE + 0x33)
#define RSC_QUERYBOX            (RSC_NOTYPE + 0x34)
#define RSC_WINDOW              (RSC_NOTYPE + 0x35)
#define RSC_SYSWINDOW           (RSC_NOTYPE + 0x36)
#define RSC_WORKWIN             (RSC_NOTYPE + 0x37)
#define RSC_MDIWINDOW           (RSC_NOTYPE + 0x38)
#define RSC_FLOATINGWINDOW      (RSC_NOTYPE + 0x39)
#define RSC_DIALOG              (RSC_NOTYPE + 0x3a)
#define RSC_MODELESSDIALOG      (RSC_NOTYPE + 0x3b)
#define RSC_MODALDIALOG         (RSC_NOTYPE + 0x3c)
#define RSC_CONTROL             (RSC_NOTYPE + 0x44)
#define RSC_BUTTON              (RSC_NOTYPE + 0x45)
#define RSC_PUSHBUTTON          (RSC_NOTYPE + 0x46)
#define RSC_OKBUTTON            (RSC_NOTYPE + 0x47)
#define RSC_CANCELBUTTON        (RSC_NOTYPE + 0x48)
#define RSC_HELPBUTTON          (RSC_NOTYPE + 0x49)
#define RSC_IMAGEBUTTON         (RSC_NOTYPE + 0x4a)
#define RSC_MENUBUTTON          (RSC_NOTYPE + 0x4b)
#define RSC_MOREBUTTON          (RSC_NOTYPE + 0x4c)
#define RSC_SPINBUTTON          (RSC_NOTYPE + 0x4d)
#define RSC_RADIOBUTTON         (RSC_NOTYPE + 0x4e)
#define RSC_IMAGERADIOBUTTON    (RSC_NOTYPE + 0x4f)
#define RSC_CHECKBOX            (RSC_NOTYPE + 0x50)
#define RSC_TRISTATEBOX         (RSC_NOTYPE + 0x51)
#define RSC_EDIT                (RSC_NOTYPE + 0x52)
#define RSC_MULTILINEEDIT       (RSC_NOTYPE + 0x53)
#define RSC_COMBOBOX            (RSC_NOTYPE + 0x54)
#define RSC_LISTBOX             (RSC_NOTYPE + 0x55)
#define RSC_MULTILISTBOX        (RSC_NOTYPE + 0x56)
#define RSC_TEXT                (RSC_NOTYPE + 0x57)
#define RSC_FIXEDLINE           (RSC_NOTYPE + 0x58)
#define RSC_FIXEDBITMAP         (RSC_NOTYPE + 0x59)
#define RSC_FIXEDIMAGE          (RSC_NOTYPE + 0x5a)
#define RSC_GROUPBOX            (RSC_NOTYPE + 0x5c)
#define RSC_SCROLLBAR           (RSC_NOTYPE + 0x5d)
#define RSC_SCROLLBARBOX        (RSC_NOTYPE + 0x5e)
#define RSC_SPLITTER            (RSC_NOTYPE + 0x5f)
#define RSC_SPLITWINDOW         (RSC_NOTYPE + 0x60)
#define RSC_SPINFIELD           (RSC_NOTYPE + 0x61)
#define RSC_PATTERNFIELD        (RSC_NOTYPE + 0x62)
#define RSC_NUMERICFIELD        (RSC_NOTYPE + 0x63)
#define RSC_METRICFIELD         (RSC_NOTYPE + 0x64)
#define RSC_CURRENCYFIELD       (RSC_NOTYPE + 0x65)
#define RSC_DATEFIELD           (RSC_NOTYPE + 0x66)
#define RSC_TIMEFIELD           (RSC_NOTYPE + 0x67)
#define RSC_PATTERNBOX          (RSC_NOTYPE + 0x68)
#define RSC_NUMERICBOX          (RSC_NOTYPE + 0x69)
#define RSC_METRICBOX           (RSC_NOTYPE + 0x6a)
#define RSC_CURRENCYBOX         (RSC_NOTYPE + 0x6b)
#define RSC_DATEBOX             (RSC_NOTYPE + 0x6c)
#define RSC_TIMEBOX             (RSC_NOTYPE + 0x6d)
#define RSC_LONGCURRENCYFIELD   (RSC_NOTYPE + 0x6e)
#define RSC_LONGCURRENCYBOX     (RSC_NOTYPE + 0x6f)
#define RSC_TOOLBOXITEM         (RSC_NOTYPE + 0x70)
#define RSC_TOOLBOX             (RSC_NOTYPE + 0x71)
#define RSC_DOCKINGWINDOW       (RSC_NOTYPE + 0x72)
#define RSC_STATUSBAR           (RSC_NOTYPE + 0x73)
#define RSC_TABPAGE             (RSC_NOTYPE + 0x74)
#define RSC_TABCONTROL          (RSC_NOTYPE + 0x75)
#define RSC_TABDIALOG           (RSC_NOTYPE + 0x76)
#define RSC_TABCONTROLITEM      (RSC_NOTYPE + 0x77)
#define RSC_TREELISTBOX         (RSC_NOTYPE + 0x78)
#define RSC_STRINGARRAY         (RSC_NOTYPE + 0x79)

// (RSC_NOTYPE + 0x200) - (RSC_NOTYPE + 0x300) fuer Sfx reserviert

#define RT_SYS_BITMAP           (RSC_NOTYPE + 0xf2)
#define RT_SYS_STRING           (RSC_NOTYPE + 0xf3)     // Sonderbehandlung
#define RT_SYS_FILE             (RSC_NOTYPE + 0xf4)     // nicht implementiert

// (RSC_NOTYPE + 0x200) - (RSC_NOTYPE + 0x300) fuer Sfx reserviert

#endif // _RCID_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

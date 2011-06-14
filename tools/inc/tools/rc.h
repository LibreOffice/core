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

#ifndef _TOOLS_RC_H
#define _TOOLS_RC_H

#include <tools/rcid.h>

// Definition der Struktur die alle "Window"-Resourcen haben
// Hier sind die Fensterstyles definiert die nicht in WinBits sind
#define RSC_EXTRAMAPUNIT                0xFFFF //Wert nicht veraendern!!!

typedef short RSWND_STYLE;
#define RSWND_DISABLED                  0x01
#define RSWND_CLIENTSIZE                0x02

#define WINDOW_XYMAPMODE                0x0001
#define WINDOW_X                        0x0002
#define WINDOW_Y                        0x0004
#define WINDOW_WHMAPMODE                0x0008
#define WINDOW_WIDTH                    0x0010
#define WINDOW_HEIGHT                   0x0020
#define WINDOW_TEXT                     0x0080
#define WINDOW_HELPTEXT                 0x0100
#define WINDOW_QUICKTEXT                0x0200
#define WINDOW_EXTRALONG                0x0800
#define WINDOW_UNIQUEID                 0x1000
#define WINDOW_BORDER_STYLE             0x2000
#define WINDOW_HELPID                   0x4000

// Definition der Struktur die alle "WorkWindow"-Resourcen haben
#define WORKWIN_SHOWNORMAL              0
#define WORKWIN_SHOWMINIMIZED           1
#define WORKWIN_SHOWMAXIMIZED           2

// Definition der Struktur der FixedBitmap-Resource.
#define RSC_FIXEDBITMAP_BITMAP          0x0001

// Definition der Struktur der FixedImage-Resource.
#define RSC_FIXEDIMAGE_IMAGE            0x0001

// Definition der Struktur des MenuButtons
#define RSCMENUBUTTON_MENU              0x01

// Definition der Struktur die alle Menu-Resourcen haben
#define RSC_MENU_ITEMS                  0x01
#define RSC_MENU_TEXT                   0x02
#define RSC_MENU_DEFAULTITEMID          0x04

// Hier sind die MenuItem-Resourceoptionen definiert
#define RSC_MENUITEM_SEPARATOR          0x001
#define RSC_MENUITEM_ID                 0x002
#define RSC_MENUITEM_STATUS             0x004
#define RSC_MENUITEM_TEXT               0x008
#define RSC_MENUITEM_BITMAP             0x010
#define RSC_MENUITEM_HELPTEXT           0x020
#define RSC_MENUITEM_HELPID             0x040
#define RSC_MENUITEM_KEYCODE            0x080
#define RSC_MENUITEM_MENU               0x100
#define RSC_MENUITEM_CHECKED            0x200
#define RSC_MENUITEM_DISABLE            0x400
#define RSC_MENUITEM_COMMAND            0x800

// Hier sind die AccelKey-Resourceoptionen definiert
#define ACCELITEM_ACCEL                 0x01
#define ACCELITEM_KEY                   0x02

// Definition der Struktur die alle "Field"-Resourcen haben
#define PATTERNFORMATTER_STRICTFORMAT   0x01
#define PATTERNFORMATTER_EDITMASK       0x02
#define PATTERNFORMATTER_LITTERALMASK   0x04

#define NUMERICFORMATTER_MIN            0x01
#define NUMERICFORMATTER_MAX            0x02
#define NUMERICFORMATTER_STRICTFORMAT   0x04
//#define NUMERICFORMATTER_I12          0x08    // erAck: got rid of class International (2005-06-17)
#define NUMERICFORMATTER_DECIMALDIGITS  0x10
#define NUMERICFORMATTER_VALUE          0x20
#define NUMERICFORMATTER_NOTHOUSANDSEP  0x40

#define METRICFORMATTER_UNIT            0x01
#define METRICFORMATTER_CUSTOMUNITTEXT  0x02

#define DATEFORMATTER_MIN               0x01
#define DATEFORMATTER_MAX               0x02
#define DATEFORMATTER_LONGFORMAT        0x04
#define DATEFORMATTER_STRICTFORMAT      0x08
//#define DATEFORMATTER_I12             0x10    // erAck: got rid of class International (2005-06-17)
#define DATEFORMATTER_VALUE             0x20

#define TIMEFORMATTER_MIN               0x01
#define TIMEFORMATTER_MAX               0x02
#define TIMEFORMATTER_TIMEFIELDFORMAT   0x04
#define TIMEFORMATTER_DURATION          0x08
#define TIMEFORMATTER_STRICTFORMAT      0x10
//#define TIMEFORMATTER_I12             0x20    // erAck: got rid of class International (2005-06-17)
#define TIMEFORMATTER_VALUE             0x40

#define NUMERICFIELD_FIRST              0x01
#define NUMERICFIELD_LAST               0x02
#define NUMERICFIELD_SPINSIZE           0x04

#define METRICFIELD_FIRST               0x01
#define METRICFIELD_LAST                0x02
#define METRICFIELD_SPINSIZE            0x04

#define CURRENCYFIELD_FIRST             0x01
#define CURRENCYFIELD_LAST              0x02
#define CURRENCYFIELD_SPINSIZE          0x04

#define DATEFIELD_FIRST                 0x01
#define DATEFIELD_LAST                  0x02

#define TIMEFIELD_FIRST                 0x01
#define TIMEFIELD_LAST                  0x02

// Definition der Struktur die alle "ToolBoxItem"-Resourcen haben
#define RSC_TOOLBOXITEM_ID              0x0001
#define RSC_TOOLBOXITEM_TYPE            0x0002
#define RSC_TOOLBOXITEM_STATUS          0x0004
#define RSC_TOOLBOXITEM_HELPID          0x0008
#define RSC_TOOLBOXITEM_TEXT            0x0010
#define RSC_TOOLBOXITEM_HELPTEXT        0x0020
#define RSC_TOOLBOXITEM_BITMAP          0x0040
#define RSC_TOOLBOXITEM_IMAGE           0x0080
#define RSC_TOOLBOXITEM_DISABLE         0x0100
#define RSC_TOOLBOXITEM_STATE           0x0200
#define RSC_TOOLBOXITEM_HIDE            0x0400
#define RSC_TOOLBOXITEM_COMMAND         0x0800
#define RSC_TOOLBOXITEM_MENUSTRINGS     0x1000

// Definition der Struktur die alle ToolBox-Resourcen haben
#define RSC_TOOLBOX_BUTTONTYPE          0x01
#define RSC_TOOLBOX_ALIGN               0x02
#define RSC_TOOLBOX_LINECOUNT           0x04
#define RSC_TOOLBOX_FLOATLINES          0x08
#define RSC_TOOLBOX_CUSTOMIZE           0x10
#define RSC_TOOLBOX_MENUSTRINGS         0x20
#define RSC_TOOLBOX_ITEMIMAGELIST       0x40
#define RSC_TOOLBOX_ITEMLIST            0x80

// Definition der Struktur die alle MoreButton-Resourcen haben
#define RSC_MOREBUTTON_STATE            0x01
#define RSC_MOREBUTTON_MAPUNIT          0x02
#define RSC_MOREBUTTON_DELTA            0x04

// Definition der Struktur die alle FloatingWindow-Resourcen haben
#define RSC_FLOATINGWINDOW_WHMAPMODE    0x01
#define RSC_FLOATINGWINDOW_WIDTH        0x02
#define RSC_FLOATINGWINDOW_HEIGHT       0x04
#define RSC_FLOATINGWINDOW_ZOOMIN       0x08

// Definition der Struktur die alle DockingWindow-Resourcen haben
#define RSC_DOCKINGWINDOW_XYMAPMODE     0x01
#define RSC_DOCKINGWINDOW_X             0x02
#define RSC_DOCKINGWINDOW_Y             0x04
#define RSC_DOCKINGWINDOW_FLOATING      0x08

// Definition der Struktur die alle "TabControlItem"-Resourcen haben
#define RSC_TABCONTROLITEM_ID           0x0001
#define RSC_TABCONTROLITEM_TEXT         0x0002
#define RSC_TABCONTROLITEM_PAGERESID    0x0008

// Definition der Struktur die alle TabControl-Resourcen haben
#define RSC_TABCONTROL_ITEMLIST         0x01

// Definition der Struktur des ImageButtons
#define RSC_IMAGEBUTTON_IMAGE           0x01
#define RSC_IMAGEBUTTON_SYMBOL          0x02
#define RSC_IMAGEBUTTON_STATE           0x04

// Definition der Struktur des ImageRadioButtons
#define RSC_IMAGERADIOBUTTON_IMAGE      0x01

// Definition der Struktur des Image
#define RSC_IMAGE_IMAGEBITMAP           0x01
#define RSC_IMAGE_MASKBITMAP            0x02
#define RSC_IMAGE_MASKCOLOR             0x04

// Definition der Struktur des Image
#define RSC_IMAGELIST_IMAGEBITMAP       0x01
#define RSC_IMAGELIST_MASKBITMAP        0x02
#define RSC_IMAGELIST_MASKCOLOR         0x04
#define RSC_IMAGELIST_IDLIST            0x08
#define RSC_IMAGELIST_IDCOUNT           0x10

// obsolete, should be removed by MM
#define RSC_COLOR                       (RSC_NOTYPE + 0x16)
#define TIME_HOUR                       0x01
#define TIME_MINUTE                     0x02
#define TIME_SECOND                     0x04
#define TIME_SEC100                     0x08
#define DATE_YEAR                       0x01
#define DATE_MONTH                      0x02
#define DATE_DAY                        0x04

#endif // _TOOLS_RC_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

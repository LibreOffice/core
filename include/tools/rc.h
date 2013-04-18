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

#ifndef _TOOLS_RC_H
#define _TOOLS_RC_H

#include <tools/rcid.h>

// Defines for all "Window" resources.
// (Here are all window styles that are not in WinBits)
#define RSC_EXTRAMAPUNIT                0xFFFF // do not change that value!

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

// For "WorkWindow" resources:
#define WORKWIN_SHOWNORMAL              0
#define WORKWIN_SHOWMINIMIZED           1
#define WORKWIN_SHOWMAXIMIZED           2

// For "FixedBitmap" resource:
#define RSC_FIXEDBITMAP_BITMAP          0x0001

// For "FixedImage" resource:
#define RSC_FIXEDIMAGE_IMAGE            0x0001

// For "MenuButton" resources:
#define RSCMENUBUTTON_MENU              0x01

// For all menu resources:
#define RSC_MENU_ITEMS                  0x01
#define RSC_MENU_TEXT                   0x02
#define RSC_MENU_DEFAULTITEMID          0x04

// "MenuItem" resource options:
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

// "AccelKey" resource options:
#define ACCELITEM_ACCEL                 0x01
#define ACCELITEM_KEY                   0x02

// For "Field" resources:
#define PATTERNFORMATTER_STRICTFORMAT   0x01
#define PATTERNFORMATTER_EDITMASK       0x02
#define PATTERNFORMATTER_LITTERALMASK   0x04

#define NUMERICFORMATTER_MIN            0x01
#define NUMERICFORMATTER_MAX            0x02
#define NUMERICFORMATTER_STRICTFORMAT   0x04
//#define NUMERICFORMATTER_I12          0x08 // removed (2005-06-17)
#define NUMERICFORMATTER_DECIMALDIGITS  0x10
#define NUMERICFORMATTER_VALUE          0x20
#define NUMERICFORMATTER_NOTHOUSANDSEP  0x40

#define METRICFORMATTER_UNIT            0x01
#define METRICFORMATTER_CUSTOMUNITTEXT  0x02

#define DATEFORMATTER_MIN               0x01
#define DATEFORMATTER_MAX               0x02
#define DATEFORMATTER_LONGFORMAT        0x04
#define DATEFORMATTER_STRICTFORMAT      0x08
//#define DATEFORMATTER_I12             0x10 // removed (2005-06-17)
#define DATEFORMATTER_VALUE             0x20

#define TIMEFORMATTER_MIN               0x01
#define TIMEFORMATTER_MAX               0x02
#define TIMEFORMATTER_TIMEFIELDFORMAT   0x04
#define TIMEFORMATTER_DURATION          0x08
#define TIMEFORMATTER_STRICTFORMAT      0x10
//#define TIMEFORMATTER_I12             0x20 // removed (2005-06-17)
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

// For "ToolBoxItem" resources:
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

// For "ToolBox" resources:
#define RSC_TOOLBOX_BUTTONTYPE          0x01
#define RSC_TOOLBOX_ALIGN               0x02
#define RSC_TOOLBOX_LINECOUNT           0x04
#define RSC_TOOLBOX_FLOATLINES          0x08
#define RSC_TOOLBOX_CUSTOMIZE           0x10
#define RSC_TOOLBOX_MENUSTRINGS         0x20
#define RSC_TOOLBOX_ITEMIMAGELIST       0x40
#define RSC_TOOLBOX_ITEMLIST            0x80

// For "MoreButton" resources:
#define RSC_MOREBUTTON_STATE            0x01
#define RSC_MOREBUTTON_MAPUNIT          0x02
#define RSC_MOREBUTTON_DELTA            0x04

// For "FloatingWindow" resources:
#define RSC_FLOATINGWINDOW_WHMAPMODE    0x01
#define RSC_FLOATINGWINDOW_WIDTH        0x02
#define RSC_FLOATINGWINDOW_HEIGHT       0x04
#define RSC_FLOATINGWINDOW_ZOOMIN       0x08

// For "DockingWindow" resources:
#define RSC_DOCKINGWINDOW_XYMAPMODE     0x01
#define RSC_DOCKINGWINDOW_X             0x02
#define RSC_DOCKINGWINDOW_Y             0x04
#define RSC_DOCKINGWINDOW_FLOATING      0x08

// For "TabControlItem" resources:
#define RSC_TABCONTROLITEM_ID           0x0001
#define RSC_TABCONTROLITEM_TEXT         0x0002
#define RSC_TABCONTROLITEM_PAGERESID    0x0008

// For "TabControl" resources:
#define RSC_TABCONTROL_ITEMLIST         0x01

// For "ImageButtons":
#define RSC_IMAGEBUTTON_IMAGE           0x01
#define RSC_IMAGEBUTTON_SYMBOL          0x02
#define RSC_IMAGEBUTTON_STATE           0x04

// For "ImageRadioButtons":
#define RSC_IMAGERADIOBUTTON_IMAGE      0x01

// For "Image":
#define RSC_IMAGE_IMAGEBITMAP           0x01
#define RSC_IMAGE_MASKBITMAP            0x02
#define RSC_IMAGE_MASKCOLOR             0x04

// For "ImageList":
#define RSC_IMAGELIST_IMAGEBITMAP       0x01
#define RSC_IMAGELIST_MASKBITMAP        0x02
#define RSC_IMAGELIST_MASKCOLOR         0x04
#define RSC_IMAGELIST_IDLIST            0x08
#define RSC_IMAGELIST_IDCOUNT           0x10

// FIXME obsolete, should be removed by MM
#define RSC_COLOR                       (RSC_NOTYPE + 0x16)
#define TIME_HOUR                       0x01
#define TIME_MINUTE                     0x02
#define TIME_SECOND                     0x04
#define TIME_SEC100                     0x08
#define DATE_YEAR                       0x01
#define DATE_MONTH                      0x02
#define DATE_DAY                        0x04

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

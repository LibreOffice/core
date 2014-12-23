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
#include <sfx2/sfxcommands.h>
#include <svx/svxcommands.h>
#include "sdcommands.h"

#define HID_SDDRAWVIEWSHELL                                     "SD_HID_SDDRAWVIEWSHELL"
#define HID_SDGRAPHICVIEWSHELL                                  "SD_HID_SDGRAPHICVIEWSHELL"
#define HID_SDOUTLINEVIEWSHELL                                  "SD_HID_SDOUTLINEVIEWSHELL"
#define HID_SD_WIN_DOCUMENT                                     "SD_HID_SD_WIN_DOCUMENT"
#define HID_SD_NAVIGATOR_TLB                                    "SD_HID_SD_NAVIGATOR_TLB"
#define HID_SD_NAVIGATOR                                        "SD_HID_SD_NAVIGATOR"
#define HID_SD_NAVIGATOR_TBX                                    "SD_HID_SD_NAVIGATOR_TBX"
#define HID_SD_NAVIGATOR_LB                                     "SD_HID_SD_NAVIGATOR_LB"
#define HID_SD_NAVIGATOR_TBI_PEN                                "SD_HID_SD_NAVIGATOR_TBI_PEN"
#define HID_SD_NAVIGATOR_TBI_FIRST                              "SD_HID_SD_NAVIGATOR_TBI_FIRST"
#define HID_SD_NAVIGATOR_TBI_PREV                               "SD_HID_SD_NAVIGATOR_TBI_PREV"
#define HID_SD_NAVIGATOR_TBI_NEXT                               "SD_HID_SD_NAVIGATOR_TBI_NEXT"
#define HID_SD_NAVIGATOR_TBI_LAST                               "SD_HID_SD_NAVIGATOR_TBI_LAST"
#define HID_SD_NAVIGATOR_TBI_DRAGTYPE                           "SD_HID_SD_NAVIGATOR_TBI_DRAGTYPE"
#define HID_SD_NAVIGATOR_TBI_SHAPE_FILTER                       "SD_HID_SD_NAVIGATOR_TBI_SHAPE_FILTER"

// tool boxes
#define HID_SD_DRAW_TOOLBOX                                     "SD_HID_SD_DRAW_TOOLBOX"
#define HID_SD_DRAW_OBJ_TOOLBOX                                 "SD_HID_SD_DRAW_OBJ_TOOLBOX"
#define HID_SD_DRAW_OPTIONS_TOOLBOX                             "SD_HID_SD_DRAW_OPTIONS_TOOLBOX"
#define HID_SD_TEXT_TOOLBOX                                     "SD_HID_SD_TEXT_TOOLBOX"

#define HID_SD_NAVIGATOR_MENU1                                  "SD_HID_SD_NAVIGATOR_MENU1"
#define HID_SD_NAVIGATOR_MENU2                                  "SD_HID_SD_NAVIGATOR_MENU2"
#define HID_SD_NAVIGATOR_MENU3                                  "SD_HID_SD_NAVIGATOR_MENU3"

#define HID_SD_WIN_PRESENTATION                                 "SD_HID_SD_WIN_PRESENTATION"


#define HID_SD_HTMLEXPORT_PAGE1                                 "SD_HID_SD_HTMLEXPORT_PAGE1"
#define HID_SD_HTMLEXPORT_PAGE2                                 "SD_HID_SD_HTMLEXPORT_PAGE2"
#define HID_SD_HTMLEXPORT_PAGE3                                 "SD_HID_SD_HTMLEXPORT_PAGE3"
#define HID_SD_HTMLEXPORT_PAGE4                                 "SD_HID_SD_HTMLEXPORT_PAGE4"
#define HID_SD_HTMLEXPORT_PAGE5                                 "SD_HID_SD_HTMLEXPORT_PAGE5"
#define HID_SD_HTMLEXPORT_PAGE6                                 "SD_HID_SD_HTMLEXPORT_PAGE6"



#define HID_SD_TABBAR_PAGES                                     "SD_HID_SD_TABBAR_PAGES"
#define HID_SD_TABBAR_LAYERS                                    "SD_HID_SD_TABBAR_LAYERS"


#define HID_SD_NAMEDIALOG_OBJECT                                "SD_HID_SD_NAMEDIALOG_OBJECT"
#define HID_SD_NAMEDIALOG_PAGE                                  "SD_HID_SD_NAMEDIALOG_PAGE"
#define HID_SD_NAMEDIALOG_LINEEND                               "SD_HID_SD_NAMEDIALOG_LINEEND"




#define HID_SD_TASK_PANE_PREVIEW_CURRENT                        "SD_HID_SD_TASK_PANE_PREVIEW_CURRENT"
#define HID_SD_TASK_PANE_PREVIEW_RECENT                         "SD_HID_SD_TASK_PANE_PREVIEW_RECENT"
#define HID_SD_TASK_PANE_PREVIEW_ALL                            "SD_HID_SD_TASK_PANE_PREVIEW_ALL"
#define HID_SD_TASK_PANE_PREVIEW_LAYOUTS                        "SD_HID_SD_TASK_PANE_PREVIEW_LAYOUTS"

#define HID_SD_CUSTOMANIMATIONPANE_PRESETPROPERTYBOX            "SD_HID_SD_CUSTOMANIMATIONPANE_PRESETPROPERTYBOX"
#define HID_SD_CUSTOMANIMATIONPANE_COLORPROPERTYBOX             "SD_HID_SD_CUSTOMANIMATIONPANE_COLORPROPERTYBOX"
#define HID_SD_CUSTOMANIMATIONPANE_FONTPROPERTYBOX              "SD_HID_SD_CUSTOMANIMATIONPANE_FONTPROPERTYBOX"
#define HID_SD_CUSTOMANIMATIONPANE_CHARHEIGHTPROPERTYBOX        "SD_HID_SD_CUSTOMANIMATIONPANE_CHARHEIGHTPROPERTYBOX"
#define HID_SD_CUSTOMANIMATIONPANE_ROTATIONPROPERTYBOX          "SD_HID_SD_CUSTOMANIMATIONPANE_ROTATIONPROPERTYBOX"
#define HID_SD_CUSTOMANIMATIONPANE_TRANSPARENCYPROPERTYBOX      "SD_HID_SD_CUSTOMANIMATIONPANE_TRANSPARENCYPROPERTYBOX"
#define HID_SD_CUSTOMANIMATIONPANE_SCALEPROPERTYBOX             "SD_HID_SD_CUSTOMANIMATIONPANE_SCALEPROPERTYBOX"
#define HID_SD_CUSTOMANIMATIONPANE_FONTSTYLEPROPERTYBOX         "SD_HID_SD_CUSTOMANIMATIONPANE_FONTSTYLEPROPERTYBOX"


#define HID_SD_RULER_HORIZONTAL                                 "SD_HID_SD_RULER_HORIZONTAL"
#define HID_SD_RULER_VERTICAL                                   "SD_HID_SD_RULER_VERTICAL"


#define HID_MN_SUB_GRAPHIC                                      "SD_HID_MN_SUB_GRAPHIC"
#define HID_MN_SUB_FIELD                                        "SD_HID_MN_SUB_FIELD"
#define HID_MN_SUB_TEMPLATES                                    "SD_HID_MN_SUB_TEMPLATES"
#define HID_MN_SUB_SPELLING                                     "SD_HID_MN_SUB_SPELLING"
#define HID_MN_SUB_CONVERT                                      "SD_HID_MN_SUB_CONVERT"
#define HID_MN_SUB_MIRROR                                       "SD_HID_MN_SUB_MIRROR"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

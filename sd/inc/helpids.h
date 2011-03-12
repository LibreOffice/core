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
#include <sfx2/sfxcommands.h>
#include <svx/svxcommands.h>
#include "sdcommands.h"

#define HID_SDDRAWVIEWSHELL                                     "SD_HID_SDDRAWVIEWSHELL"
#define HID_SDGRAPHICVIEWSHELL                                  "SD_HID_SDGRAPHICVIEWSHELL"
#define HID_SDOUTLINEVIEWSHELL                                  "SD_HID_SDOUTLINEVIEWSHELL"
#define HID_SD_PRINT_OPTIONS                                    "SD_HID_SD_PRINT_OPTIONS"
#define HID_SD_OPTIONS_SNAP                                     "SD_HID_SD_OPTIONS_SNAP"
#define HID_SD_OPTIONS_VIEW                                     "SD_HID_SD_OPTIONS_VIEW"
#define HID_SD_AUTOPILOT_PAGE1                                  "SD_HID_SD_AUTOPILOT_PAGE1"
#define HID_SD_AUTOPILOT_PAGE2                                  "SD_HID_SD_AUTOPILOT_PAGE2"
#define HID_SD_AUTOPILOT_PAGE3                                  "SD_HID_SD_AUTOPILOT_PAGE3"
#define HID_SD_AUTOPILOT_PAGE4                                  "SD_HID_SD_AUTOPILOT_PAGE4"
#define HID_SD_AUTOPILOT_PAGE5                                  "SD_HID_SD_AUTOPILOT_PAGE5"
#define HID_SD_AUTOPILOT_PAGE6                                  "SD_HID_SD_AUTOPILOT_PAGE6"
#define HID_SD_CBX_ONLYSELECTED                                 "SD_HID_SD_CBX_ONLYSELECTED"
#define HID_SD_BTN_FILTERDIALOG                                 "SD_HID_SD_BTN_FILTERDIALOG"
#define HID_SD_BTN_PAGE                                         "SD_HID_SD_BTN_PAGE"
#define HID_SD_BTN_MASTERPAGE                                   "SD_HID_SD_BTN_MASTERPAGE"
#define HID_SD_BTN_LAYER                                        "SD_HID_SD_BTN_LAYER"
#define HID_SD_BTN_DRAW                                         "SD_HID_SD_BTN_DRAW"
#define HID_SD_BTN_SLIDE                                        "SD_HID_SD_BTN_SLIDE"
#define HID_SD_BTN_OUTLINE                                      "SD_HID_SD_BTN_OUTLINE"
#define HID_SD_BTN_NOTES                                        "SD_HID_SD_BTN_NOTES"
#define HID_SD_BTN_HANDOUT                                      "SD_HID_SD_BTN_HANDOUT"
#define HID_SD_WIN_DOCUMENT                                     "SD_HID_SD_WIN_DOCUMENT"
#define HID_SD_OPTIONS_LAYOUT                                   "SD_HID_SD_OPTIONS_LAYOUT"
#define HID_SD_OPTIONS_CONTENTS                                 "SD_HID_SD_OPTIONS_CONTENTS"
#define HID_SD_OPTIONS_MISC                                     "SD_HID_SD_OPTIONS_MISC"
#define HID_SD_EFFECT_PREVIEW                                   "SD_HID_SD_EFFECT_PREVIEW"
#define HID_SD_CTL_NEWFOIL                                      "SD_HID_SD_CTL_NEWFOIL"
#define HID_SD_NAVIGATOR_TLB                                    "SD_HID_SD_NAVIGATOR_TLB"
#define HID_SD_NAVIGATOR                                        "SD_HID_SD_NAVIGATOR"
#define HID_SD_CTL_EFFECTS                                      "SD_HID_SD_CTL_EFFECTS"
#define HID_SD_CTL_TEXT_EFFECTS                                 "SD_HID_SD_CTL_TEXT_EFFECTS"
#define HID_SD_CTL_SLIDE_CHANGE                                 "SD_HID_SD_CTL_SLIDE_CHANGE"
#define HID_SD_NAVIGATOR_TBX                                    "SD_HID_SD_NAVIGATOR_TBX"
#define HID_SD_NAVIGATOR_LB                                     "SD_HID_SD_NAVIGATOR_LB"
#define HID_SD_NAVIGATOR_TBI_PEN                                "SD_HID_SD_NAVIGATOR_TBI_PEN"
#define HID_SD_NAVIGATOR_TBI_FIRST                              "SD_HID_SD_NAVIGATOR_TBI_FIRST"
#define HID_SD_NAVIGATOR_TBI_PREV                               "SD_HID_SD_NAVIGATOR_TBI_PREV"
#define HID_SD_NAVIGATOR_TBI_NEXT                               "SD_HID_SD_NAVIGATOR_TBI_NEXT"
#define HID_SD_NAVIGATOR_TBI_LAST                               "SD_HID_SD_NAVIGATOR_TBI_LAST"
#define HID_SD_NAVIGATOR_TBI_DRAGTYPE                           "SD_HID_SD_NAVIGATOR_TBI_DRAGTYPE"
#define HID_SD_NAVIGATOR_TBI_SHAPE_FILTER                       "SD_HID_SD_NAVIGATOR_TBI_SHAPE_FILTER"

// Toolboxen
#define HID_SD_DRAW_DEFAULTTOOLBOX                              "SD_HID_SD_DRAW_DEFAULTTOOLBOX"
#define HID_SD_PLUGINTOOLBOX                                    "SD_HID_SD_PLUGINTOOLBOX"
#define HID_SD_DRAW_TOOLBOX                                     "SD_HID_SD_DRAW_TOOLBOX"
#define HID_SD_SLIDE_TOOLBOX                                    "SD_HID_SD_SLIDE_TOOLBOX"
#define HID_SD_OUTLINE_TOOLBOX                                  "SD_HID_SD_OUTLINE_TOOLBOX"
#define HID_SD_DRAW_OBJ_TOOLBOX                                 "SD_HID_SD_DRAW_OBJ_TOOLBOX"
#define HID_SD_DRAW_OPTIONS_TOOLBOX                             "SD_HID_SD_DRAW_OPTIONS_TOOLBOX"
#define HID_SD_DRAW_COMMONTASK_TOOLBOX                          "SD_HID_SD_DRAW_COMMONTASK_TOOLBOX"
#define HID_SD_BEZIER_TOOLBOX                                   "SD_HID_SD_BEZIER_TOOLBOX"
#define HID_SD_TEXT_TOOLBOX                                     "SD_HID_SD_TEXT_TOOLBOX"
#define HID_SD_GLUEPOINTS_TOOLBOX                               "SD_HID_SD_GLUEPOINTS_TOOLBOX"
#define HID_SD_SLIDE_OBJ_TOOLBOX                                "SD_HID_SD_SLIDE_OBJ_TOOLBOX"

// FloatingWindows
#define HID_SD_FW_ALIGNMENT                                     "SD_HID_SD_FW_ALIGNMENT"
#define HID_SD_FW_ZOOM                                          "SD_HID_SD_FW_ZOOM"
#define HID_SD_FW_CHOOSE_MODE                                   "SD_HID_SD_FW_CHOOSE_MODE"
#define HID_SD_FW_TEXT                                          "SD_HID_SD_FW_TEXT"
#define HID_SD_FW_RECTANGLES                                    "SD_HID_SD_FW_RECTANGLES"
#define HID_SD_FW_ELLIPSES                                      "SD_HID_SD_FW_ELLIPSES"
#define HID_SD_FW_LINES                                         "SD_HID_SD_FW_LINES"
#define HID_SD_FW_ARROWS                                        "SD_HID_SD_FW_ARROWS"
#define HID_SD_FW_3D_OBJECTS                                    "SD_HID_SD_FW_3D_OBJECTS"
#define HID_SD_FW_CONNECTORS                                    "SD_HID_SD_FW_CONNECTORS"
#define HID_SD_FW_INSERT                                        "SD_HID_SD_FW_INSERT"
#define HID_SD_FW_POSITION                                      "SD_HID_SD_FW_POSITION"

#define HID_SD_BTN_PREVIEW                                      "SD_HID_SD_BTN_PREVIEW"

#define HID_SD_NAVIGATOR_MENU1                                  "SD_HID_SD_NAVIGATOR_MENU1"
#define HID_SD_NAVIGATOR_MENU2                                  "SD_HID_SD_NAVIGATOR_MENU2"
#define HID_SD_NAVIGATOR_MENU3                                  "SD_HID_SD_NAVIGATOR_MENU3"

#define HID_SD_WIN_PRESENTATION                                 "SD_HID_SD_WIN_PRESENTATION"

#define HID_DLG_DEFINE_CUSTOMSHOW                               "SD_HID_DLG_DEFINE_CUSTOMSHOW"
#define HID_DLG_DEFINE_CUSTOMSHOW_CTL                           "SD_HID_DLG_DEFINE_CUSTOMSHOW_CTL"

#define HID_CTL_ACTION_DLG_1                                    "SD_HID_CTL_ACTION_DLG_1"
#define HID_CTL_ACTION_DLG_2                                    "SD_HID_CTL_ACTION_DLG_2"

#define HID_SD_BTN_PRESENTATION                                 "SD_HID_SD_BTN_PRESENTATION"

#define HID_SD_HTMLEXPORT_PAGE1                                 "SD_HID_SD_HTMLEXPORT_PAGE1"
#define HID_SD_HTMLEXPORT_PAGE2                                 "SD_HID_SD_HTMLEXPORT_PAGE2"
#define HID_SD_HTMLEXPORT_PAGE3                                 "SD_HID_SD_HTMLEXPORT_PAGE3"
#define HID_SD_HTMLEXPORT_PAGE4                                 "SD_HID_SD_HTMLEXPORT_PAGE4"
#define HID_SD_HTMLEXPORT_PAGE5                                 "SD_HID_SD_HTMLEXPORT_PAGE5"
#define HID_SD_HTMLEXPORT_PAGE6                                 "SD_HID_SD_HTMLEXPORT_PAGE6"

#define HID_SD_CHGFOIL_LAYOUT                                   "SD_HID_SD_CHGFOIL_LAYOUT"
#define HID_SD_CHGFOIL_BACKGROUND                               "SD_HID_SD_CHGFOIL_BACKGROUND"
#define HID_SD_CHGFOIL_OBJECTS                                  "SD_HID_SD_CHGFOIL_OBJECTS"
#define HID_SD_CHGFOIL_NAME                                     "SD_HID_SD_CHGFOIL_NAME"

#define HID_SD_HTMLEXPORT_DLG_DNAME                             "SD_HID_SD_HTMLEXPORT_DLG_DNAME"

#define HID_FM_CTL_SELECTION                                    "SD_HID_FM_CTL_SELECTION"

#define HID_SD_AUTOPILOT_REGION                                 "SD_HID_SD_AUTOPILOT_REGION"
#define HID_SD_AUTOPILOT_TEMPLATES                              "SD_HID_SD_AUTOPILOT_TEMPLATES"
#define HID_SD_AUTOPILOT_OPEN                                   "SD_HID_SD_AUTOPILOT_OPEN"
#define HID_SD_AUTOPILOT_PAGETEMPLATES                          "SD_HID_SD_AUTOPILOT_PAGETEMPLATES"

#define HID_SD_DRAW_GRAF_TOOLBOX                                "SD_HID_SD_DRAW_GRAF_TOOLBOX"

#define HID_SD_FW_GRAFFILTERS                                   "SD_HID_SD_FW_GRAFFILTERS"

#define HID_SD_TABBAR_PAGES                                     "SD_HID_SD_TABBAR_PAGES"
#define HID_SD_TABBAR_LAYERS                                    "SD_HID_SD_TABBAR_LAYERS"

#define HID_SD_UNDODELETEWARNING_DLG                            "SD_HID_SD_UNDODELETEWARNING_DLG"
#define HID_SD_UNDODELETEWARNING_CBX                            "SD_HID_SD_UNDODELETEWARNING_CBX"

#define HID_SD_PAGEOBJSTLB                                      "SD_HID_SD_PAGEOBJSTLB"

#define HID_SD_NAMEDIALOG_OBJECT                                "SD_HID_SD_NAMEDIALOG_OBJECT"
#define HID_SD_NAMEDIALOG_PAGE                                  "SD_HID_SD_NAMEDIALOG_PAGE"
#define HID_SD_NAMEDIALOG_LINEEND                               "SD_HID_SD_NAMEDIALOG_LINEEND"

#define HID_SD_DRAW_3D_TOOLBOX                                  "SD_HID_SD_DRAW_3D_TOOLBOX"
#define HID_SD_DRAW_FONTWORK_TOOLBOX                            "SD_HID_SD_DRAW_FONTWORK_TOOLBOX"

#define HID_SD_TABPAGE_HEADERFOOTER_SLIDE                       "SD_HID_SD_TABPAGE_HEADERFOOTER_SLIDE"
#define HID_SD_TABPAGE_HEADERFOOTER_NOTESHANDOUT                "SD_HID_SD_TABPAGE_HEADERFOOTER_NOTESHANDOUT"

#define HID_SD_DRAW_MEDIA_TOOLBOX                               "SD_HID_SD_DRAW_MEDIA_TOOLBOX"

#define HID_SD_TASK_PANE                                        "SD_HID_SD_TASK_PANE"
#define HID_SD_SLIDE_DESIGNS                                    "SD_HID_SD_SLIDE_DESIGNS"
#define HID_SD_CURRENT_MASTERS                                  "SD_HID_SD_CURRENT_MASTERS"
#define HID_SD_RECENT_MASTERS                                   "SD_HID_SD_RECENT_MASTERS"
#define HID_SD_ALL_MASTERS                                      "SD_HID_SD_ALL_MASTERS"
#define HID_SD_SLIDE_LAYOUTS                                    "SD_HID_SD_SLIDE_LAYOUTS"
#define HID_SD_CUSTOM_ANIMATIONS                                "SD_HID_SD_CUSTOM_ANIMATIONS"
#define HID_SD_SLIDE_TRANSITIONS                                "SD_HID_SD_SLIDE_TRANSITIONS"

#define HID_SD_CUSTOMANIMATIONPANE_PB_ADD_EFFECT                "SD_HID_SD_CUSTOMANIMATIONPANE_PB_ADD_EFFECT"
#define HID_SD_CUSTOMANIMATIONPANE_PB_CHANGE_EFFECT             "SD_HID_SD_CUSTOMANIMATIONPANE_PB_CHANGE_EFFECT"
#define HID_SD_CUSTOMANIMATIONPANE_PB_REMOVE_EFFECT             "SD_HID_SD_CUSTOMANIMATIONPANE_PB_REMOVE_EFFECT"
#define HID_SD_CUSTOMANIMATIONPANE_LB_START                     "SD_HID_SD_CUSTOMANIMATIONPANE_LB_START"
#define HID_SD_CUSTOMANIMATIONPANE_LB_PROPERTY                  "SD_HID_SD_CUSTOMANIMATIONPANE_LB_PROPERTY"
#define HID_SD_CUSTOMANIMATIONPANE_PB_PROPERTY_MORE             "SD_HID_SD_CUSTOMANIMATIONPANE_PB_PROPERTY_MORE"
#define HID_SD_CUSTOMANIMATIONPANE_CB_SPEED                     "SD_HID_SD_CUSTOMANIMATIONPANE_CB_SPEED"
#define HID_SD_CUSTOMANIMATIONPANE_CT_CUSTOM_ANIMATION_LIST     "SD_HID_SD_CUSTOMANIMATIONPANE_CT_CUSTOM_ANIMATION_LIST"
#define HID_SD_CUSTOMANIMATIONPANE_PB_MOVE_UP                   "SD_HID_SD_CUSTOMANIMATIONPANE_PB_MOVE_UP"
#define HID_SD_CUSTOMANIMATIONPANE_PB_MOVE_DOWN                 "SD_HID_SD_CUSTOMANIMATIONPANE_PB_MOVE_DOWN"
#define HID_SD_CUSTOMANIMATIONPANE_PB_PLAY                      "SD_HID_SD_CUSTOMANIMATIONPANE_PB_PLAY"
#define HID_SD_CUSTOMANIMATIONPANE_PB_SLIDE_SHOW                "SD_HID_SD_CUSTOMANIMATIONPANE_PB_SLIDE_SHOW"
#define HID_SD_CUSTOMANIMATIONPANE_CB_AUTOPREVIEW               "SD_HID_SD_CUSTOMANIMATIONPANE_CB_AUTOPREVIEW"

#define HID_SD_ANIMATIONSCHEMESPANE_LB_ANIMATION_SCHEMES        "SD_HID_SD_ANIMATIONSCHEMESPANE_LB_ANIMATION_SCHEMES"
#define HID_SD_ANIMATIONSCHEMESPANE_PB_APPLY_TO_ALL             "SD_HID_SD_ANIMATIONSCHEMESPANE_PB_APPLY_TO_ALL"
#define HID_SD_ANIMATIONSCHEMESPANE_PB_PLAY                     "SD_HID_SD_ANIMATIONSCHEMESPANE_PB_PLAY"
#define HID_SD_ANIMATIONSCHEMESPANE_PB_SLIDE_SHOW               "SD_HID_SD_ANIMATIONSCHEMESPANE_PB_SLIDE_SHOW"
#define HID_SD_ANIMATIONSCHEMESPANE_CB_AUTO_PREVIEW             "SD_HID_SD_ANIMATIONSCHEMESPANE_CB_AUTO_PREVIEW"

#define HID_SD_SLIDETRANSITIONPANE_LB_SLIDE_TRANSITIONS         "SD_HID_SD_SLIDETRANSITIONPANE_LB_SLIDE_TRANSITIONS"
#define HID_SD_SLIDETRANSITIONPANE_LB_SPEED                     "SD_HID_SD_SLIDETRANSITIONPANE_LB_SPEED"
#define HID_SD_SLIDETRANSITIONPANE_LB_SOUND                     "SD_HID_SD_SLIDETRANSITIONPANE_LB_SOUND"
#define HID_SD_SLIDETRANSITIONPANE_CB_LOOP_SOUND                "SD_HID_SD_SLIDETRANSITIONPANE_CB_LOOP_SOUND"
#define HID_SD_SLIDETRANSITIONPANE_RB_ADVANCE_ON_MOUSE          "SD_HID_SD_SLIDETRANSITIONPANE_RB_ADVANCE_ON_MOUSE"
#define HID_SD_SLIDETRANSITIONPANE_RB_ADVANCE_AUTO              "SD_HID_SD_SLIDETRANSITIONPANE_RB_ADVANCE_AUTO"
#define HID_SD_SLIDETRANSITIONPANE_MF_ADVANCE_AUTO_AFTER        "SD_HID_SD_SLIDETRANSITIONPANE_MF_ADVANCE_AUTO_AFTER"
#define HID_SD_SLIDETRANSITIONPANE_PB_APPLY_TO_ALL              "SD_HID_SD_SLIDETRANSITIONPANE_PB_APPLY_TO_ALL"
#define HID_SD_SLIDETRANSITIONPANE_PB_PLAY                      "SD_HID_SD_SLIDETRANSITIONPANE_PB_PLAY"
#define HID_SD_SLIDETRANSITIONPANE_PB_SLIDE_SHOW                "SD_HID_SD_SLIDETRANSITIONPANE_PB_SLIDE_SHOW"
#define HID_SD_SLIDETRANSITIONPANE_CB_AUTO_PREVIEW              "SD_HID_SD_SLIDETRANSITIONPANE_CB_AUTO_PREVIEW"

#define HID_SD_TASK_PANE_VIEW_MENU                              "SD_HID_SD_TASK_PANE_VIEW_MENU"
#define HID_SD_TASK_PANE_PREVIEW_CURRENT                        "SD_HID_SD_TASK_PANE_PREVIEW_CURRENT"
#define HID_SD_TASK_PANE_PREVIEW_RECENT                         "SD_HID_SD_TASK_PANE_PREVIEW_RECENT"
#define HID_SD_TASK_PANE_PREVIEW_ALL                            "SD_HID_SD_TASK_PANE_PREVIEW_ALL"
#define HID_SD_TASK_PANE_PREVIEW_LAYOUTS                        "SD_HID_SD_TASK_PANE_PREVIEW_LAYOUTS"

#define HID_SD_CUSTOMANIMATIONDIALOG_ENTRANCE                   "SD_HID_SD_CUSTOMANIMATIONDIALOG_ENTRANCE"
#define HID_SD_CUSTOMANIMATIONDIALOG_EMPHASIS                   "SD_HID_SD_CUSTOMANIMATIONDIALOG_EMPHASIS"
#define HID_SD_CUSTOMANIMATIONDIALOG_EXIT                       "SD_HID_SD_CUSTOMANIMATIONDIALOG_EXIT"
#define HID_SD_CUSTOMANIMATIONDIALOG_MOTIONPATH                 "SD_HID_SD_CUSTOMANIMATIONDIALOG_MOTIONPATH"
#define HID_SD_CUSTOMANIMATIONDIALOG_MISCEFFECTS                "SD_HID_SD_CUSTOMANIMATIONDIALOG_MISCEFFECTS"

#define HID_SD_CUSTOMANIMATIONPANE_PRESETPROPERTYBOX            "SD_HID_SD_CUSTOMANIMATIONPANE_PRESETPROPERTYBOX"
#define HID_SD_CUSTOMANIMATIONPANE_COLORPROPERTYBOX             "SD_HID_SD_CUSTOMANIMATIONPANE_COLORPROPERTYBOX"
#define HID_SD_CUSTOMANIMATIONPANE_FONTPROPERTYBOX              "SD_HID_SD_CUSTOMANIMATIONPANE_FONTPROPERTYBOX"
#define HID_SD_CUSTOMANIMATIONPANE_CHARHEIGHTPROPERTYBOX        "SD_HID_SD_CUSTOMANIMATIONPANE_CHARHEIGHTPROPERTYBOX"
#define HID_SD_CUSTOMANIMATIONPANE_ROTATIONPROPERTYBOX          "SD_HID_SD_CUSTOMANIMATIONPANE_ROTATIONPROPERTYBOX"
#define HID_SD_CUSTOMANIMATIONPANE_TRANSPARENCYPROPERTYBOX      "SD_HID_SD_CUSTOMANIMATIONPANE_TRANSPARENCYPROPERTYBOX"
#define HID_SD_CUSTOMANIMATIONPANE_SCALEPROPERTYBOX             "SD_HID_SD_CUSTOMANIMATIONPANE_SCALEPROPERTYBOX"
#define HID_SD_CUSTOMANIMATIONPANE_FONTSTYLEPROPERTYBOX         "SD_HID_SD_CUSTOMANIMATIONPANE_FONTSTYLEPROPERTYBOX"

#define HID_SD_SLIDESHOW_DISPLAY                                "SD_HID_SD_SLIDESHOW_DISPLAY"

#define HID_SD_RULER_HORIZONTAL                                 "SD_HID_SD_RULER_HORIZONTAL"
#define HID_SD_RULER_VERTICAL                                   "SD_HID_SD_RULER_VERTICAL"

#define HID_SD_TABLE_DESIGN                                     "SD_HID_SD_TABLE_DESIGN"
#define HID_SD_ANNOTATIONS                                      "SD_HID_SD_ANNOTATIONS"

#define HID_DLG_INSERT_PAGES_OBJS                               "SD_HID_DLG_INSERT_PAGES_OBJS"
#define HID_TABPAGE_PARA_NUMBERING                              "SD_HID_TABPAGE_PARA_NUMBERING"
#define HID_VECTORIZE_DLG                                       "SD_HID_VECTORIZE_DLG"
#define HID_UNDO_DELETE_WARNING                                 "SD_HID_UNDO_DELETE_WARNING"

#define HID_MN_SUB_GROUP                                        "SD_HID_MN_SUB_GROUP"
#define HID_MN_SUB_GRAPHIC                                      "SD_HID_MN_SUB_GRAPHIC"
#define HID_MN_SUB_SEND                                         "SD_HID_MN_SUB_SEND"
#define HID_MN_SUB_MASTER_LAYOUTS                               "SD_HID_MN_SUB_MASTER_LAYOUTS"
#define HID_MN_SUB_VIEW                                         "SD_HID_MN_SUB_VIEW"
#define HID_MN_SUB_MASTERPAGE                                   "SD_HID_MN_SUB_MASTERPAGE"
#define HID_MN_SUB_FIELD                                        "SD_HID_MN_SUB_FIELD"
#define HID_MN_SUB_TEMPLATES                                    "SD_HID_MN_SUB_TEMPLATES"
#define HID_MN_SUB_SPELLING                                     "SD_HID_MN_SUB_SPELLING"
#define HID_MN_SUB_POSITION                                     "SD_HID_MN_SUB_POSITION"
#define HID_MN_SUB_OUTPUT_QUALITY                               "SD_HID_MN_SUB_OUTPUT_QUALITY"
#define HID_MN_SUB_PREVIEW_OUTPUT_QUALITY                       "SD_HID_MN_SUB_PREVIEW_OUTPUT_QUALITY"
#define HID_MN_SUB_CONVERT                                      "SD_HID_MN_SUB_CONVERT"
#define HID_MN_SUB_MIRROR                                       "SD_HID_MN_SUB_MIRROR"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
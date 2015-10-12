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
#include "swcommands.h"
#include <svx/svxcommands.h>
#include <sfx2/sfxcommands.h>

#define HID_CONFIG_SAVE                                         "SW_HID_CONFIG_SAVE"



#define HID_EDIT_WIN                                            "SW_HID_EDIT_WIN"


#define HID_SOURCEVIEW                                          "SW_HID_SOURCEVIEW"

#define HID_NAVI_DRAG_HYP                                       "SW_HID_NAVI_DRAG_HYP"
#define HID_NAVI_DRAG_LINK                                      "SW_HID_NAVI_DRAG_LINK"
#define HID_NAVI_DRAG_COPY                                      "SW_HID_NAVI_DRAG_COPY"
#define HID_NAVI_OUTLINES                                       "SW_HID_NAVI_OUTLINES"


#define HID_PAGEPREVIEW                                         "SW_HID_PAGEPREVIEW"
#define HID_SOURCE_EDITWIN                                      "SW_HID_SOURCE_EDITWIN"

// Dialog Help-IDs

#define HID_NAVIGATOR_TREELIST                                  "SW_HID_NAVIGATOR_TREELIST"
#define HID_NAVIGATOR_TOOLBOX                                   "SW_HID_NAVIGATOR_TOOLBOX"
#define HID_NAVIGATOR_LISTBOX                                   "SW_HID_NAVIGATOR_LISTBOX"
#define HID_NAVI_TBX2                                           "SW_HID_NAVI_TBX2"
#define HID_NAVI_TBX3                                           "SW_HID_NAVI_TBX3"
#define HID_NAVI_TBX4                                           "SW_HID_NAVI_TBX4"
#define HID_NAVI_TBX5                                           "SW_HID_NAVI_TBX5"
#define HID_NAVI_TBX6                                           "SW_HID_NAVI_TBX6"
#define HID_NAVI_TBX7                                           "SW_HID_NAVI_TBX7"
#define HID_NAVI_TBX8                                           "SW_HID_NAVI_TBX8"
#define HID_NAVI_TBX9                                           "SW_HID_NAVI_TBX9"
#define HID_NAVI_TBX10                                          "SW_HID_NAVI_TBX10"
#define HID_NAVI_TBX11                                          "SW_HID_NAVI_TBX11"
#define HID_NAVI_TBX12                                          "SW_HID_NAVI_TBX12"
#define HID_NAVI_TBX13                                          "SW_HID_NAVI_TBX13"
#define HID_NAVI_TBX14                                          "SW_HID_NAVI_TBX14"
#define HID_NAVI_TBX15                                          "SW_HID_NAVI_TBX15"
#define HID_NAVI_VS                                             "SW_HID_NAVI_VS"
#define HID_NAVI_TBX16                                          "SW_HID_NAVI_TBX16"
#define HID_NAVI_TBX17                                          "SW_HID_NAVI_TBX17"
#define HID_NAVI_TBX18                                          "SW_HID_NAVI_TBX18"
#define HID_NAVI_TBX19                                          "SW_HID_NAVI_TBX19"
#define HID_NAVI_TBX20                                          "SW_HID_NAVI_TBX20"
#define HID_NAVI_TBX21                                          "SW_HID_NAVI_TBX21"
#define HID_NAVI_TBX22                                          "SW_HID_NAVI_TBX22"
#define HID_NAVI_TBX23                                          "SW_HID_NAVI_TBX23"
#define HID_NAVI_TBX24                                          "SW_HID_NAVI_TBX24"
#define HID_NAVIGATOR_GLOBAL_TOOLBOX                            "SW_HID_NAVIGATOR_GLOBAL_TOOLBOX"
#define HID_NAVIGATOR_GLOB_TREELIST                             "SW_HID_NAVIGATOR_GLOB_TREELIST"
#define HID_GLBLTREE_UPDATE                                     "SW_HID_GLBLTREE_UPDATE"
#define HID_GLBLTREE_INSERT                                     "SW_HID_GLBLTREE_INSERT"
#define HID_GLBLTREE_EDIT                                       "SW_HID_GLBLTREE_EDIT"
#define HID_GLBLTREE_DEL                                        "SW_HID_GLBLTREE_DEL"
#define HID_GLBLTREE_INS_IDX                                    "SW_HID_GLBLTREE_INS_IDX"
#define HID_GLBLTREE_INS_FILE                                   "SW_HID_GLBLTREE_INS_FILE"
#define HID_GLBLTREE_INS_NEW_FILE                               "SW_HID_GLBLTREE_INS_NEW_FILE"
#define HID_GLBLTREE_INS_TEXT                                   "SW_HID_GLBLTREE_INS_TEXT"
#define HID_GLBLTREE_UPD_SEL                                    "SW_HID_GLBLTREE_UPD_SEL"
#define HID_GLBLTREE_UPD_IDX                                    "SW_HID_GLBLTREE_UPD_IDX"
#define HID_GLBLTREE_UPD_LINK                                   "SW_HID_GLBLTREE_UPD_LINK"
#define HID_GLBLTREEUPD_ALL                                     "SW_HID_GLBLTREEUPD_ALL"
#define HID_NAVI_CONTENT                                        "SW_HID_NAVI_CONTENT"
#define HID_NAVI_GLOBAL                                         "SW_HID_NAVI_GLOBAL"
#define HID_SORT_ACTION                                         "SW_HID_SORT_ACTION"
#define HID_SORT_AUTHOR                                         "SW_HID_SORT_AUTHOR"
#define HID_SORT_DATE                                           "SW_HID_SORT_DATE"
#define HID_SORT_COMMENT                                        "SW_HID_SORT_COMMENT"
#define HID_SW_SORT_POSITION                                    "SW_HID_SW_SORT_POSITION"
#define HID_EDIT_COMMENT                                        "SW_HID_EDIT_COMMENT"


#define HID_GLBLTREE_EDIT_LINK                                  "SW_HID_GLBLTREE_EDIT_LINK"


// TabPage Help-IDs

#define HID_REDLINE_CTRL                                        "SW_HID_REDLINE_CTRL"



// sw::sidebar::PagePropertyPanel
#define HID_SWPAGE_LEFT_MARGIN              "HID_SWPAGE_LEFT_MARGIN"
#define HID_SWPAGE_RIGHT_MARGIN             "HID_SWPAGE_RIGHT_MARGIN"
#define HID_SWPAGE_TOP_MARGIN               "HID_SWPAGE_TOP_MARGIN"
#define HID_SWPAGE_BOTTOM_MARGIN            "HID_SWPAGE_BOTTOM_MARGIN"
#define HID_SWPAGE_SIZE_MORE                "HID_SWPAGE_SIZE_MORE"
#define HID_SWPAGE_COLUMN_MORE              "HID_SWPAGE_COLUMN_MORE"
#define HID_SWPAGE_VS_ORIENTATION           "HID_SWPAGE_VS_ORIENTATION"
#define HID_SWPAGE_VS_MARGIN                "HID_SWPAGE_VS_MARGIN"
#define HID_SWPAGE_VS_SIZE                  "HID_SWPAGE_VS_SIZE"
#define HID_SWPAGE_VS_COLUMN                "HID_SWPAGE_VS_COLUMN"

// HelpIds for Menu

#define HID_MN_SUB_TBLROW                                       "SW_HID_MN_SUB_TBLROW"
#define HID_MN_SUB_TBLCOL                                       "SW_HID_MN_SUB_TBLCOL"
#define HID_MN_SUB_FIELD                                        "SW_HID_MN_SUB_FIELD"
#define HID_MN_SUB_GRAPHIC                                      "SW_HID_MN_SUB_GRAPHIC"
#define HID_MN_SUB_TEMPLATES                                    "SW_HID_MN_SUB_TEMPLATES"
#define HID_MN_SUB_ARRANGE                                      "SW_HID_MN_SUB_ARRANGE"
#define HID_MN_SUB_SPELLING                                     "SW_HID_MN_SUB_SPELLING"
#define HID_MN_SUB_MIRROR                                       "SW_HID_MN_SUB_MIRROR"
#define HID_MN_SUB_ALIGN                                        "SW_HID_MN_SUB_ALIGN"

#define HID_MN_CALC_PHD                                         "SW_HID_MN_CALC_PHD"
#define HID_MN_CALC_SQRT                                        "SW_HID_MN_CALC_SQRT"
#define HID_MN_CALC_OR                                          "SW_HID_MN_CALC_OR"
#define HID_MN_CALC_XOR                                         "SW_HID_MN_CALC_XOR"
#define HID_MN_CALC_AND                                         "SW_HID_MN_CALC_AND"
#define HID_MN_CALC_NOT                                         "SW_HID_MN_CALC_NOT"
#define HID_MN_CALC_EQ                                          "SW_HID_MN_CALC_EQ"
#define HID_MN_CALC_NEQ                                         "SW_HID_MN_CALC_NEQ"
#define HID_MN_CALC_LEQ                                         "SW_HID_MN_CALC_LEQ"
#define HID_MN_CALC_GEQ                                         "SW_HID_MN_CALC_GEQ"
#define HID_MN_CALC_LES                                         "SW_HID_MN_CALC_LES"
#define HID_MN_CALC_GRE                                         "SW_HID_MN_CALC_GRE"
#define HID_MN_CALC_SUM                                         "SW_HID_MN_CALC_SUM"
#define HID_MN_CALC_MEAN                                        "SW_HID_MN_CALC_MEAN"
#define HID_MN_CALC_MIN                                         "SW_HID_MN_CALC_MIN"
#define HID_MN_CALC_MAX                                         "SW_HID_MN_CALC_MAX"
#define HID_MN_CALC_SIN                                         "SW_HID_MN_CALC_SIN"
#define HID_MN_CALC_COS                                         "SW_HID_MN_CALC_COS"
#define HID_MN_CALC_TAN                                         "SW_HID_MN_CALC_TAN"
#define HID_MN_CALC_ASIN                                        "SW_HID_MN_CALC_ASIN"
#define HID_MN_CALC_ACOS                                        "SW_HID_MN_CALC_ACOS"
#define HID_MN_CALC_ATAN                                        "SW_HID_MN_CALC_ATAN"
#define HID_MN_CALC_POW                                         "SW_HID_MN_CALC_POW"
#define HID_MN_CALC_LISTSEP                                     "SW_HID_MN_CALC_LISTSEP"
#define HID_MN_POP_OPS                                          "SW_HID_MN_POP_OPS"
#define HID_MN_POP_STATISTICS                                   "SW_HID_MN_POP_STATISTICS"
#define HID_MN_POP_FUNC                                         "SW_HID_MN_POP_FUNC"
#define HID_MN_CALC_ROUND                                       "SW_HID_MN_CALC_ROUND"

#define HID_MN_READONLY_SAVEGRAPHIC                             "SW_HID_MN_READONLY_SAVEGRAPHIC"
#define HID_MN_READONLY_GRAPHICTOGALLERY                        "SW_HID_MN_READONLY_GRAPHICTOGALLERY"
#define HID_MN_READONLY_TOGALLERYLINK                           "SW_HID_MN_READONLY_TOGALLERYLINK"
#define HID_MN_READONLY_TOGALLERYCOPY                           "SW_HID_MN_READONLY_TOGALLERYCOPY"
#define HID_MN_READONLY_SAVEBACKGROUND                          "SW_HID_MN_READONLY_SAVEBACKGROUND"
#define HID_MN_READONLY_BACKGROUNDTOGALLERY                     "SW_HID_MN_READONLY_BACKGROUNDTOGALLERY"
#define HID_MN_READONLY_COPYLINK                                "SW_HID_MN_READONLY_COPYLINK"
#define HID_MN_READONLY_COPYGRAPHIC                             "SW_HID_MN_READONLY_COPYGRAPHIC"
#define HID_MN_READONLY_LOADGRAPHIC                             "SW_HID_MN_READONLY_LOADGRAPHIC"
#define HID_MN_READONLY_GRAPHICOFF                              "SW_HID_MN_READONLY_GRAPHICOFF"
#define HID_MN_READONLY_PLUGINOFF                               "SW_HID_MN_READONLY_PLUGINOFF"

#define HID_LINGU_ADD_WORD                                      "SW_HID_LINGU_ADD_WORD"
#define HID_LINGU_IGNORE_WORD                                   "SW_HID_LINGU_IGNORE_WORD"
#define HID_LINGU_SPELLING_DLG                                  "SW_HID_LINGU_SPELLING_DLG"
#define HID_LINGU_AUTOCORR                                      "SW_HID_LINGU_AUTOCORR"
#define HID_LINGU_REPLACE                                       "SW_HID_LINGU_REPLACE"
#define HID_SMARTTAG_MAIN                                       "SW_HID_SMARTTAG_MAIN"    // SMARTTAGS
#define HID_LINGU_IGNORE_SELECTION                              "SW_HID_LINGU_IGNORE_SELECTION"    // grammar check context menu

// More Help-IDs
#define HID_EDIT_FORMULA                                        "SW_HID_EDIT_FORMULA"
#define HID_NAVIGATION_PI                                       "SW_HID_NAVIGATION_PI"


#define HID_CALC_TOOLBOX                                        "SW_HID_CALC_TOOLBOX"



#define HID_AUTOFORMAT_REJECT                                   "SW_HID_AUTOFORMAT_REJECT"
#define HID_AUTOFORMAT_ACCEPT                                   "SW_HID_AUTOFORMAT_ACCEPT"
#define HID_AUTOFORMAT_EDIT_CHG                                 "SW_HID_AUTOFORMAT_EDIT_CHG"

#define HID_AUTH_FIELD_IDENTIFIER                               "SW_HID_AUTH_FIELD_IDENTIFIER"
#define HID_AUTH_FIELD_AUTHORITY_TYPE                           "SW_HID_AUTH_FIELD_AUTHORITY_TYPE"
#define HID_AUTH_FIELD_ADDRESS                                  "SW_HID_AUTH_FIELD_ADDRESS"
#define HID_AUTH_FIELD_ANNOTE                                   "SW_HID_AUTH_FIELD_ANNOTE"
#define HID_AUTH_FIELD_AUTHOR                                   "SW_HID_AUTH_FIELD_AUTHOR"
#define HID_AUTH_FIELD_BOOKTITLE                                "SW_HID_AUTH_FIELD_BOOKTITLE"
#define HID_AUTH_FIELD_CHAPTER                                  "SW_HID_AUTH_FIELD_CHAPTER"
#define HID_AUTH_FIELD_EDITION                                  "SW_HID_AUTH_FIELD_EDITION"
#define HID_AUTH_FIELD_EDITOR                                   "SW_HID_AUTH_FIELD_EDITOR"
#define HID_AUTH_FIELD_HOWPUBLISHED                             "SW_HID_AUTH_FIELD_HOWPUBLISHED"
#define HID_AUTH_FIELD_INSTITUTION                              "SW_HID_AUTH_FIELD_INSTITUTION"
#define HID_AUTH_FIELD_JOURNAL                                  "SW_HID_AUTH_FIELD_JOURNAL"
#define HID_AUTH_FIELD_MONTH                                    "SW_HID_AUTH_FIELD_MONTH"
#define HID_AUTH_FIELD_NOTE                                     "SW_HID_AUTH_FIELD_NOTE"
#define HID_AUTH_FIELD_NUMBER                                   "SW_HID_AUTH_FIELD_NUMBER"
#define HID_AUTH_FIELD_ORGANIZATIONS                            "SW_HID_AUTH_FIELD_ORGANIZATIONS"
#define HID_AUTH_FIELD_PAGES                                    "SW_HID_AUTH_FIELD_PAGES"
#define HID_AUTH_FIELD_PUBLISHER                                "SW_HID_AUTH_FIELD_PUBLISHER"
#define HID_AUTH_FIELD_SCHOOL                                   "SW_HID_AUTH_FIELD_SCHOOL"
#define HID_AUTH_FIELD_SERIES                                   "SW_HID_AUTH_FIELD_SERIES"
#define HID_AUTH_FIELD_TITLE                                    "SW_HID_AUTH_FIELD_TITLE"
#define HID_AUTH_FIELD_REPORT_TYPE                              "SW_HID_AUTH_FIELD_REPORT_TYPE"
#define HID_AUTH_FIELD_VOLUME                                   "SW_HID_AUTH_FIELD_VOLUME"
#define HID_AUTH_FIELD_YEAR                                     "SW_HID_AUTH_FIELD_YEAR"
#define HID_AUTH_FIELD_URL                                      "SW_HID_AUTH_FIELD_URL"
#define HID_AUTH_FIELD_CUSTOM1                                  "SW_HID_AUTH_FIELD_CUSTOM1"
#define HID_AUTH_FIELD_CUSTOM2                                  "SW_HID_AUTH_FIELD_CUSTOM2"
#define HID_AUTH_FIELD_CUSTOM3                                  "SW_HID_AUTH_FIELD_CUSTOM3"
#define HID_AUTH_FIELD_CUSTOM4                                  "SW_HID_AUTH_FIELD_CUSTOM4"
#define HID_AUTH_FIELD_CUSTOM5                                  "SW_HID_AUTH_FIELD_CUSTOM5"
#define HID_AUTH_FIELD_ISBN                                     "SW_HID_AUTH_FIELD_ISBN"

#define HID_BUSINESS_FMT_PAGE                                   "SW_HID_BUSINESS_FMT_PAGE"
#define HID_BUSINESS_FMT_PAGE_CONT                              "SW_HID_BUSINESS_FMT_PAGE_CONT"
#define HID_BUSINESS_FMT_PAGE_SHEET                             "SW_HID_BUSINESS_FMT_PAGE_SHEET"
#define HID_BUSINESS_FMT_PAGE_BRAND                             "SW_HID_BUSINESS_FMT_PAGE_BRAND"
#define HID_BUSINESS_FMT_PAGE_TYPE                              "SW_HID_BUSINESS_FMT_PAGE_TYPE"
#define HID_SEND_MASTER_CTRL_PUSHBUTTON_OK                      "SW_HID_SEND_MASTER_CTRL_PUSHBUTTON_OK"
#define HID_SEND_MASTER_CTRL_PUSHBUTTON_CANCEL                  "SW_HID_SEND_MASTER_CTRL_PUSHBUTTON_CANCEL"
#define HID_SEND_MASTER_CTRL_LISTBOX_FILTER                     "SW_HID_SEND_MASTER_CTRL_LISTBOX_FILTER"
#define HID_SEND_MASTER_CTRL_CONTROL_FILEVIEW                   "SW_HID_SEND_MASTER_CTRL_CONTROL_FILEVIEW"
#define HID_SEND_MASTER_CTRL_EDIT_FILEURL                       "SW_HID_SEND_MASTER_CTRL_EDIT_FILEURL"
#define HID_SEND_MASTER_CTRL_CHECKBOX_AUTOEXTENSION             "SW_HID_SEND_MASTER_CTRL_CHECKBOX_AUTOEXTENSION"
#define HID_SEND_MASTER_CTRL_LISTBOX_TEMPLATE                   "SW_HID_SEND_MASTER_CTRL_LISTBOX_TEMPLATE"

#define HID_SEND_HTML_CTRL_PUSHBUTTON_OK                        "SW_HID_SEND_HTML_CTRL_PUSHBUTTON_OK"
#define HID_SEND_HTML_CTRL_PUSHBUTTON_CANCEL                    "SW_HID_SEND_HTML_CTRL_PUSHBUTTON_CANCEL"
#define HID_SEND_HTML_CTRL_LISTBOX_FILTER                       "SW_HID_SEND_HTML_CTRL_LISTBOX_FILTER"
#define HID_SEND_HTML_CTRL_CONTROL_FILEVIEW                     "SW_HID_SEND_HTML_CTRL_CONTROL_FILEVIEW"
#define HID_SEND_HTML_CTRL_EDIT_FILEURL                         "SW_HID_SEND_HTML_CTRL_EDIT_FILEURL"
#define HID_SEND_HTML_CTRL_CHECKBOX_AUTOEXTENSION               "SW_HID_SEND_HTML_CTRL_CHECKBOX_AUTOEXTENSION"
#define HID_SEND_HTML_CTRL_LISTBOX_TEMPLATE                     "SW_HID_SEND_HTML_CTRL_LISTBOX_TEMPLATE"

#define HID_PVIEW_ZOOM_LB                                       "SW_HID_PVIEW_ZOOM_LB"
#define HID_JUMP_TO_SPEC_PAGE                                   "SW_HID_JUMP_TO_SPEC_PAGE"


#define HID_NID_TBL                                             "SW_HID_NID_TBL"
#define HID_NID_FRM                                             "SW_HID_NID_FRM"
#define HID_NID_GRF                                             "SW_HID_NID_GRF"
#define HID_NID_OLE                                             "SW_HID_NID_OLE"
#define HID_NID_PGE                                             "SW_HID_NID_PGE"
#define HID_NID_OUTL                                            "SW_HID_NID_OUTL"
#define HID_NID_MARK                                            "SW_HID_NID_MARK"
#define HID_NID_DRW                                             "SW_HID_NID_DRW"
#define HID_NID_CTRL                                            "SW_HID_NID_CTRL"
#define HID_NID_PREV                                            "SW_HID_NID_PREV"
#define HID_NID_REG                                             "SW_HID_NID_REG"
#define HID_NID_BKM                                             "SW_HID_NID_BKM"
#define HID_NID_SEL                                             "SW_HID_NID_SEL"
#define HID_NID_FTN                                             "SW_HID_NID_FTN"
#define HID_NID_POSTIT                                          "SW_HID_NID_POSTIT"
#define HID_NID_SRCH_REP                                        "SW_HID_NID_SRCH_REP"
#define HID_NID_INDEX_ENTRY                                     "SW_HID_NID_INDEX_ENTRY"
#define HID_NID_TABLE_FORMULA                                   "SW_HID_NID_TABLE_FORMULA"
#define HID_NID_TABLE_FORMULA_ERROR                             "SW_HID_NID_TABLE_FORMULA_ERROR"
#define HID_NID_NEXT                                            "SW_HID_NID_NEXT"
#define HID_MM_NEXT_PAGE                                        "SW_HID_MM_NEXT_PAGE"
#define HID_MM_PREV_PAGE                                        "SW_HID_MM_PREV_PAGE"
#define HID_MM_BODY_CB_PERSONALIZED                             "SW_HID_MM_BODY_CB_PERSONALIZED"
#define HID_MM_BODY_LB_FEMALE                                   "SW_HID_MM_BODY_LB_FEMALE"
#define HID_MM_BODY_PB_FEMALE                                   "SW_HID_MM_BODY_PB_FEMALE"
#define HID_MM_BODY_LB_MALE                                     "SW_HID_MM_BODY_LB_MALE"
#define HID_MM_BODY_PB_MALE                                     "SW_HID_MM_BODY_PB_MALE"
#define HID_MM_BODY_LB_FEMALECOLUMN                             "SW_HID_MM_BODY_LB_FEMALECOLUMN"
#define HID_MM_BODY_CB_FEMALEFIELD                              "SW_HID_MM_BODY_CB_FEMALEFIELD"
#define HID_MM_BODY_CB_NEUTRAL                                  "SW_HID_MM_BODY_CB_NEUTRAL"

#define HID_TBX_FORMULA_CALC                                    "SW_HID_TBX_FORMULA_CALC"
#define HID_TBX_FORMULA_CANCEL                                  "SW_HID_TBX_FORMULA_CANCEL"
#define HID_TBX_FORMULA_APPLY                                   "SW_HID_TBX_FORMULA_APPLY"


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

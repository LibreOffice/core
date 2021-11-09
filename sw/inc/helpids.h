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

#ifndef INCLUDED_SW_INC_HELPIDS_H
#define INCLUDED_SW_INC_HELPIDS_H

#include <rtl/string.hxx>

inline constexpr OStringLiteral HID_EDIT_WIN = "SW_HID_EDIT_WIN";

#define HID_NAVI_DRAG_HYP                                       "SW_HID_NAVI_DRAG_HYP"
#define HID_NAVI_DRAG_LINK                                      "SW_HID_NAVI_DRAG_LINK"
#define HID_NAVI_DRAG_COPY                                      "SW_HID_NAVI_DRAG_COPY"
#define HID_NAVI_OUTLINES                                       "SW_HID_NAVI_OUTLINES"


inline constexpr OStringLiteral HID_PAGEPREVIEW = "SW_HID_PAGEPREVIEW";
inline constexpr OStringLiteral HID_SOURCE_EDITWIN = "SW_HID_SOURCE_EDITWIN";

// Dialog Help-IDs

inline constexpr OStringLiteral HID_NAVIGATOR_TREELIST = "SW_HID_NAVIGATOR_TREELIST";
inline constexpr OStringLiteral HID_NAVIGATOR_TOOLBOX = "SW_HID_NAVIGATOR_TOOLBOX";
inline constexpr OStringLiteral HID_NAVIGATOR_LISTBOX = "SW_HID_NAVIGATOR_LISTBOX";
#define HID_NAVI_VS                                             "SW_HID_NAVI_VS"
inline constexpr OStringLiteral HID_NAVIGATOR_GLOBAL_TOOLBOX = "SW_HID_NAVIGATOR_GLOBAL_TOOLBOX";
inline constexpr OStringLiteral HID_NAVIGATOR_GLOB_TREELIST = "SW_HID_NAVIGATOR_GLOB_TREELIST";
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

#define HID_GLBLTREE_EDIT_LINK                                  "SW_HID_GLBLTREE_EDIT_LINK"


// TabPage Help-IDs

inline constexpr OStringLiteral HID_REDLINE_CTRL = "SW_HID_REDLINE_CTRL";

inline constexpr OStringLiteral HID_LINGU_AUTOCORR = "SW_HID_LINGU_AUTOCORR";
inline constexpr OStringLiteral HID_LINGU_REPLACE = "SW_HID_LINGU_REPLACE";
inline constexpr OStringLiteral HID_LINGU_IGNORE_SELECTION = "SW_HID_LINGU_IGNORE_SELECTION";    // grammar check context menu

// More Help-IDs
inline constexpr OStringLiteral HID_EDIT_FORMULA = "SW_HID_EDIT_FORMULA";

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
#define HID_AUTH_FIELD_LOCAL_URL                                "SW_HID_AUTH_FIELD_LOCAL_URL"

inline constexpr OStringLiteral HID_BUSINESS_FMT_PAGE = "SW_HID_BUSINESS_FMT_PAGE";
inline constexpr OStringLiteral HID_BUSINESS_FMT_PAGE_CONT = "SW_HID_BUSINESS_FMT_PAGE_CONT";
inline constexpr OStringLiteral HID_BUSINESS_FMT_PAGE_SHEET = "SW_HID_BUSINESS_FMT_PAGE_SHEET";
inline constexpr OStringLiteral HID_BUSINESS_FMT_PAGE_BRAND = "SW_HID_BUSINESS_FMT_PAGE_BRAND";
inline constexpr OStringLiteral HID_BUSINESS_FMT_PAGE_TYPE = "SW_HID_BUSINESS_FMT_PAGE_TYPE";
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

inline constexpr OStringLiteral HID_PVIEW_ZOOM_LB = "SW_HID_PVIEW_ZOOM_LB";

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
inline constexpr OStringLiteral HID_MM_NEXT_PAGE = "SW_HID_MM_NEXT_PAGE";
inline constexpr OStringLiteral HID_MM_PREV_PAGE = "SW_HID_MM_PREV_PAGE";
inline constexpr OStringLiteral HID_MM_ADDBLOCK_ELEMENTS = "SW_HID_MM_ADDBLOCK_ELEMENTS";
inline constexpr OStringLiteral HID_MM_ADDBLOCK_INSERT = "SW_HID_MM_ADDBLOCK_INSERT";
inline constexpr OStringLiteral HID_MM_ADDBLOCK_REMOVE = "SW_HID_MM_ADDBLOCK_REMOVE";
inline constexpr OStringLiteral HID_MM_ADDBLOCK_DRAG = "SW_HID_MM_ADDBLOCK_DRAG";
inline constexpr OStringLiteral HID_MM_ADDBLOCK_PREVIEW = "SW_HID_MM_ADDBLOCK_PREVIEW";
inline constexpr OStringLiteral HID_MM_ADDBLOCK_MOVEBUTTONS = "SW_HID_MM_ADDBLOCK_MOVEBUTTONS";

inline constexpr OStringLiteral HID_TBX_FORMULA_CALC = "SW_HID_TBX_FORMULA_CALC";
inline constexpr OStringLiteral HID_TBX_FORMULA_CANCEL = "SW_HID_TBX_FORMULA_CANCEL";
inline constexpr OStringLiteral HID_TBX_FORMULA_APPLY = "SW_HID_TBX_FORMULA_APPLY";

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#pragma once


#include <rtl/ustring.hxx>

inline constexpr OUString HID_EDIT_WIN = u"SW_HID_EDIT_WIN"_ustr;

inline constexpr OUString HID_PAGEPREVIEW = u"SW_HID_PAGEPREVIEW"_ustr;
inline constexpr OUString HID_SOURCE_EDITWIN = u"SW_HID_SOURCE_EDITWIN"_ustr;

// Dialog Help-IDs

inline constexpr OUString HID_NAVIGATOR_TREELIST = u"SW_HID_NAVIGATOR_TREELIST"_ustr;
inline constexpr OUString HID_NAVIGATOR_TOOLBOX = u"SW_HID_NAVIGATOR_TOOLBOX"_ustr;
inline constexpr OUString HID_NAVIGATOR_LISTBOX = u"SW_HID_NAVIGATOR_LISTBOX"_ustr;
inline constexpr OUString HID_NAVIGATOR_GLOBAL_TOOLBOX = u"SW_HID_NAVIGATOR_GLOBAL_TOOLBOX"_ustr;
inline constexpr OUString HID_NAVIGATOR_GLOB_TREELIST = u"SW_HID_NAVIGATOR_GLOB_TREELIST"_ustr;

// TabPage Help-IDs

inline constexpr OUString HID_LINGU_AUTOCORR = u"SW_HID_LINGU_AUTOCORR"_ustr;
inline constexpr OUString HID_LINGU_REPLACE = u"SW_HID_LINGU_REPLACE"_ustr;
inline constexpr OUString HID_LINGU_IGNORE_SELECTION = u"SW_HID_LINGU_IGNORE_SELECTION"_ustr;    // grammar check context menu

// More Help-IDs
inline constexpr OUString HID_EDIT_FORMULA = u"SW_HID_EDIT_FORMULA"_ustr;

inline constexpr OUString HID_AUTH_FIELD_IDENTIFIER = u"SW_HID_AUTH_FIELD_IDENTIFIER"_ustr;
inline constexpr OUString HID_AUTH_FIELD_AUTHORITY_TYPE = u"SW_HID_AUTH_FIELD_AUTHORITY_TYPE"_ustr;
inline constexpr OUString HID_AUTH_FIELD_ADDRESS = u"SW_HID_AUTH_FIELD_ADDRESS"_ustr;
inline constexpr OUString HID_AUTH_FIELD_ANNOTE = u"SW_HID_AUTH_FIELD_ANNOTE"_ustr;
inline constexpr OUString HID_AUTH_FIELD_AUTHOR = u"SW_HID_AUTH_FIELD_AUTHOR"_ustr;
inline constexpr OUString HID_AUTH_FIELD_BOOKTITLE = u"SW_HID_AUTH_FIELD_BOOKTITLE"_ustr;
inline constexpr OUString HID_AUTH_FIELD_CHAPTER = u"SW_HID_AUTH_FIELD_CHAPTER"_ustr;
inline constexpr OUString HID_AUTH_FIELD_EDITION = u"SW_HID_AUTH_FIELD_EDITION"_ustr;
inline constexpr OUString HID_AUTH_FIELD_EDITOR = u"SW_HID_AUTH_FIELD_EDITOR"_ustr;
inline constexpr OUString HID_AUTH_FIELD_HOWPUBLISHED = u"SW_HID_AUTH_FIELD_HOWPUBLISHED"_ustr;
inline constexpr OUString HID_AUTH_FIELD_INSTITUTION = u"SW_HID_AUTH_FIELD_INSTITUTION"_ustr;
inline constexpr OUString HID_AUTH_FIELD_JOURNAL = u"SW_HID_AUTH_FIELD_JOURNAL"_ustr;
inline constexpr OUString HID_AUTH_FIELD_MONTH = u"SW_HID_AUTH_FIELD_MONTH"_ustr;
inline constexpr OUString HID_AUTH_FIELD_NOTE = u"SW_HID_AUTH_FIELD_NOTE"_ustr;
inline constexpr OUString HID_AUTH_FIELD_NUMBER = u"SW_HID_AUTH_FIELD_NUMBER"_ustr;
inline constexpr OUString HID_AUTH_FIELD_ORGANIZATIONS = u"SW_HID_AUTH_FIELD_ORGANIZATIONS"_ustr;
inline constexpr OUString HID_AUTH_FIELD_PAGES = u"SW_HID_AUTH_FIELD_PAGES"_ustr;
inline constexpr OUString HID_AUTH_FIELD_PUBLISHER = u"SW_HID_AUTH_FIELD_PUBLISHER"_ustr;
inline constexpr OUString HID_AUTH_FIELD_SCHOOL = u"SW_HID_AUTH_FIELD_SCHOOL"_ustr;
inline constexpr OUString HID_AUTH_FIELD_SERIES = u"SW_HID_AUTH_FIELD_SERIES"_ustr;
inline constexpr OUString HID_AUTH_FIELD_TITLE = u"SW_HID_AUTH_FIELD_TITLE"_ustr;
inline constexpr OUString HID_AUTH_FIELD_REPORT_TYPE = u"SW_HID_AUTH_FIELD_REPORT_TYPE"_ustr;
inline constexpr OUString HID_AUTH_FIELD_VOLUME = u"SW_HID_AUTH_FIELD_VOLUME"_ustr;
inline constexpr OUString HID_AUTH_FIELD_YEAR = u"SW_HID_AUTH_FIELD_YEAR"_ustr;
inline constexpr OUString HID_AUTH_FIELD_URL = u"SW_HID_AUTH_FIELD_URL"_ustr;
inline constexpr OUString HID_AUTH_FIELD_CUSTOM1 = u"SW_HID_AUTH_FIELD_CUSTOM1"_ustr;
inline constexpr OUString HID_AUTH_FIELD_CUSTOM2 = u"SW_HID_AUTH_FIELD_CUSTOM2"_ustr;
inline constexpr OUString HID_AUTH_FIELD_CUSTOM3 = u"SW_HID_AUTH_FIELD_CUSTOM3"_ustr;
inline constexpr OUString HID_AUTH_FIELD_CUSTOM4 = u"SW_HID_AUTH_FIELD_CUSTOM4"_ustr;
inline constexpr OUString HID_AUTH_FIELD_CUSTOM5 = u"SW_HID_AUTH_FIELD_CUSTOM5"_ustr;
inline constexpr OUString HID_AUTH_FIELD_ISBN = u"SW_HID_AUTH_FIELD_ISBN"_ustr;
inline constexpr OUString HID_AUTH_FIELD_LOCAL_URL = u"SW_HID_AUTH_FIELD_LOCAL_URL"_ustr;
inline constexpr OUString HID_AUTH_FIELD_TARGET_TYPE = u"SW_HID_AUTH_FIELD_TARGET_TYPE"_ustr;
inline constexpr OUString HID_AUTH_FIELD_TARGET_URL = u"SW_HID_AUTH_FIELD_TARGET_URL"_ustr;


inline constexpr OUString HID_BUSINESS_FMT_PAGE = u"SW_HID_BUSINESS_FMT_PAGE"_ustr;
inline constexpr OUString HID_BUSINESS_FMT_PAGE_CONT = u"SW_HID_BUSINESS_FMT_PAGE_CONT"_ustr;
inline constexpr OUString HID_BUSINESS_FMT_PAGE_SHEET = u"SW_HID_BUSINESS_FMT_PAGE_SHEET"_ustr;
inline constexpr OUString HID_BUSINESS_FMT_PAGE_BRAND = u"SW_HID_BUSINESS_FMT_PAGE_BRAND"_ustr;
inline constexpr OUString HID_BUSINESS_FMT_PAGE_TYPE = u"SW_HID_BUSINESS_FMT_PAGE_TYPE"_ustr;
                      "SW_HID_SEND_MASTER_CTRL_PUSHBUTTON_OK"
                  "SW_HID_SEND_MASTER_CTRL_PUSHBUTTON_CANCEL"
                     "SW_HID_SEND_MASTER_CTRL_LISTBOX_FILTER"
                   "SW_HID_SEND_MASTER_CTRL_CONTROL_FILEVIEW"
                       "SW_HID_SEND_MASTER_CTRL_EDIT_FILEURL"
             "SW_HID_SEND_MASTER_CTRL_CHECKBOX_AUTOEXTENSION"
                   "SW_HID_SEND_MASTER_CTRL_LISTBOX_TEMPLATE"

                        "SW_HID_SEND_HTML_CTRL_PUSHBUTTON_OK"
                    "SW_HID_SEND_HTML_CTRL_PUSHBUTTON_CANCEL"
                       "SW_HID_SEND_HTML_CTRL_LISTBOX_FILTER"
                     "SW_HID_SEND_HTML_CTRL_CONTROL_FILEVIEW"
                         "SW_HID_SEND_HTML_CTRL_EDIT_FILEURL"
               "SW_HID_SEND_HTML_CTRL_CHECKBOX_AUTOEXTENSION"
                     "SW_HID_SEND_HTML_CTRL_LISTBOX_TEMPLATE"

inline constexpr OUString HID_PVIEW_ZOOM_LB = u"SW_HID_PVIEW_ZOOM_LB"_ustr;

inline constexpr OUString HID_MM_NEXT_PAGE = u"SW_HID_MM_NEXT_PAGE"_ustr;
inline constexpr OUString HID_MM_PREV_PAGE = u"SW_HID_MM_PREV_PAGE"_ustr;
inline constexpr OUString HID_MM_ADDBLOCK_ELEMENTS = u"SW_HID_MM_ADDBLOCK_ELEMENTS"_ustr;
inline constexpr OUString HID_MM_ADDBLOCK_INSERT = u"SW_HID_MM_ADDBLOCK_INSERT"_ustr;
inline constexpr OUString HID_MM_ADDBLOCK_REMOVE = u"SW_HID_MM_ADDBLOCK_REMOVE"_ustr;
inline constexpr OUString HID_MM_ADDBLOCK_DRAG = u"SW_HID_MM_ADDBLOCK_DRAG"_ustr;
inline constexpr OUString HID_MM_ADDBLOCK_PREVIEW = u"SW_HID_MM_ADDBLOCK_PREVIEW"_ustr;
inline constexpr OUString HID_MM_ADDBLOCK_MOVEBUTTONS = u"SW_HID_MM_ADDBLOCK_MOVEBUTTONS"_ustr;

inline constexpr OUString HID_TBX_FORMULA_CALC = u"SW_HID_TBX_FORMULA_CALC"_ustr;
inline constexpr OUString HID_TBX_FORMULA_CANCEL = u"SW_HID_TBX_FORMULA_CANCEL"_ustr;
inline constexpr OUString HID_TBX_FORMULA_APPLY = u"SW_HID_TBX_FORMULA_APPLY"_ustr;

// Navigator context menu
inline constexpr OUString HID_NAV_OUTLINE_TRACKING = u"SW_HID_NAV_OUTLINE_TRACKING"_ustr;
inline constexpr OUString HID_NAV_OUTLINE_LEVEL = u"SW_HID_NAV_OUTLINE_LEVEL"_ustr;
inline constexpr OUString HID_NAV_DRAG_MODE = u"SW_HID_NAV_DRAG_MODE"_ustr;
inline constexpr OUString HID_NAV_DISPLAY = u"SW_HID_NAV_DISPLAY"_ustr;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

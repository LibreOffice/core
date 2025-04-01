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

inline constexpr OUString HID_DATABROWSE_HEADER = u"DBACCESS_HID_DATABROWSE_HEADER"_ustr;
inline constexpr OUString HID_CTL_TABBROWSER = u"DBACCESS_HID_CTL_TABBROWSER"_ustr;
inline constexpr OUString HID_CTL_TREEVIEW = u"DBACCESS_HID_CTL_TREEVIEW"_ustr;

inline constexpr OUString HID_DSADMIN_BROWSECONN = u"DBACCESS_HID_DSADMIN_BROWSECONN"_ustr;

inline constexpr OUString HID_CTL_QRYSQLEDIT = u"DBACCESS_HID_CTL_QRYSQLEDIT"_ustr;

inline constexpr OUString HID_CTL_QRYDGNTAB = u"DBACCESS_HID_CTL_QRYDGNTAB"_ustr;
inline constexpr OUString HID_CTL_QRYDGNCRIT = u"DBACCESS_HID_CTL_QRYDGNCRIT"_ustr;
inline constexpr OUString HID_QRYDGN_ROW_VISIBLE = u"DBACCESS_HID_QRYDGN_ROW_VISIBLE"_ustr;
inline constexpr OUString HID_QRYDGN_ROW_TABLE = u"DBACCESS_HID_QRYDGN_ROW_TABLE"_ustr;
inline constexpr OUString HID_QRYDGN_ROW_FIELD = u"DBACCESS_HID_QRYDGN_ROW_FIELD"_ustr;
inline constexpr OUString HID_QRYDGN_ROW_ORDER = u"DBACCESS_HID_QRYDGN_ROW_ORDER"_ustr;
inline constexpr OUString HID_QRYDGN_ROW_FUNCTION = u"DBACCESS_HID_QRYDGN_ROW_FUNCTION"_ustr;
inline constexpr OUString HID_QRYDGN_ROW_ALIAS = u"DBACCESS_HID_QRYDGN_ROW_ALIAS"_ustr;
inline constexpr OUString HID_QRYDGN_ROW_CRIT = u"DBACCESS_HID_QRYDGN_ROW_CRIT"_ustr;

inline constexpr OUString HID_TAB_DESIGN_FIELDCONTROL = u"DBACCESS_HID_TAB_DESIGN_FIELDCONTROL"_ustr;
inline constexpr OUString HID_TAB_DESIGN_DESCWIN = u"DBACCESS_HID_TAB_DESIGN_DESCWIN"_ustr;
inline constexpr OUString HID_TABDESIGN_BACKGROUND = u"DBACCESS_HID_TABDESIGN_BACKGROUND"_ustr;
inline constexpr OUString HID_CTL_TABLEEDIT = u"DBACCESS_HID_CTL_TABLEEDIT"_ustr;
inline constexpr OUString HID_TABDESIGN_NAMECELL = u"DBACCESS_HID_TABDESIGN_NAMECELL"_ustr;
inline constexpr OUString HID_TABDESIGN_TYPECELL = u"DBACCESS_HID_TABDESIGN_TYPECELL"_ustr;
inline constexpr OUString HID_TABDESIGN_COMMENTCELL = u"DBACCESS_HID_TABDESIGN_COMMENTCELL"_ustr;
inline constexpr OUString HID_TAB_DESIGN_HELP_TEXT_FRAME = u"DBACCESS_HID_TAB_DESIGN_HELP_TEXT_FRAME"_ustr;
inline constexpr OUString HID_TAB_ENT_DEFAULT = u"DBACCESS_HID_TAB_ENT_DEFAULT"_ustr;
inline constexpr OUString HID_TAB_ENT_FORMAT_SAMPLE = u"DBACCESS_HID_TAB_ENT_FORMAT_SAMPLE"_ustr;
inline constexpr OUString HID_TAB_ENT_FORMAT = u"DBACCESS_HID_TAB_ENT_FORMAT"_ustr;
inline constexpr OUString HID_TAB_ENT_BOOL_DEFAULT = u"DBACCESS_HID_TAB_ENT_BOOL_DEFAULT"_ustr;
inline constexpr OUString HID_TAB_ENT_REQUIRED = u"DBACCESS_HID_TAB_ENT_REQUIRED"_ustr;
inline constexpr OUString HID_TAB_ENT_AUTOINCREMENT = u"DBACCESS_HID_TAB_ENT_AUTOINCREMENT"_ustr;
inline constexpr OUString HID_TAB_ENT_TEXT_LEN = u"DBACCESS_HID_TAB_ENT_TEXT_LEN"_ustr;
inline constexpr OUString HID_TAB_ENT_TYPE = u"DBACCESS_HID_TAB_ENT_TYPE"_ustr;
inline constexpr OUString HID_TAB_ENT_COLUMNNAME = u"DBACCESS_HID_TAB_ENT_COLUMNNAME"_ustr;
inline constexpr OUString HID_TAB_ENT_NUMTYP = u"DBACCESS_HID_TAB_ENT_NUMTYP"_ustr;
inline constexpr OUString HID_TAB_ENT_LEN = u"DBACCESS_HID_TAB_ENT_LEN"_ustr;
inline constexpr OUString HID_TAB_ENT_SCALE = u"DBACCESS_HID_TAB_ENT_SCALE"_ustr;
inline constexpr OUString HID_CTL_RELATIONTAB = u"DBACCESS_HID_CTL_RELATIONTAB"_ustr;
inline constexpr OUString HID_RELATIONDIALOG_LEFTFIELDCELL = u"DBACCESS_HID_RELATIONDIALOG_LEFTFIELDCELL"_ustr;
inline constexpr OUString HID_RELATIONDIALOG_RIGHTFIELDCELL = u"DBACCESS_HID_RELATIONDIALOG_RIGHTFIELDCELL"_ustr;
inline constexpr OUString HID_CONFIRM_DROP_BUTTON_ALL = u"DBACCESS_HID_CONFIRM_DROP_BUTTON_ALL"_ustr;
inline constexpr OUString HID_DSADMIN_LDAP_HOSTNAME = u"DBACCESS_HID_DSADMIN_LDAP_HOSTNAME"_ustr;
inline constexpr OUString HID_DSADMIN_MOZILLA_PROFILE_NAME = u"DBACCESS_HID_DSADMIN_MOZILLA_PROFILE_NAME"_ustr;
inline constexpr OUString HID_DSADMIN_THUNDERBIRD_PROFILE_NAME = u"DBACCESS_HID_DSADMIN_THUNDERBIRD_PROFILE_NAME"_ustr;

inline constexpr OUString HID_DLGINDEX_INDEXDETAILS_FIELD = u"DBACCESS_HID_DLGINDEX_INDEXDETAILS_FIELD"_ustr;
inline constexpr OUString HID_DLGINDEX_INDEXDETAILS_SORTORDER = u"DBACCESS_HID_DLGINDEX_INDEXDETAILS_SORTORDER"_ustr;

inline constexpr OUString HID_TAB_AUTOINCREMENTVALUE = u"DBACCESS_HID_TAB_AUTOINCREMENTVALUE"_ustr;

inline constexpr OUString HID_DSADMIN_DBASE_PATH = u"DBACCESS_HID_DSADMIN_DBASE_PATH"_ustr;
inline constexpr OUString HID_DSADMIN_MYSQL_DATABASE = u"DBACCESS_HID_DSADMIN_MYSQL_DATABASE"_ustr;
inline constexpr OUString HID_DSADMIN_MYSQL_ODBC_DATASOURCE = u"DBACCESS_HID_DSADMIN_MYSQL_ODBC_DATASOURCE"_ustr;
inline constexpr OUString HID_DSADMIN_ODBC_DATASOURCE = u"DBACCESS_HID_DSADMIN_ODBC_DATASOURCE"_ustr;
inline constexpr OUString HID_DSADMIN_MSACCESS_MDB_FILE = u"DBACCESS_HID_DSADMIN_MSACCESS_MDB_FILE"_ustr;
inline constexpr OUString HID_DSADMIN_FLAT_PATH = u"DBACCESS_HID_DSADMIN_FLAT_PATH"_ustr;
inline constexpr OUString HID_DSADMIN_CALC_PATH = u"DBACCESS_HID_DSADMIN_CALC_PATH"_ustr;
inline constexpr OUString HID_DSADMIN_WRITER_PATH = u"DBACCESS_HID_DSADMIN_WRITER_PATH"_ustr;
inline constexpr OUString HID_DSADMIN_ORACLE_DATABASE = u"DBACCESS_HID_DSADMIN_ORACLE_DATABASE"_ustr;

inline constexpr OUString HID_APP_TABLE_TREE = u"DBACCESS_HID_APP_TABLE_TREE"_ustr;
inline constexpr OUString HID_APP_FORM_TREE = u"DBACCESS_HID_APP_FORM_TREE"_ustr;
inline constexpr OUString HID_APP_QUERY_TREE = u"DBACCESS_HID_APP_QUERY_TREE"_ustr;
inline constexpr OUString HID_APP_REPORT_TREE = u"DBACCESS_HID_APP_REPORT_TREE"_ustr;
inline constexpr OUString HID_APP_CREATION_LIST = u"DBACCESS_HID_APP_CREATION_LIST"_ustr;
inline constexpr OUString HID_APP_SWAP_ICONCONTROL = u"DBACCESS_HID_APP_SWAP_ICONCONTROL"_ustr;


inline constexpr OUString HID_APP_HELP_TEXT = u"DBACCESS_HID_APP_HELP_TEXT"_ustr;
inline constexpr OUString HID_APP_DESCRIPTION_TEXT = u"DBACCESS_HID_APP_DESCRIPTION_TEXT"_ustr;
inline constexpr OUString HID_APP_VIEW_PREVIEW_CB = u"DBACCESS_HID_APP_VIEW_PREVIEW_CB"_ustr;
inline constexpr OUString HID_APP_VIEW_PREVIEW_1 = u"DBACCESS_HID_APP_VIEW_PREVIEW_1"_ustr;
inline constexpr OUString HID_APP_VIEW_PREVIEW_2 = u"DBACCESS_HID_APP_VIEW_PREVIEW_2"_ustr;
inline constexpr OUString HID_TABDESIGN_HELPTEXT = u"DBACCESS_HID_TABDESIGN_HELPTEXT"_ustr;
inline constexpr OUString HID_APP_VIEW_PREVIEW_3 = u"DBACCESS_HID_APP_VIEW_PREVIEW_3"_ustr;


inline constexpr OUString HID_DBWIZ_PREVIOUS = u"DBACCESS_HID_DBWIZ_PREVIOUS"_ustr;
inline constexpr OUString HID_DBWIZ_NEXT = u"DBACCESS_HID_DBWIZ_NEXT"_ustr;
inline constexpr OUString HID_DBWIZ_CANCEL = u"DBACCESS_HID_DBWIZ_CANCEL"_ustr;
inline constexpr OUString HID_DBWIZ_FINISH = u"DBACCESS_HID_DBWIZ_FINISH"_ustr;
inline constexpr OUString HID_DBWIZ_HELP = u"DBACCESS_HID_DBWIZ_HELP"_ustr;

inline constexpr OUString HID_DBWIZ_ROADMAP = u"DBACCESS_HID_DBWIZ_ROADMAP"_ustr;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

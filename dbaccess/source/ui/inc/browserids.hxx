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

#include <svx/svxids.hrc>
#include <dbaccess_slotid.hrc>

#define ID_BROWSER_COPY                     SID_COPY
#define ID_BROWSER_CUT                      SID_CUT
#define ID_BROWSER_EDITDOC                  SID_EDITDOC
#define ID_BROWSER_UNDORECORD               SID_FM_RECORD_UNDO
#define ID_BROWSER_SAVERECORD               SID_FM_RECORD_SAVE
#define ID_BROWSER_PASTE                    SID_PASTE
#define ID_BROWSER_CLIPBOARD_FORMAT_ITEMS   SID_CLIPBOARD_FORMAT_ITEMS
#define ID_BROWSER_REDO                     SID_REDO
#define ID_BROWSER_SAVEDOC                  SID_SAVEDOC
#define ID_BROWSER_SAVEASDOC                SID_SAVEASDOC
#define ID_BROWSER_TITLE                    SID_DOCINFO_TITLE
#define ID_BROWSER_UNDO                     SID_UNDO
#define ID_BROWSER_INSERTCOLUMNS            SID_SBA_BRW_INSERT
#define ID_BROWSER_FORMLETTER               SID_SBA_BRW_MERGE
#define ID_BROWSER_INSERTCONTENT            SID_SBA_BRW_UPDATE

#define ID_BROWSER_SEARCH                   SID_FM_SEARCH
#define ID_BROWSER_SORTUP                   SID_FM_SORTUP
#define ID_BROWSER_SORTDOWN                 SID_FM_SORTDOWN
#define ID_BROWSER_AUTOFILTER               SID_FM_AUTOFILTER
#define ID_BROWSER_FILTERCRIT               SID_FM_FILTERCRIT
#define ID_BROWSER_ORDERCRIT                SID_FM_ORDERCRIT
#define ID_BROWSER_REMOVEFILTER             SID_FM_REMOVE_FILTER_SORT
#define ID_BROWSER_FILTERED                 SID_FM_FORM_FILTERED
#define ID_BROWSER_REFRESH                  SID_FM_REFRESH
#define ID_BROWSER_COLATTRSET               10020  // column formatting
#define ID_BROWSER_COLWIDTH                 10021  // column width
#define ID_BROWSER_TABLEATTR                10022  // table format attributes
#define ID_BROWSER_ROWHEIGHT                10023  // row height
#define ID_BROWSER_COLUMNINFO               10024  // copies the column description to insert it into the table design
#define ID_BROWSER_ADDTABLE                 SID_FM_ADDTABLE
#define ID_BROWSER_EXPLORER                 SID_DSBROWSER_EXPLORER
#define ID_BROWSER_DOCUMENT_DATASOURCE      SID_DOCUMENT_DATA_SOURCE

// The following ids are local to special components (e.g. menus), so they don't need to be unique
// overall. Please have this in mind when changing anything
#define ID_TREE_EDIT_DATABASE                1
#define ID_TREE_CLOSE_CONN                   2
    // FREE
#define ID_TREE_ADMINISTRATE                 4

#define ID_REPORT_NEW_TEXT                  14
#define ID_FORM_NEW_TEXT                    15
#define ID_FORM_NEW_CALC                    16
#define ID_FORM_NEW_IMPRESS                 17
#define ID_NEW_QUERY_DESIGN                 20
#define ID_EDIT_QUERY_DESIGN                21
#define ID_NEW_QUERY_SQL                    22
#define ID_EDIT_QUERY_SQL                   23
#define ID_NEW_TABLE_DESIGN                 25
#define ID_NEW_VIEW_DESIGN                  28
#define ID_DIRECT_SQL                       32
#define ID_BROWSER_REFRESH_REBUILD          34
#define ID_INDEX_NEW                        36
#define ID_INDEX_DROP                       37
#define ID_INDEX_RENAME                     38
#define ID_INDEX_SAVE                       39
#define ID_INDEX_RESET                      40
#define ID_DOCUMENT_CREATE_REPWIZ           41
#define ID_BROWSER_SQL                      42

#define ID_APP_NEW_QUERY_AUTO_PILOT         44
#define ID_NEW_TABLE_DESIGN_AUTO_PILOT      45
#define ID_NEW_VIEW_DESIGN_AUTO_PILOT       46


// other
#define ID_BROWSER_QUERY_EXECUTE            SID_FM_EXECUTE

#define ID_BROWSER_CLOSE                    SID_CLOSEWIN
#define ID_BROWSER_ESCAPEPROCESSING         SID_FM_NATIVESQL

#define ID_BROWSER_INSERT_ROW               (SID_SBA_START + 46) // insert row


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

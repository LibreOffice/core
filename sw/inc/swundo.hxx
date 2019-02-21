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
#ifndef INCLUDED_SW_INC_SWHXX
#define INCLUDED_SW_INC_SWHXX

#include <vector>

#include <rtl/ustring.hxx>

typedef std::vector< OUString > SwUndoComments_t;

// The IDs for StdActions.
enum class SwUndoId
{
    EMPTY = 0,
    STD_BEGIN = 1,
    START = STD_BEGIN,                 //  1
    END,                               //  2

        REPEAT_START,               // All UndoIDs between REPEAT_START and
                                    // REPEAT_END are Repeat-enabled !!
    DELETE = REPEAT_START,             //  3
    INSERT,                            //  4
    OVERWRITE,                         //  5
    SPLITNODE,                         //  6
    INSATTR,                           //  7
    SETFMTCOLL,                        //  8
    RESETATTR,                         //  9
    INSFMTATTR,                        // 10
    INSDOKUMENT,                       // 11
    COPY,                              // 12
    INSTABLE,                          // 13
    TABLETOTEXT,                       // 14
    TEXTTOTABLE,                       // 15
    SORT_TXT,                          // 16
    INSLAYFMT,                         // 17
    TABLEHEADLINE,                     // 18
    INSSECTION,                        // 19
    OUTLINE_LR,                        // 20
    OUTLINE_UD,                        // 21
    INSNUM,                            // 22
    NUMUP,                             // 23
    MOVENUM,                           // 24
    INSDRAWFMT,                        // 25
    NUMORNONUM,                        // 26
    INC_LEFTMARGIN,                    // 27
    DEC_LEFTMARGIN,                    // 28
    INSERTLABEL,                       // 29
    SETNUMRULESTART,                   // 30
    CHGFTN,                            // 31
    REDLINE,                           // 32
    ACCEPT_REDLINE,                    // 33
    REJECT_REDLINE,                    // 34
    SPLIT_TABLE,                       // 35
    DONTEXPAND,                        // 36
    AUTOCORRECT,                       // 37
    MERGE_TABLE,                       // 38
    TRANSLITERATE,                     // 39
    PASTE_CLIPBOARD,                   // 40
    TYPING,                            // 41
        REPEAT_END = 46,

    MOVE = REPEAT_END,                 // 46
    INSGLOSSARY,                       // 47
    DELBOOKMARK,                       // 48
    INSBOOKMARK,                       // 49
    SORT_TBL,                          // 50
    DELLAYFMT,                         // 51
    AUTOFORMAT,                        // 52
    REPLACE,                           // 53
    DELSECTION,                        // 54
    CHGSECTION,                        // 55
    SETDEFTATTR = 57,                  // 57
    DELNUM,                            // 58
    DRAWUNDO,                          // 59
    DRAWGROUP,                         // 60
    DRAWUNGROUP,                       // 61
    DRAWDELETE,                        // 62
    REREAD,                            // 63
    DELGRF,                            // 64
    TABLE_ATTR = 66,                   // 66
    TABLE_AUTOFMT,                     // 67
    TABLE_INSCOL,                      // 68
    TABLE_INSROW,                      // 69
    TABLE_DELBOX,                      // 70
    TABLE_SPLIT,                       // 71
    TABLE_MERGE,                       // 72
    TBLNUMFMT,                         // 73
    INSTOX,                            // 74
    CLEARTOXRANGE,                     // 75
    TBLCPYTBL,                         // 76
    CPYTBL,                            // 77
    INS_FROM_SHADOWCRSR,               // 78
    CHAINE,                            // 79
    UNCHAIN,                           // 80
    FTNINFO,                           // 81
    COMPAREDOC = 83,                   // 83
    SETFLYFRMFMT,                      // 84
    SETRUBYATTR,                       // 85
    TOXCHANGE = 87,                    // 87
    CREATE_PAGEDESC,                   // 88
    CHANGE_PAGEDESC,                   // 89
    DELETE_PAGEDESC,                   // 90
    HEADER_FOOTER,                     // 91 #i7983#
    FIELD,                             // 92
    TXTFMTCOL_CREATE,                  // 93
    TXTFMTCOL_DELETE,                  // 94
    TXTFMTCOL_RENAME,                  // 95
    CHARFMT_CREATE,                    // 96
    CHARFMT_DELETE,                    // 97
    CHARFMT_RENAME,                    // 98
    FRMFMT_CREATE,                     // 99
    FRMFMT_DELETE,                     // 100
    FRMFMT_RENAME,                     // 101
    NUMRULE_CREATE,                    // 102
    NUMRULE_DELETE,                    // 103
    NUMRULE_RENAME,                    // 104
    BOOKMARK_RENAME,                   // 105
    INDEX_ENTRY_INSERT,                // 106
    INDEX_ENTRY_DELETE,                // 107
    COL_DELETE,                        // 108
    ROW_DELETE,                        // 109
    RENAME_PAGEDESC,                   // 110
    NUMDOWN,                           // 111
    // --> #i73249#
    FLYFRMFMT_TITLE,                   // 112
    FLYFRMFMT_DESCRIPTION,             // 113
    TBLSTYLE_CREATE,                   // 114
    TBLSTYLE_DELETE,                   // 115
    TBLSTYLE_UPDATE,                   // 116
    PARA_SIGN_ADD,                     // 117

    // UI undo ID's...
    UI_REPLACE,                        // 118
    UI_INSERT_PAGE_BREAK,              // 119
    UI_INSERT_COLUMN_BREAK,            // 120
    UI_INSERT_ENVELOPE = 122,          // 122
    UI_DRAG_AND_COPY,                  // 123
    UI_DRAG_AND_MOVE,                  // 124
    UI_INSERT_CHART,                   // 125
    UI_INSERT_FOOTNOTE,                // 126
    UI_INSERT_URLBTN,                  // 127
    UI_INSERT_URLTXT,                  // 128
    UI_DELETE_INVISIBLECNTNT,          // 129
    UI_REPLACE_STYLE,                  // 130
    UI_DELETE_PAGE_BREAK,              // 131
    UI_TEXT_CORRECTION,                // 132
    UI_TABLE_DELETE,                   // 133
    CONFLICT,                          // 134

    INSERT_FORM_FIELD                  // 135
};

OUString GetUndoComment(SwUndoId eId);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

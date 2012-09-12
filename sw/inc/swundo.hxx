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
#ifndef SWUNDO_HXX
#define SWUNDO_HXX

#include <vector>

#include <rtl/ustring.hxx>


typedef ::std::vector< ::rtl::OUString > SwUndoComments_t;


// The IDs for StdActions.
enum SwUndoId
{
    UNDO_EMPTY = 0,
    UNDO_STD_BEGIN = 1,
    UNDO_START = UNDO_STD_BEGIN,            //  1
    UNDO_END,                               //  2
        REPEAT_START,               // All UndoIDs between REPEAT_START and
                                    // REPEAT_END are Repeat-enabled !!
    UNDO_DELETE = REPEAT_START,             //  3
    UNDO_INSERT,                            //  4
    UNDO_OVERWRITE,                         //  5
    UNDO_SPLITNODE,                         //  6
    UNDO_INSATTR,                           //  7
    UNDO_SETFMTCOLL,                        //  8
    UNDO_RESETATTR,                         //  9
    UNDO_INSFMTATTR,                        // 10
    UNDO_INSDOKUMENT,                       // 11
    UNDO_COPY,                              // 12
    UNDO_INSTABLE,                          // 13
    UNDO_TABLETOTEXT,                       // 14
    UNDO_TEXTTOTABLE,                       // 15
    UNDO_SORT_TXT,                          // 16
    UNDO_INSLAYFMT,                         // 17
    UNDO_TABLEHEADLINE,                     // 18
    UNDO_INSSECTION,                        // 19
    UNDO_OUTLINE_LR,                        // 20
    UNDO_OUTLINE_UD,                        // 21
    UNDO_INSNUM,                            // 22
    UNDO_NUMUP,                             // 23
    UNDO_MOVENUM,                           // 24
    UNDO_INSDRAWFMT,                        // 25
    UNDO_NUMORNONUM,                        // 26
    UNDO_INC_LEFTMARGIN,                    // 27
    UNDO_DEC_LEFTMARGIN,                    // 28
    UNDO_INSERTLABEL,                       // 29
    UNDO_SETNUMRULESTART,                   // 30
    UNDO_CHGFTN,                            // 31
    UNDO_REDLINE,                           // 32
    UNDO_ACCEPT_REDLINE,                    // 33
    UNDO_REJECT_REDLINE,                    // 34
    UNDO_SPLIT_TABLE,                       // 35
    UNDO_DONTEXPAND,                        // 36
    UNDO_AUTOCORRECT,                       // 37
    UNDO_MERGE_TABLE,                       // 38
    UNDO_TRANSLITERATE,                     // 39

    UNDO_PASTE_CLIPBOARD,                   // 40
    UNDO_TYPING,                           // 41
    UNDO_REPEAT_DUMMY_6,                    // 42
    UNDO_REPEAT_DUMMY_7,                    // 43
    UNDO_REPEAT_DUMMY_8,                    // 44
    UNDO_REPEAT_DUMMY_9,                    // 45
    REPEAT_END,

    UNDO_MOVE = REPEAT_END,                 // 46
    UNDO_INSGLOSSARY,                       // 47
    UNDO_DELBOOKMARK,                       // 48
    UNDO_INSBOOKMARK,                       // 49
    UNDO_SORT_TBL,                          // 50
    UNDO_DELLAYFMT,                         // 51
    UNDO_AUTOFORMAT,                        // 52
    UNDO_REPLACE,                           // 53
    UNDO_DELSECTION,                        // 54
    UNDO_CHGSECTION,                        // 55
    UNDO_CHGSECTIONPASSWD,                  // 56
    UNDO_SETDEFTATTR,                       // 57
    UNDO_DELNUM,                            // 58
    UNDO_DRAWUNDO,                          // 59
    UNDO_DRAWGROUP,                         // 60
    UNDO_DRAWUNGROUP,                       // 61
    UNDO_DRAWDELETE,                        // 62
    UNDO_REREAD,                            // 63
    UNDO_DELGRF,                            // 64
    UNDO_DELOLE,                            // 65
    UNDO_TABLE_ATTR,                        // 66
    UNDO_TABLE_AUTOFMT,                     // 67
    UNDO_TABLE_INSCOL,                      // 68
    UNDO_TABLE_INSROW,                      // 69
    UNDO_TABLE_DELBOX,                      // 70
    UNDO_TABLE_SPLIT,                       // 71
    UNDO_TABLE_MERGE,                       // 72
    UNDO_TBLNUMFMT,                         // 73
    UNDO_INSTOX,                            // 74
    UNDO_CLEARTOXRANGE,                     // 75
    UNDO_TBLCPYTBL,                         // 76
    UNDO_CPYTBL,                            // 77
    UNDO_INS_FROM_SHADOWCRSR,               // 78
    UNDO_CHAINE,                            // 79
    UNDO_UNCHAIN,                           // 80
    UNDO_FTNINFO,                           // 81
    UNDO_ENDNOTEINFO,                       // 82
    UNDO_COMPAREDOC,                        // 83
    UNDO_SETFLYFRMFMT,                      // 84
    UNDO_SETRUBYATTR,                       // 85

    UNDO_TMPAUTOCORR,                       // 86
    UNDO_TOXCHANGE,                         // 87
    UNDO_CREATE_PAGEDESC,                           // 88
    UNDO_CHANGE_PAGEDESC,                           // 89
    UNDO_DELETE_PAGEDESC,                           // 90
    UNDO_HEADER_FOOTER,                           // 91 #i7983#
    UNDO_FIELD,                             // 92
    UNDO_TXTFMTCOL_CREATE,                   // 93
    UNDO_TXTFMTCOL_DELETE,                   // 94
    UNDO_TXTFMTCOL_RENAME, // 95
    UNDO_CHARFMT_CREATE, // 96
    UNDO_CHARFMT_DELETE, // 97
    UNDO_CHARFMT_RENAME, // 98
    UNDO_FRMFMT_CREATE, // 99
    UNDO_FRMFMT_DELETE, // 100
    UNDO_FRMFMT_RENAME, // 101
    UNDO_NUMRULE_CREATE,// 102
    UNDO_NUMRULE_DELETE,// 103
    UNDO_NUMRULE_RENAME,// 104
    UNDO_BOOKMARK_RENAME, // 105
    UNDO_INDEX_ENTRY_INSERT, // 106
    UNDO_INDEX_ENTRY_DELETE, // 107
    UNDO_COL_DELETE, // 108
    UNDO_ROW_DELETE, // 109
    UNDO_RENAME_PAGEDESC, // 110
    UNDO_NUMDOWN, // 111
    // --> #i73249#
    UNDO_FLYFRMFMT_TITLE,           // 112
    UNDO_FLYFRMFMT_DESCRIPTION,     // 113
    UNDO_STD_END= UNDO_FLYFRMFMT_DESCRIPTION,

    // UI undo ID's...
    UNDO_UI_REPLACE             =  UNDO_STD_END + 1,
    UNDO_UI_INSERT_PAGE_BREAK,
    UNDO_UI_INSERT_COLUMN_BREAK,
    UNDO_UI_PLAY_MACRO,
    UNDO_UI_INSERT_ENVELOPE,
    UNDO_UI_DRAG_AND_COPY,
    UNDO_UI_DRAG_AND_MOVE,
    UNDO_UI_INSERT_RULER,
    UNDO_UI_INSERT_CHART,
    UNDO_UI_INSERT_FOOTNOTE,
    UNDO_UI_INSERT_URLBTN,
    UNDO_UI_INSERT_URLTXT,
    UNDO_UI_DELETE_INVISIBLECNTNT,
    UNDO_UI_REPLACE_STYLE,
    UNDO_UI_DELETE_PAGE_BREAK,
    UNDO_UI_TEXT_CORRECTION
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

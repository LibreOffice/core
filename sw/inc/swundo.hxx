/*************************************************************************
 *
 *  $RCSfile: swundo.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2005-01-25 13:58:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SWUNDO_HXX
#define _SWUNDO_HXX

#include <svtools/svarray.hxx>

// die Ids fuer StdAktionen
enum SwUndoStdId
{
    UNDO_STD_BEGIN = 1,
    UNDO_START = UNDO_STD_BEGIN,            //  1
    UNDO_END,                               //  2
        REPEAT_START,               // alle UndoIds zwischen REPEAT_START und
                                    // REPEAT_END sind Repeat-Faehig !!
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

    // -> #111827#
    UNDO_PASTE_CLIPBOARD,                           // 40
    UNDO_TYPING,                           // 41
    // <- #111827#
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

    UNDO_TMPAUTOCORR,                       // 86 #102505#
    UNDO_TOXCHANGE,                         // 87
    UNDO_CREATE_PAGEDESC,                           // 88
    UNDO_CHANGE_PAGEDESC,                           // 89
    UNDO_DELETE_PAGEDESC,                           // 90
    UNDO_HEADER_FOOTER,                           // 91 #i7983#
    UNDO_FIELD,                             // 92 #111840#
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
    UNDO_STD_END= UNDO_NUMDOWN
};


#define INIT_UNDOIDS 20
#define GROW_UNDOIDS 32
// Das Array der verwendeten Undo-Ids
class String;
class SwUndoIdAndName
{
    USHORT nUndoId;
    String* pUndoStr;

public:
    SwUndoIdAndName() : nUndoId( 0 ), pUndoStr( 0 ) {}
    SwUndoIdAndName( USHORT nId, const String* pStr = 0 );
    ~SwUndoIdAndName();

    USHORT GetUndoId() const            { return nUndoId; }
    const String* GetUndoStr() const    { return pUndoStr; }
};
typedef SwUndoIdAndName* SwUndoIdAndNamePtr;
SV_DECL_PTRARR_DEL( SwUndoIds, SwUndoIdAndNamePtr, INIT_UNDOIDS, GROW_UNDOIDS )


// Undo-Ids fuer die UI-Seite
enum SwUIUndoIds
{
    UIUNDO_REPLACE             =  UNDO_STD_END + 1,
    UIUNDO_INSERT_PAGE_BREAK,
    UIUNDO_INSERT_COLUMN_BREAK,
    UIUNDO_PLAY_MACRO,
    UIUNDO_INSERT_ENVELOPE,
    UIUNDO_DRAG_AND_COPY,
    UIUNDO_DRAG_AND_MOVE,
    UIUNDO_INSERT_RULER,
    UIUNDO_INSERT_CHART,
    UIUNDO_INSERT_FOOTNOTE,
    UIUNDO_INSERT_URLBTN,
    UIUNDO_INSERT_URLTXT,
    UIUNDO_DELETE_INVISIBLECNTNT,
    UIUNDO_REPLACE_STYLE
};


#endif

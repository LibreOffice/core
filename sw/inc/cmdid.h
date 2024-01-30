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

#ifndef INCLUDED_SW_INC_CMDID_H
#define INCLUDED_SW_INC_CMDID_H

#include <sfx2/sfxsids.hrc>
#include <svx/svxids.hrc>
#include <svl/solar.hrc>

class SvxFrameDirectionItem;
class SvxSizeItem;
class SwAddPrinterItem;
class SwPageFootnoteInfoItem;
class SwPtrItem;
class SwUINumRuleItem;

#define FN_FILE                 SID_SW_START
//#define FN_EDIT                 (SID_SW_START +  100) defined in svxids.hrc
#define FN_VIEW                 (SID_SW_START +  200) // 20200
//#define FN_INSERT               (SID_SW_START +  300) defined in svxids.hrc
//#define FN_FORMAT               (SID_SW_START +  400) defined in svxids.hrc
//#define FN_EXTRA                (SID_SW_START +  600) defined in svxids.hrc
//#define FN_SELECTION            (SID_SW_START +  900) defined in svxids.hrc
#define FN_QUERY                (SID_SW_START + 1000)
#define FN_ENVELP               (SID_SW_START + 1050)
#define FN_PARAM                (SID_SW_START + 1100)
#define FN_STAT                 (SID_SW_START + 1180)
//#define FN_PGPREVIEW            (SID_SW_START + 1250) defined in svxids.hrc
#define FN_FRAME                (SID_SW_START + 1300)
#define FN_INSERT2              (SID_SW_START + 1400)
#define FN_FORMAT2              (SID_SW_START + 1600)
//#define FN_EDIT2                (SID_SW_START + 1800) defined in svxids.hrc
#define FN_QUERY2               (SID_SW_START + 2000)
#define FN_EXTRA2               (SID_SW_START + 2200)
#define FN_PARAM2               (SID_SW_START + 2400)
#define FN_NOTES                (SID_SW_START + 2500)
#define FN_SIDEBAR              (SID_SW_START + 2550)
#define FN_HEADERFOOTER         (SID_SW_START + 2600)
#define FN_PAGEBREAK            (SID_SW_START + 2610)

 /* More accurately, this range should be from FN_EXTRA2 to FN_PARAM2-1, but
 * FN_NUMBER_NEWSTART comes from FN_FORMAT2, and FN_PARAM_LINK_DISPLAY_NAME
 * comes from FN_PARAM2 so we need to include FORMAT2,
 * EDIT2 and QUERY2 and PARAM2 in the range...hopefully this will be fixed
 * soon */

#define FN_UNO_RANGE_BEGIN      FN_FORMAT2
#define FN_UNO_RANGE_END        (FN_PARAM2 + 199)

// Region: File
#define FN_NEW_GLOBAL_DOC       (FN_FILE + 4 )    /* Create Global Document */
#define FN_OPEN_FILE            (FN_FILE + 7 )    /* Open */

#define FN_OUTLINE_TO_IMPRESS   (FN_FILE + 36)  /* Send outline to impress */
#define FN_OUTLINE_TO_CLIPBOARD (FN_FILE + 37)  /* Send outline to clipboard */

#define FN_NEW_HTML_DOC         (FN_FILE + 40 ) /* Create HTML document */

// Region: Edit
#define FN_EDIT_FIELD           (FN_EDIT + 4 )    /* Edit field */
#define FN_EDIT_LINK_DLG        (FN_EDIT + 9 )    /* Edit link dialog */

#define FN_NUMBER_BULLETS       (FN_EDIT + 21)    /* Bullets */
#define FN_UPDATE_SEL_FIELD     (FN_EDIT + 22 )   /* Update selected field */
#define FN_EDIT_IDX_ENTRY_DLG   (FN_EDIT + 23)    /* Edit Index-Entry */
#define FN_UPDATE_FIELDS        (FN_EDIT + 26)    /* Update fields */
#define FN_EXECUTE_MACROFIELD   (FN_EDIT + 27)    /* Execute macrofield */
#define FN_EDIT_FORMULA         TypedWhichId<SfxStringItem>(FN_EDIT + 28)    /* Edit formula in RibbonBar */
#define FN_CALC_TABLE           (FN_EDIT + 29)

// Region: Bullets
#define FN_NUM_BULLET_DOWN      (FN_EDIT + 30)  /* Bullet down */
#define FN_NUM_BULLET_UP        (FN_EDIT + 31)  /* Bullet up */
#define FN_NUM_BULLET_PREV      (FN_EDIT + 32)  /* to previous bullet */
#define FN_NUM_BULLET_NEXT      (FN_EDIT + 33)  /* to next bullet */
#define FN_NUM_BULLET_MOVEUP    (FN_EDIT + 34)  /* move bullet up */
#define FN_NUM_BULLET_MOVEDOWN  (FN_EDIT + 35)  /* move bullet down */
#define FN_NUM_BULLET_NONUM     (FN_EDIT + 36)  /* bullet without number */
#ifndef FN_NUM_BULLET_OFF //in SVX already
#define FN_NUM_BULLET_OFF       (FN_EDIT + 37)  /* turn off numbering */
#endif

#define FN_NUM_BULLET_OUTLINE_DOWN      (FN_EDIT + 39)  /* reduce in rank with sub points */
#define FN_NUM_BULLET_OUTLINE_UP        (FN_EDIT + 40)  /* increase in rank with sub-items */
#define FN_NUM_BULLET_OUTLINE_MOVEUP    (FN_EDIT + 41)  /* Push up with sub-items */
#define FN_NUM_BULLET_OUTLINE_MOVEDOWN  (FN_EDIT + 42)  /* Push down with sub-items */
#define FN_UPDATE_INPUTFIELDS           (FN_EDIT + 43)  /* Update input fields */

#define FN_NUM_OR_NONUM         (FN_EDIT + 46)  /* Number on/off */

#define FN_GOTO_NEXT_INPUTFLD   (FN_EDIT + 47)  /* go to next inputfield */
#define FN_GOTO_PREV_INPUTFLD   (FN_EDIT + 48)  /* go to previous inputfield    */
#define FN_GOTO_MARK            (FN_EDIT + 49)  /* go to bookmark by name */

#define FN_REPEAT_SEARCH        (FN_EDIT + 50)  /* Search again */

#define FN_REPAGINATE           (FN_EDIT + 61)  /* Force repagination */
#define FN_EDIT_FOOTNOTE        (FN_EDIT + 62)  /* Edit footnote */

#define FN_EDIT_REGION          (FN_EDIT + 65)  /* Edit region */
#define FN_GOTO_REFERENCE       (FN_EDIT + 66)  /* From Refmark to the reference */
#define FN_EDIT_CURRENT_REGION  (FN_EDIT + 67)  /* Edit current region*/

// already defined in SVX
#define FN_NEXT_BOOKMARK        (FN_EDIT + 68)  /*  */
#define FN_PREV_BOOKMARK        (FN_EDIT + 69)  /*  */

#define FN_PAGENUMBER                (FN_EDIT + 80)  /**/

#define FN_SELECT_PARA               (FN_EDIT + 97) /* select paragraph*/

#define FN_UPDATE_ALL_LINKS          (FN_EDIT2 + 24) /* update all links */

#define FN_REDLINE_SHOW              (FN_EDIT2 + 26) /* show Redlining */
#define FN_REDLINE_COMMENT           (FN_EDIT2 + 27) /* comment Redlining */

#define FN_UPDATE_ALL                (FN_EDIT2 + 28) /* FN_UPDATE_ALL_LINKS,
                                                        FN_UPDATE_FIELDS,
                                                        FN_UPDATE_TOX,
                                                        CalcLayout */

#define FN_REDLINE_ACCEPT            (FN_EDIT2 + 29) /* Redlining Accept/Reject */
#define FN_ATTR_COLUMNS              (FN_EDIT2 + 31) /* SlotId for SwFormatCol */
#define FN_EDIT_CURRENT_TOX          (FN_EDIT2 + 32) /* edit current index */
#define FN_EDIT_AUTH_ENTRY_DLG       (FN_EDIT2 + 33) /* edit authorities entry*/
#define FN_UPDATE_CHARTS             (FN_EDIT2 + 34) /* update all charts */
// free
#define FN_NUM_CONTINUE              (FN_EDIT2 + 36)    /* continue previous numbering */
#define FN_REDLINE_ACCEPT_DIRECT     (FN_EDIT2 + 37)    /* accept redline at current position*/
#define FN_REDLINE_REJECT_DIRECT     (FN_EDIT2 + 38)    /* reject redline at current position*/
// free
// free
#define FN_REDLINE_NEXT_CHANGE       (FN_EDIT2 + 41)    /* Go to the next change */
#define FN_REDLINE_PREV_CHANGE       (FN_EDIT2 + 42)    /* Go to the previous change */
#define FN_REDLINE_ACCEPT_ALL        (FN_EDIT2 + 43)    /* Redlining Accept All*/
#define FN_REDLINE_REJECT_ALL        (FN_EDIT2 + 44)    /* Redlining Reject All*/
#define FN_REDLINE_ACCEPT_TONEXT     (FN_EDIT2 + 45)    /* Redlining Accept and jump to next*/
#define FN_REDLINE_REJECT_TONEXT     (FN_EDIT2 + 46)    /* Redlining Reject and jump to next*/

// Region: View
#define FN_DRAW_WRAP_DLG        TypedWhichId<SfxInt16Item>(FN_VIEW + 3)   /* Draw wrapping dlg */
#define FN_RULER                (FN_VIEW + 11)  /* Horizontal ruler */

#define FN_VIEW_GRAPHIC         (FN_VIEW + 13)  /* View graphic */
#define FN_VIEW_BOUNDS          (FN_VIEW + 14)  /* View bounds */
#define FN_VIEW_FIELDS          (FN_VIEW + 15)  /* View fields */
#define FN_VLINEAL              (FN_VIEW + 16)  /* Vertical Liner */
#define FN_VSCROLLBAR           (FN_VIEW + 17)  /* Vertical Scrollbar */
#define FN_HSCROLLBAR           (FN_VIEW + 18)  /* Horizontal Scrollbar */
#define FN_VIEW_SECTION_BOUNDARIES (FN_VIEW + 19)  /* View section boundaries */

#define FN_VIEW_META_CHARS      (FN_VIEW + 24)  /* View meta chars */
#define FN_VIEW_MARKS           (FN_VIEW + 25)  /* View marks */
#define FN_VIEW_FIELDNAME       (FN_VIEW + 26)  /* View field names */
#define FN_VIEW_TABLEGRID       (FN_VIEW + 27)  /* View tablegrid */

#define FN_HIGHLIGHT_CHAR_DF    (FN_VIEW + 28)

#define FN_SET_PAGE             (FN_VIEW + 29)  /* Set page template to paragraph */

#define FN_PRINT_LAYOUT         (FN_VIEW + 37)  /* print layout */

#define FN_SCROLL_NAVIGATION    (FN_VIEW + 40)  /* Navigation Controller */

#define FN_VIEW_HIDDEN_PARA     (FN_VIEW + 42)  /* View hidden paragraphs */
#define FN_VIEW_SMOOTH_SCROLL   (FN_VIEW + 43)

#define FN_PREVIEW_ZOOM         TypedWhichId<SfxUInt16Item>(FN_VIEW + 51)  /* create table controller for zoom */

#define FN_SET_MODOPT_TBLNUMFMT  (FN_VIEW + 52) /* Number recognition in tables */
#define FN_HSCROLL_METRIC       TypedWhichId<SfxUInt16Item>(FN_VIEW + 53) /* Metric horizontal scrollbar */
#define FN_VSCROLL_METRIC       TypedWhichId<SfxUInt16Item>(FN_VIEW + 54) /* Metric vertical scrollbar */

#define FN_NAV_ELEMENT          TypedWhichId<SfxUInt32Item>(FN_VIEW + 55)  /* Navigate By drop down controller */
#define FN_SCROLL_PREV          (FN_VIEW + 57)  /* uno:ScrollToPrevious */
#define FN_SCROLL_NEXT          (FN_VIEW + 58)  /* uno:ScrollToNext */
#define FN_VIEW_HIDE_WHITESPACE (FN_VIEW + 59)  /* Hide header, footer, and pagebreak */
#define FN_SHOW_INLINETOOLTIPS  (FN_VIEW + 60)  /* Show tooltips for tracked changes */
#define FN_USE_HEADERFOOTERMENU (FN_VIEW + 61)  /* Show advanced header/footer menu */
#define FN_VIEW_SHOW_WHITESPACE (FN_VIEW + 62)  /* Show header, footer, and pagebreak */

#define FN_SHOW_OUTLINECONTENTVISIBILITYBUTTON (FN_VIEW + 63)  /* Show outline content visibility toggle button */
#define FN_TOGGLE_OUTLINE_CONTENT_VISIBILITY   (FN_VIEW + 64)
#define FN_SHOW_CHANGES_IN_MARGIN              (FN_VIEW + 65)  /* Show tracked deletions in margin */
#define FN_SHOW_TRACKED_CHANGES_MENU           (FN_VIEW + 66)  /* Menu for the next layout modes */
#define FN_SET_TRACKED_CHANGES_IN_TEXT         (FN_VIEW + 67)  /* Show tracked deletions and insertions in text */
#define FN_SET_TRACKED_DELETIONS_IN_MARGIN     (FN_VIEW + 68)  /* Show final text (deletions in margin) */
#define FN_SET_TRACKED_INSERTIONS_IN_MARGIN    (FN_VIEW + 69)  /* Show original text (insertions in margin) */
#define FN_OUTLINE_LEVELS_SHOWN                (FN_VIEW + 70)

// Region: Insert
#define FN_INSERT_BOOKMARK      (FN_INSERT + 2 )  /* Bookmark */
// FN_INSERT + 3 is FN_INSERT_BREAK

#define FN_INSERT_BREAK_DLG     (FN_INSERT + 4 )  /* Break */
#define FN_INSERT_COLUMN_BREAK  (FN_INSERT + 5 )  /* Column break */
#define FN_INSERT_FIELD         TypedWhichId<SfxStringItem>(FN_INSERT + 8 )  /* Textcommand */
#define FN_CHANGE_DBFIELD       (FN_INSERT + 9 )  /* Change the database field */

#define FN_INSERT_CAPTION       (FN_INSERT + 10)  /* Caption */
#define FN_INSERT_DBFIELD       TypedWhichId<SfxStringItem>(FN_INSERT + 11)  /* Insert database field - for recording */
#define FN_INSERT_FOOTNOTE_DLG  (FN_INSERT + 12)  /* Footnote Dialog */

#define FN_INSERT_REF_FIELD     (FN_INSERT + 13)  /* Insert Reference Field */

#define FN_INSERT_HYPERLINK     (FN_INSERT + 14)  /* Character dialogue / hyperlink page */

#define FN_INSERT_LINEBREAK     (FN_INSERT + 18)    /* Newline */
#define FN_INSERT_FIELD_DATA_ONLY (FN_INSERT + 19)  /* Field dialog for mail merge*/
#define FN_INSERT_CONTENT_CONTROL (FN_INSERT + 20)  /* Rich text content control */
#define FN_INSERT_CHECKBOX_CONTENT_CONTROL (FN_INSERT + 21)  /* Checkbox content control */
#define FN_INSERT_OBJECT_DLG    (FN_INSERT + 22)    /* Object */
#define FN_INSERT_PAGEBREAK     (FN_INSERT + 23)    /* Page break*/
#define FN_INSERT_DROPDOWN_CONTENT_CONTROL (FN_INSERT + 24)  /* Dropdown content control */
#define FN_CONTENT_CONTROL_PROPERTIES (FN_INSERT + 25)  /* Content control properties */
#define FN_INSERT_PICTURE_CONTENT_CONTROL (FN_INSERT + 26) /* Picture content control */
#define FN_INSERT_DATE_CONTENT_CONTROL (FN_INSERT + 27) /* Date content control */
#define FN_INSERT_PLAIN_TEXT_CONTENT_CONTROL (FN_INSERT + 28) /* Plain text content control */
#define FN_POSTIT               (FN_INSERT + 29)    /* Insert/edit PostIt */
#define FN_INSERT_TABLE         TypedWhichId<SfxStringItem>(FN_INSERT + 30)    /* Insert Table */
#define FN_INSERT_STRING        TypedWhichId<SfxStringItem>(FN_INSERT+31)
#define FN_INSERT_COMBO_BOX_CONTENT_CONTROL (FN_INSERT + 32) /* Combo box content control */
#define FN_INSERT_FRAME_INTERACT (FN_INSERT + 33)   /* Insert interactive frame */
#define FN_INSERT_FRAME         TypedWhichId<SfxUInt16Item>(FN_INSERT + 34)    /* Insert Frame */

#define FN_INSERT_IDX_ENTRY_DLG (FN_INSERT + 35)    /* insert index entry */
#define FN_INSERT_FRAME_INTERACT_NOCOL (FN_INSERT + 36) /*insert interactive non column frame*/
#define FN_UNFLOAT_FRAME TypedWhichId<SfxUInt16Item>(FN_INSERT + 37) /* Unfloat Frame */

#define FN_TOOL_ANCHOR_PAGE     (FN_INSERT + 50)    /* anchor Draw object to page */
#define FN_TOOL_ANCHOR_PARAGRAPH (FN_INSERT + 51)   /* anchor Draw object to paragraph */
#define FN_TOOL_HIERARCHIE      (FN_INSERT + 52)    /* change hierarchy */
#define FN_PGNUMBER_WIZARD      (FN_INSERT + 53)    /* page number wizard */

#define FN_MAILMERGE_WIZARD     (FN_INSERT + 64)    /* mail merge wizard */
#define FN_TOOL_ANCHOR_FRAME    (FN_INSERT + 66)    /* anchor Draw-Object to frame*/
#define FN_QRY_MERGE            (FN_INSERT + 67)    /* insert record (serial letter) */
#define FN_INSERT_SMA           (FN_INSERT + 69)    /* insert StarMath  */

#define FN_MAILMERGE_FIRST_ENTRY (FN_INSERT + 70)   /* mail merge toolbar - go to the first entry */
#define FN_MAILMERGE_PREV_ENTRY (FN_INSERT + 71)    /* mail merge toolbar - go to the previous entry */
#define FN_MAILMERGE_NEXT_ENTRY (FN_INSERT + 72)    /* mail merge toolbar - go to the next entry */
#define FN_MAILMERGE_LAST_ENTRY (FN_INSERT + 73)    /* mail merge toolbar - go to the next entry */
#define FN_MAILMERGE_CURRENT_ENTRY (FN_INSERT + 74) /* mail merge toolbar - show or change the current entry */
#define FN_MAILMERGE_EXCLUDE_ENTRY (FN_INSERT + 75) /* mail merge toolbar - checkbox to exclude the current entry */

#define FN_DRAWTEXT_ATTR_DLG    (FN_INSERT + 76)    /* position DrawText */

#define FN_MAILMERGE_CREATE_DOCUMENTS (FN_INSERT + 77) /* mail merge toolbar - create the merged document */
#define FN_MAILMERGE_SAVE_DOCUMENTS (FN_INSERT + 78) /* mail merge toolbar - save merged documents */
#define FN_MAILMERGE_PRINT_DOCUMENTS (FN_INSERT + 79) /* mail merge toolbar - print merged documents */
#define FN_MAILMERGE_EMAIL_DOCUMENTS (FN_INSERT + 80) /* mail merge toolbar - email merged documents */

#define FN_TOOL_ANCHOR_CHAR     (FN_INSERT + 82)    /* anchor Draw object to character */

#define FN_QRY_INSERT           (FN_INSERT + 83)    /* insert record selection in to text */
#define FN_QRY_MERGE_FIELD      (FN_INSERT + 84)    /* insert record selection into fields */
#define FN_QRY_INSERT_FIELD     (FN_INSERT + 86)    /* insert database field */

#define FN_INSERT_CTRL          (FN_INSERT + 87)    /* toolbar controller insert*/
#define FN_INSERT_OBJ_CTRL      (FN_INSERT + 88)    /* toolbar controller (insert/object) */
#define FN_INSERT_FIELD_CTRL    (FN_INSERT + 89)    /* toolbar controller insert/field commands */

#define FN_INSERT_FLD_DATE      (FN_INSERT + 90)
#define FN_INSERT_FLD_DATE_VAR  (FN_INSERT + 91)
#define FN_INSERT_FLD_TIME      (FN_INSERT + 92)
#define FN_INSERT_FLD_TIME_VAR  (FN_INSERT + 93)
#define FN_INSERT_FLD_PGNUMBER  (FN_INSERT + 94)
#define FN_INSERT_FLD_PGCOUNT   (FN_INSERT + 95)
#define FN_INSERT_FLD_TOPIC     (FN_INSERT + 96)
#define FN_INSERT_FLD_TITLE     (FN_INSERT + 97)
#define FN_INSERT_FLD_AUTHOR    (FN_INSERT + 98)
#define FN_INSERT_FOOTNOTE      (FN_INSERT + 99)

#define FN_SET_REMINDER         (FN_INSERT + 100)

// Region: Paste (Part 2)
#define FN_MAILMERGE_SENDMAIL_CHILDWINDOW (FN_INSERT2 + 5)  /* child window provided by mailmerge */

#define FN_SBA_BRW_UPDATE       (FN_INSERT2 + 7)    /* insert records into fields */
#define FN_SBA_BRW_INSERT       (FN_INSERT2 + 8)    /* insert records into text */
#define FN_SBA_BRW_MERGE        (FN_INSERT2 + 9)    /* serial letter print */
#define FN_JAVAEDIT             TypedWhichId<SfxStringItem>(FN_INSERT2 + 10)   /* edit script field */
#define FN_TOOL_ANCHOR_AT_CHAR  (FN_INSERT2 + 12)   /* anchor object to character*/

#define FN_INSERT_PAGEHEADER    TypedWhichId<SfxStringListItem>(FN_INSERT2 + 13)   /* insert default header */
#define FN_INSERT_PAGEFOOTER    TypedWhichId<SfxStringListItem>(FN_INSERT2 + 14)   /* insert default footer */

#define FN_INSERT_ENDNOTE           (FN_INSERT2 + 18)   /* insert endnote*/
#define FN_INSERT_REGION            TypedWhichId<SfxUInt16Item>(FN_INSERT2 + 19)   /* Insert section */

#define FN_INSERT_MULTI_TOX         (FN_INSERT2 + 20)   /* insert any TOX */
#define FN_INSERT_AUTH_ENTRY_DLG    (FN_INSERT2 + 21)   /* insert entry for table of authorities*/

// MSO legacy form fields
#define FN_INSERT_TEXT_FORMFIELD        (FN_INSERT2 + 22)
#define FN_INSERT_CHECKBOX_FORMFIELD    (FN_INSERT2 + 23)
#define FN_INSERT_DROPDOWN_FORMFIELD    (FN_INSERT2 + 24)

// MSO content controls
#define FN_INSERT_DATE_FORMFIELD        (FN_INSERT2 + 25)

#define FN_PROTECT_FIELDS               (FN_INSERT2 + 26)
#define FN_PROTECT_BOOKMARKS            (FN_INSERT2 + 27)

#define FN_UPDATE_TEXT_FORMFIELDS       (FN_INSERT2 + 28)
#define FN_UPDATE_TEXT_FORMFIELD        (FN_INSERT2 + 29)

// clipboard table content
#define FN_PASTE_NESTED_TABLE       (FN_INSERT2 + 30)  /* instead of the cell-by-cell copy between source and target tables */
#define FN_TABLE_PASTE_ROW_BEFORE   (FN_INSERT2 + 31)  /* paste table as new table rows */
#define FN_TABLE_PASTE_COL_BEFORE   (FN_INSERT2 + 32)  /* paste table as new table columns */

#define FN_EDIT_BOOKMARK    TypedWhichId<SfxStringItem>(FN_INSERT2 + 33 )  /* Bookmark */
#define FN_UPDATE_BOOKMARKS (FN_INSERT2 + 34)
#define FN_UPDATE_SECTIONS (FN_INSERT2 + 35)
#define FN_DELETE_TEXT_FORMFIELDS (FN_INSERT2 + 36)
#define FN_UPDATE_BOOKMARK (FN_INSERT2 + 37)
#define FN_UPDATE_FIELD (FN_INSERT2 + 38)
#define FN_DELETE_BOOKMARKS (FN_INSERT2 + 39)
#define FN_DELETE_FIELDS (FN_INSERT2 + 40)
#define FN_DELETE_SECTIONS (FN_INSERT2 + 41)

// Region: Format
#define FN_AUTOFORMAT_APPLY     (FN_FORMAT + 1 ) /* apply autoformat options */
#define FN_AUTOFORMAT_AUTO      (FN_FORMAT + 2 ) /* apply autoformat during user input */
#define FN_GROW_FONT_SIZE       (FN_FORMAT + 3 ) /* size */
#define FN_SHRINK_FONT_SIZE     (FN_FORMAT + 4 ) /* size */
#define FN_AUTOFORMAT_REDLINE_APPLY (FN_FORMAT + 5 ) /* apply autoformat with Redlining */
#define FN_SET_SUPER_SCRIPT     (FN_FORMAT + 11) /* superscript */
#define FN_SET_SUB_SCRIPT       (FN_FORMAT + 12) /* subscript */
#define FN_SET_SMALL_CAPS       (FN_FORMAT + 13) /* small caps */

#define FN_FORMAT_PAGE_SETTING_DLG  (FN_FORMAT + 42)  /*  */
#define FN_NUM_FORMAT_TABLE_DLG     (FN_FORMAT + 45)  /* number format in table */
#define FN_FORMAT_BORDER_DLG        (FN_FORMAT + 48)  /* */
#define FN_FORMAT_PAGE_COLUMN_DLG   (FN_FORMAT + 49)  /* columns per page */
#define FN_FORMAT_BACKGROUND_DLG    (FN_FORMAT + 50)  /* background */
#define FN_FORMAT_PAGE_DLG          TypedWhichId<SfxStringItem>(FN_FORMAT + 52)  /* page */
#define FN_FORMAT_COLUMN            (FN_FORMAT + 53)  /* columns */
#define FN_FORMAT_DROPCAPS          (FN_FORMAT + 54)  /* initials */
#define FN_FORMAT_FRAME_DLG         TypedWhichId<SfxStringItem>(FN_FORMAT + 56)  /* frame */
#define FN_FORMAT_GRAFIC_DLG        (FN_FORMAT + 58)  /* graphic */
#define FN_FORMAT_TABLE_DLG         TypedWhichId<SfxStringItem>(FN_FORMAT + 60)  /* table */
#define FN_FORMAT_PAGE_AREA_DLG     (FN_FORMAT + 62)  /* area/background */

#define FN_UPDATE_STYLE_BY_EXAMPLE            (FN_FORMAT + 63)  /* update style */
#define FN_FORMAT_FOOTNOTE_DLG                (FN_FORMAT + 68)  /* footnote dialog */
#define FN_FORMAT_CURRENT_FOOTNOTE_DLG        (FN_FORMAT + 64)  /* Contextual footnote dialog */

#define FN_FRAME_NOWRAP             (FN_FORMAT + 72)
#define FN_FRAME_WRAP               (FN_FORMAT + 73)
#define FN_FRAME_WRAPTHRU           (FN_FORMAT + 74)

#define FN_FRAME_ALIGN_HORZ_LEFT    (FN_FORMAT + 75)
#define FN_FRAME_ALIGN_HORZ_RIGHT   (FN_FORMAT + 76)
#define FN_FRAME_ALIGN_HORZ_CENTER  (FN_FORMAT + 77)
#define FN_FRAME_ALIGN_VERT_TOP     (FN_FORMAT + 78)
#define FN_FRAME_ALIGN_VERT_BOTTOM  (FN_FORMAT + 79)
#define FN_FRAME_ALIGN_VERT_CENTER  (FN_FORMAT + 80)

#define FN_SET_FRM_POSITION         (FN_FORMAT + 82)/* frame position -- " -- */

#define FN_SET_PAGE_STYLE           TypedWhichId<SfxStringItem>(FN_FORMAT + 93)  /* apply page style */

#define FN_FORMAT_TITLEPAGE_DLG     (FN_FORMAT + 98)  /* Title Page */

#define FN_TABLE_REP                TypedWhichId<SwPtrItem>(FN_FORMAT + 99)  /* TableRepresentation */
#define FN_CONVERT_TEXT_TABLE       (FN_FORMAT + 100)  /* convert text <-> table */
#define FN_TABLE_INSERT_ROW         (FN_FORMAT + 101)  /* */
#define FN_TABLE_INSERT_COL         (FN_FORMAT + 102)  /* */
#define FN_TABLE_DELETE_ROW         (FN_FORMAT + 103)  /* */
#define FN_TABLE_DELETE_COL         (FN_FORMAT + 104)  /* */
#define FN_TABLE_SPLIT_CELLS        (FN_FORMAT + 105)  /* */
#define FN_TABLE_MERGE_CELLS        (FN_FORMAT + 106)  /* */
#define FN_TABLE_SET_ROW_HEIGHT     (FN_FORMAT + 107)  /* */
#define FN_TABLE_SET_COL_WIDTH      (FN_FORMAT + 108)  /* */
#define FN_TABLE_INSERT_ROW_AFTER   TypedWhichId<SfxUInt16Item>(FN_FORMAT + 109)  /* */
#define FN_OPTIMIZE_TABLE           (FN_FORMAT + 110)  /* ToolBoxItem for optimizing tables */
#define FN_TABLE_INSERT_ROW_BEFORE  (FN_FORMAT + 111)  /* */
#define FN_TABLE_INSERT_COL_BEFORE  (FN_FORMAT + 112)  /* */
#define FN_TABLE_SELECT_ROW         (FN_FORMAT + 113)  /* */
#define FN_TABLE_SELECT_COL         (FN_FORMAT + 114)  /* */
#define FN_TABLE_SELECT_ALL         (FN_FORMAT + 115)  /* */
#define FN_TABLE_INSERT_COL_AFTER   TypedWhichId<SfxUInt16Item>(FN_FORMAT + 116)  /* */
#define FN_TABLE_SET_READ_ONLY_CELLS (FN_FORMAT + 117)  /* protect table cells */
#define FN_TABLE_UNSET_READ_ONLY_CELLS  (FN_FORMAT + 119)  /* undo table cell protection */
#define FN_TABLE_HEADLINE_REPEAT    (FN_FORMAT + 120)   /* also used in SwXTextTable*/
#define FN_TABLE_ADJUST_CELLS       (FN_FORMAT + 121)  /* */

#define FN_FRAME_UP                 (FN_FORMAT + 122)  /* frame by one level up */
#define FN_FRAME_DOWN               (FN_FORMAT + 123)  /* frame by one level down */

#define FN_TABLE_OPTIMAL_HEIGHT     (FN_FORMAT + 128)  /* optimal cell height */
#define FN_TABLE_DELETE_TABLE       (FN_FORMAT + 129)  /* remove current table*/
#define FN_TABLE_SELECT_CELL        (FN_FORMAT + 130)  /* selects the current table cell */
#define FN_CONVERT_TEXT_TO_TABLE    (FN_FORMAT + 131)  /* convert selected text to table */
#define FN_CONVERT_TABLE_TO_TEXT    (FN_FORMAT + 132)  /* convert a table to text */
#define FN_TABLE_SORT_DIALOG        (FN_FORMAT + 133)  /* sorting in tables*/

// Region: Page Template
#define FN_PAGE_STYLE_SET_LR_MARGIN (FN_FORMAT + 130) /* left / right margin */
#define FN_PAGE_STYLE_SET_UL_MARGIN (FN_FORMAT + 131) /* upper / lower margin */
#define FN_PAGE_STYLE_SET_PAPER_SIZE (FN_FORMAT + 135) /* */
#define FN_PAGE_STYLE_SET_PAPER_BIN (FN_FORMAT + 136) /* paper tray */
#define FN_PAGE_STYLE_SET_NUMBER_FORMAT (FN_FORMAT + 137) /* */
#define FN_PAGE_STYLE_SET_COLS      (FN_FORMAT + 138) /* number of columns */

/* these Ids are only required by Help and are replaced by the Ids without _DLG
 * for the dialog */
#define FN_TABLE_INSERT_COL_DLG     (FN_FORMAT + 142)  /* */
#define FN_TABLE_INSERT_ROW_DLG     (FN_FORMAT + 143)  /* */

#define FN_GOTO_NEXT_CELL           (FN_FORMAT + 145)  /* Table: go to next cell */
#define FN_GOTO_PREV_CELL           (FN_FORMAT + 146)  /* Table: go to previous cell */

#define FN_TABLE_HEADLINE_COUNT     (FN_FORMAT + 147) /* number of repeated headlines of tables */

// Region: Ribbon
#define FN_TABLE_UNSET_READ_ONLY    (FN_FORMAT + 159)  /* undo cell protection for table */
// 156..158 are toolbox item ids

#define FN_FRAME_WRAP_IDEAL         (FN_FORMAT + 162)  /* frame ideal wrapping */
#define FN_FRAME_WRAPTHRU_TOGGLE    (FN_FORMAT + 163)  /* toggle frame transparent wrapping */
#define FN_FRAME_WRAPTHRU_TRANSP    (FN_FORMAT + 164)  /* frame transparent wrapping */

#define FN_FRAME_ALIGN_VERT_ROW_TOP     (FN_FORMAT + 165)  /* */
#define FN_FRAME_ALIGN_VERT_ROW_BOTTOM  (FN_FORMAT + 166)  /* */
#define FN_FRAME_ALIGN_VERT_ROW_CENTER  (FN_FORMAT + 167)  /* */

#define FN_FRAME_ALIGN_VERT_CHAR_TOP    (FN_FORMAT + 168)  /* */
#define FN_FRAME_ALIGN_VERT_CHAR_BOTTOM (FN_FORMAT + 169)  /* */
#define FN_FRAME_ALIGN_VERT_CHAR_CENTER (FN_FORMAT + 170)  /* */

#define FN_FRAME_WRAP_LEFT              (FN_FORMAT + 172)  /* */
#define FN_FRAME_WRAP_RIGHT             (FN_FORMAT + 173)  /* */

#define FN_WRAP_ANCHOR_ONLY         (FN_FORMAT + 181)  /* wrap only for first paragraph */

#define FN_TABLE_BALANCE_CELLS      (FN_FORMAT + 182)  /* evenly distribute columns */
#define FN_TABLE_BALANCE_ROWS       (FN_FORMAT + 183)  /* evenly distribute rows */

#define FN_FRAME_WRAP_CONTOUR       (FN_FORMAT + 184)  /* */

// +185..+187 removed in favor of corresponding globally available slot

#define FN_TABLE_SET_VERT_ALIGN     TypedWhichId<SfxUInt16Item>(FN_FORMAT + 188)  /* vertical alignment in Writer table cells */
#define FN_TABLE_MODE_FIX           (FN_FORMAT + 189)  /* table mode */
#define FN_TABLE_MODE_FIX_PROP      (FN_FORMAT + 190)  /*  -"-  */
#define FN_TABLE_MODE_VARIABLE      (FN_FORMAT + 191)  /*  -"-  */
#define FN_TABLE_BOX_TEXTORIENTATION TypedWhichId<SvxFrameDirectionItem>(FN_FORMAT + 192)  /* text orientation of table cells */
#define SID_ATTR_TABLE_ROW_HEIGHT    TypedWhichId<SfxUInt32Item>(FN_FORMAT + 193)
#define SID_ATTR_TABLE_COLUMN_WIDTH  TypedWhichId<SfxUInt32Item>(FN_FORMAT2 + 194)

#define FN_TABLE_AUTOSUM            (FN_FORMAT + 195)  /* */

#define FN_GOTO_NEXT_REGION          (FN_FORMAT2 + 9)   /* */
#define FN_GOTO_PREV_REGION          (FN_FORMAT2 + 10)  /* */

#define FN_ABSTRACT_NEWDOC           (FN_FORMAT2 + 12)  /* abstract in new document */
#define FN_ABSTRACT_STARIMPRESS      (FN_FORMAT2 + 13)  /* abstract to StarImpress */

#define FN_NAME_SHAPE                (FN_FORMAT2 + 14)  /* Name shapes */

#define FN_TITLE_DESCRIPTION_SHAPE   (FN_FORMAT2 + 15)  /* shape title and description */

#define FN_NUMBER_FORMAT                    TypedWhichId<SfxStringItem>(FN_FORMAT2 + 120) /* set Boxes/NumberFormatter */
#define FN_NUMBER_STANDARD                  (FN_FORMAT2 + 121)
#define FN_NUMBER_TWODEC                    (FN_FORMAT2 + 123)
#define FN_NUMBER_SCIENTIFIC                (FN_FORMAT2 + 124)
#define FN_NUMBER_DATE                      (FN_FORMAT2 + 125)
#define FN_NUMBER_TIME                      (FN_FORMAT2 + 126)
#define FN_NUMBER_CURRENCY                  (FN_FORMAT2 + 127)
#define FN_NUMBER_PERCENT                   (FN_FORMAT2 + 128)

#define FN_FRAME_CHAIN                      (FN_FORMAT2 + 136)
#define FN_FRAME_UNCHAIN                    (FN_FORMAT2 + 137)

#define FN_NUMBER_NEWSTART                  (FN_FORMAT2 + 138)
#define FN_NUMBER_NEWSTART_AT               TypedWhichId<SfxUInt16Item>(FN_FORMAT2 + 139)

#define FN_FRAME_MIRROR_ON_EVEN_PAGES       (FN_FORMAT2 + 140)
#define FN_GRAPHIC_MIRROR_ON_EVEN_PAGES     (FN_FORMAT2 + 141)

#define FN_TABLE_SPLIT_TABLE                (FN_FORMAT2 + 142)
#define FN_SYNC_LABELS                      (FN_FORMAT2 + 143)

#define FN_FORMAT_LINENUMBER                (FN_FORMAT2 + 144)

#define FN_TABLE_RELATIVE_WIDTH             (FN_FORMAT2 + 147)  /* UNO */
#define FN_TABLE_WIDTH                      (FN_FORMAT2 + 148)  /* UNO */
#define FN_TABLE_IS_RELATIVE_WIDTH          (FN_FORMAT2 + 149)  /* UNO */

#define FN_INC_INDENT_OFFSET                (FN_FORMAT2 + 150)
#define FN_DEC_INDENT_OFFSET                (FN_FORMAT2 + 151)

#define FN_TABLE_MERGE_TABLE                (FN_FORMAT2 + 152)
#define FN_TABLE_ROW_SPLIT                  (FN_FORMAT2 + 153)

#define FN_FORMAT_APPLY_HEAD1               (FN_FORMAT2 + 154)
#define FN_FORMAT_APPLY_HEAD2               (FN_FORMAT2 + 155)
#define FN_FORMAT_APPLY_HEAD3               (FN_FORMAT2 + 156)
#define FN_FORMAT_APPLY_DEFAULT             (FN_FORMAT2 + 157)
#define FN_FORMAT_APPLY_TEXTBODY            (FN_FORMAT2 + 158)
#define FN_REMOVE_DIRECT_CHAR_FORMATS       (FN_FORMAT2 + 159)
#define SID_ATTR_TABLE_ALIGNMENT            (FN_FORMAT2 + 160)
#define SID_ATTR_TABLE_LEFT_SPACE           (FN_FORMAT2 + 161)
#define SID_ATTR_TABLE_RIGHT_SPACE          (FN_FORMAT2 + 162)
//free (163 except 194 already used above)

// Region: Extras
#define FN_LINE_NUMBERING_DLG       (FN_EXTRA + 2 )   /* */
#define FN_HYPHENATE_OPT_DLG        (FN_EXTRA + 5 )   /* */
#define FN_ADD_UNKNOWN              (FN_EXTRA + 6 )   /* learn words */
#define FN_NUMBERING_OUTLINE_DLG    (FN_EXTRA + 12)   /* */
#define FN_SORTING_DLG              (FN_EXTRA + 14)   /* */
#define FN_CALCULATE                (FN_EXTRA + 15)   /* */
#define FN_GLOSSARY_DLG             (FN_EXTRA + 20)   /* text building blocks */

#define FN_EXPAND_GLOSSARY          (FN_EXTRA + 28)   /* expand text building blocks */
#define FN_CHANGE_PAGENUM           TypedWhichId<SfxUInt16Item>(FN_EXTRA + 34)   /* change page numbers*/
#define FN_ADD_TEXT_BOX             (FN_EXTRA + 35)   /* add text box to draw shape */
#define FN_REMOVE_TEXT_BOX          (FN_EXTRA + 36)   /* remove text box of draw shape */
#define FN_OPEN_LOCAL_URL           (FN_EXTRA + 37)   /* open local copy for bibliography */

//  Region: Glossary

#define FN_AUTO_CORRECT             (FN_EXTRA + 49 )  /* Autocorrect from Basic */

#define FN_UPDATE_TOX               (FN_EXTRA + 53)   /* update all indices */
#define FN_UPDATE_CUR_TOX           (FN_EXTRA + 54)   /* update current index */
#define FN_REMOVE_CUR_TOX           (FN_EXTRA + 55)  /* remove the current TOX*/

#define FN_GOTO_PAGE                (FN_EXTRA + 59 )  /* goto page */

#define FN_COLL_TYPE                (FN_EXTRA + 98)   /* type for GlobalDoc-Collection*/
#define FN_COLL_ADD                 (FN_EXTRA + 99)

#define FN_COLL_TITLE               (FN_EXTRA2 + 1)  /*  area name or index title */
#define FN_SHADOWCURSOR             (FN_EXTRA2 + 4)  /* enable/disable Shadow Cursor */

#define FN_UNO_PARA_STYLE           (FN_EXTRA2 + 9)  // starting here are UNI-Ids for the PropertyMap listed
#define FN_UNO_PAGE_STYLE           (FN_EXTRA2 + 10)

#define FN_UNO_PARA_NUM_AUTO_FORMAT (FN_EXTRA2 + 11)
#define FN_UNO_FRAME_STYLE_NAME     (FN_EXTRA2 + 12)
#define FN_UNO_NUM_START_VALUE      (FN_EXTRA2 + 13)
#define FN_UNO_NUM_LEVEL            (FN_EXTRA2 + 14)
#define FN_UNO_NUM_RULES            (FN_EXTRA2 + 15)
#define FN_UNO_DOCUMENT_INDEX_MARK  (FN_EXTRA2 + 16)
#define FN_UNO_DOCUMENT_INDEX       (FN_EXTRA2 + 17)
#define FN_UNO_TEXT_FIELD           (FN_EXTRA2 + 18)
#define FN_UNO_TEXT_TABLE           (FN_EXTRA2 + 19)
#define FN_UNO_CELL                 (FN_EXTRA2 + 20)
#define FN_UNO_TEXT_FRAME           (FN_EXTRA2 + 21)
#define FN_UNO_REFERENCE_MARK       (FN_EXTRA2 + 22)
#define FN_UNO_TEXT_SECTION         (FN_EXTRA2 + 23)
#define FN_UNO_FOOTNOTE             (FN_EXTRA2 + 24)
#define FN_UNO_ENDNOTE              (FN_EXTRA2 + 25)
#define FN_UNO_RANGE_COL_LABEL      (FN_EXTRA2 + 26)
#define FN_UNO_RANGE_ROW_LABEL      (FN_EXTRA2 + 27)
#define FN_UNO_TABLE_BORDER         (FN_EXTRA2 + 29)
#define FN_UNO_TABLE_COLUMN_SEPARATORS      (FN_EXTRA2 + 30)
#define FN_UNO_TABLE_COLUMN_RELATIVE_SUM    (FN_EXTRA2 + 31)
#define FN_UNO_TABLE_CELL_BACKGROUND        (FN_EXTRA2 + 32)
#define FN_UNO_ROW_HEIGHT                   (FN_EXTRA2 + 33)
#define FN_UNO_ROW_AUTO_HEIGHT              (FN_EXTRA2 + 34)
#define FN_UNO_HEADER                       (FN_EXTRA2 + 35)
#define FN_UNO_HEADER_LEFT                  (FN_EXTRA2 + 36)
#define FN_UNO_HEADER_RIGHT                 (FN_EXTRA2 + 37)
#define FN_UNO_FOOTER                       (FN_EXTRA2 + 38)
#define FN_UNO_FOOTER_LEFT                  (FN_EXTRA2 + 39)
#define FN_UNO_FOOTER_RIGHT                 (FN_EXTRA2 + 40)
#define FN_UNO_TEXT_PARAGRAPH               (FN_EXTRA2 + 41)
#define FN_UNO_PARENT_TEXT                  (FN_EXTRA2 + 42)
#define FN_UNO_SORTED_TEXT_ID               (FN_EXTRA2 + 43)
#define FN_UNO_FOLLOW_STYLE                 (FN_EXTRA2 + 59)

#define FN_API_CALL                         TypedWhichId<SfxBoolItem>(FN_EXTRA2 + 60)

#define FN_UNO_IS_PHYSICAL                  (FN_EXTRA2 + 61)
#define FN_UNO_IS_AUTO_UPDATE               (FN_EXTRA2 + 62)
#define FN_UNO_DISPLAY_NAME                 (FN_EXTRA2 + 63)

#define FN_UNO_WRAP                         (FN_EXTRA2 + 64)
#define FN_UNO_ANCHOR_TYPES                 (FN_EXTRA2 + 65)
#define FN_UNO_PARA_CHAPTER_NUMBERING_LEVEL (FN_EXTRA2 + 66)
#define FN_UNO_PARA_CONDITIONAL_STYLE_NAME  (FN_EXTRA2 + 67)

#define FN_UNO_CATEGORY                     (FN_EXTRA2 + 68)
#define FN_UNO_IS_NUMBER                    (FN_EXTRA2 + 69)
#define FN_UNO_TEXT_WRAP                    (FN_EXTRA2 + 70)
#define FN_UNO_ANCHOR_TYPE                  (FN_EXTRA2 + 71)
#define FN_SKIP_HIDDEN_TEXT                 (FN_EXTRA2 + 72)
#define FN_SKIP_PROTECTED_TEXT              (FN_EXTRA2 + 73)
#define FN_UNO_Z_ORDER                      (FN_EXTRA2 + 74)
#define FN_UNO_REDLINE_NODE_START           (FN_EXTRA2 + 75)
#define FN_UNO_REDLINE_NODE_END             (FN_EXTRA2 + 76)
#define FN_UNO_TEXT_PORTION_TYPE            (FN_EXTRA2 + 77)
#define FN_UNO_CONTROL_CHARACTER            (FN_EXTRA2 + 78)
#define FN_UNO_BOOKMARK                     (FN_EXTRA2 + 79)
#define FN_UNO_IS_COLLAPSED                 (FN_EXTRA2 + 80)
#define FN_UNO_IS_START                     (FN_EXTRA2 + 81)
#define FN_UNO_IS_AUTOMATIC_CONTOUR         (FN_EXTRA2 + 82)
#define FN_UNO_IS_PIXEL_CONTOUR             (FN_EXTRA2 + 83)
#define FN_UNO_ALTERNATIVE_TEXT             (FN_EXTRA2 + 84)
#define FN_UNO_ACTUAL_SIZE                  (FN_EXTRA2 + 85)
#define FN_UNO_GRAPHIC_URL                  (FN_EXTRA2 + 86)
#define FN_UNO_GRAPHIC_FILTER               (FN_EXTRA2 + 87)
#define FN_UNO_CELL_NAME                    (FN_EXTRA2 + 88)
#define FN_INSERT_GLOSSARY                  TypedWhichId<SfxStringItem>(FN_EXTRA2 + 89)
#define FN_NEW_GLOSSARY                     TypedWhichId<SfxStringItem>(FN_EXTRA2 + 90)
#define FN_SET_ACT_GLOSSARY                 TypedWhichId<SfxStringItem>(FN_EXTRA2 + 91)
#define FN_UNO_LINK_STYLE (FN_EXTRA2 + 92)

#define FN_UNO_CHARFMT_SEQUENCE             (FN_EXTRA2 + 94)
#define FN_UNO_CLSID                        (FN_EXTRA2 + 95)
#define FN_UNO_MODEL                        (FN_EXTRA2 + 96)
#define FN_UNO_COMPONENT                    (FN_EXTRA2 + 97)
#define FN_WORDCOUNT_DIALOG                 (FN_EXTRA2 + 98)

#define FN_XFORMS_DESIGN_MODE               (FN_EXTRA2 + 99)

#define FN_UNO_PARA_STYLE_CONDITIONS        (FN_EXTRA2 + 100)
#define FN_UNO_GRAPHIC                      (FN_EXTRA2 + 101)

#define FN_UNO_REPLACEMENT_GRAPHIC_URL      (FN_EXTRA2 + 102)
#define FN_UNO_CELL_ROW_SPAN                (FN_EXTRA2 + 103)
#define FN_UNO_TABLE_BORDER_DISTANCES       (FN_EXTRA2 + 104)
#define FN_SPELL_GRAMMAR_DIALOG             (FN_EXTRA2 + 105)
#define FN_UNO_STREAM_NAME                  (FN_EXTRA2 + 106)
#define FN_UNO_LIST_ID                      (FN_EXTRA2 + 107)
#define FN_UNO_REPLACEMENT_GRAPHIC          (FN_EXTRA2 + 108)

#define FN_UNO_PARA_CONT_PREV_SUBTREE       (FN_EXTRA2 + 109)
#define FN_UNO_PARA_NUM_STRING              (FN_EXTRA2 + 110)
#define FN_UNO_TABLE_NAME                   (FN_EXTRA2 + 111)
#define FN_UNO_META                         (FN_EXTRA2 + 112)
#define FN_UNO_NESTED_TEXT_CONTENT          (FN_EXTRA2 + 113)

/* Navigation buttons */
#define FN_NAVIGATION_BACK                  (FN_EXTRA2 + 114)
#define FN_NAVIGATION_FORWARD               (FN_EXTRA2 + 115)

// #i972: bool items to be passed to SwFramePage for evaluation
#define FN_OLE_IS_MATH                      TypedWhichId<SfxBoolItem>(FN_EXTRA2 + 116)
#define FN_MATH_BASELINE_ALIGNMENT          TypedWhichId<SfxBoolItem>(FN_EXTRA2 + 117)

#define FN_EMBEDDED_OBJECT                  (FN_EXTRA2 + 118)
#define FN_UNO_DRAW_ASPECT                  (FN_EXTRA2 + 119)
#define FN_UNO_HEADER_FIRST                 (FN_EXTRA2 + 120)
#define FN_UNO_FOOTER_FIRST                 (FN_EXTRA2 + 121)
#define FN_UNO_TABLE_BORDER2                (FN_EXTRA2 + 122)

#define FN_FIELD_DIALOG_DOC_PROPS           TypedWhichId<SfxUnoAnyItem>(FN_EXTRA2 + 123)
#define FN_UNO_HIDDEN                       (FN_EXTRA2 + 124)
#define FN_UNO_STYLE_INTEROP_GRAB_BAG       (FN_EXTRA2 + 125)
#define FN_UNO_TABLE_TEMPLATE_NAME          (FN_EXTRA2 + 126)
#define FN_UNO_VISIBLE_AREA_WIDTH           (FN_EXTRA2 + 127)
#define FN_UNO_VISIBLE_AREA_HEIGHT          (FN_EXTRA2 + 128)

#define FN_UNO_TRANSFORMED_GRAPHIC          (FN_EXTRA2 + 129)
#define FN_UNO_GRAPHIC_PREVIEW              (FN_EXTRA2 + 130)
#define FN_UNO_LINEBREAK (FN_EXTRA2 + 131)
#define FN_UNO_CONTENT_CONTROL (FN_EXTRA2 + 132)

// Area: Help
// Region: Traveling & Selection
#define FN_CHAR_LEFT                (FN_SELECTION + 1 ) /* */
#define FN_CHAR_RIGHT               (FN_SELECTION + 2 ) /* */
#define FN_LINE_UP                  (FN_SELECTION + 3 ) /* */
#define FN_LINE_DOWN                (FN_SELECTION + 4 ) /* */
#define FN_START_OF_LINE            (FN_SELECTION + 5 ) /* StartOfLine */
#define FN_END_OF_LINE              (FN_SELECTION + 6 ) /* EndOfLine */
#define FN_START_OF_DOCUMENT        (FN_SELECTION + 7 ) /* StartOfDocument */
#define FN_START_OF_NEXT_PAGE       (FN_SELECTION + 9 ) /* StartOfNextPage ??? */
#define FN_END_OF_NEXT_PAGE         (FN_SELECTION + 10) /* ??? */
#define FN_START_OF_PREV_PAGE       (FN_SELECTION + 11) /* StartOfPrevPage ??? */
#define FN_END_OF_PREV_PAGE         (FN_SELECTION + 12) /* ??? */
#define FN_START_OF_PAGE            (FN_SELECTION + 13) /* StartOfPage */
#define FN_END_OF_PAGE              (FN_SELECTION + 14) /* EndOfPage */
#define FN_START_OF_COLUMN          (FN_SELECTION + 17) /* StartOfColumn */
#define FN_END_OF_COLUMN            (FN_SELECTION + 18) /* EndOfColumn */
#define FN_START_OF_PARA            (FN_SELECTION + 19) /* StartOfPara */
#define FN_END_OF_PARA              (FN_SELECTION + 20) /* EndOfPara */
#define FN_NEXT_WORD                (FN_SELECTION + 21) /* NextWord */
#define FN_PREV_WORD                (FN_SELECTION + 22) /* PrevWord */
#define FN_NEXT_SENT                (FN_SELECTION + 23) /* NextSentence */
#define FN_PREV_SENT                (FN_SELECTION + 24) /* PrevSentence */
#define FN_DELETE                   (FN_SELECTION + 25) /* Delete */
#define FN_BACKSPACE                (FN_SELECTION + 26) /* Backspace */
#define FN_DELETE_SENT              (FN_SELECTION + 27) /* DeleteToEndOfSentence */
#define FN_DELETE_BACK_SENT         (FN_SELECTION + 28) /* DeleteToStartOfSentence */
#define FN_DELETE_WORD              (FN_SELECTION + 29) /* DeleteToEndOfWord */
#define FN_DELETE_BACK_WORD         (FN_SELECTION + 30) /* DeleteToStartOfWord */
#define FN_DELETE_LINE              (FN_SELECTION + 31) /* DeleteToEndOfLine */
#define FN_DELETE_BACK_LINE         (FN_SELECTION + 32) /* DeleteToStartOfLine */
#define FN_DELETE_PARA              (FN_SELECTION + 33) /* DeleteToEndOfPara */
#define FN_DELETE_BACK_PARA         (FN_SELECTION + 34) /* DeleteToStartOfPara */
#define FN_DELETE_WHOLE_LINE        (FN_SELECTION + 35) /* DeleteLine ??? */
#define FN_PAGEUP                   (FN_SELECTION + 37) /* PageUpWithCursor */
#define FN_PAGEDOWN                 (FN_SELECTION + 38) /* PageDownWithCursor */
#define FN_SET_ADD_MODE             (FN_SELECTION + 39) /* Selectionmode */
#define FN_SET_EXT_MODE             (FN_SELECTION + 40) /* Selectionmode */
#define FN_ESCAPE                   (FN_SELECTION + 41) /* Normal */
#define FN_SHIFT_BACKSPACE          (FN_SELECTION + 42) /* like Backspace */
#define FN_SELECT_WORD              (FN_SELECTION + 43) /* select word*/

#define FN_GOTO_NEXT_OBJ            (FN_SELECTION + 44)   /* Goto next object */
#define FN_GOTO_PREV_OBJ            (FN_SELECTION + 45)   /* Go to previous object */

#define FN_START_TABLE              (FN_SELECTION + 47) /* to the beginning of the table */
#define FN_END_TABLE                (FN_SELECTION + 48) /* to the end of the table */
#define FN_NEXT_TABLE               (FN_SELECTION + 49) /* to the next table */
#define FN_PREV_TABLE               (FN_SELECTION + 50) /* to the previous table */

#define FN_START_OF_NEXT_COLUMN     (FN_SELECTION + 51) /* to the start of the next column */
#define FN_END_OF_NEXT_COLUMN       (FN_SELECTION + 52) /* to the end of the next column */
#define FN_START_OF_PREV_COLUMN     (FN_SELECTION + 53) /* to the start of the previous column */
#define FN_END_OF_PREV_COLUMN       (FN_SELECTION + 54) /* to the end of the previous column */

#define FN_FOOTNOTE_TO_ANCHOR       (FN_SELECTION + 55) /* from the footnote to the anchor */
#define FN_NEXT_FOOTNOTE            (FN_SELECTION + 56) /* to the next footnote */
#define FN_PREV_FOOTNOTE            (FN_SELECTION + 57) /* to the previous footnote */

#define FN_CNTNT_TO_NEXT_FRAME      (FN_SELECTION + 58) /* from content to the next frame */
#define FN_FRAME_TO_ANCHOR          (FN_SELECTION + 59) /* from frame to the anchor */

#define FN_TO_HEADER                (FN_SELECTION + 60) /* toggle between content and header */
#define FN_TO_FOOTER                (FN_SELECTION + 61) /* toggle between content and footer */

#define FN_IDX_MARK_TO_IDX          (FN_SELECTION + 62) /* from the index mark to the index */

#define FN_TO_FOOTNOTE_AREA         (FN_SELECTION + 63) /* jump into the appropriate footnote area */

#define FN_SET_BLOCK_MODE           (FN_SELECTION + 64) /* selection mode: block */

#define FN_PREV_PARA                (FN_SELECTION + 74)
#define FN_NEXT_PARA                (FN_SELECTION + 75)

#define FN_GOTO_NEXT_MARK           (FN_SELECTION + 76)   /* Go to next marker */
#define FN_GOTO_PREV_MARK           (FN_SELECTION + 77)   /* Go to previous marker */

#define FN_NEXT_TOXMARK             (FN_SELECTION + 83)   /* to the next index mark */
#define FN_PREV_TOXMARK             (FN_SELECTION + 84)   /* to the previous index mark */
#define FN_NEXT_TBLFML              (FN_SELECTION + 85)   /* to the next table formula */
#define FN_PREV_TBLFML              (FN_SELECTION + 86)   /* to the previous table formula */
#define FN_NEXT_TBLFML_ERR          (FN_SELECTION + 87)   /* to the next broken table formula */
#define FN_PREV_TBLFML_ERR          (FN_SELECTION + 88)   /* to the previous broken table formula */

#define FN_READONLY_SELECTION_MODE  (FN_SELECTION + 89)   /* switches text selection mode in readonly documents*/
#define FN_SELECTION_MODE_DEFAULT   (FN_SELECTION + 90)   /* use default selection mode - not block mode */
#define FN_SELECTION_MODE_BLOCK     (FN_SELECTION + 91)   /* switch on block selection */
#define SID_JUMP_TO_SPECIFIC_PAGE    (FN_SELECTION + 92)
#define SID_TRACK_CHANGES_BAR       (FN_SELECTION + 93)   /* shows/hides track changes bar */

#define FN_SELECT_SENTENCE          (FN_SELECTION + 94)   /* select paragraph*/

#define FN_TO_NEXT_PAGE             (FN_SELECTION + 95)
#define FN_TO_PREV_PAGE             (FN_SELECTION + 96)
#define FN_SELECTION_CYCLE          (FN_SELECTION + 97)

// QUERY-Block
#define FN_TXTATR_INET              (FN_QUERY +29)      /* INet-Attribute */

#define FN_GET_PRINT_AREA           (FN_QUERY +32)      /* Get attribute for printable area of page */
#define FN_DB_CONNECTION_ANY            TypedWhichId<SfxUnoAnyItem>(FN_QUERY2 + 3)
#define FN_DB_COLUMN_ANY                TypedWhichId<SfxUnoAnyItem>(FN_QUERY2 + 4)
#define FN_DB_DATA_SOURCE_ANY           TypedWhichId<SfxUnoAnyItem>(FN_QUERY2 + 5)
#define FN_DB_DATA_COMMAND_ANY          TypedWhichId<SfxUnoAnyItem>(FN_QUERY2 + 6)
#define FN_DB_DATA_COMMAND_TYPE_ANY     TypedWhichId<SfxUnoAnyItem>(FN_QUERY2 + 7)
#define FN_DB_DATA_COLUMN_NAME_ANY      TypedWhichId<SfxUnoAnyItem>(FN_QUERY2 + 8)
#define FN_DB_DATA_SELECTION_ANY        TypedWhichId<SfxUnoAnyItem>(FN_QUERY2 + 9)
#define FN_DB_DATA_CURSOR_ANY           TypedWhichId<SfxUnoAnyItem>(FN_QUERY2 + 10)

// Region: Envelope
#define FN_ENVELOP                  (FN_ENVELP    )

#define FN_DELETE_NOTE_AUTHOR       TypedWhichId<SfxStringItem>(FN_NOTES+1)
#define FN_DELETE_ALL_NOTES         (FN_NOTES+2)
#define FN_HIDE_NOTE                (FN_NOTES+3)
#define FN_HIDE_NOTE_AUTHOR         TypedWhichId<SfxStringItem>(FN_NOTES+4)
#define FN_HIDE_ALL_NOTES           (FN_NOTES+5)

#define FN_DELETE_COMMENT           (FN_NOTES+6)
#define FN_REPLY                    (FN_NOTES+7)
#define FN_FORMAT_ALL_NOTES         (FN_NOTES+8)
#define FN_RESOLVE_NOTE             (FN_NOTES+9)
#define FN_RESOLVE_NOTE_THREAD      (FN_NOTES+10)
#define FN_DELETE_COMMENT_THREAD    (FN_NOTES+11)

// Region: Parameter
#define FN_PARAM_MOVE_COUNT         TypedWhichId<SfxInt32Item>(FN_PARAM+2)
#define FN_PARAM_MOVE_SELECTION     TypedWhichId<SfxBoolItem>(FN_PARAM+3)

#define FN_PARAM_ADDPRINTER         TypedWhichId<SwAddPrinterItem>(FN_PARAM+18)
#define FN_PARAM_DOCDISP            TypedWhichId<SwDocDisplayItem>(FN_PARAM+20)
#define FN_PARAM_ELEM               TypedWhichId<SwElemItem>(FN_PARAM+21)
#define FN_PARAM_SWTEST             TypedWhichId<SwTestItem>(FN_PARAM+22)

#define FN_PARAM_FTN_INFO           TypedWhichId<SwPageFootnoteInfoItem>(FN_PARAM+23)

#define FN_PARAM_REGION_NAME            TypedWhichId<SfxStringItem>(FN_PARAM+24)
#define FN_PARAM_REGION_CONDITION       TypedWhichId<SfxStringItem>(FN_PARAM+25)
#define FN_PARAM_REGION_HIDDEN          TypedWhichId<SfxBoolItem>(FN_PARAM+26)
#define FN_PARAM_REGION_PROTECT         TypedWhichId<SfxBoolItem>(FN_PARAM+27)
#define FN_PARAM_REGION_EDIT_IN_READONLY TypedWhichId<SfxBoolItem>(FN_PARAM+28)

#define FN_PARAM_INSERT_AFTER           TypedWhichId<SfxBoolItem>(FN_PARAM+29)

#define FN_PARAM_TABLE_NAME             TypedWhichId<SfxStringItem>(FN_PARAM+44)
#define FN_PARAM_TABLE_HEADLINE         TypedWhichId<SfxUInt16Item>(FN_PARAM+50)

#define FN_PARAM_GRF_CONNECT            TypedWhichId<SfxBoolItem>(FN_PARAM+53)
#define FN_PARAM_FIELD_TYPE             TypedWhichId<SfxUInt16Item>(FN_PARAM+54)
#define FN_PARAM_FIELD_SUBTYPE          TypedWhichId<SfxUInt16Item>(FN_PARAM+56)
#define FN_PARAM_FIELD_FORMAT           TypedWhichId<SfxUInt32Item>(FN_PARAM+57)

#define FN_PARAM_PRINTER                TypedWhichId<SwPtrItem>(FN_PARAM+78) /* Printer */
#define FN_PARAM_STDFONTS               TypedWhichId<SwPtrItem>(FN_PARAM+79) /* ConfigItem Standardfonts */

#define FN_PARAM_WRTSHELL               TypedWhichId<SwPtrItem>(FN_PARAM2) /* SwWrtShell */

#define FN_COND_COLL                    TypedWhichId<SwCondCollItem>(FN_PARAM2+1) /* Item for conditional templates */
#define FN_PARAM_SELECTION              TypedWhichId<SfxStringItem>(FN_PARAM2+2) /* selected word for format/character/hyperlink */

#define FN_PARAM_ACT_NUMBER             TypedWhichId<SwUINumRuleItem>(FN_PARAM2+3) /* PointerItem the current NumRule */

#define FN_PARAM_NUM_PRESET             TypedWhichId<SfxBoolItem>(FN_PARAM2+5) /* predefined numbering */

#define FN_PARAM_SHADOWCURSOR           TypedWhichId<SwShadowCursorItem>(FN_PARAM2+8) /* for ShadowCursor options */

#define FN_TEXT_RANGE                   (FN_PARAM2+12) /* TextRange Property*/

#define FN_PARAM_CRSR_IN_PROTECTED      TypedWhichId<SfxBoolItem>(FN_PARAM2+13) /* Cursor in protected areas */
#define FN_PARAM_TOX_TYPE               TypedWhichId<SfxUInt16Item>(FN_PARAM2+14) /* TOX type in tox dialog*/
#define FN_PARAM_LINK_DISPLAY_NAME      (FN_PARAM2+15) /* LinkDisplayName property*/
// free
#define FN_PARAM_CONTOUR_PP             (FN_PARAM2+17) /* contour PolyPolygon*/

#define FN_ANCHOR_POSITION              (FN_PARAM2+18) /* AnchorPosition property */
#define FN_DROP_TEXT                    TypedWhichId<SfxStringItem>(FN_PARAM2+18) /* text set in drop caps tab page - for recording */
#define FN_DROP_CHAR_STYLE_NAME         TypedWhichId<SfxStringItem>(FN_PARAM2+19) /* character style of drop caps - for recording */
#define FN_PARAM_CHAIN_PREVIOUS         TypedWhichId<SfxStringItem>(FN_PARAM2+20) /* Name of frame to be added as chain successor */
#define FN_PARAM_CHAIN_NEXT             TypedWhichId<SfxStringItem>(FN_PARAM2+21) /* Name of frame to be added as chain predecessor */
#define FN_PARAM_DATABASE_PROPERTIES    TypedWhichId<SfxUnoAnyItem>(FN_PARAM2+22) /* transport a Sequence<PropertyValue> containing database properties*/
#define FN_SHAPE_TRANSFORMATION_IN_HORI_L2R (FN_PARAM2+23)
#define FN_SHAPE_POSITION_LAYOUT_DIR    (FN_PARAM2+24)
#define FN_SHAPE_STARTPOSITION_IN_HORI_L2R (FN_PARAM2+25)
#define FN_SHAPE_ENDPOSITION_IN_HORI_L2R   (FN_PARAM2+26)
#define FN_PARAM_PAM                    TypedWhichId<SwPaMItem>(FN_PARAM2+27) /* Point and Mark */
#define FN_TEXT_BOX                     (FN_PARAM2+28) /* TextBox Property*/
#define FN_BOOKMARK_HIDDEN              (FN_PARAM2+29) /* Hidden Property of bookmarks*/
#define FN_BOOKMARK_CONDITION           (FN_PARAM2+30) /* Condition Property of bookmarks*/

// Status: not more than 19!
#define FN_STAT_PAGE                TypedWhichId<SfxStringListItem>(FN_STAT + 1)
#define FN_STAT_TEMPLATE            TypedWhichId<SfxStringItem>(FN_STAT + 2)
#define FN_STAT_ZOOM                (FN_STAT + 3)
#define FN_STAT_SELMODE             TypedWhichId<SfxUInt16Item>(FN_STAT + 5)
#define FN_STAT_BOOKMARK            TypedWhichId<SfxUInt16Item>(FN_STAT + 8)  /* For Popup Bookmarks*/
#define FN_STAT_WORDCOUNT           TypedWhichId<SfxStringItem>(FN_STAT + 9)
#define FN_STAT_ACCESSIBILITY_CHECK (FN_STAT + 10)

// Region: Page preview
#define FN_SHOW_TWO_PAGES           (FN_PGPREVIEW + 1)
#ifndef FN_SHOW_MULTIPLE_PAGES
#define FN_SHOW_MULTIPLE_PAGES      (FN_PGPREVIEW + 2)
#endif
#define FN_PRINT_PAGEPREVIEW        (FN_PGPREVIEW + 3)
#define FN_CLOSE_PAGEPREVIEW        (FN_PGPREVIEW + 4)
#define FN_SHOW_BOOKVIEW            (FN_PGPREVIEW + 5)
#define FN_SHOW_SINGLE_PAGE         (FN_PGPREVIEW + 6)

// Region: Setting Attributes
#define FN_OPAQUE                       (FN_FRAME + 1)
#define FN_SET_PROTECT                  (FN_FRAME + 2)
#define FN_SURROUND                     (FN_FRAME + 3)
#define FN_VERT_ORIENT                  (FN_FRAME + 4)
#define FN_HORI_ORIENT                  (FN_FRAME + 5)
#define FN_SET_FRM_NAME                 TypedWhichId<SfxStringItem>(FN_FRAME + 6)
#define FN_KEEP_ASPECT_RATIO            TypedWhichId<SfxBoolItem>(FN_FRAME + 7)

#define FN_SET_FRM_ALT_NAME             TypedWhichId<SfxStringItem>(FN_FRAME + 18)
#define FN_UNO_TITLE                    (FN_FRAME + 19)
#define FN_UNO_DESCRIPTION              TypedWhichId<SfxStringItem>(FN_FRAME + 20)
#define FN_UNO_TOOLTIP                  (FN_FRAME + 21)

#define SID_ATTR_PAGE_COLUMN            TypedWhichId<SfxInt16Item>(FN_SIDEBAR + 0)
#define SID_ATTR_PAGE_HEADER            (FN_SIDEBAR + 3)
#define SID_ATTR_PAGE_HEADER_LRMARGIN   TypedWhichId<SvxLongLRSpaceItem>(FN_SIDEBAR + 4)
#define SID_ATTR_PAGE_HEADER_SPACING    TypedWhichId<SvxLongULSpaceItem>(FN_SIDEBAR + 5)
#define SID_ATTR_PAGE_HEADER_LAYOUT     TypedWhichId<SfxInt16Item>(FN_SIDEBAR + 6)
#define SID_ATTR_PAGE_FOOTER            (FN_SIDEBAR + 7)
#define SID_ATTR_PAGE_FOOTER_LRMARGIN   TypedWhichId<SvxLongLRSpaceItem>(FN_SIDEBAR + 8)
#define SID_ATTR_PAGE_FOOTER_SPACING    TypedWhichId<SvxLongULSpaceItem>(FN_SIDEBAR + 9)
#define SID_ATTR_PAGE_FOOTER_LAYOUT     TypedWhichId<SfxInt16Item>(FN_SIDEBAR + 10)
#define SID_ATTR_PAGE_MARGIN            (FN_SIDEBAR + 11)

//Member IDs for Fill / SetVariable of items
#define         MID_STYLE       0xe0
#define         MID_PWIDTH      0xe1
#define         MID_ADJUST      0xe2
#define         MID_TDIST       0xe3
#define         MID_BDIST       0xe4
#define         MID_LINES       0xe5
#define         MID_CHARS       0xe6
#define         MID_DIST        0xe7

#define         MID_1           0xe8
#define         MID_2           0xe9
#define         MID_3           0xea
#define         MID_4           0xeb
#define         MID_5           0xec
#define         MID_6           0xed
#define         MID_7           0xee
#define         MID_8           0xef
#define         MID_9           0xf0
#define         MID_A           0xf1
#define         MID_B           0xf2
#define         MID_C           0xf3
#define         MID_D           0xf4
#define         MID_E           0xf5
#define         MID_F           0xf6
#define         MID_10          0xf7
#define         MID_11          0xf8
#define         MID_12          0xf9
#define         MID_13          0xfa

#endif // INCLUDED_SW_INC_CMDID_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

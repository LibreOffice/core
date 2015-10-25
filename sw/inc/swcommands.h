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
#ifndef INCLUDED_SW_INC_SWCOMMANDS_H
#define INCLUDED_SW_INC_SWCOMMANDS_H

#define CMD_SID_CREATE_SW_DRAWVIEW                  ".uno:CreateSWDrawView"
#define CMD_FN_ADD_TEXT_BOX                         ".uno:AddTextBox"
#define CMD_FN_FRAME_ALIGN_VERT_BOTTOM              ".uno:AlignBottom"
#define CMD_FN_FRAME_ALIGN_HORZ_CENTER              ".uno:AlignHorizontalCenter"
#define CMD_FN_FRAME_ALIGN_HORZ_LEFT                ".uno:AlignLeft"
#define CMD_FN_FRAME_ALIGN_HORZ_RIGHT               ".uno:AlignRight"
#define CMD_FN_FRAME_ALIGN_VERT_TOP                 ".uno:AlignTop"
#define CMD_FN_FRAME_ALIGN_VERT_CENTER              ".uno:AlignVerticalCenter"
#define CMD_FN_EDIT_AUTH_ENTRY_DLG                  ".uno:AuthoritiesEntryDialog"
#define CMD_SID_TABLE_VERT_BOTTOM                   ".uno:CellVertBottom"
#define CMD_SID_TABLE_VERT_CENTER                   ".uno:CellVertCenter"
#define CMD_SID_TABLE_VERT_NONE                     ".uno:CellVertTop"
#define CMD_FN_NUM_BULLET_DOWN                      ".uno:DecrementLevel"
#define CMD_FN_TABLE_DELETE_COL                     ".uno:DeleteColumns"
#define CMD_FN_TABLE_DELETE_ROW                     ".uno:DeleteRows"
#define CMD_FN_TABLE_BALANCE_CELLS                  ".uno:DistributeColumns"
#define CMD_FN_TABLE_BALANCE_ROWS                   ".uno:DistributeRows"
#define CMD_FN_EDIT_CURRENT_TOX                     ".uno:EditCurIndex"
#define CMD_FN_EDIT_HYPERLINK                       ".uno:EditHyperlink"
#define CMD_FN_TABLE_SELECT_COL                     ".uno:EntireColumn"
#define CMD_FN_TABLE_SELECT_ROW                     ".uno:EntireRow"
#define CMD_FN_EDIT_FIELD                           ".uno:FieldDialog"
#define CMD_FN_EDIT_FOOTNOTE                        ".uno:EditFootnote"
#define CMD_FN_FORMAT_FRAME_DLG                     ".uno:FrameDialog"
#define CMD_FN_FORMAT_GRAFIC_DLG                    ".uno:GraphicDialog"
#define CMD_FN_NUM_BULLET_UP                        ".uno:IncrementLevel"
#define CMD_FN_EDIT_IDX_ENTRY_DLG                   ".uno:IndexEntryDialog"
#define CMD_FN_INSERT_FLD_AUTHOR                    ".uno:InsertAuthorField"
#define CMD_FN_INSERT_CAPTION                       ".uno:InsertCaptionDialog"
#define CMD_FN_TABLE_INSERT_COL_DLG                 ".uno:InsertColumnDialog"
#define CMD_FN_INSERT_FLD_DATE                      ".uno:InsertDateField"
#define CMD_FN_INSERT_FIELD                         ".uno:InsertField"
#define CMD_FN_INSERT_FLD_PGCOUNT                   ".uno:InsertPageCountField"
#define CMD_FN_INSERT_FLD_PGNUMBER                  ".uno:InsertPageNumberField"
#define CMD_FN_TABLE_INSERT_ROW_DLG                 ".uno:InsertRowDialog"
#define CMD_FN_INSERT_FLD_TIME                      ".uno:InsertTimeField"
#define CMD_FN_INSERT_FLD_TITLE                     ".uno:InsertTitleField"
#define CMD_FN_INSERT_FLD_TOPIC                     ".uno:InsertTopicField"
#define CMD_FN_TABLE_MERGE_CELLS                    ".uno:MergeCells"
#define CMD_FN_TABLE_MERGE_TABLE                    ".uno:MergeTable"
#define CMD_FN_TITLE_DESCRIPTION_SHAPE              ".uno:ObjectTitleDescription"
#define CMD_FN_NAME_SHAPE                           ".uno:NameGroup"
#define CMD_FN_NUMBER_NEWSTART                      ".uno:NumberingStart"
#define CMD_FN_FRAME_DOWN                           ".uno:ObjectBackOne"
#define CMD_FN_FRAME_UP                             ".uno:ObjectForwardOne"
#define CMD_FN_FORMAT_PAGE_DLG                      ".uno:PageDialog"
#define CMD_FN_PAGEDOWN                             ".uno:PageDown"
#define CMD_FN_PAGEUP                               ".uno:PageUp"
#define CMD_FN_PRINT_PAGEPREVIEW                    ".uno:PrintPagePreview"
#define CMD_FN_TABLE_SET_READ_ONLY_CELLS            ".uno:Protect"
#define CMD_FN_REMOVE_CUR_TOX                       ".uno:RemoveTableOf"
#define CMD_FN_REMOVE_TEXT_BOX                      ".uno:RemoveTextBox"
#define CMD_FN_FORMAT_RESET                         ".uno:ResetAttributes"
#define CMD_FN_TOOL_ANCHOR_AT_CHAR                  ".uno:SetAnchorAtChar"
#define CMD_FN_TOOL_ANCHOR_CHAR                     ".uno:SetAnchorToChar"
#define CMD_FN_TOOL_ANCHOR_FRAME                    ".uno:SetAnchorToFrame"
#define CMD_FN_TOOL_ANCHOR_PAGE                     ".uno:SetAnchorToPage"
#define CMD_FN_TOOL_ANCHOR_PARAGRAPH                ".uno:SetAnchorToPara"
#define CMD_FN_TABLE_SET_COL_WIDTH                  ".uno:SetColumnWidth"
#define CMD_FN_TABLE_ADJUST_CELLS                   ".uno:SetOptimalColumnWidth"
#define CMD_FN_TABLE_OPTIMAL_HEIGHT                 ".uno:SetOptimalRowHeight"
#define CMD_FN_TABLE_SET_ROW_HEIGHT                 ".uno:SetRowHeight"
#define CMD_FN_TABLE_SPLIT_CELLS                    ".uno:SplitCell"
#define CMD_FN_TABLE_SPLIT_TABLE                    ".uno:SplitTable"
#define CMD_FN_SET_SUB_SCRIPT                       ".uno:SubScript"
#define CMD_FN_SET_SUPER_SCRIPT                     ".uno:SuperScript"
#define CMD_FN_FORMAT_TABLE_DLG                     ".uno:TableDialog"
#define CMD_FN_NUM_FORMAT_TABLE_DLG                 ".uno:TableNumberFormatDialog"
#define CMD_FN_DRAWTEXT_ATTR_DLG                    ".uno:TextAttributes"
#define CMD_FN_DRAW_WRAP_DLG                        ".uno:TextWrap"
#define CMD_FN_TOOL_ANCHOR                          ".uno:ToggleAnchorType"
#define CMD_FN_TABLE_UNSET_READ_ONLY_CELLS          ".uno:UnsetCellsReadOnly"
#define CMD_FN_UPDATE_CUR_TOX                       ".uno:UpdateCurIndex"
#define CMD_FN_WRAP_ANCHOR_ONLY                     ".uno:WrapAnchorOnly"
#define CMD_FN_FRAME_WRAP_CONTOUR                   ".uno:WrapContour"
#define CMD_FN_FRAME_WRAP_IDEAL                     ".uno:WrapIdeal"
#define CMD_FN_FRAME_NOWRAP                         ".uno:WrapOff"
#define CMD_FN_FRAME_WRAP                           ".uno:WrapOn"
#define CMD_FN_FRAME_WRAPTHRU                       ".uno:WrapThrough"
#define CMD_FN_FRAME_WRAPTHRU_TRANSP                ".uno:WrapThroughTransparent"
#define CMD_FN_READONLY_SELECTION_MODE              ".uno:SelectTextMode"
#define CMD_FN_TABLE_ROW_SPLIT                      ".uno:RowSplit"
#define CMD_FN_NUM_CONTINUE                         ".uno:ContinueNumbering"
#define CMD_FN_REPLY                                ".uno:ReplyNote"
#define CMD_FN_DELETE_COMMENT                       ".uno:DeleteComment"
#define CMD_FN_DELETE_NOTE_AUTHOR                   ".uno:DeleteAuthor"
#define CMD_FN_DELETE_ALL_NOTES                     ".uno:DeleteAllNotes"
#define CMD_FN_FORMAT_ALL_NOTES                     ".uno:FormatAllNotes"
#define CMD_FN_HIDE_NOTE                            ".uno:HideNote"
#define CMD_FN_HIDE_NOTE_AUTHOR                     ".uno:HideAuthor"
#define CMD_FN_HIDE_ALL_NOTES                       ".uno:HideAllNotes"
#define CMD_FN_REDLINE_ACCEPT_DIRECT                ".uno:AcceptTrackedChange"
#define CMD_FN_REDLINE_REJECT_DIRECT                ".uno:RejectTrackedChange"
#define CMD_FN_REDLINE_NEXT_CHANGE                  ".uno:NextTrackedChange"
#define CMD_FN_REDLINE_PREV_CHANGE                  ".uno:PreviousTrackedChange"
#define CMD_FN_REMOVE_HYPERLINK                     ".uno:RemoveHyperlink"
#define CMD_FN_COPY_HYPERLINK_LOCATION              ".uno:CopyHyperlinkLocation"
#define CMD_FN_HEADERFOOTER_EDIT                    ".uno:HeaderFooterEdit"
#define CMD_FN_HEADERFOOTER_DELETE                  ".uno:HeaderFooterDelete"
#define CMD_FN_HEADERFOOTER_BORDERBACK              ".uno:HeaderFooterBorderBackground"
#define CMD_FN_PAGEBREAK_EDIT                       ".uno:PageBreakEdit"
#define CMD_FN_PAGEBREAK_DELETE                     ".uno:PageBreakDelete"
#define CMD_SID_JUMP_TO_SPECIFIC_PAGE                ".uno:JumpToSpecificPage"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


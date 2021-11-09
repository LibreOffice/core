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

#ifndef INCLUDED_SW_INC_UNOPRNMS_HXX
#define INCLUDED_SW_INC_UNOPRNMS_HXX

#include <sal/config.h>

inline constexpr OUStringLiteral UNO_NAME_FOLLOW_STYLE = u"FollowStyle";
inline constexpr OUStringLiteral UNO_NAME_LINK_STYLE = u"LinkStyle";
inline constexpr OUStringLiteral UNO_NAME_IS_PHYSICAL = u"IsPhysical";
inline constexpr OUStringLiteral UNO_NAME_IS_AUTO_UPDATE = u"IsAutoUpdate";
inline constexpr OUStringLiteral UNO_NAME_DISPLAY_NAME = u"DisplayName";
inline constexpr OUStringLiteral UNO_NAME_PARA_GRAPHIC = u"ParaBackGraphic";
inline constexpr OUStringLiteral UNO_NAME_PARA_GRAPHIC_URL = u"ParaBackGraphicURL";
inline constexpr OUStringLiteral UNO_NAME_PARA_GRAPHIC_FILTER = u"ParaBackGraphicFilter";
inline constexpr OUStringLiteral UNO_NAME_HEADER_GRAPHIC = u"HeaderBackGraphic";
inline constexpr OUStringLiteral UNO_NAME_HEADER_GRAPHIC_URL = u"HeaderBackGraphicURL";
inline constexpr OUStringLiteral UNO_NAME_HEADER_GRAPHIC_FILTER = u"HeaderBackGraphicFilter";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_GRAPHIC = u"FooterBackGraphic";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_GRAPHIC_URL = u"FooterBackGraphicURL";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_GRAPHIC_FILTER = u"FooterBackGraphicFilter";
inline constexpr OUStringLiteral UNO_NAME_BACK_GRAPHIC_URL = u"BackGraphicURL";
inline constexpr OUStringLiteral UNO_NAME_BACK_GRAPHIC = u"BackGraphic";
inline constexpr OUStringLiteral UNO_NAME_BACK_GRAPHIC_FILTER = u"BackGraphicFilter";
inline constexpr OUStringLiteral UNO_NAME_BACK_GRAPHIC_LOCATION = u"BackGraphicLocation";
inline constexpr OUStringLiteral UNO_NAME_GRAPHIC_FILTER = u"GraphicFilter";
inline constexpr OUStringLiteral UNO_NAME_GRAPHIC_SIZE = u"GraphicSize";
inline constexpr OUStringLiteral UNO_NAME_GRAPHIC_BITMAP = u"GraphicBitmap";
inline constexpr OUStringLiteral UNO_NAME_GRAPHIC_URL = u"GraphicURL";
#define UNO_NAME_BULLET_ID "BulletId"
#define UNO_NAME_BULLET_CHAR "BulletChar"
#define UNO_NAME_BULLET_REL_SIZE "BulletRelSize"
inline constexpr OUStringLiteral UNO_NAME_BULLET_FONT = u"BulletFont";
#define UNO_NAME_BULLET_FONT_NAME "BulletFontName"
#define UNO_NAME_BULLET_COLOR "BulletColor"
inline constexpr OUStringLiteral UNO_NAME_PARA_GRAPHIC_LOCATION = u"ParaBackGraphicLocation";
inline constexpr OUStringLiteral UNO_NAME_HEADER_GRAPHIC_LOCATION = u"HeaderBackGraphicLocation";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_GRAPHIC_LOCATION = u"FooterBackGraphicLocation";
inline constexpr OUStringLiteral UNO_NAME_PARA_LEFT_MARGIN = u"ParaLeftMargin";
inline constexpr OUStringLiteral UNO_NAME_PARA_RIGHT_MARGIN = u"ParaRightMargin";
inline constexpr OUStringLiteral UNO_NAME_PARA_LEFT_MARGIN_RELATIVE = u"ParaLeftMarginRelative";
inline constexpr OUStringLiteral UNO_NAME_PARA_RIGHT_MARGIN_RELATIVE = u"ParaRightMarginRelative";
inline constexpr OUStringLiteral UNO_NAME_PARA_IS_AUTO_FIRST_LINE_INDENT
    = u"ParaIsAutoFirstLineIndent";
inline constexpr OUStringLiteral UNO_NAME_PARA_FIRST_LINE_INDENT = u"ParaFirstLineIndent";
inline constexpr OUStringLiteral UNO_NAME_PARA_FIRST_LINE_INDENT_RELATIVE
    = u"ParaFirstLineIndentRelative";
inline constexpr OUStringLiteral UNO_NAME_PARA_IS_HYPHENATION = u"ParaIsHyphenation";
inline constexpr OUStringLiteral UNO_NAME_PARA_HYPHENATION_MAX_LEADING_CHARS
    = u"ParaHyphenationMaxLeadingChars";
inline constexpr OUStringLiteral UNO_NAME_PARA_HYPHENATION_MAX_TRAILING_CHARS
    = u"ParaHyphenationMaxTrailingChars";
inline constexpr OUStringLiteral UNO_NAME_PARA_HYPHENATION_MAX_HYPHENS
    = u"ParaHyphenationMaxHyphens";
inline constexpr OUStringLiteral UNO_NAME_PARA_HYPHENATION_NO_CAPS = u"ParaHyphenationNoCaps";
inline constexpr OUStringLiteral UNO_NAME_LEFT_MARGIN = u"LeftMargin";
inline constexpr OUStringLiteral UNO_NAME_RIGHT_MARGIN = u"RightMargin";
inline constexpr OUStringLiteral UNO_NAME_GUTTER_MARGIN = u"GutterMargin";
inline constexpr OUStringLiteral UNO_NAME_HEADER_LEFT_MARGIN = u"HeaderLeftMargin";
inline constexpr OUStringLiteral UNO_NAME_HEADER_RIGHT_MARGIN = u"HeaderRightMargin";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_LEFT_MARGIN = u"FooterLeftMargin";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_RIGHT_MARGIN = u"FooterRightMargin";
inline constexpr OUStringLiteral UNO_NAME_TEXT_RANGE = u"TextRange";
inline constexpr OUStringLiteral UNO_NAME_TEXT_BOX = u"TextBox";
inline constexpr OUStringLiteral UNO_NAME_NAME = u"Name";
inline constexpr OUStringLiteral UNO_NAME_CHAR_STYLE_NAME = u"CharStyleName";
inline constexpr OUStringLiteral UNO_NAME_ANCHOR_CHAR_STYLE_NAME = u"AnchorCharStyleName";
inline constexpr OUStringLiteral UNO_NAME_SUFFIX = u"Suffix";
inline constexpr OUStringLiteral UNO_NAME_PREFIX = u"Prefix";
#define UNO_NAME_LIST_FORMAT "ListFormat"
#define UNO_NAME_PARENT_NUMBERING "ParentNumbering"
inline constexpr OUStringLiteral UNO_NAME_CHAR_FONT_NAME = u"CharFontName";
inline constexpr OUStringLiteral UNO_NAME_CHAR_FONT_STYLE_NAME = u"CharFontStyleName";
inline constexpr OUStringLiteral UNO_NAME_CHAR_FONT_FAMILY = u"CharFontFamily";
inline constexpr OUStringLiteral UNO_NAME_CHAR_FONT_CHAR_SET = u"CharFontCharSet";
inline constexpr OUStringLiteral UNO_NAME_CHAR_FONT_PITCH = u"CharFontPitch";
inline constexpr OUStringLiteral UNO_NAME_CHAR_LOCALE = u"CharLocale";
inline constexpr OUStringLiteral UNO_NAME_CHAR_FONT_NAME_ASIAN = u"CharFontNameAsian";
inline constexpr OUStringLiteral UNO_NAME_CHAR_FONT_STYLE_NAME_ASIAN = u"CharFontStyleNameAsian";
inline constexpr OUStringLiteral UNO_NAME_CHAR_FONT_FAMILY_ASIAN = u"CharFontFamilyAsian";
inline constexpr OUStringLiteral UNO_NAME_CHAR_FONT_CHAR_SET_ASIAN = u"CharFontCharSetAsian";
inline constexpr OUStringLiteral UNO_NAME_CHAR_FONT_PITCH_ASIAN = u"CharFontPitchAsian";
inline constexpr OUStringLiteral UNO_NAME_CHAR_POSTURE_ASIAN = u"CharPostureAsian";
inline constexpr OUStringLiteral UNO_NAME_CHAR_WEIGHT_ASIAN = u"CharWeightAsian";
inline constexpr OUStringLiteral UNO_NAME_CHAR_HEIGHT_ASIAN = u"CharHeightAsian";
inline constexpr OUStringLiteral UNO_NAME_CHAR_LOCALE_ASIAN = u"CharLocaleAsian";
inline constexpr OUStringLiteral UNO_NAME_CHAR_FONT_NAME_COMPLEX = u"CharFontNameComplex";
inline constexpr OUStringLiteral UNO_NAME_CHAR_FONT_STYLE_NAME_COMPLEX
    = u"CharFontStyleNameComplex";
inline constexpr OUStringLiteral UNO_NAME_CHAR_FONT_FAMILY_COMPLEX = u"CharFontFamilyComplex";
inline constexpr OUStringLiteral UNO_NAME_CHAR_FONT_CHAR_SET_COMPLEX = u"CharFontCharSetComplex";
inline constexpr OUStringLiteral UNO_NAME_CHAR_FONT_PITCH_COMPLEX = u"CharFontPitchComplex";
inline constexpr OUStringLiteral UNO_NAME_CHAR_POSTURE_COMPLEX = u"CharPostureComplex";
inline constexpr OUStringLiteral UNO_NAME_CHAR_WEIGHT_COMPLEX = u"CharWeightComplex";
inline constexpr OUStringLiteral UNO_NAME_CHAR_HEIGHT_COMPLEX = u"CharHeightComplex";
inline constexpr OUStringLiteral UNO_NAME_CHAR_LOCALE_COMPLEX = u"CharLocaleComplex";
inline constexpr OUStringLiteral UNO_NAME_CHAR_AUTO_KERNING = u"CharAutoKerning";
inline constexpr OUStringLiteral UNO_NAME_CHAR_UNDERLINE_COLOR = u"CharUnderlineColor";
inline constexpr OUStringLiteral UNO_NAME_CHAR_UNDERLINE_HAS_COLOR = u"CharUnderlineHasColor";
inline constexpr OUStringLiteral UNO_NAME_CHAR_ESCAPEMENT = u"CharEscapement";
inline constexpr OUStringLiteral UNO_NAME_CHAR_CASE_MAP = u"CharCaseMap";
inline constexpr OUStringLiteral UNO_NAME_CHAR_STRIKEOUT = u"CharStrikeout";
inline constexpr OUStringLiteral UNO_NAME_CHAR_CROSSED_OUT = u"CharCrossedOut";
inline constexpr OUStringLiteral UNO_NAME_CHAR_NO_HYPHENATION = u"CharNoHyphenation";
inline constexpr OUStringLiteral UNO_NAME_CHAR_AUTO_ESCAPEMENT = u"CharAutoEscapement";
inline constexpr OUStringLiteral UNO_NAME_CHAR_PROP_HEIGHT = u"CharPropHeight";
inline constexpr OUStringLiteral UNO_NAME_CHAR_DIFF_HEIGHT = u"CharDiffHeight";
inline constexpr OUStringLiteral UNO_NAME_CHAR_PROP_HEIGHT_ASIAN = u"CharPropHeightAsian";
inline constexpr OUStringLiteral UNO_NAME_CHAR_DIFF_HEIGHT_ASIAN = u"CharDiffHeightAsian";
inline constexpr OUStringLiteral UNO_NAME_CHAR_PROP_HEIGHT_COMPLEX = u"CharPropHeightComplex";
inline constexpr OUStringLiteral UNO_NAME_CHAR_DIFF_HEIGHT_COMPLEX = u"CharDiffHeightComplex";
inline constexpr OUStringLiteral UNO_NAME_CHAR_ESCAPEMENT_HEIGHT = u"CharEscapementHeight";
inline constexpr OUStringLiteral UNO_NAME_CHAR_TRANSPARENCE = u"CharTransparence";
inline constexpr OUStringLiteral UNO_NAME_HIDE_TAB_LEADER_AND_PAGE_NUMBERS
    = u"HideTabLeaderAndPageNumber";
inline constexpr OUStringLiteral UNO_NAME_TAB_IN_TOC = u"TabInTOC";
inline constexpr OUStringLiteral UNO_NAME_TOC_BOOKMARK = u"TOCBookmark";
inline constexpr OUStringLiteral UNO_NAME_TOC_NEWLINE = u"TOCNewLine";
inline constexpr OUStringLiteral UNO_NAME_TOC_PARAGRAPH_OUTLINE_LEVEL = u"TOCParagraphOutlineLevel";

inline constexpr OUStringLiteral UNO_NAME_CHAR_FLASH = u"CharFlash";
inline constexpr OUStringLiteral UNO_NAME_CHAR_KERNING = u"CharKerning";
inline constexpr OUStringLiteral UNO_NAME_CHAR_BACK_COLOR = u"CharBackColor";
inline constexpr OUStringLiteral UNO_NAME_CHAR_BACK_TRANSPARENT = u"CharBackTransparent";
inline constexpr OUStringLiteral UNO_NAME_CHAR_COMBINE_IS_ON = u"CharCombineIsOn";
inline constexpr OUStringLiteral UNO_NAME_CHAR_COMBINE_PREFIX = u"CharCombinePrefix";
inline constexpr OUStringLiteral UNO_NAME_CHAR_COMBINE_SUFFIX = u"CharCombineSuffix";
inline constexpr OUStringLiteral UNO_NAME_CHAR_EMPHASIS = u"CharEmphasis";
inline constexpr OUStringLiteral UNO_NAME_PARA_LINE_SPACING = u"ParaLineSpacing";
inline constexpr OUStringLiteral UNO_NAME_PARA_TOP_MARGIN = u"ParaTopMargin";
inline constexpr OUStringLiteral UNO_NAME_PARA_BOTTOM_MARGIN = u"ParaBottomMargin";
inline constexpr OUStringLiteral UNO_NAME_PARA_TOP_MARGIN_RELATIVE = u"ParaTopMarginRelative";
inline constexpr OUStringLiteral UNO_NAME_PARA_BOTTOM_MARGIN_RELATIVE = u"ParaBottomMarginRelative";
inline constexpr OUStringLiteral UNO_NAME_PARA_EXPAND_SINGLE_WORD = u"ParaExpandSingleWord";
inline constexpr OUStringLiteral UNO_NAME_END_NOTICE = u"EndNotice";
inline constexpr OUStringLiteral UNO_NAME_BEGIN_NOTICE = u"BeginNotice";
inline constexpr OUStringLiteral UNO_NAME_FRAME_STYLE_NAME = u"FrameStyleName";
inline constexpr OUStringLiteral UNO_NAME_NUMBERING_STYLE_NAME = u"NumberingStyleName";
inline constexpr OUStringLiteral UNO_NAME_NUMBERING_START_VALUE = u"NumberingStartValue";
inline constexpr OUStringLiteral UNO_NAME_NUMBERING_IS_NUMBER = u"NumberingIsNumber";
inline constexpr OUStringLiteral UNO_NAME_COUNT_LINES_IN_FRAMES = u"CountLinesInFrames";
inline constexpr OUStringLiteral UNO_NAME_DDE_COMMAND_TYPE = u"DDECommandType";
inline constexpr OUStringLiteral UNO_NAME_DDE_COMMAND_FILE = u"DDECommandFile";
inline constexpr OUStringLiteral UNO_NAME_DDE_COMMAND_ELEMENT = u"DDECommandElement";
inline constexpr OUStringLiteral UNO_NAME_IS_AUTOMATIC_UPDATE = u"IsAutomaticUpdate";
inline constexpr OUStringLiteral UNO_NAME_DISTANCE = u"Distance";
inline constexpr OUStringLiteral UNO_NAME_DROP_CAP_FORMAT = u"DropCapFormat";
inline constexpr OUStringLiteral UNO_NAME_DROP_CAP_WHOLE_WORD = u"DropCapWholeWord";
inline constexpr OUStringLiteral UNO_NAME_DROP_CAP_CHAR_STYLE_NAME = u"DropCapCharStyleName";
inline constexpr OUStringLiteral UNO_NAME_FILE_LINK = u"FileLink";
inline constexpr OUStringLiteral UNO_NAME_GRAPHIC = u"Graphic";
inline constexpr OUStringLiteral UNO_NAME_TRANSFORMED_GRAPHIC = u"TransformedGraphic";
inline constexpr OUStringLiteral UNO_NAME_IS_PROTECTED = u"IsProtected";
inline constexpr OUStringLiteral UNO_NAME_PARA_KEEP_TOGETHER = u"ParaKeepTogether";
inline constexpr OUStringLiteral UNO_NAME_KEEP_TOGETHER = u"KeepTogether";
inline constexpr OUStringLiteral UNO_NAME_IS_LANDSCAPE = u"IsLandscape";
inline constexpr OUStringLiteral UNO_NAME_SEPARATOR_TEXT = u"SeparatorText";
inline constexpr OUStringLiteral UNO_NAME_NUMBER_POSITION = u"NumberPosition";
inline constexpr OUStringLiteral UNO_NAME_PAGE_STYLE_NAME = u"PageStyleName";
inline constexpr OUStringLiteral UNO_NAME_PAGE_STYLE_LAYOUT = u"PageStyleLayout";
inline constexpr OUStringLiteral UNO_NAME_PARA_ADJUST = u"ParaAdjust";
inline constexpr OUStringLiteral UNO_NAME_PARA_REGISTER_MODE_ACTIVE = u"ParaRegisterModeActive";
inline constexpr OUStringLiteral UNO_NAME_PARA_STYLE_NAME = u"ParaStyleName";
inline constexpr OUStringLiteral UNO_NAME_PARA_LAST_LINE_ADJUST = u"ParaLastLineAdjust";
inline constexpr OUStringLiteral UNO_NAME_PARA_LINE_NUMBER_COUNT = u"ParaLineNumberCount";
inline constexpr OUStringLiteral UNO_NAME_PARA_LINE_NUMBER_START_VALUE
    = u"ParaLineNumberStartValue";
inline constexpr OUStringLiteral UNO_NAME_BACK_COLOR = u"BackColor";
inline constexpr OUStringLiteral UNO_NAME_PARA_BACK_COLOR = u"ParaBackColor";
inline constexpr OUStringLiteral UNO_NAME_PARA_WIDOWS = u"ParaWidows";
inline constexpr OUStringLiteral UNO_NAME_PARA_ORPHANS = u"ParaOrphans";
inline constexpr OUStringLiteral UNO_NAME_PARA_BACK_TRANSPARENT = u"ParaBackTransparent";
inline constexpr OUStringLiteral UNO_NAME_POSITION_END_OF_DOC = u"PositionEndOfDoc";
inline constexpr OUStringLiteral UNO_NAME_POSITION_PROTECTED = u"PositionProtected";
inline constexpr OUStringLiteral UNO_NAME_ALTERNATIVE_TEXT = u"AlternativeText";
inline constexpr OUStringLiteral UNO_NAME_PRIMARY_KEY = u"PrimaryKey";
inline constexpr OUStringLiteral UNO_NAME_PRINTER_PAPER_TRAY = u"PrinterPaperTray";
#define UNO_NAME_REGISTER_MODE_ACTIVE "RegisterModeActive"
inline constexpr OUStringLiteral UNO_NAME_RELATIVE_WIDTH = u"RelativeWidth";
inline constexpr OUStringLiteral UNO_NAME_RELATIVE_WIDTH_RELATION = u"RelativeWidthRelation";
inline constexpr OUStringLiteral UNO_NAME_RELATIVE_HEIGHT = u"RelativeHeight";
inline constexpr OUStringLiteral UNO_NAME_RELATIVE_HEIGHT_RELATION = u"RelativeHeightRelation";
inline constexpr OUStringLiteral UNO_NAME_REPEAT_HEADLINE = u"RepeatHeadline";
inline constexpr OUStringLiteral UNO_NAME_SEARCH_STYLES = u"SearchStyles";
inline constexpr OUStringLiteral UNO_NAME_SEARCH_BACKWARDS = u"SearchBackwards";
inline constexpr OUStringLiteral UNO_NAME_SEARCH_SIMILARITY = u"SearchSimilarity";
inline constexpr OUStringLiteral UNO_NAME_SEARCH_CASE_SENSITIVE = u"SearchCaseSensitive";
inline constexpr OUStringLiteral UNO_NAME_SEARCH_SIMILARITY_ADD = u"SearchSimilarityAdd";
inline constexpr OUStringLiteral UNO_NAME_SEARCH_SIMILARITY_RELAX = u"SearchSimilarityRelax";
inline constexpr OUStringLiteral UNO_NAME_SEARCH_SIMILARITY_REMOVE = u"SearchSimilarityRemove";
inline constexpr OUStringLiteral UNO_NAME_SEARCH_REGULAR_EXPRESSION = u"SearchRegularExpression";
inline constexpr OUStringLiteral UNO_NAME_SEARCH_SIMILARITY_EXCHANGE = u"SearchSimilarityExchange";
inline constexpr OUStringLiteral UNO_NAME_SECONDARY_KEY = u"SecondaryKey";
inline constexpr OUStringLiteral UNO_NAME_SEPARATOR_INTERVAL = u"SeparatorInterval";
inline constexpr OUStringLiteral UNO_NAME_SHOW_BREAKS = u"ShowBreaks";
inline constexpr OUStringLiteral UNO_NAME_SHOW_SPACES = u"ShowSpaces";
inline constexpr OUStringLiteral UNO_NAME_SHOW_TABLES = u"ShowTables";
inline constexpr OUStringLiteral UNO_NAME_SHOW_GRAPHICS = u"ShowGraphics";

inline constexpr OUStringLiteral UNO_NAME_SHOW_DRAWINGS = u"ShowDrawings";
inline constexpr OUStringLiteral UNO_NAME_SHOW_TABSTOPS = u"ShowTabstops";
inline constexpr OUStringLiteral UNO_NAME_SHOW_VERT_RULER = u"ShowVertRuler";
inline constexpr OUStringLiteral UNO_NAME_SHOW_PARA_BREAKS = u"ShowParaBreaks";
inline constexpr OUStringLiteral UNO_NAME_SHOW_HIDDEN_TEXT = u"ShowHiddenText";
inline constexpr OUStringLiteral UNO_NAME_SHOW_SOFT_HYPHENS = u"ShowSoftHyphens";
inline constexpr OUStringLiteral UNO_NAME_SHOW_VERT_SCROLL_BAR = u"ShowVertScrollBar";
inline constexpr OUStringLiteral UNO_NAME_SHOW_HORI_SCROLL_BAR = u"ShowHoriScrollBar";
inline constexpr OUStringLiteral UNO_NAME_SHOW_FIELD_COMMANDS = u"ShowFieldCommands";
inline constexpr OUStringLiteral UNO_NAME_SHOW_PROTECTED_SPACES = u"ShowProtectedSpaces";
inline constexpr OUStringLiteral UNO_NAME_SHOW_HIDDEN_PARAGRAPHS = u"ShowHiddenParagraphs";
inline constexpr OUStringLiteral UNO_NAME_HIDE_WHITESPACE = u"HideWhitespace";
inline constexpr OUStringLiteral UNO_NAME_IS_SYNC_WIDTH_TO_HEIGHT = u"IsSyncWidthToHeight";
inline constexpr OUStringLiteral UNO_NAME_IS_SYNC_HEIGHT_TO_WIDTH = u"IsSyncHeightToWidth";
inline constexpr OUStringLiteral UNO_NAME_SIZE_PROTECTED = u"SizeProtected";
inline constexpr OUStringLiteral UNO_NAME_TEXT_COLUMNS = u"TextColumns";
inline constexpr OUStringLiteral UNO_NAME_BACK_TRANSPARENT = u"BackTransparent";
inline constexpr OUStringLiteral UNO_NAME_ANCHOR_TYPE = u"AnchorType";
inline constexpr OUStringLiteral UNO_NAME_ANCHOR_TYPES = u"AnchorTypes";
inline constexpr OUStringLiteral UNO_NAME_ANCHOR_PAGE_NO = u"AnchorPageNo";
inline constexpr OUStringLiteral UNO_NAME_ANCHOR_FRAME = u"AnchorFrame";
inline constexpr OUStringLiteral UNO_NAME_AUTHOR = u"Author";
inline constexpr OUStringLiteral UNO_NAME_BREAK_TYPE = u"BreakType";
inline constexpr OUStringLiteral UNO_NAME_CHAIN_NEXT_NAME = u"ChainNextName";
inline constexpr OUStringLiteral UNO_NAME_CHAIN_PREV_NAME = u"ChainPrevName";
inline constexpr OUStringLiteral UNO_NAME_CHAIN_NAME = u"ChainName";
inline constexpr OUStringLiteral UNO_NAME_CHAPTER_FORMAT = u"ChapterFormat";
inline constexpr OUStringLiteral UNO_NAME_CLIENT_MAP = u"ClientMap";
inline constexpr OUStringLiteral UNO_NAME_CONDITION = u"Condition";
inline constexpr OUStringLiteral UNO_NAME_CONTENT = u"Content";
inline constexpr OUStringLiteral UNO_NAME_CHAR_CONTOURED = u"CharContoured";
inline constexpr OUStringLiteral UNO_NAME_CONTOUR_OUTSIDE = u"ContourOutside";
inline constexpr OUStringLiteral UNO_NAME_CONTENT_PROTECTED = u"ContentProtected";
inline constexpr OUStringLiteral UNO_NAME_COUNT_EMPTY_LINES = u"CountEmptyLines";
inline constexpr OUStringLiteral UNO_NAME_RESTART_AT_EACH_PAGE = u"RestartAtEachPage";
inline constexpr OUStringLiteral UNO_NAME_DATA_BASE_NAME = u"DataBaseName";
inline constexpr OUStringLiteral UNO_NAME_DATA_TABLE_NAME = u"DataTableName";
inline constexpr OUStringLiteral UNO_NAME_DATA_COMMAND_TYPE = u"DataCommandType";
inline constexpr OUStringLiteral UNO_NAME_DATA_COLUMN_NAME = u"DataColumnName";
inline constexpr OUStringLiteral UNO_NAME_IS_DATA_BASE_FORMAT = u"DataBaseFormat";
inline constexpr OUStringLiteral UNO_NAME_DATE = u"Date";
inline constexpr OUStringLiteral UNO_NAME_IS_DATE = u"IsDate";
inline constexpr OUStringLiteral UNO_NAME_EDIT_IN_READONLY = u"EditInReadonly";
inline constexpr OUStringLiteral UNO_NAME_FALSE_CONTENT = u"FalseContent";
inline constexpr OUStringLiteral UNO_NAME_FILE_FORMAT = u"FileFormat";
inline constexpr OUStringLiteral UNO_NAME_IS_FIXED = u"IsFixed";
inline constexpr OUStringLiteral UNO_NAME_FOOTNOTE_COUNTING = u"FootnoteCounting";
inline constexpr OUStringLiteral UNO_NAME_FULL_NAME = u"FullName";
inline constexpr OUStringLiteral UNO_NAME_HEIGHT = u"Height";
inline constexpr OUStringLiteral UNO_NAME_IS_AUTO_HEIGHT = u"IsAutoHeight";
inline constexpr OUStringLiteral UNO_NAME_SIZE_TYPE = u"SizeType";
inline constexpr OUStringLiteral UNO_NAME_HINT = u"Hint";
inline constexpr OUStringLiteral UNO_NAME_HORI_ORIENT = u"HoriOrient";
inline constexpr OUStringLiteral UNO_NAME_HORI_MIRRORED_ON_EVEN_PAGES = u"HoriMirroredOnEvenPages";
inline constexpr OUStringLiteral UNO_NAME_HORI_MIRRORED_ON_ODD_PAGES = u"HoriMirroredOnOddPages";
inline constexpr OUStringLiteral UNO_NAME_HORI_ORIENT_RELATION = u"HoriOrientRelation";
inline constexpr OUStringLiteral UNO_NAME_HORI_ORIENT_POSITION = u"HoriOrientPosition";
inline constexpr OUStringLiteral UNO_NAME_HYPER_LINK_U_R_L = u"HyperLinkURL";
inline constexpr OUStringLiteral UNO_NAME_HYPER_LINK_TARGET = u"HyperLinkTarget";
inline constexpr OUStringLiteral UNO_NAME_HYPER_LINK_NAME = u"HyperLinkName";
inline constexpr OUStringLiteral UNO_NAME_HYPER_LINK_EVENTS = u"HyperLinkEvents";
inline constexpr OUStringLiteral UNO_NAME_INFO_TYPE = u"InfoType";
inline constexpr OUStringLiteral UNO_NAME_INFO_FORMAT = u"InfoFormat";
inline constexpr OUStringLiteral UNO_NAME_IS_INPUT = u"IsInput";
inline constexpr OUStringLiteral UNO_NAME_LEVEL = u"Level";
inline constexpr OUStringLiteral UNO_NAME_INTERVAL = u"Interval";
inline constexpr OUStringLiteral UNO_NAME_LINK_REGION = u"LinkRegion";
inline constexpr OUStringLiteral UNO_NAME_MACRO_NAME = u"MacroName";
inline constexpr OUStringLiteral UNO_NAME_SPLIT = u"Split";
inline constexpr OUStringLiteral UNO_NAME_PARA_SPLIT = u"ParaSplit";
inline constexpr OUStringLiteral UNO_NAME_NUMBER_FORMAT = u"NumberFormat";
inline constexpr OUStringLiteral UNO_NAME_NUMBERING_TYPE = u"NumberingType";
inline constexpr OUStringLiteral UNO_NAME_OFFSET = u"Offset";
inline constexpr OUStringLiteral UNO_NAME_ON = u"On";
inline constexpr OUStringLiteral UNO_NAME_OPAQUE = u"Opaque";
inline constexpr OUStringLiteral UNO_NAME_PAGE_TOGGLE = u"PageToggle";
inline constexpr OUStringLiteral UNO_NAME_PAGE_DESC_NAME = u"PageDescName";
inline constexpr OUStringLiteral UNO_NAME_PAGE_NUMBER_OFFSET = u"PageNumberOffset";
inline constexpr OUStringLiteral UNO_NAME_PLACEHOLDER = u"PlaceHolder";
inline constexpr OUStringLiteral UNO_NAME_PLACEHOLDER_TYPE = u"PlaceHolderType";
inline constexpr OUStringLiteral UNO_NAME_PRINT = u"Print";
inline constexpr OUStringLiteral UNO_NAME_REFERENCE_FIELD_PART = u"ReferenceFieldPart";
inline constexpr OUStringLiteral UNO_NAME_REFERENCE_FIELD_SOURCE = u"ReferenceFieldSource";
inline constexpr OUStringLiteral UNO_NAME_REFERENCE_FIELD_LANGUAGE = u"ReferenceFieldLanguage";
inline constexpr OUStringLiteral UNO_NAME_REGISTER_PARAGRAPH_STYLE = u"RegisterParagraphStyle";
inline constexpr OUStringLiteral UNO_NAME_SCRIPT_TYPE = u"ScriptType";
inline constexpr OUStringLiteral UNO_NAME_SEARCH_ALL = u"SearchAll";

inline constexpr OUStringLiteral UNO_NAME_SEARCH_WORDS = u"SearchWords";
inline constexpr OUStringLiteral UNO_NAME_SEQUENCE_VALUE = u"SequenceValue";
inline constexpr OUStringLiteral UNO_NAME_SERVER_MAP = u"ServerMap";
inline constexpr OUStringLiteral UNO_NAME_IMAGE_MAP = u"ImageMap";
inline constexpr OUStringLiteral UNO_NAME_SET_NUMBER = u"SetNumber";
inline constexpr OUStringLiteral UNO_NAME_SHADOW_FORMAT = u"ShadowFormat";
inline constexpr OUStringLiteral UNO_NAME_SHOW_HORI_RULER = u"ShowHoriRuler";
inline constexpr OUStringLiteral UNO_NAME_SIZE = u"Size";
inline constexpr OUStringLiteral UNO_NAME_ACTUAL_SIZE = u"ActualSize";
inline constexpr OUStringLiteral UNO_NAME_SOURCE_NAME = u"SourceName";
inline constexpr OUStringLiteral UNO_NAME_START_AT = u"StartAt";
#define UNO_NAME_START_WITH "StartWith"
#define UNO_NAME_STATISTIC_TYPE_ID "StatisticTypeId"
inline constexpr OUStringLiteral UNO_NAME_SUB_TYPE = u"SubType";
inline constexpr OUStringLiteral UNO_NAME_SURROUND = u"Surround";
inline constexpr OUStringLiteral UNO_NAME_IS_EXPRESSION = u"IsExpression";
inline constexpr OUStringLiteral UNO_NAME_IS_SHOW_FORMULA = u"IsShowFormula";
inline constexpr OUStringLiteral UNO_NAME_TEXT_WRAP = u"TextWrap";
inline constexpr OUStringLiteral UNO_NAME_SURROUND_CONTOUR = u"SurroundContour";
inline constexpr OUStringLiteral UNO_NAME_SURROUND_ANCHORONLY = u"SurroundAnchorOnly";
inline constexpr OUStringLiteral UNO_NAME_TABLE_NAME = u"TableName";
inline constexpr OUStringLiteral UNO_NAME_TABLE_TEMPLATE_NAME = u"TableTemplateName";
inline constexpr OUStringLiteral UNO_NAME_TABSTOPS = u"ParaTabStops";
inline constexpr OUStringLiteral UNO_NAME_TITLE = u"Title";
inline constexpr OUStringLiteral UNO_NAME_TOP_MARGIN = u"TopMargin";
inline constexpr OUStringLiteral UNO_NAME_BOTTOM_MARGIN = u"BottomMargin";
inline constexpr OUStringLiteral UNO_NAME_TRUE_CONTENT = u"TrueContent";
inline constexpr OUStringLiteral UNO_NAME_URL_CONTENT = u"URLContent";
inline constexpr OUStringLiteral UNO_NAME_USERTEXT = u"UserText";
inline constexpr OUStringLiteral UNO_NAME_USER_DATA_TYPE = u"UserDataType";
inline constexpr OUStringLiteral UNO_NAME_VALUE = u"Value";
inline constexpr OUStringLiteral UNO_NAME_VARIABLE_NAME = u"VariableName";
inline constexpr OUStringLiteral UNO_NAME_VARIABLE_SUBTYPE = u"VariableSubtype";
inline constexpr OUStringLiteral UNO_NAME_VERT_ORIENT = u"VertOrient";
inline constexpr OUStringLiteral UNO_NAME_VERT_MIRRORED = u"VertMirrored";
inline constexpr OUStringLiteral UNO_NAME_VERT_ORIENT_POSITION = u"VertOrientPosition";
inline constexpr OUStringLiteral UNO_NAME_VERT_ORIENT_RELATION = u"VertOrientRelation";
inline constexpr OUStringLiteral UNO_NAME_IS_VISIBLE = u"IsVisible";
inline constexpr OUStringLiteral UNO_NAME_WIDTH = u"Width";
inline constexpr OUStringLiteral UNO_NAME_CHAR_WORD_MODE = u"CharWordMode";
inline constexpr OUStringLiteral UNO_NAME_GRAPHIC_CROP = u"GraphicCrop";
#define UNO_NAME_CHARACTER_FORMAT_NONE "CharacterFormatNone"
inline constexpr OUStringLiteral UNO_NAME_DOCUMENT_INDEX_MARK = u"DocumentIndexMark";
inline constexpr OUStringLiteral UNO_NAME_DOCUMENT_INDEX = u"DocumentIndex";
inline constexpr OUStringLiteral UNO_NAME_IS_GLOBAL_DOCUMENT_SECTION = u"IsGlobalDocumentSection";
inline constexpr OUStringLiteral UNO_NAME_TEXT_FIELD = u"TextField";
inline constexpr OUStringLiteral UNO_NAME_BOOKMARK = u"Bookmark";
inline constexpr OUStringLiteral UNO_NAME_BOOKMARK_HIDDEN = u"BookmarkHidden";
inline constexpr OUStringLiteral UNO_NAME_BOOKMARK_CONDITION = u"BookmarkCondition";
inline constexpr OUStringLiteral UNO_NAME_TEXT_TABLE = u"TextTable";
inline constexpr OUStringLiteral UNO_NAME_CELL = u"Cell";
inline constexpr OUStringLiteral UNO_NAME_TEXT_FRAME = u"TextFrame";
inline constexpr OUStringLiteral UNO_NAME_REFERENCE_MARK = u"ReferenceMark";
inline constexpr OUStringLiteral UNO_NAME_TEXT_SECTION = u"TextSection";
inline constexpr OUStringLiteral UNO_NAME_FOOTNOTE = u"Footnote";
inline constexpr OUStringLiteral UNO_NAME_ENDNOTE = u"Endnote";
inline constexpr OUStringLiteral UNO_NAME_CHART_ROW_AS_LABEL = u"ChartRowAsLabel";
inline constexpr OUStringLiteral UNO_NAME_CHART_COLUMN_AS_LABEL = u"ChartColumnAsLabel";
inline constexpr OUStringLiteral UNO_NAME_LEFT_BORDER = u"LeftBorder";
inline constexpr OUStringLiteral UNO_NAME_RIGHT_BORDER = u"RightBorder";
inline constexpr OUStringLiteral UNO_NAME_TOP_BORDER = u"TopBorder";
inline constexpr OUStringLiteral UNO_NAME_BOTTOM_BORDER = u"BottomBorder";
inline constexpr OUStringLiteral UNO_NAME_BORDER_DISTANCE = u"BorderDistance";
inline constexpr OUStringLiteral UNO_NAME_LEFT_BORDER_DISTANCE = u"LeftBorderDistance";
inline constexpr OUStringLiteral UNO_NAME_RIGHT_BORDER_DISTANCE = u"RightBorderDistance";
inline constexpr OUStringLiteral UNO_NAME_TOP_BORDER_DISTANCE = u"TopBorderDistance";
inline constexpr OUStringLiteral UNO_NAME_BOTTOM_BORDER_DISTANCE = u"BottomBorderDistance";
inline constexpr OUStringLiteral UNO_NAME_TABLE_BORDER = u"TableBorder";
inline constexpr OUStringLiteral UNO_NAME_TABLE_COLUMN_SEPARATORS = u"TableColumnSeparators";
inline constexpr OUStringLiteral UNO_NAME_TABLE_COLUMN_RELATIVE_SUM = u"TableColumnRelativeSum";
inline constexpr OUStringLiteral UNO_NAME_HEADER_TEXT = u"HeaderText";
inline constexpr OUStringLiteral UNO_NAME_HEADER_TEXT_LEFT = u"HeaderTextLeft";
inline constexpr OUStringLiteral UNO_NAME_HEADER_TEXT_RIGHT = u"HeaderTextRight";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_TEXT = u"FooterText";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_TEXT_LEFT = u"FooterTextLeft";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_TEXT_RIGHT = u"FooterTextRight";
inline constexpr OUStringLiteral UNO_NAME_HEADER_BACK_COLOR = u"HeaderBackColor";
inline constexpr OUStringLiteral UNO_NAME_HEADER_BACK_TRANSPARENT = u"HeaderBackTransparent";
inline constexpr OUStringLiteral UNO_NAME_HEADER_LEFT_BORDER = u"HeaderLeftBorder";
inline constexpr OUStringLiteral UNO_NAME_HEADER_RIGHT_BORDER = u"HeaderRightBorder";
inline constexpr OUStringLiteral UNO_NAME_HEADER_TOP_BORDER = u"HeaderTopBorder";
inline constexpr OUStringLiteral UNO_NAME_HEADER_BOTTOM_BORDER = u"HeaderBottomBorder";
inline constexpr OUStringLiteral UNO_NAME_HEADER_BORDER_DISTANCE = u"HeaderBorderDistance";
inline constexpr OUStringLiteral UNO_NAME_HEADER_SHADOW_FORMAT = u"HeaderShadowFormat";
inline constexpr OUStringLiteral UNO_NAME_HEADER_BODY_DISTANCE = u"HeaderBodyDistance";
inline constexpr OUStringLiteral UNO_NAME_HEADER_IS_DYNAMIC_HEIGHT = u"HeaderIsDynamicHeight";
inline constexpr OUStringLiteral UNO_NAME_HEADER_IS_SHARED = u"HeaderIsShared";
inline constexpr OUStringLiteral UNO_NAME_HEADER_HEIGHT = u"HeaderHeight";
inline constexpr OUStringLiteral UNO_NAME_HEADER_IS_ON = u"HeaderIsOn";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_BACK_COLOR = u"FooterBackColor";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_BACK_TRANSPARENT = u"FooterBackTransparent";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_LEFT_BORDER = u"FooterLeftBorder";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_RIGHT_BORDER = u"FooterRightBorder";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_TOP_BORDER = u"FooterTopBorder";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_BOTTOM_BORDER = u"FooterBottomBorder";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_BORDER_DISTANCE = u"FooterBorderDistance";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_SHADOW_FORMAT = u"FooterShadowFormat";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_BODY_DISTANCE = u"FooterBodyDistance";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_IS_DYNAMIC_HEIGHT = u"FooterIsDynamicHeight";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_IS_SHARED = u"FooterIsShared";
inline constexpr OUStringLiteral UNO_NAME_TEXT_PARAGRAPH = u"TextParagraph";
inline constexpr OUStringLiteral UNO_NAME_PARENT_TEXT = u"ParentText";

inline constexpr OUStringLiteral UNO_NAME_FOOTER_HEIGHT = u"FooterHeight";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_IS_ON = u"FooterIsOn";
inline constexpr OUStringLiteral UNO_NAME_OVERWRITE_STYLES = u"OverwriteStyles";
inline constexpr OUStringLiteral UNO_NAME_LOAD_NUMBERING_STYLES = u"LoadNumberingStyles";
inline constexpr OUStringLiteral UNO_NAME_LOAD_PAGE_STYLES = u"LoadPageStyles";
inline constexpr OUStringLiteral UNO_NAME_LOAD_FRAME_STYLES = u"LoadFrameStyles";
inline constexpr OUStringLiteral UNO_NAME_LOAD_TEXT_STYLES = u"LoadTextStyles";
#define UNO_NAME_FILE_NAME "FileName"
#define UNO_NAME_COPY_COUNT "CopyCount"
#define UNO_NAME_COLLATE "Collate"
#define UNO_NAME_SORT "Sort"
#define UNO_NAME_PAGES "Pages"
inline constexpr OUStringLiteral UNO_NAME_FIRST_LINE_OFFSET = u"FirstLineOffset";
inline constexpr OUStringLiteral UNO_NAME_SYMBOL_TEXT_DISTANCE = u"SymbolTextDistance";
inline constexpr OUStringLiteral UNO_NAME_USER_INDEX_NAME = u"UserIndexName";
inline constexpr OUStringLiteral UNO_NAME_REVISION = u"Revision";
inline constexpr OUStringLiteral UNO_NAME_UNVISITED_CHAR_STYLE_NAME = u"UnvisitedCharStyleName";
inline constexpr OUStringLiteral UNO_NAME_VISITED_CHAR_STYLE_NAME = u"VisitedCharStyleName";
inline constexpr OUStringLiteral UNO_NAME_PARAGRAPH_COUNT = u"ParagraphCount";
inline constexpr OUStringLiteral UNO_NAME_WORD_COUNT = u"WordCount";
inline constexpr OUStringLiteral UNO_NAME_WORD_SEPARATOR = u"WordSeparator";
inline constexpr OUStringLiteral UNO_NAME_CHARACTER_COUNT = u"CharacterCount";
inline constexpr OUStringLiteral UNO_NAME_ZOOM_VALUE = u"ZoomValue";
inline constexpr OUStringLiteral UNO_NAME_ZOOM_TYPE = u"ZoomType";
inline constexpr OUStringLiteral UNO_NAME_CREATE_FROM_MARKS = u"CreateFromMarks";
inline constexpr OUStringLiteral UNO_NAME_CREATE_FROM_OUTLINE = u"CreateFromOutline";
inline constexpr OUStringLiteral UNO_NAME_CREATE_FROM_CHAPTER = u"CreateFromChapter";
inline constexpr OUStringLiteral UNO_NAME_CREATE_FROM_LABELS = u"CreateFromLabels";
inline constexpr OUStringLiteral UNO_NAME_USE_ALPHABETICAL_SEPARATORS
    = u"UseAlphabeticalSeparators";
inline constexpr OUStringLiteral UNO_NAME_USE_KEY_AS_ENTRY = u"UseKeyAsEntry";
inline constexpr OUStringLiteral UNO_NAME_USE_COMBINED_ENTRIES = u"UseCombinedEntries";
inline constexpr OUStringLiteral UNO_NAME_IS_CASE_SENSITIVE = u"IsCaseSensitive";
inline constexpr OUStringLiteral UNO_NAME_USE_P_P = u"UsePP";
inline constexpr OUStringLiteral UNO_NAME_USE_DASH = u"UseDash";
inline constexpr OUStringLiteral UNO_NAME_USE_UPPER_CASE = u"UseUpperCase";
inline constexpr OUStringLiteral UNO_NAME_LABEL_CATEGORY = u"LabelCategory";
inline constexpr OUStringLiteral UNO_NAME_LABEL_DISPLAY_TYPE = u"LabelDisplayType";
inline constexpr OUStringLiteral UNO_NAME_USE_LEVEL_FROM_SOURCE = u"UseLevelFromSource";
inline constexpr OUStringLiteral UNO_NAME_LEVEL_FORMAT = u"LevelFormat";
inline constexpr OUStringLiteral UNO_NAME_LEVEL_PARAGRAPH_STYLES = u"LevelParagraphStyles";
inline constexpr OUStringLiteral UNO_NAME_MAIN_ENTRY_CHARACTER_STYLE_NAME
    = u"MainEntryCharacterStyleName";
inline constexpr OUStringLiteral UNO_NAME_CREATE_FROM_TABLES = u"CreateFromTables";
inline constexpr OUStringLiteral UNO_NAME_CREATE_FROM_TEXT_FRAMES = u"CreateFromTextFrames";
inline constexpr OUStringLiteral UNO_NAME_CREATE_FROM_GRAPHIC_OBJECTS = u"CreateFromGraphicObjects";
inline constexpr OUStringLiteral UNO_NAME_CREATE_FROM_EMBEDDED_OBJECTS
    = u"CreateFromEmbeddedObjects";
inline constexpr OUStringLiteral UNO_NAME_CREATE_FROM_STAR_MATH = u"CreateFromStarMath";
inline constexpr OUStringLiteral UNO_NAME_CREATE_FROM_STAR_CHART = u"CreateFromStarChart";
inline constexpr OUStringLiteral UNO_NAME_CREATE_FROM_STAR_CALC = u"CreateFromStarCalc";
inline constexpr OUStringLiteral UNO_NAME_CREATE_FROM_STAR_DRAW = u"CreateFromStarDraw";
inline constexpr OUStringLiteral UNO_NAME_CREATE_FROM_OTHER_EMBEDDED_OBJECTS
    = u"CreateFromOtherEmbeddedObjects";
inline constexpr OUStringLiteral UNO_NAME_INDEX_AUTO_MARK_FILE_U_R_L = u"IndexAutoMarkFileURL";
inline constexpr OUStringLiteral UNO_NAME_IS_COMMA_SEPARATED = u"IsCommaSeparated";
inline constexpr OUStringLiteral UNO_NAME_IS_RELATIVE_TABSTOPS = u"IsRelativeTabstops";
inline constexpr OUStringLiteral UNO_NAME_CREATE_FROM_LEVEL_PARAGRAPH_STYLES
    = u"CreateFromLevelParagraphStyles";
inline constexpr OUStringLiteral UNO_NAME_SHOW_CHANGES = u"ShowChanges";
inline constexpr OUStringLiteral UNO_NAME_RECORD_CHANGES = u"RecordChanges";
inline constexpr OUStringLiteral UNO_LINK_DISPLAY_NAME = u"LinkDisplayName";
inline constexpr OUStringLiteral UNO_LINK_DISPLAY_BITMAP = u"LinkDisplayBitmap";
inline constexpr OUStringLiteral UNO_NAME_HEADING_STYLE_NAME = u"HeadingStyleName";
inline constexpr OUStringLiteral UNO_NAME_SHOW_ONLINE_LAYOUT = u"ShowOnlineLayout";
inline constexpr OUStringLiteral UNO_NAME_USER_DEFINED_ATTRIBUTES = u"UserDefinedAttributes";
inline constexpr OUStringLiteral UNO_NAME_TEXT_USER_DEFINED_ATTRIBUTES
    = u"TextUserDefinedAttributes";
inline constexpr OUStringLiteral UNO_NAME_FILE_PATH = u"FilePath";
inline constexpr OUStringLiteral UNO_NAME_PARA_CHAPTER_NUMBERING_LEVEL
    = u"ParaChapterNumberingLevel";
inline constexpr OUStringLiteral UNO_NAME_PARA_CONDITIONAL_STYLE_NAME = u"ParaConditionalStyleName";
inline constexpr OUStringLiteral UNO_NAME_CHAPTER_NUMBERING_LEVEL = u"ChapterNumberingLevel";
inline constexpr OUStringLiteral UNO_NAME_NUMBERING_SEPARATOR = u"NumberingSeparator";
inline constexpr OUStringLiteral UNO_NAME_IS_CONTINUOUS_NUMBERING = u"IsContinuousNumbering";
inline constexpr OUStringLiteral UNO_NAME_IS_AUTOMATIC = u"IsAutomatic";
inline constexpr OUStringLiteral UNO_NAME_IS_ABSOLUTE_MARGINS = u"IsAbsoluteMargins";
inline constexpr OUStringLiteral UNO_NAME_CATEGORY = u"Category";
inline constexpr OUStringLiteral UNO_NAME_DEPENDENT_TEXT_FIELDS = u"DependentTextFields";
inline constexpr OUStringLiteral UNO_NAME_CURRENT_PRESENTATION = u"CurrentPresentation";
inline constexpr OUStringLiteral UNO_NAME_ADJUST = u"Adjust";
inline constexpr OUStringLiteral UNO_NAME_INSTANCE_NAME = u"InstanceName";
inline constexpr OUStringLiteral UNO_NAME_TEXT_PORTION_TYPE = u"TextPortionType";
inline constexpr OUStringLiteral UNO_NAME_CONTROL_CHARACTER = u"ControlCharacter";
inline constexpr OUStringLiteral UNO_NAME_IS_COLLAPSED = u"IsCollapsed";
inline constexpr OUStringLiteral UNO_NAME_IS_START = u"IsStart";
inline constexpr OUStringLiteral UNO_NAME_SEQUENCE_NUMBER = u"SequenceNumber";
inline constexpr OUStringLiteral UNO_NAME_REFERENCE_ID = u"ReferenceId";
inline constexpr OUStringLiteral UNO_NAME_HEADER_LEFT_BORDER_DISTANCE = u"HeaderLeftBorderDistance";
inline constexpr OUStringLiteral UNO_NAME_HEADER_RIGHT_BORDER_DISTANCE
    = u"HeaderRightBorderDistance";
inline constexpr OUStringLiteral UNO_NAME_HEADER_TOP_BORDER_DISTANCE = u"HeaderTopBorderDistance";
inline constexpr OUStringLiteral UNO_NAME_HEADER_BOTTOM_BORDER_DISTANCE
    = u"HeaderBottomBorderDistance";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_LEFT_BORDER_DISTANCE = u"FooterLeftBorderDistance";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_RIGHT_BORDER_DISTANCE
    = u"FooterRightBorderDistance";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_TOP_BORDER_DISTANCE = u"FooterTopBorderDistance";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_BOTTOM_BORDER_DISTANCE
    = u"FooterBottomBorderDistance";
inline constexpr OUStringLiteral UNO_NAME_PARA_IS_NUMBERING_RESTART = u"ParaIsNumberingRestart";
inline constexpr OUStringLiteral UNO_NAME_HIDE_FIELD_TIPS = u"HideFieldTips";
inline constexpr OUStringLiteral UNO_NAME_PARA_SHADOW_FORMAT = u"ParaShadowFormat";
inline constexpr OUStringLiteral UNO_NAME_CONTOUR_POLY_POLYGON = u"ContourPolyPolygon";

inline constexpr OUStringLiteral UNO_NAME_IS_PIXEL_CONTOUR = u"IsPixelContour";
inline constexpr OUStringLiteral UNO_NAME_IS_AUTOMATIC_CONTOUR = u"IsAutomaticContour";
inline constexpr OUStringLiteral UNO_NAME_SEPARATOR_LINE_WIDTH = u"SeparatorLineWidth";
inline constexpr OUStringLiteral UNO_NAME_SEPARATOR_LINE_COLOR = u"SeparatorLineColor";
inline constexpr OUStringLiteral UNO_NAME_SEPARATOR_LINE_RELATIVE_HEIGHT
    = u"SeparatorLineRelativeHeight";
inline constexpr OUStringLiteral UNO_NAME_SEPARATOR_LINE_VERTIVAL_ALIGNMENT
    = u"SeparatorLineVerticalAlignment";
inline constexpr OUStringLiteral UNO_NAME_SEPARATOR_LINE_IS_ON = u"SeparatorLineIsOn";
inline constexpr OUStringLiteral UNO_NAME_IS_SKIP_HIDDEN_TEXT = u"IsSkipHiddenText";
inline constexpr OUStringLiteral UNO_NAME_IS_SKIP_PROTECTED_TEXT = u"IsSkipProtectedText";
inline constexpr OUStringLiteral UNO_NAME_DOCUMENT_INDEX_MARKS = u"DocumentIndexMarks";
inline constexpr OUStringLiteral UNO_NAME_FOOTNOTE_IS_COLLECT_AT_TEXT_END
    = u"FootnoteIsCollectAtTextEnd";
inline constexpr OUStringLiteral UNO_NAME_FOOTNOTE_IS_RESTART_NUMBERING
    = u"FootnoteIsRestartNumbering";
inline constexpr OUStringLiteral UNO_NAME_FOOTNOTE_RESTART_NUMBERING_AT
    = u"FootnoteRestartNumberingAt";
inline constexpr OUStringLiteral UNO_NAME_FOOTNOTE_IS_OWN_NUMBERING = u"FootnoteIsOwnNumbering";
inline constexpr OUStringLiteral UNO_NAME_FOOTNOTE_NUMBERING_TYPE = u"FootnoteNumberingType";
inline constexpr OUStringLiteral UNO_NAME_FOOTNOTE_NUMBERING_PREFIX = u"FootnoteNumberingPrefix";
inline constexpr OUStringLiteral UNO_NAME_FOOTNOTE_NUMBERING_SUFFIX = u"FootnoteNumberingSuffix";
inline constexpr OUStringLiteral UNO_NAME_ENDNOTE_IS_COLLECT_AT_TEXT_END
    = u"EndnoteIsCollectAtTextEnd";
inline constexpr OUStringLiteral UNO_NAME_ENDNOTE_IS_RESTART_NUMBERING
    = u"EndnoteIsRestartNumbering";
inline constexpr OUStringLiteral UNO_NAME_ENDNOTE_RESTART_NUMBERING_AT
    = u"EndnoteRestartNumberingAt";
inline constexpr OUStringLiteral UNO_NAME_ENDNOTE_IS_OWN_NUMBERING = u"EndnoteIsOwnNumbering";
inline constexpr OUStringLiteral UNO_NAME_ENDNOTE_NUMBERING_TYPE = u"EndnoteNumberingType";
inline constexpr OUStringLiteral UNO_NAME_ENDNOTE_NUMBERING_PREFIX = u"EndnoteNumberingPrefix";
inline constexpr OUStringLiteral UNO_NAME_ENDNOTE_NUMBERING_SUFFIX = u"EndnoteNumberingSuffix";
inline constexpr OUStringLiteral UNO_NAME_BRACKET_BEFORE = u"BracketBefore";
inline constexpr OUStringLiteral UNO_NAME_BRACKET_AFTER = u"BracketAfter";
inline constexpr OUStringLiteral UNO_NAME_IS_NUMBER_ENTRIES = u"IsNumberEntries";
inline constexpr OUStringLiteral UNO_NAME_IS_SORT_BY_POSITION = u"IsSortByPosition";
inline constexpr OUStringLiteral UNO_NAME_SORT_KEYS = u"SortKeys";
inline constexpr OUStringLiteral UNO_NAME_IS_SORT_ASCENDING = u"IsSortAscending";
inline constexpr OUStringLiteral UNO_NAME_SORT_KEY = u"SortKey";
inline constexpr OUStringLiteral UNO_NAME_FIELDS = u"Fields";
inline constexpr OUStringLiteral UNO_NAME_DATE_TIME_VALUE = u"DateTimeValue";
inline constexpr OUStringLiteral UNO_NAME_IS_ON = u"IsOn";
inline constexpr OUStringLiteral UNO_NAME_Z_ORDER = u"ZOrder";
inline constexpr OUStringLiteral UNO_NAME_CONTENT_SECTION = u"ContentSection";
inline constexpr OUStringLiteral UNO_NAME_HEADER_SECTION = u"HeaderSection";
inline constexpr OUStringLiteral UNO_NAME_PARA_IS_HANGING_PUNCTUATION = u"ParaIsHangingPunctuation";
inline constexpr OUStringLiteral UNO_NAME_PARA_IS_CHARACTER_DISTANCE = u"ParaIsCharacterDistance";
inline constexpr OUStringLiteral UNO_NAME_PARA_IS_FORBIDDEN_RULES = u"ParaIsForbiddenRules";
inline constexpr OUStringLiteral UNO_NAME_PARA_VERT_ALIGNMENT = u"ParaVertAlignment";
inline constexpr OUStringLiteral UNO_NAME_IS_MAIN_ENTRY = u"IsMainEntry";
inline constexpr OUStringLiteral UNO_NAME_GRAPHIC_ROTATION = u"GraphicRotation";
inline constexpr OUStringLiteral UNO_NAME_ADJUST_LUMINANCE = u"AdjustLuminance";
inline constexpr OUStringLiteral UNO_NAME_ADJUST_CONTRAST = u"AdjustContrast";
inline constexpr OUStringLiteral UNO_NAME_ADJUST_RED = u"AdjustRed";
inline constexpr OUStringLiteral UNO_NAME_ADJUST_GREEN = u"AdjustGreen";
inline constexpr OUStringLiteral UNO_NAME_ADJUST_BLUE = u"AdjustBlue";
inline constexpr OUStringLiteral UNO_NAME_GAMMA = u"Gamma";
inline constexpr OUStringLiteral UNO_NAME_GRAPHIC_IS_INVERTED = u"GraphicIsInverted";
inline constexpr OUStringLiteral UNO_NAME_TRANSPARENCY = u"Transparency";
inline constexpr OUStringLiteral UNO_NAME_REDLINE_AUTHOR = u"RedlineAuthor";
inline constexpr OUStringLiteral UNO_NAME_REDLINE_DATE_TIME = u"RedlineDateTime";
inline constexpr OUStringLiteral UNO_NAME_REDLINE_COMMENT = u"RedlineComment";
inline constexpr OUStringLiteral UNO_NAME_REDLINE_DESCRIPTION = u"RedlineDescription";
inline constexpr OUStringLiteral UNO_NAME_REDLINE_TYPE = u"RedlineType";
inline constexpr OUStringLiteral UNO_NAME_REDLINE_SUCCESSOR_DATA = u"RedlineSuccessorData";
inline constexpr OUStringLiteral UNO_NAME_REDLINE_IDENTIFIER = u"RedlineIdentifier";
inline constexpr OUStringLiteral UNO_NAME_IS_IN_HEADER_FOOTER = u"IsInHeaderFooter";
inline constexpr OUStringLiteral UNO_NAME_START_REDLINE = u"StartRedline";
inline constexpr OUStringLiteral UNO_NAME_END_REDLINE = u"EndRedline";
inline constexpr OUStringLiteral UNO_NAME_REDLINE_START = u"RedlineStart";
inline constexpr OUStringLiteral UNO_NAME_REDLINE_END = u"RedlineEnd";
inline constexpr OUStringLiteral UNO_NAME_REDLINE_TEXT = u"RedlineText";
inline constexpr OUStringLiteral UNO_NAME_REDLINE_DISPLAY_TYPE = u"RedlineDisplayType";
inline constexpr OUStringLiteral UNO_NAME_FORBIDDEN_CHARACTERS = u"ForbiddenCharacters";
inline constexpr OUStringLiteral UNO_NAME_RUBY_BASE_TEXT = u"RubyBaseText";
inline constexpr OUStringLiteral UNO_NAME_RUBY_TEXT = u"RubyText";
inline constexpr OUStringLiteral UNO_NAME_RUBY_ADJUST = u"RubyAdjust";
inline constexpr OUStringLiteral UNO_NAME_RUBY_CHAR_STYLE_NAME = u"RubyCharStyleName";
inline constexpr OUStringLiteral UNO_NAME_RUBY_IS_ABOVE = u"RubyIsAbove";
inline constexpr OUStringLiteral UNO_NAME_RUBY_POSITION = u"RubyPosition";
inline constexpr OUStringLiteral UNO_NAME_FOOTNOTE_HEIGHT = u"FootnoteHeight";
inline constexpr OUStringLiteral UNO_NAME_FOOTNOTE_LINE_WEIGHT = u"FootnoteLineWeight";
inline constexpr OUStringLiteral UNO_NAME_FOOTNOTE_LINE_COLOR = u"FootnoteLineColor";
inline constexpr OUStringLiteral UNO_NAME_FOOTNOTE_LINE_RELATIVE_WIDTH
    = u"FootnoteLineRelativeWidth";
inline constexpr OUStringLiteral UNO_NAME_FOOTNOTE_LINE_ADJUST = u"FootnoteLineAdjust";
inline constexpr OUStringLiteral UNO_NAME_FOOTNOTE_LINE_TEXT_DISTANCE = u"FootnoteLineTextDistance";
inline constexpr OUStringLiteral UNO_NAME_FOOTNOTE_LINE_DISTANCE = u"FootnoteLineDistance";
inline constexpr OUStringLiteral UNO_NAME_CHAR_ROTATION = u"CharRotation";
inline constexpr OUStringLiteral UNO_NAME_CHAR_ROTATION_IS_FIT_TO_LINE = u"CharRotationIsFitToLine";
inline constexpr OUStringLiteral UNO_NAME_CHAR_SCALE_WIDTH = u"CharScaleWidth";
inline constexpr OUStringLiteral UNO_NAME_TAB_STOP_DISTANCE = u"TabStopDistance";
inline constexpr OUStringLiteral UNO_NAME_IS_WIDTH_RELATIVE = u"IsWidthRelative";
inline constexpr OUStringLiteral UNO_NAME_CHAR_RELIEF = u"CharRelief";
inline constexpr OUStringLiteral UNO_NAME_IS_HIDDEN = u"IsHidden";
inline constexpr OUStringLiteral UNO_NAME_IS_CONDITION_TRUE = u"IsConditionTrue";
inline constexpr OUStringLiteral UNO_NAME_TWO_DIGIT_YEAR = u"TwoDigitYear";
inline constexpr OUStringLiteral UNO_NAME_PROTECTION_KEY = u"ProtectionKey";
inline constexpr OUStringLiteral UNO_NAME_REDLINE_PROTECTION_KEY = u"RedlineProtectionKey";
inline constexpr OUStringLiteral UNO_NAME_AUTOMATIC_DISTANCE = u"AutomaticDistance";
inline constexpr OUStringLiteral UNO_NAME_AUTOMATIC_CONTROL_FOCUS = u"AutomaticControlFocus";
inline constexpr OUStringLiteral UNO_NAME_APPLY_FORM_DESIGN_MODE = u"ApplyFormDesignMode";
inline constexpr OUStringLiteral UNO_NAME_LOCALE = u"Locale";
inline constexpr OUStringLiteral UNO_NAME_SORT_ALGORITHM = u"SortAlgorithm";
inline constexpr OUStringLiteral UNO_NAME_FRAME_HEIGHT_ABSOLUTE = u"FrameHeightAbsolute";
inline constexpr OUStringLiteral UNO_NAME_FRAME_HEIGHT_PERCENT = u"FrameHeightPercent";
inline constexpr OUStringLiteral UNO_NAME_FRAME_ISAUTOMATIC_HEIGHT = u"FrameIsAutomaticHeight";
inline constexpr OUStringLiteral UNO_NAME_FRAME_WIDTH_ABSOLUTE = u"FrameWidthAbsolute";
inline constexpr OUStringLiteral UNO_NAME_FRAME_WIDTH_PERCENT = u"FrameWidthPercent";

// names for FillAttributes from SVX
#define UNO_NAME_SW_FILLBMP_LOGICAL_SIZE UNO_NAME_FILLBMP_LOGICAL_SIZE
#define UNO_NAME_SW_FILLBMP_OFFSET_X UNO_NAME_FILLBMP_OFFSET_X
#define UNO_NAME_SW_FILLBMP_OFFSET_Y UNO_NAME_FILLBMP_OFFSET_Y
#define UNO_NAME_SW_FILLBMP_POSITION_OFFSET_X UNO_NAME_FILLBMP_POSITION_OFFSET_X
#define UNO_NAME_SW_FILLBMP_POSITION_OFFSET_Y UNO_NAME_FILLBMP_POSITION_OFFSET_Y
#define UNO_NAME_SW_FILLBMP_RECTANGLE_POINT UNO_NAME_FILLBMP_RECTANGLE_POINT
#define UNO_NAME_SW_FILLBMP_SIZE_X UNO_NAME_FILLBMP_SIZE_X
#define UNO_NAME_SW_FILLBMP_SIZE_Y UNO_NAME_FILLBMP_SIZE_Y
#define UNO_NAME_SW_FILLBMP_STRETCH UNO_NAME_FILLBMP_STRETCH
#define UNO_NAME_SW_FILLBMP_TILE UNO_NAME_FILLBMP_TILE
#define UNO_NAME_SW_FILLBMP_MODE UNO_NAME_FILLBMP_MODE
#define UNO_NAME_SW_FILLCOLOR UNO_NAME_FILLCOLOR
#define UNO_NAME_SW_FILLBACKGROUND UNO_NAME_FILLBACKGROUND
#define UNO_NAME_SW_FILLBITMAP UNO_NAME_FILLBITMAP
#define UNO_NAME_SW_FILLBITMAPURL UNO_NAME_FILLBITMAPURL
#define UNO_NAME_SW_FILLBITMAPNAME UNO_NAME_FILLBITMAPNAME
#define UNO_NAME_SW_FILLGRADIENTSTEPCOUNT UNO_NAME_FILLGRADIENTSTEPCOUNT
#define UNO_NAME_SW_FILLGRADIENT UNO_NAME_FILLGRADIENT
#define UNO_NAME_SW_FILLGRADIENTNAME UNO_NAME_FILLGRADIENTNAME
#define UNO_NAME_SW_FILLHATCH UNO_NAME_FILLHATCH
#define UNO_NAME_SW_FILLHATCHNAME UNO_NAME_FILLHATCHNAME
#define UNO_NAME_SW_FILLSTYLE UNO_NAME_FILLSTYLE
#define UNO_NAME_SW_FILL_TRANSPARENCE UNO_NAME_FILL_TRANSPARENCE
#define UNO_NAME_SW_FILLTRANSPARENCEGRADIENT UNO_NAME_FILLTRANSPARENCEGRADIENT
#define UNO_NAME_SW_FILLTRANSPARENCEGRADIENTNAME UNO_NAME_FILLTRANSPARENCEGRADIENTNAME
#define UNO_NAME_SW_FILLCOLOR_2 UNO_NAME_FILLCOLOR_2

inline constexpr OUStringLiteral UNO_NAME_HEADER_FILLBMP_LOGICAL_SIZE
    = u"HeaderFillBitmapLogicalSize";
inline constexpr OUStringLiteral UNO_NAME_HEADER_FILLBMP_OFFSET_X = u"HeaderFillBitmapOffsetX";
inline constexpr OUStringLiteral UNO_NAME_HEADER_FILLBMP_OFFSET_Y = u"HeaderFillBitmapOffsetY";
inline constexpr OUStringLiteral UNO_NAME_HEADER_FILLBMP_POSITION_OFFSET_X
    = u"HeaderFillBitmapPositionOffsetX";
inline constexpr OUStringLiteral UNO_NAME_HEADER_FILLBMP_POSITION_OFFSET_Y
    = u"HeaderFillBitmapPositionOffsetY";
inline constexpr OUStringLiteral UNO_NAME_HEADER_FILLBMP_RECTANGLE_POINT
    = u"HeaderFillBitmapRectanglePoint";
inline constexpr OUStringLiteral UNO_NAME_HEADER_FILLBMP_SIZE_X = u"HeaderFillBitmapSizeX";
inline constexpr OUStringLiteral UNO_NAME_HEADER_FILLBMP_SIZE_Y = u"HeaderFillBitmapSizeY";
inline constexpr OUStringLiteral UNO_NAME_HEADER_FILLBMP_STRETCH = u"HeaderFillBitmapStretch";
inline constexpr OUStringLiteral UNO_NAME_HEADER_FILLBMP_TILE = u"HeaderFillBitmapTile";
inline constexpr OUStringLiteral UNO_NAME_HEADER_FILLBMP_MODE = u"HeaderFillBitmapMode";
inline constexpr OUStringLiteral UNO_NAME_HEADER_FILLCOLOR = u"HeaderFillColor";
inline constexpr OUStringLiteral UNO_NAME_HEADER_FILLBACKGROUND = u"HeaderFillBackground";
inline constexpr OUStringLiteral UNO_NAME_HEADER_FILLBITMAP = u"HeaderFillBitmap";
inline constexpr OUStringLiteral UNO_NAME_HEADER_FILLBITMAPNAME = u"HeaderFillBitmapName";
inline constexpr OUStringLiteral UNO_NAME_HEADER_FILLGRADIENTSTEPCOUNT
    = u"HeaderFillGradientStepCount";
inline constexpr OUStringLiteral UNO_NAME_HEADER_FILLGRADIENT = u"HeaderFillGradient";
inline constexpr OUStringLiteral UNO_NAME_HEADER_FILLGRADIENTNAME = u"HeaderFillGradientName";
inline constexpr OUStringLiteral UNO_NAME_HEADER_FILLHATCH = u"HeaderFillHatch";
inline constexpr OUStringLiteral UNO_NAME_HEADER_FILLHATCHNAME = u"HeaderFillHatchName";
inline constexpr OUStringLiteral UNO_NAME_HEADER_FILLSTYLE = u"HeaderFillStyle";
inline constexpr OUStringLiteral UNO_NAME_HEADER_FILL_TRANSPARENCE = u"HeaderFillTransparence";
inline constexpr OUStringLiteral UNO_NAME_HEADER_FILLTRANSPARENCEGRADIENT
    = u"HeaderFillTransparenceGradient";
inline constexpr OUStringLiteral UNO_NAME_HEADER_FILLTRANSPARENCEGRADIENTNAME
    = u"HeaderFillTransparenceGradientName";
inline constexpr OUStringLiteral UNO_NAME_HEADER_FILLCOLOR_2 = u"HeaderFillColor2";

inline constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBMP_LOGICAL_SIZE
    = u"FooterFillBitmapLogicalSize";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBMP_OFFSET_X = u"FooterFillBitmapOffsetX";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBMP_OFFSET_Y = u"FooterFillBitmapOffsetY";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBMP_POSITION_OFFSET_X
    = u"FooterFillBitmapPositionOffsetX";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBMP_POSITION_OFFSET_Y
    = u"FooterFillBitmapPositionOffsetY";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBMP_RECTANGLE_POINT
    = u"FooterFillBitmapRectanglePoint";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBMP_SIZE_X = u"FooterFillBitmapSizeX";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBMP_SIZE_Y = u"FooterFillBitmapSizeY";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBMP_STRETCH = u"FooterFillBitmapStretch";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBMP_TILE = u"FooterFillBitmapTile";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBMP_MODE = u"FooterFillBitmapMode";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_FILLCOLOR = u"FooterFillColor";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBACKGROUND = u"FooterFillBackground";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBITMAP = u"FooterFillBitmap";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBITMAPNAME = u"FooterFillBitmapName";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_FILLGRADIENTSTEPCOUNT
    = u"FooterFillGradientStepCount";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_FILLGRADIENT = u"FooterFillGradient";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_FILLGRADIENTNAME = u"FooterFillGradientName";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_FILLHATCH = u"FooterFillHatch";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_FILLHATCHNAME = u"FooterFillHatchName";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_FILLSTYLE = u"FooterFillStyle";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_FILL_TRANSPARENCE = u"FooterFillTransparence";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_FILLTRANSPARENCEGRADIENT
    = u"FooterFillTransparenceGradient";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_FILLTRANSPARENCEGRADIENTNAME
    = u"FooterFillTransparenceGradientName";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_FILLCOLOR_2 = u"FooterFillColor2";

inline constexpr OUStringLiteral UNO_NAME_PARA_STYLEHEADING = u"ParaStyleHeading";
inline constexpr OUStringLiteral UNO_NAME_PARA_STYLELEVEL1 = u"ParaStyleLevel1";
inline constexpr OUStringLiteral UNO_NAME_PARA_STYLELEVEL10 = u"ParaStyleLevel10";
inline constexpr OUStringLiteral UNO_NAME_PARA_STYLELEVEL2 = u"ParaStyleLevel2";
inline constexpr OUStringLiteral UNO_NAME_PARA_STYLELEVEL3 = u"ParaStyleLevel3";
inline constexpr OUStringLiteral UNO_NAME_PARA_STYLELEVEL4 = u"ParaStyleLevel4";
inline constexpr OUStringLiteral UNO_NAME_PARA_STYLELEVEL5 = u"ParaStyleLevel5";
inline constexpr OUStringLiteral UNO_NAME_PARA_STYLELEVEL6 = u"ParaStyleLevel6";
inline constexpr OUStringLiteral UNO_NAME_PARA_STYLELEVEL7 = u"ParaStyleLevel7";
inline constexpr OUStringLiteral UNO_NAME_PARA_STYLELEVEL8 = u"ParaStyleLevel8";
inline constexpr OUStringLiteral UNO_NAME_PARA_STYLELEVEL9 = u"ParaStyleLevel9";
inline constexpr OUStringLiteral UNO_NAME_PARA_STYLESEPARATOR = u"ParaStyleSeparator";
inline constexpr OUStringLiteral UNO_NAME_MACRO_LIBRARY = u"MacroLibrary";
inline constexpr OUStringLiteral UNO_NAME_CELL_NAME = u"CellName";
inline constexpr OUStringLiteral UNO_NAME_PARA_USER_DEFINED_ATTRIBUTES
    = u"ParaUserDefinedAttributes";
inline constexpr OUStringLiteral UNO_NAME_MERGE_LAST_PARA = u"MergeLastPara";
inline constexpr OUStringLiteral UNO_NAME_WRITING_MODE = u"WritingMode";
inline constexpr OUStringLiteral UNO_NAME_GRID_COLOR = u"GridColor";
inline constexpr OUStringLiteral UNO_NAME_GRID_LINES = u"GridLines";
inline constexpr OUStringLiteral UNO_NAME_GRID_BASE_HEIGHT = u"GridBaseHeight";
inline constexpr OUStringLiteral UNO_NAME_GRID_RUBY_HEIGHT = u"GridRubyHeight";
inline constexpr OUStringLiteral UNO_NAME_GRID_MODE = u"GridMode";
inline constexpr OUStringLiteral UNO_NAME_GRID_RUBY_BELOW = u"RubyBelow";
inline constexpr OUStringLiteral UNO_NAME_GRID_PRINT = u"GridPrint";
inline constexpr OUStringLiteral UNO_NAME_GRID_DISPLAY = u"GridDisplay";
inline constexpr OUStringLiteral UNO_NAME_SNAP_TO_GRID = u"SnapToGrid";
inline constexpr OUStringLiteral UNO_NAME_DONT_BALANCE_TEXT_COLUMNS = u"DontBalanceTextColumns";
inline constexpr OUStringLiteral UNO_NAME_IS_CURRENTLY_VISIBLE = u"IsCurrentlyVisible";
inline constexpr OUStringLiteral UNO_NAME_ANCHOR_POSITION = u"AnchorPosition";
inline constexpr OUStringLiteral UNO_NAME_TEXT_READING = u"TextReading";
inline constexpr OUStringLiteral UNO_NAME_PRIMARY_KEY_READING = u"PrimaryKeyReading";
inline constexpr OUStringLiteral UNO_NAME_SECONDARY_KEY_READING = u"SecondaryKeyReading";
inline constexpr OUStringLiteral UNO_NAME_BACK_GRAPHIC_TRANSPARENCY = u"BackGraphicTransparency";
inline constexpr OUStringLiteral UNO_NAME_BACK_COLOR_R_G_B = u"BackColorRGB";
inline constexpr OUStringLiteral UNO_NAME_BACK_COLOR_TRANSPARENCY = u"BackColorTransparency";
inline constexpr OUStringLiteral UNO_NAME_HEADER_DYNAMIC_SPACING = u"HeaderDynamicSpacing";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_DYNAMIC_SPACING = u"FooterDynamicSpacing";
inline constexpr OUStringLiteral UNO_NAME_BASIC_LIBRARIES = u"BasicLibraries";
inline constexpr OUStringLiteral UNO_NAME_IS_FIXED_LANGUAGE = u"IsFixedLanguage";
inline constexpr OUStringLiteral UNO_NAME_SELECTION = u"Selection";
inline constexpr OUStringLiteral UNO_NAME_RESULT_SET = u"ResultSet";
inline constexpr OUStringLiteral UNO_NAME_CONNECTION = u"ActiveConnection";
inline constexpr OUStringLiteral UNO_NAME_MODEL = u"Model";
inline constexpr OUStringLiteral UNO_NAME_DATA_SOURCE_NAME = u"DataSourceName";
inline constexpr OUStringLiteral UNO_NAME_FILTER = u"Filter";
inline constexpr OUStringLiteral UNO_NAME_DOCUMENT_URL = u"DocumentURL";
inline constexpr OUStringLiteral UNO_NAME_OUTPUT_URL = u"OutputURL";
inline constexpr OUStringLiteral UNO_NAME_OUTPUT_TYPE = u"OutputType";
inline constexpr OUStringLiteral UNO_NAME_ESCAPE_PROCESSING = u"EscapeProcessing";
inline constexpr OUStringLiteral UNO_NAME_SINGLE_PRINT_JOBS = u"SinglePrintJobs";
inline constexpr OUStringLiteral UNO_NAME_FILE_NAME_FROM_COLUMN = u"FileNameFromColumn";
inline constexpr OUStringLiteral UNO_NAME_FILE_NAME_PREFIX = u"FileNamePrefix";
inline constexpr OUStringLiteral UNO_NAME_CHAR_STYLE_NAMES = u"CharStyleNames";
inline constexpr OUStringLiteral UNO_NAME_DAD_COMMAND
    = u"Command"; /* DAD = DataAccessDescriptor (see com.sun.star.sdb.DataAccessDescriptor) */
inline constexpr OUStringLiteral UNO_NAME_DAD_COMMAND_TYPE = u"CommandType";
inline constexpr OUStringLiteral UNO_NAME_CLSID = u"CLSID";
inline constexpr OUStringLiteral UNO_NAME_COMPONENT = u"Component";
inline constexpr OUStringLiteral UNO_NAME_SECT_LEFT_MARGIN = u"SectionLeftMargin";
inline constexpr OUStringLiteral UNO_NAME_SECT_RIGHT_MARGIN = u"SectionRightMargin";
inline constexpr OUStringLiteral UNO_NAME_PARA_IS_CONNECT_BORDER = u"ParaIsConnectBorder";
inline constexpr OUStringLiteral UNO_NAME_ITEMS = u"Items";
inline constexpr OUStringLiteral UNO_NAME_SELITEM = u"SelectedItem";
inline constexpr OUStringLiteral UNO_NAME_IS_SPLIT_ALLOWED = u"IsSplitAllowed";
inline constexpr OUStringLiteral UNO_NAME_HAS_TEXT_CHANGES_ONLY = u"HasTextChangesOnly";
inline constexpr OUStringLiteral UNO_NAME_CHAR_HIDDEN = u"CharHidden";
inline constexpr OUStringLiteral UNO_NAME_IS_FOLLOWING_TEXT_FLOW = u"IsFollowingTextFlow";
inline constexpr OUStringLiteral UNO_NAME_WIDTH_TYPE = u"WidthType";
inline constexpr OUStringLiteral UNO_NAME_SCRIPT_URL = u"ScriptURL";
inline constexpr OUStringLiteral UNO_NAME_RUNTIME_UID = u"RuntimeUID";
inline constexpr OUStringLiteral UNO_NAME_COLLAPSING_BORDERS = u"CollapsingBorders";
inline constexpr OUStringLiteral UNO_NAME_WRAP_INFLUENCE_ON_POSITION = u"WrapInfluenceOnPosition";
inline constexpr OUStringLiteral UNO_NAME_DATA_BASE_URL = u"DataBaseURL";
inline constexpr OUStringLiteral UNO_NAME_TRANSFORMATION_IN_HORI_L2R = u"TransformationInHoriL2R";
inline constexpr OUStringLiteral UNO_NAME_POSITION_LAYOUT_DIR = u"PositionLayoutDir";
inline constexpr OUStringLiteral UNO_NAME_NUMBERING_IS_OUTLINE = u"NumberingIsOutline";
inline constexpr OUStringLiteral UNO_NAME_STARTPOSITION_IN_HORI_L2R = u"StartPositionInHoriL2R";
inline constexpr OUStringLiteral UNO_NAME_ENDPOSITION_IN_HORI_L2R = u"EndPositionInHoriL2R";
inline constexpr OUStringLiteral UNO_NAME_SUBJECT = u"Subject";
inline constexpr OUStringLiteral UNO_NAME_ADDRESS_FROM_COLUMN = u"AddressFromColumn";
inline constexpr OUStringLiteral UNO_NAME_SEND_AS_HTML = u"SendAsHTML";
inline constexpr OUStringLiteral UNO_NAME_SEND_AS_ATTACHMENT = u"SendAsAttachment";
inline constexpr OUStringLiteral UNO_NAME_MAIL_BODY = u"MailBody";
inline constexpr OUStringLiteral UNO_NAME_ATTACHMENT_NAME = u"AttachmentName";
inline constexpr OUStringLiteral UNO_NAME_ATTACHMENT_FILTER = u"AttachmentFilter";
inline constexpr OUStringLiteral UNO_NAME_PRINT_OPTIONS = u"PrintOptions";
inline constexpr OUStringLiteral UNO_NAME_SAVE_AS_SINGLE_FILE = u"SaveAsSingleFile";
inline constexpr OUStringLiteral UNO_NAME_SAVE_FILTER = u"SaveFilter";
inline constexpr OUStringLiteral UNO_NAME_COPIES_TO = u"CopiesTo";
inline constexpr OUStringLiteral UNO_NAME_BLIND_COPIES_TO = u"BlindCopiesTo";
inline constexpr OUStringLiteral UNO_NAME_IN_SERVER_PASSWORD = u"InServerPassword";
inline constexpr OUStringLiteral UNO_NAME_OUT_SERVER_PASSWORD = u"OutServerPassword";
inline constexpr OUStringLiteral UNO_NAME_SAVE_FILTER_OPTIONS = u"SaveFilterOptions";
inline constexpr OUStringLiteral UNO_NAME_SAVE_FILTER_DATA = u"SaveFilterData";
inline constexpr OUStringLiteral UNO_NAME_LOCK_UPDATES = u"LockUpdates";
inline constexpr OUStringLiteral UNO_NAME_PAGE_COUNT = u"PageCount";
inline constexpr OUStringLiteral UNO_NAME_LINE_COUNT = u"LineCount";
inline constexpr OUStringLiteral UNO_NAME_IS_CONSTANT_SPELLCHECK = u"IsConstantSpellcheck";
inline constexpr OUStringLiteral UNO_NAME_IS_HIDE_SPELL_MARKS
    = u"IsHideSpellMarks"; /* deprecated #i91949 */

inline constexpr OUStringLiteral UNO_NAME_IS_FIELD_USED = u"IsFieldUsed";
inline constexpr OUStringLiteral UNO_NAME_IS_FIELD_DISPLAYED = u"IsFieldDisplayed";
inline constexpr OUStringLiteral UNO_NAME_BUILDID = u"BuildId";
inline constexpr OUStringLiteral UNO_NAME_PARA_STYLE_CONDITIONS = u"ParaStyleConditions";
inline constexpr OUStringLiteral UNO_NAME_ROLE = u"Role";
inline constexpr OUStringLiteral UNO_NAME_FIELD_CODE = u"FieldCode";
inline constexpr OUStringLiteral UNO_NAME_HAS_VALID_SIGNATURES = u"HasValidSignatures";
inline constexpr OUStringLiteral UNO_NAME_CHAR_AUTO_STYLE_NAME = u"CharAutoStyleName";
inline constexpr OUStringLiteral UNO_NAME_PARA_AUTO_STYLE_NAME = u"ParaAutoStyleName";
inline constexpr OUStringLiteral UNO_NAME_LAYOUT_SIZE = u"LayoutSize";
inline constexpr OUStringLiteral UNO_NAME_HELP = u"Help";
inline constexpr OUStringLiteral UNO_NAME_TOOLTIP = u"Tooltip";
inline constexpr OUStringLiteral UNO_NAME_DIALOG_LIBRARIES = u"DialogLibraries";
inline constexpr OUStringLiteral UNO_NAME_HEADER_ROW_COUNT = u"HeaderRowCount";
inline constexpr OUStringLiteral UNO_NAME_INPUT = u"Input";
inline constexpr OUStringLiteral UNO_NAME_ROW_SPAN = u"RowSpan";
inline constexpr OUStringLiteral UNO_NAME_POSITION_AND_SPACE_MODE = u"PositionAndSpaceMode";
inline constexpr OUStringLiteral UNO_NAME_LABEL_FOLLOWED_BY = u"LabelFollowedBy";
inline constexpr OUStringLiteral UNO_NAME_LISTTAB_STOP_POSITION = u"ListtabStopPosition";
inline constexpr OUStringLiteral UNO_NAME_FIRST_LINE_INDENT = u"FirstLineIndent";
inline constexpr OUStringLiteral UNO_NAME_INDENT_AT = u"IndentAt";
inline constexpr OUStringLiteral UNO_NAME_GRID_BASE_WIDTH = u"GridBaseWidth";
inline constexpr OUStringLiteral UNO_NAME_GRID_SNAP_TO_CHARS = u"GridSnapToChars";
inline constexpr OUStringLiteral UNO_NAME_GRID_STANDARD_PAGE_MODE = u"StandardPageMode";
inline constexpr OUStringLiteral UNO_NAME_TABLE_BORDER_DISTANCES = u"TableBorderDistances";
inline constexpr OUStringLiteral UNO_NAME_LIST_ID = u"ListId";
inline constexpr OUStringLiteral UNO_NAME_DEFAULT_LIST_ID = u"DefaultListId";
inline constexpr OUStringLiteral UNO_NAME_STREAM_NAME = u"StreamName";
inline constexpr OUStringLiteral UNO_NAME_PARA_CONTINUEING_PREVIOUS_SUB_TREE
    = u"ContinueingPreviousSubTree";
inline constexpr OUStringLiteral UNO_NAME_PARA_LIST_LABEL_STRING = u"ListLabelString";
inline constexpr OUStringLiteral UNO_NAME_PARA_LIST_AUTO_FORMAT = u"ListAutoFormat";
inline constexpr OUStringLiteral UNO_NAME_CHAR_OVERLINE = u"CharOverline";
inline constexpr OUStringLiteral UNO_NAME_CHAR_OVERLINE_COLOR = u"CharOverlineColor";
inline constexpr OUStringLiteral UNO_NAME_CHAR_OVERLINE_HAS_COLOR = u"CharOverlineHasColor";
inline constexpr OUStringLiteral UNO_NAME_OUTLINE_CONTENT_VISIBLE = u"OutlineContentVisible";
inline constexpr OUStringLiteral UNO_NAME_OUTLINE_LEVEL = u"OutlineLevel";
inline constexpr OUStringLiteral UNO_NAME_DESCRIPTION = u"Description";
inline constexpr OUStringLiteral UNO_NAME_META = u"InContentMetadata";
inline constexpr OUStringLiteral UNO_NAME_IS_TEMPLATE = u"IsTemplate";
inline constexpr OUStringLiteral UNO_NAME_VBA_DOCOBJ = u"VBAGlobalConstantName";
inline constexpr OUStringLiteral UNO_NAME_NESTED_TEXT_CONTENT = u"NestedTextContent";
inline constexpr OUStringLiteral UNO_NAME_SEPARATOR_LINE_STYLE = u"SeparatorLineStyle";
inline constexpr OUStringLiteral UNO_NAME_FOOTNOTE_LINE_STYLE = u"FootnoteLineStyle";
inline constexpr OUStringLiteral UNO_NAME_EMBEDDED_OBJECT = u"EmbeddedObject";
inline constexpr OUStringLiteral UNO_NAME_RSID = u"Rsid";
inline constexpr OUStringLiteral UNO_NAME_PARRSID = u"ParRsid";
inline constexpr OUStringLiteral UNO_NAME_DRAW_ASPECT = u"DrawAspect";
inline constexpr OUStringLiteral UNO_NAME_VISIBLE_AREA_WIDTH = u"VisibleAreaWidth";
inline constexpr OUStringLiteral UNO_NAME_VISIBLE_AREA_HEIGHT = u"VisibleAreaHeight";

inline constexpr OUStringLiteral UNO_NAME_LINE_STYLE = u"LineStyle";
inline constexpr OUStringLiteral UNO_NAME_LINE_WIDTH = u"LineWidth";

inline constexpr OUStringLiteral UNO_NAME_PARA_CONTEXT_MARGIN = u"ParaContextMargin";
#define UNO_NAME_PARAGRAPH_STYLE_NAME "ParagraphStyleName"
inline constexpr OUStringLiteral UNO_NAME_FIRST_IS_SHARED = u"FirstIsShared";
inline constexpr OUStringLiteral UNO_NAME_HEADER_TEXT_FIRST = u"HeaderTextFirst";
inline constexpr OUStringLiteral UNO_NAME_FOOTER_TEXT_FIRST = u"FooterTextFirst";
inline constexpr OUStringLiteral UNO_NAME_INITIALS = u"Initials";
inline constexpr OUStringLiteral UNO_NAME_TABLE_BORDER2 = u"TableBorder2";
inline constexpr OUStringLiteral UNO_NAME_REPLACEMENT_GRAPHIC = u"ReplacementGraphic";
inline constexpr OUStringLiteral UNO_NAME_HIDDEN = u"Hidden";
inline constexpr OUStringLiteral UNO_NAME_DEFAULT_PAGE_MODE = u"DefaultPageMode";
inline constexpr OUStringLiteral UNO_NAME_CHAR_SHADING_VALUE = u"CharShadingValue";
inline constexpr OUStringLiteral UNO_NAME_PARA_INTEROP_GRAB_BAG = u"ParaInteropGrabBag";
inline constexpr OUStringLiteral UNO_NAME_CHAR_LEFT_BORDER = u"CharLeftBorder";
inline constexpr OUStringLiteral UNO_NAME_CHAR_RIGHT_BORDER = u"CharRightBorder";
inline constexpr OUStringLiteral UNO_NAME_CHAR_TOP_BORDER = u"CharTopBorder";
inline constexpr OUStringLiteral UNO_NAME_CHAR_BOTTOM_BORDER = u"CharBottomBorder";
inline constexpr OUStringLiteral UNO_NAME_CHAR_BORDER_DISTANCE = u"CharBorderDistance";
inline constexpr OUStringLiteral UNO_NAME_CHAR_LEFT_BORDER_DISTANCE = u"CharLeftBorderDistance";
inline constexpr OUStringLiteral UNO_NAME_CHAR_RIGHT_BORDER_DISTANCE = u"CharRightBorderDistance";
inline constexpr OUStringLiteral UNO_NAME_CHAR_TOP_BORDER_DISTANCE = u"CharTopBorderDistance";
inline constexpr OUStringLiteral UNO_NAME_CHAR_BOTTOM_BORDER_DISTANCE = u"CharBottomBorderDistance";
inline constexpr OUStringLiteral UNO_NAME_CHAR_SHADOW_FORMAT = u"CharShadowFormat";
inline constexpr OUStringLiteral UNO_NAME_SHADOW_TRANSPARENCE = u"ShadowTransparence";
inline constexpr OUStringLiteral UNO_NAME_DOC_INTEROP_GRAB_BAG = u"InteropGrabBag";
inline constexpr OUStringLiteral UNO_NAME_FRAME_INTEROP_GRAB_BAG = u"FrameInteropGrabBag";
inline constexpr OUStringLiteral UNO_NAME_CHAR_HIGHLIGHT = u"CharHighlight";
inline constexpr OUStringLiteral UNO_NAME_STYLE_INTEROP_GRAB_BAG = u"StyleInteropGrabBag";
inline constexpr OUStringLiteral UNO_NAME_CHAR_INTEROP_GRAB_BAG = u"CharInteropGrabBag";
inline constexpr OUStringLiteral UNO_NAME_TEXT_VERT_ADJUST = u"TextVerticalAdjust";
inline constexpr OUStringLiteral UNO_NAME_CELL_INTEROP_GRAB_BAG = u"CellInteropGrabBag";
inline constexpr OUStringLiteral UNO_NAME_ROW_INTEROP_GRAB_BAG = u"RowInteropGrabBag";
inline constexpr OUStringLiteral UNO_NAME_TABLE_INTEROP_GRAB_BAG = u"TableInteropGrabBag";

inline constexpr OUStringLiteral UNO_NAME_TABLE_FIRST_ROW_END_COLUMN = u"FirstRowEndColumn";
inline constexpr OUStringLiteral UNO_NAME_TABLE_FIRST_ROW_START_COLUMN = u"FirstRowStartColumn";
inline constexpr OUStringLiteral UNO_NAME_TABLE_LAST_ROW_END_COLUMN = u"LastRowEndColumn";
inline constexpr OUStringLiteral UNO_NAME_TABLE_LAST_ROW_START_COLUMN = u"LastRowStartColumn";

inline constexpr OUStringLiteral UNO_NAME_RESOLVED = u"Resolved";
inline constexpr OUStringLiteral UNO_NAME_ALLOW_OVERLAP = u"AllowOverlap";
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

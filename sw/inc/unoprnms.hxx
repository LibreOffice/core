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

constexpr OUStringLiteral UNO_NAME_FOLLOW_STYLE = u"FollowStyle";
constexpr OUStringLiteral UNO_NAME_LINK_STYLE = u"LinkStyle";
constexpr OUStringLiteral UNO_NAME_IS_PHYSICAL = u"IsPhysical";
constexpr OUStringLiteral UNO_NAME_IS_AUTO_UPDATE = u"IsAutoUpdate";
constexpr OUStringLiteral UNO_NAME_DISPLAY_NAME = u"DisplayName";
constexpr OUStringLiteral UNO_NAME_PARA_GRAPHIC = u"ParaBackGraphic";
constexpr OUStringLiteral UNO_NAME_PARA_GRAPHIC_URL = u"ParaBackGraphicURL";
constexpr OUStringLiteral UNO_NAME_PARA_GRAPHIC_FILTER = u"ParaBackGraphicFilter";
constexpr OUStringLiteral UNO_NAME_HEADER_GRAPHIC = u"HeaderBackGraphic";
constexpr OUStringLiteral UNO_NAME_HEADER_GRAPHIC_URL = u"HeaderBackGraphicURL";
constexpr OUStringLiteral UNO_NAME_HEADER_GRAPHIC_FILTER = u"HeaderBackGraphicFilter";
constexpr OUStringLiteral UNO_NAME_FOOTER_GRAPHIC = u"FooterBackGraphic";
constexpr OUStringLiteral UNO_NAME_FOOTER_GRAPHIC_URL = u"FooterBackGraphicURL";
constexpr OUStringLiteral UNO_NAME_FOOTER_GRAPHIC_FILTER = u"FooterBackGraphicFilter";
constexpr OUStringLiteral UNO_NAME_BACK_GRAPHIC_URL = u"BackGraphicURL";
constexpr OUStringLiteral UNO_NAME_BACK_GRAPHIC = u"BackGraphic";
constexpr OUStringLiteral UNO_NAME_BACK_GRAPHIC_FILTER = u"BackGraphicFilter";
constexpr OUStringLiteral UNO_NAME_BACK_GRAPHIC_LOCATION = u"BackGraphicLocation";
constexpr OUStringLiteral UNO_NAME_GRAPHIC_FILTER = u"GraphicFilter";
constexpr OUStringLiteral UNO_NAME_GRAPHIC_SIZE = u"GraphicSize";
constexpr OUStringLiteral UNO_NAME_GRAPHIC_BITMAP = u"GraphicBitmap";
constexpr OUStringLiteral UNO_NAME_GRAPHIC_URL = u"GraphicURL";
#define UNO_NAME_BULLET_ID "BulletId"
#define UNO_NAME_BULLET_CHAR "BulletChar"
#define UNO_NAME_BULLET_REL_SIZE "BulletRelSize"
constexpr OUStringLiteral UNO_NAME_BULLET_FONT = u"BulletFont";
#define UNO_NAME_BULLET_FONT_NAME "BulletFontName"
#define UNO_NAME_BULLET_COLOR "BulletColor"
constexpr OUStringLiteral UNO_NAME_PARA_GRAPHIC_LOCATION = u"ParaBackGraphicLocation";
constexpr OUStringLiteral UNO_NAME_HEADER_GRAPHIC_LOCATION = u"HeaderBackGraphicLocation";
constexpr OUStringLiteral UNO_NAME_FOOTER_GRAPHIC_LOCATION = u"FooterBackGraphicLocation";
constexpr OUStringLiteral UNO_NAME_PARA_LEFT_MARGIN = u"ParaLeftMargin";
constexpr OUStringLiteral UNO_NAME_PARA_RIGHT_MARGIN = u"ParaRightMargin";
constexpr OUStringLiteral UNO_NAME_PARA_LEFT_MARGIN_RELATIVE = u"ParaLeftMarginRelative";
constexpr OUStringLiteral UNO_NAME_PARA_RIGHT_MARGIN_RELATIVE = u"ParaRightMarginRelative";
constexpr OUStringLiteral UNO_NAME_PARA_IS_AUTO_FIRST_LINE_INDENT = u"ParaIsAutoFirstLineIndent";
constexpr OUStringLiteral UNO_NAME_PARA_FIRST_LINE_INDENT = u"ParaFirstLineIndent";
constexpr OUStringLiteral UNO_NAME_PARA_FIRST_LINE_INDENT_RELATIVE = u"ParaFirstLineIndentRelative";
constexpr OUStringLiteral UNO_NAME_PARA_IS_HYPHENATION = u"ParaIsHyphenation";
constexpr OUStringLiteral UNO_NAME_PARA_HYPHENATION_MAX_LEADING_CHARS
    = u"ParaHyphenationMaxLeadingChars";
constexpr OUStringLiteral UNO_NAME_PARA_HYPHENATION_MAX_TRAILING_CHARS
    = u"ParaHyphenationMaxTrailingChars";
constexpr OUStringLiteral UNO_NAME_PARA_HYPHENATION_MAX_HYPHENS = u"ParaHyphenationMaxHyphens";
constexpr OUStringLiteral UNO_NAME_PARA_HYPHENATION_NO_CAPS = u"ParaHyphenationNoCaps";
constexpr OUStringLiteral UNO_NAME_LEFT_MARGIN = u"LeftMargin";
constexpr OUStringLiteral UNO_NAME_RIGHT_MARGIN = u"RightMargin";
constexpr OUStringLiteral UNO_NAME_GUTTER_MARGIN = u"GutterMargin";
constexpr OUStringLiteral UNO_NAME_HEADER_LEFT_MARGIN = u"HeaderLeftMargin";
constexpr OUStringLiteral UNO_NAME_HEADER_RIGHT_MARGIN = u"HeaderRightMargin";
constexpr OUStringLiteral UNO_NAME_FOOTER_LEFT_MARGIN = u"FooterLeftMargin";
constexpr OUStringLiteral UNO_NAME_FOOTER_RIGHT_MARGIN = u"FooterRightMargin";
constexpr OUStringLiteral UNO_NAME_TEXT_RANGE = u"TextRange";
constexpr OUStringLiteral UNO_NAME_TEXT_BOX = u"TextBox";
constexpr OUStringLiteral UNO_NAME_NAME = u"Name";
constexpr OUStringLiteral UNO_NAME_CHAR_STYLE_NAME = u"CharStyleName";
constexpr OUStringLiteral UNO_NAME_ANCHOR_CHAR_STYLE_NAME = u"AnchorCharStyleName";
constexpr OUStringLiteral UNO_NAME_SUFFIX = u"Suffix";
constexpr OUStringLiteral UNO_NAME_PREFIX = u"Prefix";
#define UNO_NAME_LIST_FORMAT "ListFormat"
#define UNO_NAME_PARENT_NUMBERING "ParentNumbering"
constexpr OUStringLiteral UNO_NAME_CHAR_FONT_NAME = u"CharFontName";
constexpr OUStringLiteral UNO_NAME_CHAR_FONT_STYLE_NAME = u"CharFontStyleName";
constexpr OUStringLiteral UNO_NAME_CHAR_FONT_FAMILY = u"CharFontFamily";
constexpr OUStringLiteral UNO_NAME_CHAR_FONT_CHAR_SET = u"CharFontCharSet";
constexpr OUStringLiteral UNO_NAME_CHAR_FONT_PITCH = u"CharFontPitch";
constexpr OUStringLiteral UNO_NAME_CHAR_LOCALE = u"CharLocale";
constexpr OUStringLiteral UNO_NAME_CHAR_FONT_NAME_ASIAN = u"CharFontNameAsian";
constexpr OUStringLiteral UNO_NAME_CHAR_FONT_STYLE_NAME_ASIAN = u"CharFontStyleNameAsian";
constexpr OUStringLiteral UNO_NAME_CHAR_FONT_FAMILY_ASIAN = u"CharFontFamilyAsian";
constexpr OUStringLiteral UNO_NAME_CHAR_FONT_CHAR_SET_ASIAN = u"CharFontCharSetAsian";
constexpr OUStringLiteral UNO_NAME_CHAR_FONT_PITCH_ASIAN = u"CharFontPitchAsian";
constexpr OUStringLiteral UNO_NAME_CHAR_POSTURE_ASIAN = u"CharPostureAsian";
constexpr OUStringLiteral UNO_NAME_CHAR_WEIGHT_ASIAN = u"CharWeightAsian";
constexpr OUStringLiteral UNO_NAME_CHAR_HEIGHT_ASIAN = u"CharHeightAsian";
constexpr OUStringLiteral UNO_NAME_CHAR_LOCALE_ASIAN = u"CharLocaleAsian";
constexpr OUStringLiteral UNO_NAME_CHAR_FONT_NAME_COMPLEX = u"CharFontNameComplex";
constexpr OUStringLiteral UNO_NAME_CHAR_FONT_STYLE_NAME_COMPLEX = u"CharFontStyleNameComplex";
constexpr OUStringLiteral UNO_NAME_CHAR_FONT_FAMILY_COMPLEX = u"CharFontFamilyComplex";
constexpr OUStringLiteral UNO_NAME_CHAR_FONT_CHAR_SET_COMPLEX = u"CharFontCharSetComplex";
constexpr OUStringLiteral UNO_NAME_CHAR_FONT_PITCH_COMPLEX = u"CharFontPitchComplex";
constexpr OUStringLiteral UNO_NAME_CHAR_POSTURE_COMPLEX = u"CharPostureComplex";
constexpr OUStringLiteral UNO_NAME_CHAR_WEIGHT_COMPLEX = u"CharWeightComplex";
constexpr OUStringLiteral UNO_NAME_CHAR_HEIGHT_COMPLEX = u"CharHeightComplex";
constexpr OUStringLiteral UNO_NAME_CHAR_LOCALE_COMPLEX = u"CharLocaleComplex";
constexpr OUStringLiteral UNO_NAME_CHAR_AUTO_KERNING = u"CharAutoKerning";
constexpr OUStringLiteral UNO_NAME_CHAR_UNDERLINE_COLOR = u"CharUnderlineColor";
constexpr OUStringLiteral UNO_NAME_CHAR_UNDERLINE_HAS_COLOR = u"CharUnderlineHasColor";
constexpr OUStringLiteral UNO_NAME_CHAR_ESCAPEMENT = u"CharEscapement";
constexpr OUStringLiteral UNO_NAME_CHAR_CASE_MAP = u"CharCaseMap";
constexpr OUStringLiteral UNO_NAME_CHAR_STRIKEOUT = u"CharStrikeout";
constexpr OUStringLiteral UNO_NAME_CHAR_CROSSED_OUT = u"CharCrossedOut";
constexpr OUStringLiteral UNO_NAME_CHAR_NO_HYPHENATION = u"CharNoHyphenation";
constexpr OUStringLiteral UNO_NAME_CHAR_AUTO_ESCAPEMENT = u"CharAutoEscapement";
constexpr OUStringLiteral UNO_NAME_CHAR_PROP_HEIGHT = u"CharPropHeight";
constexpr OUStringLiteral UNO_NAME_CHAR_DIFF_HEIGHT = u"CharDiffHeight";
constexpr OUStringLiteral UNO_NAME_CHAR_PROP_HEIGHT_ASIAN = u"CharPropHeightAsian";
constexpr OUStringLiteral UNO_NAME_CHAR_DIFF_HEIGHT_ASIAN = u"CharDiffHeightAsian";
constexpr OUStringLiteral UNO_NAME_CHAR_PROP_HEIGHT_COMPLEX = u"CharPropHeightComplex";
constexpr OUStringLiteral UNO_NAME_CHAR_DIFF_HEIGHT_COMPLEX = u"CharDiffHeightComplex";
constexpr OUStringLiteral UNO_NAME_CHAR_ESCAPEMENT_HEIGHT = u"CharEscapementHeight";
constexpr OUStringLiteral UNO_NAME_CHAR_TRANSPARENCE = u"CharTransparence";
constexpr OUStringLiteral UNO_NAME_HIDE_TAB_LEADER_AND_PAGE_NUMBERS = u"HideTabLeaderAndPageNumber";
constexpr OUStringLiteral UNO_NAME_TAB_IN_TOC = u"TabInTOC";
constexpr OUStringLiteral UNO_NAME_TOC_BOOKMARK = u"TOCBookmark";
constexpr OUStringLiteral UNO_NAME_TOC_NEWLINE = u"TOCNewLine";
constexpr OUStringLiteral UNO_NAME_TOC_PARAGRAPH_OUTLINE_LEVEL = u"TOCParagraphOutlineLevel";

constexpr OUStringLiteral UNO_NAME_CHAR_FLASH = u"CharFlash";
constexpr OUStringLiteral UNO_NAME_CHAR_KERNING = u"CharKerning";
constexpr OUStringLiteral UNO_NAME_CHAR_BACK_COLOR = u"CharBackColor";
constexpr OUStringLiteral UNO_NAME_CHAR_BACK_TRANSPARENT = u"CharBackTransparent";
constexpr OUStringLiteral UNO_NAME_CHAR_COMBINE_IS_ON = u"CharCombineIsOn";
constexpr OUStringLiteral UNO_NAME_CHAR_COMBINE_PREFIX = u"CharCombinePrefix";
constexpr OUStringLiteral UNO_NAME_CHAR_COMBINE_SUFFIX = u"CharCombineSuffix";
constexpr OUStringLiteral UNO_NAME_CHAR_EMPHASIS = u"CharEmphasis";
constexpr OUStringLiteral UNO_NAME_PARA_LINE_SPACING = u"ParaLineSpacing";
constexpr OUStringLiteral UNO_NAME_PARA_TOP_MARGIN = u"ParaTopMargin";
constexpr OUStringLiteral UNO_NAME_PARA_BOTTOM_MARGIN = u"ParaBottomMargin";
constexpr OUStringLiteral UNO_NAME_PARA_TOP_MARGIN_RELATIVE = u"ParaTopMarginRelative";
constexpr OUStringLiteral UNO_NAME_PARA_BOTTOM_MARGIN_RELATIVE = u"ParaBottomMarginRelative";
constexpr OUStringLiteral UNO_NAME_PARA_EXPAND_SINGLE_WORD = u"ParaExpandSingleWord";
constexpr OUStringLiteral UNO_NAME_END_NOTICE = u"EndNotice";
constexpr OUStringLiteral UNO_NAME_BEGIN_NOTICE = u"BeginNotice";
constexpr OUStringLiteral UNO_NAME_FRAME_STYLE_NAME = u"FrameStyleName";
constexpr OUStringLiteral UNO_NAME_NUMBERING_STYLE_NAME = u"NumberingStyleName";
constexpr OUStringLiteral UNO_NAME_NUMBERING_START_VALUE = u"NumberingStartValue";
constexpr OUStringLiteral UNO_NAME_NUMBERING_IS_NUMBER = u"NumberingIsNumber";
constexpr OUStringLiteral UNO_NAME_COUNT_LINES_IN_FRAMES = u"CountLinesInFrames";
constexpr OUStringLiteral UNO_NAME_DDE_COMMAND_TYPE = u"DDECommandType";
constexpr OUStringLiteral UNO_NAME_DDE_COMMAND_FILE = u"DDECommandFile";
constexpr OUStringLiteral UNO_NAME_DDE_COMMAND_ELEMENT = u"DDECommandElement";
constexpr OUStringLiteral UNO_NAME_IS_AUTOMATIC_UPDATE = u"IsAutomaticUpdate";
constexpr OUStringLiteral UNO_NAME_DISTANCE = u"Distance";
constexpr OUStringLiteral UNO_NAME_DROP_CAP_FORMAT = u"DropCapFormat";
constexpr OUStringLiteral UNO_NAME_DROP_CAP_WHOLE_WORD = u"DropCapWholeWord";
constexpr OUStringLiteral UNO_NAME_DROP_CAP_CHAR_STYLE_NAME = u"DropCapCharStyleName";
constexpr OUStringLiteral UNO_NAME_FILE_LINK = u"FileLink";
constexpr OUStringLiteral UNO_NAME_GRAPHIC = u"Graphic";
constexpr OUStringLiteral UNO_NAME_TRANSFORMED_GRAPHIC = u"TransformedGraphic";
constexpr OUStringLiteral UNO_NAME_IS_PROTECTED = u"IsProtected";
constexpr OUStringLiteral UNO_NAME_PARA_KEEP_TOGETHER = u"ParaKeepTogether";
constexpr OUStringLiteral UNO_NAME_KEEP_TOGETHER = u"KeepTogether";
constexpr OUStringLiteral UNO_NAME_IS_LANDSCAPE = u"IsLandscape";
constexpr OUStringLiteral UNO_NAME_SEPARATOR_TEXT = u"SeparatorText";
constexpr OUStringLiteral UNO_NAME_NUMBER_POSITION = u"NumberPosition";
constexpr OUStringLiteral UNO_NAME_PAGE_STYLE_NAME = u"PageStyleName";
constexpr OUStringLiteral UNO_NAME_PAGE_STYLE_LAYOUT = u"PageStyleLayout";
constexpr OUStringLiteral UNO_NAME_PARA_ADJUST = u"ParaAdjust";
constexpr OUStringLiteral UNO_NAME_PARA_REGISTER_MODE_ACTIVE = u"ParaRegisterModeActive";
constexpr OUStringLiteral UNO_NAME_PARA_STYLE_NAME = u"ParaStyleName";
constexpr OUStringLiteral UNO_NAME_PARA_LAST_LINE_ADJUST = u"ParaLastLineAdjust";
constexpr OUStringLiteral UNO_NAME_PARA_LINE_NUMBER_COUNT = u"ParaLineNumberCount";
constexpr OUStringLiteral UNO_NAME_PARA_LINE_NUMBER_START_VALUE = u"ParaLineNumberStartValue";
constexpr OUStringLiteral UNO_NAME_BACK_COLOR = u"BackColor";
constexpr OUStringLiteral UNO_NAME_PARA_BACK_COLOR = u"ParaBackColor";
constexpr OUStringLiteral UNO_NAME_PARA_WIDOWS = u"ParaWidows";
constexpr OUStringLiteral UNO_NAME_PARA_ORPHANS = u"ParaOrphans";
constexpr OUStringLiteral UNO_NAME_PARA_BACK_TRANSPARENT = u"ParaBackTransparent";
constexpr OUStringLiteral UNO_NAME_POSITION_END_OF_DOC = u"PositionEndOfDoc";
constexpr OUStringLiteral UNO_NAME_POSITION_PROTECTED = u"PositionProtected";
constexpr OUStringLiteral UNO_NAME_ALTERNATIVE_TEXT = u"AlternativeText";
constexpr OUStringLiteral UNO_NAME_PRIMARY_KEY = u"PrimaryKey";
constexpr OUStringLiteral UNO_NAME_PRINTER_PAPER_TRAY = u"PrinterPaperTray";
#define UNO_NAME_REGISTER_MODE_ACTIVE "RegisterModeActive"
constexpr OUStringLiteral UNO_NAME_RELATIVE_WIDTH = u"RelativeWidth";
constexpr OUStringLiteral UNO_NAME_RELATIVE_WIDTH_RELATION = u"RelativeWidthRelation";
constexpr OUStringLiteral UNO_NAME_RELATIVE_HEIGHT = u"RelativeHeight";
constexpr OUStringLiteral UNO_NAME_RELATIVE_HEIGHT_RELATION = u"RelativeHeightRelation";
constexpr OUStringLiteral UNO_NAME_REPEAT_HEADLINE = u"RepeatHeadline";
constexpr OUStringLiteral UNO_NAME_SEARCH_STYLES = u"SearchStyles";
constexpr OUStringLiteral UNO_NAME_SEARCH_BACKWARDS = u"SearchBackwards";
constexpr OUStringLiteral UNO_NAME_SEARCH_SIMILARITY = u"SearchSimilarity";
constexpr OUStringLiteral UNO_NAME_SEARCH_CASE_SENSITIVE = u"SearchCaseSensitive";
constexpr OUStringLiteral UNO_NAME_SEARCH_SIMILARITY_ADD = u"SearchSimilarityAdd";
constexpr OUStringLiteral UNO_NAME_SEARCH_SIMILARITY_RELAX = u"SearchSimilarityRelax";
constexpr OUStringLiteral UNO_NAME_SEARCH_SIMILARITY_REMOVE = u"SearchSimilarityRemove";
constexpr OUStringLiteral UNO_NAME_SEARCH_REGULAR_EXPRESSION = u"SearchRegularExpression";
constexpr OUStringLiteral UNO_NAME_SEARCH_SIMILARITY_EXCHANGE = u"SearchSimilarityExchange";
constexpr OUStringLiteral UNO_NAME_SECONDARY_KEY = u"SecondaryKey";
constexpr OUStringLiteral UNO_NAME_SEPARATOR_INTERVAL = u"SeparatorInterval";
#define UNO_NAME_SHOW_BREAKS "ShowBreaks"
#define UNO_NAME_SHOW_SPACES "ShowSpaces"
#define UNO_NAME_SHOW_TABLES "ShowTables"
#define UNO_NAME_SHOW_GRAPHICS "ShowGraphics"

#define UNO_NAME_SHOW_DRAWINGS "ShowDrawings"
#define UNO_NAME_SHOW_TABSTOPS "ShowTabstops"
#define UNO_NAME_SHOW_VERT_RULER "ShowVertRuler"
#define UNO_NAME_SHOW_PARA_BREAKS "ShowParaBreaks"
#define UNO_NAME_SHOW_HIDDEN_TEXT "ShowHiddenText"
#define UNO_NAME_SHOW_SOFT_HYPHENS "ShowSoftHyphens"
#define UNO_NAME_SHOW_VERT_SCROLL_BAR "ShowVertScrollBar"
#define UNO_NAME_SHOW_HORI_SCROLL_BAR "ShowHoriScrollBar"
#define UNO_NAME_SHOW_FIELD_COMMANDS "ShowFieldCommands"
#define UNO_NAME_SHOW_PROTECTED_SPACES "ShowProtectedSpaces"
#define UNO_NAME_SHOW_HIDDEN_PARAGRAPHS "ShowHiddenParagraphs"
#define UNO_NAME_HIDE_WHITESPACE "HideWhitespace"
constexpr OUStringLiteral UNO_NAME_IS_SYNC_WIDTH_TO_HEIGHT = u"IsSyncWidthToHeight";
constexpr OUStringLiteral UNO_NAME_IS_SYNC_HEIGHT_TO_WIDTH = u"IsSyncHeightToWidth";
constexpr OUStringLiteral UNO_NAME_SIZE_PROTECTED = u"SizeProtected";
constexpr OUStringLiteral UNO_NAME_TEXT_COLUMNS = u"TextColumns";
constexpr OUStringLiteral UNO_NAME_BACK_TRANSPARENT = u"BackTransparent";
constexpr OUStringLiteral UNO_NAME_ANCHOR_TYPE = u"AnchorType";
constexpr OUStringLiteral UNO_NAME_ANCHOR_TYPES = u"AnchorTypes";
constexpr OUStringLiteral UNO_NAME_ANCHOR_PAGE_NO = u"AnchorPageNo";
constexpr OUStringLiteral UNO_NAME_ANCHOR_FRAME = u"AnchorFrame";
constexpr OUStringLiteral UNO_NAME_AUTHOR = u"Author";
constexpr OUStringLiteral UNO_NAME_BREAK_TYPE = u"BreakType";
constexpr OUStringLiteral UNO_NAME_CHAIN_NEXT_NAME = u"ChainNextName";
constexpr OUStringLiteral UNO_NAME_CHAIN_PREV_NAME = u"ChainPrevName";
constexpr OUStringLiteral UNO_NAME_CHAIN_NAME = u"ChainName";
constexpr OUStringLiteral UNO_NAME_CHAPTER_FORMAT = u"ChapterFormat";
constexpr OUStringLiteral UNO_NAME_CLIENT_MAP = u"ClientMap";
constexpr OUStringLiteral UNO_NAME_CONDITION = u"Condition";
constexpr OUStringLiteral UNO_NAME_CONTENT = u"Content";
constexpr OUStringLiteral UNO_NAME_CHAR_CONTOURED = u"CharContoured";
constexpr OUStringLiteral UNO_NAME_CONTOUR_OUTSIDE = u"ContourOutside";
constexpr OUStringLiteral UNO_NAME_CONTENT_PROTECTED = u"ContentProtected";
constexpr OUStringLiteral UNO_NAME_COUNT_EMPTY_LINES = u"CountEmptyLines";
constexpr OUStringLiteral UNO_NAME_RESTART_AT_EACH_PAGE = u"RestartAtEachPage";
constexpr OUStringLiteral UNO_NAME_DATA_BASE_NAME = u"DataBaseName";
constexpr OUStringLiteral UNO_NAME_DATA_TABLE_NAME = u"DataTableName";
constexpr OUStringLiteral UNO_NAME_DATA_COMMAND_TYPE = u"DataCommandType";
constexpr OUStringLiteral UNO_NAME_DATA_COLUMN_NAME = u"DataColumnName";
constexpr OUStringLiteral UNO_NAME_IS_DATA_BASE_FORMAT = u"DataBaseFormat";
constexpr OUStringLiteral UNO_NAME_DATE = u"Date";
constexpr OUStringLiteral UNO_NAME_IS_DATE = u"IsDate";
constexpr OUStringLiteral UNO_NAME_EDIT_IN_READONLY = u"EditInReadonly";
constexpr OUStringLiteral UNO_NAME_FALSE_CONTENT = u"FalseContent";
constexpr OUStringLiteral UNO_NAME_FILE_FORMAT = u"FileFormat";
constexpr OUStringLiteral UNO_NAME_IS_FIXED = u"IsFixed";
constexpr OUStringLiteral UNO_NAME_FOOTNOTE_COUNTING = u"FootnoteCounting";
constexpr OUStringLiteral UNO_NAME_FULL_NAME = u"FullName";
constexpr OUStringLiteral UNO_NAME_HEIGHT = u"Height";
constexpr OUStringLiteral UNO_NAME_IS_AUTO_HEIGHT = u"IsAutoHeight";
constexpr OUStringLiteral UNO_NAME_SIZE_TYPE = u"SizeType";
constexpr OUStringLiteral UNO_NAME_HINT = u"Hint";
constexpr OUStringLiteral UNO_NAME_HORI_ORIENT = u"HoriOrient";
constexpr OUStringLiteral UNO_NAME_HORI_MIRRORED_ON_EVEN_PAGES = u"HoriMirroredOnEvenPages";
constexpr OUStringLiteral UNO_NAME_HORI_MIRRORED_ON_ODD_PAGES = u"HoriMirroredOnOddPages";
constexpr OUStringLiteral UNO_NAME_HORI_ORIENT_RELATION = u"HoriOrientRelation";
constexpr OUStringLiteral UNO_NAME_HORI_ORIENT_POSITION = u"HoriOrientPosition";
constexpr OUStringLiteral UNO_NAME_HYPER_LINK_U_R_L = u"HyperLinkURL";
constexpr OUStringLiteral UNO_NAME_HYPER_LINK_TARGET = u"HyperLinkTarget";
constexpr OUStringLiteral UNO_NAME_HYPER_LINK_NAME = u"HyperLinkName";
constexpr OUStringLiteral UNO_NAME_HYPER_LINK_EVENTS = u"HyperLinkEvents";
constexpr OUStringLiteral UNO_NAME_INFO_TYPE = u"InfoType";
constexpr OUStringLiteral UNO_NAME_INFO_FORMAT = u"InfoFormat";
constexpr OUStringLiteral UNO_NAME_IS_INPUT = u"IsInput";
constexpr OUStringLiteral UNO_NAME_LEVEL = u"Level";
constexpr OUStringLiteral UNO_NAME_INTERVAL = u"Interval";
constexpr OUStringLiteral UNO_NAME_LINK_REGION = u"LinkRegion";
constexpr OUStringLiteral UNO_NAME_MACRO_NAME = u"MacroName";
constexpr OUStringLiteral UNO_NAME_SPLIT = u"Split";
constexpr OUStringLiteral UNO_NAME_PARA_SPLIT = u"ParaSplit";
constexpr OUStringLiteral UNO_NAME_NUMBER_FORMAT = u"NumberFormat";
constexpr OUStringLiteral UNO_NAME_NUMBERING_TYPE = u"NumberingType";
constexpr OUStringLiteral UNO_NAME_OFFSET = u"Offset";
constexpr OUStringLiteral UNO_NAME_ON = u"On";
constexpr OUStringLiteral UNO_NAME_OPAQUE = u"Opaque";
constexpr OUStringLiteral UNO_NAME_PAGE_TOGGLE = u"PageToggle";
constexpr OUStringLiteral UNO_NAME_PAGE_DESC_NAME = u"PageDescName";
constexpr OUStringLiteral UNO_NAME_PAGE_NUMBER_OFFSET = u"PageNumberOffset";
constexpr OUStringLiteral UNO_NAME_PLACEHOLDER = u"PlaceHolder";
constexpr OUStringLiteral UNO_NAME_PLACEHOLDER_TYPE = u"PlaceHolderType";
constexpr OUStringLiteral UNO_NAME_PRINT = u"Print";
constexpr OUStringLiteral UNO_NAME_REFERENCE_FIELD_PART = u"ReferenceFieldPart";
constexpr OUStringLiteral UNO_NAME_REFERENCE_FIELD_SOURCE = u"ReferenceFieldSource";
constexpr OUStringLiteral UNO_NAME_REFERENCE_FIELD_LANGUAGE = u"ReferenceFieldLanguage";
constexpr OUStringLiteral UNO_NAME_REGISTER_PARAGRAPH_STYLE = u"RegisterParagraphStyle";
constexpr OUStringLiteral UNO_NAME_SCRIPT_TYPE = u"ScriptType";
constexpr OUStringLiteral UNO_NAME_SEARCH_ALL = u"SearchAll";

constexpr OUStringLiteral UNO_NAME_SEARCH_WORDS = u"SearchWords";
constexpr OUStringLiteral UNO_NAME_SEQUENCE_VALUE = u"SequenceValue";
constexpr OUStringLiteral UNO_NAME_SERVER_MAP = u"ServerMap";
constexpr OUStringLiteral UNO_NAME_IMAGE_MAP = u"ImageMap";
constexpr OUStringLiteral UNO_NAME_SET_NUMBER = u"SetNumber";
constexpr OUStringLiteral UNO_NAME_SHADOW_FORMAT = u"ShadowFormat";
#define UNO_NAME_SHOW_HORI_RULER "ShowHoriRuler"
constexpr OUStringLiteral UNO_NAME_SIZE = u"Size";
constexpr OUStringLiteral UNO_NAME_ACTUAL_SIZE = u"ActualSize";
constexpr OUStringLiteral UNO_NAME_SOURCE_NAME = u"SourceName";
constexpr OUStringLiteral UNO_NAME_START_AT = u"StartAt";
#define UNO_NAME_START_WITH "StartWith"
#define UNO_NAME_STATISTIC_TYPE_ID "StatisticTypeId"
constexpr OUStringLiteral UNO_NAME_SUB_TYPE = u"SubType";
constexpr OUStringLiteral UNO_NAME_SURROUND = u"Surround";
constexpr OUStringLiteral UNO_NAME_IS_EXPRESSION = u"IsExpression";
constexpr OUStringLiteral UNO_NAME_IS_SHOW_FORMULA = u"IsShowFormula";
constexpr OUStringLiteral UNO_NAME_TEXT_WRAP = u"TextWrap";
constexpr OUStringLiteral UNO_NAME_SURROUND_CONTOUR = u"SurroundContour";
constexpr OUStringLiteral UNO_NAME_SURROUND_ANCHORONLY = u"SurroundAnchorOnly";
constexpr OUStringLiteral UNO_NAME_TABLE_NAME = u"TableName";
constexpr OUStringLiteral UNO_NAME_TABLE_TEMPLATE_NAME = u"TableTemplateName";
constexpr OUStringLiteral UNO_NAME_TABSTOPS = u"ParaTabStops";
constexpr OUStringLiteral UNO_NAME_TITLE = u"Title";
constexpr OUStringLiteral UNO_NAME_TOP_MARGIN = u"TopMargin";
constexpr OUStringLiteral UNO_NAME_BOTTOM_MARGIN = u"BottomMargin";
constexpr OUStringLiteral UNO_NAME_TRUE_CONTENT = u"TrueContent";
constexpr OUStringLiteral UNO_NAME_URL_CONTENT = u"URLContent";
constexpr OUStringLiteral UNO_NAME_USERTEXT = u"UserText";
constexpr OUStringLiteral UNO_NAME_USER_DATA_TYPE = u"UserDataType";
constexpr OUStringLiteral UNO_NAME_VALUE = u"Value";
constexpr OUStringLiteral UNO_NAME_VARIABLE_NAME = u"VariableName";
constexpr OUStringLiteral UNO_NAME_VARIABLE_SUBTYPE = u"VariableSubtype";
constexpr OUStringLiteral UNO_NAME_VERT_ORIENT = u"VertOrient";
constexpr OUStringLiteral UNO_NAME_VERT_MIRRORED = u"VertMirrored";
constexpr OUStringLiteral UNO_NAME_VERT_ORIENT_POSITION = u"VertOrientPosition";
constexpr OUStringLiteral UNO_NAME_VERT_ORIENT_RELATION = u"VertOrientRelation";
constexpr OUStringLiteral UNO_NAME_IS_VISIBLE = u"IsVisible";
constexpr OUStringLiteral UNO_NAME_WIDTH = u"Width";
constexpr OUStringLiteral UNO_NAME_CHAR_WORD_MODE = u"CharWordMode";
constexpr OUStringLiteral UNO_NAME_GRAPHIC_CROP = u"GraphicCrop";
#define UNO_NAME_CHARACTER_FORMAT_NONE "CharacterFormatNone"
constexpr OUStringLiteral UNO_NAME_DOCUMENT_INDEX_MARK = u"DocumentIndexMark";
constexpr OUStringLiteral UNO_NAME_DOCUMENT_INDEX = u"DocumentIndex";
constexpr OUStringLiteral UNO_NAME_IS_GLOBAL_DOCUMENT_SECTION = u"IsGlobalDocumentSection";
constexpr OUStringLiteral UNO_NAME_TEXT_FIELD = u"TextField";
constexpr OUStringLiteral UNO_NAME_BOOKMARK = u"Bookmark";
constexpr OUStringLiteral UNO_NAME_BOOKMARK_HIDDEN = u"BookmarkHidden";
constexpr OUStringLiteral UNO_NAME_BOOKMARK_CONDITION = u"BookmarkCondition";
constexpr OUStringLiteral UNO_NAME_TEXT_TABLE = u"TextTable";
constexpr OUStringLiteral UNO_NAME_CELL = u"Cell";
constexpr OUStringLiteral UNO_NAME_TEXT_FRAME = u"TextFrame";
constexpr OUStringLiteral UNO_NAME_REFERENCE_MARK = u"ReferenceMark";
constexpr OUStringLiteral UNO_NAME_TEXT_SECTION = u"TextSection";
constexpr OUStringLiteral UNO_NAME_FOOTNOTE = u"Footnote";
constexpr OUStringLiteral UNO_NAME_ENDNOTE = u"Endnote";
constexpr OUStringLiteral UNO_NAME_CHART_ROW_AS_LABEL = u"ChartRowAsLabel";
constexpr OUStringLiteral UNO_NAME_CHART_COLUMN_AS_LABEL = u"ChartColumnAsLabel";
constexpr OUStringLiteral UNO_NAME_LEFT_BORDER = u"LeftBorder";
constexpr OUStringLiteral UNO_NAME_RIGHT_BORDER = u"RightBorder";
constexpr OUStringLiteral UNO_NAME_TOP_BORDER = u"TopBorder";
constexpr OUStringLiteral UNO_NAME_BOTTOM_BORDER = u"BottomBorder";
constexpr OUStringLiteral UNO_NAME_BORDER_DISTANCE = u"BorderDistance";
constexpr OUStringLiteral UNO_NAME_LEFT_BORDER_DISTANCE = u"LeftBorderDistance";
constexpr OUStringLiteral UNO_NAME_RIGHT_BORDER_DISTANCE = u"RightBorderDistance";
constexpr OUStringLiteral UNO_NAME_TOP_BORDER_DISTANCE = u"TopBorderDistance";
constexpr OUStringLiteral UNO_NAME_BOTTOM_BORDER_DISTANCE = u"BottomBorderDistance";
constexpr OUStringLiteral UNO_NAME_TABLE_BORDER = u"TableBorder";
constexpr OUStringLiteral UNO_NAME_TABLE_COLUMN_SEPARATORS = u"TableColumnSeparators";
constexpr OUStringLiteral UNO_NAME_TABLE_COLUMN_RELATIVE_SUM = u"TableColumnRelativeSum";
constexpr OUStringLiteral UNO_NAME_HEADER_TEXT = u"HeaderText";
constexpr OUStringLiteral UNO_NAME_HEADER_TEXT_LEFT = u"HeaderTextLeft";
constexpr OUStringLiteral UNO_NAME_HEADER_TEXT_RIGHT = u"HeaderTextRight";
constexpr OUStringLiteral UNO_NAME_FOOTER_TEXT = u"FooterText";
constexpr OUStringLiteral UNO_NAME_FOOTER_TEXT_LEFT = u"FooterTextLeft";
constexpr OUStringLiteral UNO_NAME_FOOTER_TEXT_RIGHT = u"FooterTextRight";
constexpr OUStringLiteral UNO_NAME_HEADER_BACK_COLOR = u"HeaderBackColor";
constexpr OUStringLiteral UNO_NAME_HEADER_BACK_TRANSPARENT = u"HeaderBackTransparent";
constexpr OUStringLiteral UNO_NAME_HEADER_LEFT_BORDER = u"HeaderLeftBorder";
constexpr OUStringLiteral UNO_NAME_HEADER_RIGHT_BORDER = u"HeaderRightBorder";
constexpr OUStringLiteral UNO_NAME_HEADER_TOP_BORDER = u"HeaderTopBorder";
constexpr OUStringLiteral UNO_NAME_HEADER_BOTTOM_BORDER = u"HeaderBottomBorder";
constexpr OUStringLiteral UNO_NAME_HEADER_BORDER_DISTANCE = u"HeaderBorderDistance";
constexpr OUStringLiteral UNO_NAME_HEADER_SHADOW_FORMAT = u"HeaderShadowFormat";
constexpr OUStringLiteral UNO_NAME_HEADER_BODY_DISTANCE = u"HeaderBodyDistance";
constexpr OUStringLiteral UNO_NAME_HEADER_IS_DYNAMIC_HEIGHT = u"HeaderIsDynamicHeight";
constexpr OUStringLiteral UNO_NAME_HEADER_IS_SHARED = u"HeaderIsShared";
constexpr OUStringLiteral UNO_NAME_HEADER_HEIGHT = u"HeaderHeight";
constexpr OUStringLiteral UNO_NAME_HEADER_IS_ON = u"HeaderIsOn";
constexpr OUStringLiteral UNO_NAME_FOOTER_BACK_COLOR = u"FooterBackColor";
constexpr OUStringLiteral UNO_NAME_FOOTER_BACK_TRANSPARENT = u"FooterBackTransparent";
constexpr OUStringLiteral UNO_NAME_FOOTER_LEFT_BORDER = u"FooterLeftBorder";
constexpr OUStringLiteral UNO_NAME_FOOTER_RIGHT_BORDER = u"FooterRightBorder";
constexpr OUStringLiteral UNO_NAME_FOOTER_TOP_BORDER = u"FooterTopBorder";
constexpr OUStringLiteral UNO_NAME_FOOTER_BOTTOM_BORDER = u"FooterBottomBorder";
constexpr OUStringLiteral UNO_NAME_FOOTER_BORDER_DISTANCE = u"FooterBorderDistance";
constexpr OUStringLiteral UNO_NAME_FOOTER_SHADOW_FORMAT = u"FooterShadowFormat";
constexpr OUStringLiteral UNO_NAME_FOOTER_BODY_DISTANCE = u"FooterBodyDistance";
constexpr OUStringLiteral UNO_NAME_FOOTER_IS_DYNAMIC_HEIGHT = u"FooterIsDynamicHeight";
constexpr OUStringLiteral UNO_NAME_FOOTER_IS_SHARED = u"FooterIsShared";
constexpr OUStringLiteral UNO_NAME_TEXT_PARAGRAPH = u"TextParagraph";
constexpr OUStringLiteral UNO_NAME_PARENT_TEXT = u"ParentText";

constexpr OUStringLiteral UNO_NAME_FOOTER_HEIGHT = u"FooterHeight";
constexpr OUStringLiteral UNO_NAME_FOOTER_IS_ON = u"FooterIsOn";
constexpr OUStringLiteral UNO_NAME_OVERWRITE_STYLES = u"OverwriteStyles";
constexpr OUStringLiteral UNO_NAME_LOAD_NUMBERING_STYLES = u"LoadNumberingStyles";
constexpr OUStringLiteral UNO_NAME_LOAD_PAGE_STYLES = u"LoadPageStyles";
constexpr OUStringLiteral UNO_NAME_LOAD_FRAME_STYLES = u"LoadFrameStyles";
constexpr OUStringLiteral UNO_NAME_LOAD_TEXT_STYLES = u"LoadTextStyles";
#define UNO_NAME_FILE_NAME "FileName"
#define UNO_NAME_COPY_COUNT "CopyCount"
#define UNO_NAME_COLLATE "Collate"
#define UNO_NAME_SORT "Sort"
#define UNO_NAME_PAGES "Pages"
constexpr OUStringLiteral UNO_NAME_FIRST_LINE_OFFSET = u"FirstLineOffset";
constexpr OUStringLiteral UNO_NAME_SYMBOL_TEXT_DISTANCE = u"SymbolTextDistance";
constexpr OUStringLiteral UNO_NAME_USER_INDEX_NAME = u"UserIndexName";
constexpr OUStringLiteral UNO_NAME_REVISION = u"Revision";
constexpr OUStringLiteral UNO_NAME_UNVISITED_CHAR_STYLE_NAME = u"UnvisitedCharStyleName";
constexpr OUStringLiteral UNO_NAME_VISITED_CHAR_STYLE_NAME = u"VisitedCharStyleName";
constexpr OUStringLiteral UNO_NAME_PARAGRAPH_COUNT = u"ParagraphCount";
constexpr OUStringLiteral UNO_NAME_WORD_COUNT = u"WordCount";
constexpr OUStringLiteral UNO_NAME_WORD_SEPARATOR = u"WordSeparator";
constexpr OUStringLiteral UNO_NAME_CHARACTER_COUNT = u"CharacterCount";
#define UNO_NAME_ZOOM_VALUE "ZoomValue"
#define UNO_NAME_ZOOM_TYPE "ZoomType"
constexpr OUStringLiteral UNO_NAME_CREATE_FROM_MARKS = u"CreateFromMarks";
constexpr OUStringLiteral UNO_NAME_CREATE_FROM_OUTLINE = u"CreateFromOutline";
constexpr OUStringLiteral UNO_NAME_CREATE_FROM_CHAPTER = u"CreateFromChapter";
constexpr OUStringLiteral UNO_NAME_CREATE_FROM_LABELS = u"CreateFromLabels";
constexpr OUStringLiteral UNO_NAME_USE_ALPHABETICAL_SEPARATORS = u"UseAlphabeticalSeparators";
constexpr OUStringLiteral UNO_NAME_USE_KEY_AS_ENTRY = u"UseKeyAsEntry";
constexpr OUStringLiteral UNO_NAME_USE_COMBINED_ENTRIES = u"UseCombinedEntries";
constexpr OUStringLiteral UNO_NAME_IS_CASE_SENSITIVE = u"IsCaseSensitive";
constexpr OUStringLiteral UNO_NAME_USE_P_P = u"UsePP";
constexpr OUStringLiteral UNO_NAME_USE_DASH = u"UseDash";
constexpr OUStringLiteral UNO_NAME_USE_UPPER_CASE = u"UseUpperCase";
constexpr OUStringLiteral UNO_NAME_LABEL_CATEGORY = u"LabelCategory";
constexpr OUStringLiteral UNO_NAME_LABEL_DISPLAY_TYPE = u"LabelDisplayType";
constexpr OUStringLiteral UNO_NAME_USE_LEVEL_FROM_SOURCE = u"UseLevelFromSource";
constexpr OUStringLiteral UNO_NAME_LEVEL_FORMAT = u"LevelFormat";
constexpr OUStringLiteral UNO_NAME_LEVEL_PARAGRAPH_STYLES = u"LevelParagraphStyles";
constexpr OUStringLiteral UNO_NAME_MAIN_ENTRY_CHARACTER_STYLE_NAME = u"MainEntryCharacterStyleName";
constexpr OUStringLiteral UNO_NAME_CREATE_FROM_TABLES = u"CreateFromTables";
constexpr OUStringLiteral UNO_NAME_CREATE_FROM_TEXT_FRAMES = u"CreateFromTextFrames";
constexpr OUStringLiteral UNO_NAME_CREATE_FROM_GRAPHIC_OBJECTS = u"CreateFromGraphicObjects";
constexpr OUStringLiteral UNO_NAME_CREATE_FROM_EMBEDDED_OBJECTS = u"CreateFromEmbeddedObjects";
constexpr OUStringLiteral UNO_NAME_CREATE_FROM_STAR_MATH = u"CreateFromStarMath";
constexpr OUStringLiteral UNO_NAME_CREATE_FROM_STAR_CHART = u"CreateFromStarChart";
constexpr OUStringLiteral UNO_NAME_CREATE_FROM_STAR_CALC = u"CreateFromStarCalc";
constexpr OUStringLiteral UNO_NAME_CREATE_FROM_STAR_DRAW = u"CreateFromStarDraw";
constexpr OUStringLiteral UNO_NAME_CREATE_FROM_OTHER_EMBEDDED_OBJECTS
    = u"CreateFromOtherEmbeddedObjects";
constexpr OUStringLiteral UNO_NAME_INDEX_AUTO_MARK_FILE_U_R_L = u"IndexAutoMarkFileURL";
constexpr OUStringLiteral UNO_NAME_IS_COMMA_SEPARATED = u"IsCommaSeparated";
constexpr OUStringLiteral UNO_NAME_IS_RELATIVE_TABSTOPS = u"IsRelativeTabstops";
constexpr OUStringLiteral UNO_NAME_CREATE_FROM_LEVEL_PARAGRAPH_STYLES
    = u"CreateFromLevelParagraphStyles";
constexpr OUStringLiteral UNO_NAME_SHOW_CHANGES = u"ShowChanges";
constexpr OUStringLiteral UNO_NAME_RECORD_CHANGES = u"RecordChanges";
constexpr OUStringLiteral UNO_LINK_DISPLAY_NAME = u"LinkDisplayName";
constexpr OUStringLiteral UNO_LINK_DISPLAY_BITMAP = u"LinkDisplayBitmap";
constexpr OUStringLiteral UNO_NAME_HEADING_STYLE_NAME = u"HeadingStyleName";
#define UNO_NAME_SHOW_ONLINE_LAYOUT "ShowOnlineLayout"
constexpr OUStringLiteral UNO_NAME_USER_DEFINED_ATTRIBUTES = u"UserDefinedAttributes";
constexpr OUStringLiteral UNO_NAME_TEXT_USER_DEFINED_ATTRIBUTES = u"TextUserDefinedAttributes";
constexpr OUStringLiteral UNO_NAME_FILE_PATH = u"FilePath";
constexpr OUStringLiteral UNO_NAME_PARA_CHAPTER_NUMBERING_LEVEL = u"ParaChapterNumberingLevel";
constexpr OUStringLiteral UNO_NAME_PARA_CONDITIONAL_STYLE_NAME = u"ParaConditionalStyleName";
constexpr OUStringLiteral UNO_NAME_CHAPTER_NUMBERING_LEVEL = u"ChapterNumberingLevel";
constexpr OUStringLiteral UNO_NAME_NUMBERING_SEPARATOR = u"NumberingSeparator";
constexpr OUStringLiteral UNO_NAME_IS_CONTINUOUS_NUMBERING = u"IsContinuousNumbering";
constexpr OUStringLiteral UNO_NAME_IS_AUTOMATIC = u"IsAutomatic";
constexpr OUStringLiteral UNO_NAME_IS_ABSOLUTE_MARGINS = u"IsAbsoluteMargins";
constexpr OUStringLiteral UNO_NAME_CATEGORY = u"Category";
constexpr OUStringLiteral UNO_NAME_DEPENDENT_TEXT_FIELDS = u"DependentTextFields";
constexpr OUStringLiteral UNO_NAME_CURRENT_PRESENTATION = u"CurrentPresentation";
constexpr OUStringLiteral UNO_NAME_ADJUST = u"Adjust";
constexpr OUStringLiteral UNO_NAME_INSTANCE_NAME = u"InstanceName";
constexpr OUStringLiteral UNO_NAME_TEXT_PORTION_TYPE = u"TextPortionType";
constexpr OUStringLiteral UNO_NAME_CONTROL_CHARACTER = u"ControlCharacter";
constexpr OUStringLiteral UNO_NAME_IS_COLLAPSED = u"IsCollapsed";
constexpr OUStringLiteral UNO_NAME_IS_START = u"IsStart";
constexpr OUStringLiteral UNO_NAME_SEQUENCE_NUMBER = u"SequenceNumber";
constexpr OUStringLiteral UNO_NAME_REFERENCE_ID = u"ReferenceId";
constexpr OUStringLiteral UNO_NAME_HEADER_LEFT_BORDER_DISTANCE = u"HeaderLeftBorderDistance";
constexpr OUStringLiteral UNO_NAME_HEADER_RIGHT_BORDER_DISTANCE = u"HeaderRightBorderDistance";
constexpr OUStringLiteral UNO_NAME_HEADER_TOP_BORDER_DISTANCE = u"HeaderTopBorderDistance";
constexpr OUStringLiteral UNO_NAME_HEADER_BOTTOM_BORDER_DISTANCE = u"HeaderBottomBorderDistance";
constexpr OUStringLiteral UNO_NAME_FOOTER_LEFT_BORDER_DISTANCE = u"FooterLeftBorderDistance";
constexpr OUStringLiteral UNO_NAME_FOOTER_RIGHT_BORDER_DISTANCE = u"FooterRightBorderDistance";
constexpr OUStringLiteral UNO_NAME_FOOTER_TOP_BORDER_DISTANCE = u"FooterTopBorderDistance";
constexpr OUStringLiteral UNO_NAME_FOOTER_BOTTOM_BORDER_DISTANCE = u"FooterBottomBorderDistance";
constexpr OUStringLiteral UNO_NAME_PARA_IS_NUMBERING_RESTART = u"ParaIsNumberingRestart";
constexpr OUStringLiteral UNO_NAME_HIDE_FIELD_TIPS = u"HideFieldTips";
constexpr OUStringLiteral UNO_NAME_PARA_SHADOW_FORMAT = u"ParaShadowFormat";
constexpr OUStringLiteral UNO_NAME_CONTOUR_POLY_POLYGON = u"ContourPolyPolygon";

constexpr OUStringLiteral UNO_NAME_IS_PIXEL_CONTOUR = u"IsPixelContour";
constexpr OUStringLiteral UNO_NAME_IS_AUTOMATIC_CONTOUR = u"IsAutomaticContour";
constexpr OUStringLiteral UNO_NAME_SEPARATOR_LINE_WIDTH = u"SeparatorLineWidth";
constexpr OUStringLiteral UNO_NAME_SEPARATOR_LINE_COLOR = u"SeparatorLineColor";
constexpr OUStringLiteral UNO_NAME_SEPARATOR_LINE_RELATIVE_HEIGHT = u"SeparatorLineRelativeHeight";
constexpr OUStringLiteral UNO_NAME_SEPARATOR_LINE_VERTIVAL_ALIGNMENT
    = u"SeparatorLineVerticalAlignment";
constexpr OUStringLiteral UNO_NAME_SEPARATOR_LINE_IS_ON = u"SeparatorLineIsOn";
#define UNO_NAME_IS_SKIP_HIDDEN_TEXT "IsSkipHiddenText"
#define UNO_NAME_IS_SKIP_PROTECTED_TEXT "IsSkipProtectedText"
constexpr OUStringLiteral UNO_NAME_DOCUMENT_INDEX_MARKS = u"DocumentIndexMarks";
constexpr OUStringLiteral UNO_NAME_FOOTNOTE_IS_COLLECT_AT_TEXT_END = u"FootnoteIsCollectAtTextEnd";
constexpr OUStringLiteral UNO_NAME_FOOTNOTE_IS_RESTART_NUMBERING = u"FootnoteIsRestartNumbering";
constexpr OUStringLiteral UNO_NAME_FOOTNOTE_RESTART_NUMBERING_AT = u"FootnoteRestartNumberingAt";
constexpr OUStringLiteral UNO_NAME_FOOTNOTE_IS_OWN_NUMBERING = u"FootnoteIsOwnNumbering";
constexpr OUStringLiteral UNO_NAME_FOOTNOTE_NUMBERING_TYPE = u"FootnoteNumberingType";
constexpr OUStringLiteral UNO_NAME_FOOTNOTE_NUMBERING_PREFIX = u"FootnoteNumberingPrefix";
constexpr OUStringLiteral UNO_NAME_FOOTNOTE_NUMBERING_SUFFIX = u"FootnoteNumberingSuffix";
constexpr OUStringLiteral UNO_NAME_ENDNOTE_IS_COLLECT_AT_TEXT_END = u"EndnoteIsCollectAtTextEnd";
constexpr OUStringLiteral UNO_NAME_ENDNOTE_IS_RESTART_NUMBERING = u"EndnoteIsRestartNumbering";
constexpr OUStringLiteral UNO_NAME_ENDNOTE_RESTART_NUMBERING_AT = u"EndnoteRestartNumberingAt";
constexpr OUStringLiteral UNO_NAME_ENDNOTE_IS_OWN_NUMBERING = u"EndnoteIsOwnNumbering";
constexpr OUStringLiteral UNO_NAME_ENDNOTE_NUMBERING_TYPE = u"EndnoteNumberingType";
constexpr OUStringLiteral UNO_NAME_ENDNOTE_NUMBERING_PREFIX = u"EndnoteNumberingPrefix";
constexpr OUStringLiteral UNO_NAME_ENDNOTE_NUMBERING_SUFFIX = u"EndnoteNumberingSuffix";
constexpr OUStringLiteral UNO_NAME_BRACKET_BEFORE = u"BracketBefore";
constexpr OUStringLiteral UNO_NAME_BRACKET_AFTER = u"BracketAfter";
constexpr OUStringLiteral UNO_NAME_IS_NUMBER_ENTRIES = u"IsNumberEntries";
constexpr OUStringLiteral UNO_NAME_IS_SORT_BY_POSITION = u"IsSortByPosition";
constexpr OUStringLiteral UNO_NAME_SORT_KEYS = u"SortKeys";
#define UNO_NAME_IS_SORT_ASCENDING "IsSortAscending"
#define UNO_NAME_SORT_KEY "SortKey"
constexpr OUStringLiteral UNO_NAME_FIELDS = u"Fields";
constexpr OUStringLiteral UNO_NAME_DATE_TIME_VALUE = u"DateTimeValue";
constexpr OUStringLiteral UNO_NAME_IS_ON = u"IsOn";
constexpr OUStringLiteral UNO_NAME_Z_ORDER = u"ZOrder";
constexpr OUStringLiteral UNO_NAME_CONTENT_SECTION = u"ContentSection";
constexpr OUStringLiteral UNO_NAME_HEADER_SECTION = u"HeaderSection";
constexpr OUStringLiteral UNO_NAME_PARA_IS_HANGING_PUNCTUATION = u"ParaIsHangingPunctuation";
constexpr OUStringLiteral UNO_NAME_PARA_IS_CHARACTER_DISTANCE = u"ParaIsCharacterDistance";
constexpr OUStringLiteral UNO_NAME_PARA_IS_FORBIDDEN_RULES = u"ParaIsForbiddenRules";
constexpr OUStringLiteral UNO_NAME_PARA_VERT_ALIGNMENT = u"ParaVertAlignment";
constexpr OUStringLiteral UNO_NAME_IS_MAIN_ENTRY = u"IsMainEntry";
constexpr OUStringLiteral UNO_NAME_GRAPHIC_ROTATION = u"GraphicRotation";
constexpr OUStringLiteral UNO_NAME_ADJUST_LUMINANCE = u"AdjustLuminance";
constexpr OUStringLiteral UNO_NAME_ADJUST_CONTRAST = u"AdjustContrast";
constexpr OUStringLiteral UNO_NAME_ADJUST_RED = u"AdjustRed";
constexpr OUStringLiteral UNO_NAME_ADJUST_GREEN = u"AdjustGreen";
constexpr OUStringLiteral UNO_NAME_ADJUST_BLUE = u"AdjustBlue";
constexpr OUStringLiteral UNO_NAME_GAMMA = u"Gamma";
constexpr OUStringLiteral UNO_NAME_GRAPHIC_IS_INVERTED = u"GraphicIsInverted";
constexpr OUStringLiteral UNO_NAME_TRANSPARENCY = u"Transparency";
constexpr OUStringLiteral UNO_NAME_REDLINE_AUTHOR = u"RedlineAuthor";
constexpr OUStringLiteral UNO_NAME_REDLINE_DATE_TIME = u"RedlineDateTime";
constexpr OUStringLiteral UNO_NAME_REDLINE_COMMENT = u"RedlineComment";
constexpr OUStringLiteral UNO_NAME_REDLINE_DESCRIPTION = u"RedlineDescription";
constexpr OUStringLiteral UNO_NAME_REDLINE_TYPE = u"RedlineType";
constexpr OUStringLiteral UNO_NAME_REDLINE_SUCCESSOR_DATA = u"RedlineSuccessorData";
constexpr OUStringLiteral UNO_NAME_REDLINE_IDENTIFIER = u"RedlineIdentifier";
constexpr OUStringLiteral UNO_NAME_IS_IN_HEADER_FOOTER = u"IsInHeaderFooter";
constexpr OUStringLiteral UNO_NAME_START_REDLINE = u"StartRedline";
constexpr OUStringLiteral UNO_NAME_END_REDLINE = u"EndRedline";
constexpr OUStringLiteral UNO_NAME_REDLINE_START = u"RedlineStart";
constexpr OUStringLiteral UNO_NAME_REDLINE_END = u"RedlineEnd";
constexpr OUStringLiteral UNO_NAME_REDLINE_TEXT = u"RedlineText";
constexpr OUStringLiteral UNO_NAME_REDLINE_DISPLAY_TYPE = u"RedlineDisplayType";
constexpr OUStringLiteral UNO_NAME_FORBIDDEN_CHARACTERS = u"ForbiddenCharacters";
#define UNO_NAME_RUBY_BASE_TEXT "RubyBaseText"
constexpr OUStringLiteral UNO_NAME_RUBY_TEXT = u"RubyText";
constexpr OUStringLiteral UNO_NAME_RUBY_ADJUST = u"RubyAdjust";
constexpr OUStringLiteral UNO_NAME_RUBY_CHAR_STYLE_NAME = u"RubyCharStyleName";
constexpr OUStringLiteral UNO_NAME_RUBY_IS_ABOVE = u"RubyIsAbove";
constexpr OUStringLiteral UNO_NAME_RUBY_POSITION = u"RubyPosition";
constexpr OUStringLiteral UNO_NAME_FOOTNOTE_HEIGHT = u"FootnoteHeight";
constexpr OUStringLiteral UNO_NAME_FOOTNOTE_LINE_WEIGHT = u"FootnoteLineWeight";
constexpr OUStringLiteral UNO_NAME_FOOTNOTE_LINE_COLOR = u"FootnoteLineColor";
constexpr OUStringLiteral UNO_NAME_FOOTNOTE_LINE_RELATIVE_WIDTH = u"FootnoteLineRelativeWidth";
constexpr OUStringLiteral UNO_NAME_FOOTNOTE_LINE_ADJUST = u"FootnoteLineAdjust";
constexpr OUStringLiteral UNO_NAME_FOOTNOTE_LINE_TEXT_DISTANCE = u"FootnoteLineTextDistance";
constexpr OUStringLiteral UNO_NAME_FOOTNOTE_LINE_DISTANCE = u"FootnoteLineDistance";
constexpr OUStringLiteral UNO_NAME_CHAR_ROTATION = u"CharRotation";
constexpr OUStringLiteral UNO_NAME_CHAR_ROTATION_IS_FIT_TO_LINE = u"CharRotationIsFitToLine";
constexpr OUStringLiteral UNO_NAME_CHAR_SCALE_WIDTH = u"CharScaleWidth";
constexpr OUStringLiteral UNO_NAME_TAB_STOP_DISTANCE = u"TabStopDistance";
constexpr OUStringLiteral UNO_NAME_IS_WIDTH_RELATIVE = u"IsWidthRelative";
constexpr OUStringLiteral UNO_NAME_CHAR_RELIEF = u"CharRelief";
constexpr OUStringLiteral UNO_NAME_IS_HIDDEN = u"IsHidden";
constexpr OUStringLiteral UNO_NAME_IS_CONDITION_TRUE = u"IsConditionTrue";
constexpr OUStringLiteral UNO_NAME_TWO_DIGIT_YEAR = u"TwoDigitYear";
constexpr OUStringLiteral UNO_NAME_PROTECTION_KEY = u"ProtectionKey";
constexpr OUStringLiteral UNO_NAME_REDLINE_PROTECTION_KEY = u"RedlineProtectionKey";
constexpr OUStringLiteral UNO_NAME_AUTOMATIC_DISTANCE = u"AutomaticDistance";
constexpr OUStringLiteral UNO_NAME_AUTOMATIC_CONTROL_FOCUS = u"AutomaticControlFocus";
constexpr OUStringLiteral UNO_NAME_APPLY_FORM_DESIGN_MODE = u"ApplyFormDesignMode";
constexpr OUStringLiteral UNO_NAME_LOCALE = u"Locale";
constexpr OUStringLiteral UNO_NAME_SORT_ALGORITHM = u"SortAlgorithm";
constexpr OUStringLiteral UNO_NAME_FRAME_HEIGHT_ABSOLUTE = u"FrameHeightAbsolute";
constexpr OUStringLiteral UNO_NAME_FRAME_HEIGHT_PERCENT = u"FrameHeightPercent";
constexpr OUStringLiteral UNO_NAME_FRAME_ISAUTOMATIC_HEIGHT = u"FrameIsAutomaticHeight";
constexpr OUStringLiteral UNO_NAME_FRAME_WIDTH_ABSOLUTE = u"FrameWidthAbsolute";
constexpr OUStringLiteral UNO_NAME_FRAME_WIDTH_PERCENT = u"FrameWidthPercent";

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

constexpr OUStringLiteral UNO_NAME_HEADER_FILLBMP_LOGICAL_SIZE = u"HeaderFillBitmapLogicalSize";
constexpr OUStringLiteral UNO_NAME_HEADER_FILLBMP_OFFSET_X = u"HeaderFillBitmapOffsetX";
constexpr OUStringLiteral UNO_NAME_HEADER_FILLBMP_OFFSET_Y = u"HeaderFillBitmapOffsetY";
constexpr OUStringLiteral UNO_NAME_HEADER_FILLBMP_POSITION_OFFSET_X
    = u"HeaderFillBitmapPositionOffsetX";
constexpr OUStringLiteral UNO_NAME_HEADER_FILLBMP_POSITION_OFFSET_Y
    = u"HeaderFillBitmapPositionOffsetY";
constexpr OUStringLiteral UNO_NAME_HEADER_FILLBMP_RECTANGLE_POINT
    = u"HeaderFillBitmapRectanglePoint";
constexpr OUStringLiteral UNO_NAME_HEADER_FILLBMP_SIZE_X = u"HeaderFillBitmapSizeX";
constexpr OUStringLiteral UNO_NAME_HEADER_FILLBMP_SIZE_Y = u"HeaderFillBitmapSizeY";
constexpr OUStringLiteral UNO_NAME_HEADER_FILLBMP_STRETCH = u"HeaderFillBitmapStretch";
constexpr OUStringLiteral UNO_NAME_HEADER_FILLBMP_TILE = u"HeaderFillBitmapTile";
constexpr OUStringLiteral UNO_NAME_HEADER_FILLBMP_MODE = u"HeaderFillBitmapMode";
constexpr OUStringLiteral UNO_NAME_HEADER_FILLCOLOR = u"HeaderFillColor";
constexpr OUStringLiteral UNO_NAME_HEADER_FILLBACKGROUND = u"HeaderFillBackground";
constexpr OUStringLiteral UNO_NAME_HEADER_FILLBITMAP = u"HeaderFillBitmap";
constexpr OUStringLiteral UNO_NAME_HEADER_FILLBITMAPNAME = u"HeaderFillBitmapName";
constexpr OUStringLiteral UNO_NAME_HEADER_FILLGRADIENTSTEPCOUNT = u"HeaderFillGradientStepCount";
constexpr OUStringLiteral UNO_NAME_HEADER_FILLGRADIENT = u"HeaderFillGradient";
constexpr OUStringLiteral UNO_NAME_HEADER_FILLGRADIENTNAME = u"HeaderFillGradientName";
constexpr OUStringLiteral UNO_NAME_HEADER_FILLHATCH = u"HeaderFillHatch";
constexpr OUStringLiteral UNO_NAME_HEADER_FILLHATCHNAME = u"HeaderFillHatchName";
constexpr OUStringLiteral UNO_NAME_HEADER_FILLSTYLE = u"HeaderFillStyle";
constexpr OUStringLiteral UNO_NAME_HEADER_FILL_TRANSPARENCE = u"HeaderFillTransparence";
constexpr OUStringLiteral UNO_NAME_HEADER_FILLTRANSPARENCEGRADIENT
    = u"HeaderFillTransparenceGradient";
constexpr OUStringLiteral UNO_NAME_HEADER_FILLTRANSPARENCEGRADIENTNAME
    = u"HeaderFillTransparenceGradientName";
constexpr OUStringLiteral UNO_NAME_HEADER_FILLCOLOR_2 = u"HeaderFillColor2";

constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBMP_LOGICAL_SIZE = u"FooterFillBitmapLogicalSize";
constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBMP_OFFSET_X = u"FooterFillBitmapOffsetX";
constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBMP_OFFSET_Y = u"FooterFillBitmapOffsetY";
constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBMP_POSITION_OFFSET_X
    = u"FooterFillBitmapPositionOffsetX";
constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBMP_POSITION_OFFSET_Y
    = u"FooterFillBitmapPositionOffsetY";
constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBMP_RECTANGLE_POINT
    = u"FooterFillBitmapRectanglePoint";
constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBMP_SIZE_X = u"FooterFillBitmapSizeX";
constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBMP_SIZE_Y = u"FooterFillBitmapSizeY";
constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBMP_STRETCH = u"FooterFillBitmapStretch";
constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBMP_TILE = u"FooterFillBitmapTile";
constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBMP_MODE = u"FooterFillBitmapMode";
constexpr OUStringLiteral UNO_NAME_FOOTER_FILLCOLOR = u"FooterFillColor";
constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBACKGROUND = u"FooterFillBackground";
constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBITMAP = u"FooterFillBitmap";
constexpr OUStringLiteral UNO_NAME_FOOTER_FILLBITMAPNAME = u"FooterFillBitmapName";
constexpr OUStringLiteral UNO_NAME_FOOTER_FILLGRADIENTSTEPCOUNT = u"FooterFillGradientStepCount";
constexpr OUStringLiteral UNO_NAME_FOOTER_FILLGRADIENT = u"FooterFillGradient";
constexpr OUStringLiteral UNO_NAME_FOOTER_FILLGRADIENTNAME = u"FooterFillGradientName";
constexpr OUStringLiteral UNO_NAME_FOOTER_FILLHATCH = u"FooterFillHatch";
constexpr OUStringLiteral UNO_NAME_FOOTER_FILLHATCHNAME = u"FooterFillHatchName";
constexpr OUStringLiteral UNO_NAME_FOOTER_FILLSTYLE = u"FooterFillStyle";
constexpr OUStringLiteral UNO_NAME_FOOTER_FILL_TRANSPARENCE = u"FooterFillTransparence";
constexpr OUStringLiteral UNO_NAME_FOOTER_FILLTRANSPARENCEGRADIENT
    = u"FooterFillTransparenceGradient";
constexpr OUStringLiteral UNO_NAME_FOOTER_FILLTRANSPARENCEGRADIENTNAME
    = u"FooterFillTransparenceGradientName";
constexpr OUStringLiteral UNO_NAME_FOOTER_FILLCOLOR_2 = u"FooterFillColor2";

constexpr OUStringLiteral UNO_NAME_PARA_STYLEHEADING = u"ParaStyleHeading";
constexpr OUStringLiteral UNO_NAME_PARA_STYLELEVEL1 = u"ParaStyleLevel1";
constexpr OUStringLiteral UNO_NAME_PARA_STYLELEVEL10 = u"ParaStyleLevel10";
constexpr OUStringLiteral UNO_NAME_PARA_STYLELEVEL2 = u"ParaStyleLevel2";
constexpr OUStringLiteral UNO_NAME_PARA_STYLELEVEL3 = u"ParaStyleLevel3";
constexpr OUStringLiteral UNO_NAME_PARA_STYLELEVEL4 = u"ParaStyleLevel4";
constexpr OUStringLiteral UNO_NAME_PARA_STYLELEVEL5 = u"ParaStyleLevel5";
constexpr OUStringLiteral UNO_NAME_PARA_STYLELEVEL6 = u"ParaStyleLevel6";
constexpr OUStringLiteral UNO_NAME_PARA_STYLELEVEL7 = u"ParaStyleLevel7";
constexpr OUStringLiteral UNO_NAME_PARA_STYLELEVEL8 = u"ParaStyleLevel8";
constexpr OUStringLiteral UNO_NAME_PARA_STYLELEVEL9 = u"ParaStyleLevel9";
constexpr OUStringLiteral UNO_NAME_PARA_STYLESEPARATOR = u"ParaStyleSeparator";
constexpr OUStringLiteral UNO_NAME_MACRO_LIBRARY = u"MacroLibrary";
constexpr OUStringLiteral UNO_NAME_CELL_NAME = u"CellName";
constexpr OUStringLiteral UNO_NAME_PARA_USER_DEFINED_ATTRIBUTES = u"ParaUserDefinedAttributes";
constexpr OUStringLiteral UNO_NAME_MERGE_LAST_PARA = u"MergeLastPara";
constexpr OUStringLiteral UNO_NAME_WRITING_MODE = u"WritingMode";
constexpr OUStringLiteral UNO_NAME_GRID_COLOR = u"GridColor";
constexpr OUStringLiteral UNO_NAME_GRID_LINES = u"GridLines";
constexpr OUStringLiteral UNO_NAME_GRID_BASE_HEIGHT = u"GridBaseHeight";
constexpr OUStringLiteral UNO_NAME_GRID_RUBY_HEIGHT = u"GridRubyHeight";
constexpr OUStringLiteral UNO_NAME_GRID_MODE = u"GridMode";
constexpr OUStringLiteral UNO_NAME_GRID_RUBY_BELOW = u"RubyBelow";
constexpr OUStringLiteral UNO_NAME_GRID_PRINT = u"GridPrint";
constexpr OUStringLiteral UNO_NAME_GRID_DISPLAY = u"GridDisplay";
constexpr OUStringLiteral UNO_NAME_SNAP_TO_GRID = u"SnapToGrid";
constexpr OUStringLiteral UNO_NAME_DONT_BALANCE_TEXT_COLUMNS = u"DontBalanceTextColumns";
constexpr OUStringLiteral UNO_NAME_IS_CURRENTLY_VISIBLE = u"IsCurrentlyVisible";
constexpr OUStringLiteral UNO_NAME_ANCHOR_POSITION = u"AnchorPosition";
constexpr OUStringLiteral UNO_NAME_TEXT_READING = u"TextReading";
constexpr OUStringLiteral UNO_NAME_PRIMARY_KEY_READING = u"PrimaryKeyReading";
constexpr OUStringLiteral UNO_NAME_SECONDARY_KEY_READING = u"SecondaryKeyReading";
constexpr OUStringLiteral UNO_NAME_BACK_GRAPHIC_TRANSPARENCY = u"BackGraphicTransparency";
constexpr OUStringLiteral UNO_NAME_BACK_COLOR_R_G_B = u"BackColorRGB";
constexpr OUStringLiteral UNO_NAME_BACK_COLOR_TRANSPARENCY = u"BackColorTransparency";
constexpr OUStringLiteral UNO_NAME_HEADER_DYNAMIC_SPACING = u"HeaderDynamicSpacing";
constexpr OUStringLiteral UNO_NAME_FOOTER_DYNAMIC_SPACING = u"FooterDynamicSpacing";
constexpr OUStringLiteral UNO_NAME_BASIC_LIBRARIES = u"BasicLibraries";
constexpr OUStringLiteral UNO_NAME_IS_FIXED_LANGUAGE = u"IsFixedLanguage";
constexpr OUStringLiteral UNO_NAME_SELECTION = u"Selection";
constexpr OUStringLiteral UNO_NAME_RESULT_SET = u"ResultSet";
constexpr OUStringLiteral UNO_NAME_CONNECTION = u"ActiveConnection";
constexpr OUStringLiteral UNO_NAME_MODEL = u"Model";
constexpr OUStringLiteral UNO_NAME_DATA_SOURCE_NAME = u"DataSourceName";
constexpr OUStringLiteral UNO_NAME_FILTER = u"Filter";
constexpr OUStringLiteral UNO_NAME_DOCUMENT_URL = u"DocumentURL";
constexpr OUStringLiteral UNO_NAME_OUTPUT_URL = u"OutputURL";
constexpr OUStringLiteral UNO_NAME_OUTPUT_TYPE = u"OutputType";
constexpr OUStringLiteral UNO_NAME_ESCAPE_PROCESSING = u"EscapeProcessing";
constexpr OUStringLiteral UNO_NAME_SINGLE_PRINT_JOBS = u"SinglePrintJobs";
constexpr OUStringLiteral UNO_NAME_FILE_NAME_FROM_COLUMN = u"FileNameFromColumn";
constexpr OUStringLiteral UNO_NAME_FILE_NAME_PREFIX = u"FileNamePrefix";
constexpr OUStringLiteral UNO_NAME_CHAR_STYLE_NAMES = u"CharStyleNames";
constexpr OUStringLiteral UNO_NAME_DAD_COMMAND
    = u"Command"; /* DAD = DataAccessDescriptor (see com.sun.star.sdb.DataAccessDescriptor) */
constexpr OUStringLiteral UNO_NAME_DAD_COMMAND_TYPE = u"CommandType";
constexpr OUStringLiteral UNO_NAME_CLSID = u"CLSID";
constexpr OUStringLiteral UNO_NAME_COMPONENT = u"Component";
constexpr OUStringLiteral UNO_NAME_SECT_LEFT_MARGIN = u"SectionLeftMargin";
constexpr OUStringLiteral UNO_NAME_SECT_RIGHT_MARGIN = u"SectionRightMargin";
constexpr OUStringLiteral UNO_NAME_PARA_IS_CONNECT_BORDER = u"ParaIsConnectBorder";
constexpr OUStringLiteral UNO_NAME_ITEMS = u"Items";
constexpr OUStringLiteral UNO_NAME_SELITEM = u"SelectedItem";
constexpr OUStringLiteral UNO_NAME_IS_SPLIT_ALLOWED = u"IsSplitAllowed";
constexpr OUStringLiteral UNO_NAME_HAS_TEXT_CHANGES_ONLY = u"HasTextChangesOnly";
constexpr OUStringLiteral UNO_NAME_CHAR_HIDDEN = u"CharHidden";
constexpr OUStringLiteral UNO_NAME_IS_FOLLOWING_TEXT_FLOW = u"IsFollowingTextFlow";
constexpr OUStringLiteral UNO_NAME_WIDTH_TYPE = u"WidthType";
constexpr OUStringLiteral UNO_NAME_SCRIPT_URL = u"ScriptURL";
constexpr OUStringLiteral UNO_NAME_RUNTIME_UID = u"RuntimeUID";
constexpr OUStringLiteral UNO_NAME_COLLAPSING_BORDERS = u"CollapsingBorders";
constexpr OUStringLiteral UNO_NAME_WRAP_INFLUENCE_ON_POSITION = u"WrapInfluenceOnPosition";
constexpr OUStringLiteral UNO_NAME_DATA_BASE_URL = u"DataBaseURL";
constexpr OUStringLiteral UNO_NAME_TRANSFORMATION_IN_HORI_L2R = u"TransformationInHoriL2R";
constexpr OUStringLiteral UNO_NAME_POSITION_LAYOUT_DIR = u"PositionLayoutDir";
constexpr OUStringLiteral UNO_NAME_NUMBERING_IS_OUTLINE = u"NumberingIsOutline";
constexpr OUStringLiteral UNO_NAME_STARTPOSITION_IN_HORI_L2R = u"StartPositionInHoriL2R";
constexpr OUStringLiteral UNO_NAME_ENDPOSITION_IN_HORI_L2R = u"EndPositionInHoriL2R";
constexpr OUStringLiteral UNO_NAME_SUBJECT = u"Subject";
constexpr OUStringLiteral UNO_NAME_ADDRESS_FROM_COLUMN = u"AddressFromColumn";
constexpr OUStringLiteral UNO_NAME_SEND_AS_HTML = u"SendAsHTML";
constexpr OUStringLiteral UNO_NAME_SEND_AS_ATTACHMENT = u"SendAsAttachment";
constexpr OUStringLiteral UNO_NAME_MAIL_BODY = u"MailBody";
constexpr OUStringLiteral UNO_NAME_ATTACHMENT_NAME = u"AttachmentName";
constexpr OUStringLiteral UNO_NAME_ATTACHMENT_FILTER = u"AttachmentFilter";
constexpr OUStringLiteral UNO_NAME_PRINT_OPTIONS = u"PrintOptions";
constexpr OUStringLiteral UNO_NAME_SAVE_AS_SINGLE_FILE = u"SaveAsSingleFile";
constexpr OUStringLiteral UNO_NAME_SAVE_FILTER = u"SaveFilter";
constexpr OUStringLiteral UNO_NAME_COPIES_TO = u"CopiesTo";
constexpr OUStringLiteral UNO_NAME_BLIND_COPIES_TO = u"BlindCopiesTo";
constexpr OUStringLiteral UNO_NAME_IN_SERVER_PASSWORD = u"InServerPassword";
constexpr OUStringLiteral UNO_NAME_OUT_SERVER_PASSWORD = u"OutServerPassword";
constexpr OUStringLiteral UNO_NAME_SAVE_FILTER_OPTIONS = u"SaveFilterOptions";
constexpr OUStringLiteral UNO_NAME_SAVE_FILTER_DATA = u"SaveFilterData";
constexpr OUStringLiteral UNO_NAME_LOCK_UPDATES = u"LockUpdates";
constexpr OUStringLiteral UNO_NAME_PAGE_COUNT = u"PageCount";
constexpr OUStringLiteral UNO_NAME_LINE_COUNT = u"LineCount";
constexpr OUStringLiteral UNO_NAME_IS_CONSTANT_SPELLCHECK = u"IsConstantSpellcheck";
constexpr OUStringLiteral UNO_NAME_IS_HIDE_SPELL_MARKS
    = u"IsHideSpellMarks"; /* deprecated #i91949 */

constexpr OUStringLiteral UNO_NAME_IS_FIELD_USED = u"IsFieldUsed";
constexpr OUStringLiteral UNO_NAME_IS_FIELD_DISPLAYED = u"IsFieldDisplayed";
constexpr OUStringLiteral UNO_NAME_BUILDID = u"BuildId";
constexpr OUStringLiteral UNO_NAME_PARA_STYLE_CONDITIONS = u"ParaStyleConditions";
constexpr OUStringLiteral UNO_NAME_ROLE = u"Role";
constexpr OUStringLiteral UNO_NAME_FIELD_CODE = u"FieldCode";
constexpr OUStringLiteral UNO_NAME_HAS_VALID_SIGNATURES = u"HasValidSignatures";
constexpr OUStringLiteral UNO_NAME_CHAR_AUTO_STYLE_NAME = u"CharAutoStyleName";
constexpr OUStringLiteral UNO_NAME_PARA_AUTO_STYLE_NAME = u"ParaAutoStyleName";
constexpr OUStringLiteral UNO_NAME_LAYOUT_SIZE = u"LayoutSize";
constexpr OUStringLiteral UNO_NAME_HELP = u"Help";
constexpr OUStringLiteral UNO_NAME_TOOLTIP = u"Tooltip";
constexpr OUStringLiteral UNO_NAME_DIALOG_LIBRARIES = u"DialogLibraries";
constexpr OUStringLiteral UNO_NAME_HEADER_ROW_COUNT = u"HeaderRowCount";
constexpr OUStringLiteral UNO_NAME_INPUT = u"Input";
constexpr OUStringLiteral UNO_NAME_ROW_SPAN = u"RowSpan";
constexpr OUStringLiteral UNO_NAME_POSITION_AND_SPACE_MODE = u"PositionAndSpaceMode";
constexpr OUStringLiteral UNO_NAME_LABEL_FOLLOWED_BY = u"LabelFollowedBy";
constexpr OUStringLiteral UNO_NAME_LISTTAB_STOP_POSITION = u"ListtabStopPosition";
constexpr OUStringLiteral UNO_NAME_FIRST_LINE_INDENT = u"FirstLineIndent";
constexpr OUStringLiteral UNO_NAME_INDENT_AT = u"IndentAt";
constexpr OUStringLiteral UNO_NAME_GRID_BASE_WIDTH = u"GridBaseWidth";
constexpr OUStringLiteral UNO_NAME_GRID_SNAP_TO_CHARS = u"GridSnapToChars";
constexpr OUStringLiteral UNO_NAME_GRID_STANDARD_PAGE_MODE = u"StandardPageMode";
constexpr OUStringLiteral UNO_NAME_TABLE_BORDER_DISTANCES = u"TableBorderDistances";
constexpr OUStringLiteral UNO_NAME_LIST_ID = u"ListId";
constexpr OUStringLiteral UNO_NAME_DEFAULT_LIST_ID = u"DefaultListId";
constexpr OUStringLiteral UNO_NAME_STREAM_NAME = u"StreamName";
constexpr OUStringLiteral UNO_NAME_PARA_CONTINUEING_PREVIOUS_SUB_TREE
    = u"ContinueingPreviousSubTree";
constexpr OUStringLiteral UNO_NAME_PARA_LIST_LABEL_STRING = u"ListLabelString";
constexpr OUStringLiteral UNO_NAME_PARA_LIST_AUTO_FORMAT = u"ListAutoFormat";
constexpr OUStringLiteral UNO_NAME_CHAR_OVERLINE = u"CharOverline";
constexpr OUStringLiteral UNO_NAME_CHAR_OVERLINE_COLOR = u"CharOverlineColor";
constexpr OUStringLiteral UNO_NAME_CHAR_OVERLINE_HAS_COLOR = u"CharOverlineHasColor";
constexpr OUStringLiteral UNO_NAME_OUTLINE_CONTENT_VISIBLE = u"OutlineContentVisible";
constexpr OUStringLiteral UNO_NAME_OUTLINE_LEVEL = u"OutlineLevel";
constexpr OUStringLiteral UNO_NAME_DESCRIPTION = u"Description";
constexpr OUStringLiteral UNO_NAME_META = u"InContentMetadata";
constexpr OUStringLiteral UNO_NAME_IS_TEMPLATE = u"IsTemplate";
constexpr OUStringLiteral UNO_NAME_VBA_DOCOBJ = u"VBAGlobalConstantName";
constexpr OUStringLiteral UNO_NAME_NESTED_TEXT_CONTENT = u"NestedTextContent";
constexpr OUStringLiteral UNO_NAME_SEPARATOR_LINE_STYLE = u"SeparatorLineStyle";
constexpr OUStringLiteral UNO_NAME_FOOTNOTE_LINE_STYLE = u"FootnoteLineStyle";
constexpr OUStringLiteral UNO_NAME_EMBEDDED_OBJECT = u"EmbeddedObject";
constexpr OUStringLiteral UNO_NAME_RSID = u"Rsid";
constexpr OUStringLiteral UNO_NAME_PARRSID = u"ParRsid";
constexpr OUStringLiteral UNO_NAME_DRAW_ASPECT = u"DrawAspect";
constexpr OUStringLiteral UNO_NAME_VISIBLE_AREA_WIDTH = u"VisibleAreaWidth";
constexpr OUStringLiteral UNO_NAME_VISIBLE_AREA_HEIGHT = u"VisibleAreaHeight";

constexpr OUStringLiteral UNO_NAME_LINE_STYLE = u"LineStyle";
constexpr OUStringLiteral UNO_NAME_LINE_WIDTH = u"LineWidth";

constexpr OUStringLiteral UNO_NAME_PARA_CONTEXT_MARGIN = u"ParaContextMargin";
#define UNO_NAME_PARAGRAPH_STYLE_NAME "ParagraphStyleName"
constexpr OUStringLiteral UNO_NAME_FIRST_IS_SHARED = u"FirstIsShared";
constexpr OUStringLiteral UNO_NAME_HEADER_TEXT_FIRST = u"HeaderTextFirst";
constexpr OUStringLiteral UNO_NAME_FOOTER_TEXT_FIRST = u"FooterTextFirst";
constexpr OUStringLiteral UNO_NAME_INITIALS = u"Initials";
constexpr OUStringLiteral UNO_NAME_TABLE_BORDER2 = u"TableBorder2";
constexpr OUStringLiteral UNO_NAME_REPLACEMENT_GRAPHIC = u"ReplacementGraphic";
constexpr OUStringLiteral UNO_NAME_HIDDEN = u"Hidden";
constexpr OUStringLiteral UNO_NAME_DEFAULT_PAGE_MODE = u"DefaultPageMode";
constexpr OUStringLiteral UNO_NAME_CHAR_SHADING_VALUE = u"CharShadingValue";
constexpr OUStringLiteral UNO_NAME_PARA_INTEROP_GRAB_BAG = u"ParaInteropGrabBag";
constexpr OUStringLiteral UNO_NAME_CHAR_LEFT_BORDER = u"CharLeftBorder";
constexpr OUStringLiteral UNO_NAME_CHAR_RIGHT_BORDER = u"CharRightBorder";
constexpr OUStringLiteral UNO_NAME_CHAR_TOP_BORDER = u"CharTopBorder";
constexpr OUStringLiteral UNO_NAME_CHAR_BOTTOM_BORDER = u"CharBottomBorder";
constexpr OUStringLiteral UNO_NAME_CHAR_BORDER_DISTANCE = u"CharBorderDistance";
constexpr OUStringLiteral UNO_NAME_CHAR_LEFT_BORDER_DISTANCE = u"CharLeftBorderDistance";
constexpr OUStringLiteral UNO_NAME_CHAR_RIGHT_BORDER_DISTANCE = u"CharRightBorderDistance";
constexpr OUStringLiteral UNO_NAME_CHAR_TOP_BORDER_DISTANCE = u"CharTopBorderDistance";
constexpr OUStringLiteral UNO_NAME_CHAR_BOTTOM_BORDER_DISTANCE = u"CharBottomBorderDistance";
constexpr OUStringLiteral UNO_NAME_CHAR_SHADOW_FORMAT = u"CharShadowFormat";
constexpr OUStringLiteral UNO_NAME_SHADOW_TRANSPARENCE = u"ShadowTransparence";
constexpr OUStringLiteral UNO_NAME_DOC_INTEROP_GRAB_BAG = u"InteropGrabBag";
constexpr OUStringLiteral UNO_NAME_FRAME_INTEROP_GRAB_BAG = u"FrameInteropGrabBag";
constexpr OUStringLiteral UNO_NAME_CHAR_HIGHLIGHT = u"CharHighlight";
constexpr OUStringLiteral UNO_NAME_STYLE_INTEROP_GRAB_BAG = u"StyleInteropGrabBag";
constexpr OUStringLiteral UNO_NAME_CHAR_INTEROP_GRAB_BAG = u"CharInteropGrabBag";
constexpr OUStringLiteral UNO_NAME_TEXT_VERT_ADJUST = u"TextVerticalAdjust";
constexpr OUStringLiteral UNO_NAME_CELL_INTEROP_GRAB_BAG = u"CellInteropGrabBag";
constexpr OUStringLiteral UNO_NAME_ROW_INTEROP_GRAB_BAG = u"RowInteropGrabBag";
constexpr OUStringLiteral UNO_NAME_TABLE_INTEROP_GRAB_BAG = u"TableInteropGrabBag";

constexpr OUStringLiteral UNO_NAME_TABLE_FIRST_ROW_END_COLUMN = u"FirstRowEndColumn";
constexpr OUStringLiteral UNO_NAME_TABLE_FIRST_ROW_START_COLUMN = u"FirstRowStartColumn";
constexpr OUStringLiteral UNO_NAME_TABLE_LAST_ROW_END_COLUMN = u"LastRowEndColumn";
constexpr OUStringLiteral UNO_NAME_TABLE_LAST_ROW_START_COLUMN = u"LastRowStartColumn";

constexpr OUStringLiteral UNO_NAME_RESOLVED = u"Resolved";
constexpr OUStringLiteral UNO_NAME_ALLOW_OVERLAP = u"AllowOverlap";
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

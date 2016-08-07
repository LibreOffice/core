/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#include <PropertyIds.hxx>
#include <rtl/ustring.hxx>
#include <map>
#include <string>
#include <stdio.h>

namespace writerfilter {
namespace dmapper{

typedef ::std::map< PropertyIds, rtl::OUString> PropertyNameMap_t;
typedef ::std::map< rtl::OUString, PropertyIds> PropertyBackwardNameMap_t;

struct PropertyNameSupplier_Impl
{
    PropertyNameMap_t aNameMap;
    PropertyBackwardNameMap_t aNameBackwardMap;

    void add(PropertyIds aId, const rtl::OUString & aStr);

    PropertyNameSupplier_Impl();
};

PropertyNameSupplier_Impl::PropertyNameSupplier_Impl()
{
    add(PROP_CHAR_WEIGHT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharWeight")));
    add(PROP_CHAR_POSTURE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharPosture")));
    add(PROP_CHAR_STRIKEOUT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharStrikeout")));
    add(PROP_CHAR_CONTOURED, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharContoured")));
    add(PROP_CHAR_SHADOWED, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharShadowed")));
    add(PROP_CHAR_CASE_MAP, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharCaseMap")));
    add(PROP_CHAR_COLOR, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharColor")));
    add(PROP_CHAR_RELIEF, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharRelief")));
    add(PROP_CHAR_UNDERLINE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharUnderline")));
    add(PROP_CHAR_UNDERLINE_COLOR, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharUnderlineColor")));
    add(PROP_CHAR_UNDERLINE_HAS_COLOR, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharUnderlineHasColor")));
    add(PROP_CHAR_WORD_MODE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharWordMode")));
    add(PROP_CHAR_ESCAPEMENT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharEscapement")));
    add(PROP_CHAR_ESCAPEMENT_HEIGHT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharEscapementHeight")));
    add(PROP_CHAR_HEIGHT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharHeight")));
    add(PROP_CHAR_HEIGHT_COMPLEX, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharHeightComplex")));
    add(PROP_CHAR_LOCALE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharLocale")));
    add(PROP_CHAR_LOCALE_ASIAN, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharLocaleAsian")));
    add(PROP_CHAR_LOCALE_COMPLEX, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharLocaleComplex")));
    add(PROP_CHAR_WEIGHT_COMPLEX, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharWeightComplex")));
    add(PROP_CHAR_POSTURE_COMPLEX, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharPostureComplex")));
    add(PROP_CHAR_CHAR_KERNING, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharKerning")));
    add(PROP_CHAR_AUTO_KERNING, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharAutoKerning")));
    add(PROP_CHAR_SCALE_WIDTH, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharScaleWidth")));
    add(PROP_CHAR_STYLE_NAME, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharStyleName")));
    add(PROP_CHAR_FONT_NAME, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontName")));
    add(PROP_CHAR_FONT_STYLE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontStyle")));
    add(PROP_CHAR_FONT_FAMILY, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontFamily")));
    add(PROP_CHAR_FONT_CHAR_SET, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontCharSet")));
    add(PROP_CHAR_FONT_PITCH, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontPitch")));
    add(PROP_CHAR_FONT_NAME_ASIAN, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontNameAsian")));
    add(PROP_CHAR_HEIGHT_ASIAN, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharHeightAsian")));
    add(PROP_CHAR_FONT_STYLE_ASIAN, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontStyleAsian")));
    add(PROP_CHAR_FONT_FAMILY_ASIAN, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontFamilyAsian")));
    add(PROP_CHAR_FONT_CHAR_SET_ASIAN, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontCharSetAsian")));
    add(PROP_CHAR_FONT_PITCH_ASIAN, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontPitchAsian")));
    add(PROP_CHAR_FONT_NAME_COMPLEX, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontNameComplex")));
    add(PROP_CHAR_FONT_STYLE_COMPLEX, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontStyleComplex")));
    add(PROP_CHAR_FONT_FAMILY_COMPLEX, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontFamilyComplex")));
    add(PROP_CHAR_FONT_CHAR_SET_COMPLEX, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontCharSetComplex")));
    add(PROP_CHAR_FONT_PITCH_COMPLEX, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFontPitchComplex")));
    add(PROP_CHAR_HIDDEN, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharHidden")));
    add(PROP_CHAR_WEIGHT_ASIAN, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharWeightAsian")));
    add(PROP_CHAR_POSTURE_ASIAN, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharPostureAsian")));
    add(PROP_CHAR_BACK_COLOR, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharBackColor")));
    add(PROP_CHAR_EMPHASIS, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharEmphasis")));
    add(PROP_CHAR_COMBINE_IS_ON, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharCombineIsOn")));
    add(PROP_CHAR_COMBINE_PREFIX, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharCombinePrefix")));
    add(PROP_CHAR_COMBINE_SUFFIX, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharCombineSuffix")));
    add(PROP_CHAR_ROTATION, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharRotation")));
    add(PROP_CHAR_ROTATION_IS_FIT_TO_LINE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharRotationIsFitToLine")));
    add(PROP_CHAR_FLASH, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharFlash")));
    add(PROP_PARA_STYLE_NAME, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaStyleName")));
    add(PROP_PARA_ADJUST, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaAdjust")));
    add(PROP_PARA_LAST_LINE_ADJUST, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaLastLineAdjust")));
    add(PROP_PARA_RIGHT_MARGIN, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaRightMargin")));
    add(PROP_PARA_LEFT_MARGIN, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaLeftMargin")));
    add(PROP_PARA_FIRST_LINE_INDENT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaFirstLineIndent")));
    add(PROP_PARA_KEEP_TOGETHER, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaKeepTogether")));
    add(PROP_PARA_TOP_MARGIN, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaTopMargin")));
    add(PROP_PARA_BOTTOM_MARGIN, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaBottomMargin")));
    add(PROP_PARA_IS_HYPHENATION, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaIsHyphenation")));
    add(PROP_PARA_LINE_NUMBER_COUNT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaLineNumberCount")));
    add(PROP_PARA_IS_HANGING_PUNCTUATION, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaIsHangingPunctuation")));
    add(PROP_PARA_LINE_SPACING, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaLineSpacing")));
    add(PROP_PARA_TAB_STOPS, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaTabStops")));
    add(PROP_PARA_WIDOWS, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaWidows")));
    add(PROP_PARA_ORPHANS, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaOrphans")));
    add(PROP_PARA_LINE_NUMBER_START_VALUE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaLineNumberStartValue")));
    add(PROP_NUMBERING_LEVEL, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NumberingLevel")));
    add(PROP_NUMBERING_RULES, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NumberingRules")));
    add(PROP_NUMBERING_TYPE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NumberingType")));
    add(PROP_START_WITH, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StartWith")));
    add(PROP_ADJUST, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Adjust")));
    add(PROP_PARENT_NUMBERING, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParentNumbering")));
    add(PROP_RIGHT_MARGIN, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RightMargin")));
    add(PROP_LEFT_MARGIN, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LeftMargin")));
    add(PROP_TOP_MARGIN, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TopMargin")));
    add(PROP_BOTTOM_MARGIN, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BottomMargin")));
    add(PROP_FIRST_LINE_OFFSET, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FirstLineOffset")));
    add(PROP_LEFT_BORDER, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LeftBorder")));
    add(PROP_RIGHT_BORDER, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RightBorder")));
    add(PROP_TOP_BORDER, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TopBorder")));
    add(PROP_BOTTOM_BORDER, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BottomBorder")));
    add(PROP_TABLE_BORDER, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TableBorder")));
    add(PROP_LEFT_BORDER_DISTANCE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LeftBorderDistance")));
    add(PROP_RIGHT_BORDER_DISTANCE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RightBorderDistance")));
    add(PROP_TOP_BORDER_DISTANCE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TopBorderDistance")));
    add(PROP_BOTTOM_BORDER_DISTANCE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BottomBorderDistance")));
    add(PROP_CURRENT_PRESENTATION, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CurrentPresentation")));
    add(PROP_IS_FIXED, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsFixed")));
    add(PROP_SUB_TYPE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SubType")));
    add(PROP_FILE_FORMAT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FileFormat")));
    add(PROP_HYPER_LINK_U_R_L, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HyperLinkURL")));
    add(PROP_NUMBER_FORMAT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NumberFormat")));
    add(PROP_NAME, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name")));
    add(PROP_IS_INPUT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsInput")));
    add(PROP_HINT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Hint")));
    add(PROP_FULL_NAME, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FullName")));
    add(PROP_KEYWORDS, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Keywords")));
    add(PROP_DESCRIPTION, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Description")));
    add(PROP_MACRO_NAME, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MacroName")));
    add(PROP_SUBJECT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Subject")));
    add(PROP_USER_DATA_TYPE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UserDataType")));
    add(PROP_TITLE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Title")));
    add(PROP_CONTENT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Content")));
    add(PROP_DATA_COLUMN_NAME, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataColumnName")));
    add(PROP_INPUT_STREAM, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("InputStream")));
    add(PROP_GRAPHIC, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Graphic")));
    add(PROP_ANCHOR_TYPE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AnchorType")));
    add(PROP_SIZE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Size")));
    add(PROP_HORI_ORIENT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HoriOrient")));
    add(PROP_HORI_ORIENT_POSITION, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HoriOrientPosition")));
    add(PROP_HORI_ORIENT_RELATION, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HoriOrientRelation")));
    add(PROP_VERT_ORIENT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VertOrient")));
    add(PROP_VERT_ORIENT_POSITION, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VertOrientPosition")));
    add(PROP_VERT_ORIENT_RELATION, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VertOrientRelation")));
    add(PROP_GRAPHIC_CROP, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GraphicCrop")));
    add(PROP_SIZE100th_M_M, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Size100thMM")));
    add(PROP_SIZE_PIXEL, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SizePixel")));
    add(PROP_SURROUND, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Surround")));
    add(PROP_SURROUND_CONTOUR, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SurroundContour")));
    add(PROP_ADJUST_CONTRAST, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AdjustContrast")));
    add(PROP_ADJUST_LUMINANCE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AdjustLuminance")));
    add(PROP_GRAPHIC_COLOR_MODE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GraphicColorMode")));
    add(PROP_GAMMA, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Gamma")));
    add(PROP_HORI_MIRRORED_ON_EVEN_PAGES, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HoriMirroredOnEvenPages")));
    add(PROP_HORI_MIRRORED_ON_ODD_PAGES, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HoriMirroredOnOddPages")));
    add(PROP_VERT_MIRRORED, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VertMirrored")));
    add(PROP_CONTOUR_OUTSIDE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ContourOutside")));
    add(PROP_CONTOUR_POLY_POLYGON, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ContourPolyPolygon")));
    add(PROP_PAGE_TOGGLE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PageToggle")));
    add(PROP_BACK_COLOR, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BackColor")));
    add(PROP_ALTERNATIVE_TEXT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AlternativeText")));
    add(PROP_HEADER_TEXT_LEFT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HeaderTextLeft")));
    add(PROP_HEADER_TEXT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HeaderText")));
    add(PROP_HEADER_IS_SHARED, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HeaderIsShared")));
    add(PROP_HEADER_IS_ON, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HeaderIsOn")));
    add(PROP_FOOTER_TEXT_LEFT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FooterTextLeft")));
    add(PROP_FOOTER_TEXT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FooterText")));
    add(PROP_FOOTER_IS_SHARED, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FooterIsShared")));
    add(PROP_FOOTER_IS_ON, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FooterIsOn")));
    add(PROP_WIDTH, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Width")));
    add(PROP_HEIGHT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Height")));
    add(PROP_SEPARATOR_LINE_IS_ON, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SeparatorLineIsOn")));
    add(PROP_TEXT_COLUMNS, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TextColumns")));
    add(PROP_AUTOMATIC_DISTANCE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AutomaticDistance")));
    add(PROP_IS_LANDSCAPE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsLandscape")));
    add(PROP_PRINTER_PAPER_TRAY_INDEX, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PrinterPaperTrayIndex")));
    add(PROP_FIRST_PAGE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("First Page")));
    add(PROP_DEFAULT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Default")));
    add(PROP_PAGE_DESC_NAME, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PageDescName")));
    add(PROP_PAGE_NUMBER_OFFSET, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PageNumberOffset")));
    add(PROP_BREAK_TYPE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BreakType")));
    add(PROP_FOOTER_IS_DYNAMIC_HEIGHT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FooterIsDynamicHeight")));
    add(PROP_FOOTER_DYNAMIC_SPACING, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FooterDynamicSpacing")));
    add(PROP_FOOTER_HEIGHT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FooterHeight")));
    add(PROP_FOOTER_BODY_DISTANCE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FooterBodyDistance")));
    add(PROP_HEADER_IS_DYNAMIC_HEIGHT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HeaderIsDynamicHeight")));
    add(PROP_HEADER_DYNAMIC_SPACING, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HeaderDynamicSpacing")));
    add(PROP_HEADER_HEIGHT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HeaderHeight")));
    add(PROP_HEADER_BODY_DISTANCE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HeaderBodyDistance")));
    add(PROP_WRITING_MODE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WritingMode")));
    add(PROP_GRID_MODE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GridMode")));
    add(PROP_GRID_DISPLAY, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GridDisplay")));
    add(PROP_GRID_PRINT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GridPrint")));
    add(PROP_ADD_EXTERNAL_LEADING, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AddExternalLeading")));
    add(PROP_GRID_LINES, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GridLines")));
    add(PROP_GRID_BASE_HEIGHT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GridBaseHeight")));
    add(PROP_GRID_RUBY_HEIGHT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GridRubyHeight")));
    add(PROP_GRID_STANDARD_MODE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StandardPageMode")));
    add(PROP_IS_ON, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsOn")));
    add(PROP_RESTART_AT_EACH_PAGE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RestartAtEachPage")));
    add(PROP_COUNT_EMPTY_LINES, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CountEmptyLines")));
    add(PROP_COUNT_LINES_IN_FRAMES, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CountLinesInFrames")));
    add(PROP_INTERVAL, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Interval")));
    add(PROP_DISTANCE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Distance")));
    add(PROP_NUMBER_POSITION, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NumberPosition")));
    add(PROP_LEVEL, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Level")));
    add(PROP_LEVEL_PARAGRAPH_STYLES, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LevelParagraphStyles")));
    add(PROP_LEVEL_FORMAT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LevelFormat")));
    add(PROP_TOKEN_TYPE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TokenType")));
    add(PROP_TOKEN_HYPERLINK_START, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TokenHyperlinkStart")));
    add(PROP_TOKEN_HYPERLINK_END, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TokenHyperlinkEnd")));
    add(PROP_TOKEN_CHAPTER_INFO, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TokenChapterInfo")));
    add(PROP_CHAPTER_FORMAT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ChapterFormat")));
    add(PROP_TOKEN_TEXT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TokenText")));
    add(PROP_TEXT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Text")));
    add(PROP_CREATE_FROM_OUTLINE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CreateFromOutline")));
    add(PROP_CREATE_FROM_MARKS, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CreateFromMarks")));
    add(PROP_STANDARD, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Standard")));
    add(PROP_IS_SPLIT_ALLOWED, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsSplitAllowed")));
    add(META_PROP_VERTICAL_BORDER, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VerticalBorder")));
    add(META_PROP_HORIZONTAL_BORDER, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HorizontalBorder")));
    add(PROP_HEADER_ROW_COUNT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HeaderRowCount")));
    add(PROP_IS_AUTO_HEIGHT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsAutoHeight")));
    add(PROP_SIZE_TYPE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SizeType")));
    add(PROP_TABLE_COLUMN_SEPARATORS, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TableColumnSeparators")));
    add(META_PROP_TABLE_STYLE_NAME, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TableStyleName")));
    add(PROP_REDLINE_AUTHOR, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RedlineAuthor")));
    add(PROP_REDLINE_DATE_TIME, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RedlineDateTime")));
    add(PROP_REDLINE_COMMENT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RedlineComment")));
    add(PROP_REDLINE_TYPE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RedlineType")));
    add(PROP_REDLINE_SUCCESSOR_DATA, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RedlineSuccessorData")));
    add(PROP_REDLINE_IDENTIFIER, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RedlineIdentifier")));
    add(PROP_SIZE_PROTECTED, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SizeProtected")));
    add(PROP_POSITION_PROTECTED, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PositionProtected")));
    add(PROP_OPAQUE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Opaque")));
    add(PROP_VERTICAL_MERGE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VerticalMerge")));
    add(PROP_BULLET_CHAR, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BulletChar")));
    add(PROP_BULLET_FONT_NAME, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BulletFontName")));
    add(PROP_PARA_BACK_COLOR, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaBackColor")));
    add(PROP_TABS_RELATIVE_TO_INDENT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TabsRelativeToIndent")));
    add(PROP_PREFIX, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Prefix")));
    add(PROP_SUFFIX, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Suffix")));
    add(PROP_CREATE_FROM_LEVEL_PARAGRAPH_STYLES, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CreateFromLevelParagraphStyles")));
    add(PROP_DROP_CAP_FORMAT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DropCapFormat")));
    add(PROP_REFERENCE_FIELD_PART, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReferenceFieldPart")));
    add(PROP_SOURCE_NAME, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SourceName")));
    add(PROP_REFERENCE_FIELD_SOURCE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReferenceFieldSource")));
    add(PROP_WIDTH_TYPE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("WidthType")));
    add( PROP_TEXT_RANGE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TextRange")));
    add(PROP_SERVICE_CHAR_STYLE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.style.CharacterStyle")));
    add(PROP_SERVICE_PARA_STYLE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.style.ParagraphStyle")));
    add(PROP_CHARACTER_STYLES, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharacterStyles")));
    add(PROP_PARAGRAPH_STYLES, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParagraphStyles")));
    add(PROP_TABLE_BORDER_DISTANCES, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TableBorderDistances")));
    add(META_PROP_CELL_MAR_TOP, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MetaPropCellMarTop")));
    add(META_PROP_CELL_MAR_BOTTOM, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MetaPropCellMarBottom")));
    add(META_PROP_CELL_MAR_LEFT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MetaPropCellMarLeft")));
    add(META_PROP_CELL_MAR_RIGHT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MetaPropCellMarRight")));
    add(PROP_START_AT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StartAt")));
    add(PROP_ADD_PARA_TABLE_SPACING, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AddParaTableSpacing")));
    add(PROP_CHAR_PROP_HEIGHT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharPropHeight")));
    add(PROP_CHAR_PROP_HEIGHT_ASIAN, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharPropHeightAsian")));
    add(PROP_CHAR_PROP_HEIGHT_COMPLEX, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharPropHeightComplex")));
    add(PROP_FORMAT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Format")));
    add(PROP_INSERT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Insert")));
    add(PROP_DELETE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Delete")));
    add(PROP_STREAM_NAME, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StreamName")));
    add(PROP_BITMAP, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Bitmap")));
    add(PROP_IS_DATE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsDate")));
    add(PROP_TAB_STOP_DISTANCE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TabStopDistance")));
    add(PROP_CNF_STYLE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CnfStyle")));
    add(PROP_INDENT_AT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IndentAt")));
    add(PROP_FIRST_LINE_INDENT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FirstLineIndent")));
    add(PROP_NUMBERING_STYLE_NAME, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NumberingStyleName")));
    add(PROP_OUTLINE_LEVEL, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OutlineLevel")));
    add(PROP_LISTTAB_STOP_POSITION, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ListtabStopPosition")));
    add(PROP_POSITION_AND_SPACE_MODE, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PositionAndSpaceMode")));
    add(PROP_PARA_SPLIT, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaSplit")));
    add(PROP_HELP, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Help")));
    add(PROP_HEADING_STYLE_NAME, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HeadingStyleName")));
}

void PropertyNameSupplier_Impl::add(PropertyIds aId, const ::rtl::OUString & aStr)
{
    aNameMap[aId] = aStr;
    aNameBackwardMap[aStr] = aId;
}

/*-- 14.06.2006 11:01:31---------------------------------------------------

  -----------------------------------------------------------------------*/
PropertyNameSupplier::PropertyNameSupplier() :
    m_pImpl(new PropertyNameSupplier_Impl)
{
}
/*-- 14.06.2006 11:01:32---------------------------------------------------

  -----------------------------------------------------------------------*/
PropertyNameSupplier::~PropertyNameSupplier()
{
    delete m_pImpl;
}
/*-- 14.06.2006 11:01:32---------------------------------------------------

  -----------------------------------------------------------------------*/
const rtl::OUString& PropertyNameSupplier::GetName( PropertyIds eId ) const
{
    PropertyNameMap_t::const_iterator aIt = m_pImpl->aNameMap.find(eId);

    return aIt->second;
}

PropertyIds PropertyNameSupplier::GetId(const ::rtl::OUString & rStr) const
{
    PropertyBackwardNameMap_t::const_iterator aIt= m_pImpl->aNameBackwardMap.find(rStr);

    return aIt->second;
}

PropertyNameSupplier& PropertyNameSupplier::GetPropertyNameSupplier()
{
    static PropertyNameSupplier aNameSupplier;
    return aNameSupplier;
}

PropertySequence::PropertySequence()
    : m_rPropNameSupplier(PropertyNameSupplier::GetPropertyNameSupplier())
{
}

PropertySequence::PropertySequence(const uno::Sequence<beans::PropertyValue> & rSeq)
    : m_sequence(rSeq), m_rPropNameSupplier(PropertyNameSupplier::GetPropertyNameSupplier())
{
    sal_Int32 nLength = rSeq.getLength();

    for (sal_Int32 n = 0; n < nLength; n++)
    {
        PropertyIds aId = m_rPropNameSupplier.GetId(m_sequence[n].Name);
        m_indexMap[aId] = n;
    }
}

PropertySequence::~PropertySequence()
{
}

uno::Any PropertySequence::get(PropertyIds aPropId)
{
    Map_t::const_iterator aIt = m_indexMap.find(aPropId);

    if (aIt != m_indexMap.end())
    {
        return m_sequence[aIt->second].Value;
    }

    return uno::Any();
}

int PropertySequence::getOrCreateIndex(PropertyIds aPropId)
{
    Map_t::const_iterator aIt = m_indexMap.find(aPropId);

    sal_Int32 nIndex = 0;
    if (aIt == m_indexMap.end())
    {
        sal_uInt32 nCount = m_sequence.getLength() + 1;
        m_sequence.realloc(nCount);
        nIndex = nCount - 1;
        m_indexMap[aPropId] = nIndex;
    }
    else
    {
        nIndex = aIt->second;
    }

    return nIndex;
}

void PropertySequence::set(PropertyIds aPropId, const uno::Any & rValue)
{
    sal_Int32 nIndex = getOrCreateIndex(aPropId);

    m_sequence[nIndex].Name = m_rPropNameSupplier.GetName(aPropId);
    m_sequence[nIndex].Value = rValue;
}

void PropertySequence::set(PropertyIds aPropId, sal_uInt32 nValue)
{
    uno::Any aAny;

    aAny <<= nValue;
    set(aPropId, aAny);
}

void PropertySequence::set(PropertyIds aPropId, sal_Int32 nValue)
{
    uno::Any aAny;

    aAny <<= nValue;
    set(aPropId, aAny);
}

void PropertySequence::set(PropertyIds aPropId, sal_uInt16 nValue)
{
    uno::Any aAny;

    aAny <<= nValue;
    set(aPropId, aAny);
}

void PropertySequence::set(PropertyIds aPropId, sal_Int16 nValue)
{
    uno::Any aAny;

    aAny <<= nValue;
    set(aPropId, aAny);
}

uno::Sequence<beans::PropertyValue> & PropertySequence::getSequence()
{
    return m_sequence;
}

::std::string PropertySequence::toString() const
{
    ::std::string sResult = "";

    sal_Int32 nCount = m_sequence.getLength();

    for (sal_Int32 n = 0; n < nCount; n++)
    {
        if (n > 0)
            sResult +=", ";

        ::std::string sTmp = ::rtl::OUStringToOString(m_sequence[n].Name, RTL_TEXTENCODING_ASCII_US).getStr();

        sResult += sTmp;

        if (m_sequence[n].Value.hasValue())
        {
            sal_Int32 nValue = 0;
            m_sequence[n].Value >>= nValue;

            static char buffer[256];
            snprintf(buffer, sizeof(buffer), " = %" SAL_PRIdINT32, nValue);
            sResult += buffer;
        }
    }

    return sResult;
}

} //namespace dmapper
} //namespace writerfilter

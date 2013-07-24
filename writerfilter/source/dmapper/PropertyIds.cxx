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
#include <rtl/instance.hxx>
#include <rtl/ustring.hxx>
#include "PropertyIds.hxx"
#include <map>

namespace writerfilter {
namespace dmapper{

typedef ::std::map< PropertyIds, OUString> PropertyNameMap_t;

struct PropertyNameSupplier_Impl
{
    PropertyNameMap_t aNameMap;
};



PropertyNameSupplier::PropertyNameSupplier() :
    m_pImpl(new PropertyNameSupplier_Impl)
{
}


PropertyNameSupplier::~PropertyNameSupplier()
{
    delete m_pImpl;
}


const OUString& PropertyNameSupplier::GetName( PropertyIds eId ) const
{
    PropertyNameMap_t::iterator aIt = m_pImpl->aNameMap.find(eId);
    if(aIt == m_pImpl->aNameMap.end())
    {
        OUString sName;
        switch(eId)
        {
            case PROP_CHAR_WEIGHT:     sName = "CharWeight"; break;
            case PROP_CHAR_POSTURE:    sName = "CharPosture"; break;
            case PROP_CHAR_STRIKEOUT:  sName = "CharStrikeout"; break;
            case PROP_CHAR_CONTOURED:  sName = "CharContoured"; break;
            case PROP_CHAR_SHADOWED:   sName = "CharShadowed"; break;
            case PROP_CHAR_CASE_MAP:   sName = "CharCaseMap"; break;
            case PROP_CHAR_COLOR:      sName = "CharColor"; break;
            case PROP_CHAR_RELIEF:     sName = "CharRelief"; break;
            case PROP_CHAR_UNDERLINE:  sName = "CharUnderline"; break;
            case PROP_CHAR_UNDERLINE_COLOR:  sName = "CharUnderlineColor"; break;
            case PROP_CHAR_UNDERLINE_HAS_COLOR:  sName = "CharUnderlineHasColor"; break;
            case PROP_CHAR_WORD_MODE:  sName = "CharWordMode"; break;
            case PROP_CHAR_ESCAPEMENT       :     sName = "CharEscapement"; break;
            case PROP_CHAR_ESCAPEMENT_HEIGHT:     sName = "CharEscapementHeight"; break;
            case PROP_CHAR_HEIGHT:     sName = "CharHeight"; break;
            case PROP_CHAR_HEIGHT_COMPLEX:     sName = "CharHeightComplex"; break;
            case PROP_CHAR_LOCALE:     sName = "CharLocale"; break;
            case PROP_CHAR_LOCALE_ASIAN:     sName = "CharLocaleAsian"; break;
            case PROP_CHAR_LOCALE_COMPLEX:     sName = "CharLocaleComplex"; break;
            case PROP_CHAR_WEIGHT_COMPLEX :  sName = "CharWeightComplex"; break;
            case PROP_CHAR_POSTURE_COMPLEX:  sName = "CharPostureComplex"; break;
            case PROP_CHAR_CHAR_KERNING:     sName = "CharKerning"; break;
            case PROP_CHAR_AUTO_KERNING:     sName = "CharAutoKerning"; break;
            case PROP_CHAR_SCALE_WIDTH:      sName = "CharScaleWidth"; break;
            case PROP_CHAR_STYLE_NAME:      sName = "CharStyleName"; break;
            case PROP_CHAR_FONT_NAME:                sName = "CharFontName"; break;
            case PROP_CHAR_FONT_STYLE:               sName = "CharFontStyle"; break;
            case PROP_CHAR_FONT_FAMILY:              sName = "CharFontFamily"; break;
            case PROP_CHAR_FONT_CHAR_SET:            sName = "CharFontCharSet"; break;
            case PROP_CHAR_FONT_PITCH:               sName = "CharFontPitch"; break;
            case PROP_CHAR_FONT_NAME_ASIAN     :     sName = "CharFontNameAsian"; break;
            case PROP_CHAR_HEIGHT_ASIAN        :     sName = "CharHeightAsian"; break;
            case PROP_CHAR_FONT_STYLE_ASIAN    :     sName = "CharFontStyleAsian"; break;
            case PROP_CHAR_FONT_FAMILY_ASIAN   :     sName = "CharFontFamilyAsian"; break;
            case PROP_CHAR_FONT_CHAR_SET_ASIAN :     sName = "CharFontCharSetAsian"; break;
            case PROP_CHAR_FONT_PITCH_ASIAN    :     sName = "CharFontPitchAsian"; break;
            case PROP_CHAR_FONT_NAME_COMPLEX   :     sName = "CharFontNameComplex"; break;
            case PROP_CHAR_FONT_STYLE_COMPLEX  :     sName = "CharFontStyleComplex"; break;
            case PROP_CHAR_FONT_FAMILY_COMPLEX:      sName = "CharFontFamilyComplex"; break;
            case PROP_CHAR_FONT_CHAR_SET_COMPLEX:    sName = "CharFontCharSetComplex"; break;
            case PROP_CHAR_FONT_PITCH_COMPLEX:       sName = "CharFontPitchComplex"; break;
            case PROP_CHAR_HIDDEN           :     sName = "CharHidden"; break;
            case PROP_CHAR_WEIGHT_ASIAN     :     sName = "CharWeightAsian"; break;
            case PROP_CHAR_POSTURE_ASIAN    :     sName = "CharPostureAsian"; break;
            case PROP_CHAR_BACK_COLOR:     sName = "CharBackColor"; break;
            case PROP_CHAR_EMPHASIS:     sName = "CharEmphasis"; break;
            case PROP_CHAR_COMBINE_IS_ON:     sName = "CharCombineIsOn"; break;
            case PROP_CHAR_COMBINE_PREFIX:     sName = "CharCombinePrefix"; break;
            case PROP_CHAR_COMBINE_SUFFIX:     sName = "CharCombineSuffix"; break;
            case PROP_CHAR_ROTATION:     sName = "CharRotation"; break;
            case PROP_CHAR_ROTATION_IS_FIT_TO_LINE:     sName = "CharRotationIsFitToLine"; break;
            case PROP_CHAR_FLASH:     sName = "CharFlash"; break;

            case PROP_PARA_STYLE_NAME:      sName = "ParaStyleName"; break;
            case PROP_PARA_ADJUST:     sName = "ParaAdjust"; break;
            case PROP_PARA_VERT_ALIGNMENT: sName = "ParaVertAlignment"; break;
            case PROP_PARA_LAST_LINE_ADJUST:     sName = "ParaLastLineAdjust"; break;
            case PROP_PARA_RIGHT_MARGIN     :    sName = "ParaRightMargin"; break;
            case PROP_PARA_LEFT_MARGIN      :    sName = "ParaLeftMargin"; break;
            case PROP_PARA_FIRST_LINE_INDENT:    sName = "ParaFirstLineIndent"; break;
            case PROP_PARA_KEEP_TOGETHER:       sName = "ParaKeepTogether"; break;
            case PROP_PARA_TOP_MARGIN:       sName = "ParaTopMargin"; break;
            case PROP_PARA_CONTEXT_MARGIN:       sName = "ParaContextMargin"; break;
            case PROP_PARA_BOTTOM_MARGIN:       sName = "ParaBottomMargin"; break;
            case PROP_PARA_IS_HYPHENATION:     sName = "ParaIsHyphenation"; break;
            case PROP_PARA_LINE_NUMBER_COUNT:     sName = "ParaLineNumberCount"; break;
            case PROP_PARA_IS_HANGING_PUNCTUATION: sName = "ParaIsHangingPunctuation"; break;
            case PROP_PARA_LINE_SPACING:     sName = "ParaLineSpacing"; break;
            case PROP_PARA_TAB_STOPS:       sName = "ParaTabStops"; break;
            case PROP_PARA_WIDOWS:          sName = "ParaWidows"; break;
            case PROP_PARA_ORPHANS:         sName = "ParaOrphans"; break;
            case PROP_PARA_LINE_NUMBER_START_VALUE:    sName = "ParaLineNumberStartValue"; break;
            case PROP_NUMBERING_LEVEL:  sName = "NumberingLevel"; break;
            case PROP_NUMBERING_RULES:  sName = "NumberingRules"; break;
            case PROP_NUMBERING_TYPE:   sName = "NumberingType"; break;
            case PROP_START_WITH:       sName = "StartWith"; break;
            case PROP_ADJUST:           sName = "Adjust"; break;
            case PROP_PARENT_NUMBERING: sName = "ParentNumbering"; break;
            case PROP_RIGHT_MARGIN     :    sName = "RightMargin"; break;
            case PROP_LEFT_MARGIN      :    sName = "LeftMargin"; break;
            case PROP_TOP_MARGIN          :    sName = "TopMargin"; break;
            case PROP_BOTTOM_MARGIN       :    sName = "BottomMargin"; break;
            case PROP_FIRST_LINE_OFFSET:    sName = "FirstLineOffset"; break;
            case PROP_LEFT_BORDER           :    sName = "LeftBorder";break;
            case PROP_RIGHT_BORDER          :    sName = "RightBorder";break;
            case PROP_TOP_BORDER            :    sName = "TopBorder";break;
            case PROP_BOTTOM_BORDER         :    sName = "BottomBorder";break;
            case PROP_TABLE_BORDER         :    sName = "TableBorder";break;
            case PROP_LEFT_BORDER_DISTANCE  :    sName = "LeftBorderDistance"; break;
            case PROP_RIGHT_BORDER_DISTANCE :    sName = "RightBorderDistance"; break;
            case PROP_TOP_BORDER_DISTANCE   :    sName = "TopBorderDistance";break;
            case PROP_BOTTOM_BORDER_DISTANCE:    sName = "BottomBorderDistance"; break;
            case PROP_CURRENT_PRESENTATION  :    sName = "CurrentPresentation"; break;
            case PROP_IS_FIXED              :    sName = "IsFixed"; break;
            case PROP_SUB_TYPE              :    sName = "SubType"; break;
            case PROP_FILE_FORMAT           :    sName = "FileFormat"; break;
            case PROP_HYPER_LINK_U_R_L      :    sName = "HyperLinkURL"; break;
            case PROP_NUMBER_FORMAT         :    sName = "NumberFormat"; break;
            case PROP_NAME                  :    sName = "Name"; break;
            case PROP_IS_INPUT              :    sName = "IsInput"; break;
            case PROP_HINT                  :    sName = "Hint"; break;
            case PROP_FULL_NAME             :    sName = "FullName"; break;
            case PROP_KEYWORDS              :    sName = "Keywords"; break;
            case PROP_DESCRIPTION           :    sName = "Description"; break;
            case PROP_MACRO_NAME            :    sName = "MacroName"; break;
            case PROP_SUBJECT               :    sName = "Subject"; break;
            case PROP_USER_DATA_TYPE        :    sName = "UserDataType"; break;
            case PROP_TITLE                 :    sName = "Title"; break;
            case PROP_CONTENT               :    sName = "Content"; break;
            case PROP_DATA_COLUMN_NAME      :    sName = "DataColumnName"; break;
            case PROP_INPUT_STREAM          :    sName = "InputStream"; break;
            case PROP_GRAPHIC               :    sName = "Graphic"; break;
            case PROP_ANCHOR_TYPE           :    sName = "AnchorType"; break;
            case PROP_SIZE                  :    sName = "Size"; break;
            case PROP_HORI_ORIENT           :    sName = "HoriOrient"; break;
            case PROP_HORI_ORIENT_POSITION  :    sName = "HoriOrientPosition"; break;
            case PROP_HORI_ORIENT_RELATION  :    sName = "HoriOrientRelation"; break;
            case PROP_VERT_ORIENT           :    sName = "VertOrient"; break;
            case PROP_VERT_ORIENT_POSITION  :    sName = "VertOrientPosition"; break;
            case PROP_VERT_ORIENT_RELATION  :    sName = "VertOrientRelation"; break;
            case PROP_GRAPHIC_CROP          :    sName = "GraphicCrop"; break;
            case PROP_SIZE100th_M_M         :    sName = "Size100thMM"; break;
            case PROP_SIZE_PIXEL            :    sName = "SizePixel"; break;
            case PROP_SURROUND              :    sName = "Surround"; break;
            case PROP_SURROUND_CONTOUR      :    sName = "SurroundContour"; break;
            case PROP_ADJUST_CONTRAST       :    sName = "AdjustContrast"; break;
            case PROP_ADJUST_LUMINANCE      :    sName = "AdjustLuminance"; break;
            case PROP_GRAPHIC_COLOR_MODE    :    sName = "GraphicColorMode"; break;
            case PROP_GAMMA                 :    sName = "Gamma"; break;
            case PROP_HORI_MIRRORED_ON_EVEN_PAGES:    sName = "HoriMirroredOnEvenPages"; break;
            case PROP_HORI_MIRRORED_ON_ODD_PAGES :    sName = "HoriMirroredOnOddPages"; break;
            case PROP_VERT_MIRRORED        :    sName = "VertMirrored"; break;
            case PROP_CONTOUR_OUTSIDE      :    sName = "ContourOutside"; break;
            case PROP_CONTOUR_POLY_POLYGON :    sName = "ContourPolyPolygon"; break;
            case PROP_PAGE_TOGGLE          :    sName = "PageToggle"; break;
            case PROP_BACK_COLOR           :    sName = "BackColor"; break;
            case PROP_BACK_COLOR_TRANSPARENCY:  sName = "BackColorTransparency"; break;
            case PROP_ALTERNATIVE_TEXT     :    sName = "AlternativeText"; break;
            case PROP_HEADER_TEXT_LEFT     :    sName = "HeaderTextLeft"; break;
            case PROP_HEADER_TEXT          :    sName = "HeaderText"; break;
            case PROP_HEADER_IS_SHARED     :    sName = "HeaderIsShared"; break;
            case PROP_HEADER_IS_ON         :    sName = "HeaderIsOn"; break;
            case PROP_FOOTER_TEXT_LEFT     :    sName = "FooterTextLeft"; break;
            case PROP_FOOTER_TEXT          :    sName = "FooterText"; break;
            case PROP_FOOTER_IS_SHARED     :    sName = "FooterIsShared"; break;
            case PROP_FOOTER_IS_ON         :    sName = "FooterIsOn"; break;
            case PROP_FOOTNOTE_COUNTING    :    sName = "FootnoteCounting"; break;
            case PROP_WIDTH                :    sName = "Width"; break;
            case PROP_HEIGHT               :    sName = "Height"; break;
            case PROP_SEPARATOR_LINE_IS_ON :    sName = "SeparatorLineIsOn"; break;
            case PROP_TEXT_COLUMNS         :    sName = "TextColumns"; break;
            case PROP_AUTOMATIC_DISTANCE   :    sName = "AutomaticDistance"; break;
            case PROP_IS_LANDSCAPE         :    sName = "IsLandscape"; break;
            case PROP_PRINTER_PAPER_TRAY_INDEX:    sName = "PrinterPaperTrayIndex"; break;
            case PROP_FIRST_PAGE       :    sName = "First Page"; break;
            case PROP_DEFAULT          :    sName = "Default"; break;
            case PROP_PAGE_DESC_NAME   :    sName = "PageDescName"; break;
            case PROP_PAGE_NUMBER_OFFSET:    sName = "PageNumberOffset"; break;
            case PROP_BREAK_TYPE       :    sName = "BreakType"; break;
            case PROP_FOOTER_IS_DYNAMIC_HEIGHT:  sName = "FooterIsDynamicHeight"; break;
            case PROP_FOOTER_DYNAMIC_SPACING:    sName = "FooterDynamicSpacing"; break;
            case PROP_FOOTER_HEIGHT         :    sName = "FooterHeight"; break;
            case PROP_FOOTER_BODY_DISTANCE  :    sName = "FooterBodyDistance"; break;
            case PROP_HEADER_IS_DYNAMIC_HEIGHT:  sName = "HeaderIsDynamicHeight"; break;
            case PROP_HEADER_DYNAMIC_SPACING:    sName = "HeaderDynamicSpacing"; break;
            case PROP_HEADER_HEIGHT         :    sName = "HeaderHeight"; break;
            case PROP_HEADER_BODY_DISTANCE  :    sName = "HeaderBodyDistance"; break;
            case PROP_WRITING_MODE          :    sName = "WritingMode"; break;
            case PROP_GRID_MODE             :    sName = "GridMode"; break;
            case PROP_GRID_DISPLAY          :    sName = "GridDisplay"; break;
            case PROP_GRID_PRINT            :    sName = "GridPrint"; break;
            case PROP_ADD_EXTERNAL_LEADING  :    sName = "AddExternalLeading"; break;
            case PROP_GRID_LINES            :    sName = "GridLines"; break;
            case PROP_GRID_BASE_HEIGHT      :    sName = "GridBaseHeight"; break;
            case PROP_GRID_RUBY_HEIGHT      :    sName = "GridRubyHeight"; break;
            case PROP_GRID_STANDARD_MODE     :    sName = "StandardPageMode"; break;
            case PROP_IS_ON                  :    sName = "IsOn"; break;
            case PROP_RESTART_AT_EACH_PAGE   :    sName = "RestartAtEachPage"; break;
            case PROP_COUNT_EMPTY_LINES      :    sName = "CountEmptyLines"; break;
            case PROP_COUNT_LINES_IN_FRAMES  :    sName = "CountLinesInFrames"; break;
            case PROP_INTERVAL               :    sName = "Interval"; break;
            case PROP_DISTANCE               :    sName = "Distance"; break;
            case PROP_NUMBER_POSITION        :    sName = "NumberPosition"; break;
            case PROP_LEVEL                  :    sName = "Level"; break;
            case PROP_LEVEL_FOLLOW           :    sName = "LabelFollowedBy"; break;
            case PROP_LEVEL_PARAGRAPH_STYLES :    sName = "LevelParagraphStyles"; break;
            case PROP_LEVEL_FORMAT           :    sName = "LevelFormat"; break;
            case PROP_TOKEN_TYPE             :    sName = "TokenType"; break;
            case PROP_TOKEN_HYPERLINK_START  :    sName = "TokenHyperlinkStart"; break;
            case PROP_TOKEN_HYPERLINK_END    :    sName = "TokenHyperlinkEnd"; break;
            case PROP_TOKEN_CHAPTER_INFO     :    sName = "TokenChapterInfo"; break;
            case PROP_CHAPTER_FORMAT         :    sName = "ChapterFormat"; break;
            case PROP_TOKEN_TEXT             :    sName = "TokenText"; break;
            case PROP_TEXT                   :    sName = "Text"; break;
            case PROP_CREATE_FROM_OUTLINE    :    sName = "CreateFromOutline"; break;
            case PROP_CREATE_FROM_MARKS      :    sName = "CreateFromMarks"; break;
            case PROP_STANDARD               :    sName = "Standard"; break;
            case PROP_IS_SPLIT_ALLOWED       :    sName = "IsSplitAllowed"; break;
            case META_PROP_VERTICAL_BORDER   :    sName = "VerticalBorder"; break;
            case META_PROP_HORIZONTAL_BORDER :    sName = "HorizontalBorder"; break;
            case PROP_HEADER_ROW_COUNT      :    sName = "HeaderRowCount"; break;
            case PROP_IS_AUTO_HEIGHT        :    sName = "IsAutoHeight"; break;
            case PROP_SIZE_TYPE             :    sName = "SizeType"; break;
            case PROP_TABLE_COLUMN_SEPARATORS:   sName = "TableColumnSeparators"; break;
            case META_PROP_TABLE_STYLE_NAME  :    sName = "TableStyleName"; break;
            case PROP_REDLINE_AUTHOR        :    sName = "RedlineAuthor"; break;
            case PROP_REDLINE_DATE_TIME     :    sName = "RedlineDateTime"; break;
            case PROP_REDLINE_COMMENT       :    sName = "RedlineComment"; break;
            case PROP_REDLINE_TYPE          :    sName = "RedlineType"; break;
            case PROP_REDLINE_SUCCESSOR_DATA:    sName = "RedlineSuccessorData"; break;
            case PROP_REDLINE_IDENTIFIER    :    sName = "RedlineIdentifier"; break;
            case PROP_SIZE_PROTECTED        :    sName = "SizeProtected"; break;
            case PROP_POSITION_PROTECTED    :    sName = "PositionProtected"; break;
            case PROP_OPAQUE                :    sName = "Opaque"; break;
            case PROP_VERTICAL_MERGE   :    sName = "VerticalMerge"; break;
            case PROP_BULLET_CHAR      :    sName = "BulletChar"; break;
            case PROP_BULLET_FONT_NAME :    sName = "BulletFontName"; break;
            case PROP_PARA_BACK_COLOR  :    sName = "ParaBackColor"; break;
            case PROP_TABS_RELATIVE_TO_INDENT:    sName = "TabsRelativeToIndent"; break;
            case PROP_PREFIX           :    sName = "Prefix"; break;
            case PROP_SUFFIX           :    sName = "Suffix"; break;
            case PROP_CREATE_FROM_LEVEL_PARAGRAPH_STYLES:    sName = "CreateFromLevelParagraphStyles"; break;
            case PROP_DROP_CAP_FORMAT  :    sName = "DropCapFormat"; break;
            case PROP_REFERENCE_FIELD_PART :    sName = "ReferenceFieldPart"; break;
            case PROP_SOURCE_NAME:    sName = "SourceName"; break;
            case PROP_REFERENCE_FIELD_SOURCE :    sName = "ReferenceFieldSource"; break;
            case PROP_WIDTH_TYPE :    sName = "WidthType"; break;
            case PROP_TBL_LOOK : sName = "TblLook"; break;
            case PROP_TEXT_RANGE:    sName = "TextRange"; break;
            case PROP_SERVICE_CHAR_STYLE       :    sName = "com.sun.star.style.CharacterStyle"; break;
            case PROP_SERVICE_PARA_STYLE       :    sName = "com.sun.star.style.ParagraphStyle"; break;
            case PROP_CHARACTER_STYLES :    sName = "CharacterStyles"; break;
            case PROP_PARAGRAPH_STYLES :    sName = "ParagraphStyles"; break;
            case PROP_PARAGRAPH_STYLE_NAME : sName = "ParagraphStyleName"; break;
            case PROP_TABLE_BORDER_DISTANCES:    sName = "TableBorderDistances"; break;
            case META_PROP_CELL_MAR_TOP    :    sName = "MetaPropCellMarTop"; break;
            case META_PROP_CELL_MAR_BOTTOM :    sName = "MetaPropCellMarBottom"; break;
            case META_PROP_CELL_MAR_LEFT   :    sName = "MetaPropCellMarLeft"; break;
            case META_PROP_CELL_MAR_RIGHT  :    sName = "MetaPropCellMarRight"; break;
            case PROP_START_AT :    sName = "StartAt"; break;
            case PROP_ADD_PARA_TABLE_SPACING :    sName = "AddParaTableSpacing"; break;
            case PROP_CHAR_PROP_HEIGHT        :    sName = "CharPropHeight"; break;
            case PROP_CHAR_PROP_HEIGHT_ASIAN  :    sName = "CharPropHeightAsian"; break;
            case PROP_CHAR_PROP_HEIGHT_COMPLEX:    sName = "CharPropHeightComplex"; break;
            case PROP_FORMAT   :    sName = "Format"; break;
            case PROP_INSERT   :    sName = "Insert"; break;
            case PROP_DELETE   :    sName = "Delete"; break;
            case PROP_STREAM_NAME:    sName = "StreamName"; break;
            case PROP_BITMAP :    sName = "Bitmap"; break;
            case PROP_IS_DATE :   sName = "IsDate"; break;
            case PROP_TAB_STOP_DISTANCE :    sName = "TabStopDistance"; break;
            case PROP_INDENT_AT :    sName = "IndentAt"; break;
            case PROP_FIRST_LINE_INDENT :    sName = "FirstLineIndent"; break;
            case PROP_NUMBERING_STYLE_NAME  :    sName = "NumberingStyleName"; break;
            case PROP_OUTLINE_LEVEL  :    sName = "OutlineLevel"; break;
            case PROP_LISTTAB_STOP_POSITION :    sName = "ListtabStopPosition"; break;
            case PROP_POSITION_AND_SPACE_MODE :    sName = "PositionAndSpaceMode"; break;
            case PROP_PARA_SPLIT:    sName = "ParaSplit"; break;
            case PROP_HELP:    sName = "Help"; break;
            case PROP_HEADING_STYLE_NAME:    sName = "HeadingStyleName"; break;
            case PROP_FRM_DIRECTION:        sName = "FRMDirection"; break;
            case PROP_EMBEDDED_OBJECT           :    sName = "EmbeddedObject"; break;
            case PROP_IS_VISIBLE: sName = "IsVisible"; break;
            case PROP_PAGE_STYLE_LAYOUT: sName = "PageStyleLayout"; break;
            case PROP_Z_ORDER: sName = "ZOrder"; break;
            case PROP_EMBED_FONTS: sName = "EmbedFonts"; break;
            case PROP_EMBED_SYSTEM_FONTS: sName = "EmbedSystemFonts"; break;
            case PROP_SHADOW_FORMAT: sName = "ShadowFormat"; break;
            case PROP_RELATIVE_WIDTH: sName = "RelativeWidth"; break;
            case PROP_IS_WIDTH_RELATIVE: sName = "IsWidthRelative"; break;
            case PROP_GRAPHIC_URL: sName = "GraphicURL"; break;
            case PROP_GRAPHIC_BITMAP: sName = "GraphicBitmap"; break;
            case PROP_CHAR_SHADING_VALUE: sName = "CharShadingValue"; break;
            case PROP_LABEL_CATEGORY: sName = "LabelCategory"; break;
            case PROP_FIRST_IS_SHARED : sName = "FirstIsShared"; break;
            case PROP_MIRROR_INDENTS : sName = "MirrorIndents"; break;
        }
        ::std::pair<PropertyNameMap_t::iterator,bool> aInsertIt =
                m_pImpl->aNameMap.insert( PropertyNameMap_t::value_type( eId, sName ));
        if(aInsertIt.second)
            aIt = aInsertIt.first;
    }
    return aIt->second;
}

namespace
{
    class thePropertyNameSupplier : public rtl::Static<PropertyNameSupplier, PropertyNameSupplier> {};
}

PropertyNameSupplier& PropertyNameSupplier::GetPropertyNameSupplier()
{
    return thePropertyNameSupplier::get();
}

} //namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

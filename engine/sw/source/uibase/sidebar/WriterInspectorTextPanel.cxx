/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include "WriterInspectorTextPanel.hxx"

#include <doc.hxx>
#include <ndtxt.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <unoprnms.hxx>
#include <unocoll.hxx>
#include <unosection.hxx>
#include <editeng/unoprnms.hxx>
#include <com/sun/star/text/XBookmarksSupplier.hpp>
#include <com/sun/star/text/XTextSectionsSupplier.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/rdf/XMetadatable.hpp>
#include <com/sun/star/rdf/XDocumentMetadataAccess.hpp>
#include <com/sun/star/container/XChild.hpp>

#include <unotextrange.hxx>
#include <comphelper/string.hxx>
#include <comphelper/processfactory.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/settings.hxx>
#include <inspectorproperties.hrc>
#include <strings.hrc>
#include <rdfhelper.hxx>
#include <unotxdoc.hxx>
#include <unobookmark.hxx>
#include <unostyle.hxx>
#include <unoxstyle.hxx>

using namespace css;

namespace sw::sidebar
{
static void UpdateTree(SwDocShell& rDocSh, const SwEditShell& rEditSh,
                       std::vector<svx::sidebar::TreeNode>& aStore, sal_Int32& rParIdx);

std::unique_ptr<PanelLayout> WriterInspectorTextPanel::Create(weld::Widget* pParent,
                                                              SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException(
            u"no parent Window given to WriterInspectorTextPanel::Create"_ustr, nullptr, 0);
    return std::make_unique<WriterInspectorTextPanel>(pParent, pBindings);
}

namespace
{
SwWrtShell* GetWrtShell()
{
    SwDocShell* pDocSh = dynamic_cast<SwDocShell*>(SfxObjectShell::Current());
    return pDocSh ? pDocSh->GetWrtShell() : nullptr;
}
}
WriterInspectorTextPanel::WriterInspectorTextPanel(weld::Widget* pParent, SfxBindings* pBindings)
    : InspectorTextPanel(pParent, pBindings)
    , m_pShell(GetWrtShell())
    , m_nParIdx(0)
{
    if (m_pShell)
    {
        m_oldLink = m_pShell->GetChgLnk();
        m_pShell->SetChgLnk(LINK(this, WriterInspectorTextPanel, AttrChangedNotify));
    }

    // Update panel on start
    std::vector<svx::sidebar::TreeNode> aStore;
    SwDocShell* pDocSh = dynamic_cast<SwDocShell*>(SfxObjectShell::Current());
    SwEditShell* pEditSh = pDocSh ? pDocSh->GetDoc()->GetEditShell() : nullptr;
    if (pEditSh && pEditSh->GetCursor()->GetPointNode().GetTextNode())
        UpdateTree(*pDocSh, *pEditSh, aStore, m_nParIdx);
    updateEntries(aStore, m_nParIdx);
}

WriterInspectorTextPanel::~WriterInspectorTextPanel()
{
    if (m_pShell)
    {
        m_pShell->SetChgLnk(m_oldLink);
    }
}

static OUString PropertyNametoRID(const OUString& rName)
{
    static const std::map<OUString, TranslateId> aNameToRID = {
        { u"BorderDistance"_ustr, RID_BORDER_DISTANCE },
        { u"BottomBorder"_ustr, RID_BOTTOM_BORDER },
        { u"BottomBorderDistance"_ustr, RID_BOTTOM_BORDER_DISTANCE },
        { u"BreakType"_ustr, RID_BREAK_TYPE },
        { u"Category"_ustr, RID_CATEGORY },
        { u"Cell"_ustr, RID_CELL },
        { u"CharAutoEscapement"_ustr, RID_CHAR_AUTO_ESCAPEMENT },
        { u"CharAutoKerning"_ustr, RID_CHAR_AUTO_KERNING },
        { u"CharAutoStyleName"_ustr, RID_CHAR_AUTO_STYLE_NAME },
        { u"CharBackColor"_ustr, RID_CHAR_BACK_COLOR },
        { u"CharBackTransparent"_ustr, RID_CHAR_BACK_TRANSPARENT },
        { u"CharBorderDistance"_ustr, RID_CHAR_BORDER_DISTANCE },
        { u"CharBottomBorder"_ustr, RID_CHAR_BOTTOM_BORDER },
        { u"CharBottomBorderDistance"_ustr, RID_CHAR_BOTTOM_BORDER_DISTANCE },
        { u"CharCaseMap"_ustr, RID_CHAR_CASE_MAP },
        { u"CharColor"_ustr, RID_CHAR_COLOR },
        { u"CharCombineIsOn"_ustr, RID_CHAR_COMBINE_IS_ON },
        { u"CharCombinePrefix"_ustr, RID_CHAR_COMBINE_PREFIX },
        { u"CharCombineSuffix"_ustr, RID_CHAR_COMBINE_SUFFIX },
        { u"CharContoured"_ustr, RID_CHAR_CONTOURED },
        { u"CharCrossedOut"_ustr, RID_CHAR_CROSSED_OUT },
        { u"CharDiffHeight"_ustr, RID_CHAR_DIFF_HEIGHT },
        { u"CharDiffHeightAsian"_ustr, RID_CHAR_DIFF_HEIGHT_ASIAN },
        { u"CharDiffHeightComplex"_ustr, RID_CHAR_DIFF_HEIGHT_COMPLEX },
        { u"CharEmphasis"_ustr, RID_CHAR_EMPHASIS },
        { u"CharEscapement"_ustr, RID_CHAR_ESCAPEMENT },
        { u"CharEscapementHeight"_ustr, RID_CHAR_ESCAPEMENT_HEIGHT },
        { u"CharFlash"_ustr, RID_CHAR_FLASH },
        { u"CharFontCharSet"_ustr, RID_CHAR_FONT_CHAR_SET },
        { u"CharFontCharSetAsian"_ustr, RID_CHAR_FONT_CHAR_SET_ASIAN },
        { u"CharFontCharSetComplex"_ustr, RID_CHAR_FONT_CHAR_SET_COMPLEX },
        { u"CharFontFamily"_ustr, RID_CHAR_FONT_FAMILY },
        { u"CharFontFamilyAsian"_ustr, RID_CHAR_FONT_FAMILY_ASIAN },
        { u"CharFontFamilyComplex"_ustr, RID_CHAR_FONT_FAMILY_COMPLEX },
        { u"CharFontName"_ustr, RID_CHAR_FONT_NAME },
        { u"CharFontNameAsian"_ustr, RID_CHAR_FONT_NAME_ASIAN },
        { u"CharFontNameComplex"_ustr, RID_CHAR_FONT_NAME_COMPLEX },
        { u"CharFontPitch"_ustr, RID_CHAR_FONT_PITCH },
        { u"CharFontPitchAsian"_ustr, RID_CHAR_FONT_PITCH_ASIAN },
        { u"CharFontPitchComplex"_ustr, RID_CHAR_FONT_PITCH_COMPLEX },
        { u"CharFontStyleName"_ustr, RID_CHAR_FONT_STYLE_NAME },
        { u"CharFontStyleNameAsian"_ustr, RID_CHAR_FONT_STYLE_NAME_ASIAN },
        { u"CharFontStyleNameComplex"_ustr, RID_CHAR_FONT_STYLE_NAME_COMPLEX },
        { u"CharHeight"_ustr, RID_CHAR_HEIGHT },
        { u"CharHeightAsian"_ustr, RID_CHAR_HEIGHT_ASIAN },
        { u"CharHeightComplex"_ustr, RID_CHAR_HEIGHT_COMPLEX },
        { u"CharHidden"_ustr, RID_CHAR_HIDDEN },
        { u"CharHighlight"_ustr, RID_CHAR_HIGHLIGHT },
        { u"CharInteropGrabBag"_ustr, RID_CHAR_INTEROP_GRAB_BAG },
        { u"CharKerning"_ustr, RID_CHAR_KERNING },
        { u"CharLeftBorder"_ustr, RID_CHAR_LEFT_BORDER },
        { u"CharLeftBorderDistance"_ustr, RID_CHAR_LEFT_BORDER_DISTANCE },
        { u"CharLocale"_ustr, RID_CHAR_LOCALE },
        { u"CharLocaleAsian"_ustr, RID_CHAR_LOCALE_ASIAN },
        { u"CharLocaleComplex"_ustr, RID_CHAR_LOCALE_COMPLEX },
        { u"CharNoHyphenation"_ustr, RID_CHAR_NO_HYPHENATION },
        { u"CharOverline"_ustr, RID_CHAR_OVERLINE },
        { u"CharOverlineColor"_ustr, RID_CHAR_OVERLINE_COLOR },
        { u"CharOverlineHasColor"_ustr, RID_CHAR_OVERLINE_HAS_COLOR },
        { u"CharPosture"_ustr, RID_CHAR_POSTURE },
        { u"CharPostureAsian"_ustr, RID_CHAR_POSTURE_ASIAN },
        { u"CharPostureComplex"_ustr, RID_CHAR_POSTURE_COMPLEX },
        { u"CharPropHeight"_ustr, RID_CHAR_PROP_HEIGHT },
        { u"CharPropHeightAsian"_ustr, RID_CHAR_PROP_HEIGHT_ASIAN },
        { u"CharPropHeightComplex"_ustr, RID_CHAR_PROP_HEIGHT_COMPLEX },
        { u"CharRelief"_ustr, RID_CHAR_RELIEF },
        { u"CharRightBorder"_ustr, RID_CHAR_RIGHT_BORDER },
        { u"CharRightBorderDistance"_ustr, RID_CHAR_RIGHT_BORDER_DISTANCE },
        { u"CharRotation"_ustr, RID_CHAR_ROTATION },
        { u"CharRotationIsFitToLine"_ustr, RID_CHAR_ROTATION_IS_FIT_TO_LINE },
        { u"CharScaleWidth"_ustr, RID_CHAR_SCALE_WIDTH },
        { u"CharScriptHint"_ustr, RID_CHAR_SCRIPT_HINT },
        { u"CharShadingValue"_ustr, RID_CHAR_SHADING_VALUE },
        { u"CharShadowFormat"_ustr, RID_CHAR_SHADOW_FORMAT },
        { u"CharShadowed"_ustr, RID_CHAR_SHADOWED },
        { u"CharStrikeout"_ustr, RID_CHAR_STRIKEOUT },
        { u"CharStyleName"_ustr, RID_CHAR_STYLE_NAME },
        { u"CharStyleNames"_ustr, RID_CHAR_STYLE_NAMES },
        { u"CharTopBorder"_ustr, RID_CHAR_TOP_BORDER },
        { u"CharTopBorderDistance"_ustr, RID_CHAR_TOP_BORDER_DISTANCE },
        { u"CharTransparence"_ustr, RID_CHAR_TRANSPARENCE },
        { u"CharUnderline"_ustr, RID_CHAR_UNDERLINE },
        { u"CharUnderlineColor"_ustr, RID_CHAR_UNDERLINE_COLOR },
        { u"CharUnderlineHasColor"_ustr, RID_CHAR_UNDERLINE_HAS_COLOR },
        { u"CharWeight"_ustr, RID_CHAR_WEIGHT },
        { u"CharWeightAsian"_ustr, RID_CHAR_WEIGHT_ASIAN },
        { u"CharWeightComplex"_ustr, RID_CHAR_WEIGHT_COMPLEX },
        { u"CharWordMode"_ustr, RID_CHAR_WORD_MODE },
        { u"ContinueingPreviousSubTree"_ustr, RID_CONTINUING_PREVIOUS_SUB_TREE },
        { u"DisplayName"_ustr, RID_DISPLAY_NAME },
        { u"DocumentIndex"_ustr, RID_DOCUMENT_INDEX },
        { u"DocumentIndexMark"_ustr, RID_DOCUMENT_INDEX_MARK },
        { u"DropCapCharStyleName"_ustr, RID_DROP_CAP_CHAR_STYLE_NAME },
        { u"DropCapFormat"_ustr, RID_DROP_CAP_FORMAT },
        { u"DropCapWholeWord"_ustr, RID_DROP_CAP_WHOLE_WORD },
        { u"Endnote"_ustr, RID_ENDNOTE },
        { u"FillBackground"_ustr, RID_FILL_BACKGROUND },
        { u"FillBitmap"_ustr, RID_FILL_BITMAP },
        { u"FillBitmapLogicalSize"_ustr, RID_FILL_BITMAP_LOGICAL_SIZE },
        { u"FillBitmapMode"_ustr, RID_FILL_BITMAP_MODE },
        { u"FillBitmapName"_ustr, RID_FILL_BITMAP_NAME },
        { u"FillBitmapOffsetX"_ustr, RID_FILL_BITMAP_OFFSET_X },
        { u"FillBitmapOffsetY"_ustr, RID_FILL_BITMAP_OFFSET_Y },
        { u"FillBitmapPositionOffsetX"_ustr, RID_FILL_BITMAP_POSITION_OFFSET_X },
        { u"FillBitmapPositionOffsetY"_ustr, RID_FILL_BITMAP_POSITION_OFFSET_Y },
        { u"FillBitmapRectanglePoint"_ustr, RID_FILL_BITMAP_RECTANGLE_POINT },
        { u"FillBitmapSizeX"_ustr, RID_FILL_BITMAP_SIZE_X },
        { u"FillBitmapSizeY"_ustr, RID_FILL_BITMAP_SIZE_Y },
        { u"FillBitmapStretch"_ustr, RID_FILL_BITMAP_STRETCH },
        { u"FillBitmapTile"_ustr, RID_FILL_BITMAP_TILE },
        { u"FillBitmapURL"_ustr, RID_FILL_BITMAP_URL },
        { u"FillColor"_ustr, RID_FILL_COLOR },
        { u"FillColor2"_ustr, RID_FILL_COLOR2 },
        { u"FillGradient"_ustr, RID_FILL_GRADIENT },
        { u"FillGradientName"_ustr, RID_FILL_GRADIENT_NAME },
        { u"FillGradientStepCount"_ustr, RID_FILL_GRADIENT_STEP_COUNT },
        { u"FillHatch"_ustr, RID_FILL_HATCH },
        { u"FillHatchName"_ustr, RID_FILL_HATCH_NAME },
        { u"FillStyle"_ustr, RID_FILL_STYLE },
        { u"FillTransparence"_ustr, RID_FILL_TRANSPARENCE },
        { u"FillTransparenceGradient"_ustr, RID_FILL_TRANSPARENCE_GRADIENT },
        { u"FillTransparenceGradientName"_ustr, RID_FILL_TRANSPARENCE_GRADIENT_NAME },
        { u"FollowStyle"_ustr, RID_FOLLOW_STYLE },
        { u"Footnote"_ustr, RID_FOOTNOTE },
        { u"Hidden"_ustr, RID_HIDDEN },
        { u"HyperLinkEvents"_ustr, RID_HYPERLINK_EVENTS },
        { u"HyperLinkName"_ustr, RID_HYPERLINK_NAME },
        { u"HyperLinkTarget"_ustr, RID_HYPERLINK_TARGET },
        { u"HyperLinkURL"_ustr, RID_HYPERLINK_URL },
        { u"IsAutoUpdate"_ustr, RID_IS_AUTO_UPDATE },
        { u"IsPhysical"_ustr, RID_IS_PHYSICAL },
        { u"LeftBorder"_ustr, RID_LEFT_BORDER },
        { u"LeftBorderDistance"_ustr, RID_LEFT_BORDER_DISTANCE },
        { u"ListAutoFormat"_ustr, RID_LIST_AUTO_FORMAT },
        { u"ListId"_ustr, RID_LIST_ID },
        { u"ListLabelString"_ustr, RID_LIST_LABEL_STRING },
        { u"MetadataReference"_ustr, RID_METADATA_REFERENCE },
        { u"NestedTextContent"_ustr, RID_NESTED_TEXT_CONTENT },
        { u"NumberingIsNumber"_ustr, RID_NUMBERING_IS_NUMBER },
        { u"NumberingLevel"_ustr, RID_NUMBERING_LEVEL },
        { u"NumberingRules"_ustr, RID_NUMBERING_RULES },
        { u"NumberingStartValue"_ustr, RID_NUMBERING_START_VALUE },
        { u"NumberingStyleName"_ustr, RID_NUMBERING_STYLE_NAME },
        { u"OutlineContentVisible"_ustr, RID_OUTLINE_CONTENT_VISIBLE },
        { u"OutlineLevel"_ustr, RID_OUTLINE_LEVEL },
        { u"PageDescName"_ustr, RID_PAGE_DESC_NAME },
        { u"PageNumberOffset"_ustr, RID_PAGE_NUMBER_OFFSET },
        { u"PageStyleName"_ustr, RID_PAGE_STYLE_NAME },
        { u"ParRsid"_ustr, RID_PAR_RSID },
        { u"ParaAdjust"_ustr, RID_PARA_ADJUST },
        { u"ParaAutoStyleName"_ustr, RID_PARA_AUTO_STYLE_NAME },
        { u"ParaBackColor"_ustr, RID_PARA_BACK_COLOR },
        { u"ParaBackGraphic"_ustr, RID_PARA_BACK_GRAPHIC },
        { u"ParaBackGraphicFilter"_ustr, RID_PARA_BACK_GRAPHIC_FILTER },
        { u"ParaBackGraphicLocation"_ustr, RID_PARA_BACK_GRAPHIC_LOCATION },
        { u"ParaBackGraphicURL"_ustr, RID_PARA_BACK_GRAPHIC_URL },
        { u"ParaBackTransparent"_ustr, RID_PARA_BACK_TRANSPARENT },
        { u"ParaBottomMargin"_ustr, RID_PARA_BOTTOM_MARGIN },
        { u"ParaBottomMarginRelative"_ustr, RID_PARA_BOTTOM_MARGIN_RELATIVE },
        { u"ParaChapterNumberingLevel"_ustr, RID_PARA_CHAPTER_NUMBERING_LEVEL },
        { u"ParaConditionalStyleName"_ustr, RID_PARA_CONDITIONAL_STYLE_NAME },
        { u"ParaContextMargin"_ustr, RID_PARA_CONTEXT_MARGIN },
        { u"ParaExpandSingleWord"_ustr, RID_PARA_EXPAND_SINGLE_WORD },
        { u"ParaFirstLineIndent"_ustr, RID_PARA_FIRST_LINE_INDENT },
        { u"ParaFirstLineIndentRelative"_ustr, RID_PARA_FIRST_LINE_INDENT_RELATIVE },
        { u"ParaHyphenationMaxHyphens"_ustr, RID_PARA_HYPHENATION_MAX_HYPHENS },
        { u"ParaHyphenationMaxLeadingChars"_ustr, RID_PARA_HYPHENATION_MAX_LEADING_CHARS },
        { u"ParaHyphenationMaxTrailingChars"_ustr, RID_PARA_HYPHENATION_MAX_TRAILING_CHARS },
        { u"ParaHyphenationCompoundMinLeadingChars"_ustr,
          RID_PARA_HYPHENATION_COMPOUND_MIN_LEADING_CHARS },
        { u"ParaHyphenationNoCaps"_ustr, RID_PARA_HYPHENATION_NO_CAPS },
        { u"ParaHyphenationNoLastWord"_ustr, RID_PARA_HYPHENATION_NO_LAST_WORD },
        { u"ParaHyphenationMinWordLength"_ustr, RID_PARA_HYPHENATION_MIN_WORD_LENGTH },
        { u"ParaHyphenationZone"_ustr, RID_PARA_HYPHENATION_ZONE },
        { u"ParaHyphenationZoneAlways"_ustr, RID_PARA_HYPHENATION_ZONE_ALWAYS },
        { u"ParaHyphenationZoneColumn"_ustr, RID_PARA_HYPHENATION_ZONE_COLUMN },
        { u"ParaHyphenationZonePage"_ustr, RID_PARA_HYPHENATION_ZONE_PAGE },
        { u"ParaHyphenationZoneSpread"_ustr, RID_PARA_HYPHENATION_ZONE_SPREAD },
        { u"ParaHyphenationKeep"_ustr, RID_PARA_HYPHENATION_KEEP },
        { u"ParaHyphenationKeepType"_ustr, RID_PARA_HYPHENATION_KEEP_TYPE },
        { u"ParaHyphenationKeepLine"_ustr, RID_PARA_HYPHENATION_KEEP_LINE },
        { u"ParaInteropGrabBag"_ustr, RID_PARA_INTEROP_GRAB_BAG },
        { u"ParaIsAutoFirstLineIndent"_ustr, RID_PARA_IS_AUTO_FIRST_LINE_INDENT },
        { u"ParaIsCharacterDistance"_ustr, RID_PARA_IS_CHARACTER_DISTANCE },
        { u"ParaIsConnectBorder"_ustr, RID_PARA_IS_CONNECT_BORDER },
        { u"ParaIsForbiddenRules"_ustr, RID_PARA_IS_FORBIDDEN_RULES },
        { u"ParaIsHangingPunctuation"_ustr, RID_PARA_IS_HANGING_PUNCTUATION },
        { u"ParaIsHyphenation"_ustr, RID_PARA_IS_HYPHENATION },
        { u"ParaIsNumberingRestart"_ustr, RID_PARA_IS_NUMBERING_RESTART },
        { u"ParaKeepTogether"_ustr, RID_PARA_KEEP_TOGETHER },
        { u"ParaLastLineAdjust"_ustr, RID_PARA_LAST_LINE_ADJUST },
        { u"ParaLeftMargin"_ustr, RID_PARA_LEFT_MARGIN },
        { u"ParaLeftMarginRelative"_ustr, RID_PARA_LEFT_MARGIN_RELATIVE },
        { u"ParaLetterSpacingMinimum"_ustr, RID_PARA_LETTER_SPACING_MIN },
        { u"ParaLetterSpacingMaximum"_ustr, RID_PARA_LETTER_SPACING_MAX },
        { u"ParaLineNumberCount"_ustr, RID_PARA_LINE_NUMBER_COUNT },
        { u"ParaLineNumberStartValue"_ustr, RID_PARA_LINE_NUMBER_START_VALUE },
        { u"ParaLineSpacing"_ustr, RID_PARA_LINE_SPACING },
        { u"ParaOrphans"_ustr, RID_PARA_ORPHANS },
        { u"ParaRegisterModeActive"_ustr, RID_PARA_REGISTER_MODE_ACTIVE },
        { u"ParaRightMargin"_ustr, RID_PARA_RIGHT_MARGIN },
        { u"ParaRightMarginRelative"_ustr, RID_PARA_RIGHT_MARGIN_RELATIVE },
        { u"ParaScaleWidthMinimum"_ustr, RID_PARA_SCALE_WIDTH_MIN },
        { u"ParaScaleWidthMaximum"_ustr, RID_PARA_SCALE_WIDTH_MAX },
        { u"ParaShadowFormat"_ustr, RID_PARA_SHADOW_FORMAT },
        { u"ParaSplit"_ustr, RID_PARA_SPLIT },
        { u"ParaStyleName"_ustr, RID_PARA_STYLE_NAME },
        { u"ParaTabStops"_ustr, RID_PARA_TAB_STOPS },
        { u"ParaTopMargin"_ustr, RID_PARA_TOP_MARGIN },
        { u"ParaTopMarginRelative"_ustr, RID_PARA_TOP_MARGIN_RELATIVE },
        { u"ParaUserDefinedAttributes"_ustr, RID_PARA_USER_DEFINED_ATTRIBUTES },
        { u"ParaVertAlignment"_ustr, RID_PARA_VERT_ALIGNMENT },
        { u"ParaWidows"_ustr, RID_PARA_WIDOWS },
        { u"ParaWordSpacingMinimum"_ustr, RID_PARA_WORD_SPACING_MIN },
        { u"ParaWordSpacing"_ustr, RID_PARA_WORD_SPACING },
        { u"ParaWordSpacingMaximum"_ustr, RID_PARA_WORD_SPACING_MAX },
        { u"ReferenceMark"_ustr, RID_REFERENCE_MARK },
        { u"RightBorder"_ustr, RID_RIGHT_BORDER },
        { u"RightBorderDistance"_ustr, RID_RIGHT_BORDER_DISTANCE },
        { u"Rsid"_ustr, RID_RSID },
        { u"RubyAdjust"_ustr, RID_RUBY_ADJUST },
        { u"RubyCharStyleName"_ustr, RID_RUBY_CHAR_STYLE_NAME },
        { u"RubyIsAbove"_ustr, RID_RUBY_IS_ABOVE },
        { u"RubyPosition"_ustr, RID_RUBY_POSITION },
        { u"RubyText"_ustr, RID_RUBY_TEXT },
        { u"SnapToGrid"_ustr, RID_SNAP_TO_GRID },
        { u"StyleInteropGrabBag"_ustr, RID_STYLE_INTEROP_GRAB_BAG },
        { u"TextField"_ustr, RID_TEXT_FIELD },
        { u"TextFrame"_ustr, RID_TEXT_FRAME },
        { u"TextParagraph"_ustr, RID_TEXT_PARAGRAPH },
        { u"TextSection"_ustr, RID_TEXT_SECTION },
        { u"TextTable"_ustr, RID_TEXT_TABLE },
        { u"TextUserDefinedAttributes"_ustr, RID_TEXT_USER_DEFINED_ATTRIBUTES },
        { u"TopBorder"_ustr, RID_TOP_BORDER },
        { u"TopBorderDistance"_ustr, RID_TOP_BORDER_DISTANCE },
        { u"UnvisitedCharStyleName"_ustr, RID_UNVISITED_CHAR_STYLE_NAME },
        { u"VisitedCharStyleName"_ustr, RID_VISITED_CHAR_STYLE_NAME },
        { u"WritingMode"_ustr, RID_WRITING_MODE },
        { u"WritingModeAutomatic"_ustr, RID_WRITING_MODE_AUTOMATIC },
        { u"BorderColor"_ustr, RID_BORDER_COLOR },
        { u"BorderInnerLineWidth"_ustr, RID_BORDER_INNER_LINE_WIDTH },
        { u"BorderLineDistance"_ustr, RID_BORDER_LINE_DISTANCE },
        { u"BorderLineStyle"_ustr, RID_BORDER_LINE_STYLE },
        { u"BorderLineWidth"_ustr, RID_BORDER_LINE_WIDTH },
        { u"BorderOuterLineWidth"_ustr, RID_BORDER_OUTER_LINE_WIDTH },
    };

    auto itr = aNameToRID.find(rName);
    if (itr != aNameToRID.end())
        return SwResId(itr->second);
    return rName;
}

static svx::sidebar::TreeNode SimplePropToTreeNode(const OUString& rName, const cpo::uno::Any& rVal)
{
    svx::sidebar::TreeNode aCurNode;
    aCurNode.sNodeName = PropertyNametoRID(rName);
    aCurNode.aValue = rVal;

    return aCurNode;
}

static svx::sidebar::TreeNode BorderToTreeNode(const OUString& rName, const cpo::uno::Any& rVal)
{
    table::BorderLine2 aBorder;
    rVal >>= aBorder;
    svx::sidebar::TreeNode aCurNode;
    aCurNode.sNodeName = PropertyNametoRID(rName);
    aCurNode.NodeType = svx::sidebar::TreeNode::ComplexProperty;

    aCurNode.children = {
        SimplePropToTreeNode(u"BorderColor"_ustr, cpo::uno::Any(aBorder.Color)),
        SimplePropToTreeNode(u"BorderLineWidth"_ustr, cpo::uno::Any(aBorder.LineWidth)),
        SimplePropToTreeNode(u"BorderLineStyle"_ustr, cpo::uno::Any(aBorder.LineStyle)),
        SimplePropToTreeNode(u"BorderLineDistance"_ustr, cpo::uno::Any(aBorder.LineDistance)),
        SimplePropToTreeNode(u"BorderInnerLineWidth"_ustr, cpo::uno::Any(aBorder.InnerLineWidth)),
        SimplePropToTreeNode(u"BorderOuterLineWidth"_ustr, cpo::uno::Any(aBorder.OuterLineWidth))
    };
    return aCurNode;
}

static svx::sidebar::TreeNode LocaleToTreeNode(const OUString& rName, const cpo::uno::Any& rVal)
{
    svx::sidebar::TreeNode aCurNode;
    aCurNode.sNodeName = PropertyNametoRID(rName);
    lang::Locale aLocale;
    rVal >>= aLocale;
    OUString aLocaleText(aLocale.Language + "-" + aLocale.Country);
    if (!aLocale.Variant.isEmpty())
        aLocaleText += " (" + aLocale.Variant + ")";
    aCurNode.aValue <<= aLocaleText;

    return aCurNode;
}

// Collect text of the current level of the annotated text
// ranges (InContentMetadata) and metadata fields (MetadataField)
static OUString NestedTextContentToText(const cpo::uno::Any& rVal)
{
    uno::Reference<container::XEnumerationAccess> xMeta;
    if (rVal >>= xMeta)
    {
        uno::Reference<container::XEnumeration> xMetaPortions = xMeta->createEnumeration();

        OUStringBuffer aBuf;
        while (xMetaPortions->hasMoreElements())
        {
            uno::Reference<css::text::XTextRange> xRng(xMetaPortions->nextElement(),
                                                       uno::UNO_QUERY);
            aBuf.append(xRng->getString());
        }
        return aBuf.makeStringAndClear();
    }

    return OUString();
}

// List metadata associated to the paragraph or character range
static void MetadataToTreeNode(const css::uno::Reference<css::uno::XInterface>& rSource,
                               svx::sidebar::TreeNode& rNode)
{
    uno::Reference<rdf::XMetadatable> xMeta(rSource, uno::UNO_QUERY_THROW);
    // don't add tree node "Metadata Reference", if there is no xml:id
    if (!xMeta.is() || xMeta->getMetadataReference().Second.isEmpty())
        return;

    // add metadata of parents for nested annotated text ranges
    uno::Reference<container::XChild> xChild(rSource, uno::UNO_QUERY);
    if (xChild.is())
    {
        uno::Reference<container::XEnumerationAccess> xParentMeta(xChild->getParent(),
                                                                  uno::UNO_QUERY);
        if (xParentMeta.is())
            MetadataToTreeNode(xParentMeta, rNode);
    }

    svx::sidebar::TreeNode aCurNode;
    aCurNode.sNodeName = PropertyNametoRID(u"MetadataReference"_ustr);
    aCurNode.NodeType = svx::sidebar::TreeNode::ComplexProperty;

    aCurNode.children.push_back(
        SimplePropToTreeNode(u"xml:id"_ustr, cpo::uno::Any(xMeta->getMetadataReference().Second)));

    // list associated (predicate, object) pairs of the actual subject
    // under the tree node "Metadata Reference"
    if (SwDocShell* pDocSh = static_cast<SwDocShell*>(SfxObjectShell::Current()))
    {
        rtl::Reference<SwXTextDocument> pSwXTextDocument(pDocSh->GetBaseModel());
        const uno::Reference<rdf::XRepository> xRepo = pSwXTextDocument->getRDFRepository();
        const css::uno::Reference<css::rdf::XResource> xSubject(rSource, uno::UNO_QUERY);
        std::map<OUString, OUString> xStatements
            = SwRDFHelper::getStatements(pSwXTextDocument, xRepo->getGraphNames(), xSubject);
        for (const auto& pair : xStatements)
            aCurNode.children.push_back(
                SimplePropToTreeNode(pair.first, cpo::uno::Any(pair.second)));
    }

    rNode.children.push_back(std::move(aCurNode));
}

static svx::sidebar::TreeNode
PropertyToTreeNode(const css::beans::Property& rProperty,
                   const uno::Reference<beans::XPropertySet>& xPropertiesSet, const bool rIsGrey)
{
    const OUString& rPropName = rProperty.Name;
    svx::sidebar::TreeNode aCurNode;
    const cpo::uno::Any aAny = xPropertiesSet->getPropertyValue(rPropName);
    aCurNode.sNodeName = PropertyNametoRID(rPropName);

    // These properties are handled separately as they are stored in STRUCT and not in single data members
    if (rPropName == "CharTopBorder" || rPropName == "CharBottomBorder"
        || rPropName == "CharLeftBorder" || rPropName == "CharRightBorder"
        || rPropName == "TopBorder" || rPropName == "BottomBorder" || rPropName == "LeftBorder"
        || rPropName == "RightBorder")
    {
        aCurNode = BorderToTreeNode(rPropName, aAny);
    }
    else if (rPropName == "CharLocale")
    {
        aCurNode = LocaleToTreeNode(rPropName, aAny);
    }
    else
        aCurNode = SimplePropToTreeNode(rPropName, aAny);

    if (rIsGrey)
    {
        aCurNode.isGrey = true;
        for (svx::sidebar::TreeNode& rChildNode : aCurNode.children)
            rChildNode.isGrey = true; // grey out all the children nodes
    }

    return aCurNode;
}

static void InsertValues(const css::uno::Reference<css::uno::XInterface>& rSource,
                         std::unordered_map<OUString, bool>& rIsDefined,
                         svx::sidebar::TreeNode& rNode, const bool isRoot,
                         const std::vector<OUString>& rHiddenProperty,
                         svx::sidebar::TreeNode& rFieldsNode)
{
    uno::Reference<beans::XPropertySet> xPropertiesSet(rSource, uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertyState> xPropertiesState(rSource, uno::UNO_QUERY_THROW);
    const cpo::uno::Sequence<beans::Property> aProperties
        = xPropertiesSet->getPropertySetInfo()->getProperties();

    for (const beans::Property& rProperty : aProperties)
    {
        const OUString& rPropName = rProperty.Name;
        if (std::find(rHiddenProperty.begin(), rHiddenProperty.end(), rPropName)
            != rHiddenProperty.end())
            continue;

        if (isRoot
            || xPropertiesState->getPropertyState(rPropName) == beans::PropertyState_DIRECT_VALUE)
        {
            svx::sidebar::TreeNode aCurNode
                = PropertyToTreeNode(rProperty, xPropertiesSet, rIsDefined[rPropName]);
            rIsDefined[rPropName] = true;

            // process NestedTextContent and show associated metadata
            // under the tree node "Metadata Reference", if they exist
            if (rPropName == "NestedTextContent")
            {
                uno::Reference<container::XEnumerationAccess> xMeta;
                if (aCurNode.aValue >>= xMeta)
                    MetadataToTreeNode(xMeta, rFieldsNode);
                aCurNode.aValue <<= NestedTextContentToText(aCurNode.aValue);
            }

            rNode.children.push_back(std::move(aCurNode));
        }
    }

    const comphelper::string::NaturalStringSorter aSorter(
        comphelper::getProcessComponentContext(),
        Application::GetSettings().GetUILanguageTag().getLocale());

    std::sort(
        rNode.children.begin(), rNode.children.end(),
        [&aSorter](svx::sidebar::TreeNode const& rEntry1, svx::sidebar::TreeNode const& rEntry2) {
            return aSorter.compare(rEntry1.sNodeName, rEntry2.sNodeName) < 0;
        });
}

static void UpdateTree(SwDocShell& rDocSh, const SwEditShell& rEditSh,
                       std::vector<svx::sidebar::TreeNode>& aStore, sal_Int32& rParIdx)
{
    SwDoc* pDoc = rDocSh.GetDoc();
    SwPaM* pCursor = rEditSh.GetCursor();
    svx::sidebar::TreeNode aCharDFNode;
    svx::sidebar::TreeNode aCharNode;
    svx::sidebar::TreeNode aParaNode;
    svx::sidebar::TreeNode aParaDFNode;
    svx::sidebar::TreeNode aBookmarksNode;
    svx::sidebar::TreeNode aFieldsNode;
    svx::sidebar::TreeNode aTextSectionsNode;

    aCharNode.sNodeName = SwResId(STR_CHARACTERSTYLEFAMILY);
    aParaNode.sNodeName = SwResId(STR_PARAGRAPHSTYLEFAMILY);
    aCharDFNode.sNodeName = SwResId(RID_CHAR_DIRECTFORMAT);
    aParaDFNode.sNodeName = SwResId(RID_PARA_DIRECTFORMAT);
    aBookmarksNode.sNodeName = SwResId(STR_CONTENT_TYPE_BOOKMARK);
    aFieldsNode.sNodeName = SwResId(STR_CONTENT_TYPE_TEXTFIELD);
    aTextSectionsNode.sNodeName = SwResId(STR_CONTENT_TYPE_REGION);
    aCharDFNode.NodeType = svx::sidebar::TreeNode::Category;
    aCharNode.NodeType = svx::sidebar::TreeNode::Category;
    aParaNode.NodeType = svx::sidebar::TreeNode::Category;
    aParaDFNode.NodeType = svx::sidebar::TreeNode::Category;
    aBookmarksNode.NodeType = svx::sidebar::TreeNode::Category;
    aFieldsNode.NodeType = svx::sidebar::TreeNode::Category;
    aTextSectionsNode.NodeType = svx::sidebar::TreeNode::Category;

    rtl::Reference<SwXTextRange> xRange(
        SwXTextRange::CreateXTextRange(*pDoc, *pCursor->GetPoint(), nullptr));
    if (!xRange)
        throw uno::RuntimeException();
    std::unordered_map<OUString, bool> aIsDefined;

    const std::vector<OUString> aHiddenProperties{ UNO_NAME_RSID,
                                                   UNO_NAME_PARA_IS_NUMBERING_RESTART,
                                                   UNO_NAME_PARA_STYLE_NAME,
                                                   UNO_NAME_PARA_CONDITIONAL_STYLE_NAME,
                                                   UNO_NAME_PAGE_STYLE_NAME,
                                                   UNO_NAME_NUMBERING_START_VALUE,
                                                   UNO_NAME_NUMBERING_IS_NUMBER,
                                                   UNO_NAME_PARA_CONTINUEING_PREVIOUS_SUB_TREE,
                                                   UNO_NAME_CHAR_STYLE_NAME,
                                                   UNO_NAME_NUMBERING_LEVEL,
                                                   UNO_NAME_SORTED_TEXT_ID,
                                                   UNO_NAME_PARRSID,
                                                   UNO_NAME_CHAR_COLOR_THEME,
                                                   UNO_NAME_CHAR_COLOR_TINT_OR_SHADE };

    const std::vector<OUString> aHiddenCharacterProperties{ UNO_NAME_CHAR_COLOR_THEME,
                                                            UNO_NAME_CHAR_COLOR_TINT_OR_SHADE };

    InsertValues(static_cast<cppu::OWeakObject*>(xRange.get()), aIsDefined, aCharDFNode, false,
                 aHiddenProperties, aFieldsNode);

    rtl::Reference<SwXTextDocument> pSwTextDocument(rDocSh.GetBaseModel());
    rtl::Reference<SwXStyleFamilies> xStyleFamilies = pSwTextDocument->getSwStyleFamilies();
    OUString sCurrentCharStyle, sCurrentParaStyle, sDisplayName;

    rtl::Reference<SwXStyleFamily> xStyleFamily = xStyleFamilies->GetCharacterStyles();
    xRange->getPropertyValue(u"CharStyleName"_ustr) >>= sCurrentCharStyle;
    xRange->getPropertyValue(u"ParaStyleName"_ustr) >>= sCurrentParaStyle;

    if (!sCurrentCharStyle.isEmpty())
    {
        rtl::Reference<SwXBaseStyle> xPropertiesSet
            = xStyleFamily->getStyleByName(sCurrentCharStyle);
        xPropertiesSet->getPropertyValue(u"DisplayName"_ustr) >>= sDisplayName;
        svx::sidebar::TreeNode aCurrentChild;
        aCurrentChild.sNodeName = sDisplayName;
        aCurrentChild.NodeType = svx::sidebar::TreeNode::ComplexProperty;

        InsertValues(cppu::getXWeak(xPropertiesSet.get()), aIsDefined, aCurrentChild, false,
                     aHiddenCharacterProperties, aFieldsNode);

        aCharNode.children.push_back(std::move(aCurrentChild));
    }

    // Collect paragraph direct formatting
    uno::Reference<container::XEnumeration> xParaEnum = xRange->createEnumeration();
    uno::Reference<text::XTextRange> xThisParagraphRange(xParaEnum->nextElement(), uno::UNO_QUERY);
    if (xThisParagraphRange.is())
    {
        // Collect metadata of the current paragraph
        MetadataToTreeNode(xThisParagraphRange, aParaDFNode);
        InsertValues(xThisParagraphRange, aIsDefined, aParaDFNode, false, aHiddenProperties,
                     aFieldsNode);
    }

    xStyleFamily = xStyleFamilies->GetParagraphStyles();

    while (!sCurrentParaStyle.isEmpty())
    {
        rtl::Reference<SwXStyle> xPropertiesStyle(
            xStyleFamily->getParagraphStyleByName(sCurrentParaStyle));
        xPropertiesStyle->getPropertyValue(u"DisplayName"_ustr) >>= sDisplayName;
        OUString aParentParaStyle = xPropertiesStyle->getParentStyle();
        svx::sidebar::TreeNode aCurrentChild;
        aCurrentChild.sNodeName = sDisplayName;
        aCurrentChild.NodeType = svx::sidebar::TreeNode::ComplexProperty;

        InsertValues(uno::Reference<beans::XPropertySet>(xPropertiesStyle), aIsDefined,
                     aCurrentChild, aParentParaStyle.isEmpty(), aHiddenCharacterProperties,
                     aFieldsNode);

        aParaNode.children.push_back(std::move(aCurrentChild));
        sCurrentParaStyle = aParentParaStyle;
    }

    std::reverse(aParaNode.children.begin(),
                 aParaNode.children.end()); // Parent style should be first then children

    // Collect bookmarks at character position
    rtl::Reference<SwXBookmarks> xBookmarks(pSwTextDocument->getSwBookmarks());
    for (sal_Int32 i = 0; i < xBookmarks->getCount(); ++i)
    {
        svx::sidebar::TreeNode aCurNode;
        rtl::Reference<SwXBookmark> xBookmark = xBookmarks->getBookmarkByIndex(i);

        try
        {
            uno::Reference<text::XTextRange> bookmarkRange = xBookmark->getAnchor();
            uno::Reference<text::XTextRangeCompare> xTextRangeCompare(xRange->getText(),
                                                                      uno::UNO_QUERY);
            if (xTextRangeCompare.is()
                && xTextRangeCompare->compareRegionStarts(bookmarkRange, xRange) != -1
                && xTextRangeCompare->compareRegionEnds(xRange, bookmarkRange) != -1)
            {
                aCurNode.sNodeName = xBookmark->getName();
                aCurNode.NodeType = svx::sidebar::TreeNode::ComplexProperty;

                MetadataToTreeNode(cppu::getXWeak(xBookmark.get()), aCurNode);
                // show bookmark only if it has RDF metadata
                if (aCurNode.children.size() > 0)
                    aBookmarksNode.children.push_back(std::move(aCurNode));
            }
        }
        catch (const lang::IllegalArgumentException&)
        {
        }
    }

    // Collect sections at character position
    rtl::Reference<SwXTextSections> xTextSections(pSwTextDocument->getSwTextSections());
    for (sal_Int32 i = 0; i < xTextSections->getCount(); ++i)
    {
        svx::sidebar::TreeNode aCurNode;
        rtl::Reference<SwXTextSection> section = xTextSections->getSwTextSectionByIndex(i);
        uno::Reference<container::XNamed> xTextSection(section);

        try
        {
            uno::Reference<text::XTextRange> sectionRange = section->getAnchor();
            uno::Reference<text::XTextRangeCompare> xTextRangeCompare(xRange->getText(),
                                                                      uno::UNO_QUERY);
            if (xTextRangeCompare.is()
                && xTextRangeCompare->compareRegionStarts(sectionRange, xRange) != -1
                && xTextRangeCompare->compareRegionEnds(xRange, sectionRange) != -1)
            {
                aCurNode.sNodeName = section->getName();
                aCurNode.NodeType = svx::sidebar::TreeNode::ComplexProperty;

                MetadataToTreeNode(xTextSection, aCurNode);
                // show section only if it has RDF metadata
                if (aCurNode.children.size() > 0)
                    aTextSectionsNode.children.push_back(std::move(aCurNode));
            }
        }
        catch (const lang::IllegalArgumentException&)
        {
        }
    }

    /*
    Display Order :-
    SECTIONS with RDF metadata (optional)
    BOOKMARKS with RDF metadata (optional)
    FIELDS with RDF metadata (optional)
    PARAGRAPH STYLE
    PARAGRAPH DIRECT FORMATTING
    CHARACTER STYLE
    DIRECT FORMATTING
    */
    rParIdx = 0;
    // show sections, bookmarks and fields only if they have RDF metadata
    if (aTextSectionsNode.children.size() > 0)
    {
        aStore.push_back(std::move(aTextSectionsNode));
        rParIdx++;
    }
    if (aBookmarksNode.children.size() > 0)
    {
        aStore.push_back(std::move(aBookmarksNode));
        rParIdx++;
    }
    if (aFieldsNode.children.size() > 0)
    {
        aStore.push_back(std::move(aFieldsNode));
        rParIdx++;
    }
    aStore.push_back(std::move(aParaNode));
    aStore.push_back(std::move(aParaDFNode));
    aStore.push_back(std::move(aCharNode));
    aStore.push_back(std::move(aCharDFNode));
}

IMPL_LINK(WriterInspectorTextPanel, AttrChangedNotify, LinkParamNone*, pLink, void)
{
    if (m_oldLink.IsSet())
        m_oldLink.Call(pLink);

    if (m_pShell->IsViewLocked())
    {
        return; // tdf#142806 avoid slowdown when storing files
    }

    SwDocShell* pDocSh = m_pShell->GetDoc()->GetDocShell();
    if (!pDocSh)
        return;

    std::vector<svx::sidebar::TreeNode> aStore;

    if (m_pShell->GetCursor()->GetPointNode().GetTextNode())
    {
        UpdateTree(*pDocSh, *m_pShell, aStore, m_nParIdx);
    }

    updateEntries(aStore, m_nParIdx);
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

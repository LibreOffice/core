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

#include "WriterInspectorTextPanel.hxx"

#include <doc.hxx>
#include <ndtxt.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <unoprnms.hxx>
#include <editeng/unoprnms.hxx>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <unotextrange.hxx>
#include <comphelper/string.hxx>
#include <comphelper/processfactory.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/settings.hxx>
#include <inspectorproperties.hrc>

namespace sw::sidebar
{
static void UpdateTree(SwDocShell* pDocSh, std::vector<svx::sidebar::TreeNode>& aStore);

VclPtr<vcl::Window> WriterInspectorTextPanel::Create(vcl::Window* pParent,
                                                     const uno::Reference<frame::XFrame>& rxFrame)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException(
            "no parent Window given to WriterInspectorTextPanel::Create", nullptr, 0);
    if (!rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to WriterInspectorTextPanel::Create",
                                             nullptr, 1);

    return VclPtr<WriterInspectorTextPanel>::Create(pParent, rxFrame);
}

WriterInspectorTextPanel::WriterInspectorTextPanel(vcl::Window* pParent,
                                                   const uno::Reference<frame::XFrame>& rxFrame)
    : InspectorTextPanel(pParent, rxFrame)
{
    SwDocShell* pDocSh = static_cast<SwDocShell*>(SfxObjectShell::Current());
    m_pShell = pDocSh->GetWrtShell();
    if (m_pShell)
    {
        m_oldLink = m_pShell->GetChgLnk();
        m_pShell->SetChgLnk(LINK(this, WriterInspectorTextPanel, AttrChangedNotify));
    }

    // Update panel on start
    std::vector<svx::sidebar::TreeNode> aStore;
    if (pDocSh->GetDoc()->GetEditShell()->GetCursor()->GetNode().GetTextNode())
        UpdateTree(pDocSh, aStore);
    updateEntries(aStore);
}

WriterInspectorTextPanel::~WriterInspectorTextPanel() { disposeOnce(); }

void WriterInspectorTextPanel::dispose()
{
    m_pShell->SetChgLnk(m_oldLink);

    InspectorTextPanel::dispose();
}

static OUString PropertyNametoRID(const OUString& rName)
{
    static const std::map<OUString, const char*> aNameToRID = {
        { "BorderDistance", RID_BORDER_DISTANCE },
        { "BottomBorder", RID_BOTTOM_BORDER },
        { "BottomBorderDistance", RID_BOTTOM_BORDER_DISTANCE },
        { "BreakType", RID_BREAK_TYPE },
        { "Category", RID_CATEGORY },
        { "Cell", RID_CELL },
        { "CharAutoEscapement", RID_CHAR_AUTO_ESCAPEMENT },
        { "CharAutoKerning", RID_CHAR_AUTO_KERNING },
        { "CharAutoStyleName", RID_CHAR_AUTO_STYLE_NAME },
        { "CharBackColor", RID_CHAR_BACK_COLOR },
        { "CharBackTransparent", RID_CHAR_BACK_TRANSPARENT },
        { "CharBorderDistance", RID_CHAR_BORDER_DISTANCE },
        { "CharBottomBorder", RID_CHAR_BOTTOM_BORDER },
        { "CharBottomBorderDistance", RID_CHAR_BOTTOM_BORDER_DISTANCE },
        { "CharCaseMap", RID_CHAR_CASE_MAP },
        { "CharColor", RID_CHAR_COLOR },
        { "CharCombineIsOn", RID_CHAR_COMBINE_IS_ON },
        { "CharCombinePrefix", RID_CHAR_COMBINE_PREFIX },
        { "CharCombineSuffix", RID_CHAR_COMBINE_SUFFIX },
        { "CharContoured", RID_CHAR_CONTOURED },
        { "CharCrossedOut", RID_CHAR_CROSSED_OUT },
        { "CharDiffHeight", RID_CHAR_DIFF_HEIGHT },
        { "CharDiffHeightAsian", RID_CHAR_DIFF_HEIGHT_ASIAN },
        { "CharDiffHeightComplex", RID_CHAR_DIFF_HEIGHT_COMPLEX },
        { "CharEmphasis", RID_CHAR_EMPHASIS },
        { "CharEscapement", RID_CHAR_ESCAPEMENT },
        { "CharEscapementHeight", RID_CHAR_ESCAPEMENT_HEIGHT },
        { "CharFlash", RID_CHAR_FLASH },
        { "CharFontCharSet", RID_CHAR_FONT_CHAR_SET },
        { "CharFontCharSetAsian", RID_CHAR_FONT_CHAR_SET_ASIAN },
        { "CharFontCharSetComplex", RID_CHAR_FONT_CHAR_SET_COMPLEX },
        { "CharFontFamily", RID_CHAR_FONT_FAMILY },
        { "CharFontFamilyAsian", RID_CHAR_FONT_FAMILY_ASIAN },
        { "CharFontFamilyComplex", RID_CHAR_FONT_FAMILY_COMPLEX },
        { "CharFontName", RID_CHAR_FONT_NAME },
        { "CharFontNameAsian", RID_CHAR_FONT_NAME_ASIAN },
        { "CharFontNameComplex", RID_CHAR_FONT_NAME_COMPLEX },
        { "CharFontPitch", RID_CHAR_FONT_PITCH },
        { "CharFontPitchAsian", RID_CHAR_FONT_PITCH_ASIAN },
        { "CharFontPitchComplex", RID_CHAR_FONT_PITCH_COMPLEX },
        { "CharFontStyleName", RID_CHAR_FONT_STYLE_NAME },
        { "CharFontStyleNameAsian", RID_CHAR_FONT_STYLE_NAME_ASIAN },
        { "CharFontStyleNameComplex", RID_CHAR_FONT_STYLE_NAME_COMPLEX },
        { "CharHeight", RID_CHAR_HEIGHT },
        { "CharHeightAsian", RID_CHAR_HEIGHT_ASIAN },
        { "CharHeightComplex", RID_CHAR_HEIGHT_COMPLEX },
        { "CharHidden", RID_CHAR_HIDDEN },
        { "CharHighlight", RID_CHAR_HIGHLIGHT },
        { "CharInteropGrabBag", RID_CHAR_INTEROP_GRAB_BAG },
        { "CharKerning", RID_CHAR_KERNING },
        { "CharLeftBorder", RID_CHAR_LEFT_BORDER },
        { "CharLeftBorderDistance", RID_CHAR_LEFT_BORDER_DISTANCE },
        { "CharLocale", RID_CHAR_LOCALE },
        { "CharLocaleAsian", RID_CHAR_LOCALE_ASIAN },
        { "CharLocaleComplex", RID_CHAR_LOCALE_COMPLEX },
        { "CharNoHyphenation", RID_CHAR_NO_HYPHENATION },
        { "CharOverline", RID_CHAR_OVERLINE },
        { "CharOverlineColor", RID_CHAR_OVERLINE_COLOR },
        { "CharOverlineHasColor", RID_CHAR_OVERLINE_HAS_COLOR },
        { "CharPosture", RID_CHAR_POSTURE },
        { "CharPostureAsian", RID_CHAR_POSTURE_ASIAN },
        { "CharPostureComplex", RID_CHAR_POSTURE_COMPLEX },
        { "CharPropHeight", RID_CHAR_PROP_HEIGHT },
        { "CharPropHeightAsian", RID_CHAR_PROP_HEIGHT_ASIAN },
        { "CharPropHeightComplex", RID_CHAR_PROP_HEIGHT_COMPLEX },
        { "CharRelief", RID_CHAR_RELIEF },
        { "CharRightBorder", RID_CHAR_RIGHT_BORDER },
        { "CharRightBorderDistance", RID_CHAR_RIGHT_BORDER_DISTANCE },
        { "CharRotation", RID_CHAR_ROTATION },
        { "CharRotationIsFitToLine", RID_CHAR_ROTATION_IS_FIT_TO_LINE },
        { "CharScaleWidth", RID_CHAR_SCALE_WIDTH },
        { "CharShadingValue", RID_CHAR_SHADING_VALUE },
        { "CharShadowFormat", RID_CHAR_SHADOW_FORMAT },
        { "CharShadowed", RID_CHAR_SHADOWED },
        { "CharStrikeout", RID_CHAR_STRIKEOUT },
        { "CharStyleName", RID_CHAR_STYLE_NAME },
        { "CharStyleNames", RID_CHAR_STYLE_NAMES },
        { "CharTopBorder", RID_CHAR_TOP_BORDER },
        { "CharTopBorderDistance", RID_CHAR_TOP_BORDER_DISTANCE },
        { "CharTransparence", RID_CHAR_TRANSPARENCE },
        { "CharUnderline", RID_CHAR_UNDERLINE },
        { "CharUnderlineColor", RID_CHAR_UNDERLINE_COLOR },
        { "CharUnderlineHasColor", RID_CHAR_UNDERLINE_HAS_COLOR },
        { "CharWeight", RID_CHAR_WEIGHT },
        { "CharWeightAsian", RID_CHAR_WEIGHT_ASIAN },
        { "CharWeightComplex", RID_CHAR_WEIGHT_COMPLEX },
        { "CharWordMode", RID_CHAR_WORD_MODE },
        { "ContinueingPreviousSubTree", RID_CONTINUING_PREVIOUS_SUB_TREE },
        { "DisplayName", RID_DISPLAY_NAME },
        { "DocumentIndex", RID_DOCUMENT_INDEX },
        { "DocumentIndexMark", RID_DOCUMENT_INDEX_MARK },
        { "DropCapCharStyleName", RID_DROP_CAP_CHAR_STYLE_NAME },
        { "DropCapFormat", RID_DROP_CAP_FORMAT },
        { "DropCapWholeWord", RID_DROP_CAP_WHOLE_WORD },
        { "Endnote", RID_ENDNOTE },
        { "FillBackground", RID_FILL_BACKGROUND },
        { "FillBitmap", RID_FILL_BITMAP },
        { "FillBitmapLogicalSize", RID_FILL_BITMAP_LOGICAL_SIZE },
        { "FillBitmapMode", RID_FILL_BITMAP_MODE },
        { "FillBitmapName", RID_FILL_BITMAP_NAME },
        { "FillBitmapOffsetX", RID_FILL_BITMAP_OFFSET_X },
        { "FillBitmapOffsetY", RID_FILL_BITMAP_OFFSET_Y },
        { "FillBitmapPositionOffsetX", RID_FILL_BITMAP_POSITION_OFFSET_X },
        { "FillBitmapPositionOffsetY", RID_FILL_BITMAP_POSITION_OFFSET_Y },
        { "FillBitmapRectanglePoint", RID_FILL_BITMAP_RECTANGLE_POINT },
        { "FillBitmapSizeX", RID_FILL_BITMAP_SIZE_X },
        { "FillBitmapSizeY", RID_FILL_BITMAP_SIZE_Y },
        { "FillBitmapStretch", RID_FILL_BITMAP_STRETCH },
        { "FillBitmapTile", RID_FILL_BITMAP_TILE },
        { "FillBitmapURL", RID_FILL_BITMAP_URL },
        { "FillColor", RID_FILL_COLOR },
        { "FillColor2", RID_FILL_COLOR2 },
        { "FillGradient", RID_FILL_GRADIENT },
        { "FillGradientName", RID_FILL_GRADIENT_NAME },
        { "FillGradientStepCount", RID_FILL_GRADIENT_STEP_COUNT },
        { "FillHatch", RID_FILL_HATCH },
        { "FillHatchName", RID_FILL_HATCH_NAME },
        { "FillStyle", RID_FILL_STYLE },
        { "FillTransparence", RID_FILL_TRANSPARENCE },
        { "FillTransparenceGradient", RID_FILL_TRANSPARENCE_GRADIENT },
        { "FillTransparenceGradientName", RID_FILL_TRANSPARENCE_GRADIENT_NAME },
        { "FollowStyle", RID_FOLLOW_STYLE },
        { "Footnote", RID_FOOTNOTE },
        { "Hidden", RID_HIDDEN },
        { "HyperLinkEvents", RID_HYPERLINK_EVENTS },
        { "HyperLinkName", RID_HYPERLINK_NAME },
        { "HyperLinkTarget", RID_HYPERLINK_TARGET },
        { "HyperLinkURL", RID_HYPERLINK_URL },
        { "IsAutoUpdate", RID_IS_AUTO_UPDATE },
        { "IsPhysical", RID_IS_PHYSICAL },
        { "LeftBorder", RID_LEFT_BORDER },
        { "LeftBorderDistance", RID_LEFT_BORDER_DISTANCE },
        { "ListAutoFormat", RID_LIST_AUTO_FORMAT },
        { "ListId", RID_LIST_ID },
        { "ListLabelString", RID_LIST_LABEL_STRING },
        { "NestedTextContent", RID_NESTED_TEXT_CONTENT },
        { "NumberingIsNumber", RID_NUMBERING_IS_NUMBER },
        { "NumberingLevel", RID_NUMBERING_LEVEL },
        { "NumberingRules", RID_NUMBERING_RULES },
        { "NumberingStartValue", RID_NUMBERING_START_VALUE },
        { "NumberingStyleName", RID_NUMBERING_STYLE_NAME },
        { "OutlineContentVisible", RID_OUTLINE_CONTENT_VISIBLE },
        { "OutlineLevel", RID_OUTLINE_LEVEL },
        { "PageDescName", RID_PAGE_DESC_NAME },
        { "PageNumberOffset", RID_PAGE_NUMBER_OFFSET },
        { "PageStyleName", RID_PAGE_STYLE_NAME },
        { "ParRsid", RID_PAR_RSID },
        { "ParaAdjust", RID_PARA_ADJUST },
        { "ParaAutoStyleName", RID_PARA_AUTO_STYLE_NAME },
        { "ParaBackColor", RID_PARA_BACK_COLOR },
        { "ParaBackGraphic", RID_PARA_BACK_GRAPHIC },
        { "ParaBackGraphicFilter", RID_PARA_BACK_GRAPHIC_FILTER },
        { "ParaBackGraphicLocation", RID_PARA_BACK_GRAPHIC_LOCATION },
        { "ParaBackGraphicURL", RID_PARA_BACK_GRAPHIC_URL },
        { "ParaBackTransparent", RID_PARA_BACK_TRANSPARENT },
        { "ParaBottomMargin", RID_PARA_BOTTOM_MARGIN },
        { "ParaBottomMarginRelative", RID_PARA_BOTTOM_MARGIN_RELATIVE },
        { "ParaChapterNumberingLevel", RID_PARA_CHAPTER_NUMBERING_LEVEL },
        { "ParaConditionalStyleName", RID_PARA_CONDITIONAL_STYLE_NAME },
        { "ParaContextMargin", RID_PARA_CONTEXT_MARGIN },
        { "ParaExpandSingleWord", RID_PARA_EXPAND_SINGLE_WORD },
        { "ParaFirstLineIndent", RID_PARA_FIRST_LINE_INDENT },
        { "ParaFirstLineIndentRelative", RID_PARA_FIRST_LINE_INDENT_RELATIVE },
        { "ParaHyphenationMaxHyphens", RID_PARA_HYPHENATION_MAX_HYPHENS },
        { "ParaHyphenationMaxLeadingChars", RID_PARA_HYPHENATION_MAX_LEADING_CHARS },
        { "ParaHyphenationMaxTrailingChars", RID_PARA_HYPHENATION_MAX_TRAILING_CHARS },
        { "ParaHyphenationNoCaps", RID_PARA_HYPHENATION_NO_CAPS },
        { "ParaInteropGrabBag", RID_PARA_INTEROP_GRAB_BAG },
        { "ParaIsAutoFirstLineIndent", RID_PARA_IS_AUTO_FIRST_LINE_INDENT },
        { "ParaIsCharacterDistance", RID_PARA_IS_CHARACTER_DISTANCE },
        { "ParaIsConnectBorder", RID_PARA_IS_CONNECT_BORDER },
        { "ParaIsForbiddenRules", RID_PARA_IS_FORBIDDEN_RULES },
        { "ParaIsHangingPunctuation", RID_PARA_IS_HANGING_PUNCTUATION },
        { "ParaIsHyphenation", RID_PARA_IS_HYPHENATION },
        { "ParaIsNumberingRestart", RID_PARA_IS_NUMBERING_RESTART },
        { "ParaKeepTogether", RID_PARA_KEEP_TOGETHER },
        { "ParaLastLineAdjust", RID_PARA_LAST_LINE_ADJUST },
        { "ParaLeftMargin", RID_PARA_LEFT_MARGIN },
        { "ParaLeftMarginRelative", RID_PARA_LEFT_MARGIN_RELATIVE },
        { "ParaLineNumberCount", RID_PARA_LINE_NUMBER_COUNT },
        { "ParaLineNumberStartValue", RID_PARA_LINE_NUMBER_START_VALUE },
        { "ParaLineSpacing", RID_PARA_LINE_SPACING },
        { "ParaOrphans", RID_PARA_ORPHANS },
        { "ParaRegisterModeActive", RID_PARA_REGISTER_MODE_ACTIVE },
        { "ParaRightMargin", RID_PARA_RIGHT_MARGIN },
        { "ParaRightMarginRelative", RID_PARA_RIGHT_MARGIN_RELATIVE },
        { "ParaShadowFormat", RID_PARA_SHADOW_FORMAT },
        { "ParaSplit", RID_PARA_SPLIT },
        { "ParaStyleName", RID_PARA_STYLE_NAME },
        { "ParaTabStops", RID_PARA_TAB_STOPS },
        { "ParaTopMargin", RID_PARA_TOP_MARGIN },
        { "ParaTopMarginRelative", RID_PARA_TOP_MARGIN_RELATIVE },
        { "ParaUserDefinedAttributes", RID_PARA_USER_DEFINED_ATTRIBUTES },
        { "ParaVertAlignment", RID_PARA_VERT_ALIGNMENT },
        { "ParaWidows", RID_PARA_WIDOWS },
        { "ReferenceMark", RID_REFERENCE_MARK },
        { "RightBorder", RID_RIGHT_BORDER },
        { "RightBorderDistance", RID_RIGHT_BORDER_DISTANCE },
        { "Rsid", RID_RSID },
        { "RubyAdjust", RID_RUBY_ADJUST },
        { "RubyCharStyleName", RID_RUBY_CHAR_STYLE_NAME },
        { "RubyIsAbove", RID_RUBY_IS_ABOVE },
        { "RubyPosition", RID_RUBY_POSITION },
        { "RubyText", RID_RUBY_TEXT },
        { "SnapToGrid", RID_SNAP_TO_GRID },
        { "StyleInteropGrabBag", RID_STYLE_INTEROP_GRAB_BAG },
        { "TextField", RID_TEXT_FIELD },
        { "TextFrame", RID_TEXT_FRAME },
        { "TextParagraph", RID_TEXT_PARAGRAPH },
        { "TextSection", RID_TEXT_SECTION },
        { "TextTable", RID_TEXT_TABLE },
        { "TextUserDefinedAttributes", RID_TEXT_USER_DEFINED_ATTRIBUTES },
        { "TopBorder", RID_TOP_BORDER },
        { "TopBorderDistance", RID_TOP_BORDER_DISTANCE },
        { "UnvisitedCharStyleName", RID_UNVISITED_CHAR_STYLE_NAME },
        { "VisitedCharStyleName", RID_VISITED_CHAR_STYLE_NAME },
        { "WritingMode", RID_WRITING_MODE },
        { "BorderColor", RID_BORDER_COLOR },
        { "BorderInnerLineWidth", RID_BORDER_INNER_LINE_WIDTH },
        { "BorderLineDistance", RID_BORDER_LINE_DISTANCE },
        { "BorderLineStyle", RID_BORDER_LINE_STYLE },
        { "BorderLineWidth", RID_BORDER_LINE_WIDTH },
        { "BorderOuterLineWidth", RID_BORDER_OUTER_LINE_WIDTH },
    };

    auto itr = aNameToRID.find(rName);
    if (itr != aNameToRID.end())
        return SwResId(itr->second);
    return rName;
}

static svx::sidebar::TreeNode SimplePropToTreeNode(const OUString& rName, const css::uno::Any& rVal)
{
    svx::sidebar::TreeNode aCurNode;
    aCurNode.sNodeName = PropertyNametoRID(rName);
    aCurNode.aValue = rVal;

    return aCurNode;
}

static svx::sidebar::TreeNode BorderToTreeNode(const OUString& rName, const css::uno::Any& rVal)
{
    table::BorderLine2 aBorder;
    rVal >>= aBorder;
    svx::sidebar::TreeNode aCurNode;
    aCurNode.sNodeName = PropertyNametoRID(rName);
    aCurNode.NodeType = svx::sidebar::TreeNode::ComplexProperty;

    aCurNode.children.push_back(SimplePropToTreeNode("BorderColor", css::uno::Any(aBorder.Color)));
    aCurNode.children.push_back(
        SimplePropToTreeNode("BorderLineWidth", css::uno::Any(aBorder.LineWidth)));
    aCurNode.children.push_back(
        SimplePropToTreeNode("BorderLineStyle", css::uno::Any(aBorder.LineStyle)));
    aCurNode.children.push_back(
        SimplePropToTreeNode("BorderLineDistance", css::uno::Any(aBorder.LineDistance)));
    aCurNode.children.push_back(
        SimplePropToTreeNode("BorderInnerLineWidth", css::uno::Any(aBorder.InnerLineWidth)));
    aCurNode.children.push_back(
        SimplePropToTreeNode("BorderOuterLineWidth", css::uno::Any(aBorder.OuterLineWidth)));

    return aCurNode;
}

static svx::sidebar::TreeNode
PropertyToTreeNode(const css::beans::Property& rProperty,
                   const uno::Reference<beans::XPropertySet>& xPropertiesSet, const bool& rIsGrey)
{
    const OUString& rPropName = rProperty.Name;
    svx::sidebar::TreeNode aCurNode;
    const uno::Any aAny = xPropertiesSet->getPropertyValue(rPropName);
    aCurNode.sNodeName = PropertyNametoRID(rPropName);

    // These properties are handled separately as they are stored in STRUCT and not in single data members
    if (rPropName == "CharTopBorder" || rPropName == "CharBottomBorder"
        || rPropName == "CharLeftBorder" || rPropName == "CharRightBorder"
        || rPropName == "TopBorder" || rPropName == "BottomBorder" || rPropName == "LeftBorder"
        || rPropName == "RightBorder")
    {
        aCurNode = BorderToTreeNode(rPropName, aAny);
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
                         svx::sidebar::TreeNode& rNode, const bool& isRoot,
                         const std::vector<OUString>& rHiddenProperty)
{
    uno::Reference<beans::XPropertySet> xPropertiesSet(rSource, uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertyState> xPropertiesState(rSource, uno::UNO_QUERY_THROW);
    const uno::Sequence<beans::Property> aProperties
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
            rNode.children.push_back(aCurNode);
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

static void UpdateTree(SwDocShell* pDocSh, std::vector<svx::sidebar::TreeNode>& aStore)
{
    SwDoc* pDoc = pDocSh->GetDoc();
    SwPaM* pCursor = pDoc->GetEditShell()->GetCursor();
    svx::sidebar::TreeNode aDFNode;
    svx::sidebar::TreeNode aCharNode;
    svx::sidebar::TreeNode aParaNode;

    aDFNode.sNodeName = "Direct Formatting";
    aCharNode.sNodeName = "Character Styles";
    aParaNode.sNodeName = "Paragraph Styles";
    aDFNode.NodeType = svx::sidebar::TreeNode::Category;
    aCharNode.NodeType = svx::sidebar::TreeNode::Category;
    aParaNode.NodeType = svx::sidebar::TreeNode::Category;

    uno::Reference<text::XTextRange> xRange(
        SwXTextRange::CreateXTextRange(*pDoc, *pCursor->GetPoint(), nullptr));
    uno::Reference<beans::XPropertySet> xPropertiesSet(xRange, uno::UNO_QUERY_THROW);
    std::unordered_map<OUString, bool> aIsDefined;

    InsertValues(xRange, aIsDefined, aDFNode, false,
                 { UNO_NAME_RSID, UNO_NAME_PARA_IS_NUMBERING_RESTART, UNO_NAME_PARA_STYLE_NAME,
                   UNO_NAME_PARA_CONDITIONAL_STYLE_NAME, UNO_NAME_PAGE_STYLE_NAME,
                   UNO_NAME_NUMBERING_START_VALUE, UNO_NAME_NUMBERING_IS_NUMBER,
                   UNO_NAME_PARA_CONTINUEING_PREVIOUS_SUB_TREE, UNO_NAME_CHAR_STYLE_NAME,
                   UNO_NAME_NUMBERING_LEVEL });

    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(pDocSh->GetBaseModel(),
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    OUString sCurrentCharStyle, sCurrentParaStyle, sDisplayName;

    uno::Reference<container::XNameAccess> xStyleFamily(
        xStyleFamilies->getByName("CharacterStyles"), uno::UNO_QUERY_THROW);
    xPropertiesSet->getPropertyValue("CharStyleName") >>= sCurrentCharStyle;
    xPropertiesSet->getPropertyValue("ParaStyleName") >>= sCurrentParaStyle;

    if (!sCurrentCharStyle.isEmpty())
    {
        xPropertiesSet.set(xStyleFamily->getByName(sCurrentCharStyle), css::uno::UNO_QUERY_THROW);
        xPropertiesSet->getPropertyValue("DisplayName") >>= sDisplayName;
        svx::sidebar::TreeNode aCurrentChild;
        aCurrentChild.sNodeName = sDisplayName;
        aCurrentChild.NodeType = svx::sidebar::TreeNode::ComplexProperty;

        InsertValues(xPropertiesSet, aIsDefined, aCurrentChild, false, {});

        aCharNode.children.push_back(aCurrentChild);
    }

    xStyleFamily.set(xStyleFamilies->getByName("ParagraphStyles"), uno::UNO_QUERY_THROW);

    while (!sCurrentParaStyle.isEmpty())
    {
        uno::Reference<style::XStyle> xPropertiesStyle(xStyleFamily->getByName(sCurrentParaStyle),
                                                       uno::UNO_QUERY_THROW);
        xPropertiesSet.set(xPropertiesStyle, css::uno::UNO_QUERY_THROW);
        xPropertiesSet->getPropertyValue("DisplayName") >>= sDisplayName;
        OUString aParentParaStyle = xPropertiesStyle->getParentStyle();
        svx::sidebar::TreeNode aCurrentChild;
        aCurrentChild.sNodeName = sDisplayName;
        aCurrentChild.NodeType = svx::sidebar::TreeNode::ComplexProperty;

        InsertValues(xPropertiesSet, aIsDefined, aCurrentChild, aParentParaStyle.isEmpty(), {});

        aParaNode.children.push_back(aCurrentChild);
        sCurrentParaStyle = aParentParaStyle;
    }

    std::reverse(aParaNode.children.begin(),
                 aParaNode.children.end()); // Parent style should be first then children

    /*
    Display Order :-
    PARAGRAPH STYLE
    CHARACTER STYLE
    DIRECT FORMATTING
    */
    aStore.push_back(aParaNode);
    aStore.push_back(aCharNode);
    aStore.push_back(aDFNode);
}

IMPL_LINK(WriterInspectorTextPanel, AttrChangedNotify, LinkParamNone*, pLink, void)
{
    if (m_oldLink.IsSet())
        m_oldLink.Call(pLink);

    SwDocShell* pDocSh = m_pShell->GetDoc()->GetDocShell();
    std::vector<svx::sidebar::TreeNode> aStore;

    if (pDocSh && pDocSh->GetDoc()->GetEditShell()->GetCursor()->GetNode().GetTextNode())
        UpdateTree(pDocSh, aStore);

    updateEntries(aStore);
}

} // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <libxml/xmlwriter.h>

#include <DocumentSettingManager.hxx>
#include <doc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <osl/diagnose.h>
#include <svx/svdmodel.hxx>
#include <svl/asiancfg.hxx>
#include <unotools/compatibility.hxx>
#include <comphelper/configuration.hxx>
#include <drawdoc.hxx>
#include <swmodule.hxx>
#include <linkenum.hxx>
#include <rootfrm.hxx>
#include <breakit.hxx>
#include <docary.hxx>

/* IDocumentSettingAccess */

sw::DocumentSettingManager::DocumentSettingManager(SwDoc &rDoc)
    :m_rDoc(rDoc),
    mnLinkUpdMode( GLOBALSETTING ),
    meFieldUpdMode( AUTOUPD_GLOBALSETTING ),
    meChrCmprType( CharCompressType::NONE ),
    mn32DummyCompatibilityOptions1(0),
    mn32DummyCompatibilityOptions2(0),
    mbHTMLMode(false),
    mbIsGlobalDoc(false),
    mbGlblDocSaveLinks(false),
    mbIsLabelDoc(false),
    mbPurgeOLE(true),
    mbKernAsianPunctuation(false),

    // COMPATIBILITY FLAGS START

    mbAddFlyOffsets(false),
    mbAddVerticalFlyOffsets(false),
    mbUseHiResolutionVirtualDevice(true),
    mbMathBaselineAlignment(false), // default for *old* documents is 'off'
    mbStylesNoDefault(false),
    mEmbedFonts(false),
    mEmbedUsedFonts(false),
    mEmbedLatinScriptFonts(true),
    mEmbedAsianScriptFonts(true),
    mEmbedComplexScriptFonts(true),
    mEmbedSystemFonts(false),
    mbOldNumbering(false),
    mbIgnoreFirstLineIndentInNumbering(false),
    mbDoNotResetParaAttrsForNumFont(false),
    mbTableRowKeep(false),
    mbIgnoreTabsAndBlanksForLineCalculation(false),
    mbDoNotCaptureDrawObjsOnPage(false),
    mbClipAsCharacterAnchoredWriterFlyFrames(false),
    mbUnixForceZeroExtLeading(false),
    mbTabRelativeToIndent(true),
    mbProtectForm(false), // i#78591#
    mbMsWordCompTrailingBlanks(false), // tdf#104349 tdf#104668
    mbMsWordCompMinLineHeightByFly(false),
    mbInvertBorderSpacing (false),
    mbCollapseEmptyCellPara(true),
    mbTabAtLeftIndentForParagraphsInList(false), //#i89181#
    mbSmallCapsPercentage66(false),
    mbTabOverflow(true),
    mbUnbreakableNumberings(false),
    mbClippedPictures(false),
    mbBackgroundParaOverDrawings(false),
    mbTabOverMargin(false),
    mbTabOverSpacing(false),
    mbTreatSingleColumnBreakAsPageBreak(false),
    mbSurroundTextWrapSmall(false),
    mbPropLineSpacingShrinksFirstLine(true),
    mbSubtractFlys(false),
    mApplyParagraphMarkFormatToNumbering(false),
    mbLastBrowseMode( false ),
    mbDisableOffPagePositioning ( false ),
    mbProtectBookmarks(false),
    mbProtectFields(false),
    mbHeaderSpacingBelowLastPara(false),
    mbFrameAutowidthWithMorePara(false),
    mbGutterAtTop(false),
    mbFootnoteInColumnToPageEnd(false),
    mnImagePreferredDPI(0),
    mbAutoFirstLineIndentDisregardLineSpace(true),
    mbNoNumberingShowFollowBy(false),
    mbDropCapPunctuation(true),
    mbUseVariableWidthNBSP(false)

    // COMPATIBILITY FLAGS END
{
    // COMPATIBILITY FLAGS START

    // Note: Any non-hidden compatibility flag should obtain its default
    // by asking SvtCompatibilityDefault, see below.

    if (!comphelper::IsFuzzing())
    {
        const SvtCompatibilityDefault aOptions;
        mbParaSpaceMax                      = aOptions.get(u"AddSpacing"_ustr);
        mbParaSpaceMaxAtPages               = aOptions.get(u"AddSpacingAtPages"_ustr);
        mbTabCompat                         = !aOptions.get(u"UseOurTabStopFormat"_ustr);
        mbUseVirtualDevice                  = true;
        mbAddExternalLeading                = !aOptions.get(u"NoExternalLeading"_ustr);
        mbOldLineSpacing                    = aOptions.get(u"UseLineSpacing"_ustr);
        mbAddParaSpacingToTableCells        = aOptions.get(u"AddTableSpacing"_ustr);
        mbAddParaLineSpacingToTableCells    = aOptions.get(u"AddTableLineSpacing"_ustr);
        mbUseFormerObjectPos                = aOptions.get(u"UseObjectPositioning"_ustr);
        mbUseFormerTextWrapping             = aOptions.get(u"UseOurTextWrapping"_ustr);
        mbConsiderWrapOnObjPos              = aOptions.get(u"ConsiderWrappingStyle"_ustr);
        mbDoNotJustifyLinesWithManualBreak  = !aOptions.get(u"ExpandWordSpace"_ustr);
        mbProtectForm                       = aOptions.get(u"ProtectForm"_ustr);
        mbMsWordCompTrailingBlanks          = aOptions.get(u"MsWordCompTrailingBlanks"_ustr);
        mbSubtractFlys                      = aOptions.get(u"SubtractFlysAnchoredAtFlys"_ustr);
        mbEmptyDbFieldHidesPara             = aOptions.get(u"EmptyDbFieldHidesPara"_ustr);
        mbUseVariableWidthNBSP              = aOptions.get(u"UseVariableWidthNBSP"_ustr);
        mbNoGapAfterNoteNumber              = aOptions.get(u"NoGapAfterNoteNumber"_ustr);
        mbTabRelativeToIndent               = aOptions.get(u"TabsRelativeToIndent"_ustr);
        mbTabOverMargin                     = aOptions.get(u"TabOverMargin"_ustr);
        mbDoNotMirrorRtlDrawObjs            = aOptions.get(u"DoNotMirrorRtlDrawObjs"_ustr);
    }
    else
    {
        mbParaSpaceMax                      = false;
        mbParaSpaceMaxAtPages               = false;
        mbTabCompat                         = true;
        mbUseVirtualDevice                  = true;
        mbAddExternalLeading                = true;
        mbOldLineSpacing                    = false;
        mbAddParaSpacingToTableCells        = false;
        mbAddParaLineSpacingToTableCells    = false;
        mbUseFormerObjectPos                = false;
        mbUseFormerTextWrapping             = false;
        mbConsiderWrapOnObjPos              = false;
        mbDoNotJustifyLinesWithManualBreak  = true;
        mbProtectForm                       = false;
        mbMsWordCompTrailingBlanks          = false;
        mbSubtractFlys                      = false;
        mbEmptyDbFieldHidesPara             = true;
        mbUseVariableWidthNBSP              = false;
    }

    // COMPATIBILITY FLAGS END

}


sw::DocumentSettingManager::~DocumentSettingManager()
{
}

/* IDocumentSettingAccess */
bool sw::DocumentSettingManager::get(/*[in]*/ DocumentSettingId id) const
{
    switch (id)
    {
        // COMPATIBILITY FLAGS START
        case DocumentSettingId::PARA_SPACE_MAX: return mbParaSpaceMax; //(n8Dummy1 & DUMMY_PARASPACEMAX);
        case DocumentSettingId::PARA_SPACE_MAX_AT_PAGES: return mbParaSpaceMaxAtPages; //(n8Dummy1 & DUMMY_PARASPACEMAX_AT_PAGES);
        case DocumentSettingId::TAB_COMPAT: return mbTabCompat; //(n8Dummy1 & DUMMY_TAB_COMPAT);
        case DocumentSettingId::ADD_FLY_OFFSETS: return mbAddFlyOffsets; //(n8Dummy2 & DUMMY_ADD_FLY_OFFSETS);
        case DocumentSettingId::ADD_VERTICAL_FLY_OFFSETS: return mbAddVerticalFlyOffsets;
        case DocumentSettingId::ADD_EXT_LEADING: return mbAddExternalLeading; //(n8Dummy2 & DUMMY_ADD_EXTERNAL_LEADING);
        case DocumentSettingId::USE_VIRTUAL_DEVICE: return mbUseVirtualDevice; //(n8Dummy1 & DUMMY_USE_VIRTUAL_DEVICE);
        case DocumentSettingId::USE_HIRES_VIRTUAL_DEVICE: return mbUseHiResolutionVirtualDevice; //(n8Dummy2 & DUMMY_USE_HIRES_VIR_DEV);
        case DocumentSettingId::OLD_NUMBERING: return mbOldNumbering;
        case DocumentSettingId::OLD_LINE_SPACING: return mbOldLineSpacing;
        case DocumentSettingId::ADD_PARA_SPACING_TO_TABLE_CELLS: return mbAddParaSpacingToTableCells;
        case DocumentSettingId::ADD_PARA_LINE_SPACING_TO_TABLE_CELLS: return mbAddParaLineSpacingToTableCells;
        case DocumentSettingId::USE_FORMER_OBJECT_POS: return mbUseFormerObjectPos;
        case DocumentSettingId::USE_FORMER_TEXT_WRAPPING: return mbUseFormerTextWrapping;
        case DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION: return mbConsiderWrapOnObjPos;
        case DocumentSettingId::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK: return mbDoNotJustifyLinesWithManualBreak;
        case DocumentSettingId::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING: return mbIgnoreFirstLineIndentInNumbering;
        case DocumentSettingId::NO_GAP_AFTER_NOTE_NUMBER: return mbNoGapAfterNoteNumber;
        case DocumentSettingId::TABLE_ROW_KEEP: return mbTableRowKeep;
        case DocumentSettingId::IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION: return mbIgnoreTabsAndBlanksForLineCalculation;
        case DocumentSettingId::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE: return mbDoNotCaptureDrawObjsOnPage;
        // #i68949#
        case DocumentSettingId::CLIP_AS_CHARACTER_ANCHORED_WRITER_FLY_FRAME: return mbClipAsCharacterAnchoredWriterFlyFrames;
        case DocumentSettingId::UNIX_FORCE_ZERO_EXT_LEADING: return mbUnixForceZeroExtLeading;
        case DocumentSettingId::TABS_RELATIVE_TO_INDENT : return mbTabRelativeToIndent;
        case DocumentSettingId::PROTECT_FORM: return mbProtectForm;
        // tdf#104349 tdf#104668
        case DocumentSettingId::MS_WORD_COMP_TRAILING_BLANKS: return mbMsWordCompTrailingBlanks;
        case DocumentSettingId::MS_WORD_COMP_MIN_LINE_HEIGHT_BY_FLY: return mbMsWordCompMinLineHeightByFly;
        // #i89181#
        case DocumentSettingId::TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST: return mbTabAtLeftIndentForParagraphsInList;
        case DocumentSettingId::INVERT_BORDER_SPACING: return mbInvertBorderSpacing;
        case DocumentSettingId::COLLAPSE_EMPTY_CELL_PARA: return mbCollapseEmptyCellPara;
        case DocumentSettingId::SMALL_CAPS_PERCENTAGE_66: return mbSmallCapsPercentage66;
        case DocumentSettingId::TAB_OVERFLOW: return mbTabOverflow;
        case DocumentSettingId::UNBREAKABLE_NUMBERINGS: return mbUnbreakableNumberings;
        case DocumentSettingId::CLIPPED_PICTURES: return mbClippedPictures;
        case DocumentSettingId::BACKGROUND_PARA_OVER_DRAWINGS: return mbBackgroundParaOverDrawings;
        case DocumentSettingId::TAB_OVER_MARGIN: return mbTabOverMargin;
        case DocumentSettingId::TAB_OVER_SPACING: return mbTabOverSpacing;
        case DocumentSettingId::TREAT_SINGLE_COLUMN_BREAK_AS_PAGE_BREAK: return mbTreatSingleColumnBreakAsPageBreak;
        case DocumentSettingId::SURROUND_TEXT_WRAP_SMALL: return mbSurroundTextWrapSmall;
        case DocumentSettingId::PROP_LINE_SPACING_SHRINKS_FIRST_LINE: return mbPropLineSpacingShrinksFirstLine;
        case DocumentSettingId::SUBTRACT_FLYS: return mbSubtractFlys;

        case DocumentSettingId::BROWSE_MODE: return mbLastBrowseMode; // Attention: normally the SwViewShell has to be asked!
        case DocumentSettingId::HTML_MODE: return mbHTMLMode;
        case DocumentSettingId::GLOBAL_DOCUMENT: return mbIsGlobalDoc;
        case DocumentSettingId::GLOBAL_DOCUMENT_SAVE_LINKS: return mbGlblDocSaveLinks;
        case DocumentSettingId::LABEL_DOCUMENT: return mbIsLabelDoc;
        case DocumentSettingId::PURGE_OLE: return mbPurgeOLE;
        case DocumentSettingId::KERN_ASIAN_PUNCTUATION: return mbKernAsianPunctuation;
        case DocumentSettingId::DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT: return mbDoNotResetParaAttrsForNumFont;
        case DocumentSettingId::MATH_BASELINE_ALIGNMENT: return mbMathBaselineAlignment;
        case DocumentSettingId::STYLES_NODEFAULT: return mbStylesNoDefault;
        case DocumentSettingId::EMBED_FONTS: return mEmbedFonts;
        case DocumentSettingId::EMBED_USED_FONTS: return mEmbedUsedFonts;
        case DocumentSettingId::EMBED_LATIN_SCRIPT_FONTS: return mEmbedLatinScriptFonts;
        case DocumentSettingId::EMBED_ASIAN_SCRIPT_FONTS: return mEmbedAsianScriptFonts;
        case DocumentSettingId::EMBED_COMPLEX_SCRIPT_FONTS: return mEmbedComplexScriptFonts;
        case DocumentSettingId::EMBED_SYSTEM_FONTS: return mEmbedSystemFonts;
        case DocumentSettingId::APPLY_PARAGRAPH_MARK_FORMAT_TO_NUMBERING: return mApplyParagraphMarkFormatToNumbering;
        case DocumentSettingId::DISABLE_OFF_PAGE_POSITIONING: return mbDisableOffPagePositioning;
        case DocumentSettingId::EMPTY_DB_FIELD_HIDES_PARA: return mbEmptyDbFieldHidesPara;
        case DocumentSettingId::CONTINUOUS_ENDNOTES: return mbContinuousEndnotes;
        case DocumentSettingId::PROTECT_BOOKMARKS: return mbProtectBookmarks;
        case DocumentSettingId::PROTECT_FIELDS: return mbProtectFields;
        case DocumentSettingId::HEADER_SPACING_BELOW_LAST_PARA: return mbHeaderSpacingBelowLastPara;
        case DocumentSettingId::FRAME_AUTOWIDTH_WITH_MORE_PARA: return mbFrameAutowidthWithMorePara;
        case DocumentSettingId::GUTTER_AT_TOP:
            return mbGutterAtTop;
        case DocumentSettingId::FOOTNOTE_IN_COLUMN_TO_PAGEEND: return mbFootnoteInColumnToPageEnd;
        case DocumentSettingId::AUTO_FIRST_LINE_INDENT_DISREGARD_LINE_SPACE:
            return mbAutoFirstLineIndentDisregardLineSpace;
        case DocumentSettingId::HYPHENATE_URLS: return mbHyphenateURLs;
        case DocumentSettingId::APPLY_TEXT_ATTR_TO_EMPTY_LINE_AT_END_OF_PARAGRAPH:
            return mbApplyTextAttrToEmptyLineAtEndOfParagraph;
        case DocumentSettingId::DO_NOT_BREAK_WRAPPED_TABLES:
            return mbDoNotBreakWrappedTables;
        case DocumentSettingId::ALLOW_TEXT_AFTER_FLOATING_TABLE_BREAK:
            return mbAllowTextAfterFloatingTableBreak;
        case DocumentSettingId::DO_NOT_MIRROR_RTL_DRAW_OBJS:
            return mbDoNotMirrorRtlDrawObjs;
        case DocumentSettingId::JUSTIFY_LINES_WITH_SHRINKING:
            return mbJustifyLinesWithShrinking;
        case DocumentSettingId::NO_NUMBERING_SHOW_FOLLOWBY: return mbNoNumberingShowFollowBy;
        case DocumentSettingId::DROP_CAP_PUNCTUATION: return mbDropCapPunctuation;
        case DocumentSettingId::USE_VARIABLE_WIDTH_NBSP: return mbUseVariableWidthNBSP;
        default:
            OSL_FAIL("Invalid setting id");
    }
    return false;
}

void sw::DocumentSettingManager::set(/*[in]*/ DocumentSettingId id, /*[in]*/ bool value)
{
    switch (id)
    {
        // COMPATIBILITY FLAGS START
        case DocumentSettingId::PARA_SPACE_MAX:
            mbParaSpaceMax = value;
            break;
        case DocumentSettingId::PARA_SPACE_MAX_AT_PAGES:
            mbParaSpaceMaxAtPages = value;
            break;
        case DocumentSettingId::TAB_COMPAT:
            mbTabCompat = value;
            break;
        case DocumentSettingId::ADD_FLY_OFFSETS:
            mbAddFlyOffsets = value;
            break;
        case DocumentSettingId::ADD_VERTICAL_FLY_OFFSETS:
            mbAddVerticalFlyOffsets = value;
            break;
        case DocumentSettingId::ADD_EXT_LEADING:
            mbAddExternalLeading = value;
            break;
        case DocumentSettingId::USE_VIRTUAL_DEVICE:
            mbUseVirtualDevice = value;
            break;
        case DocumentSettingId::USE_HIRES_VIRTUAL_DEVICE:
            mbUseHiResolutionVirtualDevice = value;
            break;
        case DocumentSettingId::OLD_NUMBERING:
            if (mbOldNumbering != value)
            {
                mbOldNumbering = value;

                const SwNumRuleTable& rNmTable = m_rDoc.GetNumRuleTable();
                for( SwNumRuleTable::size_type n = 0; n < rNmTable.size(); ++n )
                    rNmTable[n]->Invalidate();

                m_rDoc.UpdateNumRule();

                SwNumRule *pOutlineRule = m_rDoc.GetOutlineNumRule();
                if (pOutlineRule)
                {
                    pOutlineRule->Validate(m_rDoc);
                    // counting of phantoms depends on <IsOldNumbering()>
                    pOutlineRule->SetCountPhantoms( !mbOldNumbering );
                }
            }
            break;
        case DocumentSettingId::OLD_LINE_SPACING:
            mbOldLineSpacing = value;
            break;
        case DocumentSettingId::ADD_PARA_SPACING_TO_TABLE_CELLS:
            mbAddParaSpacingToTableCells = value;
            break;
        case DocumentSettingId::ADD_PARA_LINE_SPACING_TO_TABLE_CELLS:
            mbAddParaLineSpacingToTableCells = value;
            break;
        case DocumentSettingId::USE_FORMER_OBJECT_POS:
            mbUseFormerObjectPos = value;
            break;
        case DocumentSettingId::USE_FORMER_TEXT_WRAPPING:
            mbUseFormerTextWrapping = value;
            break;
        case DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION:
            mbConsiderWrapOnObjPos = value;
            break;
        case DocumentSettingId::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK:
            mbDoNotJustifyLinesWithManualBreak = value;
            break;
        case DocumentSettingId::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING:
            mbIgnoreFirstLineIndentInNumbering = value;
            break;
        case DocumentSettingId::NO_GAP_AFTER_NOTE_NUMBER:
            mbNoGapAfterNoteNumber = value;
            break;

        case DocumentSettingId::TABLE_ROW_KEEP:
            mbTableRowKeep = value;
            break;

        case DocumentSettingId::IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION:
            mbIgnoreTabsAndBlanksForLineCalculation = value;
            break;

        case DocumentSettingId::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE:
            mbDoNotCaptureDrawObjsOnPage = value;
            break;

        // #i68949#
        case DocumentSettingId::CLIP_AS_CHARACTER_ANCHORED_WRITER_FLY_FRAME:
            mbClipAsCharacterAnchoredWriterFlyFrames = value;
            break;

        case DocumentSettingId::UNIX_FORCE_ZERO_EXT_LEADING:
            mbUnixForceZeroExtLeading = value;
            break;

        case DocumentSettingId::PROTECT_FORM:
            mbProtectForm = value;
            break;

        // tdf#140349
        case DocumentSettingId::MS_WORD_COMP_TRAILING_BLANKS:
            mbMsWordCompTrailingBlanks = value;
            break;

        case DocumentSettingId::MS_WORD_COMP_MIN_LINE_HEIGHT_BY_FLY:
            mbMsWordCompMinLineHeightByFly = value;
            break;

        case DocumentSettingId::TABS_RELATIVE_TO_INDENT:
            mbTabRelativeToIndent = value;
            break;
        // #i89181#
        case DocumentSettingId::TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST:
            mbTabAtLeftIndentForParagraphsInList = value;
            break;

        case DocumentSettingId::INVERT_BORDER_SPACING:
            mbInvertBorderSpacing = value;
            break;

        case DocumentSettingId::COLLAPSE_EMPTY_CELL_PARA:
            mbCollapseEmptyCellPara = value;
            break;

        case DocumentSettingId::SMALL_CAPS_PERCENTAGE_66:
            mbSmallCapsPercentage66 = value;
            break;

        case DocumentSettingId::TAB_OVERFLOW:
            mbTabOverflow = value;
            break;

        case DocumentSettingId::UNBREAKABLE_NUMBERINGS:
            mbUnbreakableNumberings = value;
            break;

        case DocumentSettingId::CLIPPED_PICTURES:
            mbClippedPictures = value;
            break;

        case DocumentSettingId::BACKGROUND_PARA_OVER_DRAWINGS:
            mbBackgroundParaOverDrawings = value;
            break;

        case DocumentSettingId::TAB_OVER_MARGIN:
            mbTabOverMargin = value;
            break;

        case DocumentSettingId::TAB_OVER_SPACING:
            mbTabOverSpacing = value;
            break;

        case DocumentSettingId::TREAT_SINGLE_COLUMN_BREAK_AS_PAGE_BREAK:
            mbTreatSingleColumnBreakAsPageBreak = value;
            break;

        case DocumentSettingId::SURROUND_TEXT_WRAP_SMALL:
            mbSurroundTextWrapSmall = value;
            break;

        case DocumentSettingId::PROP_LINE_SPACING_SHRINKS_FIRST_LINE:
            mbPropLineSpacingShrinksFirstLine = value;
            break;

        case DocumentSettingId::SUBTRACT_FLYS:
            mbSubtractFlys = value;
            break;

        case DocumentSettingId::AUTO_FIRST_LINE_INDENT_DISREGARD_LINE_SPACE:
            mbAutoFirstLineIndentDisregardLineSpace = value;
            break;

        case DocumentSettingId::HYPHENATE_URLS:
            mbHyphenateURLs = value;
            break;

        case DocumentSettingId::APPLY_TEXT_ATTR_TO_EMPTY_LINE_AT_END_OF_PARAGRAPH:
            mbApplyTextAttrToEmptyLineAtEndOfParagraph = value;
            break;

        case DocumentSettingId::DO_NOT_MIRROR_RTL_DRAW_OBJS:
            mbDoNotMirrorRtlDrawObjs = value;
            break;

        case DocumentSettingId::DO_NOT_BREAK_WRAPPED_TABLES:
            mbDoNotBreakWrappedTables = value;
            break;

        case DocumentSettingId::ALLOW_TEXT_AFTER_FLOATING_TABLE_BREAK:
            mbAllowTextAfterFloatingTableBreak = value;
            break;

        case DocumentSettingId::JUSTIFY_LINES_WITH_SHRINKING:
            mbJustifyLinesWithShrinking = value;
            break;

        case DocumentSettingId::NO_NUMBERING_SHOW_FOLLOWBY:
            mbNoNumberingShowFollowBy = value;
            break;

        case DocumentSettingId::DROP_CAP_PUNCTUATION:
            mbDropCapPunctuation = value;
            break;

        case DocumentSettingId::USE_VARIABLE_WIDTH_NBSP:
            mbUseVariableWidthNBSP = value;
            break;

        // COMPATIBILITY FLAGS END

        case DocumentSettingId::BROWSE_MODE: //can be used temporary (load/save) when no SwViewShell is available
            // Can't render in webview successfully.
            if (comphelper::LibreOfficeKit::isActive())
                mbLastBrowseMode = false;
            else
                mbLastBrowseMode = value;
            break;

        case DocumentSettingId::HTML_MODE:
            mbHTMLMode = value;
            break;

        case DocumentSettingId::GLOBAL_DOCUMENT:
            mbIsGlobalDoc = value;
            break;

        case DocumentSettingId::GLOBAL_DOCUMENT_SAVE_LINKS:
            mbGlblDocSaveLinks = value;
            break;

        case DocumentSettingId::LABEL_DOCUMENT:
            mbIsLabelDoc = value;
            break;

        case DocumentSettingId::PURGE_OLE:
            mbPurgeOLE = value;
            break;

        case DocumentSettingId::KERN_ASIAN_PUNCTUATION:
            mbKernAsianPunctuation = value;
            break;

        case DocumentSettingId::DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT:
            mbDoNotResetParaAttrsForNumFont = value;
            break;
        case DocumentSettingId::MATH_BASELINE_ALIGNMENT:
            mbMathBaselineAlignment  = value;
            break;
        case DocumentSettingId::STYLES_NODEFAULT:
            mbStylesNoDefault  = value;
            break;
        case DocumentSettingId::EMBED_FONTS:
            mEmbedFonts = value;
            break;
        case DocumentSettingId::EMBED_USED_FONTS:
            mEmbedUsedFonts = value;
            break;
        case DocumentSettingId::EMBED_LATIN_SCRIPT_FONTS:
            mEmbedLatinScriptFonts = value;
            break;
        case DocumentSettingId::EMBED_ASIAN_SCRIPT_FONTS:
            mEmbedAsianScriptFonts = value;
            break;
        case DocumentSettingId::EMBED_COMPLEX_SCRIPT_FONTS:
            mEmbedComplexScriptFonts = value;
            break;
        case DocumentSettingId::EMBED_SYSTEM_FONTS:
            mEmbedSystemFonts = value;
            break;
        case DocumentSettingId::APPLY_PARAGRAPH_MARK_FORMAT_TO_NUMBERING:
            mApplyParagraphMarkFormatToNumbering = value;
            break;
        case DocumentSettingId::DISABLE_OFF_PAGE_POSITIONING:
            mbDisableOffPagePositioning = value;
            break;
        case DocumentSettingId::EMPTY_DB_FIELD_HIDES_PARA:
            mbEmptyDbFieldHidesPara = value;
            break;
        case DocumentSettingId::CONTINUOUS_ENDNOTES:
            mbContinuousEndnotes = value;
            break;
        case DocumentSettingId::PROTECT_BOOKMARKS:
            mbProtectBookmarks = value;
            break;
        case DocumentSettingId::PROTECT_FIELDS:
            mbProtectFields = value;
            break;
        case DocumentSettingId::HEADER_SPACING_BELOW_LAST_PARA:
            mbHeaderSpacingBelowLastPara = value;
            break;
        case DocumentSettingId::FRAME_AUTOWIDTH_WITH_MORE_PARA:
            mbFrameAutowidthWithMorePara = value;
            break;
        case DocumentSettingId::GUTTER_AT_TOP:
            mbGutterAtTop = value;
            break;
        case DocumentSettingId::FOOTNOTE_IN_COLUMN_TO_PAGEEND:
            mbFootnoteInColumnToPageEnd = value;
            break;
        default:
            OSL_FAIL("Invalid setting id");
    }
}

const css::i18n::ForbiddenCharacters*
    sw::DocumentSettingManager::getForbiddenCharacters(/*[in]*/ LanguageType nLang, /*[in]*/ bool bLocaleData ) const
{
    const css::i18n::ForbiddenCharacters* pRet = nullptr;
    if (mxForbiddenCharsTable)
        pRet = mxForbiddenCharsTable->GetForbiddenCharacters( nLang, false );
    if( bLocaleData && !pRet && g_pBreakIt )
        pRet = &g_pBreakIt->GetForbidden( nLang );
    return pRet;
}

void sw::DocumentSettingManager::setForbiddenCharacters(/*[in]*/ LanguageType nLang,
                                   /*[in]*/ const css::i18n::ForbiddenCharacters& rFChars )
{
    if (!mxForbiddenCharsTable)
        mxForbiddenCharsTable = SvxForbiddenCharactersTable::makeForbiddenCharactersTable(::comphelper::getProcessComponentContext());
    mxForbiddenCharsTable->SetForbiddenCharacters( nLang, rFChars );

    SdrModel *pDrawModel = m_rDoc.getIDocumentDrawModelAccess().GetDrawModel();
    if( pDrawModel )
    {
        pDrawModel->SetForbiddenCharsTable(mxForbiddenCharsTable);
        if( !m_rDoc.IsInReading() )
            pDrawModel->ReformatAllTextObjects();
    }

    SwRootFrame* pTmpRoot = m_rDoc.getIDocumentLayoutAccess().GetCurrentLayout();
    if( pTmpRoot && !m_rDoc.IsInReading() )
    {
        pTmpRoot->StartAllAction();
        for(SwRootFrame* aLayout : m_rDoc.GetAllLayouts())
            aLayout->InvalidateAllContent(SwInvalidateFlags::Size);
        pTmpRoot->EndAllAction();
    }
    m_rDoc.getIDocumentState().SetModified();
}

std::shared_ptr<SvxForbiddenCharactersTable>& sw::DocumentSettingManager::getForbiddenCharacterTable()
{
    if (!mxForbiddenCharsTable)
        mxForbiddenCharsTable = SvxForbiddenCharactersTable::makeForbiddenCharactersTable(::comphelper::getProcessComponentContext());
    return mxForbiddenCharsTable;
}

const std::shared_ptr<SvxForbiddenCharactersTable>& sw::DocumentSettingManager::getForbiddenCharacterTable() const
{
    return mxForbiddenCharsTable;
}

sal_uInt16 sw::DocumentSettingManager::getLinkUpdateMode( /*[in]*/bool bGlobalSettings ) const
{
    sal_uInt16 nRet = mnLinkUpdMode;
    if( bGlobalSettings && GLOBALSETTING == nRet )
        nRet = SW_MOD()->GetLinkUpdMode();
    return nRet;
}

void sw::DocumentSettingManager::setLinkUpdateMode( /*[in]*/sal_uInt16 eMode )
{
    mnLinkUpdMode = eMode;
}

SwFieldUpdateFlags sw::DocumentSettingManager::getFieldUpdateFlags( /*[in]*/bool bGlobalSettings ) const
{
    SwFieldUpdateFlags eRet = meFieldUpdMode;
    if( bGlobalSettings && AUTOUPD_GLOBALSETTING == eRet )
        eRet = SW_MOD()->GetFieldUpdateFlags();
    return eRet;
}

void sw::DocumentSettingManager::setFieldUpdateFlags(/*[in]*/SwFieldUpdateFlags eMode )
{
    meFieldUpdMode = eMode;
}

CharCompressType sw::DocumentSettingManager::getCharacterCompressionType() const
{
    return meChrCmprType;
}

void sw::DocumentSettingManager::setCharacterCompressionType( /*[in]*/CharCompressType n )
{
    if( meChrCmprType == n )
        return;

    meChrCmprType = n;

    SdrModel *pDrawModel = m_rDoc.getIDocumentDrawModelAccess().GetDrawModel();
    if( pDrawModel )
    {
        pDrawModel->SetCharCompressType( n );
        if( !m_rDoc.IsInReading() )
            pDrawModel->ReformatAllTextObjects();
    }

    SwRootFrame* pTmpRoot = m_rDoc.getIDocumentLayoutAccess().GetCurrentLayout();
    if( pTmpRoot && !m_rDoc.IsInReading() )
    {
        pTmpRoot->StartAllAction();
        for( auto aLayout : m_rDoc.GetAllLayouts() )
            aLayout->InvalidateAllContent(SwInvalidateFlags::Size);
        pTmpRoot->EndAllAction();
    }
    m_rDoc.getIDocumentState().SetModified();
}


void sw::DocumentSettingManager::ReplaceCompatibilityOptions(const DocumentSettingManager& rSource)
{
    Setn32DummyCompatibilityOptions1( rSource.Getn32DummyCompatibilityOptions1() );
    Setn32DummyCompatibilityOptions2( rSource.Getn32DummyCompatibilityOptions2() );

    // No mbHTMLMode: this is the subset of mbLastBrowseMode, which can be temporarily enabled even
    // for non-SwWebDocShells.
    // No mbIsGlobalDoc: this is true for SwGlobalDocShells.
    mbGlblDocSaveLinks = rSource.mbGlblDocSaveLinks;
    mbIsLabelDoc = rSource.mbIsLabelDoc;
    mbPurgeOLE = rSource.mbPurgeOLE;
    mbKernAsianPunctuation = rSource.mbKernAsianPunctuation;
    mbParaSpaceMax = rSource.mbParaSpaceMax;
    mbParaSpaceMaxAtPages = rSource.mbParaSpaceMaxAtPages;
    mbTabCompat = rSource.mbTabCompat;
    mbUseVirtualDevice = rSource.mbUseVirtualDevice;
    mbAddFlyOffsets = rSource.mbAddFlyOffsets;
    mbAddVerticalFlyOffsets = rSource.mbAddVerticalFlyOffsets;
    mbAddExternalLeading = rSource.mbAddExternalLeading;
    mbUseHiResolutionVirtualDevice = rSource.mbUseHiResolutionVirtualDevice;
    mbOldLineSpacing = rSource.mbOldLineSpacing;
    mbAddParaSpacingToTableCells = rSource.mbAddParaSpacingToTableCells;
    mbAddParaLineSpacingToTableCells = rSource.mbAddParaLineSpacingToTableCells;
    mbUseFormerObjectPos = rSource.mbUseFormerObjectPos;
    mbUseFormerTextWrapping = rSource.mbUseFormerTextWrapping;
    mbConsiderWrapOnObjPos = rSource.mbConsiderWrapOnObjPos;
    mbMathBaselineAlignment = rSource.mbMathBaselineAlignment;
    mbStylesNoDefault = rSource.mbStylesNoDefault;
    mbOldNumbering = rSource.mbOldNumbering;
    mbIgnoreFirstLineIndentInNumbering = rSource.mbIgnoreFirstLineIndentInNumbering;
    mbNoGapAfterNoteNumber = rSource.mbNoGapAfterNoteNumber;
    mbDoNotJustifyLinesWithManualBreak = rSource.mbDoNotJustifyLinesWithManualBreak;
    mbDoNotResetParaAttrsForNumFont = rSource.mbDoNotResetParaAttrsForNumFont;
    mbTableRowKeep = rSource.mbTableRowKeep;
    mbIgnoreTabsAndBlanksForLineCalculation = rSource.mbIgnoreTabsAndBlanksForLineCalculation;
    mbDoNotCaptureDrawObjsOnPage = rSource.mbDoNotCaptureDrawObjsOnPage;
    mbClipAsCharacterAnchoredWriterFlyFrames = rSource.mbClipAsCharacterAnchoredWriterFlyFrames;
    mbUnixForceZeroExtLeading = rSource.mbUnixForceZeroExtLeading;
    mbTabRelativeToIndent = rSource.mbTabRelativeToIndent;
    mbProtectForm = rSource.mbProtectForm;
    mbMsWordCompTrailingBlanks = rSource.mbMsWordCompTrailingBlanks;
    mbMsWordCompMinLineHeightByFly = rSource.mbMsWordCompMinLineHeightByFly;
    mbInvertBorderSpacing = rSource.mbInvertBorderSpacing;
    mbCollapseEmptyCellPara = rSource.mbCollapseEmptyCellPara;
    mbTabAtLeftIndentForParagraphsInList = rSource.mbTabAtLeftIndentForParagraphsInList;
    mbSmallCapsPercentage66 = rSource.mbSmallCapsPercentage66;
    mbTabOverflow = rSource.mbTabOverflow;
    mbUnbreakableNumberings = rSource.mbUnbreakableNumberings;
    mbClippedPictures = rSource.mbClippedPictures;
    mbBackgroundParaOverDrawings = rSource.mbBackgroundParaOverDrawings;
    mbTabOverMargin = rSource.mbTabOverMargin;
    mbTabOverSpacing = rSource.mbTabOverSpacing;
    mbTreatSingleColumnBreakAsPageBreak = rSource.mbTreatSingleColumnBreakAsPageBreak;
    mbSurroundTextWrapSmall = rSource.mbSurroundTextWrapSmall;
    mbPropLineSpacingShrinksFirstLine = rSource.mbPropLineSpacingShrinksFirstLine;
    mbSubtractFlys = rSource.mbSubtractFlys;
    // No mbLastBrowseMode: this is false by default everywhere
    mbDisableOffPagePositioning = rSource.mbDisableOffPagePositioning;
    mbEmptyDbFieldHidesPara = rSource.mbEmptyDbFieldHidesPara;
    mbContinuousEndnotes = rSource.mbContinuousEndnotes;
    // No mbProtectBookmarks: this is false by default everywhere
    // No mbProtectFields: this is false by default everywhere
    mbHeaderSpacingBelowLastPara = rSource.mbHeaderSpacingBelowLastPara;
    mbFrameAutowidthWithMorePara = rSource.mbFrameAutowidthWithMorePara;
    mbFootnoteInColumnToPageEnd = rSource.mbFootnoteInColumnToPageEnd;
    mbDropCapPunctuation = rSource.mbDropCapPunctuation;
    mbUseVariableWidthNBSP = rSource.mbUseVariableWidthNBSP;
}

sal_uInt32 sw::DocumentSettingManager::Getn32DummyCompatibilityOptions1() const
{
    return mn32DummyCompatibilityOptions1;
}

void sw::DocumentSettingManager::Setn32DummyCompatibilityOptions1( const sal_uInt32 CompatibilityOptions1 )
{
    mn32DummyCompatibilityOptions1 = CompatibilityOptions1;
}

sal_uInt32 sw::DocumentSettingManager::Getn32DummyCompatibilityOptions2() const
{
    return mn32DummyCompatibilityOptions2;
}

void sw::DocumentSettingManager::Setn32DummyCompatibilityOptions2( const sal_uInt32 CompatibilityOptions2 )
{
    mn32DummyCompatibilityOptions2 = CompatibilityOptions2;
}

void sw::DocumentSettingManager::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("DocumentSettingManager"));

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbHTMLMode"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbHTMLMode).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbIsGlobalDoc"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbIsGlobalDoc).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbGlblDocSaveLinks"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbGlblDocSaveLinks).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbIsLabelDoc"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbIsLabelDoc).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbPurgeOLE"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbPurgeOLE).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbKernAsianPunctuation"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbKernAsianPunctuation).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbParaSpaceMax"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbParaSpaceMax).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbParaSpaceMaxAtPages"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbParaSpaceMaxAtPages).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbTabCompat"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbTabCompat).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbUseVirtualDevice"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbUseVirtualDevice).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbAddFlyOffsets"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbAddFlyOffsets).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbAddVerticalFlyOffsets"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbAddVerticalFlyOffsets).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbAddExternalLeading"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbAddExternalLeading).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbUseHiResolutionVirtualDevice"));
    (void)xmlTextWriterWriteAttribute(
        pWriter, BAD_CAST("value"),
        BAD_CAST(OString::boolean(mbUseHiResolutionVirtualDevice).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbOldLineSpacing"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbOldLineSpacing).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbAddParaSpacingToTableCells"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbAddParaSpacingToTableCells).getStr()));
    (void)xmlTextWriterEndElement(pWriter);
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbAddParaLineSpacingToTableCells"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbAddParaLineSpacingToTableCells).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbUseFormerObjectPos"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbUseFormerObjectPos).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbUseFormerTextWrapping"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbUseFormerTextWrapping).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbConsiderWrapOnObjPos"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbConsiderWrapOnObjPos).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbMathBaselineAlignment"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbMathBaselineAlignment).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbStylesNoDefault"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbStylesNoDefault).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbOldNumbering"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbOldNumbering).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbIgnoreFirstLineIndentInNumbering"));
    (void)xmlTextWriterWriteAttribute(
        pWriter, BAD_CAST("value"),
        BAD_CAST(OString::boolean(mbIgnoreFirstLineIndentInNumbering).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbNoGapAfterNoteNumber"));
    (void)xmlTextWriterWriteAttribute(
        pWriter, BAD_CAST("value"),
        BAD_CAST(OString::boolean(mbNoGapAfterNoteNumber).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbDoNotJustifyLinesWithManualBreak"));
    (void)xmlTextWriterWriteAttribute(
        pWriter, BAD_CAST("value"),
        BAD_CAST(OString::boolean(mbDoNotJustifyLinesWithManualBreak).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbDoNotResetParaAttrsForNumFont"));
    (void)xmlTextWriterWriteAttribute(
        pWriter, BAD_CAST("value"),
        BAD_CAST(OString::boolean(mbDoNotResetParaAttrsForNumFont).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbTableRowKeep"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbTableRowKeep).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbIgnoreTabsAndBlanksForLineCalculation"));
    (void)xmlTextWriterWriteAttribute(
        pWriter, BAD_CAST("value"),
        BAD_CAST(OString::boolean(mbIgnoreTabsAndBlanksForLineCalculation).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbDoNotCaptureDrawObjsOnPage"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbDoNotCaptureDrawObjsOnPage).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbClipAsCharacterAnchoredWriterFlyFrames"));
    (void)xmlTextWriterWriteAttribute(
        pWriter, BAD_CAST("value"),
        BAD_CAST(OString::boolean(mbClipAsCharacterAnchoredWriterFlyFrames).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbUnixForceZeroExtLeading"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbUnixForceZeroExtLeading).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbTabRelativeToIndent"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbTabRelativeToIndent).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbProtectForm"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbProtectForm).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbMsWordCompTrailingBlanks"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbMsWordCompTrailingBlanks).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbMsWordCompMinLineHeightByFly"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbMsWordCompMinLineHeightByFly).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbInvertBorderSpacing"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbInvertBorderSpacing).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbCollapseEmptyCellPara"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbCollapseEmptyCellPara).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbTabAtLeftIndentForParagraphsInList"));
    (void)xmlTextWriterWriteAttribute(
        pWriter, BAD_CAST("value"),
        BAD_CAST(OString::boolean(mbTabAtLeftIndentForParagraphsInList).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbSmallCapsPercentage66"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbSmallCapsPercentage66).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbTabOverflow"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbTabOverflow).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbUnbreakableNumberings"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbUnbreakableNumberings).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbClippedPictures"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbClippedPictures).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbBackgroundParaOverDrawings"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbBackgroundParaOverDrawings).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbTabOverMargin"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbTabOverMargin).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbTabOverSpacing"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbTabOverSpacing).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbTreatSingleColumnBreakAsPageBreak"));
    (void)xmlTextWriterWriteAttribute(
        pWriter, BAD_CAST("value"),
        BAD_CAST(OString::boolean(mbTreatSingleColumnBreakAsPageBreak).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbSurroundTextWrapSmall"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbSurroundTextWrapSmall).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbPropLineSpacingShrinksFirstLine"));
    (void)xmlTextWriterWriteAttribute(
        pWriter, BAD_CAST("value"),
        BAD_CAST(OString::boolean(mbPropLineSpacingShrinksFirstLine).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbSubtractFlys"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbSubtractFlys).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbLastBrowseMode"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbLastBrowseMode).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbDisableOffPagePositioning"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbDisableOffPagePositioning).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbEmptyDbFieldHidesPara"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbEmptyDbFieldHidesPara).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbUseVariableWidthNBSP"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbUseVariableWidthNBSP).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbContinuousEndnotes"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbContinuousEndnotes).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbHeaderSpacingBelowLastPara"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbHeaderSpacingBelowLastPara).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbFrameAutowidthWithMorePara"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbFrameAutowidthWithMorePara).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbGutterAtTop"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbGutterAtTop).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbFootnoteInColumnToPageEnd"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbFootnoteInColumnToPageEnd).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbHyphenateURLs"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbHyphenateURLs).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbDoNotBreakWrappedTables"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbDoNotBreakWrappedTables).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbAllowTextAfterFloatingTableBreak"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbAllowTextAfterFloatingTableBreak).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbJustifyLinesWithShrinking"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbJustifyLinesWithShrinking).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mnImagePreferredDPI"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::number(mnImagePreferredDPI).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbApplyTextAttrToEmptyLineAtEndOfParagraph"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbApplyTextAttrToEmptyLineAtEndOfParagraph).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("mbDoNotMirrorRtlDrawObjs"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(mbDoNotMirrorRtlDrawObjs).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

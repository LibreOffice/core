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

#include <DocumentSettingManager.hxx>
#include <doc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <comphelper/processfactory.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <svx/svdmodel.hxx>
#include <unotools/compatibility.hxx>
#include <unotools/configmgr.hxx>
#include <drawdoc.hxx>
#include <swmodule.hxx>
#include <linkenum.hxx>
#include <rootfrm.hxx>
#include <breakit.hxx>
#include <docary.hxx>
#include <SwUndoFmt.hxx>

/* IDocumentSettingAccess */

sw::DocumentSettingManager::DocumentSettingManager(SwDoc &rDoc)
    :m_rDoc(rDoc),
    mnLinkUpdMode( GLOBALSETTING ),
    meFieldUpdMode( AUTOUPD_GLOBALSETTING ),
    meChrCmprType( CHARCOMPRESS_NONE ),
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
    mbFloattableNomargins(false),
    mEmbedFonts(false),
    mEmbedSystemFonts(false),
    mbOldNumbering(false),
    mbIgnoreFirstLineIndentInNumbering(false),
    mbDoNotResetParaAttrsForNumFont(false),
    mbTableRowKeep(false),
    mbIgnoreTabsAndBlanksForLineCalculation(false),
    mbDoNotCaptureDrawObjsOnPage(false),
    mbOutlineLevelYieldsOutlineRule(false),
    mbClipAsCharacterAnchoredWriterFlyFrames(false),
    mbUnixForceZeroExtLeading(false),
    mbTabRelativeToIndent(true),
    mbProtectForm(false), // i#78591#
    mbInvertBorderSpacing (false),
    mbCollapseEmptyCellPara(true),
    mbTabAtLeftIndentForParagraphsInList(false), //#i89181#
    mbSmallCapsPercentage66(false),
    mbTabOverflow(true),
    mbUnbreakableNumberings(false),
    mbClippedPictures(false),
    mbBackgroundParaOverDrawings(false),
    mbTabOverMargin(false),
    mbSurroundTextWrapSmall(false),
    mbPropLineSpacingShrinksFirstLine(true),
    mbSubtractFlys(false),
    mApplyParagraphMarkFormatToNumbering(false),
    mbLastBrowseMode( false )

    // COMPATIBILITY FLAGS END
{
    // COMPATIBILITY FLAGS START

    // Note: Any non-hidden compatibility flag should obtain its default
    // by asking SvtCompatibilityOptions, see below.

    if (!utl::ConfigManager::IsAvoidConfig())
    {
        const SvtCompatibilityOptions aOptions;
        mbParaSpaceMax                      = aOptions.IsAddSpacing();
        mbParaSpaceMaxAtPages               = aOptions.IsAddSpacingAtPages();
        mbTabCompat                         = !aOptions.IsUseOurTabStops();
        mbUseVirtualDevice                  = !aOptions.IsUsePrtDevice();
        mbAddExternalLeading                = !aOptions.IsNoExtLeading();
        mbOldLineSpacing                    = aOptions.IsUseLineSpacing();
        mbAddParaSpacingToTableCells        = aOptions.IsAddTableSpacing();
        mbUseFormerObjectPos                = aOptions.IsUseObjectPositioning();
        mbUseFormerTextWrapping             = aOptions.IsUseOurTextWrapping();
        mbConsiderWrapOnObjPos              = aOptions.IsConsiderWrappingStyle();

        mbDoNotJustifyLinesWithManualBreak      = !aOptions.IsExpandWordSpace();
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
        mbUseFormerObjectPos                = false;
        mbUseFormerTextWrapping             = false;
        mbConsiderWrapOnObjPos              = false;
        mbDoNotJustifyLinesWithManualBreak  = true;
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
        case DocumentSettingId::USE_FORMER_OBJECT_POS: return mbUseFormerObjectPos;
        case DocumentSettingId::USE_FORMER_TEXT_WRAPPING: return mbUseFormerTextWrapping;
        case DocumentSettingId::CONSIDER_WRAP_ON_OBJECT_POSITION: return mbConsiderWrapOnObjPos;
        case DocumentSettingId::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK: return mbDoNotJustifyLinesWithManualBreak;
        case DocumentSettingId::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING: return mbIgnoreFirstLineIndentInNumbering;
        case DocumentSettingId::OUTLINE_LEVEL_YIELDS_OUTLINE_RULE: return mbOutlineLevelYieldsOutlineRule;
        case DocumentSettingId::TABLE_ROW_KEEP: return mbTableRowKeep;
        case DocumentSettingId::IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION: return mbIgnoreTabsAndBlanksForLineCalculation;
        case DocumentSettingId::DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE: return mbDoNotCaptureDrawObjsOnPage;
        // #i68949#
        case DocumentSettingId::CLIP_AS_CHARACTER_ANCHORED_WRITER_FLY_FRAME: return mbClipAsCharacterAnchoredWriterFlyFrames;
        case DocumentSettingId::UNIX_FORCE_ZERO_EXT_LEADING: return mbUnixForceZeroExtLeading;
        case DocumentSettingId::TABS_RELATIVE_TO_INDENT : return mbTabRelativeToIndent;
        case DocumentSettingId::PROTECT_FORM: return mbProtectForm;
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
        case DocumentSettingId::FLOATTABLE_NOMARGINS: return mbFloattableNomargins;
        case DocumentSettingId::EMBED_FONTS: return mEmbedFonts;
        case DocumentSettingId::EMBED_SYSTEM_FONTS: return mEmbedSystemFonts;
        case DocumentSettingId::APPLY_PARAGRAPH_MARK_FORMAT_TO_NUMBERING: return mApplyParagraphMarkFormatToNumbering;
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
                    rNmTable[n]->SetInvalidRule(true);

                m_rDoc.UpdateNumRule();

                SwNumRule *pOutlineRule = m_rDoc.GetOutlineNumRule();
                if (pOutlineRule)
                {
                    pOutlineRule->Validate();
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

        case DocumentSettingId::OUTLINE_LEVEL_YIELDS_OUTLINE_RULE:
            mbOutlineLevelYieldsOutlineRule = value;
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

        case DocumentSettingId::SURROUND_TEXT_WRAP_SMALL:
            mbSurroundTextWrapSmall = value;
            break;

        case DocumentSettingId::PROP_LINE_SPACING_SHRINKS_FIRST_LINE:
            mbPropLineSpacingShrinksFirstLine = value;
            break;

        case DocumentSettingId::SUBTRACT_FLYS:
            mbSubtractFlys = value;
            break;

        // COMPATIBILITY FLAGS END

        case DocumentSettingId::BROWSE_MODE: //can be used temporary (load/save) when no SwViewShell is available
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
        case DocumentSettingId::FLOATTABLE_NOMARGINS:
            mbFloattableNomargins = value;
            break;
        case DocumentSettingId::EMBED_FONTS:
            mEmbedFonts = value;
            break;
        case DocumentSettingId::EMBED_SYSTEM_FONTS:
            mEmbedSystemFonts = value;
            break;
        case DocumentSettingId::APPLY_PARAGRAPH_MARK_FORMAT_TO_NUMBERING:
            mApplyParagraphMarkFormatToNumbering = value;
            break;
        default:
            OSL_FAIL("Invalid setting id");
    }
}

const css::i18n::ForbiddenCharacters*
    sw::DocumentSettingManager::getForbiddenCharacters(/*[in]*/ sal_uInt16 nLang, /*[in]*/ bool bLocaleData ) const
{
    const css::i18n::ForbiddenCharacters* pRet = nullptr;
    if( mxForbiddenCharsTable.is() )
        pRet = mxForbiddenCharsTable->GetForbiddenCharacters( nLang, false );
    if( bLocaleData && !pRet && g_pBreakIt )
        pRet = &g_pBreakIt->GetForbidden( (LanguageType)nLang );
    return pRet;
}

void sw::DocumentSettingManager::setForbiddenCharacters(/*[in]*/ sal_uInt16 nLang,
                                   /*[in]*/ const css::i18n::ForbiddenCharacters& rFChars )
{
    if( !mxForbiddenCharsTable.is() )
    {
        mxForbiddenCharsTable = new SvxForbiddenCharactersTable( ::comphelper::getProcessComponentContext() );
    }
    mxForbiddenCharsTable->SetForbiddenCharacters( nLang, rFChars );

    SdrModel *pDrawModel = m_rDoc.getIDocumentDrawModelAccess().GetDrawModel();
    if( pDrawModel )
    {
        pDrawModel->SetForbiddenCharsTable( mxForbiddenCharsTable );
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

rtl::Reference<SvxForbiddenCharactersTable>& sw::DocumentSettingManager::getForbiddenCharacterTable()
{
    if( !mxForbiddenCharsTable.is() )
    {
        mxForbiddenCharsTable = new SvxForbiddenCharactersTable( ::comphelper::getProcessComponentContext() );
    }
    return mxForbiddenCharsTable;
}

const rtl::Reference<SvxForbiddenCharactersTable>& sw::DocumentSettingManager::getForbiddenCharacterTable() const
{
    return mxForbiddenCharsTable;
}

sal_uInt16 sw::DocumentSettingManager::getLinkUpdateMode( /*[in]*/bool bGlobalSettings ) const
{
    sal_uInt16 nRet = mnLinkUpdMode;
    if( bGlobalSettings && GLOBALSETTING == nRet )
        nRet = SW_MOD()->GetLinkUpdMode(get(DocumentSettingId::HTML_MODE));
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
        eRet = SW_MOD()->GetFieldUpdateFlags(get(DocumentSettingId::HTML_MODE));
    return eRet;
}

void sw::DocumentSettingManager::setFieldUpdateFlags(/*[in]*/SwFieldUpdateFlags eMode )
{
    meFieldUpdMode = eMode;
}

SwCharCompressType sw::DocumentSettingManager::getCharacterCompressionType() const
{
    return meChrCmprType;
}

void sw::DocumentSettingManager::setCharacterCompressionType( /*[in]*/SwCharCompressType n )
{
    if( meChrCmprType != n )
    {
        meChrCmprType = n;

        SdrModel *pDrawModel = m_rDoc.getIDocumentDrawModelAccess().GetDrawModel();
        if( pDrawModel )
        {
            pDrawModel->SetCharCompressType( static_cast<sal_uInt16>(n) );
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
}


void sw::DocumentSettingManager::ReplaceCompatibilityOptions(const DocumentSettingManager& rSource)
{
    Setn32DummyCompatibilityOptions1( rSource.Getn32DummyCompatibilityOptions1() );
    Setn32DummyCompatibilityOptions2( rSource.Getn32DummyCompatibilityOptions2() );

    mbParaSpaceMax = rSource.mbParaSpaceMax;
    mbParaSpaceMaxAtPages = rSource.mbParaSpaceMaxAtPages;
    mbTabCompat = rSource.mbTabCompat;
    mbUseVirtualDevice = rSource.mbUseVirtualDevice;
    mbAddExternalLeading = rSource.mbAddExternalLeading;
    mbOldLineSpacing = rSource.mbOldLineSpacing;
    mbAddParaSpacingToTableCells = rSource.mbAddParaSpacingToTableCells;
    mbUseFormerObjectPos = rSource.mbUseFormerObjectPos;
    mbUseFormerTextWrapping = rSource.mbUseFormerTextWrapping;
    mbConsiderWrapOnObjPos = rSource.mbConsiderWrapOnObjPos;
    mbAddFlyOffsets = rSource.mbAddFlyOffsets;
    mbOldNumbering = rSource.mbOldNumbering;
    mbUseHiResolutionVirtualDevice = rSource.mbUseHiResolutionVirtualDevice;
    mbIgnoreFirstLineIndentInNumbering = rSource.mbIgnoreFirstLineIndentInNumbering;
    mbDoNotJustifyLinesWithManualBreak = rSource.mbDoNotJustifyLinesWithManualBreak;
    mbDoNotResetParaAttrsForNumFont = rSource.mbDoNotResetParaAttrsForNumFont;
    mbOutlineLevelYieldsOutlineRule = rSource.mbOutlineLevelYieldsOutlineRule;
    mbTableRowKeep = rSource.mbTableRowKeep;
    mbIgnoreTabsAndBlanksForLineCalculation = rSource.mbIgnoreTabsAndBlanksForLineCalculation;
    mbDoNotCaptureDrawObjsOnPage = rSource.mbDoNotCaptureDrawObjsOnPage;
    mbClipAsCharacterAnchoredWriterFlyFrames = rSource.mbClipAsCharacterAnchoredWriterFlyFrames;
    mbUnixForceZeroExtLeading = rSource.mbUnixForceZeroExtLeading;
    mbTabRelativeToIndent = rSource.mbTabRelativeToIndent;
    mbTabAtLeftIndentForParagraphsInList = rSource.mbTabAtLeftIndentForParagraphsInList;
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
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

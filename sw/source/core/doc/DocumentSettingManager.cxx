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

#include <doc.hxx>
#include <DocumentSettingManager.hxx>
#include <comphelper/processfactory.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <svx/svdmodel.hxx>
#include <unotools/compatibility.hxx>
#include <swmodule.hxx>
#include <linkenum.hxx>
#include <rootfrm.hxx>
#include <breakit.hxx>
#include <docary.hxx>
#include <SwUndoFmt.hxx>

/* IDocumentSettingAccess */

sw::DocumentSettingManager::DocumentSettingManager(SwDoc &rDoc)
    :m_rDoc(rDoc)
    ,
    mnLinkUpdMode( GLOBALSETTING ),
    meFldUpdMode( AUTOUPD_GLOBALSETTING ),
    meChrCmprType( CHARCOMPRESS_NONE ),
    mbHTMLMode(false),
    mbIsGlobalDoc(false),
    mbGlblDocSaveLinks(false),
    mbIsLabelDoc(false),
    mbPurgeOLE(true),
    mbKernAsianPunctuation(false),

    // COMPATIBILITY FLAGS START

    mbAddFlyOffsets(false),
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
    mbLastBrowseMode( false )

    // COMPATIBILITY FLAGS END
{
    // COMPATIBILITY FLAGS START

    // Note: Any non-hidden compatibility flag should obtain its default
    // by asking SvtCompatibilityOptions, see below.

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
        case PARA_SPACE_MAX: return mbParaSpaceMax; //(n8Dummy1 & DUMMY_PARASPACEMAX);
        case PARA_SPACE_MAX_AT_PAGES: return mbParaSpaceMaxAtPages; //(n8Dummy1 & DUMMY_PARASPACEMAX_AT_PAGES);
        case TAB_COMPAT: return mbTabCompat; //(n8Dummy1 & DUMMY_TAB_COMPAT);
        case ADD_FLY_OFFSETS: return mbAddFlyOffsets; //(n8Dummy2 & DUMMY_ADD_FLY_OFFSETS);
        case ADD_EXT_LEADING: return mbAddExternalLeading; //(n8Dummy2 & DUMMY_ADD_EXTERNAL_LEADING);
        case USE_VIRTUAL_DEVICE: return mbUseVirtualDevice; //(n8Dummy1 & DUMMY_USE_VIRTUAL_DEVICE);
        case USE_HIRES_VIRTUAL_DEVICE: return mbUseHiResolutionVirtualDevice; //(n8Dummy2 & DUMMY_USE_HIRES_VIR_DEV);
        case OLD_NUMBERING: return mbOldNumbering;
        case OLD_LINE_SPACING: return mbOldLineSpacing;
        case ADD_PARA_SPACING_TO_TABLE_CELLS: return mbAddParaSpacingToTableCells;
        case USE_FORMER_OBJECT_POS: return mbUseFormerObjectPos;
        case USE_FORMER_TEXT_WRAPPING: return mbUseFormerTextWrapping;
        case CONSIDER_WRAP_ON_OBJECT_POSITION: return mbConsiderWrapOnObjPos;
        case DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK: return mbDoNotJustifyLinesWithManualBreak;
        case IGNORE_FIRST_LINE_INDENT_IN_NUMBERING: return mbIgnoreFirstLineIndentInNumbering;
        case OUTLINE_LEVEL_YIELDS_OUTLINE_RULE: return mbOutlineLevelYieldsOutlineRule;
        case TABLE_ROW_KEEP: return mbTableRowKeep;
        case IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION: return mbIgnoreTabsAndBlanksForLineCalculation;
        case DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE: return mbDoNotCaptureDrawObjsOnPage;
        // #i68949#
        case CLIP_AS_CHARACTER_ANCHORED_WRITER_FLY_FRAME: return mbClipAsCharacterAnchoredWriterFlyFrames;
        case UNIX_FORCE_ZERO_EXT_LEADING: return mbUnixForceZeroExtLeading;
        case TABS_RELATIVE_TO_INDENT : return mbTabRelativeToIndent;
        case PROTECT_FORM: return mbProtectForm;
        // #i89181#
        case TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST: return mbTabAtLeftIndentForParagraphsInList;
        case INVERT_BORDER_SPACING: return mbInvertBorderSpacing;
        case COLLAPSE_EMPTY_CELL_PARA: return mbCollapseEmptyCellPara;
        case SMALL_CAPS_PERCENTAGE_66: return mbSmallCapsPercentage66;
        case TAB_OVERFLOW: return mbTabOverflow;
        case UNBREAKABLE_NUMBERINGS: return mbUnbreakableNumberings;
        case CLIPPED_PICTURES: return mbClippedPictures;
        case BACKGROUND_PARA_OVER_DRAWINGS: return mbBackgroundParaOverDrawings;
        case TAB_OVER_MARGIN: return mbTabOverMargin;
        case SURROUND_TEXT_WRAP_SMALL: return mbSurroundTextWrapSmall;

        case BROWSE_MODE: return mbLastBrowseMode; // Attention: normally the SwViewShell has to be asked!
        case HTML_MODE: return mbHTMLMode;
        case GLOBAL_DOCUMENT: return mbIsGlobalDoc;
        case GLOBAL_DOCUMENT_SAVE_LINKS: return mbGlblDocSaveLinks;
        case LABEL_DOCUMENT: return mbIsLabelDoc;
        case PURGE_OLE: return mbPurgeOLE;
        case KERN_ASIAN_PUNCTUATION: return mbKernAsianPunctuation;
        case DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT: return mbDoNotResetParaAttrsForNumFont;
        case MATH_BASELINE_ALIGNMENT: return mbMathBaselineAlignment;
        case STYLES_NODEFAULT: return mbStylesNoDefault;
        case FLOATTABLE_NOMARGINS: return mbFloattableNomargins;
        case EMBED_FONTS: return mEmbedFonts;
        case EMBED_SYSTEM_FONTS: return mEmbedSystemFonts;
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
        case PARA_SPACE_MAX:
            mbParaSpaceMax = value;
            break;
        case PARA_SPACE_MAX_AT_PAGES:
            mbParaSpaceMaxAtPages = value;
            break;
        case TAB_COMPAT:
            mbTabCompat = value;
            break;
        case ADD_FLY_OFFSETS:
            mbAddFlyOffsets = value;
            break;
        case ADD_EXT_LEADING:
            mbAddExternalLeading = value;
            break;
        case USE_VIRTUAL_DEVICE:
            mbUseVirtualDevice = value;
            break;
        case USE_HIRES_VIRTUAL_DEVICE:
            mbUseHiResolutionVirtualDevice = value;
            break;
        case OLD_NUMBERING:
            if (mbOldNumbering != value)
            {
                mbOldNumbering = value;

                const SwNumRuleTbl& rNmTbl = m_rDoc.GetNumRuleTbl();
                for( sal_uInt16 n = 0; n < rNmTbl.size(); ++n )
                    rNmTbl[n]->SetInvalidRule(sal_True);

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
        case OLD_LINE_SPACING:
            mbOldLineSpacing = value;
            break;
        case ADD_PARA_SPACING_TO_TABLE_CELLS:
            mbAddParaSpacingToTableCells = value;
            break;
        case USE_FORMER_OBJECT_POS:
            mbUseFormerObjectPos = value;
            break;
        case USE_FORMER_TEXT_WRAPPING:
            mbUseFormerTextWrapping = value;
            break;
        case CONSIDER_WRAP_ON_OBJECT_POSITION:
            mbConsiderWrapOnObjPos = value;
            break;
        case DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK:
            mbDoNotJustifyLinesWithManualBreak = value;
            break;
        case IGNORE_FIRST_LINE_INDENT_IN_NUMBERING:
            mbIgnoreFirstLineIndentInNumbering = value;
            break;

        case OUTLINE_LEVEL_YIELDS_OUTLINE_RULE:
            mbOutlineLevelYieldsOutlineRule = value;
            break;

        case TABLE_ROW_KEEP:
            mbTableRowKeep = value;
            break;

        case IGNORE_TABS_AND_BLANKS_FOR_LINE_CALCULATION:
            mbIgnoreTabsAndBlanksForLineCalculation = value;
            break;

        case DO_NOT_CAPTURE_DRAW_OBJS_ON_PAGE:
            mbDoNotCaptureDrawObjsOnPage = value;
            break;

        // #i68949#
        case CLIP_AS_CHARACTER_ANCHORED_WRITER_FLY_FRAME:
            mbClipAsCharacterAnchoredWriterFlyFrames = value;
            break;

        case UNIX_FORCE_ZERO_EXT_LEADING:
            mbUnixForceZeroExtLeading = value;
            break;

        case PROTECT_FORM:
            mbProtectForm = value;
            break;

        case TABS_RELATIVE_TO_INDENT:
            mbTabRelativeToIndent = value;
            break;
        // #i89181#
        case TAB_AT_LEFT_INDENT_FOR_PARA_IN_LIST:
            mbTabAtLeftIndentForParagraphsInList = value;
            break;

        case INVERT_BORDER_SPACING:
            mbInvertBorderSpacing = value;
            break;

        case COLLAPSE_EMPTY_CELL_PARA:
            mbCollapseEmptyCellPara = value;
            break;

        case SMALL_CAPS_PERCENTAGE_66:
            mbSmallCapsPercentage66 = value;
            break;

        case TAB_OVERFLOW:
            mbTabOverflow = value;
            break;

        case UNBREAKABLE_NUMBERINGS:
            mbUnbreakableNumberings = value;
            break;

        case CLIPPED_PICTURES:
            mbClippedPictures = value;
            break;

        case BACKGROUND_PARA_OVER_DRAWINGS:
            mbBackgroundParaOverDrawings = value;
            break;

        case TAB_OVER_MARGIN:
            mbTabOverMargin = value;
            break;

        case SURROUND_TEXT_WRAP_SMALL:
            mbSurroundTextWrapSmall = value;
            break;

        // COMPATIBILITY FLAGS END

        case BROWSE_MODE: //can be used temporary (load/save) when no SwViewShell is avaiable
            mbLastBrowseMode = value;
            break;

        case HTML_MODE:
            mbHTMLMode = value;
            break;

        case GLOBAL_DOCUMENT:
            mbIsGlobalDoc = value;
            break;

        case GLOBAL_DOCUMENT_SAVE_LINKS:
            mbGlblDocSaveLinks = value;
            break;

        case LABEL_DOCUMENT:
            mbIsLabelDoc = value;
            break;

        case PURGE_OLE:
            mbPurgeOLE = value;
            break;

        case KERN_ASIAN_PUNCTUATION:
            mbKernAsianPunctuation = value;
            break;

        case DO_NOT_RESET_PARA_ATTRS_FOR_NUM_FONT:
            mbDoNotResetParaAttrsForNumFont = value;
            break;
        case MATH_BASELINE_ALIGNMENT:
            mbMathBaselineAlignment  = value;
            break;
        case STYLES_NODEFAULT:
            mbStylesNoDefault  = value;
            break;
        case FLOATTABLE_NOMARGINS:
            mbFloattableNomargins = value;
            break;
        case EMBED_FONTS:
            mEmbedFonts = value;
            break;
        case EMBED_SYSTEM_FONTS:
            mEmbedSystemFonts = value;
            break;
        default:
            OSL_FAIL("Invalid setting id");
    }
}

const i18n::ForbiddenCharacters*
    sw::DocumentSettingManager::getForbiddenCharacters(/*[in]*/ sal_uInt16 nLang, /*[in]*/ bool bLocaleData ) const
{
    const i18n::ForbiddenCharacters* pRet = 0;
    if( mxForbiddenCharsTable.is() )
        pRet = mxForbiddenCharsTable->GetForbiddenCharacters( nLang, false );
    if( bLocaleData && !pRet && g_pBreakIt )
        pRet = &g_pBreakIt->GetForbidden( (LanguageType)nLang );
    return pRet;
}

void sw::DocumentSettingManager::setForbiddenCharacters(/*[in]*/ sal_uInt16 nLang,
                                   /*[in]*/ const com::sun::star::i18n::ForbiddenCharacters& rFChars )
{
    if( !mxForbiddenCharsTable.is() )
    {
        mxForbiddenCharsTable = new SvxForbiddenCharactersTable( ::comphelper::getProcessComponentContext() );
    }
    mxForbiddenCharsTable->SetForbiddenCharacters( nLang, rFChars );

    SdrModel *pDrawModel = m_rDoc.GetDrawModel();
    if( pDrawModel )
    {
        pDrawModel->SetForbiddenCharsTable( mxForbiddenCharsTable );
        if( !m_rDoc.IsInReading() )
            pDrawModel->ReformatAllTextObjects();
    }

    SwRootFrm* pTmpRoot = m_rDoc.GetCurrentLayout();
    if( pTmpRoot && !m_rDoc.IsInReading() )
    {
        pTmpRoot->StartAllAction();
        std::set<SwRootFrm*> aAllLayouts = m_rDoc.GetAllLayouts();
        std::for_each( aAllLayouts.begin(), aAllLayouts.end(), std::bind2nd(std::mem_fun(&SwRootFrm::InvalidateAllCntnt), INV_SIZE));
        pTmpRoot->EndAllAction();
    }
    m_rDoc.SetModified();
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
        nRet = SW_MOD()->GetLinkUpdMode(get(IDocumentSettingAccess::HTML_MODE));
    return nRet;
}

void sw::DocumentSettingManager::setLinkUpdateMode( /*[in]*/sal_uInt16 eMode )
{
    mnLinkUpdMode = eMode;
}

SwFldUpdateFlags sw::DocumentSettingManager::getFieldUpdateFlags( /*[in]*/bool bGlobalSettings ) const
{
    SwFldUpdateFlags eRet = meFldUpdMode;
    if( bGlobalSettings && AUTOUPD_GLOBALSETTING == eRet )
        eRet = SW_MOD()->GetFldUpdateFlags(get(IDocumentSettingAccess::HTML_MODE));
    return eRet;
}

void sw::DocumentSettingManager::setFieldUpdateFlags(/*[in]*/SwFldUpdateFlags eMode )
{
    meFldUpdMode = eMode;
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

        SdrModel *pDrawModel = m_rDoc.GetDrawModel();
        if( pDrawModel )
        {
            pDrawModel->SetCharCompressType( static_cast<sal_uInt16>(n) );
            if( !m_rDoc.IsInReading() )
                pDrawModel->ReformatAllTextObjects();
        }

        SwRootFrm* pTmpRoot = m_rDoc.GetCurrentLayout();
        if( pTmpRoot && !m_rDoc.IsInReading() )
        {
            pTmpRoot->StartAllAction();
            std::set<SwRootFrm*> aAllLayouts = m_rDoc.GetAllLayouts();
            std::for_each( aAllLayouts.begin(), aAllLayouts.end(), std::bind2nd(std::mem_fun(&SwRootFrm::InvalidateAllCntnt), INV_SIZE));
            pTmpRoot->EndAllAction();
        }
        m_rDoc.SetModified();
    }
}


void sw::DocumentSettingManager::ReplaceCompatabilityOptions(const DocumentSettingManager& rSource)
{
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

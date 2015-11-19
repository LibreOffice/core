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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTSETTINGMANAGER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTSETTINGMANAGER_HXX

#include <IDocumentSettingAccess.hxx>
class SwDoc;

namespace sw {
class DocumentSettingManager :
    public IDocumentSettingAccess
{
    rtl::Reference<SvxForbiddenCharactersTable> mxForbiddenCharsTable;
    SwDoc &m_rDoc;

    sal_uInt16  mnLinkUpdMode;       //< UpdateMode for links.

    SwFieldUpdateFlags    meFieldUpdMode;//< Automatically Update Mode for fields/charts.
    SwCharCompressType meChrCmprType;//< for ASIAN: compress punctuation/kana

    sal_uInt32  mn32DummyCompatibilityOptions1;
    sal_uInt32  mn32DummyCompatibilityOptions2;

    // COMPATIBILITY FLAGS START
    //
    //
    // HISTORY OF THE COMPATIBILITY FLAGS:
    //
    // SO5:
    // mbParaSpaceMax                        def = false, true since SO8
    // mbParaSpaceMaxAtPages                 def = false, true since SO8
    //
    // SO6:
    // mbTabCompat                           def = false, true since SO8
    //
    // SO7:
    // mbUseVirtualDevice                    def = true
    // mbAddFlyOffsets                       def = false, hidden
    //
    // SO7pp1:
    // bOldNumbering                        def = false, hidden
    //
    // SO8:
    // mbAddExternalLeading                  def = true
    // mbUseHiResolutionVirtualDevice        def = true, hidden
    // mbOldLineSpacing                      def = false
    // mbAddParaSpacingToTableCells          def = true
    // mbUseFormerObjectPos                  def = false
    // mbUseFormerTextWrapping               def = false
    // mbConsiderWrapOnObjPos                def = false
    //
    // SO8pp1:
    // mbIgnoreFirstLineIndentInNumbering    def = false, hidden
    // mbDoNotJustifyLinesWithManualBreak    def = false, hidden
    // mbDoNotResetParaAttrsForNumFont       def = false, hidden
    //
    // SO8pp3
    // mbDoNotCaptureDrawObjsOnPage         def = false, hidden
    // - Relevant for drawing objects, which don't follow the text flow, but
    //   whose position is outside the page area:
    //   false: Such drawing objects are captured on the page area of its anchor.
    //   true: Such drawing objects can leave the page area, they aren't captured.
    // mbTableRowKeep                            def = false, hidden
    // mbIgnoreTabsAndBlanksForLineCalculation   def = false, hidden
    // mbClipAsCharacterAnchoredWriterFlyFrame   def = false, hidden
    // - Introduced in order to re-activate clipping of as-character anchored
    //   Writer fly frames in method <SwFlyInContentFrame::MakeAll()> for documents,
    //   which are created with version prior SO8/OOo 2.0
    //
    // SO8pp4
    // mbUnixForceZeroExtLeading                def = false, hidden
    //
    // SO8pu8
    //
    // SO9
    // #i24363# tab stops relative to indent
    // mbTabRelativeToIndent                    def = true, hidden
    // #i89181# suppress tab stop at left indent for paragraphs in lists, whose
    // list level position and space mode equals LABEL_ALIGNMENT and whose list
    // label is followed by a tab character.
    // mbTabAtLeftIndentForParagraphsInList     def = false, hidden

    bool mbHTMLMode              : 1;    //< true: Document is in HTMLMode.
    bool mbIsGlobalDoc           : 1;    //< true: It's a global document.
    bool mbGlblDocSaveLinks      : 1;    //< true: Save sections linked in global document.
    bool mbIsLabelDoc            : 1;    //< true: It's a label document.
    bool mbPurgeOLE              : 1;    //< true: Purge OLE-Objects
    bool mbKernAsianPunctuation  : 1;    //< true: kerning also for ASIAN punctuation

    bool mbParaSpaceMax                     : 1;
    bool mbParaSpaceMaxAtPages              : 1;
    bool mbTabCompat                        : 1;
    bool mbUseVirtualDevice                 : 1;
    bool mbAddFlyOffsets                    : 1;
    bool mbAddExternalLeading               : 1;
    bool mbUseHiResolutionVirtualDevice     : 1;
    bool mbOldLineSpacing                   : 1;    // #i11859#
    bool mbAddParaSpacingToTableCells       : 1;
    bool mbUseFormerObjectPos               : 1;    // #i11860#
    bool mbUseFormerTextWrapping            : 1;
    bool mbConsiderWrapOnObjPos             : 1;    // #i28701#
                                                    // true: object positioning algorithm has consider the wrapping style of                                                    //       the floating screen objects as given by its attribute 'WrapInfluenceOnObjPos'
                                                    // floating screen objects as given by its
                                                    // attribute 'WrapInfluenceOnObjPos'.
    bool mbMathBaselineAlignment            : 1;    // TL  2010-10-29 #i972#
    bool mbStylesNoDefault                  : 1;
    bool mbFloattableNomargins              : 1; //< If paragraph margins next to a floating table should be ignored.
    bool mEmbedFonts                        : 1;  //< Whether to embed fonts used by the document when saving.
    bool mEmbedSystemFonts                  : 1;  //< Whether to embed also system fonts.

    // non-ui-compatibility flags:
    bool mbOldNumbering                             : 1;
    bool mbIgnoreFirstLineIndentInNumbering         : 1;   // #i47448#
    bool mbDoNotJustifyLinesWithManualBreak         : 1;   // #i49277#
    bool mbDoNotResetParaAttrsForNumFont            : 1;   // #i53199#
    bool mbTableRowKeep                             : 1;
    bool mbIgnoreTabsAndBlanksForLineCalculation    : 1;   // #i3952#
    bool mbDoNotCaptureDrawObjsOnPage               : 1;   // #i62875#
    bool mbOutlineLevelYieldsOutlineRule            : 1;
    bool mbClipAsCharacterAnchoredWriterFlyFrames   : 1;
    bool mbUnixForceZeroExtLeading                  : 1;   // #i60945#
    bool mbTabRelativeToIndent                      : 1;   // #i24363# tab stops relative to indent
    bool mbProtectForm                              : 1;
    bool mbInvertBorderSpacing                      : 1;
    bool mbCollapseEmptyCellPara                    : 1;
    bool mbTabAtLeftIndentForParagraphsInList;             // #i89181# - see above
    bool mbSmallCapsPercentage66;
    bool mbTabOverflow;
    bool mbUnbreakableNumberings;
    bool mbClippedPictures;
    bool mbBackgroundParaOverDrawings;
    bool mbTabOverMargin;
    bool mbSurroundTextWrapSmall;
    bool mbPropLineSpacingShrinksFirstLine; // fdo#79602
    bool mbSubtractFlys; // tdf#86578
    bool mApplyParagraphMarkFormatToNumbering;

    bool mbLastBrowseMode                           : 1;

public:

    DocumentSettingManager(SwDoc &rDoc);
    virtual ~DocumentSettingManager();

    // IDocumentSettingAccess
    virtual bool get(/*[in]*/ DocumentSettingId id) const override;
    virtual void set(/*[in]*/ DocumentSettingId id, /*[in]*/ bool value) override;
    virtual const css::i18n::ForbiddenCharacters* getForbiddenCharacters(/*[in]*/ sal_uInt16 nLang, /*[in]*/ bool bLocaleData ) const override;
    virtual void setForbiddenCharacters(/*[in]*/ sal_uInt16 nLang, /*[in]*/ const css::i18n::ForbiddenCharacters& rForbiddenCharacters ) override;
    virtual rtl::Reference<SvxForbiddenCharactersTable>& getForbiddenCharacterTable() override;
    virtual const rtl::Reference<SvxForbiddenCharactersTable>& getForbiddenCharacterTable() const override;
    virtual sal_uInt16 getLinkUpdateMode( /*[in]*/bool bGlobalSettings ) const override;
    virtual void setLinkUpdateMode( /*[in]*/ sal_uInt16 nMode ) override;
    virtual SwFieldUpdateFlags getFieldUpdateFlags( /*[in]*/bool bGlobalSettings ) const override;
    virtual void setFieldUpdateFlags( /*[in]*/ SwFieldUpdateFlags eMode ) override;
    virtual SwCharCompressType getCharacterCompressionType() const override;
    virtual void setCharacterCompressionType( /*[in]*/SwCharCompressType nType ) override;


// Replace all compatibility options with those from rSource.
    void ReplaceCompatibilityOptions(const DocumentSettingManager& rSource);

    sal_uInt32 Getn32DummyCompatibilityOptions1() const override;
    void Setn32DummyCompatibilityOptions1( const sal_uInt32 CompatibilityOptions1 ) override;
    sal_uInt32 Getn32DummyCompatibilityOptions2() const override;
    void Setn32DummyCompatibilityOptions2( const sal_uInt32 CompatibilityOptions2 ) override;

};

}

#endif  //_DOCSETTING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

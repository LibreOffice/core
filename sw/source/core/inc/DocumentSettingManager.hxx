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
#ifndef INCLUDED_SW_INC_DOCSETTING_HXX
#define INCLUDED_SW_INC_DOCSETTING_HXX

#include <IDocumentSettingAccess.hxx>
class SwDoc;

namespace sw {
class DocumentSettingManager :
    public IDocumentSettingAccess
{
    rtl::Reference<SvxForbiddenCharactersTable> mxForbiddenCharsTable;
    SwDoc &m_rDoc;

    sal_uInt16  mnLinkUpdMode;       //< UpdateMode for links.

    SwFldUpdateFlags    meFldUpdMode;//< Automatically Update Mode for fields/charts.
    SwCharCompressType meChrCmprType;//< for ASIAN: compress punctuation/kana

    bool mbHTMLMode              : 1;    //< TRUE: Document is in HTMLMode.
    bool mbIsGlobalDoc           : 1;    //< TRUE: It's a global document.
    bool mbGlblDocSaveLinks      : 1;    //< TRUE: Save sections linked in global document.
    bool mbIsLabelDoc            : 1;    //< TRUE: It's a label document.
    bool mbPurgeOLE              : 1;    //< sal_True: Purge OLE-Objects
    bool mbKernAsianPunctuation  : 1;    //< sal_True: kerning also for ASIAN punctuation

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
                                                    // sal_True: object positioning algorithm has consider the wrapping style of                                                    //       the floating screen objects as given by its attribute 'WrapInfluenceOnObjPos'
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

    bool mbLastBrowseMode                           : 1;

public:

    DocumentSettingManager(SwDoc &rDoc);
    virtual ~DocumentSettingManager();

    // IDocumentSettingAccess
    virtual bool get(/*[in]*/ DocumentSettingId id) const SAL_OVERRIDE;
    virtual void set(/*[in]*/ DocumentSettingId id, /*[in]*/ bool value) SAL_OVERRIDE;
    virtual const com::sun::star::i18n::ForbiddenCharacters* getForbiddenCharacters(/*[in]*/ sal_uInt16 nLang, /*[in]*/ bool bLocaleData ) const SAL_OVERRIDE;
    virtual void setForbiddenCharacters(/*[in]*/ sal_uInt16 nLang, /*[in]*/ const com::sun::star::i18n::ForbiddenCharacters& rForbiddenCharacters ) SAL_OVERRIDE;
    virtual rtl::Reference<SvxForbiddenCharactersTable>& getForbiddenCharacterTable() SAL_OVERRIDE;
    virtual const rtl::Reference<SvxForbiddenCharactersTable>& getForbiddenCharacterTable() const SAL_OVERRIDE;
    virtual sal_uInt16 getLinkUpdateMode( /*[in]*/bool bGlobalSettings ) const SAL_OVERRIDE;
    virtual void setLinkUpdateMode( /*[in]*/ sal_uInt16 nMode ) SAL_OVERRIDE;
    virtual SwFldUpdateFlags getFieldUpdateFlags( /*[in]*/bool bGlobalSettings ) const SAL_OVERRIDE;
    virtual void setFieldUpdateFlags( /*[in]*/ SwFldUpdateFlags eMode ) SAL_OVERRIDE;
    virtual SwCharCompressType getCharacterCompressionType() const SAL_OVERRIDE;
    virtual void setCharacterCompressionType( /*[in]*/SwCharCompressType nType ) SAL_OVERRIDE;


// Replace all compatability options with those from rSource.
    void ReplaceCompatabilityOptions(const DocumentSettingManager& rSource);

};

}

#endif  //_DOCSETTING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

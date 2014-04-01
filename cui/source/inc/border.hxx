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
#ifndef INCLUDED_CUI_SOURCE_INC_BORDER_HXX
#define INCLUDED_CUI_SOURCE_INC_BORDER_HXX

#include <svtools/ctrlbox.hxx>
#include <vcl/group.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <svtools/valueset.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/frmsel.hxx>
#include <svx/flagsdef.hxx>

#include <set>

// forward ---------------------------------------------------------------

namespace editeng
{
    class SvxBorderLine;
}


class SvxBorderTabPage : public SfxTabPage
{
    using TabPage::DeactivatePage;

public:
    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);
    static sal_uInt16*      GetRanges();

    virtual bool        FillItemSet( SfxItemSet& rCoreAttrs ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet& ) SAL_OVERRIDE;

    void                HideShadowControls();
    virtual void        PageCreated (SfxAllItemSet aSet) SAL_OVERRIDE;
protected:
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 ) SAL_OVERRIDE;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

private:
    SvxBorderTabPage( Window* pParent, const SfxItemSet& rCoreAttrs );
    virtual ~SvxBorderTabPage();

    // Controls
    ValueSet*           m_pWndPresets;
    FixedText*          m_pUserDefFT;
    svx::FrameSelector* m_pFrameSel;

    LineListBox*        m_pLbLineStyle;
    ColorListBox*       m_pLbLineColor;
    MetricField*        m_pLineWidthMF;

    VclContainer*       m_pSpacingFrame;
    FixedText*          m_pLeftFT;
    MetricField*        m_pLeftMF;
    FixedText*          m_pRightFT;
    MetricField*        m_pRightMF;
    FixedText*          m_pTopFT;
    MetricField*        m_pTopMF;
    FixedText*          m_pBottomFT;
    MetricField*        m_pBottomMF;
    CheckBox*           m_pSynchronizeCB;

    VclContainer*       m_pShadowFrame;
    ValueSet*           m_pWndShadows;
    FixedText*          m_pFtShadowSize;
    MetricField*        m_pEdShadowSize;
    FixedText*          m_pFtShadowColor;
    ColorListBox*       m_pLbShadowColor;


    VclContainer*       m_pPropertiesFrame;///< properties - "Merge with next paragraph" in Writer
    CheckBox*           m_pMergeWithNextCB;
    // #i29550#
    CheckBox*           m_pMergeAdjacentBordersCB;

    ImageList           aShadowImgLstH;
    ImageList           aShadowImgLst;
    ImageList           aBorderImgLstH;
    ImageList           aBorderImgLst;

    long                nMinValue;  ///< minimum distance
    int                 nSWMode;    ///< table, textframe, paragraph

    bool                mbHorEnabled;       ///< true = Inner horizontal border enabled.
    bool                mbVerEnabled;       ///< true = Inner vertical border enabled.
    bool                mbTLBREnabled;      ///< true = Top-left to bottom-right border enabled.
    bool                mbBLTREnabled;      ///< true = Bottom-left to top-right border enabled.
    bool                mbUseMarginItem;
    bool                mbSync;

    std::set<sal_Int16> maUsedBorderStyles;

    // Handler
    DECL_LINK( SelStyleHdl_Impl, ListBox* pLb );
    DECL_LINK( SelColHdl_Impl, ListBox* pLb );
    DECL_LINK( SelPreHdl_Impl, void* );
    DECL_LINK( SelSdwHdl_Impl, void* );
    DECL_LINK( LinesChanged_Impl, void* );
    DECL_LINK( ModifyDistanceHdl_Impl, MetricField*);
    DECL_LINK( ModifyWidthHdl_Impl, void*);
    DECL_LINK( SyncHdl_Impl, CheckBox*);

    sal_uInt16              GetPresetImageId( sal_uInt16 nValueSetIdx ) const;
    sal_uInt16              GetPresetStringId( sal_uInt16 nValueSetIdx ) const;

    void                FillPresetVS();
    void                FillShadowVS();
    void                FillValueSets();

    // Filler
    void                FillLineListBox_Impl();

    /// share for individual Frame-/Core-Line
    void                ResetFrameLine_Impl( svx::FrameBorderType eBorder,
                                             const editeng::SvxBorderLine* pCurLine,
                                             bool bValid );

    bool IsBorderLineStyleAllowed( sal_Int16 nStyle ) const;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

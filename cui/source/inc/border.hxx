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
    friend class VclPtr<SvxBorderTabPage>;
    using TabPage::DeactivatePage;

    static const sal_uInt16 pRanges[];

public:
    virtual ~SvxBorderTabPage();
    virtual void dispose() SAL_OVERRIDE;
    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent,
                                const SfxItemSet* rAttrSet);
    static const sal_uInt16*      GetRanges() { return pRanges; }

    virtual bool        FillItemSet( SfxItemSet* rCoreAttrs ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet* ) SAL_OVERRIDE;
    virtual void        ChangesApplied() SAL_OVERRIDE;

    void                HideShadowControls();
    virtual void        PageCreated(const SfxAllItemSet& aSet) SAL_OVERRIDE;
    void                SetTableMode();
protected:
    virtual sfxpg        DeactivatePage( SfxItemSet* pSet = 0 ) SAL_OVERRIDE;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

private:
    SvxBorderTabPage( vcl::Window* pParent, const SfxItemSet& rCoreAttrs );

    // Controls
    VclPtr<ValueSet>           m_pWndPresets;
    VclPtr<FixedText>          m_pUserDefFT;
    VclPtr<svx::FrameSelector> m_pFrameSel;

    VclPtr<LineListBox>        m_pLbLineStyle;
    VclPtr<ColorListBox>       m_pLbLineColor;
    VclPtr<MetricField>        m_pLineWidthMF;

    VclPtr<VclContainer>       m_pSpacingFrame;
    VclPtr<FixedText>          m_pLeftFT;
    VclPtr<MetricField>        m_pLeftMF;
    VclPtr<FixedText>          m_pRightFT;
    VclPtr<MetricField>        m_pRightMF;
    VclPtr<FixedText>          m_pTopFT;
    VclPtr<MetricField>        m_pTopMF;
    VclPtr<FixedText>          m_pBottomFT;
    VclPtr<MetricField>        m_pBottomMF;
    VclPtr<CheckBox>           m_pSynchronizeCB;

    VclPtr<VclContainer>       m_pShadowFrame;
    VclPtr<ValueSet>           m_pWndShadows;
    VclPtr<FixedText>          m_pFtShadowSize;
    VclPtr<MetricField>        m_pEdShadowSize;
    VclPtr<FixedText>          m_pFtShadowColor;
    VclPtr<ColorListBox>       m_pLbShadowColor;


    VclPtr<VclContainer>       m_pPropertiesFrame;///< properties - "Merge with next paragraph" in Writer
    VclPtr<CheckBox>           m_pMergeWithNextCB;
    // #i29550#
    VclPtr<CheckBox>           m_pMergeAdjacentBordersCB;

    ImageList           aShadowImgLstH;
    ImageList           aShadowImgLst;
    ImageList           aBorderImgLstH;
    ImageList           aBorderImgLst;

    long                nMinValue;  ///< minimum distance
    SwBorderModes       nSWMode;    ///< table, textframe, paragraph

    bool                mbHorEnabled;       ///< true = Inner horizontal border enabled.
    bool                mbVerEnabled;       ///< true = Inner vertical border enabled.
    bool                mbTLBREnabled;      ///< true = Top-left to bottom-right border enabled.
    bool                mbBLTREnabled;      ///< true = Bottom-left to top-right border enabled.
    bool                mbUseMarginItem;
    bool                mbSync;

    std::set<sal_Int16> maUsedBorderStyles;

    // Handler
    DECL_LINK( SelStyleHdl_Impl, ListBox* );
    DECL_LINK( SelColHdl_Impl, ListBox* );
    DECL_LINK_TYPED( SelPreHdl_Impl, ValueSet*, void );
    DECL_LINK_TYPED( SelSdwHdl_Impl, ValueSet*, void );
    DECL_LINK( LinesChanged_Impl, void* );
    DECL_LINK( ModifyDistanceHdl_Impl, MetricField*);
    DECL_LINK( ModifyWidthHdl_Impl, void*);
    DECL_LINK_TYPED( SyncHdl_Impl, Button*, void);

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

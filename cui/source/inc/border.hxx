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
#ifndef _SVX_BORDER_HXX
#define _SVX_BORDER_HXX

#include <svtools/ctrlbox.hxx>
#include <vcl/group.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <svtools/valueset.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/frmsel.hxx>

// forward ---------------------------------------------------------------

namespace editeng
{
    class SvxBorderLine;
}

#include <svx/flagsdef.hxx>

class SvxBorderTabPage : public SfxTabPage
{
    using TabPage::DeactivatePage;

public:
    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);
    static sal_uInt16*      GetRanges();

    virtual sal_Bool        FillItemSet( SfxItemSet& rCoreAttrs );
    virtual void        Reset( const SfxItemSet& );

    void                HideShadowControls();
    virtual void        PageCreated (SfxAllItemSet aSet);
protected:
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

private:
    SvxBorderTabPage( Window* pParent, const SfxItemSet& rCoreAttrs );
    ~SvxBorderTabPage();

    // Controls
    FixedLine           aFlBorder;
    FixedText           aDefaultFT;
    ValueSet            aWndPresets;
    FixedText           aUserDefFT;
    svx::FrameSelector  aFrameSel;

    FixedLine           aFlSep1;
    FixedLine           aFlLine;
    FixedText           aStyleFT;
    LineListBox    aLbLineStyle;
    FixedText           aColorFT;
    ColorListBox        aLbLineColor;
    FixedText           aWidthFT;
    MetricField         aLineWidthMF;

    FixedLine           aFlSep2;
    FixedLine           aDistanceFL;
    FixedText           aLeftFT;
    MetricField         aLeftMF;
    FixedText           aRightFT;
    MetricField         aRightMF;
    FixedText           aTopFT;
    MetricField         aTopMF;
    FixedText           aBottomFT;
    MetricField         aBottomMF;
    CheckBox            aSynchronizeCB;

    FixedLine           aFlShadow;
    FixedText           aFtShadowPos;
    ValueSet            aWndShadows;
    FixedText           aFtShadowSize;
    MetricField         aEdShadowSize;
    FixedText           aFtShadowColor;
    ColorListBox        aLbShadowColor;


    FixedLine           aPropertiesFL;///< properties - "Merge with next paragraph" in Writer
    CheckBox            aMergeWithNextCB;
    // #i29550#
    CheckBox            aMergeAdjacentBordersCB;

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

#ifdef _SVX_BORDER_CXX
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
#endif
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

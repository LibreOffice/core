/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVX_BORDER_HXX
#define _SVX_BORDER_HXX

// include ---------------------------------------------------------------


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

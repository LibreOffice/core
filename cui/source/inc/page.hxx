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
#ifndef INCLUDED_CUI_SOURCE_INC_PAGE_HXX
#define INCLUDED_CUI_SOURCE_INC_PAGE_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/stdctrl.hxx>
#include <svx/pagectrl.hxx>
#include <svx/papersizelistbox.hxx>
#include <svx/frmdirlbox.hxx>
#include <editeng/svxenum.hxx>
#include <i18nutil/paper.hxx>
#include <svx/flagsdef.hxx>

// class SvxPageDescPage -------------------------------------------------
/*
    [Description]
    TabPage for page settings (size, margins, ...)

    [Items]
    <SvxPageItem>:          <SID_ATTR_PAGE>
    <SvxSizeItem>:          <SID_ATTR_SIZE>
    <SvxSizeItem>:          <SID_ATTR_MAXSIZE>
    <SvxULSpaceItem>:       <SID_ATTR_LRSPACE>
    <SvxLRSpaceItem>:       <SID_ATTR_ULSPACE>
    <SfxAllEnumItem>:       <SID_ATTR_PAPERTRAY>
    <SvxPaperBinItem>:      <SID_ATTR_PAPERBIN>
    <SvxBoolItem>:          <SID_ATTR_EXT1>
    <SvxBoolItem>:          <SID_ATTR_EXT2>

    <SfxSetItem>:           <SID_ATTR_HEADERSET>
        <SfxBoolItem>:          <SID_ATTR_ON>
        <SfxBoolItem>:          <SID_ATTR_DYNAMIC>
        <SfxBoolItem>:          <SID_ATTR_SHARED>
        <SvxSizeItem>:          <SID_ATTR_SIZE>
        <SvxULSpaceItem>:       <SID_ATTR_ULSPACE>
        <SvxLRSpaceItem>:       <SID_ATTR_LRSPACE>

    <SfxSetItem>:           <SID_ATTR_FOOTERSET>
        <SfxBoolItem>:          <SID_ATTR_ON>
        <SfxBoolItem>:          <SID_ATTR_DYNAMIC>
        <SfxBoolItem>:          <SID_ATTR_SHARED>
        <SvxSizeItem>:          <SID_ATTR_SIZE>
        <SvxULSpaceItem>:       <SID_ATTR_ULSPACE>
        <SvxLRSpaceItem>:       <SID_ATTR_LRSPACE>
*/

//UUUU struct   SvxPage_Impl;
typedef sal_uInt16 MarginPosition;

class SvxPageDescPage : public SfxTabPage
{
    friend class VclPtr<SvxPageDescPage>;
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    static const sal_uInt16 pRanges[];
private:
    // paper format
    VclPtr<PaperSizeListBox>    m_pPaperSizeBox;

    VclPtr<MetricField>         m_pPaperWidthEdit;
    VclPtr<MetricField>         m_pPaperHeightEdit;

    VclPtr<FixedText>           m_pOrientationFT;
    VclPtr<RadioButton>         m_pPortraitBtn;
    VclPtr<RadioButton>         m_pLandscapeBtn;

    VclPtr<SvxPageWindow>       m_pBspWin;

    VclPtr<FixedText>           m_pTextFlowLbl;
    VclPtr<svx::FrameDirectionListBox>  m_pTextFlowBox;

    VclPtr<ListBox>             m_pPaperTrayBox;

    // Margins
    VclPtr<FixedText>           m_pLeftMarginLbl;
    VclPtr<MetricField>         m_pLeftMarginEdit;
    VclPtr<FixedText>           m_pRightMarginLbl;
    VclPtr<MetricField>         m_pRightMarginEdit;
    VclPtr<MetricField>         m_pTopMarginEdit;
    VclPtr<MetricField>         m_pBottomMarginEdit;

    // layout settings
    VclPtr<FixedText>           m_pPageText;
    VclPtr<ListBox>             m_pLayoutBox;
    VclPtr<ListBox>             m_pNumberFormatBox;

    //Extras Calc
    VclPtr<FixedText>           m_pTblAlignFT;
    VclPtr<CheckBox>            m_pHorzBox;
    VclPtr<CheckBox>            m_pVertBox;

    // Impress and Draw
    VclPtr<CheckBox>            m_pAdaptBox;

    //Register Writer
    VclPtr<CheckBox>            m_pRegisterCB;
    VclPtr<FixedText>           m_pRegisterFT;
    VclPtr<ListBox>             m_pRegisterLB;

    OUString             sStandardRegister;

    VclPtr<FixedText>           m_pInsideLbl;
    VclPtr<FixedText>           m_pOutsideLbl;
    VclPtr<FixedText>           m_pPrintRangeQueryText;

    long                nFirstLeftMargin;
    long                nFirstRightMargin;
    long                nFirstTopMargin;
    long                nFirstBottomMargin;
    long                nLastLeftMargin;
    long                nLastRightMargin;
    long                nLastTopMargin;
    long                nLastBottomMargin;

    bool                bLandscape;
    bool                bBorderModified;
    SvxModeType         eMode;
    Paper               ePaperStart;
    Paper               ePaperEnd;

    //UUUU SvxPage_Impl*        pImpl;
    MarginPosition      m_nPos;
    VclPtr<Printer>     mpDefPrinter;

    bool                mbDelPrinter : 1;

    //UUUU
    bool mbEnableDrawingLayerFillStyles : 1;

    void                Init_Impl();
    DECL_LINK_TYPED(    LayoutHdl_Impl, ListBox&, void);
    DECL_LINK_TYPED(    PaperBinHdl_Impl, Control&, void);
    DECL_LINK_TYPED(    SwapOrientation_Impl, Button*, void );
    void                SwapFirstValues_Impl( bool bSet );
    DECL_LINK_TYPED(    BorderModify_Impl, Edit&, void);
    void                InitHeadFoot_Impl( const SfxItemSet& rSet );
    DECL_LINK_TYPED(    CenterHdl_Impl, Button*, void);
    void                UpdateExample_Impl( bool bResetbackground = false );

    DECL_LINK_TYPED(    PaperSizeSelect_Impl, ListBox&, void );
    DECL_LINK_TYPED(    PaperSizeModify_Impl, Edit&, void);

    DECL_LINK_TYPED(    FrameDirectionModify_Impl, ListBox&, void );

    void                ResetBackground_Impl( const SfxItemSet& rSet );

    DECL_LINK_TYPED(    RangeHdl_Impl, Control&, void );
    void                CalcMargin_Impl();

    DECL_LINK_TYPED(    RegisterModify, Button*, void );

    // page direction
    /** Disables vertical page direction entries in the text flow listbox. */
    void                DisableVerticalPageDir();

    bool                IsPrinterRangeOverflow( MetricField& rField, long nFirstMargin,
                                                long nLastMargin, MarginPosition nPos );
    void                CheckMarginEdits( bool _bClear );
    bool                IsMarginOutOfRange();

    SvxPageDescPage( vcl::Window* pParent, const SfxItemSet& rSet );

    //UUUU
    void EnableDrawingLayerFillStyles(bool bNew) { mbEnableDrawingLayerFillStyles = bNew; }

protected:
    virtual void        ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

public:
    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rSet );
    // returns the range of the Which values
    static const sal_uInt16* GetRanges() { return pRanges; }

    virtual bool        FillItemSet( SfxItemSet* rOutSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        FillUserData() override;

    virtual ~SvxPageDescPage();
    virtual void        dispose() override;

    void                SetMode( SvxModeType eMType ) { eMode = eMType; }
    void                SetPaperFormatRanges( Paper eStart, Paper eEnd )
                            { ePaperStart = eStart; ePaperEnd = eEnd; }

    void                SetCollectionList(const std::vector<OUString> &aList);
    virtual void        PageCreated(const SfxAllItemSet& aSet) override;
};

#endif // INCLUDED_CUI_SOURCE_INC_PAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

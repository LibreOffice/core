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
#include <svx/pagectrl.hxx>
#include <svx/pagenumberlistbox.hxx>
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

typedef sal_uInt16 MarginPosition;

class SvxPageDescPage : public SfxTabPage
{
    friend class VclPtr<SvxPageDescPage>;
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    static const sal_uInt16 pRanges[];
private:
    OUString            sStandardRegister;
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

    MarginPosition      m_nPos;
    VclPtr<Printer>     mpDefPrinter;

    bool                mbDelPrinter : 1;
    bool                mbEnableDrawingLayerFillStyles : 1;

    SvxPageWindow m_aBspWin;

    // paper format
    std::unique_ptr<SvxPaperSizeListBox> m_xPaperSizeBox;
    std::unique_ptr<weld::MetricSpinButton> m_xPaperWidthEdit;
    std::unique_ptr<weld::MetricSpinButton> m_xPaperHeightEdit;
    std::unique_ptr<weld::Label> m_xOrientationFT;
    std::unique_ptr<weld::RadioButton> m_xPortraitBtn;
    std::unique_ptr<weld::RadioButton> m_xLandscapeBtn;
    std::unique_ptr<weld::Label> m_xTextFlowLbl;
    std::unique_ptr<svx::FrameDirectionListBox>  m_xTextFlowBox;
    std::unique_ptr<weld::ComboBox> m_xPaperTrayBox;
    // Margins
    std::unique_ptr<weld::Label> m_xLeftMarginLbl;
    std::unique_ptr<weld::MetricSpinButton> m_xLeftMarginEdit;
    std::unique_ptr<weld::Label> m_xRightMarginLbl;
    std::unique_ptr<weld::MetricSpinButton> m_xRightMarginEdit;
    std::unique_ptr<weld::MetricSpinButton> m_xTopMarginEdit;
    std::unique_ptr<weld::MetricSpinButton> m_xBottomMarginEdit;
    // layout settings
    std::unique_ptr<weld::Label> m_xPageText;
    std::unique_ptr<weld::ComboBox> m_xLayoutBox;
    std::unique_ptr<SvxPageNumberListBox> m_xNumberFormatBox;
    //Extras Calc
    std::unique_ptr<weld::Label> m_xTblAlignFT;
    std::unique_ptr<weld::CheckButton> m_xHorzBox;
    std::unique_ptr<weld::CheckButton> m_xVertBox;
    // Impress and Draw
    std::unique_ptr<weld::CheckButton> m_xAdaptBox;
    //Register Writer
    std::unique_ptr<weld::CheckButton> m_xRegisterCB;
    std::unique_ptr<weld::Label> m_xRegisterFT;
    std::unique_ptr<weld::ComboBox> m_xRegisterLB;
    std::unique_ptr<weld::Label> m_xInsideLbl;
    std::unique_ptr<weld::Label> m_xOutsideLbl;
    std::unique_ptr<weld::Label> m_xPrintRangeQueryText;
    std::unique_ptr<weld::CustomWeld> m_xBspWin;

    void                Init_Impl();
    DECL_LINK(LayoutHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(PaperBinHdl_Impl, weld::Widget&, void);
    DECL_LINK(SwapOrientation_Impl, weld::Button&, void);
    void SwapFirstValues_Impl( bool bSet );
    DECL_LINK(BorderModify_Impl, weld::MetricSpinButton&, void);
    void InitHeadFoot_Impl( const SfxItemSet& rSet );
    DECL_LINK(CenterHdl_Impl, weld::ToggleButton&, void);
    void UpdateExample_Impl( bool bResetbackground = false );

    DECL_LINK(PaperSizeSelect_Impl, weld::ComboBox&, void );
    DECL_LINK(PaperSizeModify_Impl, weld::MetricSpinButton&, void);

    DECL_LINK(FrameDirectionModify_Impl, weld::ComboBox&, void );

    void ResetBackground_Impl( const SfxItemSet& rSet );

    void RangeHdl_Impl();
    void CalcMargin_Impl();

    DECL_LINK(RegisterModify, weld::ToggleButton&, void);

    // page direction
    /** Disables vertical page direction entries in the text flow listbox. */
    void                DisableVerticalPageDir();

    bool                IsPrinterRangeOverflow(weld::MetricSpinButton& rField, long nFirstMargin,
                                               long nLastMargin, MarginPosition nPos);
    void                CheckMarginEdits( bool _bClear );
    bool                IsMarginOutOfRange();

    SvxPageDescPage(TabPageParent pParent, const SfxItemSet& rSet);

protected:
    virtual void        ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

public:
    static VclPtr<SfxTabPage>  Create( TabPageParent pParent, const SfxItemSet* rSet );
    // returns the range of the Which values
    static const sal_uInt16* GetRanges() { return pRanges; }

    virtual bool        FillItemSet( SfxItemSet* rOutSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        FillUserData() override;

    virtual ~SvxPageDescPage() override;
    virtual void        dispose() override;

    void                SetPaperFormatRanges( Paper eStart )
                            { ePaperStart = eStart; }

    void                SetCollectionList(const std::vector<OUString> &aList);
    virtual void        PageCreated(const SfxAllItemSet& aSet) override;
};

#endif // INCLUDED_CUI_SOURCE_INC_PAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#ifndef INCLUDED_CUI_SOURCE_INC_NUMFMT_HXX
#define INCLUDED_CUI_SOURCE_INC_NUMFMT_HXX


#include <rtl/ustring.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/langbox.hxx>
#include <tools/color.hxx>
#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>

class SvxNumberFormatShell;
class SvxNumberInfoItem;
class vector;


class SvxNumberPreview : public weld::CustomWidgetController
{
private:
    OUString        aPrevStr;
    Color           aPrevCol;
    sal_Int32       mnPos;
    sal_Unicode     mnChar;

protected:
    virtual void    Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect) override;

public:
    SvxNumberPreview();

    void            NotifyChange( const OUString& rPrevStr, const Color* pColor = nullptr );

    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override
    {
        CustomWidgetController::SetDrawingArea(pDrawingArea);
        pDrawingArea->set_size_request(-1, pDrawingArea->get_text_height() * 3);
    }
};

class SvxNumberFormatTabPage : public SfxTabPage
{
    static const sal_uInt16 pRanges[];

public:
    SvxNumberFormatTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreAttrs);
    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController,
                                    const SfxItemSet* rAttrSet );
    virtual ~SvxNumberFormatTabPage() override;
    // Returns area information.
    static const sal_uInt16* GetRanges() { return pRanges; }

    virtual bool            FillItemSet( SfxItemSet* rSet ) override;
    virtual void            Reset( const SfxItemSet* rSet ) override;
    virtual DeactivateRC    DeactivatePage  ( SfxItemSet* pSet ) override;

    void                    HideLanguage(bool bFlag=true);
    virtual void            PageCreated(const SfxAllItemSet& aSet) override;

private:
    std::unique_ptr<SvxNumberInfoItem>    pNumItem;
    std::unique_ptr<SvxNumberFormatShell> pNumFmtShell;
    sal_uLong               nInitFormat;

    bool                bNumItemFlag; ///< for handling with DocShell
    bool                bOneAreaFlag;
    bool                bLegacyAutomaticCurrency;
    short                   nFixedCategory;

    OUString sAutomaticLangEntry;
    OUString sAutomaticCurrencyEntry;

    SvxNumberPreview m_aWndPreview;
    std::unique_ptr<weld::Label> m_xFtCategory;
    std::unique_ptr<weld::TreeView> m_xLbCategory;
    std::unique_ptr<weld::Label> m_xFtFormat;
    std::unique_ptr<weld::ComboBox> m_xLbCurrency;
    std::unique_ptr<weld::TreeView> m_xLbFormat;
    std::unique_ptr<weld::Label> m_xFtLanguage;
    std::unique_ptr<weld::CheckButton> m_xCbSourceFormat;
    std::unique_ptr<weld::Label> m_xFtOptions;
    std::unique_ptr<weld::Label> m_xFtDecimals;
    std::unique_ptr<weld::SpinButton> m_xEdDecimals;
    std::unique_ptr<weld::Label> m_xFtDenominator;
    std::unique_ptr<weld::SpinButton> m_xEdDenominator;
    std::unique_ptr<weld::CheckButton> m_xBtnNegRed;
    std::unique_ptr<weld::Label> m_xFtLeadZeroes;
    std::unique_ptr<weld::SpinButton> m_xEdLeadZeroes;
    std::unique_ptr<weld::CheckButton> m_xBtnThousand;
    std::unique_ptr<weld::CheckButton> m_xBtnEngineering;
    std::unique_ptr<weld::Widget> m_xFormatCodeFrame;
    std::unique_ptr<weld::Entry> m_xEdFormat;
    std::unique_ptr<weld::Button> m_xIbAdd;
    std::unique_ptr<weld::Button> m_xIbInfo;
    std::unique_ptr<weld::Button> m_xIbRemove;
    std::unique_ptr<weld::Label> m_xFtComment;
    std::unique_ptr<weld::Entry> m_xEdComment;
    std::unique_ptr<SvxLanguageBox> m_xLbLanguage;
    std::unique_ptr<weld::CustomWeld> m_xWndPreview;

    void    Init_Impl();
    void    FillCurrencyBox();
    void    FillFormatListBox_Impl( std::vector<OUString>& rEntries );
    void    UpdateOptions_Impl( bool bCheckCatChange );
    void    UpdateFormatListBox_Impl( bool bCat, bool bUpdateEdit );
    void    UpdateThousandEngineeringCheckBox();
    void    UpdateDecimalsDenominatorEditBox();
    void    Obstructing();
    void    EnableBySourceFormat_Impl();
    void    SetCategory( sal_uInt16 nPos );
    OUString  GetExpColorString( const Color*& rpPreviewColor, const OUString& aFormatStr, short nTmpCatPos );
    void    MakePreviewText( const OUString& rFormat );
    void    ChangePreviewText( sal_uInt16 nPos );
    void    AddAutomaticLanguage_Impl(LanguageType eAutoLang, bool bSelect);
    bool    Click_Impl(weld::Button& rIB);
    // Handler
    DECL_LINK(LostFocusHdl_Impl, weld::Widget&, void);
    DECL_LINK(DoubleClickHdl_Impl, weld::TreeView&, bool);
    DECL_LINK(SelFormatListBoxHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(SelFormatTreeListBoxHdl_Impl, weld::TreeView&, void);
    DECL_LINK(SelFormatClickHdl_Impl, weld::Button&, void);
    void SelFormatHdl_Impl(weld::Widget*);
    DECL_LINK(ClickHdl_Impl, weld::Button&, void);
    DECL_LINK(EditModifyHdl_Impl, weld::Entry&, void);
    DECL_LINK(OptEditHdl_Impl, weld::SpinButton&, void);
    DECL_LINK(OptClickHdl_Impl, weld::Button&, void);
    void EditHdl_Impl(const weld::Entry*);
    void OptHdl_Impl(const weld::Widget*);

    // set and get currency, taking into account if the legacy
    // automatic currency entry exists
    void set_active_currency(sal_Int32 nCurCurrencyEntryPos);
    sal_uInt32 get_active_currency() const;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

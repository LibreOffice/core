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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_OPTLOAD_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_OPTLOAD_HXX

#include <tools/globname.hxx>
#include <sfx2/tabdlg.hxx>

#include <vcl/customweld.hxx>
#include <vcl/textfilter.hxx>
#include <vcl/weld.hxx>
#include <sfx2/basedlgs.hxx>

#include <SwCapObjType.hxx>

class SwFieldMgr;
class SwWrtShell;

class SwFieldUnitTable
{
public:
    static OUString GetString(sal_uInt32 i);
    static sal_uInt32 Count();
    static FieldUnit GetValue(sal_uInt32 i);
};

class TextFilterAutoConvert : public TextFilter
{
private:
    OUString m_sLastGoodText;
    OUString m_sNone;
public:
    TextFilterAutoConvert(const OUString &rNone)
        : m_sNone(rNone)
    {
    }
    virtual OUString filter(const OUString &rText) override;
};

class SwLoadOptPage : public SfxTabPage
{
private:
    SwWrtShell*      m_pWrtShell;
    sal_uInt16       m_nLastTab;
    sal_Int32        m_nOldLinkMode;

    std::unique_ptr<weld::RadioButton> m_xAlwaysRB;
    std::unique_ptr<weld::RadioButton> m_xRequestRB;
    std::unique_ptr<weld::RadioButton> m_xNeverRB;

    std::unique_ptr<weld::CheckButton> m_xAutoUpdateFields;
    std::unique_ptr<weld::CheckButton> m_xAutoUpdateCharts;

    std::unique_ptr<weld::ComboBox> m_xMetricLB;
    std::unique_ptr<weld::Label> m_xTabFT;
    std::unique_ptr<weld::MetricSpinButton> m_xTabMF;
    std::unique_ptr<weld::CheckButton> m_xUseSquaredPageMode;
    std::unique_ptr<weld::CheckButton> m_xUseCharUnit;
    std::unique_ptr<weld::Entry> m_xWordCountED;
    std::unique_ptr<weld::CheckButton> m_xShowStandardizedPageCount;
    std::unique_ptr<weld::SpinButton> m_xStandardizedPageSizeNF;

    DECL_LINK(MetricHdl, weld::ComboBox&, void);
    DECL_LINK(StandardizedPageCountCheckHdl, weld::Button&, void);

public:
    SwLoadOptPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~SwLoadOptPage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController,
                                      const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

class SwCaptionOptDlg : public SfxSingleTabDialogController
{
public:
    SwCaptionOptDlg(weld::Window* pParent, const SfxItemSet& rSet);
};

class SwCaptionPreview : public weld::CustomWidgetController
{
private:
    OUString maText;
    bool mbFontInitialized;
    vcl::Font maFont;

    void ApplySettings(vcl::RenderContext& rRenderContext);

public:
    SwCaptionPreview();
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    void SetPreviewText(const OUString& rText);
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
};

class SwCaptionOptPage : public SfxTabPage
{
private:
    OUString m_sSWTable;
    OUString m_sSWFrame;
    OUString m_sSWGraphic;
    OUString m_sOLE;

    OUString m_sIllustration;
    OUString m_sTable;
    OUString m_sText;
    OUString m_sDrawing;

    OUString m_sBegin;
    OUString m_sEnd;
    OUString m_sAbove;
    OUString m_sBelow;

    OUString m_sNone;

    int m_nPrevSelectedEntry;

    std::unique_ptr<SwFieldMgr> pMgr;
    bool bHTMLMode;

    TextFilterAutoConvert m_aTextFilter;

    SwCaptionPreview m_aPreview;
    std::unique_ptr<weld::TreeView> m_xCheckLB;
    std::unique_ptr<weld::ComboBox> m_xLbCaptionOrder;

    std::unique_ptr<weld::Widget> m_xSettingsGroup;
    std::unique_ptr<weld::ComboBox> m_xCategoryBox;
    std::unique_ptr<weld::Label> m_xFormatText;
    std::unique_ptr<weld::ComboBox> m_xFormatBox;
    //#i61007# order of captions
    std::unique_ptr<weld::Label> m_xNumberingSeparatorFT;
    std::unique_ptr<weld::Entry> m_xNumberingSeparatorED;
    std::unique_ptr<weld::Label> m_xTextText;
    std::unique_ptr<weld::Entry> m_xTextEdit;
    std::unique_ptr<weld::ComboBox> m_xPosBox;

    std::unique_ptr<weld::Widget> m_xNumCapt;
    std::unique_ptr<weld::ComboBox> m_xLbLevel;
    std::unique_ptr<weld::Entry> m_xEdDelim;

    std::unique_ptr<weld::Widget> m_xCategory;
    std::unique_ptr<weld::ComboBox> m_xCharStyleLB;
    std::unique_ptr<weld::CheckButton> m_xApplyBorderCB;
    std::unique_ptr<weld::CustomWeld> m_xPreview;

    DECL_LINK(SelectHdl, weld::ComboBox&, void);
    DECL_LINK(SelectListBoxHdl, weld::ComboBox&, void);
    DECL_LINK(ModifyEntryHdl, weld::Entry&, void);
    DECL_LINK(ModifyComboHdl, weld::ComboBox&, void);
    DECL_LINK(OrderHdl, weld::ComboBox&, void );
    DECL_LINK(ShowEntryHdl, weld::TreeView&, void);
    DECL_LINK(ToggleEntryHdl, const weld::TreeView::iter_col&, void);
    DECL_LINK(TextFilterHdl, OUString&, bool);

    void ModifyHdl();
    void UpdateEntry(int nRow);
    void DelUserData();
    void SetOptions(const sal_uLong nPos, const SwCapObjType eType, const SvGlobalName *pOleId = nullptr);
    void SaveEntry(int nEntry);
    void InvalidatePreview();

public:
    SwCaptionOptPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~SwCaptionOptPage() override;

    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController,
                                     const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

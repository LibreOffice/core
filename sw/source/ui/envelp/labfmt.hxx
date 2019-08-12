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
#ifndef INCLUDED_SW_SOURCE_UI_ENVELP_LABFMT_HXX
#define INCLUDED_SW_SOURCE_UI_ENVELP_LABFMT_HXX

#include <labimg.hxx>
#include <label.hxx>

#include <sfx2/tabdlg.hxx>
#include <vcl/idle.hxx>
#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>

class SwLabFormatPage;

class SwLabPreview : public weld::CustomWidgetController
{
    Color const m_aGrayColor;

    OUString const m_aHDistStr;
    OUString const m_aVDistStr;
    OUString const m_aWidthStr;
    OUString const m_aHeightStr;
    OUString const m_aLeftStr;
    OUString const m_aUpperStr;
    OUString const m_aColsStr;
    OUString const m_aRowsStr;

    long m_lHDistWidth;
    long m_lVDistWidth;
    long m_lHeightWidth;
    long m_lLeftWidth;
    long m_lUpperWidth;
    long m_lColsWidth;

    long m_lXWidth;
    long m_lXHeight;

    SwLabItem m_aItem;

    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;

public:

    SwLabPreview();

    void UpdateItem(const SwLabItem& rItem);
};

class SwLabFormatPage : public SfxTabPage
{
    Idle aPreviewIdle;
    SwLabItem aItem;
    bool bModified;

    SwLabPreview m_aPreview;

    std::unique_ptr<weld::Label>  m_xMakeFI;
    std::unique_ptr<weld::Label>  m_xTypeFI;
    std::unique_ptr<weld::CustomWeld> m_xPreview;
    std::unique_ptr<weld::MetricSpinButton>  m_xHDistField;
    std::unique_ptr<weld::MetricSpinButton>  m_xVDistField;
    std::unique_ptr<weld::MetricSpinButton>  m_xWidthField;
    std::unique_ptr<weld::MetricSpinButton>  m_xHeightField;
    std::unique_ptr<weld::MetricSpinButton>  m_xLeftField;
    std::unique_ptr<weld::MetricSpinButton>  m_xUpperField;
    std::unique_ptr<weld::SpinButton> m_xColsField;
    std::unique_ptr<weld::SpinButton> m_xRowsField;
    std::unique_ptr<weld::MetricSpinButton>  m_xPWidthField;
    std::unique_ptr<weld::MetricSpinButton>  m_xPHeightField;
    std::unique_ptr<weld::Button> m_xSavePB;


    DECL_LINK( ModifyHdl, weld::SpinButton&, void );
    DECL_LINK( MetricModifyHdl, weld::MetricSpinButton&, void );
    DECL_LINK( PreviewHdl, Timer *, void );
    DECL_LINK( SaveHdl, weld::Button&, void );

    void ChangeMinMax();

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:
    SwLabFormatPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~SwLabFormatPage() override;

    static VclPtr<SfxTabPage> Create(TabPageParent pParent, const SfxItemSet* rSet);

    virtual void ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet) override;
            void FillItem(SwLabItem& rItem);
    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;

    SwLabDlg* GetParentSwLabDlg() {return static_cast<SwLabDlg*>(GetDialogController());}
};

class SwSaveLabelDlg : public weld::GenericDialogController
{
    bool        bSuccess;
    SwLabDlg*   m_pLabDialog;
    SwLabRec&   rLabRec;

    std::unique_ptr<weld::ComboBox> m_xMakeCB;
    std::unique_ptr<weld::Entry>        m_xTypeED;
    std::unique_ptr<weld::Button>       m_xOKPB;

    DECL_LINK(OkHdl, weld::Button&, void);
    DECL_LINK(ModifyEntryHdl, weld::Entry&, void);
    DECL_LINK(ModifyComboHdl, weld::ComboBox&, void);

    void Modify();

public:
    SwSaveLabelDlg(SwLabDlg* pParent, SwLabRec& rRec);
    virtual ~SwSaveLabelDlg() override;

    void SetLabel(const OUString& rMake, const OUString& rType)
    {
        m_xMakeCB->set_entry_text(rMake);
        m_xTypeED->set_text(rType);
    }
    bool GetLabel(SwLabItem& rItem);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

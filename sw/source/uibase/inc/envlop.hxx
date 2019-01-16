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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_ENVLOP_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_ENVLOP_HXX

#include <svtools/svmedit.hxx>
#include <sfx2/tabdlg.hxx>
#include <vcl/weld.hxx>
#include <vcl/customweld.hxx>
#include "envimg.hxx"

#define GetFieldVal(rField)         (rField).Denormalize((rField).GetValue(FieldUnit::TWIP))
#define SetFieldVal(rField, lValue) (rField).SetValue((rField).Normalize(lValue), FieldUnit::TWIP)

inline int getfieldval(const weld::MetricSpinButton& rField)
{
    return rField.denormalize(rField.get_value(FieldUnit::TWIP));
}

inline void setfieldval(weld::MetricSpinButton& rField, int lValue)
{
    rField.set_value(rField.normalize(lValue), FieldUnit::TWIP);
}

class SwEnvDlg;
class SwEnvPage;
class SwEnvFormatPage;
class SwWrtShell;
class Printer;

class SwEnvPreview : public weld::CustomWidgetController
{
private:
    SwEnvDlg* m_pDialog;

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

public:
    SwEnvPreview();
    void SetDialog(SwEnvDlg* pDialog) { m_pDialog = pDialog; }
};

class SwEnvDlg : public SfxTabDialogController
{
friend class SwEnvPage;
friend class SwEnvFormatPage;
friend class SwEnvPrtPage;
friend class SwEnvPreview;

    SwEnvItem       aEnvItem;
    SwWrtShell      *pSh;
    VclPtr<Printer> pPrinter;
    std::unique_ptr<SfxItemSet> pAddresseeSet;
    std::unique_ptr<SfxItemSet> pSenderSet;

    std::unique_ptr<weld::Button> m_xModify;

    virtual void    PageCreated(const OString& rId, SfxTabPage &rPage) override;
    virtual short   Ok() override;

public:
    SwEnvDlg(weld::Window* pParent, const SfxItemSet& rSet, SwWrtShell* pWrtSh, Printer* pPrt, bool bInsert);
    virtual ~SwEnvDlg() override;
};

class SwEnvPage : public SfxTabPage
{
    SwEnvDlg* m_pDialog;
    SwWrtShell*   m_pSh;
    OUString      m_sActDBName;

    SwEnvPreview m_aPreview;
    std::unique_ptr<weld::TextView> m_xAddrEdit;
    std::unique_ptr<weld::ComboBox> m_xDatabaseLB;
    std::unique_ptr<weld::ComboBox> m_xTableLB;
    std::unique_ptr<weld::ComboBox> m_xDBFieldLB;
    std::unique_ptr<weld::Button> m_xInsertBT;
    std::unique_ptr<weld::CheckButton> m_xSenderBox;
    std::unique_ptr<weld::TextView> m_xSenderEdit;
    std::unique_ptr<weld::CustomWeld> m_xPreview;

    DECL_LINK(DatabaseHdl, weld::ComboBox&, void);
    DECL_LINK(FieldHdl, weld::Button&, void);
    DECL_LINK(SenderHdl, weld::Button&, void);

    void InitDatabaseBox();

    SwEnvDlg* GetParentSwEnvDlg() { return m_pDialog; }

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:
    SwEnvPage(TabPageParent pParent, const SfxItemSet& rSet);
    void Init(SwEnvDlg* pDialog);
    virtual ~SwEnvPage() override;

    static VclPtr<SfxTabPage> Create(TabPageParent pParent, const SfxItemSet* rSet);

    virtual void ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet) override;
            void FillItem(SwEnvItem& rItem);
    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

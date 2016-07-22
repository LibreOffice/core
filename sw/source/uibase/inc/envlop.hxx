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

#include <vcl/fixed.hxx>

#include <vcl/edit.hxx>

#include <vcl/lstbox.hxx>

#include <vcl/button.hxx>

#include "envimg.hxx"

#define GetFieldVal(rField)         (rField).Denormalize((rField).GetValue(FUNIT_TWIP))
#define SetFieldVal(rField, lValue) (rField).SetValue((rField).Normalize(lValue), FUNIT_TWIP)

class SwEnvPage;
class SwEnvFormatPage;
class SwWrtShell;
class Printer;

class SwEnvPreview : public vcl::Window
{
    void Paint(vcl::RenderContext& rRenderContext, const Rectangle&) override;

public:

    SwEnvPreview(vcl::Window * pParent, WinBits nStyle);

protected:
    virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual Size GetOptimalSize() const override;
};

class SwEnvDlg : public SfxTabDialog
{
friend class SwEnvPage;
friend class SwEnvFormatPage;
friend class SwEnvPrtPage;
friend class SwEnvPreview;

    SwEnvItem       aEnvItem;
    SwWrtShell      *pSh;
    VclPtr<Printer> pPrinter;
    SfxItemSet      *pAddresseeSet;
    SfxItemSet      *pSenderSet;
    sal_uInt16      m_nEnvPrintId;

    virtual void    PageCreated( sal_uInt16 nId, SfxTabPage &rPage ) override;
    virtual short   Ok() override;

public:
     SwEnvDlg(vcl::Window* pParent, const SfxItemSet& rSet, SwWrtShell* pWrtSh, Printer* pPrt, bool bInsert);
    virtual ~SwEnvDlg();
    virtual void dispose() override;
};

class SwEnvPage : public SfxTabPage
{
    VclPtr<VclMultiLineEdit> m_pAddrEdit;
    VclPtr<ListBox>      m_pDatabaseLB;
    VclPtr<ListBox>      m_pTableLB;
    VclPtr<ListBox>      m_pDBFieldLB;
    VclPtr<PushButton>   m_pInsertBT;
    VclPtr<CheckBox>     m_pSenderBox;
    VclPtr<VclMultiLineEdit> m_pSenderEdit;
    VclPtr<SwEnvPreview> m_pPreview;

    SwWrtShell*   pSh;
    OUString      sActDBName;

    DECL_LINK_TYPED( DatabaseHdl, ListBox&, void );
    DECL_LINK_TYPED(FieldHdl, Button*, void );
    DECL_LINK_TYPED(SenderHdl, Button*, void );

    void InitDatabaseBox();

    SwEnvDlg* GetParentSwEnvDlg() {return static_cast<SwEnvDlg*>(GetParentDialog());}

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:
    SwEnvPage(vcl::Window* pParent, const SfxItemSet& rSet);
    virtual ~SwEnvPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window* pParent, const SfxItemSet* rSet);

    virtual void ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet) override;
            void FillItem(SwEnvItem& rItem);
    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

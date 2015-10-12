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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_DRPCPS_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_DRPCPS_HXX

#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>

#include <vcl/button.hxx>

#include <vcl/fixed.hxx>

#include <vcl/field.hxx>

#include <vcl/edit.hxx>

#include <vcl/lstbox.hxx>

class SwWrtShell;

class SwDropCapsDlg : public SfxSingleTabDialog
{
public:
    SwDropCapsDlg(vcl::Window *pParent, const SfxItemSet &rSet );
};

class SwDropCapsPict;

class SwDropCapsPage : public SfxTabPage
{
friend class SwDropCapsPict;
    VclPtr<CheckBox>        m_pDropCapsBox;
    VclPtr<CheckBox>        m_pWholeWordCB;
    VclPtr<FixedText>       m_pSwitchText;
    VclPtr<NumericField>    m_pDropCapsField;
    VclPtr<FixedText>       m_pLinesText;
    VclPtr<NumericField>    m_pLinesField;
    VclPtr<FixedText>       m_pDistanceText;
    VclPtr<MetricField>     m_pDistanceField;
    VclPtr<FixedText>       m_pTextText;
    VclPtr<Edit>            m_pTextEdit;
    VclPtr<FixedText>       m_pTemplateText;
    VclPtr<ListBox>         m_pTemplateBox;

    VclPtr<SwDropCapsPict>  m_pPict;

    bool          bModified;
    bool          bFormat;
    bool          bHtmlMode;

    SwWrtShell &rSh;

    virtual sfxpg   DeactivatePage(SfxItemSet *pSet) override;
    void    FillSet( SfxItemSet &rSet );

    DECL_LINK_TYPED(ClickHdl, Button*, void);
    DECL_LINK( ModifyHdl, Edit * );
    DECL_LINK_TYPED(SelectHdl, ListBox&, void);
    DECL_LINK_TYPED(WholeWordHdl, Button*, void);

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

    static const sal_uInt16 aPageRg[];

public:
    SwDropCapsPage(vcl::Window *pParent, const SfxItemSet &rSet);
    virtual ~SwDropCapsPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window *pParent, const SfxItemSet *rSet);
    static const sal_uInt16* GetRanges() { return aPageRg; }


    virtual bool FillItemSet(      SfxItemSet *rSet) override;
    virtual void Reset      (const SfxItemSet *rSet) override;

    void    SetFormat(bool bSet){bFormat = bSet;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

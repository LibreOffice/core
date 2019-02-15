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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_OPTCOMP_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_OPTCOMP_HXX

#include <memory>
#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <unotools/compatibility.hxx>
#include <unotools/compatibilityviewoptions.hxx>
#include <svx/checklbx.hxx>
#include <rtl/ustring.hxx>

class SwWrtShell;
struct SwCompatibilityOptPage_Impl;

class SwCompatibilityOptPage : public SfxTabPage
{
private:
    // controls
    VclPtr<VclFrame>               m_pMain;
    VclPtr<VclFrame>               m_pGlobalOptionsFrame;
    VclPtr<ListBox>                m_pFormattingLB;
    VclPtr<ListBox>                m_pGlobalOptionsLB;
    VclPtr<SvxCheckListBox>        m_pOptionsLB;
    VclPtr<SvxCheckListBox>        m_pGlobalOptionsCLB;
    VclPtr<PushButton>             m_pDefaultPB;
    // config item
    SvtCompatibilityOptions m_aConfigItem;
    // config item
    SvtCompatibilityViewOptions m_aViewConfigItem;
    // text of the user entry
    OUString                m_sUserEntry;
    // shell of the current document
    SwWrtShell*             m_pWrtShell;
    // impl object
    std::unique_ptr<SwCompatibilityOptPage_Impl> m_pImpl;
    // saved options after "Reset"; used in "FillItemSet" for comparison
    sal_uLong                   m_nSavedOptions;
    bool                        m_bSavedMSFormsMenuOption;

    // handler
    DECL_LINK(SelectHdl, ListBox&, void);
    DECL_LINK(UseAsDefaultHdl, Button*, void);

    // private methods
    void                    InitControls( const SfxItemSet& rSet );
    void                    SetCurrentOptions( sal_uLong nOptions );
    sal_uLong                   GetDocumentOptions() const;
    void                    WriteOptions();

public:
    SwCompatibilityOptPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SwCompatibilityOptPage() override;
    virtual void            dispose() override;

    static VclPtr<SfxTabPage> Create( TabPageParent pParent, const SfxItemSet* rAttrSet );

    virtual bool            FillItemSet( SfxItemSet* rSet ) override;
    virtual void            Reset( const SfxItemSet* rSet ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

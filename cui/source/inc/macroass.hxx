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
#pragma once

#include <sal/config.h>

#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>
#include <svl/macitem.hxx>
#include <vcl/weld.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <memory>

class SfxMacroTabPage_;
class SfxMacroTabPage_Impl;

class SfxMacroTabPage final : public SfxTabPage
{
    SvxMacroTableDtor           aTbl;
    DECL_LINK(SelectEvent_Impl, weld::TreeView&, void);
    DECL_LINK(SelectGroup_Impl, weld::TreeView&, void);
    DECL_LINK(SelectMacro_Impl, weld::TreeView&, void);

    DECL_LINK(AssignDeleteHdl_Impl, weld::TreeView&, bool);
    DECL_LINK(AssignDeleteClickHdl_Impl, weld::Button&, void);
    void AssignDeleteHdl(const weld::Widget*);
    DECL_LINK( TimeOut_Impl, Timer*, void );

    std::unique_ptr<SfxMacroTabPage_Impl>       mpImpl;

    void                        InitAndSetHandler();
    void                        FillEvents();
    void                        EnableButtons();

public:
    SfxMacroTabPage(
        weld::Container* pPage, weld::DialogController* pController,
        const css::uno::Reference< css::frame::XFrame >& rxDocumentFrame,
        const SfxItemSet& rSet
    );

    virtual                     ~SfxMacroTabPage() override;

    void                        AddEvent( const OUString & rEventName, SvMacroItemId nEventId );

    void                        ScriptChanged();
    virtual void                PageCreated (const SfxAllItemSet& aSet) override;
    virtual void                ActivatePage( const SfxItemSet& ) override;
    void                        LaunchFillGroup();

    // --------- inherit from the base -------------
    virtual bool                FillItemSet( SfxItemSet* rSet ) override;
    virtual void                Reset( const SfxItemSet* rSet ) override;

    bool                        IsReadOnly() const override;

    // --------- inherit from the base -------------
    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet );
};

class SfxMacroAssignDlg : public SfxSingleTabDialogController
{
public:
    SfxMacroAssignDlg(weld::Widget* pParent,
                      const css::uno::Reference< css::frame::XFrame >& rxDocumentFrame,
                      const SfxItemSet& rSet);
    SfxMacroTabPage* GetTabPage()
    {
        return static_cast<SfxMacroTabPage*>(m_xSfxPage.get());
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

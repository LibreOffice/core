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

#include "cfgutil.hxx"
#include "headertablistbox.hxx"

#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>
#include <svl/macitem.hxx>
#include <vcl/idle.hxx>
#include <vcl/weld/Button.hxx>
#include <vcl/weld/Frame.hxx>
#include <vcl/weld/TreeView.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <memory>

class Timer;

class SfxMacroTabPage final : public SfxTabPage
{
    SvxMacroTableDtor           aTbl;

    OUString m_aStaticMacroLBLabel;
    std::unique_ptr<weld::Button> m_xAssignPB;
    std::unique_ptr<weld::Button> m_xDeletePB;
    std::unique_ptr<MacroEventListBox> m_xEventLB;
    std::unique_ptr<weld::Widget> m_xGroupFrame;
    std::unique_ptr<CuiConfigGroupListBox> m_xGroupLB;
    std::unique_ptr<weld::Frame> m_xMacroFrame;
    std::unique_ptr<CuiConfigFunctionListBox> m_xMacroLB;

    Idle m_aFillGroupIdle{ "cui SfxMacroTabPage m_aFillGroupIdle" };
    bool m_bGotEvents;

    DECL_LINK(SelectEvent_Impl, weld::ItemView&, void);
    DECL_LINK(SelectGroup_Impl, weld::ItemView&, void);
    DECL_LINK(SelectMacro_Impl, weld::ItemView&, void);

    DECL_LINK(MacroTreeViewActivatedHdl, const weld::TreeIter&, bool);
    DECL_LINK(AssignmentsTreeViewActivatedHdl, const weld::TreeIter&, bool);
    DECL_LINK(AssignDeleteClickHdl_Impl, weld::Button&, void);
    void AssignDeleteHdl(const weld::Widget*);
    DECL_LINK( TimeOut_Impl, Timer*, void );

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
                      std::unique_ptr<const SfxItemSet> xSet);
    SfxMacroTabPage* GetTabPage()
    {
        return static_cast<SfxMacroTabPage*>(m_xSfxPage.get());
    }
private:
    std::unique_ptr<const SfxItemSet> mxItemSet;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

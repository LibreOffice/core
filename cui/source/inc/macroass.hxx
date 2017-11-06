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
#ifndef INCLUDED_CUI_SOURCE_INC_MACROASS_HXX
#define INCLUDED_CUI_SOURCE_INC_MACROASS_HXX

#include <sal/config.h>

#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>
#include <svl/macitem.hxx>
#include <vcl/lstbox.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <memory>

class SfxMacroTabPage_;
class SvTabListBox;
class SvTreeListBox;
class SfxMacroTabPage_Impl;

class SfxMacroTabPage final : public SfxTabPage
{
    SvxMacroTableDtor           aTbl;
    DECL_LINK( SelectEvent_Impl, SvTreeListBox*, void );
    DECL_LINK( SelectGroup_Impl, SvTreeListBox*, void );
    DECL_LINK( SelectMacro_Impl, SvTreeListBox*, void );

    DECL_LINK( AssignDeleteHdl_Impl, SvTreeListBox*, bool );
    DECL_LINK( AssignDeleteClickHdl_Impl, Button *, void );
    bool                        AssignDeleteHdl(Control const *);
    DECL_LINK( TimeOut_Impl, Timer*, void );

    std::unique_ptr<SfxMacroTabPage_Impl>       mpImpl;

    void                        InitAndSetHandler();
    void                        FillEvents();
    void                        EnableButtons();

public:
    SfxMacroTabPage(
        vcl::Window* pParent,
        const css::uno::Reference< css::frame::XFrame >& rxDocumentFrame,
        const SfxItemSet& rSet
    );

    virtual                     ~SfxMacroTabPage() override;
    virtual void                dispose() override;

    void                        AddEvent( const OUString & rEventName, SvMacroItemId nEventId );

    void                        ScriptChanged();
    virtual void                PageCreated (const SfxAllItemSet& aSet) override;
    using TabPage::ActivatePage; // FIXME WTF is this nonsense?
    virtual void                ActivatePage( const SfxItemSet& ) override;
    void                        LaunchFillGroup();

    // --------- inherit from the base -------------
    virtual bool                FillItemSet( SfxItemSet* rSet ) override;
    virtual void                Reset( const SfxItemSet* rSet ) override;

    bool                        IsReadOnly() const override;

    // --------- inherit from the base -------------
    static VclPtr<SfxTabPage> Create( vcl::Window* pParent, const SfxItemSet* rAttrSet );
};

class SfxMacroAssignDlg : public SfxSingleTabDialog
{
public:
    SfxMacroAssignDlg(
        vcl::Window* pParent,
        const css::uno::Reference< css::frame::XFrame >& rxDocumentFrame,
        const SfxItemSet& rSet );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include "sal/config.h"

#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>
#include <svl/macitem.hxx>
#include <vcl/lstbox.hxx>
#include <com/sun/star/frame/XFrame.hpp>

class _SfxMacroTabPage;
class SvTabListBox;
class SvTreeListBox;
class _SfxMacroTabPage_Impl;

class _SfxMacroTabPage : public SfxTabPage
{
    SvxMacroTableDtor           aTbl;
    DECL_DLLPRIVATE_LINK_TYPED( SelectEvent_Impl, SvTreeListBox*, void );
    DECL_DLLPRIVATE_LINK_TYPED( SelectGroup_Impl, SvTreeListBox*, void );
    DECL_DLLPRIVATE_LINK_TYPED( SelectMacro_Impl, SvTreeListBox*, void );

    DECL_DLLPRIVATE_LINK_TYPED( AssignDeleteHdl_Impl, SvTreeListBox*, bool );
    DECL_DLLPRIVATE_LINK_TYPED( AssignDeleteClickHdl_Impl, Button *, void );
    bool                        AssignDeleteHdl(Control *);
    DECL_DLLPRIVATE_LINK_TYPED( TimeOut_Impl, Idle*, void );

protected:
    _SfxMacroTabPage_Impl*      mpImpl;

                                _SfxMacroTabPage( vcl::Window* pParent, const SfxItemSet& rItemSet );

    void                        InitAndSetHandler();
    void                        FillEvents();
    void                        FillMacroList();
    void                        EnableButtons();

public:

    virtual                     ~_SfxMacroTabPage();
    virtual void                dispose() override;

    void                        AddEvent( const OUString & rEventName, sal_uInt16 nEventId );

    void                        ScriptChanged();
    virtual void                PageCreated (const SfxAllItemSet& aSet) override;
    using TabPage::ActivatePage; // FIXME WTF is this nonsense?
    virtual void                ActivatePage( const SfxItemSet& ) override;
    void                        LaunchFillGroup();

    // --------- inherit from the base -------------
    virtual bool                FillItemSet( SfxItemSet* rSet ) override;
    virtual void                Reset( const SfxItemSet* rSet ) override;

    bool                        IsReadOnly() const override;
};

class SfxMacroTabPage : public _SfxMacroTabPage
{
public:
    SfxMacroTabPage(
        vcl::Window* pParent,
        const css::uno::Reference< css::frame::XFrame >& rxDocumentFrame,
        const SfxItemSet& rSet
    );

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

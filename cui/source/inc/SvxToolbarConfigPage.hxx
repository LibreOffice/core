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

#include <vcl/weld.hxx>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>

#include <memory>

#include "cfg.hxx" //for SvxConfigPage and SaveInData

class SvxToolbarConfigPage : public SvxConfigPage
{
private:

    DECL_LINK( SelectToolbarEntry, weld::TreeView&, void );
    DECL_LINK( MoveHdl, weld::Button&, void );

    DECL_LINK( GearHdl, const OString&, void );

    DECL_LINK( SelectCategory, weld::ComboBox&, void );

    DECL_LINK( ContentContextMenuHdl, const CommandEvent&, bool );
    DECL_LINK( FunctionContextMenuHdl, const CommandEvent&, bool );

    DECL_LINK( AddCommandHdl, weld::Button&, void );
    DECL_LINK( RemoveCommandHdl, weld::Button&, void );

    DECL_LINK( InsertHdl, const OString&, void );
    DECL_LINK( ModifyItemHdl, const OString&, void );
    DECL_LINK( ResetToolbarHdl, weld::Button&, void );

    virtual void ListModified() override;

    void            UpdateButtonStates() override;
    short           QueryReset() override;
    void            Init() override;
    void            DeleteSelectedContent() override;
    void            DeleteSelectedTopLevel() override;
    virtual void    SelectElement() override;

public:
    SvxToolbarConfigPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rItemSet);
    virtual ~SvxToolbarConfigPage() override;

    void            AddFunction(int nTarget = -1);

    void            MoveEntry( bool bMoveUp ) override;

    SaveInData*     CreateSaveInData(
        const css::uno::Reference <
            css::ui::XUIConfigurationManager >&,
        const css::uno::Reference <
            css::ui::XUIConfigurationManager >&,
        const OUString& aModuleId,
        bool docConfig ) override;
};

class SvxToolbarEntriesListBox final : public SvxMenuEntriesListBox
{
    void ChangedVisibility(int nRow);

    DECL_LINK(CheckButtonHdl, const weld::TreeView::iter_col&, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);

public:

    SvxToolbarEntriesListBox(std::unique_ptr<weld::TreeView> xControl, SvxToolbarConfigPage* pPg);
    virtual ~SvxToolbarEntriesListBox() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

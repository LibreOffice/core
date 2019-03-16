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
#ifndef INCLUDED_SW_SOURCE_UI_FLDUI_FLDFUNC_HXX
#define INCLUDED_SW_SOURCE_UI_FLDUI_FLDFUNC_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>

#include <condedit.hxx>
#include "fldpage.hxx"
#include <actctrl.hxx>

class SwFieldFuncPage : public SwFieldPage
{
    OUString        m_sOldValueFT;
    OUString        m_sOldNameFT;

    sal_uLong           nOldFormat;
    bool            bDropDownLBChanged;

    std::unique_ptr<weld::TreeView> m_xTypeLB;
    std::unique_ptr<weld::TreeView> m_xSelectionLB;
    std::unique_ptr<weld::Widget> m_xFormat;
    std::unique_ptr<weld::TreeView> m_xFormatLB;
    std::unique_ptr<weld::Label> m_xNameFT;
    std::unique_ptr<SwConditionEdit> m_xNameED;
    std::unique_ptr<weld::Widget> m_xValueGroup;
    std::unique_ptr<weld::Label> m_xValueFT;
    std::unique_ptr<weld::Entry> m_xValueED;
    std::unique_ptr<weld::Label> m_xCond1FT;
    std::unique_ptr<SwConditionEdit> m_xCond1ED;
    std::unique_ptr<weld::Label> m_xCond2FT;
    std::unique_ptr<SwConditionEdit> m_xCond2ED;
    std::unique_ptr<weld::Button> m_xMacroBT;

    //controls of "Input list"
    std::unique_ptr<weld::Widget> m_xListGroup;
    std::unique_ptr<weld::Label> m_xListItemFT;
    std::unique_ptr<weld::Entry> m_xListItemED;
    std::unique_ptr<weld::Button> m_xListAddPB;
    std::unique_ptr<weld::Label> m_xListItemsFT;
    std::unique_ptr<weld::TreeView> m_xListItemsLB;
    std::unique_ptr<weld::Button> m_xListRemovePB;
    std::unique_ptr<weld::Button> m_xListUpPB;
    std::unique_ptr<weld::Button> m_xListDownPB;
    std::unique_ptr<weld::Label> m_xListNameFT;
    std::unique_ptr<weld::Entry> m_xListNameED;

    DECL_LINK( TypeHdl, weld::TreeView&, void );
    DECL_LINK( SelectHdl, weld::TreeView&, void );
    DECL_LINK( InsertMacroHdl, weld::TreeView&, void );
    DECL_LINK( ModifyHdl, weld::Entry&, void );
    DECL_LINK( ListModifyReturnActionHdl, weld::Entry&, bool );
    DECL_LINK( ListModifyButtonHdl, weld::Button&, void );
    DECL_LINK( ListEnableHdl, weld::Entry&, void );
    DECL_LINK( ListEnableListBoxHdl, weld::TreeView&, void );
    void ListModifyHdl(const weld::Widget*);

    // select Macro
    DECL_LINK( MacroHdl, weld::Button&, void );

    void                UpdateSubType();

protected:
    virtual sal_uInt16      GetGroup() override;

public:
    SwFieldFuncPage(TabPageParent pParent, const SfxItemSet* pSet);
    virtual ~SwFieldFuncPage() override;

    static VclPtr<SfxTabPage>  Create(TabPageParent pParent, const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    virtual void        FillUserData() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

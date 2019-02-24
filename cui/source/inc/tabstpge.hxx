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
#ifndef INCLUDED_CUI_SOURCE_INC_TABSTPGE_HXX
#define INCLUDED_CUI_SOURCE_INC_TABSTPGE_HXX

#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>
#include <sfx2/tabdlg.hxx>

#include <editeng/tstpitem.hxx>
#include <svx/flagsdef.hxx>

class SvxTabulatorTabPage;

// class TabWin_Impl -----------------------------------------------------

class TabWin_Impl : public weld::CustomWidgetController
{
private:
    sal_uInt16  nTabStyle;

public:

    TabWin_Impl() : nTabStyle(0)
    {
    }
    virtual void Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect) override;

    void SetTabStyle(sal_uInt16 nStyle) {nTabStyle = nStyle; }
};

// class SvxTabulatorTabPage ---------------------------------------------
/*
    [Description]
    In this TabPage tabulators are managed.

    [Items]
    <SvxTabStopItem><SID_ATTR_TABSTOP>
    <SfxUInt16Item><SID_ATTR_TABSTOP_DEFAULTS>
    <SfxUInt16Item><SID_ATTR_TABSTOP_POS>
    <SfxInt32Item><SID_ATTR_TABSTOP_OFFSET>
*/

class SvxTabulatorTabPage : public SfxTabPage
{
    friend class VclPtr<SvxTabulatorTabPage>;
    using TabPage::DeactivatePage;
    static const sal_uInt16 pRanges[];

public:
    virtual ~SvxTabulatorTabPage() override;
    virtual void dispose() override;
    static VclPtr<SfxTabPage>  Create( TabPageParent pParent, const SfxItemSet* rSet );
    static const sal_uInt16* GetRanges() { return pRanges; }

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    void                DisableControls( const TabulatorDisableFlags nFlag );

protected:
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

private:
    SvxTabulatorTabPage(TabPageParent pParent, const SfxItemSet& rSet);

    // local variables, internal functions
    SvxTabStop      aCurrentTab;
    SvxTabStopItem  aNewTabs;
    long            nDefDist;

    TabWin_Impl    m_aLeftWin;
    TabWin_Impl    m_aRightWin;
    TabWin_Impl    m_aCenterWin;
    TabWin_Impl    m_aDezWin;

    // just to format the numbers, not shown
    std::unique_ptr<weld::MetricSpinButton> m_xTabSpin;
    // tabulators and positions
    std::unique_ptr<weld::EntryTreeView> m_xTabBox;
    // TabType
    std::unique_ptr<weld::RadioButton> m_xLeftTab;
    std::unique_ptr<weld::RadioButton> m_xRightTab;
    std::unique_ptr<weld::RadioButton> m_xCenterTab;
    std::unique_ptr<weld::RadioButton> m_xDezTab;

    std::unique_ptr<weld::Entry> m_xDezChar;
    std::unique_ptr<weld::Label> m_xDezCharLabel;

    std::unique_ptr<weld::RadioButton> m_xNoFillChar;
    std::unique_ptr<weld::RadioButton> m_xFillPoints;
    std::unique_ptr<weld::RadioButton> m_xFillDashLine ;
    std::unique_ptr<weld::RadioButton> m_xFillSolidLine;
    std::unique_ptr<weld::RadioButton> m_xFillSpecial;
    std::unique_ptr<weld::Entry> m_xFillChar;

    std::unique_ptr<weld::Button> m_xNewBtn;
    std::unique_ptr<weld::Button> m_xDelAllBtn;
    std::unique_ptr<weld::Button> m_xDelBtn;

    std::unique_ptr<weld::Container> m_xTypeFrame;
    std::unique_ptr<weld::Container> m_xFillFrame;

    std::unique_ptr<weld::CustomWeld> m_xLeftWin;
    std::unique_ptr<weld::CustomWeld> m_xRightWin;
    std::unique_ptr<weld::CustomWeld> m_xCenterWin;
    std::unique_ptr<weld::CustomWeld> m_xDezWin;

    void            InitTabPos_Impl( sal_uInt16 nPos = 0 );
    void            SetFillAndTabType_Impl();
    void            NewHdl_Impl(const weld::Button*);

    OUString        FormatTab();

    // Handler
    DECL_LINK(NewHdl_Impl, weld::Button&, void);
    DECL_LINK(DelHdl_Impl, weld::Button&, void);
    DECL_LINK(DelAllHdl_Impl, weld::Button&, void);

    DECL_LINK(FillTypeCheckHdl_Impl, weld::Button&, void);
    DECL_LINK(TabTypeCheckHdl_Impl, weld::Button&, void);

    DECL_LINK(SelectHdl_Impl, weld::TreeView&, void);
    DECL_LINK(ModifyHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(ReformatHdl_Impl, weld::Widget&, void);
    DECL_LINK(GetFillCharHdl_Impl, weld::Widget&, void);
    DECL_LINK(GetDezCharHdl_Impl, weld::Widget&, void);

    int FindCurrentTab();

    virtual void            PageCreated(const SfxAllItemSet& aSet) override;
};

#endif // INCLUDED_CUI_SOURCE_INC_TABSTPGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <vcl/group.hxx>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <sfx2/tabdlg.hxx>

#include <editeng/tstpitem.hxx>
#include <svx/flagsdef.hxx>

class TabWin_Impl;

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
    virtual ~SvxTabulatorTabPage();
    virtual void dispose() override;
    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rSet );
    static const sal_uInt16* GetRanges() { return pRanges; }

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    void                DisableControls( const TabulatorDisableFlags nFlag );

protected:
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

private:
    SvxTabulatorTabPage( vcl::Window* pParent, const SfxItemSet& rSet );

    // tabulators and positions
    VclPtr<MetricBox>      m_pTabBox;
    // TabType
    VclPtr<RadioButton>    m_pLeftTab;
    VclPtr<RadioButton>    m_pRightTab;
    VclPtr<RadioButton>    m_pCenterTab;
    VclPtr<RadioButton>    m_pDezTab;

    VclPtr<TabWin_Impl>    m_pLeftWin;
    VclPtr<TabWin_Impl>    m_pRightWin;
    VclPtr<TabWin_Impl>    m_pCenterWin;
    VclPtr<TabWin_Impl>    m_pDezWin;

    VclPtr<FixedText>      m_pDezCharLabel;
    VclPtr<Edit>           m_pDezChar;

    VclPtr<RadioButton>    m_pNoFillChar;
    VclPtr<RadioButton>    m_pFillPoints;
    VclPtr<RadioButton>    m_pFillDashLine ;
    VclPtr<RadioButton>    m_pFillSolidLine;
    VclPtr<RadioButton>    m_pFillSpecial;
    VclPtr<Edit>           m_pFillChar;

    VclPtr<PushButton>     m_pNewBtn;
    VclPtr<PushButton>     m_pDelAllBtn;
    VclPtr<PushButton>     m_pDelBtn;

    VclPtr<VclContainer>   m_pTypeFrame;
    VclPtr<VclContainer>   m_pFillFrame;

    // local variables, internal functions
    SvxTabStop      aAktTab;
    SvxTabStopItem  aNewTabs;
    long            nDefDist;
    FieldUnit       eDefUnit;
    bool        bCheck;

    void            InitTabPos_Impl( sal_uInt16 nPos = 0 );
    void            SetFillAndTabType_Impl();

    // Handler
    DECL_LINK_TYPED( NewHdl_Impl, Button*, void );
    DECL_LINK_TYPED( DelHdl_Impl, Button*, void );
    DECL_LINK_TYPED( DelAllHdl_Impl, Button*, void );

    DECL_LINK_TYPED( FillTypeCheckHdl_Impl, Button*, void );
    DECL_LINK_TYPED( TabTypeCheckHdl_Impl, Button*, void );

    DECL_LINK_TYPED( SelectHdl_Impl, ComboBox&, void );
    DECL_LINK_TYPED( ModifyHdl_Impl, Edit&, void );
    DECL_LINK_TYPED( GetFillCharHdl_Impl, Control&, void );
    DECL_LINK_TYPED( GetDezCharHdl_Impl, Control&, void );

    virtual void            PageCreated(const SfxAllItemSet& aSet) override;
};

#endif // INCLUDED_CUI_SOURCE_INC_TABSTPGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

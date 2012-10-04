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
#ifndef _SVX_TABSTPGE_HXX
#define _SVX_TABSTPGE_HXX

#include <vcl/group.hxx>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/tabdlg.hxx>

#include <editeng/tstpitem.hxx>
#include <svx/flagsdef.hxx>

// forward ---------------------------------------------------------------

class TabWin_Impl;

// class SvxTabulatorTabPage ---------------------------------------------
/*
    {k:\svx\prototyp\dialog\tabstop.bmp}

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
    using TabPage::DeactivatePage;

public:
    ~SvxTabulatorTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static sal_uInt16*      GetRanges();

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    void                DisableControls( const sal_uInt16 nFlag );

protected:
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );

private:
    SvxTabulatorTabPage( Window* pParent, const SfxItemSet& rSet );

    // tabulators and positions
    FixedLine       aTabLabel;
    MetricBox       aTabBox;
    FixedLine       aTabLabelVert;

    FixedLine       aTabTypeLabel;
    // TabType
    RadioButton     aLeftTab;
    RadioButton     aRightTab;
    RadioButton     aCenterTab;
    RadioButton     aDezTab;

    TabWin_Impl*    pLeftWin;
    TabWin_Impl*    pRightWin;
    TabWin_Impl*    pCenterWin;
    TabWin_Impl*    pDezWin;

    FixedText       aDezCharLabel;
    Edit            aDezChar;

    FixedLine       aFillLabel;

    RadioButton     aNoFillChar;
    RadioButton     aFillPoints;
    RadioButton     aFillDashLine ;
    RadioButton     aFillSolidLine;
    RadioButton     aFillSpecial;
    Edit            aFillChar;

    PushButton      aNewBtn;
    PushButton      aDelAllBtn;
    PushButton      aDelBtn;

    // local variables, internal functions
    SvxTabStop      aAktTab;
    SvxTabStopItem  aNewTabs;
    long            nDefDist;
    FieldUnit       eDefUnit;
    sal_Bool            bCheck;

#ifdef _SVX_TABSTPGE_CXX
    void            InitTabPos_Impl( sal_uInt16 nPos = 0 );
    void            SetFillAndTabType_Impl();

    // Handler
    DECL_LINK( NewHdl_Impl, Button* );
    DECL_LINK(DelHdl_Impl, void *);
    DECL_LINK(DelAllHdl_Impl, void *);

    DECL_LINK( FillTypeCheckHdl_Impl, RadioButton* );
    DECL_LINK( TabTypeCheckHdl_Impl, RadioButton* );

    DECL_LINK(SelectHdl_Impl, void *);
    DECL_LINK(ModifyHdl_Impl, void *);
    DECL_LINK( GetFillCharHdl_Impl, Edit* );
    DECL_LINK( GetDezCharHdl_Impl, Edit* );
#endif
    virtual void            PageCreated(SfxAllItemSet aSet);
};

#endif // #ifndef _SVX_TABSTPGE_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

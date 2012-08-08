/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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

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
#ifndef _SVX_HDFT_HXX
#define _SVX_HDFT_HXX

#include <sfx2/tabdlg.hxx>

#include <vcl/fixed.hxx>

#include <vcl/field.hxx>

#include <vcl/group.hxx>
#include <svx/pagectrl.hxx>
#include "svx/svxdllapi.h"

namespace svx
{
    SVX_DLLPUBLIC bool ShowBorderBackgroundDlg( Window* pParent, SfxItemSet* pBBSet,
            bool bEnableBackgroundSelector );
}

// class  SvxHFPage ------------------------------------------------------

class SVX_DLLPUBLIC SvxHFPage: public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:

    virtual sal_Bool    FillItemSet( SfxItemSet& rOutSet );
    virtual void    Reset( const SfxItemSet& rSet );

    virtual         ~SvxHFPage();

    void            DisableDeleteQueryBox() { bDisableQueryBox = sal_True; }
    void            EnableBackgroundSelector( sal_Bool bNew ) { bEnableBackgroundSelector = bNew; }
    void            EnableDynamicSpacing();

protected:
    virtual void    ActivatePage( const SfxItemSet& rSet );
    virtual int     DeactivatePage( SfxItemSet* pSet = 0 );

    SvxHFPage( Window* pParent, sal_uInt16 nResId, const SfxItemSet& rSet,
               sal_uInt16 nSetId );

    FixedLine       aFrm;
    CheckBox        aTurnOnBox;
    CheckBox        aCntSharedBox;
    CheckBox        aCntSharedFirstBox;
    FixedText       aLMLbl;
    MetricField     aLMEdit;
    FixedText       aRMLbl;
    MetricField     aRMEdit;
    FixedText       aDistFT;
    MetricField     aDistEdit;
    CheckBox        aDynSpacingCB;
    FixedText       aHeightFT;
    MetricField     aHeightEdit;
    CheckBox        aHeightDynBtn;
    SvxPageWindow   aBspWin;
    PushButton      aBackgroundBtn;

    sal_uInt16          nId;
    SfxItemSet*     pBBSet;
    sal_Bool            bDisableQueryBox;
    sal_Bool            bEnableBackgroundSelector;
    bool            bInReset;

    void            InitHandler();
    DECL_LINK( TurnOnHdl, CheckBox*);
    DECL_LINK(DistModify, void *);
    DECL_LINK(HeightModify, void *);
    DECL_LINK(BorderModify, void *);
    DECL_LINK(BackgroundHdl, void *);

    void            UpdateExample();
    DECL_LINK(RangeHdl, void *);

private:
#ifdef _SVX_HDFT_CXX
    SVX_DLLPRIVATE void         ResetBackground_Impl( const SfxItemSet& rSet );
#endif
};

// class SvxHeaderPage ---------------------------------------------------

class SVX_DLLPUBLIC SvxHeaderPage : public SvxHFPage
{
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static sal_uInt16*      GetRanges();

private:
    SvxHeaderPage( Window* pParent, const SfxItemSet& rSet );
};

// class SvxFooterPage ---------------------------------------------------

class SVX_DLLPUBLIC SvxFooterPage : public SvxHFPage
{
public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rSet );
    static sal_uInt16*      GetRanges();

private:
    SVX_DLLPRIVATE SvxFooterPage(   Window* pParent, const SfxItemSet& rSet );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

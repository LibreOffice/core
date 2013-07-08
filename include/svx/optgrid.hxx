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
#ifndef _SVX_OPTGRID_HXX
#define _SVX_OPTGRID_HXX

#include <sfx2/tabdlg.hxx>
#include <svl/eitem.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include "svx/svxdllapi.h"

class SvxGridTabPage;

// class SvxOptionsGrid --------------------------------------------------

class SVX_DLLPUBLIC SvxOptionsGrid
{
protected:
    sal_uInt32  nFldDrawX;
    sal_uInt32  nFldDivisionX;
    sal_uInt32  nFldDrawY;
    sal_uInt32  nFldDivisionY;
    sal_uInt32  nFldSnapX;
    sal_uInt32  nFldSnapY;
    sal_Bool    bUseGridsnap:1;
    sal_Bool    bSynchronize:1;
    sal_Bool    bGridVisible:1;
    sal_Bool    bEqualGrid:  1;

public:
    SvxOptionsGrid();
    ~SvxOptionsGrid();

    void    SetFldDrawX(    sal_uInt32 nSet){nFldDrawX      = nSet;}
    void    SetFldDivisionX(sal_uInt32 nSet){nFldDivisionX  = nSet;}
    void    SetFldDrawY   ( sal_uInt32 nSet){nFldDrawY      = nSet;}
    void    SetFldDivisionY(sal_uInt32 nSet){nFldDivisionY  = nSet;}
    void    SetFldSnapX(    sal_uInt32 nSet){nFldSnapX      = nSet;}
    void    SetFldSnapY   ( sal_uInt32 nSet){nFldSnapY      = nSet;}
    void    SetUseGridSnap( sal_Bool bSet ) {bUseGridsnap   = bSet;}
    void    SetSynchronize( sal_Bool bSet ) {bSynchronize   = bSet;}
    void    SetGridVisible( sal_Bool bSet ) {bGridVisible   = bSet;}
    void    SetEqualGrid( sal_Bool bSet )   {bEqualGrid     = bSet;}

    sal_uInt32  GetFldDrawX(    ) const {  return nFldDrawX;    }
    sal_uInt32  GetFldDivisionX() const {  return nFldDivisionX;}
    sal_uInt32  GetFldDrawY   ( ) const {  return nFldDrawY;    }
    sal_uInt32  GetFldDivisionY() const {  return nFldDivisionY;}
    sal_uInt32  GetFldSnapX(    ) const {  return nFldSnapX;    }
    sal_uInt32  GetFldSnapY   ( ) const {  return nFldSnapY;    }
    sal_Bool    GetUseGridSnap( ) const {  return bUseGridsnap; }
    sal_Bool    GetSynchronize( ) const {  return bSynchronize; }
    sal_Bool    GetGridVisible( ) const {  return bGridVisible; }
    sal_Bool    GetEqualGrid()    const {  return bEqualGrid;   }
};

// class SvxGridItem -----------------------------------------------------

class SVX_DLLPUBLIC SvxGridItem : public SvxOptionsGrid, public SfxPoolItem
{
    // #i9076#
    friend class SvxGridTabPage;

public:
    SvxGridItem( sal_uInt16 _nWhich) : SfxPoolItem(_nWhich){};
    SvxGridItem( const SvxGridItem& pTestItem );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const;

};

// class SvxGridTabPage --------------------------------------------------

class SVX_DLLPUBLIC SvxGridTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:
    SvxGridTabPage( Window* pParent, const SfxItemSet& rSet );

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    virtual void        ActivatePage( const SfxItemSet& rSet );
    virtual int         DeactivatePage( SfxItemSet* pSet );

private:
    CheckBox*            pCbxUseGridsnap;
    CheckBox*            pCbxGridVisible;

    MetricField*         pMtrFldDrawX;
    MetricField*         pMtrFldDrawY;

    NumericField*        pNumFldDivisionX;
    NumericField*        pNumFldDivisionY;

    CheckBox*            pCbxSynchronize;
protected:
    //these controls are used in draw and impress
    VclContainer*    pSnapFrames;
    CheckBox*        pCbxSnapHelplines;
    CheckBox*        pCbxSnapBorder;
    CheckBox*        pCbxSnapFrame;
    CheckBox*        pCbxSnapPoints;
    MetricField*     pMtrFldSnapArea;

    CheckBox*        pCbxOrtho;
    CheckBox*        pCbxBigOrtho;
    CheckBox*        pCbxRotate;
    MetricField*     pMtrFldAngle;
    MetricField*     pMtrFldBezAngle;

private:
    sal_Bool                bAttrModified;

    DECL_LINK( ClickRotateHdl_Impl, void * );
    DECL_LINK( ChangeDrawHdl_Impl, MetricField * );
    DECL_LINK( ChangeGridsnapHdl_Impl, void * );
    DECL_LINK( ChangeDivisionHdl_Impl, NumericField * );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

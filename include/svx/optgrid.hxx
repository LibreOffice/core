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
#ifndef INCLUDED_SVX_OPTGRID_HXX
#define INCLUDED_SVX_OPTGRID_HXX

#include <sfx2/tabdlg.hxx>
#include <svl/eitem.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <svx/svxdllapi.h>

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
    bool        bUseGridsnap:1;
    bool        bSynchronize:1;
    bool        bGridVisible:1;
    bool        bEqualGrid:  1;

public:
    SvxOptionsGrid();
    ~SvxOptionsGrid();

    void    SetFieldDrawX(    sal_uInt32 nSet){nFldDrawX      = nSet;}
    void    SetFieldDivisionX(sal_uInt32 nSet){nFldDivisionX  = nSet;}
    void    SetFieldDrawY   ( sal_uInt32 nSet){nFldDrawY      = nSet;}
    void    SetFieldDivisionY(sal_uInt32 nSet){nFldDivisionY  = nSet;}
    void    SetFieldSnapX(    sal_uInt32 nSet){nFldSnapX      = nSet;}
    void    SetFieldSnapY   ( sal_uInt32 nSet){nFldSnapY      = nSet;}
    void    SetUseGridSnap( bool bSet ) {bUseGridsnap   = bSet;}
    void    SetSynchronize( bool bSet ) {bSynchronize   = bSet;}
    void    SetGridVisible( bool bSet ) {bGridVisible   = bSet;}
    void    SetEqualGrid( bool bSet )   {bEqualGrid     = bSet;}

    sal_uInt32  GetFieldDrawX(    ) const {  return nFldDrawX;    }
    sal_uInt32  GetFieldDivisionX() const {  return nFldDivisionX;}
    sal_uInt32  GetFieldDrawY   ( ) const {  return nFldDrawY;    }
    sal_uInt32  GetFieldDivisionY() const {  return nFldDivisionY;}
    sal_uInt32  GetFieldSnapX(    ) const {  return nFldSnapX;    }
    sal_uInt32  GetFieldSnapY   ( ) const {  return nFldSnapY;    }
    bool        GetUseGridSnap( ) const {  return bUseGridsnap; }
    bool        GetSynchronize( ) const {  return bSynchronize; }
    bool        GetGridVisible( ) const {  return bGridVisible; }
    bool        GetEqualGrid()    const {  return bEqualGrid;   }
};

// class SvxGridItem -----------------------------------------------------

class SVX_DLLPUBLIC SvxGridItem : public SvxOptionsGrid, public SfxPoolItem
{
    // #i9076#
    friend class SvxGridTabPage;

public:
    SvxGridItem( sal_uInt16 _nWhich) : SfxPoolItem(_nWhich){};
    SvxGridItem( const SvxGridItem& pTestItem );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const override;

};

// class SvxGridTabPage --------------------------------------------------

class SVX_DLLPUBLIC SvxGridTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:
    SvxGridTabPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SvxGridTabPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create( vcl::Window* pParent, const SfxItemSet& rAttrSet );

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    virtual void        ActivatePage( const SfxItemSet& rSet ) override;
    virtual sfxpg       DeactivatePage( SfxItemSet* pSet ) override;

private:
    VclPtr<CheckBox>            pCbxUseGridsnap;
    VclPtr<CheckBox>            pCbxGridVisible;

    VclPtr<MetricField>         pMtrFldDrawX;
    VclPtr<MetricField>         pMtrFldDrawY;

    VclPtr<NumericField>        pNumFldDivisionX;
    VclPtr<NumericField>        pNumFldDivisionY;

    VclPtr<CheckBox>            pCbxSynchronize;
protected:
    //these controls are used in draw and impress
    VclPtr<VclContainer>    pSnapFrames;
    VclPtr<CheckBox>        pCbxSnapHelplines;
    VclPtr<CheckBox>        pCbxSnapBorder;
    VclPtr<CheckBox>        pCbxSnapFrame;
    VclPtr<CheckBox>        pCbxSnapPoints;
    VclPtr<MetricField>     pMtrFldSnapArea;

    VclPtr<CheckBox>        pCbxOrtho;
    VclPtr<CheckBox>        pCbxBigOrtho;
    VclPtr<CheckBox>        pCbxRotate;
    VclPtr<MetricField>     pMtrFldAngle;
    VclPtr<MetricField>     pMtrFldBezAngle;

private:
    bool                bAttrModified;

    DECL_LINK_TYPED( ClickRotateHdl_Impl, Button*, void );
    DECL_LINK_TYPED( ChangeDrawHdl_Impl, Edit&, void );
    DECL_LINK_TYPED( ChangeGridsnapHdl_Impl, Button*, void );
    DECL_LINK_TYPED( ChangeDivisionHdl_Impl, Edit&, void );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

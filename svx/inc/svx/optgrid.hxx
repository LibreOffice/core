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
                                    String &rText, const IntlWrapper * = 0 ) const;

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
    CheckBox            aCbxUseGridsnap;
    CheckBox            aCbxGridVisible;

    FixedLine           aFlResolution;
    FixedText           aFtDrawX;
    MetricField         aMtrFldDrawX;
    FixedText           aFtDrawY;
    MetricField         aMtrFldDrawY;

    FixedLine           aFlDivision;
    FixedText           aFtDivisionX;
    NumericField        aNumFldDivisionX;
    FixedText           aDivisionPointX;

    FixedText           aFtDivisionY;
    NumericField        aNumFldDivisionY;
    FixedText           aDivisionPointY;

    CheckBox            aCbxSynchronize;
    FixedLine           aGrpDrawGrid;

protected:
    //these controls are used in draw and impress
    FixedLine       aGrpSnap;
    CheckBox        aCbxSnapHelplines;
    CheckBox        aCbxSnapBorder;
    CheckBox        aCbxSnapFrame;
    CheckBox        aCbxSnapPoints;
    FixedText       aFtSnapArea;
    MetricField     aMtrFldSnapArea;

    FixedLine       aSeparatorFL;

    FixedLine       aGrpOrtho;
    CheckBox        aCbxOrtho;
    CheckBox        aCbxBigOrtho;
    CheckBox        aCbxRotate;
    MetricField     aMtrFldAngle;
    FixedText       aFtBezAngle;
    MetricField     aMtrFldBezAngle;

private:
    sal_Bool                bAttrModified;

#ifdef _SVX_OPTGRID_CXX
    DECL_LINK( ClickRotateHdl_Impl, void * );
    DECL_LINK( ChangeDrawHdl_Impl, MetricField * );
    DECL_LINK( ChangeGridsnapHdl_Impl, void * );
    DECL_LINK( ChangeDivisionHdl_Impl, NumericField * );

#endif
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

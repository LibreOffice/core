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

// include ---------------------------------------------------------------

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
    UINT32  nFldDrawX;
    UINT32  nFldDivisionX;
    UINT32  nFldDrawY;
    UINT32  nFldDivisionY;
    UINT32  nFldSnapX;
    UINT32  nFldSnapY;
    BOOL    bUseGridsnap:1;
    BOOL    bSynchronize:1;
    BOOL    bGridVisible:1;
    BOOL    bEqualGrid:  1;

public:
    SvxOptionsGrid();
    ~SvxOptionsGrid();

    void    SetFldDrawX(    UINT32 nSet){nFldDrawX      = nSet;}
    void    SetFldDivisionX(UINT32 nSet){nFldDivisionX  = nSet;}
    void    SetFldDrawY   ( UINT32 nSet){nFldDrawY      = nSet;}
    void    SetFldDivisionY(UINT32 nSet){nFldDivisionY  = nSet;}
    void    SetFldSnapX(    UINT32 nSet){nFldSnapX      = nSet;}
    void    SetFldSnapY   ( UINT32 nSet){nFldSnapY      = nSet;}
    void    SetUseGridSnap( BOOL bSet ) {bUseGridsnap   = bSet;}
    void    SetSynchronize( BOOL bSet ) {bSynchronize   = bSet;}
    void    SetGridVisible( BOOL bSet ) {bGridVisible   = bSet;}
    void    SetEqualGrid( BOOL bSet )   {bEqualGrid     = bSet;}

    UINT32  GetFldDrawX(    ) const {  return nFldDrawX;    }
    UINT32  GetFldDivisionX() const {  return nFldDivisionX;}
    UINT32  GetFldDrawY   ( ) const {  return nFldDrawY;    }
    UINT32  GetFldDivisionY() const {  return nFldDivisionY;}
    UINT32  GetFldSnapX(    ) const {  return nFldSnapX;    }
    UINT32  GetFldSnapY   ( ) const {  return nFldSnapY;    }
    BOOL    GetUseGridSnap( ) const {  return bUseGridsnap; }
    BOOL    GetSynchronize( ) const {  return bSynchronize; }
    BOOL    GetGridVisible( ) const {  return bGridVisible; }
    BOOL    GetEqualGrid()    const {  return bEqualGrid;   }
};

// class SvxGridItem -----------------------------------------------------

class SVX_DLLPUBLIC SvxGridItem : public SvxOptionsGrid, public SfxPoolItem
{
    // #i9076#
    friend class SvxGridTabPage;

public:
    SvxGridItem( USHORT _nWhich) : SfxPoolItem(_nWhich){};
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

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
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
    BOOL                bAttrModified;

#ifdef _SVX_OPTGRID_CXX
    DECL_LINK( ClickRotateHdl_Impl, void * );
    DECL_LINK( ChangeDrawHdl_Impl, MetricField * );
    DECL_LINK( ChangeGridsnapHdl_Impl, void * );
    DECL_LINK( ChangeDivisionHdl_Impl, NumericField * );

#endif
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

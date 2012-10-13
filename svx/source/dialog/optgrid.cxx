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

#include <tools/shl.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <svl/intitem.hxx>

#define _SVX_OPTGRID_CXX

#include <svx/svxids.hrc>
#include <svx/dialmgr.hxx>
#include "svx/optgrid.hxx"
#include <svx/dialogs.hrc>
#include "optgrid.hrc"
#include "svx/dlgutil.hxx"

// local functions
static void    lcl_GetMinMax(MetricField& rField, long& nFirst, long& nLast, long& nMin, long& nMax)
{
    nFirst  = static_cast<long>(rField.Denormalize( rField.GetFirst( FUNIT_TWIP ) ));
    nLast = static_cast<long>(rField.Denormalize( rField.GetLast( FUNIT_TWIP ) ));
    nMin = static_cast<long>(rField.Denormalize( rField.GetMin( FUNIT_TWIP ) ));
    nMax = static_cast<long>(rField.Denormalize( rField.GetMax( FUNIT_TWIP ) ));
}

static void    lcl_SetMinMax(MetricField& rField, long nFirst, long nLast, long nMin, long nMax)
{
    rField.SetFirst( rField.Normalize( nFirst ), FUNIT_TWIP );
    rField.SetLast( rField.Normalize( nLast ), FUNIT_TWIP );
    rField.SetMin( rField.Normalize( nMin ), FUNIT_TWIP );
    rField.SetMax( rField.Normalize( nMax ), FUNIT_TWIP );
}

SvxOptionsGrid::SvxOptionsGrid() :
    nFldDrawX       ( 100 ),
    nFldDivisionX   ( 0 ),
    nFldDrawY       ( 100 ),
    nFldDivisionY   ( 0 ),
    nFldSnapX       ( 100 ),
    nFldSnapY       ( 100 ),
    bUseGridsnap    ( 0 ),
    bSynchronize    ( 1 ),
    bGridVisible    ( 0 ),
    bEqualGrid      ( 1 )
{
}

SvxOptionsGrid::~SvxOptionsGrid()
{
}

SvxGridItem::SvxGridItem( const SvxGridItem& rItem )
:   SvxOptionsGrid()
,   SfxPoolItem(rItem)
{
    bUseGridsnap = rItem.bUseGridsnap ;
    bSynchronize = rItem.bSynchronize ;
    bGridVisible = rItem.bGridVisible ;
    bEqualGrid   = rItem.bEqualGrid   ;
    nFldDrawX    = rItem.nFldDrawX    ;
    nFldDivisionX= rItem.nFldDivisionX;
    nFldDrawY    = rItem.nFldDrawY    ;
    nFldDivisionY= rItem.nFldDivisionY;
    nFldSnapX    = rItem.nFldSnapX    ;
    nFldSnapY    = rItem.nFldSnapY    ;

};

SfxPoolItem*  SvxGridItem::Clone( SfxItemPool* ) const
{
    return new SvxGridItem( *this );
}

int SvxGridItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "different types ");

    const SvxGridItem& rItem = (const SvxGridItem&) rAttr;

    return (    bUseGridsnap == rItem.bUseGridsnap &&
                bSynchronize == rItem.bSynchronize &&
                bGridVisible == rItem.bGridVisible &&
                bEqualGrid   == rItem.bEqualGrid   &&
                nFldDrawX    == rItem.nFldDrawX    &&
                nFldDivisionX== rItem.nFldDivisionX&&
                nFldDrawY    == rItem.nFldDrawY    &&
                nFldDivisionY== rItem.nFldDivisionY&&
                nFldSnapX    == rItem.nFldSnapX    &&
                nFldSnapY    == rItem.nFldSnapY     );
}

SfxItemPresentation  SvxGridItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    String&             rText, const IntlWrapper *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = rtl::OUString("SvxGridItem");
            return ePres;
        default:
            return SFX_ITEM_PRESENTATION_NONE;
    }
}

// TabPage Screen Settings
SvxGridTabPage::SvxGridTabPage( Window* pParent, const SfxItemSet& rCoreSet) :

    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_GRID ), rCoreSet ),

    aCbxUseGridsnap ( this, SVX_RES( CBX_USE_GRIDSNAP ) ),
    aCbxGridVisible ( this, SVX_RES( CBX_GRID_VISIBLE ) ),

    aFlResolution   ( this, SVX_RES( FL_RESOLUTION ) ),
    aFtDrawX        ( this, SVX_RES( FT_DRAW_X ) ),
    aMtrFldDrawX    ( this, SVX_RES( MTR_FLD_DRAW_X ) ),
    aFtDrawY        ( this, SVX_RES( FT_DRAW_Y ) ),
    aMtrFldDrawY    ( this, SVX_RES( MTR_FLD_DRAW_Y ) ),

    aFlDivision     ( this, SVX_RES( FL_DIVISION ) ),
    aFtDivisionX(     this, SVX_RES( FT_DIVISION_X) ),
    aNumFldDivisionX( this, SVX_RES( NUM_FLD_DIVISION_X ) ),
    aDivisionPointX(  this, SVX_RES( FT_HORZ_POINTS) ),

    aFtDivisionY(     this, SVX_RES( FT_DIVISION_Y) ),
    aNumFldDivisionY( this, SVX_RES( NUM_FLD_DIVISION_Y ) ),
    aDivisionPointY(  this, SVX_RES( FT_VERT_POINTS) ),

    aCbxSynchronize ( this, SVX_RES( CBX_SYNCHRONIZE ) ),
    aGrpDrawGrid    ( this, SVX_RES( GRP_DRAWGRID ) ),

    aGrpSnap            ( this, SVX_RES( GRP_SNAP ) ),
    aCbxSnapHelplines   ( this, SVX_RES( CBX_SNAP_HELPLINES ) ),
    aCbxSnapBorder      ( this, SVX_RES( CBX_SNAP_BORDER ) ),
    aCbxSnapFrame       ( this, SVX_RES( CBX_SNAP_FRAME ) ),
    aCbxSnapPoints      ( this, SVX_RES( CBX_SNAP_POINTS ) ),
    aFtSnapArea         ( this, SVX_RES( FT_SNAP_AREA ) ),
    aMtrFldSnapArea     ( this, SVX_RES( MTR_FLD_SNAP_AREA ) ),

    aSeparatorFL        ( this, SVX_RES( FL_SEPARATOR ) ),

    aGrpOrtho           ( this, SVX_RES( GRP_ORTHO ) ),
    aCbxOrtho           ( this, SVX_RES( CBX_ORTHO ) ),
    aCbxBigOrtho        ( this, SVX_RES( CBX_BIGORTHO ) ),
    aCbxRotate          ( this, SVX_RES( CBX_ROTATE ) ),
    aMtrFldAngle        ( this, SVX_RES( MTR_FLD_ANGLE ) ),
    aFtBezAngle         ( this, SVX_RES( FT_BEZ_ANGLE ) ),
    aMtrFldBezAngle     ( this, SVX_RES( MTR_FLD_BEZ_ANGLE ) ),

    bAttrModified( sal_False )
{
    // This page requires exchange Support
    SetExchangeSupport();

    FreeResource();

    aDivisionPointY.SetText(aDivisionPointX.GetText());
    // Set Metrics
    FieldUnit eFUnit = GetModuleFieldUnit( rCoreSet );
    long nFirst, nLast, nMin, nMax;

    lcl_GetMinMax(aMtrFldDrawX, nFirst, nLast, nMin, nMax);
    SetFieldUnit( aMtrFldDrawX, eFUnit, sal_True );
    lcl_SetMinMax(aMtrFldDrawX, nFirst, nLast, nMin, nMax);

    lcl_GetMinMax(aMtrFldDrawY, nFirst, nLast, nMin, nMax);
    SetFieldUnit( aMtrFldDrawY, eFUnit, sal_True );
    lcl_SetMinMax(aMtrFldDrawY, nFirst, nLast, nMin, nMax);


    aCbxRotate.SetClickHdl( LINK( this, SvxGridTabPage, ClickRotateHdl_Impl ) );
    Link aLink = LINK( this, SvxGridTabPage, ChangeGridsnapHdl_Impl );
    aCbxUseGridsnap.SetClickHdl( aLink );
    aCbxSynchronize.SetClickHdl( aLink );
    aCbxGridVisible.SetClickHdl( aLink );
    aMtrFldDrawX.SetModifyHdl(
        LINK( this, SvxGridTabPage, ChangeDrawHdl_Impl ) );
    aMtrFldDrawY.SetModifyHdl(
        LINK( this, SvxGridTabPage, ChangeDrawHdl_Impl ) );
    aNumFldDivisionX.SetModifyHdl(
        LINK( this, SvxGridTabPage, ChangeDivisionHdl_Impl ) );
    aNumFldDivisionY.SetModifyHdl(
        LINK( this, SvxGridTabPage, ChangeDivisionHdl_Impl ) );

    ::rtl::OUString sFlResolution( aFlResolution.GetDisplayText() );
    ::rtl::OUString sFtDrawX(aFtDrawX.GetDisplayText());
    ::rtl::OUString sFtDrawY(aFtDrawY.GetDisplayText());
    aMtrFldDrawX.SetAccessibleName( sFtDrawX + sFlResolution   );
    aMtrFldDrawY.SetAccessibleName( sFtDrawY + sFlResolution   );
    ::rtl::OUString sFlDivision( aFlDivision.GetDisplayText() );
    ::rtl::OUString sFtDivisionX(aFtDivisionX.GetDisplayText());
    ::rtl::OUString sFtDivisionY(aFtDivisionY.GetDisplayText());
    aNumFldDivisionX.SetAccessibleName(  sFtDivisionX + sFlDivision );
    aNumFldDivisionY.SetAccessibleName(  sFtDivisionY + sFlDivision );
}

//------------------------------------------------------------------------

SfxTabPage* SvxGridTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return ( new SvxGridTabPage( pParent, rAttrSet ) );
}

//------------------------------------------------------------------------

sal_Bool SvxGridTabPage::FillItemSet( SfxItemSet& rCoreSet )
{
    if ( bAttrModified )
    {
        SvxGridItem aGridItem( SID_ATTR_GRID_OPTIONS );

        aGridItem.bUseGridsnap  = aCbxUseGridsnap.IsChecked();
        aGridItem.bSynchronize  = aCbxSynchronize.IsChecked();
        aGridItem.bGridVisible  = aCbxGridVisible.IsChecked();

        SfxMapUnit eUnit =
            rCoreSet.GetPool()->GetMetric( GetWhich( SID_ATTR_GRID_OPTIONS ) );
        long nX =GetCoreValue(  aMtrFldDrawX, eUnit );
        long nY = GetCoreValue( aMtrFldDrawY, eUnit );

        aGridItem.nFldDrawX    = (sal_uInt32) nX;
        aGridItem.nFldDrawY    = (sal_uInt32) nY;
        aGridItem.nFldDivisionX = static_cast<long>(aNumFldDivisionX.GetValue()-1);
        aGridItem.nFldDivisionY = static_cast<long>(aNumFldDivisionY.GetValue()-1);

        rCoreSet.Put( aGridItem );
    }
    return bAttrModified;
}

//------------------------------------------------------------------------

void SvxGridTabPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem* pAttr = 0;

    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_GRID_OPTIONS , sal_False,
                                    (const SfxPoolItem**)&pAttr ))
    {
        const SvxGridItem* pGridAttr = (SvxGridItem*)pAttr;
        aCbxUseGridsnap.Check( pGridAttr->bUseGridsnap == 1 );
        aCbxSynchronize.Check( pGridAttr->bSynchronize == 1 );
        aCbxGridVisible.Check( pGridAttr->bGridVisible == 1 );

        SfxMapUnit eUnit =
            rSet.GetPool()->GetMetric( GetWhich( SID_ATTR_GRID_OPTIONS ) );
        SetMetricValue( aMtrFldDrawX , pGridAttr->nFldDrawX, eUnit );
        SetMetricValue( aMtrFldDrawY , pGridAttr->nFldDrawY, eUnit );

        aNumFldDivisionX.SetValue( pGridAttr->nFldDivisionX+1 );
        aNumFldDivisionY.SetValue( pGridAttr->nFldDivisionY+1 );
    }

    ChangeGridsnapHdl_Impl( &aCbxUseGridsnap );
    bAttrModified = sal_False;
}

// -----------------------------------------------------------------------

void SvxGridTabPage::ActivatePage( const SfxItemSet& rSet )
{
    const SfxPoolItem* pAttr = NULL;
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_GRID_OPTIONS , sal_False,
                                    (const SfxPoolItem**)&pAttr ))
    {
        const SvxGridItem* pGridAttr = (SvxGridItem*) pAttr;
        aCbxUseGridsnap.Check( pGridAttr->bUseGridsnap == 1 );

        ChangeGridsnapHdl_Impl( &aCbxUseGridsnap );
    }

    // Metric Change if necessary (as TabPage is in the dialog, where the
    // metric can be set
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_METRIC , sal_False,
                                    (const SfxPoolItem**)&pAttr ))
    {
        const SfxUInt16Item* pItem = (SfxUInt16Item*) pAttr;

        FieldUnit eFUnit = (FieldUnit)(long)pItem->GetValue();

        if( eFUnit != aMtrFldDrawX.GetUnit() )
        {
            // Set Metrics
            long nFirst, nLast, nMin, nMax;
            long nVal = static_cast<long>(aMtrFldDrawX.Denormalize( aMtrFldDrawX.GetValue( FUNIT_TWIP ) ));

            lcl_GetMinMax(aMtrFldDrawX, nFirst, nLast, nMin, nMax);
            SetFieldUnit( aMtrFldDrawX, eFUnit, sal_True );
            lcl_SetMinMax(aMtrFldDrawX, nFirst, nLast, nMin, nMax);

            aMtrFldDrawX.SetValue( aMtrFldDrawX.Normalize( nVal ), FUNIT_TWIP );

            nVal = static_cast<long>(aMtrFldDrawY.Denormalize( aMtrFldDrawY.GetValue( FUNIT_TWIP ) ));
            lcl_GetMinMax(aMtrFldDrawY, nFirst, nLast, nMin, nMax);
            SetFieldUnit( aMtrFldDrawY, eFUnit, sal_True );
            lcl_SetMinMax(aMtrFldDrawY, nFirst, nLast, nMin, nMax);
            aMtrFldDrawY.SetValue( aMtrFldDrawY.Normalize( nVal ), FUNIT_TWIP );

        }
    }
}

// -----------------------------------------------------------------------
int SvxGridTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( *_pSet );
    return( LEAVE_PAGE );
}
//------------------------------------------------------------------------
IMPL_LINK( SvxGridTabPage, ChangeDrawHdl_Impl, MetricField *, pField )
{
    bAttrModified = sal_True;
    if( aCbxSynchronize.IsChecked() )
    {
        if(pField == &aMtrFldDrawX)
            aMtrFldDrawY.SetValue( aMtrFldDrawX.GetValue() );
        else
            aMtrFldDrawX.SetValue( aMtrFldDrawY.GetValue() );
    }
    return 0;
}
//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxGridTabPage, ClickRotateHdl_Impl)
{
    if( aCbxRotate.IsChecked() )
        aMtrFldAngle.Enable();
    else
        aMtrFldAngle.Disable();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxGridTabPage, ChangeDivisionHdl_Impl, NumericField *, pField )
{
    bAttrModified = sal_True;
    if( aCbxSynchronize.IsChecked() )
    {
        if(&aNumFldDivisionX == pField)
            aNumFldDivisionY.SetValue( aNumFldDivisionX.GetValue() );
        else
            aNumFldDivisionX.SetValue( aNumFldDivisionY.GetValue() );
    }
    return 0;
}
//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxGridTabPage, ChangeGridsnapHdl_Impl)
{
    bAttrModified = sal_True;
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

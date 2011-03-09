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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cui.hxx"

#include <tools/shl.hxx>
#include <svl/eitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <sfx2/sfxsids.hrc>
#include <dialmgr.hxx>
#include <svx/dlgutil.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/brshitem.hxx>
#include <grfpage.hxx>
#include <svx/grfcrop.hxx>
#include <grfpage.hrc>
#include <cuires.hrc>
#include <svx/dialogs.hrc> // for RID_SVXPAGE_GRFCROP

#define CM_1_TO_TWIP        567


inline long lcl_GetValue( MetricField& rMetric, FieldUnit eUnit )
{
    return static_cast<long>(rMetric.Denormalize( rMetric.GetValue( eUnit )));
}

/*--------------------------------------------------------------------
    Beschreibung: Grafik zuschneiden
 --------------------------------------------------------------------*/

SvxGrfCropPage::SvxGrfCropPage ( Window *pParent, const SfxItemSet &rSet )
    : SfxTabPage( pParent,  CUI_RES( RID_SVXPAGE_GRFCROP ), rSet ),
    aCropFL(        this, CUI_RES( FL_CROP    )),
    aZoomConstRB(   this, CUI_RES( RB_ZOOMCONST)),
    aSizeConstRB(   this, CUI_RES( RB_SIZECONST)),
    aLeftFT(        this, CUI_RES( FT_LEFT    )),
    aLeftMF(        this, CUI_RES( MF_LEFT    )),
    aRightFT(       this, CUI_RES( FT_RIGHT   )),
    aRightMF(       this, CUI_RES( MF_RIGHT   )),
    aTopFT(         this, CUI_RES( FT_TOP     )),
    aTopMF(         this, CUI_RES( MF_TOP     )),
    aBottomFT(      this, CUI_RES( FT_BOTTOM  )),
    aBottomMF(      this, CUI_RES( MF_BOTTOM  )),
    aZoomFL(        this, CUI_RES( FL_ZOOM    )),
    aWidthZoomFT(   this, CUI_RES( FT_WIDTHZOOM )),
    aWidthZoomMF(   this, CUI_RES( MF_WIDTHZOOM )),
    aHeightZoomFT(  this, CUI_RES( FT_HEIGHTZOOM)),
    aHeightZoomMF(  this, CUI_RES( MF_HEIGHTZOOM)),
    aSizeFL(        this, CUI_RES( FL_SIZE    )),
    aWidthFT(       this, CUI_RES( FT_WIDTH   )),
    aWidthMF(       this, CUI_RES( MF_WIDTH   )),
    aHeightFT(      this, CUI_RES( FT_HEIGHT  )),
    aHeightMF(      this, CUI_RES( MF_HEIGHT  )),
    aOrigSizeFT(    this, CUI_RES(FT_ORIG_SIZE)),
    aOrigSizePB(    this, CUI_RES( PB_ORGSIZE )),
    aExampleWN(     this, CUI_RES( WN_BSP     )),
    pLastCropField(0),
    bInitialized(sal_False),
    bSetOrigSize(sal_False)
{
    FreeResource();

    SetExchangeSupport();

    // set the correct metric
    const FieldUnit eMetric = GetModuleFieldUnit( rSet );

    SetFieldUnit( aWidthMF, eMetric );
    SetFieldUnit( aHeightMF, eMetric );
    SetFieldUnit( aLeftMF, eMetric );
    SetFieldUnit( aRightMF, eMetric );
    SetFieldUnit( aTopMF , eMetric );
    SetFieldUnit( aBottomMF, eMetric );

    Link aLk = LINK(this, SvxGrfCropPage, SizeHdl);
    aWidthMF.SetModifyHdl( aLk );
    aHeightMF.SetModifyHdl( aLk );

    aLk = LINK(this, SvxGrfCropPage, ZoomHdl);
    aWidthZoomMF.SetModifyHdl( aLk );
    aHeightZoomMF.SetModifyHdl( aLk );

    aLk = LINK(this, SvxGrfCropPage, CropHdl);
    aLeftMF.SetDownHdl( aLk );
    aRightMF.SetDownHdl( aLk );
    aTopMF.SetDownHdl( aLk );
    aBottomMF.SetDownHdl( aLk );
    aLeftMF.SetUpHdl( aLk );
    aRightMF.SetUpHdl( aLk );
    aTopMF.SetUpHdl( aLk );
    aBottomMF.SetUpHdl( aLk );

    aLk = LINK(this, SvxGrfCropPage, CropModifyHdl);
    aLeftMF.SetModifyHdl( aLk );
    aRightMF.SetModifyHdl( aLk );
    aTopMF.SetModifyHdl( aLk );
    aBottomMF.SetModifyHdl( aLk );

    aLk = LINK(this, SvxGrfCropPage, CropLoseFocusHdl);
    aLeftMF.SetLoseFocusHdl( aLk );
    aRightMF.SetLoseFocusHdl( aLk );
    aTopMF.SetLoseFocusHdl( aLk );
    aBottomMF.SetLoseFocusHdl( aLk );

    aLk = LINK(this, SvxGrfCropPage, OrigSizeHdl);
    aOrigSizePB.SetClickHdl( aLk );

    aTimer.SetTimeoutHdl(LINK(this, SvxGrfCropPage, Timeout));
    aTimer.SetTimeout( 1500 );

    aOrigSizePB.SetAccessibleRelationLabeledBy( &aOrigSizeFT );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SvxGrfCropPage::~SvxGrfCropPage()
{
    aTimer.Stop();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SfxTabPage* SvxGrfCropPage::Create(Window *pParent, const SfxItemSet &rSet)
{
    return new SvxGrfCropPage( pParent, rSet );
}
/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SvxGrfCropPage::Reset( const SfxItemSet &rSet )
{
    const SfxPoolItem* pItem;
    const SfxItemPool& rPool = *rSet.GetPool();

    if(SFX_ITEM_SET == rSet.GetItemState( rPool.GetWhich(
                                    SID_ATTR_GRAF_KEEP_ZOOM ), sal_True, &pItem ))
    {
        if( ((const SfxBoolItem*)pItem)->GetValue() )
            aZoomConstRB.Check();
        else
            aSizeConstRB.Check();
        aZoomConstRB.SaveValue();
    }

    sal_uInt16 nW = rPool.GetWhich( SID_ATTR_GRAF_CROP );
    if( SFX_ITEM_SET == rSet.GetItemState( nW, sal_True, &pItem))
    {
        FieldUnit eUnit = MapToFieldUnit( rSet.GetPool()->GetMetric( nW ));

        SvxGrfCrop* pCrop =  (SvxGrfCrop*)pItem;

        aExampleWN.SetLeft(     pCrop->GetLeft());
        aExampleWN.SetRight(    pCrop->GetRight());
        aExampleWN.SetTop(      pCrop->GetTop());
        aExampleWN.SetBottom(   pCrop->GetBottom());

        aLeftMF.SetValue( aLeftMF.Normalize( pCrop->GetLeft()), eUnit );
        aRightMF.SetValue( aRightMF.Normalize( pCrop->GetRight()), eUnit );
        aTopMF.SetValue( aTopMF.Normalize( pCrop->GetTop()), eUnit );
        aBottomMF.SetValue( aBottomMF.Normalize( pCrop->GetBottom()), eUnit );
    }
    else
    {
        aLeftMF.SetValue( 0 );
        aRightMF.SetValue( 0 );
        aTopMF.SetValue( 0 );
        aBottomMF.SetValue( 0 );
    }

    nW = rPool.GetWhich( SID_ATTR_PAGE_SIZE );
    if ( SFX_ITEM_SET == rSet.GetItemState( nW, sal_False, &pItem ) )
    {
        // Orientation und Size aus dem PageItem
        FieldUnit eUnit = MapToFieldUnit( rSet.GetPool()->GetMetric( nW ));

        aPageSize = ((const SvxSizeItem*)pItem)->GetSize();

        sal_Int64 nTmp = aHeightMF.Normalize(aPageSize.Height());
        aHeightMF.SetMax( nTmp, eUnit );
        nTmp = aWidthMF.Normalize(aPageSize.Width());
        aWidthMF.SetMax( nTmp, eUnit );
        nTmp = aWidthMF.Normalize( 23 );
        aHeightMF.SetMin( nTmp, eUnit );
        aWidthMF.SetMin( nTmp, eUnit );
    }
    else
    {
        aPageSize = OutputDevice::LogicToLogic(
                        Size( CM_1_TO_TWIP,  CM_1_TO_TWIP ),
                        MapMode( MAP_TWIP ),
                        MapMode( (MapUnit)rSet.GetPool()->GetMetric( nW ) ) );
    }

    sal_Bool bFound = sal_False;
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_GRAF_GRAPHIC, sal_False, &pItem ) )
    {
        const Graphic* pGrf = ((SvxBrushItem*)pItem)->GetGraphic();
        if( pGrf )
            aOrigSize = GetGrfOrigSize( *pGrf );

        if( aOrigSize.Width() && aOrigSize.Height() )
        {
            CalcMinMaxBorder();
            aExampleWN.SetGraphic( *pGrf );
            aExampleWN.SetFrameSize( aOrigSize );

            bFound = sal_True;
            if( ((SvxBrushItem*)pItem)->GetGraphicLink() )
                aGraphicName = *((SvxBrushItem*)pItem)->GetGraphicLink();
        }
    }

    GraphicHasChanged( bFound );
    bReset = sal_True;
    ActivatePage( rSet );
    bReset = sal_False;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

sal_Bool SvxGrfCropPage::FillItemSet(SfxItemSet &rSet)
{
    const SfxItemPool& rPool = *rSet.GetPool();
    sal_Bool bModified = sal_False;
    if( aWidthMF.GetSavedValue() != aWidthMF.GetText() ||
        aHeightMF.GetSavedValue() != aHeightMF.GetText() )
    {
        sal_uInt16 nW = rPool.GetWhich( SID_ATTR_GRAF_FRMSIZE );
        FieldUnit eUnit = MapToFieldUnit( rSet.GetPool()->GetMetric( nW ));

        SvxSizeItem aSz( nW );

        // die Groesse koennte schon von einer anderen Page gesetzt worden sein
        // #44204#
        const SfxItemSet* pExSet = GetTabDialog() ? GetTabDialog()->GetExampleSet() : NULL;
        const SfxPoolItem* pItem = 0;
        if( pExSet && SFX_ITEM_SET ==
                pExSet->GetItemState( nW, sal_False, &pItem ) )
            aSz = *(const SvxSizeItem*)pItem;
        else
            aSz = (const SvxSizeItem&)GetItemSet().Get( nW );

        Size aTmpSz( aSz.GetSize() );
        if( aWidthMF.GetText() != aWidthMF.GetSavedValue() )
            aTmpSz.Width() = lcl_GetValue( aWidthMF, eUnit );
        if( aHeightMF.GetText() != aHeightMF.GetSavedValue() )
            aTmpSz.Height() = lcl_GetValue( aHeightMF, eUnit );
        aSz.SetSize( aTmpSz );
        aWidthMF.SaveValue();
        aHeightMF.SaveValue();

        bModified |= 0 != rSet.Put( aSz );

        if( bSetOrigSize )
        {
            bModified |= 0 != rSet.Put( SvxSizeItem( rPool.GetWhich(
                        SID_ATTR_GRAF_FRMSIZE_PERCENT ), Size( 0, 0 )) );
        }
    }
    if( aLeftMF.IsModified() || aRightMF.IsModified() ||
        aTopMF.IsModified()  || aBottomMF.IsModified() )
    {
        sal_uInt16 nW = rPool.GetWhich( SID_ATTR_GRAF_CROP );
        FieldUnit eUnit = MapToFieldUnit( rSet.GetPool()->GetMetric( nW ));
        SvxGrfCrop* pNew = (SvxGrfCrop*)rSet.Get( nW ).Clone();

        pNew->SetLeft( lcl_GetValue( aLeftMF, eUnit ) );
        pNew->SetRight( lcl_GetValue( aRightMF, eUnit ) );
        pNew->SetTop( lcl_GetValue( aTopMF, eUnit ) );
        pNew->SetBottom( lcl_GetValue( aBottomMF, eUnit ) );
        bModified |= 0 != rSet.Put( *pNew );
        delete pNew;
    }

    if( aZoomConstRB.GetSavedValue() != aZoomConstRB.IsChecked() )
    {
        bModified |= 0 != rSet.Put( SfxBoolItem( rPool.GetWhich(
                    SID_ATTR_GRAF_KEEP_ZOOM), aZoomConstRB.IsChecked() ) );
    }

    bInitialized = sal_False;

    return bModified;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SvxGrfCropPage::ActivatePage(const SfxItemSet& rSet)
{
#ifdef DBG_UTIL
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
#endif

    bSetOrigSize = sal_False;

    // Size
    Size aSize;
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_GRAF_FRMSIZE, sal_False, &pItem ) )
        aSize = ((const SvxSizeItem*)pItem)->GetSize();

    nOldWidth = aSize.Width();
    nOldHeight = aSize.Height();

    sal_Int64 nWidth = aWidthMF.Normalize(nOldWidth);
    sal_Int64 nHeight = aHeightMF.Normalize(nOldHeight);

    if (nWidth != aWidthMF.GetValue(FUNIT_TWIP))
    {
        if(!bReset)
        {
            // Wert wurde von Umlauf-Tabpage geaendert und muss
            // mit Modify-Flag gesetzt werden
            aWidthMF.SetUserValue(nWidth, FUNIT_TWIP);
        }
        else
            aWidthMF.SetValue(nWidth, FUNIT_TWIP);
    }
    aWidthMF.SaveValue();

    if (nHeight != aHeightMF.GetValue(FUNIT_TWIP))
    {
        if (!bReset)
        {
            // Wert wurde von Umlauf-Tabpage geaendert und muss
            // mit Modify-Flag gesetzt werden
            aHeightMF.SetUserValue(nHeight, FUNIT_TWIP);
        }
        else
            aHeightMF.SetValue(nHeight, FUNIT_TWIP);
    }
    aHeightMF.SaveValue();
    bInitialized = sal_True;

    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_GRAF_GRAPHIC, sal_False, &pItem ) )
    {
        const SvxBrushItem& rBrush = *(SvxBrushItem*)pItem;
        if( rBrush.GetGraphicLink() &&
            aGraphicName != *rBrush.GetGraphicLink() )
            aGraphicName = *rBrush.GetGraphicLink();

        const Graphic* pGrf = rBrush.GetGraphic();
        if( pGrf )
        {
            aExampleWN.SetGraphic( *pGrf );
            aOrigSize = GetGrfOrigSize( *pGrf );
            aExampleWN.SetFrameSize(aOrigSize);
            GraphicHasChanged( aOrigSize.Width() && aOrigSize.Height() );
            CalcMinMaxBorder();
        }
        else
            GraphicHasChanged( sal_False );
    }

    CalcZoom();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

int SvxGrfCropPage::DeactivatePage(SfxItemSet *_pSet)
{
    if ( _pSet )
        FillItemSet( *_pSet );
    return sal_True;
}

/*--------------------------------------------------------------------
    Beschreibung: Massstab geaendert, Groesse anpassen
 --------------------------------------------------------------------*/

IMPL_LINK( SvxGrfCropPage, ZoomHdl, MetricField *, pField )
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ) );

    if( pField == &aWidthZoomMF )
    {
        long nLRBorders = lcl_GetValue(aLeftMF, eUnit)
                         +lcl_GetValue(aRightMF, eUnit);
        aWidthMF.SetValue( aWidthMF.Normalize(
            ((aOrigSize.Width() - nLRBorders) * pField->GetValue())/100L),
            eUnit);
    }
    else
    {
        long nULBorders = lcl_GetValue(aTopMF, eUnit)
                         +lcl_GetValue(aBottomMF, eUnit);
        aHeightMF.SetValue( aHeightMF.Normalize(
            ((aOrigSize.Height() - nULBorders ) * pField->GetValue())/100L) ,
            eUnit );
    }

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung: Groesse aendern, Massstab anpassen
 --------------------------------------------------------------------*/

IMPL_LINK( SvxGrfCropPage, SizeHdl, MetricField *, pField )
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ) );

    Size aSize( lcl_GetValue(aWidthMF, eUnit),
                lcl_GetValue(aHeightMF, eUnit) );

    if(pField == &aWidthMF)
    {
        long nWidth = aOrigSize.Width() -
                ( lcl_GetValue(aLeftMF, eUnit) +
                  lcl_GetValue(aRightMF, eUnit) );
        if(!nWidth)
            nWidth++;
        sal_uInt16 nZoom = (sal_uInt16)( aSize.Width() * 100L / nWidth);
        aWidthZoomMF.SetValue(nZoom);
    }
    else
    {
        long nHeight = aOrigSize.Height() -
                ( lcl_GetValue(aTopMF, eUnit) +
                  lcl_GetValue(aBottomMF, eUnit));
        if(!nHeight)
            nHeight++;
        sal_uInt16 nZoom = (sal_uInt16)( aSize.Height() * 100L/ nHeight);
        aHeightZoomMF.SetValue(nZoom);
    }

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung: Raender auswerten
 --------------------------------------------------------------------*/

IMPL_LINK( SvxGrfCropPage, CropHdl, const MetricField *, pField )
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ) );

    sal_Bool bZoom = aZoomConstRB.IsChecked();
    if( pField == &aLeftMF || pField == &aRightMF )
    {
        long nLeft = lcl_GetValue( aLeftMF, eUnit );
        long nRight = lcl_GetValue( aRightMF, eUnit );
        long nWidthZoom = static_cast<long>(aWidthZoomMF.GetValue());
        if(bZoom && ( ( ( aOrigSize.Width() - (nLeft + nRight )) * nWidthZoom )
                            / 100 >= aPageSize.Width() ) )
        {
            if(pField == &aLeftMF)
            {
//              nLeft = aPageSize.Width() -
//                  ((nRight + aOrigSize.Width()) * nWidthZoom) / 100;
                nLeft = aOrigSize.Width() -
                            ( aPageSize.Width() * 100 / nWidthZoom + nRight );
                aLeftMF.SetValue( aLeftMF.Normalize( nLeft ), eUnit );
            }
            else
            {
//              nRight = aPageSize.Width() -
//                  ((nLeft - aOrigSize.Width()) * nWidthZoom) / 100;
                nRight = aOrigSize.Width() -
                            ( aPageSize.Width() * 100 / nWidthZoom + nLeft );
                aRightMF.SetValue( aRightMF.Normalize( nRight ), eUnit );
            }
        }
        aExampleWN.SetLeft(nLeft);
        aExampleWN.SetRight(nRight);
        if(bZoom)
        {
            //Massstab bleibt -> Breite neu berechnen
            ZoomHdl(&aWidthZoomMF);
        }
    }
    else
    {
        long nTop = lcl_GetValue( aTopMF, eUnit );
        long nBottom = lcl_GetValue( aBottomMF, eUnit );
        long nHeightZoom = static_cast<long>(aHeightZoomMF.GetValue());
        if(bZoom && ( ( ( aOrigSize.Height() - (nTop + nBottom )) * nHeightZoom)
                                            / 100 >= aPageSize.Height()))
        {
            if(pField == &aTopMF)
            {
//              nTop = aPageSize.Height() -
//                  ((aOrigSize.Height() - nBottom) * nHeightZoom)/ 100;
                nTop = aOrigSize.Height() -
                            ( aPageSize.Height() * 100 / nHeightZoom + nBottom);
                aTopMF.SetValue( aWidthMF.Normalize( nTop ), eUnit );
            }
            else
            {
//              nBottom = aPageSize.Height() -
//                  ((aOrigSize.Height() - nTop)*nHeightZoom) / 100;
                nBottom = aOrigSize.Height() -
                            ( aPageSize.Height() * 100 / nHeightZoom + nTop);
                aBottomMF.SetValue( aWidthMF.Normalize( nBottom ), eUnit );
            }
        }
        aExampleWN.SetTop( nTop );
        aExampleWN.SetBottom( nBottom );
        if(bZoom)
        {
            //Massstab bleibt -> Hoehe neu berechnen
            ZoomHdl(&aHeightZoomMF);
        }
    }
    aExampleWN.Invalidate();
    //Groesse und Raender veraendert -> Massstab neu berechnen
    if(!bZoom)
        CalcZoom();
    CalcMinMaxBorder();
    return 0;
}
/*--------------------------------------------------------------------
    Beschreibung: Originalgroesse einstellen
 --------------------------------------------------------------------*/

IMPL_LINK( SvxGrfCropPage, OrigSizeHdl, PushButton *, EMPTYARG )
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ) );

    long nWidth = aOrigSize.Width() -
        lcl_GetValue( aLeftMF, eUnit ) -
        lcl_GetValue( aRightMF, eUnit );
    aWidthMF.SetValue( aWidthMF.Normalize( nWidth ), eUnit );
    long nHeight = aOrigSize.Height() -
        lcl_GetValue( aTopMF, eUnit ) -
        lcl_GetValue( aBottomMF, eUnit );
    aHeightMF.SetValue( aHeightMF.Normalize( nHeight ), eUnit );
    aWidthZoomMF.SetValue(100);
    aHeightZoomMF.SetValue(100);
    bSetOrigSize = sal_True;
    return 0;
}
/*--------------------------------------------------------------------
    Beschreibung: Massstab berechnen
 --------------------------------------------------------------------*/

void SvxGrfCropPage::CalcZoom()
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ) );

    long nWidth = lcl_GetValue( aWidthMF, eUnit );
    long nHeight = lcl_GetValue( aHeightMF, eUnit );
    long nLRBorders = lcl_GetValue( aLeftMF, eUnit ) +
                      lcl_GetValue( aRightMF, eUnit );
    long nULBorders = lcl_GetValue( aTopMF, eUnit ) +
                      lcl_GetValue( aBottomMF, eUnit );
    sal_uInt16 nZoom = 0;
    long nDen;
    if( (nDen = aOrigSize.Width() - nLRBorders) > 0)
        nZoom = (sal_uInt16)((( nWidth  * 1000L / nDen )+5)/10);
    aWidthZoomMF.SetValue(nZoom);
    if( (nDen = aOrigSize.Height() - nULBorders) > 0)
        nZoom = (sal_uInt16)((( nHeight * 1000L / nDen )+5)/10);
    else
        nZoom = 0;
    aHeightZoomMF.SetValue(nZoom);
}

/*--------------------------------------------------------------------
    Beschreibung: Minimal-/Maximalwerte fuer die Raender setzen
 --------------------------------------------------------------------*/

void SvxGrfCropPage::CalcMinMaxBorder()
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ) );
    long nR = lcl_GetValue(aRightMF, eUnit );
    long nMinWidth = (aOrigSize.Width() * 10) /11;
    long nMin = nMinWidth - (nR >= 0 ? nR : 0);
    aLeftMF.SetMax( aLeftMF.Normalize(nMin), eUnit );

    long nL = lcl_GetValue(aLeftMF, eUnit );
    nMin = nMinWidth - (nL >= 0 ? nL : 0);
    aRightMF.SetMax( aRightMF.Normalize(nMin), eUnit );

    // Zoom nicht unter 2%
/*  nMin = (aOrigSize.Width() * 102) /100;
    aLeftMF.SetMax(aPageSize.Width() - nR - nMin);
    aRightMF.SetMax(aPageSize.Width() - nL - nMin);
*/
    long nUp  = lcl_GetValue( aTopMF, eUnit );
    long nMinHeight = (aOrigSize.Height() * 10) /11;
    nMin = nMinHeight - (nUp >= 0 ? nUp : 0);
    aBottomMF.SetMax( aBottomMF.Normalize(nMin), eUnit );

    long nLow = lcl_GetValue(aBottomMF, eUnit );
    nMin = nMinHeight - (nLow >= 0 ? nLow : 0);
    aTopMF.SetMax( aTopMF.Normalize(nMin), eUnit );

    // Zoom nicht unter 2%
/*  nMin = (aOrigSize.Height() * 102) /100;
    aTopMF.SetMax(aPageSize.Height() - nLow - nMin);
    aBottomMF.SetMax(aPageSize.Height() - nUp - nMin);*/
}
/*--------------------------------------------------------------------
    Beschreibung:   Spinsize auf 1/20 der Originalgroesse setzen,
                    FixedText mit der Originalgroesse fuellen
 --------------------------------------------------------------------*/

void SvxGrfCropPage::GraphicHasChanged( sal_Bool bFound )
{
    if( bFound )
    {
        SfxItemPool* pPool = GetItemSet().GetPool();
        DBG_ASSERT( pPool, "Wo ist der Pool" );
        FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ));

        sal_Int64 nSpin = aLeftMF.Normalize(aOrigSize.Width()) / 20;
        nSpin = MetricField::ConvertValue( nSpin, aOrigSize.Width(), 0,
                                               eUnit, aLeftMF.GetUnit());

        // Ist der Rand zu gross, wird er auf beiden Seiten auf 1/3 eingestellt.
        long nR = lcl_GetValue( aRightMF, eUnit );
        long nL = lcl_GetValue( aLeftMF, eUnit );
        if((nL + nR) < - aOrigSize.Width())
        {
            long nVal = aOrigSize.Width() / -3;
            aRightMF.SetValue( aRightMF.Normalize( nVal ), eUnit );
            aLeftMF.SetValue( aLeftMF.Normalize( nVal ), eUnit );
            aExampleWN.SetLeft(nVal);
            aExampleWN.SetRight(nVal);
        }
        long nUp  = lcl_GetValue(aTopMF, eUnit );
        long nLow = lcl_GetValue(aBottomMF, eUnit );
        if((nUp + nLow) < - aOrigSize.Height())
        {
            long nVal = aOrigSize.Height() / -3;
            aTopMF.SetValue( aTopMF.Normalize( nVal ), eUnit );
            aBottomMF.SetValue( aBottomMF.Normalize( nVal ), eUnit );
            aExampleWN.SetTop(nVal);
            aExampleWN.SetBottom(nVal);
        }

        aLeftMF.SetSpinSize(nSpin);
        aRightMF.SetSpinSize(nSpin);
        nSpin = aTopMF.Normalize(aOrigSize.Height()) / 20;
        nSpin = MetricField::ConvertValue( nSpin, aOrigSize.Width(), 0,
                                               eUnit, aLeftMF.GetUnit() );
        aTopMF.SetSpinSize(nSpin);
        aBottomMF.SetSpinSize(nSpin);

        //Originalgroesse anzeigen
        const FieldUnit eMetric = GetModuleFieldUnit( GetItemSet() );

        MetricField aFld(this, WB_HIDE);
        SetFieldUnit( aFld, eMetric );
        aFld.SetDecimalDigits( aWidthMF.GetDecimalDigits() );
        aFld.SetMax( LONG_MAX - 1 );

        aFld.SetValue( aFld.Normalize( aOrigSize.Width() ), eUnit );
        String sTemp = aFld.GetText();
        aFld.SetValue( aFld.Normalize( aOrigSize.Height() ), eUnit );
        sTemp += UniString::CreateFromAscii(" x ");
        sTemp += aFld.GetText();
        aOrigSizeFT.SetText(sTemp);
    }
    aLeftFT         .Enable(bFound);
    aLeftMF         .Enable(bFound);
    aRightFT        .Enable(bFound);
    aRightMF        .Enable(bFound);
    aTopFT          .Enable(bFound);
    aTopMF          .Enable(bFound);
    aBottomFT       .Enable(bFound);
    aBottomMF       .Enable(bFound);
    aSizeConstRB    .Enable(bFound);
    aZoomConstRB    .Enable(bFound);
    aWidthFT        .Enable(bFound);
    aWidthMF        .Enable(bFound);
    aHeightFT       .Enable(bFound);
    aHeightMF       .Enable(bFound);
    aWidthZoomFT    .Enable(bFound);
    aWidthZoomMF    .Enable(bFound);
    aHeightZoomFT   .Enable(bFound);
    aHeightZoomMF   .Enable(bFound);
    aExampleWN      .Enable(bFound);
    aOrigSizePB     .Enable(bFound);
    aOrigSizeFT     .Enable(bFound);
}

IMPL_LINK( SvxGrfCropPage, Timeout, Timer *, EMPTYARG )
{
    DBG_ASSERT(pLastCropField,"Timeout ohne Feld?");
    CropHdl(pLastCropField);
    pLastCropField = 0;
    return 0;
}


IMPL_LINK( SvxGrfCropPage, CropLoseFocusHdl, MetricField*, pField )
{
    aTimer.Stop();
    CropHdl(pField);
    pLastCropField = 0;
    return 0;
}


IMPL_LINK( SvxGrfCropPage, CropModifyHdl, MetricField *, pField )
{
    aTimer.Start();
    pLastCropField = pField;
    return 0;
}

Size SvxGrfCropPage::GetGrfOrigSize( const Graphic& rGrf ) const
{
    const MapMode aMapTwip( MAP_TWIP );
    Size aSize( rGrf.GetPrefSize() );
    if( MAP_PIXEL == rGrf.GetPrefMapMode().GetMapUnit() )
        aSize = PixelToLogic( aSize, aMapTwip );
    else
        aSize = OutputDevice::LogicToLogic( aSize,
                                        rGrf.GetPrefMapMode(), aMapTwip );
    return aSize;
}

/*****************************************************************/

SvxGrfCropPage::SvxCropExample::SvxCropExample( Window* pPar,
                                                const ResId& rResId )
    : Window( pPar, rResId ),
    aFrameSize( OutputDevice::LogicToLogic(
                            Size( CM_1_TO_TWIP / 2, CM_1_TO_TWIP / 2 ),
                            MapMode( MAP_TWIP ), GetMapMode() )),
    aTopLeft(0,0), aBottomRight(0,0)
{
    SetBorderStyle( WINDOW_BORDER_MONO );
}

void SvxGrfCropPage::SvxCropExample::Paint( const Rectangle& )
{
    Size aWinSize( PixelToLogic(GetOutputSizePixel() ));
    SetLineColor();
    SetFillColor( GetSettings().GetStyleSettings().GetWindowColor() );
    SetRasterOp( ROP_OVERPAINT );
    DrawRect( Rectangle( Point(), aWinSize ) );

    SetLineColor( Color( COL_WHITE ) );
    Rectangle aRect(Point((aWinSize.Width() - aFrameSize.Width())/2,
                          (aWinSize.Height() - aFrameSize.Height())/2),
                          aFrameSize );
    aGrf.Draw( this,  aRect.TopLeft(), aRect.GetSize() );

    Size aSz( 2, 0 );
    aSz = PixelToLogic( aSz );
    SetFillColor( Color( COL_TRANSPARENT ) );
    SetRasterOp( ROP_INVERT );
    aRect.Left()    += aTopLeft.Y();
    aRect.Top()     += aTopLeft.X();
    aRect.Right()   -= aBottomRight.Y();
    aRect.Bottom()  -= aBottomRight.X();
    DrawRect( aRect );
}

void SvxGrfCropPage::SvxCropExample::SetFrameSize( const Size& rSz )
{
    aFrameSize = rSz;
    if(!aFrameSize.Width())
        aFrameSize.Width() = 1;
    if(!aFrameSize.Height())
        aFrameSize.Height() = 1;
    Size aWinSize( GetOutputSizePixel() );
    Fraction aXScale( aWinSize.Width() * 4, aFrameSize.Width() * 5 );
    Fraction aYScale( aWinSize.Height() * 4, aFrameSize.Height() * 5 );

    if( aYScale < aXScale )
        aXScale = aYScale;

    MapMode aMapMode( GetMapMode() );

    aMapMode.SetScaleX( aXScale );
    aMapMode.SetScaleY( aXScale );

    SetMapMode( aMapMode );
    Invalidate();
}





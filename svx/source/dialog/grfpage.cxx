/*************************************************************************
 *
 *  $RCSfile: grfpage.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: pb $ $Date: 2000-10-23 09:31:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#define ITEMID_GRF_CROP         0
#define ITEMID_SIZE             0
#define ITEMID_BRUSH            0

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXMODULE_HXX //autogen
#include <sfx2/module.hxx>
#endif

#ifndef _SFXSIDS_HRC
#include <sfx2/sfxsids.hrc>
#endif


#ifndef _SVX_DIALMGR_HXX //autogen
#include <dialmgr.hxx>
#endif
#ifndef _SVX_DLGUTIL_HXX
#include <dlgutil.hxx>
#endif
#ifndef _SVX_SIZEITEM_HXX //autogen
#include <sizeitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <brshitem.hxx>
#endif

#ifndef _GRFPAGE_HXX
#include <grfpage.hxx>
#endif
#ifndef _SVX_GRFCROP_HXX
#include <grfcrop.hxx>
#endif

#ifndef _GRFPAGE_HRC
#include <grfpage.hrc>
#endif
#ifndef _SVX_DIALOGS_HRC
#include <dialogs.hrc>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svxids.hrc>
#endif


#define CM_1_TO_TWIP        567


inline long lcl_GetValue( MetricField& rMetric, FieldUnit eUnit )
{
    return rMetric.Denormalize( rMetric.GetValue( eUnit ));
}

/*--------------------------------------------------------------------
    Beschreibung: Grafik zuschneiden
 --------------------------------------------------------------------*/

SvxGrfCropPage::SvxGrfCropPage ( Window *pParent, const SfxItemSet &rSet )
    : SfxTabPage( pParent,  SVX_RES( RID_SVXPAGE_GRFCROP ), rSet ),
    aSizeGB(        this, ResId( GB_SIZE    )),
    aWidthFT(       this, ResId( FT_WIDTH   )),
    aWidthMF(       this, ResId( MF_WIDTH   )),
    aHeightFT(      this, ResId( FT_HEIGHT  )),
    aHeightMF(      this, ResId( MF_HEIGHT  )),
    aZoomGB(        this, ResId( GB_ZOOM    )),
    aWidthZoomFT(   this, ResId( FT_WIDTHZOOM )),
    aWidthZoomMF(   this, ResId( MF_WIDTHZOOM )),
    aHeightZoomFT(  this, ResId( FT_HEIGHTZOOM)),
    aHeightZoomMF(  this, ResId( MF_HEIGHTZOOM)),
    aCropGB(        this, ResId( GB_CROP    )),
    aLeftFT(        this, ResId( FT_LEFT    )),
    aLeftMF(        this, ResId( MF_LEFT    )),
    aRightFT(       this, ResId( FT_RIGHT   )),
    aRightMF(       this, ResId( MF_RIGHT   )),
    aTopFT(         this, ResId( FT_TOP     )),
    aTopMF(         this, ResId( MF_TOP     )),
    aBottomFT(      this, ResId( FT_BOTTOM  )),
    aBottomMF(      this, ResId( MF_BOTTOM  )),
    aSizeConstRB(   this, ResId( RB_SIZECONST)),
    aZoomConstRB(   this, ResId( RB_ZOOMCONST)),
    aExampleWN(     this, ResId( WN_BSP     )),
    aExampleGB(     this, ResId( GB_EXAMPLE )),
    aOrigSizeFT(    this, ResId(FT_ORIG_SIZE)),
    aOrigSizePB(    this, ResId( PB_ORGSIZE )),
    pLastCropField(0),
    bInitialized(FALSE),
    bSetOrigSize(FALSE)
{
    FreeResource();

    SetExchangeSupport();

    // set the correct Metrik
    const FieldUnit eMetric = GetModuleFieldUnit( &rSet );

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
                                    SID_ATTR_GRAF_KEEP_ZOOM ), TRUE, &pItem ))
    {
        if( ((const SfxBoolItem*)pItem)->GetValue() )
            aZoomConstRB.Check();
        else
            aSizeConstRB.Check();
        aZoomConstRB.SaveValue();
    }

    USHORT nW = rPool.GetWhich( SID_ATTR_GRAF_CROP );
    if( SFX_ITEM_SET == rSet.GetItemState( nW, TRUE, &pItem))
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
    if ( SFX_ITEM_SET == rSet.GetItemState( nW, FALSE, &pItem ) )
    {
        // Orientation und Size aus dem PageItem
        FieldUnit eUnit = MapToFieldUnit( rSet.GetPool()->GetMetric( nW ));

        aPageSize = ((const SvxSizeItem*)pItem)->GetSize();

        long nTmp = aHeightMF.Normalize(aPageSize.Height());
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

    BOOL bFound = FALSE;
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_GRAF_GRAPHIC, FALSE, &pItem ) )
    {
        const Graphic* pGrf = ((SvxBrushItem*)pItem)->GetGraphic();
        if( pGrf )
            aOrigSize = GetGrfOrigSize( *pGrf );

        if( aOrigSize.Width() && aOrigSize.Height() )
        {
            CalcMinMaxBorder();
            aExampleWN.SetGraphic( *pGrf );
            aExampleWN.SetFrameSize( aOrigSize );

            bFound = TRUE;
            if( ((SvxBrushItem*)pItem)->GetGraphicLink() )
                aGraphicName = *((SvxBrushItem*)pItem)->GetGraphicLink();
        }
    }

    GraphicHasChanged( bFound );
    bReset = TRUE;
    ActivatePage( rSet );
    bReset = FALSE;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

BOOL SvxGrfCropPage::FillItemSet(SfxItemSet &rSet)
{
    const SfxItemPool& rPool = *rSet.GetPool();
    BOOL bModified = FALSE;
    if( aWidthMF.GetSavedValue() != aWidthMF.GetText() ||
        aHeightMF.GetSavedValue() != aHeightMF.GetText() )
    {
        USHORT nW = rPool.GetWhich( SID_ATTR_GRAF_FRMSIZE );
        FieldUnit eUnit = MapToFieldUnit( rSet.GetPool()->GetMetric( nW ));

        SvxSizeItem aSz( nW );

        // die Groesse koennte schon von einer anderen Page gesetzt worden sein
        // #44204#
        const SfxItemSet* pExSet = GetTabDialog() ? GetTabDialog()->GetExampleSet() : NULL;
        const SfxPoolItem* pItem = 0;
        if( pExSet && SFX_ITEM_SET ==
                pExSet->GetItemState( nW, FALSE, &pItem ) )
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
        USHORT nW = rPool.GetWhich( SID_ATTR_GRAF_CROP );
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

    bInitialized = FALSE;

    return bModified;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SvxGrfCropPage::ActivatePage(const SfxItemSet& rSet)
{
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ) );

    bSetOrigSize = FALSE;

    // Size
    Size aSize;
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_GRAF_FRMSIZE, FALSE, &pItem ) )
        aSize = ((const SvxSizeItem*)pItem)->GetSize();

    nOldWidth = aSize.Width();
    nOldHeight = aSize.Height();

    long nWidth = aWidthMF.Normalize(nOldWidth);
    long nHeight = aHeightMF.Normalize(nOldHeight);

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
    bInitialized = TRUE;

    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_GRAF_GRAPHIC, FALSE, &pItem ) )
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
            GraphicHasChanged( TRUE );
            CalcMinMaxBorder();
        }
        else
            GraphicHasChanged( FALSE );
    }

    CalcZoom();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

int SvxGrfCropPage::DeactivatePage(SfxItemSet *pSet)
{
    if ( pSet )
        FillItemSet( *pSet );
    return TRUE;
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
        USHORT nZoom = (USHORT)( aSize.Width() * 100L / nWidth);
        aWidthZoomMF.SetValue(nZoom);
    }
    else
    {
        long nHeight = aOrigSize.Height() -
                ( lcl_GetValue(aTopMF, eUnit) +
                  lcl_GetValue(aBottomMF, eUnit));
        if(!nHeight)
            nHeight++;
        USHORT nZoom = (USHORT)( aSize.Height() * 100L/ nHeight);
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

    BOOL bZoom = aZoomConstRB.IsChecked();
    if( pField == &aLeftMF || pField == &aRightMF )
    {
        long nLeft = lcl_GetValue( aLeftMF, eUnit );
        long nRight = lcl_GetValue( aRightMF, eUnit );
        long nWidthZoom = aWidthZoomMF.GetValue();
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
        long nHeightZoom = aHeightZoomMF.GetValue();
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

    long nWidth = aOrigSize.Width() +
        lcl_GetValue( aLeftMF, eUnit ) +
        lcl_GetValue( aRightMF, eUnit );
    aWidthMF.SetValue( aWidthMF.Normalize( nWidth ), eUnit );
    long nHeight = aOrigSize.Height() +
        lcl_GetValue( aTopMF, eUnit ) +
        lcl_GetValue( aBottomMF, eUnit );
    aHeightMF.SetValue( aHeightMF.Normalize( nHeight ), eUnit );
    aWidthZoomMF.SetValue(100);
    aHeightZoomMF.SetValue(100);
    bSetOrigSize = TRUE;
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
    USHORT nZoom = 0;
    long nDen;
    if( (nDen = aOrigSize.Width() - nLRBorders) > 0)
        nZoom = (USHORT)((( nWidth  * 1000L / nDen )+5)/10);
    aWidthZoomMF.SetValue(nZoom);
    if( (nDen = aOrigSize.Height() - nULBorders) > 0)
        nZoom = (USHORT)((( nHeight * 1000L / nDen )+5)/10);
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

void SvxGrfCropPage::GraphicHasChanged( BOOL bFound )
{
    if( bFound )
    {
        SfxItemPool* pPool = GetItemSet().GetPool();
        DBG_ASSERT( pPool, "Wo ist der Pool" );
        FieldUnit eUnit = MapToFieldUnit( pPool->GetMetric( pPool->GetWhich(
                                                    SID_ATTR_GRAF_CROP ) ));

        long nSpin = aLeftMF.Normalize(aOrigSize.Width()) / 20;
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
        const FieldUnit eMetric = GetModuleFieldUnit( &GetItemSet() );

        MetricField aFld(this, WB_HIDE);
        SetFieldUnit( aFld, eMetric );
        aFld.SetDecimalDigits( aWidthMF.GetDecimalDigits() );
        aFld.SetMax( LONG_MAX - 1 );

        aFld.SetValue( aFld.Normalize( aOrigSize.Width() ), eUnit );
        String sTemp = aFld.GetText();
        aFld.SetValue( aFld.Normalize( aOrigSize.Height() ), eUnit );
        sTemp += UniString::CreateFromAscii(" x ");
        sTemp += aFld.GetText();
        long nXSize = aOrigSizeFT.GetTextWidth( sTemp );
        long nXPos = aOrigSizePB.GetPosPixel().X() + aOrigSizePB.GetSizePixel().Width()/2;
        nXPos -= (nXSize/2);
        Point aTxtPos(aOrigSizeFT.GetPosPixel());
        aTxtPos.X() = nXPos;
        aOrigSizeFT.SetPosPixel(aTxtPos);
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
    DBG_ASSERT(pLastCropField,"Timeout ohne Feld?")
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
    aTopLeft(0,0), aBottomRight(0,0),
    aFrameSize( OutputDevice::LogicToLogic(
                            Size( CM_1_TO_TWIP / 2, CM_1_TO_TWIP / 2 ),
                            MapMode( MAP_TWIP ), GetMapMode() ))

{
}

void SvxGrfCropPage::SvxCropExample::Paint( const Rectangle& rRect )
{
    Size aWinSize( PixelToLogic(GetOutputSizePixel() ));
    SetLineColor( Color( COL_GRAY ));
    Rectangle aRect(Point((aWinSize.Width() - aFrameSize.Width())/2,
                          (aWinSize.Height() - aFrameSize.Height())/2),
                          aFrameSize );
    SetRasterOp( ROP_OVERPAINT );
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





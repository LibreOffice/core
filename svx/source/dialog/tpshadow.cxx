/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tpshadow.cxx,v $
 * $Revision: 1.19 $
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
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

// include ---------------------------------------------------------------
#include <tools/shl.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>

#define _SVX_TPSHADOW_CXX

#include "xattr.hxx"
#include <svx/xpool.hxx>
#include <svx/dialogs.hrc>
#include "tabarea.hrc"



#include <svx/svdattr.hxx>
#include <svx/xoutx.hxx>

#include "drawitem.hxx"
#include "cuitabarea.hxx"
#include <svx/dialmgr.hxx>
#include "dlgutil.hxx"
#include <cuitabline.hxx>

#include <svx/xlineit0.hxx>
#include <sfx2/request.hxx> //add CHINA001

#define DLGWIN this->GetParent()->GetParent()

// static ----------------------------------------------------------------

static USHORT pShadowRanges[] =
{
    SDRATTR_SHADOWCOLOR,
    SDRATTR_SHADOWTRANSPARENCE,
    SID_ATTR_FILL_SHADOW,
    SID_ATTR_FILL_SHADOW,
    0
};

/*************************************************************************
|*
|*  Dialog zum Aendern des Schattens
|*
\************************************************************************/

SvxShadowTabPage::SvxShadowTabPage( Window* pParent, const SfxItemSet& rInAttrs ) :

    SvxTabPage          ( pParent, SVX_RES( RID_SVXPAGE_SHADOW ), rInAttrs ),

    aFlProp             ( this, SVX_RES( FL_PROP ) ),
    aTsbShowShadow      ( this, SVX_RES( TSB_SHOW_SHADOW ) ),
    aFtPosition         ( this, SVX_RES( FT_POSITION ) ),
    aCtlPosition        ( this, SVX_RES( CTL_POSITION ) ),
    aFtDistance         ( this, SVX_RES( FT_DISTANCE ) ),
    aMtrDistance        ( this, SVX_RES( MTR_FLD_DISTANCE ) ),
    aFtShadowColor      ( this, SVX_RES( FT_SHADOW_COLOR ) ),
    aLbShadowColor      ( this, SVX_RES( LB_SHADOW_COLOR ) ),
    aFtTransparent      ( this, SVX_RES( FT_TRANSPARENT ) ),
    aMtrTransparent      ( this, SVX_RES( MTR_SHADOW_TRANSPARENT ) ),
    aCtlXRectPreview    ( this, SVX_RES( CTL_COLOR_PREVIEW ), &XOut,
                                    (XOutdevItemPool*) rInAttrs.GetPool() ),
    rOutAttrs           ( rInAttrs ),
    pColorTab( NULL ),
    bDisable            ( FALSE ),
    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    XOut                ( &aCtlXRectPreview ),
    aXFillAttr          ( pXPool ),
    rXFSet              ( aXFillAttr.GetItemSet() )
{
    FreeResource();

    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    // Metrik einstellen
    FieldUnit eFUnit = GetModuleFieldUnit( &rInAttrs );

    switch ( eFUnit )
    {
        case FUNIT_M:
        case FUNIT_KM:
            eFUnit = FUNIT_MM;
            break;
        default: ;//prevent warning
    }
    SetFieldUnit( aMtrDistance, eFUnit );

    // PoolUnit ermitteln
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool?" );
    ePoolUnit = pPool->GetMetric( SDRATTR_SHADOWXDIST );

    // Setzen des Output-Devices
    XFillStyle eXFS = XFILL_SOLID;
    if( rOutAttrs.GetItemState( XATTR_FILLSTYLE ) != SFX_ITEM_DONTCARE )
    {
        eXFS = (XFillStyle) ( ( ( const XFillStyleItem& ) rOutAttrs.
                                Get( GetWhich( XATTR_FILLSTYLE ) ) ).GetValue() );
        switch( eXFS )
        {
            //case XFILL_NONE: --> NICHTS

            case XFILL_SOLID:
                if( SFX_ITEM_DONTCARE != rOutAttrs.GetItemState( XATTR_FILLCOLOR ) )
                {
                    XFillColorItem aColorItem( ( const XFillColorItem& )
                                        rOutAttrs.Get( XATTR_FILLCOLOR ) );
                    rXFSet.Put( aColorItem );
                }
            break;

            case XFILL_GRADIENT:
                if( SFX_ITEM_DONTCARE != rOutAttrs.GetItemState( XATTR_FILLGRADIENT ) )
                {
                    XFillGradientItem aGradientItem( ( ( const XFillGradientItem& )
                                            rOutAttrs.Get( XATTR_FILLGRADIENT ) ) );
                    rXFSet.Put( aGradientItem );
                }
            break;

            case XFILL_HATCH:
                if( SFX_ITEM_DONTCARE != rOutAttrs.GetItemState( XATTR_FILLHATCH ) )
                {
                    XFillHatchItem aHatchItem( ( ( const XFillHatchItem& )
                                    rOutAttrs.Get( XATTR_FILLHATCH ) ) );
                    rXFSet.Put( aHatchItem );
                }
            break;

            case XFILL_BITMAP:
            {
                if( SFX_ITEM_DONTCARE != rOutAttrs.GetItemState( XATTR_FILLBITMAP ) )
                {
                    XFillBitmapItem aBitmapItem( ( const XFillBitmapItem& )
                                        rOutAttrs.Get( XATTR_FILLBITMAP ) );
                    rXFSet.Put( aBitmapItem );
                }
            }
            break;
            case XFILL_NONE : break;
        }
    }
    else
    {
        rXFSet.Put( XFillColorItem( String(), COL_LIGHTRED ) );
    }
    rXFSet.Put( XFillStyleItem( eXFS ) );
    aCtlXRectPreview.SetRectAttr( &aXFillAttr );
    //XOut.SetFillAttr( aXFillAttr );

    // Set line at the OutputDevice
    XLineAttrSetItem aXLineAttr( pXPool );
    aXLineAttr.GetItemSet().Put( XLineStyleItem( XLINE_NONE ) );
    XOut.SetLineAttr( aXLineAttr.GetItemSet() );

    aTsbShowShadow.SetClickHdl( LINK( this, SvxShadowTabPage, ClickShadowHdl_Impl ) );
    Link aLink = LINK( this, SvxShadowTabPage, ModifyShadowHdl_Impl );
    aLbShadowColor.SetSelectHdl( aLink );
    aMtrTransparent.SetModifyHdl( aLink );
    aMtrDistance.SetModifyHdl( aLink );

}

// -----------------------------------------------------------------------

void SvxShadowTabPage::Construct()
{
    // Farbtabelle fuellen
    aLbShadowColor.Fill( pColorTab );

    if( bDisable )
    {
        aTsbShowShadow.Disable();
        aFtPosition.Disable();
        aCtlPosition.Disable();
        aFtDistance.Disable();
        aMtrDistance.Disable();
        aFtShadowColor.Disable();
        aLbShadowColor.Disable();
        aFtTransparent.Disable();
        aMtrTransparent.Disable();
    }
}

// -----------------------------------------------------------------------

void SvxShadowTabPage::ActivatePage( const SfxItemSet& rSet )
{
    USHORT nPos;
    USHORT nCount;
    //add CHINA001 Begin
    SFX_ITEMSET_ARG (&rSet,pPageTypeItem,SfxUInt16Item,SID_PAGE_TYPE,sal_False);
    if (pPageTypeItem)
        SetPageType(pPageTypeItem->GetValue());
    //add CHINA001 end
    if( nDlgType == 0 ) //CHINA001 // Flaechen-Dialogif( *pDlgType == 0 ) // Flaechen-Dialog
    {
        if( pColorTab )
        {
            // ColorTable
            if( *pnColorTableState & CT_CHANGED ||
                *pnColorTableState & CT_MODIFIED )
            {
                if( *pnColorTableState & CT_CHANGED )
                {
                    SvxAreaTabDialog* pArea = dynamic_cast< SvxAreaTabDialog* >( DLGWIN );
                    if( pArea )
                    {
                        pColorTab = pArea->GetNewColorTable();
                    }
                    else
                    {
                        SvxLineTabDialog* pLine = dynamic_cast< SvxLineTabDialog* >( DLGWIN );
                        if( pLine )
                            pColorTab = pLine->GetNewColorTable();
                    }
                }

                // aLbShadowColor
                nPos = aLbShadowColor.GetSelectEntryPos();
                aLbShadowColor.Clear();
                aLbShadowColor.Fill( pColorTab );
                nCount = aLbShadowColor.GetEntryCount();
                if( nCount == 0 )
                    ; // Dieser Fall sollte nicht auftreten
                else if( nCount <= nPos )
                    aLbShadowColor.SelectEntryPos( 0 );
                else
                    aLbShadowColor.SelectEntryPos( nPos );

                ModifyShadowHdl_Impl( this );
            }
            nPageType = PT_SHADOW;//CHINA001 *pPageType = PT_SHADOW;
        }
    }
}

// -----------------------------------------------------------------------

int SvxShadowTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( _pSet )
        FillItemSet( *_pSet );

    return( LEAVE_PAGE );
}

// -----------------------------------------------------------------------

BOOL SvxShadowTabPage::FillItemSet( SfxItemSet& rAttrs )
{
    const SfxPoolItem*  pOld = NULL;
    BOOL                bModified = FALSE;

    if( !bDisable )
    {
        // Schatten
        TriState eState = aTsbShowShadow.GetState();
        if( eState != aTsbShowShadow.GetSavedValue() )
        {
            SdrShadowItem aItem( sal::static_int_cast< BOOL >( eState ) );
            pOld = GetOldItem( rAttrs, SDRATTR_SHADOW );
            if ( !pOld || !( *(const SdrShadowItem*)pOld == aItem ) )
            {
                rAttrs.Put( aItem );
                bModified = TRUE;
            }
        }

        // Schatten-Entfernung
        // Etwas umstaendliche Abfrage, ob etwas geaendert wurde,
        // da Items nicht direkt auf Controls abbildbar sind
        INT32 nX = 0L, nY = 0L;
        INT32 nXY = GetCoreValue( aMtrDistance, ePoolUnit );

        switch( aCtlPosition.GetActualRP() )
        {
            case RP_LT: nX = nY = -nXY;      break;
            case RP_MT: nY = -nXY;           break;
            case RP_RT: nX = nXY; nY = -nXY; break;
            case RP_LM: nX = -nXY;           break;
            case RP_RM: nX = nXY;            break;
            case RP_LB: nX = -nXY; nY = nXY; break;
            case RP_MB: nY = nXY;            break;
            case RP_RB: nX = nY = nXY;       break;
            case RP_MM: break;
        }

        // Wenn die Werte des Schattenabstanden==SFX_ITEM_DONTCARE und der angezeigte
        // String im entspr. MetricField=="", dann w�rde der Vergleich zw. alten und
        // neuen Distance-Werte ein falsches Ergebnis liefern, da in so einem Fall die
        // neuen Distance-Werte den Default-Werten des MetricField entspr�chen !!!!
        if ( !aMtrDistance.IsEmptyFieldValue()                                  ||
             rOutAttrs.GetItemState( SDRATTR_SHADOWXDIST ) != SFX_ITEM_DONTCARE ||
             rOutAttrs.GetItemState( SDRATTR_SHADOWYDIST ) != SFX_ITEM_DONTCARE    )
        {
            INT32 nOldX = 9876543; // Unmoeglicher Wert, entspr. DontCare
            INT32 nOldY = 9876543;
            if( rOutAttrs.GetItemState( SDRATTR_SHADOWXDIST ) != SFX_ITEM_DONTCARE &&
                rOutAttrs.GetItemState( SDRATTR_SHADOWYDIST ) != SFX_ITEM_DONTCARE )
            {
                nOldX = ( ( const SdrShadowXDistItem& ) rOutAttrs.
                                    Get( SDRATTR_SHADOWXDIST ) ).GetValue();
                nOldY = ( ( const SdrShadowYDistItem& ) rOutAttrs.
                                    Get( SDRATTR_SHADOWYDIST ) ).GetValue();
            }
            SdrShadowXDistItem aXItem( nX );
            pOld = GetOldItem( rAttrs, SDRATTR_SHADOWXDIST );
            if ( nX != nOldX &&
                ( !pOld || !( *(const SdrShadowXDistItem*)pOld == aXItem ) ) )
            {
                rAttrs.Put( aXItem );
                bModified = TRUE;
            }
            SdrShadowYDistItem aYItem( nY );
            pOld = GetOldItem( rAttrs, SDRATTR_SHADOWYDIST );
            if ( nY != nOldY &&
                ( !pOld || !( *(const SdrShadowYDistItem*)pOld == aYItem ) ) )
            {
                rAttrs.Put( aYItem );
                bModified = TRUE;
            }
        }

        // ShadowColor
        USHORT nPos = aLbShadowColor.GetSelectEntryPos();
        if( nPos != LISTBOX_ENTRY_NOTFOUND &&
            nPos != aLbShadowColor.GetSavedValue() )
        {
            SdrShadowColorItem aItem( aLbShadowColor.GetSelectEntry(),
                                    aLbShadowColor.GetSelectEntryColor() );
            pOld = GetOldItem( rAttrs, SDRATTR_SHADOWCOLOR );
            if ( !pOld || !( *(const SdrShadowColorItem*)pOld == aItem ) )
            {
                rAttrs.Put( aItem );
                bModified = TRUE;
            }
        }

        // Transparenz
        UINT16 nVal = (UINT16)aMtrTransparent.GetValue();
        if( nVal != (UINT16)aMtrTransparent.GetSavedValue().ToInt32() )
        {
            SdrShadowTransparenceItem aItem( nVal );
            pOld = GetOldItem( rAttrs, SDRATTR_SHADOWTRANSPARENCE );
            if ( !pOld || !( *(const SdrShadowTransparenceItem*)pOld == aItem ) )
            {
                rAttrs.Put( aItem );
                bModified = TRUE;
            }
        }
    }
    //add CHINA001  begin
    rAttrs.Put (CntUInt16Item(SID_PAGE_TYPE,nPageType));
    //add CHINA001  end
    return( bModified );
}

// -----------------------------------------------------------------------

void SvxShadowTabPage::Reset( const SfxItemSet& rAttrs )
{
    if( !bDisable )
    {
        // Alle Objekte koennen einen Schatten besitzen
        // z.Z. gibt es nur 8 m�gliche Positionen den Schatten zu setzen

        // Ist Schatten gesetzt?
        if( rAttrs.GetItemState( SDRATTR_SHADOW ) != SFX_ITEM_DONTCARE )
        {
            aTsbShowShadow.EnableTriState( FALSE );

            if( ( ( const SdrShadowItem& ) rAttrs.Get( SDRATTR_SHADOW ) ).GetValue() )
                aTsbShowShadow.SetState( STATE_CHECK );
            else
            {
                aTsbShowShadow.SetState( STATE_NOCHECK );
            }
        }
        else
            aTsbShowShadow.SetState( STATE_DONTKNOW );

        // Entfernung (nur 8 moegliche Positionen), deshalb
        // wird nur ein Item ausgewertet

        if( rAttrs.GetItemState( SDRATTR_SHADOWXDIST ) != SFX_ITEM_DONTCARE &&
            rAttrs.GetItemState( SDRATTR_SHADOWYDIST ) != SFX_ITEM_DONTCARE )
        {
            INT32 nX = ( ( const SdrShadowXDistItem& ) rAttrs.Get( SDRATTR_SHADOWXDIST ) ).GetValue();
            INT32 nY = ( ( const SdrShadowYDistItem& ) rAttrs.Get( SDRATTR_SHADOWYDIST ) ).GetValue();

            if( nX != 0 )
                SetMetricValue( aMtrDistance, nX < 0L ? -nX : nX, ePoolUnit );
            else
                SetMetricValue( aMtrDistance, nY < 0L ? -nY : nY, ePoolUnit );

            // Setzen des Schatten-Controls
            if     ( nX <  0L && nY <  0L ) aCtlPosition.SetActualRP( RP_LT );
            else if( nX == 0L && nY <  0L ) aCtlPosition.SetActualRP( RP_MT );
            else if( nX >  0L && nY <  0L ) aCtlPosition.SetActualRP( RP_RT );
            else if( nX <  0L && nY == 0L ) aCtlPosition.SetActualRP( RP_LM );
            // Mittelpunkt gibt es nicht mehr
            else if( nX == 0L && nY == 0L ) aCtlPosition.SetActualRP( RP_RB );
            else if( nX >  0L && nY == 0L ) aCtlPosition.SetActualRP( RP_RM );
            else if( nX <  0L && nY >  0L ) aCtlPosition.SetActualRP( RP_LB );
            else if( nX == 0L && nY >  0L ) aCtlPosition.SetActualRP( RP_MB );
            else if( nX >  0L && nY >  0L ) aCtlPosition.SetActualRP( RP_RB );
        }
        else
        {
            // determine default-distance
            SfxItemPool* pPool = rOutAttrs.GetPool();
            SdrShadowXDistItem* pXDistItem = (SdrShadowXDistItem*)&pPool->GetDefaultItem (SDRATTR_SHADOWXDIST);
            SdrShadowYDistItem* pYDistItem = (SdrShadowYDistItem*)&pPool->GetDefaultItem (SDRATTR_SHADOWYDIST);
            if (pXDistItem && pYDistItem)
            {
                INT32 nX = pXDistItem->GetValue();
                INT32 nY = pYDistItem->GetValue();
                if( nX != 0 )
                    SetMetricValue( aMtrDistance, nX < 0L ? -nX : nX, ePoolUnit );
                else
                    SetMetricValue( aMtrDistance, nY < 0L ? -nY : nY, ePoolUnit );
            }

            // Tristate, z.B. mehrer Objekte wurden markiert, wovon einige einen Schatten besitzen, einige nicht.
            // Der anzuzeigende Text des MetricFields wird auf "" gesetzt und dient in der Methode FillItemSet
            // als Erkennungszeichen daf�r, das der Distance-Wert NICHT ver�ndert wurde !!!!
            aMtrDistance.SetText( String() );
            aCtlPosition.SetActualRP( RP_MM );
        }

        // SchattenFarbe:
        if( rAttrs.GetItemState( SDRATTR_SHADOWCOLOR ) != SFX_ITEM_DONTCARE )
        {
            aLbShadowColor.SelectEntry( ( ( const SdrShadowColorItem& ) rAttrs.Get( SDRATTR_SHADOWCOLOR ) ).GetColorValue() );
        }
        else
            aLbShadowColor.SetNoSelection();

        // Transparenz
        if( rAttrs.GetItemState( SDRATTR_SHADOWTRANSPARENCE ) != SFX_ITEM_DONTCARE )
        {
            USHORT nTransp = ( ( const SdrShadowTransparenceItem& ) rAttrs.Get( SDRATTR_SHADOWTRANSPARENCE ) ).GetValue();
            aMtrTransparent.SetValue( nTransp );
        }
        else
            aMtrTransparent.SetText( String() );

        // Werte sichern
        //aCtlPosition
        aMtrDistance.SaveValue();
        aLbShadowColor.SaveValue();
        aTsbShowShadow.SaveValue();

        // #66832# This field was not saved, but used to determine changes.
        // Why? Seems to be the error.
        // It IS the error.
        aMtrTransparent.SaveValue();

        ClickShadowHdl_Impl( NULL );
        ModifyShadowHdl_Impl( NULL );
    }
}

// -----------------------------------------------------------------------

SfxTabPage* SvxShadowTabPage::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SvxShadowTabPage( pWindow, rAttrs ) );
}

//------------------------------------------------------------------------

USHORT* SvxShadowTabPage::GetRanges()
{
    return( pShadowRanges );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxShadowTabPage, ClickShadowHdl_Impl, void *, EMPTYARG )
{
    if( aTsbShowShadow.GetState() == STATE_NOCHECK )
    {
        aFtPosition.Disable();
        aCtlPosition.Disable();
        aFtDistance.Disable();
        aMtrDistance.Disable();
        aFtShadowColor.Disable();
        aLbShadowColor.Disable();
        aFtTransparent.Disable();
        aMtrTransparent.Disable();
    }
    else
    {
        aFtPosition.Enable();
        aCtlPosition.Enable();
        aFtDistance.Enable();
        aMtrDistance.Enable();
        aFtShadowColor.Enable();
        aLbShadowColor.Enable();
        aFtTransparent.Enable();
        aMtrTransparent.Enable();
    }
    aCtlPosition.Invalidate();

    ModifyShadowHdl_Impl( NULL );

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxShadowTabPage, ModifyShadowHdl_Impl, void *, EMPTYARG )
{
    if( aTsbShowShadow.GetState() == STATE_CHECK )
        rXFSet.Put( XFillStyleItem( XFILL_SOLID ) );
    else
        rXFSet.Put( XFillStyleItem( XFILL_NONE ) );

    USHORT nPos = aLbShadowColor.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        rXFSet.Put( XFillColorItem( String(),
                        aLbShadowColor.GetSelectEntryColor() ) );
    }
    UINT16 nVal = (UINT16)aMtrTransparent.GetValue();
    XFillTransparenceItem aItem( nVal );
    rXFSet.Put( XFillTransparenceItem( aItem ) );

    // Schatten-Entfernung
    INT32 nX = 0L, nY = 0L;
    INT32 nXY = GetCoreValue( aMtrDistance, ePoolUnit );
    switch( aCtlPosition.GetActualRP() )
    {
        case RP_LT: nX = nY = -nXY;      break;
        case RP_MT: nY = -nXY;           break;
        case RP_RT: nX = nXY; nY = -nXY; break;
        case RP_LM: nX = -nXY;           break;
        case RP_RM: nX = nXY;            break;
        case RP_LB: nX = -nXY; nY = nXY; break;
        case RP_MB: nY = nXY;            break;
        case RP_RB: nX = nY = nXY;       break;
        case RP_MM: break;
    }
    aCtlXRectPreview.SetShadowPos( Point( nX, nY ) );

    aCtlXRectPreview.SetShadowAttr( &aXFillAttr );
    //XOut.SetFillAttr( aXFillAttr );
    aCtlXRectPreview.Invalidate();

    return( 0L );
}

//------------------------------------------------------------------------

void SvxShadowTabPage::PointChanged( Window* pWindow, RECT_POINT eRcPt )
{
    eRP = eRcPt;

    // Schatten neu zeichnen
    ModifyShadowHdl_Impl( pWindow );
}

void SvxShadowTabPage::PageCreated (SfxAllItemSet aSet) //add CHINA001
{
    SFX_ITEMSET_ARG (&aSet,pColorTabItem,SvxColorTableItem,SID_COLOR_TABLE,sal_False);
    SFX_ITEMSET_ARG (&aSet,pPageTypeItem,SfxUInt16Item,SID_PAGE_TYPE,sal_False);
    SFX_ITEMSET_ARG (&aSet,pDlgTypeItem,SfxUInt16Item,SID_DLG_TYPE,sal_False);


    if (pColorTabItem)
        SetColorTable(pColorTabItem->GetColorTable());
    if (pPageTypeItem)
        SetPageType(pPageTypeItem->GetValue());
    if (pDlgTypeItem)
        SetDlgType(pDlgTypeItem->GetValue());
    Construct();
}

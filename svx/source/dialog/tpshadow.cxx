/*************************************************************************
 *
 *  $RCSfile: tpshadow.cxx,v $
 *
 *  $Revision: 1.3 $
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

// include ---------------------------------------------------------------

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFXMODULE_HXX
#include <sfx2/module.hxx>
#endif
#ifndef _SFX_SAVEOPT_HXX //autogen
#include <sfx2/saveopt.hxx>
#endif

#define _SVX_TPSHADOW_CXX

#include "xpool.hxx"
#include "dialogs.hrc"
#include "tabarea.hrc"

#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE

#include "svdattr.hxx"
#include "xoutx.hxx"

#include "drawitem.hxx"
#include "tabarea.hxx"
#include "dialmgr.hxx"
#include "dlgutil.hxx"

#ifndef _SVX_XLINEIT0_HXX //autogen
#include "xlineit0.hxx"
#endif

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

    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    XOut                ( &aCtlXRectPreview ),
    aXFillAttr          ( pXPool ),
    rXFSet              ( aXFillAttr.GetItemSet() ),

    aFtPosition         ( this, ResId( FT_POSITION ) ),
    aCtlPosition        ( this, ResId( CTL_POSITION ),
                                    RP_RM, 200, 80, CS_SHADOW ),
    aFtDistance         ( this, ResId( FT_DISTANCE ) ),
    aMtrDistance        ( this, ResId( MTR_FLD_DISTANCE ) ),
    aFtShadowColor      ( this, ResId( FT_SHADOW_COLOR ) ),
    aLbShadowColor      ( this, ResId( LB_SHADOW_COLOR ) ),
    aFtTransparent      ( this, ResId( FT_TRANSPARENT ) ),
    aMtrTransparent      ( this, ResId( MTR_SHADOW_TRANSPARENT ) ),
    aTsbShowShadow      ( this, ResId( TSB_SHOW_SHADOW ) ),
    aGrpShadow          ( this, ResId( GRP_SHADOW ) ),
    aCtlXRectPreview    ( this, ResId( CTL_COLOR_PREVIEW ), &XOut,
                                    (XOutdevItemPool*) rInAttrs.GetPool() ),
    aGrpPreview         ( this, ResId( GRP_PREVIEW ) ),
    rOutAttrs           ( rInAttrs ),
    bDisable            ( FALSE )

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
        }
    }
    else
    {
        rXFSet.Put( XFillColorItem( String(), COL_LIGHTRED ) );
    }
    rXFSet.Put( XFillStyleItem( eXFS ) );
    aCtlXRectPreview.SetRectAttr( &aXFillAttr );
    //XOut.SetFillAttr( aXFillAttr );

    // Setzen der Linie auf None im OutputDevice
    XLineAttrSetItem aXLineAttr( pXPool );
    aXLineAttr.GetItemSet().Put( XLineStyleItem( XLINE_NONE ) );
    XOut.SetLineAttr( aXLineAttr );

    aTsbShowShadow.SetClickHdl( LINK( this, SvxShadowTabPage, ClickShadowHdl_Impl ) );
    Link aLink = LINK( this, SvxShadowTabPage, ModifyShadowHdl_Impl );
    aLbShadowColor.SetSelectHdl( aLink );
    aMtrTransparent.SetModifyHdl( aLink );
    aMtrDistance.SetModifyHdl( aLink );

    pColorTab = NULL;
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
    int nPos;
    int nCount;

    if( *pDlgType == 0 ) // Flaechen-Dialog
    {
        if( pColorTab )
        {
            // ColorTable
            if( *pnColorTableState & CT_CHANGED ||
                *pnColorTableState & CT_MODIFIED )
            {
                if( *pnColorTableState & CT_CHANGED )
                    pColorTab = ( (SvxAreaTabDialog*) DLGWIN )->GetNewColorTable();

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
            *pPageType = PT_SHADOW;
        }
    }
}

// -----------------------------------------------------------------------

int SvxShadowTabPage::DeactivatePage( SfxItemSet* pSet )
{
    FillItemSet( *pSet );

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
            SdrShadowItem aItem( eState );
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
        }

        // Wenn die Werte des Schattenabstanden==SFX_ITEM_DONTCARE und der angezeigte
        // String im entspr. MetricField=="", dann würde der Vergleich zw. alten und
        // neuen Distance-Werte ein falsches Ergebnis liefern, da in so einem Fall die
        // neuen Distance-Werte den Default-Werten des MetricField entsprächen !!!!
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
    return( bModified );
}

// -----------------------------------------------------------------------

void SvxShadowTabPage::Reset( const SfxItemSet& rAttrs )
{
    if( !bDisable )
    {
        // Alle Objekte koennen einen Schatten besitzen
        // z.Z. gibt es nur 8 m”gliche Positionen den Schatten zu setzen

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
            // als Erkennungszeichen dafür, das der Distance-Wert NICHT verändert wurde !!!!
            aMtrDistance.SetText( String() );
            aCtlPosition.SetActualRP( RP_MM );
        }

        // SchattenFarbe:
        if( rAttrs.GetItemState( SDRATTR_SHADOWCOLOR ) != SFX_ITEM_DONTCARE )
        {
            aLbShadowColor.SelectEntry( ( ( const SdrShadowColorItem& ) rAttrs.Get( SDRATTR_SHADOWCOLOR ) ).GetValue() );
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



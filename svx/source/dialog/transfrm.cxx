/*************************************************************************
 *
 *  $RCSfile: transfrm.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:01:13 $
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

#pragma hdrstop

#define ITEMID_SIZE 0

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif

#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif

#ifndef _SVDVIEW_HXX //autogen
#include "svdview.hxx"
#endif
#ifndef _SVDOBJ_HXX //autogen
#include "svdobj.hxx"
#endif
#ifndef _SVDPAGV_HXX //autogen
#include "svdpagv.hxx"
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include "svdotext.hxx"
#endif
#ifndef _SDERITM_HXX //autogen
#include "sderitm.hxx"
#endif

#include <svxids.hrc>
#include "dialogs.hrc"
#include "transfrm.hrc"
#include "sizeitem.hxx"

#include "transfrm.hxx"
#include "dialmgr.hxx"
#include "dlgutil.hxx"
#include "svxenum.hxx"
#include "anchorid.hxx"

#ifndef _SFXMODULE_HXX
#include <sfx2/module.hxx>
#endif

#ifndef _SFXRECTITEM_HXX //autogen
#include <svtools/rectitem.hxx>
#endif

#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif

// Toleranz fuer WorkingArea
#define DIFF 1000

// static ----------------------------------------------------------------

static USHORT pPosRanges[] =
{
    SID_ATTR_TRANSFORM_POS_X,
    SID_ATTR_TRANSFORM_POS_Y,
    SID_ATTR_TRANSFORM_PROTECT_POS,
    SID_ATTR_TRANSFORM_PROTECT_POS,
    SID_ATTR_TRANSFORM_INTERN,
    SID_ATTR_TRANSFORM_INTERN,
    SID_ATTR_TRANSFORM_ANCHOR,
    SID_ATTR_TRANSFORM_VERT_ORIENT,
    0
};

static USHORT pSizeRanges[] =
{
    SID_ATTR_TRANSFORM_WIDTH,
    SID_ATTR_TRANSFORM_SIZE_POINT,
    SID_ATTR_TRANSFORM_PROTECT_POS,
    SID_ATTR_TRANSFORM_INTERN,
    SID_ATTR_TRANSFORM_AUTOWIDTH,
    SID_ATTR_TRANSFORM_AUTOHEIGHT,
    0
};

static USHORT pPosSizeRanges[] =
{
    SID_ATTR_TRANSFORM_POS_X,
    SID_ATTR_TRANSFORM_POS_Y,
    SID_ATTR_TRANSFORM_PROTECT_POS,
    SID_ATTR_TRANSFORM_PROTECT_POS,
    SID_ATTR_TRANSFORM_INTERN,
    SID_ATTR_TRANSFORM_INTERN,
    SID_ATTR_TRANSFORM_ANCHOR,
    SID_ATTR_TRANSFORM_VERT_ORIENT,
    SID_ATTR_TRANSFORM_WIDTH,
    SID_ATTR_TRANSFORM_SIZE_POINT,
    SID_ATTR_TRANSFORM_PROTECT_POS,
    SID_ATTR_TRANSFORM_INTERN,
    SID_ATTR_TRANSFORM_AUTOWIDTH,
    SID_ATTR_TRANSFORM_AUTOHEIGHT,
    0
};

static USHORT pAngleRanges[] =
{
    SID_ATTR_TRANSFORM_ROT_X,
    SID_ATTR_TRANSFORM_ANGLE,
    SID_ATTR_TRANSFORM_INTERN,
    SID_ATTR_TRANSFORM_INTERN,
    0
};

static USHORT pSlantRanges[] =
{
    SDRATTR_ECKENRADIUS,
    SDRATTR_ECKENRADIUS,
    SID_ATTR_TRANSFORM_SHEAR,
    SID_ATTR_TRANSFORM_SHEAR_VERTICAL,
    SID_ATTR_TRANSFORM_INTERN,
    SID_ATTR_TRANSFORM_INTERN,
    0
};


// Funktion ConvertRect
Rectangle lcl_ConvertRect( const Rectangle& rInRect, USHORT nDigits, MapUnit ePoolUnit, FieldUnit eDlgUnit )
{
    Rectangle aRect;
    aRect.Left()   = MetricField::ConvertValue( rInRect.Left(), nDigits, ePoolUnit, eDlgUnit );
    aRect.Right()  = MetricField::ConvertValue( rInRect.Right(), nDigits, ePoolUnit, eDlgUnit );
    aRect.Top()    = MetricField::ConvertValue( rInRect.Top(), nDigits, ePoolUnit, eDlgUnit );
    aRect.Bottom() = MetricField::ConvertValue( rInRect.Bottom(), nDigits, ePoolUnit, eDlgUnit );
    return( aRect );
}

// Funktion ConvertPoint
Point lcl_ConvertPoint( Point aInPt, USHORT nDigits, MapUnit ePoolUnit, FieldUnit eDlgUnit )
{
    Point aPt;
    aPt.X() = MetricField::ConvertValue( aInPt.X(), nDigits, ePoolUnit, eDlgUnit );
    aPt.Y() = MetricField::ConvertValue( aInPt.Y(), nDigits, ePoolUnit, eDlgUnit );
    return( aPt );
}

// Funktion ScaleRect (Beruecksichtigung des Massstabes)
void lcl_ScaleRect( Rectangle& aRect, Fraction aUIScale )
{
    aRect.Left()      = Fraction( aRect.Left() )   / aUIScale;
    aRect.Right()     = Fraction( aRect.Right() )  / aUIScale;
    aRect.Top()       = Fraction( aRect.Top() )    / aUIScale;
    aRect.Bottom()    = Fraction( aRect.Bottom() ) / aUIScale;
}

// Funktion ScalePoint (Beruecksichtigung des Massstabes)
void lcl_ScalePoint( Point& aPt, Fraction aUIScale )
{
    aPt.X() = Fraction( aPt.X() ) / aUIScale;
    aPt.Y() = Fraction( aPt.Y() ) / aUIScale;
}

/*************************************************************************
|*
|* Konstruktor des Tab-Dialogs: Fuegt die Seiten zum Dialog hinzu
|*
\************************************************************************/

SvxTransformTabDialog::SvxTransformTabDialog( Window* pParent, const SfxItemSet* pAttr,
                                const SdrView* pSdrView, USHORT nAnchorTypes ) :
    SfxTabDialog( pParent, ResId( RID_SVXDLG_TRANSFORM, DIALOG_MGR() ), pAttr ),
    pView       ( pSdrView ),
    nAnchorCtrls(nAnchorTypes)
{
    FreeResource();

    DBG_ASSERT( pView, "Keine gueltige View Uebergeben!" );

    AddTabPage( RID_SVXPAGE_POSITION_SIZE, SvxPositionSizeTabPage::Create,
                            SvxPositionSizeTabPage::GetRanges );
    AddTabPage( RID_SVXPAGE_ANGLE, SvxAngleTabPage::Create,
                            SvxAngleTabPage::GetRanges );
    AddTabPage( RID_SVXPAGE_SLANT, SvxSlantTabPage::Create,
                            SvxSlantTabPage::GetRanges );
}

// -----------------------------------------------------------------------

SvxTransformTabDialog::~SvxTransformTabDialog()
{
}

// -----------------------------------------------------------------------

void SvxTransformTabDialog::PageCreated( USHORT nId, SfxTabPage &rPage )
{
    switch( nId )
    {
        case RID_SVXPAGE_POSITION_SIZE:
            ( (SvxPositionSizeTabPage&) rPage ).SetView( pView );
            ( (SvxPositionSizeTabPage&) rPage ).Construct();
            if( nAnchorCtrls & SVX_OBJ_NORESIZE )
                ( (SvxPositionSizeTabPage&) rPage ).DisableResize();

            if( nAnchorCtrls & SVX_OBJ_NOPROTECT )
                ( (SvxPositionSizeTabPage&) rPage ).DisableProtect();

            if(nAnchorCtrls & 0x00ff )
                ( (SvxPositionSizeTabPage&) rPage ).ShowAnchorCtrls(nAnchorCtrls);
        break;

        case RID_SVXPAGE_ANGLE:
            ( (SvxAngleTabPage&) rPage ).SetView( pView );
            ( (SvxAngleTabPage&) rPage ).Construct();
        break;

        case RID_SVXPAGE_SLANT:
            ( (SvxSlantTabPage&) rPage ).SetView( pView );
            ( (SvxSlantTabPage&) rPage ).Construct();
        break;
    }
}

/*************************************************************************
|*
|*      Dialog zum Aendern der Position des Drehwinkels und des Drehwinkels
|*      der Grafikobjekte
|*
\************************************************************************/

SvxAngleTabPage::SvxAngleTabPage( Window* pParent, const SfxItemSet& rInAttrs  ) :
    SvxTabPage              ( pParent, ResId( RID_SVXPAGE_ANGLE, DIALOG_MGR() ), rInAttrs ),
    aFlPosition             ( this, ResId( FL_POSITION ) ),
    aFtPosX                 ( this, ResId( FT_POS_X ) ),
    aMtrPosX                ( this, ResId( MTR_FLD_POS_X ) ),
    aFtPosY                 ( this, ResId( FT_POS_Y ) ),
    aMtrPosY                ( this, ResId( MTR_FLD_POS_Y ) ),
    aFtPosPresets           ( this, ResId(FT_POSPRESETS) ),
    aCtlRect                ( this, ResId( CTL_RECT ) ),

    aFlAngle                ( this, ResId( FL_ANGLE ) ),
    aFtAngle                ( this, ResId( FT_ANGLE ) ),
    aMtrAngle               ( this, ResId( MTR_FLD_ANGLE ) ),
    aFtAnglePresets         ( this, ResId(FT_ANGLEPRESETS) ),
    aCtlAngle               ( this, ResId( CTL_ANGLE ),
                                RP_RB, 200, 80, CS_ANGLE ),
    rOutAttrs               ( rInAttrs )
{
    FreeResource();

    // diese Page braucht ExchangeSupport
//!     SetExchangeSupport(); noch nicht

    // PoolUnit ermitteln
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    ePoolUnit = pPool->GetMetric( SID_ATTR_TRANSFORM_POS_X );

    aMtrAngle.SetModifyHdl( LINK( this, SvxAngleTabPage, ModifiedHdl ) );
}

// -----------------------------------------------------------------------

void SvxAngleTabPage::Construct()
{
    // Setzen des Rechtecks
    DBG_ASSERT( pView, "Keine gueltige View Uebergeben!" );
    eDlgUnit = GetModuleFieldUnit( &GetItemSet() );
    SetFieldUnit( aMtrPosX, eDlgUnit, TRUE );
    SetFieldUnit( aMtrPosY, eDlgUnit, TRUE );

    if( eDlgUnit == FUNIT_MILE ||
        eDlgUnit == FUNIT_KM )
    {
        aMtrPosX.SetDecimalDigits( 3 );
        aMtrPosY.SetDecimalDigits( 3 );
    }

    aRect = pView->GetAllMarkedRect();
    pView->GetPageViewPvNum( 0 )->LogicToPagePos( aRect );

    // Ankerposition beachten (Writer)
    const SdrMarkList& rMarkList = pView->GetMarkList();
    if( rMarkList.GetMarkCount() >= 1 )
    {
        const SdrObject* pObj = rMarkList.GetMark( 0 )->GetObj();
        aAnchorPos = pObj->GetAnchorPos();

        if( aAnchorPos != Point( 0, 0 ) ) // -> Writer
        {
            aRect.Left() -= aAnchorPos.X();
            aRect.Right() -= aAnchorPos.X();
            aRect.Top() -= aAnchorPos.Y();
            aRect.Bottom() -= aAnchorPos.Y();
        }
    }

    // Massstab
    Fraction aUIScale = pView->GetModel()->GetUIScale();
    lcl_ScaleRect( aRect, aUIScale );

    // Umrechnung auf UI-Unit
    int nDigits = aMtrPosX.GetDecimalDigits();
    aRect = lcl_ConvertRect( aRect, nDigits, (MapUnit) ePoolUnit, eDlgUnit );

    if( !pView->IsRotateAllowed() )
    {
        aFlPosition.Disable();
        aFtPosX.Disable();
        aMtrPosX.Disable();
        aFtPosY.Disable();
        aMtrPosY.Disable();
        aFtPosPresets.Disable();
        aCtlRect.Disable();
        aFlAngle.Disable();
        aFtAngle.Disable();
        aMtrAngle.Disable();
        aFtAnglePresets.Disable();
        aCtlAngle.Disable();
    }
}

// -----------------------------------------------------------------------

BOOL SvxAngleTabPage::FillItemSet( SfxItemSet& rOutAttrs )
{
    BOOL bModified = FALSE;

    if(aMtrAngle.IsValueModified()
        || aMtrPosX.IsValueModified()
        || aMtrPosY.IsValueModified())
    {
        rOutAttrs.Put( SfxInt32Item( GetWhich( SID_ATTR_TRANSFORM_ANGLE ),
                        aMtrAngle.GetValue() ) );

        Fraction aUIScale = pView->GetModel()->GetUIScale();
        long nTmp = GetCoreValue( aMtrPosX, ePoolUnit );
        nTmp += aAnchorPos.X();
        nTmp = Fraction( nTmp ) * aUIScale;
        rOutAttrs.Put( SfxInt32Item( GetWhich( SID_ATTR_TRANSFORM_ROT_X ), nTmp ) );

        nTmp = GetCoreValue( aMtrPosY, ePoolUnit );
        nTmp += aAnchorPos.Y();
        nTmp = Fraction( nTmp ) * aUIScale;
        rOutAttrs.Put( SfxInt32Item( GetWhich( SID_ATTR_TRANSFORM_ROT_Y ), nTmp ) );

        bModified |= TRUE;
    }
    return bModified;
}

// -----------------------------------------------------------------------

void SvxAngleTabPage::Reset( const SfxItemSet& rAttrs )
{
    const SfxPoolItem* pItem = GetItem( rAttrs, SID_ATTR_TRANSFORM_ROT_X );

    Fraction aUIScale = pView->GetModel()->GetUIScale();
    if ( pItem )
    {
        long nTmp = ( (const SfxInt32Item*)pItem )->GetValue() - aAnchorPos.X();;
        nTmp = Fraction( nTmp ) / aUIScale;
        SetMetricValue( aMtrPosX, nTmp, ePoolUnit );
    }
    else
        aMtrPosX.SetText( String() );

    pItem = GetItem( rAttrs, SID_ATTR_TRANSFORM_ROT_Y );
    if ( pItem )
    {
        long nTmp = ( (const SfxInt32Item*)pItem )->GetValue() - aAnchorPos.Y();;
        nTmp = Fraction( nTmp ) / aUIScale;
        SetMetricValue( aMtrPosY, nTmp, ePoolUnit );
    }
    else
        aMtrPosX.SetText( String() );
    pItem = GetItem( rAttrs, SID_ATTR_TRANSFORM_ANGLE );

    if ( pItem )
        aMtrAngle.SetValue( ( (const SfxInt32Item*)pItem )->GetValue() );
    else
        aMtrAngle.SetText( String() );
    aMtrAngle.SaveValue();

    ModifiedHdl( this );
}

// -----------------------------------------------------------------------

SfxTabPage* SvxAngleTabPage::Create( Window* pWindow,
                                     const SfxItemSet& rOutAttrs )
{
    return( new SvxAngleTabPage( pWindow, rOutAttrs ) );
}

//------------------------------------------------------------------------

USHORT* SvxAngleTabPage::GetRanges()
{
    return( pAngleRanges );
}

// -----------------------------------------------------------------------

void SvxAngleTabPage::ActivatePage( const SfxItemSet& rSet )
{
}

// -----------------------------------------------------------------------

int SvxAngleTabPage::DeactivatePage( SfxItemSet* pSet )
{
    if( pSet )
        FillItemSet( *pSet );

    return( LEAVE_PAGE );
}

//------------------------------------------------------------------------

void SvxAngleTabPage::PointChanged( Window* pWindow, RECT_POINT eRP )
{
    if( pWindow == &aCtlRect )
    {
        switch( eRP )
        {
            case RP_LT:
                aMtrPosX.SetUserValue( aRect.Left(), FUNIT_NONE );
                aMtrPosY.SetUserValue( aRect.Top(), FUNIT_NONE );
                break;
            case RP_MT:
                aMtrPosX.SetUserValue( aRect.Center().X(), FUNIT_NONE );
                aMtrPosY.SetUserValue( aRect.Top(), FUNIT_NONE );
                break;
            case RP_RT:
                aMtrPosX.SetUserValue( aRect.Right(), FUNIT_NONE );
                aMtrPosY.SetUserValue( aRect.Top(), FUNIT_NONE );
                break;
            case RP_LM:
                aMtrPosX.SetUserValue( aRect.Left(), FUNIT_NONE );
                aMtrPosY.SetUserValue( aRect.Center().Y(), FUNIT_NONE );
                break;
            case RP_MM:
                aMtrPosX.SetUserValue( aRect.Center().X(), FUNIT_NONE );
                aMtrPosY.SetUserValue( aRect.Center().Y(), FUNIT_NONE );
                break;
            case RP_RM:
                aMtrPosX.SetUserValue( aRect.Right(), FUNIT_NONE );
                aMtrPosY.SetUserValue( aRect.Center().Y(), FUNIT_NONE );
                break;
            case RP_LB:
                aMtrPosX.SetUserValue( aRect.Left(), FUNIT_NONE );
                aMtrPosY.SetUserValue( aRect.Bottom(), FUNIT_NONE );
                break;
            case RP_MB:
                aMtrPosX.SetUserValue( aRect.Center().X(), FUNIT_NONE );
                aMtrPosY.SetUserValue( aRect.Bottom(), FUNIT_NONE );
                break;
            case RP_RB:
                aMtrPosX.SetUserValue( aRect.Right(), FUNIT_NONE );
                aMtrPosY.SetUserValue( aRect.Bottom(), FUNIT_NONE );
                break;
        }
    }
    else if( pWindow == &aCtlAngle )
    {
        switch( eRP )
        {
            case RP_LT: aMtrAngle.SetUserValue( 13500, FUNIT_NONE ); break;
            case RP_MT: aMtrAngle.SetUserValue(  9000, FUNIT_NONE ); break;
            case RP_RT: aMtrAngle.SetUserValue(  4500, FUNIT_NONE ); break;
            case RP_LM: aMtrAngle.SetUserValue( 18000, FUNIT_NONE ); break;
            case RP_RM: aMtrAngle.SetUserValue(     0, FUNIT_NONE ); break;
            case RP_LB: aMtrAngle.SetUserValue( 22500, FUNIT_NONE ); break;
            case RP_MB: aMtrAngle.SetUserValue( 27000, FUNIT_NONE ); break;
            case RP_RB: aMtrAngle.SetUserValue( 31500, FUNIT_NONE ); break;
        }
    }
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAngleTabPage, ModifiedHdl, void *, p )
{
    // Setzen des Winkels im AngleControl
    switch( aMtrAngle.GetValue() )
    {
        case 13500: aCtlAngle.SetActualRP( RP_LT ); break;
        case  9000: aCtlAngle.SetActualRP( RP_MT ); break;
        case  4500: aCtlAngle.SetActualRP( RP_RT ); break;
        case 18000: aCtlAngle.SetActualRP( RP_LM ); break;
        case     0: aCtlAngle.SetActualRP( RP_RM ); break;
        case 22500: aCtlAngle.SetActualRP( RP_LB ); break;
        case 27000: aCtlAngle.SetActualRP( RP_MB ); break;
        case 31500: aCtlAngle.SetActualRP( RP_RB ); break;
        default:    aCtlAngle.SetActualRP( RP_MM ); break;
    }

    return( 0L );
}

/*************************************************************************
|*
|*      Dialog zum Aendern des Eckenradius und zum Schraegstellen
|*
\************************************************************************/

SvxSlantTabPage::SvxSlantTabPage( Window* pParent, const SfxItemSet& rInAttrs  ) :
    SvxTabPage              ( pParent, ResId( RID_SVXPAGE_SLANT, DIALOG_MGR() ), rInAttrs ),

    aFlRadius               ( this, ResId( FL_RADIUS ) ),
    aFtRadius               ( this, ResId( FT_RADIUS ) ),
    aMtrRadius              ( this, ResId( MTR_FLD_RADIUS ) ),
    aFlAngle                ( this, ResId( FL_SLANT ) ),
    aFtAngle                ( this, ResId( FT_ANGLE ) ),
    aMtrAngle               ( this, ResId( MTR_FLD_ANGLE ) ),
    rOutAttrs               ( rInAttrs )
{
    FreeResource();

    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    // PoolUnit ermitteln
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    ePoolUnit = pPool->GetMetric( SID_ATTR_TRANSFORM_POS_X );
}

// -----------------------------------------------------------------------

void SvxSlantTabPage::Construct()
{
    // Setzen des Rechtecks
    DBG_ASSERT( pView, "Keine gueltige View Uebergeben!" );
    eDlgUnit = GetModuleFieldUnit( &GetItemSet() );
    SetFieldUnit( aMtrRadius, eDlgUnit, TRUE );

    aRect = pView->GetAllMarkedRect();
    pView->GetPageViewPvNum( 0 )->LogicToPagePos( aRect );
}

//------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxSlantTabPage, ModifiedHdl, void *, p )
{
    return( 0L );
}
IMPL_LINK_INLINE_END( SvxSlantTabPage, ModifiedHdl, void *, p )

// -----------------------------------------------------------------------

BOOL SvxSlantTabPage::FillItemSet( SfxItemSet& rAttrs )
{
    BOOL  bModified = FALSE;
    INT32 nValue = 0L;

    String aStr = aMtrRadius.GetText();
    if( aStr != aMtrRadius.GetSavedValue() )
    {
        Fraction aUIScale = pView->GetModel()->GetUIScale();
        long nTmp = GetCoreValue( aMtrRadius, ePoolUnit );
        nTmp = Fraction( nTmp ) * aUIScale;

        rAttrs.Put( SdrEckenradiusItem( nTmp ) );
        bModified = TRUE;
    }

    aStr = aMtrAngle.GetText();
    if( aStr != aMtrAngle.GetSavedValue() )
    {
        nValue = aMtrAngle.GetValue();
        rAttrs.Put( SfxInt32Item( SID_ATTR_TRANSFORM_SHEAR, nValue ) );
        bModified = TRUE;
    }

    if( bModified )
    {
        // Referenzpunkt setzen
        // #75897#
        Rectangle aObjectRect(pView->GetAllMarkedRect());
        pView->GetPageViewPvNum(0)->LogicToPagePos(aObjectRect);
        Point aPt = aObjectRect.Center();

        rAttrs.Put(SfxInt32Item(SID_ATTR_TRANSFORM_SHEAR_X, aPt.X()));
        rAttrs.Put(SfxInt32Item(SID_ATTR_TRANSFORM_SHEAR_Y, aPt.Y()));

        // Referenzpunkt setzen
//      Point aPt = aRect.Center();
//      rAttrs.Put( SfxInt32Item( SID_ATTR_TRANSFORM_SHEAR_X, aPt.X() ) );
//  rAttrs.Put( SfxInt32Item( SID_ATTR_TRANSFORM_SHEAR_Y, aPt.Y() ) );

        rAttrs.Put( SfxBoolItem( SID_ATTR_TRANSFORM_SHEAR_VERTICAL, FALSE ) );
    }
    return( bModified );
}

// -----------------------------------------------------------------------

void SvxSlantTabPage::Reset( const SfxItemSet& rAttrs )
{
    // Wenn die View selektierte Objekte besitzt, muessen entspr. Items,
    // die SFX_ITEM_DEFAULT sind, disabled werden
    BOOL bMarkedObj = pView->HasMarkedObj();
    const SfxPoolItem* pItem;

    // Eckenradius
//A if( bMarkedObj && SFX_ITEM_DEFAULT == rAttrs.GetItemState( SDRATTR_ECKENRADIUS ) )
//A {
    if( !pView->IsEdgeRadiusAllowed() )
    {
        aFlRadius.Disable();
        aFtRadius.Disable();
        aMtrRadius.Disable();
        aMtrRadius.SetText( String() );
    }
    else
    {
        pItem = GetItem( rAttrs, SDRATTR_ECKENRADIUS );

        if( pItem )
        {
            Fraction aUIScale = pView->GetModel()->GetUIScale();
            long nTmp = ( (const SdrEckenradiusItem*)pItem )->GetValue();
            nTmp = Fraction( nTmp ) / aUIScale;
            SetMetricValue( aMtrRadius, nTmp, ePoolUnit );
        }
        else
            aMtrRadius.SetText( String() );
    }
    aMtrRadius.SaveValue();

    // Schraegstellen: Winkel
    //SfxItemState eState = rAttrs.GetItemState( SID_ATTR_TRANSFORM_SHEAR );
    //if( ( bMarkedObj && SFX_ITEM_DEFAULT == eState ) ||
    if( !pView->IsShearAllowed() )
    {
        aFlAngle.Disable();
        aFtAngle.Disable();
        aMtrAngle.Disable();
        aMtrAngle.SetText( String() );
    }
    else
    {
        pItem = GetItem( rAttrs, SID_ATTR_TRANSFORM_SHEAR );

        if( pItem )
            aMtrAngle.SetValue( ( (const SfxInt32Item*)pItem )->GetValue() );
        else
            aMtrAngle.SetText( String() );
    }
    aMtrAngle.SaveValue();

    ModifiedHdl( this );
}

// -----------------------------------------------------------------------

SfxTabPage* SvxSlantTabPage::Create( Window* pWindow,
                const SfxItemSet& rOutAttrs )
{
    return( new SvxSlantTabPage( pWindow, rOutAttrs ) );
}

//------------------------------------------------------------------------

USHORT* SvxSlantTabPage::GetRanges()
{
    return( pSlantRanges );
}

// -----------------------------------------------------------------------

void SvxSlantTabPage::ActivatePage( const SfxItemSet& rSet )
{
    SfxRectangleItem* pRectItem = NULL;

    if( SFX_ITEM_SET == rSet.GetItemState( GetWhich( SID_ATTR_TRANSFORM_INTERN ) , FALSE,
                                    (const SfxPoolItem**) &pRectItem ) )
    {
        aRect = pRectItem->GetValue();
    }
}

// -----------------------------------------------------------------------

int SvxSlantTabPage::DeactivatePage( SfxItemSet* pSet )
{
    if( pSet )
        FillItemSet( *pSet );

    return( LEAVE_PAGE );
}

//------------------------------------------------------------------------

void SvxSlantTabPage::PointChanged( Window* pWindow, RECT_POINT eRP )
{
}


/*************************************************************************
|*
|*      Dialog for changing position and size of graphic objects
|*
\************************************************************************/

SvxPositionSizeTabPage::SvxPositionSizeTabPage( Window* pParent, const SfxItemSet& rInAttrs  ) :
    SvxTabPage      ( pParent, ResId( RID_SVXPAGE_POSITION_SIZE, DIALOG_MGR() ), rInAttrs ),
    maFlPosition        ( this, ResId( FL_POSITION ) ),
    maFtPosX            ( this, ResId( FT_POS_X ) ),
    maMtrPosX           ( this, ResId( MTR_FLD_POS_X ) ),
    maFtPosY            ( this, ResId( FT_POS_Y ) ),
    maMtrPosY           ( this, ResId( MTR_FLD_POS_Y ) ),
    maFlProtect         ( this, ResId( FL_PROTECT) ),
    maTsbPosProtect     ( this, ResId( TSB_POSPROTECT ) ),
    maFtPosReference    ( this, ResId( FT_POSREFERENCE ) ),
    maCtlPos            ( this, ResId( CTL_POSRECT ), RP_LT ),
    maAnchorBox      ( this, ResId( FL_ANCHOR ) ),
    maFtAnchor       ( this, ResId( FT_ANCHOR ) ),
    maDdLbAnchor     ( this, ResId( LB_ANCHOR ) ),
    maFtOrient       ( this, ResId( FT_ORIENT ) ),
    maDdLbOrient     ( this, ResId( LB_ORIENT ) ),
    mbPageDisabled   ( FALSE ),

    maFlSize                         ( this, ResId( FL_SIZE ) ),
    maFtWidth                        ( this, ResId( FT_WIDTH ) ),
    maMtrWidth                       ( this, ResId( MTR_FLD_WIDTH ) ),
    maFtHeight                       ( this, ResId( FT_HEIGHT ) ),
    maMtrHeight                      ( this, ResId( MTR_FLD_HEIGHT ) ),
    maCbxScale                       ( this, ResId( CBX_SCALE ) ),
    maTsbSizeProtect                 ( this, ResId( TSB_SIZEPROTECT ) ),
    maFtSizeReference                ( this, ResId( FT_SIZEREFERENCE) ),
    maCtlSize                        ( this, ResId( CTL_SIZERECT ), RP_LT ),
    maFlAdjust                       ( this, ResId( FL_ADJUST ) ),
    maTsbAutoGrowWidth              ( this, ResId( TSB_AUTOGROW_WIDTH ) ),
    maTsbAutoGrowHeight             ( this, ResId( TSB_AUTOGROW_HEIGHT ) ),

    maFlDivider                     (this, ResId( FL_DIVIDER ) ),
    mnProtectSizeState( STATE_NOCHECK ),
    mbProtectDisabled( false ),
    mbSizeDisabled( false ),

    mrOutAttrs       ( rInAttrs )
{
    FreeResource();

    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    // PoolUnit ermitteln
    SfxItemPool* pPool = mrOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    mePoolUnit = pPool->GetMetric( SID_ATTR_TRANSFORM_POS_X );

    maDdLbAnchor.SetSelectHdl( LINK( this, SvxPositionSizeTabPage, SetAnchorHdl ) );
    maDdLbOrient.SetSelectHdl( LINK( this, SvxPositionSizeTabPage, SetOrientHdl ) );

    meRP = RP_LT; // s.o.

    maMtrWidth.SetModifyHdl( LINK( this, SvxPositionSizeTabPage, ChangeWidthHdl ) );
    maMtrHeight.SetModifyHdl( LINK( this, SvxPositionSizeTabPage, ChangeHeightHdl ) );

    maCbxScale.SetClickHdl( LINK( this, SvxPositionSizeTabPage, ClickAutoHdl ) );

    maTsbAutoGrowWidth.Disable();
    maTsbAutoGrowHeight.Disable();
    maFlAdjust.Disable();

    // #i2379# disable controls when protected
    maTsbPosProtect.SetClickHdl( LINK( this, SvxPositionSizeTabPage, ChangePosProtectHdl ) );
    maTsbSizeProtect.SetClickHdl( LINK( this, SvxPositionSizeTabPage, ChangeSizeProtectHdl ) );

}

// -----------------------------------------------------------------------

void SvxPositionSizeTabPage::Construct()
{
    // Setzen des Rechtecks und der Workingarea
    DBG_ASSERT( mpView, "Keine gueltige View Uebergeben!" );
    meDlgUnit = GetModuleFieldUnit( &GetItemSet() );
    SetFieldUnit( maMtrPosX, meDlgUnit, TRUE );
    SetFieldUnit( maMtrPosY, meDlgUnit, TRUE );
    SetFieldUnit( maMtrWidth, meDlgUnit, TRUE );
    SetFieldUnit( maMtrHeight, meDlgUnit, TRUE );

    if( meDlgUnit == FUNIT_MILE ||
        meDlgUnit == FUNIT_KM )
    {
        maMtrPosX.SetDecimalDigits( 3 );
        maMtrPosY.SetDecimalDigits( 3 );
        maMtrWidth.SetDecimalDigits( 3 );
        maMtrHeight.SetDecimalDigits( 3 );

    }

    maRect = mpView->GetAllMarkedRect();
    mpView->GetPageViewPvNum( 0 )->LogicToPagePos( maRect );

    // WorkArea holen und umrechnen:
    maWorkArea = mpView->GetWorkArea();

    // Beruecksichtigung Ankerposition (bei Writer)
    const SdrMarkList& rMarkList = mpView->GetMarkList();
    if( rMarkList.GetMarkCount() >= 1 )
    {
        const SdrObject* pObj = rMarkList.GetMark( 0 )->GetObj();
        maAnchorPos = pObj->GetAnchorPos();

        if( maAnchorPos != Point(0,0) ) // -> Writer
        {
            for( USHORT i = 1; i < rMarkList.GetMarkCount(); i++ )
            {
                pObj = rMarkList.GetMark( i )->GetObj();
                if( maAnchorPos != pObj->GetAnchorPos() )
                {
                    // Unterschiedliche Ankerpositionen
                    maFtPosX.Disable();
                    maMtrPosX.Disable();
                    maMtrPosX.SetText( String() );
                    maFtPosY.Disable();
                    maMtrPosY.Disable();
                    maMtrPosY.SetText( String() );
                    maFlPosition.Disable();
                    maFtPosReference.Disable();
                    maCtlPos.Disable();
                    maTsbPosProtect.Disable();
                    mbPageDisabled = TRUE;
                    return;
                }
            }
        }
        Point aPt = maAnchorPos * -1;
        Point aPt2 = aPt;

        aPt += maWorkArea.TopLeft();
        maWorkArea.SetPos( aPt );

        aPt2 += maRect.TopLeft();
        maRect.SetPos( aPt2 );
    }

    // this should happen via SID_ATTR_TRANSFORM_AUTOSIZE
    if( rMarkList.GetMarkCount() == 1 )
    {
        const SdrObject* pObj = rMarkList.GetMark( 0 )->GetObj();
        SdrObjKind eKind = (SdrObjKind) pObj->GetObjIdentifier();
        if( ( pObj->GetObjInventor() == SdrInventor ) &&
            ( eKind==OBJ_TEXT || eKind==OBJ_TITLETEXT || eKind==OBJ_OUTLINETEXT) &&
            ( (SdrTextObj*) pObj )->HasText() )
        {
            maFlAdjust.Enable();
            maTsbAutoGrowWidth.Enable();
            maTsbAutoGrowHeight.Enable();
            maTsbAutoGrowWidth.SetClickHdl( LINK( this, SvxPositionSizeTabPage, ClickSizeProtectHdl ) );
            maTsbAutoGrowHeight.SetClickHdl( LINK( this, SvxPositionSizeTabPage, ClickSizeProtectHdl ) );

            // Wird als Flag benutzt, um zu ermitteln, ob anwaehlbar ist
            maTsbAutoGrowWidth.EnableTriState( FALSE );
            maTsbAutoGrowHeight.EnableTriState( FALSE );
        }
    }


    // use page offset and recalculate
    Point aPt( mpView->GetPageViewPvNum( 0 )->GetPageOrigin() );

    // Massstab
    Fraction aUIScale = mpView->GetModel()->GetUIScale();

    lcl_ScaleRect( maWorkArea, aUIScale );
    lcl_ScaleRect( maRect, aUIScale );
    lcl_ScalePoint( aPt, aUIScale );

    // Metrik konvertieren
    int nDigits = maMtrPosX.GetDecimalDigits();

    aPt = lcl_ConvertPoint( aPt, nDigits, (MapUnit) mePoolUnit, meDlgUnit );
    maWorkArea = lcl_ConvertRect( maWorkArea, nDigits, (MapUnit) mePoolUnit, meDlgUnit );
    maRect = lcl_ConvertRect( maRect, nDigits, (MapUnit) mePoolUnit, meDlgUnit );

    // use page offset
    aPt *= -1;
    aPt += maWorkArea.TopLeft();
    maWorkArea.SetPos( aPt );

    SetMinMaxPosition();
}

// -----------------------------------------------------------------------

BOOL SvxPositionSizeTabPage::FillItemSet( SfxItemSet& rOutAttrs )
{
    if ( maMtrWidth.HasFocus() )
        ChangeWidthHdl( this );
    if ( maMtrHeight.HasFocus() )
        ChangeHeightHdl( this );

    BOOL bModified = FALSE;
    if( !mbPageDisabled )
    {
        if ( maMtrPosX.IsValueModified() || maMtrPosY.IsValueModified() )
        {
            long lX = GetCoreValue( maMtrPosX, mePoolUnit );
            long lY = GetCoreValue( maMtrPosY, mePoolUnit );

            // Altes Rechteck mit CoreUnit
            maRect = mpView->GetAllMarkedRect();
            mpView->GetPageViewPvNum( 0 )->LogicToPagePos( maRect );

            Fraction aUIScale = mpView->GetModel()->GetUIScale();
            lX += maAnchorPos.X();
            lX = Fraction( lX ) * aUIScale;
            lY += maAnchorPos.Y();
            lY = Fraction( lY ) * aUIScale;

            // #101581# GetTopLeftPosition(...) needs coordinates
            // after UI scaling, in real PagePositions.
            GetTopLeftPosition( lX, lY, maRect );

            rOutAttrs.Put( SfxInt32Item( GetWhich( SID_ATTR_TRANSFORM_POS_X ), (INT32) lX ) );
            rOutAttrs.Put( SfxInt32Item( GetWhich( SID_ATTR_TRANSFORM_POS_Y ), (INT32) lY ) );

            bModified |= TRUE;
        }

        if ( maTsbPosProtect.GetState() != maTsbPosProtect.GetSavedValue() )
        {
            if( maTsbPosProtect.GetState() == STATE_DONTKNOW )
                rOutAttrs.InvalidateItem( SID_ATTR_TRANSFORM_PROTECT_POS );
            else
                rOutAttrs.Put(
                    SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_PROTECT_POS ),
                    maTsbPosProtect.GetState() == STATE_CHECK ? TRUE : FALSE ) );
            bModified |= TRUE;
        }
        if(maAnchorBox.IsVisible()) //nur fuer den Writer
        {
            if(maDdLbAnchor.GetSavedValue() != maDdLbAnchor.GetSelectEntryPos())
            {
                bModified |= TRUE;
                rOutAttrs.Put(SfxUInt16Item(
                        SID_ATTR_TRANSFORM_ANCHOR, (USHORT)(ULONG)maDdLbAnchor.GetEntryData(maDdLbAnchor.GetSelectEntryPos())));
            }
            if(maDdLbOrient.GetSavedValue() != maDdLbOrient.GetSelectEntryPos())
            {
                bModified |= TRUE;
                rOutAttrs.Put(SfxUInt16Item(
                        SID_ATTR_TRANSFORM_VERT_ORIENT, maDdLbOrient.GetSelectEntryPos()));
            }
        }
    }

    if ( maMtrWidth.IsValueModified() || maMtrHeight.IsValueModified() )
    {
        Fraction aUIScale = mpView->GetModel()->GetUIScale();

        // get Width
        double nWidth = maMtrWidth.GetValue( meDlgUnit );
        nWidth = MetricField::ConvertDoubleValue( nWidth, maMtrWidth.GetBaseValue(), maMtrWidth.GetDecimalDigits(), meDlgUnit, FUNIT_100TH_MM );
        long lWidth = nWidth * (double)aUIScale;
        lWidth = OutputDevice::LogicToLogic( lWidth, MAP_100TH_MM, (MapUnit)mePoolUnit );
        lWidth = maMtrWidth.Denormalize( lWidth );

        // get Height
        double nHeight = maMtrHeight.GetValue( meDlgUnit );
        nHeight = MetricField::ConvertDoubleValue( nHeight, maMtrHeight.GetBaseValue(), maMtrHeight.GetDecimalDigits(), meDlgUnit, FUNIT_100TH_MM );
        long lHeight = nHeight * (double)aUIScale;
        lHeight = OutputDevice::LogicToLogic( lHeight, MAP_100TH_MM, (MapUnit)mePoolUnit );
        lHeight = maMtrWidth.Denormalize( lHeight );

        // put Width & Height to itemset
        rOutAttrs.Put( SfxUInt32Item( GetWhich( SID_ATTR_TRANSFORM_WIDTH ),
                        (UINT32) lWidth ) );
        rOutAttrs.Put( SfxUInt32Item( GetWhich( SID_ATTR_TRANSFORM_HEIGHT ),
                        (UINT32) lHeight ) );
        rOutAttrs.Put( SfxAllEnumItem( GetWhich( SID_ATTR_TRANSFORM_SIZE_POINT ), meRP ) );
        bModified |= TRUE;
    }

    if ( maTsbSizeProtect.GetState() != maTsbSizeProtect.GetSavedValue() )
    {
        if ( maTsbSizeProtect.GetState() == STATE_DONTKNOW )
            rOutAttrs.InvalidateItem( SID_ATTR_TRANSFORM_PROTECT_SIZE );
        else
            rOutAttrs.Put(
                SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_PROTECT_SIZE ),
                maTsbSizeProtect.GetState() == STATE_CHECK ? TRUE : FALSE ) );
        bModified |= TRUE;
    }

    if ( maTsbAutoGrowWidth.GetState() != maTsbAutoGrowWidth.GetSavedValue() )
    {
        if ( !maTsbAutoGrowWidth.IsTriStateEnabled() )
        {
            if( maTsbAutoGrowWidth.GetState() == STATE_DONTKNOW )
                rOutAttrs.InvalidateItem( SID_ATTR_TRANSFORM_AUTOWIDTH );
            else
                rOutAttrs.Put(
                    SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_AUTOWIDTH ),
                    maTsbAutoGrowWidth.GetState() == STATE_CHECK ? TRUE : FALSE ) );
        }
        bModified |= TRUE;
    }

    if ( maTsbAutoGrowHeight.GetState() != maTsbAutoGrowHeight.GetSavedValue() )
    {
        if ( !maTsbAutoGrowHeight.IsTriStateEnabled() )
        {
            if( maTsbAutoGrowHeight.GetState() == STATE_DONTKNOW )
                rOutAttrs.InvalidateItem( SID_ATTR_TRANSFORM_AUTOHEIGHT );
            else
                rOutAttrs.Put(
                    SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_AUTOHEIGHT ),
                    maTsbAutoGrowHeight.GetState() == STATE_CHECK ? TRUE : FALSE ) );
        }
        bModified |= TRUE;
    }


    return bModified;
}

// -----------------------------------------------------------------------

void SvxPositionSizeTabPage::Reset( const SfxItemSet& rOutAttrs )
{
    const SfxPoolItem* pItem;

    if ( !mbPageDisabled )
    {
        pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_POS_X );

        Fraction aUIScale = mpView->GetModel()->GetUIScale();
        if ( pItem )
        {
            long nTmp = ( (const SfxInt32Item*)pItem )->GetValue() - maAnchorPos.X();
            nTmp = Fraction( nTmp ) / aUIScale;

            SetMetricValue( maMtrPosX, nTmp, mePoolUnit );
        }

        pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_POS_Y );
        if ( pItem )
        {
            long nTmp = ( (const SfxInt32Item*)pItem )->GetValue() - maAnchorPos.Y();
            nTmp = Fraction( nTmp ) / aUIScale;

            SetMetricValue( maMtrPosY, nTmp, mePoolUnit );
        }

        pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_PROTECT_POS );
        if ( pItem )
        {
            sal_Bool bProtected = ( ( const SfxBoolItem* )pItem )->GetValue();
            maTsbPosProtect.SetState( bProtected ? STATE_CHECK : STATE_NOCHECK );
            maTsbPosProtect.EnableTriState( FALSE );
            maTsbSizeProtect.Enable( !mbProtectDisabled & !bProtected );
        }
        else
        {
            maTsbPosProtect.SetState( STATE_DONTKNOW );
            maTsbSizeProtect.Enable( !mbProtectDisabled );
        }

        maTsbPosProtect.SaveValue();
        maCtlPos.Reset();

        // #i2379# Disable controls for protected objects
        ChangePosProtectHdl( this );

        if(maAnchorBox.IsVisible()) //nur fuer den Writer
        {
            pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_ANCHOR );
            USHORT nAnchorPos = 0;
            if(pItem)
            {
                nAnchorPos = ((const SfxUInt16Item*)pItem)->GetValue();
                for (USHORT i = 0; i < maDdLbAnchor.GetEntryCount(); i++)
                {
                    if ((ULONG)maDdLbAnchor.GetEntryData(i) == (ULONG)nAnchorPos)
                    {
                        maDdLbAnchor.SelectEntryPos(i);
                        break;
                    }
                }
                maDdLbAnchor.SaveValue();
                SetAnchorHdl(&maDdLbAnchor);
            }
            if(nAnchorPos == (USHORT)SVX_FLY_IN_CNTNT)
            {
                maCtlPos.Disable();
                pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_VERT_ORIENT );
                if(pItem)
                {
                    maDdLbOrient.SelectEntryPos(((const SfxUInt16Item*)pItem)->GetValue());
                }
            }
            else
            {
                maDdLbOrient.SelectEntryPos(     (USHORT)SVX_VERT_LINE_CENTER );
            }
            maDdLbOrient.SaveValue();
            SetOrientHdl(&maDdLbOrient);
            maCtlPos.Invalidate();
        }
    }

    pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_WIDTH );
    mlOldWidth = Max( pItem ? ( (const SfxUInt32Item*)pItem )->GetValue() : 0, (UINT32)1 );

    pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_HEIGHT );
    mlOldHeight = Max( pItem ? ( (const SfxUInt32Item*)pItem )->GetValue() : 0, (UINT32)1 );

    Fraction aUIScale = mpView->GetModel()->GetUIScale();

    // set Width & Height
    double nTmpWidth  = (double)OutputDevice::LogicToLogic( mlOldWidth, (MapUnit)mePoolUnit, MAP_100TH_MM );
    double nTmpHeight = (double)OutputDevice::LogicToLogic( mlOldHeight, (MapUnit)mePoolUnit, MAP_100TH_MM );
    nTmpWidth  = Fraction( nTmpWidth ) / aUIScale;
    nTmpHeight = Fraction( nTmpHeight ) / aUIScale;

    UINT32 nNorm = 10;
    for( int i=0; i<maMtrWidth.GetDecimalDigits()-1; i++ )
        nNorm*=10;
    nTmpWidth*=(double)nNorm;

    nNorm = 10;
    for( i=0; i<maMtrHeight.GetDecimalDigits()-1; i++ )
        nNorm*=10;
    nTmpHeight*=(double)nNorm;

    nTmpWidth =  MetricField::ConvertDoubleValue( (double)nTmpWidth, maMtrWidth.GetBaseValue(), maMtrWidth.GetDecimalDigits(), FUNIT_100TH_MM, meDlgUnit );
    nTmpHeight = MetricField::ConvertDoubleValue( (double)nTmpHeight, maMtrHeight.GetBaseValue(), maMtrHeight.GetDecimalDigits(), FUNIT_100TH_MM, meDlgUnit );

    maMtrWidth.SetValue( nTmpWidth, meDlgUnit );
    maMtrHeight.SetValue( nTmpHeight, meDlgUnit );

    pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_PROTECT_SIZE );

    if ( pItem )
    {
        maTsbSizeProtect.SetState( ( (const SfxBoolItem*)pItem )->GetValue()
                              ? STATE_CHECK : STATE_NOCHECK );
        maTsbSizeProtect.EnableTriState( FALSE );
    }
    else
        maTsbSizeProtect.SetState( STATE_DONTKNOW );

    pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_AUTOWIDTH );
    if ( pItem )
    {
        maTsbAutoGrowWidth.SetState( ( ( const SfxBoolItem* )pItem )->GetValue()
                           ? STATE_CHECK : STATE_NOCHECK );
    }
    else
        maTsbAutoGrowWidth.SetState( STATE_DONTKNOW );

    pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_AUTOHEIGHT );
    if ( pItem )
    {
        maTsbAutoGrowHeight.SetState( ( ( const SfxBoolItem* )pItem )->GetValue()
                           ? STATE_CHECK : STATE_NOCHECK );
    }
    else
        maTsbAutoGrowHeight.SetState( STATE_DONTKNOW );

    // Ist Abgleich gesetzt?
    String aStr = GetUserData();
    maCbxScale.Check( (BOOL)aStr.ToInt32() );

    maTsbSizeProtect.SaveValue();
    maTsbAutoGrowWidth.SaveValue();
    maTsbAutoGrowHeight.SaveValue();
    ClickSizeProtectHdl( NULL );

    // #i2379# Disable controls for protected objects
    ChangeSizeProtectHdl( this );
}

// -----------------------------------------------------------------------

SfxTabPage* SvxPositionSizeTabPage::Create( Window* pWindow,
                const SfxItemSet& rOutAttrs )
{
    return( new SvxPositionSizeTabPage( pWindow, rOutAttrs ) );
}

//------------------------------------------------------------------------

USHORT* SvxPositionSizeTabPage::GetRanges()
{
    return( pPosSizeRanges );
}

// -----------------------------------------------------------------------

void SvxPositionSizeTabPage::ActivatePage( const SfxItemSet& rSet )
{
    SfxRectangleItem* pRectItem = NULL;

    if( SFX_ITEM_SET == rSet.GetItemState( GetWhich( SID_ATTR_TRANSFORM_INTERN ) , FALSE,
                                    (const SfxPoolItem**) &pRectItem ) )
    {
        // Setzen der MinMax-Position
        maRect = pRectItem->GetValue();
        SetMinMaxPosition();
    }
}

// -----------------------------------------------------------------------

int SvxPositionSizeTabPage::DeactivatePage( SfxItemSet* pSet )
{
    if( pSet )
    {
        INT32 lX = maMtrPosX.GetValue();
        INT32 lY = maMtrPosY.GetValue();

        // #106330#
        // The below BugFix assumed that GetTopLeftPosition()
        // needs special coordinate systems. This is not true,
        // GetTopLeftPosition() just needs all parameters in one
        // coor system. Thus, this part below is not necessary and
        // leads to this new bug. Thus, i remove it again.
        // I checked that #106330# works and #101581# is fixed, too.
        //
        // // #101581# GetTopLeftPosition(...) needs coordinates
        // // after UI scaling, in real PagePositions. Thus I added
        // // that calculation here
        // Fraction aUIScale = mpView->GetModel()->GetUIScale();
        // lX += maAnchorPos.X();
        // lX = Fraction( lX ) * aUIScale;
        // lY += maAnchorPos.Y();
        // lY = Fraction( lY ) * aUIScale;

        GetTopLeftPosition( lX, lY, maRect );

        maRect.SetPos( Point( lX, lY ) );

        pSet->Put( SfxRectangleItem( SID_ATTR_TRANSFORM_INTERN, maRect ) );

        FillItemSet( *pSet );
    }
    return( LEAVE_PAGE );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxPositionSizeTabPage, ChangePosProtectHdl, void *, p )
{
    maTsbSizeProtect.Enable( !mbProtectDisabled & (maTsbPosProtect.GetState() != STATE_CHECK) );

    // #106572# Remember user's last choice
    maTsbSizeProtect.SetState( maTsbPosProtect.GetState() == STATE_CHECK ?
                               STATE_CHECK : mnProtectSizeState );

    DisableSizeControls();

    if( maTsbPosProtect.GetState() == STATE_CHECK )
    {
        maFlPosition.Disable();
        maFtPosX.Disable();
        maMtrPosX.Disable();
        maFtPosY.Disable();
        maMtrPosY.Disable();
        maFtPosReference.Disable();
        maCtlPos.Disable();
        maCtlPos.Invalidate();
    }
    else
    {
        maFlPosition.Enable();
        maFtPosX.Enable();
        maMtrPosX.Enable();
        maFtPosY.Enable();
        maMtrPosY.Enable();
        maFtPosReference.Enable();
        maCtlPos.Enable();
        maCtlPos.Invalidate();
    }

    return( 0L );
}

//------------------------------------------------------------------------

void SvxPositionSizeTabPage::DisableSizeControls()
{
    if( maTsbSizeProtect.GetState() == STATE_CHECK )
    {
        maFlSize.Disable();
        maFtWidth.Disable();
        maMtrWidth.Disable();
        maFtHeight.Disable();
        maMtrHeight.Disable();
        maCbxScale.Disable();
        maFtSizeReference.Disable();
        maCtlSize.Disable();
        maCtlSize.Invalidate();
    }
    else
    {
        maFlSize.Enable();
        maFtWidth.Enable();
        maMtrWidth.Enable();
        maFtHeight.Enable();
        maMtrHeight.Enable();
        maCbxScale.Enable();
        maFtSizeReference.Enable();
        maCtlSize.Enable();
        maCtlSize.Invalidate();
    }
}

//------------------------------------------------------------------------

IMPL_LINK( SvxPositionSizeTabPage, ChangeSizeProtectHdl, void *, p )
{
    if( maTsbSizeProtect.IsEnabled() )
    {
        // #106572# Remember user's last choice

        // Note: this works only as long as the dialog is open.  When
        // the user closes the dialog, there is no way to remember
        // whether size was enabled or disabled befor pos protect was
        // clicked. Thus, if pos protect is selected, the dialog is
        // closed and reopened again, unchecking pos protect will
        // always uncheck size protect, too. That's life.
        mnProtectSizeState = maTsbSizeProtect.GetState();
    }

    DisableSizeControls();

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxPositionSizeTabPage, ChangePosXHdl, void *, EMPTYARG )
{
    return( 0L );
}
IMPL_LINK_INLINE_END( SvxPositionSizeTabPage, ChangePosXHdl, void *, EMPTYARG )

//------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxPositionSizeTabPage, ChangePosYHdl, void *, EMPTYARG )
{
    return( 0L );
}
IMPL_LINK_INLINE_END( SvxPositionSizeTabPage, ChangePosYHdl, void *, EMPTYARG )

//------------------------------------------------------------------------

void SvxPositionSizeTabPage::SetMinMaxPosition()
{
    // position
    Rectangle aTmpRect = maWorkArea;

    switch ( maCtlPos.GetActualRP() )
    {
        case RP_LT:
            aTmpRect.Right()  -= maRect.Right() - maRect.Left();
            aTmpRect.Bottom() -= maRect.Bottom() - maRect.Top();
            break;
        case RP_MT:
            aTmpRect.Left()   += maRect.Center().X() - maRect.Left();
            aTmpRect.Right()  -= maRect.Center().X() - maRect.Left();
            aTmpRect.Bottom() -= maRect.Bottom() - maRect.Top();
            break;
        case RP_RT:
            aTmpRect.Left()   += maRect.Right() - maRect.Left();
            aTmpRect.Bottom() -= maRect.Bottom() - maRect.Top();
            break;
        case RP_LM:
            aTmpRect.Right()  -= maRect.Right() - maRect.Left();
            aTmpRect.Top()    += maRect.Center().Y() - maRect.Top();
            aTmpRect.Bottom() -= maRect.Center().Y() - maRect.Top();
            break;
        case RP_MM:
            aTmpRect.Left()   += maRect.Center().X() - maRect.Left();
            aTmpRect.Right()  -= maRect.Center().X() - maRect.Left();
            aTmpRect.Top()    += maRect.Center().Y() - maRect.Top();
            aTmpRect.Bottom() -= maRect.Center().Y() - maRect.Top();
            break;
        case RP_RM:
            aTmpRect.Left()   += maRect.Right() - maRect.Left();
            aTmpRect.Top()    += maRect.Center().Y() - maRect.Top();
            aTmpRect.Bottom() -= maRect.Center().Y() - maRect.Top();
            break;
        case RP_LB:
            aTmpRect.Right()  -= maRect.Right() - maRect.Left();
            aTmpRect.Top()    += maRect.Bottom() - maRect.Top();
            break;
        case RP_MB:
            aTmpRect.Left()   += maRect.Center().X() - maRect.Left();
            aTmpRect.Right()  -= maRect.Center().X() - maRect.Left();
            aTmpRect.Top()    += maRect.Bottom() - maRect.Top();
            break;
        case RP_RB:
            aTmpRect.Left()   += maRect.Right() - maRect.Left();
            aTmpRect.Top()    += maRect.Bottom() - maRect.Top();
            break;
    }

    long nMaxLong = MetricField::ConvertValue( LONG_MAX, 0, MAP_100TH_MM, meDlgUnit ) - 1L;

    if( Abs( aTmpRect.Left() ) > nMaxLong )
    {
        long nMult = aTmpRect.Left() < 0 ? -1 : 1;
        aTmpRect.Left() = nMaxLong * nMult;
    }
    if( Abs( aTmpRect.Right() ) > nMaxLong )
    {
        long nMult = aTmpRect.Right() < 0 ? -1 : 1;
        aTmpRect.Right() = nMaxLong * nMult;
    }
    if( Abs( aTmpRect.Top() ) > nMaxLong )
    {
        long nMult = aTmpRect.Top() < 0 ? -1 : 1;
        aTmpRect.Top() = nMaxLong * nMult;
    }
    if( Abs( aTmpRect.Bottom() ) > nMaxLong )
    {
        long nMult = aTmpRect.Bottom() < 0 ? -1 : 1;
        aTmpRect.Bottom() = nMaxLong * nMult;
    }

    maMtrPosX.SetMin( aTmpRect.Left() );
    maMtrPosX.SetFirst( aTmpRect.Left() );
    maMtrPosX.SetMax( aTmpRect.Right() );
    maMtrPosX.SetLast( aTmpRect.Right() );

    maMtrPosY.SetMin( aTmpRect.Top() );
    maMtrPosY.SetFirst( aTmpRect.Top() );
    maMtrPosY.SetMax( aTmpRect.Bottom() );
    maMtrPosY.SetLast( aTmpRect.Bottom() );

    // size
    aTmpRect = maWorkArea;

    switch ( maCtlSize.GetActualRP() )
    {
        case RP_LT:
            aTmpRect.SetSize( Size(
                aTmpRect.GetWidth() - ( maRect.Left() - aTmpRect.Left() ),
                aTmpRect.GetHeight() - ( maRect.Top() - aTmpRect.Top() ) ) );
            break;
        case RP_MT:
            aTmpRect.SetSize( Size(
                    Min( maRect.Center().X() - aTmpRect.Left(),
                         aTmpRect.Right() - maRect.Center().X() ) * 2,
                aTmpRect.GetHeight() - ( maRect.Top() - aTmpRect.Top() ) ) );
            break;
        case RP_RT:
            aTmpRect.SetSize( Size(
                aTmpRect.GetWidth() - ( aTmpRect.Right() - maRect.Right() ),
                aTmpRect.GetHeight() - ( maRect.Top() - aTmpRect.Top() ) ) );
            break;
        case RP_LM:
            aTmpRect.SetSize( Size(
                aTmpRect.GetWidth() - ( maRect.Left() - aTmpRect.Left() ),
                    Min( maRect.Center().Y() - aTmpRect.Top(),
                          aTmpRect.Bottom() - maRect.Center().Y() ) * 2 ) );
            break;
        case RP_MM:
        {
            long n1, n2, n3, n4, n5, n6;
            n1 = maRect.Center().X() - aTmpRect.Left();
            n2 = aTmpRect.Right() - maRect.Center().X();
            n3 = Min( n1, n2 );
            n4 = maRect.Center().Y() - aTmpRect.Top();
            n5 = aTmpRect.Bottom() - maRect.Center().Y();
            n6 = Min( n4, n5 );
            aTmpRect.SetSize( Size( n3 * 2, n6 * 3 ) );
            break;
        }
        case RP_RM:
            aTmpRect.SetSize( Size(
                aTmpRect.GetWidth() - ( aTmpRect.Right() - maRect.Right() ),
                    Min( maRect.Center().Y() - aTmpRect.Top(),
                          aTmpRect.Bottom() - maRect.Center().Y() ) * 2 ) );
            break;
        case RP_LB:
            aTmpRect.SetSize( Size(
                aTmpRect.GetWidth() - ( maRect.Left() - aTmpRect.Left() ),
                aTmpRect.GetHeight() - ( aTmpRect.Bottom() - maRect.Bottom() ) ) );
            break;
        case RP_MB:
            aTmpRect.SetSize( Size(
                    Min( maRect.Center().X() - aTmpRect.Left(),
                             aTmpRect.Right() - maRect.Center().X() ) * 2,
                aTmpRect.GetHeight() - ( maRect.Bottom() - aTmpRect.Bottom() ) ) );
            break;
        case RP_RB:
            aTmpRect.SetSize( Size(
                aTmpRect.GetWidth() - ( aTmpRect.Right() - maRect.Right() ),
                aTmpRect.GetHeight() - ( aTmpRect.Bottom() - maRect.Bottom() ) ) );
            break;
    }

    maMtrWidth.SetMax( aTmpRect.GetWidth() );
    maMtrWidth.SetLast( aTmpRect.GetWidth() );

    maMtrHeight.SetMax( aTmpRect.GetHeight() );
    maMtrHeight.SetLast( aTmpRect.GetHeight() );

}

//------------------------------------------------------------------------

void SvxPositionSizeTabPage::GetTopLeftPosition( INT32& rX, INT32& rY,
                                                const Rectangle& rRect )
{
    switch ( maCtlPos.GetActualRP() )
    {
        case RP_LT:
            break;
        case RP_MT:
            rX = rX- ( rRect.Center().X() - rRect.Left() );
            break;
        case RP_RT:
            rX = rX- ( rRect.Right() - rRect.Left() );
            break;
        case RP_LM:
            rY = rY- ( rRect.Center().Y() - rRect.Top() );
            break;
        case RP_MM:
            rX = rX- ( rRect.Center().X() - rRect.Left() );
            rY = rY- ( rRect.Center().Y() - rRect.Top() );
            break;
        case RP_RM:
            rX = rX- ( rRect.Right() - rRect.Left() );
            rY = rY- ( rRect.Center().Y() - rRect.Top() );
            break;
        case RP_LB:
            rY = rY- ( rRect.Bottom() - rRect.Top() );
            break;
        case RP_MB:
            rX = rX- ( rRect.Center().X() - rRect.Left() );
            rY = rY- ( rRect.Bottom() - rRect.Top() );
            break;
        case RP_RB:
            rX = rX- ( rRect.Right() - rRect.Left() );
            rY = rY- ( rRect.Bottom() - rRect.Top() );
            break;
    }
}

//------------------------------------------------------------------------

void SvxPositionSizeTabPage::PointChanged( Window* pWindow, RECT_POINT eRP )
{
    if( pWindow == &maCtlPos )
    {
        SetMinMaxPosition();
        switch( eRP )
        {
            case RP_LT:
                maMtrPosX.SetValue( maRect.Left() );
                maMtrPosY.SetValue( maRect.Top() );
                break;
            case RP_MT:
                maMtrPosX.SetValue( maRect.Center().X() );
                maMtrPosY.SetValue( maRect.Top() );
                break;
            case RP_RT:
                maMtrPosX.SetValue( maRect.Right() );
                maMtrPosY.SetValue( maRect.Top() );
                break;
            case RP_LM:
                maMtrPosX.SetValue( maRect.Left() );
                maMtrPosY.SetValue( maRect.Center().Y() );
                break;
            case RP_MM:
                maMtrPosX.SetValue( maRect.Center().X() );
                maMtrPosY.SetValue( maRect.Center().Y() );
                break;
            case RP_RM:
                maMtrPosX.SetValue( maRect.Right() );
                maMtrPosY.SetValue( maRect.Center().Y() );
                break;
            case RP_LB:
                maMtrPosX.SetValue( maRect.Left() );
                maMtrPosY.SetValue( maRect.Bottom() );
                break;
            case RP_MB:
                maMtrPosX.SetValue( maRect.Center().X() );
                maMtrPosY.SetValue( maRect.Bottom() );
                break;
            case RP_RB:
                maMtrPosX.SetValue( maRect.Right() );
                maMtrPosY.SetValue( maRect.Bottom() );
                break;
        }
    }
    else
    {
        meRP = eRP;

        Rectangle aTmpRect( GetRect() );
        SetMinMaxPosition();
    }
}

//------------------------------------------------------------------------

void SvxPositionSizeTabPage::ShowAnchorCtrls(USHORT nAnchorCtrls)
{
    maTsbAutoGrowWidth.Hide();
    maTsbAutoGrowHeight.Hide();
    maFlAdjust.Hide();

    maAnchorBox      .Show();
    maFtAnchor       .Show();
    maFtOrient       .Show();
    maDdLbOrient     .Show();

    for (USHORT i = 0; i < maDdLbAnchor.GetEntryCount(); i++)
        maDdLbAnchor.SetEntryData(i, (void *)(long)i);

    if (!(nAnchorCtrls & SVX_OBJ_AT_FLY))
        maDdLbAnchor.RemoveEntry(3);
    if (!(nAnchorCtrls & SVX_OBJ_PAGE))
        maDdLbAnchor.RemoveEntry(2);
    if (!(nAnchorCtrls & SVX_OBJ_IN_CNTNT))
        maDdLbAnchor.RemoveEntry(1);
    if (!(nAnchorCtrls & SVX_OBJ_AT_CNTNT))
        maDdLbAnchor.RemoveEntry(0);

    maDdLbAnchor     .Show();
};

//------------------------------------------------------------------------

void SvxPositionSizeTabPage::DisableResize()
{
    maFlSize.Disable();
    maFtWidth.Disable();
    maMtrWidth.Disable();
    maFtHeight.Disable();
    maMtrHeight.Disable();
    maCbxScale.Disable();
    maFtSizeReference.Disable();
    maCtlSize.Disable();
    mbSizeDisabled = true;
}

//------------------------------------------------------------------------

void SvxPositionSizeTabPage::DisableProtect()
{
    maFlProtect.Disable();
    maTsbPosProtect.Disable();
    maTsbSizeProtect.Disable();
    mbProtectDisabled = true;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxPositionSizeTabPage, SetAnchorHdl, ListBox *, pBox)
{
    BOOL bDisable = TRUE;
    switch( (ULONG)pBox->GetEntryData(pBox->GetSelectEntryPos()) )
    {
        case SVX_FLY_AT_CNTNT:
        case SVX_FLY_PAGE:
        break;
        case SVX_FLY_IN_CNTNT: bDisable = FALSE;
        break;
    }
    maCtlPos.Enable(bDisable);
    maCtlPos.Invalidate();
    if(bDisable)
    {
        maDdLbOrient.Disable();
        maFtOrient.Disable();
        maMtrPosX.Enable();
        maMtrPosY.Enable();
    }
    else
    {
        maMtrPosX.Disable();
        maDdLbOrient.Enable();
        maFtOrient.Enable();
        SetOrientHdl(&maDdLbOrient);
    }
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxPositionSizeTabPage, SetOrientHdl, ListBox *, pBox )
{
    if(pBox->IsEnabled())
    switch( pBox->GetSelectEntryPos() )
    {
        case SVX_VERT_TOP         :
        case SVX_VERT_CENTER      :
        case SVX_VERT_BOTTOM      :
        case SVX_VERT_LINE_TOP    :
        case SVX_VERT_LINE_CENTER :
        case SVX_VERT_LINE_BOTTOM :
                    maMtrPosY.Disable();
        break;
        case SVX_VERT_NONE:
                    maMtrPosY.Enable();
        break;
    }
    return 0;
}

Rectangle SvxPositionSizeTabPage::GetRect()
{
    Rectangle aTmpRect( maRect );
    aTmpRect.SetSize( Size( maMtrWidth.GetValue(), maMtrHeight.GetValue() ) );

    switch ( maCtlSize.GetActualRP() )
    {
        case RP_LT:
            // nichts!
            break;
        case RP_MT:
            aTmpRect.SetPos( Point( maRect.Left() -
                        ( aTmpRect.Right() - maRect.Right() ) / 2, maRect.Top() ) );
            break;
        case RP_RT:
            aTmpRect.SetPos( Point( maRect.Left() -
                        ( aTmpRect.Right() - maRect.Right() ), maRect.Top() ) );
            break;
        case RP_LM:
            aTmpRect.SetPos( Point( maRect.Left(), maRect.Top() -
                        ( aTmpRect.Bottom() - maRect.Bottom() ) / 2 ) );
            break;
        case RP_MM:
            aTmpRect.SetPos( Point( maRect.Left() -
                        ( aTmpRect.Right() - maRect.Right() ) / 2, maRect.Top() -
                        ( aTmpRect.Bottom() - maRect.Bottom() ) / 2 ) );
            break;
        case RP_RM:
            aTmpRect.SetPos( Point( maRect.Left() -
                        ( aTmpRect.Right() - maRect.Right() ), maRect.Top() -
                        ( aTmpRect.Bottom() - maRect.Bottom() ) / 2 ) );
            break;
        case RP_LB:
            aTmpRect.SetPos( Point( maRect.Left(), maRect.Top() -
                        ( aTmpRect.Bottom() - maRect.Bottom() ) ) );
            break;
        case RP_MB:
            aTmpRect.SetPos( Point( maRect.Left() -
                        ( aTmpRect.Right() - maRect.Right() ) / 2, maRect.Top() -
                        ( aTmpRect.Bottom() - maRect.Bottom() ) ) );
            break;
        case RP_RB:
            aTmpRect.SetPos( Point( maRect.Left() -
                        ( aTmpRect.Right() - maRect.Right() ), maRect.Top() -
                        ( aTmpRect.Bottom() - maRect.Bottom() ) ) );
            break;
    }
    return( aTmpRect );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxPositionSizeTabPage, ChangeWidthHdl, void *, EMPTYARG )
{
    if( maCbxScale.IsChecked() &&
        maCbxScale.IsEnabled() )
    {
        long nHeight = (long) ( ((double) mlOldHeight * (double) maMtrWidth.GetValue()) / (double) mlOldWidth );
        if( nHeight <= maMtrHeight.GetMax( FUNIT_NONE ) )
        {
            maMtrHeight.SetUserValue( nHeight, FUNIT_NONE );
        }
        else
        {
            nHeight = maMtrHeight.GetMax( FUNIT_NONE );
            maMtrHeight.SetUserValue( nHeight );
            const long nWidth = (long) ( ((double) mlOldWidth * (double) nHeight) / (double) mlOldHeight );
            maMtrWidth.SetUserValue( nWidth, FUNIT_NONE );
        }
    }
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxPositionSizeTabPage, ChangeHeightHdl, void *, EMPTYARG )
{
    if( maCbxScale.IsChecked() &&
        maCbxScale.IsEnabled() )
    {
        long nWidth = (long) ( ((double) mlOldWidth * (double) maMtrHeight.GetValue()) / (double) mlOldHeight );
        if( nWidth <= maMtrWidth.GetMax( FUNIT_NONE ) )
        {
            maMtrWidth.SetUserValue( nWidth, FUNIT_NONE );
        }
        else
        {
            nWidth = maMtrWidth.GetMax( FUNIT_NONE );
            maMtrWidth.SetUserValue( nWidth );
            const long nHeight = (long) ( ((double) mlOldHeight * (double) nWidth) / (double) mlOldWidth );
            maMtrHeight.SetUserValue( nHeight, FUNIT_NONE );
        }
    }
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxPositionSizeTabPage, ClickSizeProtectHdl, void *, p )
{
    if( !mbSizeDisabled )
    {
        BOOL bHeightChecked = !maTsbAutoGrowHeight.IsTriStateEnabled() &&
                              maTsbAutoGrowHeight.GetState() == STATE_CHECK;
        BOOL bWidthChecked = !maTsbAutoGrowWidth.IsTriStateEnabled() &&
                             maTsbAutoGrowWidth.GetState() == STATE_CHECK;
        if( p == &maTsbAutoGrowHeight || p == NULL )
        {
            if( bHeightChecked )
            {
                maFtHeight.Disable();
                maMtrHeight.Disable();
                maCbxScale.Disable();
            }
            else
            {
                maFtHeight.Enable();
                maMtrHeight.Enable();
                if( !bWidthChecked )
                    maCbxScale.Enable();
            }
        }
        if( p == &maTsbAutoGrowWidth || p == NULL )
        {
            if( bWidthChecked )
            {
                maFtWidth.Disable();
                maMtrWidth.Disable();
                maCbxScale.Disable();
            }
            else
            {
                maFtWidth.Enable();
                maMtrWidth.Enable();
                if( !bHeightChecked )
                    maCbxScale.Enable();
            }
        }
    }
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxPositionSizeTabPage, ClickAutoHdl, void *, p )
{
    if( maCbxScale.IsChecked() )
    {
        mlOldWidth  = Max( GetCoreValue( maMtrWidth,  mePoolUnit ), 1L );
        mlOldHeight = Max( GetCoreValue( maMtrHeight, mePoolUnit ), 1L );
    }
    return( 0L );
}

//------------------------------------------------------------------------

void SvxPositionSizeTabPage::FillUserData()
{
    // Abgleich wird in der Ini-Datei festgehalten
    UniString aStr = UniString::CreateFromInt32( (sal_Int32) maCbxScale.IsChecked() );
    SetUserData( aStr );
}

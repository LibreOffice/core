/*************************************************************************
 *
 *  $RCSfile: transfrm.cxx,v $
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

#pragma hdrstop

#define ITEMID_SIZE 0

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif

#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_SAVEOPT_HXX //autogen
#include <sfx2/saveopt.hxx>
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

    AddTabPage( RID_SVXPAGE_POSITION, SvxPositionTabPage::Create,
                            SvxPositionTabPage::GetRanges );
    AddTabPage( RID_SVXPAGE_SIZE, SvxSizeTabPage::Create,
                            SvxSizeTabPage::GetRanges );
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
        case RID_SVXPAGE_POSITION:
            ( (SvxPositionTabPage&) rPage ).SetView( pView );
            ( (SvxPositionTabPage&) rPage ).Construct();
            if(nAnchorCtrls)
                ( (SvxPositionTabPage&) rPage ).ShowAnchorCtrls(nAnchorCtrls);
        break;

        case RID_SVXPAGE_SIZE:
            ( (SvxSizeTabPage&) rPage ).SetView( pView );
            ( (SvxSizeTabPage&) rPage ).Construct();
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
|*      Dialog zum Aendern der Position der Grafikobjekte
|*
\************************************************************************/

SvxPositionTabPage::SvxPositionTabPage( Window* pParent, const SfxItemSet& rInAttrs  ) :
    SvxTabPage      ( pParent, ResId( RID_SVXPAGE_POSITION, DIALOG_MGR() ), rInAttrs ),
    aFtPosX         ( this, ResId( FT_POS_X ) ),
    aMtrPosX        ( this, ResId( MTR_FLD_POS_X ) ),
    aFtPosY         ( this, ResId( FT_POS_Y ) ),
    aMtrPosY        ( this, ResId( MTR_FLD_POS_Y ) ),
    aGrpPosition    ( this, ResId( GRP_POSITION ) ),
    aCtl            ( this, ResId( CTL_RECT ), RP_LT ),
    aAnchorBox      ( this, ResId( GB_ANCHOR ) ),
    aFtAnchor       ( this, ResId( FT_ANCHOR ) ),
    aDdLbAnchor     ( this, ResId( LB_ANCHOR ) ),
    aFtOrient       ( this, ResId( FT_ORIENT ) ),
    aDdLbOrient     ( this, ResId( LB_ORIENT ) ),
    aTsbProtect     ( this, ResId( TSB_PROTECT ) ),
    bPageDisabled   ( FALSE ),
    rOutAttrs       ( rInAttrs )
{
    FreeResource();

    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    // PoolUnit ermitteln
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    ePoolUnit = pPool->GetMetric( SID_ATTR_TRANSFORM_POS_X );

    // Wird nicht mehr gebraucht:
    // aTsbProtect.SetClickHdl( LINK( this, SvxPositionTabPage, ClickProtectHdl ) );
    aDdLbAnchor.SetSelectHdl( LINK( this, SvxPositionTabPage, SetAnchorHdl ) );
    aDdLbOrient.SetSelectHdl( LINK( this, SvxPositionTabPage, SetOrientHdl ) );
    // Bis zur 364 sollte die Verankerung "Am Rahmen" nicht freigeschaltet sein
#if SUPD <= 364
    aDdLbAnchor.RemoveEntry( 3 );
#endif
}

// -----------------------------------------------------------------------

void SvxPositionTabPage::Construct()
{
    // Setzen des Rechtecks und der Workingarea
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

    // WorkArea holen und umrechnen:
    aWorkArea = pView->GetWorkArea();

    // Beruecksichtigung Ankerposition (bei Writer)
    const SdrMarkList& rMarkList = pView->GetMarkList();
    if( rMarkList.GetMarkCount() >= 1 )
    {
        const SdrObject* pObj = rMarkList.GetMark( 0 )->GetObj();
        aAnchorPos = pObj->GetAnchorPos();

        if( aAnchorPos != Point(0,0) ) // -> Writer
        {
            for( USHORT i = 1; i < rMarkList.GetMarkCount(); i++ )
            {
                pObj = rMarkList.GetMark( i )->GetObj();
                if( aAnchorPos != pObj->GetAnchorPos() )
                {
                    // Unterschiedliche Ankerpositionen
                    aFtPosX.Disable();
                    aMtrPosX.Disable();
                    aMtrPosX.SetText( String() );
                    aFtPosY.Disable();
                    aMtrPosY.Disable();
                    aMtrPosY.SetText( String() );
                    aGrpPosition.Disable();
                    aCtl.Disable();
                    aTsbProtect.Disable();
                    bPageDisabled = TRUE;
                    return;
                }
            }
        }
        Point aPt = aAnchorPos * -1;
        Point aPt2 = aPt;

        aPt += aWorkArea.TopLeft();
        aWorkArea.SetPos( aPt );

        aPt2 += aRect.TopLeft();
        aRect.SetPos( aPt2 );
    }

    // Beruecksictigung Seiten-Offset und Umrechnung
    Point aPt( pView->GetPageViewPvNum( 0 )->GetPageOrigin() );

    // Massstab
    Fraction aUIScale = pView->GetModel()->GetUIScale();

    lcl_ScaleRect( aWorkArea, aUIScale );
    lcl_ScaleRect( aRect, aUIScale );
    lcl_ScalePoint( aPt, aUIScale );

    // Metrik konvertieren
    int nDigits = aMtrPosX.GetDecimalDigits();

    aPt = lcl_ConvertPoint( aPt, nDigits, (MapUnit) ePoolUnit, eDlgUnit );
    aWorkArea = lcl_ConvertRect( aWorkArea, nDigits, (MapUnit) ePoolUnit, eDlgUnit );
    aRect = lcl_ConvertRect( aRect, nDigits, (MapUnit) ePoolUnit, eDlgUnit );


    // Beruecksichtigung Seiten-Offset
    aPt *= -1;
    aPt += aWorkArea.TopLeft();
    aWorkArea.SetPos( aPt );

    SetMinMaxPosition();
}

// -----------------------------------------------------------------------

BOOL SvxPositionTabPage::FillItemSet( SfxItemSet& rOutAttrs )
{
    BOOL bModified = FALSE;
    if( !bPageDisabled )
    {
        if ( aMtrPosX.IsValueModified() || aMtrPosY.IsValueModified() )
        {
            long lX = GetCoreValue( aMtrPosX, ePoolUnit );
            long lY = GetCoreValue( aMtrPosY, ePoolUnit );

            // Altes Rechteck mit CoreUnit
            aRect = pView->GetAllMarkedRect();
            pView->GetPageViewPvNum( 0 )->LogicToPagePos( aRect );

            GetTopLeftPosition( lX, lY, aRect );

            Fraction aUIScale = pView->GetModel()->GetUIScale();
            lX += aAnchorPos.X();
            lX = Fraction( lX ) * aUIScale;
            lY += aAnchorPos.Y();
            lY = Fraction( lY ) * aUIScale;
            rOutAttrs.Put( SfxInt32Item( GetWhich( SID_ATTR_TRANSFORM_POS_X ), (INT32) lX ) );
            rOutAttrs.Put( SfxInt32Item( GetWhich( SID_ATTR_TRANSFORM_POS_Y ), (INT32) lY ) );

            bModified |= TRUE;
        }

        if ( aTsbProtect.GetState() != aTsbProtect.GetSavedValue() )
        {
            if( aTsbProtect.GetState() == STATE_DONTKNOW )
                rOutAttrs.InvalidateItem( SID_ATTR_TRANSFORM_PROTECT_POS );
            else
                rOutAttrs.Put(
                    SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_PROTECT_POS ),
                    aTsbProtect.GetState() == STATE_CHECK ? TRUE : FALSE ) );
            bModified |= TRUE;
        }
        if(aAnchorBox.IsVisible()) //nur fuer den Writer
        {
            if(aDdLbAnchor.GetSavedValue() != aDdLbAnchor.GetSelectEntryPos())
            {
                bModified |= TRUE;
                rOutAttrs.Put(SfxUInt16Item(
                        SID_ATTR_TRANSFORM_ANCHOR, (USHORT)(ULONG)aDdLbAnchor.GetEntryData(aDdLbAnchor.GetSelectEntryPos())));
            }
            if(aDdLbOrient.GetSavedValue() != aDdLbOrient.GetSelectEntryPos())
            {
                bModified |= TRUE;
                rOutAttrs.Put(SfxUInt16Item(
                        SID_ATTR_TRANSFORM_VERT_ORIENT, aDdLbOrient.GetSelectEntryPos()));
            }
        }
    }
    return bModified;
}

// -----------------------------------------------------------------------

void SvxPositionTabPage::Reset( const SfxItemSet& rOutAttrs )
{
    if ( !bPageDisabled )
    {
        const SfxPoolItem* pItem =
            GetItem( rOutAttrs, SID_ATTR_TRANSFORM_POS_X );

        Fraction aUIScale = pView->GetModel()->GetUIScale();
        if ( pItem )
        {
            long nTmp = ( (const SfxInt32Item*)pItem )->GetValue() - aAnchorPos.X();
            nTmp = Fraction( nTmp ) / aUIScale;

            SetMetricValue( aMtrPosX, nTmp, ePoolUnit );
        }

        pItem = GetItem( rOutAttrs, SID_ATTR_TRANSFORM_POS_Y );
        if ( pItem )
        {
            long nTmp = ( (const SfxInt32Item*)pItem )->GetValue() - aAnchorPos.Y();
            nTmp = Fraction( nTmp ) / aUIScale;

            SetMetricValue( aMtrPosY, nTmp, ePoolUnit );
        }

        pItem = GetItem( rOutAttrs, SID_ATTR_TRANSFORM_PROTECT_POS );
        if ( pItem )
        {
            aTsbProtect.SetState( ( ( const SfxBoolItem* )pItem )->GetValue()
                                  ? STATE_CHECK : STATE_NOCHECK );
            aTsbProtect.EnableTriState( FALSE );
        }
        else
            aTsbProtect.SetState( STATE_DONTKNOW );

        aTsbProtect.SaveValue();
        aCtl.Reset();
        ClickProtectHdl( this );

        if(aAnchorBox.IsVisible()) //nur fuer den Writer
        {
            pItem = GetItem( rOutAttrs, SID_ATTR_TRANSFORM_ANCHOR );
            USHORT nAnchorPos = 0;
            if(pItem)
            {
                nAnchorPos = ((const SfxUInt16Item*)pItem)->GetValue();
                for (USHORT i = 0; i < aDdLbAnchor.GetEntryCount(); i++)
                {
                    if ((ULONG)aDdLbAnchor.GetEntryData(i) == (ULONG)nAnchorPos)
                    {
                        aDdLbAnchor.SelectEntryPos(i);
                        break;
                    }
                }
                aDdLbAnchor.SaveValue();
                SetAnchorHdl(&aDdLbAnchor);
            }
            if(nAnchorPos == (USHORT)SVX_FLY_IN_CNTNT)
            {
                aCtl.Disable();
                pItem = GetItem( rOutAttrs, SID_ATTR_TRANSFORM_VERT_ORIENT );
                if(pItem)
                {
                    aDdLbOrient.SelectEntryPos(((const SfxUInt16Item*)pItem)->GetValue());
                }
            }
            else
            {
                aDdLbOrient.SelectEntryPos(     (USHORT)SVX_VERT_LINE_CENTER );
            }
            aDdLbOrient.SaveValue();
            SetOrientHdl(&aDdLbOrient);
            aCtl.Invalidate();
        }
    }
}

// -----------------------------------------------------------------------

SfxTabPage* SvxPositionTabPage::Create( Window* pWindow,
                const SfxItemSet& rOutAttrs )
{
    return( new SvxPositionTabPage( pWindow, rOutAttrs ) );
}

//------------------------------------------------------------------------

USHORT* SvxPositionTabPage::GetRanges()
{
    return( pPosRanges );
}

// -----------------------------------------------------------------------

void SvxPositionTabPage::ActivatePage( const SfxItemSet& rSet )
{
    SfxRectangleItem* pRectItem = NULL;

    if( SFX_ITEM_SET == rSet.GetItemState( GetWhich( SID_ATTR_TRANSFORM_INTERN ) , FALSE,
                                    (const SfxPoolItem**) &pRectItem ) )
    {
        // Setzen der MinMax-Position
        aRect = pRectItem->GetValue();
        SetMinMaxPosition();
    }
}

// -----------------------------------------------------------------------

int SvxPositionTabPage::DeactivatePage( SfxItemSet* pSet )
{
    if( pSet )
    {
        INT32 lX = aMtrPosX.GetValue();
        INT32 lY = aMtrPosY.GetValue();

        GetTopLeftPosition( lX, lY, aRect );
        aRect.SetPos( Point( lX, lY ) );

        pSet->Put( SfxRectangleItem( SID_ATTR_TRANSFORM_INTERN, aRect ) );

        FillItemSet( *pSet );
    }
    return( LEAVE_PAGE );
}

//------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxPositionTabPage, ChangePosXHdl, void *, EMPTYARG )
{
    return( 0L );
}
IMPL_LINK_INLINE_END( SvxPositionTabPage, ChangePosXHdl, void *, EMPTYARG )

//------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxPositionTabPage, ChangePosYHdl, void *, EMPTYARG )
{
    return( 0L );
}
IMPL_LINK_INLINE_END( SvxPositionTabPage, ChangePosYHdl, void *, EMPTYARG )

//------------------------------------------------------------------------

void SvxPositionTabPage::SetMinMaxPosition()
{
    Rectangle aTmpRect = aWorkArea;

    switch ( aCtl.GetActualRP() )
    {
        case RP_LT:
            aTmpRect.Right()  -= aRect.Right() - aRect.Left();
            aTmpRect.Bottom() -= aRect.Bottom() - aRect.Top();
            break;
        case RP_MT:
            aTmpRect.Left()   += aRect.Center().X() - aRect.Left();
            aTmpRect.Right()  -= aRect.Center().X() - aRect.Left();
            aTmpRect.Bottom() -= aRect.Bottom() - aRect.Top();
            break;
        case RP_RT:
            aTmpRect.Left()   += aRect.Right() - aRect.Left();
            aTmpRect.Bottom() -= aRect.Bottom() - aRect.Top();
            break;
        case RP_LM:
            aTmpRect.Right()  -= aRect.Right() - aRect.Left();
            aTmpRect.Top()    += aRect.Center().Y() - aRect.Top();
            aTmpRect.Bottom() -= aRect.Center().Y() - aRect.Top();
            break;
        case RP_MM:
            aTmpRect.Left()   += aRect.Center().X() - aRect.Left();
            aTmpRect.Right()  -= aRect.Center().X() - aRect.Left();
            aTmpRect.Top()    += aRect.Center().Y() - aRect.Top();
            aTmpRect.Bottom() -= aRect.Center().Y() - aRect.Top();
            break;
        case RP_RM:
            aTmpRect.Left()   += aRect.Right() - aRect.Left();
            aTmpRect.Top()    += aRect.Center().Y() - aRect.Top();
            aTmpRect.Bottom() -= aRect.Center().Y() - aRect.Top();
            break;
        case RP_LB:
            aTmpRect.Right()  -= aRect.Right() - aRect.Left();
            aTmpRect.Top()    += aRect.Bottom() - aRect.Top();
            break;
        case RP_MB:
            aTmpRect.Left()   += aRect.Center().X() - aRect.Left();
            aTmpRect.Right()  -= aRect.Center().X() - aRect.Left();
            aTmpRect.Top()    += aRect.Bottom() - aRect.Top();
            break;
        case RP_RB:
            aTmpRect.Left()   += aRect.Right() - aRect.Left();
            aTmpRect.Top()    += aRect.Bottom() - aRect.Top();
            break;
    }

    long nMaxLong = MetricField::ConvertValue( LONG_MAX, 0, MAP_100TH_MM, eDlgUnit ) - 1L;

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

    aMtrPosX.SetMin( aTmpRect.Left() );
    aMtrPosX.SetFirst( aTmpRect.Left() );
    aMtrPosX.SetMax( aTmpRect.Right() );
    aMtrPosX.SetLast( aTmpRect.Right() );

    aMtrPosY.SetMin( aTmpRect.Top() );
    aMtrPosY.SetFirst( aTmpRect.Top() );
    aMtrPosY.SetMax( aTmpRect.Bottom() );
    aMtrPosY.SetLast( aTmpRect.Bottom() );
}

//------------------------------------------------------------------------

void SvxPositionTabPage::GetTopLeftPosition( INT32& rX, INT32& rY,
                                                const Rectangle& rRect )
{
    switch ( aCtl.GetActualRP() )
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

IMPL_LINK( SvxPositionTabPage, ClickProtectHdl, void *, p )
{
    /* Soll nicht mehr benutzt werden

    if( aTsbProtect.GetState() == STATE_CHECK )
    {
        aFtPosX.Disable();
        aMtrPosX.Disable();
        aFtPosY.Disable();
        aMtrPosY.Disable();
        aGrpPosition.Disable();
        aCtl.Disable();
        aCtl.Invalidate();
    }
    else
    {
        aFtPosX.Enable();
        aMtrPosX.Enable();
        aFtPosY.Enable();
        aMtrPosY.Enable();
        aGrpPosition.Enable();
        aCtl.Enable();
        aCtl.Invalidate();
    }
    */

    return( 0L );
}

//------------------------------------------------------------------------

void SvxPositionTabPage::PointChanged( Window* pWindow, RECT_POINT eRP )
{
    SetMinMaxPosition();
    switch( eRP )
    {
        case RP_LT:
            aMtrPosX.SetValue( aRect.Left() );
            aMtrPosY.SetValue( aRect.Top() );
            break;
        case RP_MT:
            aMtrPosX.SetValue( aRect.Center().X() );
            aMtrPosY.SetValue( aRect.Top() );
            break;
        case RP_RT:
            aMtrPosX.SetValue( aRect.Right() );
            aMtrPosY.SetValue( aRect.Top() );
            break;
        case RP_LM:
            aMtrPosX.SetValue( aRect.Left() );
            aMtrPosY.SetValue( aRect.Center().Y() );
            break;
        case RP_MM:
            aMtrPosX.SetValue( aRect.Center().X() );
            aMtrPosY.SetValue( aRect.Center().Y() );
            break;
        case RP_RM:
            aMtrPosX.SetValue( aRect.Right() );
            aMtrPosY.SetValue( aRect.Center().Y() );
            break;
        case RP_LB:
            aMtrPosX.SetValue( aRect.Left() );
            aMtrPosY.SetValue( aRect.Bottom() );
            break;
        case RP_MB:
            aMtrPosX.SetValue( aRect.Center().X() );
            aMtrPosY.SetValue( aRect.Bottom() );
            break;
        case RP_RB:
            aMtrPosX.SetValue( aRect.Right() );
            aMtrPosY.SetValue( aRect.Bottom() );
            break;
    }
}

//------------------------------------------------------------------------

void SvxPositionTabPage::ShowAnchorCtrls(USHORT nAnchorCtrls)
{
    aAnchorBox      .Show();
    aFtAnchor       .Show();
    aFtOrient       .Show();
    aDdLbOrient     .Show();

    for (USHORT i = 0; i < aDdLbAnchor.GetEntryCount(); i++)
        aDdLbAnchor.SetEntryData(i, (void *)(long)i);

    if (!(nAnchorCtrls & SVX_OBJ_AT_FLY))
        aDdLbAnchor.RemoveEntry(3);
    if (!(nAnchorCtrls & SVX_OBJ_PAGE))
        aDdLbAnchor.RemoveEntry(2);
    if (!(nAnchorCtrls & SVX_OBJ_IN_CNTNT))
        aDdLbAnchor.RemoveEntry(1);
    if (!(nAnchorCtrls & SVX_OBJ_AT_CNTNT))
        aDdLbAnchor.RemoveEntry(0);

    aDdLbAnchor     .Show();
};

//------------------------------------------------------------------------

IMPL_LINK( SvxPositionTabPage, SetAnchorHdl, ListBox *, pBox)
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
    aCtl.Enable(bDisable);
    aCtl.Invalidate();
    if(bDisable)
    {
        aDdLbOrient.Disable();
        aFtOrient.Disable();
        aMtrPosX.Enable();
        aMtrPosY.Enable();
    }
    else
    {
        aMtrPosX.Disable();
        aDdLbOrient.Enable();
        aFtOrient.Enable();
        SetOrientHdl(&aDdLbOrient);
    }
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxPositionTabPage, SetOrientHdl, ListBox *, pBox )
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
                    aMtrPosY.Disable();
        break;
        case SVX_VERT_NONE:
                    aMtrPosY.Enable();
        break;
    }
    return 0;
}


/*************************************************************************
|*
|*      Dialog zum Aendern der Groesse der Grafikobjekte
|*
\*************************************************************************/

SvxSizeTabPage::SvxSizeTabPage( Window* pParent, const SfxItemSet& rInAttrs  ) :
    SvxTabPage              ( pParent, ResId( RID_SVXPAGE_SIZE, DIALOG_MGR() ), rInAttrs ),
    aFtWidth                        ( this, ResId( FT_WIDTH ) ),
    aMtrWidth                       ( this, ResId( MTR_FLD_WIDTH ) ),
    aFtHeight                       ( this, ResId( FT_HEIGHT ) ),
    aMtrHeight                      ( this, ResId( MTR_FLD_HEIGHT ) ),
    aGrpSize                        ( this, ResId( GRP_SIZE ) ),
    aGrpAdjust                      ( this, ResId( GRP_ADJUST ) ),
    aCbxScale                       ( this, ResId( CBX_SCALE ) ),
    aCtl                            ( this, ResId( CTL_RECT ), RP_LT ),
    aTsbProtect             ( this, ResId( TSB_PROTECT ) ),
    aTsbAutoGrowWidth   ( this, ResId( TSB_AUTOGROW_WIDTH ) ),
    aTsbAutoGrowHeight  ( this, ResId( TSB_AUTOGROW_HEIGHT ) ),
    rOutAttrs                       ( rInAttrs )
{
    FreeResource();

    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    // PoolUnit ermitteln
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    ePoolUnit = pPool->GetMetric( SID_ATTR_TRANSFORM_POS_X );

    eRP = RP_LT; // s.o.

    aMtrWidth.SetModifyHdl( LINK( this, SvxSizeTabPage, ChangeWidthHdl ) );
    aMtrHeight.SetModifyHdl( LINK( this, SvxSizeTabPage, ChangeHeightHdl ) );

    // Wird nicht mehr gebraucht:
    // aTsbProtect.SetClickHdl( LINK( this, SvxSizeTabPage, ClickProtectHdl ) );
    // Auto-Hdl wird fuer Scale genutzt
    aCbxScale.SetClickHdl( LINK( this, SvxSizeTabPage, ClickAutoHdl ) );

    aTsbAutoGrowWidth.Disable();
    aTsbAutoGrowHeight.Disable();
    aGrpAdjust.Disable();
}

// -----------------------------------------------------------------------

void SvxSizeTabPage::Construct()
{
    // Setzen des Rechtecks und Automatisch bei Textobjekten
    DBG_ASSERT( pView, "Keine gueltige View Uebergeben!" );
    eDlgUnit = GetModuleFieldUnit( &GetItemSet() );
    SetFieldUnit( aMtrWidth, eDlgUnit, TRUE );
    SetFieldUnit( aMtrHeight, eDlgUnit, TRUE );

    if( eDlgUnit == FUNIT_MILE ||
        eDlgUnit == FUNIT_KM )
    {
        aMtrWidth.SetDecimalDigits( 3 );
        aMtrHeight.SetDecimalDigits( 3 );
    }

    aRect = pView->GetAllMarkedRect();
    pView->GetPageViewPvNum( 0 )->LogicToPagePos( aRect );

    const SdrMarkList& rMarkList = pView->GetMarkList();

    // Dies sollte ueber SID_ATTR_TRANSFORM_AUTOSIZE erfolgen
    if( rMarkList.GetMarkCount() == 1 )
    {
        const SdrObject* pObj = rMarkList.GetMark( 0 )->GetObj();
        SdrObjKind eKind = (SdrObjKind) pObj->GetObjIdentifier();
        if( ( pObj->GetObjInventor() == SdrInventor ) &&
            ( eKind==OBJ_TEXT || eKind==OBJ_TITLETEXT || eKind==OBJ_OUTLINETEXT) &&
            ( (SdrTextObj*) pObj )->HasText() )
        {
            aGrpAdjust.Enable();
            aTsbAutoGrowWidth.Enable();
            aTsbAutoGrowHeight.Enable();
            aTsbAutoGrowWidth.SetClickHdl( LINK( this, SvxSizeTabPage, ClickProtectHdl ) );
            aTsbAutoGrowHeight.SetClickHdl( LINK( this, SvxSizeTabPage, ClickProtectHdl ) );

            // Wird als Flag benutzt, um zu ermitteln, ob anwaehlbar ist
            aTsbAutoGrowWidth.EnableTriState( FALSE );
            aTsbAutoGrowHeight.EnableTriState( FALSE );
        }
    }
    // Setzen der Maximalen Groesse
    // (Wie SvxPositionTabPage (etwas weniger Code))

    // WorkArea holen und umrechnen:
    aWorkArea = pView->GetWorkArea();

    if( rMarkList.GetMarkCount() >= 1 )
    {
        const SdrObject* pObj = rMarkList.GetMark( 0 )->GetObj();
        Point aAnchorPos = pObj->GetAnchorPos();

        if( aAnchorPos != Point(0,0) ) // -> Writer
        {
            Point aPt = aAnchorPos * -1;
            Point aPt2 = aPt;

            aPt += aWorkArea.TopLeft();
            aWorkArea.SetPos( aPt );

            aPt2 += aRect.TopLeft();
            aRect.SetPos( aPt2 );
        }
    }

    // Beruecksictigung Seiten-Offset und Umrechnung
    Point aPt( pView->GetPageViewPvNum( 0 )->GetPageOrigin() );

    // Massstab
    Fraction aUIScale = pView->GetModel()->GetUIScale();

    lcl_ScaleRect( aWorkArea, aUIScale );
    lcl_ScaleRect( aRect, aUIScale );
    lcl_ScalePoint( aPt, aUIScale );

    // Umrechnung auf UI-Unit
    int nDigits = aMtrWidth.GetDecimalDigits();

    aPt = lcl_ConvertPoint( aPt, nDigits, (MapUnit) ePoolUnit, eDlgUnit );
    aWorkArea = lcl_ConvertRect( aWorkArea, nDigits, (MapUnit) ePoolUnit, eDlgUnit );
    aRect = lcl_ConvertRect( aRect, nDigits, (MapUnit) ePoolUnit, eDlgUnit );


    // Beruecksictigung Seiten-Offset
    aPt *= -1;
    aPt += aWorkArea.TopLeft();
    aWorkArea.SetPos( aPt );

    SetMaxSize( aRect );
}

// -----------------------------------------------------------------------

BOOL SvxSizeTabPage::FillItemSet( SfxItemSet& rOutAttrs )
{
    if ( aMtrWidth.HasFocus() )
        ChangeWidthHdl( this );
    if ( aMtrHeight.HasFocus() )
        ChangeHeightHdl( this );

    BOOL bModified = FALSE;

    if ( aMtrWidth.IsValueModified() || aMtrHeight.IsValueModified() )
    {
        Fraction aUIScale = pView->GetModel()->GetUIScale();

        // get Width
        double nWidth = aMtrWidth.GetValue( eDlgUnit );
        nWidth = MetricField::ConvertDoubleValue( nWidth, aMtrWidth.GetBaseValue(), aMtrWidth.GetDecimalDigits(), eDlgUnit, FUNIT_100TH_MM );
        long lWidth = nWidth * (double)aUIScale;
        lWidth = OutputDevice::LogicToLogic( lWidth, MAP_100TH_MM, (MapUnit)ePoolUnit );
        lWidth = aMtrWidth.Denormalize( lWidth );

        // get Height
        double nHeight = aMtrHeight.GetValue( eDlgUnit );
        nHeight = MetricField::ConvertDoubleValue( nHeight, aMtrHeight.GetBaseValue(), aMtrHeight.GetDecimalDigits(), eDlgUnit, FUNIT_100TH_MM );
        long lHeight = nHeight * (double)aUIScale;
        lHeight = OutputDevice::LogicToLogic( lHeight, MAP_100TH_MM, (MapUnit)ePoolUnit );
        lHeight = aMtrWidth.Denormalize( lHeight );

        // put Width & Height to itemset
        rOutAttrs.Put( SfxUInt32Item( GetWhich( SID_ATTR_TRANSFORM_WIDTH ),
                        (UINT32) lWidth ) );
        rOutAttrs.Put( SfxUInt32Item( GetWhich( SID_ATTR_TRANSFORM_HEIGHT ),
                        (UINT32) lHeight ) );
        rOutAttrs.Put(
            SfxAllEnumItem( GetWhich( SID_ATTR_TRANSFORM_SIZE_POINT ), eRP ) );
        bModified |= TRUE;
    }

    if ( aTsbProtect.GetState() != aTsbProtect.GetSavedValue() )
    {
        if ( aTsbProtect.GetState() == STATE_DONTKNOW )
            rOutAttrs.InvalidateItem( SID_ATTR_TRANSFORM_PROTECT_SIZE );
        else
            rOutAttrs.Put(
                SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_PROTECT_SIZE ),
                aTsbProtect.GetState() == STATE_CHECK ? TRUE : FALSE ) );
        bModified |= TRUE;
    }

    if ( aTsbAutoGrowWidth.GetState() != aTsbAutoGrowWidth.GetSavedValue() )
    {
        if ( !aTsbAutoGrowWidth.IsTriStateEnabled() )
        {
            if( aTsbAutoGrowWidth.GetState() == STATE_DONTKNOW )
                rOutAttrs.InvalidateItem( SID_ATTR_TRANSFORM_AUTOWIDTH );
            else
                rOutAttrs.Put(
                    SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_AUTOWIDTH ),
                    aTsbAutoGrowWidth.GetState() == STATE_CHECK ? TRUE : FALSE ) );
        }
        bModified |= TRUE;
    }

    if ( aTsbAutoGrowHeight.GetState() != aTsbAutoGrowHeight.GetSavedValue() )
    {
        if ( !aTsbAutoGrowHeight.IsTriStateEnabled() )
        {
            if( aTsbAutoGrowHeight.GetState() == STATE_DONTKNOW )
                rOutAttrs.InvalidateItem( SID_ATTR_TRANSFORM_AUTOHEIGHT );
            else
                rOutAttrs.Put(
                    SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_AUTOHEIGHT ),
                    aTsbAutoGrowHeight.GetState() == STATE_CHECK ? TRUE : FALSE ) );
        }
        bModified |= TRUE;
    }

    return bModified;
}

// -----------------------------------------------------------------------

void SvxSizeTabPage::Reset( const SfxItemSet& rOutAttrs )
{
    const SfxPoolItem* pItem = GetItem( rOutAttrs, SID_ATTR_TRANSFORM_WIDTH );
    lOldWidth = Max( pItem ? ( (const SfxUInt32Item*)pItem )->GetValue() : 0, (UINT32)1 );

    pItem = GetItem( rOutAttrs, SID_ATTR_TRANSFORM_HEIGHT );
    lOldHeight = Max( pItem ? ( (const SfxUInt32Item*)pItem )->GetValue() : 0, (UINT32)1 );

    Fraction aUIScale = pView->GetModel()->GetUIScale();

    // set Width & Height
    double nTmpWidth  = (double)OutputDevice::LogicToLogic( lOldWidth, (MapUnit)ePoolUnit, MAP_100TH_MM );
    double nTmpHeight = (double)OutputDevice::LogicToLogic( lOldHeight, (MapUnit)ePoolUnit, MAP_100TH_MM );
    nTmpWidth  = Fraction( nTmpWidth ) / aUIScale;
    nTmpHeight = Fraction( nTmpHeight ) / aUIScale;

    UINT32 nNorm = 10;
    for( int i=0; i<aMtrWidth.GetDecimalDigits()-1; i++ )
        nNorm*=10;
    nTmpWidth*=(double)nNorm;

    nNorm = 10;
    for( i=0; i<aMtrHeight.GetDecimalDigits()-1; i++ )
        nNorm*=10;
    nTmpHeight*=(double)nNorm;

    nTmpWidth =  MetricField::ConvertDoubleValue( (double)nTmpWidth, aMtrWidth.GetBaseValue(), aMtrWidth.GetDecimalDigits(), FUNIT_100TH_MM, eDlgUnit );
    nTmpHeight = MetricField::ConvertDoubleValue( (double)nTmpHeight, aMtrHeight.GetBaseValue(), aMtrHeight.GetDecimalDigits(), FUNIT_100TH_MM, eDlgUnit );

    aMtrWidth.SetValue( nTmpWidth, eDlgUnit );
    aMtrHeight.SetValue( nTmpHeight, eDlgUnit );

    pItem = GetItem( rOutAttrs, SID_ATTR_TRANSFORM_PROTECT_SIZE );

    if ( pItem )
    {
        aTsbProtect.SetState( ( (const SfxBoolItem*)pItem )->GetValue()
                              ? STATE_CHECK : STATE_NOCHECK );
        aTsbProtect.EnableTriState( FALSE );
    }
    else
        aTsbProtect.SetState( STATE_DONTKNOW );

    pItem = GetItem( rOutAttrs, SID_ATTR_TRANSFORM_AUTOWIDTH );
    if ( pItem )
    {
        aTsbAutoGrowWidth.SetState( ( ( const SfxBoolItem* )pItem )->GetValue()
                           ? STATE_CHECK : STATE_NOCHECK );
//!             aTsbAutoGrowWidth.EnableTriState( FALSE );
    }
    else
        aTsbAutoGrowWidth.SetState( STATE_DONTKNOW );

    pItem = GetItem( rOutAttrs, SID_ATTR_TRANSFORM_AUTOHEIGHT );
    if ( pItem )
    {
        aTsbAutoGrowHeight.SetState( ( ( const SfxBoolItem* )pItem )->GetValue()
                           ? STATE_CHECK : STATE_NOCHECK );
//!             aTsbAutoGrowHeight.EnableTriState( FALSE );
    }
    else
        aTsbAutoGrowHeight.SetState( STATE_DONTKNOW );

    // Ist Abgleich gesetzt?
    String aStr = GetUserData();
    aCbxScale.Check( (BOOL)aStr.ToInt32() );

    aTsbProtect.SaveValue();
    aTsbAutoGrowWidth.SaveValue();
    aTsbAutoGrowHeight.SaveValue();
    ClickProtectHdl( NULL );
}

// -----------------------------------------------------------------------

SfxTabPage* SvxSizeTabPage::Create( Window* pWindow,
                                    const SfxItemSet& rOutAttrs )
{
    return( new SvxSizeTabPage( pWindow, rOutAttrs ) );
}

//------------------------------------------------------------------------

USHORT* SvxSizeTabPage::GetRanges()
{
    return( pSizeRanges );
}

// -----------------------------------------------------------------------

void SvxSizeTabPage::ActivatePage( const SfxItemSet& rSet )
{
    SfxRectangleItem* pRectItem = NULL;

    if( SFX_ITEM_SET == rSet.GetItemState( GetWhich( SID_ATTR_TRANSFORM_INTERN ) , FALSE,
                                    (const SfxPoolItem**) &pRectItem ) )
    {
        // Setzen der MinMax-Groesse
        aRect = pRectItem->GetValue();
        SetMaxSize( aRect );
    }
}

// -----------------------------------------------------------------------

int SvxSizeTabPage::DeactivatePage( SfxItemSet* pSet )
{
    if( pSet )
    {
        pSet->Put( SfxRectangleItem( SID_ATTR_TRANSFORM_INTERN, GetRect() ) );

        FillItemSet( *pSet );
    }
    return( LEAVE_PAGE );
}

//------------------------------------------------------------------------

void SvxSizeTabPage::SetMaxSize( Rectangle aRect )
{
    Rectangle aTmpRect = aWorkArea;

    switch ( aCtl.GetActualRP() )
    {
        case RP_LT:
            aTmpRect.SetSize( Size(
                aTmpRect.GetWidth() - ( aRect.Left() - aTmpRect.Left() ),
                aTmpRect.GetHeight() - ( aRect.Top() - aTmpRect.Top() ) ) );
            break;
        case RP_MT:
            aTmpRect.SetSize( Size(
                    Min( aRect.Center().X() - aTmpRect.Left(),
                         aTmpRect.Right() - aRect.Center().X() ) * 2,
                aTmpRect.GetHeight() - ( aRect.Top() - aTmpRect.Top() ) ) );
            break;
        case RP_RT:
            aTmpRect.SetSize( Size(
                aTmpRect.GetWidth() - ( aTmpRect.Right() - aRect.Right() ),
                aTmpRect.GetHeight() - ( aRect.Top() - aTmpRect.Top() ) ) );
            break;
        case RP_LM:
            aTmpRect.SetSize( Size(
                aTmpRect.GetWidth() - ( aRect.Left() - aTmpRect.Left() ),
                    Min( aRect.Center().Y() - aTmpRect.Top(),
                          aTmpRect.Bottom() - aRect.Center().Y() ) * 2 ) );
            break;
        case RP_MM:
        {
            long n1, n2, n3, n4, n5, n6;
            n1 = aRect.Center().X() - aTmpRect.Left();
            n2 = aTmpRect.Right() - aRect.Center().X();
            n3 = Min( n1, n2 );
            n4 = aRect.Center().Y() - aTmpRect.Top();
            n5 = aTmpRect.Bottom() - aRect.Center().Y();
            n6 = Min( n4, n5 );
            aTmpRect.SetSize( Size( n3 * 2, n6 * 3 ) );
            break;
        }
        case RP_RM:
            aTmpRect.SetSize( Size(
                aTmpRect.GetWidth() - ( aTmpRect.Right() - aRect.Right() ),
                    Min( aRect.Center().Y() - aTmpRect.Top(),
                          aTmpRect.Bottom() - aRect.Center().Y() ) * 2 ) );
            break;
        case RP_LB:
            aTmpRect.SetSize( Size(
                aTmpRect.GetWidth() - ( aRect.Left() - aTmpRect.Left() ),
                aTmpRect.GetHeight() - ( aTmpRect.Bottom() - aRect.Bottom() ) ) );
            break;
        case RP_MB:
            aTmpRect.SetSize( Size(
                    Min( aRect.Center().X() - aTmpRect.Left(),
                             aTmpRect.Right() - aRect.Center().X() ) * 2,
                aTmpRect.GetHeight() - ( aRect.Bottom() - aTmpRect.Bottom() ) ) );
            break;
        case RP_RB:
            aTmpRect.SetSize( Size(
                aTmpRect.GetWidth() - ( aTmpRect.Right() - aRect.Right() ),
                aTmpRect.GetHeight() - ( aTmpRect.Bottom() - aRect.Bottom() ) ) );
            break;
    }

    aMtrWidth.SetMax( aTmpRect.GetWidth() );
    aMtrWidth.SetLast( aTmpRect.GetWidth() );

    aMtrHeight.SetMax( aTmpRect.GetHeight() );
    aMtrHeight.SetLast( aTmpRect.GetHeight() );
}

//------------------------------------------------------------------------

Rectangle SvxSizeTabPage::GetRect()
{
    Rectangle aTmpRect( aRect );
    aTmpRect.SetSize( Size( aMtrWidth.GetValue(), aMtrHeight.GetValue() ) );

    switch ( aCtl.GetActualRP() )
    {
        case RP_LT:
            // nichts!
            break;
        case RP_MT:
            aTmpRect.SetPos( Point( aRect.Left() -
                        ( aTmpRect.Right() - aRect.Right() ) / 2, aRect.Top() ) );
            break;
        case RP_RT:
            aTmpRect.SetPos( Point( aRect.Left() -
                        ( aTmpRect.Right() - aRect.Right() ), aRect.Top() ) );
            break;
        case RP_LM:
            aTmpRect.SetPos( Point( aRect.Left(), aRect.Top() -
                        ( aTmpRect.Bottom() - aRect.Bottom() ) / 2 ) );
            break;
        case RP_MM:
            aTmpRect.SetPos( Point( aRect.Left() -
                        ( aTmpRect.Right() - aRect.Right() ) / 2, aRect.Top() -
                        ( aTmpRect.Bottom() - aRect.Bottom() ) / 2 ) );
            break;
        case RP_RM:
            aTmpRect.SetPos( Point( aRect.Left() -
                        ( aTmpRect.Right() - aRect.Right() ), aRect.Top() -
                        ( aTmpRect.Bottom() - aRect.Bottom() ) / 2 ) );
            break;
        case RP_LB:
            aTmpRect.SetPos( Point( aRect.Left(), aRect.Top() -
                        ( aTmpRect.Bottom() - aRect.Bottom() ) ) );
            break;
        case RP_MB:
            aTmpRect.SetPos( Point( aRect.Left() -
                        ( aTmpRect.Right() - aRect.Right() ) / 2, aRect.Top() -
                        ( aTmpRect.Bottom() - aRect.Bottom() ) ) );
            break;
        case RP_RB:
            aTmpRect.SetPos( Point( aRect.Left() -
                        ( aTmpRect.Right() - aRect.Right() ), aRect.Top() -
                        ( aTmpRect.Bottom() - aRect.Bottom() ) ) );
            break;
    }
    return( aTmpRect );
}

//------------------------------------------------------------------------

void SvxSizeTabPage::PointChanged( Window* pWindow, RECT_POINT eRcPt )
{
    eRP = eRcPt;

    Rectangle aTmpRect( GetRect() );
    SetMaxSize( aTmpRect );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxSizeTabPage, ChangeWidthHdl, void *, EMPTYARG )
{
    if( aCbxScale.IsChecked() &&
        aCbxScale.IsEnabled() )
    {
        long nHeight = (long) ( ((double) lOldHeight * (double) aMtrWidth.GetValue()) / (double) lOldWidth );
        if( nHeight <= aMtrHeight.GetMax( FUNIT_NONE ) )
        {
            aMtrHeight.SetUserValue( nHeight, FUNIT_NONE );
        }
        else
        {
            nHeight = aMtrHeight.GetMax( FUNIT_NONE );
            aMtrHeight.SetUserValue( nHeight );
            const long nWidth = (long) ( ((double) lOldWidth * (double) nHeight) / (double) lOldHeight );
            aMtrWidth.SetUserValue( nWidth, FUNIT_NONE );
        }
    }
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxSizeTabPage, ChangeHeightHdl, void *, EMPTYARG )
{
    if( aCbxScale.IsChecked() &&
        aCbxScale.IsEnabled() )
    {
        long nWidth = (long) ( ((double) lOldWidth * (double) aMtrHeight.GetValue()) / (double) lOldHeight );
        if( nWidth <= aMtrWidth.GetMax( FUNIT_NONE ) )
        {
            aMtrWidth.SetUserValue( nWidth, FUNIT_NONE );
        }
        else
        {
            nWidth = aMtrWidth.GetMax( FUNIT_NONE );
            aMtrWidth.SetUserValue( nWidth );
            const long nHeight = (long) ( ((double) lOldHeight * (double) nWidth) / (double) lOldWidth );
            aMtrHeight.SetUserValue( nHeight, FUNIT_NONE );
        }
    }
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxSizeTabPage, ClickProtectHdl, void *, p )
{
    /*
    if( aTsbProtect.GetState() == STATE_CHECK )
    {
        aFtWidth.Disable();
        aMtrWidth.Disable();
        aFtHeight.Disable();
        aMtrHeight.Disable();
        aGrpSize.Disable();
        aCbxScale.Disable();
        aTsbAuto.Disable();
        aCtl.Disable();
        aCtl.Invalidate();
    }
    else if( !aTsbAuto.IsTriStateEnabled() && aTsbAuto.GetState() == STATE_CHECK )
    {
        aFtWidth.Enable();
        aMtrWidth.Enable();
        aFtHeight.Disable();
        aMtrHeight.Disable();
        aGrpSize.Enable();
        aCbxScale.Disable();
        aTsbAuto.Enable();
        aCtl.Enable();
        aCtl.Invalidate();
    }
    else
    {
        aFtWidth.Enable();
        aMtrWidth.Enable();
        aFtHeight.Enable();
        aMtrHeight.Enable();
        aGrpSize.Enable();
        aCbxScale.Enable();
        if( aTsbAuto.IsTriStateEnabled() )
            aTsbAuto.Disable();
        else
            aTsbAuto.Enable();
        aCtl.Enable();
        aCtl.Invalidate();
    }
    */
    BOOL bHeightChecked = !aTsbAutoGrowHeight.IsTriStateEnabled() &&
                          aTsbAutoGrowHeight.GetState() == STATE_CHECK;
    BOOL bWidthChecked = !aTsbAutoGrowWidth.IsTriStateEnabled() &&
                         aTsbAutoGrowWidth.GetState() == STATE_CHECK;
    if( p == &aTsbAutoGrowHeight || p == NULL )
    {
        if( bHeightChecked )
        {
            aFtHeight.Disable();
            aMtrHeight.Disable();
            aCbxScale.Disable();
        }
        else
        {
            aFtHeight.Enable();
            aMtrHeight.Enable();
            if( !bWidthChecked )
                aCbxScale.Enable();
        }
    }
    if( p == &aTsbAutoGrowWidth || p == NULL )
    {
        if( bWidthChecked )
        {
            aFtWidth.Disable();
            aMtrWidth.Disable();
            aCbxScale.Disable();
        }
        else
        {
            aFtWidth.Enable();
            aMtrWidth.Enable();
            if( !bHeightChecked )
                aCbxScale.Enable();
        }
    }
    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxSizeTabPage, ClickAutoHdl, void *, p )
{
    if( aCbxScale.IsChecked() )
    {
        lOldWidth  = Max( GetCoreValue( aMtrWidth,  ePoolUnit ), 1L );
        lOldHeight = Max( GetCoreValue( aMtrHeight, ePoolUnit ), 1L );
    }
    return( 0L );
}

//------------------------------------------------------------------------

void SvxSizeTabPage::FillUserData()
{
    // Abgleich wird in der Ini-Datei festgehalten
    UniString aStr = UniString::CreateFromInt32( (sal_Int32) aCbxScale.IsChecked() );
    SetUserData( aStr );
}

/*************************************************************************
|*
|*      Dialog zum Aendern der Position des Drehwinkels und des Drehwinkels
|*      der Grafikobjekte
|*
\************************************************************************/

SvxAngleTabPage::SvxAngleTabPage( Window* pParent, const SfxItemSet& rInAttrs  ) :
    SvxTabPage              ( pParent, ResId( RID_SVXPAGE_ANGLE, DIALOG_MGR() ), rInAttrs ),
    aFtPosX                 ( this, ResId( FT_POS_X ) ),
    aMtrPosX                ( this, ResId( MTR_FLD_POS_X ) ),
    aFtPosY                 ( this, ResId( FT_POS_Y ) ),
    aMtrPosY                ( this, ResId( MTR_FLD_POS_Y ) ),
    aGrpPosition    ( this, ResId( GRP_POSITION ) ),
    aFtAngle                ( this, ResId( FT_ANGLE ) ),
    aMtrAngle               ( this, ResId( MTR_FLD_ANGLE ) ),
    aGrpAngle               ( this, ResId( GRP_ANGLE ) ),
    aCtlRect                ( this, ResId( CTL_RECT ) ),
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
        aFtPosX.Disable();
        aMtrPosX.Disable();
        aFtPosY.Disable();
        aMtrPosY.Disable();
        aGrpPosition.Disable();
        aFtAngle.Disable();
        aMtrAngle.Disable();
        aGrpAngle.Disable();
        aCtlRect.Disable();
        aCtlAngle.Disable();
    }
}

// -----------------------------------------------------------------------

BOOL SvxAngleTabPage::FillItemSet( SfxItemSet& rOutAttrs )
{
    BOOL bModified = FALSE;

    if ( aMtrAngle.IsValueModified() )
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
#ifndef VCL
            case RP_LT:
                aMtrPosX.SetUserValue( aRect.Left() );
                aMtrPosY.SetUserValue( aRect.Top() );
                break;
            case RP_MT:
                aMtrPosX.SetUserValue( aRect.Center().X() );
                aMtrPosY.SetUserValue( aRect.Top() );
                break;
            case RP_RT:
                aMtrPosX.SetUserValue( aRect.Right() );
                aMtrPosY.SetUserValue( aRect.Top() );
                break;
            case RP_LM:
                aMtrPosX.SetUserValue( aRect.Left() );
                aMtrPosY.SetUserValue( aRect.Center().Y() );
                break;
            case RP_MM:
                aMtrPosX.SetUserValue( aRect.Center().X() );
                aMtrPosY.SetUserValue( aRect.Center().Y() );
                break;
            case RP_RM:
                aMtrPosX.SetUserValue( aRect.Right() );
                aMtrPosY.SetUserValue( aRect.Center().Y() );
                break;
            case RP_LB:
                aMtrPosX.SetUserValue( aRect.Left() );
                aMtrPosY.SetUserValue( aRect.Bottom() );
                break;
            case RP_MB:
                aMtrPosX.SetUserValue( aRect.Center().X() );
                aMtrPosY.SetUserValue( aRect.Bottom() );
                break;
            case RP_RB:
                aMtrPosX.SetUserValue( aRect.Right() );
                aMtrPosY.SetUserValue( aRect.Bottom() );
                break;
        }
    }
    else if( pWindow == &aCtlAngle )
    {
        switch( eRP )
        {
            case RP_LT: aMtrAngle.SetUserValue( 13500 ); break;
            case RP_MT: aMtrAngle.SetUserValue(  9000 ); break;
            case RP_RT: aMtrAngle.SetUserValue(  4500 ); break;
            case RP_LM: aMtrAngle.SetUserValue( 18000 ); break;
            case RP_RM: aMtrAngle.SetUserValue(     0 ); break;
            case RP_LB: aMtrAngle.SetUserValue( 22500 ); break;
            case RP_MB: aMtrAngle.SetUserValue( 27000 ); break;
            case RP_RB: aMtrAngle.SetUserValue( 31500 ); break;
        }
    }
#else
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
#endif
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

    aFtRadius               ( this, ResId( FT_RADIUS ) ),
    aMtrRadius              ( this, ResId( MTR_FLD_RADIUS ) ),
    aGrpRadius              ( this, ResId( GRP_RADIUS ) ),
    aFtAngle                ( this, ResId( FT_ANGLE ) ),
    aMtrAngle               ( this, ResId( MTR_FLD_ANGLE ) ),
    aGrpAngle               ( this, ResId( GRP_ANGLE ) ),
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
        Point aPt = aRect.Center();
        rAttrs.Put( SfxInt32Item( SID_ATTR_TRANSFORM_SHEAR_X, aPt.X() ) );
    rAttrs.Put( SfxInt32Item( SID_ATTR_TRANSFORM_SHEAR_Y, aPt.Y() ) );

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
    if( bMarkedObj && SFX_ITEM_DEFAULT == rAttrs.GetItemState( SDRATTR_ECKENRADIUS ) )
    {
        aFtRadius.Disable();
        aMtrRadius.Disable();
        aMtrRadius.SetText( String() );
        aGrpRadius.Disable();
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
        aFtAngle.Disable();
        aMtrAngle.Disable();
        aMtrAngle.SetText( String() );
        aGrpAngle.Disable();
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
    FillItemSet( *pSet );

    return( LEAVE_PAGE );
}

//------------------------------------------------------------------------

void SvxSlantTabPage::PointChanged( Window* pWindow, RECT_POINT eRP )
{
}



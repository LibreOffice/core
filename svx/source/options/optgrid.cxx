/*************************************************************************
 *
 *  $RCSfile: optgrid.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: os $ $Date: 2001-03-22 09:47:09 $
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

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXMODULE_HXX //autogen
#include <sfx2/module.hxx>
#endif
#pragma hdrstop

#define _SVX_OPTGRID_CXX

#include "svxids.hrc"
#include "dialmgr.hxx"
#include "optgrid.hxx"
#include "dialogs.hrc"
#include "optgrid.hrc"
#include "dlgutil.hxx"

/* -----------------18.08.98 17:41-------------------
 * local functions
 * --------------------------------------------------*/
void    lcl_GetMinMax(MetricField& rField, long& nFirst, long& nLast, long& nMin, long& nMax)
{
    nFirst  = rField.Denormalize( rField.GetFirst( FUNIT_TWIP ) );
    nLast = rField.Denormalize( rField.GetLast( FUNIT_TWIP ) );
    nMin = rField.Denormalize( rField.GetMin( FUNIT_TWIP ) );
    nMax = rField.Denormalize( rField.GetMax( FUNIT_TWIP ) );
}

void    lcl_SetMinMax(MetricField& rField, long nFirst, long nLast, long nMin, long nMax)
{
    rField.SetFirst( rField.Normalize( nFirst ), FUNIT_TWIP );
    rField.SetLast( rField.Normalize( nLast ), FUNIT_TWIP );
    rField.SetMin( rField.Normalize( nMin ), FUNIT_TWIP );
    rField.SetMax( rField.Normalize( nMax ), FUNIT_TWIP );
}

/*--------------------------------------------------------------------
    Beschreibung: Rastereinstellungen Ctor
 --------------------------------------------------------------------*/

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

/*--------------------------------------------------------------------
    Beschreibung: Rastereinstellungen Dtor
 --------------------------------------------------------------------*/

SvxOptionsGrid::~SvxOptionsGrid()
{
}

/*--------------------------------------------------------------------
    Beschreibung: Item fuer Rastereinstellungen
 --------------------------------------------------------------------*/

SvxGridItem::SvxGridItem( const SvxGridItem& rItem ):
            SfxPoolItem(rItem)
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

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SfxPoolItem*  SvxGridItem::Clone( SfxItemPool* ) const
{
    return new SvxGridItem( *this );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

int SvxGridItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unterschiedliche Typen" );

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

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SfxItemPresentation  SvxGridItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    String&             rText, const International *
)   const
{
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            return SFX_ITEM_PRESENTATION_NONE;
        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = String::CreateFromAscii("SvxGridItem");
            return ePres;
    }
    return SFX_ITEM_PRESENTATION_NONE;
}


/*----------------- OS 23.02.95  -----------------------
 TabPage Rastereinstellungen
-------------------------------------------------------*/

SvxGridTabPage::SvxGridTabPage( Window* pParent, const SfxItemSet& rCoreSet) :

    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_GRID ), rCoreSet ),

    aFtResolution   ( this, ResId( FT_RESOLUTION ) ),
    aFtDrawX        ( this, ResId( FT_DRAW_X ) ),
    aMtrFldDrawX    ( this, ResId( MTR_FLD_DRAW_X ) ),
    aFtDivision     ( this, ResId( FT_DIVISION ) ),
    aMtrFldDivisionX( this, ResId( MTR_FLD_DIVISION_X ) ),
    aFtDrawY        ( this, ResId( FT_DRAW_Y ) ),
    aMtrFldDrawY    ( this, ResId( MTR_FLD_DRAW_Y ) ),
    aMtrFldDivisionY( this, ResId( MTR_FLD_DIVISION_Y ) ),
    aGrpDrawGrid    ( this, ResId( GRP_DRAWGRID ) ),
    aCbxUseGridsnap ( this, ResId( CBX_USE_GRIDSNAP ) ),
    aCbxSynchronize ( this, ResId( CBX_SYNCHRONIZE ) ),
    aCbxGridVisible ( this, ResId( CBX_GRID_VISIBLE ) ),
    bAttrModified( FALSE )
{
    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    FreeResource();

    // Metrik einstellen
    FieldUnit eFUnit = GetModuleFieldUnit( &rCoreSet );
    long nFirst, nLast, nMin, nMax;

    lcl_GetMinMax(aMtrFldDrawX, nFirst, nLast, nMin, nMax);
    SetFieldUnit( aMtrFldDrawX, eFUnit, TRUE );
    lcl_SetMinMax(aMtrFldDrawX, nFirst, nLast, nMin, nMax);

    lcl_GetMinMax(aMtrFldDrawY, nFirst, nLast, nMin, nMax);
    SetFieldUnit( aMtrFldDrawY, eFUnit, TRUE );
    lcl_SetMinMax(aMtrFldDrawY, nFirst, nLast, nMin, nMax);


    Link aLink = LINK( this, SvxGridTabPage, ChangeGridsnapHdl_Impl );
    aCbxUseGridsnap.SetClickHdl( aLink );
    aCbxSynchronize.SetClickHdl( aLink );
    aCbxGridVisible.SetClickHdl( aLink );
    aMtrFldDrawX.SetModifyHdl(
        LINK( this, SvxGridTabPage, ChangeDrawXHdl_Impl ) );
    aMtrFldDrawY.SetModifyHdl(
        LINK( this, SvxGridTabPage, ChangeDrawYHdl_Impl ) );
    aMtrFldDivisionX.SetModifyHdl(
        LINK( this, SvxGridTabPage, ChangeDivisionXHdl_Impl ) );
    aMtrFldDivisionY.SetModifyHdl(
        LINK( this, SvxGridTabPage, ChangeDivisionYHdl_Impl ) );
}

//------------------------------------------------------------------------

SfxTabPage* SvxGridTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return ( new SvxGridTabPage( pParent, rAttrSet ) );
}

//------------------------------------------------------------------------

BOOL SvxGridTabPage::FillItemSet( SfxItemSet& rCoreSet )
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

        aGridItem.nFldDrawX    = (UINT32) nX;
        aGridItem.nFldDrawY    = (UINT32) nY;
        aGridItem.nFldDivisionX = aMtrFldDivisionX.GetValue();
        aGridItem.nFldDivisionY = aMtrFldDivisionY.GetValue();

        rCoreSet.Put( aGridItem );
    }
    return bAttrModified;
}

//------------------------------------------------------------------------

void SvxGridTabPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem* pAttr = 0;

    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_GRID_OPTIONS , FALSE,
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

//      UINT32 nFineX = pGridAttr->nFldDivisionX;
//      UINT32 nFineY = pGridAttr->nFldDivisionY;
//      aMtrFldDivisionX.SetValue( nFineX ? (pGridAttr->nFldDrawX / nFineX - 1) : 0 );
//      aMtrFldDivisionY.SetValue( nFineY ? (pGridAttr->nFldDrawY / nFineY - 1) : 0 );
        aMtrFldDivisionX.SetValue( pGridAttr->nFldDivisionX );
        aMtrFldDivisionY.SetValue( pGridAttr->nFldDivisionY );
    }

    ChangeGridsnapHdl_Impl( &aCbxUseGridsnap );
    bAttrModified = FALSE;
}

// -----------------------------------------------------------------------

void SvxGridTabPage::ActivatePage( const SfxItemSet& rSet )
{
    const SfxPoolItem* pAttr = NULL;
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_GRID_OPTIONS , FALSE,
                                    (const SfxPoolItem**)&pAttr ))
    {
        const SvxGridItem* pGridAttr = (SvxGridItem*) pAttr;
        aCbxUseGridsnap.Check( pGridAttr->bUseGridsnap == 1 );

        ChangeGridsnapHdl_Impl( &aCbxUseGridsnap );
    }

    // Metrik ggfs. aendern (da TabPage im Dialog liegt,
    // wo die Metrik eingestellt werden kann
    //USHORT nWhich = GetWhich( SID_ATTR_METRIC );
    //if( rSet.GetItemState( GetWhich( SID_ATTR_METRIC ) ) >= SFX_ITEM_AVAILABLE )
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_METRIC , FALSE,
                                    (const SfxPoolItem**)&pAttr ))
    {
        const SfxUInt16Item* pItem = (SfxUInt16Item*) pAttr;

        FieldUnit eFUnit = (FieldUnit)(long)pItem->GetValue();

        if( eFUnit != aMtrFldDrawX.GetUnit() )
        {
            // Metriken einstellen
            long nFirst, nLast, nMin, nMax;
            long nVal = aMtrFldDrawX.Denormalize( aMtrFldDrawX.GetValue( FUNIT_TWIP ) );

            lcl_GetMinMax(aMtrFldDrawX, nFirst, nLast, nMin, nMax);
            SetFieldUnit( aMtrFldDrawX, eFUnit, TRUE );
            lcl_SetMinMax(aMtrFldDrawX, nFirst, nLast, nMin, nMax);

            aMtrFldDrawX.SetValue( aMtrFldDrawX.Normalize( nVal ), FUNIT_TWIP );

            nVal = aMtrFldDrawY.Denormalize( aMtrFldDrawY.GetValue( FUNIT_TWIP ) );
            lcl_GetMinMax(aMtrFldDrawY, nFirst, nLast, nMin, nMax);
            SetFieldUnit( aMtrFldDrawY, eFUnit, TRUE );
            lcl_SetMinMax(aMtrFldDrawY, nFirst, nLast, nMin, nMax);
            aMtrFldDrawY.SetValue( aMtrFldDrawY.Normalize( nVal ), FUNIT_TWIP );

        }
    }
}

// -----------------------------------------------------------------------

int SvxGridTabPage::DeactivatePage( SfxItemSet* pSet )
{
    if ( pSet )
        FillItemSet( *pSet );
    return( LEAVE_PAGE );
}

//------------------------------------------------------------------------

IMPL_LINK( SvxGridTabPage, ChangeDrawXHdl_Impl, void *, EMPTYARG )
{
    bAttrModified = TRUE;
    if( aCbxSynchronize.IsChecked() )
    {
        aMtrFldDrawY.SetValue( aMtrFldDrawX.GetValue() );
    }
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxGridTabPage, ChangeDrawYHdl_Impl, void *, EMPTYARG )
{
    bAttrModified = TRUE;
    if( aCbxSynchronize.IsChecked() )
    {
        aMtrFldDrawX.SetValue( aMtrFldDrawY.GetValue() );
    }
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxGridTabPage, ChangeDivisionXHdl_Impl, void *, EMPTYARG )
{
    bAttrModified = TRUE;
    if( aCbxSynchronize.IsChecked() )
    {
        aMtrFldDivisionY.SetValue( aMtrFldDivisionX.GetValue() );
    }
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxGridTabPage, ChangeDivisionYHdl_Impl, void *, EMPTYARG )
{
    bAttrModified = TRUE;
    if( aCbxSynchronize.IsChecked() )
    {
        aMtrFldDivisionX.SetValue( aMtrFldDivisionY.GetValue() );
    }
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxGridTabPage, ChangeSnapXHdl_Impl, void *, EMPTYARG )
{
    bAttrModified = TRUE;
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxGridTabPage, ChangeSnapYHdl_Impl, void *, EMPTYARG )
{
    bAttrModified = TRUE;
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxGridTabPage, ChangeGridsnapHdl_Impl, void *, p )
{
    bAttrModified = TRUE;
    return 0;
}



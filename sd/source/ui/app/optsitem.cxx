/*************************************************************************
 *
 *  $RCSfile: optsitem.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ka $ $Date: 2000-09-28 17:58:43 $
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

#ifndef _SFX_SAVEOPT_HXX //autogen
#include <sfx2/saveopt.hxx>
#endif
#ifndef _SVDMODEL_HXX //autogen
#include <svx/svdmodel.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_HRC //autogen
#include <sfx2/sfx.hrc>
#endif

#include "app.hxx"
#include "optsitem.hxx"
#include "cfgids.hxx"
#include "frmview.hxx"

using namespace ::rtl;
using namespace ::utl;
using namespace ::com::sun::star::uno;

#define B2U(_def_aStr) (OUString::createFromAscii(_def_aStr))


// -----------------
// - SdOptionsItem -
// -----------------

SdOptionsItem::SdOptionsItem( const SdOptionsGeneric& rParent, const OUString rSubTree ) :
    ConfigItem  ( rSubTree ),
    mrParent    ( rParent )
{
}

// -----------------------------------------------------------------------------

SdOptionsItem::~SdOptionsItem()
{
}

// -----------------------------------------------------------------------------

void SdOptionsItem::Commit()
{
    if( IsModified() )
        mrParent.Commit( *this );
};

// -----------------------------------------------------------------------------

Sequence< Any > SdOptionsItem::GetProperties( const Sequence< OUString >& rNames )
{
    return ConfigItem::GetProperties( rNames );
}

// -----------------------------------------------------------------------------

sal_Bool SdOptionsItem::PutProperties( const Sequence< OUString >& rNames, const Sequence< Any>& rValues )
{
    return ConfigItem::PutProperties( rNames, rValues );
}

// -----------------------------------------------------------------------------

void SdOptionsItem::SetModified()
{
    ConfigItem::SetModified();
}

// --------------------
// - SdOptionsGeneric -
// --------------------

SdOptionsGeneric::SdOptionsGeneric( USHORT nConfigId, const OUString& rSubTree ) :
    maSubTree   ( rSubTree ),
    mpCfgItem   ( NULL ),
    mnConfigId  ( nConfigId ),
    mbInit      ( rSubTree.getLength() == 0 )
{
}

// -----------------------------------------------------------------------------

void SdOptionsGeneric::Init() const
{
    if( !mbInit )
    {
        SdOptionsGeneric* pThis = (SdOptionsGeneric*) this;

        if( !mpCfgItem )
            pThis->mpCfgItem = new SdOptionsItem( *this, maSubTree );

        const Sequence< OUString >  aNames( GetPropertyNames() );
        const Sequence< Any >       aValues = mpCfgItem->GetProperties( aNames );

        if( aNames.getLength() && ( aValues.getLength() == aNames.getLength() ) )
        {
            const Any* pValues = aValues.getConstArray();

            pThis->EnableModify( FALSE );
            pThis->mbInit = pThis->ReadData( pValues );
            pThis->EnableModify( TRUE );
        }
        else
            pThis->mbInit = TRUE;
    }
}

// -----------------------------------------------------------------------------

SdOptionsGeneric::~SdOptionsGeneric()
{
    if( mpCfgItem )
        delete mpCfgItem;
}

// -----------------------------------------------------------------------------

void SdOptionsGeneric::Commit( SdOptionsItem& rCfgItem ) const
{
    const Sequence< OUString >  aNames( GetPropertyNames() );
    Sequence< Any >             aValues( aNames.getLength() );

    if( aNames.getLength() && ( aValues.getLength() == aNames.getLength() ) )
    {
        if( ( (SdOptionsGeneric*) this )->WriteData( aValues.getArray() ) )
            rCfgItem.PutProperties( aNames, aValues );
        else
        {
            DBG_ERROR( "PutProperties failed" );
        }
    }
}

// -----------------------------------------------------------------------------

Sequence< OUString > SdOptionsGeneric::GetPropertyNames() const
{
    ULONG           nCount;
    const char**    ppPropNames;

    GetPropNameArray( ppPropNames, nCount );

    Sequence< OUString > aNames( nCount );
    OUString*            pNames = aNames.getArray();

    for( long i = 0; i < nCount; i++ )
        pNames[ i ] = OUString::createFromAscii( ppPropNames[ i ] );

    return aNames;
}


/*************************************************************************
|*
|* SdOptionsLayout
|*
\************************************************************************/

SdOptionsLayout::SdOptionsLayout(  USHORT nConfigId, BOOL bUseConfig ) :
    SdOptionsGeneric( nConfigId, bUseConfig ?
                      ( ( SDCFG_DRAW == nConfigId ) ?
                        B2U( "Office.Draw/Layout" ) :
                        B2U( "Office.Impress/Layout" ) ) :
                      OUString() )
{
    EnableModify( FALSE );
    SetDefaults();
    EnableModify( TRUE );
}

// -----------------------------------------------------------------------------

void SdOptionsLayout::SetDefaults()
{
    SetRulerVisible( TRUE );
    SetHelplines( TRUE );
    SetHandlesBezier( FALSE );
    SetMoveOutline( TRUE );
    SetDragStripes( FALSE );
    SetMetric( 0xffff );
    SetDefTab( 1250 );
}

// -----------------------------------------------------------------------------

BOOL SdOptionsLayout::operator==( const SdOptionsLayout& rOpt ) const
{
    return( IsRulerVisible() == rOpt.IsRulerVisible() &&
            IsMoveOutline() == rOpt.IsMoveOutline() &&
            IsDragStripes() == rOpt.IsDragStripes() &&
            IsHandlesBezier() == rOpt.IsHandlesBezier() &&
            IsHelplines() == rOpt.IsHelplines() &&
            GetMetric() == rOpt.GetMetric() &&
            GetDefTab() == rOpt.GetDefTab() );
}

// -----------------------------------------------------------------------------

void SdOptionsLayout::GetPropNameArray( const char**& ppNames, ULONG& rCount ) const
{
    static const char* aPropNames[] =
    {
        "Display/Ruler",
        // ??? MoveOutline
        // ??? DragStripes
        "Display/Bezier",
        // ??? Helplines
        "Other/MeasureUnit",
        "Other/TabStop"
    };

    // rCount = 7;
    rCount = 4;
    ppNames = aPropNames;
}

// -----------------------------------------------------------------------------

BOOL SdOptionsLayout::ReadData( const Any* pValues )
{
    SetRulerVisible( *(sal_Bool*) pValues[ 0 ].getValue() );
    SetHandlesBezier( *(sal_Bool*)pValues[ 1 ].getValue() );
    SetMetric( *(sal_Int32*) pValues[ 2 ].getValue() );
    SetDefTab( *(sal_Int32*) pValues[ 3 ].getValue() );

    return TRUE;
}

// -----------------------------------------------------------------------------

BOOL SdOptionsLayout::WriteData( Any* pValues ) const
{
    pValues[ 0 ] <<= IsRulerVisible();
    pValues[ 1 ] <<= IsHandlesBezier();
    pValues[ 2 ] <<= (sal_Int32) GetMetric();
    pValues[ 3 ] <<= (sal_Int32) GetDefTab();

    return TRUE;
}

/*************************************************************************
|*
|* SdOptionsLayoutItem
|*
\************************************************************************/

SdOptionsLayoutItem::SdOptionsLayoutItem( USHORT nWhich ) :
    SfxPoolItem     ( nWhich ),
    SdOptionsLayout ( 0, FALSE )
{
}

// ----------------------------------------------------------------------

SdOptionsLayoutItem::SdOptionsLayoutItem( USHORT nWhich, SdOptions* pOpts, FrameView* pView ) :
    SfxPoolItem     ( nWhich ),
    SdOptionsLayout ( 0, FALSE )
{
    SetMetric( pOpts->GetMetric() );
    SetDefTab( pOpts->GetDefTab() );

    if( pView )
    {
        SetRulerVisible( pView->HasRuler() );
        SetMoveOutline( !pView->IsNoDragXorPolys() );
        SetDragStripes( pView->IsDragStripes() );
        SetHandlesBezier( pView->IsPlusHandlesAlwaysVisible() );
        SetHelplines( pView->IsHlplVisible() );
    }
    else
    {
        SetRulerVisible( pOpts->IsRulerVisible() );
        SetMoveOutline( pOpts->IsMoveOutline() );
        SetDragStripes( pOpts->IsDragStripes() );
        SetHandlesBezier( pOpts->IsHandlesBezier() );
        SetHelplines( pOpts->IsHelplines() );
    }
}

// ----------------------------------------------------------------------

SfxPoolItem* SdOptionsLayoutItem::Clone( SfxItemPool* ) const
{
    return new SdOptionsLayoutItem( *this );
}


// ----------------------------------------------------------------------

int SdOptionsLayoutItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==( rAttr ), "unterschiedliche Typen" );
    return( (SdOptionsLayout&) *this == (const SdOptionsLayout&)(const SdOptionsLayoutItem&) rAttr );
}

// -----------------------------------------------------------------------

void SdOptionsLayoutItem::SetOptions( SdOptions* pOpts ) const
{
    pOpts->SetRulerVisible( IsRulerVisible() );
    pOpts->SetMoveOutline( IsMoveOutline() );
    pOpts->SetDragStripes( IsDragStripes() );
    pOpts->SetHandlesBezier( IsHandlesBezier() );
    pOpts->SetHelplines( IsHelplines() );
    pOpts->SetMetric( GetMetric() );
    pOpts->SetDefTab( GetDefTab() );
}

/*************************************************************************
|*
|* SdOptionsContents
|*
\************************************************************************/

SdOptionsContents::SdOptionsContents( USHORT nConfigId, BOOL bUseConfig ) :
    SdOptionsGeneric( nConfigId, bUseConfig ?
                      ( ( SDCFG_DRAW == nConfigId ) ?
                        B2U( "Office.Draw/Content" ) :
                        B2U( "Office.Impress/Content" ) ) :
                      OUString() )
{
    EnableModify( FALSE );
    SetDefaults();
    EnableModify( TRUE );
}

// -----------------------------------------------------------------------------

void SdOptionsContents::SetDefaults()
{
    SetExternGraphic( FALSE );
    SetOutlineMode( FALSE );
    SetHairlineMode( FALSE);
    SetNoText( FALSE );
    SetSolidDragging( FALSE );
    SetSolidMarkHdl( TRUE );
}

// -----------------------------------------------------------------------------

BOOL SdOptionsContents::operator==( const SdOptionsContents& rOpt ) const
{
    return( IsExternGraphic() == rOpt.IsExternGraphic() &&
            IsOutlineMode() == rOpt.IsOutlineMode() &&
            IsHairlineMode() == rOpt.IsHairlineMode() &&
            IsNoText() == rOpt.IsNoText() &&
            IsSolidDragging() == rOpt.IsSolidDragging() &&
            IsSolidMarkHdl() == rOpt.IsSolidMarkHdl() );
}

// -----------------------------------------------------------------------------

void SdOptionsContents::GetPropNameArray( const char**& ppNames, ULONG& rCount ) const
{
    static const char* aPropNames[] =
    {
        "Display/PicturePlaceholder",
        "Display/ContourMode",
        "Display/LineContour",
        "Display/TextPlaceholder"
        // "bSolidDragging",
        // "bSolidMarkHdl"
    };

    // rCount = 6;
    rCount = 4;
    ppNames = aPropNames;
}

// -----------------------------------------------------------------------------

BOOL SdOptionsContents::ReadData( const Any* pValues )
{
    SetExternGraphic( *(sal_Bool*) pValues[ 0 ].getValue() );
    SetOutlineMode( *(sal_Bool*)pValues[ 1 ].getValue() );
    SetHairlineMode( *(sal_Bool*) pValues[ 2 ].getValue() );
    SetNoText( *(sal_Bool*) pValues[ 3 ].getValue() );

    return TRUE;
}

// -----------------------------------------------------------------------------

BOOL SdOptionsContents::WriteData( Any* pValues ) const
{
    pValues[ 0 ] <<= IsExternGraphic();
    pValues[ 1 ] <<= IsOutlineMode();
    pValues[ 2 ] <<= IsHairlineMode();
    pValues[ 3 ] <<= IsNoText();

    return TRUE;
}

/*************************************************************************
|*
|* SdOptionsContentsItem
|*
\************************************************************************/

SdOptionsContentsItem::SdOptionsContentsItem( USHORT nWhich ) :
    SfxPoolItem         ( nWhich ),
    SdOptionsContents   ( 0, FALSE )
{
}

// ----------------------------------------------------------------------

SdOptionsContentsItem::SdOptionsContentsItem( USHORT nWhich, SdOptions* pOpts, FrameView* pView ) :
    SfxPoolItem         ( nWhich ),
    SdOptionsContents   ( 0, FALSE )
{
    if( pView )
    {
        SetExternGraphic( pView->IsGrafDraft() );
        SetOutlineMode( pView->IsFillDraft() );
        SetHairlineMode( pView->IsLineDraft() );
        SetNoText( pView->IsTextDraft() );
        SetSolidDragging( pView->IsSolidDragging() );
        SetSolidMarkHdl( pView->IsSolidMarkHdl() );
    }
    else
    {
        SetExternGraphic( pOpts->IsExternGraphic() );
        SetOutlineMode( pOpts->IsOutlineMode() );
        SetHairlineMode( pOpts->IsHairlineMode() );
        SetNoText( pOpts->IsNoText() );
        SetSolidDragging( pOpts->IsSolidDragging() );
        SetSolidMarkHdl( pOpts->IsSolidMarkHdl() );
    }
}

// ----------------------------------------------------------------------

SfxPoolItem* SdOptionsContentsItem::Clone( SfxItemPool* ) const
{
    return new SdOptionsContentsItem( *this );
}

// ----------------------------------------------------------------------

int SdOptionsContentsItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unterschiedliche Typen" );
    return( (SdOptionsContents&) *this == (const SdOptionsContents&)(const SdOptionsContentsItem&) rAttr );
}

// -----------------------------------------------------------------------

void SdOptionsContentsItem::SetOptions( SdOptions* pOpts ) const
{
    pOpts->SetExternGraphic( IsExternGraphic() );
    pOpts->SetOutlineMode( IsOutlineMode() );
    pOpts->SetHairlineMode(IsHairlineMode() );
    pOpts->SetNoText( IsNoText() );
    pOpts->SetSolidDragging( IsSolidDragging() );
    pOpts->SetSolidMarkHdl( IsSolidMarkHdl() );
}

/*************************************************************************
|*
|* SdOptionsMisc
|*
\************************************************************************/

SdOptionsMisc::SdOptionsMisc( USHORT nConfigId, BOOL bUseConfig ) :
    SdOptionsGeneric( nConfigId, bUseConfig ?
                      ( ( SDCFG_DRAW == nConfigId ) ?
                        B2U( "Office.Draw/Misc" ) :
                        B2U( "Office.Impress/Misc" ) ) :
                      OUString() )
{
    EnableModify( FALSE );
    SetDefaults();
    EnableModify( TRUE );
}

// -----------------------------------------------------------------------------

void SdOptionsMisc::SetDefaults()
{
    SetStartWithTemplate( TRUE );
    SetMarkedHitMovesAlways( TRUE );
    SetMoveOnlyDragging( FALSE );
    SetCrookNoContortion( FALSE );
    SetQuickEdit( GetConfigId() != SDCFG_DRAW );
    SetMasterPagePaintCaching( TRUE );
    SetDragWithCopy( FALSE );
    SetPickThrough( TRUE );
    SetBigHandles( FALSE );
    SetDoubleClickTextEdit( TRUE );
    SetClickChangeRotation( FALSE );
    SetStartWithActualPage( FALSE );
    SetPreviewQuality( DRAWMODE_DEFAULT );
}

// -----------------------------------------------------------------------------

BOOL SdOptionsMisc::operator==( const SdOptionsMisc& rOpt ) const
{
    return( IsStartWithTemplate() == rOpt.IsStartWithTemplate() &&
            IsMarkedHitMovesAlways() == rOpt.IsMarkedHitMovesAlways() &&
            IsMoveOnlyDragging() == rOpt.IsMoveOnlyDragging() &&
            IsCrookNoContortion() == rOpt.IsCrookNoContortion() &&
            IsQuickEdit() == rOpt.IsQuickEdit() &&
            IsMasterPagePaintCaching() == rOpt.IsMasterPagePaintCaching() &&
            IsDragWithCopy() == rOpt.IsDragWithCopy() &&
            IsPickThrough() == rOpt.IsPickThrough() &&
            IsBigHandles() == rOpt.IsBigHandles() &&
            IsDoubleClickTextEdit() == rOpt.IsDoubleClickTextEdit() &&
            IsClickChangeRotation() == rOpt.IsClickChangeRotation() &&
            IsStartWithActualPage() == rOpt.IsStartWithActualPage() &&
            GetPreviewQuality() == rOpt.GetPreviewQuality() );
}

// -----------------------------------------------------------------------------

void SdOptionsMisc::GetPropNameArray( const char**& ppNames, ULONG& rCount ) const
{
    static const char* aPropNames[] =
    {
        // bMarkedHitMovesAlways,
        // bMoveOnlyDragging,
        "NoDistort",
        "TextObject/QuickEditing",
        "BackgroundCache",
        "CopyWhileMoving",
        // bPickThrough
        "BigHandles",
        "DclickTextedit",
        "RotateClick",
        "Preview",

        // just for impress
        "NewDoc/AutoPilot",
        "Start/CurrentPage"
    };

    rCount = ( ( GetConfigId() == SDCFG_IMPRESS ) ? 10 : 8 );
    ppNames = aPropNames;
}

// -----------------------------------------------------------------------------

BOOL SdOptionsMisc::ReadData( const Any* pValues )
{
    SetCrookNoContortion( *(sal_Bool*) pValues[ 0 ].getValue() );
    SetQuickEdit( *(sal_Bool*)pValues[ 1 ].getValue() );
    SetMasterPagePaintCaching( *(sal_Bool*) pValues[ 2 ].getValue() );
    SetDragWithCopy( *(sal_Bool*) pValues[ 3 ].getValue() );
    SetBigHandles( *(sal_Bool*) pValues[ 4 ].getValue() );
    SetDoubleClickTextEdit( *(sal_Bool*) pValues[ 5 ].getValue() );
    SetClickChangeRotation( *(sal_Bool*) pValues[ 6 ].getValue() );
    SetPreviewQuality( *(sal_Int32*) pValues[ 7 ].getValue() );

    // just for Impress
    if( GetConfigId() == SDCFG_IMPRESS )
    {
        SetStartWithTemplate( *(sal_Bool*) pValues[ 8 ].getValue() );
        SetStartWithActualPage( *(sal_Bool*) pValues[ 9 ].getValue() );
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

BOOL SdOptionsMisc::WriteData( Any* pValues ) const
{
    pValues[ 0 ] <<= IsCrookNoContortion();
    pValues[ 1 ] <<= IsQuickEdit();
    pValues[ 2 ] <<= IsMasterPagePaintCaching();
    pValues[ 3 ] <<= IsDragWithCopy();
    pValues[ 4 ] <<= IsBigHandles();
    pValues[ 5 ] <<= IsDoubleClickTextEdit();
    pValues[ 6 ] <<= IsClickChangeRotation();
    pValues[ 7 ] <<= (sal_Int32) GetPreviewQuality();

    // just for Impress
    if( GetConfigId() == SDCFG_IMPRESS )
    {
        pValues[ 8 ] <<= IsStartWithTemplate();
        pValues[ 9 ] <<= IsStartWithActualPage();
    }

    return TRUE;
}

/*************************************************************************
|*
|* SdOptionsMiscItem
|*
\************************************************************************/

SdOptionsMiscItem::SdOptionsMiscItem( USHORT nWhich ) :
    SfxPoolItem     ( nWhich ),
    SdOptionsMisc   ( 0, FALSE )
{
}

// ----------------------------------------------------------------------

SdOptionsMiscItem::SdOptionsMiscItem( USHORT nWhich, SdOptions* pOpts, FrameView* pView ) :
    SfxPoolItem     ( nWhich ),
    SdOptionsMisc   ( 0, FALSE )
{
    SetStartWithTemplate( pOpts->IsStartWithTemplate() );
    SetStartWithActualPage( pOpts->IsStartWithActualPage() );

    if( pView )
    {
        SetMarkedHitMovesAlways( pView->IsMarkedHitMovesAlways() );
        SetMoveOnlyDragging( pView->IsMoveOnlyDragging() );
        SetCrookNoContortion( pView->IsCrookNoContortion() );
        SetQuickEdit( pView->IsQuickEdit() );
        SetMasterPagePaintCaching(pView->IsMasterPagePaintCaching() );
        SetDragWithCopy( pView->IsDragWithCopy() );
        SetPickThrough( pView->GetModel()->IsPickThroughTransparentTextFrames() );
        SetBigHandles( pView->IsBigHandles() );
        SetDoubleClickTextEdit( pView->IsDoubleClickTextEdit() );
        SetClickChangeRotation( pView->IsClickChangeRotation() );
        SetPreviewQuality( pView->GetPreviewDrawMode() );

    }
    else
    {
        SetMarkedHitMovesAlways( pOpts->IsMarkedHitMovesAlways() );
        SetMoveOnlyDragging( pOpts->IsMoveOnlyDragging() );
        SetCrookNoContortion( pOpts->IsCrookNoContortion() );
        SetQuickEdit( pOpts->IsQuickEdit() );
        SetMasterPagePaintCaching( pOpts->IsMasterPagePaintCaching() );
        SetDragWithCopy( pOpts->IsDragWithCopy() );
        SetPickThrough( pOpts->IsPickThrough() );
        SetBigHandles( pOpts->IsBigHandles() );
        SetDoubleClickTextEdit( pOpts->IsDoubleClickTextEdit() );
        SetClickChangeRotation( pOpts->IsClickChangeRotation() );
        SetPreviewQuality( pOpts->GetPreviewQuality() );
    }
}

// ----------------------------------------------------------------------

SfxPoolItem* SdOptionsMiscItem::Clone( SfxItemPool* ) const
{
    return new SdOptionsMiscItem( *this );
}


// ----------------------------------------------------------------------

int SdOptionsMiscItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unterschiedliche Typen" );
    return( (SdOptionsMisc&) *this == (const SdOptionsMisc&)(const SdOptionsMiscItem&) rAttr );
}

// -----------------------------------------------------------------------

void SdOptionsMiscItem::SetOptions( SdOptions* pOpts ) const
{
    pOpts->SetStartWithTemplate( IsStartWithTemplate() );
    pOpts->SetMarkedHitMovesAlways( IsMarkedHitMovesAlways() );
    pOpts->SetMoveOnlyDragging( IsMoveOnlyDragging() );
    pOpts->SetCrookNoContortion( IsCrookNoContortion() );
    pOpts->SetQuickEdit( IsQuickEdit() );
    pOpts->SetMasterPagePaintCaching( IsMasterPagePaintCaching() );
    pOpts->SetDragWithCopy( IsDragWithCopy() );
    pOpts->SetPickThrough( IsPickThrough() );
    pOpts->SetBigHandles( IsBigHandles() );
    pOpts->SetDoubleClickTextEdit( IsDoubleClickTextEdit() );
    pOpts->SetClickChangeRotation( IsClickChangeRotation() );
    pOpts->SetStartWithActualPage( IsStartWithActualPage() );
    pOpts->SetPreviewQuality( GetPreviewQuality() );
}

/*************************************************************************
|*
|* SdOptionsSnap
|*
\************************************************************************/

SdOptionsSnap::SdOptionsSnap( USHORT nConfigId, BOOL bUseConfig ) :
    SdOptionsGeneric( nConfigId, bUseConfig ?
                      ( ( SDCFG_DRAW == nConfigId ) ?
                        B2U( "Office.Draw/Snap" ) :
                        B2U( "Office.Impress/Snap" ) ) :
                      OUString() )
{
    EnableModify( FALSE );
    SetDefaults();
    EnableModify( TRUE );
}

// -----------------------------------------------------------------------------

void SdOptionsSnap::SetDefaults()
{
    SetSnapHelplines( TRUE );
    SetSnapBorder( TRUE );
    SetSnapFrame( FALSE );
    SetSnapPoints( FALSE );
    SetOrtho( FALSE );
    SetBigOrtho( TRUE );
    SetRotate( FALSE );
    SetSnapArea( 5 );
    SetAngle( 1500 );
    SetEliminatePolyPointLimitAngle( 1500 );
}

// -----------------------------------------------------------------------------

BOOL SdOptionsSnap::operator==( const SdOptionsSnap& rOpt ) const
{
    return( IsSnapHelplines() == rOpt.IsSnapHelplines() &&
            IsSnapBorder() == rOpt.IsSnapBorder() &&
            IsSnapFrame() == rOpt.IsSnapFrame() &&
            IsSnapPoints() == rOpt.IsSnapPoints() &&
            IsOrtho() == rOpt.IsOrtho() &&
            IsBigOrtho() == rOpt.IsBigOrtho() &&
            IsRotate() == rOpt.IsRotate() &&
            GetSnapArea() == rOpt.GetSnapArea() &&
            GetAngle() == rOpt.GetAngle() &&
            GetEliminatePolyPointLimitAngle() == rOpt.GetEliminatePolyPointLimitAngle() );
}

// -----------------------------------------------------------------------------

void SdOptionsSnap::GetPropNameArray( const char**& ppNames, ULONG& rCount ) const
{
    static const char* aPropNames[] =
    {
        "Object/SnapLine",
        "Object/PageMargin",
        "Object/ObjectFrame",
        "Object/ObjectPoint",
        "Position/CreatingMoving",
        "Position/ExtendEdges",
        "Position/Rotating",
        "Object/Range",
        "Position/RotatingValue",
        "Position/PointReduction"
    };

    rCount = 10;
    ppNames = aPropNames;
}

// -----------------------------------------------------------------------------

BOOL SdOptionsSnap::ReadData( const Any* pValues )
{
    SetSnapHelplines( *(sal_Bool*) pValues[ 0 ].getValue() );
    SetSnapBorder( *(sal_Bool*)pValues[ 1 ].getValue() );
    SetSnapFrame( *(sal_Bool*) pValues[ 2 ].getValue() );
    SetSnapPoints( *(sal_Bool*) pValues[ 3 ].getValue() );
    SetOrtho( *(sal_Bool*) pValues[ 4 ].getValue() );
    SetBigOrtho( *(sal_Bool*) pValues[ 5 ].getValue() );
    SetRotate( *(sal_Bool*) pValues[ 6 ].getValue() );
    SetSnapArea( *(sal_Int32*) pValues[ 7 ].getValue() );
    SetAngle( *(sal_Int32*) pValues[ 8 ].getValue() );
    SetEliminatePolyPointLimitAngle( *(sal_Int32*) pValues[ 9 ].getValue() );

    return TRUE;
}

// -----------------------------------------------------------------------------

BOOL SdOptionsSnap::WriteData( Any* pValues ) const
{
    pValues[ 0 ] <<= IsSnapHelplines();
    pValues[ 1 ] <<= IsSnapBorder();
    pValues[ 2 ] <<= IsSnapFrame();
    pValues[ 3 ] <<= IsSnapPoints();
    pValues[ 4 ] <<= IsOrtho();
    pValues[ 5 ] <<= IsBigOrtho();
    pValues[ 6 ] <<= IsRotate();
    pValues[ 7 ] <<= (sal_Int32) GetSnapArea();
    pValues[ 8 ] <<= (sal_Int32) GetAngle();
    pValues[ 9 ] <<= (sal_Int32) GetEliminatePolyPointLimitAngle();

    return TRUE;
}

/*************************************************************************
|*
|* SdOptionsSnapItem
|*
\************************************************************************/

SdOptionsSnapItem::SdOptionsSnapItem( USHORT nWhich ) :
    SfxPoolItem     ( nWhich ),
    SdOptionsSnap   ( 0, FALSE )
{
}

// ----------------------------------------------------------------------

SdOptionsSnapItem::SdOptionsSnapItem( USHORT nWhich, SdOptions* pOpts, FrameView* pView ) :
    SfxPoolItem     ( nWhich ),
    SdOptionsSnap   ( 0, FALSE )
{
    if( pView )
    {
        SetSnapHelplines( pView->IsHlplSnap() );
        SetSnapBorder( pView->IsBordSnap() );
        SetSnapFrame( pView->IsOFrmSnap() );
        SetSnapPoints( pView->IsOPntSnap() );
        SetOrtho( pView->IsOrtho() );
        SetBigOrtho( pView->IsBigOrtho() );
        SetRotate( pView->IsAngleSnapEnabled() );
        SetSnapArea( pView->GetSnapMagneticPixel() );
        SetAngle( (INT16) pView->GetSnapAngle() );
        SetEliminatePolyPointLimitAngle( (INT16) pView->GetEliminatePolyPointLimitAngle() );
    }
    else
    {
        SetSnapHelplines( pOpts->IsSnapHelplines() );
        SetSnapBorder( pOpts->IsSnapBorder() );
        SetSnapFrame( pOpts->IsSnapFrame() );
        SetSnapPoints( pOpts->IsSnapPoints() );
        SetOrtho( pOpts->IsOrtho() );
        SetBigOrtho( pOpts->IsBigOrtho() );
        SetRotate( pOpts->IsRotate() );
        SetSnapArea( pOpts->GetSnapArea() );
        SetAngle( pOpts->GetAngle() );
        SetEliminatePolyPointLimitAngle( pOpts->GetEliminatePolyPointLimitAngle() );
    }
}

// ----------------------------------------------------------------------

SfxPoolItem* SdOptionsSnapItem::Clone( SfxItemPool* ) const
{
    return new SdOptionsSnapItem( *this );
}


// ----------------------------------------------------------------------

int SdOptionsSnapItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unterschiedliche Typen" );
    return( (SdOptionsSnap&) *this == (const SdOptionsSnap&)(const SdOptionsSnapItem&) rAttr );
}

// -----------------------------------------------------------------------

void SdOptionsSnapItem::SetOptions( SdOptions* pOpts ) const
{
    pOpts->SetSnapHelplines( IsSnapHelplines() );
    pOpts->SetSnapBorder( IsSnapBorder() );
    pOpts->SetSnapFrame( IsSnapFrame() );
    pOpts->SetSnapPoints( IsSnapPoints() );
    pOpts->SetOrtho( IsOrtho() );
    pOpts->SetBigOrtho( IsBigOrtho() );
    pOpts->SetRotate( IsRotate() );
    pOpts->SetSnapArea( GetSnapArea() );
    pOpts->SetAngle( GetAngle() );
    pOpts->SetEliminatePolyPointLimitAngle( GetEliminatePolyPointLimitAngle() );
}

/*************************************************************************
|*
|* SdOptionsZoom
|*
\************************************************************************/

SdOptionsZoom::SdOptionsZoom( USHORT nConfigId, BOOL bUseConfig ) :
    SdOptionsGeneric( nConfigId, bUseConfig ?
                      ( ( SDCFG_DRAW == nConfigId ) ?
                        B2U( "Office.Draw/Zoom" ) :
                        B2U( "Office.Impress/Zoom" ) ) :
                      OUString() )
{
    EnableModify( FALSE );
    SetDefaults();
    EnableModify( TRUE );
}

// -----------------------------------------------------------------------------

void SdOptionsZoom::SetDefaults()
{
    SetScale( 1, 1 );
}

// -----------------------------------------------------------------------------

BOOL SdOptionsZoom::operator==( const SdOptionsZoom& rOpt ) const
{
    INT32 nX1, nX2, nY1, nY2;

    GetScale( nX1, nY1 );
    rOpt.GetScale( nX2, nY2 );

    return( ( nX1 == nX2 ) &&
            ( nY1 == nY2 ) );
}

// -----------------------------------------------------------------------------

void SdOptionsZoom::GetPropNameArray( const char**& ppNames, ULONG& rCount ) const
{
    static const char* aPropNames[] =
    {
        "ScaleX",
        "ScaleY"
    };

    rCount = ( GetConfigId() == SDCFG_DRAW ) ? 2 : 0;
    ppNames = aPropNames;
}

// -----------------------------------------------------------------------------

BOOL SdOptionsZoom::ReadData( const Any* pValues )
{
    INT32 nX, nY;

    nX = ( *(sal_Int32*) pValues[ 0 ].getValue() );
    nY = ( *(sal_Int32*) pValues[ 1 ].getValue() );

    SetScale( nX, nY );

    return TRUE;
}

// -----------------------------------------------------------------------------

BOOL SdOptionsZoom::WriteData( Any* pValues ) const
{
    INT32 nX, nY;

    GetScale( nX, nY );

    pValues[ 0 ] <<= (sal_Int32) nX;
    pValues[ 1 ] <<= (sal_Int32) nY;

    return TRUE;
}

/*************************************************************************
|*
|* SdOptionsZoomItem
|*
\************************************************************************/

SdOptionsZoomItem::SdOptionsZoomItem( USHORT nWhich ) :
    SfxPoolItem     ( nWhich ),
    SdOptionsZoom   ( 0, FALSE )
{
}

// ----------------------------------------------------------------------

SdOptionsZoomItem::SdOptionsZoomItem( USHORT nWhich, SdOptions* pOpts, FrameView* pView ) :
    SfxPoolItem     ( nWhich ),
    SdOptionsZoom   ( 0, FALSE )
{
    INT32 nX, nY;

    pOpts->GetScale( nX, nY );
    SetScale( nX, nY );
}

// ----------------------------------------------------------------------

SfxPoolItem* SdOptionsZoomItem::Clone( SfxItemPool* ) const
{
    return new SdOptionsZoomItem( *this );
}


// ----------------------------------------------------------------------

int SdOptionsZoomItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unterschiedliche Typen" );
    return( (SdOptionsZoom&) *this == (const SdOptionsZoom&)(const SdOptionsZoomItem&) rAttr );
}

// -----------------------------------------------------------------------

void SdOptionsZoomItem::SetOptions( SdOptions* pOpts ) const
{
    INT32 nX, nY;

    GetScale( nX, nY );
    pOpts->SetScale( nX, nY );
}

/*************************************************************************
|*
|* SdOptionsGrid
|*
\************************************************************************/

SdOptionsGrid::SdOptionsGrid( USHORT nConfigId, BOOL bUseConfig ) :
    SdOptionsGeneric( nConfigId, bUseConfig ?
                      ( ( SDCFG_DRAW == nConfigId ) ?
                        B2U( "Office.Draw/Grid" ) :
                        B2U( "Office.Impress/Grid" ) ) :
                      OUString() )
{
    EnableModify( FALSE );
    SetDefaults();
    EnableModify( TRUE );
}

// -----------------------------------------------------------------------------

SdOptionsGrid::~SdOptionsGrid()
{
}

// -----------------------------------------------------------------------------

void SdOptionsGrid::SetDefaults()
{
    UINT32 nVal;

    if( GetConfigId() == SDCFG_DRAW )
        nVal = ( ( /*!!!GetMetric() == FUNIT_INCH*/1 ) ? 254 : 250 );
    else
        nVal = ( ( /*!!!GetMetric() == FUNIT_INCH*/1 ) ? 635 : 500 );

    SetFldDivisionX( nVal );
    SetFldDivisionY( nVal );
    SetFldDrawX( nVal = ( ( /*!!!GetMetric() == FUNIT_INCH*/1 ) ? 1270 : 1000 ) );
    SetFldDrawY( nVal );
    SetFldSnapX( nVal );
    SetFldSnapY( nVal );
    SetUseGridSnap( FALSE );
    SetSynchronize( TRUE );
    SetGridVisible( FALSE );
    SetEqualGrid( TRUE );
}

// -----------------------------------------------------------------------------

BOOL SdOptionsGrid::operator==( const SdOptionsGrid& rOpt ) const
{
    return( GetFldDrawX() == rOpt.GetFldDrawX() &&
            GetFldDivisionX() == rOpt.GetFldDivisionX() &&
            GetFldDrawY() == rOpt.GetFldDrawY() &&
            GetFldDivisionY() == rOpt.GetFldDivisionY() &&
            GetFldSnapX() == rOpt.GetFldSnapX() &&
            GetFldSnapY() == rOpt.GetFldSnapY() &&
            IsUseGridSnap() == rOpt.IsUseGridSnap() &&
            IsSynchronize() == rOpt.IsSynchronize() &&
            IsGridVisible() == rOpt.IsGridVisible() &&
            IsEqualGrid() == rOpt.IsEqualGrid() );
}

// -----------------------------------------------------------------------------

void SdOptionsGrid::GetPropNameArray( const char**& ppNames, ULONG& rCount ) const
{
    static const char* aPropNames[] =
    {
        "Resolution/XAxis",
        "Subdivision/XAxis",
        "Resolution/YAxis",
        "Subdivision/YAxis",
        "SnapGrid/XAxis",
        "SnapGrid/YAxis",
        "Option/SnapToGrid",
        "Option/Synchronize",
        "Option/VisibleGrid",
        "SnapGrid/Size"
    };

    rCount = 10;
    ppNames = aPropNames;
}

// -----------------------------------------------------------------------------

BOOL SdOptionsGrid::ReadData( const Any* pValues )
{
    SetFldDrawX( *(sal_Int32*) pValues[ 0 ].getValue() );
    SetFldDivisionX( *(sal_Int32*) pValues[ 1 ].getValue() );
    SetFldDrawY( *(sal_Int32*) pValues[ 2 ].getValue() );
    SetFldDivisionY( *(sal_Int32*) pValues[ 3 ].getValue() );
    SetFldSnapX( *(sal_Int32*) pValues[ 4 ].getValue() );
    SetFldSnapY( *(sal_Int32*) pValues[ 5 ].getValue() );
    SetUseGridSnap( *(sal_Bool*) pValues[ 6 ].getValue() );
    SetSynchronize( *(sal_Bool*) pValues[ 7 ].getValue() );
    SetGridVisible( *(sal_Bool*) pValues[ 8 ].getValue() );
    SetEqualGrid( *(sal_Bool*) pValues[ 9 ].getValue() );

    return TRUE;
}

// -----------------------------------------------------------------------------

BOOL SdOptionsGrid::WriteData( Any* pValues ) const
{
    pValues[ 0 ] <<= (sal_Int32) GetFldDrawX();
    pValues[ 1 ] <<= (sal_Int32) GetFldDivisionX();
    pValues[ 2 ] <<= (sal_Int32) GetFldDrawY();
    pValues[ 3 ] <<= (sal_Int32) GetFldDivisionY();
    pValues[ 4 ] <<= (sal_Int32) GetFldSnapX();
    pValues[ 5 ] <<= (sal_Int32) GetFldSnapY();
    pValues[ 6 ] <<= IsUseGridSnap();
    pValues[ 7 ] <<= IsSynchronize();
    pValues[ 8 ] <<= IsGridVisible();
    pValues[ 9 ] <<= IsEqualGrid();

    return TRUE;
}

/*************************************************************************
|*
|* SdOptionsGridItem
|*
\************************************************************************/

SdOptionsGridItem::SdOptionsGridItem( USHORT nWhich ) :
    SfxPoolItem     ( nWhich ),
    SdOptionsGrid   ( 0, FALSE )
{
}

// -----------------------------------------------------------------------------

SdOptionsGridItem::SdOptionsGridItem( USHORT nWhich, SdOptions* pOpts, FrameView* pView ) :
    SfxPoolItem     ( nWhich ),
    SdOptionsGrid   ( 0, FALSE )
{
    SetSynchronize( pOpts->IsSynchronize() );
    SetEqualGrid( pOpts->IsEqualGrid() );

    if( pView )
    {
        SetFldDrawX( pView->GetGridCoarse().Width() );
        SetFldDrawY( pView->GetGridCoarse().Height() );
        SetFldDivisionX( pView->GetGridFine().Width() ? ( GetFldDrawX() / pView->GetGridFine().Width() - 1 ) : 0 );
        SetFldDivisionY( pView->GetGridFine().Height() ? ( GetFldDrawY() / pView->GetGridFine().Height() - 1 ) : 0 );
        SetFldSnapX( pView->GetSnapGrid().Width() );
        SetFldSnapY( pView->GetSnapGrid().Height() );
        SetUseGridSnap( pView->IsGridSnap() );
        SetGridVisible( pView->IsGridVisible() );
    }
    else
    {
        SetFldDrawX( pOpts->GetFldDrawX() );
        SetFldDrawY( pOpts->GetFldDrawY() );
        SetFldDivisionX( pOpts->GetFldDivisionX() );
        SetFldDivisionY( pOpts->GetFldDivisionY() );
        SetFldSnapX( pOpts->GetFldSnapX() );
        SetFldSnapY( pOpts->GetFldSnapY() );
        SetUseGridSnap( pOpts->IsUseGridSnap() );
        SetGridVisible( pOpts->IsGridVisible() );
    }
}

// ----------------------------------------------------------------------

SfxPoolItem* SdOptionsGridItem::Clone( SfxItemPool* ) const
{
    return new SdOptionsGridItem( *this );
}


// ----------------------------------------------------------------------

int SdOptionsGridItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unterschiedliche Typen" );
    return( (SdOptionsGrid&) *this == (const SdOptionsGrid&)(const SdOptionsGridItem&) rAttr );
}

// -----------------------------------------------------------------------

void SdOptionsGridItem::SetOptions( SdOptions* pOpts ) const
{
    pOpts->SetFldDrawX( GetFldDrawX() );
    pOpts->SetFldDivisionX( GetFldDivisionX() );
    pOpts->SetFldDrawY( GetFldDrawY() );
    pOpts->SetFldDivisionY( GetFldDivisionY() );
    pOpts->SetFldSnapX( GetFldSnapX() );
    pOpts->SetFldSnapY( GetFldSnapY() );
    pOpts->SetUseGridSnap( IsUseGridSnap() );
    pOpts->SetSynchronize( IsSynchronize() );
    pOpts->SetGridVisible( IsGridVisible() );
    pOpts->SetEqualGrid( IsEqualGrid() );
}

/*************************************************************************
|*
|* SdOptionsPrint
|*
\************************************************************************/

SdOptionsPrint::SdOptionsPrint( USHORT nConfigId, BOOL bUseConfig ) :
    SdOptionsGeneric( nConfigId, bUseConfig ?
                      ( ( SDCFG_DRAW == nConfigId ) ?
                        B2U( "Office.Draw/Print" ) :
                        B2U( "Office.Impress/Print" ) ) :
                      OUString() )
{
    EnableModify( FALSE );
    SetDefaults();
    EnableModify( TRUE );
}

// -----------------------------------------------------------------------------

void SdOptionsPrint::SetDefaults()
{
    SetDraw( TRUE );
    SetNotes( FALSE );
    SetHandout( FALSE );
    SetOutline( FALSE );
    SetDate( FALSE );
    SetTime( FALSE );
    SetPagename( FALSE );
    SetHiddenPages( TRUE );
    SetPagesize( FALSE );
    SetPagetile( FALSE );
    SetWarningPrinter( TRUE );
    SetWarningSize( FALSE );
    SetWarningOrientation( FALSE );
    SetBooklet( FALSE );
    SetFrontPage( TRUE );
    SetBackPage( TRUE );
    SetCutPage( FALSE );
    SetPaperbin( FALSE );
    SetOutputQuality( 0 );
}

// -----------------------------------------------------------------------------

BOOL SdOptionsPrint::operator==( const SdOptionsPrint& rOpt ) const
{
    return( IsDraw() == rOpt.IsDraw() &&
            IsNotes() == rOpt.IsNotes() &&
            IsHandout() == rOpt.IsHandout() &&
            IsOutline() == rOpt.IsOutline() &&
            IsDate() == rOpt.IsDate() &&
            IsTime() == rOpt.IsTime() &&
            IsPagename() == rOpt.IsPagename() &&
            IsHiddenPages() == rOpt.IsHiddenPages() &&
            IsPagesize() == rOpt.IsPagesize() &&
            IsPagetile() == rOpt.IsPagetile() &&
            IsWarningPrinter() == rOpt.IsWarningPrinter() &&
            IsWarningSize() == rOpt.IsWarningSize() &&
            IsWarningOrientation() == rOpt.IsWarningOrientation() &&
            IsBooklet() == rOpt.IsBooklet() &&
            IsFrontPage() == rOpt.IsFrontPage() &&
            IsBackPage() == rOpt.IsBackPage() &&
            IsCutPage() == rOpt.IsCutPage() &&
            IsPaperbin() == rOpt.IsPaperbin() &&
            GetOutputQuality() == rOpt.GetOutputQuality() );
}

// -----------------------------------------------------------------------------

void SdOptionsPrint::GetPropNameArray( const char**& ppNames, ULONG& rCount ) const
{
    static const char* aPropNames[] =
    {
        "Content/Drawing",
        "Other/Date",
        "Other/Time",
        "Other/PageName",
        "Other/HiddenPage",
        "Page/PageSize",
        "Page/PageTile",
        // bWarningPrinter
        // bWarningSize
        // bWarningOrientation
        "Page/Booklet",
        "Page/BookletFront",
        "Page/BookletFront",
        // bCutPage
        "Other/FromPrinterSetup",
        "Other/Quality",

        // just for Impress
        "Content/Note",
        "Content/Handout",
        "Content/Outline"
    };

    rCount = ( ( GetConfigId() == SDCFG_IMPRESS ) ? 15 : 12 );
    ppNames = aPropNames;
}

// -----------------------------------------------------------------------------

BOOL SdOptionsPrint::ReadData( const Any* pValues )
{
    SetDraw( *(sal_Bool*) pValues[ 0 ].getValue() );
    SetDate( *(sal_Bool*) pValues[ 1 ].getValue() );
    SetTime( *(sal_Bool*) pValues[ 2 ].getValue() );
    SetPagename( *(sal_Bool*) pValues[ 3 ].getValue() );
    SetHiddenPages( *(sal_Bool*) pValues[ 4 ].getValue() );
    SetPagesize( *(sal_Bool*) pValues[ 5 ].getValue() );
    SetPagetile( *(sal_Bool*) pValues[ 6 ].getValue() );
    SetBooklet( *(sal_Bool*) pValues[ 7 ].getValue() );
    SetFrontPage( *(sal_Bool*) pValues[ 8 ].getValue() );
    SetBackPage( *(sal_Bool*) pValues[ 9 ].getValue() );
    SetPaperbin( *(sal_Bool*) pValues[ 10 ].getValue() );
    SetOutputQuality( *(sal_Int32*) pValues[ 11 ].getValue() );

    // just for impress
    if( GetConfigId() == SDCFG_IMPRESS )
    {
        SetNotes( *(sal_Bool*) pValues[ 12 ].getValue() );
        SetHandout( *(sal_Bool*) pValues[ 13 ].getValue() );
        SetOutline( *(sal_Bool*) pValues[ 14 ].getValue() );
    }

    return TRUE;
}

// -----------------------------------------------------------------------------

BOOL SdOptionsPrint::WriteData( Any* pValues ) const
{
    pValues[ 0 ] <<= IsDraw();
    pValues[ 1 ] <<= IsDate();
    pValues[ 2 ] <<= IsTime();
    pValues[ 3 ] <<= IsPagename();
    pValues[ 4 ] <<= IsHiddenPages();
    pValues[ 5 ] <<= IsPagesize();
    pValues[ 6 ] <<= IsPagetile();
    pValues[ 7 ] <<= IsBooklet();
    pValues[ 8 ] <<= IsFrontPage();
    pValues[ 9 ] <<= IsBackPage();
    pValues[ 10 ] <<= IsPaperbin();
    pValues[ 11 ] <<= (sal_Int32) GetOutputQuality();

    // just for impress
    if( GetConfigId() == SDCFG_IMPRESS )
    {
        pValues[ 12 ] <<= IsNotes();
        pValues[ 13 ] <<= IsHandout();
        pValues[ 14 ] <<= IsOutline();
    }

    return TRUE;
}

/*************************************************************************
|*
|* SdOptionsPrintItem
|*
\************************************************************************/

SdOptionsPrintItem::SdOptionsPrintItem( USHORT nWhich ) :
    SfxPoolItem     ( nWhich ),
    SdOptionsPrint  ( 0, FALSE )
{
}

// ----------------------------------------------------------------------

SdOptionsPrintItem::SdOptionsPrintItem( USHORT nWhich, SdOptions* pOpts, FrameView* pView ) :
    SfxPoolItem     ( nWhich ),
    SdOptionsPrint  ( 0, FALSE )
{
    SetDraw( pOpts->IsDraw() );
    SetNotes( pOpts->IsNotes() );
    SetHandout( pOpts->IsHandout() );
    SetOutline( pOpts->IsOutline() );
    SetDate( pOpts->IsDate() );
    SetTime( pOpts->IsTime() );
    SetPagename( pOpts->IsPagename() );
    SetHiddenPages( pOpts->IsHiddenPages() );
    SetPagesize( pOpts->IsPagesize() );
    SetPagetile( pOpts->IsPagetile() );
    SetWarningPrinter( pOpts->IsWarningPrinter() );
    SetWarningSize( pOpts->IsWarningSize() );
    SetWarningOrientation( pOpts->IsWarningOrientation() );
    SetBooklet( pOpts->IsBooklet() );
    SetFrontPage( pOpts->IsFrontPage() );
    SetBackPage( pOpts->IsBackPage() );
    SetCutPage( pOpts->IsCutPage() );
    SetPaperbin( pOpts->IsPaperbin() );
    SetOutputQuality( pOpts->GetOutputQuality() );
}

// ----------------------------------------------------------------------

SfxPoolItem* SdOptionsPrintItem::Clone( SfxItemPool* ) const
{
    return new SdOptionsPrintItem( *this );
}

// ----------------------------------------------------------------------

int SdOptionsPrintItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unterschiedliche Typen" );
    return( (SdOptionsPrint&) *this == (const SdOptionsPrint&)(const SdOptionsPrintItem&) rAttr );
}

// -----------------------------------------------------------------------

void SdOptionsPrintItem::SetOptions( SdOptions* pOpts ) const
{
    pOpts->SetDraw( IsDraw() );
    pOpts->SetNotes( IsNotes() );
    pOpts->SetHandout( IsHandout() );
    pOpts->SetOutline( IsOutline() );
    pOpts->SetDate( IsDate() );
    pOpts->SetTime( IsTime() );
    pOpts->SetPagename( IsPagename() );
    pOpts->SetHiddenPages( IsHiddenPages() );
    pOpts->SetPagesize( IsPagesize() );
    pOpts->SetPagetile( IsPagetile() );
    pOpts->SetWarningPrinter( IsWarningPrinter() );
    pOpts->SetWarningSize( IsWarningSize() );
    pOpts->SetWarningOrientation( IsWarningOrientation() );
    pOpts->SetBooklet( IsBooklet() );
    pOpts->SetFrontPage( IsFrontPage() );
    pOpts->SetBackPage( IsBackPage() );
    pOpts->SetCutPage( IsCutPage() );
    pOpts->SetPaperbin( IsPaperbin() );
    pOpts->SetOutputQuality( GetOutputQuality() );
}

/*************************************************************************
|*
|* SdOptions
|*
\************************************************************************/

SdOptions::SdOptions( USHORT nConfigId ) :
    SdOptionsLayout( nConfigId, TRUE ),
    SdOptionsContents( nConfigId, TRUE ),
    SdOptionsMisc( nConfigId, TRUE ),
    SdOptionsSnap( nConfigId, TRUE ),
    SdOptionsZoom( nConfigId, TRUE ),
    SdOptionsGrid( nConfigId, TRUE ),
    SdOptionsPrint( nConfigId, TRUE )
{
}

// ----------------------------------------------------------------------

SdOptions::~SdOptions()
{
}

// ----------------------------------------------------------------------

void SdOptions::SetDefaults( ULONG nOptionsRange )
{
    if( nOptionsRange & SD_OPTIONS_LAYOUT )
        SdOptionsLayout::SetDefaults();

    if( nOptionsRange & SD_OPTIONS_CONTENTS )
        SdOptionsContents::SetDefaults();

    if( nOptionsRange & SD_OPTIONS_MISC )
        SdOptionsMisc::SetDefaults();

    if( nOptionsRange & SD_OPTIONS_SNAP )
        SdOptionsSnap::SetDefaults();

    if( nOptionsRange & SD_OPTIONS_ZOOM )
        SdOptionsZoom::SetDefaults();

    if( nOptionsRange & SD_OPTIONS_GRID )
        SdOptionsGrid::SetDefaults();

    if( nOptionsRange & SD_OPTIONS_PRINT )
        SdOptionsPrint::SetDefaults();
}

// ----------------------------------------------------------------------

void SdOptions::StoreConfig()
{
    DBG_ERROR( "SdOptions::StoreConfig(): Missing" );
}

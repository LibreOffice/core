/*************************************************************************
 *
 *  $RCSfile: tpoption.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:34 $
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

#ifndef _SFXMODULE_HXX //autogen
#include <sfx2/module.hxx>
#endif
#include <sfx2/app.hxx>
#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>
#include <svx/strarray.hxx>
#include <svx/dlgutil.hxx>
#ifndef _SFX_SAVEOPT_HXX //autogen
#include <sfx2/saveopt.hxx>
#endif

#include "sdattr.hxx"
#include "sdresid.hxx"
#include "optsitem.hxx"
#include "tpoption.hrc"
#include "tpoption.hxx"

#define DLGWIN this->GetParent()->GetParent()



/*************************************************************************
|*
|*  TabPage zum Einstellen der Fang-Optionen
|*
\************************************************************************/

SdTpOptionsSnap::SdTpOptionsSnap( Window* pParent, const SfxItemSet& rInAttrs  ) :
        SfxTabPage          ( pParent, SdResId( TP_OPTIONS_SNAP ), rInAttrs ),

        aFiSnap             ( this, SdResId( FI_SNAP ) ),
        aCbxSnapGrid        ( this, SdResId( CBX_SNAP_GRID ) ),
        aCbxSnapHelplines   ( this, SdResId( CBX_SNAP_HELPLINES ) ),
        aCbxSnapBorder      ( this, SdResId( CBX_SNAP_BORDER ) ),
        aCbxSnapFrame       ( this, SdResId( CBX_SNAP_FRAME ) ),
        aCbxSnapPoints      ( this, SdResId( CBX_SNAP_POINTS ) ),
        aFtSnapArea         ( this, SdResId( FT_SNAP_AREA ) ),
        aMtrFldSnapArea     ( this, SdResId( MTR_FLD_SNAP_AREA ) ),
        aGrpSnap            ( this, SdResId( GRP_SNAP ) ),
        aFiOrtho            ( this, SdResId( FI_ORTHO ) ),
        aCbxOrtho           ( this, SdResId( CBX_ORTHO ) ),
        aCbxBigOrtho        ( this, SdResId( CBX_BIGORTHO ) ),
        aCbxRotate          ( this, SdResId( CBX_ROTATE ) ),
        aMtrFldAngle        ( this, SdResId( MTR_FLD_ANGLE ) ),
        aFtBezAngle         ( this, SdResId( FT_BEZ_ANGLE ) ),
        aMtrFldBezAngle     ( this, SdResId( MTR_FLD_BEZ_ANGLE ) ),
        aGrpOrtho           ( this, SdResId( GRP_ORTHO ) ),
        rOutAttrs           ( rInAttrs ),
        pExampleSet         (0)
{
    FreeResource();

    SetExchangeSupport();

    aCbxRotate.SetClickHdl( LINK( this, SdTpOptionsSnap, ClickRotateHdl ) );
}

// -----------------------------------------------------------------------

SdTpOptionsSnap::~SdTpOptionsSnap()
{
}

// -----------------------------------------------------------------------

BOOL SdTpOptionsSnap::FillItemSet( SfxItemSet& rAttrs )
{
    SdOptionsSnapItem* pOptsItem = NULL;
    if(SFX_ITEM_SET != rAttrs.GetItemState( ATTR_OPTIONS_SNAP, FALSE, (const SfxPoolItem**)&pOptsItem ))
        pExampleSet->GetItemState( ATTR_OPTIONS_SNAP, FALSE, (const SfxPoolItem**)&pOptsItem );

    SdOptionsSnapItem aOptsItem( ATTR_OPTIONS_SNAP );

    aOptsItem.SetSnapHelplines( aCbxSnapHelplines.IsChecked() );
    aOptsItem.SetSnapBorder( aCbxSnapBorder.IsChecked() );
    aOptsItem.SetSnapFrame( aCbxSnapFrame.IsChecked() );
    aOptsItem.SetSnapPoints( aCbxSnapPoints.IsChecked() );
    aOptsItem.SetOrtho( aCbxOrtho.IsChecked() );
    aOptsItem.SetBigOrtho( aCbxBigOrtho.IsChecked() );
    aOptsItem.SetRotate( aCbxRotate.IsChecked() );
    aOptsItem.SetSnapArea( (INT16) aMtrFldSnapArea.GetValue() );
    aOptsItem.SetAngle( (INT16) aMtrFldAngle.GetValue() );
    aOptsItem.SetEliminatePolyPointLimitAngle( (INT16) aMtrFldBezAngle.GetValue() );

    if( pOptsItem == NULL || !(aOptsItem == *pOptsItem) )
        rAttrs.Put( aOptsItem );

    // Evtl. vorhandenes GridItem wird geholt, um nicht versehentlich
    // irgendwelche Standardwerte einzustellen
    const SfxPoolItem* pAttr = NULL;

    // der ExampleSet wird im Reset mit dem inputset initialisiert
    DBG_ASSERT(pExampleSet, "Wo ist der ExampleSet?")
    if( SFX_ITEM_SET != rAttrs.GetItemState( SID_ATTR_GRID_OPTIONS , FALSE, (const SfxPoolItem**) &pAttr ) )
        pExampleSet->GetItemState( SID_ATTR_GRID_OPTIONS , FALSE, (const SfxPoolItem**) &pAttr );

    if( pAttr )
    {
        SvxGridItem* pGridAttr = (SvxGridItem*) pAttr;
        if(pGridAttr->GetUseGridSnap() != aCbxSnapGrid.IsChecked())
        {
            pGridAttr->SetUseGridSnap( aCbxSnapGrid.IsChecked() );
            rAttrs.Put( *pGridAttr );
        }
    }

    return( TRUE );
}

// -----------------------------------------------------------------------

void __EXPORT SdTpOptionsSnap::Reset( const SfxItemSet& rAttrs )
{
    pExampleSet = &rAttrs;

    SdOptionsSnapItem aOptsItem( (const SdOptionsSnapItem&) rAttrs.
                        Get( ATTR_OPTIONS_SNAP ) );

    aCbxSnapHelplines.Check( aOptsItem.IsSnapHelplines() );
    aCbxSnapBorder.Check( aOptsItem.IsSnapBorder() );
    aCbxSnapFrame.Check( aOptsItem.IsSnapFrame() );
    aCbxSnapPoints.Check( aOptsItem.IsSnapPoints() );
    aCbxOrtho.Check( aOptsItem.IsOrtho() );
    aCbxBigOrtho.Check( aOptsItem.IsBigOrtho() );
    aCbxRotate.Check( aOptsItem.IsRotate() );
    aMtrFldSnapArea.SetValue( aOptsItem.GetSnapArea() );
    aMtrFldAngle.SetValue( aOptsItem.GetAngle() );
    aMtrFldBezAngle.SetValue( aOptsItem.GetEliminatePolyPointLimitAngle() );

    ClickRotateHdl( NULL );
}

// -----------------------------------------------------------------------

SfxTabPage* __EXPORT SdTpOptionsSnap::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SdTpOptionsSnap( pWindow, rAttrs ) );
}

// -----------------------------------------------------------------------

IMPL_LINK( SdTpOptionsSnap, ClickRotateHdl, void *, p )
{
    if( aCbxRotate.IsChecked() )
        aMtrFldAngle.Enable();
    else
        aMtrFldAngle.Disable();

    return( 0L );
}

// -----------------------------------------------------------------------

void SdTpOptionsSnap::ActivatePage( const SfxItemSet& rSet )
{
    const SfxPoolItem* pAttr = NULL;

    if( SFX_ITEM_SET != rSet.GetItemState( SID_ATTR_GRID_OPTIONS , FALSE, (const SfxPoolItem**)&pAttr ))
        if( SFX_ITEM_SET != pExampleSet->GetItemState( SID_ATTR_GRID_OPTIONS, FALSE, (const SfxPoolItem**)&pAttr ))
            pAttr = NULL;

    if(pAttr)
    {
        SvxGridItem* pGridAttr = (SvxGridItem*) pAttr;
        aCbxSnapGrid.Check( pGridAttr->GetUseGridSnap() );
    }
}

// -----------------------------------------------------------------------

int SdTpOptionsSnap::DeactivatePage( SfxItemSet* pSet )
{
    FillItemSet( *pSet );

    return( LEAVE_PAGE );
}

/*************************************************************************
|*
|*  TabPage zum Einstellen der Layout-Optionen
|*
\************************************************************************/

SdTpOptionsLayout::SdTpOptionsLayout( Window* pParent, const SfxItemSet& rInAttrs  ) :
        SfxTabPage          ( pParent, SdResId( TP_OPTIONS_LAYOUT ), rInAttrs ),
        aCbxRuler           ( this, SdResId( CBX_RULER ) ),
        aCbxMoveOutline     ( this, SdResId( CBX_MOVE_OUTLINE ) ),
        aCbxDragStripes     ( this, SdResId( CBX_HELPLINES ) ),
        aCbxHandlesBezier   ( this, SdResId( CBX_HANDLES_BEZIER ) ),
        aGrpDisplay         ( this, SdResId( GRP_DISPLAY ) ),

        aLbMetric           ( this, ResId( LB_METRIC ) ),
        aGrpMetric          ( this, ResId( GRP_METRIC ) ),

        aMtrFldTabstop      ( this, ResId( MTR_FLD_TABSTOP ) ),
        aGrpTabstop         ( this, ResId( GRP_TABSTOP ) ),

        rOutAttrs           ( rInAttrs )
{
    FreeResource();

    SetExchangeSupport();

    // Metrik einstellen
    FieldUnit eFUnit;// = SFX_APP()->GetOptions().GetMetric();

    USHORT nWhich = GetWhich( SID_ATTR_METRIC );
    if ( rInAttrs.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
    {
        const SfxUInt16Item& rItem = (SfxUInt16Item&)rInAttrs.Get( nWhich );
        eFUnit = (FieldUnit)rItem.GetValue();
    }
    else
        GET_MODULE_FIELDUNIT( eFUnit );

    SetFieldUnit( aMtrFldTabstop, eFUnit );

    // ListBox mit Metriken f"ullen
    SvxStringArray aMetricArr( RID_SVXSTR_FIELDUNIT_TABLE );

    for ( USHORT i = 0; i < aMetricArr.Count(); ++i )
    {
        String sMetric = aMetricArr.GetStringByPos( i );
        long nFieldUnit = aMetricArr.GetValue( i );
        USHORT nPos = aLbMetric.InsertEntry( sMetric );
        aLbMetric.SetEntryData( nPos, (void*)nFieldUnit );
    }
    aLbMetric.SetSelectHdl( LINK( this, SdTpOptionsLayout, SelectMetricHdl_Impl ) );
}

// -----------------------------------------------------------------------

SdTpOptionsLayout::~SdTpOptionsLayout()
{
}

// -----------------------------------------------------------------------

BOOL SdTpOptionsLayout::FillItemSet( SfxItemSet& rAttrs )
{
    BOOL bModified = FALSE;

    if( aCbxRuler.GetSavedValue()           != aCbxRuler.IsChecked() ||
        aCbxMoveOutline.GetSavedValue()     != aCbxMoveOutline.IsChecked() ||
        aCbxDragStripes.GetSavedValue()     != aCbxDragStripes.IsChecked() ||
        //aCbxHelplines.GetSavedValue()     != aCbxHelplines.IsChecked() ||
        aCbxHandlesBezier.GetSavedValue()   != aCbxHandlesBezier.IsChecked() )
    {
        bModified = TRUE;
    }

    SdOptionsLayoutItem aOptsItem( ATTR_OPTIONS_LAYOUT );

    aOptsItem.SetRulerVisible( aCbxRuler.IsChecked() );
    aOptsItem.SetMoveOutline( aCbxMoveOutline.IsChecked() );
    aOptsItem.SetDragStripes( aCbxDragStripes.IsChecked() );
    aOptsItem.SetHandlesBezier( aCbxHandlesBezier.IsChecked() );
    //aOptsItem.SetHelplines( aCbxHelplines.IsChecked() );

    rAttrs.Put( aOptsItem );

    // Metrik
    const USHORT nMPos = aLbMetric.GetSelectEntryPos();
    if ( nMPos != aLbMetric.GetSavedValue() )
    {
        USHORT nFieldUnit = (USHORT)(long)aLbMetric.GetEntryData( nMPos );
        rAttrs.Put( SfxUInt16Item( GetWhich( SID_ATTR_METRIC ),
                                     (UINT16)nFieldUnit ) );
        bModified |= TRUE;
    }

    // Tabulatorabstand
    if( aMtrFldTabstop.GetText() != aMtrFldTabstop.GetSavedValue() )
    {
        USHORT nWh = GetWhich( SID_ATTR_DEFTABSTOP );
        SfxMapUnit eUnit = rAttrs.GetPool()->GetMetric( nWh );
        SfxUInt16Item aDef( nWh,(USHORT)GetCoreValue( aMtrFldTabstop, eUnit ) );
        rAttrs.Put( aDef );
        bModified |= TRUE;
    }
    return( bModified );
}

// -----------------------------------------------------------------------

void SdTpOptionsLayout::Reset( const SfxItemSet& rAttrs )
{
    SdOptionsLayoutItem aOptsItem( (const SdOptionsLayoutItem&) rAttrs.
                        Get( ATTR_OPTIONS_LAYOUT ) );

    aCbxRuler.Check( aOptsItem.IsRulerVisible() );
    aCbxMoveOutline.Check( aOptsItem.IsMoveOutline() );
    aCbxDragStripes.Check( aOptsItem.IsDragStripes() );
    aCbxHandlesBezier.Check( aOptsItem.IsHandlesBezier() );
    //aCbxHelplines.Check( aOptsItem.IsHelplines() );

    aCbxRuler.SaveValue();
    aCbxMoveOutline.SaveValue();
    aCbxDragStripes.SaveValue();
    aCbxHandlesBezier.SaveValue();
    //aCbxHelplines.SaveValue();

    // Metrik
    USHORT nWhich = GetWhich( SID_ATTR_METRIC );
    aLbMetric.SetNoSelection();

    if ( rAttrs.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
    {
        const SfxUInt16Item& rItem = (SfxUInt16Item&)rAttrs.Get( nWhich );
        long nFieldUnit = (long)rItem.GetValue();

        for ( USHORT i = 0; i < aLbMetric.GetEntryCount(); ++i )
        {
            if ( (long)aLbMetric.GetEntryData( i ) == nFieldUnit )
            {
                aLbMetric.SelectEntryPos( i );
                break;
            }
        }
    }

    // Tabulatorabstand
    nWhich = GetWhich( SID_ATTR_DEFTABSTOP );
    if( rAttrs.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
    {
        SfxMapUnit eUnit = rAttrs.GetPool()->GetMetric( nWhich );
        const SfxUInt16Item& rItem = (SfxUInt16Item&)rAttrs.Get( nWhich );
        SetMetricValue( aMtrFldTabstop, rItem.GetValue(), eUnit );
    }
    aLbMetric.SaveValue();
    aMtrFldTabstop.SaveValue();
}

// -----------------------------------------------------------------------

SfxTabPage* __EXPORT SdTpOptionsLayout::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SdTpOptionsLayout( pWindow, rAttrs ) );
}

// -----------------------------------------------------------------------

void SdTpOptionsLayout::ActivatePage( const SfxItemSet& rSet )
{
    // Hier muss noch einmal SaveValue gerufen werden, da sonst u.U.
    // der Wert in anderen TabPages keine Wirkung hat
    aLbMetric.SaveValue();
}

// -----------------------------------------------------------------------

int SdTpOptionsLayout::DeactivatePage( SfxItemSet* pSet )
{
    FillItemSet( *pSet );

    return LEAVE_PAGE;
}

//------------------------------------------------------------------------

IMPL_LINK( SdTpOptionsLayout, SelectMetricHdl_Impl, ListBox *, EMPTYARG )
{
    USHORT nPos = aLbMetric.GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        FieldUnit eUnit = (FieldUnit)(long)aLbMetric.GetEntryData( nPos );
        long nVal =
            aMtrFldTabstop.Denormalize( aMtrFldTabstop.GetValue( FUNIT_TWIP ) );
        SetFieldUnit( aMtrFldTabstop, eUnit );
        aMtrFldTabstop.SetValue( aMtrFldTabstop.Normalize( nVal ), FUNIT_TWIP );
    }
    return 0;
}

/*************************************************************************
|*
|*  TabPage zum Einstellen der Inhalte-Optionen
|*
\************************************************************************/

SdTpOptionsContents::SdTpOptionsContents( Window* pParent, const SfxItemSet& rInAttrs  ) :
        SfxTabPage          ( pParent, SdResId( TP_OPTIONS_CONTENTS ), rInAttrs ),
        aCbxExternGraphic   ( this, SdResId( CBX_EXTERN_GRAPHIC ) ),
        aCbxOutlineMode     ( this, SdResId( CBX_OUTLINEMODE ) ),
        aCbxNoText          ( this, SdResId( CBX_NOTEXT ) ),
        aCbxHairlineMode    ( this, SdResId( CBX_HAIRLINEMODE ) ),
        aGrpViewSubstitute  ( this, SdResId( GRP_VIEW_SUBSTITUTE ) ),

        rOutAttrs           ( rInAttrs )
{
    FreeResource();
}

// -----------------------------------------------------------------------

SdTpOptionsContents::~SdTpOptionsContents()
{
}

// -----------------------------------------------------------------------

BOOL SdTpOptionsContents::FillItemSet( SfxItemSet& rAttrs )
{
    BOOL bModified = FALSE;

    if( aCbxExternGraphic.GetSavedValue()   != aCbxExternGraphic.IsChecked() ||
        aCbxOutlineMode.GetSavedValue()     != aCbxOutlineMode.IsChecked() ||
        aCbxNoText.GetSavedValue()          != aCbxNoText.IsChecked() ||
        aCbxHairlineMode.GetSavedValue()    != aCbxHairlineMode.IsChecked() )
    {
        SdOptionsContentsItem aOptsItem( ATTR_OPTIONS_CONTENTS );

        aOptsItem.SetExternGraphic( aCbxExternGraphic.IsChecked() );
        aOptsItem.SetOutlineMode( aCbxOutlineMode.IsChecked() );
        aOptsItem.SetNoText( aCbxNoText.IsChecked() );
        aOptsItem.SetHairlineMode( aCbxHairlineMode.IsChecked() );

        rAttrs.Put( aOptsItem );

        bModified = TRUE;
    }
    return( bModified );
}

// -----------------------------------------------------------------------

void SdTpOptionsContents::Reset( const SfxItemSet& rAttrs )
{
    SdOptionsContentsItem aOptsItem( (const SdOptionsContentsItem&) rAttrs.
                        Get( ATTR_OPTIONS_CONTENTS ) );

    aCbxExternGraphic.Check( aOptsItem.IsExternGraphic() );
    aCbxOutlineMode.Check(aOptsItem.IsOutlineMode() );
    aCbxNoText.Check( aOptsItem.IsNoText() );
    aCbxHairlineMode.Check( aOptsItem.IsHairlineMode() );

    aCbxExternGraphic.SaveValue();
    aCbxOutlineMode.SaveValue();
    aCbxNoText.SaveValue();
    aCbxHairlineMode.SaveValue();
}

// -----------------------------------------------------------------------

SfxTabPage* __EXPORT SdTpOptionsContents::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SdTpOptionsContents( pWindow, rAttrs ) );
}

/*************************************************************************
|*
|*  TabPage zum Einstellen der Sonstige-Optionen
|*
\************************************************************************/

SdTpOptionsMisc::SdTpOptionsMisc( Window* pParent, const SfxItemSet& rInAttrs  ) :
        SfxTabPage          ( pParent, SdResId( TP_OPTIONS_MISC ), rInAttrs ),
    aCbxQuickEdit               ( this, SdResId( CBX_QUICKEDIT ) ),
    aCbxPickThrough             ( this, SdResId( CBX_PICKTHROUGH ) ),
    aGrpText                    ( this, SdResId( GRP_TEXT ) ),

    // Template & Layout laufen z.Z. synchron!
    aCbxStartWithTemplate       ( this, SdResId( CBX_START_WITH_TEMPLATE ) ),
//    aCbxStartWithLayout         ( this, SdResId( CBX_START_WITH_LAYOUT ) ),
    aGrpProgramStart            ( this, SdResId( GRP_PROGRAMSTART ) ),

    aCbxMasterPageCache         ( this, SdResId( CBX_MASTERPAGE_CACHE ) ),
    aGrpDisplay                 ( this, SdResId( GRP_DISPLAY ) ),

    aCbxCopy                    ( this, SdResId( CBX_COPY ) ),
    aCbxMarkedHitMovesAlways    ( this, SdResId( CBX_MARKED_HIT_MOVES_ALWAYS ) ),
    aCbxCrookNoContortion       ( this, SdResId( CBX_CROOK_NO_CONTORTION ) ),
    aGrpOther                   ( this, SdResId( GRP_OTHER ) ),

    /// NEU
    aCbxStartWithActualPage     ( this, SdResId( CBX_START_WITH_ACTUAL_PAGE ) ),
    aGrpStartWithActualPage     ( this, SdResId( GRP_START_WITH_ACTUAL_PAGE ) ),

    rOutAttrs                   ( rInAttrs )
{
    FreeResource();
}

// -----------------------------------------------------------------------

SdTpOptionsMisc::~SdTpOptionsMisc()
{
}

// -----------------------------------------------------------------------

BOOL SdTpOptionsMisc::FillItemSet( SfxItemSet& rAttrs )
{
    BOOL bModified = FALSE;

    if( aCbxStartWithTemplate.GetSavedValue()   != aCbxStartWithTemplate.IsChecked() ||
        aCbxMarkedHitMovesAlways.GetSavedValue()!= aCbxMarkedHitMovesAlways.IsChecked() ||
        aCbxCrookNoContortion.GetSavedValue()   != aCbxCrookNoContortion.IsChecked() ||
        aCbxQuickEdit.GetSavedValue()           != aCbxQuickEdit.IsChecked() ||
        aCbxPickThrough.GetSavedValue()         != aCbxPickThrough.IsChecked() ||
        aCbxMasterPageCache.GetSavedValue()     != aCbxMasterPageCache.IsChecked() ||
        aCbxCopy.GetSavedValue()                != aCbxCopy.IsChecked() ||
        /// NEU
        aCbxStartWithActualPage.GetSavedValue() != aCbxStartWithActualPage.IsChecked() )
    {
        SdOptionsMiscItem aOptsItem( ATTR_OPTIONS_MISC );

        // Layout & Template laufen z.Z. synchron!
        aOptsItem.SetStartWithTemplate( aCbxStartWithTemplate.IsChecked() );
        // aOptsItem.SetStartWithLayout( aCbxStartWithLayout.IsChecked() );

        aOptsItem.SetMarkedHitMovesAlways( aCbxMarkedHitMovesAlways.IsChecked() );
        aOptsItem.SetCrookNoContortion( aCbxCrookNoContortion.IsChecked() );
        aOptsItem.SetQuickEdit( aCbxQuickEdit.IsChecked() );
        aOptsItem.SetPickThrough( aCbxPickThrough.IsChecked() );
        aOptsItem.SetMasterPagePaintCaching( aCbxMasterPageCache.IsChecked() );
        aOptsItem.SetDragWithCopy( aCbxCopy.IsChecked() );

        /// NEU
        aOptsItem.SetStartWithActualPage( aCbxStartWithActualPage.IsChecked() );

        rAttrs.Put( aOptsItem );

        bModified = TRUE;
    }
    return( bModified );
}

// -----------------------------------------------------------------------

void SdTpOptionsMisc::Reset( const SfxItemSet& rAttrs )
{
    SdOptionsMiscItem aOptsItem( (const SdOptionsMiscItem&) rAttrs.
                        Get( ATTR_OPTIONS_MISC ) );

    // Template & Layout laufen z.Z. synchron!
    aCbxStartWithTemplate.Check( aOptsItem.IsStartWithTemplate() );
    // aCbxStartWithLayout.Check( aOptsItem.IsStartWithLayout() );

    aCbxMarkedHitMovesAlways.Check( aOptsItem.IsMarkedHitMovesAlways() );
    aCbxCrookNoContortion.Check( aOptsItem.IsCrookNoContortion() );
    aCbxQuickEdit.Check( aOptsItem.IsQuickEdit() );
    aCbxPickThrough.Check( aOptsItem.IsPickThrough() );
    aCbxMasterPageCache.Check( aOptsItem.IsMasterPagePaintCaching() );
    aCbxCopy.Check( aOptsItem.IsDragWithCopy() );
    /// NEU
    aCbxStartWithActualPage.Check( aOptsItem.IsStartWithActualPage() );

    aCbxStartWithTemplate.SaveValue();
    // aCbxStartWithLayout.SaveValue();
    aCbxMarkedHitMovesAlways.SaveValue();
    aCbxCrookNoContortion.SaveValue();
    aCbxQuickEdit.SaveValue();
    aCbxPickThrough.SaveValue();
    aCbxMasterPageCache.SaveValue();
    aCbxCopy.SaveValue();
    /// NEU
    aCbxStartWithActualPage.SaveValue();
}

// -----------------------------------------------------------------------

SfxTabPage* __EXPORT SdTpOptionsMisc::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SdTpOptionsMisc( pWindow, rAttrs ) );
}




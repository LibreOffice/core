/*************************************************************************
 *
 *  $RCSfile: tpoption.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: os $ $Date: 2001-03-22 14:14:49 $
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
        SvxGridTabPage(pParent, rInAttrs)
{
    aGrpSnap.Show();
    aCbxSnapHelplines.Show();
    aCbxSnapBorder.Show();
    aCbxSnapFrame.Show();
    aCbxSnapPoints.Show();
    aFtSnapArea.Show();
    aMtrFldSnapArea.Show();
    aGrpOrtho.Show();
    aCbxOrtho.Show();
    aCbxBigOrtho.Show();
    aCbxRotate.Show();
    aMtrFldAngle.Show();
    aFtBezAngle.Show();
    aMtrFldBezAngle.Show();
}

// -----------------------------------------------------------------------

SdTpOptionsSnap::~SdTpOptionsSnap()
{
}

// -----------------------------------------------------------------------

BOOL SdTpOptionsSnap::FillItemSet( SfxItemSet& rAttrs )
{
    SvxGridTabPage::FillItemSet(rAttrs);
    SdOptionsSnapItem* pOptsItem = NULL;
//    if(SFX_ITEM_SET != rAttrs.GetItemState( ATTR_OPTIONS_SNAP, FALSE, (const SfxPoolItem**)&pOptsItem ))
//        pExampleSet->GetItemState( ATTR_OPTIONS_SNAP, FALSE, (const SfxPoolItem**)&pOptsItem );

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
    return( TRUE );
}

// -----------------------------------------------------------------------

void __EXPORT SdTpOptionsSnap::Reset( const SfxItemSet& rAttrs )
{
    SvxGridTabPage::Reset(rAttrs);

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

    aCbxRotate.GetClickHdl().Call(0);
}

// -----------------------------------------------------------------------

SfxTabPage* __EXPORT SdTpOptionsSnap::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SdTpOptionsSnap( pWindow, rAttrs ) );
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
        aCbxRuler           ( this, SdResId( CBX_RULER ) ),
        aCbxMoveOutline     ( this, SdResId( CBX_MOVE_OUTLINE ) ),
        aCbxDragStripes     ( this, SdResId( CBX_HELPLINES ) ),
        aCbxHandlesBezier   ( this, SdResId( CBX_HANDLES_BEZIER ) ),
        aGrpDisplay         ( this, SdResId( GRP_DISPLAY ) )
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

    if( aCbxRuler.GetSavedValue()           != aCbxRuler.IsChecked() ||
        aCbxMoveOutline.GetSavedValue()     != aCbxMoveOutline.IsChecked() ||
        aCbxDragStripes.GetSavedValue()     != aCbxDragStripes.IsChecked() ||
        //aCbxHelplines.GetSavedValue()     != aCbxHelplines.IsChecked() ||
        aCbxHandlesBezier.GetSavedValue()   != aCbxHandlesBezier.IsChecked() )
    {
        SdOptionsLayoutItem aOptsItem( ATTR_OPTIONS_LAYOUT );

        aOptsItem.SetRulerVisible( aCbxRuler.IsChecked() );
        aOptsItem.SetMoveOutline( aCbxMoveOutline.IsChecked() );
        aOptsItem.SetDragStripes( aCbxDragStripes.IsChecked() );
        aOptsItem.SetHandlesBezier( aCbxHandlesBezier.IsChecked() );
        //aOptsItem.SetHelplines( aCbxHelplines.IsChecked() );

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

    SdOptionsLayoutItem aLayoutItem( (const SdOptionsLayoutItem&) rAttrs.
                        Get( ATTR_OPTIONS_LAYOUT ) );

    aCbxRuler.Check( aLayoutItem.IsRulerVisible() );
    aCbxMoveOutline.Check( aLayoutItem.IsMoveOutline() );
    aCbxDragStripes.Check( aLayoutItem.IsDragStripes() );
    aCbxHandlesBezier.Check( aLayoutItem.IsHandlesBezier() );
    //aCbxHelplines.Check( aLayoutItem.IsHelplines() );

    aCbxRuler.SaveValue();
    aCbxMoveOutline.SaveValue();
    aCbxDragStripes.SaveValue();
    aCbxHandlesBezier.SaveValue();
    //aCbxHelplines.SaveValue();
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
    aGrpProgramStart            ( this, SdResId( GRP_PROGRAMSTART ) ),

    aCbxMasterPageCache         ( this, SdResId( CBX_MASTERPAGE_CACHE ) ),

    aCbxCopy                    ( this, SdResId( CBX_COPY ) ),
    aCbxMarkedHitMovesAlways    ( this, SdResId( CBX_MARKED_HIT_MOVES_ALWAYS ) ),
    aCbxCrookNoContortion       ( this, SdResId( CBX_CROOK_NO_CONTORTION ) ),
    aGrpOther                   ( this, SdResId( GRP_OTHER ) ),

    /// NEU
    aCbxStartWithActualPage     ( this, SdResId( CBX_START_WITH_ACTUAL_PAGE ) ),
    aGrpStartWithActualPage     ( this, SdResId( GRP_START_WITH_ACTUAL_PAGE ) ),

    aGrpSettings                ( this, SdResId( GRP_SETTINGS ) ),
    aTxtMetric                  ( this, SdResId( FT_METRIC ) ),
    aLbMetric                   ( this, SdResId( LB_METRIC ) ),
    aTxtTabstop                 ( this, SdResId( FT_TABSTOP ) ),
    aMtrFldTabstop              ( this, SdResId( MTR_FLD_TABSTOP ) )
{
    FreeResource();
    SetExchangeSupport();

    // Metrik einstellen
    FieldUnit eFUnit;

    USHORT nWhich = GetWhich( SID_ATTR_METRIC );
    if ( rInAttrs.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
    {
        const SfxUInt16Item& rItem = (SfxUInt16Item&)rInAttrs.Get( nWhich );
        eFUnit = (FieldUnit)rItem.GetValue();
    }
    else
        eFUnit = GetModuleFieldUnit();

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
    aLbMetric.SetSelectHdl( LINK( this, SdTpOptionsMisc, SelectMetricHdl_Impl ) );
}

// -----------------------------------------------------------------------

SdTpOptionsMisc::~SdTpOptionsMisc()
{
}
// -----------------------------------------------------------------------
void SdTpOptionsMisc::ActivatePage( const SfxItemSet& rSet )
{
    // Hier muss noch einmal SaveValue gerufen werden, da sonst u.U.
    // der Wert in anderen TabPages keine Wirkung hat
    aLbMetric.SaveValue();
}

// -----------------------------------------------------------------------

int SdTpOptionsMisc::DeactivatePage( SfxItemSet* pSet )
{
    FillItemSet( *pSet );

    return LEAVE_PAGE;
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

        aOptsItem.SetStartWithTemplate( aCbxStartWithTemplate.IsChecked() );
        aOptsItem.SetMarkedHitMovesAlways( aCbxMarkedHitMovesAlways.IsChecked() );
        aOptsItem.SetCrookNoContortion( aCbxCrookNoContortion.IsChecked() );
        aOptsItem.SetQuickEdit( aCbxQuickEdit.IsChecked() );
        aOptsItem.SetPickThrough( aCbxPickThrough.IsChecked() );
        aOptsItem.SetMasterPagePaintCaching( aCbxMasterPageCache.IsChecked() );
        aOptsItem.SetDragWithCopy( aCbxCopy.IsChecked() );
        aOptsItem.SetStartWithActualPage( aCbxStartWithActualPage.IsChecked() );

        rAttrs.Put( aOptsItem );

        bModified = TRUE;
    }

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

void SdTpOptionsMisc::Reset( const SfxItemSet& rAttrs )
{
    SdOptionsMiscItem aOptsItem( (const SdOptionsMiscItem&) rAttrs.
                        Get( ATTR_OPTIONS_MISC ) );

    aCbxStartWithTemplate.Check( aOptsItem.IsStartWithTemplate() );
    aCbxMarkedHitMovesAlways.Check( aOptsItem.IsMarkedHitMovesAlways() );
    aCbxCrookNoContortion.Check( aOptsItem.IsCrookNoContortion() );
    aCbxQuickEdit.Check( aOptsItem.IsQuickEdit() );
    aCbxPickThrough.Check( aOptsItem.IsPickThrough() );
    aCbxMasterPageCache.Check( aOptsItem.IsMasterPagePaintCaching() );
    aCbxCopy.Check( aOptsItem.IsDragWithCopy() );
    aCbxStartWithActualPage.Check( aOptsItem.IsStartWithActualPage() );
    aCbxStartWithTemplate.SaveValue();
    aCbxMarkedHitMovesAlways.SaveValue();
    aCbxCrookNoContortion.SaveValue();
    aCbxQuickEdit.SaveValue();
    aCbxPickThrough.SaveValue();
    aCbxMasterPageCache.SaveValue();
    aCbxCopy.SaveValue();
    aCbxStartWithActualPage.SaveValue();

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

SfxTabPage* __EXPORT SdTpOptionsMisc::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SdTpOptionsMisc( pWindow, rAttrs ) );
}
//------------------------------------------------------------------------

IMPL_LINK( SdTpOptionsMisc, SelectMetricHdl_Impl, ListBox *, EMPTYARG )
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
/* -----------------------------22.03.01 13:03--------------------------------

 ---------------------------------------------------------------------------*/
void lcl_MoveWin( Window& rWin, long nDiff )
{
    Point aPos(rWin.GetPosPixel());
    aPos.Y() -= nDiff;
    rWin.SetPosPixel(aPos);
}
/* -----------------------------22.03.01 13:10--------------------------------

 ---------------------------------------------------------------------------*/
void    SdTpOptionsMisc::SetDrawMode()
{
    aCbxStartWithTemplate.Hide();
    aGrpProgramStart.Hide();
    aCbxStartWithActualPage.Hide();
    aGrpStartWithActualPage.Hide();
    aCbxCrookNoContortion.Show();

    long nDiff = aGrpOther.GetPosPixel().Y() - aGrpProgramStart.GetPosPixel().Y();
    lcl_MoveWin( aGrpOther, nDiff );
    lcl_MoveWin( aCbxMasterPageCache, nDiff );
    lcl_MoveWin( aCbxCopy, nDiff );
    lcl_MoveWin( aCbxMarkedHitMovesAlways, nDiff );
    lcl_MoveWin( aCbxCrookNoContortion, nDiff );

    nDiff += aGrpSettings.GetPosPixel().Y() - aGrpStartWithActualPage.GetPosPixel().Y();
    nDiff -= aCbxCrookNoContortion.GetPosPixel().Y() - aCbxMarkedHitMovesAlways.GetPosPixel().Y();

    lcl_MoveWin( aGrpSettings, nDiff );
    lcl_MoveWin( aTxtMetric, nDiff );
    lcl_MoveWin( aLbMetric, nDiff );
    lcl_MoveWin( aTxtTabstop, nDiff );
    lcl_MoveWin( aMtrFldTabstop, nDiff );
}



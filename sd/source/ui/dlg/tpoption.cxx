/*************************************************************************
 *
 *  $RCSfile: tpoption.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: os $ $Date: 2001-04-04 11:09:50 $
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

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#include "sdattr.hxx"
#include "sdresid.hxx"
#include "optsitem.hxx"
#include "tpoption.hrc"
#include "tpoption.hxx"
#include "strings.hrc"

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
#define TABLE_COUNT 12
#define TOKEN (sal_Unicode(':'))

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
    aGrpSettings                ( this, SdResId( GRP_SETTINGS ) ),

    aTxtMetric                  ( this, SdResId( FT_METRIC ) ),
    aLbMetric                   ( this, SdResId( LB_METRIC ) ),
    aTxtTabstop                 ( this, SdResId( FT_TABSTOP ) ),
    aMtrFldTabstop              ( this, SdResId( MTR_FLD_TABSTOP ) ),

    aCbxStartWithActualPage     ( this, SdResId( CBX_START_WITH_ACTUAL_PAGE ) ),
    aGrpStartWithActualPage     ( this, SdResId( GRP_START_WITH_ACTUAL_PAGE ) ),
    aTxtCompatibility           ( this, SdResId( FT_COMPATIBILITY ) ),
    aCbxCompatibility           ( this, SdResId( CB_MERGE_PARA_DIST ) ),
    aGrpScale                   ( this, SdResId( GRP_SCALE ) ),
    aFtScale                    ( this, SdResId( FT_SCALE ) ),
    aCbScale                    ( this, SdResId( CB_SCALE ) ),
    aFtOriginal                 ( this, SdResId( FT_ORIGINAL ) ),
    aFtEquivalent               ( this, SdResId( FT_EQUIVALENT ) ),
    aFtPageWidth                ( this, SdResId( FT_PAGEWIDTH ) ),
    aFiInfo1                    ( this, SdResId( FI_INFO_1 ) ),
    aMtrFldOriginalWidth        ( this, SdResId( MTR_FLD_ORIGINAL_WIDTH ) ),
    aFtPageHeight               ( this, SdResId( FT_PAGEHEIGHT ) ),
    aFiInfo2                    ( this, SdResId( FI_INFO_2 ) ),
    aMtrFldOriginalHeight       ( this, SdResId( MTR_FLD_ORIGINAL_HEIGHT ) ),
    aMtrFldInfo1                ( this, WinBits( WB_HIDE ) ),
    aMtrFldInfo2                ( this, WinBits( WB_HIDE ) )
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

    SetFieldUnit( aMtrFldOriginalWidth, eFUnit );
    SetFieldUnit( aMtrFldOriginalHeight, eFUnit );
    aMtrFldOriginalWidth.SetLast( 999999999 );
    aMtrFldOriginalWidth.SetMax( 999999999 );
    aMtrFldOriginalHeight.SetLast( 999999999 );
    aMtrFldOriginalHeight.SetMax( 999999999 );

    // Temporaere Fields fuer Info-Texte (fuer Formatierung/Berechnung)
    aMtrFldInfo1.SetUnit( eFUnit );
    aMtrFldInfo1.SetMax( 999999999 );
    aMtrFldInfo1.SetDecimalDigits( 2 );
    aMtrFldInfo2.SetUnit( eFUnit );
    aMtrFldInfo2.SetMax( 999999999 );
    aMtrFldInfo2.SetDecimalDigits( 2 );

    // PoolUnit ermitteln
    SfxItemPool* pPool = rInAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool?" );
    ePoolUnit = pPool->GetMetric( SID_ATTR_FILL_HATCH );

    // Fuellen der CB
    USHORT aTable[ TABLE_COUNT ] =
        { 1, 2, 4, 5, 8, 10, 16, 20, 30, 40, 50, 100 };

    for( i = 0; i < TABLE_COUNT; i++ )
        aCbScale.InsertEntry( GetScale( 1, aTable[i] ) );
    for( i = 1; i < TABLE_COUNT; i++ )
        aCbScale.InsertEntry( GetScale(  aTable[i], 1 ) );
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
    // Metrik ggfs. aendern (da TabPage im Dialog liegt,
    // wo die Metrik eingestellt werden kann
    const SfxPoolItem* pAttr = NULL;
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_METRIC , FALSE,
                                    (const SfxPoolItem**)&pAttr ))
    {
        const SfxUInt16Item* pItem = (SfxUInt16Item*) pAttr;

        FieldUnit eFUnit = (FieldUnit)(long)pItem->GetValue();

        if( eFUnit != aMtrFldOriginalWidth.GetUnit() )
        {
            // Metriken einstellen
            long nVal = aMtrFldOriginalWidth.Denormalize( aMtrFldOriginalWidth.GetValue( FUNIT_TWIP ) );
            SetFieldUnit( aMtrFldOriginalWidth, eFUnit, TRUE );
            aMtrFldOriginalWidth.SetValue( aMtrFldOriginalWidth.Normalize( nVal ), FUNIT_TWIP );

            nVal = aMtrFldOriginalHeight.Denormalize( aMtrFldOriginalHeight.GetValue( FUNIT_TWIP ) );
            SetFieldUnit( aMtrFldOriginalHeight, eFUnit, TRUE );
            aMtrFldOriginalHeight.SetValue( aMtrFldOriginalHeight.Normalize( nVal ), FUNIT_TWIP );


            if( nWidth != 0 && nHeight != 0 )
            {
                aMtrFldInfo1.SetUnit( eFUnit );
                aMtrFldInfo2.SetUnit( eFUnit );

                SetMetricValue( aMtrFldInfo1, nWidth, ePoolUnit );
                aInfo1 = aMtrFldInfo1.GetText();
                aFiInfo1.SetText( aInfo1 );

                SetMetricValue( aMtrFldInfo2, nHeight, ePoolUnit );
                aInfo2 = aMtrFldInfo2.GetText();
                aFiInfo2.SetText( aInfo2 );
            }
        }
    }
}

// -----------------------------------------------------------------------

int SdTpOptionsMisc::DeactivatePage( SfxItemSet* pSet )
{
    // Parsercheck
    INT32 nX, nY;
    if( SetScale( aCbScale.GetText(), nX, nY ) )
    {
        FillItemSet( *pSet );
        return( LEAVE_PAGE );
    }
    WarningBox aWarnBox( GetParent(), WB_YES_NO, String( SdResId( STR_WARN_SCALE_FAIL ) ) );
    short nReturn = aWarnBox.Execute();

    if( nReturn == RET_YES )
        return( KEEP_PAGE );

    FillItemSet( *pSet );

    return( LEAVE_PAGE );
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
        aCbxStartWithActualPage.GetSavedValue() != aCbxStartWithActualPage.IsChecked() ||
        aCbxCompatibility.GetSavedValue()       != aCbxCompatibility.GetSavedValue() )
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
#if SUPD>627
        aOptsItem.SetSummationOfParagraphs( aCbxCompatibility.IsChecked() );
#endif

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

    INT32 nX, nY;
    if( SetScale( aCbScale.GetText(), nX, nY ) )
    {
        rAttrs.Put( SfxInt32Item( ATTR_OPTIONS_SCALE_X, nX ) );
        rAttrs.Put( SfxInt32Item( ATTR_OPTIONS_SCALE_Y, nY ) );

        bModified = TRUE;
    }
    return( bModified );

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
#if SUPD>627
    aCbxCompatibility.Check( aOptsItem.IsSummationOfParagraphs() );
#endif
    aCbxStartWithTemplate.SaveValue();
    aCbxMarkedHitMovesAlways.SaveValue();
    aCbxCrookNoContortion.SaveValue();
    aCbxQuickEdit.SaveValue();
    aCbxPickThrough.SaveValue();
    aCbxMasterPageCache.SaveValue();
    aCbxCopy.SaveValue();
    aCbxStartWithActualPage.SaveValue();
    aCbxCompatibility.SaveValue();

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
    //Scale
    INT32 nX = ( (const SfxInt32Item&) rAttrs.
                 Get( ATTR_OPTIONS_SCALE_X ) ).GetValue();
    INT32 nY = ( (const SfxInt32Item&) rAttrs.
                 Get( ATTR_OPTIONS_SCALE_Y ) ).GetValue();
    nWidth = ( (const SfxUInt32Item&) rAttrs.
                    Get( ATTR_OPTIONS_SCALE_WIDTH ) ).GetValue();
    nHeight = ( (const SfxUInt32Item&) rAttrs.
                    Get( ATTR_OPTIONS_SCALE_HEIGHT ) ).GetValue();

    aCbScale.SetText( GetScale( nX, nY ) );

    if( nWidth == 0 || nHeight == 0 )
    {
        aFtOriginal.Hide();
        aFtEquivalent.Hide();
        aMtrFldOriginalWidth.Hide();
        aMtrFldOriginalWidth.SetText( aInfo1 ); // leer
        aMtrFldOriginalHeight.Hide();
        aMtrFldOriginalHeight.SetText( aInfo2 ); //leer
        aFtPageWidth.Hide();
        aFtPageHeight.Hide();
        aFiInfo1.Hide();
        aFiInfo2.Hide();
    }
    else
    {
        // Links setzen
        aCbScale.SetModifyHdl( LINK( this, SdTpOptionsMisc, ModifyScaleHdl ) );
        aCbScale.SetSelectHdl( LINK( this, SdTpOptionsMisc, ModifyScaleHdl ) );
        aMtrFldOriginalWidth.SetModifyHdl( LINK( this, SdTpOptionsMisc, ModifyOriginalScaleHdl ) );
        aMtrFldOriginalHeight.SetModifyHdl( LINK( this, SdTpOptionsMisc, ModifyOriginalScaleHdl ) );

        // Hier werden die MetricFields zur Hilfe genommen, um
        // die Seiteninformation richtig auszugeben.
        // Die MetricFields werden erst im ModifyScaleHdl() richtig gesetzt.
        SetMetricValue( aMtrFldInfo1, nWidth, ePoolUnit );
        aInfo1 = aMtrFldInfo1.GetText();
        SetMetricValue( aMtrFldInfo2, nHeight, ePoolUnit );
        aInfo2 = aMtrFldInfo2.GetText();

        aFiInfo1.SetText( aInfo1 );
        aFiInfo2.SetText( aInfo2 );

        ModifyScaleHdl( NULL );
    }

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
    aCbxCompatibility.Hide();
    aTxtCompatibility.Hide();
    aGrpStartWithActualPage.Hide();
    aCbxCrookNoContortion.Show();

    aGrpScale.Show();
    aFtScale.Show();
    aCbScale.Show();

    aFtOriginal.Show();
    aFtEquivalent.Show();

    aFtPageWidth.Show();
    aFiInfo1.Show();
    aMtrFldOriginalWidth.Show();

    aFtPageHeight.Show();
    aFiInfo2.Show();
    aMtrFldOriginalHeight.Show();

    long nDiff = aGrpSettings.GetPosPixel().Y() - aGrpProgramStart.GetPosPixel().Y();
    lcl_MoveWin( aGrpSettings, nDiff );
    lcl_MoveWin( aCbxMasterPageCache, nDiff );
    lcl_MoveWin( aCbxCopy, nDiff );
    lcl_MoveWin( aCbxMarkedHitMovesAlways, nDiff );
    lcl_MoveWin( aCbxCrookNoContortion, nDiff );
    lcl_MoveWin( aTxtMetric, nDiff );
    lcl_MoveWin( aLbMetric, nDiff );
    lcl_MoveWin( aTxtTabstop, nDiff );
    lcl_MoveWin( aMtrFldTabstop, nDiff );
}
// -----------------------------------------------------------------------

IMPL_LINK( SdTpOptionsMisc, ModifyScaleHdl, void *, p )
{
    // Originalgroesse berechnen
    INT32 nX, nY;
    if( SetScale( aCbScale.GetText(), nX, nY ) )
    {
        INT32 nW = nWidth * nY / nX;
        INT32 nH = nHeight * nY / nX;

        SetMetricValue( aMtrFldOriginalWidth, nW, ePoolUnit );
        SetMetricValue( aMtrFldOriginalHeight, nH, ePoolUnit );
    }

    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK( SdTpOptionsMisc, ModifyOriginalScaleHdl, void *, p )
{
    // Berechnen des Massstabs
    long nOrgW = aMtrFldOriginalWidth.GetValue();
    long nOrgH = aMtrFldOriginalHeight.GetValue();

    if( nOrgW == 0 || nOrgH == 0 )
        return( 0L );

    Fraction aFract1( nOrgW, aMtrFldInfo1.GetValue() );
    Fraction aFract2( nOrgH, aMtrFldInfo2.GetValue() );
    Fraction aFract( aFract1 > aFract2 ? aFract1 : aFract2 );

    long nValue;
    if( aFract < Fraction( 1, 1 ) )
    {
        // Fraction umdrehen
        aFract1 = aFract;
        aFract = Fraction( aFract1.GetDenominator(), aFract1.GetNumerator() );
        nValue = aFract;
        aCbScale.SetText( GetScale( 1, nValue ) );
    }
    else
    {
        double fValue = aFract;
        nValue = aFract;
        if( fValue > (double)nValue )
            nValue++;
        aCbScale.SetText( GetScale( nValue, 1 ) );
    }
    return( 0L );
}

// -----------------------------------------------------------------------

String SdTpOptionsMisc::GetScale( INT32 nX, INT32 nY )
{
    String aScale( UniString::CreateFromInt32( nX ) );
    aScale.Append( TOKEN );
    aScale.Append( UniString::CreateFromInt32( nY ) );

    return( aScale );
}

// -----------------------------------------------------------------------

BOOL SdTpOptionsMisc::SetScale( const String& aScale, INT32& rX, INT32& rY )
{
    if( aScale.GetTokenCount( TOKEN ) != 2 )
        return( FALSE );

    ByteString aTmp( aScale.GetToken( 0, TOKEN ), RTL_TEXTENCODING_ASCII_US );
    if( !aTmp.IsNumericAscii() )
        return( FALSE );

    rX = (long) aTmp.ToInt32();
    if( rX == 0 )
        return( FALSE );

    aTmp = ByteString( aScale.GetToken( 1, TOKEN ), RTL_TEXTENCODING_ASCII_US );
    if( !aTmp.IsNumericAscii() )
        return( FALSE );

    rY = (long) aTmp.ToInt32();
    if( rY == 0 )
        return( FALSE );

    return( TRUE );
}



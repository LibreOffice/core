/*************************************************************************
 *
 *  $RCSfile: align.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pb $ $Date: 2000-10-09 11:39:37 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
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

#include <svxenum.hxx>
#pragma hdrstop

#define _SVX_ALIGN_CXX

#include "dialogs.hrc"
#include "align.hrc"
#include "rotmodit.hxx"

#define ITEMID_HORJUSTIFY       SID_ATTR_ALIGN_HOR_JUSTIFY
#define ITEMID_VERJUSTIFY       SID_ATTR_ALIGN_VER_JUSTIFY
#define ITEMID_ORIENTATION      SID_ATTR_ALIGN_ORIENTATION
#define ITEMID_LINEBREAK        SID_ATTR_ALIGN_LINEBREAK
#define ITEMID_MARGIN           SID_ATTR_ALIGN_MARGIN

#include "algitem.hxx"
#include "align.hxx"
#include "dialmgr.hxx"
#include "dlgutil.hxx"

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXMODULE_HXX
#include <sfx2/module.hxx>
#endif

// static ----------------------------------------------------------------

static USHORT pRanges[] =
{
    SID_ATTR_ALIGN_HOR_JUSTIFY,
    SID_ATTR_ALIGN_LINEBREAK,
    SID_ATTR_ALIGN_INDENT,SID_ATTR_ALIGN_INDENT,
    SID_ATTR_ALIGN_DEGREES,SID_ATTR_ALIGN_DEGREES,
    SID_ATTR_ALIGN_LOCKPOS,SID_ATTR_ALIGN_LOCKPOS,
    0
};

//--------------------------------------------------------------
// class SvxAlignmentTabPage
//--------------------------------------------------------------
// Konstruktor

SvxAlignmentTabPage::SvxAlignmentTabPage( Window* pParent,
                                          const SfxItemSet& rCoreAttrs ) :

    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_ALIGNMENT ), rCoreAttrs ),

    aBtnHorStd      ( this, ResId( BTN_HORSTD ) ),
    aBtnHorLeft     ( this, ResId( BTN_HORLEFT ) ),
    aBtnHorCenter   ( this, ResId( BTN_HORCENTER ) ),
    aBtnHorRight    ( this, ResId( BTN_HORRIGHT ) ),
    aBtnHorBlock    ( this, ResId( BTN_HORBLOCK ) ),
    aIndentFT       ( this, ResId( FT_INDENT ) ),
    aIndentED       ( this, ResId( ED_INDENT ) ),
    aGbHorAlign     ( this, ResId( GB_HORALIGN ) ),

    aBtnVerStd      ( this, ResId( BTN_VERSTD ) ),
    aBtnVerTop      ( this, ResId( BTN_VERTOP ) ),
    aBtnVerMid      ( this, ResId( BTN_VERMID ) ),
    aBtnVerBot      ( this, ResId( BTN_VERBOT ) ),
    aGbVerAlign     ( this, ResId( GB_VERALIGN ) ),

    aWinOrient      ( this,ResId(CTR_DIAL),ResId(BTN_TXTSTACKED),
                        ResId(FT_DEGREES),ResId(NF_ORIENT),ResId(FT_BORDER_LOCK),
                        ResId(CTR_BORDER_LOCK),ResId( GB_ALIGN)),   //@ 12.09.97

    aFtLeftSpace    ( this, ResId( FT_LEFTSPACE ) ),
    aEdLeftSpace    ( this, ResId( ED_LEFTSPACE ) ),
    aFtRightSpace   ( this, ResId( FT_RIGHTSPACE ) ),
    aEdRightSpace   ( this, ResId( ED_RIGHTSPACE ) ),
    aFtTopSpace     ( this, ResId( FT_TOPSPACE ) ),
    aEdTopSpace     ( this, ResId( ED_TOPSPACE ) ),
    aFtBottomSpace  ( this, ResId( FT_BOTTOMSPACE ) ),
    aEdBottomSpace  ( this, ResId( ED_BOTTOMSPACE ) ),
    aGbSpace        ( this, ResId( GB_SPACE ) ),

    aBtnWrap        ( this, ResId( BTN_WRAP ) ),
    aWrapGB         ( this, ResId( GB_WRAP ) )

{
    Link aLink = LINK( this, SvxAlignmentTabPage, HorizontalClickHdl_Impl );
    aBtnHorStd.SetClickHdl( aLink );
    aBtnHorLeft.SetClickHdl( aLink );
    aBtnHorCenter.SetClickHdl( aLink );
    aBtnHorRight.SetClickHdl( aLink );
    aBtnHorBlock.SetClickHdl( aLink );

    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    // Metrik einstellen
    FieldUnit eFUnit = GetModuleFieldUnit();

    switch ( eFUnit )
    {
        case FUNIT_CM:
        case FUNIT_M:
        case FUNIT_KM:
            eFUnit = FUNIT_MM;
            break;
    }
    SetFieldUnit( aEdLeftSpace, eFUnit );
    SetFieldUnit( aEdRightSpace, eFUnit );
    SetFieldUnit( aEdTopSpace, eFUnit );
    SetFieldUnit( aEdBottomSpace, eFUnit );

    FillForLockMode();
    FreeResource();
}

//------------------------------------------------------------------------

SvxAlignmentTabPage::~SvxAlignmentTabPage()
{
}

// LockMode in WinOrientation initialisieren -----------------------------

void SvxAlignmentTabPage::FillForLockMode()
{
    ImageList   aIlLock( ResId( IL_LOCK_BMPS ) );
    Size        aItemSize = aIlLock.GetImage(IID_BOTTOMLOCK).GetSizePixel();
    Size        aSize;


    aWinOrient.GetVSLockMode().SetColCount( 3 );
    aWinOrient.GetVSLockMode().SetStyle(
        aWinOrient.GetVSLockMode().GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER );
    aSize = aWinOrient.GetVSLockMode().CalcWindowSizePixel( aItemSize );

    aWinOrient.GetVSLockMode().SetSizePixel( aSize );
    //aWinOrient.GetVSLockMode().SetColCount( 5 );
    aWinOrient.GetVSLockMode().InsertItem( 1, aIlLock.GetImage(IID_BOTTOMLOCK),String(ResId(STR_BOTTOMLOCK)));
    aWinOrient.GetVSLockMode().InsertItem( 2, aIlLock.GetImage(IID_TOPLOCK),String(ResId(STR_TOPLOCK)));
    aWinOrient.GetVSLockMode().InsertItem( 3, aIlLock.GetImage(IID_CELLLOCK),String(ResId(STR_CELLLOCK)));
    aWinOrient.GetVSLockMode().SetNoSelection();
    aWinOrient.GetVSLockMode().Show();
}

//------------------------------------------------------------------------

USHORT* SvxAlignmentTabPage::GetRanges()
{
    return pRanges;
}

// -----------------------------------------------------------------------

SfxTabPage* SvxAlignmentTabPage::Create( Window* pParent,
                                         const SfxItemSet& rAttrSet )
{
    return ( new SvxAlignmentTabPage( pParent, rAttrSet ) );
}

// -----------------------------------------------------------------------

void SvxAlignmentTabPage::Reset( const SfxItemSet& rCoreAttrs )
{
    // erstmal alles zur"ucksetzen
    aBtnHorStd.Check( FALSE );
    aBtnHorLeft.Check( FALSE );
    aBtnHorCenter.Check( FALSE );
    aBtnHorRight.Check( FALSE );
    aBtnHorBlock.Check( FALSE );

    aBtnVerStd.Check( FALSE );
    aBtnVerTop.Check( FALSE );
    aBtnVerMid.Check( FALSE );
    aBtnVerBot.Check( FALSE );

    // und dann einstellen
    const SfxPoolItem* pItem =
        GetItem( rCoreAttrs, SID_ATTR_ALIGN_HOR_JUSTIFY );

    if ( pItem )
    {
        switch ( (SvxCellHorJustify)
            ( (const SvxHorJustifyItem*)pItem )->GetValue() )
        {
            case SVX_HOR_JUSTIFY_STANDARD:  aBtnHorStd.Check();     break;
            case SVX_HOR_JUSTIFY_LEFT:      aBtnHorLeft.Check();    break;
            case SVX_HOR_JUSTIFY_CENTER:    aBtnHorCenter.Check();  break;
            case SVX_HOR_JUSTIFY_RIGHT:     aBtnHorRight.Check();   break;
            case SVX_HOR_JUSTIFY_BLOCK:     aBtnHorBlock.Check();   break;
            default:                        aBtnHorStd.Check();
        }
    }
    // else DON'T-KNOW

    pItem = GetItem( rCoreAttrs, SID_ATTR_ALIGN_INDENT );

    if ( pItem )
    {
        // Einzug in Twips -> umrechnen in Point
        USHORT nVal = (USHORT)( (const SfxUInt16Item*)pItem )->GetValue();
        nVal /= 20;
        aIndentED.SetValue( nVal );
    }
    else
        aIndentED.SetText( String() );

    pItem = GetItem( rCoreAttrs, SID_ATTR_ALIGN_VER_JUSTIFY );

    if ( pItem )
    {
        switch ( (SvxCellVerJustify)
                    ( (const SvxVerJustifyItem*)pItem )->GetValue() )
        {
            case SVX_VER_JUSTIFY_STANDARD:  aBtnVerStd.Check(); break;
            case SVX_VER_JUSTIFY_TOP:       aBtnVerTop.Check(); break;
            case SVX_VER_JUSTIFY_CENTER:    aBtnVerMid.Check(); break;
            case SVX_VER_JUSTIFY_BOTTOM:    aBtnVerBot.Check(); break;
            default:                        aBtnVerStd.Check();
        }
    }
    // else DON'T-KNOW

    pItem = GetItem( rCoreAttrs, SID_ATTR_ALIGN_ORIENTATION );

    aWinOrient.SetDegrees(0);
    aWinOrient.SetStackedTxt(FALSE);
    if ( pItem )
    {
        switch ( (SvxCellOrientation)
                    ( (const SvxOrientationItem*)pItem )->GetValue() )
        {
            case SVX_ORIENTATION_STANDARD:
                    pItem = GetItem( rCoreAttrs, SID_ATTR_ALIGN_DEGREES );
                    if(pItem)
                    {
                        aWinOrient.SetDegrees(
                            ((const SfxInt32Item*) pItem)->GetValue()/100);
                    }
                    break;
            case SVX_ORIENTATION_TOPBOTTOM: aWinOrient.SetDegrees(270); break;
            case SVX_ORIENTATION_STACKED:   aWinOrient.SetStackedTxt(TRUE); break;
            case SVX_ORIENTATION_BOTTOMTOP: aWinOrient.SetDegrees(90); break;
            default:                        aWinOrient.SetDegrees(0);
        };
    }

    if (rCoreAttrs.GetItemState(GetWhich(SID_ATTR_ALIGN_LOCKPOS),TRUE) != SFX_ITEM_DONTCARE)
    {
        pItem=GetItem( rCoreAttrs,SID_ATTR_ALIGN_LOCKPOS);
        if (pItem)
        {
            aWinOrient.SetRotateMode((SvxRotateMode)
                            ( (const SvxRotateModeItem*)pItem )->GetValue() );
        }
    }

    pItem = GetItem( rCoreAttrs, SID_ATTR_ALIGN_MARGIN );

    if ( pItem )
    {
        const SvxMarginItem* pMarginAttr = (const SvxMarginItem*)pItem;

        aEdLeftSpace.SetValue( aEdLeftSpace.Normalize( (long)
                        (pMarginAttr->GetLeftMargin()) ),
                        FUNIT_TWIP );
        aEdRightSpace.SetValue( aEdRightSpace.Normalize( (long)
                        (pMarginAttr->GetRightMargin()) ),
                        FUNIT_TWIP );
        aEdTopSpace.SetValue( aEdTopSpace.Normalize( (long)
                        (pMarginAttr->GetTopMargin()) ),
                        FUNIT_TWIP );
        aEdBottomSpace.SetValue ( aEdBottomSpace.Normalize( (long)
                        (pMarginAttr->GetBottomMargin()) ),
                        FUNIT_TWIP );
    }
    else // DON'T-KNOW
    {
        aEdLeftSpace.SetText    ( String() );
        aEdRightSpace.SetText   ( String() );
        aEdTopSpace.SetText     ( String() );
        aEdBottomSpace.SetText  ( String() );
    }

    pItem = GetItem( rCoreAttrs, SID_ATTR_ALIGN_LINEBREAK );

    if ( pItem )
    {
        aBtnWrap.EnableTriState( FALSE );
        aBtnWrap.SetState( ( (const SfxBoolItem*)pItem )->GetValue()
                           ? TriState( STATE_CHECK )
                           : TriState( STATE_NOCHECK ) );
    }
    else
    {
        aBtnWrap.EnableTriState();
        aBtnWrap.SetState( TriState( STATE_DONTKNOW ) );
    }

    HorizontalClickHdl_Impl( NULL );

    aBtnHorStd       .SaveValue();  // RadioButton
    aBtnHorLeft      .SaveValue();
    aBtnHorCenter    .SaveValue();
    aBtnHorRight     .SaveValue();
    aBtnHorBlock     .SaveValue();
    aBtnVerStd       .SaveValue();
    aBtnVerTop       .SaveValue();
    aBtnVerMid       .SaveValue();
    aBtnVerBot       .SaveValue();
    aBtnWrap         .SaveValue();  // TriStateButton
}

// -----------------------------------------------------------------------

BOOL SvxAlignmentTabPage::FillItemSet( SfxItemSet& rCoreAttrs )
{
    BOOL                bAttrsChanged   = FALSE;
    const SfxItemSet&   rOldSet         = GetItemSet();
    USHORT              nWhich;
    TriState            eState;
    const SfxPoolItem*  pOld            = 0;

    // Horizontale Ausrichtung
    nWhich = GetWhich( SID_ATTR_ALIGN_HOR_JUSTIFY );
    USHORT nTmp = USHRT_MAX;

    if ( aBtnHorStd.IsChecked() )
        nTmp = SVX_HOR_JUSTIFY_STANDARD;
    else if ( aBtnHorLeft.IsChecked() )
        nTmp = SVX_HOR_JUSTIFY_LEFT;
    else if ( aBtnHorCenter.IsChecked() )
        nTmp = SVX_HOR_JUSTIFY_CENTER;
    else if ( aBtnHorRight.IsChecked() )
        nTmp = SVX_HOR_JUSTIFY_RIGHT;
    else if ( aBtnHorBlock.IsChecked() )
        nTmp = SVX_HOR_JUSTIFY_BLOCK;
    DBG_ASSERT( (USHRT_MAX != nTmp) || !aBtnHorStd.IsEnabled(), "no button checked" );
        // if aBtnHorStd is disabled SetFlags was called with the WBA_NO_HORIZONTAL flag set
        // 67977 - 30.07.99 - FS
    pOld = GetOldItem( rCoreAttrs, SID_ATTR_ALIGN_HOR_JUSTIFY );

    if ( !pOld || ( (const SvxHorJustifyItem*)pOld )->GetValue() != nTmp )
    {
        rCoreAttrs.Put( SvxHorJustifyItem( (SvxCellHorJustify)nTmp, nWhich ) );
        bAttrsChanged |= TRUE;
    }
    else if ( rOldSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_DEFAULT )
        rCoreAttrs.ClearItem( nWhich );

    nWhich = GetWhich( SID_ATTR_ALIGN_INDENT );
    pOld = GetOldItem( rCoreAttrs, SID_ATTR_ALIGN_INDENT );
    nTmp = aIndentED.GetValue() * 20; // Point in twips

    if ( !pOld || ( (const SfxUInt16Item*)pOld )->GetValue() != nTmp )
    {
        rCoreAttrs.Put( SfxUInt16Item( nWhich, nTmp ) );
        bAttrsChanged |= TRUE;
    }
    else if ( rOldSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_DEFAULT )
        rCoreAttrs.ClearItem( nWhich );

    // Vertikale Ausrichtung
    nWhich = GetWhich( SID_ATTR_ALIGN_VER_JUSTIFY );
    nTmp = USHRT_MAX;

    if ( aBtnVerStd.IsChecked() )
        nTmp = SVX_VER_JUSTIFY_STANDARD;
    else if ( aBtnVerTop.IsChecked() )
        nTmp = SVX_VER_JUSTIFY_TOP;
    else if ( aBtnVerMid.IsChecked() )
        nTmp = SVX_VER_JUSTIFY_CENTER;
    else if ( aBtnVerBot.IsChecked() )
        nTmp = SVX_VER_JUSTIFY_BOTTOM;
    DBG_ASSERT( (USHRT_MAX != nTmp) || !aBtnVerStd.IsEnabled(), "no button checked" );
        // if aBtnVerStd is disabled SetFlags was called with the WBA_NO_VERTICAL flag set
        // 67977 - 30.07.99 - FS
    pOld = GetOldItem( rCoreAttrs, SID_ATTR_ALIGN_VER_JUSTIFY );

    if ( !pOld || ( (const SvxVerJustifyItem*)pOld )->GetValue() != nTmp )
    {
        rCoreAttrs.Put( SvxVerJustifyItem( (SvxCellVerJustify)nTmp, nWhich ) );
        bAttrsChanged |= TRUE;
    }
    else if ( rOldSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_DEFAULT )
        rCoreAttrs.ClearItem( nWhich );

    // Schreibrichtung
    nWhich = GetWhich( SID_ATTR_ALIGN_ORIENTATION );

    nTmp = SVX_ORIENTATION_STANDARD;

    if ( aWinOrient.IsStackedTxt() )
        nTmp = SVX_ORIENTATION_STACKED;
    else if ( aWinOrient.GetDegrees()==90 )
        nTmp = SVX_ORIENTATION_BOTTOMTOP;
    else if ( aWinOrient.GetDegrees()==270 )
        nTmp = SVX_ORIENTATION_TOPBOTTOM;

    pOld = GetOldItem( rCoreAttrs, SID_ATTR_ALIGN_ORIENTATION );

    if ( !pOld || ( (const SvxOrientationItem*)pOld )->GetValue() != nTmp )
    {
        rCoreAttrs.Put(
            SvxOrientationItem( (SvxCellOrientation)nTmp, nWhich ) );
        bAttrsChanged |= TRUE;
    }
    else if ( rOldSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_DEFAULT )
        rCoreAttrs.ClearItem( nWhich );

    // Schreibwinkel
    nWhich = GetWhich( SID_ATTR_ALIGN_DEGREES );

    long nDegrees=aWinOrient.GetDegrees()*100L;

    pOld = GetOldItem( rCoreAttrs, SID_ATTR_ALIGN_DEGREES );

    if ( !pOld || ( (const SfxInt32Item*)pOld )->GetValue() != nDegrees )
    {
        rCoreAttrs.Put(SfxInt32Item( nWhich,nDegrees) );
        bAttrsChanged |= TRUE;
    }
    else if ( rOldSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_DEFAULT )
        rCoreAttrs.ClearItem( nWhich );

    // Bezugskante

    nWhich = GetWhich( SID_ATTR_ALIGN_LOCKPOS );

    SvxRotateMode nRotMode=aWinOrient.GetRotateMode();

    pOld = GetOldItem( rCoreAttrs, SID_ATTR_ALIGN_LOCKPOS );

    if(aWinOrient.IsSelRotateMode())
    {
        if ( !pOld ||
            ((SvxRotateMode)((const SvxRotateModeItem*)pOld )->GetValue()) != nRotMode )
        {
            rCoreAttrs.Put(SvxRotateModeItem( nRotMode,nWhich) );
            bAttrsChanged |= TRUE;
        }
        else if ( rOldSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_DEFAULT )
            rCoreAttrs.ClearItem( nWhich );
    }

    // Abst"ande zum Zellinhalt
    nWhich = GetWhich( SID_ATTR_ALIGN_MARGIN );
    SvxMarginItem aMargin(
        (INT16)aEdLeftSpace.Denormalize(
            aEdLeftSpace.GetValue( FUNIT_TWIP ) ),
        (INT16)aEdTopSpace.Denormalize(
            aEdTopSpace.GetValue( FUNIT_TWIP ) ),
        (INT16)aEdRightSpace.Denormalize(
            aEdRightSpace.GetValue( FUNIT_TWIP ) ),
        (INT16)aEdBottomSpace.Denormalize(
            aEdBottomSpace.GetValue( FUNIT_TWIP ) ),
        nWhich );
    pOld = GetOldItem( rCoreAttrs, SID_ATTR_ALIGN_MARGIN );

    if ( !pOld || !( *(const SvxMarginItem*)pOld == aMargin ) )
    {
        rCoreAttrs.Put( aMargin );
        bAttrsChanged |= TRUE;
    }
    else if ( rOldSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_DEFAULT )
        rCoreAttrs.ClearItem( nWhich );

    // Zeilenumbruch
    nWhich = GetWhich( SID_ATTR_ALIGN_LINEBREAK );
    eState = aBtnWrap.GetState();
    pOld = GetOldItem( rCoreAttrs, SID_ATTR_ALIGN_LINEBREAK );

    if ( !pOld || ( (const SfxBoolItem*)pOld )->GetValue()
                  != ( eState == STATE_CHECK ) )
    {
        rCoreAttrs.Put( SfxBoolItem( nWhich, (eState == STATE_CHECK) ) );
        bAttrsChanged = TRUE;
    }
    else if ( rOldSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_DEFAULT )
        rCoreAttrs.ClearItem( nWhich );
    return bAttrsChanged;
}

//------------------------------------------------------------------------

int SvxAlignmentTabPage::DeactivatePage( SfxItemSet* pSet )
{
    if ( pSet )
        FillItemSet( *pSet );
    return LEAVE_PAGE;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAlignmentTabPage, HorizontalClickHdl_Impl, RadioButton *, EMPTYARG )
{
    BOOL bChecked = aBtnHorLeft.IsChecked();
    aIndentFT.Enable( bChecked );
    aIndentED.Enable( bChecked );
    return 0;
}

//------------------------------------------------------------------------

void SvxAlignmentTabPage::SetFlags( USHORT nFlags )

/*  [Beschreibung]

    Mit dieser Methode k"onnen einzelne Gruppen diabled werden.
*/

{
    if ( ( nFlags & WBA_NO_ORIENTATION ) == WBA_NO_ORIENTATION )
    {
        aWinOrient.Disable();
    }

    if ( ( nFlags & WBA_NO_LINEBREAK ) == WBA_NO_LINEBREAK )
    {
        aBtnWrap.Disable();
        aWrapGB.Disable();
    }

    if ( ( nFlags & WBA_NO_HORIZONTAL ) == WBA_NO_HORIZONTAL )
    {
        aBtnHorStd.Disable();
        aBtnHorLeft.Disable();
        aBtnHorRight.Disable();
        aBtnHorCenter.Disable();
        aBtnHorBlock.Disable();
        aIndentFT.Disable();
        aIndentED.Disable();
        aGbHorAlign.Disable();
    }

    if ( ( nFlags & WBA_NO_LEFTINDENT ) == WBA_NO_LEFTINDENT )
    {
        aIndentFT.Hide();
        aIndentED.Hide();
    }

    if ( ( nFlags & WBA_NO_VERTICAL ) == WBA_NO_VERTICAL )
    {
        aBtnVerStd.Disable();
        aBtnVerTop.Disable();
        aBtnVerBot.Disable();
        aBtnVerMid.Disable();
        aGbVerAlign.Disable();
    }

    if ( ( nFlags & WBA_NO_GRIDLINES ) == WBA_NO_GRIDLINES )
    {
        aFtLeftSpace.Disable();
        aEdLeftSpace.Disable();
        aFtRightSpace.Disable();
        aEdRightSpace.Disable();
        aFtTopSpace.Disable();
        aEdTopSpace.Disable();
        aFtBottomSpace.Disable();
        aEdBottomSpace.Disable();
        aGbSpace.Disable();
    }
}



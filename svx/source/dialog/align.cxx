/*************************************************************************
 *
 *  $RCSfile: align.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-28 15:27:09 $
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
#define ITEMID_FRAMEDIR         SID_ATTR_FRAMEDIRECTION

#include "algitem.hxx"
#include "frmdiritem.hxx"
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
#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include <svtools/cjkoptions.hxx>
#endif
#ifndef _SVTOOLS_LANGUAGEOPTIONS_HXX
#include <svtools/languageoptions.hxx>
#endif
#ifndef _SVTOOLS_LOCALRESACCESS_HXX_
#include <svtools/localresaccess.hxx>
#endif

// static ----------------------------------------------------------------

static USHORT pRanges[] =
{
    SID_ATTR_ALIGN_HOR_JUSTIFY,
    SID_ATTR_ALIGN_LINEBREAK,
    SID_ATTR_ALIGN_INDENT,SID_ATTR_ALIGN_INDENT,
    SID_ATTR_ALIGN_DEGREES,SID_ATTR_ALIGN_DEGREES,
    SID_ATTR_ALIGN_LOCKPOS,SID_ATTR_ALIGN_LOCKPOS,
    SID_ATTR_ALIGN_HYPHENATION,SID_ATTR_ALIGN_HYPHENATION,
    SID_ATTR_ALIGN_ASIANVERTICAL,SID_ATTR_ALIGN_ASIANVERTICAL,
    SID_ATTR_FRAMEDIRECTION,SID_ATTR_FRAMEDIRECTION,
    0
};

//--------------------------------------------------------------
// class SvxAlignmentTabPage
//--------------------------------------------------------------
// Konstruktor

SvxAlignmentTabPage::SvxAlignmentTabPage( Window* pParent,
                                          const SfxItemSet& rCoreAttrs ) :

    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_ALIGNMENT ), rCoreAttrs ),

    aFlAlignment    ( this, ResId( FL_ALIGNMENT ) ),
    aFtHorAlign     ( this, ResId( FT_HORALIGN ) ),
    aLbHorAlign     ( this, ResId( LB_HORALIGN ) ),
    aFtIndent       ( this, ResId( FT_INDENT ) ),
    aEdIndent       ( this, ResId( ED_INDENT ) ),
    aFtVerAlign     ( this, ResId( FT_VERALIGN ) ),
    aLbVerAlign     ( this, ResId( LB_VERALIGN ) ),

    aWinOrient      ( this,ResId(CTR_DIAL),ResId(BTN_TXTSTACKED),
                        ResId(FT_DEGREES),ResId(NF_DEGREES),ResId(FT_BORDER_LOCK),
                        ResId(CTR_BORDER_LOCK),ResId(FL_ORIENTATION)),   //@ 12.09.97
    aBtnAsianVert   ( this, ResId( BTN_ASIAN_VERTICAL ) ),

    aFlSpace        ( this, ResId( FL_SPACE ) ),
    aFtLeftSpace    ( this, ResId( FT_LEFTSPACE ) ),
    aEdLeftSpace    ( this, ResId( ED_LEFTSPACE ) ),
    aFtRightSpace   ( this, ResId( FT_RIGHTSPACE ) ),
    aEdRightSpace   ( this, ResId( ED_RIGHTSPACE ) ),
    aFtTopSpace     ( this, ResId( FT_TOPSPACE ) ),
    aEdTopSpace     ( this, ResId( ED_TOPSPACE ) ),
    aFtBottomSpace  ( this, ResId( FT_BOTTOMSPACE ) ),
    aEdBottomSpace  ( this, ResId( ED_BOTTOMSPACE ) ),

    aFlWrap         ( this, ResId( FL_WRAP ) ),
    aBtnWrap        ( this, ResId( BTN_WRAP ) ),
    aBtnHyphen      ( this, ResId( BTN_HYPH ) ),
    aFtTextFlow     ( this, ResId( FT_TEXTFLOW ) ),
    aLbFrameDir     ( this, ResId( LB_FRAMEDIR ) ),

    bHyphenDisabled ( FALSE )
{
    aLbHorAlign.SetSelectHdl( LINK( this, SvxAlignmentTabPage, HorAlignSelectHdl_Impl ) );
    aBtnWrap.SetClickHdl( LINK( this, SvxAlignmentTabPage, WrapClickHdl_Impl ) );

    // Asian vertical mode
    if( SvtCJKOptions().IsVerticalTextEnabled() )
        aWinOrient.SetTxtStackedClickHdl( LINK( this, SvxAlignmentTabPage, TxtStackedClickHdl_Impl ) );
    else
        aBtnAsianVert.Hide();

    // CTL frame direction
    aLbFrameDir.InsertEntryValue( SVX_RESSTR( RID_SVXSTR_FRAMEDIR_LTR ), FRMDIR_HORI_LEFT_TOP );
    aLbFrameDir.InsertEntryValue( SVX_RESSTR( RID_SVXSTR_FRAMEDIR_RTL ), FRMDIR_HORI_RIGHT_TOP );
    aLbFrameDir.InsertEntryValue( SVX_RESSTR( RID_SVXSTR_FRAMEDIR_SUPER ), FRMDIR_ENVIRONMENT );
    if( !SvtLanguageOptions().IsCTLFontEnabled() )
    {
        aFtTextFlow.Hide();
        aLbFrameDir.Hide();
    }

    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    // Metrik einstellen
    FieldUnit eFUnit = GetModuleFieldUnit( &rCoreAttrs );

    switch ( eFUnit )
    {
        //  #103396# the default value (1pt) can't be accurately represented in
        //  inches or pica with two decimals, so point is used instead.
        case FUNIT_PICA:
        case FUNIT_INCH:
        case FUNIT_FOOT:
        case FUNIT_MILE:
            eFUnit = FUNIT_POINT;
            break;

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
    ResId TmpId( GetBackground().GetColor().IsDark()? IL_LOCK_BMPS_HC : IL_LOCK_BMPS );
    ImageList   aIlLock( TmpId );
    Size        aItemSize = aIlLock.GetImage(IID_BOTTOMLOCK).GetSizePixel();
    Size        aSize;

    ValueSet&   rValSet = aWinOrient.GetVSLockMode();

    rValSet.SetColCount( 3 );
    rValSet.SetStyle( rValSet.GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER );
    aSize = rValSet.CalcWindowSizePixel( aItemSize );

    rValSet.SetSizePixel( aSize );
    //aWinOrient.GetVSLockMode().SetColCount( 5 );
    rValSet.InsertItem( 1, aIlLock.GetImage( IID_BOTTOMLOCK ), String( ResId( STR_BOTTOMLOCK ) ) );
    rValSet.InsertItem( 2, aIlLock.GetImage( IID_TOPLOCK ), String( ResId( STR_TOPLOCK ) ) );
    rValSet.InsertItem( 3, aIlLock.GetImage( IID_CELLLOCK ),String( ResId( STR_CELLLOCK ) ) );
    rValSet.SetNoSelection();
    rValSet.Show();
}

//------------------------------------------------------------------------

void SvxAlignmentTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxTabPage::DataChanged( rDCEvt );

    if( ( rDCEvt.GetType() == DATACHANGED_SETTINGS ) && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
    {
        svt::OLocalResourceAccess   aLocalResAcc( SVX_RES( RID_SVXPAGE_ALIGNMENT ), RSC_TABPAGE );
        aWinOrient.GetVSLockMode().Clear();
        FillForLockMode();

        aWinOrient.HandleUpdatedSystemsettings();
    }
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

const SfxPoolItem* SvxAlignmentTabPage::GetUniqueItem( const SfxItemSet& rItemSet, sal_uInt16 nSlotId ) const
{
    return (rItemSet.GetItemState( GetWhich( nSlotId ), TRUE ) >= SFX_ITEM_DEFAULT) ?
        GetItem( rItemSet, nSlotId ) : NULL;
}

void SvxAlignmentTabPage::Reset( const SfxItemSet& rCoreAttrs )
{
    const SfxPoolItem* pItem;

    pItem = GetUniqueItem( rCoreAttrs, SID_ATTR_ALIGN_HOR_JUSTIFY );
    if ( pItem )
    {
        USHORT nPos = ALIGNDLG_HORALIGN_STD;
        switch ( (SvxCellHorJustify)
            ( (const SvxHorJustifyItem*)pItem )->GetValue() )
        {
            case SVX_HOR_JUSTIFY_LEFT:      nPos = ALIGNDLG_HORALIGN_LEFT;      break;
            case SVX_HOR_JUSTIFY_CENTER:    nPos = ALIGNDLG_HORALIGN_CENTER;    break;
            case SVX_HOR_JUSTIFY_RIGHT:     nPos = ALIGNDLG_HORALIGN_RIGHT;     break;
            case SVX_HOR_JUSTIFY_BLOCK:     nPos = ALIGNDLG_HORALIGN_BLOCK;     break;
        }
        aLbHorAlign.SelectEntryPos( nPos );
    }
    else
        aLbHorAlign.SetNoSelection();

    pItem = GetUniqueItem( rCoreAttrs, SID_ATTR_ALIGN_INDENT );
    if ( pItem )
    {
        // Einzug in Twips -> umrechnen in Point
        USHORT nVal = (USHORT)( (const SfxUInt16Item*)pItem )->GetValue();
        nVal /= 20;
        aEdIndent.SetValue( nVal );
    }
    else
        aEdIndent.SetText( String() );

    pItem = GetUniqueItem( rCoreAttrs, SID_ATTR_ALIGN_VER_JUSTIFY );
    if ( pItem )
    {
        USHORT nPos = ALIGNDLG_VERALIGN_STD;
        switch ( (SvxCellVerJustify)
                    ( (const SvxVerJustifyItem*)pItem )->GetValue() )
        {
            case SVX_VER_JUSTIFY_TOP:       nPos = ALIGNDLG_VERALIGN_TOP;       break;
            case SVX_VER_JUSTIFY_CENTER:    nPos = ALIGNDLG_VERALIGN_MID;       break;
            case SVX_VER_JUSTIFY_BOTTOM:    nPos = ALIGNDLG_VERALIGN_BOTTOM;    break;
        }
        aLbVerAlign.SelectEntryPos( nPos );
    }
    else
        aLbVerAlign.SetNoSelection();

    pItem = GetUniqueItem( rCoreAttrs, SID_ATTR_ALIGN_ORIENTATION );
    aWinOrient.SetNoDegrees();
    if ( pItem )
    {
        switch ( (SvxCellOrientation)
                    ( (const SvxOrientationItem*)pItem )->GetValue() )
        {
            case SVX_ORIENTATION_STANDARD:
                    pItem = GetUniqueItem( rCoreAttrs, SID_ATTR_ALIGN_DEGREES );
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

    pItem = GetUniqueItem( rCoreAttrs, SID_ATTR_ALIGN_LOCKPOS );
    if( pItem )
    {
        aWinOrient.SetRotateMode((SvxRotateMode)
                        ( (const SvxRotateModeItem*)pItem )->GetValue() );
    }

    pItem = GetUniqueItem( rCoreAttrs, SID_ATTR_ALIGN_ASIANVERTICAL );
    if( pItem )
    {
        aBtnAsianVert.EnableTriState( FALSE );
        aBtnAsianVert.SetState( TriState(
            ((const SfxBoolItem*)pItem)->GetValue() ? STATE_CHECK : STATE_NOCHECK ) );
    }
    else
    {
        aBtnAsianVert.EnableTriState();
        aBtnAsianVert.SetState( TriState( STATE_DONTKNOW ) );
    }

    pItem = GetUniqueItem( rCoreAttrs, SID_ATTR_ALIGN_MARGIN );
    if( pItem )
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

    pItem = GetUniqueItem( rCoreAttrs, SID_ATTR_ALIGN_LINEBREAK );
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

    if (rCoreAttrs.GetItemState(GetWhich(SID_ATTR_ALIGN_HYPHENATION),TRUE) == SFX_ITEM_UNKNOWN)
        bHyphenDisabled = TRUE;
    else
    {
        pItem = GetUniqueItem( rCoreAttrs, SID_ATTR_ALIGN_HYPHENATION );
        if ( pItem )
        {
            aBtnHyphen.EnableTriState( FALSE );
            aBtnHyphen.SetState( ( (const SfxBoolItem*)pItem )->GetValue()
                               ? TriState( STATE_CHECK )
                               : TriState( STATE_NOCHECK ) );
        }
        else
        {
            aBtnHyphen.EnableTriState();
            aBtnHyphen.SetState( TriState( STATE_DONTKNOW ) );
        }
    }

    pItem = GetUniqueItem( rCoreAttrs, SID_ATTR_FRAMEDIRECTION );
    if( pItem )
    {
        SvxFrameDirection eDir = (SvxFrameDirection)((const SvxFrameDirectionItem*)pItem)->GetValue();
        aLbFrameDir.SelectEntryValue( eDir );
    }
    else
        aLbFrameDir.SetNoSelection();

    HorAlignSelectHdl_Impl( NULL );

    aBtnWrap.SaveValue();  // TriStateButton
}

// -----------------------------------------------------------------------

BOOL SvxAlignmentTabPage::FillItemSet( SfxItemSet& rCoreAttrs )
{
    BOOL                bAttrsChanged   = FALSE;
    const SfxItemSet&   rOldSet         = GetItemSet();
    USHORT              nWhich;
    TriState            eState;
    const SfxPoolItem*  pOld            = 0;
    bool                bSelected       = false;

    // Horizontale Ausrichtung
    nWhich = GetWhich( SID_ATTR_ALIGN_HOR_JUSTIFY );
    USHORT nTmp = USHRT_MAX;

    switch( aLbHorAlign.GetSelectEntryPos() )
    {
        case ALIGNDLG_HORALIGN_STD:     nTmp = SVX_HOR_JUSTIFY_STANDARD;    break;
        case ALIGNDLG_HORALIGN_LEFT:    nTmp = SVX_HOR_JUSTIFY_LEFT;        break;
        case ALIGNDLG_HORALIGN_CENTER:  nTmp = SVX_HOR_JUSTIFY_CENTER;      break;
        case ALIGNDLG_HORALIGN_RIGHT:   nTmp = SVX_HOR_JUSTIFY_RIGHT;       break;
        case ALIGNDLG_HORALIGN_BLOCK:   nTmp = SVX_HOR_JUSTIFY_BLOCK;       break;
    }
    bSelected = aLbHorAlign.GetSelectEntryCount() > 0;
    pOld = GetUniqueItem( rOldSet, SID_ATTR_ALIGN_HOR_JUSTIFY );

    DBG_ASSERT( !bSelected || (USHRT_MAX != nTmp) || !aLbHorAlign.IsEnabled(), "no button checked" );
        // if aLbHorAlign is disabled SetFlags was called with the WBA_NO_HORIZONTAL flag set
        // 67977 - 30.07.99 - FS

    if ( bSelected && (!pOld || (((const SvxHorJustifyItem*)pOld)->GetValue() != nTmp)) )
    {
        rCoreAttrs.Put( SvxHorJustifyItem( (SvxCellHorJustify)nTmp, nWhich ) );
        bAttrsChanged |= TRUE;
    }
    else if ( rOldSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_DEFAULT )
        rCoreAttrs.ClearItem( nWhich );

    nWhich = GetWhich( SID_ATTR_ALIGN_INDENT );
    pOld = GetOldItem( rCoreAttrs, SID_ATTR_ALIGN_INDENT );
    nTmp = aEdIndent.GetValue() * 20; // Point in twips

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

    switch( aLbVerAlign.GetSelectEntryPos() )
    {
        case ALIGNDLG_VERALIGN_STD:     nTmp = SVX_VER_JUSTIFY_STANDARD;    break;
        case ALIGNDLG_VERALIGN_TOP:     nTmp = SVX_VER_JUSTIFY_TOP;         break;
        case ALIGNDLG_VERALIGN_MID:     nTmp = SVX_VER_JUSTIFY_CENTER;      break;
        case ALIGNDLG_VERALIGN_BOTTOM:  nTmp = SVX_VER_JUSTIFY_BOTTOM;      break;
    }
    bSelected = aLbVerAlign.GetSelectEntryCount() > 0;
    pOld = GetUniqueItem( rOldSet, SID_ATTR_ALIGN_VER_JUSTIFY );

    DBG_ASSERT( !bSelected || (USHRT_MAX != nTmp) || !aLbVerAlign.IsEnabled(), "no button checked" );
        // if aLbVerAlign is disabled SetFlags was called with the WBA_NO_VERTICAL flag set
        // 67977 - 30.07.99 - FS

    if ( bSelected && (!pOld || (((const SvxVerJustifyItem*)pOld )->GetValue() != nTmp)) )
    {
        rCoreAttrs.Put( SvxVerJustifyItem( (SvxCellVerJustify)nTmp, nWhich ) );
        bAttrsChanged |= TRUE;
    }
    else if ( rOldSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_DEFAULT )
        rCoreAttrs.ClearItem( nWhich );

    if ( aWinOrient.HasDegrees() )
    {
        // Schreibrichtung
        nWhich = GetWhich( SID_ATTR_ALIGN_ORIENTATION );

        nTmp = SVX_ORIENTATION_STANDARD;

        if ( aWinOrient.IsStackedTxt() )
            nTmp = SVX_ORIENTATION_STACKED;
        else if ( aWinOrient.GetDegrees()==90 )
            nTmp = SVX_ORIENTATION_BOTTOMTOP;
        else if ( aWinOrient.GetDegrees()==270 )
            nTmp = SVX_ORIENTATION_TOPBOTTOM;

        pOld = GetUniqueItem( rOldSet, SID_ATTR_ALIGN_ORIENTATION );

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

        pOld = GetUniqueItem( rOldSet, SID_ATTR_ALIGN_DEGREES );

        if ( !pOld || ( (const SfxInt32Item*)pOld )->GetValue() != nDegrees )
        {
            rCoreAttrs.Put(SfxInt32Item( nWhich,nDegrees) );
            bAttrsChanged |= TRUE;
        }
        else if ( rOldSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_DEFAULT )
            rCoreAttrs.ClearItem( nWhich );
    }

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

    // Asian vertical mode
    nWhich = GetWhich( SID_ATTR_ALIGN_ASIANVERTICAL );
    eState = aBtnAsianVert.GetState();
    pOld = GetOldItem( rCoreAttrs, SID_ATTR_ALIGN_ASIANVERTICAL );

    if ( !pOld || (((const SfxBoolItem*)pOld )->GetValue() != (eState == STATE_CHECK)) ||
        (aBtnAsianVert.IsTriStateEnabled() && (eState != STATE_DONTKNOW)) ) // changed from "don't care"
    {
        rCoreAttrs.Put( SfxBoolItem( nWhich, (eState == STATE_CHECK) ) );
        bAttrsChanged = TRUE;
    }
    else if ( rOldSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_DEFAULT )
        rCoreAttrs.ClearItem( nWhich );

    // Abst"ande zum Zellinhalt

    // #106224# all edits empty: do nothing
    if( aEdLeftSpace.GetText().Len() || aEdRightSpace.GetText().Len() ||
        aEdTopSpace.GetText().Len() || aEdBottomSpace.GetText().Len() )
    {
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
    }

    // Zeilenumbruch
    nWhich = GetWhich( SID_ATTR_ALIGN_LINEBREAK );
    eState = aBtnWrap.GetState();
    pOld = GetOldItem( rCoreAttrs, SID_ATTR_ALIGN_LINEBREAK );

    if( !pOld || (((const SfxBoolItem*)pOld )->GetValue() != (eState == STATE_CHECK)) ||
        (aBtnWrap.IsTriStateEnabled() && (eState != STATE_DONTKNOW)) ) // changed from "don't care"
    {
        rCoreAttrs.Put( SfxBoolItem( nWhich, (eState == STATE_CHECK) ) );
        bAttrsChanged = TRUE;
    }
    else if ( rOldSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_DEFAULT )
        rCoreAttrs.ClearItem( nWhich );

    // Hyphenation
    nWhich = GetWhich( SID_ATTR_ALIGN_HYPHENATION );
    eState = aBtnHyphen.GetState();
    pOld = GetOldItem( rCoreAttrs, SID_ATTR_ALIGN_HYPHENATION );

    if ( !pOld || (((const SfxBoolItem*)pOld )->GetValue() != (eState == STATE_CHECK)) ||
        (aBtnHyphen.IsTriStateEnabled() && (eState != STATE_DONTKNOW)) ) // changed from "don't care"
    {
        rCoreAttrs.Put( SfxBoolItem( nWhich, (eState == STATE_CHECK) ) );
        bAttrsChanged = TRUE;
    }
    else if ( rOldSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_DEFAULT )
        rCoreAttrs.ClearItem( nWhich );

    // CTL frame direction
    nWhich = GetWhich( SID_ATTR_FRAMEDIRECTION );
    pOld = GetUniqueItem( rOldSet, SID_ATTR_FRAMEDIRECTION );
    SvxFrameDirection eDir = aLbFrameDir.GetSelectEntryValue();
    bSelected = aLbFrameDir.GetSelectEntryCount() > 0;

    if( bSelected && (!pOld || (((const SvxFrameDirectionItem*)pOld)->GetValue() != eDir)) )
    {
        rCoreAttrs.Put( SvxFrameDirectionItem( eDir, nWhich ) );
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

IMPL_LINK( SvxAlignmentTabPage, HorAlignSelectHdl_Impl, ListBox *, EMPTYARG )
{
    BOOL bChecked = (aLbHorAlign.GetSelectEntryPos() == ALIGNDLG_HORALIGN_LEFT);
    aFtIndent.Enable( bChecked );
    aEdIndent.Enable( bChecked );
    EnableHyphen_Impl();
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAlignmentTabPage, WrapClickHdl_Impl, TriStateBox *, EMPTYARG )
{
    EnableHyphen_Impl();
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( SvxAlignmentTabPage, TxtStackedClickHdl_Impl, void *, EMPTYARG )
{
    aBtnAsianVert.Enable( aWinOrient.IsStackedTxt() );
    return 0;
}

//------------------------------------------------------------------------

void SvxAlignmentTabPage::EnableHyphen_Impl()
{
    BOOL bWrap = (aBtnWrap.GetState() == STATE_CHECK);
    BOOL bBlock = (aLbHorAlign.GetSelectEntryPos() == ALIGNDLG_HORALIGN_BLOCK);
    BOOL bEnable = ( ( bWrap || bBlock ) && !bHyphenDisabled );
    aBtnHyphen.Enable( bEnable );
}

//------------------------------------------------------------------------

void SvxAlignmentTabPage::SetFlags( USHORT nFlags )
{
    /* This method allows to disable specific control groups. */

    if ( ( nFlags & WBA_NO_ORIENTATION ) == WBA_NO_ORIENTATION )
    {
        aWinOrient.Disable();
        aBtnAsianVert.Disable();
    }

    if ( ( nFlags & WBA_NO_LINEBREAK ) == WBA_NO_LINEBREAK )
    {
        aFlWrap.Disable();
        aBtnWrap.Disable();
        aFtTextFlow.Disable();
        aLbFrameDir.Disable();
    }

    if ( nFlags & ( WBA_NO_LINEBREAK | WBA_NO_HYPHENATION ) )
    {
        //  WBA_NO_LINEBREAK also disables hyphenation

        bHyphenDisabled = TRUE;
        aBtnHyphen.Disable();
    }

    if ( ( nFlags & WBA_NO_HORIZONTAL ) == WBA_NO_HORIZONTAL )
    {
        aFtHorAlign.Disable();
        aLbHorAlign.Disable();
        aFtIndent.Disable();
        aEdIndent.Disable();
    }

    if ( ( nFlags & WBA_NO_LEFTINDENT ) == WBA_NO_LEFTINDENT )
    {
        aFtIndent.Hide();
        aEdIndent.Hide();
    }

    if ( ( nFlags & WBA_NO_VERTICAL ) == WBA_NO_VERTICAL )
    {
        aFtVerAlign.Disable();
        aLbVerAlign.Disable();
    }

    if ( ( nFlags & WBA_NO_GRIDLINES ) == WBA_NO_GRIDLINES )
    {
        aFlSpace.Disable();
        aFtLeftSpace.Disable();
        aEdLeftSpace.Disable();
        aFtRightSpace.Disable();
        aEdRightSpace.Disable();
        aFtTopSpace.Disable();
        aEdTopSpace.Disable();
        aFtBottomSpace.Disable();
        aEdBottomSpace.Disable();
    }
}



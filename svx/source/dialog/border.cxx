/*************************************************************************
 *
 *  $RCSfile: border.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:41:22 $
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

#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFXMODULE_HXX
#include <sfx2/module.hxx>
#endif
#pragma hdrstop

#define _SVX_BORDER_CXX

#include "dialogs.hrc"
#include "svxitems.hrc"
#include "border.hrc"
#include "helpid.hrc"

#define ITEMID_COLOR_TABLE  0
#define ITEMID_BOX          SID_ATTR_BORDER_OUTER
#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#define ITEMID_SHADOW       SID_ATTR_BORDER_SHADOW

#define LINESTYLE_HTML_MAX 5

#include "xtable.hxx"               // XColorTable
#include "drawitem.hxx"
#include "boxitem.hxx"
#include "shaditem.hxx"
#include "border.hxx"
#include "dlgutil.hxx"
#include "dialmgr.hxx"
#include "htmlmode.hxx"
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#include "svxids.hrc" //CHINA001
#include "flagsdef.hxx" //CHINA001
#include <sfx2/request.hxx>
#ifndef _SFXINTITEM_HXX //CHINA001
#include <svtools/intitem.hxx> //CHINA001
#endif //CHINA001
#ifndef SFX_ITEMCONNECT_HXX
#include <sfx2/itemconnect.hxx>
#endif

#ifndef SVX_BORDERCONN_HXX
#include "borderconn.hxx"
#endif

// -----------------------------------------------------------------------

/*
 * [Beschreibung:]
 * TabPage zum Einstellen der Rahmen-Attribute.
 * Benoetigt
 *      ein SvxShadowItem: Schatten
 *      ein SvxBoxItem:    Linien links,rechts,oben,unten ),
 *      ein SvxBoxInfo:    Linien vertikal,horizontal, Abstaende, Flags )
 *
 * Linien koennen drei Zustaende haben.
 *      1. Show     ( -> gueltige Werte )
 *      2. Hide     ( -> NULL-Pointer )
 *      3. DontCare ( -> gesonderte Valid-Flags im InfoItem )
 */

// static ----------------------------------------------------------------

static USHORT pRanges[] =
{
    SID_ATTR_BORDER_INNER,      SID_ATTR_BORDER_SHADOW,
    SID_ATTR_ALIGN_MARGIN,      SID_ATTR_ALIGN_MARGIN,
    SID_ATTR_BORDER_CONNECT,    SID_ATTR_BORDER_CONNECT,
    SID_SW_COLLAPSING_BORDERS,  SID_SW_COLLAPSING_BORDERS,
    SID_ATTR_BORDER_DIAG_TLBR,  SID_ATTR_BORDER_DIAG_BLTR,
    0
};

BOOL SvxBorderTabPage::bSync = TRUE;


//------------------------------------------------------------------------

#define LINE_WIDTH0     (DEF_LINE_WIDTH_0 *100)
#define LINE_WIDTH1     (DEF_LINE_WIDTH_1 *100)
#define LINE_WIDTH2     (DEF_LINE_WIDTH_2 *100)
#define LINE_WIDTH3     (DEF_LINE_WIDTH_3 *100)
#define LINE_WIDTH4     (DEF_LINE_WIDTH_4 *100)
#define LINE_WIDTH5     (DEF_LINE_WIDTH_5 *100)

#define DLINE0_OUT      (DEF_DOUBLE_LINE0_OUT  *100)
#define DLINE0_IN       (DEF_DOUBLE_LINE0_IN   *100)
#define DLINE0_DIST     (DEF_DOUBLE_LINE0_DIST *100)

#define DLINE1_OUT      (DEF_DOUBLE_LINE1_OUT  *100)
#define DLINE1_IN       (DEF_DOUBLE_LINE1_IN   *100)
#define DLINE1_DIST     (DEF_DOUBLE_LINE1_DIST *100)

#define DLINE2_OUT      (DEF_DOUBLE_LINE2_OUT  *100)
#define DLINE2_IN       (DEF_DOUBLE_LINE2_IN   *100)
#define DLINE2_DIST     (DEF_DOUBLE_LINE2_DIST *100)

#define DLINE3_OUT      (DEF_DOUBLE_LINE3_OUT  *100)
#define DLINE3_IN       (DEF_DOUBLE_LINE3_IN   *100)
#define DLINE3_DIST     (DEF_DOUBLE_LINE3_DIST *100)

#define DLINE4_OUT      (DEF_DOUBLE_LINE4_OUT  *100)
#define DLINE4_IN       (DEF_DOUBLE_LINE4_IN   *100)
#define DLINE4_DIST     (DEF_DOUBLE_LINE4_DIST *100)

#define DLINE5_OUT      (DEF_DOUBLE_LINE5_OUT  *100)
#define DLINE5_IN       (DEF_DOUBLE_LINE5_IN   *100)
#define DLINE5_DIST     (DEF_DOUBLE_LINE5_DIST *100)

#define DLINE6_OUT      (DEF_DOUBLE_LINE6_OUT  *100)
#define DLINE6_IN       (DEF_DOUBLE_LINE6_IN   *100)
#define DLINE6_DIST     (DEF_DOUBLE_LINE6_DIST *100)

#define DLINE7_OUT      (DEF_DOUBLE_LINE7_OUT  *100)
#define DLINE7_IN       (DEF_DOUBLE_LINE7_IN   *100)
#define DLINE7_DIST     (DEF_DOUBLE_LINE7_DIST *100)

#define DLINE8_OUT      (DEF_DOUBLE_LINE8_OUT  *100)
#define DLINE8_IN       (DEF_DOUBLE_LINE8_IN   *100)
#define DLINE8_DIST     (DEF_DOUBLE_LINE8_DIST *100)

#define DLINE9_OUT      (DEF_DOUBLE_LINE9_OUT  *100)
#define DLINE9_IN       (DEF_DOUBLE_LINE9_IN   *100)
#define DLINE9_DIST     (DEF_DOUBLE_LINE9_DIST *100)

#define DLINE10_OUT     (DEF_DOUBLE_LINE10_OUT *100)
#define DLINE10_IN      (DEF_DOUBLE_LINE10_IN  *100)
#define DLINE10_DIST    (DEF_DOUBLE_LINE10_DIST*100)

#define RGBCOL(eColorName) (TpBorderRGBColor(eColorName))

// LOKALE FUNKTION
// Konvertiert in echte RGB-Farben, damit in den Listboxen
// endlich mal richtig selektiert werden kann.

Color TpBorderRGBColor( ColorData aColorData )
{
    Color aRGBColor( aColorData );

    return( aRGBColor );
}

// -----------------------------------------------------------------------

Color TpBorderRGBColor( const Color& rColor )
{
    Color aRGBColor( rColor.GetRed(),
                     rColor.GetGreen(),
                     rColor.GetBlue() );

    return( aRGBColor );
}

// -----------------------------------------------------------------------

SvxBorderTabPage::SvxBorderTabPage( Window* pParent,
                                    const SfxItemSet& rCoreAttrs )

    :   SfxTabPage( pParent, ResId( RID_SVXPAGE_BORDER, DIALOG_MGR() ), rCoreAttrs ),

        aFlBorder       ( this, ResId( FL_BORDER ) ),
        aDefaultFT      ( this, ResId( FT_DEFAULT ) ),
        aUserDefFT      ( this, ResId( FT_USERDEF ) ),
        aStyleFT        ( this, ResId( FT_STYLE ) ),
        aColorFT        ( this, ResId( FT_COLOR ) ),
        aWndPresets     ( this, ResId( WIN_PRESETS ) ),
        aFrameSel       ( this, ResId( WIN_FRAMESEL ) ),
        aFlSep1         ( this, ResId( FL_SEPARATOR1 ) ),
        aFlLine         ( this, ResId( FL_LINE ) ),
        aLbLineStyle    ( this, ResId( LB_LINESTYLE ) ),
        aLbLineColor    ( this, ResId( LB_LINECOLOR ) ),

        aFlSep2         ( this, ResId( FL_SEPARATOR2 ) ),
        aDistanceFL     ( this, ResId( FL_DISTANCE ) ),
        aLeftFT         ( this, ResId( FT_LEFT ) ),
        aLeftMF         ( this, ResId( MF_LEFT ) ),
        aRightFT        ( this, ResId( FT_RIGHT ) ),
        aRightMF        ( this, ResId( MF_RIGHT ) ),
        aTopFT          ( this, ResId( FT_TOP ) ),
        aTopMF          ( this, ResId( MF_TOP ) ),
        aBottomFT       ( this, ResId( FT_BOTTOM ) ),
        aBottomMF       ( this, ResId( MF_BOTTOM ) ),
        aSynchronizeCB  ( this, ResId( CB_SYNC ) ),

        aFlShadow       ( this, ResId( FL_SHADOW ) ),
        aFtShadowPos    ( this, ResId( FT_SHADOWPOS ) ),
        aWndShadows     ( this, ResId( WIN_SHADOWS ) ),
        aFtShadowSize   ( this, ResId( FT_SHADOWSIZE ) ),
        aEdShadowSize   ( this, ResId( ED_SHADOWSIZE ) ),
        aFtShadowColor  ( this, ResId( FT_SHADOWCOLOR ) ),
        aLbShadowColor  ( this, ResId( LB_SHADOWCOLOR ) ),
        aPropertiesFL   ( this, ResId( FL_PROPERTIES ) ),
        aMergeWithNextCB( this, ResId( CB_MERGEWITHNEXT ) ),
        aMergeAdjacentBordersCB( this, ResId( CB_MERGEADJACENTBORDERS ) ),
        aShadowImgLstH( ResId(ILH_SDW_BITMAPS)),
        aShadowImgLst( ResId(IL_SDW_BITMAPS)),
        aBorderImgLstH( ResId(ILH_PRE_BITMAPS)),
        aBorderImgLst( ResId(IL_PRE_BITMAPS)),
        nMinValue(0),
        nSWMode(0),
        mbHorEnabled( false ),
        mbVerEnabled( false ),
        mbTLBREnabled( false ),
        mbBLTREnabled( false ),
        mbUseMarginItem( false )

{
    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    /*  Use SvxMarginItem instead of margins from SvxBoxItem, if present.
        ->  Remember this state in mbUseMarginItem, because other special handling
            is needed across various functions... */
    mbUseMarginItem = rCoreAttrs.GetItemState(GetWhich(SID_ATTR_ALIGN_MARGIN),TRUE) != SFX_ITEM_UNKNOWN;

    // Metrik einstellen
    FieldUnit eFUnit = GetModuleFieldUnit( &rCoreAttrs );

    if( mbUseMarginItem )
    {
        // copied from SvxAlignmentTabPage
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
    }
    else
    {
        switch ( eFUnit )
        {
            case FUNIT_M:
            case FUNIT_KM:
                eFUnit = FUNIT_MM;
                break;
        }
    }

    SetFieldUnit( aEdShadowSize, eFUnit );

    USHORT nWhich = GetWhich( SID_ATTR_BORDER_INNER );
    BOOL bIsDontCare = TRUE;

    if ( rCoreAttrs.GetItemState( nWhich, TRUE ) >= SFX_ITEM_AVAILABLE )
    {
        // Absatz oder Tabelle
        const SvxBoxInfoItem* pBoxInfo =
            (const SvxBoxInfoItem*)&( rCoreAttrs.Get( nWhich ) );

        mbHorEnabled = pBoxInfo->IsHorEnabled();
        mbVerEnabled = pBoxInfo->IsVerEnabled();
        mbTLBREnabled = sfx::ItemWrapperHelper::IsKnownItem( rCoreAttrs, SID_ATTR_BORDER_DIAG_TLBR );
        mbBLTREnabled = sfx::ItemWrapperHelper::IsKnownItem( rCoreAttrs, SID_ATTR_BORDER_DIAG_BLTR );

        if(pBoxInfo->IsDist())
        {
            SetFieldUnit(aLeftMF, eFUnit);
            SetFieldUnit(aRightMF, eFUnit);
            SetFieldUnit(aTopMF, eFUnit);
            SetFieldUnit(aBottomMF, eFUnit);
            aSynchronizeCB.Check(bSync);
            aSynchronizeCB.SetClickHdl(LINK(this, SvxBorderTabPage, SyncHdl_Impl));
            aLeftMF.SetModifyHdl(LINK(this, SvxBorderTabPage, ModifyDistanceHdl_Impl));
            aRightMF.SetModifyHdl(LINK(this, SvxBorderTabPage, ModifyDistanceHdl_Impl));
            aTopMF.SetModifyHdl(LINK(this, SvxBorderTabPage, ModifyDistanceHdl_Impl));
            aBottomMF.SetModifyHdl(LINK(this, SvxBorderTabPage, ModifyDistanceHdl_Impl));
        }
        else
        {
            aFlSep2.Hide();
            aDistanceFL.Hide();
            aLeftFT.Hide();
            aLeftMF.Hide();
            aRightFT.Hide();
            aRightMF.Hide();
            aTopFT.Hide();
            aTopMF.Hide();
            aBottomFT.Hide();
            aBottomMF.Hide();
            aSynchronizeCB.Hide();
        }
        bIsDontCare = !pBoxInfo->IsValid( VALID_DISABLE );
    }
    if(!mbUseMarginItem && eFUnit == FUNIT_MM && SFX_MAPUNIT_TWIP == rCoreAttrs.GetPool()->GetMetric( GetWhich( SID_ATTR_BORDER_INNER ) ))
    {
        aLeftMF.SetDecimalDigits(1);
        aRightMF.SetDecimalDigits(1);
        aTopMF.SetDecimalDigits(1);
        aBottomMF.SetDecimalDigits(1);
        aEdShadowSize.SetDecimalDigits(1);
    }

    svx::FrameSelFlags nFlags = svx::FRAMESEL_OUTER;
    if( mbHorEnabled )
        nFlags |= svx::FRAMESEL_INNER_HOR;
    if( mbVerEnabled )
        nFlags |= svx::FRAMESEL_INNER_VER;
    if( mbTLBREnabled )
        nFlags |= svx::FRAMESEL_DIAG_TLBR;
    if( mbBLTREnabled )
        nFlags |= svx::FRAMESEL_DIAG_BLTR;
    if( bIsDontCare )
        nFlags |= svx::FRAMESEL_DONTCARE;
    aFrameSel.Initialize( nFlags );

    aFrameSel.SetSelectHdl(LINK(this, SvxBorderTabPage, LinesChanged_Impl));
    aLbLineStyle.SetSelectHdl( LINK( this, SvxBorderTabPage, SelStyleHdl_Impl ) );
    aLbLineColor.SetSelectHdl( LINK( this, SvxBorderTabPage, SelColHdl_Impl ) );
    aLbShadowColor.SetSelectHdl( LINK( this, SvxBorderTabPage, SelColHdl_Impl ) );
    aWndPresets.SetSelectHdl( LINK( this, SvxBorderTabPage, SelPreHdl_Impl ) );
    aWndShadows.SetSelectHdl( LINK( this, SvxBorderTabPage, SelSdwHdl_Impl ) );

    FillValueSets();
    FillLineListBox_Impl();

    // ColorBox aus der XColorTable fuellen.
    SfxObjectShell*     pDocSh      = SfxObjectShell::Current();
    const SfxPoolItem*  pItem       = NULL;
    XColorTable*        pColorTable = NULL;

    DBG_ASSERT( pDocSh, "DocShell not found!" );

    if ( pDocSh && ( pItem = pDocSh->GetItem( SID_COLOR_TABLE ) ) )
        pColorTable = ( (SvxColorTableItem*)pItem )->GetColorTable();

    DBG_ASSERT( pColorTable, "ColorTable not found!" );

    if ( pColorTable )
    {
        // fuellen der Linienfarben-Box
        aLbLineColor.SetUpdateMode( FALSE );

        for ( long i = 0; i < pColorTable->Count(); ++i )
        {
            XColorEntry* pEntry = pColorTable->Get(i);
            aLbLineColor.InsertEntry( pEntry->GetColor(), pEntry->GetName() );
        }
        aLbLineColor.SetUpdateMode( TRUE );
        // dann nur noch in die Schattenfarben-Box kopieren
        aLbShadowColor.CopyEntries( aLbLineColor );
    }
    FreeResource();

    // connections

    AddItemConnection( svx::CreateShadowConnection( rCoreAttrs, aWndShadows, aEdShadowSize, aLbShadowColor ) );
    if( mbUseMarginItem )
        AddItemConnection( svx::CreateMarginConnection( rCoreAttrs, aLeftMF, aRightMF, aTopMF, aBottomMF ) );
    if( aFrameSel.IsBorderEnabled( svx::FRAMEBORDER_TLBR ) )
        AddItemConnection( svx::CreateFrameLineConnection( SID_ATTR_BORDER_DIAG_TLBR, aFrameSel, svx::FRAMEBORDER_TLBR ) );
    if( aFrameSel.IsBorderEnabled( svx::FRAMEBORDER_BLTR ) )
        AddItemConnection( svx::CreateFrameLineConnection( SID_ATTR_BORDER_DIAG_BLTR, aFrameSel, svx::FRAMEBORDER_BLTR ) );
    AddItemConnection( new sfx::CheckBoxConnection( SID_ATTR_BORDER_CONNECT, aMergeWithNextCB, sfx::ITEMCONN_CLONE_ITEM | sfx::ITEMCONN_SHOW_KNOWN ) );
    AddItemConnection( new sfx::CheckBoxConnection( SID_SW_COLLAPSING_BORDERS, aMergeAdjacentBordersCB, sfx::ITEMCONN_CLONE_ITEM | sfx::ITEMCONN_SHOW_KNOWN ) );
    AddItemConnection( new sfx::DummyItemConnection( SID_ATTR_BORDER_CONNECT, aPropertiesFL, sfx::ITEMCONN_SHOW_KNOWN ) );
    AddItemConnection( new sfx::DummyItemConnection( SID_SW_COLLAPSING_BORDERS, aPropertiesFL, sfx::ITEMCONN_SHOW_KNOWN ) );
}

// -----------------------------------------------------------------------

SvxBorderTabPage::~SvxBorderTabPage()
{
}

// -----------------------------------------------------------------------

USHORT* SvxBorderTabPage::GetRanges()
{
    return pRanges;
}

// -----------------------------------------------------------------------

SfxTabPage* SvxBorderTabPage::Create( Window* pParent,
                                      const SfxItemSet& rAttrSet )
{
    return ( new SvxBorderTabPage( pParent, rAttrSet ) );
}

// -----------------------------------------------------------------------

void SvxBorderTabPage::ResetFrameLine_Impl( svx::FrameBorderType eBorder, const SvxBorderLine* pCoreLine, bool bValid )
{
    if( aFrameSel.IsBorderEnabled( eBorder ) )
    {
        if( bValid )
            aFrameSel.ShowBorder( eBorder, pCoreLine );
        else
            aFrameSel.SetBorderDontCare( eBorder );
    }
}

// -----------------------------------------------------------------------

void SvxBorderTabPage::Reset( const SfxItemSet& rSet )
{
    SfxTabPage::Reset( rSet );

    const SvxBoxItem*       pBoxItem;
    const SvxBoxInfoItem*   pBoxInfoItem;
    USHORT                  nWhichBox       = GetWhich(SID_ATTR_BORDER_OUTER);
    SfxMapUnit              eCoreUnit;
    const Color             aColBlack       = RGBCOL(COL_BLACK);

    pBoxItem  = (const SvxBoxItem*)GetItem( rSet, SID_ATTR_BORDER_OUTER );
    pBoxInfoItem = (const SvxBoxInfoItem*)GetItem( rSet, SID_ATTR_BORDER_INNER );

    eCoreUnit = rSet.GetPool()->GetMetric( nWhichBox );

    if ( pBoxItem && pBoxInfoItem ) // -> Don't Care
    {
        ResetFrameLine_Impl( svx::FRAMEBORDER_LEFT,   pBoxItem->GetLeft(),     pBoxInfoItem->IsValid( VALID_LEFT ) );
        ResetFrameLine_Impl( svx::FRAMEBORDER_RIGHT,  pBoxItem->GetRight(),    pBoxInfoItem->IsValid( VALID_RIGHT ) );
        ResetFrameLine_Impl( svx::FRAMEBORDER_TOP,    pBoxItem->GetTop(),      pBoxInfoItem->IsValid( VALID_TOP ) );
        ResetFrameLine_Impl( svx::FRAMEBORDER_BOTTOM, pBoxItem->GetBottom(),   pBoxInfoItem->IsValid( VALID_BOTTOM ) );
        ResetFrameLine_Impl( svx::FRAMEBORDER_VER,    pBoxInfoItem->GetVert(), pBoxInfoItem->IsValid( VALID_VERT ) );
        ResetFrameLine_Impl( svx::FRAMEBORDER_HOR,    pBoxInfoItem->GetHori(), pBoxInfoItem->IsValid( VALID_HORI ) );

        //-------------------
        // Abstand nach innen
        //-------------------
        if( !mbUseMarginItem )
        {
            if ( aLeftMF.IsVisible() )
            {
                SetMetricValue( aLeftMF,    pBoxInfoItem->GetDefDist(), eCoreUnit );
                SetMetricValue( aRightMF,   pBoxInfoItem->GetDefDist(), eCoreUnit );
                SetMetricValue( aTopMF,     pBoxInfoItem->GetDefDist(), eCoreUnit );
                SetMetricValue( aBottomMF,  pBoxInfoItem->GetDefDist(), eCoreUnit );

                nMinValue = aLeftMF.GetValue();

                if ( pBoxInfoItem->IsMinDist() )
                {
                    aLeftMF.SetFirst( nMinValue );
                    aRightMF.SetFirst( nMinValue );
                    aTopMF.SetFirst( nMinValue );
                    aBottomMF.SetFirst( nMinValue );
                }

                if ( pBoxInfoItem->IsDist() )
                {
                    if( rSet.GetItemState( nWhichBox, TRUE ) >= SFX_ITEM_DEFAULT )
                    {
                        BOOL bIsAnyBorderVisible = aFrameSel.IsAnyBorderVisible();
                        if( !bIsAnyBorderVisible || !pBoxInfoItem->IsMinDist() )
                        {
                            aLeftMF.SetMin( 0 );
                            aLeftMF.SetFirst( 0 );
                            aRightMF.SetMin( 0 );
                            aRightMF.SetFirst( 0 );
                            aTopMF.SetMin( 0 );
                            aTopMF.SetFirst( 0 );
                            aBottomMF.SetMin( 0 );
                            aBottomMF.SetFirst( 0 );
                        }
                        long nLeftDist = pBoxItem->GetDistance( BOX_LINE_LEFT);
                        SetMetricValue( aLeftMF, nLeftDist, eCoreUnit );
                        long nRightDist = pBoxItem->GetDistance( BOX_LINE_RIGHT);
                        SetMetricValue( aRightMF, nRightDist, eCoreUnit );
                        long nTopDist = pBoxItem->GetDistance( BOX_LINE_TOP);
                        SetMetricValue( aTopMF, nTopDist, eCoreUnit );
                        long nBottomDist = pBoxItem->GetDistance( BOX_LINE_BOTTOM);
                        SetMetricValue( aBottomMF, nBottomDist, eCoreUnit );

                        // ist der Abstand auf nicht-default gesetzt,
                        // dann soll der Wert auch nicht
                        // mehr autom. veraendert werden

                        // if the distance is set with no active border line
                        // or it is null with an active border line
                        // no automatic changes should be made
                        const long nDefDist = bIsAnyBorderVisible ? pBoxInfoItem->GetDefDist() : 0;
                        BOOL bDiffDist = (nDefDist != nLeftDist ||
                                    nDefDist != nRightDist ||
                                    nDefDist != nTopDist   ||
                                    nDefDist != nBottomDist);
                        if((pBoxItem->GetDistance() ||
                                bIsAnyBorderVisible) && bDiffDist )
                        {
                            aLeftMF.SetModifyFlag();
                            aRightMF.SetModifyFlag();
                            aTopMF.SetModifyFlag();
                            aBottomMF.SetModifyFlag();
                        }
                    }
                    else
                    {
                        // #106224# different margins -> do not fill the edits
                        aLeftMF.SetText( String() );
                        aRightMF.SetText( String() );
                        aTopMF.SetText( String() );
                        aBottomMF.SetText( String() );
                    }
                }
                aLeftMF.SaveValue();
                aRightMF.SaveValue();
                aTopMF.SaveValue();
                aBottomMF.SaveValue();
            }
        }
    }
    else
    {
        // ResetFrameLine-Aufrufe einsparen:
        aFrameSel.HideAllBorders();
    }

    //-------------------------------------------------------------
    // Linie/Linienfarbe in Controllern darstellen, wenn eindeutig:
    //-------------------------------------------------------------
    {
        // Do all visible lines show the same line widths?
        USHORT nPrim, nDist, nSecn;
        bool bWidthEq = aFrameSel.GetVisibleWidth( nPrim, nDist, nSecn );
        if( bWidthEq )
            aLbLineStyle.SelectEntry( nPrim * 100, nSecn * 100, nDist * 100 );
        else
            aLbLineStyle.SelectEntryPos( 1 );

        // Do all visible lines show the same line color?
        Color aColor;
        bool bColorEq = aFrameSel.GetVisibleColor( aColor );
        if( !bColorEq )
            aColor.SetColor( COL_BLACK );

        USHORT nSelPos = aLbLineColor.GetEntryPos( aColor );
        if( nSelPos == LISTBOX_ENTRY_NOTFOUND )
            nSelPos = aLbLineColor.InsertEntry( aColor, SVX_RESSTR( RID_SVXSTR_COLOR_USER ) );

        aLbLineColor.SelectEntryPos( nSelPos );
        aLbLineStyle.SetColor( aColor );

        // Select all visible lines, if they are all equal.
        if( bWidthEq && bColorEq )
            aFrameSel.SelectAllVisibleBorders();

        // set the current style and color (caches style in control even if nothing is selected)
        SelStyleHdl_Impl( &aLbLineStyle );
        SelColHdl_Impl( &aLbLineColor );
    }

    BOOL bEnable = aWndShadows.GetSelectItemId() > 1 ;
    aFtShadowSize.Enable(bEnable);
    aEdShadowSize.Enable(bEnable);
    aFtShadowColor.Enable(bEnable);
    aLbShadowColor.Enable(bEnable);

    aWndPresets.SetNoSelection();

    // - keine Line - sollte nicht selektiert sein

    if ( aLbLineStyle.GetSelectEntryPos() == 0 )
    {
        aLbLineStyle.SelectEntryPos( 1 );
        SelStyleHdl_Impl( &aLbLineStyle );
    }

    const SfxPoolItem* pItem;
    SfxObjectShell* pShell;
    if(SFX_ITEM_SET == rSet.GetItemState(SID_HTML_MODE, FALSE, &pItem) ||
        ( 0 != (pShell = SfxObjectShell::Current()) &&
                    0 != (pItem = pShell->GetItem(SID_HTML_MODE))))
    {
        USHORT nHtmlMode = ((SfxUInt16Item*)pItem)->GetValue();
        if(nHtmlMode & HTMLMODE_ON)
        {
            //Im Html-Mode gibt es keinen Schatten und nur komplette Umrandungen
            aFtShadowPos  .Disable();
            aWndShadows   .Disable();
            aFtShadowSize .Disable();
            aEdShadowSize .Disable();
            aFtShadowColor.Disable();
            aLbShadowColor.Disable();
            aFlShadow     .Disable();

            USHORT nLBCount = aLbLineStyle.GetEntryCount();
            // ist es ein Absatzdialog, dann alle Linien fuer
            // Sw-Export, sonst ist die Page nicht da
            if(!(mbHorEnabled || mbVerEnabled)
                 && 0 == (nHtmlMode & HTMLMODE_FULL_ABS_POS) &&
                SFX_ITEM_AVAILABLE > rSet.GetItemState(GetWhich( SID_ATTR_PARA_LINESPACE )))
            {
                for( USHORT i = nLBCount - 1; i > LINESTYLE_HTML_MAX; --i)
                    aLbLineStyle.RemoveEntry(i);
            }

            if( !(nSWMode & SW_BORDER_MODE_TABLE) )
            {
                aUserDefFT.Disable();
                aFrameSel.Disable();
                aWndPresets.RemoveItem(3);
                aWndPresets.RemoveItem(4);
                aWndPresets.RemoveItem(5);
            }
        }
    }

    LinesChanged_Impl( 0 );
}

// -----------------------------------------------------------------------

int SvxBorderTabPage::DeactivatePage( SfxItemSet* pSet )
{
    if ( pSet )
        FillItemSet( *pSet );

    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

#define IS_DONT_CARE(a) ((a).GetState() == svx::FRAMESTATE_DONTCARE )

BOOL SvxBorderTabPage::FillItemSet( SfxItemSet& rCoreAttrs )
{
    bool bAttrsChanged = SfxTabPage::FillItemSet( rCoreAttrs );

    BOOL                  bPut          = TRUE;
    USHORT                nBoxWhich     = GetWhich( SID_ATTR_BORDER_OUTER );
    USHORT                nBoxInfoWhich = GetWhich( SID_ATTR_BORDER_INNER );
    const SfxItemSet&     rOldSet       = GetItemSet();
    SvxBoxItem            aBoxItem      ( nBoxWhich );
    SvxBoxInfoItem        aBoxInfoItem  ( nBoxInfoWhich );
    SvxBoxItem* pOldBoxItem = (SvxBoxItem*)GetOldItem( rCoreAttrs, SID_ATTR_BORDER_OUTER );

    SfxMapUnit eCoreUnit = rOldSet.GetPool()->GetMetric( nBoxWhich );
    const SfxPoolItem* pOld = 0;

    //------------------
    // Umrandung aussen:
    //------------------
    typedef ::std::pair<svx::FrameBorderType,USHORT> TBorderPair;
    TBorderPair eTypes1[] = {
                                TBorderPair(svx::FRAMEBORDER_TOP,BOX_LINE_TOP),
                                TBorderPair(svx::FRAMEBORDER_BOTTOM,BOX_LINE_BOTTOM),
                                TBorderPair(svx::FRAMEBORDER_LEFT,BOX_LINE_LEFT),
                                TBorderPair(svx::FRAMEBORDER_RIGHT,BOX_LINE_RIGHT),
                            };

    for (sal_Int32 i=0; i < sizeof(eTypes1)/sizeof(TBorderPair); ++i)
        aBoxItem.SetLine( aFrameSel.GetBorderStyle( eTypes1[i].first ), eTypes1[i].second );

    //--------------------------------
    // Umrandung hor/ver und TableFlag
    //--------------------------------
    TBorderPair eTypes2[] = {
                                TBorderPair(svx::FRAMEBORDER_HOR,BOXINFO_LINE_HORI),
                                TBorderPair(svx::FRAMEBORDER_VER,BOXINFO_LINE_VERT)
                            };
    for (sal_Int32 j=0; j < sizeof(eTypes2)/sizeof(TBorderPair); ++j)
        aBoxInfoItem.SetLine( aFrameSel.GetBorderStyle( eTypes2[j].first ), eTypes2[j].second );

    aBoxInfoItem.EnableHor( mbHorEnabled );
    aBoxInfoItem.EnableVer( mbVerEnabled );

    //-------------------
    // Abstand nach Innen
    //-------------------
    if( !mbUseMarginItem )
    {
        if( aLeftMF.IsVisible() )
        {
            aBoxInfoItem.SetDist( TRUE );

            // #106224# all edits empty: do nothing
            if( aLeftMF.GetText().Len() || aRightMF.GetText().Len() ||
                aTopMF.GetText().Len() || aBottomMF.GetText().Len() )
            {
                if ( ((mbHorEnabled || mbVerEnabled || (nSWMode & SW_BORDER_MODE_TABLE)) &&
                        (aLeftMF.IsModified()||aRightMF.IsModified()||
                            aTopMF.IsModified()||aBottomMF.IsModified()) )||
                     aFrameSel.GetBorderState( svx::FRAMEBORDER_TOP ) != svx::FRAMESTATE_HIDE
                     || aFrameSel.GetBorderState( svx::FRAMEBORDER_BOTTOM ) != svx::FRAMESTATE_HIDE
                     || aFrameSel.GetBorderState( svx::FRAMEBORDER_LEFT ) != svx::FRAMESTATE_HIDE
                     || aFrameSel.GetBorderState( svx::FRAMEBORDER_RIGHT ) != svx::FRAMESTATE_HIDE )
                {
                    SvxBoxInfoItem* pOldBoxInfoItem = (SvxBoxInfoItem*)GetOldItem(
                                                        rCoreAttrs, SID_ATTR_BORDER_INNER );
                    if(!pOldBoxItem ||
                    aLeftMF  .GetText() != aLeftMF  .GetSavedValue() ||
                    aRightMF .GetText() != aRightMF .GetSavedValue() ||
                    aTopMF   .GetText() != aTopMF   .GetSavedValue() ||
                    aBottomMF.GetText() != aBottomMF.GetSavedValue() ||
                    nMinValue == aLeftMF  .GetValue() ||
                    nMinValue == aRightMF .GetValue() ||
                    nMinValue == aTopMF   .GetValue() ||
                    nMinValue == aBottomMF.GetValue() ||
                        pOldBoxInfoItem && !pOldBoxInfoItem->IsValid(VALID_DISTANCE))
                    {
                        aBoxItem.SetDistance( (USHORT)GetCoreValue( aLeftMF, eCoreUnit ), BOX_LINE_LEFT  );
                        aBoxItem.SetDistance( (USHORT)GetCoreValue( aRightMF, eCoreUnit ), BOX_LINE_RIGHT );
                        aBoxItem.SetDistance( (USHORT)GetCoreValue( aTopMF, eCoreUnit ), BOX_LINE_TOP   );
                        aBoxItem.SetDistance( (USHORT)GetCoreValue( aBottomMF, eCoreUnit ), BOX_LINE_BOTTOM);
                    }
                    else
                    {
                        aBoxItem.SetDistance(pOldBoxItem->GetDistance(BOX_LINE_LEFT ), BOX_LINE_LEFT);
                        aBoxItem.SetDistance(pOldBoxItem->GetDistance(BOX_LINE_RIGHT),  BOX_LINE_RIGHT);
                        aBoxItem.SetDistance(pOldBoxItem->GetDistance(BOX_LINE_TOP  ), BOX_LINE_TOP);
                        aBoxItem.SetDistance(pOldBoxItem->GetDistance(BOX_LINE_BOTTOM), BOX_LINE_BOTTOM);
                    }
                    aBoxInfoItem.SetValid( VALID_DISTANCE, TRUE );
                }
                else
                    aBoxInfoItem.SetValid( VALID_DISTANCE, FALSE );
            }
        }
    }

    //------------------------------------------
    // Don't Care Status im Info-Item vermerken:
    //------------------------------------------
    aBoxInfoItem.SetValid( VALID_TOP,    aFrameSel.GetBorderState( svx::FRAMEBORDER_TOP )    != svx::FRAMESTATE_DONTCARE );
    aBoxInfoItem.SetValid( VALID_BOTTOM, aFrameSel.GetBorderState( svx::FRAMEBORDER_BOTTOM ) != svx::FRAMESTATE_DONTCARE );
    aBoxInfoItem.SetValid( VALID_LEFT,   aFrameSel.GetBorderState( svx::FRAMEBORDER_LEFT )   != svx::FRAMESTATE_DONTCARE );
    aBoxInfoItem.SetValid( VALID_RIGHT,  aFrameSel.GetBorderState( svx::FRAMEBORDER_RIGHT )  != svx::FRAMESTATE_DONTCARE );
    aBoxInfoItem.SetValid( VALID_HORI,   aFrameSel.GetBorderState( svx::FRAMEBORDER_HOR )    != svx::FRAMESTATE_DONTCARE );
    aBoxInfoItem.SetValid( VALID_VERT,   aFrameSel.GetBorderState( svx::FRAMEBORDER_VER )    != svx::FRAMESTATE_DONTCARE );

    //
    // Put oder Clear der Umrandung?
    //
    bPut = TRUE;

    if (   SFX_ITEM_DEFAULT == rOldSet.GetItemState( nBoxWhich,     FALSE )
        || SFX_ITEM_DEFAULT == rOldSet.GetItemState( nBoxInfoWhich, FALSE ) )
    {
        const SvxBoxInfoItem& rOldBoxInfo = (const SvxBoxInfoItem&)
                                rOldSet.Get(nBoxInfoWhich);

        aBoxInfoItem.SetMinDist( rOldBoxInfo.IsMinDist() );
        aBoxInfoItem.SetDefDist( rOldBoxInfo.GetDefDist() );
        bPut =
            (   (aBoxItem     != (const SvxBoxItem&)(rOldSet.Get(nBoxWhich)))
             || (aBoxInfoItem != rOldBoxInfo ) );
    }

    if ( bPut )
    {
        if ( !pOldBoxItem || !( *pOldBoxItem == aBoxItem ) )
        {
            rCoreAttrs.Put( aBoxItem );
            bAttrsChanged |= TRUE;
        }
        pOld = GetOldItem( rCoreAttrs, SID_ATTR_BORDER_INNER );

        if ( !pOld || !( *(const SvxBoxInfoItem*)pOld == aBoxInfoItem ) )
        {
            rCoreAttrs.Put( aBoxInfoItem );
            bAttrsChanged |= TRUE;
        }
    }
    else
    {
        rCoreAttrs.ClearItem( nBoxWhich );
        rCoreAttrs.ClearItem( nBoxInfoWhich );
    }

    return bAttrsChanged;
}

// -----------------------------------------------------------------------

void SvxBorderTabPage::HideShadowControls()
{
    aFtShadowPos.Hide();
    aWndShadows.Hide();
    aFtShadowSize.Hide();
    aEdShadowSize.Hide();
    aFtShadowColor.Hide();
    aLbShadowColor.Hide();
    aFlShadow.Hide();
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxBorderTabPage, SelPreHdl_Impl, void *, EMPTYARG )
{
    const svx::FrameBorderState SHOW = svx::FRAMESTATE_SHOW;
    const svx::FrameBorderState HIDE = svx::FRAMESTATE_HIDE;
    const svx::FrameBorderState DONT = svx::FRAMESTATE_DONTCARE;

    static const svx::FrameBorderState ppeStates[][ svx::FRAMEBORDERTYPE_COUNT ] =
    {                   /*    Left  Right Top   Bot   Hor   Ver   TLBR  BLTR */
/* ---------------------+--------------------------------------------------- */
/* IID_PRE_CELL_NONE    */  { HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_CELL_ALL     */  { SHOW, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_CELL_LR      */  { SHOW, SHOW, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_CELL_TB      */  { HIDE, HIDE, SHOW, SHOW, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_CELL_L       */  { SHOW, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_CELL_DIAG    */  { HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, SHOW, SHOW },
/* IID_PRE_HOR_NONE     */  { HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_HOR_OUTER    */  { SHOW, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_HOR_HOR      */  { HIDE, HIDE, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE },
/* IID_PRE_HOR_ALL      */  { SHOW, SHOW, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE },
/* IID_PRE_HOR_OUTER2   */  { SHOW, SHOW, SHOW, SHOW, DONT, HIDE, HIDE, HIDE },
/* IID_PRE_VER_NONE     */  { HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_VER_OUTER    */  { SHOW, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_VER_VER      */  { SHOW, SHOW, HIDE, HIDE, HIDE, SHOW, HIDE, HIDE },
/* IID_PRE_VER_ALL      */  { SHOW, SHOW, SHOW, SHOW, HIDE, SHOW, HIDE, HIDE },
/* IID_PRE_VER_OUTER2   */  { SHOW, SHOW, SHOW, SHOW, HIDE, DONT, HIDE, HIDE },
/* IID_PRE_TABLE_NONE   */  { HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_TABLE_OUTER  */  { SHOW, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE, HIDE },
/* IID_PRE_TABLE_OUTERH */  { SHOW, SHOW, SHOW, SHOW, SHOW, HIDE, HIDE, HIDE },
/* IID_PRE_TABLE_ALL    */  { SHOW, SHOW, SHOW, SHOW, SHOW, SHOW, HIDE, HIDE },
/* IID_PRE_TABLE_OUTER2 */  { SHOW, SHOW, SHOW, SHOW, DONT, DONT, HIDE, HIDE }
    };

    // first hide and deselect all frame borders
    aFrameSel.HideAllBorders();
    aFrameSel.DeselectAllBorders();

    // Using image ID to find correct line in table above.
    USHORT nLine = GetPresetImageId( aWndPresets.GetSelectItemId() ) - 1;

    // Apply all styles from the table
    for( int nBorder = 0; nBorder < svx::FRAMEBORDERTYPE_COUNT; ++nBorder )
    {
        svx::FrameBorderType eBorder = svx::GetFrameBorderTypeFromIndex( nBorder );
        switch( ppeStates[ nLine ][ nBorder ] )
        {
            case SHOW:  aFrameSel.SelectBorder( eBorder );      break;
            case HIDE:  /* nothing to do */                     break;
            case DONT:  aFrameSel.SetBorderDontCare( eBorder ); break;
        }
    }

    // Show all lines that have been selected above
    if( aFrameSel.IsAnyBorderSelected() )
    {
        // any visible style, but "no-line" in line list box? -> use hair-line
        if( (aLbLineStyle.GetSelectEntryPos() == 0) || (aLbLineStyle.GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND) )
            aLbLineStyle.SelectEntryPos( 1 );

        // set current style to all previously selected lines
        SelStyleHdl_Impl( &aLbLineStyle );
        SelColHdl_Impl( &aLbLineColor );
    }

    // Presets ValueSet does not show a selection (used as push buttons).
    aWndPresets.SetNoSelection();

    LinesChanged_Impl( 0 );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxBorderTabPage, SelSdwHdl_Impl, void *, EMPTYARG )
{
    BOOL bEnable = aWndShadows.GetSelectItemId() > 1;
    aFtShadowSize.Enable(bEnable);
    aEdShadowSize.Enable(bEnable);
    aFtShadowColor.Enable(bEnable);
    aLbShadowColor.Enable(bEnable);
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxBorderTabPage, SelColHdl_Impl, ListBox *, pLb )
{
    ColorListBox* pColLb = (ColorListBox*)pLb;

    if ( pLb == &aLbLineColor )
    {
        aFrameSel.SetColorToSelection( pColLb->GetSelectEntryColor() );
        aLbLineStyle.SetColor( pColLb->GetSelectEntryColor() );
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxBorderTabPage, SelStyleHdl_Impl, ListBox *, pLb )
{
    if ( pLb == &aLbLineStyle )
        aFrameSel.SetStyleToSelection(
            static_cast< USHORT >( aLbLineStyle.GetSelectEntryLine1() / 100 ),
            static_cast< USHORT >( aLbLineStyle.GetSelectEntryDistance() / 100 ),
            static_cast< USHORT >( aLbLineStyle.GetSelectEntryLine2() / 100 ) );

    return 0;
}

// ============================================================================
// ValueSet handling
// ============================================================================

// number of preset images to show
const USHORT SVX_BORDER_PRESET_COUNT = 5;

// number of shadow images to show
const USHORT SVX_BORDER_SHADOW_COUNT = 5;

// ----------------------------------------------------------------------------

USHORT SvxBorderTabPage::GetPresetImageId( USHORT nValueSetIdx ) const
{
    // table with all sets of predefined border styles
    static const USHORT ppnImgIds[][ SVX_BORDER_PRESET_COUNT ] =
    {
        // simple cell without diagonal frame borders
        {   IID_PRE_CELL_NONE,  IID_PRE_CELL_ALL,       IID_PRE_CELL_LR,        IID_PRE_CELL_TB,    IID_PRE_CELL_L          },
        // simple cell with diagonal frame borders
        {   IID_PRE_CELL_NONE,  IID_PRE_CELL_ALL,       IID_PRE_CELL_LR,        IID_PRE_CELL_TB,    IID_PRE_CELL_DIAG       },
        // with horizontal inner frame border
        {   IID_PRE_HOR_NONE,   IID_PRE_HOR_OUTER,      IID_PRE_HOR_HOR,        IID_PRE_HOR_ALL,    IID_PRE_HOR_OUTER2      },
        // with vertical inner frame border
        {   IID_PRE_VER_NONE,   IID_PRE_VER_OUTER,      IID_PRE_VER_VER,        IID_PRE_VER_ALL,    IID_PRE_VER_OUTER2      },
        // with horizontal and vertical inner frame borders
        {   IID_PRE_TABLE_NONE, IID_PRE_TABLE_OUTER,    IID_PRE_TABLE_OUTERH,   IID_PRE_TABLE_ALL,  IID_PRE_TABLE_OUTER2    }
    };

    // find correct set of presets
    int nLine = 0;
    if( !mbHorEnabled && !mbVerEnabled )
        nLine = (mbTLBREnabled || mbBLTREnabled) ? 1 : 0;
    else if( mbHorEnabled && !mbVerEnabled )
        nLine = 2;
    else if( !mbHorEnabled && mbVerEnabled )
        nLine = 3;
    else
        nLine = 4;

    DBG_ASSERT( (1 <= nValueSetIdx) && (nValueSetIdx <= SVX_BORDER_PRESET_COUNT),
        "SvxBorderTabPage::GetPresetImageId - wrong index" );
    return ppnImgIds[ nLine ][ nValueSetIdx - 1 ];
}

USHORT SvxBorderTabPage::GetPresetStringId( USHORT nValueSetIdx ) const
{
    // string resource IDs for each image (in order of the IID_PRE_* image IDs)
    static const USHORT pnStrIds[] =
    {
        RID_SVXSTR_TABLE_PRESET_NONE,
        RID_SVXSTR_PARA_PRESET_ALL,
        RID_SVXSTR_PARA_PRESET_LEFTRIGHT,
        RID_SVXSTR_PARA_PRESET_TOPBOTTOM,
        RID_SVXSTR_PARA_PRESET_ONLYLEFT,
        RID_SVXSTR_PARA_PRESET_DIAGONAL,

        RID_SVXSTR_TABLE_PRESET_NONE,
        RID_SVXSTR_TABLE_PRESET_ONLYOUTER,
        RID_SVXSTR_HOR_PRESET_ONLYHOR,
        RID_SVXSTR_TABLE_PRESET_OUTERALL,
        RID_SVXSTR_TABLE_PRESET_OUTERINNER,

        RID_SVXSTR_TABLE_PRESET_NONE,
        RID_SVXSTR_TABLE_PRESET_ONLYOUTER,
        RID_SVXSTR_VER_PRESET_ONLYVER,
        RID_SVXSTR_TABLE_PRESET_OUTERALL,
        RID_SVXSTR_TABLE_PRESET_OUTERINNER,

        RID_SVXSTR_TABLE_PRESET_NONE,
        RID_SVXSTR_TABLE_PRESET_ONLYOUTER,
        RID_SVXSTR_TABLE_PRESET_OUTERHORI,
        RID_SVXSTR_TABLE_PRESET_OUTERALL,
        RID_SVXSTR_TABLE_PRESET_OUTERINNER
    };
    return pnStrIds[ GetPresetImageId( nValueSetIdx ) - 1 ];
}

// ----------------------------------------------------------------------------

void SvxBorderTabPage::FillPresetVS()
{
    // find correct image list
    bool bDark = aWndPresets.GetDisplayBackground().GetColor().IsDark();
    ImageList& rImgList = bDark ? aBorderImgLstH : aBorderImgLst;
    Size aImgSize( rImgList.GetImage( IID_PRE_CELL_NONE ).GetSizePixel() );

    // basic initialization of the ValueSet
    aWndPresets.SetColCount( SVX_BORDER_PRESET_COUNT );
    aWndPresets.SetStyle( aWndPresets.GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER );
    aWndPresets.SetSizePixel( aWndPresets.CalcWindowSizePixel( aImgSize ) );

    // insert images and help texts
    for( USHORT nVSIdx = 1; nVSIdx <= SVX_BORDER_PRESET_COUNT; ++nVSIdx )
    {
        aWndPresets.InsertItem( nVSIdx );
        aWndPresets.SetItemImage( nVSIdx, rImgList.GetImage( GetPresetImageId( nVSIdx ) ) );
        aWndPresets.SetItemText( nVSIdx, SVX_RESSTR( GetPresetStringId( nVSIdx ) ) );
    }

    // show the control
    aWndPresets.SetNoSelection();
    aWndPresets.Show();
}

// ----------------------------------------------------------------------------

void SvxBorderTabPage::FillShadowVS()
{
    // find correct image list
    bool bDark = aWndShadows.GetDisplayBackground().GetColor().IsDark();
    ImageList& rImgList = bDark ? aShadowImgLstH : aShadowImgLst;
    Size aImgSize( rImgList.GetImage( IID_SHADOWNONE ).GetSizePixel() );

    // basic initialization of the ValueSet
    aWndShadows.SetColCount( SVX_BORDER_SHADOW_COUNT );
    aWndShadows.SetStyle( aWndShadows.GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER );
    aWndShadows.SetSizePixel( aWndShadows.CalcWindowSizePixel( aImgSize ) );

    // image resource IDs
    static const USHORT pnImgIds[ SVX_BORDER_SHADOW_COUNT ] =
        { IID_SHADOWNONE, IID_SHADOW_BOT_RIGHT, IID_SHADOW_TOP_RIGHT, IID_SHADOW_BOT_LEFT, IID_SHADOW_TOP_LEFT };
    // string resource IDs for each image
    static const USHORT pnStrIds[ SVX_BORDER_SHADOW_COUNT ] =
        { RID_SVXSTR_SHADOW_STYLE_NONE, RID_SVXSTR_SHADOW_STYLE_BOTTOMRIGHT, RID_SVXSTR_SHADOW_STYLE_TOPRIGHT, RID_SVXSTR_SHADOW_STYLE_BOTTOMLEFT, RID_SVXSTR_SHADOW_STYLE_TOPLEFT };

    // insert images and help texts
    for( USHORT nVSIdx = 1; nVSIdx <= SVX_BORDER_SHADOW_COUNT; ++nVSIdx )
    {
        aWndShadows.InsertItem( nVSIdx );
        aWndShadows.SetItemImage( nVSIdx, rImgList.GetImage( pnImgIds[ nVSIdx - 1 ] ) );
        aWndShadows.SetItemText( nVSIdx, SVX_RESSTR( pnStrIds[ nVSIdx - 1 ] ) );
    }

    // show the control
    aWndShadows.SelectItem( 1 );
    aWndShadows.Show();
}

// ----------------------------------------------------------------------------

void SvxBorderTabPage::FillValueSets()
{
    FillPresetVS();
    FillShadowVS();
}

// ============================================================================

void SvxBorderTabPage::FillLineListBox_Impl()
{
    aLbLineStyle.SetUnit( FUNIT_POINT );
    aLbLineStyle.SetSourceUnit( FUNIT_TWIP );

    // Writer 2.0 Defaults:
    aLbLineStyle.InsertEntry( SVX_RESSTR( STR_NONE ) );

    aLbLineStyle.InsertEntry( LINE_WIDTH0 );
    aLbLineStyle.InsertEntry( LINE_WIDTH5 );
    aLbLineStyle.InsertEntry( LINE_WIDTH1 );
    aLbLineStyle.InsertEntry( LINE_WIDTH2 );
    aLbLineStyle.InsertEntry( LINE_WIDTH3 );
    aLbLineStyle.InsertEntry( LINE_WIDTH4 );

    // OS: wenn hier neue Linienstaerken zugfuegt werden, dann
    // LINESTYLE_HTML_MAX anpassen

    aLbLineStyle.InsertEntry( DLINE0_OUT, DLINE0_IN, DLINE0_DIST );
    aLbLineStyle.InsertEntry( DLINE7_OUT, DLINE7_IN, DLINE7_DIST );
    aLbLineStyle.InsertEntry( DLINE1_OUT, DLINE1_IN, DLINE1_DIST );
    aLbLineStyle.InsertEntry( DLINE2_OUT, DLINE2_IN, DLINE2_DIST );
    aLbLineStyle.InsertEntry( DLINE8_OUT, DLINE8_IN, DLINE8_DIST );
    aLbLineStyle.InsertEntry( DLINE9_OUT, DLINE9_IN, DLINE9_DIST );
    aLbLineStyle.InsertEntry( DLINE10_OUT,DLINE10_IN,DLINE10_DIST);
    aLbLineStyle.InsertEntry( DLINE3_OUT, DLINE3_IN, DLINE3_DIST );
    aLbLineStyle.InsertEntry( DLINE4_OUT, DLINE4_IN, DLINE4_DIST );
    aLbLineStyle.InsertEntry( DLINE5_OUT, DLINE5_IN, DLINE5_DIST );
    aLbLineStyle.InsertEntry( DLINE6_OUT, DLINE6_IN, DLINE6_DIST );
}

// -----------------------------------------------------------------------
IMPL_LINK( SvxBorderTabPage, LinesChanged_Impl, void*, EMPTYARG )
{
    if(!mbUseMarginItem && aLeftMF.IsVisible())
    {
        BOOL bLineSet = aFrameSel.IsAnyBorderVisible();
        BOOL bMinAllowed = 0 != (nSWMode & (SW_BORDER_MODE_FRAME|SW_BORDER_MODE_TABLE));
        BOOL bSpaceModified =   aLeftMF  .IsModified()||
                                aRightMF .IsModified()||
                                aTopMF   .IsModified()||
                                aBottomMF.IsModified();

        if(bLineSet)
        {
            if(!bMinAllowed)
            {
                aLeftMF  .SetFirst(nMinValue);
                aRightMF .SetFirst(nMinValue);
                aTopMF   .SetFirst(nMinValue);
                aBottomMF.SetFirst(nMinValue);
            }
            if(!bSpaceModified)
            {
                aLeftMF  .SetValue(nMinValue);
                aRightMF .SetValue(nMinValue);
                aTopMF   .SetValue(nMinValue);
                aBottomMF.SetValue(nMinValue);
            }
        }
        else
        {
            aLeftMF  .SetMin(0);
            aRightMF .SetMin(0);
            aTopMF   .SetMin(0);
            aBottomMF.SetMin(0);
            aLeftMF  .SetFirst(0);
            aRightMF .SetFirst(0);
            aTopMF   .SetFirst(0);
            aBottomMF.SetFirst(0);
            if(!bSpaceModified)
            {
                aLeftMF  .SetValue(0);
                aRightMF .SetValue(0);
                aTopMF   .SetValue(0);
                aBottomMF.SetValue(0);
            }
        }
        //fuer Tabellen ist alles erlaubt
        USHORT nValid = VALID_TOP|VALID_BOTTOM|VALID_LEFT|VALID_RIGHT;

        //fuer Rahmen und  Absatz wird das Edit disabled, wenn keine Border gesetzt ist
        if(nSWMode & (SW_BORDER_MODE_FRAME|SW_BORDER_MODE_PARA))
        {
            if(bLineSet)
            {
                nValid  = (aFrameSel.GetBorderState( svx::FRAMEBORDER_TOP)    == svx::FRAMESTATE_SHOW) ? VALID_TOP : 0;
                nValid |= (aFrameSel.GetBorderState( svx::FRAMEBORDER_BOTTOM) == svx::FRAMESTATE_SHOW) ? VALID_BOTTOM : 0;
                nValid |= (aFrameSel.GetBorderState( svx::FRAMEBORDER_LEFT)   == svx::FRAMESTATE_SHOW) ? VALID_LEFT : 0;
                nValid |= (aFrameSel.GetBorderState( svx::FRAMEBORDER_RIGHT ) == svx::FRAMESTATE_SHOW) ? VALID_RIGHT : 0;
            }
            else
                nValid = 0;
        }
        aLeftFT.Enable(0 != (nValid&VALID_LEFT));
        aRightFT.Enable(0 != (nValid&VALID_RIGHT));
        aTopFT.Enable(0 != (nValid&VALID_TOP));
        aBottomFT.Enable(0 != (nValid&VALID_BOTTOM));
        aLeftMF.Enable(0 != (nValid&VALID_LEFT));
        aRightMF.Enable(0 != (nValid&VALID_RIGHT));
        aTopMF.Enable(0 != (nValid&VALID_TOP));
        aBottomMF.Enable(0 != (nValid&VALID_BOTTOM));
        aSynchronizeCB.Enable( aRightMF.IsEnabled() || aTopMF.IsEnabled() ||
                               aBottomMF.IsEnabled() || aLeftMF.IsEnabled() );
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxBorderTabPage, ModifyDistanceHdl_Impl, MetricField*, pField)
{
    if ( bSync )
    {
        long nVal = pField->GetValue();
        if(pField != &aLeftMF)
            aLeftMF.SetValue(nVal);
        if(pField != &aRightMF)
            aRightMF.SetValue(nVal);
        if(pField != &aTopMF)
            aTopMF.SetValue(nVal);
        if(pField != &aBottomMF)
            aBottomMF.SetValue(nVal);
    }
    return 0;
}

IMPL_LINK( SvxBorderTabPage, SyncHdl_Impl, CheckBox*, pBox)
{
    bSync = pBox->IsChecked();
    return 0;
}


void    SvxBorderTabPage::SetSWMode(BYTE nSet)
{
//#define SW_BORDER_MODE_PARA   0x01
//#define SW_BORDER_MODE_TABLE  0x02
//#define SW_BORDER_MODE_FRAME  0x04
    nSWMode = nSet;
}
/* -----------------------------03.06.2002 10:15------------------------------

 ---------------------------------------------------------------------------*/
void SvxBorderTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    if( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
        FillValueSets();

    SfxTabPage::DataChanged( rDCEvt );
}

void SvxBorderTabPage::PageCreated (SfxAllItemSet aSet) //add CHINA001
{
    SFX_ITEMSET_ARG (&aSet,pSWModeItem,SfxUInt16Item,SID_SWMODE_TYPE,sal_False);
    SFX_ITEMSET_ARG (&aSet,pFlagItem,SfxUInt32Item,SID_FLAG_TYPE,sal_False);
    if (pSWModeItem)
        SetSWMode(pSWModeItem->GetValue());
    if (pFlagItem)
        if ( ( pFlagItem->GetValue() & SVX_HIDESHADOWCTL ) == SVX_HIDESHADOWCTL )
            HideShadowControls();
}

// ============================================================================


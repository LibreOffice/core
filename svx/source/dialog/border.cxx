/*************************************************************************
 *
 *  $RCSfile: border.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:00:45 $
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
#include "linelink.hxx"
#include "dlgutil.hxx"
#include "dialmgr.hxx"
#include "htmlmode.hxx"
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
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
    SID_ATTR_BORDER_INNER,
    SID_ATTR_BORDER_SHADOW,
    0
};

BOOL SvxBorderTabPage::bSync = TRUE;


//------------------------------------------------------------------------

#define LINE_WIDTH0     (DEF_LINE_WIDTH_0 *100)
#define LINE_WIDTH1     (DEF_LINE_WIDTH_1 *100)
#define LINE_WIDTH2     (DEF_LINE_WIDTH_2 *100)
#define LINE_WIDTH3     (DEF_LINE_WIDTH_3 *100)
#define LINE_WIDTH4     (DEF_LINE_WIDTH_4 *100)

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

#define EQSTYLE(s1,s2) \
    (   (s1).nLeft   == (s2).nLeft      \
     && (s1).nMiddle == (s2).nMiddle    \
     && (s1).nRight  == (s2).nRight     \
    )

#define SET_STYLE(s,l,m,r)  \
        ((s).nLeft   = l),  \
        ((s).nMiddle = m),   \
         ((s).nRight  = r)

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
        aFrameSel(          this, ResId( WIN_FRAMESEL )),
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
        aShadowImgLstH( ResId(ILH_SDW_BITMAPS)),
        aShadowImgLst( ResId(IL_SDW_BITMAPS)),
        aBorderImgLstH( ResId(ILH_PRE_BITMAPS)),
        aBorderImgLst( ResId(IL_PRE_BITMAPS)),
        nMinValue(0),
        bIsTableBorder  ( FALSE ),
        nSWMode(0)
{
    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    // Metrik einstellen
    FieldUnit eFUnit = GetModuleFieldUnit( &rCoreAttrs );

    switch ( eFUnit )
    {
        case FUNIT_M:
        case FUNIT_KM:
            eFUnit = FUNIT_MM;
            break;
    }

    SetFieldUnit( aEdShadowSize, eFUnit );

    USHORT nWhich = GetWhich( SID_ATTR_BORDER_INNER );
    BOOL bIsDontCare = TRUE;

    if ( rCoreAttrs.GetItemState( nWhich, TRUE ) >= SFX_ITEM_AVAILABLE )
    {
        // Absatz oder Tabelle
        const SvxBoxInfoItem* pBoxInfo =
            (const SvxBoxInfoItem*)&( rCoreAttrs.Get( nWhich ) );
        bIsTableBorder = pBoxInfo->IsTable();

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
    if(eFUnit == FUNIT_MM && SFX_MAPUNIT_TWIP == rCoreAttrs.GetPool()->GetMetric( GetWhich( SID_ATTR_BORDER_INNER ) ))
    {
        aLeftMF.SetDecimalDigits(1);
        aRightMF.SetDecimalDigits(1);
        aTopMF.SetDecimalDigits(1);
        aBottomMF.SetDecimalDigits(1);
        aEdShadowSize.SetDecimalDigits(1);
    }

    aFrameSel.Initialize(   bIsTableBorder
                                ? SVX_FRMSELTYPE_TABLE
                                : SVX_FRMSELTYPE_PARAGRAPH,
                                bIsDontCare );

    aFrameSel.SetSelectLink(LINK(this, SvxBorderTabPage, LinesChanged_Impl));
    aLbLineStyle.SetSelectHdl( LINK( this, SvxBorderTabPage, SelStyleHdl_Impl ) );
    aLbLineColor.SetSelectHdl( LINK( this, SvxBorderTabPage, SelColHdl_Impl ) );
    aLbShadowColor.SetSelectHdl( LINK( this, SvxBorderTabPage, SelColHdl_Impl ) );
    aWndPresets.SetSelectHdl( LINK( this, SvxBorderTabPage, SelPreHdl_Impl ) );
    aWndShadows.SetSelectHdl( LINK( this, SvxBorderTabPage, SelSdwHdl_Impl ) );

    FillValueSets_Impl();
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

void SvxBorderTabPage::ResetFrameLine_Impl( const SvxBorderLine* pCoreLine,
                                            SvxFrameLine& rFrameLine )
{
    SvxLineStruct   newLineStyle = { 0,0,0 };

    if ( !pCoreLine ) // == Linie nicht darstellen
    {
        rFrameLine.SetStyle( newLineStyle );
        rFrameLine.SetColor( RGBCOL(COL_BLACK) );
    }
    else
    {
        newLineStyle.nLeft   = pCoreLine->GetOutWidth()*100;
        newLineStyle.nMiddle = pCoreLine->GetDistance()*100;
        newLineStyle.nRight  = pCoreLine->GetInWidth() *100;
        rFrameLine.SetStyle( newLineStyle );
        rFrameLine.SetColor( pCoreLine->GetColor() );
    }
}

// -----------------------------------------------------------------------

void SvxBorderTabPage::Reset( const SfxItemSet& rSet )
{
    const SvxBoxItem*       pBoxItem;
    const SvxBoxInfoItem*   pBoxInfoItem;
    const SvxShadowItem*    pShadowItem;
    USHORT                  nWhichBox       = GetWhich(SID_ATTR_BORDER_OUTER);
    USHORT                  nWhichShadow    = GetWhich(SID_ATTR_BORDER_SHADOW);
    SfxMapUnit              eCoreUnit;
    const Color             aColBlack       = RGBCOL(COL_BLACK);

    pBoxItem  = (const SvxBoxItem*)GetItem( rSet, SID_ATTR_BORDER_OUTER );
    pBoxInfoItem = (const SvxBoxInfoItem*)GetItem( rSet, SID_ATTR_BORDER_INNER );
    pShadowItem = (const SvxShadowItem*)GetItem( rSet, SID_ATTR_BORDER_SHADOW );

    eCoreUnit = rSet.GetPool()->GetMetric( nWhichBox );

    if ( pBoxItem && pBoxInfoItem ) // -> Don't Care
    {
        //-----------------
        // Umrandung links:
        //-----------------
        if ( pBoxInfoItem->IsValid( VALID_LEFT ) )
            ResetFrameLine_Impl( pBoxItem->GetLeft(),
                                 aFrameSel.GetLine(SVX_FRMSELLINE_LEFT) );
        else
            aFrameSel.GetLine(SVX_FRMSELLINE_LEFT).SetState( SVX_FRMLINESTATE_DONT_CARE );

        //------------------
        // Umrandung rechts:
        //------------------
        if ( pBoxInfoItem->IsValid( VALID_RIGHT ) )
            ResetFrameLine_Impl( pBoxItem->GetRight(),
                                 aFrameSel.GetLine(SVX_FRMSELLINE_RIGHT) );
        else
            aFrameSel.GetLine(SVX_FRMSELLINE_RIGHT).SetState( SVX_FRMLINESTATE_DONT_CARE );

        //----------------
        // Umrandung oben:
        //----------------
        if ( pBoxInfoItem->IsValid( VALID_TOP ) )
            ResetFrameLine_Impl( pBoxItem->GetTop(),
                                 aFrameSel.GetLine(SVX_FRMSELLINE_TOP) );
        else
            aFrameSel.GetLine(SVX_FRMSELLINE_TOP).SetState( SVX_FRMLINESTATE_DONT_CARE );

        //-----------------
        // Umrandung unten:
        //-----------------
        if ( pBoxInfoItem->IsValid( VALID_BOTTOM ) )
            ResetFrameLine_Impl( pBoxItem->GetBottom(),
                                 aFrameSel.GetLine(SVX_FRMSELLINE_BOTTOM) );
        else
            aFrameSel.GetLine(SVX_FRMSELLINE_BOTTOM).SetState( SVX_FRMLINESTATE_DONT_CARE );

        //-------------------
        // Abstand nach innen
        //-------------------
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
                if ( SFX_ITEM_SET == rSet.GetItemState( nWhichBox, FALSE ) &&
                     pBoxInfoItem->IsValid( VALID_DISTANCE ) )
                {
                    BOOL bIsAnyLineSet = aFrameSel.IsAnyLineSet();
                    if( !bIsAnyLineSet||
                        !pBoxInfoItem->IsMinDist() )
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
                    const long nDefDist = bIsAnyLineSet ? pBoxInfoItem->GetDefDist() : 0;
                    BOOL bDiffDist = (nDefDist != nLeftDist ||
                                nDefDist != nRightDist ||
                                nDefDist != nTopDist   ||
                                nDefDist != nBottomDist);
                    if((pBoxItem->GetDistance() ||
                            (nSWMode & (SW_BORDER_MODE_FRAME|SW_BORDER_MODE_TABLE))&&
                                aFrameSel.IsAnyLineSet()) &&
                                bDiffDist )
                    {
                        aLeftMF.SetModifyFlag();
                        aRightMF.SetModifyFlag();
                        aTopMF.SetModifyFlag();
                        aBottomMF.SetModifyFlag();
                    }
                }
                else
                {
                    SetMetricValue( aLeftMF,    pBoxInfoItem->GetDefDist(), eCoreUnit );
                    SetMetricValue( aRightMF,   pBoxInfoItem->GetDefDist(), eCoreUnit );
                    SetMetricValue( aTopMF,     pBoxInfoItem->GetDefDist(), eCoreUnit );
                    SetMetricValue( aBottomMF,  pBoxInfoItem->GetDefDist(), eCoreUnit );
                }
            }
            aLeftMF.SaveValue();
            aRightMF.SaveValue();
            aTopMF.SaveValue();
            aBottomMF.SaveValue();
        }

        //----------------
        // innen vertikal:
        //----------------
        if ( pBoxInfoItem->IsValid( VALID_VERT ) )
            ResetFrameLine_Impl( pBoxInfoItem->GetVert(),
                                 aFrameSel.GetLine(SVX_FRMSELLINE_VER) );
        else
            aFrameSel.GetLine(SVX_FRMSELLINE_VER).SetState( SVX_FRMLINESTATE_DONT_CARE );

        //------------------
        // innen horizontal:
        //------------------
        if ( pBoxInfoItem->IsValid( VALID_HORI ) )
            ResetFrameLine_Impl( pBoxInfoItem->GetHori(),
                                 aFrameSel.GetLine(SVX_FRMSELLINE_HOR) );
        else
            aFrameSel.GetLine(SVX_FRMSELLINE_HOR).SetState( SVX_FRMLINESTATE_DONT_CARE );

    }
    else
    {
        // ResetFrameLine-Aufrufe einsparen:
        Color         aColBlack = RGBCOL(COL_BLACK);
        SvxLineStruct aNullLine = { 0,0,0 };
        SvxFrameLine& rLeft     = aFrameSel.GetLine(SVX_FRMSELLINE_LEFT);
        SvxFrameLine& rRight    = aFrameSel.GetLine(SVX_FRMSELLINE_RIGHT);
        SvxFrameLine& rTop      = aFrameSel.GetLine(SVX_FRMSELLINE_TOP);
        SvxFrameLine& rBottom   = aFrameSel.GetLine(SVX_FRMSELLINE_BOTTOM);
        SvxFrameLine& rHor      = aFrameSel.GetLine(SVX_FRMSELLINE_HOR);
        SvxFrameLine& rVer      = aFrameSel.GetLine(SVX_FRMSELLINE_VER);

        rLeft   .SetStyle( aNullLine ); rLeft   .SetColor( aColBlack );
        rRight  .SetStyle( aNullLine ); rRight  .SetColor( aColBlack );
        rTop    .SetStyle( aNullLine ); rTop    .SetColor( aColBlack );
        rBottom .SetStyle( aNullLine ); rBottom .SetColor( aColBlack );
        rHor    .SetStyle( aNullLine ); rHor    .SetColor( aColBlack );
        rVer    .SetStyle( aNullLine ); rVer    .SetColor( aColBlack );
    }

    //-------------------------------
    // Linien im Selektor darstellen:
    //-------------------------------
    aFrameSel.ShowLines();

    //-------------------------------------------------------------
    // Linie/Linienfarbe in Controllern darstellen, wenn eindeutig:
    //-------------------------------------------------------------
    {
        SvxLineStruct   aDefStyle;
        List            aList;

        SvxFrameSelectorLine eTypes[] = {   SVX_FRMSELLINE_TOP,
                                            SVX_FRMSELLINE_BOTTOM,
                                            SVX_FRMSELLINE_LEFT,
                                            SVX_FRMSELLINE_RIGHT,
                                            SVX_FRMSELLINE_HOR,
                                            SVX_FRMSELLINE_VER
                                        };

        for (sal_Int32 i=0; i < sizeof(eTypes)/sizeof(SvxFrameSelectorLine); ++i)
        {
            if ( aFrameSel.GetLine(eTypes[i]).GetState() == SVX_FRMLINESTATE_SHOW )
            {
                aList.Insert( &(aFrameSel.GetLine(eTypes[i])) );
                aFrameSel.SelectLine( eTypes[i] );
            }
            else
                aFrameSel.SelectLine( eTypes[i], FALSE );
        }

        if ( aList.Count() > 0 )
        {
            SvxFrameLine*   pLine = (SvxFrameLine*)aList.First();
            Color           aColor( pLine->GetColor() );
            SvxLineStruct   aStyle = pLine->GetStyle();
            BOOL            bEqual = TRUE;

            // Linienfarbe -------------------------------------

            while ( pLine && bEqual )
            {
                bEqual = (aColor == pLine->GetColor());
                pLine  = (SvxFrameLine*)aList.Next();
            }

            if ( !bEqual )
                aColor = aColBlack;

            USHORT nSelPos = aLbLineColor.GetEntryPos( aColor );

            if ( LISTBOX_ENTRY_NOTFOUND != nSelPos )
                aLbLineColor.SelectEntryPos( nSelPos );
            else
            {
                nSelPos = aLbLineColor.GetEntryPos( aColor );

                if ( LISTBOX_ENTRY_NOTFOUND != nSelPos )
                {
                    aLbLineColor.SelectEntryPos( nSelPos );
                }
                else
                    aLbLineColor.SelectEntryPos( aLbLineColor.InsertEntry(
                        aColor, SVX_RESSTR( RID_SVXSTR_COLOR_USER ) ) );
            }
            aLbLineStyle.SetColor( aColor );
            if ( bEqual )
                aFrameSel.SetCurLineColor( aColor );

            // Linienstil --------------------------------------

            pLine = (SvxFrameLine*)aList.First();
            bEqual = TRUE;

            while ( pLine && bEqual )
            {
                bEqual = EQSTYLE( pLine->GetStyle(), aStyle );
                pLine = (SvxFrameLine*)aList.Next();
            }

            if ( bEqual )
            {
                aLbLineStyle.SelectEntry( aStyle.nLeft,
                                          aStyle.nRight,
                                          aStyle.nMiddle );
                aFrameSel.SetCurLineStyle( aStyle );
            }
            else
            {
                SET_STYLE( aDefStyle, LINE_WIDTH0, 0, 0 );
                aLbLineStyle.SelectEntry( LINE_WIDTH0 );
                aFrameSel.SelectLine( SVX_FRMSELLINE_NONE );
                aFrameSel.SetCurLineStyle( aDefStyle );
            }
        }
        else
        {
            aLbLineColor.SelectEntry( aColBlack, TRUE );
            aLbLineStyle.SetColor( aColBlack );
            aFrameSel.SetCurLineColor( aColBlack );

            SET_STYLE( aDefStyle, 0, 0, 0 );
            aLbLineStyle.SelectEntryPos( 0 );
            aFrameSel.SetCurLineStyle( aDefStyle );
            aFrameSel.SelectLine( SVX_FRMSELLINE_NONE );
        }
    }

    //----------
    // Schatten:
    //----------
    if ( pShadowItem )
    {
        Color  aColor( pShadowItem->GetColor() );
        USHORT nSelPos = aLbShadowColor.GetEntryPos( aColor );

        if ( LISTBOX_ENTRY_NOTFOUND != nSelPos )
            aLbShadowColor.SelectEntryPos( nSelPos );
        else
        {
            nSelPos = aLbShadowColor.GetEntryPos( aColor );

            if ( LISTBOX_ENTRY_NOTFOUND != nSelPos )
            {
                aLbShadowColor.SelectEntryPos( nSelPos );
            }
            else
                aLbShadowColor.SelectEntryPos(
                    aLbShadowColor.InsertEntry(
                        aColor, SVX_RESSTR( RID_SVXSTR_COLOR_USER ) ) );
        }

        aFrameSel.SetShadowColor( aColor );

        // Shadow-Attribut auslesen
        SetMetricValue( aEdShadowSize, pShadowItem->GetWidth(), eCoreUnit );
        USHORT nItem = 1;
        SvxFrameShadow ePos = SVX_FRMSHADOW_NONE;

        switch ( pShadowItem->GetLocation() )
        {
            case SVX_SHADOW_NONE:
                break;
            case SVX_SHADOW_BOTTOMRIGHT:
                nItem = 2;
                ePos = SVX_FRMSHADOW_BOT_RIGHT;
                break;
            case SVX_SHADOW_TOPRIGHT:
                nItem = 3;
                ePos = SVX_FRMSHADOW_TOP_RIGHT;
                break;
            case SVX_SHADOW_BOTTOMLEFT:
                nItem = 4;
                ePos = SVX_FRMSHADOW_BOT_LEFT;
                break;
            case SVX_SHADOW_TOPLEFT:
                nItem = 5;
                ePos = SVX_FRMSHADOW_TOP_LEFT;
                break;
        }
        aWndShadows.SelectItem( nItem );
        aFrameSel.SetShadowPos( ePos );
    }
    else // Don't Care
    {
        // diese Bedingung erfordert noch eine geeignete
        // Schatten-TriState-Darstellung
        aWndShadows.SelectItem( 1 );
        aFrameSel.SetShadowPos( SVX_FRMSHADOW_NONE );

        Color  aColor( aColBlack );
        USHORT nSelPos = aLbShadowColor.GetEntryPos( aColor );

        if ( LISTBOX_ENTRY_NOTFOUND != nSelPos )
            aLbShadowColor.SelectEntryPos( nSelPos );
        else
        {
            nSelPos = aLbShadowColor.GetEntryPos( aColor );

            if ( LISTBOX_ENTRY_NOTFOUND != nSelPos )
            {
                aLbShadowColor.SelectEntryPos( nSelPos );
            }
            else
                aLbShadowColor.SelectEntryPos(
                    aLbShadowColor.InsertEntry(
                        aColor, SVX_RESSTR( RID_SVXSTR_COLOR_USER ) ) );
        }

        aFrameSel.SetShadowColor( aColor );

        // Default-Schattenbreite vom Pool abholen
        SetMetricValue( aEdShadowSize,
                        ((const SvxShadowItem&)rSet.GetPool()->
                            GetDefaultItem( nWhichShadow )).GetWidth(),
                        eCoreUnit );
    }

    BOOL bEnable = aWndShadows.GetSelectItemId() > 1 ;
    aFtShadowSize.Enable(bEnable);
    aEdShadowSize.Enable(bEnable);

    //---------------------------------
    // Schatten im Selektor darstellen:
    //---------------------------------
//    aFrameSel.ShowShadow();

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
            aFtShadowPos  .Enable(FALSE);
            aWndShadows   .Enable(FALSE);
            aFtShadowSize .Enable(FALSE);
            aEdShadowSize .Enable(FALSE);
            aFtShadowColor.Enable(FALSE);
            aLbShadowColor.Enable(FALSE);
            aFlShadow     .Enable(FALSE);

            USHORT nLBCount = aLbLineStyle.GetEntryCount();
            // ist es ein Absatzdialog, dann alle Linien fuer
            // Sw-Export, sonst ist die Page nicht da
            if(!bIsTableBorder && 0 == (nHtmlMode & HTMLMODE_FULL_ABS_POS) &&
                SFX_ITEM_AVAILABLE > rSet.GetItemState(GetWhich( SID_ATTR_PARA_LINESPACE )))
            {
                for( USHORT i = nLBCount - 1; i > LINESTYLE_HTML_MAX; --i)
                    aLbLineStyle.RemoveEntry(i);
            }

            if(!bIsTableBorder)
            {
                aFlBorder   .Enable(FALSE);
                aFrameSel.Enable(FALSE);
                aWndPresets.RemoveItem(3);
                aWndPresets.RemoveItem(4);
                aWndPresets.RemoveItem(5);
            }
        }
    }
}

// -----------------------------------------------------------------------

int SvxBorderTabPage::DeactivatePage( SfxItemSet* pSet )
{
    if ( pSet )
        FillItemSet( *pSet );

    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

void SvxBorderTabPage::SetCoreLine_Impl( const SvxFrameLine* pFrameLine,
                                         SvxBorderLine*& rpCoreLine )
{
    if ( !pFrameLine )
        rpCoreLine = NULL;
    else if ( pFrameLine->GetState() == SVX_FRMLINESTATE_HIDE )
        rpCoreLine = NULL;
    else if ( pFrameLine->GetState() == SVX_FRMLINESTATE_SHOW )
    {
        const USHORT nOut  = (USHORT)pFrameLine->GetStyle().nLeft;
        const USHORT nDist = (USHORT)pFrameLine->GetStyle().nMiddle;
        const USHORT nIn   = (USHORT)pFrameLine->GetStyle().nRight;

        if ( 0 == nOut && 0 == nDist && 0 == nIn )
            rpCoreLine = NULL;
        else
        {
            rpCoreLine->SetColor    ( pFrameLine->GetColor() );
            rpCoreLine->SetOutWidth ( nOut  ? (nOut  / 100) : 0 );
            rpCoreLine->SetDistance ( nDist ? (nDist / 100) : 0 );
            rpCoreLine->SetInWidth  ( nDist ? (nIn   / 100) : 0 );
        }
    }
}

// -----------------------------------------------------------------------

#define IS_DONT_CARE(a) ((a).GetState() == SVX_FRMLINESTATE_DONT_CARE )

BOOL SvxBorderTabPage::FillItemSet( SfxItemSet& rCoreAttrs )
{
    BOOL                  bAttrsChanged = FALSE;
    BOOL                  bPut          = TRUE;
    USHORT                nBoxWhich     = GetWhich( SID_ATTR_BORDER_OUTER );
    USHORT                nBoxInfoWhich = GetWhich( SID_ATTR_BORDER_INNER );
    USHORT                nShadowWhich  = GetWhich( SID_ATTR_BORDER_SHADOW );
    const SfxItemSet&     rOldSet       = GetItemSet();
    SvxBoxItem            aBoxItem      ( nBoxWhich );
    SvxBoxInfoItem        aBoxInfoItem  ( nBoxInfoWhich );
    SvxShadowItem         aShadowItem   ( nShadowWhich );
    SvxBorderLine         aCoreLine;
    SvxBorderLine*        pCoreLine;
    SvxBoxItem* pOldBoxItem = (SvxBoxItem*)GetOldItem( rCoreAttrs, SID_ATTR_BORDER_OUTER );

    SfxMapUnit eCoreUnit = rOldSet.GetPool()->GetMetric( nBoxWhich );
    const SfxPoolItem* pOld = 0;

    //------------------
    // Umrandung aussen:
    //------------------
    typedef ::std::pair<SvxFrameSelectorLine,USHORT> TBorderPair;
    TBorderPair eTypes1[] = {
                                TBorderPair(SVX_FRMSELLINE_TOP,BOX_LINE_TOP),
                                TBorderPair(SVX_FRMSELLINE_BOTTOM,BOX_LINE_BOTTOM),
                                TBorderPair(SVX_FRMSELLINE_LEFT,BOX_LINE_LEFT),
                                TBorderPair(SVX_FRMSELLINE_RIGHT,BOX_LINE_RIGHT),
                            };

    for (sal_Int32 i=0; i < sizeof(eTypes1)/sizeof(TBorderPair); ++i)
    {
        pCoreLine = &aCoreLine;
        SetCoreLine_Impl( &aFrameSel.GetLine(eTypes1[i].first), pCoreLine );
        aBoxItem.SetLine( pCoreLine, eTypes1[i].second );
    }

    //--------------------------------
    // Umrandung hor/ver und TableFlag
    //--------------------------------
    TBorderPair eTypes2[] = {
                                TBorderPair(SVX_FRMSELLINE_HOR,BOXINFO_LINE_HORI),
                                TBorderPair(SVX_FRMSELLINE_VER,BOXINFO_LINE_VERT)
                            };
    for (sal_Int32 j=0; j < sizeof(eTypes2)/sizeof(TBorderPair); ++j)
    {
        pCoreLine = &aCoreLine;
        SetCoreLine_Impl( &aFrameSel.GetLine(eTypes2[j].first), pCoreLine );
        aBoxInfoItem.SetLine( pCoreLine, eTypes2[j].second );
    }

    aBoxInfoItem.SetTable( bIsTableBorder );

    //-------------------
    // Abstand nach Innen
    //-------------------
    if( aLeftMF.IsVisible() )
    {
        aBoxInfoItem.SetDist( TRUE );

        if ( ((bIsTableBorder || (nSWMode & SW_BORDER_MODE_TABLE)) &&
                (aLeftMF.IsModified()||aRightMF.IsModified()||
                    aTopMF.IsModified()||aBottomMF.IsModified()) )||
             aFrameSel.GetLine(SVX_FRMSELLINE_TOP)   .GetState() != SVX_FRMLINESTATE_HIDE
             || aFrameSel.GetLine(SVX_FRMSELLINE_BOTTOM).GetState() != SVX_FRMLINESTATE_HIDE
             || aFrameSel.GetLine(SVX_FRMSELLINE_LEFT)  .GetState() != SVX_FRMLINESTATE_HIDE
             || aFrameSel.GetLine(SVX_FRMSELLINE_RIGHT) .GetState() != SVX_FRMLINESTATE_HIDE )
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

    //------------------------------------------
    // Don't Care Status im Info-Item vermerken:
    //------------------------------------------
    aBoxInfoItem.SetValid( VALID_TOP, !IS_DONT_CARE( aFrameSel.GetLine(SVX_FRMSELLINE_TOP) ) );
    aBoxInfoItem.SetValid( VALID_BOTTOM, !IS_DONT_CARE( aFrameSel.GetLine(SVX_FRMSELLINE_BOTTOM) )    );
    aBoxInfoItem.SetValid( VALID_LEFT, !IS_DONT_CARE( aFrameSel.GetLine(SVX_FRMSELLINE_LEFT) ) );
    aBoxInfoItem.SetValid( VALID_RIGHT, !IS_DONT_CARE( aFrameSel.GetLine(SVX_FRMSELLINE_RIGHT) ) );
    aBoxInfoItem.SetValid( VALID_HORI, !IS_DONT_CARE( aFrameSel.GetLine(SVX_FRMSELLINE_HOR) ) );
    aBoxInfoItem.SetValid( VALID_VERT, !IS_DONT_CARE( aFrameSel.GetLine(SVX_FRMSELLINE_VER) ) );

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

    //---------
    // Schatten
    //---------
    aShadowItem.SetColor( TpBorderRGBColor( aFrameSel.GetShadowColor() ) );
    aShadowItem.SetWidth( (USHORT)GetCoreValue( aEdShadowSize, eCoreUnit ) );
    SvxShadowLocation eLoc = SVX_SHADOW_NONE;

    switch ( aFrameSel.GetShadowPos() )
    {
        case SVX_FRMSHADOW_BOT_RIGHT:
            eLoc = SVX_SHADOW_BOTTOMRIGHT;
            break;

        case SVX_FRMSHADOW_TOP_RIGHT:
            eLoc = SVX_SHADOW_TOPRIGHT;
            break;

        case SVX_FRMSHADOW_BOT_LEFT:
            eLoc = SVX_SHADOW_BOTTOMLEFT;
            break;

        case SVX_FRMSHADOW_TOP_LEFT:
            eLoc = SVX_SHADOW_TOPLEFT;
            break;
    }
    aShadowItem.SetLocation( eLoc );

    //
    // Put oder Clear des Schattens?
    //
    bPut = TRUE;

    if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nShadowWhich, FALSE ) )
    {
        const SvxShadowItem& rOldShadowItem
            = (const SvxShadowItem&)(rOldSet.Get( nShadowWhich ));

        // der Vergleich von Brushes haut nicht hin, deshalb einzeln:
        if (   ( aShadowItem.GetWidth()    == rOldShadowItem.GetWidth() )
            && ( aShadowItem.GetLocation() == rOldShadowItem.GetLocation() ) )
        {
            Color aCol    = aShadowItem.GetColor();
            Color aOldCol = rOldShadowItem.GetColor();
            bPut = (   aCol.GetRed()   != aOldCol.GetRed()
                    || aCol.GetGreen() != aOldCol.GetGreen()
                    || aCol.GetBlue()  != aOldCol.GetBlue()  );
        }
    }

    if ( bPut )
    {
        if ( aShadowItem.GetLocation() == SVX_SHADOW_NONE )
        {
            // wenn kein Schatten ausgewaehlt wurde, wird das
            // Default-Item genommen:
            const SfxItemPool* pPool = rCoreAttrs.GetPool();

            if ( pPool->IsInRange( nShadowWhich ) )
                aShadowItem =
                    (const SvxShadowItem&)pPool->GetDefaultItem( nShadowWhich );
        }
        pOld = GetOldItem( rCoreAttrs, SID_ATTR_BORDER_SHADOW );

        if ( !pOld || !( *(const SvxShadowItem*)pOld == aShadowItem ) )
        {
            rCoreAttrs.Put( aShadowItem );
            bAttrsChanged |= TRUE;
        }
    }
    else
        rCoreAttrs.ClearItem( nShadowWhich );
    return bAttrsChanged;
}

#undef IS_DONT_CARE

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
    USHORT          nSelId  = aWndPresets.GetSelectItemId();
    SvxLineStruct   theNewStyle;

    if ( nSelId > 1 )
        if (    aLbLineStyle.GetSelectEntryPos() == 0
             || aLbLineStyle.GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND )
            aLbLineStyle.SelectEntryPos( 1 );

    theNewStyle.nLeft   = (short)aLbLineStyle.GetSelectEntryLine1();
    theNewStyle.nMiddle = (short)aLbLineStyle.GetSelectEntryDistance();
    theNewStyle.nRight  = (short)aLbLineStyle.GetSelectEntryLine2();

    switch ( nSelId )
    {
        case 1: // keine Linien
        {
//            aFrameSel.ShowShadow();
            aFrameSel.HideLines();
            aFrameSel.SelectLine( SVX_FRMSELLINE_NONE );
        }
        break;

        case 2: // aussen
        {

            aFrameSel.HideLines();
            static const SvxFrameSelectorLine eTypes[] = {  SVX_FRMSELLINE_NONE,
                                                            SVX_FRMSELLINE_TOP,
                                                            SVX_FRMSELLINE_BOTTOM,
                                                            SVX_FRMSELLINE_LEFT,
                                                            SVX_FRMSELLINE_RIGHT
                                                        };

            sal_Int32 i;
            for (i=1; i < sizeof(eTypes)/sizeof(SvxFrameSelectorLine); ++i)
                aFrameSel.GetLine(eTypes[i])  .SetState( SVX_FRMLINESTATE_SHOW );

            for (i=0; i < sizeof(eTypes)/sizeof(SvxFrameSelectorLine); ++i)
                aFrameSel.SelectLine( eTypes[i] );
        }
        break;

        case 3:
        {
            if ( bIsTableBorder ) // aussen/horizontal
            {
                aFrameSel.HideLines();
                static const SvxFrameSelectorLine eTypes[] = {  SVX_FRMSELLINE_NONE,
                                                                SVX_FRMSELLINE_TOP,
                                                                SVX_FRMSELLINE_BOTTOM,
                                                                SVX_FRMSELLINE_LEFT,
                                                                SVX_FRMSELLINE_RIGHT,
                                                                SVX_FRMSELLINE_HOR
                                                            };

                sal_Int32 i;
                for (i=1; i < sizeof(eTypes)/sizeof(SvxFrameSelectorLine); ++i)
                    aFrameSel.GetLine(eTypes[i])  .SetState( SVX_FRMLINESTATE_SHOW );
                for (i=0; i < sizeof(eTypes)/sizeof(SvxFrameSelectorLine); ++i)
                    aFrameSel.SelectLine( eTypes[i] );
            }
            else // links/rechts
            {
                aFrameSel.HideLines();
                aFrameSel.GetLine(SVX_FRMSELLINE_LEFT) .SetState( SVX_FRMLINESTATE_SHOW );
                aFrameSel.GetLine(SVX_FRMSELLINE_RIGHT).SetState( SVX_FRMLINESTATE_SHOW );
                aFrameSel.SelectLine( SVX_FRMSELLINE_NONE );
                aFrameSel.SelectLine( SVX_FRMSELLINE_LEFT );
                aFrameSel.SelectLine( SVX_FRMSELLINE_RIGHT );
            }
        }
        break;

        case 4:
        {
            if ( bIsTableBorder ) // aussen/hor./ver.
            {
                aFrameSel.HideLines();
                static const SvxFrameSelectorLine eTypes[] = {  SVX_FRMSELLINE_NONE,
                                                                SVX_FRMSELLINE_TOP,
                                                                SVX_FRMSELLINE_BOTTOM,
                                                                SVX_FRMSELLINE_LEFT,
                                                                SVX_FRMSELLINE_RIGHT,
                                                                SVX_FRMSELLINE_HOR,
                                                                SVX_FRMSELLINE_VER
                                                            };

                sal_Int32 i;
                for (i=1; i < sizeof(eTypes)/sizeof(SvxFrameSelectorLine); ++i)
                    aFrameSel.GetLine(eTypes[i])  .SetState( SVX_FRMLINESTATE_SHOW );
                for (i=0; i < sizeof(eTypes)/sizeof(SvxFrameSelectorLine); ++i)
                    aFrameSel.SelectLine( eTypes[i] );
            }
            else // oben/unten
            {
                aFrameSel.HideLines();
                aFrameSel.GetLine(SVX_FRMSELLINE_TOP)   .SetState( SVX_FRMLINESTATE_SHOW );
                aFrameSel.GetLine(SVX_FRMSELLINE_BOTTOM).SetState( SVX_FRMLINESTATE_SHOW );
                aFrameSel.SelectLine( SVX_FRMSELLINE_NONE );
                aFrameSel.SelectLine( SVX_FRMSELLINE_TOP );
                aFrameSel.SelectLine( SVX_FRMSELLINE_BOTTOM );
            }
        }
        break;

        case 5:
        {
            if ( bIsTableBorder ) // Aussen setzen, innen Don't Care
            {
                aFrameSel.HideLines();
                aFrameSel.GetLine(SVX_FRMSELLINE_LEFT)  .SetState( SVX_FRMLINESTATE_SHOW );
                aFrameSel.GetLine(SVX_FRMSELLINE_RIGHT) .SetState( SVX_FRMLINESTATE_SHOW );
                aFrameSel.GetLine(SVX_FRMSELLINE_TOP)   .SetState( SVX_FRMLINESTATE_SHOW );
                aFrameSel.GetLine(SVX_FRMSELLINE_BOTTOM).SetState( SVX_FRMLINESTATE_SHOW );
                aFrameSel.GetLine(SVX_FRMSELLINE_HOR)   .SetState( SVX_FRMLINESTATE_DONT_CARE );
                aFrameSel.GetLine(SVX_FRMSELLINE_VER)   .SetState( SVX_FRMLINESTATE_DONT_CARE );
                aFrameSel.SelectLine( SVX_FRMSELLINE_NONE );
                aFrameSel.SelectLine( SVX_FRMSELLINE_LEFT );
                aFrameSel.SelectLine( SVX_FRMSELLINE_RIGHT );
                aFrameSel.SelectLine( SVX_FRMSELLINE_TOP );
                aFrameSel.SelectLine( SVX_FRMSELLINE_BOTTOM );
            }
            else // links (Absatz-Markierung)
            {
                aFrameSel.HideLines();
                aFrameSel.GetLine(SVX_FRMSELLINE_LEFT).SetState( SVX_FRMLINESTATE_SHOW );
                aFrameSel.SelectLine( SVX_FRMSELLINE_NONE );
                aFrameSel.SelectLine( SVX_FRMSELLINE_LEFT );
            }
        }
        break;
    }

    aFrameSel.SetCurLineStyle( theNewStyle );
    aFrameSel.SetCurLineColor( aLbLineColor.GetSelectEntryColor() );
    aFrameSel.SetShadowColor( aLbShadowColor.GetSelectEntryColor() );
    aFrameSel.ShowLines();
//    aFrameSel.ShowShadow();
    aWndPresets.SetNoSelection(); // Nur Auswahl, kein Status
    LinesChanged_Impl(0);
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxBorderTabPage, SelSdwHdl_Impl, void *, EMPTYARG )
{
    BOOL bEnable = aWndShadows.GetSelectItemId() > 1;
    aFtShadowSize.Enable(bEnable);
    aEdShadowSize.Enable(bEnable);

    SvxFrameShadow ePos = SVX_FRMSHADOW_NONE; // case 1

    switch ( aWndShadows.GetSelectItemId() )
    {
        case 2: ePos = SVX_FRMSHADOW_BOT_RIGHT; break;
        case 3: ePos = SVX_FRMSHADOW_TOP_RIGHT; break;
        case 4: ePos = SVX_FRMSHADOW_BOT_LEFT;  break;
        case 5: ePos = SVX_FRMSHADOW_TOP_LEFT;  break;
    }
    aFrameSel.SetShadowPos( ePos );
//    aFrameSel.ShowShadow();

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxBorderTabPage, SelColHdl_Impl, ListBox *, pLb )
{
    ColorListBox* pColLb = (ColorListBox*)pLb;

    if ( pLb == &aLbLineColor )
    {
        aFrameSel.SetCurLineColor( pColLb->GetSelectEntryColor() );
        aLbLineStyle.SetColor( pColLb->GetSelectEntryColor() );
        aFrameSel.ShowLines();
    }
    else if ( pLb == &aLbShadowColor )
    {
        aFrameSel.SetShadowColor( pColLb->GetSelectEntryColor() );
//        aFrameSel.ShowShadow();
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxBorderTabPage, SelStyleHdl_Impl, ListBox *, pLb )
{
    if ( pLb == &aLbLineStyle )
    {
        SvxLineStruct theNewStyle;

        theNewStyle.nLeft   = (short)aLbLineStyle.GetSelectEntryLine1();
        theNewStyle.nMiddle = (short)aLbLineStyle.GetSelectEntryDistance();
        theNewStyle.nRight  = (short)aLbLineStyle.GetSelectEntryLine2();

        aFrameSel.SetCurLineStyle( theNewStyle );
        aFrameSel.ShowLines();
    }

    return 0;
}

// -----------------------------------------------------------------------
#define MAX_VALUESET_COUNT  5
#define FIRST_VALUESET_ITEM 1

void SvxBorderTabPage::FillValueSets_Impl()
{
    // Initialize presets window
    aWndPresets.SetColCount( MAX_VALUESET_COUNT );
    aWndPresets.SetStyle( aWndPresets.GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER );
    aWndPresets.SetSizePixel(
        aWndPresets.CalcWindowSizePixel( aBorderImgLst.GetImage(IID_PRENONE).GetSizePixel() ) );
    aWndShadows.SetColCount( MAX_VALUESET_COUNT );
    aWndShadows.SetStyle( aWndShadows.GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER );
    aWndShadows.SetPosSizePixel( aWndShadows.GetPosPixel(),
        aWndShadows.CalcWindowSizePixel( aShadowImgLst.GetImage(IID_SHADOWNONE).GetSizePixel() ) );

    for(USHORT i = FIRST_VALUESET_ITEM; i <= MAX_VALUESET_COUNT; i++)
    {
        aWndPresets.InsertItem( i );
        aWndShadows.InsertItem( i );
    }
    aWndPresets.SetNoSelection();
    aWndShadows.SelectItem( FIRST_VALUESET_ITEM );

    InitValueSets_Impl();
    aWndPresets.Show();
    aWndShadows.Show();
}
/* -----------------------------03.06.2002 10:17------------------------------

 ---------------------------------------------------------------------------*/
void SvxBorderTabPage::InitValueSets_Impl()
{
    static const USHORT aTableBorders[] =
    {
        IID_PRENONE,
        IID_TABLE_PRE1,
        IID_TABLE_PRE2,
        IID_TABLE_PRE3,
        IID_TABLE_PRE4
    };
    static const USHORT aParaBorders[] =
    {
        IID_PRENONE,
        IID_PARAGRAPH_PRE1,
        IID_PARAGRAPH_PRE2,
        IID_PARAGRAPH_PRE3,
        IID_PARAGRAPH_PRE4
    };
    static const USHORT aShadows[] =
    {
        IID_SHADOWNONE,
        IID_SHADOW_BOT_RIGHT,
        IID_SHADOW_TOP_RIGHT,
        IID_SHADOW_BOT_LEFT,
        IID_SHADOW_TOP_LEFT
    };

    BOOL bDark = aWndPresets.GetDisplayBackground().GetColor().IsDark();
    ImageList& rBorderImgLst = bDark ? aBorderImgLstH : aBorderImgLst;
    const USHORT * pBorderIds = bIsTableBorder ? aTableBorders : aParaBorders;
    const USHORT nBorderStartId = bIsTableBorder ? RID_SVXSTR_TABLE_PRESET_START : RID_SVXSTR_PARA_PRESET_START;
    for( USHORT nBorder = 0; nBorder < aWndPresets.GetItemCount(); ++nBorder )
    {
        aWndPresets.SetItemImage( nBorder + 1, rBorderImgLst.GetImage( pBorderIds[nBorder] ) );
        aWndPresets.SetItemText ( nBorder + 1, SVX_RESSTR( nBorderStartId + nBorder ) );
    }
    ImageList& rShadowImgLst = bDark ? aShadowImgLstH : aShadowImgLst;
    for ( USHORT nShadow = 0; nShadow < 5; ++nShadow )
    {
        aWndShadows.SetItemImage( nShadow + 1, rShadowImgLst.GetImage( aShadows[nShadow] ) );
        aWndShadows.SetItemText ( nShadow + 1, SVX_RESSTR( RID_SVXSTR_SHADOW_STYLE_START + nShadow ) );
    }
}
// -----------------------------------------------------------------------

void SvxBorderTabPage::FillLineListBox_Impl()
{
    aLbLineStyle.SetUnit( FUNIT_POINT );
    aLbLineStyle.SetSourceUnit( FUNIT_TWIP );

    // Writer 2.0 Defaults:
    aLbLineStyle.InsertEntry( SVX_RESSTR( STR_NONE ) );

    aLbLineStyle.InsertEntry( LINE_WIDTH0 );
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
    if(aLeftMF.IsVisible())
    {
        BOOL bLineSet = aFrameSel.IsAnyLineSet();
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
                nValid = aFrameSel.GetLine(SVX_FRMSELLINE_TOP).GetState() == SVX_FRMLINESTATE_SHOW ? VALID_TOP : 0;
                nValid |= aFrameSel.GetLine(SVX_FRMSELLINE_BOTTOM).GetState() == SVX_FRMLINESTATE_SHOW ? VALID_BOTTOM : 0;
                nValid |= aFrameSel.GetLine(SVX_FRMSELLINE_LEFT).GetState() == SVX_FRMLINESTATE_SHOW ? VALID_LEFT : 0;
                nValid |= aFrameSel.GetLine(SVX_FRMSELLINE_RIGHT).GetState() == SVX_FRMLINESTATE_SHOW ? VALID_RIGHT : 0;
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
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
            InitValueSets_Impl();

    SfxTabPage::DataChanged( rDCEvt );
}

#undef EQSTYLE
#undef SET_STYLE



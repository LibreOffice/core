/*************************************************************************
 *
 *  $RCSfile: labdlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pb $ $Date: 2000-10-12 09:37:00 $
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
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFXMODULE_HXX
#include <sfx2/module.hxx>
#endif
#ifndef _SFX_SAVEOPT_HXX //autogen
#include <sfx2/saveopt.hxx>
#endif
#pragma hdrstop

#define _SVX_LABDLG_CXX

#include "svdattrx.hxx"
#include "dialogs.hrc"
#include "dialmgr.hxx"
#include "dlgutil.hxx"
#include "transfrm.hxx"

#include "labdlg.hrc"
#include "labdlg.hxx"

// define ----------------------------------------------------------------

#define AZ_OPTIMAL      0
#define AZ_VON_OBEN     1
#define AZ_VON_LINKS    2
#define AZ_HORIZONTAL   3
#define AZ_VERTIKAL     4

#define AT_OBEN         0
#define AT_MITTE        1
#define AT_UNTEN        2

#define WK_OPTIMAL      0
#define WK_30           1
#define WK_45           2
#define WK_60           3
#define WK_90           4

// static ----------------------------------------------------------------

static USHORT pCaptionRanges[] =
{
    SDRATTR_CAPTIONTYPE,
    SDRATTR_CAPTIONFIXEDANGLE,
    SDRATTR_CAPTIONANGLE,
    SDRATTR_CAPTIONGAP,
    SDRATTR_CAPTIONESCDIR,
    SDRATTR_CAPTIONESCISREL,
    SDRATTR_CAPTIONESCREL,
    SDRATTR_CAPTIONESCABS,
    SDRATTR_CAPTIONLINELEN,
    SDRATTR_CAPTIONFITLINELEN,
    0
};

// -----------------------------------------------------------------------

SvxCaptionTabPage::SvxCaptionTabPage(Window* pParent, const SfxItemSet& rInAttrs)
 :  SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_CAPTION ), rInAttrs ),

    aCT_CAPTTYPE(       this, ResId( CT_CAPTTYPE ) ),
    aFT_ABSTAND(        this, ResId( FT_ABSTAND ) ),
    aMF_ABSTAND(        this, ResId( MF_ABSTAND ) ),
    aFT_WINKEL(         this, ResId( FT_WINKEL ) ),
    aLB_WINKEL(         this, ResId( LB_WINKEL ) ),
    aFT_ANSATZ(         this, ResId( FT_ANSATZ ) ),
    aLB_ANSATZ(         this, ResId( LB_ANSATZ ) ),
    aFT_UM(             this, ResId( FT_UM ) ),
    aMF_ANSATZ(         this, ResId( MF_ANSATZ ) ),
    aFT_ANSATZ_REL(     this, ResId( FT_ANSATZ_REL ) ),
    aLB_ANSATZ_REL(     this, ResId( LB_ANSATZ_REL ) ),
    aFT_LAENGE(         this, ResId( FT_LAENGE ) ),
    aMF_LAENGE(         this, ResId( MF_LAENGE ) ),
    aCB_LAENGE(         this, ResId( CB_LAENGE ) ),

    aStrHorzList( ResId(STR_HORZ_LIST) ),
    aStrVertList( ResId(STR_VERT_LIST) ),

    rOutAttrs       ( rInAttrs )
{
    //------------NYI-------------------------------------------
    aFT_WINKEL.Hide();
    aLB_WINKEL.Hide();

    //------------Positionen korrigieren-------------------------
    aFT_ANSATZ_REL.SetPosPixel( aFT_UM.GetPosPixel() );
    aLB_ANSATZ_REL.SetPosPixel(
        Point(
            aFT_ANSATZ_REL.GetPosPixel().X()+aFT_ANSATZ_REL.GetSizePixel().Width()+6,
            aLB_ANSATZ.GetPosPixel().Y() )
        );

    aMF_ANSATZ.SetPosPixel(
        Point(
            aFT_UM.GetPosPixel().X()+aFT_UM.GetSizePixel().Width()+6,
            aLB_ANSATZ.GetPosPixel().Y() )
        );

    //------------ValueSet installieren--------------------------
    aCT_CAPTTYPE.SetStyle( aCT_CAPTTYPE.GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER | WB_NAMEFIELD );
    aCT_CAPTTYPE.SetColCount(5);//XXX
    aCT_CAPTTYPE.SetLineCount(1);
    aCT_CAPTTYPE.SetSelectHdl(LINK( this, SvxCaptionTabPage, SelectCaptTypeHdl_Impl));

    aCT_CAPTTYPE.InsertItem(BMP_CAPTTYPE_1,
                        Bitmap(ResId(BMP_CAPTTYPE_1)),
                        String(ResId(STR_CAPTTYPE_1)));

    aCT_CAPTTYPE.InsertItem(BMP_CAPTTYPE_2,
                        Bitmap(ResId(BMP_CAPTTYPE_2)),
                        String(ResId(STR_CAPTTYPE_2)));

    aCT_CAPTTYPE.InsertItem(BMP_CAPTTYPE_3,
                        Bitmap(ResId(BMP_CAPTTYPE_3)),
                        String(ResId(STR_CAPTTYPE_3)));

/*--------------NYI----------------------------------------------
    aCT_CAPTTYPE.InsertItem(BMP_CAPTTYPE_4,
                        Bitmap(ResId(BMP_CAPTTYPE_4)),
                        String(ResId(STR_CAPTTYPE_4)));
*/

    aLB_ANSATZ.SetSelectHdl(LINK(this,SvxCaptionTabPage,AnsatzSelectHdl_Impl));
    aLB_ANSATZ_REL.SetSelectHdl(LINK(this,SvxCaptionTabPage,AnsatzRelSelectHdl_Impl));
    aCB_LAENGE.SetClickHdl(LINK(this,SvxCaptionTabPage,LineOptHdl_Impl));

    FreeResource();
}

// -----------------------------------------------------------------------

void SvxCaptionTabPage::Construct()
{
    // Setzen des Rechtecks und der Workingarea
    DBG_ASSERT( pView, "Keine gueltige View Uebergeben!" );
}

// -----------------------------------------------------------------------

BOOL SvxCaptionTabPage::FillItemSet( SfxItemSet& rOutAttrs )
{
    SfxItemPool*    pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );

    SfxMapUnit      eUnit;

    nCaptionType = aCT_CAPTTYPE.GetSelectItemId()-1;

    rOutAttrs.Put( SdrCaptionTypeItem( (SdrCaptionType) nCaptionType ) );

    if( aMF_ABSTAND.IsValueModified() )
    {
        eUnit = pPool->GetMetric( GetWhich( SDRATTR_CAPTIONGAP ) );
        rOutAttrs.Put( SdrCaptionGapItem( GetCoreValue(aMF_ABSTAND, eUnit ) ) );
    }

    // Sonderbehandlung!!! XXX
    if( nCaptionType==SDRCAPT_TYPE1 )
    {
        switch( nEscDir )
        {
            case SDRCAPT_ESCHORIZONTAL:     nEscDir=SDRCAPT_ESCVERTICAL;break;
            case SDRCAPT_ESCVERTICAL:       nEscDir=SDRCAPT_ESCHORIZONTAL;break;
        }
    }

    rOutAttrs.Put( SdrCaptionEscDirItem( (SdrCaptionEscDir)nEscDir ) );

    bEscRel = aLB_ANSATZ_REL.IsVisible();
    rOutAttrs.Put( SdrCaptionEscIsRelItem( bEscRel ) );

    if( bEscRel )
    {
        long    nVal = 0;

        switch( aLB_ANSATZ_REL.GetSelectEntryPos() )
        {
            case AT_OBEN:   nVal=0;break;
            case AT_MITTE:  nVal=5000;break;
            case AT_UNTEN:  nVal=10000;break;
        }
        rOutAttrs.Put( SdrCaptionEscRelItem( nVal ) );
    }
    else
    {
        if( aMF_ANSATZ.IsValueModified() )
        {
            eUnit = pPool->GetMetric( GetWhich( SDRATTR_CAPTIONESCABS ) );
            rOutAttrs.Put( SdrCaptionEscAbsItem( GetCoreValue(aMF_ANSATZ, eUnit ) ) );
        }
    }

    bFitLineLen = aCB_LAENGE.IsChecked();
    rOutAttrs.Put( SdrCaptionFitLineLenItem( bFitLineLen ) );

    if( ! bFitLineLen )
    {
        if( aMF_LAENGE.IsValueModified() )
        {
            eUnit = pPool->GetMetric( GetWhich( SDRATTR_CAPTIONLINELEN ) );
            rOutAttrs.Put( SdrCaptionLineLenItem( GetCoreValue(aMF_LAENGE, eUnit ) ) );
        }
    }

//NYI-------------die Winkel muessen noch hier rein!!! XXX----------------------

    return( TRUE );
}

// -----------------------------------------------------------------------

void SvxCaptionTabPage::Reset( const SfxItemSet& rOutAttrs )
{

    //------------Metrik einstellen-----------------------------

    FieldUnit eFUnit = GetModuleFieldUnit();

    switch ( eFUnit )
    {
        case FUNIT_CM:
        case FUNIT_M:
        case FUNIT_KM:
            eFUnit = FUNIT_MM;
            break;
    }
    SetFieldUnit( aMF_ABSTAND, eFUnit );
    SetFieldUnit( aMF_ANSATZ, eFUnit );
    SetFieldUnit( aMF_LAENGE, eFUnit );

    SfxItemPool*    pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );

    USHORT          nWhich;
    SfxMapUnit      eUnit;

    //------- Winkel ----------
    nWhich = GetWhich( SDRATTR_CAPTIONANGLE );
    nFixedAngle = ( ( const SdrCaptionAngleItem& ) rOutAttrs.Get( nWhich ) ).GetValue();

    //------- absolute Ansatzentfernung ----------
    nWhich = GetWhich( SDRATTR_CAPTIONESCABS );
    eUnit = pPool->GetMetric( nWhich );
    nEscAbs = ( ( const SdrCaptionEscAbsItem& ) rOutAttrs.Get( nWhich ) ).GetValue();
    SetMetricValue( aMF_ANSATZ, nEscAbs, eUnit );
    nEscAbs = aMF_ANSATZ.GetValue();

    //------- relative Ansatzentfernung ----------
    nWhich = GetWhich( SDRATTR_CAPTIONESCREL );
    nEscRel = (long)( ( const SdrCaptionEscRelItem& ) rOutAttrs.Get( nWhich ) ).GetValue();

    //------- Linienlaenge ----------
    nWhich = GetWhich( SDRATTR_CAPTIONLINELEN );
    eUnit = pPool->GetMetric( nWhich );
    nLineLen = ( ( const SdrCaptionLineLenItem& ) rOutAttrs.Get( nWhich ) ).GetValue();
    SetMetricValue( aMF_LAENGE, nLineLen, eUnit );
    nLineLen = aMF_LAENGE.GetValue();

    //------- Abstand zur Box ----------
    nWhich = GetWhich( SDRATTR_CAPTIONGAP );
    eUnit = pPool->GetMetric( nWhich );
    nGap = ( ( const SdrCaptionGapItem& ) rOutAttrs.Get( nWhich ) ).GetValue();
    SetMetricValue( aMF_ABSTAND, nGap, eUnit );
    nGap = aMF_ABSTAND.GetValue();

    nCaptionType = (short)( ( const SdrCaptionTypeItem& ) rOutAttrs.Get( GetWhich( SDRATTR_CAPTIONTYPE ) ) ).GetValue();
    bFixedAngle = ( ( const SfxBoolItem& ) rOutAttrs.Get( GetWhich( SDRATTR_CAPTIONFIXEDANGLE ) ) ).GetValue();
    bFitLineLen = ( ( const SfxBoolItem& ) rOutAttrs.Get( GetWhich( SDRATTR_CAPTIONFITLINELEN ) ) ).GetValue();
    nEscDir = (short)( ( const SdrCaptionEscDirItem& ) rOutAttrs.Get( GetWhich( SDRATTR_CAPTIONESCDIR ) ) ).GetValue();
    bEscRel = ( ( const SfxBoolItem& ) rOutAttrs.Get( GetWhich( SDRATTR_CAPTIONESCISREL ) ) ).GetValue();

    // Sonderbehandlung!!! XXX
    if( nCaptionType==SDRCAPT_TYPE1 )
    {
        switch( nEscDir )
        {
            case SDRCAPT_ESCHORIZONTAL:     nEscDir=SDRCAPT_ESCVERTICAL;break;
            case SDRCAPT_ESCVERTICAL:       nEscDir=SDRCAPT_ESCHORIZONTAL;break;
        }
    }

    nAnsatzRelPos=AT_MITTE;
    nAnsatzTypePos=AZ_OPTIMAL;
    nWinkelTypePos=WK_OPTIMAL;

    aMF_ABSTAND.SetValue( nGap );

    if( nEscDir == SDRCAPT_ESCHORIZONTAL )
    {
        if( bEscRel )
        {
            if( nEscRel < 3333 )
                nAnsatzRelPos = AT_OBEN;
            if( nEscRel > 6666 )
                nAnsatzRelPos = AT_UNTEN;
            nAnsatzTypePos = AZ_HORIZONTAL;
        }
        else
        {
            nAnsatzTypePos = AZ_VON_OBEN;
            aMF_ANSATZ.SetValue( nEscAbs );
        }
    }
    else if( nEscDir == SDRCAPT_ESCVERTICAL )
    {
        if( bEscRel )
        {
            if( nEscRel < 3333 )
                nAnsatzRelPos = AT_OBEN;
            if( nEscRel > 6666 )
                nAnsatzRelPos = AT_UNTEN;
            nAnsatzTypePos = AZ_VERTIKAL;
        }
        else
        {
            nAnsatzTypePos = AZ_VON_LINKS;
            aMF_ANSATZ.SetValue( nEscAbs );
        }
    }
    else if( nEscDir == SDRCAPT_ESCBESTFIT )
    {
        nAnsatzTypePos = AZ_OPTIMAL;
    }

    if( bFixedAngle )
    {
        if( nFixedAngle <= 3000 )
            nWinkelTypePos=WK_30;
        else if( nFixedAngle <= 4500 )
            nWinkelTypePos=WK_45;
        else if( nFixedAngle <= 6000 )
            nWinkelTypePos=WK_60;
        else
            nWinkelTypePos=WK_90;
    }

    aCB_LAENGE.Check( bFitLineLen );
    aMF_LAENGE.SetValue( nLineLen );

    aLB_ANSATZ.SelectEntryPos( nAnsatzTypePos );
    aLB_WINKEL.SelectEntryPos( nWinkelTypePos );

    SetupAnsatz_Impl( nAnsatzTypePos );
    aCT_CAPTTYPE.SelectItem( nCaptionType+1 );// Enum beginnt bei 0!
    SetupType_Impl( nCaptionType+1 );
}

// -----------------------------------------------------------------------

SfxTabPage* SvxCaptionTabPage::Create( Window* pWindow,
                const SfxItemSet& rOutAttrs )
{
    return( new SvxCaptionTabPage( pWindow, rOutAttrs ) );
}

//------------------------------------------------------------------------

USHORT* SvxCaptionTabPage::GetRanges()
{
    return( pCaptionRanges );
}

//------------------------------------------------------------------------

void SvxCaptionTabPage::SetupAnsatz_Impl( USHORT nType )
{
    xub_StrLen  nCnt=0, nIdx=0;

    switch( nType )
    {
        case AZ_OPTIMAL:
//      aMF_ANSATZ.Hide(); //XXX auch bei OPTIMAL werden Abswerte genommen
//      aFT_UM.Hide();
        aMF_ANSATZ.Show();
        aFT_UM.Show();
        aFT_ANSATZ_REL.Hide();
        aLB_ANSATZ_REL.Hide();
        nEscDir = SDRCAPT_ESCBESTFIT;
        break;

        case AZ_VON_OBEN:
        aMF_ANSATZ.Show();
        aFT_UM.Show();
        aFT_ANSATZ_REL.Hide();
        aLB_ANSATZ_REL.Hide();
        nEscDir = SDRCAPT_ESCHORIZONTAL;
        break;

        case AZ_VON_LINKS:
        aMF_ANSATZ.Show();
        aFT_UM.Show();
        aFT_ANSATZ_REL.Hide();
        aLB_ANSATZ_REL.Hide();
        nEscDir = SDRCAPT_ESCVERTICAL;
        break;

        case AZ_HORIZONTAL:
        aLB_ANSATZ_REL.Clear();
        nCnt = aStrHorzList.GetTokenCount();
        for( nIdx=0 ; nIdx<nCnt ; nIdx++ )
            aLB_ANSATZ_REL.InsertEntry( aStrHorzList.GetToken(nIdx) );
        aLB_ANSATZ_REL.SelectEntryPos( nAnsatzRelPos );

        aMF_ANSATZ.Hide();
        aFT_UM.Hide();
        aFT_ANSATZ_REL.Show();
        aLB_ANSATZ_REL.Show();
        nEscDir = SDRCAPT_ESCHORIZONTAL;
        break;

        case AZ_VERTIKAL:
        aLB_ANSATZ_REL.Clear();
        nCnt = aStrVertList.GetTokenCount();
        for( nIdx=0 ; nIdx<nCnt ; nIdx++ )
            aLB_ANSATZ_REL.InsertEntry( aStrVertList.GetToken(nIdx) );
        aLB_ANSATZ_REL.SelectEntryPos( nAnsatzRelPos );

        aMF_ANSATZ.Hide();
        aFT_UM.Hide();
        aFT_ANSATZ_REL.Show();
        aLB_ANSATZ_REL.Show();
        nEscDir = SDRCAPT_ESCVERTICAL;
        break;
    }
}

//------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxCaptionTabPage, AnsatzSelectHdl_Impl, ListBox *, pListBox )
{
    if( pListBox == &aLB_ANSATZ )
    {
        SetupAnsatz_Impl( aLB_ANSATZ.GetSelectEntryPos() );
    }
    return 0;
}
IMPL_LINK_INLINE_END( SvxCaptionTabPage, AnsatzSelectHdl_Impl, ListBox *, pListBox )

//------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxCaptionTabPage, AnsatzRelSelectHdl_Impl, ListBox *, pListBox )
{
    if( pListBox == &aLB_ANSATZ_REL )
    {
        nAnsatzRelPos = aLB_ANSATZ_REL.GetSelectEntryPos();
    }
    return 0;
}
IMPL_LINK_INLINE_END( SvxCaptionTabPage, AnsatzRelSelectHdl_Impl, ListBox *, pListBox )

//------------------------------------------------------------------------

IMPL_LINK( SvxCaptionTabPage, LineOptHdl_Impl, Button *, pButton )
{
    if( pButton == &aCB_LAENGE )
    {
        if( aCB_LAENGE.IsChecked() || ! aCB_LAENGE.IsEnabled() )
        {
            aMF_LAENGE.Disable();
        }
        else
        {
            aMF_LAENGE.Enable();
        }
    }
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxCaptionTabPage, SelectCaptTypeHdl_Impl, void *, EMPTYARG )
{
    SetupType_Impl( aCT_CAPTTYPE.GetSelectItemId() );
    return 0;
}
IMPL_LINK_INLINE_END( SvxCaptionTabPage, SelectCaptTypeHdl_Impl, void *, EMPTYARG )

//------------------------------------------------------------------------

void SvxCaptionTabPage::SetupType_Impl( USHORT nType )
{
    switch( nType-1 )
    {
        case SDRCAPT_TYPE1:
        aLB_WINKEL.Disable();
        aCB_LAENGE.Disable();
        LineOptHdl_Impl( &aCB_LAENGE );
        break;

        case SDRCAPT_TYPE2:
        aLB_WINKEL.Enable();
        aCB_LAENGE.Disable();
        LineOptHdl_Impl( &aCB_LAENGE );
        break;

        case SDRCAPT_TYPE3:
        aLB_WINKEL.Enable();
        aCB_LAENGE.Enable();
        LineOptHdl_Impl( &aCB_LAENGE );
        break;

        case SDRCAPT_TYPE4:
        aLB_WINKEL.Enable();
        aCB_LAENGE.Enable();
        LineOptHdl_Impl( &aCB_LAENGE );
        break;
    }
}

//========================================================================


SvxCaptionTabDialog::SvxCaptionTabDialog(Window* pParent, const SdrView* pSdrView, USHORT nAnchorTypes)
 :  SfxTabDialog( pParent, SVX_RES( RID_SVXDLG_CAPTION ) ),
    nAnchorCtrls(nAnchorTypes),
    pView       ( pSdrView )
{
    FreeResource();

    DBG_ASSERT( pView, "Keine gueltige View Uebergeben!" );

    AddTabPage( RID_SVXPAGE_POSITION, SvxPositionTabPage::Create,
                            SvxPositionTabPage::GetRanges );
    AddTabPage( RID_SVXPAGE_SIZE, SvxSizeTabPage::Create,
                            SvxSizeTabPage::GetRanges );
    AddTabPage( RID_SVXPAGE_CAPTION, SvxCaptionTabPage::Create,
                            SvxCaptionTabPage::GetRanges );
}

// -----------------------------------------------------------------------

SvxCaptionTabDialog::~SvxCaptionTabDialog()
{
}

// -----------------------------------------------------------------------

void SvxCaptionTabDialog::PageCreated( USHORT nId, SfxTabPage &rPage )
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

        case RID_SVXPAGE_CAPTION:
            ( (SvxCaptionTabPage&) rPage ).SetView( pView );
            ( (SvxCaptionTabPage&) rPage ).Construct();
        break;
    }
}



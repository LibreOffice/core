/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <comphelper/string.hxx>
#include <tools/shl.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <swpossizetabpage.hxx>
#include <svx/dialogs.hrc>
#define _SVX_LABDLG_CXX

#include <svx/svdattrx.hxx>
#include <cuires.hrc>
#include <dialmgr.hxx>
#include "svx/dlgutil.hxx"
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

static sal_uInt16 pCaptionRanges[] =
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
 :  SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_CAPTION ), rInAttrs ),

    aCT_CAPTTYPE(       this, CUI_RES( CT_CAPTTYPE ) ),
    aFT_ABSTAND(        this, CUI_RES( FT_ABSTAND ) ),
    aMF_ABSTAND(        this, CUI_RES( MF_ABSTAND ) ),
    aFT_WINKEL(         this, CUI_RES( FT_WINKEL ) ),
    aLB_WINKEL(         this, CUI_RES( LB_WINKEL ) ),
    aFT_ANSATZ(         this, CUI_RES( FT_ANSATZ ) ),
    aLB_ANSATZ(         this, CUI_RES( LB_ANSATZ ) ),
    aFT_UM(             this, CUI_RES( FT_UM ) ),
    aMF_ANSATZ(         this, CUI_RES( MF_ANSATZ ) ),
    aFT_ANSATZ_REL(     this, CUI_RES( FT_ANSATZ_REL ) ),
    aLB_ANSATZ_REL(     this, CUI_RES( LB_ANSATZ_REL ) ),
    aFT_LAENGE(         this, CUI_RES( FT_LAENGE ) ),
    aMF_LAENGE(         this, CUI_RES( MF_LAENGE ) ),
    aCB_LAENGE(         this, CUI_RES( CB_LAENGE ) ),

    aStrHorzList( CUI_RES(STR_HORZ_LIST) ),
    aStrVertList( CUI_RES(STR_VERT_LIST) ),

    rOutAttrs       ( rInAttrs )
{
    //------------NYI-------------------------------------------
    aFT_WINKEL.Hide();
    aLB_WINKEL.Hide();

    //------------correct positions-------------------------
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

    sal_uInt16 nBitmap;
    for( nBitmap = 0; nBitmap < CAPTYPE_BITMAPS_COUNT; nBitmap++ )
        mpBmpCapTypes[nBitmap]  = new Image(Bitmap(CUI_RES(BMP_CAPTTYPE_1   + nBitmap)), COL_LIGHTMAGENTA );

    //------------install ValueSet--------------------------
    aCT_CAPTTYPE.SetStyle( aCT_CAPTTYPE.GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER | WB_NAMEFIELD );
    aCT_CAPTTYPE.SetColCount(5);//XXX
    aCT_CAPTTYPE.SetLineCount(1);
    aCT_CAPTTYPE.SetSelectHdl(LINK( this, SvxCaptionTabPage, SelectCaptTypeHdl_Impl));

    Image aImage;
    aCT_CAPTTYPE.InsertItem(BMP_CAPTTYPE_1, aImage, String(CUI_RES(STR_CAPTTYPE_1)));
    aCT_CAPTTYPE.InsertItem(BMP_CAPTTYPE_2, aImage, String(CUI_RES(STR_CAPTTYPE_2)));
    aCT_CAPTTYPE.InsertItem(BMP_CAPTTYPE_3, aImage, String(CUI_RES(STR_CAPTTYPE_3)));

    FillValueSet();

    aLB_ANSATZ.SetSelectHdl(LINK(this,SvxCaptionTabPage,AnsatzSelectHdl_Impl));
    aLB_ANSATZ_REL.SetSelectHdl(LINK(this,SvxCaptionTabPage,AnsatzRelSelectHdl_Impl));
    aCB_LAENGE.SetClickHdl(LINK(this,SvxCaptionTabPage,LineOptHdl_Impl));

    FreeResource();
}

// -----------------------------------------------------------------------

SvxCaptionTabPage::~SvxCaptionTabPage()
{
    sal_uInt16 nBitmap;
    for( nBitmap = 0; nBitmap < CAPTYPE_BITMAPS_COUNT; nBitmap++ )
        delete mpBmpCapTypes[nBitmap];
}

// -----------------------------------------------------------------------

void SvxCaptionTabPage::Construct()
{
    // set rectangle and working area
    DBG_ASSERT( pView, "Keine gueltige View Uebergeben!" );
}

// -----------------------------------------------------------------------

sal_Bool SvxCaptionTabPage::FillItemSet( SfxItemSet&  _rOutAttrs)
{
    SfxItemPool*    pPool = _rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );

    SfxMapUnit      eUnit;

    nCaptionType = aCT_CAPTTYPE.GetSelectItemId()-1;

    _rOutAttrs.Put( SdrCaptionTypeItem( (SdrCaptionType) nCaptionType ) );

    if( aMF_ABSTAND.IsValueModified() )
    {
        eUnit = pPool->GetMetric( GetWhich( SDRATTR_CAPTIONGAP ) );
        _rOutAttrs.Put( SdrCaptionGapItem( GetCoreValue(aMF_ABSTAND, eUnit ) ) );
    }

    // special treatment!!! XXX
    if( nCaptionType==SDRCAPT_TYPE1 )
    {
        switch( nEscDir )
        {
            case SDRCAPT_ESCHORIZONTAL:     nEscDir=SDRCAPT_ESCVERTICAL;break;
            case SDRCAPT_ESCVERTICAL:       nEscDir=SDRCAPT_ESCHORIZONTAL;break;
        }
    }

    _rOutAttrs.Put( SdrCaptionEscDirItem( (SdrCaptionEscDir)nEscDir ) );

    bEscRel = aLB_ANSATZ_REL.IsVisible();
    _rOutAttrs.Put( SdrCaptionEscIsRelItem( bEscRel ) );

    if( bEscRel )
    {
        long    nVal = 0;

        switch( aLB_ANSATZ_REL.GetSelectEntryPos() )
        {
            case AT_OBEN:   nVal=0;break;
            case AT_MITTE:  nVal=5000;break;
            case AT_UNTEN:  nVal=10000;break;
        }
        _rOutAttrs.Put( SdrCaptionEscRelItem( nVal ) );
    }
    else
    {
        if( aMF_ANSATZ.IsValueModified() )
        {
            eUnit = pPool->GetMetric( GetWhich( SDRATTR_CAPTIONESCABS ) );
            _rOutAttrs.Put( SdrCaptionEscAbsItem( GetCoreValue(aMF_ANSATZ, eUnit ) ) );
        }
    }

    bFitLineLen = aCB_LAENGE.IsChecked();
    _rOutAttrs.Put( SdrCaptionFitLineLenItem( bFitLineLen ) );

    if( ! bFitLineLen )
    {
        if( aMF_LAENGE.IsValueModified() )
        {
            eUnit = pPool->GetMetric( GetWhich( SDRATTR_CAPTIONLINELEN ) );
            _rOutAttrs.Put( SdrCaptionLineLenItem( GetCoreValue(aMF_LAENGE, eUnit ) ) );
        }
    }

//NYI-------------the angles have to be added here!!! XXX----------------------

    return( sal_True );
}

// -----------------------------------------------------------------------

void SvxCaptionTabPage::Reset( const SfxItemSet&  )
{

    //------------set metric-----------------------------

    FieldUnit eFUnit = GetModuleFieldUnit( rOutAttrs );

    switch ( eFUnit )
    {
        case FUNIT_CM:
        case FUNIT_M:
        case FUNIT_KM:
            eFUnit = FUNIT_MM;
            break;
        default: ;//prevent warning
    }
    SetFieldUnit( aMF_ABSTAND, eFUnit );
    SetFieldUnit( aMF_ANSATZ, eFUnit );
    SetFieldUnit( aMF_LAENGE, eFUnit );

    SfxItemPool*    pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );

    sal_uInt16          nWhich;
    SfxMapUnit      eUnit;

    //------- angle ----------
    nWhich = GetWhich( SDRATTR_CAPTIONANGLE );
    nFixedAngle = ( ( const SdrCaptionAngleItem& ) rOutAttrs.Get( nWhich ) ).GetValue();

    nWhich = GetWhich( SDRATTR_CAPTIONESCABS );
    eUnit = pPool->GetMetric( nWhich );
    nEscAbs = ( ( const SdrCaptionEscAbsItem& ) rOutAttrs.Get( nWhich ) ).GetValue();
    SetMetricValue( aMF_ANSATZ, nEscAbs, eUnit );
    nEscAbs = static_cast<long>(aMF_ANSATZ.GetValue());

    nWhich = GetWhich( SDRATTR_CAPTIONESCREL );
    nEscRel = (long)( ( const SdrCaptionEscRelItem& ) rOutAttrs.Get( nWhich ) ).GetValue();

    //------- line length ----------
    nWhich = GetWhich( SDRATTR_CAPTIONLINELEN );
    eUnit = pPool->GetMetric( nWhich );
    nLineLen = ( ( const SdrCaptionLineLenItem& ) rOutAttrs.Get( nWhich ) ).GetValue();
    SetMetricValue( aMF_LAENGE, nLineLen, eUnit );
    nLineLen = static_cast<long>(aMF_LAENGE.GetValue());

    //------- distance to box ----------
    nWhich = GetWhich( SDRATTR_CAPTIONGAP );
    eUnit = pPool->GetMetric( nWhich );
    nGap = ( ( const SdrCaptionGapItem& ) rOutAttrs.Get( nWhich ) ).GetValue();
    SetMetricValue( aMF_ABSTAND, nGap, eUnit );
    nGap = static_cast<long>(aMF_ABSTAND.GetValue());

    nCaptionType = (short)( ( const SdrCaptionTypeItem& ) rOutAttrs.Get( GetWhich( SDRATTR_CAPTIONTYPE ) ) ).GetValue();
    bFixedAngle = ( ( const SfxBoolItem& ) rOutAttrs.Get( GetWhich( SDRATTR_CAPTIONFIXEDANGLE ) ) ).GetValue();
    bFitLineLen = ( ( const SfxBoolItem& ) rOutAttrs.Get( GetWhich( SDRATTR_CAPTIONFITLINELEN ) ) ).GetValue();
    nEscDir = (short)( ( const SdrCaptionEscDirItem& ) rOutAttrs.Get( GetWhich( SDRATTR_CAPTIONESCDIR ) ) ).GetValue();
    bEscRel = ( ( const SfxBoolItem& ) rOutAttrs.Get( GetWhich( SDRATTR_CAPTIONESCISREL ) ) ).GetValue();

    // special treatment!!! XXX
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
    aCT_CAPTTYPE.SelectItem( nCaptionType+1 ); // Enum starts at 0!
    SetupType_Impl( nCaptionType+1 );
}

// -----------------------------------------------------------------------

SfxTabPage* SvxCaptionTabPage::Create( Window* pWindow,
                const SfxItemSet& rOutAttrs )
{
    return( new SvxCaptionTabPage( pWindow, rOutAttrs ) );
}

//------------------------------------------------------------------------

sal_uInt16* SvxCaptionTabPage::GetRanges()
{
    return( pCaptionRanges );
}

//------------------------------------------------------------------------

void SvxCaptionTabPage::SetupAnsatz_Impl( sal_uInt16 nType )
{
    xub_StrLen  nCnt=0, nIdx=0;

    switch( nType )
    {
        case AZ_OPTIMAL:
//      aMF_ANSATZ.Hide(); //XXX in case of OPTIMAL also absolute values are taken
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
        nCnt = comphelper::string::getTokenCount(aStrHorzList, ';');
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
        nCnt = comphelper::string::getTokenCount(aStrVertList, ';');
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
            aFT_LAENGE.Disable();
            aMF_LAENGE.Disable();
        }
        else
        {
            aFT_LAENGE.Enable();
            aMF_LAENGE.Enable();
        }
    }
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG_INLINE_START(SvxCaptionTabPage, SelectCaptTypeHdl_Impl)
{
    SetupType_Impl( aCT_CAPTTYPE.GetSelectItemId() );
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SvxCaptionTabPage, SelectCaptTypeHdl_Impl)

//------------------------------------------------------------------------

void SvxCaptionTabPage::SetupType_Impl( sal_uInt16 nType )
{
    switch( nType-1 )
    {
        case SDRCAPT_TYPE1:
        aFT_WINKEL.Disable();
        aLB_WINKEL.Disable();
        aFT_LAENGE.Disable();
        aCB_LAENGE.Disable();
        LineOptHdl_Impl( &aCB_LAENGE );
        break;

        case SDRCAPT_TYPE2:
        aFT_WINKEL.Enable();
        aLB_WINKEL.Enable();
        aFT_LAENGE.Disable();
        aCB_LAENGE.Disable();
        LineOptHdl_Impl( &aCB_LAENGE );
        break;

        case SDRCAPT_TYPE3:
        aFT_WINKEL.Enable();
        aLB_WINKEL.Enable();
        aFT_LAENGE.Enable();
        aCB_LAENGE.Enable();
        LineOptHdl_Impl( &aCB_LAENGE );
        break;

        case SDRCAPT_TYPE4:
        aFT_WINKEL.Enable();
        aLB_WINKEL.Enable();
        aFT_LAENGE.Enable();
        aCB_LAENGE.Enable();
        LineOptHdl_Impl( &aCB_LAENGE );
        break;
    }
}

// -----------------------------------------------------------------------

void SvxCaptionTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxTabPage::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
            FillValueSet();
}

// -----------------------------------------------------------------------

void SvxCaptionTabPage::FillValueSet()
{
    Image** ppBitmaps = mpBmpCapTypes;
    aCT_CAPTTYPE.SetItemImage(BMP_CAPTTYPE_1, *(ppBitmaps[0]) );
    aCT_CAPTTYPE.SetItemImage(BMP_CAPTTYPE_2, *(ppBitmaps[1]) );
    aCT_CAPTTYPE.SetItemImage(BMP_CAPTTYPE_3, *(ppBitmaps[2]) );
}

//========================================================================


SvxCaptionTabDialog::SvxCaptionTabDialog(Window* pParent, const SdrView* pSdrView, sal_uInt16 nAnchorTypes)
 :  SfxTabDialog( pParent, CUI_RES( RID_SVXDLG_CAPTION ) ),
    pView       ( pSdrView ),
    nAnchorCtrls(nAnchorTypes)
{
    FreeResource();

    DBG_ASSERT( pView, "Keine gueltige View Uebergeben!" );

    //different positioning page in Writer
    if(nAnchorCtrls & 0x00ff )
    {
        AddTabPage( RID_SVXPAGE_SWPOSSIZE, SvxSwPosSizeTabPage::Create,
                                SvxSwPosSizeTabPage::GetRanges );
        RemoveTabPage( RID_SVXPAGE_POSITION_SIZE);
    }
    else
    {
        AddTabPage( RID_SVXPAGE_POSITION_SIZE, SvxPositionSizeTabPage::Create,
                                SvxPositionSizeTabPage::GetRanges );
        RemoveTabPage( RID_SVXPAGE_SWPOSSIZE );
    }
    AddTabPage( RID_SVXPAGE_CAPTION, SvxCaptionTabPage::Create,
                            SvxCaptionTabPage::GetRanges );
}

// -----------------------------------------------------------------------

SvxCaptionTabDialog::~SvxCaptionTabDialog()
{
}

// -----------------------------------------------------------------------

void SvxCaptionTabDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    switch( nId )
    {
        case RID_SVXPAGE_POSITION_SIZE:
            ( (SvxPositionSizeTabPage&) rPage ).SetView( pView );
            ( (SvxPositionSizeTabPage&) rPage ).Construct();
            if( nAnchorCtrls & SVX_OBJ_NORESIZE )
                ( (SvxPositionSizeTabPage&) rPage ).DisableResize();

            if( nAnchorCtrls & SVX_OBJ_NOPROTECT )
                ( (SvxPositionSizeTabPage&) rPage ).DisableProtect();
        break;
        case RID_SVXPAGE_SWPOSSIZE :
        {
            SvxSwPosSizeTabPage& rSwPage = static_cast<SvxSwPosSizeTabPage&>(rPage);
            rSwPage.EnableAnchorTypes(nAnchorCtrls);
            rSwPage.SetValidateFramePosLink( aValidateLink );
        }
        break;

        case RID_SVXPAGE_CAPTION:
            ( (SvxCaptionTabPage&) rPage ).SetView( pView );
            ( (SvxCaptionTabPage&) rPage ).Construct();
        break;
    }
}

void SvxCaptionTabDialog::SetValidateFramePosLink( const Link& rLink )
{
    aValidateLink = rLink;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

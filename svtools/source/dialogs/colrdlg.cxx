/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <svtools/svtdata.hxx>
#include "colrdlg.hrc"
#include <svtools/colrdlg.hxx>

// ---------------
// - ColorDialog -
// ---------------

SvColorDialog::SvColorDialog( Window* pWindow ) :
        ModalDialog     ( pWindow, SvtResId( DLG_COLOR ) ),
        maColMixCtrl    ( this, SvtResId( VAL_SET_COLOR ), 8, 8 ),
        maBtn1          ( this, SvtResId( BTN_1 ) ),
        maBtn2          ( this, SvtResId( BTN_2 ) ),
        //maBtn3            ( this, SvtResId( BTN_3 ) ),
        //maBtn4            ( this, SvtResId( BTN_4 ) ),
        //maFtRGB           ( this, SvtResId( FT_RGB ) ),
        maCtlColor      ( this, SvtResId( CTL_COLOR ) ),

        maFtCyan        ( this, SvtResId( FT_CYAN ) ),
        maNumCyan       ( this, SvtResId( NUM_CYAN ) ),
        maFtMagenta     ( this, SvtResId( FT_MAGENTA ) ),
        maNumMagenta    ( this, SvtResId( NUM_MAGENTA ) ),
        maFtYellow      ( this, SvtResId( FT_YELLOW ) ),
        maNumYellow     ( this, SvtResId( NUM_YELLOW ) ),
        maFtKey         ( this, SvtResId( FT_KEY ) ),
        maNumKey        ( this, SvtResId( NUM_KEY ) ),

        maFtRed         ( this, SvtResId( FT_RED ) ),
        maNumRed        ( this, SvtResId( NUM_RED ) ),
        maFtGreen       ( this, SvtResId( FT_GREEN ) ),
        maNumGreen      ( this, SvtResId( NUM_GREEN ) ),
        maFtBlue        ( this, SvtResId( FT_BLUE ) ),
        maNumBlue       ( this, SvtResId( NUM_BLUE ) ),

        maFtHue         ( this, SvtResId( FT_HUE ) ),
        maNumHue        ( this, SvtResId( NUM_HUE ) ),
        maFtSaturation  ( this, SvtResId( FT_SATURATION ) ),
        maNumSaturation ( this, SvtResId( NUM_SATURATION ) ),
        maFtLuminance   ( this, SvtResId( FT_LUMINANCE ) ),
        maNumLuminance  ( this, SvtResId( NUM_LUMINANCE ) ),

        maCtlPreview    ( this, SvtResId( CTL_PREVIEW ) ),
        maCtlPreviewOld ( this, SvtResId( CTL_PREVIEW_OLD ) ),

        maBtnOK         ( this, SvtResId( BTN_OK ) ),
        maBtnCancel     ( this, SvtResId( BTN_CANCEL ) ),
        maBtnHelp       ( this, SvtResId( BTN_HELP ) )
{
    FreeResource();

    maColMixCtrl.SetDoubleClickHdl( LINK( this, SvColorDialog, ClickMixCtrlHdl ) );
    maColMixCtrl.SetSelectHdl( LINK( this, SvColorDialog, SelectMixCtrlHdl ) );

    Link aLink( LINK( this, SvColorDialog, ColorModifyHdl ) );
    maCtlColor.SetModifyHdl( aLink );

    maNumRed.SetModifyHdl( aLink );
    maNumGreen.SetModifyHdl( aLink );
    maNumBlue.SetModifyHdl( aLink );

    maNumCyan.SetModifyHdl( aLink );
    maNumMagenta.SetModifyHdl( aLink );
    maNumYellow.SetModifyHdl( aLink );
    maNumKey.SetModifyHdl( aLink );

    maNumHue.SetModifyHdl( aLink );
    maNumSaturation.SetModifyHdl( aLink );
    maNumLuminance.SetModifyHdl( aLink );

    aLink = ( LINK( this, SvColorDialog, ClickBtnHdl ) );
    maBtn1.SetClickHdl( aLink );
    maBtn2.SetClickHdl( aLink );
    //maBtn3.SetClickHdl( aLink );
    //maBtn4.SetClickHdl( aLink );

    maColMixCtrl.SetExtraSpacing( 0 );
}


// -----------------------------------------------------------------------
SvColorDialog::~SvColorDialog()
{
}

// -----------------------------------------------------------------------
void SvColorDialog::Initialize()
{
    maNumRed.SetValue( maColor.GetRed() );
    maNumGreen.SetValue( maColor.GetGreen() );
    maNumBlue.SetValue( maColor.GetBlue() );

    ColorCMYK aColorCMYK( maColor );

    long aCyan    = (long) ( (double)aColorCMYK.GetCyan() * 100.0 / 255.0 + 0.5 );
    long aMagenta = (long) ( (double)aColorCMYK.GetMagenta() * 100.0 / 255.0 + 0.5 );
    long aYellow  = (long) ( (double)aColorCMYK.GetYellow() * 100.0 / 255.0 + 0.5 );
    long aKey     = (long) ( (double)aColorCMYK.GetKey() * 100.0 / 255.0 + 0.5 );
    maNumCyan.SetValue( aCyan );
    maNumMagenta.SetValue( aMagenta );
    maNumYellow.SetValue( aYellow );
    maNumKey.SetValue( aKey );

    ColorHSB aColorHSB( maColor );
    maNumHue.SetValue( aColorHSB.GetHue() );
    maNumSaturation.SetValue( aColorHSB.GetSat() );
    maNumLuminance.SetValue( aColorHSB.GetBri() );

    maCtlColor.SetColor( aColorHSB );

    maColMixCtrl.SelectItem( 1 );

    maCtlPreview.SetColor( maColor );
    maCtlPreviewOld.SetColor( maColor );
}

// -----------------------------------------------------------------------
void SvColorDialog::SetColor( const Color& rColor )
{
    maColor = rColor;
}

// -----------------------------------------------------------------------
const Color& SvColorDialog::GetColor() const
{
    return( maColor );
}

// -----------------------------------------------------------------------
IMPL_LINK( SvColorDialog, ColorModifyHdl, void *, p )
{
    sal_uInt16 n = 0x00; // 1 == RGB, 2 == CMYK, 4 == HSB

    if( p == &maCtlColor )
    {
        maColor = maCtlColor.GetColor();
        maNumRed.SetValue( maColor.GetRed() );
        maNumGreen.SetValue( maColor.GetGreen() );
        maNumBlue.SetValue( maColor.GetBlue() );

        n = 7;
    }
    else if( p == &maNumRed )
    {
        maColor.SetRed( (sal_uInt8)maNumRed.GetValue() );
        maCtlColor.SetColor( maColor );
        n = 6;
    }
    else if( p == &maNumGreen )
    {
        maColor.SetGreen( (sal_uInt8)maNumGreen.GetValue() );
        maCtlColor.SetColor( maColor );
        n = 6;
    }
    else if( p == &maNumBlue )
    {
        maColor.SetBlue( (sal_uInt8)maNumBlue.GetValue() );
        maCtlColor.SetColor( maColor );
        n = 6;
    }
    else if( p == &maNumHue ||
             p == &maNumSaturation ||
             p == &maNumLuminance )
    {

        ColorHSB aColorHSB( (sal_uInt16) maNumHue.GetValue(),
                            (sal_uInt16) maNumSaturation.GetValue(),
                            (sal_uInt16) maNumLuminance.GetValue() );
        maCtlColor.SetColor( aColorHSB );
        maColor = maCtlColor.GetColor();
        n = 3;
    }
    else if( p == &maNumCyan ||
             p == &maNumMagenta ||
             p == &maNumYellow ||
             p == &maNumKey )
    {
        long aCyan    = (long) ( (double)maNumCyan.GetValue() * 255.0 / 100.0 + 0.5 );
        long aMagenta = (long) ( (double)maNumMagenta.GetValue() * 255.0 / 100.0 + 0.5 );
        long aYellow  = (long) ( (double)maNumYellow.GetValue() * 255.0 / 100.0 + 0.5 );
        long aKey     = (long) ( (double)maNumKey.GetValue() * 255.0 / 100.0 + 0.5 );

        ColorCMYK aColorCMYK( (sal_uInt16) aCyan,
                              (sal_uInt16) aMagenta,
                              (sal_uInt16) aYellow,
                              (sal_uInt16) aKey );
        maColor = aColorCMYK.GetRGB();
        maCtlColor.SetColor( maColor );
        n = 5;
    }

    if( n & 1 ) // RGB setzen
    {
        maNumRed.SetValue( maColor.GetRed() );
        maNumGreen.SetValue( maColor.GetGreen() );
        maNumBlue.SetValue( maColor.GetBlue() );
    }
    if( n & 2 ) // CMYK setzen
    {
        ColorCMYK aColorCMYK( maColor );
        long aCyan    = (long) ( (double)aColorCMYK.GetCyan() * 100.0 / 255.0 + 0.5 );
        long aMagenta = (long) ( (double)aColorCMYK.GetMagenta() * 100.0 / 255.0 + 0.5 );
        long aYellow  = (long) ( (double)aColorCMYK.GetYellow() * 100.0 / 255.0 + 0.5 );
        long aKey     = (long) ( (double)aColorCMYK.GetKey() * 100.0 / 255.0 + 0.5 );
        maNumCyan.SetValue( aCyan );
        maNumMagenta.SetValue( aMagenta );
        maNumYellow.SetValue( aYellow );
        maNumKey.SetValue( aKey );
    }
    if( n & 4 ) // HSB setzen
    {
        ColorHSB aColorHSB( maColor );
        maNumHue.SetValue( aColorHSB.GetHue() );
        maNumSaturation.SetValue( aColorHSB.GetSat() );
        maNumLuminance.SetValue( aColorHSB.GetBri() );
    }

    maCtlPreview.SetColor( maColor );

    return 0;
}

// -----------------------------------------------------------------------
IMPL_LINK( SvColorDialog, ClickBtnHdl, void *, p )
{
    /*
    Color aColor = maCtlColor.GetColor();
    if( p == &maBtn1 )
        maColMixCtrl.SetColor( CMC_TOPLEFT, aColor );
    if( p == &maBtn2 )
        maColMixCtrl.SetColor( CMC_TOPRIGHT, aColor );
    if( p == &maBtn3 )
        maColMixCtrl.SetColor( CMC_BOTTOMLEFT, aColor );
    if( p == &maBtn4 )
        maColMixCtrl.SetColor( CMC_BOTTOMRIGHT, aColor );
    */

    if( p == &maBtn1 )
    {
        CMCPosition ePos = maColMixCtrl.GetCMCPosition();
        if( ePos != CMC_OTHER )
            maColMixCtrl.SetColor( ePos, maColor );
    }
    else if( p == &maBtn2 )
    {
        sal_uInt16 nPos = maColMixCtrl.GetSelectItemId();
        maColor = maColMixCtrl.GetItemColor( nPos );
        maCtlColor.SetColor( maColor );
        ColorModifyHdl( &maCtlColor );
    }

    return 0;
}

// -----------------------------------------------------------------------
IMPL_LINK( SvColorDialog, ClickMixCtrlHdl, void *, EMPTYARG )
{
    sal_uInt16 nPos = maColMixCtrl.GetSelectItemId();
    CMCPosition ePos = maColMixCtrl.GetCMCPosition();

    if( ePos != CMC_OTHER )
        maColMixCtrl.SetColor( ePos, maColor );
    else
    {
        maColor = maColMixCtrl.GetItemColor( nPos );
        maCtlColor.SetColor( maColor );
        ColorModifyHdl( &maCtlColor );
    }

    return 0;
}

// -----------------------------------------------------------------------
IMPL_LINK( SvColorDialog, SelectMixCtrlHdl, void *, EMPTYARG )
{
    //sal_uInt16 nPos = maColMixCtrl.GetSelectItemId();
    //maFtRGB.SetText( maColMixCtrl.GetItemText( nPos ) );

    CMCPosition ePos = maColMixCtrl.GetCMCPosition();
    if( ePos == CMC_OTHER )
        maBtn1.Enable( sal_False );
    else
        maBtn1.Enable();

    return 0;
}

// -----------------------------------------------------------------------
short SvColorDialog::Execute()
{
    Initialize();

    short nRet = ModalDialog::Execute();

    return( nRet );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

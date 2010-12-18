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

#ifndef _SV_COLCTRL_HXX
#define _SV_COLCTRL_HXX

#include <tools/color.hxx>
#include <vcl/ctrl.hxx>
#include <svtools/valueset.hxx>

// ----------------
// - ColorControl -
// ----------------

class Bitmap;
class BitmapReadAccess;
class ColorHSB;

class SvColorControl : public Control
{
private:
    Bitmap*         mpBitmap;
    BitmapReadAccess* mpReadAccess;
    Color           maColor;
    short           mnLuminance;
    Point           maPosition;
    Link            maModifyHdl;

    void            Initialize();
    void            CreateBitmap();
    void            ShowPosition( const Point& aPos );

public:
                    SvColorControl( Window* pParent, WinBits nStyle = 0 );
                    SvColorControl( Window* pParent, const ResId& rResId );
                    ~SvColorControl();

    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
//    virtual void    KeyInput( const KeyEvent& rKEvent );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    Resize();
//    virtual void    GetFocus();
//    virtual void    LoseFocus();

    virtual void    Modify();

    Color           GetColor() const { return maColor; };
    void            SetColor( const Color& rCol );
    void            SetColor( const ColorHSB& rCol, BOOL bSetColor = TRUE );

    short           GetLuminance() const { return mnLuminance; };
    void            SetLuminance( short nLum );

    void            SetModifyHdl( const Link& rLink ) { maModifyHdl = rLink; }
    const Link&     GetModifyHdl() const { return maModifyHdl; }

};

// -----------------------
// - ColorPreviewControl -
// -----------------------

class ColorPreviewControl : public Control
{
private:
    Color           maColor;

public:
                    ColorPreviewControl( Window* pParent, WinBits nStyle = 0 );
                    ColorPreviewControl( Window* pParent, const ResId& rResId );
                    ~ColorPreviewControl();

    virtual void    Paint( const Rectangle& rRect );

    Color           GetColor() const { return maColor; };
    void            SetColor( const Color& rCol );
};

// -----------------------
// - ColorMixingControl -
// -----------------------

enum CMCPosition { CMC_TOPLEFT, CMC_TOPRIGHT, CMC_BOTTOMLEFT, CMC_BOTTOMRIGHT, CMC_OTHER };

class ColorMixingControl : public ValueSet
{
private:
    USHORT          mnRows;
    USHORT          mnColumns;
    Color           maColor[4];

    void            Initialize();

    Color           CalcDifferenceColor( USHORT nCol1, USHORT nCol2, USHORT nSteps );
    void            FillRow( USHORT nRow );
    void            FillColumn( USHORT nColumn );

public:
                    ColorMixingControl( Window* pParent, WinBits nStyle = 0,
                                        USHORT nRows = 4, USHORT nColumns = 4 );
                    ColorMixingControl( Window* pParent, const ResId& rResId,
                                        USHORT nRows = 4, USHORT nColumns = 4 );
                    ~ColorMixingControl();

    USHORT          GetRows() const { return mnRows; };
    void            SetRows( USHORT nRows );
    USHORT          GetColumns() const { return mnColumns; };
    void            SetColumns( USHORT nColumns );

    using ValueSet::GetColor;
    Color           GetColor( CMCPosition ePos ) const { return maColor[ ePos ]; };
    using ValueSet::SetColor;
    void            SetColor( CMCPosition ePos, const Color& rCol );

    String          GetRGBString( const Color& rColor );
    CMCPosition     GetCMCPosition() const;
};

// ------------
// - ColorHSB -
// ------------

class ColorHSB
{
private:
    USHORT      mnHue;   // Farbwinkel, 360 Grad
    USHORT      mnSat;   // Saturation, 100 %
    USHORT      mnBri;   // Brightness, 100 %

public:
                ColorHSB()
                    { mnHue=0; mnSat=0; mnBri=0; }
                ColorHSB( USHORT nH, USHORT nS, USHORT nB )
                    { mnHue=nH; mnSat=nS; mnBri=nB; }
                ColorHSB( const Color& rColor );

    void        SetHue( USHORT nH ) { mnHue=nH; }
    void        SetSat( USHORT nS ) { mnSat=nS; }
    void        SetBri( USHORT nB ) { mnBri=nB; }
    USHORT      GetHue() const { return mnHue; }
    USHORT      GetSat() const { return mnSat; }
    USHORT      GetBri() const { return mnBri; }
    Color       GetRGB() const;
};

// -------------
// - ColorCMYK -
// -------------

class ColorCMYK
{
private:
    USHORT      mnCyan;
    USHORT      mnMagenta;
    USHORT      mnYellow;
    USHORT      mnKey;

public:
                ColorCMYK()
                    { mnCyan=0; mnMagenta=0; mnYellow=0; mnKey=100; }
                ColorCMYK( USHORT nC, USHORT nM, USHORT nY, USHORT nK )
                    { mnCyan=nC; mnMagenta=nM; mnYellow=nY; mnKey=nK; }
                ColorCMYK( const Color& rColor );

    void        SetCyan( USHORT nC ) { mnCyan=nC; }
    void        SetMagenta( USHORT nM ) { mnMagenta=nM; }
    void        SetYellow( USHORT nY ) { mnYellow=nY; }
    void        SetKey( USHORT nK ) { mnKey=nK; }
    USHORT      GetCyan() const { return mnCyan; }
    USHORT      GetMagenta() const { return mnMagenta; }
    USHORT      GetYellow() const { return mnYellow; }
    USHORT      GetKey() const { return mnKey; }
    Color       GetRGB() const;
};

#endif  // _SV_COLCTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

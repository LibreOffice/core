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
    void            SetColor( const ColorHSB& rCol, sal_Bool bSetColor = sal_True );

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
    sal_uInt16          mnRows;
    sal_uInt16          mnColumns;
    Color           maColor[4];

    void            Initialize();

    Color           CalcDifferenceColor( sal_uInt16 nCol1, sal_uInt16 nCol2, sal_uInt16 nSteps );
    void            FillRow( sal_uInt16 nRow );
    void            FillColumn( sal_uInt16 nColumn );

public:
                    ColorMixingControl( Window* pParent, WinBits nStyle = 0,
                                        sal_uInt16 nRows = 4, sal_uInt16 nColumns = 4 );
                    ColorMixingControl( Window* pParent, const ResId& rResId,
                                        sal_uInt16 nRows = 4, sal_uInt16 nColumns = 4 );
                    ~ColorMixingControl();

    sal_uInt16          GetRows() const { return mnRows; };
    void            SetRows( sal_uInt16 nRows );
    sal_uInt16          GetColumns() const { return mnColumns; };
    void            SetColumns( sal_uInt16 nColumns );

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
    sal_uInt16      mnHue;   // Farbwinkel, 360 Grad
    sal_uInt16      mnSat;   // Saturation, 100 %
    sal_uInt16      mnBri;   // Brightness, 100 %

public:
                ColorHSB()
                    { mnHue=0; mnSat=0; mnBri=0; }
                ColorHSB( sal_uInt16 nH, sal_uInt16 nS, sal_uInt16 nB )
                    { mnHue=nH; mnSat=nS; mnBri=nB; }
                ColorHSB( const Color& rColor );

    void        SetHue( sal_uInt16 nH ) { mnHue=nH; }
    void        SetSat( sal_uInt16 nS ) { mnSat=nS; }
    void        SetBri( sal_uInt16 nB ) { mnBri=nB; }
    sal_uInt16      GetHue() const { return mnHue; }
    sal_uInt16      GetSat() const { return mnSat; }
    sal_uInt16      GetBri() const { return mnBri; }
    Color       GetRGB() const;
};

// -------------
// - ColorCMYK -
// -------------

class ColorCMYK
{
private:
    sal_uInt16      mnCyan;
    sal_uInt16      mnMagenta;
    sal_uInt16      mnYellow;
    sal_uInt16      mnKey;

public:
                ColorCMYK()
                    { mnCyan=0; mnMagenta=0; mnYellow=0; mnKey=100; }
                ColorCMYK( sal_uInt16 nC, sal_uInt16 nM, sal_uInt16 nY, sal_uInt16 nK )
                    { mnCyan=nC; mnMagenta=nM; mnYellow=nY; mnKey=nK; }
                ColorCMYK( const Color& rColor );

    void        SetCyan( sal_uInt16 nC ) { mnCyan=nC; }
    void        SetMagenta( sal_uInt16 nM ) { mnMagenta=nM; }
    void        SetYellow( sal_uInt16 nY ) { mnYellow=nY; }
    void        SetKey( sal_uInt16 nK ) { mnKey=nK; }
    sal_uInt16      GetCyan() const { return mnCyan; }
    sal_uInt16      GetMagenta() const { return mnMagenta; }
    sal_uInt16      GetYellow() const { return mnYellow; }
    sal_uInt16      GetKey() const { return mnKey; }
    Color       GetRGB() const;
};

#endif  // _SV_COLCTRL_HXX

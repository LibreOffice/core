/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: colctrl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:14:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_COLCTRL_HXX
#define _SV_COLCTRL_HXX

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _SV_CTRL_HXX //autogen
#include <vcl/ctrl.hxx>
#endif
#ifndef _VALUESET_HXX //autogen
#include <svtools/valueset.hxx>
#endif

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

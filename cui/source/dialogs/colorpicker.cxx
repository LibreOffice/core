/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Christian Lippka <cl@lippka.com>
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Major Contributor(s):
 *
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/awt/XWindow.hpp>

#include <cppuhelper/compbase4.hxx>

#include <comphelper/broadcasthelper.hxx>

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/decoview.hxx>
#include <vcl/svapp.hxx>

#include <toolkit/helper/vclunohelper.hxx>

#include <sot/exchange.hxx>
#include <sot/formats.hxx>

#include <sax/tools/converter.hxx>

#include <basegfx/color/bcolortools.hxx>

#include "dialmgr.hxx"
#include "colorpicker.hrc"

#include <cmath>

using rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::beans;
using namespace ::basegfx;

namespace cui
{

const sal_uInt16 COLORMODE_RGB =  0x10;
const sal_uInt16 COLORMODE_HSV =  0x20;
const sal_uInt16 COLORMODE_CMYK = 0x40;

const sal_uInt16 COLORCOMP_RED   = 0x10;
const sal_uInt16 COLORCOMP_GREEN = 0x11;
const sal_uInt16 COLORCOMP_BLUE  = 0x12;

const sal_uInt16 COLORCOMP_HUE  = 0x20;
const sal_uInt16 COLORCOMP_SAT  = 0x21;
const sal_uInt16 COLORCOMP_BRI  = 0x22;

const sal_uInt16 COLORCOMP_CYAN    = 0x40;
const sal_uInt16 COLORCOMP_YELLOW  = 0x41;
const sal_uInt16 COLORCOMP_MAGENTA = 0x42;
const sal_uInt16 COLORCOMP_KEY     = 0x43;

// color space conversion helpers

static void RGBtoHSV( double dR, double dG, double dB, double& dH, double& dS, double& dV )
{
    BColor result = tools::rgb2hsv( BColor( dR, dG, dB ) );

    dH = result.getX();
    dS = result.getY();
    dV = result.getZ();
}

static void HSVtoRGB(double dH, double dS, double dV, double& dR, double& dG, double& dB )
{
    BColor result = tools::hsv2rgb( BColor( dH, dS, dV ) );

    dR = result.getRed();
    dG = result.getGreen();
    dB = result.getBlue();
}

// -----------------------------------------------------------------------

// CMYK values from 0 to 1
static void CMYKtoRGB( double fCyan, double fMagenta, double fYellow, double fKey, double& dR, double& dG, double& dB )
{
    fCyan = (fCyan * ( 1.0 - fKey )) + fKey;
    fMagenta = (fMagenta * ( 1.0 - fKey )) + fKey;
    fYellow = (fYellow * ( 1.0 - fKey )) + fKey;

    dR = std::max( std::min( ( 1.0 - fCyan ), 1.0), 0.0 );
    dG = std::max( std::min( ( 1.0 - fMagenta ), 1.0), 0.0 );
    dB = std::max( std::min( ( 1.0 - fYellow ), 1.0), 0.0 );
}

// -----------------------------------------------------------------------

// CMY results from 0 to 1
static void RGBtoCMYK( double dR, double dG, double dB, double& fCyan, double& fMagenta, double& fYellow, double& fKey )
{
    fCyan = 1 - dR;
    fMagenta = 1 - dG;
    fYellow = 1 - dB;

    //CMYK and CMY values from 0 to 1
    fKey = 1.0;
    if( fCyan < fKey ) fKey = fCyan;
    if( fMagenta < fKey ) fKey = fMagenta;
    if( fYellow < fKey ) fKey = fYellow;

    if( fKey >= 1.0 )
    {
        //Black
       fCyan = 0.0;
       fMagenta = 0.0;
       fYellow = 0.0;
    }
    else
    {
       fCyan = ( fCyan - fKey ) / ( 1.0 - fKey );
       fMagenta = ( fMagenta - fKey ) / ( 1.0 - fKey );
       fYellow = ( fYellow - fKey ) / ( 1.0 - fKey );
    }
}

// ====================================================================

class HexColorControl : public Edit
{
public:
    HexColorControl( Window* pParent, const ResId& rResId );

    virtual long PreNotify( NotifyEvent& rNEvt );
    virtual void Paste();

    void SetColor( sal_Int32 nColor );
    sal_Int32 GetColor();

private:
    bool ImplProcessKeyInput( const KeyEvent& rKEv );
};

HexColorControl::HexColorControl( Window* pParent, const ResId& rResId )
: Edit( pParent, rResId )
{
    SetMaxTextLen( 6 );
}

// -----------------------------------------------------------------------

void HexColorControl::SetColor( sal_Int32 nColor )
{
    ::rtl::OUStringBuffer aBuffer;
    sax::Converter::convertColor( aBuffer, nColor );
    SetText( aBuffer.makeStringAndClear().copy(1) );
}

// -----------------------------------------------------------------------

sal_Int32 HexColorControl::GetColor()
{
    sal_Int32 nColor = -1;

    OUString aStr( RTL_CONSTASCII_USTRINGPARAM( "#" ) );
    aStr += GetText();
    sal_Int32 nLen = aStr.getLength();
    if( nLen < 7 )
    {
        static const sal_Char* pNullStr = "000000";
        aStr += OUString::createFromAscii( &pNullStr[nLen-1] );
    }

    sax::Converter::convertColor( nColor, aStr );

    if( nColor == -1 )
        SetControlBackground( Color( COL_RED ) );
    else
        SetControlBackground();

    return nColor;
}

// -----------------------------------------------------------------------

long HexColorControl::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplProcessKeyInput( *rNEvt.GetKeyEvent() ) )
            return 1;
    }

    return Edit::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

void HexColorControl::Paste()
{
    ::com::sun::star::uno::Reference<com::sun::star::datatransfer::clipboard::XClipboard> aClipboard(GetClipboard());
    if ( aClipboard.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > xDataObj;

        const sal_uInt32 nRef = Application::ReleaseSolarMutex();

        try
        {
            xDataObj = aClipboard->getContents();
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
        }

        Application::AcquireSolarMutex( nRef );

        if ( xDataObj.is() )
        {
            ::com::sun::star::datatransfer::DataFlavor aFlavor;
            SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aFlavor );
            try
            {
                ::com::sun::star::uno::Any aData = xDataObj->getTransferData( aFlavor );
                ::rtl::OUString aText;
                aData >>= aText;

                if( !aText.isEmpty() && aText.matchAsciiL( "#", 1, 0 ) )
                    aText = aText.copy(1);

                if( aText.getLength() > 6 )
                    aText = aText.copy( 0, 6 );

                SetText( aText );
            }
            catch( const ::com::sun::star::uno::Exception& )
            {
            }
        }
    }
}

// -----------------------------------------------------------------------

bool HexColorControl::ImplProcessKeyInput( const KeyEvent& rKEv )
{
    const KeyCode& rKeyCode = rKEv.GetKeyCode();

    if( rKeyCode.GetGroup() == KEYGROUP_ALPHA && !rKeyCode.IsMod1() && !rKeyCode.IsMod2() )
    {
        if( (rKeyCode.GetCode() < KEY_A) || (rKeyCode.GetCode() > KEY_F) )
            return true;
    }
    else if( rKeyCode.GetGroup() == KEYGROUP_NUM )
    {
        if( rKeyCode.IsShift() )
            return true;
    }
    return false;
}

// ====================================================================

class ColorPreviewControl : public Control
{
public:
    ColorPreviewControl( Window* pParent, const ResId& rResId );

    virtual void        Paint( const Rectangle& rRect );

    void SetColor( const Color& rColor );
private:
    Color maColor;
};

// -----------------------------------------------------------------------

ColorPreviewControl::ColorPreviewControl( Window* pParent, const ResId& rResId )
: Control( pParent, rResId )
{
    SetFillColor( maColor );
    SetLineColor( maColor );
}

// -----------------------------------------------------------------------

void ColorPreviewControl::SetColor( const Color& rCol )
{
    if( rCol != maColor )
    {
        maColor = rCol;
        SetFillColor( maColor );
        SetLineColor( maColor );
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void ColorPreviewControl::Paint( const Rectangle& rRect )
{
    DrawRect( rRect );
}

// ====================================================================

enum ColorMode { HUE, SATURATION, BRIGHTNESS, RED, GREEN, BLUE };
const ColorMode DefaultMode = HUE;

class ColorFieldControl : public Control
{
public:
    ColorFieldControl( Window* pParent, const ResId& rResId );
    ~ColorFieldControl();

    virtual void        MouseMove( const MouseEvent& rMEvt );
    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        MouseButtonUp( const MouseEvent& rMEvt );
    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual void        Paint( const Rectangle& rRect );
    virtual void        Resize();

    void                UpdateBitmap();
    void                ShowPosition( const Point& rPos, bool bUpdate );
    void                UpdatePosition();
    void                Modify();

    void                SetValues( Color aColor, ColorMode eMode, double x, double y );
    double              GetX();
    double              GetY();

    void                KeyMove( int dx, int dy );

    void                SetModifyHdl( Link& rLink ) { maModifyHdl = rLink; }

private:
    Link maModifyHdl;
    ColorMode meMode;
    Color maColor;
    double mdX;
    double mdY;
    Point maPosition;
    Bitmap* mpBitmap;
    std::vector< sal_uInt8 > maRGB_Horiz;
    std::vector< sal_uInt16 > maGrad_Horiz;
    std::vector< sal_uInt16 > maPercent_Horiz;
    std::vector< sal_uInt8 > maRGB_Vert;
    std::vector< sal_uInt16 > maPercent_Vert;
    bool mbInit;
};

// -----------------------------------------------------------------------

ColorFieldControl::ColorFieldControl( Window* pParent, const ResId& rResId )
: Control( pParent, rResId )
, meMode( DefaultMode )
, mdX( -1.0 )
, mdY( -1.0 )
, mpBitmap( 0 )
, mbInit( false )
{
    SetControlBackground();
}

// -----------------------------------------------------------------------

ColorFieldControl::~ColorFieldControl()
{
    delete mpBitmap;
}

// -----------------------------------------------------------------------

void ColorFieldControl::UpdateBitmap()
{
    const Size aSize( GetOutputSizePixel() );

    if( mpBitmap && mpBitmap->GetSizePixel() != aSize )
        delete mpBitmap, mpBitmap = NULL;

    const sal_Int32 nWidth = aSize.Width();
    const sal_Int32 nHeight = aSize.Height();

    if( !mpBitmap )
    {
        mpBitmap = new Bitmap( aSize, 24 );

        maRGB_Horiz.resize( nWidth );
        maGrad_Horiz.resize( nWidth );
        maPercent_Horiz.resize( nWidth );

        sal_uInt8* pRGB = &(*maRGB_Horiz.begin());
        sal_uInt16* pGrad = &(*maGrad_Horiz.begin());
        sal_uInt16* pPercent = &(*maPercent_Horiz.begin());

        for( sal_Int32 x = 0; x < nWidth; x++ )
        {
            *pRGB++ = static_cast< sal_uInt8 >( (x * 256) / nWidth );
            *pGrad++ = static_cast< sal_uInt16 >( (x * 359) / nWidth );
            *pPercent++ = static_cast< sal_uInt16 >( (x * 100) / nWidth );
        }

        maRGB_Vert.resize( nHeight );
        maPercent_Vert.resize( nHeight );

        pRGB = &(*maRGB_Vert.begin());
        pPercent = &(*maPercent_Vert.begin());

        sal_Int32 y = nHeight;
        while( y-- )
        {
            *pRGB++ = static_cast< sal_uInt8 >( (y * 256) / nHeight );
            *pPercent++ = static_cast< sal_uInt16 >( (y * 100) / nHeight );
        }
    }

    sal_uInt8* pRGB_Horiz = &(*maRGB_Horiz.begin());
    sal_uInt16* pGrad_Horiz = &(*maGrad_Horiz.begin());
    sal_uInt16* pPercent_Horiz = &(*maPercent_Horiz.begin());
    sal_uInt8* pRGB_Vert = &(*maRGB_Vert.begin());
    sal_uInt16* pPercent_Vert = &(*maPercent_Vert.begin());

    BitmapWriteAccess* pWriteAccess = mpBitmap->AcquireWriteAccess();
    if( pWriteAccess )
    {
        BitmapColor aBitmapColor( maColor );

        sal_uInt16 nHue, nSat, nBri;
        maColor.RGBtoHSB( nHue, nSat, nBri );

        // this has been unlooped for performance reason, please do not merge back!

        sal_uInt16 y = nHeight,x;

        switch( meMode )
        {
        case HUE:
            while( y-- )
            {
                nBri = pPercent_Vert[y];
                x = nWidth;
                while( x-- )
                {
                    nSat = pPercent_Horiz[x];
                    pWriteAccess->SetPixel( y, x, BitmapColor( Color( Color::HSBtoRGB( nHue, nSat, nBri ) ) ) );
                }
            }
            break;
        case SATURATION:
            while( y-- )
            {
                nBri = pPercent_Vert[y];
                x = nWidth;
                while( x-- )
                {
                    nHue = pGrad_Horiz[x];
                    pWriteAccess->SetPixel( y, x, BitmapColor( Color( Color::HSBtoRGB( nHue, nSat, nBri ) ) ) );
                }
            }
            break;
        case BRIGHTNESS:
            while( y-- )
            {
                nSat = pPercent_Vert[y];
                x = nWidth;
                while( x-- )
                {
                    nHue = pGrad_Horiz[x];
                    pWriteAccess->SetPixel( y, x, BitmapColor( Color( Color::HSBtoRGB( nHue, nSat, nBri ) ) ) );
                }
            }
            break;
        case RED:
            while( y-- )
            {
                aBitmapColor.SetGreen( pRGB_Vert[y] );
                x = nWidth;
                while( x-- )
                {
                    aBitmapColor.SetBlue( pRGB_Horiz[x] );
                    pWriteAccess->SetPixel( y, x, aBitmapColor );
                }
            }
            break;
        case GREEN:
            while( y-- )
            {
                aBitmapColor.SetRed( pRGB_Vert[y] );
                x = nWidth;
                while( x-- )
                {
                    aBitmapColor.SetBlue( pRGB_Horiz[x] );
                    pWriteAccess->SetPixel( y, x, aBitmapColor );
                }
            }
            break;
        case BLUE:
            while( y-- )
            {
                aBitmapColor.SetGreen( pRGB_Vert[y] );
                x = nWidth;
                while( x-- )
                {
                    aBitmapColor.SetRed( pRGB_Horiz[x] );
                    pWriteAccess->SetPixel( y, x, aBitmapColor );
                }
            }
            break;
        }

        mpBitmap->ReleaseAccess( pWriteAccess );
    }
}

// -----------------------------------------------------------------------

void ColorFieldControl::ShowPosition( const Point& rPos, bool bUpdate )
{
    if( !mpBitmap )
    {
        UpdateBitmap();
        Invalidate();
    }

    const Size aSize( mpBitmap->GetSizePixel() );

    long nX = rPos.X();
    long nY = rPos.Y();
    if( nX < 0L )
        nX = 0L;
    else if( nX >= aSize.Width() )
        nX = aSize.Width() - 1L;

    if( nY < 0L )
        nY= 0L;
    else if( nY >= aSize.Height() )
        nY = aSize.Height() - 1L;

    Point aPos = maPosition;
    maPosition.X() = nX - 5;
    maPosition.Y() = nY - 5;
    Invalidate( Rectangle( aPos, Size( 11, 11) ) );
    Invalidate( Rectangle( maPosition, Size( 11, 11) ) );

    if( bUpdate )
    {
        mdX = (double)nX / (double)(aSize.Width()-1);
        mdY = (double)(aSize.Height()-1-nY) / (double)(aSize.Height()-1);

        BitmapReadAccess* pReadAccess = mpBitmap->AcquireReadAccess();
        if( pReadAccess != NULL )
        {
            // mpBitmap always has a bit count of 24 => use of GetPixel(...) is safe
            maColor = pReadAccess->GetPixel( nY, nX );
            mpBitmap->ReleaseAccess( pReadAccess );
            pReadAccess = NULL;
        }
    }
}
// -----------------------------------------------------------------------

void ColorFieldControl::MouseMove( const MouseEvent& rMEvt )
{
    if( rMEvt.IsLeft() )
    {
        ShowPosition( rMEvt.GetPosPixel(), true );
        Modify();
    }
}

// -----------------------------------------------------------------------
void ColorFieldControl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( rMEvt.IsLeft() && !rMEvt.IsShift() )
    {
        CaptureMouse();
        ShowPosition( rMEvt.GetPosPixel(), true );
        Modify();
    }
}

// -----------------------------------------------------------------------
void ColorFieldControl::MouseButtonUp( const MouseEvent& )
{
    if( IsMouseCaptured() )
        ReleaseMouse();
}

// -----------------------------------------------------------------------

void ColorFieldControl::KeyMove( int dx, int dy )
{
    Size aSize( GetOutputSizePixel() );
    Point aPos(static_cast<long>(mdX * aSize.Width()), static_cast<long>((1.0 - mdY) * aSize.Height()));
    aPos.X() += dx;
    aPos.Y() += dy;
    if( aPos.X() < 0 )
        aPos.X() += aSize.Width();
    else if( aPos.X() >= aSize.Width() )
        aPos.X() -= aSize.Width();

    if( aPos.Y() < 0 )
        aPos.Y() += aSize.Height();
    else if( aPos.Y() >= aSize.Height() )
        aPos.Y() -= aSize.Height();

    ShowPosition( aPos, true );
    Modify();
}

// -----------------------------------------------------------------------

void ColorFieldControl::KeyInput( const KeyEvent& rKEvt )
{
    bool   bShift = rKEvt.GetKeyCode().IsShift();
    bool   bCtrl = rKEvt.GetKeyCode().IsMod1();
    bool   bAlt = rKEvt.GetKeyCode().IsMod2();

    if ( !bAlt && !bShift )
    {
        switch( rKEvt.GetKeyCode().GetCode() )
        {
        case KEY_DOWN:      KeyMove(  0, bCtrl ?  5  :  1 ); return;
        case KEY_UP:        KeyMove(  0, bCtrl ? -5  : -1 ); return;
        case KEY_LEFT:      KeyMove( bCtrl ? -5  : -1,  0 ); return;
        case KEY_RIGHT:     KeyMove( bCtrl ?  5  :  1,  0 ); return;
        }
    }
    Control::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

void ColorFieldControl::Paint( const Rectangle& rRect )
{
    if( !mpBitmap )
        UpdateBitmap();

    Bitmap aOutputBitmap( *mpBitmap );

    if( GetBitCount() <= 8 )
        aOutputBitmap.Dither();

    DrawBitmap( rRect.TopLeft(), rRect.GetSize(), rRect.TopLeft(), rRect.GetSize(), aOutputBitmap );

    // draw circle around current color
    if( maColor.IsDark() )
        SetLineColor( COL_WHITE );
    else
        SetLineColor( COL_BLACK );

    SetFillColor();

    DrawEllipse( Rectangle( maPosition, Size( 11, 11) ) );
}

// -----------------------------------------------------------------------

void ColorFieldControl::Resize()
{
    UpdateBitmap();
    Control::Resize();
}

// -----------------------------------------------------------------------

void ColorFieldControl::Modify()
{
    maModifyHdl.Call( this );
}

// -----------------------------------------------------------------------

void ColorFieldControl::SetValues( Color aColor, ColorMode eMode, double x, double y )
{
    bool bUpdateBitmap = (maColor!= aColor) || (meMode != eMode);
    if( bUpdateBitmap || (mdX != x) || (mdY != y) )
    {
        maColor = aColor;
        meMode = eMode;
        mdX = x;
        mdY = y;

        if( bUpdateBitmap )
            UpdateBitmap();
        UpdatePosition();
        if( bUpdateBitmap )
            Invalidate();
    }
}

// -----------------------------------------------------------------------

double ColorFieldControl::GetX()
{
    return mdX;
}

// -----------------------------------------------------------------------

double ColorFieldControl::GetY()
{
    return mdY;
}

// -----------------------------------------------------------------------

void ColorFieldControl::UpdatePosition()
{
    Size aSize( GetOutputSizePixel() );
    ShowPosition( Point(static_cast<long>(mdX * aSize.Width()), static_cast<long>((1.0 - mdY) * aSize.Height())), false );
}

// ====================================================================

class ColorSliderControl : public Control
{
public:
    ColorSliderControl( Window* pParent, const ResId& rResId );
    ~ColorSliderControl();

    virtual void        MouseMove( const MouseEvent& rMEvt );
    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        MouseButtonUp( const MouseEvent& rMEvt );
    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual void        Paint( const Rectangle& rRect );
    virtual void        Resize();

    void                UpdateBitmap();
    void                ChangePosition( long nY );
    void                Modify();

    void SetValue( const Color& rColor, ColorMode eMode, double dValue );
    double GetValue() const { return mdValue; }

    void                KeyMove( int dy );

    void SetModifyHdl( Link& rLink ) { maModifyHdl = rLink; }

    sal_Int16 GetLevel() const { return mnLevel; }

private:
    Link maModifyHdl;
    Color maColor;
    ColorMode meMode;
    Bitmap* mpBitmap;
    sal_Int16 mnLevel;
    double mdValue;
};

// -----------------------------------------------------------------------

ColorSliderControl::ColorSliderControl( Window* pParent, const ResId& rResId )
: Control( pParent, rResId )
, meMode( DefaultMode )
, mpBitmap( 0 )
, mnLevel( 0 )
, mdValue( -1.0 )
{
    SetControlBackground();
}

// -----------------------------------------------------------------------

ColorSliderControl::~ColorSliderControl()
{
    delete mpBitmap;
}

// -----------------------------------------------------------------------

void ColorSliderControl::UpdateBitmap()
{
    Size aSize( 1, GetOutputSizePixel().Height() );

    if( mpBitmap && mpBitmap->GetSizePixel() != aSize )
        delete mpBitmap, mpBitmap = NULL;

    if( !mpBitmap )
        mpBitmap = new Bitmap( aSize, 24 );

    BitmapWriteAccess* pWriteAccess = mpBitmap->AcquireWriteAccess();

    if( pWriteAccess )
    {
        const long nY = aSize.Height()-1;

        BitmapColor aBitmapColor( maColor );

        sal_uInt16 nHue, nSat, nBri;
        maColor.RGBtoHSB( nHue, nSat, nBri );

        // this has been unlooped for performance reason, please do not merge back!

        switch( meMode )
        {
        case HUE:
            nSat = 100;
            nBri = 100;
            for( long y = 0; y <= nY; y++ )
            {
                nHue = static_cast< sal_uInt16 >( (359 * y) / nY );
                aBitmapColor = BitmapColor( Color( Color::HSBtoRGB( nHue, nSat, nBri ) ) );
                pWriteAccess->SetPixel( nY-y, 0, aBitmapColor );
            }
            break;

        case SATURATION:
            nBri = std::max( (sal_uInt16)32, nBri );
            for( long y = 0; y <= nY; y++ )
            {
                nSat = static_cast< sal_uInt16 >( (100 * y) / nY );
                pWriteAccess->SetPixel( nY-y, 0, BitmapColor( Color( Color::HSBtoRGB( nHue, nSat, nBri ) ) ) );
            }
            break;

        case BRIGHTNESS:
            for( long y = 0; y <= nY; y++ )
            {
                nBri = static_cast< sal_uInt16 >( (100 * y) / nY );
                pWriteAccess->SetPixel( nY-y, 0, BitmapColor( Color( Color::HSBtoRGB( nHue, nSat, nBri ) ) ) );
            }
            break;

        case RED:
            for( long y = 0; y <= nY; y++ )
            {
                aBitmapColor.SetRed( sal_uInt8( ((long)255 * y) / nY ) );
                pWriteAccess->SetPixel( nY-y, 0, aBitmapColor );
            }
            break;

        case GREEN:
            for( long y = 0; y <= nY; y++ )
            {
                aBitmapColor.SetGreen( sal_uInt8( ((long)255 * y) / nY ) );
                pWriteAccess->SetPixel( nY-y, 0, aBitmapColor );
            }
            break;

        case BLUE:
            for( long y = 0; y <= nY; y++ )
            {
                aBitmapColor.SetBlue( sal_uInt8( ((long)255 * y) / nY ) );
                pWriteAccess->SetPixel( nY-y, 0, aBitmapColor );
            }
            break;
        }

        mpBitmap->ReleaseAccess( pWriteAccess );
    }
}

// -----------------------------------------------------------------------

void ColorSliderControl::ChangePosition( long nY )
{
    const long nHeight = GetOutputSizePixel().Height() - 1;

    if( nY < 0L )
        nY = 0;
    else if( nY > nHeight )
        nY = nHeight;

    mnLevel = nY;
    mdValue = ((double)(nHeight - nY)) / (double)nHeight;
}

// -----------------------------------------------------------------------

void ColorSliderControl::MouseMove( const MouseEvent& rMEvt )
{
    if( rMEvt.IsLeft() )
    {
        ChangePosition( rMEvt.GetPosPixel().Y() );
        Modify();
    }
}

// -----------------------------------------------------------------------
void ColorSliderControl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( rMEvt.IsLeft() && !rMEvt.IsShift() )
    {
        CaptureMouse();
        ChangePosition( rMEvt.GetPosPixel().Y() );
        Modify();
    }
}

// -----------------------------------------------------------------------
void ColorSliderControl::MouseButtonUp( const MouseEvent& )
{
    if( IsMouseCaptured() )
        ReleaseMouse();
}

// -----------------------------------------------------------------------

void ColorSliderControl::KeyMove( int dy )
{
    ChangePosition( mnLevel + dy );
    Modify();
}

// -----------------------------------------------------------------------

void ColorSliderControl::KeyInput( const KeyEvent& rKEvt )
{
    if ( !rKEvt.GetKeyCode().IsMod2() && !rKEvt.GetKeyCode().IsShift() )
    {
        switch( rKEvt.GetKeyCode().GetCode() )
        {
        case KEY_DOWN:      KeyMove(  rKEvt.GetKeyCode().IsMod1() ?  5 :  1 ); return;
        case KEY_UP:        KeyMove(  rKEvt.GetKeyCode().IsMod1() ? -5 : -1 ); return;
        }
    }

    Control::KeyInput( rKEvt );
}
// -----------------------------------------------------------------------

void ColorSliderControl::Paint( const Rectangle& /*rRect*/ )
{
    if( !mpBitmap )
        UpdateBitmap();

    const Size aSize( GetOutputSizePixel() );

    Bitmap aOutputBitmap( *mpBitmap );

    if( GetBitCount() <= 8 )
        aOutputBitmap.Dither();

    Point aPos;
    int x = aSize.Width();
    while( x-- )
    {
        DrawBitmap( aPos, aOutputBitmap );
        aPos.X() += 1;
    }
}

// -----------------------------------------------------------------------

void ColorSliderControl::Resize()
{
    UpdateBitmap();
    Control::Resize();
}

// -----------------------------------------------------------------------

void ColorSliderControl::Modify()
{
    maModifyHdl.Call( this );
}

// -----------------------------------------------------------------------

void ColorSliderControl::SetValue( const Color& rColor, ColorMode eMode, double dValue )
{
    bool bUpdateBitmap = (rColor != maColor) || (eMode != meMode);
    if( bUpdateBitmap || (mdValue != dValue))
    {
        maColor = rColor;
        mdValue = dValue;
        mnLevel = static_cast<sal_Int16>((1.0-dValue) * GetOutputSizePixel().Height());
        meMode = eMode;
        if( bUpdateBitmap )
            UpdateBitmap();
        Invalidate();
    }
}

// ====================================================================

const sal_uInt16 UPDATE_RGB = 0x01;
const sal_uInt16 UPDATE_CMYK = 0x02;
const sal_uInt16 UPDATE_HSB = 0x04;
const sal_uInt16 UPDATE_COLORCHOOSER = 0x08;
const sal_uInt16 UPDATE_COLORSLIDER = 0x10;
const sal_uInt16 UPDATE_HEX = 0x20;
const sal_uInt16 UPDATE_ALL = 0xff;

class ColorPickerDialog : public ModalDialog
{
public:
    ColorPickerDialog( Window* pParent, sal_Int32 nColor, sal_Int16 nMode );

    void update_color( sal_uInt16 n = UPDATE_ALL );

    DECL_LINK( ColorModifyHdl, void * );
    DECL_LINK( ModeModifyHdl, void * );

    sal_Int32 GetColor() const;

    void setColorComponent( sal_uInt16 nComp, double dValue );

private:
    Color maPreviousColor;
    sal_Int16 mnDialogMode;
    ColorMode meMode;

    double mdRed, mdGreen, mdBlue;
    double mdHue, mdSat, mdBri;
    double mdCyan, mdMagenta, mdYellow, mdKey;

private:
    ColorFieldControl  maColorField;
    ColorSliderControl maColorSlider;
    ColorPreviewControl maColorPreview;
    ColorPreviewControl maColorPrevious;

    FixedImage maFISliderLeft;
    FixedImage maFISliderRight;
    Image maSliderImage;

#if 0
    ImageButton maBtnPicker;
#endif

    FixedLine maFLRGB;

    RadioButton maRBRed;
    RadioButton maRBGreen;
    RadioButton maRBBlue;
    RadioButton maRBHue;
    RadioButton maRBSaturation;
    RadioButton maRBBrightness;

    FixedText maFTRed;
    MetricField maMFRed;
    FixedText maFTGreen;
    MetricField maMFGreen;
    FixedText maFTBlue;
    MetricField maMFBlue;
    FixedText maFTHex;
    HexColorControl maEDHex;

    FixedLine maFLHSB;
    FixedText maFTHue;
    MetricField maMFHue;
    FixedText maFTSaturation;
    MetricField maMFSaturation;
    FixedText maFTBrightness;
    MetricField maMFBrightness;

    FixedLine maFLCMYK;
    FixedText maFTCyan;
    MetricField maMFCyan;
    FixedText maFTMagenta;
    MetricField maMFMagenta;
    FixedText maFTYellow;
    MetricField maMFYellow;
    FixedText maFTKey;
    MetricField maMFKey;

    FixedLine maFLBottmLine;
    HelpButton maBTNHelp;
    OKButton maBTNOk;
    CancelButton maBTNCancel;
};

// --------------------------------------------------------------------

ColorPickerDialog::ColorPickerDialog( Window* pParent, sal_Int32 nColor, sal_Int16 nMode )
: ModalDialog( pParent, CUI_RES( RID_CUI_DIALOG_COLORPICKER ) )
, maPreviousColor( nColor )
, mnDialogMode( nMode )
, meMode( DefaultMode )
, maColorField( this, CUI_RES( CT_COLORFIELD ) )
, maColorSlider( this, CUI_RES( CT_COLORSLIDER ) )
, maColorPreview( this, CUI_RES( CT_PREVIEW ) )
, maColorPrevious( this, CUI_RES( CT_PREVIOUS ) )
, maFISliderLeft(  this, CUI_RES( CT_LEFT_SLIDER ) )
, maFISliderRight( this, CUI_RES( CT_RIGHT_SLIDER ) )
, maSliderImage( CUI_RES( CT_SLIDERIMG ) )
#if 0
, maBtnPicker( this, CUI_RES( PB_PICKER ) )
#endif
, maFLRGB( this, CUI_RES( FL_RGB ) )
, maRBRed( this, CUI_RES( CT_RED ) )
, maRBGreen( this, CUI_RES( CT_GREEN ) )
, maRBBlue( this, CUI_RES( CT_BLUE ) )
, maRBHue( this, CUI_RES( CT_HUE ) )
, maRBSaturation( this, CUI_RES( CT_SATURATION ) )
, maRBBrightness( this, CUI_RES( CT_BRIGHTNESS ) )
, maFTRed( this, CUI_RES( CT_RED ) )
, maMFRed( this, CUI_RES( CT_RED ) )
, maFTGreen( this, CUI_RES( CT_GREEN ) )
, maMFGreen( this, CUI_RES( CT_GREEN ) )
, maFTBlue( this, CUI_RES( CT_BLUE ) )
, maMFBlue( this, CUI_RES( CT_BLUE ) )
, maFTHex( this, CUI_RES( CT_HEX ) )
, maEDHex( this, CUI_RES( CT_HEX ) )
, maFLHSB( this, CUI_RES( FL_HSB ) )
, maFTHue( this, CUI_RES( CT_HUE ) )
, maMFHue( this, CUI_RES( CT_HUE ) )
, maFTSaturation( this, CUI_RES( CT_SATURATION ) )
, maMFSaturation( this, CUI_RES( CT_SATURATION ) )
, maFTBrightness( this, CUI_RES( CT_BRIGHTNESS ) )
, maMFBrightness( this, CUI_RES( CT_BRIGHTNESS ) )
, maFLCMYK( this, CUI_RES( FL_CMYK ) )
, maFTCyan( this, CUI_RES( CT_CYAN ) )
, maMFCyan( this, CUI_RES( CT_CYAN ) )
, maFTMagenta( this, CUI_RES( CT_MAGENTA ) )
, maMFMagenta( this, CUI_RES( CT_MAGENTA ) )
, maFTYellow( this, CUI_RES( CT_YELLOW ) )
, maMFYellow( this, CUI_RES( CT_YELLOW ) )
, maFTKey( this, CUI_RES( CT_KEY ) )
, maMFKey( this, CUI_RES( CT_KEY ) )

, maFLBottmLine( this, CUI_RES( FT_BOTTOMLINE ) )
, maBTNHelp( this, CUI_RES( BTN_HELP ) )
, maBTNOk( this, CUI_RES( BTN_OK ) )
, maBTNCancel( this, CUI_RES( BTN_CANCEL ) )
{
    FreeResource();

    Link aLink( LINK( this, ColorPickerDialog, ColorModifyHdl ) );
    maColorField.SetModifyHdl( aLink );
    maColorSlider.SetModifyHdl( aLink );

    maMFRed.SetModifyHdl( aLink );
    maMFGreen.SetModifyHdl( aLink );
    maMFBlue.SetModifyHdl( aLink );

    maMFCyan.SetModifyHdl( aLink );
    maMFMagenta.SetModifyHdl( aLink );
    maMFYellow.SetModifyHdl( aLink );
    maMFKey.SetModifyHdl( aLink );

    maMFHue.SetModifyHdl( aLink );
    maMFSaturation.SetModifyHdl( aLink );
    maMFBrightness.SetModifyHdl( aLink );

    maEDHex.SetModifyHdl( aLink );

    aLink = LINK( this, ColorPickerDialog, ModeModifyHdl );
    maRBRed.SetToggleHdl( aLink );
    maRBGreen.SetToggleHdl( aLink );
    maRBBlue.SetToggleHdl( aLink );
    maRBHue.SetToggleHdl( aLink );
    maRBSaturation.SetToggleHdl( aLink );
    maRBBrightness.SetToggleHdl( aLink );

    Image aSliderImage( maSliderImage );

    maFISliderLeft.SetImage( aSliderImage );

    BitmapEx aTmpBmp( maSliderImage.GetBitmapEx() );
    aTmpBmp.Mirror( BMP_MIRROR_HORZ );
    maFISliderRight.SetImage( Image( aTmpBmp  ) );

    Size aSize( maSliderImage.GetSizePixel() );
    maFISliderLeft.SetSizePixel( aSize );
    maFISliderRight.SetSizePixel( aSize );

    Point aPos( maColorSlider.GetPosPixel() );

    aPos.X() -= aSize.Width();
    aPos.Y() -= aSize.Height() / 2;
    maFISliderLeft.SetPosPixel( aPos );

    aPos.X() += aSize.Width() + maColorSlider.GetSizePixel().Width();
    maFISliderRight.SetPosPixel( aPos );

    Color aColor( nColor );

    // modify
    if( mnDialogMode == 2 )
    {
        maColorPreview.SetSizePixel( maColorPrevious.GetSizePixel() );
        maColorPrevious.SetColor( aColor );
        maColorPrevious.Show( true );
    }

    mdRed = ((double)aColor.GetRed()) / 255.0;
    mdGreen = ((double)aColor.GetGreen()) / 255.0;
    mdBlue = ((double)aColor.GetBlue()) / 255.0;

    RGBtoHSV( mdRed, mdGreen, mdBlue, mdHue, mdSat, mdBri );
    RGBtoCMYK( mdRed, mdGreen, mdBlue, mdCyan, mdMagenta, mdYellow, mdKey );

    update_color();
}

// --------------------------------------------------------------------

static int toInt( double dValue, double dRange )
{
    return static_cast< int >( std::floor((dValue * dRange) + 0.5 ) );
}

sal_Int32 ColorPickerDialog::GetColor() const
{
    return Color( toInt(mdRed,255.0), toInt(mdGreen,255.0), toInt(mdBlue,255.0) ).GetColor();
}

void ColorPickerDialog::update_color( sal_uInt16 n )
{
    sal_uInt8 nRed = toInt(mdRed,255.0);
    sal_uInt8 nGreen = toInt(mdGreen,255.0);
    sal_uInt8 nBlue = toInt(mdBlue,255.0);

    Color aColor( nRed, nGreen, nBlue );

    if( n & UPDATE_RGB ) // update RGB
    {
        maMFRed.SetValue( nRed );
        maMFGreen.SetValue( nGreen );
        maMFBlue.SetValue( nBlue );
    }

    if( n & UPDATE_CMYK ) // update CMYK
    {
        maMFCyan.SetValue( toInt( mdCyan, 100.0 ) );
        maMFMagenta.SetValue( toInt( mdMagenta, 100.0 ) );
        maMFYellow.SetValue( toInt( mdYellow, 100.0 ) );
        maMFKey.SetValue( toInt( mdKey, 100.0 ) );
    }

    if( n & UPDATE_HSB ) // update HSB
    {
        maMFHue.SetValue( toInt( mdHue, 1.0 ) );
        maMFSaturation.SetValue( toInt( mdSat, 100.0 ) );
        maMFBrightness.SetValue( toInt( mdBri, 100.0 ) );
    }

    if( n & UPDATE_COLORCHOOSER ) // update Color Chooser 1
    {
        switch( meMode )
        {
        case HUE:           maColorField.SetValues( aColor, meMode, mdSat, mdBri ); break;
        case SATURATION:    maColorField.SetValues( aColor, meMode, mdHue / 360.0, mdBri ); break;
        case BRIGHTNESS:    maColorField.SetValues( aColor, meMode, mdHue / 360.0, mdSat ); break;
        case RED:           maColorField.SetValues( aColor, meMode, mdBlue, mdGreen ); break;
        case GREEN:         maColorField.SetValues( aColor, meMode, mdBlue, mdRed ); break;
        case BLUE:          maColorField.SetValues( aColor, meMode, mdRed, mdGreen ); break;
        }
    }

    if( n & UPDATE_COLORSLIDER ) // update Color Chooser 2
    {
        switch( meMode )
        {
        case HUE:           maColorSlider.SetValue( aColor, meMode, mdHue / 360.0 ); break;
        case SATURATION:    maColorSlider.SetValue( aColor, meMode, mdSat ); break;
        case BRIGHTNESS:    maColorSlider.SetValue( aColor, meMode, mdBri ); break;
        case RED:           maColorSlider.SetValue( aColor, meMode, mdRed ); break;
        case GREEN:         maColorSlider.SetValue( aColor, meMode, mdGreen ); break;
        case BLUE:          maColorSlider.SetValue( aColor, meMode, mdBlue ); break;
        }
    }

    if( n & UPDATE_HEX ) // update hex
    {
        maEDHex.SetColor( aColor.GetColor()  );
    }

    {
        Point aPos( 0, maColorSlider.GetLevel() + maColorSlider.GetPosPixel().Y() - 1 );

        aPos.X() = maFISliderLeft.GetPosPixel().X();
        if( aPos != maFISliderLeft.GetPosPixel() )
        {
            maFISliderLeft.SetPosPixel( aPos );

            aPos.X() = maFISliderRight.GetPosPixel().X();
            maFISliderRight.SetPosPixel( aPos );
        }
    }

    maColorPreview.SetColor( aColor );
}

// --------------------------------------------------------------------

IMPL_LINK( ColorPickerDialog, ColorModifyHdl, void *, p )
{
    sal_uInt16 n = 0;

    if( p == &maColorField )
    {
        double x = maColorField.GetX();
        double y = maColorField.GetY();

        switch( meMode )
        {
        case HUE:           mdSat = x; setColorComponent( COLORCOMP_BRI, y ); break;
        case SATURATION:    mdHue = x * 360.0; setColorComponent( COLORCOMP_BRI, y ); break;
        case BRIGHTNESS:    mdHue = x * 360.0; setColorComponent( COLORCOMP_SAT, y ); break;
        case RED:           mdBlue = x; setColorComponent( COLORCOMP_GREEN, y ); break;
        case GREEN:         mdBlue = x; setColorComponent( COLORCOMP_RED, y ); break;
        case BLUE:          mdRed = x; setColorComponent( COLORCOMP_GREEN, y ); break;
        }

        n = UPDATE_ALL&~(UPDATE_COLORCHOOSER);
    }
    else if( p == &maColorSlider )
    {
        double dValue = maColorSlider.GetValue();
        switch( meMode )
        {
        case HUE:           setColorComponent( COLORCOMP_HUE, dValue * 360.0 ); break;
        case SATURATION:    setColorComponent( COLORCOMP_SAT, dValue ); break;
        case BRIGHTNESS:    setColorComponent( COLORCOMP_BRI, dValue ); break;
        case RED:           setColorComponent( COLORCOMP_RED, dValue ); break;
        case GREEN:         setColorComponent( COLORCOMP_GREEN, dValue ); break;
        case BLUE:          setColorComponent( COLORCOMP_BLUE, dValue ); break;
        }

        n = UPDATE_ALL&~(UPDATE_COLORSLIDER);
    }
    else if( p == &maMFRed )
    {
        setColorComponent( COLORCOMP_RED, ((double)maMFRed.GetValue()) / 255.0 );
        n = UPDATE_ALL&~(UPDATE_RGB);
    }
    else if( p == &maMFGreen )
    {
        setColorComponent( COLORCOMP_GREEN, ((double)maMFGreen.GetValue()) / 255.0 );
        n = UPDATE_ALL&~(UPDATE_RGB);
    }
    else if( p == &maMFBlue )
    {
        setColorComponent( COLORCOMP_BLUE, ((double)maMFBlue.GetValue()) / 255.0 );
        n = UPDATE_ALL&~(UPDATE_RGB);
    }
    else if( p == &maMFHue )
    {
        setColorComponent( COLORCOMP_HUE, (double)maMFHue.GetValue() );
        n = UPDATE_ALL&~(UPDATE_HSB);
    }
    else if( p == &maMFSaturation )
    {
        setColorComponent( COLORCOMP_SAT, ((double)maMFSaturation.GetValue()) / 100.0 );
        n = UPDATE_ALL&~(UPDATE_HSB);
    }
    else if( p == &maMFBrightness )
    {
        setColorComponent( COLORCOMP_BRI, ((double)maMFBrightness.GetValue()) / 100.0 );
        n = UPDATE_ALL&~(UPDATE_HSB);
    }
    else if( p == &maMFCyan )
    {
        setColorComponent( COLORCOMP_CYAN, ((double)maMFCyan.GetValue()) / 100.0 );
        n = UPDATE_ALL&~(UPDATE_CMYK);
    }
    else if( p == &maMFMagenta )
    {
        setColorComponent( COLORCOMP_MAGENTA, ((double)maMFMagenta.GetValue()) / 100.0 );
        n = UPDATE_ALL&~(UPDATE_CMYK);
    }
    else if( p == &maMFYellow )
    {
        setColorComponent( COLORCOMP_YELLOW, ((double)maMFYellow.GetValue()) / 100.0 );
        n = UPDATE_ALL&~(UPDATE_CMYK);
    }
    else if( p == &maMFKey )
    {
        setColorComponent( COLORCOMP_KEY, ((double)maMFKey.GetValue()) / 100.0 );
        n = UPDATE_ALL&~(UPDATE_CMYK);
    }
    else if( p == &maEDHex )
    {
        sal_Int32 nColor = maEDHex.GetColor();

        if( nColor != -1 )
        {
            Color aColor( nColor );

            if( aColor != GetColor() )
            {
                mdRed = ((double)aColor.GetRed()) / 255.0;
                mdGreen = ((double)aColor.GetRed()) / 255.0;
                mdBlue = ((double)aColor.GetRed()) / 255.0;

                RGBtoHSV( mdRed, mdGreen, mdBlue, mdHue, mdSat, mdBri );
                RGBtoCMYK( mdRed, mdGreen, mdBlue, mdCyan, mdMagenta, mdYellow, mdKey );
                n = UPDATE_ALL&~(UPDATE_HEX);
            }
        }
    }

    if( n )
        update_color( n );

    return 0;
}

// --------------------------------------------------------------------

IMPL_LINK_NOARG(ColorPickerDialog, ModeModifyHdl)
{
    ColorMode eMode = HUE;

    if( maRBRed.IsChecked() )
    {
        eMode = RED;
    }
    else if( maRBGreen.IsChecked() )
    {
        eMode = GREEN;
    }
    else if( maRBBlue.IsChecked() )
    {
        eMode = BLUE;
    }
    else if( maRBSaturation.IsChecked() )
    {
        eMode = SATURATION;
    }
    else if( maRBBrightness.IsChecked() )
    {
        eMode = BRIGHTNESS;
    }

    if( meMode != eMode )
    {
        meMode = eMode;
        update_color( UPDATE_COLORCHOOSER | UPDATE_COLORSLIDER );
    }

    return 0;
}

// --------------------------------------------------------------------

void ColorPickerDialog::setColorComponent( sal_uInt16 nComp, double dValue )
{
    switch( nComp )
    {
    case COLORCOMP_RED:     mdRed = dValue; break;
    case COLORCOMP_GREEN:   mdGreen = dValue; break;
    case COLORCOMP_BLUE:    mdBlue = dValue; break;
    case COLORCOMP_HUE:     mdHue = dValue; break;
    case COLORCOMP_SAT:     mdSat = dValue; break;
    case COLORCOMP_BRI:     mdBri = dValue; break;
    case COLORCOMP_CYAN:    mdCyan = dValue; break;
    case COLORCOMP_YELLOW:  mdYellow = dValue; break;
    case COLORCOMP_MAGENTA: mdMagenta = dValue; break;
    case COLORCOMP_KEY:     mdKey = dValue; break;
    }

    if( nComp & COLORMODE_RGB )
    {
        RGBtoHSV( mdRed, mdGreen, mdBlue, mdHue, mdSat, mdBri );
        RGBtoCMYK( mdRed, mdGreen, mdBlue, mdCyan, mdMagenta, mdYellow, mdKey );
    }
    else if( nComp & COLORMODE_HSV )
    {
        HSVtoRGB( mdHue, mdSat, mdBri, mdRed, mdGreen, mdBlue );
        RGBtoCMYK( mdRed, mdGreen, mdBlue, mdCyan, mdMagenta, mdYellow, mdKey );
    }
    else
    {
        CMYKtoRGB( mdCyan, mdMagenta, mdYellow, mdKey, mdRed, mdGreen, mdBlue );
        RGBtoHSV( mdRed, mdGreen, mdBlue, mdHue, mdSat, mdBri );
    }
}

// --------------------------------------------------------------------

typedef ::cppu::WeakComponentImplHelper4< XServiceInfo, XExecutableDialog, XInitialization, XPropertyAccess > ColorPickerBase;

class ColorPicker : protected ::comphelper::OBaseMutex,    // Struct for right initalization of mutex member! Must be first of baseclasses.
                    public ColorPickerBase
{
public:
    ColorPicker( Reference< XComponentContext > const & xContext );

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException);

    // XInitialization
    virtual OUString SAL_CALL getImplementationName(  ) throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (RuntimeException);

    // XPropertyAccess
    virtual Sequence< PropertyValue > SAL_CALL getPropertyValues(  ) throw (RuntimeException);
    virtual void SAL_CALL setPropertyValues( const Sequence< PropertyValue >& aProps ) throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException);

    // XExecutableDialog
    virtual void SAL_CALL setTitle( const OUString& aTitle ) throw (RuntimeException);
    virtual sal_Int16 SAL_CALL execute(  ) throw (RuntimeException);

private:
    Reference< XComponentContext > mxContext;
    OUString msTitle;
    const OUString msColorKey;
    const OUString msModeKey;
    sal_Int32 mnColor;
    sal_Int16 mnMode;
    Reference< ::com::sun::star::awt::XWindow > mxParent;
};

// --------------------------------------------------------------------

OUString SAL_CALL ColorPicker_getImplementationName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.cui.ColorPicker" ) );
}

// --------------------------------------------------------------------

Reference< XInterface > SAL_CALL ColorPicker_createInstance( Reference< XComponentContext > const & xContext ) SAL_THROW( (Exception) )
{
    return static_cast<XWeak*>( new ColorPicker( xContext ) );
}

// --------------------------------------------------------------------

Sequence< OUString > SAL_CALL ColorPicker_getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > seq(1);
    seq[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.dialogs.ColorPicker" ) );
    return seq;
}

// --------------------------------------------------------------------

ColorPicker::ColorPicker( Reference< XComponentContext > const & xContext )
: ColorPickerBase( m_aMutex )
, mxContext( xContext )
, msColorKey( RTL_CONSTASCII_USTRINGPARAM( "Color" ) )
, msModeKey( RTL_CONSTASCII_USTRINGPARAM( "Mode" ) )
, mnColor( 0 )
, mnMode( 0 )
{
}

// --------------------------------------------------------------------

// XInitialization
void SAL_CALL ColorPicker::initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException)
{
    if( aArguments.getLength() == 1 )
    {
        aArguments[0] >>= mxParent;
    }
}

// --------------------------------------------------------------------

// XInitialization
OUString SAL_CALL ColorPicker::getImplementationName(  ) throw (RuntimeException)
{
    return ColorPicker_getImplementationName();
}

// --------------------------------------------------------------------

sal_Bool SAL_CALL ColorPicker::supportsService( const OUString& sServiceName ) throw (RuntimeException)
{
    return sServiceName == "com.sun.star.ui.dialogs.ColorPicker";
}

// --------------------------------------------------------------------

Sequence< OUString > SAL_CALL ColorPicker::getSupportedServiceNames(  ) throw (RuntimeException)
{
    return ColorPicker_getSupportedServiceNames();
}

// --------------------------------------------------------------------

// XPropertyAccess
Sequence< PropertyValue > SAL_CALL ColorPicker::getPropertyValues(  ) throw (RuntimeException)
{
    Sequence< PropertyValue > props(1);
    props[0].Name = msColorKey;
    props[0].Value <<= mnColor;
    return props;
}

// --------------------------------------------------------------------

void SAL_CALL ColorPicker::setPropertyValues( const Sequence< PropertyValue >& aProps ) throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    for( sal_Int32 n = 0; n < aProps.getLength(); n++ )
    {
        if( aProps[n].Name.equals( msColorKey ) )
        {
            aProps[n].Value >>= mnColor;
        }
        else if( aProps[n].Name.equals( msModeKey ) )
        {
            aProps[n].Value >>= mnMode;
        }
    }
}

// --------------------------------------------------------------------

// XExecutableDialog
void SAL_CALL ColorPicker::setTitle( const OUString& sTitle ) throw (RuntimeException)
{
    msTitle = sTitle;
}

// --------------------------------------------------------------------

sal_Int16 SAL_CALL ColorPicker::execute(  ) throw (RuntimeException)
{
    ColorPickerDialog aDlg( VCLUnoHelper::GetWindow( mxParent ), mnColor, mnMode );
    sal_Int16 ret = aDlg.Execute();
    if( ret )
        mnColor = aDlg.GetColor();

    return ret;
}

// --------------------------------------------------------------------

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/decoview.hxx>
#include <vcl/svapp.hxx>
#include <vcl/builderfactory.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <sax/tools/converter.hxx>
#include <basegfx/color/bcolortools.hxx>
#include "dialmgr.hxx"
#include "colorpicker.hxx"
#include <cmath>
#include <limits>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::beans;
using namespace ::basegfx;

namespace cui
{
const sal_uInt16 COLORMODE_RGB =  0x10;
const sal_uInt16 COLORMODE_HSV =  0x20;

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
    BColor result = basegfx::tools::rgb2hsv( BColor( dR, dG, dB ) );

    dH = result.getX();
    dS = result.getY();
    dV = result.getZ();
}

static void HSVtoRGB(double dH, double dS, double dV, double& dR, double& dG, double& dB )
{
    BColor result = basegfx::tools::hsv2rgb( BColor( dH, dS, dV ) );

    dR = result.getRed();
    dG = result.getGreen();
    dB = result.getBlue();
}

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

class HexColorControl : public Edit
{
public:
    HexColorControl( vcl::Window* pParent, const WinBits& nStyle );

    virtual bool PreNotify( NotifyEvent& rNEvt ) override;
    virtual void Paste() override;

    void SetColor( sal_Int32 nColor );
    sal_Int32 GetColor();

private:
    static bool ImplProcessKeyInput( const KeyEvent& rKEv );
};

HexColorControl::HexColorControl( vcl::Window* pParent, const WinBits& nStyle )
    : Edit(pParent, nStyle)
{
    SetMaxTextLen( 6 );
}

VCL_BUILDER_FACTORY_ARGS(HexColorControl, WB_BORDER)

void HexColorControl::SetColor(sal_Int32 nColor)
{
    OUStringBuffer aBuffer;
    sax::Converter::convertColor(aBuffer, nColor);
    SetText(aBuffer.makeStringAndClear().copy(1));
}

sal_Int32 HexColorControl::GetColor()
{
    sal_Int32 nColor = -1;

    OUString aStr("#");
    aStr += GetText();
    sal_Int32 nLen = aStr.getLength();

    if (nLen < 7)
    {
        static const sal_Char* pNullStr = "000000";
        aStr += OUString::createFromAscii( &pNullStr[nLen-1] );
    }

    sax::Converter::convertColor(nColor, aStr);

    if (nColor == -1)
        SetControlBackground(Color(COL_RED));
    else
        SetControlBackground();

    return nColor;
}

bool HexColorControl::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplProcessKeyInput( *rNEvt.GetKeyEvent() ) )
            return true;
    }

    return Edit::PreNotify( rNEvt );
}

void HexColorControl::Paste()
{
    css::uno::Reference<css::datatransfer::clipboard::XClipboard> aClipboard(GetClipboard());
    if (aClipboard.is())
    {
        css::uno::Reference<css::datatransfer::XTransferable> xDataObj;

        try
        {
            SolarMutexReleaser aReleaser;
            xDataObj = aClipboard->getContents();
        }
        catch (const css::uno::Exception&)
        {
        }

        if (xDataObj.is())
        {
            css::datatransfer::DataFlavor aFlavor;
            SotExchange::GetFormatDataFlavor(SotClipboardFormatId::STRING, aFlavor);
            try
            {
                css::uno::Any aData = xDataObj->getTransferData(aFlavor);
                OUString aText;
                aData >>= aText;

                if( !aText.isEmpty() && aText.startsWith( "#" ) )
                    aText = aText.copy(1);

                if( aText.getLength() > 6 )
                    aText = aText.copy( 0, 6 );

                SetText(aText);
            }
            catch(const css::uno::Exception&)
            {}
        }
    }
}

bool HexColorControl::ImplProcessKeyInput( const KeyEvent& rKEv )
{
    const vcl::KeyCode& rKeyCode = rKEv.GetKeyCode();

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

class ColorPreviewControl : public Control
{
public:
    ColorPreviewControl( vcl::Window* pParent, const WinBits& nStyle );

    virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) override;

    void SetColor(const Color& rColor);

private:
    Color maColor;
};

ColorPreviewControl::ColorPreviewControl(vcl::Window* pParent, const WinBits& nStyle)
    : Control(pParent, nStyle)
{
}

VCL_BUILDER_DECL_FACTORY(ColorPreviewControl)
{
    WinBits nBits = 0;

    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nBits |= WB_BORDER;

    rRet = VclPtr<ColorPreviewControl>::Create(pParent, nBits);
}

void ColorPreviewControl::SetColor( const Color& rCol )
{
    if (rCol != maColor)
    {
        maColor = rCol;
        Invalidate();
    }
}

void ColorPreviewControl::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    rRenderContext.SetFillColor(maColor);
    rRenderContext.SetLineColor(maColor);
    rRenderContext.DrawRect(rRect);
}

enum ColorMode { HUE, SATURATION, BRIGHTNESS, RED, GREEN, BLUE };
const ColorMode DefaultMode = HUE;

class ColorFieldControl : public Control
{
public:
    ColorFieldControl(vcl::Window* pParent, const WinBits& nStyle);
    virtual ~ColorFieldControl();

    virtual void dispose() override;

    virtual void MouseMove( const MouseEvent& rMEvt ) override;
    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void KeyInput( const KeyEvent& rKEvt ) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) override;
    virtual void Resize() override;

    virtual Size GetOptimalSize() const override;

    void UpdateBitmap();
    void ShowPosition( const Point& rPos, bool bUpdate );
    void UpdatePosition();
    void Modify();

    void SetValues(Color aColor, ColorMode eMode, double x, double y);
    double GetX() { return mdX;}
    double GetY() { return mdY;}

    void KeyMove(int dx, int dy);

    void SetModifyHdl(const Link<ColorFieldControl&,void>& rLink) { maModifyHdl = rLink; }

private:
    Link<ColorFieldControl&,void> maModifyHdl;
    ColorMode meMode;
    Color maColor;
    double mdX;
    double mdY;
    Point maPosition;
    Bitmap* mpBitmap;
    std::vector<sal_uInt8>  maRGB_Horiz;
    std::vector<sal_uInt16> maGrad_Horiz;
    std::vector<sal_uInt16> maPercent_Horiz;
    std::vector<sal_uInt8>  maRGB_Vert;
    std::vector<sal_uInt16> maPercent_Vert;
};

ColorFieldControl::ColorFieldControl( vcl::Window* pParent, const WinBits& nStyle )
: Control( pParent, nStyle )
, meMode( DefaultMode )
, mdX( -1.0 )
, mdY( -1.0 )
, mpBitmap( nullptr )
{
    SetControlBackground();
}

ColorFieldControl::~ColorFieldControl()
{
    disposeOnce();
}

void ColorFieldControl::dispose()
{
    delete mpBitmap;
    mpBitmap = nullptr;
    Control::dispose();
}

VCL_BUILDER_DECL_FACTORY(ColorFieldControl)
{
    WinBits nBits = 0;

    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nBits |= WB_BORDER;

    rRet = VclPtr<ColorFieldControl>::Create(pParent, nBits);
}

Size ColorFieldControl::GetOptimalSize() const
{
    return LogicToPixel(Size(158, 158), MAP_APPFONT);
}

void ColorFieldControl::UpdateBitmap()
{
    const Size aSize(GetOutputSizePixel());

    if (mpBitmap && mpBitmap->GetSizePixel() != aSize)
        delete mpBitmap, mpBitmap = nullptr;

    const sal_Int32 nWidth = aSize.Width();
    const sal_Int32 nHeight = aSize.Height();

    if (nWidth == 0 || nHeight == 0)
        return;

    if (!mpBitmap)
    {
        mpBitmap = new Bitmap( aSize, 24 );

        maRGB_Horiz.resize( nWidth );
        maGrad_Horiz.resize( nWidth );
        maPercent_Horiz.resize( nWidth );

        sal_uInt8* pRGB = maRGB_Horiz.data();
        sal_uInt16* pGrad = maGrad_Horiz.data();
        sal_uInt16* pPercent = maPercent_Horiz.data();

        for( sal_Int32 x = 0; x < nWidth; x++ )
        {
            *pRGB++ = static_cast<sal_uInt8>((x * 256) / nWidth);
            *pGrad++ = static_cast<sal_uInt16>((x * 359) / nWidth);
            *pPercent++ = static_cast<sal_uInt16>((x * 100) / nWidth);
        }

        maRGB_Vert.resize(nHeight);
        maPercent_Vert.resize(nHeight);

        pRGB = maRGB_Vert.data();
        pPercent = maPercent_Vert.data();

        sal_Int32 y = nHeight;
        while (y--)
        {
            *pRGB++ = static_cast<sal_uInt8>((y * 256) / nHeight);
            *pPercent++ = static_cast<sal_uInt16>((y * 100) / nHeight);
        }
    }

    sal_uInt8* pRGB_Horiz = maRGB_Horiz.data();
    sal_uInt16* pGrad_Horiz = maGrad_Horiz.data();
    sal_uInt16* pPercent_Horiz = maPercent_Horiz.data();
    sal_uInt8* pRGB_Vert = maRGB_Vert.data();
    sal_uInt16* pPercent_Vert = maPercent_Vert.data();

    BitmapWriteAccess* pWriteAccess = mpBitmap->AcquireWriteAccess();
    if (pWriteAccess)
    {
        BitmapColor aBitmapColor(maColor);

        sal_uInt16 nHue, nSat, nBri;
        maColor.RGBtoHSB(nHue, nSat, nBri);

        // this has been unlooped for performance reason, please do not merge back!

        sal_uInt16 y = nHeight,x;

        switch(meMode)
        {
        case HUE:
            while (y--)
            {
                nBri = pPercent_Vert[y];
                x = nWidth;
                while (x--)
                {
                    nSat = pPercent_Horiz[x];
                    pWriteAccess->SetPixel(y, x, BitmapColor(Color(Color::HSBtoRGB(nHue, nSat, nBri))));
                }
            }
            break;
        case SATURATION:
            while (y--)
            {
                nBri = pPercent_Vert[y];
                x = nWidth;
                while (x--)
                {
                    nHue = pGrad_Horiz[x];
                    pWriteAccess->SetPixel(y, x, BitmapColor(Color(Color::HSBtoRGB(nHue, nSat, nBri))));
                }
            }
            break;
        case BRIGHTNESS:
            while (y--)
            {
                nSat = pPercent_Vert[y];
                x = nWidth;
                while (x--)
                {
                    nHue = pGrad_Horiz[x];
                    pWriteAccess->SetPixel(y, x, BitmapColor(Color(Color::HSBtoRGB(nHue, nSat, nBri))));
                }
            }
            break;
        case RED:
            while (y--)
            {
                aBitmapColor.SetGreen(pRGB_Vert[y]);
                x = nWidth;
                while (x--)
                {
                    aBitmapColor.SetBlue(pRGB_Horiz[x]);
                    pWriteAccess->SetPixel(y, x, aBitmapColor);
                }
            }
            break;
        case GREEN:
            while (y--)
            {
                aBitmapColor.SetRed(pRGB_Vert[y]);
                x = nWidth;
                while (x--)
                {
                    aBitmapColor.SetBlue(pRGB_Horiz[x]);
                    pWriteAccess->SetPixel(y, x, aBitmapColor);
                }
            }
            break;
        case BLUE:
            while (y--)
            {
                aBitmapColor.SetGreen(pRGB_Vert[y]);
                x = nWidth;
                while (x--)
                {
                    aBitmapColor.SetRed(pRGB_Horiz[x]);
                    pWriteAccess->SetPixel(y, x, aBitmapColor);
                }
            }
            break;
        }

        Bitmap::ReleaseAccess(pWriteAccess);
    }
}

void ColorFieldControl::ShowPosition( const Point& rPos, bool bUpdate )
{
    if (!mpBitmap)
    {
        UpdateBitmap();
        Invalidate();
    }

    if (!mpBitmap)
        return;

    const Size aSize(mpBitmap->GetSizePixel());

    long nX = rPos.X();
    long nY = rPos.Y();
    if (nX < 0L)
        nX = 0L;
    else if (nX >= aSize.Width())
        nX = aSize.Width() - 1L;

    if (nY < 0L)
        nY = 0L;
    else if (nY >= aSize.Height())
        nY = aSize.Height() - 1L;

    Point aPos = maPosition;
    maPosition.X() = nX - 5;
    maPosition.Y() = nY - 5;
    Invalidate(Rectangle(aPos, Size(11, 11)));
    Invalidate(Rectangle(maPosition, Size(11, 11)));

    if (bUpdate)
    {
        mdX = double(nX) / double(aSize.Width() - 1.0);
        mdY = double(aSize.Height() - 1.0 - nY) / double(aSize.Height() - 1.0);

        BitmapReadAccess* pReadAccess = mpBitmap->AcquireReadAccess();
        if (pReadAccess != nullptr)
        {
            // mpBitmap always has a bit count of 24 => use of GetPixel(...) is safe
            maColor = pReadAccess->GetPixel(nY, nX);
            Bitmap::ReleaseAccess(pReadAccess);
            pReadAccess = nullptr;
        }
    }
}

void ColorFieldControl::MouseMove( const MouseEvent& rMEvt )
{
    if( rMEvt.IsLeft() )
    {
        ShowPosition( rMEvt.GetPosPixel(), true );
        Modify();
    }
}

void ColorFieldControl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( rMEvt.IsLeft() && !rMEvt.IsShift() )
    {
        CaptureMouse();
        ShowPosition( rMEvt.GetPosPixel(), true );
        Modify();
    }
}

void ColorFieldControl::MouseButtonUp( const MouseEvent& )
{
    if( IsMouseCaptured() )
        ReleaseMouse();
}

void ColorFieldControl::KeyMove( int dx, int dy )
{
    Size aSize(GetOutputSizePixel());
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

void ColorFieldControl::KeyInput( const KeyEvent& rKEvt )
{
    bool   bShift = rKEvt.GetKeyCode().IsShift();
    bool   bCtrl = rKEvt.GetKeyCode().IsMod1();
    bool   bAlt = rKEvt.GetKeyCode().IsMod2();

    if (!bAlt && !bShift)
    {
        switch( rKEvt.GetKeyCode().GetCode() )
        {
        case KEY_DOWN:
            KeyMove(0, bCtrl ? 5 : 1);
            return;
        case KEY_UP:
            KeyMove(0, bCtrl ? -5 : -1);
            return;
        case KEY_LEFT:
            KeyMove(bCtrl ? -5 : -1,  0);
            return;
        case KEY_RIGHT:
            KeyMove(bCtrl ? 5 :  1,  0);
            return;
        }
    }
    Control::KeyInput(rKEvt);
}

void ColorFieldControl::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    if (!mpBitmap)
        UpdateBitmap();

    if (mpBitmap)
    {
        Bitmap aOutputBitmap(*mpBitmap);

        if (GetBitCount() <= 8)
            aOutputBitmap.Dither();

        rRenderContext.DrawBitmap(rRect.TopLeft(), rRect.GetSize(), rRect.TopLeft(), rRect.GetSize(), aOutputBitmap);
    }

    // draw circle around current color
    if (maColor.IsDark())
        rRenderContext.SetLineColor( COL_WHITE );
    else
        rRenderContext.SetLineColor( COL_BLACK );

    rRenderContext.SetFillColor();

    rRenderContext.DrawEllipse(Rectangle(maPosition, Size(11, 11)));
}

void ColorFieldControl::Resize()
{
    UpdateBitmap();
    UpdatePosition();
    Control::Resize();
}

void ColorFieldControl::Modify()
{
    maModifyHdl.Call( *this );
}

void ColorFieldControl::SetValues( Color aColor, ColorMode eMode, double x, double y )
{
    bool bUpdateBitmap = (maColor!= aColor) || (meMode != eMode);
    if( bUpdateBitmap || (mdX != x) || (mdY != y) )
    {
        maColor = aColor;
        meMode = eMode;
        mdX = x;
        mdY = y;

        if (bUpdateBitmap)
            UpdateBitmap();
        UpdatePosition();
        if (bUpdateBitmap)
            Invalidate();
    }
}

void ColorFieldControl::UpdatePosition()
{
    Size aSize(GetOutputSizePixel());
    ShowPosition(Point(static_cast<long>(mdX * aSize.Width()), static_cast<long>((1.0 - mdY) * aSize.Height())), false);
}

class ColorSliderControl : public Control
{
public:
    ColorSliderControl( vcl::Window* pParent, const WinBits& nStyle );
    virtual ~ColorSliderControl();
    virtual void dispose() override;

    virtual void MouseMove( const MouseEvent& rMEvt ) override;
    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void KeyInput( const KeyEvent& rKEvt ) override;
    virtual void Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
    virtual void Resize() override;

    void UpdateBitmap();
    void ChangePosition( long nY );
    void Modify();

    void SetValue( const Color& rColor, ColorMode eMode, double dValue );
    double GetValue() const { return mdValue; }

    void KeyMove( int dy );

    void SetModifyHdl( const Link<ColorSliderControl&,void>& rLink ) { maModifyHdl = rLink; }

    sal_Int16 GetLevel() const { return mnLevel; }

private:
    Link<ColorSliderControl&,void> maModifyHdl;
    Color maColor;
    ColorMode meMode;
    Bitmap* mpBitmap;
    sal_Int16 mnLevel;
    double mdValue;
};

ColorSliderControl::ColorSliderControl( vcl::Window* pParent, const WinBits& nStyle )
    : Control( pParent, nStyle )
    , meMode( DefaultMode )
    , mpBitmap( nullptr )
    , mnLevel( 0 )
    , mdValue( -1.0 )
{
    SetControlBackground();
}

ColorSliderControl::~ColorSliderControl()
{
    disposeOnce();
}

void ColorSliderControl::dispose()
{
    delete mpBitmap;
    mpBitmap = nullptr;
    Control::dispose();
}

VCL_BUILDER_DECL_FACTORY(ColorSliderControl)
{
    WinBits nBits = 0;

    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nBits |= WB_BORDER;

    rRet = VclPtr<ColorSliderControl>::Create(pParent, nBits);
}

void ColorSliderControl::UpdateBitmap()
{
    Size aSize(1, GetOutputSizePixel().Height());

    if (mpBitmap && mpBitmap->GetSizePixel() != aSize)
        delete mpBitmap, mpBitmap = nullptr;

    if (!mpBitmap)
        mpBitmap = new Bitmap(aSize, 24);

    BitmapWriteAccess* pWriteAccess = mpBitmap->AcquireWriteAccess();

    if (pWriteAccess)
    {
        const long nY = aSize.Height() - 1;

        BitmapColor aBitmapColor(maColor);

        sal_uInt16 nHue, nSat, nBri;
        maColor.RGBtoHSB(nHue, nSat, nBri);

        // this has been unlooped for performance reason, please do not merge back!

        switch (meMode)
        {
        case HUE:
            nSat = 100;
            nBri = 100;
            for (long y = 0; y <= nY; y++)
            {
                nHue = static_cast<sal_uInt16>((359 * y) / nY);
                aBitmapColor = BitmapColor(Color(Color::HSBtoRGB(nHue, nSat, nBri)));
                pWriteAccess->SetPixel(nY - y, 0, aBitmapColor);
            }
            break;

        case SATURATION:
            nBri = std::max(sal_uInt16(32), nBri);
            for (long y = 0; y <= nY; y++)
            {
                nSat = static_cast<sal_uInt16>((100 * y) / nY);
                pWriteAccess->SetPixel(nY - y, 0, BitmapColor(Color(Color::HSBtoRGB(nHue, nSat, nBri))));
            }
            break;

        case BRIGHTNESS:
            for (long y = 0; y <= nY; y++)
            {
                nBri = static_cast<sal_uInt16>((100 * y) / nY);
                pWriteAccess->SetPixel(nY - y, 0, BitmapColor(Color(Color::HSBtoRGB(nHue, nSat, nBri))));
            }
            break;

        case RED:
            for (long y = 0; y <= nY; y++)
            {
                aBitmapColor.SetRed(sal_uInt8(((long)255 * y) / nY));
                pWriteAccess->SetPixel(nY - y, 0, aBitmapColor);
            }
            break;

        case GREEN:
            for (long y = 0; y <= nY; y++)
            {
                aBitmapColor.SetGreen(sal_uInt8(((long)255 * y) / nY));
                pWriteAccess->SetPixel(nY - y, 0, aBitmapColor);
            }
            break;

        case BLUE:
            for (long y = 0; y <= nY; y++)
            {
                aBitmapColor.SetBlue(sal_uInt8(((long)255 * y) / nY));
                pWriteAccess->SetPixel(nY - y, 0, aBitmapColor);
            }
            break;
        }

        Bitmap::ReleaseAccess(pWriteAccess);
    }
}

void ColorSliderControl::ChangePosition(long nY)
{
    const long nHeight = GetOutputSizePixel().Height() - 1;

    if (nY < 0L)
        nY = 0;
    else if (nY > nHeight)
        nY = nHeight;

    mnLevel = nY;
    mdValue = double(nHeight - nY) / double(nHeight);
}

void ColorSliderControl::MouseMove( const MouseEvent& rMEvt )
{
    if (rMEvt.IsLeft())
    {
        ChangePosition(rMEvt.GetPosPixel().Y());
        Modify();
    }
}

void ColorSliderControl::MouseButtonDown(const MouseEvent& rMEvt)
{
    if (rMEvt.IsLeft() && !rMEvt.IsShift())
    {
        CaptureMouse();
        ChangePosition( rMEvt.GetPosPixel().Y() );
        Modify();
    }
}

void ColorSliderControl::MouseButtonUp(const MouseEvent&)
{
    if (IsMouseCaptured())
        ReleaseMouse();
}

void ColorSliderControl::KeyMove(int dy)
{
    ChangePosition( mnLevel + dy );
    Modify();
}

void ColorSliderControl::KeyInput(const KeyEvent& rKEvt)
{
    if (!rKEvt.GetKeyCode().IsMod2() && !rKEvt.GetKeyCode().IsShift())
    {
        switch (rKEvt.GetKeyCode().GetCode())
        {
        case KEY_DOWN:
            KeyMove(rKEvt.GetKeyCode().IsMod1() ?  5 :  1);
            return;
        case KEY_UP:
            KeyMove(rKEvt.GetKeyCode().IsMod1() ? -5 : -1);
            return;
        }
    }

    Control::KeyInput( rKEvt );
}

void ColorSliderControl::Paint(vcl::RenderContext& rRenderContext, const Rectangle& /*rRect*/)
{
    if (!mpBitmap)
        UpdateBitmap();

    const Size aSize(GetOutputSizePixel());

    Bitmap aOutputBitmap(*mpBitmap);

    if (GetBitCount() <= 8)
        aOutputBitmap.Dither();

    Point aPos;
    int x = aSize.Width();
    while (x--)
    {
        rRenderContext.DrawBitmap(aPos, aOutputBitmap);
        aPos.X() += 1;
    }
}

void ColorSliderControl::Resize()
{
    UpdateBitmap();
    Control::Resize();
}

void ColorSliderControl::Modify()
{
    maModifyHdl.Call(*this);
}

void ColorSliderControl::SetValue(const Color& rColor, ColorMode eMode, double dValue)
{
    bool bUpdateBitmap = (rColor != maColor) || (eMode != meMode);
    if( bUpdateBitmap || (mdValue != dValue))
    {
        maColor = rColor;
        mdValue = dValue;
        mnLevel = static_cast<sal_Int16>((1.0-dValue) * GetOutputSizePixel().Height());
        meMode = eMode;
        if (bUpdateBitmap)
            UpdateBitmap();
        Invalidate();
    }
}

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
    ColorPickerDialog(vcl::Window* pParent, sal_Int32 nColor, sal_Int16 nMode);
    virtual ~ColorPickerDialog()
    {
        disposeOnce();
    }
    virtual void dispose() override;

    void update_color(sal_uInt16 n = UPDATE_ALL);

    DECL_LINK_TYPED(ColorFieldControlModifydl, ColorFieldControl&, void);
    DECL_LINK_TYPED(ColorSliderControlModifyHdl, ColorSliderControl&, void);
    DECL_LINK_TYPED(ColorModifyEditHdl, Edit&, void);
    DECL_LINK_TYPED(ModeModifyHdl, RadioButton&, void);

    sal_Int32 GetColor() const;

    void setColorComponent(sal_uInt16 nComp, double dValue);

private:
    sal_Int16 mnDialogMode;
    ColorMode meMode;

    double mdRed, mdGreen, mdBlue;
    double mdHue, mdSat, mdBri;
    double mdCyan, mdMagenta, mdYellow, mdKey;

private:
    VclPtr<ColorFieldControl>    mpColorField;
    VclPtr<ColorSliderControl>   mpColorSlider;
    VclPtr<ColorPreviewControl>  mpColorPreview;
    VclPtr<ColorPreviewControl>  mpColorPrevious;

    VclPtr<FixedImage>   mpFISliderLeft;
    VclPtr<FixedImage>   mpFISliderRight;
    Image         maSliderImage;

    VclPtr<RadioButton>    mpRBRed;
    VclPtr<RadioButton>    mpRBGreen;
    VclPtr<RadioButton>    mpRBBlue;
    VclPtr<RadioButton>    mpRBHue;
    VclPtr<RadioButton>    mpRBSaturation;
    VclPtr<RadioButton>    mpRBBrightness;

    VclPtr<MetricField>        mpMFRed;
    VclPtr<MetricField>        mpMFGreen;
    VclPtr<MetricField>        mpMFBlue;
    VclPtr<HexColorControl>    mpEDHex;

    VclPtr<MetricField>    mpMFHue;
    VclPtr<MetricField>    mpMFSaturation;
    VclPtr<MetricField>    mpMFBrightness;

    VclPtr<MetricField>    mpMFCyan;
    VclPtr<MetricField>    mpMFMagenta;
    VclPtr<MetricField>    mpMFYellow;
    VclPtr<MetricField>    mpMFKey;
};

ColorPickerDialog::ColorPickerDialog( vcl::Window* pParent, sal_Int32 nColor, sal_Int16 nMode )
: ModalDialog( pParent, "ColorPicker", "cui/ui/colorpickerdialog.ui" )
, mnDialogMode( nMode )
, meMode( DefaultMode )
, maSliderImage( FixedImage::loadThemeImage("res/colorslider.png") )
{
    get(mpColorField, "colorField");
    get(mpColorSlider, "colorSlider");
    get(mpColorPreview, "preview");
    get(mpColorPrevious, "previous");
    get(mpRBRed, "redRadiobutton");
    get(mpRBGreen, "greenRadiobutton");
    get(mpRBBlue, "blueRadiobutton");
    get(mpRBHue, "hueRadiobutton");
    get(mpRBSaturation, "satRadiobutton");
    get(mpRBBrightness, "brightRadiobutton");
    get(mpMFRed, "redSpinbutton");
    get(mpMFGreen, "greenSpinbutton");
    get(mpMFBlue, "blueSpinbutton");
    get(mpEDHex, "hexEntry");
    get(mpMFHue, "hueSpinbutton");
    get(mpMFSaturation, "satSpinbutton");
    get(mpMFBrightness, "brightSpinbutton");
    get(mpMFCyan, "cyanSpinbutton");
    get(mpMFMagenta, "magSpinbutton");
    get(mpMFYellow, "yellowSpinbutton");
    get(mpMFKey, "keySpinbutton");
    get(mpFISliderLeft, "leftImage");
    get(mpFISliderRight, "rightImage");

    Size aDialogSize = get_preferred_size();
    set_width_request(aDialogSize.Width() + 50);
    set_height_request(aDialogSize.Height() + 30);

    mpColorField->SetModifyHdl( LINK( this, ColorPickerDialog, ColorFieldControlModifydl ) );
    mpColorSlider->SetModifyHdl( LINK( this, ColorPickerDialog, ColorSliderControlModifyHdl ) );

    Link<Edit&,void> aLink3( LINK( this, ColorPickerDialog, ColorModifyEditHdl ) );
    mpMFRed->SetModifyHdl( aLink3 );
    mpMFGreen->SetModifyHdl( aLink3 );
    mpMFBlue->SetModifyHdl( aLink3 );

    mpMFCyan->SetModifyHdl( aLink3 );
    mpMFMagenta->SetModifyHdl( aLink3 );
    mpMFYellow->SetModifyHdl( aLink3 );
    mpMFKey->SetModifyHdl( aLink3 );

    mpMFHue->SetModifyHdl( aLink3 );
    mpMFSaturation->SetModifyHdl( aLink3 );
    mpMFBrightness->SetModifyHdl( aLink3 );

    mpEDHex->SetModifyHdl( aLink3 );

    Link<RadioButton&,void> aLink2 = LINK( this, ColorPickerDialog, ModeModifyHdl );
    mpRBRed->SetToggleHdl( aLink2 );
    mpRBGreen->SetToggleHdl( aLink2 );
    mpRBBlue->SetToggleHdl( aLink2 );
    mpRBHue->SetToggleHdl( aLink2 );
    mpRBSaturation->SetToggleHdl( aLink2 );
    mpRBBrightness->SetToggleHdl( aLink2 );

    Image aSliderImage( maSliderImage );

    mpFISliderLeft->SetImage( aSliderImage );
    mpFISliderLeft->Show();

    BitmapEx aTmpBmp( maSliderImage.GetBitmapEx() );
    aTmpBmp.Mirror( BmpMirrorFlags::Horizontal );
    mpFISliderRight->SetImage( Image( aTmpBmp  ) );

    Size aSize( maSliderImage.GetSizePixel() );
    mpFISliderLeft->SetSizePixel( aSize );
    mpFISliderRight->SetSizePixel( aSize );

    Point aPos( mpColorSlider->GetPosPixel() );

    aPos.X() -= aSize.Width();
    aPos.Y() -= aSize.Height() / 2;
    mpFISliderLeft->SetPosPixel( aPos );

    aPos.X() += aSize.Width() + mpColorSlider->GetSizePixel().Width();
    mpFISliderRight->SetPosPixel( aPos );

    Color aColor( nColor );

    // modify
    if( mnDialogMode == 2 )
    {
        mpColorPreview->SetSizePixel( mpColorPrevious->GetSizePixel() );
        mpColorPrevious->SetColor( aColor );
        mpColorPrevious->Show();
    }

    mdRed = ((double)aColor.GetRed()) / 255.0;
    mdGreen = ((double)aColor.GetGreen()) / 255.0;
    mdBlue = ((double)aColor.GetBlue()) / 255.0;

    RGBtoHSV( mdRed, mdGreen, mdBlue, mdHue, mdSat, mdBri );
    RGBtoCMYK( mdRed, mdGreen, mdBlue, mdCyan, mdMagenta, mdYellow, mdKey );

    update_color();
}

void ColorPickerDialog::dispose()
{
    mpColorField.clear();
    mpColorSlider.clear();
    mpColorPreview.clear();
    mpColorPrevious.clear();
    mpFISliderLeft.clear();
    mpFISliderRight.clear();
    mpRBRed.clear();
    mpRBGreen.clear();
    mpRBBlue.clear();
    mpRBHue.clear();
    mpRBSaturation.clear();
    mpRBBrightness.clear();
    mpMFRed.clear();
    mpMFGreen.clear();
    mpMFBlue.clear();
    mpEDHex.clear();
    mpMFHue.clear();
    mpMFSaturation.clear();
    mpMFBrightness.clear();
    mpMFCyan.clear();
    mpMFMagenta.clear();
    mpMFYellow.clear();
    mpMFKey.clear();
    ModalDialog::dispose();
}

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

    Color aColor(nRed, nGreen, nBlue);

    if (n & UPDATE_RGB) // update RGB
    {
        mpMFRed->SetValue(nRed);
        mpMFGreen->SetValue(nGreen);
        mpMFBlue->SetValue(nBlue);
    }

    if (n & UPDATE_CMYK) // update CMYK
    {
        mpMFCyan->SetValue(toInt(mdCyan, 100.0));
        mpMFMagenta->SetValue(toInt(mdMagenta, 100.0));
        mpMFYellow->SetValue(toInt(mdYellow, 100.0));
        mpMFKey->SetValue(toInt(mdKey, 100.0));
    }

    if (n & UPDATE_HSB ) // update HSB
    {
        mpMFHue->SetValue(toInt(mdHue, 1.0));
        mpMFSaturation->SetValue(toInt( mdSat, 100.0));
        mpMFBrightness->SetValue(toInt( mdBri, 100.0));
    }

    if (n & UPDATE_COLORCHOOSER ) // update Color Chooser 1
    {
        switch( meMode )
        {
        case HUE:
            mpColorField->SetValues(aColor, meMode, mdSat, mdBri);
            break;
        case SATURATION:
            mpColorField->SetValues(aColor, meMode, mdHue / 360.0, mdBri);
            break;
        case BRIGHTNESS:
            mpColorField->SetValues(aColor, meMode, mdHue / 360.0, mdSat);
            break;
        case RED:
            mpColorField->SetValues(aColor, meMode, mdBlue, mdGreen);
            break;
        case GREEN:
            mpColorField->SetValues(aColor, meMode, mdBlue, mdRed);
            break;
        case BLUE:
            mpColorField->SetValues(aColor, meMode, mdRed, mdGreen);
            break;
        }
    }

    if (n & UPDATE_COLORSLIDER) // update Color Chooser 2
    {
        switch (meMode)
        {
        case HUE:
            mpColorSlider->SetValue(aColor, meMode, mdHue / 360.0);
            break;
        case SATURATION:
            mpColorSlider->SetValue(aColor, meMode, mdSat);
            break;
        case BRIGHTNESS:
            mpColorSlider->SetValue(aColor, meMode, mdBri);
            break;
        case RED:
            mpColorSlider->SetValue(aColor, meMode, mdRed);
            break;
        case GREEN:
            mpColorSlider->SetValue(aColor, meMode, mdGreen);
            break;
        case BLUE:
            mpColorSlider->SetValue(aColor, meMode, mdBlue);
            break;
        }
    }

    if (n & UPDATE_HEX) // update hex
    {
        mpEDHex->SetColor(aColor.GetColor());
    }

    {
        Point aPos(0, mpColorSlider->GetLevel() + mpColorSlider->GetPosPixel().Y() - 1);

        aPos.X() = mpFISliderLeft->GetPosPixel().X();
        if (aPos != mpFISliderLeft->GetPosPixel())
        {
            mpFISliderLeft->SetPosPixel(aPos);

            aPos.X() = mpFISliderRight->GetPosPixel().X();
            mpFISliderRight->SetPosPixel(aPos);
        }
    }

    mpColorPreview->SetColor(aColor);
}

IMPL_LINK_NOARG_TYPED(ColorPickerDialog, ColorFieldControlModifydl, ColorFieldControl&, void)
{
    sal_uInt16 n = 0;

    double x = mpColorField->GetX();
    double y = mpColorField->GetY();

    switch( meMode )
    {
    case HUE:
        mdSat = x;
        setColorComponent( COLORCOMP_BRI, y );
        break;
    case SATURATION:
        mdHue = x * 360.0;
        setColorComponent( COLORCOMP_BRI, y );
        break;
    case BRIGHTNESS:
        mdHue = x * 360.0;
        setColorComponent( COLORCOMP_SAT, y );
        break;
    case RED:
        mdBlue = x;
        setColorComponent( COLORCOMP_GREEN, y );
        break;
    case GREEN:
        mdBlue = x;
        setColorComponent( COLORCOMP_RED, y );
        break;
    case BLUE:
        mdRed = x;
        setColorComponent( COLORCOMP_GREEN, y );
        break;
    }

    n = UPDATE_ALL &~ (UPDATE_COLORCHOOSER);

    if (n)
        update_color(n);

}
IMPL_LINK_NOARG_TYPED(ColorPickerDialog, ColorSliderControlModifyHdl, ColorSliderControl&, void)
{
    sal_uInt16 n = 0;

    double dValue = mpColorSlider->GetValue();
    switch (meMode)
    {
    case HUE:
        setColorComponent( COLORCOMP_HUE, dValue * 360.0 );
        break;
    case SATURATION:
        setColorComponent( COLORCOMP_SAT, dValue );
        break;
    case BRIGHTNESS:
        setColorComponent( COLORCOMP_BRI, dValue );
        break;
    case RED:
        setColorComponent( COLORCOMP_RED, dValue );
        break;
    case GREEN:
        setColorComponent( COLORCOMP_GREEN, dValue );
        break;
    case BLUE:
        setColorComponent( COLORCOMP_BLUE, dValue );
        break;
    }

    n = UPDATE_ALL&~(UPDATE_COLORSLIDER);
    if (n)
        update_color(n);
}
IMPL_LINK_TYPED(ColorPickerDialog, ColorModifyEditHdl, Edit&, rEdit, void)
{
    sal_uInt16 n = 0;

    if (&rEdit == mpMFRed)
    {
        setColorComponent( COLORCOMP_RED, ((double)mpMFRed->GetValue()) / 255.0 );
        n = UPDATE_ALL &~ (UPDATE_RGB);
    }
    else if (&rEdit == mpMFGreen)
    {
        setColorComponent( COLORCOMP_GREEN, ((double)mpMFGreen->GetValue()) / 255.0 );
        n = UPDATE_ALL &~ (UPDATE_RGB);
    }
    else if (&rEdit == mpMFBlue)
    {
        setColorComponent( COLORCOMP_BLUE, ((double)mpMFBlue->GetValue()) / 255.0 );
        n = UPDATE_ALL &~ (UPDATE_RGB);
    }
    else if (&rEdit == mpMFHue)
    {
        setColorComponent( COLORCOMP_HUE, (double)mpMFHue->GetValue() );
        n = UPDATE_ALL &~ (UPDATE_HSB);
    }
    else if (&rEdit == mpMFSaturation)
    {
        setColorComponent( COLORCOMP_SAT, ((double)mpMFSaturation->GetValue()) / 100.0 );
        n = UPDATE_ALL &~ (UPDATE_HSB);
    }
    else if (&rEdit == mpMFBrightness)
    {
        setColorComponent( COLORCOMP_BRI, ((double)mpMFBrightness->GetValue()) / 100.0 );
        n = UPDATE_ALL &~ (UPDATE_HSB);
    }
    else if (&rEdit == mpMFCyan)
    {
        setColorComponent( COLORCOMP_CYAN, ((double)mpMFCyan->GetValue()) / 100.0 );
        n = UPDATE_ALL &~ (UPDATE_CMYK);
    }
    else if (&rEdit == mpMFMagenta)
    {
        setColorComponent( COLORCOMP_MAGENTA, ((double)mpMFMagenta->GetValue()) / 100.0 );
        n = UPDATE_ALL &~ (UPDATE_CMYK);
    }
    else if (&rEdit == mpMFYellow)
    {
        setColorComponent( COLORCOMP_YELLOW, ((double)mpMFYellow->GetValue()) / 100.0 );
        n = UPDATE_ALL &~ (UPDATE_CMYK);
    }
    else if (&rEdit == mpMFKey)
    {
        setColorComponent( COLORCOMP_KEY, ((double)mpMFKey->GetValue()) / 100.0 );
        n = UPDATE_ALL&~(UPDATE_CMYK);
    }
    else if (&rEdit == mpEDHex)
    {
        sal_Int32 nColor = mpEDHex->GetColor();

        if (nColor != -1)
        {
            Color aColor(nColor);

            if (aColor != GetColor())
            {
                mdRed = ((double)aColor.GetRed()) / 255.0;
                mdGreen = ((double)aColor.GetGreen()) / 255.0;
                mdBlue = ((double)aColor.GetBlue()) / 255.0;

                RGBtoHSV( mdRed, mdGreen, mdBlue, mdHue, mdSat, mdBri );
                RGBtoCMYK( mdRed, mdGreen, mdBlue, mdCyan, mdMagenta, mdYellow, mdKey );
                n = UPDATE_ALL &~ (UPDATE_HEX);
            }
        }
    }

    if (n)
        update_color(n);
}

IMPL_LINK_NOARG_TYPED(ColorPickerDialog, ModeModifyHdl, RadioButton&, void)
{
    ColorMode eMode = HUE;

    if (mpRBRed->IsChecked())
    {
        eMode = RED;
    }
    else if (mpRBGreen->IsChecked())
    {
        eMode = GREEN;
    }
    else if (mpRBBlue->IsChecked())
    {
        eMode = BLUE;
    }
    else if (mpRBSaturation->IsChecked())
    {
        eMode = SATURATION;
    }
    else if (mpRBBrightness->IsChecked())
    {
        eMode = BRIGHTNESS;
    }

    if (meMode != eMode)
    {
        meMode = eMode;
        update_color(UPDATE_COLORCHOOSER | UPDATE_COLORSLIDER);
    }
}

void ColorPickerDialog::setColorComponent( sal_uInt16 nComp, double dValue )
{
    switch( nComp )
    {
    case COLORCOMP_RED:
        mdRed = dValue;
        break;
    case COLORCOMP_GREEN:
        mdGreen = dValue;
        break;
    case COLORCOMP_BLUE:
        mdBlue = dValue;
        break;
    case COLORCOMP_HUE:
        mdHue = dValue;
        break;
    case COLORCOMP_SAT:
        mdSat = dValue;
        break;
    case COLORCOMP_BRI:
        mdBri = dValue;
        break;
    case COLORCOMP_CYAN:
        mdCyan = dValue;
        break;
    case COLORCOMP_YELLOW:
        mdYellow = dValue;
        break;
    case COLORCOMP_MAGENTA:
        mdMagenta = dValue;
        break;
    case COLORCOMP_KEY:
        mdKey = dValue;
        break;
    }

    if (nComp & COLORMODE_RGB)
    {
        RGBtoHSV( mdRed, mdGreen, mdBlue, mdHue, mdSat, mdBri );
        RGBtoCMYK( mdRed, mdGreen, mdBlue, mdCyan, mdMagenta, mdYellow, mdKey );
    }
    else if (nComp & COLORMODE_HSV)
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

typedef ::cppu::WeakComponentImplHelper< XServiceInfo, XExecutableDialog, XInitialization, XPropertyAccess > ColorPickerBase;

class ColorPicker : protected ::comphelper::OBaseMutex,    // Struct for right initialization of mutex member! Must be first of baseclasses.
                    public ColorPickerBase
{
public:
    explicit ColorPicker( Reference< XComponentContext > const & xContext );

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException, std::exception) override;

    // XInitialization
    virtual OUString SAL_CALL getImplementationName(  ) throw (RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (RuntimeException, std::exception) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (RuntimeException, std::exception) override;

    // XPropertyAccess
    virtual Sequence< PropertyValue > SAL_CALL getPropertyValues(  ) throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValues( const Sequence< PropertyValue >& aProps ) throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception) override;

    // XExecutableDialog
    virtual void SAL_CALL setTitle( const OUString& aTitle ) throw (RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL execute(  ) throw (RuntimeException, std::exception) override;

private:
    Reference< XComponentContext > mxContext;
    OUString msTitle;
    const OUString msColorKey;
    const OUString msModeKey;
    sal_Int32 mnColor;
    sal_Int16 mnMode;
    Reference<css::awt::XWindow> mxParent;
};

OUString SAL_CALL ColorPicker_getImplementationName()
{
    return OUString( "com.sun.star.cui.ColorPicker" );
}

Reference< XInterface > SAL_CALL ColorPicker_createInstance( Reference< XComponentContext > const & xContext )
{
    return static_cast<XWeak*>( new ColorPicker( xContext ) );
}

Sequence< OUString > SAL_CALL ColorPicker_getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > seq { "com.sun.star.ui.dialogs.ColorPicker" };
    return seq;
}

ColorPicker::ColorPicker( Reference< XComponentContext > const & xContext )
    : ColorPickerBase( m_aMutex )
    , mxContext( xContext )
    , msColorKey( "Color" )
    , msModeKey( "Mode" )
    , mnColor( 0 )
    , mnMode( 0 )
{
}

// XInitialization
void SAL_CALL ColorPicker::initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException, std::exception)
{
    if( aArguments.getLength() == 1 )
    {
        aArguments[0] >>= mxParent;
    }
}

// XInitialization
OUString SAL_CALL ColorPicker::getImplementationName(  ) throw (RuntimeException, std::exception)
{
    return ColorPicker_getImplementationName();
}

sal_Bool SAL_CALL ColorPicker::supportsService( const OUString& sServiceName ) throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, sServiceName);
}

Sequence< OUString > SAL_CALL ColorPicker::getSupportedServiceNames(  ) throw (RuntimeException, std::exception)
{
    return ColorPicker_getSupportedServiceNames();
}

// XPropertyAccess
Sequence< PropertyValue > SAL_CALL ColorPicker::getPropertyValues(  ) throw (RuntimeException, std::exception)
{
    Sequence< PropertyValue > props(1);
    props[0].Name = msColorKey;
    props[0].Value <<= mnColor;
    return props;
}

void SAL_CALL ColorPicker::setPropertyValues( const Sequence< PropertyValue >& aProps ) throw (UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception)
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

// XExecutableDialog
void SAL_CALL ColorPicker::setTitle( const OUString& sTitle ) throw (RuntimeException, std::exception)
{
    msTitle = sTitle;
}

sal_Int16 SAL_CALL ColorPicker::execute(  ) throw (RuntimeException, std::exception)
{
    ScopedVclPtrInstance< ColorPickerDialog > aDlg( VCLUnoHelper::GetWindow( mxParent ), mnColor, mnMode );
    sal_Int16 ret = aDlg->Execute();
    if( ret )
        mnColor = aDlg->GetColor();

    return ret;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#include <cppuhelper/basemutex.hxx>
#include <vcl/customweld.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <vcl/weld.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <svx/hexcolorcontrol.hxx>
#include <sax/tools/converter.hxx>
#include <basegfx/color/bcolortools.hxx>
#include <colorpicker.hxx>
#include <bitmaps.hlst>
#include <cmath>
#include <limits>
#include <o3tl/typed_flags_set.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::beans;
using namespace ::basegfx;

enum class UpdateFlags
{
    NONE         = 0x00,
    RGB          = 0x01,
    CMYK         = 0x02,
    HSB          = 0x04,
    ColorChooser = 0x08,
    ColorSlider  = 0x10,
    Hex          = 0x20,
    All          = 0x3f,
};
namespace o3tl {
    template<> struct typed_flags<UpdateFlags> : is_typed_flags<UpdateFlags, 0x3f> {};
}


namespace cui
{

enum class ColorComponent {
    Red,
    Green,
    Blue,
    Hue,
    Saturation,
    Brightness,
    Cyan,
    Yellow,
    Magenta,
    Key,
};


// color space conversion helpers

static void RGBtoHSV( double dR, double dG, double dB, double& dH, double& dS, double& dV )
{
    BColor result = basegfx::utils::rgb2hsv( BColor( dR, dG, dB ) );

    dH = result.getX();
    dS = result.getY();
    dV = result.getZ();
}

static void HSVtoRGB(double dH, double dS, double dV, double& dR, double& dG, double& dB )
{
    BColor result = basegfx::utils::hsv2rgb( BColor( dH, dS, dV ) );

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

class ColorPreviewControl : public weld::CustomWidgetController
{
private:
    Color m_aColor;

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override;
public:
    ColorPreviewControl()
    {
    }

    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override
    {
        CustomWidgetController::SetDrawingArea(pDrawingArea);
        pDrawingArea->set_size_request(pDrawingArea->get_approximate_digit_width() * 10,
                                       pDrawingArea->get_text_height() * 2);
    }

    void SetColor(const Color& rCol)
    {
        if (rCol != m_aColor)
        {
            m_aColor = rCol;
            Invalidate();
        }
    }
};

void ColorPreviewControl::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    rRenderContext.SetFillColor(m_aColor);
    rRenderContext.SetLineColor(m_aColor);
    rRenderContext.DrawRect(tools::Rectangle(Point(0, 0), GetOutputSizePixel()));
}

enum ColorMode { HUE, SATURATION, BRIGHTNESS, RED, GREEN, BLUE };
const ColorMode DefaultMode = HUE;

class ColorFieldControl : public weld::CustomWidgetController
{
public:
    ColorFieldControl()
        : meMode( DefaultMode )
        , mdX( -1.0 )
        , mdY( -1.0 )
        , mbMouseCaptured(false)
    {
    }

    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override
    {
        CustomWidgetController::SetDrawingArea(pDrawingArea);
        pDrawingArea->set_size_request(pDrawingArea->get_approximate_digit_width() * 40,
                                       pDrawingArea->get_text_height() * 10);
    }

    virtual ~ColorFieldControl() override
    {
        mxBitmap.disposeAndClear();
    }

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual void Resize() override;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual bool MouseMove(const MouseEvent& rMEvt) override;
    virtual bool MouseButtonUp(const MouseEvent& rMEvt) override;

    void UpdateBitmap();
    void ShowPosition( const Point& rPos, bool bUpdate );
    void UpdatePosition();
    void Modify();

    void SetValues(Color aColor, ColorMode eMode, double x, double y);
    double GetX() { return mdX;}
    double GetY() { return mdY;}

    void SetModifyHdl(const Link<ColorFieldControl&,void>& rLink) { maModifyHdl = rLink; }

private:
    ColorMode meMode;
    Color maColor;
    double mdX;
    double mdY;
    bool mbMouseCaptured;
    Point maPosition;
    VclPtr<VirtualDevice> mxBitmap;
    Link<ColorFieldControl&,void> maModifyHdl;
    std::vector<sal_uInt8>  maRGB_Horiz;
    std::vector<sal_uInt16> maGrad_Horiz;
    std::vector<sal_uInt16> maPercent_Horiz;
    std::vector<sal_uInt8>  maRGB_Vert;
    std::vector<sal_uInt16> maPercent_Vert;
};

void ColorFieldControl::UpdateBitmap()
{
    const Size aSize(GetOutputSizePixel());

    if (mxBitmap && mxBitmap->GetOutputSizePixel() != aSize)
        mxBitmap.disposeAndClear();

    const sal_Int32 nWidth = aSize.Width();
    const sal_Int32 nHeight = aSize.Height();

    if (nWidth == 0 || nHeight == 0)
        return;

    if (!mxBitmap)
    {
        mxBitmap = VclPtr<VirtualDevice>::Create();
        mxBitmap->SetOutputSizePixel(aSize);

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

    Color aBitmapColor(maColor);

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
                    mxBitmap->DrawPixel(Point(x,y), Color::HSBtoRGB(nHue, nSat, nBri));
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
                    mxBitmap->DrawPixel(Point(x,y), Color::HSBtoRGB(nHue, nSat, nBri));
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
                    mxBitmap->DrawPixel(Point(x,y), Color::HSBtoRGB(nHue, nSat, nBri));
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
                    mxBitmap->DrawPixel(Point(x,y), aBitmapColor);
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
                    mxBitmap->DrawPixel(Point(x,y), aBitmapColor);
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
                    mxBitmap->DrawPixel(Point(x,y), aBitmapColor);
                }
            }
            break;
    }
}

void ColorFieldControl::ShowPosition( const Point& rPos, bool bUpdate )
{
    if (!mxBitmap)
    {
        UpdateBitmap();
        Invalidate();
    }

    if (!mxBitmap)
        return;

    const Size aSize(mxBitmap->GetOutputSizePixel());

    long nX = rPos.X();
    long nY = rPos.Y();
    if (nX < 0)
        nX = 0;
    else if (nX >= aSize.Width())
        nX = aSize.Width() - 1;

    if (nY < 0)
        nY = 0;
    else if (nY >= aSize.Height())
        nY = aSize.Height() - 1;

    Point aPos = maPosition;
    maPosition.setX( nX - 5 );
    maPosition.setY( nY - 5 );
    Invalidate(tools::Rectangle(aPos, Size(11, 11)));
    Invalidate(tools::Rectangle(maPosition, Size(11, 11)));

    if (bUpdate)
    {
        mdX = double(nX) / double(aSize.Width() - 1.0);
        mdY = double(aSize.Height() - 1.0 - nY) / double(aSize.Height() - 1.0);

        maColor = mxBitmap->GetPixel(Point(nX, nY));
    }
}

bool ColorFieldControl::MouseButtonDown(const MouseEvent& rMEvt)
{
    CaptureMouse();
    mbMouseCaptured = true;
    ShowPosition(rMEvt.GetPosPixel(), true);
    Modify();
    return true;
}

bool ColorFieldControl::MouseMove(const MouseEvent& rMEvt)
{
    if (mbMouseCaptured)
    {
        ShowPosition(rMEvt.GetPosPixel(), true);
        Modify();
    }
    return true;
}

bool ColorFieldControl::MouseButtonUp(const MouseEvent&)
{
    ReleaseMouse();
    mbMouseCaptured = false;
    return true;
}

void ColorFieldControl::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    if (!mxBitmap)
        UpdateBitmap();

    if (mxBitmap)
    {
        Size aSize(GetOutputSizePixel());
        rRenderContext.DrawOutDev(Point(0, 0), aSize, Point(0, 0), aSize, *mxBitmap);
    }

    // draw circle around current color
    if (maColor.IsDark())
        rRenderContext.SetLineColor( COL_WHITE );
    else
        rRenderContext.SetLineColor( COL_BLACK );

    rRenderContext.SetFillColor();

    rRenderContext.DrawEllipse(::tools::Rectangle(maPosition, Size(11, 11)));
}

void ColorFieldControl::Resize()
{
    CustomWidgetController::Resize();
    UpdateBitmap();
    UpdatePosition();
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

class ColorSliderControl : public weld::CustomWidgetController
{
public:
    ColorSliderControl();
    virtual ~ColorSliderControl() override;

    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

    virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual bool MouseMove(const MouseEvent& rMEvt) override;
    virtual bool MouseButtonUp(const MouseEvent& rMEvt) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override;
    virtual void Resize() override;

    void UpdateBitmap();
    void ChangePosition( long nY );
    void Modify();

    void SetValue( const Color& rColor, ColorMode eMode, double dValue );
    double GetValue() const { return mdValue; }

    void SetModifyHdl( const Link<ColorSliderControl&,void>& rLink ) { maModifyHdl = rLink; }

    sal_Int16 GetLevel() const { return mnLevel; }

private:
    Link<ColorSliderControl&,void> maModifyHdl;
    Color maColor;
    ColorMode meMode;
    VclPtr<VirtualDevice> mxBitmap;
    sal_Int16 mnLevel;
    double mdValue;
};

ColorSliderControl::ColorSliderControl()
    : meMode( DefaultMode )
    , mnLevel( 0 )
    , mdValue( -1.0 )
{
}

void ColorSliderControl::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    pDrawingArea->set_size_request(pDrawingArea->get_approximate_digit_width() * 3, -1);
}

ColorSliderControl::~ColorSliderControl()
{
    mxBitmap.disposeAndClear();
}

void ColorSliderControl::UpdateBitmap()
{
    Size aSize(1, GetOutputSizePixel().Height());

    if (mxBitmap && mxBitmap->GetOutputSizePixel() != aSize)
        mxBitmap.disposeAndClear();

    if (!mxBitmap)
    {
        mxBitmap = VclPtr<VirtualDevice>::Create();
        mxBitmap->SetOutputSizePixel(aSize);
    }

    const long nY = aSize.Height() - 1;

    Color aBitmapColor(maColor);

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
            mxBitmap->DrawPixel(Point(0, nY - y), Color::HSBtoRGB(nHue, nSat, nBri));
        }
        break;

    case SATURATION:
        nBri = std::max(sal_uInt16(32), nBri);
        for (long y = 0; y <= nY; y++)
        {
            nSat = static_cast<sal_uInt16>((100 * y) / nY);
            mxBitmap->DrawPixel(Point(0, nY - y), Color::HSBtoRGB(nHue, nSat, nBri));
        }
        break;

    case BRIGHTNESS:
        for (long y = 0; y <= nY; y++)
        {
            nBri = static_cast<sal_uInt16>((100 * y) / nY);
            mxBitmap->DrawPixel(Point(0, nY - y), Color::HSBtoRGB(nHue, nSat, nBri));
        }
        break;

    case RED:
        for (long y = 0; y <= nY; y++)
        {
            aBitmapColor.SetRed(sal_uInt8((long(255) * y) / nY));
            mxBitmap->DrawPixel(Point(0, nY - y), aBitmapColor);
        }
        break;

    case GREEN:
        for (long y = 0; y <= nY; y++)
        {
            aBitmapColor.SetGreen(sal_uInt8((long(255) * y) / nY));
            mxBitmap->DrawPixel(Point(0, nY - y), aBitmapColor);
        }
        break;

    case BLUE:
        for (long y = 0; y <= nY; y++)
        {
            aBitmapColor.SetBlue(sal_uInt8((long(255) * y) / nY));
            mxBitmap->DrawPixel(Point(0, nY - y), aBitmapColor);
        }
        break;
    }
}

void ColorSliderControl::ChangePosition(long nY)
{
    const long nHeight = GetOutputSizePixel().Height() - 1;

    if (nY < 0)
        nY = 0;
    else if (nY > nHeight)
        nY = nHeight;

    mnLevel = nY;
    mdValue = double(nHeight - nY) / double(nHeight);
}

bool ColorSliderControl::MouseButtonDown(const MouseEvent& rMEvt)
{
    CaptureMouse();
    ChangePosition(rMEvt.GetPosPixel().Y());
    Modify();
    return true;
}

bool ColorSliderControl::MouseMove(const MouseEvent& rMEvt)
{
    if (IsMouseCaptured())
    {
        ChangePosition(rMEvt.GetPosPixel().Y());
        Modify();
    }
    return true;
}

bool ColorSliderControl::MouseButtonUp(const MouseEvent&)
{
    ReleaseMouse();
    return true;
}

void ColorSliderControl::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    if (!mxBitmap)
        UpdateBitmap();

    const Size aSize(GetOutputSizePixel());

    Point aPos;
    int x = aSize.Width();
    while (x--)
    {
        rRenderContext.DrawOutDev(aPos, aSize, Point(0,0), aSize, *mxBitmap);
        aPos.AdjustX(1);
    }
}

void ColorSliderControl::Resize()
{
    CustomWidgetController::Resize();
    UpdateBitmap();
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

class ColorPickerDialog : public weld::GenericDialogController
{
private:
    ColorFieldControl m_aColorField;
    ColorSliderControl m_aColorSlider;
    ColorPreviewControl m_aColorPreview;
    ColorPreviewControl m_aColorPrevious;

    std::unique_ptr<weld::CustomWeld> m_xColorField;
    std::unique_ptr<weld::CustomWeld> m_xColorSlider;
    std::unique_ptr<weld::CustomWeld> m_xColorPreview;
    std::unique_ptr<weld::CustomWeld> m_xColorPrevious;

    std::unique_ptr<weld::Widget> m_xFISliderLeft;
    std::unique_ptr<weld::Widget> m_xFISliderRight;
    std::unique_ptr<weld::RadioButton> m_xRBRed;
    std::unique_ptr<weld::RadioButton> m_xRBGreen;
    std::unique_ptr<weld::RadioButton> m_xRBBlue;
    std::unique_ptr<weld::RadioButton> m_xRBHue;
    std::unique_ptr<weld::RadioButton> m_xRBSaturation;
    std::unique_ptr<weld::RadioButton> m_xRBBrightness;

    std::unique_ptr<weld::SpinButton> m_xMFRed;
    std::unique_ptr<weld::SpinButton> m_xMFGreen;
    std::unique_ptr<weld::SpinButton> m_xMFBlue;
    std::unique_ptr<weld::HexColorControl> m_xEDHex;

    std::unique_ptr<weld::MetricSpinButton> m_xMFHue;
    std::unique_ptr<weld::MetricSpinButton> m_xMFSaturation;
    std::unique_ptr<weld::MetricSpinButton> m_xMFBrightness;

    std::unique_ptr<weld::MetricSpinButton> m_xMFCyan;
    std::unique_ptr<weld::MetricSpinButton> m_xMFMagenta;
    std::unique_ptr<weld::MetricSpinButton> m_xMFYellow;
    std::unique_ptr<weld::MetricSpinButton> m_xMFKey;

public:
    ColorPickerDialog(weld::Window* pParent, Color nColor, sal_Int16 nMode);

    void update_color(UpdateFlags n = UpdateFlags::All);

    DECL_LINK(ColorFieldControlModifydl, ColorFieldControl&, void);
    DECL_LINK(ColorSliderControlModifyHdl, ColorSliderControl&, void);
    DECL_LINK(ColorModifyMetricHdl, weld::MetricSpinButton&, void);
    DECL_LINK(ColorModifySpinHdl, weld::SpinButton&, void);
    DECL_LINK(ColorModifyEditHdl, weld::Entry&, void);
    DECL_LINK(ModeModifyHdl, weld::ToggleButton&, void);

    Color GetColor() const;

    void setColorComponent(ColorComponent nComp, double dValue);

private:
    ColorMode meMode;

    double mdRed, mdGreen, mdBlue;
    double mdHue, mdSat, mdBri;
    double mdCyan, mdMagenta, mdYellow, mdKey;
};

ColorPickerDialog::ColorPickerDialog(weld::Window* pParent, Color nColor, sal_Int16 nDialogMode)
    : GenericDialogController(pParent, "cui/ui/colorpickerdialog.ui", "ColorPicker")
    , m_xColorField(new weld::CustomWeld(*m_xBuilder, "colorField", m_aColorField))
    , m_xColorSlider(new weld::CustomWeld(*m_xBuilder, "colorSlider", m_aColorSlider))
    , m_xColorPreview(new weld::CustomWeld(*m_xBuilder, "preview", m_aColorPreview))
    , m_xColorPrevious(new weld::CustomWeld(*m_xBuilder, "previous", m_aColorPrevious))
    , m_xFISliderLeft(m_xBuilder->weld_widget("leftImage"))
    , m_xFISliderRight(m_xBuilder->weld_widget("rightImage"))
    , m_xRBRed(m_xBuilder->weld_radio_button("redRadiobutton"))
    , m_xRBGreen(m_xBuilder->weld_radio_button("greenRadiobutton"))
    , m_xRBBlue(m_xBuilder->weld_radio_button("blueRadiobutton"))
    , m_xRBHue(m_xBuilder->weld_radio_button("hueRadiobutton"))
    , m_xRBSaturation(m_xBuilder->weld_radio_button("satRadiobutton"))
    , m_xRBBrightness(m_xBuilder->weld_radio_button("brightRadiobutton"))
    , m_xMFRed(m_xBuilder->weld_spin_button("redSpinbutton"))
    , m_xMFGreen(m_xBuilder->weld_spin_button("greenSpinbutton"))
    , m_xMFBlue(m_xBuilder->weld_spin_button("blueSpinbutton"))
    , m_xEDHex(new weld::HexColorControl(m_xBuilder->weld_entry("hexEntry")))
    , m_xMFHue(m_xBuilder->weld_metric_spin_button("hueSpinbutton", FieldUnit::DEGREE))
    , m_xMFSaturation(m_xBuilder->weld_metric_spin_button("satSpinbutton", FieldUnit::PERCENT))
    , m_xMFBrightness(m_xBuilder->weld_metric_spin_button("brightSpinbutton", FieldUnit::PERCENT))
    , m_xMFCyan(m_xBuilder->weld_metric_spin_button("cyanSpinbutton", FieldUnit::PERCENT))
    , m_xMFMagenta(m_xBuilder->weld_metric_spin_button("magSpinbutton", FieldUnit::PERCENT))
    , m_xMFYellow(m_xBuilder->weld_metric_spin_button("yellowSpinbutton", FieldUnit::PERCENT))
    , m_xMFKey(m_xBuilder->weld_metric_spin_button("keySpinbutton", FieldUnit::PERCENT))
    , meMode( DefaultMode )
{
    m_aColorField.SetModifyHdl( LINK( this, ColorPickerDialog, ColorFieldControlModifydl ) );
    m_aColorSlider.SetModifyHdl( LINK( this, ColorPickerDialog, ColorSliderControlModifyHdl ) );

    int nMargin = (m_xFISliderLeft->get_preferred_size().Height() + 1) / 2;
    m_xColorSlider->set_margin_top(nMargin);
    m_xColorSlider->set_margin_bottom(nMargin);

    Link<weld::MetricSpinButton&,void> aLink3( LINK( this, ColorPickerDialog, ColorModifyMetricHdl ) );
    m_xMFCyan->connect_value_changed( aLink3 );
    m_xMFMagenta->connect_value_changed( aLink3 );
    m_xMFYellow->connect_value_changed( aLink3 );
    m_xMFKey->connect_value_changed( aLink3 );

    m_xMFHue->connect_value_changed( aLink3 );
    m_xMFSaturation->connect_value_changed( aLink3 );
    m_xMFBrightness->connect_value_changed( aLink3 );

    Link<weld::SpinButton&,void> aLink4(LINK(this, ColorPickerDialog, ColorModifySpinHdl));
    m_xMFRed->connect_value_changed(aLink4);
    m_xMFGreen->connect_value_changed(aLink4);
    m_xMFBlue->connect_value_changed(aLink4);

    m_xEDHex->connect_changed(LINK(this, ColorPickerDialog, ColorModifyEditHdl));

    Link<weld::ToggleButton&,void> aLink2 = LINK( this, ColorPickerDialog, ModeModifyHdl );
    m_xRBRed->connect_toggled( aLink2 );
    m_xRBGreen->connect_toggled( aLink2 );
    m_xRBBlue->connect_toggled( aLink2 );
    m_xRBHue->connect_toggled( aLink2 );
    m_xRBSaturation->connect_toggled( aLink2 );
    m_xRBBrightness->connect_toggled( aLink2 );

    Color aColor(nColor);

    // modify
    if (nDialogMode == 2)
    {
        m_aColorPrevious.SetColor(aColor);
        m_xColorPrevious->show();
    }

    mdRed = static_cast<double>(aColor.GetRed()) / 255.0;
    mdGreen = static_cast<double>(aColor.GetGreen()) / 255.0;
    mdBlue = static_cast<double>(aColor.GetBlue()) / 255.0;

    RGBtoHSV( mdRed, mdGreen, mdBlue, mdHue, mdSat, mdBri );
    RGBtoCMYK( mdRed, mdGreen, mdBlue, mdCyan, mdMagenta, mdYellow, mdKey );

    update_color();
}

static int toInt( double dValue, double dRange )
{
    return static_cast< int >( std::floor((dValue * dRange) + 0.5 ) );
}

Color ColorPickerDialog::GetColor() const
{
    return Color( toInt(mdRed,255.0), toInt(mdGreen,255.0), toInt(mdBlue,255.0) );
}

void ColorPickerDialog::update_color( UpdateFlags n )
{
    sal_uInt8 nRed = toInt(mdRed,255.0);
    sal_uInt8 nGreen = toInt(mdGreen,255.0);
    sal_uInt8 nBlue = toInt(mdBlue,255.0);

    Color aColor(nRed, nGreen, nBlue);

    if (n & UpdateFlags::RGB) // update RGB
    {
        m_xMFRed->set_value(nRed);
        m_xMFGreen->set_value(nGreen);
        m_xMFBlue->set_value(nBlue);
    }

    if (n & UpdateFlags::CMYK) // update CMYK
    {
        m_xMFCyan->set_value(toInt(mdCyan, 100.0), FieldUnit::PERCENT);
        m_xMFMagenta->set_value(toInt(mdMagenta, 100.0), FieldUnit::PERCENT);
        m_xMFYellow->set_value(toInt(mdYellow, 100.0), FieldUnit::PERCENT);
        m_xMFKey->set_value(toInt(mdKey, 100.0), FieldUnit::PERCENT);
    }

    if (n & UpdateFlags::HSB ) // update HSB
    {
        m_xMFHue->set_value(toInt(mdHue, 1.0), FieldUnit::DEGREE);
        m_xMFSaturation->set_value(toInt( mdSat, 100.0), FieldUnit::PERCENT);
        m_xMFBrightness->set_value(toInt( mdBri, 100.0), FieldUnit::PERCENT);
    }

    if (n & UpdateFlags::ColorChooser ) // update Color Chooser 1
    {
        switch( meMode )
        {
        case HUE:
            m_aColorField.SetValues(aColor, meMode, mdSat, mdBri);
            break;
        case SATURATION:
            m_aColorField.SetValues(aColor, meMode, mdHue / 360.0, mdBri);
            break;
        case BRIGHTNESS:
            m_aColorField.SetValues(aColor, meMode, mdHue / 360.0, mdSat);
            break;
        case RED:
            m_aColorField.SetValues(aColor, meMode, mdBlue, mdGreen);
            break;
        case GREEN:
            m_aColorField.SetValues(aColor, meMode, mdBlue, mdRed);
            break;
        case BLUE:
            m_aColorField.SetValues(aColor, meMode, mdRed, mdGreen);
            break;
        }
    }

    if (n & UpdateFlags::ColorSlider) // update Color Chooser 2
    {
        switch (meMode)
        {
        case HUE:
            m_aColorSlider.SetValue(aColor, meMode, mdHue / 360.0);
            break;
        case SATURATION:
            m_aColorSlider.SetValue(aColor, meMode, mdSat);
            break;
        case BRIGHTNESS:
            m_aColorSlider.SetValue(aColor, meMode, mdBri);
            break;
        case RED:
            m_aColorSlider.SetValue(aColor, meMode, mdRed);
            break;
        case GREEN:
            m_aColorSlider.SetValue(aColor, meMode, mdGreen);
            break;
        case BLUE:
            m_aColorSlider.SetValue(aColor, meMode, mdBlue);
            break;
        }
    }

    if (n & UpdateFlags::Hex) // update hex
    {
        m_xFISliderLeft->set_margin_top(m_aColorSlider.GetLevel());
        m_xFISliderRight->set_margin_top(m_aColorSlider.GetLevel());
        m_xEDHex->SetColor(aColor);
    }
    m_aColorPreview.SetColor(aColor);
}

IMPL_LINK_NOARG(ColorPickerDialog, ColorFieldControlModifydl, ColorFieldControl&, void)
{
    double x = m_aColorField.GetX();
    double y = m_aColorField.GetY();

    switch( meMode )
    {
    case HUE:
        mdSat = x;
        setColorComponent( ColorComponent::Brightness, y );
        break;
    case SATURATION:
        mdHue = x * 360.0;
        setColorComponent( ColorComponent::Brightness, y );
        break;
    case BRIGHTNESS:
        mdHue = x * 360.0;
        setColorComponent( ColorComponent::Saturation, y );
        break;
    case RED:
        mdBlue = x;
        setColorComponent( ColorComponent::Green, y );
        break;
    case GREEN:
        mdBlue = x;
        setColorComponent( ColorComponent::Red, y );
        break;
    case BLUE:
        mdRed = x;
        setColorComponent( ColorComponent::Green, y );
        break;
    }

    update_color(UpdateFlags::All & ~UpdateFlags::ColorChooser);
}

IMPL_LINK_NOARG(ColorPickerDialog, ColorSliderControlModifyHdl, ColorSliderControl&, void)
{
    double dValue = m_aColorSlider.GetValue();
    switch (meMode)
    {
    case HUE:
        setColorComponent( ColorComponent::Hue, dValue * 360.0 );
        break;
    case SATURATION:
        setColorComponent( ColorComponent::Saturation, dValue );
        break;
    case BRIGHTNESS:
        setColorComponent( ColorComponent::Brightness, dValue );
        break;
    case RED:
        setColorComponent( ColorComponent::Red, dValue );
        break;
    case GREEN:
        setColorComponent( ColorComponent::Green, dValue );
        break;
    case BLUE:
        setColorComponent( ColorComponent::Blue, dValue );
        break;
    }

    update_color(UpdateFlags::All & ~UpdateFlags::ColorSlider);
}

IMPL_LINK(ColorPickerDialog, ColorModifyMetricHdl, weld::MetricSpinButton&, rEdit, void)
{
    UpdateFlags n = UpdateFlags::NONE;

    if (&rEdit == m_xMFHue.get())
    {
        setColorComponent( ColorComponent::Hue, static_cast<double>(m_xMFHue->get_value(FieldUnit::DEGREE)) );
        n = UpdateFlags::All & ~UpdateFlags::HSB;
    }
    else if (&rEdit == m_xMFSaturation.get())
    {
        setColorComponent( ColorComponent::Saturation, static_cast<double>(m_xMFSaturation->get_value(FieldUnit::PERCENT)) / 100.0 );
        n = UpdateFlags::All & ~UpdateFlags::HSB;
    }
    else if (&rEdit == m_xMFBrightness.get())
    {
        setColorComponent( ColorComponent::Brightness, static_cast<double>(m_xMFBrightness->get_value(FieldUnit::PERCENT)) / 100.0 );
        n = UpdateFlags::All & ~UpdateFlags::HSB;
    }
    else if (&rEdit == m_xMFCyan.get())
    {
        setColorComponent( ColorComponent::Cyan, static_cast<double>(m_xMFCyan->get_value(FieldUnit::PERCENT)) / 100.0 );
        n = UpdateFlags::All & ~UpdateFlags::CMYK;
    }
    else if (&rEdit == m_xMFMagenta.get())
    {
        setColorComponent( ColorComponent::Magenta, static_cast<double>(m_xMFMagenta->get_value(FieldUnit::PERCENT)) / 100.0 );
        n = UpdateFlags::All & ~UpdateFlags::CMYK;
    }
    else if (&rEdit == m_xMFYellow.get())
    {
        setColorComponent( ColorComponent::Yellow, static_cast<double>(m_xMFYellow->get_value(FieldUnit::PERCENT)) / 100.0 );
        n = UpdateFlags::All & ~UpdateFlags::CMYK;
    }
    else if (&rEdit == m_xMFKey.get())
    {
        setColorComponent( ColorComponent::Key, static_cast<double>(m_xMFKey->get_value(FieldUnit::PERCENT)) / 100.0 );
        n = UpdateFlags::All & ~UpdateFlags::CMYK;
    }

    if (n != UpdateFlags::NONE)
        update_color(n);
}

IMPL_LINK_NOARG(ColorPickerDialog, ColorModifyEditHdl, weld::Entry&, void)
{
    UpdateFlags n = UpdateFlags::NONE;

    Color aColor = m_xEDHex->GetColor();

    if (aColor != Color(0xffffffff) && aColor != GetColor())
    {
        mdRed = static_cast<double>(aColor.GetRed()) / 255.0;
        mdGreen = static_cast<double>(aColor.GetGreen()) / 255.0;
        mdBlue = static_cast<double>(aColor.GetBlue()) / 255.0;

        RGBtoHSV( mdRed, mdGreen, mdBlue, mdHue, mdSat, mdBri );
        RGBtoCMYK( mdRed, mdGreen, mdBlue, mdCyan, mdMagenta, mdYellow, mdKey );
        n = UpdateFlags::All & ~UpdateFlags::Hex;
    }

    if (n != UpdateFlags::NONE)
        update_color(n);
}

IMPL_LINK(ColorPickerDialog, ColorModifySpinHdl, weld::SpinButton&, rEdit, void)
{
    UpdateFlags n = UpdateFlags::NONE;

    if (&rEdit == m_xMFRed.get())
    {
        setColorComponent( ColorComponent::Red, static_cast<double>(m_xMFRed->get_value()) / 255.0 );
        n = UpdateFlags::All & ~UpdateFlags::RGB;
    }
    else if (&rEdit == m_xMFGreen.get())
    {
        setColorComponent( ColorComponent::Green, static_cast<double>(m_xMFGreen->get_value()) / 255.0 );
        n = UpdateFlags::All & ~UpdateFlags::RGB;
    }
    else if (&rEdit == m_xMFBlue.get())
    {
        setColorComponent( ColorComponent::Blue, static_cast<double>(m_xMFBlue->get_value()) / 255.0 );
        n = UpdateFlags::All & ~UpdateFlags::RGB;
    }

    if (n != UpdateFlags::NONE)
        update_color(n);
}


IMPL_LINK_NOARG(ColorPickerDialog, ModeModifyHdl, weld::ToggleButton&, void)
{
    ColorMode eMode = HUE;

    if (m_xRBRed->get_active())
    {
        eMode = RED;
    }
    else if (m_xRBGreen->get_active())
    {
        eMode = GREEN;
    }
    else if (m_xRBBlue->get_active())
    {
        eMode = BLUE;
    }
    else if (m_xRBSaturation->get_active())
    {
        eMode = SATURATION;
    }
    else if (m_xRBBrightness->get_active())
    {
        eMode = BRIGHTNESS;
    }

    if (meMode != eMode)
    {
        meMode = eMode;
        update_color(UpdateFlags::ColorChooser | UpdateFlags::ColorSlider);
    }
}

void ColorPickerDialog::setColorComponent( ColorComponent nComp, double dValue )
{
    switch( nComp )
    {
    case ColorComponent::Red:
        mdRed = dValue;
        break;
    case ColorComponent::Green:
        mdGreen = dValue;
        break;
    case ColorComponent::Blue:
        mdBlue = dValue;
        break;
    case ColorComponent::Hue:
        mdHue = dValue;
        break;
    case ColorComponent::Saturation:
        mdSat = dValue;
        break;
    case ColorComponent::Brightness:
        mdBri = dValue;
        break;
    case ColorComponent::Cyan:
        mdCyan = dValue;
        break;
    case ColorComponent::Yellow:
        mdYellow = dValue;
        break;
    case ColorComponent::Magenta:
        mdMagenta = dValue;
        break;
    case ColorComponent::Key:
        mdKey = dValue;
        break;
    }

    if (nComp == ColorComponent::Red || nComp == ColorComponent::Green || nComp == ColorComponent::Blue)
    {
        RGBtoHSV( mdRed, mdGreen, mdBlue, mdHue, mdSat, mdBri );
        RGBtoCMYK( mdRed, mdGreen, mdBlue, mdCyan, mdMagenta, mdYellow, mdKey );
    }
    else if (nComp == ColorComponent::Hue || nComp == ColorComponent::Saturation || nComp == ColorComponent::Brightness)
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

class ColorPicker : protected ::cppu::BaseMutex,    // Struct for right initialization of mutex member! Must be first of baseclasses.
                    public ColorPickerBase
{
public:
    explicit ColorPicker();

    // XInitialization
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) override;

    // XInitialization
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // XPropertyAccess
    virtual Sequence< PropertyValue > SAL_CALL getPropertyValues(  ) override;
    virtual void SAL_CALL setPropertyValues( const Sequence< PropertyValue >& aProps ) override;

    // XExecutableDialog
    virtual void SAL_CALL setTitle( const OUString& aTitle ) override;
    virtual sal_Int16 SAL_CALL execute(  ) override;

private:
    Color mnColor;
    sal_Int16 mnMode;
    Reference<css::awt::XWindow> mxParent;
};

OUString ColorPicker_getImplementationName()
{
    return OUString( "com.sun.star.cui.ColorPicker" );
}

Reference< XInterface > ColorPicker_createInstance( Reference< XComponentContext > const & )
{
    return static_cast<XWeak*>( new ColorPicker );
}

Sequence< OUString > ColorPicker_getSupportedServiceNames()
{
    Sequence< OUString > seq { "com.sun.star.ui.dialogs.ColorPicker" };
    return seq;
}

static const OUStringLiteral gsColorKey( "Color" );
static const OUStringLiteral gsModeKey( "Mode" );

ColorPicker::ColorPicker()
    : ColorPickerBase( m_aMutex )
    , mnColor( 0 )
    , mnMode( 0 )
{
}

// XInitialization
void SAL_CALL ColorPicker::initialize( const Sequence< Any >& aArguments )
{
    if( aArguments.getLength() == 1 )
    {
        aArguments[0] >>= mxParent;
    }
}

// XInitialization
OUString SAL_CALL ColorPicker::getImplementationName(  )
{
    return ColorPicker_getImplementationName();
}

sal_Bool SAL_CALL ColorPicker::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

Sequence< OUString > SAL_CALL ColorPicker::getSupportedServiceNames(  )
{
    return ColorPicker_getSupportedServiceNames();
}

// XPropertyAccess
Sequence< PropertyValue > SAL_CALL ColorPicker::getPropertyValues(  )
{
    Sequence< PropertyValue > props(1);
    props[0].Name = gsColorKey;
    props[0].Value <<= mnColor;
    return props;
}

void SAL_CALL ColorPicker::setPropertyValues( const Sequence< PropertyValue >& aProps )
{
    for( sal_Int32 n = 0; n < aProps.getLength(); n++ )
    {
        if( aProps[n].Name == gsColorKey )
        {
            aProps[n].Value >>= mnColor;
        }
        else if( aProps[n].Name == gsModeKey )
        {
            aProps[n].Value >>= mnMode;
        }
    }
}

// XExecutableDialog
void SAL_CALL ColorPicker::setTitle( const OUString& )
{
}

sal_Int16 SAL_CALL ColorPicker::execute()
{
    std::unique_ptr<ColorPickerDialog> xDlg(new ColorPickerDialog(Application::GetFrameWeld(mxParent), mnColor, mnMode));
    sal_Int16 ret = xDlg->run();
    if (ret)
        mnColor = xDlg->GetColor();
    return ret;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

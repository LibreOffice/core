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
#include "precompiled_sdext.hxx"

#include "PresenterClock.hxx"
#include "PresenterComponent.hxx"
#include "PresenterConfigurationAccess.hxx"
#include "PresenterGeometryHelper.hxx"
#include <com/sun/star/awt/InvalidateStyle.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/deployment/XPackageInformationProvider.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/TextDirection.hpp>
#include <com/sun/star/rendering/XCanvasFont.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/util/Color.hpp>
#include <osl/mutex.hxx>
#include <osl/time.h>
#include <rtl/ref.hxx>
#include <salhelper/timer.hxx>
#include <boost/bind.hpp>
#include <cmath>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

namespace sdext { namespace presenter {


/** Wrapper around a library timer.
*/
class PresenterClock::Timer : public salhelper::Timer
{
public:
    explicit Timer (const ::rtl::Reference<PresenterClock>& rpClock);
    virtual ~Timer (void);

    void Stop (void);

protected:
    virtual void SAL_CALL onShot (void);

private:
    ::rtl::Reference<PresenterClock> mpClock;
};




namespace {
    bool GetDateTime (oslDateTime& rDateTime);

    class BitmapDescriptor
    {
    public:
        Reference<rendering::XBitmap> mxBitmap;
        awt::Point maOffset;
        Reference<rendering::XBitmap> mxScaledBitmap;
        geometry::RealPoint2D maScaledOffset;
    };
}




class PresenterClock::Painter
{
public:
    virtual void Paint (
        const Reference<rendering::XCanvas>& rxCanvas,
        const rendering::ViewState& rViewState,
        const rendering::RenderState& rRenderState,
        const util::Color& rBackgroundColor,
        const sal_Int32 nHour,
        const sal_Int32 nMinute,
        const sal_Int32 nSecond,
        const bool bShowSeconds) = 0;
    virtual void Resize (const awt::Size& rSize) = 0;
};




namespace {
    class AnalogDefaultPainter : public PresenterClock::Painter
    {
    public:
        AnalogDefaultPainter (void);
        virtual ~AnalogDefaultPainter (void) {}
        virtual void Paint (
            const Reference<rendering::XCanvas>& rxCanvas,
            const rendering::ViewState& rViewState,
            const rendering::RenderState& rRenderState,
            const util::Color& rBackgroundColor,
            const sal_Int32 nHour,
            const sal_Int32 nMinute,
            const sal_Int32 nSecond,
            const bool bShowSeconds);
        virtual void Resize (const awt::Size& rSize);
    private:
        geometry::RealPoint2D maCenter;
        double mnOuterRadius;
        awt::Size maSize;
        Reference<rendering::XBitmap> mxBitmap;

        /** Relative length (with respect to radius) from center to the tip of
            the hand.
        */
        static const double mnRelativeHourHandLength;
        /** Relative length (with respect to radius) from center to the
            oposing end of the tip of the hand.
        */
        static const double mnRelativeHourHandLength2;
        static const double mnRelativeHourHandWidth;
        static const double mnRelativeMinuteHandLength;
        static const double mnRelativeMinuteHandLength2;
        static const double mnRelativeMinuteHandWidth;
        static const double mnRelativeSecondHandLength;
        static const double mnRelativeSecondHandLength2;
        static const double mnRelativeSecondHandWidth;

        void PaintAngledLine (
            const double nAngle,
            const double nInnerRadius,
            const double nOuterRadius,
            const double nStrokeWidth,
            const Reference<rendering::XCanvas>& rxCanvas,
            const rendering::ViewState& rViewState,
            const rendering::RenderState& rRenderState);
    };


    class AnalogBitmapPainter : public PresenterClock::Painter
    {
    public:
        AnalogBitmapPainter(
            const Reference<XComponentContext>& rxContext,
            const OUString& rsThemeName);
        virtual ~AnalogBitmapPainter (void) {}
        virtual void Paint (
            const Reference<rendering::XCanvas>& rxCanvas,
            const rendering::ViewState& rViewState,
            const rendering::RenderState& rRenderState,
            const util::Color& rBackgroundColor,
            const sal_Int32 nHour,
            const sal_Int32 nMinute,
            const sal_Int32 nSecond,
            const bool bShowSeconds);
        virtual void Resize (const awt::Size& rSize);
    private:
        css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
        const OUString msThemeName;
        bool mbThemeLoaded;
        bool mbThemeLoadingFailed;
        geometry::RealPoint2D maCenter;
        double mnOuterRadius;
        BitmapDescriptor maFace;
        BitmapDescriptor maMinuteHand;
        BitmapDescriptor maHourHand;

        void PrepareBitmaps (const Reference<rendering::XCanvas>& rxCanvas);
        Reference<container::XNameAccess> GetTheme (
            PresenterConfigurationAccess& rConfiguration);
        bool ThemeNameComparator (
            const ::rtl::OUString& rsKey,
            const Reference<container::XNameAccess>& rxCandidate,
            const ::rtl::OUString& rsCurrentThemeName);
        void LoadBitmaps (
            PresenterConfigurationAccess& rConfiguration,
            const Reference<container::XNameAccess>& rxNameAccess,
            const Reference<rendering::XCanvas>& rxCanvas);
        void LoadBitmap (
            const OUString& rsKey,
            const ::std::vector<Any>& rValues,
            const OUString& rsBitmapPath,
            const Reference<container::XNameAccess>& rxBitmapLoader);
        void ScaleBitmaps (void);
    };


    class DigitalDefaultPainter : public PresenterClock::Painter
    {
    public:
        DigitalDefaultPainter (
            const ::rtl::Reference<PresenterController>& rpPresenterController,
            const Reference<XResourceId>& rxViewId);
        virtual ~DigitalDefaultPainter (void);

        virtual void Paint (
            const Reference<rendering::XCanvas>& rxCanvas,
            const rendering::ViewState& rViewState,
            const rendering::RenderState& rRenderState,
            const util::Color& rBackgroundColor,
            const sal_Int32 nHour,
            const sal_Int32 nMinute,
            const sal_Int32 nSecond,
            const bool bShowSeconds);
        virtual void Resize (const awt::Size& rSize);

    private:
        ::rtl::Reference<PresenterController> mpPresenterController;
        bool mbIs24HourFormat;
        bool mbIsAdaptFontSize;
        Reference<rendering::XCanvasFont> mxFont;
        awt::Size maWindowSize;
        OUString msViewURL;

        void CreateFont (
            const Reference<rendering::XCanvas>& rxCanvas,
            const bool bIsShowSeconds);
    };


} // end of anonymous namespace




//===== PresenterClock =================================================================

::rtl::Reference<PresenterClock> PresenterClock::Create (
    const Reference<XComponentContext>& rxContext,
    const Reference<XResourceId>& rxViewId,
    const Reference<frame::XController>& rxController,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
{
    ::rtl::Reference<PresenterClock> pClock (new PresenterClock(
        rxContext,
        rxViewId,
        rxController,
        rpPresenterController));
    pClock->LateInit();
    return pClock;
}




PresenterClock::PresenterClock (
    const Reference<XComponentContext>& rxContext,
    const Reference<XResourceId>& rxViewId,
    const Reference<frame::XController>& rxController,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
    : PresenterClockInterfaceBase(m_aMutex),
      mxComponentContext(rxContext),
      mxViewId(rxViewId),
      mxWindow(),
      mxCanvas(),
      mxPane(),
      mpPresenterController(rpPresenterController),
      mbIsResizePending(true),
      maViewState(),
      maRenderState(),
      mpTimer(),
      mpClockPainter(),
      mpClockPainter2(),
      mnMode(1),
      mnHour(-1),
      mnMinute(-1),
      mnSecond(-1),
      mbIsShowSeconds(true)
{
    SetMode(mnMode);

    maViewState.AffineTransform = geometry::AffineMatrix2D(1,0,0, 0,1,0);
    maRenderState.AffineTransform = geometry::AffineMatrix2D(1,0,0, 0,1,0);
    maRenderState.DeviceColor = Sequence<double>(4);
    PresenterCanvasHelper::SetDeviceColor(maRenderState, util::Color(0x00000000));

    try
    {

        Reference<XControllerManager> xCM (rxController, UNO_QUERY_THROW);
        Reference<XConfigurationController> xCC (xCM->getConfigurationController(), UNO_QUERY_THROW);
        mxPane = Reference<XPane>(xCC->getResource(rxViewId->getAnchor()), UNO_QUERY_THROW);

        mxWindow = mxPane->getWindow();
        if (mxWindow.is())
        {
            mxWindow->addPaintListener(this);
            mxWindow->addWindowListener(this);
            mxWindow->addMouseListener(this);
            Reference<awt::XWindowPeer> xPeer (mxWindow, UNO_QUERY);
            if (xPeer.is())
                xPeer->setBackground(util::Color(0xff000000));
            mxWindow->setVisible(sal_True);
        }

        Resize();
    }
    catch (RuntimeException&)
    {
        disposing();
        throw;
    }
}




PresenterClock::~PresenterClock (void)
{
}




void PresenterClock::LateInit (void)
{
    mpTimer = new Timer(this);
}




void SAL_CALL PresenterClock::disposing (void)
{
    //    osl::MutexGuard aGuard (m_aMutex);
    if (mpTimer != NULL)
    {
        mpTimer->Stop();
    }
    if (mxWindow.is())
    {
        mxWindow->removePaintListener(this);
        mxWindow->removeWindowListener(this);
        mxWindow->removeMouseListener(this);
        mxWindow = NULL;
    }
    mxCanvas = NULL;
    mxViewId = NULL;
}




void PresenterClock::UpdateTime (void)
{
    // Get current time and check whether it is different from last time.
    oslDateTime aDateTime;
    if ( ! GetDateTime(aDateTime))
        return;
    if (aDateTime.Hours != mnHour
        || aDateTime.Minutes != mnMinute
        || aDateTime.Seconds != mnSecond)
    {
        mnHour = aDateTime.Hours % 24;
        mnMinute = aDateTime.Minutes % 60;
        mnSecond = aDateTime.Seconds % 60;

        Reference<awt::XWindowPeer> xPeer (mxWindow, UNO_QUERY);
        if (xPeer.is())
            xPeer->invalidate(awt::InvalidateStyle::NOERASE |
            awt::InvalidateStyle::UPDATE);
    }
}




//-----  lang::XEventListener -------------------------------------------------

void SAL_CALL PresenterClock::disposing (const lang::EventObject& rEventObject)
    throw (RuntimeException)
{
    //    ::osl::MutexGuard aSolarGuard (::osl::Mutex::getGlobalMutex());
    //    osl::MutexGuard aGuard (m_aMutex);

    if (rEventObject.Source == mxWindow)
    {
        mxWindow = NULL;
        if (mpTimer != NULL)
            mpTimer->Stop();
    }
}




//----- XPaintListener --------------------------------------------------------

void SAL_CALL PresenterClock::windowPaint (const awt::PaintEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
    Paint(rEvent.UpdateRect);
}




//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterClock::windowResized (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    mbIsResizePending = true;
}




void SAL_CALL PresenterClock::windowMoved (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    mbIsResizePending = true;
}




void SAL_CALL PresenterClock::windowShown (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    mbIsResizePending = true;
}




void SAL_CALL PresenterClock::windowHidden (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
}




//----- XMouseListener --------------------------------------------------------

void SAL_CALL PresenterClock::mousePressed (const css::awt::MouseEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
    if (rEvent.Buttons == awt::MouseButton::LEFT)
    {
        SetMode(mnMode+1);
    }
}




void SAL_CALL PresenterClock::mouseReleased (const css::awt::MouseEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
}




void SAL_CALL PresenterClock::mouseEntered (const css::awt::MouseEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
}




void SAL_CALL PresenterClock::mouseExited (const css::awt::MouseEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
}




//----- XResourceId -----------------------------------------------------------

Reference<XResourceId> SAL_CALL PresenterClock::getResourceId (void)
    throw (RuntimeException)
{
    return mxViewId;
}




sal_Bool SAL_CALL PresenterClock::isAnchorOnly (void)
    throw (RuntimeException)
{
    return false;
}




//-----------------------------------------------------------------------------

void PresenterClock::Resize (void)
{
    if (mxPane.is())
        mxCanvas = Reference<rendering::XCanvas>(mxPane->getCanvas(), UNO_QUERY);
    if (mxWindow.is() && mxCanvas.is())
    {
        const awt::Rectangle aWindowBox (mxWindow->getPosSize());
        const awt::Size aWindowSize(aWindowBox.Width,aWindowBox.Height);
        if (mpClockPainter.get() != NULL)
            mpClockPainter->Resize(aWindowSize);
        if (mpClockPainter2.get() != NULL)
            mpClockPainter2->Resize(aWindowSize);
        mbIsResizePending = false;
    }
}




void PresenterClock::Paint (const awt::Rectangle& rUpdateBox)
{
    if ( ! mxCanvas.is() && mxPane.is())
        mxCanvas = Reference<rendering::XCanvas>(mxPane->getCanvas(), UNO_QUERY);
    if ( ! mxWindow.is()
        || ! mxCanvas.is()
        || ! mxCanvas->getDevice().is())
    {
        return;
    }

    try
    {
        if (mbIsResizePending)
            Resize();

        Reference<rendering::XPolyPolygon2D> xUpdatePolygon (
            PresenterGeometryHelper::CreatePolygon(rUpdateBox, mxCanvas->getDevice()));

        Clear(xUpdatePolygon);

        if (mpClockPainter.get() != NULL)
            mpClockPainter->Paint(mxCanvas,
                maViewState,
                maRenderState,
                mpPresenterController->GetViewBackgroundColor(mxViewId->getResourceURL()),
                mnHour,
                mnMinute,
                mnSecond,
                mbIsShowSeconds);

        if (mpClockPainter2.get() != NULL)
            mpClockPainter2->Paint(
                mxCanvas,
                maViewState,
                maRenderState,
                mpPresenterController->GetViewBackgroundColor(mxViewId->getResourceURL()),
                mnHour,
                mnMinute,
                mnSecond,
                mbIsShowSeconds);
    }
    catch (RuntimeException& e)
    {
        (void)e;
    }

    // Make the back buffer visible.
    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        xSpriteCanvas->updateScreen(sal_False);
}




void PresenterClock::Clear (const Reference<rendering::XPolyPolygon2D>& rxUpdatePolygon)
{
    rendering::RenderState aRenderState = maRenderState;
    const sal_Int32 nColor (
        mpPresenterController->GetViewBackgroundColor(mxViewId->getResourceURL()));
    aRenderState.DeviceColor[0] = ((nColor&0x00ff0000) >> 16) / 255.0;
    aRenderState.DeviceColor[1] = ((nColor&0x0000ff00) >>  8) / 255.0;
    aRenderState.DeviceColor[2] = ((nColor&0x000000ff) >>  0) / 255.0;

    if (rxUpdatePolygon.is())
        mxCanvas->fillPolyPolygon(
            rxUpdatePolygon,
            maViewState,
            aRenderState);
}




void PresenterClock::SetMode (const sal_Int32 nMode)
{
    mnMode = nMode % 3;

    switch (mnMode)
    {
        case 0:
            mpClockPainter.reset(
                new AnalogBitmapPainter(
                    mxComponentContext,
                    OUString::createFromAscii("ClockTheme")));
            mpClockPainter2.reset();
            break;

        case 1:
            mpClockPainter.reset();
            mpClockPainter2.reset(new AnalogDefaultPainter());
            break;

        case 2:
            mpClockPainter.reset();
            mpClockPainter2.reset(new DigitalDefaultPainter(mpPresenterController, mxViewId));
            break;

        case 3:
            mpClockPainter.reset(
                new AnalogBitmapPainter(
                    mxComponentContext,
                    OUString::createFromAscii("ClockTheme")));
            mpClockPainter2.reset(new AnalogDefaultPainter());
            break;
    }
    Resize();
}




void PresenterClock::ThrowIfDisposed (void)
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "PresenterClock object has already been disposed")),
            static_cast<uno::XWeak*>(this));
    }
}




//===== Timer =================================================================

PresenterClock::Timer::Timer (const ::rtl::Reference<PresenterClock>& rpClock)
    : salhelper::Timer(salhelper::TTimeValue(10), salhelper::TTimeValue(100/*ms*/)),
      mpClock(rpClock)
{
    acquire();
    start();
}




PresenterClock::Timer::~Timer (void)
{
    if (mpClock.is())
        Stop();
}




void PresenterClock::Timer::Stop (void)
{
    mpClock = NULL;
    stop();
    release();
}




void SAL_CALL PresenterClock::Timer::onShot (void)
{
    if (mpClock.get() != NULL)
        mpClock->UpdateTime();
}



namespace {

//=============================================================================

bool GetDateTime (oslDateTime& rDateTime)
{
    TimeValue aSystemTime;
    TimeValue aLocalTime;
    if (osl_getSystemTime(&aSystemTime))
        if (osl_getLocalTimeFromSystemTime(&aSystemTime, &aLocalTime))
            if (osl_getDateTimeFromTimeValue(&aLocalTime, &rDateTime))
                return true;
    return false;
}




//===== AnalogDefaultPainter ==================================================

const double AnalogDefaultPainter::mnRelativeHourHandLength = 0.65;
const double AnalogDefaultPainter::mnRelativeHourHandLength2 (-0.1);
const double AnalogDefaultPainter::mnRelativeHourHandWidth (0.055);
const double AnalogDefaultPainter::mnRelativeMinuteHandLength (-0.2);
const double AnalogDefaultPainter::mnRelativeMinuteHandLength2 (0.85);
const double AnalogDefaultPainter::mnRelativeMinuteHandWidth (0.025);
const double AnalogDefaultPainter::mnRelativeSecondHandLength (-0.25);
const double AnalogDefaultPainter::mnRelativeSecondHandLength2 (0.95);
const double AnalogDefaultPainter::mnRelativeSecondHandWidth (0.015);

AnalogDefaultPainter::AnalogDefaultPainter (void)
    : maCenter(0,0),
      mnOuterRadius(0),
      maSize(0,0),
      mxBitmap()
{
}




void AnalogDefaultPainter::Paint (
    const Reference<rendering::XCanvas>& rxCanvas,
    const rendering::ViewState& rViewState,
    const rendering::RenderState& rRenderState,
    const util::Color& rBackgroundColor,
    const sal_Int32 nHour,
    const sal_Int32 nMinute,
    const sal_Int32 nSecond,
    const bool bShowSeconds)
{
    double nInnerRadius (0);
    double nStrokeWidth (0.1);
    const double nClockSize (2*mnOuterRadius);

    // Some antialiasing is created by painting into a bitmap twice the
    // screen size and then scaling it down.
    const sal_Int32 nSuperSampleFactor (2);
    if ( ! mxBitmap.is())
    {
        mxBitmap = (rxCanvas->getDevice()->createCompatibleBitmap(
            geometry::IntegerSize2D(
                maSize.Width*nSuperSampleFactor,
                maSize.Height*nSuperSampleFactor)));
    }
    Reference<rendering::XCanvas> xBitmapCanvas (mxBitmap, UNO_QUERY);
    rendering::RenderState aRenderState(rRenderState);
    aRenderState.AffineTransform.m00 = nSuperSampleFactor;
    aRenderState.AffineTransform.m11 = nSuperSampleFactor;

    // Clear the background.
    aRenderState.DeviceColor[0] = ((rBackgroundColor&0x00ff0000) >> 16) / 255.0;
    aRenderState.DeviceColor[1] = ((rBackgroundColor&0x0000ff00) >>  8) / 255.0;
    aRenderState.DeviceColor[2] = ((rBackgroundColor&0x000000ff) >>  0) / 255.0;
    Reference<rendering::XPolyPolygon2D> xPolygon (
        PresenterGeometryHelper::CreatePolygon(
            awt::Rectangle(0,0,maSize.Width,maSize.Height),
            xBitmapCanvas->getDevice()));
    if (xPolygon.is())
        xBitmapCanvas->fillPolyPolygon(xPolygon, rViewState, aRenderState);

    // Clock face and clock hands are painted in black.
    aRenderState.DeviceColor[0] = 0;
    aRenderState.DeviceColor[1] = 0;
    aRenderState.DeviceColor[2] = 0;

    // Paint the clock face.
    for (sal_Int32 nHourMark=0; nHourMark<12; ++nHourMark)
    {
        if (nHourMark%3 == 0)
        {
            nInnerRadius = 0.7 * mnOuterRadius;
            nStrokeWidth = 0.05 * nClockSize;
        }
        else
        {
            nInnerRadius = 0.8 * mnOuterRadius;
            nStrokeWidth = 0.03 * nClockSize;
        }

        const double nAngle (nHourMark * 2 * M_PI / 12);
        PaintAngledLine(nAngle, nInnerRadius, mnOuterRadius, nStrokeWidth,
            xBitmapCanvas, rViewState, aRenderState);
    }

    // Paint the hour hand.
    const double nHoursAngle (((nHour%12)+nMinute/60.0) * 2 * M_PI / 12);
    PaintAngledLine(nHoursAngle,
        mnRelativeHourHandLength2*mnOuterRadius,
        mnRelativeHourHandLength*mnOuterRadius,
        mnRelativeHourHandWidth*nClockSize,
        xBitmapCanvas, rViewState, aRenderState);

    // Paint the minute hand.
    const double nMinutesAngle ((nMinute+nSecond/60.0) * 2 * M_PI / 60);
    PaintAngledLine(nMinutesAngle,
        mnRelativeMinuteHandLength2*mnOuterRadius,
        mnRelativeMinuteHandLength*mnOuterRadius,
        mnRelativeMinuteHandWidth*nClockSize,
        xBitmapCanvas, rViewState, aRenderState);

    // Optionally paint the second hand.
    if (bShowSeconds)
    {
        const double nSecondsAngle (nSecond * 2 * M_PI / 60);
        PaintAngledLine(nSecondsAngle,
            mnRelativeSecondHandLength2*mnOuterRadius,
            mnRelativeSecondHandLength*mnOuterRadius,
            mnRelativeSecondHandWidth*nClockSize,
            xBitmapCanvas, rViewState, aRenderState);
    }

    aRenderState.AffineTransform.m00 = 1.0 / nSuperSampleFactor;
    aRenderState.AffineTransform.m11 = 1.0 / nSuperSampleFactor;
    rxCanvas->drawBitmap(mxBitmap,rViewState,aRenderState);
}




void AnalogDefaultPainter::PaintAngledLine (
    const double nAngle,
    const double nInnerRadius,
    const double nOuterRadius,
    const double nStrokeWidth,
    const Reference<rendering::XCanvas>& rxCanvas,
    const rendering::ViewState& rViewState,
    const rendering::RenderState& rRenderState)
{
    if ( ! rxCanvas.is())
        return;

    rendering::StrokeAttributes aStrokeAttributes;
    aStrokeAttributes.StrokeWidth = nStrokeWidth;
    aStrokeAttributes.StartCapType = rendering::PathCapType::SQUARE;
    aStrokeAttributes.EndCapType = rendering::PathCapType::SQUARE;
    aStrokeAttributes.StartCapType = rendering::PathCapType::BUTT;
    aStrokeAttributes.EndCapType = rendering::PathCapType::BUTT;
    const double nCos (cos(nAngle - M_PI/2));
    const double nSin (sin(nAngle - M_PI/2));

    Sequence<Sequence<geometry::RealPoint2D> > aPoints(1);
    aPoints[0] = Sequence<geometry::RealPoint2D>(2);
    aPoints[0][0] = geometry::RealPoint2D(
        maCenter.X + nInnerRadius*nCos + 0.5,
        maCenter.Y + nInnerRadius*nSin + 0.5);
    aPoints[0][1] = geometry::RealPoint2D(
        maCenter.X + nOuterRadius*nCos + 0.5,
        maCenter.Y + nOuterRadius*nSin + 0.5);

    Reference<rendering::XPolyPolygon2D> xLine (
        rxCanvas->getDevice()->createCompatibleLinePolyPolygon(aPoints),
        UNO_QUERY);
    if ( ! xLine.is())
        return;
    rxCanvas->strokePolyPolygon(
        xLine,
        rViewState,
        rRenderState,
        aStrokeAttributes);
}




void AnalogDefaultPainter::Resize (const awt::Size& rWindowSize)
{
    maSize = rWindowSize;
    maCenter = geometry::RealPoint2D(rWindowSize.Width/2.0, rWindowSize.Height/2.0);
    mnOuterRadius = ::std::min(rWindowSize.Width, rWindowSize.Height) / 2.0 - 2;
    mxBitmap = NULL;
}




//===== AnalogBitmapPainter ===================================================

AnalogBitmapPainter::AnalogBitmapPainter (
    const Reference<XComponentContext>& rxContext,
    const OUString& rsThemeName)
    : mxComponentContext(rxContext),
      msThemeName(rsThemeName),
      mbThemeLoaded(false),
      mbThemeLoadingFailed(false),
      maCenter(),
      mnOuterRadius(),
      maFace(),
      maMinuteHand(),
      maHourHand()
{
}




void AnalogBitmapPainter::Paint (
    const Reference<rendering::XCanvas>& rxCanvas,
    const rendering::ViewState& rViewState,
    const rendering::RenderState& rRenderState,
    const util::Color& rBackgroundColor,
    const sal_Int32 nHour,
    const sal_Int32 nMinute,
    const sal_Int32 nSecond,
    const bool bShowSeconds)
{
    (void)rBackgroundColor;
    (void)nSecond;
    (void)bShowSeconds;

    if ( ! rxCanvas.is())
        return;

    rendering::RenderState aRenderState = rRenderState;

    try
    {
        PrepareBitmaps(rxCanvas);

        if (maFace.mxScaledBitmap.is())
        {
            aRenderState.AffineTransform = geometry::AffineMatrix2D(
                1,0, maCenter.X - maFace.maScaledOffset.X,
                0,1, maCenter.Y - maFace.maScaledOffset.Y);
            rxCanvas->drawBitmap(maFace.mxScaledBitmap, rViewState, aRenderState);
        }

        if (maMinuteHand.mxScaledBitmap.is())
        {
            const double nMinuteAngle ((nMinute+nSecond/60.0) * 2.0 * M_PI / 60.0);
            const double nCos (cos(nMinuteAngle - M_PI/2));
            const double nSin (sin(nMinuteAngle - M_PI/2));
            aRenderState.AffineTransform = geometry::AffineMatrix2D(
                nCos,
                -nSin,
                -maMinuteHand.maScaledOffset.X*nCos
                    + maMinuteHand.maScaledOffset.Y*nSin+maCenter.X,
                nSin,
                nCos,
                -maMinuteHand.maScaledOffset.X*nSin
                    - maMinuteHand.maScaledOffset.Y*nCos+maCenter.Y);
            rxCanvas->drawBitmap(maMinuteHand.mxScaledBitmap, rViewState, aRenderState);
        }

        if (maHourHand.mxScaledBitmap.is())
        {
            const double nHoursAngle ((nHour%12+nMinute/60.0) * 2.0 * M_PI / 12.0);
            const double nCos (cos(nHoursAngle - M_PI/2));
            const double nSin (sin(nHoursAngle - M_PI/2));
            aRenderState.AffineTransform = geometry::AffineMatrix2D(
                nCos,
                -nSin,
                -maHourHand.maScaledOffset.X*nCos+maHourHand.maScaledOffset.Y*nSin+maCenter.X,
                nSin,
                nCos,
                -maHourHand.maScaledOffset.X*nSin-maHourHand.maScaledOffset.Y*nCos+maCenter.Y);
            rxCanvas->drawBitmap(maHourHand.mxScaledBitmap, rViewState, aRenderState);
        }
    }
    catch(beans::UnknownPropertyException&)
    {
    }
    catch(RuntimeException&)
    {
    }
}




void AnalogBitmapPainter::Resize (const awt::Size& rWindowSize)
{
    maCenter = geometry::RealPoint2D(rWindowSize.Width/2.0, rWindowSize.Height/2.0);
    mnOuterRadius = ::std::min(rWindowSize.Width, rWindowSize.Height) / 2.0 - 2;
    maFace.mxScaledBitmap = NULL;
    maHourHand.mxScaledBitmap = NULL;
    maMinuteHand.mxScaledBitmap = NULL;
}




void AnalogBitmapPainter::PrepareBitmaps (const Reference<rendering::XCanvas>& rxCanvas)
{
    if (mbThemeLoadingFailed)
    {
        // Theme loading has failed previously.  Do not try a second time.
        return;
    }
    if ( ! rxCanvas.is())
    {
        // No canvas => bitmaps can neither be loaded, transformed into the
        // right format, nor can they be painted.
        return;
    }

    if ( ! mbThemeLoaded)
    {
        mbThemeLoaded = true;

        // Get access to the clock bitmaps in the configuration.
        PresenterConfigurationAccess aConfiguration (
            mxComponentContext,
            OUString::createFromAscii("org.openoffice.Office.extension.PresenterScreen"),
            PresenterConfigurationAccess::READ_ONLY);

        Reference<container::XNameAccess> xTheme (GetTheme(aConfiguration));
        if (xTheme.is())
            LoadBitmaps(aConfiguration, xTheme, rxCanvas);
        else
            mbThemeLoadingFailed = true;
    }

    ScaleBitmaps();
}




Reference<container::XNameAccess> AnalogBitmapPainter::GetTheme (
    PresenterConfigurationAccess& rConfiguration)
{
    Reference<container::XNameAccess> xTheme;

    // Get root of clock themes.
    Reference<container::XHierarchicalNameAccess> xClock (
        rConfiguration.GetConfigurationNode(
            OUString::createFromAscii("PresenterScreenSettings/AnalogBitmapClock")),
        UNO_QUERY);

    // Determine the name of the theme to use.
    OUString sCurrentThemeName (OUString::createFromAscii("DefaultTheme"));
    rConfiguration.GetConfigurationNode(
        xClock,
        OUString::createFromAscii("CurrentTheme")) >>= sCurrentThemeName;

    // Load the clock theme.
    Reference<container::XNameAccess> xThemes (
        rConfiguration.GetConfigurationNode(
            xClock,
            OUString::createFromAscii("Themes")),
        UNO_QUERY);
    if (xThemes.is())
    {
        xTheme = Reference<container::XNameAccess>(
            PresenterConfigurationAccess::Find(
                xThemes,
                ::boost::bind(&AnalogBitmapPainter::ThemeNameComparator,
                    this, _1, _2, sCurrentThemeName)),
            UNO_QUERY);
    }

    return xTheme;
}




bool AnalogBitmapPainter::ThemeNameComparator (
    const OUString& rsKey,
    const Reference<container::XNameAccess>& rxCandidate,
    const OUString& rsCurrentThemeName)
{
    (void)rsKey;
    if (rxCandidate.is())
    {
        OUString sThemeName;
        if (rxCandidate->getByName(OUString::createFromAscii("ThemeName")) >>= sThemeName)
        {
            return sThemeName == rsCurrentThemeName;
        }
    }
    return false;
}





void AnalogBitmapPainter::LoadBitmaps (
    PresenterConfigurationAccess& rConfiguration,
    const Reference<container::XNameAccess>& rxClockTheme,
    const Reference<rendering::XCanvas>& rxCanvas)
{
    (void)rConfiguration;

    // Get base path to bitmaps.
    Reference<deployment::XPackageInformationProvider> xInformationProvider (
        mxComponentContext->getValueByName(OUString::createFromAscii(
            "/singletons/com.sun.star.deployment.PackageInformationProvider")),
        UNO_QUERY);
    OUString sLocation;
    if (xInformationProvider.is())
        sLocation = xInformationProvider->getPackageLocation(gsExtensionIdentifier);
    sLocation += OUString::createFromAscii("/");

    // Create the bitmap loader.
    Reference<lang::XMultiComponentFactory> xFactory (
        mxComponentContext->getServiceManager(), UNO_QUERY);
    if ( ! xFactory.is())
        return;
    Sequence<Any> aArguments(1);
    aArguments[0] <<= rxCanvas;
    Reference<container::XNameAccess> xBitmapLoader(
        xFactory->createInstanceWithArgumentsAndContext(
            OUString::createFromAscii("com.sun.star.drawing.PresenterWorkaroundService"),
            aArguments,
            mxComponentContext),
        UNO_QUERY);
    if ( ! xBitmapLoader.is())
        return;


    // Iterate over all entries in the bitmap list and load the bitmaps.
    Reference<container::XNameAccess> xBitmaps (
        rxClockTheme->getByName(OUString::createFromAscii("Bitmaps")),
        UNO_QUERY);
    ::std::vector<rtl::OUString> aBitmapProperties (3);
    aBitmapProperties[0] = OUString::createFromAscii("FileName");
    aBitmapProperties[1] = OUString::createFromAscii("XOffset");
    aBitmapProperties[2] = OUString::createFromAscii("YOffset");
    PresenterConfigurationAccess::ForAll(
        xBitmaps,
        aBitmapProperties,
        ::boost::bind(&AnalogBitmapPainter::LoadBitmap,
            this,
            _1,
            _2,
            sLocation,
            xBitmapLoader));
}




void AnalogBitmapPainter::LoadBitmap (
    const OUString& rsKey,
    const ::std::vector<Any>& rValues,
    const OUString& rsBitmapPath,
    const Reference<container::XNameAccess>& rxBitmapLoader)
{
    if (rValues.size() == 3)
    {
        BitmapDescriptor* pDescriptor = NULL;
        if (rsKey == OUString::createFromAscii("Face"))
            pDescriptor = &maFace;
        else if (rsKey == OUString::createFromAscii("HourHand"))
            pDescriptor = &maHourHand;
        else if (rsKey == OUString::createFromAscii("MinuteHand"))
            pDescriptor = &maMinuteHand;

        if (pDescriptor == NULL)
            return;

        OUString sFileName;
        if ( ! (rValues[0] >>= sFileName))
            return;

        rValues[1] >>= pDescriptor->maOffset.X;
        rValues[2] >>= pDescriptor->maOffset.Y;

        pDescriptor->mxBitmap = Reference<rendering::XBitmap>(
            rxBitmapLoader->getByName(rsBitmapPath+sFileName), UNO_QUERY);

        if ( ! pDescriptor->mxBitmap.is())
            mbThemeLoadingFailed = true;
    }
}




void AnalogBitmapPainter::ScaleBitmaps (void)
{
    if (mbThemeLoadingFailed)
        return;
    if ( ! maFace.mxBitmap.is())
        return;

    const geometry::IntegerSize2D aFaceSize (maFace.mxBitmap->getSize());
    const sal_Int32 nSize = std::max(aFaceSize.Width, aFaceSize.Height);
    const double nScale = mnOuterRadius*2 / nSize;

    BitmapDescriptor* aDescriptors[3] = { &maFace, &maHourHand, &maMinuteHand };
    for (int nIndex=0; nIndex<3; ++nIndex)
    {
        BitmapDescriptor& rDescriptor (*aDescriptors[nIndex]);
        if ( ! rDescriptor.mxScaledBitmap.is() && rDescriptor.mxBitmap.is())
        {
            const geometry::IntegerSize2D aBitmapSize (rDescriptor.mxBitmap->getSize());
            rDescriptor.mxScaledBitmap = rDescriptor.mxBitmap->getScaledBitmap(
                geometry::RealSize2D(aBitmapSize.Width*nScale, aBitmapSize.Height*nScale),
                sal_False);
            rDescriptor.maScaledOffset = geometry::RealPoint2D(
                rDescriptor.maOffset.X * nScale,
                rDescriptor.maOffset.Y * nScale);
        }
    }
}




//===== DigitalDefaultPainter =================================================

DigitalDefaultPainter::DigitalDefaultPainter (
    const ::rtl::Reference<PresenterController>& rpPresenterController,
    const Reference<XResourceId>& rxViewId)
    :  mpPresenterController(rpPresenterController),
       mbIs24HourFormat(false),
       mbIsAdaptFontSize(true),
       mxFont(),
       maWindowSize(0,0),
       msViewURL(rxViewId.is() ? rxViewId->getResourceURL() : OUString())
{
}




DigitalDefaultPainter::~DigitalDefaultPainter (void)
{
}




void DigitalDefaultPainter::Paint (
    const Reference<rendering::XCanvas>& rxCanvas,
    const rendering::ViewState& rViewState,
    const rendering::RenderState& rRenderState,
    const util::Color& rBackgroundColor,
    const sal_Int32 nHour,
    const sal_Int32 nMinute,
    const sal_Int32 nSecond,
    const bool bIsShowSeconds)
{
    (void)rBackgroundColor;
    (void)rRenderState;

    if ( ! mxFont.is())
        CreateFont(rxCanvas,bIsShowSeconds);
    if ( ! mxFont.is())
        return;

    OUString sText;

    if (mbIs24HourFormat)
        sText = OUString::valueOf(nHour);
    else
    {
        sText = OUString::valueOf(nHour>12 ? nHour-12 : nHour);
    }
    sText += OUString::createFromAscii(":");
    const OUString sMinutes (OUString::valueOf(nMinute));
    switch (sMinutes.getLength())
    {
        case 1 :
            sText += OUString::createFromAscii("0") + sMinutes;
            break;
        case 2:
            sText += sMinutes;
            break;

        default:
            return;
    }
    if (bIsShowSeconds)
    {
        sText += OUString::createFromAscii(":");
        const OUString sSeconds (OUString::valueOf(nSecond));
        switch (sSeconds.getLength())
        {
            case 1 :
                sText += OUString::createFromAscii("0") + sSeconds;
                break;
            case 2:
                sText += sSeconds;
                break;

            default:
                return;
        }
    }

    rendering::StringContext aContext (
        sText,
        0,
        sText.getLength());
    Reference<rendering::XTextLayout> xLayout (mxFont->createTextLayout(
        aContext,
        rendering::TextDirection::WEAK_LEFT_TO_RIGHT,
        0));
    if ( ! xLayout.is())
        return;
    geometry::RealRectangle2D aBox (xLayout->queryTextBounds());


    rendering::RenderState aRenderState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL,
        Sequence<double>(4),
        rendering::CompositeOperation::SOURCE);

    util::Color aFontColor (mpPresenterController->GetViewFontColor(msViewURL));
    PresenterCanvasHelper::SetDeviceColor(aRenderState, aFontColor);
    aRenderState.AffineTransform.m02
        = (maWindowSize.Width - (aBox.X2-aBox.X1+1)) / 2 - aBox.X1;
    aRenderState.AffineTransform.m12
        = (maWindowSize.Height - (aBox.Y2-aBox.Y1+1)) / 2 - aBox.Y1;
    rxCanvas->drawText(
        aContext,
        mxFont,
        rViewState,
        aRenderState,
        rendering::TextDirection::WEAK_LEFT_TO_RIGHT);
}




void DigitalDefaultPainter::Resize (const awt::Size& rSize)
{
    if (maWindowSize.Width != rSize.Width || maWindowSize.Height != rSize.Height)
    {
        maWindowSize = rSize;
        if (mbIsAdaptFontSize)
            mxFont = NULL;
    }
}




void DigitalDefaultPainter::CreateFont (
    const Reference<rendering::XCanvas>& rxCanvas,
    const bool bIsShowSeconds)
{
    if (rxCanvas.is()
        && rxCanvas->getDevice().is()
        && maWindowSize.Width>0
        && maWindowSize.Height>0)
    {
        // Create a time template for determinging the right font size.
        // Assume that 0 is the widest digit or that all digits have the
        // same width.
        OUString sTimeTemplate;
        // For the case that not all digits have the same width, create
        // different templates for 12 and 24 hour mode.
        if (mbIs24HourFormat)
            sTimeTemplate = OUString::createFromAscii("20");
        else
            sTimeTemplate = OUString::createFromAscii("10");
        if (bIsShowSeconds)
            sTimeTemplate += OUString::createFromAscii(":00:00");
        else
            sTimeTemplate += OUString::createFromAscii(":00");

        rendering::StringContext aContext (
            sTimeTemplate,
            0,
            sTimeTemplate.getLength());

        // When the font size is adapted to the window size (as large as
        // possible without overlapping) then that is done in a four step
        // process:
        // 1. Create a font in a default size, e.g. 10pt.
        // 2. Determine a scale factor from enlarging the text bounding box
        // to maximal size inside the window.
        // 3. Create a new font by scaling the default size with the factor
        // calculated in step 2.
        // 4. Text may be rendered differently in different sizes.
        // Therefore repeat step 2 and 3 once.  More iterations may lead to
        // even better results but probably not to visible differences.
        rendering::FontRequest aFontRequest (mpPresenterController->GetViewFontRequest(msViewURL));
        // TODO: use font from view style from configuration
        aFontRequest.CellSize = 10;

        for (sal_Int32 nLoop=0; nLoop<3; ++nLoop)
        {
            mxFont = rxCanvas->createFont(
                aFontRequest,
                Sequence<beans::PropertyValue>(),
                geometry::Matrix2D(1,0,0,1));
            if (mxFont.is())
            {
                Reference<rendering::XTextLayout> xLayout (mxFont->createTextLayout(
                    aContext,
                    rendering::TextDirection::WEAK_LEFT_TO_RIGHT,
                    0));

                if ( ! xLayout.is())
                    break;

                geometry::RealRectangle2D aBox (xLayout->queryTextBounds());
                if (aBox.X2<=aBox.X1 || aBox.Y2<=aBox.Y1)
                    break;
                const double nHorizontalFactor = maWindowSize.Width / (aBox.X2-aBox.X1+1);
                const double nVerticalFactor = maWindowSize.Height / (aBox.Y2-aBox.Y1+1);
                aFontRequest.CellSize *= ::std::min(nHorizontalFactor,nVerticalFactor);
            }
        }
    }
}


} // end of anonymous namespace


} } // end of namespace ::sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

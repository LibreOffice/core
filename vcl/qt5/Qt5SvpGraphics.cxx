/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <sal/log.hxx>
#include <salbmp.hxx>

#include <config_cairo_canvas.h>

#include <Qt5Frame.hxx>
#include <Qt5SvpGraphics.hxx>
#include <Qt5SvpSurface.hxx>
#include <Qt5Tools.hxx>

#include <QtGui/QScreen>
#include <QtGui/QWindow>
#include <QtWidgets/QWidget>

Qt5SvpGraphics::Qt5SvpGraphics(Qt5Frame* pFrame)
    : SvpSalGraphics()
    , m_pFrame(pFrame)
{
}

Qt5SvpGraphics::~Qt5SvpGraphics() {}

void Qt5SvpGraphics::updateQWidget() const
{
    if (!m_pFrame)
        return;
    QWidget* pQWidget = m_pFrame->GetQWidget();
    if (pQWidget)
        pQWidget->update(pQWidget->rect());
}

#if ENABLE_CAIRO_CANVAS

bool Qt5SvpGraphics::SupportsCairo() const { return true; }

cairo::SurfaceSharedPtr
Qt5SvpGraphics::CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const
{
    return cairo::SurfaceSharedPtr(new cairo::Qt5SvpSurface(rSurface));
}

cairo::SurfaceSharedPtr Qt5SvpGraphics::CreateSurface(const OutputDevice& /*rRefDevice*/, int x,
                                                      int y, int width, int height) const
{
    return cairo::SurfaceSharedPtr(new cairo::Qt5SvpSurface(this, x, y, width, height));
}

#endif

static void QImage2BitmapBuffer(QImage* pImg, BitmapBuffer* pBuf)
{
    if (pImg->width() != 0 && pImg->height() != 0)
    {
        pBuf->mnWidth = pImg->width();
        pBuf->mnHeight = pImg->height();
        pBuf->mnBitCount = getFormatBits(pImg->format());
        pBuf->mpBits = pImg->bits();
        pBuf->mnScanlineSize = pImg->bytesPerLine();
    }
}

bool Qt5SvpGraphics::drawNativeControl(ControlType nType, ControlPart nPart,
                                       const tools::Rectangle& rControlRegion, ControlState nState,
                                       const ImplControlValue& aValue, const OUString& aCaption)
{
    bool bHandled
        = m_aControl.drawNativeControl(nType, nPart, rControlRegion, nState, aValue, aCaption);
    if (bHandled)
    {
        QImage* pImage = &m_aControl.getImage();
        BitmapBuffer* pBuffer = new BitmapBuffer;
        QImage2BitmapBuffer(pImage, pBuffer);
        SalTwoRect aTR(0, 0, pImage->width(), pImage->height(), rControlRegion.getX(),
                       rControlRegion.getY(), rControlRegion.GetWidth(),
                       rControlRegion.GetHeight());
        drawBitmap(aTR, pBuffer, CAIRO_OPERATOR_OVER);
    }
    return bHandled;
}

void Qt5SvpGraphics::GetResolution(sal_Int32& rDPIX, sal_Int32& rDPIY)
{
    char* pForceDpi;
    if ((pForceDpi = getenv("SAL_FORCEDPI")))
    {
        OString sForceDPI(pForceDpi);
        rDPIX = rDPIY = sForceDPI.toInt32();
        return;
    }

    if (!m_pFrame || !m_pFrame->GetQWidget()->window()->windowHandle())
        return;

    QScreen* pScreen = m_pFrame->GetQWidget()->window()->windowHandle()->screen();
    rDPIX = pScreen->logicalDotsPerInchX() * pScreen->devicePixelRatio() + 0.5;
    rDPIY = pScreen->logicalDotsPerInchY() * pScreen->devicePixelRatio() + 0.5;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

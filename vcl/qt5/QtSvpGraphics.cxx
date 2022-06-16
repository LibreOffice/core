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

#include <QtData.hxx>
#include <QtFrame.hxx>
#include <QtGraphics_Controls.hxx>
#include <QtSvpGraphics.hxx>
#include <QtSvpSurface.hxx>
#include <QtTools.hxx>

#include <QtGui/QScreen>
#include <QtGui/QWindow>
#include <QtWidgets/QWidget>

QtSvpGraphics::QtSvpGraphics(QtFrame* pFrame, sal_Int32 nScale)
    : m_pFrame(pFrame)
    , m_nScalePercentage(pFrame ? pFrame->GetDPIScalePercentage() : nScale)
{
    assert(m_nScalePercentage > 0);
    if (!QtData::noNativeControls())
        m_pWidgetDraw.reset(new QtGraphics_Controls(*this));
}

QtSvpGraphics::~QtSvpGraphics() {}

void QtSvpGraphics::updateQWidget() const
{
    if (!m_pFrame)
        return;
    QWidget* pQWidget = m_pFrame->GetQWidget();
    if (pQWidget)
        pQWidget->update(pQWidget->rect());
}

#if ENABLE_CAIRO_CANVAS

bool QtSvpGraphics::SupportsCairo() const { return true; }

cairo::SurfaceSharedPtr
QtSvpGraphics::CreateSurface(const cairo::CairoSurfaceSharedPtr& rSurface) const
{
    return std::make_shared<cairo::QtSvpSurface>(rSurface);
}

cairo::SurfaceSharedPtr QtSvpGraphics::CreateSurface(const OutputDevice& /*rRefDevice*/, int x,
                                                     int y, int width, int height) const
{
    return std::make_shared<cairo::QtSvpSurface>(this, x, y, width, height);
}

#endif

static void QImage2BitmapBuffer(QImage& rImg, BitmapBuffer& rBuf)
{
    assert(rImg.width());
    assert(rImg.height());

    rBuf.mnWidth = rImg.width();
    rBuf.mnHeight = rImg.height();
    rBuf.mnBitCount = getFormatBits(rImg.format());
    rBuf.mpBits = rImg.bits();
    rBuf.mnScanlineSize = rImg.bytesPerLine();
}

void QtSvpGraphics::handleDamage(const tools::Rectangle& rDamagedRegion)
{
    assert(m_pWidgetDraw);
    assert(dynamic_cast<QtGraphics_Controls*>(m_pWidgetDraw.get()));
    assert(!rDamagedRegion.IsEmpty());

    QImage* pImage = static_cast<QtGraphics_Controls*>(m_pWidgetDraw.get())->getImage();
    assert(pImage);
    if (pImage->width() == 0 || pImage->height() == 0)
        return;

    BitmapBuffer aBuffer;
    QImage2BitmapBuffer(*pImage, aBuffer);
    SalTwoRect aTR(0, 0, pImage->width(), pImage->height(), rDamagedRegion.Left(),
                   rDamagedRegion.Top(), rDamagedRegion.GetWidth(), rDamagedRegion.GetHeight());

    getSvpBackend()->drawBitmapBuffer(aTR, &aBuffer, CAIRO_OPERATOR_OVER);
}

sal_Int32 QtSvpGraphics::GetSgpMetric(vcl::SGPmetric eMetric) const
{
    if (eMetric == vcl::SGPmetric::ScalePercentage)
        return m_nScalePercentage;

    if (m_pWidgetDraw)
    {
        QImage* pImage = static_cast<QtGraphics_Controls*>(m_pWidgetDraw.get())->getImage();
        assert(pImage);
        return GetSgpMetricFromQImage(eMetric, *pImage);
    }
    else
        return SvpSalGraphics::GetSgpMetric(eMetric);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

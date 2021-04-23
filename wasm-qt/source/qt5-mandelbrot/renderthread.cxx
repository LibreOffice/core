/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "renderthread.h"
#include "renderthread.moc"

#include <QtGui/QImage>
#include <cmath>

RenderThread::RenderThread(QObject* parent)
    : QThread(parent)
{
    for (int i = 0; i < ColormapSize; ++i)
        m_colormap[i] = rgbFromWaveLength(380.0 + (i * 400.0 / ColormapSize));
}

RenderThread::~RenderThread()
{
    m_mutex.lock();
    m_abort = true;
    m_condition.wakeOne();
    m_mutex.unlock();

    wait();
}

void RenderThread::render(double centerX, double centerY, double scaleFactor, QSize resultSize,
                          double devicePixelRatio)
{
    QMutexLocker locker(&m_mutex);

    m_centerX = centerX;
    m_centerY = centerY;
    m_scaleFactor = scaleFactor;
    m_devicePixelRatio = devicePixelRatio;
    m_resultSize = resultSize;

    if (!isRunning())
    {
        start(LowPriority);
    }
    else
    {
        m_restart = true;
        m_condition.wakeOne();
    }
}

void RenderThread::run()
{
    forever
    {
        m_mutex.lock();
        const double devicePixelRatio = m_devicePixelRatio;
        const QSize resultSize = m_resultSize * devicePixelRatio;
        const double requestedScaleFactor = m_scaleFactor;
        const double scaleFactor = requestedScaleFactor / devicePixelRatio;
        const double centerX = m_centerX;
        const double centerY = m_centerY;
        m_mutex.unlock();

        int halfWidth = resultSize.width() / 2;
        int halfHeight = resultSize.height() / 2;
        QImage image(resultSize, QImage::Format_RGB32);
        image.setDevicePixelRatio(devicePixelRatio);

        const int NumPasses = 8;
        int pass = 0;
        while (pass < NumPasses)
        {
            const int MaxIterations = (1 << (2 * pass + 6)) + 32;
            const int Limit = 4;
            bool allBlack = true;

            for (int y = -halfHeight; y < halfHeight; ++y)
            {
                if (m_restart)
                    break;
                if (m_abort)
                    return;

                auto scanLine = reinterpret_cast<uint*>(image.scanLine(y + halfHeight));
                const double ay = centerY + (y * scaleFactor);

                for (int x = -halfWidth; x < halfWidth; ++x)
                {
                    const double ax = centerX + (x * scaleFactor);
                    double a1 = ax;
                    double b1 = ay;
                    int numIterations = 0;

                    do
                    {
                        ++numIterations;
                        const double a2 = (a1 * a1) - (b1 * b1) + ax;
                        const double b2 = (2 * a1 * b1) + ay;
                        if ((a2 * a2) + (b2 * b2) > Limit)
                            break;

                        ++numIterations;
                        a1 = (a2 * a2) - (b2 * b2) + ax;
                        b1 = (2 * a2 * b2) + ay;
                        if ((a1 * a1) + (b1 * b1) > Limit)
                            break;
                    } while (numIterations < MaxIterations);

                    if (numIterations < MaxIterations)
                    {
                        *scanLine++ = m_colormap[numIterations % ColormapSize];
                        allBlack = false;
                    }
                    else
                    {
                        *scanLine++ = qRgb(0, 0, 0);
                    }
                }
            }

            if (allBlack && pass == 0)
            {
                pass = 4;
            }
            else
            {
                if (!m_restart)
                    emit renderedImage(image, requestedScaleFactor);
                ++pass;
            }
        }

        m_mutex.lock();
        if (!m_restart)
            m_condition.wait(&m_mutex);
        m_restart = false;
        m_mutex.unlock();
    }
}

uint RenderThread::rgbFromWaveLength(double wave)
{
    double r = 0;
    double g = 0;
    double b = 0;

    if (wave >= 380.0 && wave <= 440.0)
    {
        r = -1.0 * (wave - 440.0) / (440.0 - 380.0);
        b = 1.0;
    }
    else if (wave >= 440.0 && wave <= 490.0)
    {
        g = (wave - 440.0) / (490.0 - 440.0);
        b = 1.0;
    }
    else if (wave >= 490.0 && wave <= 510.0)
    {
        g = 1.0;
        b = -1.0 * (wave - 510.0) / (510.0 - 490.0);
    }
    else if (wave >= 510.0 && wave <= 580.0)
    {
        r = (wave - 510.0) / (580.0 - 510.0);
        g = 1.0;
    }
    else if (wave >= 580.0 && wave <= 645.0)
    {
        r = 1.0;
        g = -1.0 * (wave - 645.0) / (645.0 - 580.0);
    }
    else if (wave >= 645.0 && wave <= 780.0)
    {
        r = 1.0;
    }

    double s = 1.0;
    if (wave > 700.0)
        s = 0.3 + 0.7 * (780.0 - wave) / (780.0 - 700.0);
    else if (wave < 420.0)
        s = 0.3 + 0.7 * (wave - 380.0) / (420.0 - 380.0);

    r = std::pow(r * s, 0.8);
    g = std::pow(g * s, 0.8);
    b = std::pow(b * s, 0.8);
    return qRgb(int(r * 255), int(g * 255), int(b * 255));
}

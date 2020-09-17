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

#pragma once

#include <vclpluginapi.h>
#include <WidgetDrawInterface.hxx>

#include <memory>

#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QRegion>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStyle>
#include <QtWidgets/QStyleOption>

class Qt5GraphicsBase;

class Qt5Graphics_Controls final : public vcl::WidgetDrawInterface
{
    std::unique_ptr<QImage> m_image;
    QRect m_lastPopupRect;
    Qt5GraphicsBase const& m_rGraphics;

public:
    Qt5Graphics_Controls(const Qt5GraphicsBase& rGraphics);

    QImage* getImage() { return m_image.get(); }

    bool isNativeControlSupported(ControlType nType, ControlPart nPart) override;
    bool hitTestNativeControl(ControlType nType, ControlPart nPart,
                              const tools::Rectangle& rControlRegion, const Point& aPos,
                              bool& rIsInside) override;
    bool drawNativeControl(ControlType nType, ControlPart nPart,
                           const tools::Rectangle& rControlRegion, ControlState nState,
                           const ImplControlValue& aValue, const OUString& aCaption,
                           const Color& rBackgroundColor) override;
    bool getNativeControlRegion(ControlType nType, ControlPart nPart,
                                const tools::Rectangle& rControlRegion, ControlState nState,
                                const ImplControlValue& aValue, const OUString& aCaption,
                                tools::Rectangle& rNativeBoundingRegion,
                                tools::Rectangle& rNativeContentRegion) override;

private:
    static int pixelMetric(QStyle::PixelMetric metric, const QStyleOption* option = nullptr);
    static QSize sizeFromContents(QStyle::ContentsType type, const QStyleOption* option,
                                  const QSize& contentsSize);
    static QRect subControlRect(QStyle::ComplexControl control, const QStyleOptionComplex* option,
                                QStyle::SubControl subControl);
    static QRect subElementRect(QStyle::SubElement element, const QStyleOption* option);

    void draw(QStyle::ControlElement element, QStyleOption& rOption, QImage* image,
              const Color& rBackgroundColor, QStyle::State const state = QStyle::State_None,
              QRect rect = QRect());
    void draw(QStyle::PrimitiveElement element, QStyleOption& rOption, QImage* image,
              const Color& rBackgroundColor, QStyle::State const state = QStyle::State_None,
              QRect rect = QRect());
    void draw(QStyle::ComplexControl element, QStyleOptionComplex& rOption, QImage* image,
              const Color& rBackgroundColor, QStyle::State const state = QStyle::State_None);
    void drawFrame(QStyle::PrimitiveElement element, QImage* image, const Color& rBackGroundColor,
                   QStyle::State const& state, bool bClip = true,
                   QStyle::PixelMetric eLineMetric = QStyle::PM_DefaultFrameWidth);

    static void fillQStyleOptionTab(const ImplControlValue& value, QStyleOptionTab& sot);
    void fullQStyleOptionTabWidgetFrame(QStyleOptionTabWidgetFrame& option, bool bDownscale);

    enum class Round
    {
        Floor,
        Ceil,
    };

    int downscale(int value, Round eRound);
    int upscale(int value, Round eRound);
    QRect downscale(const QRect& rect);
    QRect upscale(const QRect& rect);
    QSize downscale(const QSize& size, Round eRound);
    QSize upscale(const QSize& size, Round eRound);
    QPoint upscale(const QPoint& point, Round eRound);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

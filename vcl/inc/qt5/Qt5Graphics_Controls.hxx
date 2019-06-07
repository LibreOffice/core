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
#include <salgdi.hxx>

#include <memory>

#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QRegion>
#include <QtWidgets/QPushButton>

// Native control support
class VCLPLUG_QT5_PUBLIC Qt5Graphics_Controls
{
    std::unique_ptr<QPushButton> m_focusedButton;
    std::unique_ptr<QImage> m_image;
    QRect m_lastPopupRect;

    void initStyles();

public:
    Qt5Graphics_Controls();

    QImage& getImage() { return *m_image; }

    static bool IsNativeControlSupported(ControlType nType, ControlPart nPart);
    static bool hitTestNativeControl(ControlType nType, ControlPart nPart,
                                     const tools::Rectangle& rControlRegion, const Point& aPos,
                                     bool& rIsInside);
    bool drawNativeControl(ControlType nType, ControlPart nPart,
                           const tools::Rectangle& rControlRegion, ControlState nState,
                           const ImplControlValue& aValue, const OUString& aCaption);
    static bool getNativeControlRegion(ControlType nType, ControlPart nPart,
                                       const tools::Rectangle& rControlRegion, ControlState nState,
                                       const ImplControlValue& aValue, const OUString& aCaption,
                                       tools::Rectangle& rNativeBoundingRegion,
                                       tools::Rectangle& rNativeContentRegion);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

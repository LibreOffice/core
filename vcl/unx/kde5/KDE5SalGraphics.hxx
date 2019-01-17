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

#include <memory>

#include <rtl/string.hxx>
#include <headless/svpgdi.hxx>

#include <Qt5Graphics_Controls.hxx>

#include <QtGui/QImage>
#include <QtWidgets/QPushButton>

class Qt5Frame;

/**
 * Handles native graphics requests and performs the needed drawing operations.
 */
class KDE5SalGraphics : public SvpSalGraphics
{
public:
    KDE5SalGraphics(Qt5Frame* pFrame);
    virtual bool IsNativeControlSupported(ControlType, ControlPart) override;

    virtual bool hitTestNativeControl(ControlType, ControlPart, const tools::Rectangle&,
                                      const Point&, bool&) override;

    virtual bool drawNativeControl(ControlType, ControlPart, const tools::Rectangle&, ControlState,
                                   const ImplControlValue&, const OUString&) override;

    virtual bool getNativeControlRegion(ControlType, ControlPart, const tools::Rectangle&,
                                        ControlState, const ImplControlValue&, const OUString&,
                                        tools::Rectangle&, tools::Rectangle&) override;

    virtual void GetResolution(sal_Int32& rDPIX, sal_Int32& rDPIY) override;

private:
    Qt5Graphics_Controls m_aControl;
    Qt5Frame* m_pFrame;
};

inline bool KDE5SalGraphics::IsNativeControlSupported(ControlType nType, ControlPart nPart)
{
    return Qt5Graphics_Controls::IsNativeControlSupported(nType, nPart);
}

inline bool KDE5SalGraphics::hitTestNativeControl(ControlType nType, ControlPart nPart,
                                                  const tools::Rectangle& rControlRegion,
                                                  const Point& aPos, bool& rIsInside)
{
    return Qt5Graphics_Controls::hitTestNativeControl(nType, nPart, rControlRegion, aPos,
                                                      rIsInside);
}

inline bool KDE5SalGraphics::getNativeControlRegion(
    ControlType nType, ControlPart nPart, const tools::Rectangle& rControlRegion,
    ControlState nState, const ImplControlValue& aValue, const OUString& aCaption,
    tools::Rectangle& rNativeBoundingRegion, tools::Rectangle& rNativeContentRegion)
{
    return Qt5Graphics_Controls::getNativeControlRegion(nType, nPart, rControlRegion, nState,
                                                        aValue, aCaption, rNativeBoundingRegion,
                                                        rNativeContentRegion);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#include <unx/saldisp.hxx>
#include <unx/salgdi.h>

#include <QtGui/QImage>

/**
 * Handles native graphics requests and performs the needed drawing operations.
 */
class KDESalGraphics : public X11SalGraphics
{
public:
    virtual bool IsNativeControlSupported( ControlType, ControlPart ) override;

    virtual bool hitTestNativeControl( ControlType, ControlPart,
                                       const tools::Rectangle&, const Point&, bool& ) override;

    virtual bool drawNativeControl( ControlType, ControlPart, const tools::Rectangle&,
                                    ControlState, const ImplControlValue&, const OUString& ) override;

    virtual bool getNativeControlRegion( ControlType, ControlPart, const tools::Rectangle&,
                                         ControlState, const ImplControlValue&,
                                         const OUString&, tools::Rectangle&, tools::Rectangle& ) override;

private:
    std::unique_ptr<QImage> m_image;
    QRect lastPopupRect;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

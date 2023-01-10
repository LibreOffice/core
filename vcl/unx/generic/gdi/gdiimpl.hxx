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

#include <X11/Xlib.h>

#include <unx/x11/x11gdiimpl.h>

#include <salgdiimpl.hxx>

#include <basegfx/polygon/b2dtrapezoid.hxx>
#include <basegfx/polygon/b2dpolygontriangulator.hxx>
#include <ControlCacheKey.hxx>
#include <optional>

/* From <X11/Intrinsic.h> */
typedef unsigned long Pixel;

class SalGraphics;
class SalBitmap;
class SalPolyLine;
class X11SalGraphics;
class Gradient;

class X11SalGraphicsImpl : public SalGraphicsImpl, public X11GraphicsImpl
{
private:
    X11SalGraphics& mrParent;

    tools::Long GetGraphicsHeight() const;

public:

    explicit X11SalGraphicsImpl(X11SalGraphics& rParent);

    virtual ~X11SalGraphicsImpl() override;

    virtual void setClipRegion( const vcl::Region& ) override;
    //
    // get the depth of the device
    virtual sal_uInt16 GetBitCount() const override;

    // get the width of the device
    virtual tools::Long GetGraphicsWidth() const override;

    // set the clip region to empty
    virtual void ResetClipRegion() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

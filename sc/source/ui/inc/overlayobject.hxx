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

#include <svx/sdr/overlay/overlayobject.hxx>
#include <vcl/font.hxx>
#include <vcl/mapmod.hxx>

class ScOverlayDashedBorder : public sdr::overlay::OverlayObject
{
public:
    ScOverlayDashedBorder(const ::basegfx::B2DRange& rRange, const Color& rColor);
    virtual ~ScOverlayDashedBorder() override;

    virtual void Trigger(sal_uInt32 nTime) override;

    virtual void stripeDefinitionHasChanged() override;

protected:
    virtual drawinglayer::primitive2d::Primitive2DContainer createOverlayObjectPrimitive2DSequence() override;

private:
    ::basegfx::B2DRange maRange;
    bool mbToggle;
};

class ScOverlayHint : public sdr::overlay::OverlayObject
{
public:
    ScOverlayHint(const OUString& rTit, const OUString& rMsg, const Color& rColor, const vcl::Font& rFont);
    Size GetSizePixel() const;
    void SetPos(const Point& rPos, const MapMode& rMode);

public:
    virtual drawinglayer::primitive2d::Primitive2DContainer createOverlayObjectPrimitive2DSequence() override;

private:
    drawinglayer::primitive2d::Primitive2DContainer createOverlaySequence(sal_Int32 nLeft, sal_Int32 nTop, const MapMode &rMapMode, basegfx::B2DRange &rRange) const;

    const OUString m_aTitle;
    const OUString m_aMessage;
    const vcl::Font m_aTextFont;
    MapMode m_aMapMode;
    sal_Int32 m_nLeft;
    sal_Int32 m_nTop;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

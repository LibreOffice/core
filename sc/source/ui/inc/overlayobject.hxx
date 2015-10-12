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

#ifndef INCLUDED_SC_SOURCE_UI_INC_OVERLAYOBJECT_HXX
#define INCLUDED_SC_SOURCE_UI_INC_OVERLAYOBJECT_HXX

#include <svx/sdr/overlay/overlayobject.hxx>

class ScOverlayDashedBorder : public sdr::overlay::OverlayObject
{
public:
    ScOverlayDashedBorder(const ::basegfx::B2DRange& rRange, const Color& rColor);
    virtual ~ScOverlayDashedBorder();

    virtual void Trigger(sal_uInt32 nTime) override;

    virtual void stripeDefinitionHasChanged() override;

protected:
    virtual drawinglayer::primitive2d::Primitive2DSequence createOverlayObjectPrimitive2DSequence() override;

private:
    ::basegfx::B2DRange maRange;
    bool mbToggle;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __SC_OVERLAYOBJECT_HXX__
#define __SC_OVERLAYOBJECT_HXX__

#include "svx/sdr/overlay/overlayobject.hxx"

class ScOverlayDashedBorder : public ::sdr::overlay::OverlayObject
{
public:
    ScOverlayDashedBorder(const ::basegfx::B2DRange& rRange, const Color& rColor);
    virtual ~ScOverlayDashedBorder();

    virtual void Trigger(sal_uInt32 nTime);

    virtual void stripeDefinitionHasChanged();

protected:
    virtual drawinglayer::primitive2d::Primitive2DSequence createOverlayObjectPrimitive2DSequence();

private:
    ::basegfx::B2DRange maRange;
    bool mbToggle;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

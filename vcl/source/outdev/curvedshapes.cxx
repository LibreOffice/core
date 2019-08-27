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

#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/drawables/EllipseDrawable.hxx>
#include <vcl/drawables/ArcDrawable.hxx>
#include <vcl/drawables/PieDrawable.hxx>
#include <vcl/drawables/ChordDrawable.hxx>

#include <salgdi.hxx>

#include <cassert>

void OutputDevice::DrawEllipse( const tools::Rectangle& rRect )
{
    Draw(vcl::EllipseDrawable(rRect));
}

void OutputDevice::DrawArc( const tools::Rectangle& rRect,
                            const Point& rStartPt, const Point& rEndPt )
{
    Draw(vcl::ArcDrawable(rRect, rStartPt, rEndPt));
}

void OutputDevice::DrawPie( const tools::Rectangle& rRect,
                            const Point& rStartPt, const Point& rEndPt )
{
    Draw(vcl::PieDrawable(rRect, rStartPt, rEndPt));
}

void OutputDevice::DrawChord( const tools::Rectangle& rRect,
                              const Point& rStartPt, const Point& rEndPt )
{
    Draw(vcl::ChordDrawable(rRect, rStartPt, rEndPt));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <Qt5Tools.hxx>

#include <vcl/event.hxx>

#if !(defined MACOSX || defined _WIN32)
#include <cairo.h>

void CairoDeleter::operator()(cairo_surface_t* pSurface) const { cairo_surface_destroy(pSurface); }
#endif

sal_uInt16 GetKeyModCode(Qt::KeyboardModifiers eKeyModifiers)
{
    sal_uInt16 nCode = 0;
    if (eKeyModifiers & Qt::ShiftModifier)
        nCode |= KEY_SHIFT;
    if (eKeyModifiers & Qt::ControlModifier)
        nCode |= KEY_MOD1;
    if (eKeyModifiers & Qt::AltModifier)
        nCode |= KEY_MOD2;
    if (eKeyModifiers & Qt::MetaModifier)
        nCode |= KEY_MOD3;
    return nCode;
}

sal_uInt16 GetMouseModCode(Qt::MouseButtons eButtons)
{
    sal_uInt16 nCode = 0;
    if (eButtons & Qt::LeftButton)
        nCode |= MOUSE_LEFT;
    if (eButtons & Qt::MidButton)
        nCode |= MOUSE_MIDDLE;
    if (eButtons & Qt::RightButton)
        nCode |= MOUSE_RIGHT;
    return nCode;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#ifndef INCLUDED_SVX_XHATCH_HXX
#define INCLUDED_SVX_XHATCH_HXX

#include <tools/color.hxx>
#include <tools/degree.hxx>
#include <tools/long.hxx>
#include <svx/svxdllapi.h>
#include <com/sun/star/drawing/HatchStyle.hpp>




class SVXCORE_DLLPUBLIC XHatch final
{
    css::drawing::HatchStyle     eStyle;
    Color           aColor;
    tools::Long     nDistance;
    Degree10        nAngle;

public:
                    XHatch() : eStyle(css::drawing::HatchStyle_SINGLE), nDistance(0), nAngle(0) {}
                    XHatch(const Color& rCol, css::drawing::HatchStyle eStyle = css::drawing::HatchStyle_SINGLE,
                           tools::Long nDistance = 20, Degree10 nAngle = Degree10(0));

    bool operator==(const XHatch& rHatch) const;

    void            SetHatchStyle(css::drawing::HatchStyle eNewStyle) { eStyle = eNewStyle; }
    void            SetColor(const Color& rColor) { aColor = rColor; }
    void            SetDistance(tools::Long nNewDistance) { nDistance = nNewDistance; }
    void            SetAngle(Degree10 nNewAngle) { nAngle = nNewAngle; }

    css::drawing::HatchStyle     GetHatchStyle() const { return eStyle; }
    const Color&    GetColor() const { return aColor; }
    tools::Long     GetDistance() const { return nDistance; }
    Degree10        GetAngle() const { return nAngle; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

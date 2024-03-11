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
    css::drawing::HatchStyle     m_eStyle;
    Color           m_aColor;
    tools::Long     m_nDistance;
    Degree10        m_nAngle;

public:
                    XHatch() : m_eStyle(css::drawing::HatchStyle_SINGLE), m_nDistance(0), m_nAngle(0) {}
                    XHatch(const Color& rCol, css::drawing::HatchStyle eStyle = css::drawing::HatchStyle_SINGLE,
                           tools::Long nDistance = 20, Degree10 nAngle = 0_deg10);

    bool operator==(const XHatch& rHatch) const;

    void            SetHatchStyle(css::drawing::HatchStyle eNewStyle) { m_eStyle = eNewStyle; }
    void            SetColor(const Color& rColor) { m_aColor = rColor; }
    void            SetDistance(tools::Long nNewDistance) { m_nDistance = nNewDistance; }
    void            SetAngle(Degree10 nNewAngle) { m_nAngle = nNewAngle; }

    css::drawing::HatchStyle     GetHatchStyle() const { return m_eStyle; }
    const Color&    GetColor() const { return m_aColor; }
    tools::Long     GetDistance() const { return m_nDistance; }
    Degree10        GetAngle() const { return m_nAngle; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

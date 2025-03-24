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

#include <iostream>

#include <vcl/dllapi.h>
#include <vcl/WindowPosSize.hxx>
#include <tools/long.hxx>

// There are some unused functions, which I would keep to ease understanding.
class SalFrameGeometry : public vcl::WindowPosSize
{
    // non-drawable area / margins / frame / decorations around the client area
    sal_uInt32 m_nLeftDecoration, m_nTopDecoration, m_nRightDecoration, m_nBottomDecoration;
    unsigned int m_nDisplayScreenNumber;

public:
    SalFrameGeometry()
        : m_nLeftDecoration(0)
        , m_nTopDecoration(0)
        , m_nRightDecoration(0)
        , m_nBottomDecoration(0)
        , m_nDisplayScreenNumber(0)
    {
    }

    constexpr sal_uInt32 leftDecoration() const { return m_nLeftDecoration; }
    void setLeftDecoration(sal_uInt32 nLeftDecoration) { m_nLeftDecoration = nLeftDecoration; }
    constexpr sal_uInt32 topDecoration() const { return m_nTopDecoration; }
    void setTopDecoration(sal_uInt32 nTopDecoration) { m_nTopDecoration = nTopDecoration; }
    constexpr sal_uInt32 rightDecoration() const { return m_nRightDecoration; }
    void setRightDecoration(sal_uInt32 nRightDecoration) { m_nRightDecoration = nRightDecoration; }
    constexpr sal_uInt32 bottomDecoration() const { return m_nBottomDecoration; }
    void setBottomDecoration(sal_uInt32 nBottomDecoration)
    {
        m_nBottomDecoration = nBottomDecoration;
    }
    void setDecorations(sal_uInt32 nLeft, sal_uInt32 nTop, sal_uInt32 nRight, sal_uInt32 nBottom)
    {
        m_nLeftDecoration = nLeft;
        m_nTopDecoration = nTop;
        m_nRightDecoration = nRight;
        m_nBottomDecoration = nBottom;
    }

    unsigned int screen() const { return m_nDisplayScreenNumber; }
    void setScreen(unsigned int nScreen) { m_nDisplayScreenNumber = nScreen; }
};

inline std::ostream& operator<<(std::ostream& s, const SalFrameGeometry& rGeom)
{
    s << *static_cast<const vcl::WindowPosSize*>(&rGeom) << ":{" << rGeom.leftDecoration() << ","
      << rGeom.topDecoration() << "," << rGeom.rightDecoration() << "," << rGeom.bottomDecoration()
      << "}s" << rGeom.screen();
    return s;
}

/// Interface used to share logic on sizing between
/// SalVirtualDevices and SalFrames
class VCL_PLUGIN_PUBLIC SalGeometryProvider
{
public:
    virtual ~SalGeometryProvider() {}
    virtual tools::Long GetWidth() const = 0;
    virtual tools::Long GetHeight() const = 0;
    virtual bool IsOffScreen() const = 0;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

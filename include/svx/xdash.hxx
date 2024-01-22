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

#ifndef INCLUDED_SVX_XDASH_HXX
#define INCLUDED_SVX_XDASH_HXX




#include <svx/svxdllapi.h>
#include <com/sun/star/drawing/DashStyle.hpp>

#include <vector>

class SVXCORE_DLLPUBLIC XDash final
{
    css::drawing::DashStyle  m_eDash;
    sal_uInt16               m_nDots;
    sal_uInt16               m_nDashes;
    double                   m_nDotLen;
    double                   m_nDashLen;
    double                   m_nDistance;

public:
          XDash(css::drawing::DashStyle eDash = css::drawing::DashStyle_RECT,
                sal_uInt16 nDots = 1, double nDotLen = 20,
                sal_uInt16 nDashes = 1, double nDashLen = 20, double nDistance = 20);

    bool operator==(const XDash& rDash) const;

    void SetDashStyle(css::drawing::DashStyle eNewStyle) { m_eDash = eNewStyle; }
    void SetDots(sal_uInt16 nNewDots)                    { m_nDots = nNewDots; }
    void SetDotLen(double nNewDotLen)                    { m_nDotLen = nNewDotLen; }
    void SetDashes(sal_uInt16 nNewDashes)                { m_nDashes = nNewDashes; }
    void SetDashLen(double nNewDashLen)                  { m_nDashLen = nNewDashLen; }
    void SetDistance(double nNewDistance)                { m_nDistance = nNewDistance; }

    css::drawing::DashStyle  GetDashStyle() const        { return m_eDash; }
    sal_uInt16               GetDots() const             { return m_nDots; }
    double                   GetDotLen() const           { return m_nDotLen; }
    sal_uInt16               GetDashes() const           { return m_nDashes; }
    double                   GetDashLen() const          { return m_nDashLen; }
    double                   GetDistance() const         { return m_nDistance; }

    // XDash is translated into an array of doubles which describe the lengths of the
    // dashes, dots and empty passages. It returns the complete length of the full DashDot
    // sequence and fills the given vector of doubles accordingly (also resizing, so deleting it).
    double CreateDotDashArray(::std::vector< double >& rDotDashArray, double fLineWidth) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

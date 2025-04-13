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

#include <ChartColorScheme.hxx>

#include <cppuhelper/supportsservice.hxx>

namespace chart
{
// explicit
ChartColorScheme::ChartColorScheme(const ChartColorPalette& rPalette)
    : BaseColorScheme()
{
    m_nNumberOfColors = rPalette.size();
    m_aColorSequence.realloc(m_nNumberOfColors);
    sal_Int64* pColors = m_aColorSequence.getArray();
    for (size_t i = 0; i < rPalette.size(); ++i)
    {
        pColors[i] = static_cast<sal_Int32>(rPalette[i]);
    }
}

::sal_Int32 SAL_CALL ChartColorScheme::getColorByIndex(::sal_Int32 nIndex)
{
    assert(m_nNumberOfColors > 0);
    return static_cast<sal_Int32>(m_aColorSequence[nIndex % m_nNumberOfColors]);
}

OUString SAL_CALL ChartColorScheme::getImplementationName()
{
    return "com.sun.star.comp.chart2.ChartColorScheme";
}
} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

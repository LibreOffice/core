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

#ifndef INCLUDED_SVX_XGRAD_HXX
#define INCLUDED_SVX_XGRAD_HXX

#include <tools/color.hxx>
#include <tools/degree.hxx>
#include <svx/svxdllapi.h>
#include <com/sun/star/awt/GradientStyle.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#include <com/sun/star/awt/Gradient.hpp>

class Gradient;

class SAL_WARN_UNUSED SVXCORE_DLLPUBLIC XGradient final
{
    css::awt::GradientStyle  eStyle;
    Color               aStartColor;
    Color               aEndColor;
    Degree10            nAngle;
    sal_uInt16          nBorder;
    sal_uInt16          nOfsX;
    sal_uInt16          nOfsY;
    sal_uInt16          nIntensStart;
    sal_uInt16          nIntensEnd;
    sal_uInt16          nStepCount;

    static std::string GradientStyleToString(css::awt::GradientStyle eStyle);

public:
    XGradient();
    XGradient( const Color& rStart, const Color& rEnd,
               css::awt::GradientStyle eStyle = css::awt::GradientStyle_LINEAR, Degree10 nAngle = 0_deg10,
               sal_uInt16 nXOfs = 50, sal_uInt16 nYOfs = 50, sal_uInt16 nBorder = 0,
               sal_uInt16 nStartIntens = 100, sal_uInt16 nEndIntens = 100,
               sal_uInt16 nSteps = 0 );

    bool operator==(const XGradient& rGradient) const;

    void SetGradientStyle(css::awt::GradientStyle eNewStyle) { eStyle = eNewStyle; }
    void SetStartColor(const Color& rColor)         { aStartColor = rColor; }
    void SetEndColor(const Color& rColor)           { aEndColor = rColor; }
    void SetAngle(Degree10 nNewAngle)                { nAngle = nNewAngle; }
    void SetBorder(sal_uInt16 nNewBorder)               { nBorder = nNewBorder; }
    void SetXOffset(sal_uInt16 nNewOffset)              { nOfsX = nNewOffset; }
    void SetYOffset(sal_uInt16 nNewOffset)              { nOfsY = nNewOffset; }
    void SetStartIntens(sal_uInt16 nNewIntens)          { nIntensStart = nNewIntens; }
    void SetEndIntens(sal_uInt16 nNewIntens)            { nIntensEnd = nNewIntens; }
    void SetSteps(sal_uInt16 nSteps)                    { nStepCount = nSteps; }

    css::awt::GradientStyle GetGradientStyle() const         { return eStyle; }
    const Color&       GetStartColor() const            { return aStartColor; }
    const Color&       GetEndColor() const              { return aEndColor; }
    Degree10           GetAngle() const                 { return nAngle; }
    sal_uInt16         GetBorder() const                { return nBorder; }
    sal_uInt16         GetXOffset() const               { return nOfsX; }
    sal_uInt16         GetYOffset() const               { return nOfsY; }
    sal_uInt16         GetStartIntens() const           { return nIntensStart; }
    sal_uInt16         GetEndIntens() const             { return nIntensEnd; }
    sal_uInt16         GetSteps() const                 { return nStepCount; }

    boost::property_tree::ptree dumpAsJSON() const;
    static XGradient fromJSON(const OUString& rJSON);
    css::awt::Gradient toGradientUNO();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

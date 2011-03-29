/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

#ifndef _SVX__XGRADIENT_HXX
#define _SVX__XGRADIENT_HXX

#include <svx/xenum.hxx>
#include <tools/color.hxx>
#include "svx/svxdllapi.h"

//-----------------
// class XGradient
//-----------------

class SVX_DLLPUBLIC XGradient
{
protected:
    XGradientStyle  eStyle;
    Color           aStartColor;
    Color           aEndColor;
    long            nAngle;
    sal_uInt16          nBorder;
    sal_uInt16          nOfsX;
    sal_uInt16          nOfsY;
    sal_uInt16          nIntensStart;
    sal_uInt16          nIntensEnd;
    sal_uInt16          nStepCount;

public:
    XGradient();
    XGradient( const Color& rStart, const Color& rEnd,
               XGradientStyle eStyle = XGRAD_LINEAR, long nAngle = 0,
               sal_uInt16 nXOfs = 50, sal_uInt16 nYOfs = 50, sal_uInt16 nBorder = 0,
               sal_uInt16 nStartIntens = 100, sal_uInt16 nEndIntens = 100,
               sal_uInt16 nSteps = 0 );

    bool operator==(const XGradient& rGradient) const;

    void SetGradientStyle(XGradientStyle eNewStyle) { eStyle = eNewStyle; }
    void SetStartColor(const Color& rColor)         { aStartColor = rColor; }
    void SetEndColor(const Color& rColor)           { aEndColor = rColor; }
    void SetAngle(long nNewAngle)                   { nAngle = nNewAngle; }
    void SetBorder(sal_uInt16 nNewBorder)               { nBorder = nNewBorder; }
    void SetXOffset(sal_uInt16 nNewOffset)              { nOfsX = nNewOffset; }
    void SetYOffset(sal_uInt16 nNewOffset)              { nOfsY = nNewOffset; }
    void SetStartIntens(sal_uInt16 nNewIntens)          { nIntensStart = nNewIntens; }
    void SetEndIntens(sal_uInt16 nNewIntens)            { nIntensEnd = nNewIntens; }
    void SetSteps(sal_uInt16 nSteps)                    { nStepCount = nSteps; }

    XGradientStyle GetGradientStyle() const         { return eStyle; }
    Color          GetStartColor() const            { return aStartColor; }
    Color          GetEndColor() const              { return aEndColor; }
    long           GetAngle() const                 { return nAngle; }
    sal_uInt16         GetBorder() const                { return nBorder; }
    sal_uInt16         GetXOffset() const               { return nOfsX; }
    sal_uInt16         GetYOffset() const               { return nOfsY; }
    sal_uInt16         GetStartIntens() const           { return nIntensStart; }
    sal_uInt16         GetEndIntens() const             { return nIntensEnd; }
    sal_uInt16         GetSteps() const                 { return nStepCount; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

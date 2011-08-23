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

#include <bf_svx/xenum.hxx>

#include <tools/color.hxx>
namespace binfilter {

//-----------------
// class XGradient
//-----------------

class XGradient
{
protected:
    XGradientStyle  eStyle;
    Color           aStartColor;
    Color           aEndColor;
    long            nAngle;
    USHORT          nBorder;
    USHORT          nOfsX;
    USHORT          nOfsY;
    USHORT          nIntensStart;
    USHORT          nIntensEnd;
    USHORT          nStepCount;

public:
    XGradient() {}
    XGradient( const Color& rStart, const Color& rEnd,
               XGradientStyle eStyle = XGRAD_LINEAR, long nAngle = 0,
               USHORT nXOfs = 50, USHORT nYOfs = 50, USHORT nBorder = 0,
               USHORT nStartIntens = 100, USHORT nEndIntens = 100,
               USHORT nSteps = 0 );

    int operator==(const XGradient& rGradient) const;

    void SetGradientStyle(XGradientStyle eNewStyle) { eStyle = eNewStyle; }
    void SetStartColor(const Color& rColor)         { aStartColor = rColor; }
    void SetEndColor(const Color& rColor)           { aEndColor = rColor; }
    void SetAngle(long nNewAngle)                   { nAngle = nNewAngle; }
    void SetBorder(USHORT nNewBorder)               { nBorder = nNewBorder; }
    void SetXOffset(USHORT nNewOffset)              { nOfsX = nNewOffset; }
    void SetYOffset(USHORT nNewOffset)              { nOfsY = nNewOffset; }
    void SetStartIntens(USHORT nNewIntens)          { nIntensStart = nNewIntens; }
    void SetEndIntens(USHORT nNewIntens)            { nIntensEnd = nNewIntens; }
    void SetSteps(USHORT nSteps)                    { nStepCount = nSteps; }

    XGradientStyle GetGradientStyle() const         { return eStyle; }
    Color          GetStartColor() const            { return aStartColor; }
    Color          GetEndColor() const              { return aEndColor; }
    long           GetAngle() const                 { return nAngle; }
    USHORT         GetBorder() const                { return nBorder; }
    USHORT         GetXOffset() const               { return nOfsX; }
    USHORT         GetYOffset() const               { return nOfsY; }
    USHORT         GetStartIntens() const           { return nIntensStart; }
    USHORT         GetEndIntens() const             { return nIntensEnd; }
    USHORT         GetSteps() const                 { return nStepCount; }
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

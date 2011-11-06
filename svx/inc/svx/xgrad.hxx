/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

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



#ifndef _SVX_XHATCH_HXX
#define _SVX_XHATCH_HXX

#include <svx/xenum.hxx>
#include <tools/color.hxx>
#include "svx/svxdllapi.h"

//--------------
// class XHatch
//--------------

class SVX_DLLPUBLIC XHatch
{
protected:
    XHatchStyle     eStyle;
    Color           aColor;
    long            nDistance;
    long            nAngle;

public:
                    XHatch() : eStyle(XHATCH_SINGLE), nDistance(0), nAngle(0) {}
                    XHatch(const Color& rCol, XHatchStyle eStyle = XHATCH_SINGLE,
                           long nDistance = 20, long nAngle = 0);

    bool operator==(const XHatch& rHatch) const;

    void            SetHatchStyle(XHatchStyle eNewStyle) { eStyle = eNewStyle; }
    void            SetColor(const Color& rColor) { aColor = rColor; }
    void            SetDistance(long nNewDistance) { nDistance = nNewDistance; }
    void            SetAngle(long nNewAngle) { nAngle = nNewAngle; }

    XHatchStyle     GetHatchStyle() const { return eStyle; }
    Color           GetColor() const { return aColor; }
    long            GetDistance() const { return nDistance; }
    long            GetAngle() const { return nAngle; }
};

#endif

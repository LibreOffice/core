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



#ifndef _SVDHLPLN_HXX
#define _SVDHLPLN_HXX

#include <sal/types.h>
#include <tools/color.hxx>
#include <tools/gen.hxx>
#include <vcl/pointr.hxx>
#include <tools/contnr.hxx>
#include "svx/svxdllapi.h"
#include <basegfx/point/b2dpoint.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
// defines

enum SdrHelpLineKind
{
    SDRHELPLINE_POINT,
    SDRHELPLINE_VERTICAL,
    SDRHELPLINE_HORIZONTAL
};

#define SDRHELPLINE_NOTFOUND 0xffffffff

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrHelpLine
{
private:
    basegfx::B2DPoint   maPos; // je nach Wert von eKind ist X oder Y evtl. belanglos
    SdrHelpLineKind     meKind;

public:
    SdrHelpLine(SdrHelpLineKind eNewKind = SDRHELPLINE_POINT);
    SdrHelpLine(SdrHelpLineKind eNewKind, const basegfx::B2DPoint& rNewPos);

    bool operator==(const SdrHelpLine& rCmp) const;
    bool operator!=(const SdrHelpLine& rCmp) const { return !operator==(rCmp); }

    void SetKind(SdrHelpLineKind eNewKind) { if(meKind != eNewKind) meKind = eNewKind; }
    SdrHelpLineKind GetKind() const { return meKind; }

    void SetPos(const basegfx::B2DPoint& rPnt) { if(maPos != rPnt) maPos = rPnt; }
    const basegfx::B2DPoint& GetPos() const { return maPos; }

    Pointer         GetPointer() const;
    bool IsHit(const basegfx::B2DPoint& rPnt, double fTolLog) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrHelpLineList
{
private:
    typedef ::std::vector< SdrHelpLine* > SdrHelpLineContainerType;
    SdrHelpLineContainerType    maList;

protected:
    SdrHelpLine* GetObject(sal_uInt32 i) const;

public:
    SdrHelpLineList();
    SdrHelpLineList(const SdrHelpLineList& rSrcList);
    ~SdrHelpLineList();

    void               Clear();
    void               operator=(const SdrHelpLineList& rSrcList);
    bool operator==(const SdrHelpLineList& rCmp) const;
    bool operator!=(const SdrHelpLineList& rCmp) const                 { return !operator==(rCmp); }

    sal_uInt32 GetCount() const { return maList.size(); }

    void Insert(const SdrHelpLine& rHL, sal_uInt32 nPos = 0xffffffff);
    void Delete(sal_uInt32 nPos);

    SdrHelpLine& operator[](sal_uInt32 nPos) { return *GetObject(nPos); }
    const SdrHelpLine& operator[](sal_uInt32 nPos) const { return *GetObject(nPos); }

    sal_uInt32 HLHitTest(const basegfx::B2DPoint& rPnt, double fTolLog) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDHLPLN_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof

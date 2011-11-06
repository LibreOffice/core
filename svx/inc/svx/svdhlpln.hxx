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

#ifndef _POINTR_HXX //autogen
#include <vcl/pointr.hxx>
#endif
#include <tools/contnr.hxx>
#include "svx/svxdllapi.h"

class OutputDevice;
////////////////////////////////////////////////////////////////////////////////////////////////////

enum SdrHelpLineKind {SDRHELPLINE_POINT,SDRHELPLINE_VERTICAL,SDRHELPLINE_HORIZONTAL};
#define SDRHELPLINE_MIN SDRHELPLINE_POINT
#define SDRHELPLINE_MAX SDRHELPLINE_HORIZONTAL

#define SDRHELPLINE_POINT_PIXELSIZE 15 /* Tatsaechliche Groesse= PIXELSIZE*2+1 */

class SdrHelpLine {
    Point            aPos; // je nach Wert von eKind ist X oder Y evtl. belanglos
    SdrHelpLineKind  eKind;

    // #i27493#
    // Helper method to draw a hor or ver two-colored dashed line
    void ImpDrawDashedTwoColorLine(OutputDevice& rOut, sal_Int32 nStart, sal_Int32 nEnd, sal_Int32 nFixPos,
        sal_Int32 nStepWidth, Color aColA, Color aColB, sal_Bool bHorizontal) const;

public:
    SdrHelpLine(SdrHelpLineKind eNewKind=SDRHELPLINE_POINT): eKind(eNewKind) {}
    SdrHelpLine(SdrHelpLineKind eNewKind, const Point& rNewPos): aPos(rNewPos), eKind(eNewKind) {}
    bool operator==(const SdrHelpLine& rCmp) const { return aPos==rCmp.aPos && eKind==rCmp.eKind; }
    bool operator!=(const SdrHelpLine& rCmp) const { return !operator==(rCmp); }

    void            SetKind(SdrHelpLineKind eNewKind) { eKind=eNewKind; }
    SdrHelpLineKind GetKind() const                   { return eKind; }
    void            SetPos(const Point& rPnt)         { aPos=rPnt; }
    const Point&    GetPos() const                    { return aPos; }

    Pointer         GetPointer() const;
    FASTBOOL        IsHit(const Point& rPnt, sal_uInt16 nTolLog, const OutputDevice& rOut) const;
    // OutputDevice wird benoetigt, da Fangpunkte eine feste Pixelgroesse haben
    Rectangle       GetBoundRect(const OutputDevice& rOut) const;

    /* returns true if this and the given help line would be rendered at the same pixel position
        of the given OutputDevice. This can be used to avoid drawing multiple help lines with xor
        on same position which could render them invisible */
    bool            IsVisibleEqual( const SdrHelpLine& rHelpLine, const OutputDevice& rOut ) const;
};

#define SDRHELPLINE_NOTFOUND 0xFFFF

class SVX_DLLPUBLIC SdrHelpLineList {
    Container aList;
protected:
    SdrHelpLine* GetObject(sal_uInt16 i) const { return (SdrHelpLine*)(aList.GetObject(i)); }
public:
    SdrHelpLineList(): aList(1024,4,4) {}
    SdrHelpLineList(const SdrHelpLineList& rSrcList): aList(1024,4,4)      { *this=rSrcList; }
    ~SdrHelpLineList()                                                     { Clear(); }
    void               Clear();
    void               operator=(const SdrHelpLineList& rSrcList);
    bool operator==(const SdrHelpLineList& rCmp) const;
    bool operator!=(const SdrHelpLineList& rCmp) const                 { return !operator==(rCmp); }
    sal_uInt16             GetCount() const                                    { return sal_uInt16(aList.Count()); }
    void               Insert(const SdrHelpLine& rHL, sal_uInt16 nPos=0xFFFF)  { aList.Insert(new SdrHelpLine(rHL),nPos); }
    void               Delete(sal_uInt16 nPos)                                 { delete (SdrHelpLine*)aList.Remove(nPos); } // #i24900#
    void               Move(sal_uInt16 nPos, sal_uInt16 nNewPos)                   { aList.Insert(aList.Remove(nPos),nNewPos); }
    SdrHelpLine&       operator[](sal_uInt16 nPos)                             { return *GetObject(nPos); }
    const SdrHelpLine& operator[](sal_uInt16 nPos) const                       { return *GetObject(nPos); }
    sal_uInt16             HitTest(const Point& rPnt, sal_uInt16 nTolLog, const OutputDevice& rOut) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDHLPLN_HXX


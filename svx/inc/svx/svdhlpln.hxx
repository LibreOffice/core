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


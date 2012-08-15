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

#ifndef _SVDHLPLN_HXX
#define _SVDHLPLN_HXX

#include <sal/types.h>
#include <tools/color.hxx>
#include <tools/gen.hxx>

#include <vcl/pointr.hxx>
#include "svx/svxdllapi.h"

class OutputDevice;
////////////////////////////////////////////////////////////////////////////////////////////////////

enum SdrHelpLineKind {SDRHELPLINE_POINT,SDRHELPLINE_VERTICAL,SDRHELPLINE_HORIZONTAL};

#define SDRHELPLINE_POINT_PIXELSIZE 15 /* Tatsaechliche Groesse= PIXELSIZE*2+1 */

class SdrHelpLine {
    Point            aPos; // je nach Wert von eKind ist X oder Y evtl. belanglos
    SdrHelpLineKind  eKind;

    // #i27493#
    // Helper method to draw a hor or ver two-colored dashed line
    void ImpDrawDashedTwoColorLine(OutputDevice& rOut, sal_Int32 nStart, sal_Int32 nEnd, sal_Int32 nFixPos,
        sal_Int32 nStepWidth, Color aColA, Color aColB, sal_Bool bHorizontal) const;

public:
    explicit SdrHelpLine(SdrHelpLineKind eNewKind=SDRHELPLINE_POINT): eKind(eNewKind) {}
    SdrHelpLine(SdrHelpLineKind eNewKind, const Point& rNewPos): aPos(rNewPos), eKind(eNewKind) {}
    bool operator==(const SdrHelpLine& rCmp) const { return aPos==rCmp.aPos && eKind==rCmp.eKind; }
    bool operator!=(const SdrHelpLine& rCmp) const { return !operator==(rCmp); }

    void            SetKind(SdrHelpLineKind eNewKind) { eKind=eNewKind; }
    SdrHelpLineKind GetKind() const                   { return eKind; }
    void            SetPos(const Point& rPnt)         { aPos=rPnt; }
    const Point&    GetPos() const                    { return aPos; }

    Pointer         GetPointer() const;
    bool            IsHit(const Point& rPnt, sal_uInt16 nTolLog, const OutputDevice& rOut) const;
    // OutputDevice wird benoetigt, da Fangpunkte eine feste Pixelgroesse haben
    Rectangle       GetBoundRect(const OutputDevice& rOut) const;
};

#define SDRHELPLINE_NOTFOUND 0xFFFF

class SVX_DLLPUBLIC SdrHelpLineList {
    std::vector<SdrHelpLine*> aList;
protected:
    SdrHelpLine* GetObject(sal_uInt16 i) const { return aList[i]; }
public:
    SdrHelpLineList(): aList() {}
    SdrHelpLineList(const SdrHelpLineList& rSrcList): aList()      { *this=rSrcList; }
    ~SdrHelpLineList()                                                     { Clear(); }
    void               Clear();
    void               operator=(const SdrHelpLineList& rSrcList);
    bool operator==(const SdrHelpLineList& rCmp) const;
    bool operator!=(const SdrHelpLineList& rCmp) const                 { return !operator==(rCmp); }
    sal_uInt16             GetCount() const                                    { return sal_uInt16(aList.size()); }
    void               Insert(const SdrHelpLine& rHL)                          { aList.push_back(new SdrHelpLine(rHL)); }
    void               Insert(const SdrHelpLine& rHL, sal_uInt16 nPos)
    {
        if(nPos==0xFFFF)
            aList.push_back(new SdrHelpLine(rHL));
        else
            aList.insert(aList.begin() + nPos, new SdrHelpLine(rHL));
    }
    void               Delete(sal_uInt16 nPos)
    {
        SdrHelpLine* p = aList[nPos];
        delete p;
        aList.erase(aList.begin() + nPos);
    }
    void               Move(sal_uInt16 nPos, sal_uInt16 nNewPos)
    {
        SdrHelpLine* p = aList[nPos];
        aList.erase(aList.begin() + nPos);
        aList.insert(aList.begin() + nNewPos, p);
    }
    SdrHelpLine&       operator[](sal_uInt16 nPos)                             { return *GetObject(nPos); }
    const SdrHelpLine& operator[](sal_uInt16 nPos) const                       { return *GetObject(nPos); }
    sal_uInt16             HitTest(const Point& rPnt, sal_uInt16 nTolLog, const OutputDevice& rOut) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDHLPLN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

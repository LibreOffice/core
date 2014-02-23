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

#ifndef INCLUDED_SVX_SVDHLPLN_HXX
#define INCLUDED_SVX_SVDHLPLN_HXX

#include <sal/types.h>
#include <tools/color.hxx>
#include <tools/gen.hxx>

#include <vcl/pointr.hxx>
#include <svx/svxdllapi.h>

class OutputDevice;


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



#endif // INCLUDED_SVX_SVDHLPLN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

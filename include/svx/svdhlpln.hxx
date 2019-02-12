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
#include <tools/gen.hxx>

#include <svx/svxdllapi.h>

#include <vector>
#include <memory>

class OutputDevice;
enum class PointerStyle;

enum class SdrHelpLineKind { Point, Vertical, Horizontal };

#define SDRHELPLINE_POINT_PIXELSIZE 15 /* actual size = PIXELSIZE*2+1 */

class SdrHelpLine {
    Point            aPos; // X or Y may be unimportant, depending on the value of eKind
    SdrHelpLineKind  eKind;

public:
    explicit SdrHelpLine(SdrHelpLineKind eNewKind=SdrHelpLineKind::Point): eKind(eNewKind) {}
    SdrHelpLine(SdrHelpLineKind eNewKind, const Point& rNewPos): aPos(rNewPos), eKind(eNewKind) {}
    bool operator==(const SdrHelpLine& rCmp) const { return aPos==rCmp.aPos && eKind==rCmp.eKind; }
    bool operator!=(const SdrHelpLine& rCmp) const { return !operator==(rCmp); }

    void            SetKind(SdrHelpLineKind eNewKind) { eKind=eNewKind; }
    SdrHelpLineKind GetKind() const                   { return eKind; }
    void            SetPos(const Point& rPnt)         { aPos=rPnt; }
    const Point&    GetPos() const                    { return aPos; }

    PointerStyle    GetPointer() const;
    bool            IsHit(const Point& rPnt, sal_uInt16 nTolLog, const OutputDevice& rOut) const;
    // OutputDevice is required because capture points have a fixed pixel size
    tools::Rectangle       GetBoundRect(const OutputDevice& rOut) const;
};

#define SDRHELPLINE_NOTFOUND 0xFFFF

class SVX_DLLPUBLIC SdrHelpLineList {
    std::vector<std::unique_ptr<SdrHelpLine>> aList;

public:
    SdrHelpLineList() {}
    SdrHelpLineList(const SdrHelpLineList& rSrcList) { *this=rSrcList; }
    void               Clear();
    SdrHelpLineList&   operator=(const SdrHelpLineList& rSrcList);
    bool operator==(const SdrHelpLineList& rCmp) const;
    bool operator!=(const SdrHelpLineList& rCmp) const                 { return !operator==(rCmp); }
    sal_uInt16         GetCount() const                                    { return sal_uInt16(aList.size()); }
    void               Insert(const SdrHelpLine& rHL)                          { aList.emplace_back(new SdrHelpLine(rHL)); }
    void               Insert(const SdrHelpLine& rHL, sal_uInt16 nPos)
    {
        if(nPos==0xFFFF)
            aList.emplace_back(new SdrHelpLine(rHL));
        else
            aList.emplace(aList.begin() + nPos, new SdrHelpLine(rHL));
    }
    void               Delete(sal_uInt16 nPos)
    {
        aList.erase(aList.begin() + nPos);
    }
    SdrHelpLine&       operator[](sal_uInt16 nPos)                             { return *aList[nPos]; }
    const SdrHelpLine& operator[](sal_uInt16 nPos) const                       { return *aList[nPos]; }
    sal_uInt16             HitTest(const Point& rPnt, sal_uInt16 nTolLog, const OutputDevice& rOut) const;
};


#endif // INCLUDED_SVX_SVDHLPLN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

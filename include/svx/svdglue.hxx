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

#ifndef INCLUDED_SVX_SVDGLUE_HXX
#define INCLUDED_SVX_SVDGLUE_HXX

namespace vcl { class Window; }
class OutputDevice;
class SvStream;
class SdrObject;

#include <tools/gen.hxx>
#include <svx/svxdllapi.h>
#include <vector>
#include <o3tl/typed_flags_set.hxx>


enum class SdrEscapeDirection
{
    SMART  = 0x0000,
    LEFT   = 0x0001,
    RIGHT  = 0x0002,
    TOP    = 0x0004,
    BOTTOM = 0x0008,
    HORZ   = LEFT | RIGHT,
    VERT   = TOP | BOTTOM,
    ALL    = 0x00ff,
};
namespace o3tl
{
    template<> struct typed_flags<SdrEscapeDirection> : is_typed_flags<SdrEscapeDirection, 0x00ff> {};
}

enum class SdrAlign
{
    NONE          = 0x0000,
    HORZ_CENTER   = 0x0000,
    HORZ_LEFT     = 0x0001,
    HORZ_RIGHT    = 0x0002,
    HORZ_DONTCARE = 0x0010,
    VERT_CENTER   = 0x0000,
    VERT_TOP      = 0x0100,
    VERT_BOTTOM   = 0x0200,
    VERT_DONTCARE = 0x1000,
};
namespace o3tl
{
    template<> struct typed_flags<SdrAlign> : is_typed_flags<SdrAlign, 0x1313> {};
}

class SVX_DLLPUBLIC SdrGluePoint {
    // Reference Point is SdrObject::GetSnapRect().Center()
    // bNoPercent=false: position is -5000..5000 (1/100)% or 0..10000 (depending on align)
    // bNoPercent=true : position is in log unit, relativ to the reference point
    Point    aPos;
    SdrEscapeDirection nEscDir;
    sal_uInt16   nId;
    SdrAlign     nAlign;
    bool bNoPercent:1;
    bool bReallyAbsolute:1; // temp for transformations on the reference object
    bool bUserDefined:1; // #i38892#
public:
    SdrGluePoint(): nEscDir(SdrEscapeDirection::SMART),nId(0),nAlign(SdrAlign::NONE),bNoPercent(false),bReallyAbsolute(false),bUserDefined(true) {}
    SdrGluePoint(const Point& rNewPos): aPos(rNewPos),nEscDir(SdrEscapeDirection::SMART),nId(0),nAlign(SdrAlign::NONE),bNoPercent(false),bReallyAbsolute(false),bUserDefined(true) {}
    const Point& GetPos() const                             { return aPos; }
    void         SetPos(const Point& rNewPos)               { aPos=rNewPos; }
    SdrEscapeDirection GetEscDir() const                          { return nEscDir; }
    void         SetEscDir(SdrEscapeDirection nNewEsc)                  { nEscDir=nNewEsc; }
    sal_uInt16   GetId() const                              { return nId; }
    void         SetId(sal_uInt16 nNewId)                       { nId=nNewId; }
    bool         IsPercent() const                          { return !bNoPercent; }
    void         SetPercent(bool bOn)                   { bNoPercent  = !bOn; }
    // temp for transformations on the reference object
    void         SetReallyAbsolute(bool bOn, const SdrObject& rObj);

    // #i38892#
    bool         IsUserDefined() const                      { return bUserDefined; }
    void         SetUserDefined(bool bNew)              { bUserDefined = bNew; }

    SdrAlign     GetAlign() const                           { return nAlign; }
    void         SetAlign(SdrAlign nAlg)                    { nAlign=nAlg; }
    SdrAlign     GetHorzAlign() const                       { return nAlign & static_cast<SdrAlign>(0x00FF); }
    void         SetHorzAlign(SdrAlign nAlg)                { assert((nAlg & static_cast<SdrAlign>(0xff)) == SdrAlign::NONE); nAlign = SdrAlign(nAlign & static_cast<SdrAlign>(0xFF00)) | (nAlg & static_cast<SdrAlign>(0x00FF)); }
    SdrAlign     GetVertAlign() const                       { return nAlign & static_cast<SdrAlign>(0xFF00); }
    void         SetVertAlign(SdrAlign nAlg)                { assert((nAlg & static_cast<SdrAlign>(0xff00)) == SdrAlign::NONE); nAlign = SdrAlign(nAlign & static_cast<SdrAlign>(0x00FF)) | (nAlg & static_cast<SdrAlign>(0xFF00)); }
    bool         IsHit(const Point& rPnt, const OutputDevice& rOut, const SdrObject* pObj) const;
    void         Invalidate(vcl::Window& rWin, const SdrObject* pObj) const;
    Point        GetAbsolutePos(const SdrObject& rObj) const;
    void         SetAbsolutePos(const Point& rNewPos, const SdrObject& rObj);
    long         GetAlignAngle() const;
    void         SetAlignAngle(long nAngle);
    static long  EscDirToAngle(SdrEscapeDirection nEsc);
    static SdrEscapeDirection EscAngleToDir(long nAngle);
    void         Rotate(const Point& rRef, long nAngle, double sn, double cs, const SdrObject* pObj);
    void         Mirror(const Point& rRef1, const Point& rRef2, long nAngle, const SdrObject* pObj);
    void         Shear (const Point& rRef, long nAngle, double tn, bool bVShear, const SdrObject* pObj);
};

#define SDRGLUEPOINT_NOTFOUND 0xFFFF

class SVX_DLLPUBLIC SdrGluePointList {
    std::vector<SdrGluePoint*> aList;
protected:
    SdrGluePoint* GetObject(sal_uInt16 i) const { return aList[i]; }
public:
    SdrGluePointList(): aList() {}
    SdrGluePointList(const SdrGluePointList& rSrcList): aList()     { *this=rSrcList; }
    ~SdrGluePointList()                                                     { Clear(); }
    void                Clear();
    void                operator=(const SdrGluePointList& rSrcList);
    sal_uInt16          GetCount() const                                    { return sal_uInt16(aList.size()); }
    // At insert, the object (GluePoint) automatically gets an ID assigned.
    // Return value is the index of the new GluePoint in the list.
    sal_uInt16          Insert(const SdrGluePoint& rGP);
    void                Delete(sal_uInt16 nPos)
    {
        SdrGluePoint* p = aList[nPos];
        aList.erase(aList.begin()+nPos);
        delete p;
    }
    SdrGluePoint&       operator[](sal_uInt16 nPos)                             { return *GetObject(nPos); }
    const SdrGluePoint& operator[](sal_uInt16 nPos) const                       { return *GetObject(nPos); }
    sal_uInt16          FindGluePoint(sal_uInt16 nId) const;
    sal_uInt16          HitTest(const Point& rPnt, const OutputDevice& rOut, const SdrObject* pObj) const;
    void                Invalidate(vcl::Window& rWin, const SdrObject* pObj) const;
    // temp for transformations on the reference object
    void                SetReallyAbsolute(bool bOn, const SdrObject& rObj);
    void                Rotate(const Point& rRef, long nAngle, double sn, double cs, const SdrObject* pObj);
    void                Mirror(const Point& rRef1, const Point& rRef2, const SdrObject* pObj);
    void                Mirror(const Point& rRef1, const Point& rRef2, long nAngle, const SdrObject* pObj);
    void                Shear (const Point& rRef, long nAngle, double tn, bool bVShear, const SdrObject* pObj);
};


#endif // INCLUDED_SVX_SVDGLUE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

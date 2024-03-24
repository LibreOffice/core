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

#include <tools/gen.hxx>
#include <tools/degree.hxx>
#include <svx/svxdllapi.h>
#include <vector>
#include <o3tl/typed_flags_set.hxx>

namespace vcl { class Window; }
class OutputDevice;
class SdrObject;


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

class SVXCORE_DLLPUBLIC SdrGluePoint {
    // Reference Point is SdrObject::GetSnapRect().Center()
    // bNoPercent=false: position is -5000..5000 (1/100)% or 0..10000 (depending on align)
    // bNoPercent=true : position is in log unit, relative to the reference point
    Point    m_aPos;
    SdrEscapeDirection m_nEscDir;
    sal_uInt16   m_nId;
    SdrAlign     m_nAlign;
    bool m_bNoPercent:1;
    bool m_bReallyAbsolute:1; // temp for transformations on the reference object
    bool m_bUserDefined:1; // #i38892#
public:
    SdrGluePoint()
        : m_nEscDir(SdrEscapeDirection::SMART)
        , m_nId(0)
        , m_nAlign(SdrAlign::NONE)
        , m_bNoPercent(false)
        , m_bReallyAbsolute(false)
        , m_bUserDefined(true)
    {}
    SdrGluePoint(const Point& rNewPos)
        : m_aPos(rNewPos)
        , m_nEscDir(SdrEscapeDirection::SMART)
        , m_nId(0)
        , m_nAlign(SdrAlign::NONE)
        , m_bNoPercent(false)
        , m_bReallyAbsolute(false)
        , m_bUserDefined(true)
    {}
    const Point& GetPos() const
    {
        return m_aPos;
    }
    void SetPos(const Point& rNewPos)
    {
        m_aPos = rNewPos;
    }
    SdrEscapeDirection GetEscDir() const
    {
        return m_nEscDir;
    }
    void SetEscDir(SdrEscapeDirection nNewEsc)
    {
        m_nEscDir = nNewEsc;
    }
    sal_uInt16   GetId() const
    {
        return m_nId;
    }
    void SetId(sal_uInt16 nNewId)
    {
        m_nId = nNewId;
    }
    bool IsPercent() const
    {
        return !m_bNoPercent;
    }
    void SetPercent(bool bOn)
    {
        m_bNoPercent  = !bOn;
    }
    // temp for transformations on the reference object
    SAL_DLLPRIVATE void SetReallyAbsolute(bool bOn, const SdrObject& rObj);

    // #i38892#
    bool IsUserDefined() const
    {
        return m_bUserDefined;
    }
    void SetUserDefined(bool bNew)
    {
        m_bUserDefined = bNew;
    }

    SdrAlign GetAlign() const
    {
        return m_nAlign;
    }
    void SetAlign(SdrAlign nAlg)
    {
        m_nAlign = nAlg;
    }
    SdrAlign GetHorzAlign() const
    {
        return m_nAlign & static_cast<SdrAlign>(0x00FF);
    }
    void SetHorzAlign(SdrAlign nAlg)
    {
        assert((nAlg & static_cast<SdrAlign>(0xFF00)) == SdrAlign::NONE);
        m_nAlign = SdrAlign(m_nAlign & static_cast<SdrAlign>(0xFF00)) | (nAlg & static_cast<SdrAlign>(0x00FF));
    }
    SdrAlign GetVertAlign() const
    {
        return m_nAlign & static_cast<SdrAlign>(0xFF00);
    }
    void SetVertAlign(SdrAlign nAlg)
    {
        assert((nAlg & static_cast<SdrAlign>(0x00FF)) == SdrAlign::NONE);
        m_nAlign = SdrAlign(m_nAlign & static_cast<SdrAlign>(0x00FF)) | (nAlg & static_cast<SdrAlign>(0xFF00));
    }

    SAL_DLLPRIVATE bool IsHit(const Point& rPnt, const OutputDevice& rOut, const SdrObject* pObj) const;
    SAL_DLLPRIVATE void Invalidate(vcl::Window& rWin, const SdrObject* pObj) const;
    Point GetAbsolutePos(const SdrObject& rObj) const;
    SAL_DLLPRIVATE void SetAbsolutePos(const Point& rNewPos, const SdrObject& rObj);
    SAL_DLLPRIVATE Degree100 GetAlignAngle() const;
    SAL_DLLPRIVATE void SetAlignAngle(Degree100 nAngle);
    SAL_DLLPRIVATE static Degree100 EscDirToAngle(SdrEscapeDirection nEsc);
    SAL_DLLPRIVATE static SdrEscapeDirection EscAngleToDir(Degree100 nAngle);
    SAL_DLLPRIVATE void Rotate(const Point& rRef, Degree100 nAngle, double sn, double cs, const SdrObject* pObj);
    SAL_DLLPRIVATE void Mirror(const Point& rRef1, const Point& rRef2, Degree100 nAngle, const SdrObject* pObj);
    SAL_DLLPRIVATE void Shear (const Point& rRef, double tn, bool bVShear, const SdrObject* pObj);
};

#define SDRGLUEPOINT_NOTFOUND 0xFFFF

class SVXCORE_DLLPUBLIC SdrGluePointList
{
    std::vector<SdrGluePoint> m_aList;
public:
    SdrGluePointList() {};
    SdrGluePointList(const SdrGluePointList& rSrcList)
    {
        *this = rSrcList;
    }

    SdrGluePointList& operator=(const SdrGluePointList& rSrcList);
    sal_uInt16 GetCount() const
    {
        return sal_uInt16(m_aList.size());
    }
    // At insert, the object (GluePoint) automatically gets an ID assigned.
    // Return value is the index of the new GluePoint in the list.
    sal_uInt16 Insert(const SdrGluePoint& rGP);
    void Delete(sal_uInt16 nPos)
    {
        m_aList.erase(m_aList.begin() + nPos);
    }
    SdrGluePoint& operator[](sal_uInt16 nPos)
    {
        return m_aList[nPos];
    }
    const SdrGluePoint& operator[](sal_uInt16 nPos) const
    {
        return m_aList[nPos];
    }
    sal_uInt16 FindGluePoint(sal_uInt16 nId) const;
    SAL_DLLPRIVATE sal_uInt16 HitTest(const Point& rPnt, const OutputDevice& rOut, const SdrObject* pObj) const;
    SAL_DLLPRIVATE void Invalidate(vcl::Window& rWin, const SdrObject* pObj) const;

    // temp for transformations on the reference object
    SAL_DLLPRIVATE void SetReallyAbsolute(bool bOn, const SdrObject& rObj);
    SAL_DLLPRIVATE void Rotate(const Point& rRef, Degree100 nAngle, double sn, double cs, const SdrObject* pObj);
    SAL_DLLPRIVATE void Mirror(const Point& rRef1, const Point& rRef2, const SdrObject* pObj);
    SAL_DLLPRIVATE void Mirror(const Point& rRef1, const Point& rRef2, Degree100 nAngle, const SdrObject* pObj);
    SAL_DLLPRIVATE void Shear(const Point& rRef, double tn, bool bVShear, const SdrObject* pObj);
};


#endif // INCLUDED_SVX_SVDGLUE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

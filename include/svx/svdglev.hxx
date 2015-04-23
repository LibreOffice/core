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

#ifndef INCLUDED_SVX_SVDGLEV_HXX
#define INCLUDED_SVX_SVDGLEV_HXX

#include <svx/svxdllapi.h>
#include <svx/svdpoev.hxx>

class SdrGluePoint;


//  Edit GluePoints at the objects (GluePoints for connector)


class SVX_DLLPUBLIC SdrGlueEditView: public SdrPolyEditView
{
    // copy marked GluePoints and mark instead of the old ones
    void ImpCopyMarkedGluePoints();
    typedef void (*PGlueDoFunc)(SdrGluePoint&, const SdrObject* pObj, const void*, const void*, const void*, const void*, const void*);
    typedef void (*PGlueTrFunc)(Point&, const void*, const void*, const void*, const void*, const void*);
    void ImpDoMarkedGluePoints(PGlueDoFunc pDoFunc, bool bConst, const void* p1=NULL, const void* p2=NULL, const void* p3=NULL, const void* p4=NULL, const void* p5=NULL);
    void ImpTransformMarkedGluePoints(PGlueTrFunc pTrFunc, const void* p1=NULL, const void* p2=NULL, const void* p3=NULL, const void* p4=NULL, const void* p5=NULL);

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrGlueEditView(SdrModel* pModel1, OutputDevice* pOut = 0L);
    virtual ~SdrGlueEditView();

public:
    // the parameter nThisEsc is used to hand over the direction,
    // which one wished to check,set or delete
    // possible values for nThisEsc are:
    // SdrEscapeDirection::LEFT, SdrEscapeDirection::RIGHT, SdrEscapeDirection::TOP and SdrEscapeDirection::BOTTOM
    SDR_TRISTATE IsMarkedGluePointsEscDir(SdrEscapeDirection nThisEsc) const;
    void SetMarkedGluePointsEscDir(SdrEscapeDirection nThisEsc, bool bOn);
    bool IsSetMarkedGluePointsEscDirPossible() const { return !IsReadOnly() && HasMarkedGluePoints(); }

    // check/set, if the GluePoints are relativ to the
    // object size (Percent=sal_True) or not (Percent=sal_False)
    SDR_TRISTATE IsMarkedGluePointsPercent() const;
    void SetMarkedGluePointsPercent(bool bOn);
    bool IsSetMarkedGluePointsPercentPossible() const { return !IsReadOnly() && HasMarkedGluePoints(); }

    // bVert=FALSE: check/set hotizontal alignment
    //      SDRHORZALIGN_CENTER
    //      SDRHORZALIGN_LEFT
    //      SDRHORZALIGN_RIGHT
    //      SDRHORZALIGN_DONTCARE (only at Get())
    // bVert=TRUE: check/set vertical alignment
    //      SDRVERTALIGN_CENTER
    //      SDRVERTALIGN_TOP
    //      SDRVERTALIGN_BOTTOM
    //      SDRVERTALIGN_DONTCARE (only at Get())
    sal_uInt16 GetMarkedGluePointsAlign(bool bVert) const;
    void SetMarkedGluePointsAlign(bool bVert, sal_uInt16 nAlign);
    bool IsSetMarkedGluePointsAlignPossible() const { return !IsReadOnly() && HasMarkedGluePoints(); }

    // delete all marked GluePoints
    void DeleteMarkedGluePoints();

    void MoveMarkedGluePoints  (const Size& rSiz, bool bCopy=false);
    void ResizeMarkedGluePoints(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bCopy=false);
    void RotateMarkedGluePoints(const Point& rRef, long nAngle, bool bCopy=false);
    void MirrorMarkedGluePoints(const Point& rRef1, const Point& rRef2, bool bCopy=false);
    void MirrorMarkedGluePointsHorizontal(bool bCopy=false);
    void MirrorMarkedGluePointsVertical(bool bCopy=false);
    void ShearMarkedGluePoints(const Point& rRef, long nAngle, bool bVShear=false, bool bCopy=false);
    void CrookMarkedGluePoints(const Point& rRef, const Point& rRad, SdrCrookMode eMode, bool bVertical=false, bool bNoContortion=false, bool bCopy=false);
    void DistortMarkedGluePoints(const Rectangle& rRef, const XPolygon& rDistortedRect, bool bNoContortion=false, bool bCopy=false);

    void AlignMarkedGluePoints(SdrHorAlign eHor, SdrVertAlign eVert);
};

#endif // INCLUDED_SVX_SVDGLEV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

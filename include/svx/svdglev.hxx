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


//  Editieren von Klebepunkten an den Objekten (Klebepunkte fuer Verbinder)


class SVX_DLLPUBLIC SdrGlueEditView: public SdrPolyEditView
{
    void ImpClearVars();

    // Markierte Klebepunkte kopieren und anstelle der alten markieren
    void ImpCopyMarkedGluePoints();
    typedef void (*PGlueDoFunc)(SdrGluePoint&, const SdrObject* pObj, const void*, const void*, const void*, const void*, const void*);
    typedef void (*PGlueTrFunc)(Point&, const void*, const void*, const void*, const void*, const void*);
    void ImpDoMarkedGluePoints(PGlueDoFunc pDoFunc, sal_Bool bConst, const void* p1=NULL, const void* p2=NULL, const void* p3=NULL, const void* p4=NULL, const void* p5=NULL);
    void ImpTransformMarkedGluePoints(PGlueTrFunc pTrFunc, const void* p1=NULL, const void* p2=NULL, const void* p3=NULL, const void* p4=NULL, const void* p5=NULL);

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrGlueEditView(SdrModel* pModel1, OutputDevice* pOut = 0L);
    virtual ~SdrGlueEditView();

public:
    // Durch den Parameter nThisEsc uebergibt man die Richtung, die man
    // checken bzw. setzen/loeschen will.
    // Moegliche Werte fuer nThisEsc sind z.Zt.
    // SDRESC_LEFT, SDRESC_RIGHT, SDRESC_TOP und SDRESC_BOTTOM
    TRISTATE IsMarkedGluePointsEscDir(sal_uInt16 nThisEsc) const;
    void SetMarkedGluePointsEscDir(sal_uInt16 nThisEsc, sal_Bool bOn);
    sal_Bool IsSetMarkedGluePointsEscDirPossible() const { return !IsReadOnly() && HasMarkedGluePoints(); }

    // Checken/setzen, ob die Klebepunktpositionen relativ zur
    // Objektgroesse sind (Percent=sal_True) oder nicht (Percent=sal_False)
    TRISTATE IsMarkedGluePointsPercent() const;
    void SetMarkedGluePointsPercent(sal_Bool bOn);
    sal_Bool IsSetMarkedGluePointsPercentPossible() const { return !IsReadOnly() && HasMarkedGluePoints(); }

    // bVert=FALSE: Horizontales Alignment checken/setzen
    //      SDRHORZALIGN_CENTER
    //      SDRHORZALIGN_LEFT
    //      SDRHORZALIGN_RIGHT
    //      SDRHORZALIGN_DONTCARE (nur bei Get())
    // bVert=TRUE: Vertikales Alignment checken/setzen
    //      SDRVERTALIGN_CENTER
    //      SDRVERTALIGN_TOP
    //      SDRVERTALIGN_BOTTOM
    //      SDRVERTALIGN_DONTCARE (nur bei Get())
    sal_uInt16 GetMarkedGluePointsAlign(sal_Bool bVert) const;
    void SetMarkedGluePointsAlign(sal_Bool bVert, sal_uInt16 nAlign);
    sal_Bool IsSetMarkedGluePointsAlignPossible() const { return !IsReadOnly() && HasMarkedGluePoints(); }

    // Alle merkierten Klebepunkte entfernen
    void DeleteMarkedGluePoints();

    void MoveMarkedGluePoints  (const Size& rSiz, bool bCopy=false);
    void ResizeMarkedGluePoints(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bCopy=false);
    void RotateMarkedGluePoints(const Point& rRef, long nWink, bool bCopy=false);
    void MirrorMarkedGluePoints(const Point& rRef1, const Point& rRef2, sal_Bool bCopy=sal_False);
    void MirrorMarkedGluePointsHorizontal(sal_Bool bCopy=sal_False);
    void MirrorMarkedGluePointsVertical(sal_Bool bCopy=sal_False);
    void ShearMarkedGluePoints(const Point& rRef, long nWink, sal_Bool bVShear=sal_False, sal_Bool bCopy=sal_False);
    void CrookMarkedGluePoints(const Point& rRef, const Point& rRad, SdrCrookMode eMode, sal_Bool bVertical=sal_False, sal_Bool bNoContortion=sal_False, sal_Bool bCopy=sal_False);
    void DistortMarkedGluePoints(const Rectangle& rRef, const XPolygon& rDistortedRect, sal_Bool bNoContortion=sal_False, sal_Bool bCopy=sal_False);

    void AlignMarkedGluePoints(SdrHorAlign eHor, SdrVertAlign eVert);
};

#endif // INCLUDED_SVX_SVDGLEV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

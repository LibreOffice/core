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

#ifndef _XEXPTRANSFORM_HXX
#define _XEXPTRANSFORM_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/drawing/FlagSequence.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#include <tools/mapunit.hxx>

#include <vector>

// predeclarations

struct ImpSdXMLExpTransObj2DBase;
struct ImpSdXMLExpTransObj3DBase;
class SvXMLUnitConverter;
class SvXMLImport;
class SvXMLExport;

namespace basegfx
{
    class B2DTuple;
    class B2DHomMatrix;
    class B3DHomMatrix;
} // end of namespace basegfx

typedef ::std::vector< ImpSdXMLExpTransObj2DBase* > ImpSdXMLExpTransObj2DBaseList;
typedef ::std::vector< ImpSdXMLExpTransObj3DBase* > ImpSdXMLExpTransObj3DBaseList;

class SdXMLImExTransform2D
{
    ImpSdXMLExpTransObj2DBaseList   maList;
    OUString                   msString;

    void EmptyList();

public:
    SdXMLImExTransform2D() {}
    ~SdXMLImExTransform2D() { EmptyList(); }

    void AddRotate(double fNew);
    void AddTranslate(const ::basegfx::B2DTuple& rNew);
    void AddSkewX(double fNew);

    bool NeedsAction() const { return !maList.empty(); }
    void GetFullTransform(::basegfx::B2DHomMatrix& rFullTrans);
    const OUString& GetExportString(const SvXMLUnitConverter& rConv);
    void SetString(const OUString& rNew, const SvXMLUnitConverter& rConv);
};

class SdXMLImExTransform3D
{
    ImpSdXMLExpTransObj3DBaseList   maList;
    OUString                   msString;

    void EmptyList();

public:
    SdXMLImExTransform3D() {}
    SdXMLImExTransform3D(const OUString& rNew, const SvXMLUnitConverter& rConv);
    ~SdXMLImExTransform3D() { EmptyList(); }

    void AddMatrix(const ::basegfx::B3DHomMatrix& rNew);

    void AddHomogenMatrix(const com::sun::star::drawing::HomogenMatrix& xHomMat);
    bool NeedsAction() const { return !maList.empty(); }
    void GetFullTransform(::basegfx::B3DHomMatrix& rFullTrans);
    bool GetFullHomogenTransform(com::sun::star::drawing::HomogenMatrix& xHomMat);
    const OUString& GetExportString(const SvXMLUnitConverter& rConv);
    void SetString(const OUString& rNew, const SvXMLUnitConverter& rConv);
};

class SdXMLImExViewBox
{
    OUString               msString;
    sal_Int32                   mnX;
    sal_Int32                   mnY;
    sal_Int32                   mnW;
    sal_Int32                   mnH;

public:
    SdXMLImExViewBox(sal_Int32 nX = 0L, sal_Int32 nY = 0L, sal_Int32 nW = 1000L, sal_Int32 nH = 1000L);
    SdXMLImExViewBox(const OUString& rNew, const SvXMLUnitConverter& rConv);

    sal_Int32 GetX() const { return mnX; }
    sal_Int32 GetY() const { return mnY; }
    sal_Int32 GetWidth() const { return mnW; }
    sal_Int32 GetHeight() const { return mnH; }
    const OUString& GetExportString();
};

class SdXMLImExPointsElement
{
    OUString               msString;
    com::sun::star::drawing::PointSequenceSequence  maPoly;

public:
    SdXMLImExPointsElement(com::sun::star::drawing::PointSequence* pPoints,
        const SdXMLImExViewBox& rViewBox,
        const com::sun::star::awt::Point& rObjectPos,
        const com::sun::star::awt::Size& rObjectSize,
        // #96328#
        const bool bClosed = true);
    SdXMLImExPointsElement(const OUString& rNew,
        const SdXMLImExViewBox& rViewBox,
        const com::sun::star::awt::Point& rObjectPos,
        const com::sun::star::awt::Size& rObjectSize,
        const SvXMLUnitConverter& rConv);

    const OUString& GetExportString() const { return msString; }
    const com::sun::star::drawing::PointSequenceSequence& GetPointSequenceSequence() const { return maPoly; }
};

class SdXMLImExSvgDElement
{
    OUString                   msString;
    const SdXMLImExViewBox&         mrViewBox;
    bool                            mbIsClosed;
    bool                            mbIsCurve;
    bool                            mbRelative;
    sal_Int32                       mnLastX;
    sal_Int32                       mnLastY;

    com::sun::star::drawing::PolyPolygonBezierCoords maPoly;

public:
    SdXMLImExSvgDElement(const SdXMLImExViewBox& rViewBox, const SvXMLExport& rExport);
    SdXMLImExSvgDElement(const OUString& rNew,
        const SdXMLImExViewBox& rViewBox,
        const com::sun::star::awt::Point& rObjectPos,
        const com::sun::star::awt::Size& rObjectSize,
        const SvXMLImport& rImport);

    void AddPolygon(
        com::sun::star::drawing::PointSequence* pPoints,
        com::sun::star::drawing::FlagSequence* pFlags,
        const com::sun::star::awt::Point& rObjectPos,
        const com::sun::star::awt::Size& rObjectSize,
        bool bClosed = false);

    const OUString& GetExportString() const { return msString; }
    bool IsClosed() const { return mbIsClosed; }
    bool IsCurve() const { return mbIsCurve; }
    const com::sun::star::drawing::PointSequenceSequence& GetPointSequenceSequence() const { return maPoly.Coordinates; }
    const com::sun::star::drawing::FlagSequenceSequence& GetFlagSequenceSequence() const { return maPoly.Flags; }
};

#endif  //  _XEXPTRANSFORM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

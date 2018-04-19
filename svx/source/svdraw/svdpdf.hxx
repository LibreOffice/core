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

#ifndef INCLUDED_SVX_SOURCE_SVDRAW_SVDPDF_HXX
#define INCLUDED_SVX_SOURCE_SVDRAW_SVDPDF_HXX

#include <sal/config.h>

#include <memory>

#include <tools/contnr.hxx>
#include <tools/fract.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <svx/svdobj.hxx>
#include <svx/xdash.hxx>
#include <com/sun/star/uno/Sequence.hxx>

// Forward Declarations

class SfxItemSet;
class SdrObjList;
class SdrModel;
class SdrPage;
class SdrObject;
class SvdProgressInfo;
typedef void* FPDF_DOCUMENT;
typedef void* FPDF_PAGEOBJECT;

// Helper Class to import PDF
class ImpSdrPdfImport final
{
    class Matrix
    {
    public:
        Matrix()
            : Matrix(1, 0, 0, 1, 0, 0)
        {
        }

        Matrix(const Matrix& other)
            : Matrix(other.ma, other.mb, other.mc, other.md, other.me, other.mf)
        {
        }

        Matrix(double a, double b, double c, double d, double e, double f)
            : ma(a)
            , mb(b)
            , mc(c)
            , md(d)
            , me(e)
            , mf(f)
        {
        }

        const Matrix& operator=(const Matrix& other)
        {
            ma = other.ma;
            mb = other.mb;
            mc = other.mc;
            md = other.md;
            me = other.me;
            mf = other.mf;
            return *this;
        }

        double a() const { return ma; }
        double b() const { return mb; }
        double c() const { return mc; }
        double d() const { return md; }
        double e() const { return me; }
        double f() const { return mf; }

        /// Mutliply this * other.
        void Concatinate(const Matrix& other)
        {
            ma = ma * other.ma + mb * other.mc;
            mb = ma * other.mb + mb * other.md;
            mc = mc * other.ma + md * other.mc;
            md = mc * other.mb + md * other.md;
            me = me * other.ma + mf * other.mc + other.me;
            mf = me * other.mb + mf * other.md + other.mf;
        }

        /// Transform the point (x, y) by this Matrix.
        void Transform(double& x, double& y)
        {
            x = ma * x + mc * y + me;
            y = mb * x + md * y + mf;
        }

    private:
        double ma, mb, mc, md, me, mf;
    };

    ::std::vector<SdrObject*> maTmpList;
    ScopedVclPtr<VirtualDevice> mpVD;
    Rectangle maScaleRect;
    const std::shared_ptr<css::uno::Sequence<sal_Int8>> mpPdfData;
    size_t mnMapScalingOfs; // from here on, not edited with MapScaling
    std::unique_ptr<SfxItemSet> mpLineAttr;
    std::unique_ptr<SfxItemSet> mpFillAttr;
    std::unique_ptr<SfxItemSet> mpTextAttr;
    SdrModel* mpModel;
    SdrLayerID mnLayer;
    Color maOldLineColor;
    sal_Int32 mnLineWidth;
    basegfx::B2DLineJoin maLineJoin;
    css::drawing::LineCap maLineCap;
    XDash maDash;

    bool mbMov;
    bool mbSize;
    Point maOfs;
    double mfScaleX;
    double mfScaleY;
    Fraction maScaleX;
    Fraction maScaleY;

    bool mbFntDirty;

    // to optimize (PenNULL,Brush,DrawPoly),(Pen,BrushNULL,DrawPoly) -> two-in-one
    bool mbLastObjWasPolyWithoutLine;
    bool mbNoLine;
    bool mbNoFill;

    // to optimize multiple lines into a Polyline
    bool mbLastObjWasLine;

    // clipregion
    basegfx::B2DPolyPolygon maClip;

    FPDF_DOCUMENT mpPdfDocument;
    int mnPageCount;
    double mdPageWidthPts;
    double mdPageHeightPts;
    /// The current transformation matrix, typically used with Form objects.
    Matrix mCurMatrix;

    /// Correct the vertical coordinate to start at the top.
    /// PDF coordinate system has orign at the bottom right.
    double correctVertOrigin(double offsetPts) const { return mdPageHeightPts - offsetPts; }

    /// Convert PDF points to logic (twips).
    Rectangle PointsToLogic(double left, double right, double top, double bottom) const;
    Point PointsToLogic(double x, double y) const;

    // check for clip and evtl. fill maClip
    void checkClip();
    bool isClip() const;

    void ImportPdfObject(FPDF_PAGEOBJECT pPageObject, int nPageObjectIndex);
    void ImportForm(FPDF_PAGEOBJECT pPageObject, int nPageObjectIndex);
    void ImportImage(FPDF_PAGEOBJECT pPageObject, int nPageObjectIndex);
    void ImportPath(FPDF_PAGEOBJECT pPageObject, int nPageObjectIndex);
    void ImportText(FPDF_PAGEOBJECT pPageObject, int nPageObjectIndex);
    void ImportText(const Point& rPos, const Size& rSize, const OUString& rStr);

    void SetupPageScale(const double dPageWidth, const double dPageHeight);
    void SetAttributes(SdrObject* pObj, bool bForceTextAttr = false);
    void InsertObj(SdrObject* pObj, bool bScale = true);
    void MapScaling();

    // #i73407# reformulation to use new B2DPolygon classes
    bool CheckLastLineMerge(const basegfx::B2DPolygon& rSrcPoly);
    bool CheckLastPolyLineAndFillMerge(const basegfx::B2DPolyPolygon& rPolyPolygon);

    void DoLoopActions(SvdProgressInfo* pProgrInfo, sal_uInt32* pActionsToReport, int nPageIndex);

    // Copy assignment is forbidden and not implemented.
    ImpSdrPdfImport(const ImpSdrPdfImport&) = delete;
    ImpSdrPdfImport& operator=(const ImpSdrPdfImport&) = delete;

public:
    ImpSdrPdfImport(SdrModel& rModel, SdrLayerID nLay, const Rectangle& rRect,
                    const std::shared_ptr<css::uno::Sequence<sal_Int8>>& pPdfData);
    ~ImpSdrPdfImport();

    int GetPageCount() const { return mnPageCount; }

    size_t DoImport(SdrObjList& rDestList, size_t nInsPos, int nPageNumber,
                    SvdProgressInfo* pProgrInfo = nullptr);
};

#endif // INCLUDED_SVX_SOURCE_SVDRAW_SVDFMTF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

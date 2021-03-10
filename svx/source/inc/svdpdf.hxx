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

#include <tools/fract.hxx>
#include <vcl/virdev.hxx>
#include <vcl/graph.hxx>
#include <svx/svdobj.hxx>
#include <svx/xdash.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>

#include <vcl/filter/PDFiumLibrary.hxx>

// Forward Declarations

class SfxItemSet;
class SdrObjList;
class SdrModel;
class SdrPage;
class SdrObject;
class SvdProgressInfo;

// Helper Class to import PDF
class ImpSdrPdfImport final
{
    std::vector<SdrObject*> maTmpList;
    ScopedVclPtr<VirtualDevice> mpVD;
    tools::Rectangle maScaleRect;
    size_t mnMapScalingOfs; // from here on, not edited with MapScaling
    std::unique_ptr<SfxItemSet> mpLineAttr;
    std::unique_ptr<SfxItemSet> mpFillAttr;
    std::unique_ptr<SfxItemSet> mpTextAttr;
    SdrModel* mpModel;
    SdrLayerID mnLayer;
    Color maOldLineColor;
    sal_Int32 mnLineWidth;
    static constexpr css::drawing::LineCap gaLineCap = css::drawing::LineCap_BUTT;
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

    // clipregion
    basegfx::B2DPolyPolygon maClip;

    std::unique_ptr<vcl::pdf::PDFiumDocument> mpPdfDocument;
    int mnPageCount;
    double mdPageHeightPts;
    /// The current transformation matrix, typically used with Form objects.
    basegfx::B2DHomMatrix maCurrentMatrix;

    /// Correct the vertical coordinate to start at the top.
    /// PDF coordinate system has origin at the bottom right.
    double correctVertOrigin(double offsetPts) const { return mdPageHeightPts - offsetPts; }
    /// Convert PDF points to logic (twips).
    tools::Rectangle PointsToLogic(double left, double right, double top, double bottom) const;
    Point PointsToLogic(double x, double y) const;

    std::shared_ptr<vcl::pdf::PDFium> mpPDFium;

    // check for clip and evtl. fill maClip
    void checkClip();
    bool isClip() const;

    void ImportPdfObject(std::unique_ptr<vcl::pdf::PDFiumPageObject> const& pPageObject,
                         std::unique_ptr<vcl::pdf::PDFiumTextPage> const& pTextPage,
                         int nPageObjectIndex);
    void ImportForm(std::unique_ptr<vcl::pdf::PDFiumPageObject> const& pPageObject,
                    std::unique_ptr<vcl::pdf::PDFiumTextPage> const& pTextPage,
                    int nPageObjectIndex);
    void ImportImage(std::unique_ptr<vcl::pdf::PDFiumPageObject> const& pPageObject,
                     int nPageObjectIndex);
    void ImportPath(std::unique_ptr<vcl::pdf::PDFiumPageObject> const& pPageObject,
                    int nPageObjectIndex);
    void ImportText(std::unique_ptr<vcl::pdf::PDFiumPageObject> const& pPageObject,
                    std::unique_ptr<vcl::pdf::PDFiumTextPage> const& pTextPage,
                    int nPageObjectIndex);
    void InsertTextObject(const Point& rPos, const Size& rSize, const OUString& rStr);

    void SetupPageScale(const double dPageWidth, const double dPageHeight);
    void SetAttributes(SdrObject* pObj, bool bForceTextAttr = false);
    void InsertObj(SdrObject* pObj, bool bScale = true);
    void MapScaling();

    // #i73407# reformulation to use new B2DPolygon classes
    bool CheckLastPolyLineAndFillMerge(const basegfx::B2DPolyPolygon& rPolyPolygon);

    void DoObjects(SvdProgressInfo* pProgrInfo, sal_uInt32* pActionsToReport, int nPageIndex);

    // Copy assignment is forbidden and not implemented.
    ImpSdrPdfImport(const ImpSdrPdfImport&) = delete;
    ImpSdrPdfImport& operator=(const ImpSdrPdfImport&) = delete;

public:
    ImpSdrPdfImport(SdrModel& rModel, SdrLayerID nLay, const tools::Rectangle& rRect,
                    Graphic const& rGraphic);

    ~ImpSdrPdfImport();

    int GetPageCount() const { return mnPageCount; }
    size_t DoImport(SdrObjList& rDestList, size_t nInsPos, int nPageNumber,
                    SvdProgressInfo* pProgrInfo = nullptr);
};

#endif // INCLUDED_SVX_SOURCE_SVDRAW_SVDPDF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

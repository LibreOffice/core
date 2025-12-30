/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <memory>
#include <vector>

#include <svl/itemset.hxx>
#include <tools/fract.hxx>
#include <vcl/virdev.hxx>
#include <svx/svdobj.hxx>
#include <svx/xdash.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

class SdrModel;
class SdrPathObj;

/**
 * Base class for SdrObject import functionality shared between
 * ImpSdrGDIMetaFileImport and ImpSdrPdfImport.
 */
class ImpSdrImportBase
{
protected:
    std::vector<rtl::Reference<SdrObject>> maTmpList;
    ScopedVclPtr<VirtualDevice> mpVD;
    tools::Rectangle maScaleRect;
    std::unique_ptr<SfxItemSet> mpLineAttr;
    std::unique_ptr<SfxItemSet> mpFillAttr;
    std::unique_ptr<SfxItemSet> mpTextAttr;
    SdrModel* mpModel;
    SdrLayerID mnLayer;
    sal_Int32 mnLineWidth;
    XDash maDash;

    bool mbMov;
    bool mbSize;
    Point maOfs;
    double mfScaleX;
    double mfScaleY;
    double maScaleX;
    double maScaleY;

    bool mbFntDirty;

    // to optimize (PenNULL,Brush,DrawPoly),(Pen,BrushNULL,DrawPoly) -> two-in-one
    bool mbLastObjWasPolyWithoutLine;
    bool mbNoLine;
    bool mbNoFill;

    /**
     * @param rModel The SdrModel to use
     * @param nLay The layer to place objects on
     * @param rRect The scaling rectangle
     */
    ImpSdrImportBase(SdrModel& rModel, SdrLayerID nLay, const tools::Rectangle& rRect);

    /**
     * SetAttributes must be implemented by derived classes as they have
     * different attribute handling logic.
     */
    virtual void SetAttributes(SdrObject* pObj, bool bForceTextAttr = false) = 0;

public:
    /**
     * Check if the last polygon can be merged with the current one
     * to optimize polyline and fill rendering.
     *
     * This method is shared between GDI Metafile and PDF import.
     * #i73407# reformulation to use new B2DPolygon classes
     */
    bool CheckLastPolyLineAndFillMerge(const basegfx::B2DPolyPolygon& rPolyPolygon);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_DRAWABLES_B2DPOLYPOLYLINEDRAWABLEHELPER_HXX
#define INCLUDED_VCL_INC_DRAWABLES_B2DPOLYPOLYLINEDRAWABLEHELPER_HXX

#include <vcl/gdimtf.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/outdev.hxx>
#include <vcl/vclptr.hxx>

#include <memory>

class LineInfo;
class OutputDevice;
namespace basegfx
{
class B2DPolyPolygon;
}

namespace vcl
{
class DisableMetafileProcessing
{
public:
    DisableMetafileProcessing(VclPtr<OutputDevice> pRenderContext)
        : mpRenderContext(pRenderContext)
    {
        mpMtf = pRenderContext->GetConnectMetaFile();
        pRenderContext->SetConnectMetaFile(nullptr);
    }

    ~DisableMetafileProcessing() { mpRenderContext->SetConnectMetaFile(mpMtf); }

private:
    GDIMetaFile* mpMtf;
    VclPtr<OutputDevice> mpRenderContext;
};

class VCL_DLLPUBLIC B2DPolyPolyLineDrawableHelper
{
public:
    static bool CanApplyDashes(basegfx::B2DPolyPolygon const& rLinePolyPolygon,
                               LineInfo const& rLineInfo);

    static bool DashDotArrayHasLength(::std::vector<double> const& rDotDashArray);

    static basegfx::B2DPolyPolygon ApplyLineDashing(basegfx::B2DPolyPolygon const& rLinePolyPolygon,
                                                    LineInfo const& rLineInfo);

    static bool UseLineWidth(basegfx::B2DPolyPolygon const& rLinePolyPolygon,
                             LineInfo const& rLineInfo);

    static basegfx::B2DPolyPolygon CreateFillPolyPolygon(basegfx::B2DPolyPolygon& rLinePolyPolygon,
                                                         LineInfo const& rLineInfo);

    static void DrawPolyLine(OutputDevice* pRenderContext, SalGraphics* const pGraphics,
                             basegfx::B2DPolyPolygon const& rLinePolyPolygon);

    static void FillPolyPolygon(OutputDevice* pRenderContext, SalGraphics* const pGraphics,
                                basegfx::B2DPolyPolygon const& rFillPolyPolygon);

    static ::std::vector<double> GenerateDotDashArray(LineInfo const& rLineInfo);
};

} // namespace vcl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

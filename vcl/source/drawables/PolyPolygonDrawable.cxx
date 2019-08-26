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

#include <tools/poly.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

#include <vcl/metaact.hxx>
#include <vcl/outdev.hxx>
#include <vcl/virdev.hxx>
#include <vcl/drawables/PolyPolygonDrawable.hxx>

#include <salgdi.hxx>

#define OUTDEV_POLYPOLY_STACKBUF 32

namespace vcl
{
bool PolyPolygonDrawable::DrawCommand(OutputDevice* pRenderContext) const
{
    if (mbRecursive)
        return Draw(pRenderContext, mnPoly, maPolyPolygon);

    if (mbUsesClipping)
        return Draw(pRenderContext, maPolyPolygon, maClipPolyPolygon);

    return Draw(pRenderContext, maPolyPolygon);
}

bool PolyPolygonDrawable::Draw(OutputDevice* pRenderContext,
                               tools::PolyPolygon const& rPolyPoly) const
{
    sal_uInt16 nPoly = rPolyPoly.Count();

    if (!nPoly)
        return false;

    // use b2dpolygon drawing if possible
    if ((pRenderContext->GetAntialiasing() & AntialiasingFlags::EnableB2dDraw)
        && mpGraphics->supportsOperation(OutDevSupportType::B2DDraw)
        && pRenderContext->GetRasterOp() == RasterOp::OverPaint
        && (pRenderContext->IsLineColor() || pRenderContext->IsFillColor()))
    {
        const basegfx::B2DHomMatrix aTransform(pRenderContext->ImplGetDeviceTransformation());
        basegfx::B2DPolyPolygon aB2DPolyPolygon(rPolyPoly.getB2DPolyPolygon());
        bool bSuccess = true;

        // ensure closed - may be asserted, will prevent buffering
        if (!aB2DPolyPolygon.isClosed())
        {
            aB2DPolyPolygon.setClosed(true);
        }

        if (pRenderContext->IsFillColor())
        {
            bSuccess
                = mpGraphics->DrawPolyPolygon(aTransform, aB2DPolyPolygon, 0.0, pRenderContext);
        }

        if (bSuccess && pRenderContext->IsLineColor())
        {
            const basegfx::B2DVector aB2DLineWidth(1.0, 1.0);
            const bool bPixelSnapHairline(pRenderContext->GetAntialiasing()
                                          & AntialiasingFlags::PixelSnapHairline);

            for (auto const& rPolygon : aB2DPolyPolygon)
            {
                bSuccess = mpGraphics->DrawPolyLine(
                    aTransform, rPolygon, 0.0, aB2DLineWidth, basegfx::B2DLineJoin::NONE,
                    css::drawing::LineCap_BUTT,
                    basegfx::deg2rad(
                        15.0), // not used with B2DLineJoin::NONE, but the correct default
                    bPixelSnapHairline, pRenderContext);

                if (!bSuccess)
                    break;
            }
        }

        if (bSuccess)
        {
            DrawAlphaVirtDev(pRenderContext);
            return true;
        }
    }

    if (nPoly == 1)
    {
        // #100127# Map to DrawPolygon
        const tools::Polygon& aPoly = rPolyPoly.GetObject(0);
        if (aPoly.GetSize() >= 2)
        {
            GDIMetaFile* pOldMF = pRenderContext->GetConnectMetaFile();
            pRenderContext->SetConnectMetaFile(nullptr);

            pRenderContext->DrawPolygon(aPoly);

            pRenderContext->SetConnectMetaFile(pOldMF);
        }
    }
    else
    {
        // #100127# moved real tools::PolyPolygon draw to separate method,
        // have to call recursively, avoiding duplicate
        // ImplLogicToDevicePixel calls
        pRenderContext->Draw(
            PolyPolygonDrawable(nPoly, pRenderContext->ImplLogicToDevicePixel(rPolyPoly)));
    }

    return true;
}

bool PolyPolygonDrawable::Draw(OutputDevice* pRenderContext, tools::PolyPolygon const& rPolyPoly,
                               tools::PolyPolygon const& rClipPolyPoly) const
{
    tools::PolyPolygon* pPolyPoly = new tools::PolyPolygon;
    rPolyPoly.GetIntersection(rClipPolyPoly, *pPolyPoly);

    if (pPolyPoly->Count() == 1)
    {
        const tools::Polygon& rPoly = pPolyPoly->GetObject(0);
        sal_uInt16 nSize = rPoly.GetSize();

        if (nSize >= 2)
        {
            const SalPoint* pPtAry = reinterpret_cast<const SalPoint*>(rPoly.GetConstPointAry());
            mpGraphics->DrawPolygon(nSize, pPtAry, pRenderContext);
        }
    }
    else if (pPolyPoly->Count())
    {
        sal_uInt16 nCount = pPolyPoly->Count();
        std::unique_ptr<sal_uInt32[]> pPointAry(new sal_uInt32[nCount]);
        std::unique_ptr<PCONSTSALPOINT[]> pPointAryAry(new PCONSTSALPOINT[nCount]);
        sal_uInt16 i = 0;
        do
        {
            const tools::Polygon& rPoly = pPolyPoly->GetObject(i);
            sal_uInt16 nSize = rPoly.GetSize();
            if (nSize)
            {
                pPointAry[i] = nSize;
                pPointAryAry[i] = reinterpret_cast<PCONSTSALPOINT>(rPoly.GetConstPointAry());
                i++;
            }
            else
            {
                nCount--;
            }
        } while (i < nCount);

        if (nCount == 1)
            mpGraphics->DrawPolygon(pPointAry[0], pPointAryAry[0], pRenderContext);
        else
            mpGraphics->DrawPolyPolygon(nCount, pPointAry.get(), pPointAryAry.get(),
                                        pRenderContext);
    }

    delete pPolyPoly;

    return true;
}

bool PolyPolygonDrawable::Draw(OutputDevice* pRenderContext, sal_uInt16 nPoly,
                               tools::PolyPolygon const& rPolyPoly) const
{
    // this crashes on empty PolyPolygons, avoid that
    if (!nPoly)
        return false;

    sal_uInt32 aStackAry1[OUTDEV_POLYPOLY_STACKBUF];
    PCONSTSALPOINT aStackAry2[OUTDEV_POLYPOLY_STACKBUF];
    PolyFlags* aStackAry3[OUTDEV_POLYPOLY_STACKBUF];
    sal_uInt32* pPointAry;
    PCONSTSALPOINT* pPointAryAry;
    const PolyFlags** pFlagAryAry;
    sal_uInt16 i = 0;
    sal_uInt16 j = 0;
    sal_uInt16 last = 0;
    bool bHaveBezier = false;

    if (nPoly > OUTDEV_POLYPOLY_STACKBUF)
    {
        pPointAry = new sal_uInt32[nPoly];
        pPointAryAry = new PCONSTSALPOINT[nPoly];
        pFlagAryAry = new const PolyFlags*[nPoly];
    }
    else
    {
        pPointAry = aStackAry1;
        pPointAryAry = aStackAry2;
        pFlagAryAry = const_cast<const PolyFlags**>(aStackAry3);
    }

    do
    {
        const tools::Polygon& rPoly = rPolyPoly.GetObject(i);
        sal_uInt16 nSize = rPoly.GetSize();
        if (nSize)
        {
            pPointAry[j] = nSize;
            pPointAryAry[j] = reinterpret_cast<PCONSTSALPOINT>(rPoly.GetConstPointAry());
            pFlagAryAry[j] = rPoly.GetConstFlagAry();
            last = i;

            if (pFlagAryAry[j])
                bHaveBezier = true;

            ++j;
        }
        ++i;
    } while (i < nPoly);

    if (j == 1)
    {
        // #100127# Forward beziers to sal, if any
        if (bHaveBezier)
        {
            if (!mpGraphics->DrawPolygonBezier(*pPointAry, *pPointAryAry, *pFlagAryAry,
                                               pRenderContext))
            {
                tools::Polygon aPoly = tools::Polygon::SubdivideBezier(rPolyPoly.GetObject(last));
                mpGraphics->DrawPolygon(aPoly.GetSize(),
                                        reinterpret_cast<const SalPoint*>(aPoly.GetConstPointAry()),
                                        pRenderContext);
            }
        }
        else
        {
            mpGraphics->DrawPolygon(*pPointAry, *pPointAryAry, pRenderContext);
        }
    }
    else
    {
        // #100127# Forward beziers to sal, if any
        if (bHaveBezier)
        {
            if (!mpGraphics->DrawPolyPolygonBezier(j, pPointAry, pPointAryAry, pFlagAryAry,
                                                   pRenderContext))
            {
                tools::PolyPolygon aPolyPoly = tools::PolyPolygon::SubdivideBezier(rPolyPoly);
                Draw(pRenderContext, aPolyPoly.Count(), aPolyPoly);
            }
        }
        else
        {
            mpGraphics->DrawPolyPolygon(j, pPointAry, pPointAryAry, pRenderContext);
        }
    }

    if (pPointAry != aStackAry1)
    {
        delete[] pPointAry;
        delete[] pPointAryAry;
        delete[] pFlagAryAry;
    }

    return true;
}

bool PolyPolygonDrawable::CanDraw(OutputDevice* pRenderContext) const
{
    if (!pRenderContext->IsDeviceOutputNecessary()
        || (!pRenderContext->IsLineColor() && !pRenderContext->IsFillColor())
        || pRenderContext->ImplIsRecordLayout())
        return false;

    return true;
}

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SlideshowLayerRenderer.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdview.hxx>
#include <svx/unoapi.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/svdoutl.hxx>
#include <vcl/virdev.hxx>
#include <tools/helpers.hxx>
#include <tools/json_writer.hxx>

namespace sd
{
namespace
{
class SelectObjectRedirector : public sdr::contact::ViewObjectContactRedirector
{
public:
    virtual void createRedirectedPrimitive2DSequence(
        const sdr::contact::ViewObjectContact& rOriginal,
        const sdr::contact::DisplayInfo& rDisplayInfo,
        drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) override
    {
        SdrObject* pObject = rOriginal.GetViewContact().TryToGetSdrObject();
        SdrPage* pPage = pObject ? pObject->getSdrPageFromSdrObject() : nullptr;

        if (pObject == nullptr || pPage == nullptr)
        {
            // Not a SdrObject or a object not connected to a page (object with no page)

            //sdr::contact::ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(
            //    rOriginal, rDisplayInfo, rVisitor);
            return;
        }

        const bool bDoCreateGeometry(
            pObject->getSdrPageFromSdrObject()->checkVisibility(rOriginal, rDisplayInfo, true));

        if (!bDoCreateGeometry
            && (pObject->GetObjInventor() != SdrInventor::Default
                || pObject->GetObjIdentifier() != SdrObjKind::Page))
        {
            return;
        }

        if (pObject->IsEmptyPresObj())
            return;

        sdr::contact::ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(
            rOriginal, rDisplayInfo, rVisitor);
    }
};
}

SlideshowLayerRenderer::SlideshowLayerRenderer(SdrPage* pPage)
    : mpPage(pPage)
{
}

Size SlideshowLayerRenderer::calculateAndSetSizePixel(Size const& rDesiredSizePixel)
{
    if (!mpPage)
        return Size();

    double fRatio = double(mpPage->GetHeight()) / mpPage->GetWidth();
    Size aSize(rDesiredSizePixel.Width(), ::tools::Long(rDesiredSizePixel.Width() * fRatio));
    maSlideSize = aSize;

    return maSlideSize;
}

bool SlideshowLayerRenderer::render(unsigned char* pBuffer, OString& rJsonMsg)
{
    if (bRenderDone)
        return false;

    if (!mpPage)
        return false;

    SdrModel& rModel = mpPage->getSdrModelFromSdrPage();

    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::WITHOUT_ALPHA);
    pDevice->SetBackground(Wallpaper(COL_TRANSPARENT));

    pDevice->SetOutputSizePixelScaleOffsetAndLOKBuffer(maSlideSize, Fraction(2.0), Point(),
                                                       pBuffer);

    Point aPoint;
    Size aPageSize(mpPage->GetSize());

    MapMode aMapMode(MapUnit::Map100thMM);
    const Fraction aFracX(maSlideSize.Width(), pDevice->LogicToPixel(aPageSize, aMapMode).Width());
    aMapMode.SetScaleX(aFracX);

    const Fraction aFracY(maSlideSize.Height(),
                          pDevice->LogicToPixel(aPageSize, aMapMode).Height());
    aMapMode.SetScaleY(aFracY);

    pDevice->SetMapMode(aMapMode);

    SdrView aView(rModel, pDevice);

    aView.SetPageVisible(false);
    aView.SetPageShadowVisible(false);
    aView.SetPageBorderVisible(false);
    aView.SetBordVisible(false);
    aView.SetGridVisible(false);
    aView.SetHlplVisible(false);
    aView.SetGlueVisible(false);

    aView.ShowSdrPage(mpPage);

    vcl::Region aRegion(::tools::Rectangle(aPoint, aPageSize));
    SelectObjectRedirector aRedirector;
    aView.CompleteRedraw(pDevice, aRegion, &aRedirector);

    ::tools::JsonWriter aJsonWriter;
    aJsonWriter.put("group", "DrawPage");
    aJsonWriter.put("slideHash", GetInterfaceHash(GetXDrawPageForSdrPage(mpPage)));
    aJsonWriter.put("index", 0);
    aJsonWriter.put("type", "bitmap");
    {
        ::tools::ScopedJsonWriterNode aContentNode = aJsonWriter.startNode("content");
        aJsonWriter.put("type", "%IMAGETYPE%");
        aJsonWriter.put("checksum", "%IMAGECHECKSUM%");
    }
    rJsonMsg = aJsonWriter.finishAndGetAsOString();

    bRenderDone = true;

    return true;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

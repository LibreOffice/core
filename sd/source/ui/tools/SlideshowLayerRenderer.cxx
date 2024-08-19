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
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdpagv.hxx>
#include <vcl/virdev.hxx>
#include <tools/helpers.hxx>
#include <tools/json_writer.hxx>
#include <editeng/editeng.hxx>

namespace sd
{
struct RenderOptions
{
    bool mbIncludeBackground = true;
    bool mbSkipMainPageObjects = false;
    bool mbSkipMasterPageObjects = false;
};

struct RenderContext
{
    SdrModel& mrModel;
    SdrPage& mrPage;

    EEControlBits mnSavedControlBits;
    ScopedVclPtrInstance<VirtualDevice> maVirtualDevice;

    RenderContext(unsigned char* pBuffer, SdrModel& rModel, SdrPage& rPage, Size const& rSlideSize)
        : mrModel(rModel)
        , mrPage(rPage)
        , maVirtualDevice(DeviceFormat::WITHOUT_ALPHA)
    {
        // Turn of spelling
        SdrOutliner& rOutliner = mrModel.GetDrawOutliner();
        mnSavedControlBits = rOutliner.GetControlWord();
        rOutliner.SetControlWord(mnSavedControlBits & ~EEControlBits::ONLINESPELLING);

        maVirtualDevice->SetBackground(Wallpaper(COL_TRANSPARENT));

        maVirtualDevice->SetOutputSizePixelScaleOffsetAndLOKBuffer(rSlideSize, Fraction(1.0),
                                                                   Point(), pBuffer);
        Size aPageSize(mrPage.GetSize());

        MapMode aMapMode(MapUnit::Map100thMM);
        const Fraction aFracX(rSlideSize.Width(),
                              maVirtualDevice->LogicToPixel(aPageSize, aMapMode).Width());
        aMapMode.SetScaleX(aFracX);

        const Fraction aFracY(rSlideSize.Height(),
                              maVirtualDevice->LogicToPixel(aPageSize, aMapMode).Height());
        aMapMode.SetScaleY(aFracY);

        maVirtualDevice->SetMapMode(aMapMode);
    }

    ~RenderContext()
    {
        // Restore spelling
        SdrOutliner& rOutliner = mrModel.GetDrawOutliner();
        rOutliner.SetControlWord(mnSavedControlBits);
    }
};

namespace
{
class ObjectRedirector : public sdr::contact::ViewObjectContactRedirector
{
protected:
    RenderOptions maOptions;

public:
    ObjectRedirector(RenderOptions const& rOptions)
        : maOptions(rOptions)
    {
    }

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
            return;
        }

        if (maOptions.mbSkipMasterPageObjects && pPage->IsMasterPage())
            return;

        if (maOptions.mbSkipMainPageObjects && !pPage->IsMasterPage())
            return;

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

bool hasEmptyMaster(SdrPage const& rPage)
{
    if (!rPage.TRG_HasMasterPage())
        return true;

    SdrPage& rMaster = rPage.TRG_GetMasterPage();
    for (size_t i = 0; i < rMaster.GetObjCount(); i++)
    {
        auto pObject = rMaster.GetObj(i);
        if (!pObject->IsEmptyPresObj())
            return false;
    }
    return true;
}

} // end anonymous namespace

SlideshowLayerRenderer::SlideshowLayerRenderer(SdrPage& rPage)
    : mrPage(rPage)
    , mrModel(rPage.getSdrModelFromSdrPage())
{
    if (!hasEmptyMaster(rPage))
        maRenderStages.emplace_back(SlideRenderStage::Master);
    maRenderStages.emplace_back(SlideRenderStage::Slide);
}

Size SlideshowLayerRenderer::calculateAndSetSizePixel(Size const& rDesiredSizePixel)
{
    double fRatio = double(mrPage.GetHeight()) / mrPage.GetWidth();
    Size aSize(rDesiredSizePixel.Width(), ::tools::Long(rDesiredSizePixel.Width() * fRatio));
    maSlideSize = aSize;

    return maSlideSize;
}

void SlideshowLayerRenderer::createViewAndDraw(RenderContext& rRenderContext,
                                               RenderOptions const& rRenderOptions)
{
    SdrView aView(mrModel, rRenderContext.maVirtualDevice);
    aView.SetPageVisible(false);
    aView.SetPageShadowVisible(false);
    aView.SetPageBorderVisible(false);
    aView.SetBordVisible(false);
    aView.SetGridVisible(false);
    aView.SetHlplVisible(false);
    aView.SetGlueVisible(false);
    aView.setHideBackground(!rRenderOptions.mbIncludeBackground);
    aView.ShowSdrPage(&mrPage);

    Size aPageSize(mrPage.GetSize());
    Point aPoint;

    vcl::Region aRegion(::tools::Rectangle(aPoint, aPageSize));
    ObjectRedirector aRedirector(rRenderOptions);
    aView.CompleteRedraw(rRenderContext.maVirtualDevice, aRegion, &aRedirector);
}

bool SlideshowLayerRenderer::renderMaster(unsigned char* pBuffer, OString& rJsonMsg)
{
    RenderOptions aRenderOptions;
    aRenderOptions.mbSkipMainPageObjects = true;

    RenderContext aRenderContext(pBuffer, mrModel, mrPage, maSlideSize);
    createViewAndDraw(aRenderContext, aRenderOptions);

    ::tools::JsonWriter aJsonWriter;
    aJsonWriter.put("group", "MasterPage");
    aJsonWriter.put("slideHash", GetInterfaceHash(GetXDrawPageForSdrPage(&mrPage)));
    aJsonWriter.put("index", 0);
    aJsonWriter.put("type", "bitmap");
    {
        ::tools::ScopedJsonWriterNode aContentNode = aJsonWriter.startNode("content");
        aJsonWriter.put("type", "%IMAGETYPE%");
        aJsonWriter.put("checksum", "%IMAGECHECKSUM%");
    }
    rJsonMsg = aJsonWriter.finishAndGetAsOString();

    return true;
}

bool SlideshowLayerRenderer::renderSlide(unsigned char* pBuffer, OString& rJsonMsg)
{
    RenderOptions aRenderOptions;
    aRenderOptions.mbSkipMasterPageObjects = true;

    RenderContext aRenderContext(pBuffer, mrModel, mrPage, maSlideSize);
    createViewAndDraw(aRenderContext, aRenderOptions);

    ::tools::JsonWriter aJsonWriter;
    aJsonWriter.put("group", "DrawPage");
    aJsonWriter.put("slideHash", GetInterfaceHash(GetXDrawPageForSdrPage(&mrPage)));
    aJsonWriter.put("index", 0);
    aJsonWriter.put("type", "bitmap");
    {
        ::tools::ScopedJsonWriterNode aContentNode = aJsonWriter.startNode("content");
        aJsonWriter.put("type", "%IMAGETYPE%");
        aJsonWriter.put("checksum", "%IMAGECHECKSUM%");
    }
    rJsonMsg = aJsonWriter.finishAndGetAsOString();

    return true;
}

bool SlideshowLayerRenderer::render(unsigned char* pBuffer, OString& rJsonMsg)
{
    if (maRenderStages.empty())
        return false;

    auto eRenderStage = maRenderStages.front();
    maRenderStages.pop_front();

    switch (eRenderStage)
    {
        case SlideRenderStage::Master:
            return renderMaster(pBuffer, rJsonMsg);
        case SlideRenderStage::Slide:
            return renderSlide(pBuffer, rJsonMsg);
    };

    return true;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

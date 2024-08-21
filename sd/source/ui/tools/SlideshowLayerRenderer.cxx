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
        // Turn off spelling
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
bool hasFields(SdrObject* pObject)
{
    auto* pTextObject = dynamic_cast<SdrTextObj*>(pObject);
    if (!pTextObject)
        return false;

    OutlinerParaObject* pOutlinerParagraphObject = pTextObject->GetOutlinerParaObject();
    if (pOutlinerParagraphObject)
    {
        const EditTextObject& rEditText = pOutlinerParagraphObject->GetTextObject();
        if (rEditText.IsFieldObject())
            return true;
    }
    return false;
}

/** VOC redirector to control which object should be rendered and which not */
class ObjectRedirector : public sdr::contact::ViewObjectContactRedirector
{
protected:
    RenderState& mrRenderState;

public:
    ObjectRedirector(RenderState& rRenderState)
        : mrRenderState(rRenderState)
    {
    }

    virtual void createRedirectedPrimitive2DSequence(
        const sdr::contact::ViewObjectContact& rOriginal,
        const sdr::contact::DisplayInfo& rDisplayInfo,
        drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) override
    {
        if (mrRenderState.mbSkipAllInThisPass)
            return;

        SdrObject* pObject = rOriginal.GetViewContact().TryToGetSdrObject();
        // Check if we are rendering an object that is valid to render (exists, and not empty)
        if (pObject == nullptr || pObject->IsEmptyPresObj())
            return;

        SdrPage* pPage = pObject->getSdrPageFromSdrObject();
        // Does the object have a page
        if (pPage == nullptr)
            return;

        // is the object visible and not hidden by any option
        const bool bVisible
            = pObject->getSdrPageFromSdrObject()->checkVisibility(rOriginal, rDisplayInfo, true);

        if (!bVisible)
            return;

        // Check if we have already rendered the object
        if (mrRenderState.isObjectAlreadyRendered(pObject))
            return;

        // Check if we are in correct stage
        if (mrRenderState.meStage == RenderStage::Master && !pPage->IsMasterPage())
        {
            if (mrRenderState.mbFirstObjectInPass)
            {
                // if this is the first object - change from master to slide
                // means we are done with rendering of master layers
                mrRenderState.meStage = RenderStage::Slide;
            }
            else
            {
                // if not, we have to stop rendering all further objects
                mrRenderState.mbSkipAllInThisPass = true;
                return;
            }
        }

        if (mrRenderState.meStage == RenderStage::Master && hasFields(pObject)
            && mrRenderState.mbStopRenderingWhenField && !mrRenderState.mbFirstObjectInPass)
        {
            mrRenderState.mbStopRenderingWhenField = false;
            mrRenderState.mbSkipAllInThisPass = true;
            return;
        }

        // render the object
        sdr::contact::ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(
            rOriginal, rDisplayInfo, rVisitor);

        mrRenderState.mbFirstObjectInPass = false;
        mrRenderState.maObjectsDone.insert(pObject);
        mrRenderState.mbPassHasOutput = true;
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
        maRenderState.meStage = RenderStage::Master;
    else
        maRenderState.meStage = RenderStage::Slide;
}

Size SlideshowLayerRenderer::calculateAndSetSizePixel(Size const& rDesiredSizePixel)
{
    double fRatio = double(mrPage.GetHeight()) / mrPage.GetWidth();
    Size aSize(rDesiredSizePixel.Width(), ::tools::Long(rDesiredSizePixel.Width() * fRatio));
    maSlideSize = aSize;

    return maSlideSize;
}

void SlideshowLayerRenderer::createViewAndDraw(RenderContext& rRenderContext)
{
    SdrView aView(mrModel, rRenderContext.maVirtualDevice);
    aView.SetPageVisible(false);
    aView.SetPageShadowVisible(false);
    aView.SetPageBorderVisible(false);
    aView.SetBordVisible(false);
    aView.SetGridVisible(false);
    aView.SetHlplVisible(false);
    aView.SetGlueVisible(false);
    aView.setHideBackground(!maRenderState.includeBackground());
    aView.ShowSdrPage(&mrPage);

    Size aPageSize(mrPage.GetSize());
    Point aPoint;

    vcl::Region aRegion(::tools::Rectangle(aPoint, aPageSize));
    ObjectRedirector aRedirector(maRenderState);
    aView.CompleteRedraw(rRenderContext.maVirtualDevice, aRegion, &aRedirector);
}

void SlideshowLayerRenderer::writeJSON(OString& rJsonMsg)
{
    ::tools::JsonWriter aJsonWriter;
    aJsonWriter.put("group", maRenderState.stageString());
    aJsonWriter.put("index", maRenderState.currentIndex());
    aJsonWriter.put("slideHash", GetInterfaceHash(GetXDrawPageForSdrPage(&mrPage)));

    aJsonWriter.put("type", "bitmap");
    {
        ::tools::ScopedJsonWriterNode aContentNode = aJsonWriter.startNode("content");
        aJsonWriter.put("type", "%IMAGETYPE%");
        aJsonWriter.put("checksum", "%IMAGECHECKSUM%");
    }
    rJsonMsg = aJsonWriter.finishAndGetAsOString();

    maRenderState.incrementIndex();
}

bool SlideshowLayerRenderer::render(unsigned char* pBuffer, OString& rJsonMsg)
{
    // Reset state
    maRenderState.resetPass();

    RenderContext aRenderContext(pBuffer, mrModel, mrPage, maSlideSize);
    createViewAndDraw(aRenderContext);

    // Check if we are done rendering all passes
    if (maRenderState.noMoreOutput())
        return false;

    writeJSON(rJsonMsg);

    maRenderState.mnCurrentPass++;

    return true;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

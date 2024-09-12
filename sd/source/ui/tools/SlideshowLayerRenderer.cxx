/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SlideshowLayerRenderer.hxx>
#include <CustomAnimationEffect.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdview.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdpagv.hxx>
#include <vcl/virdev.hxx>
#include <tools/json_writer.hxx>
#include <editeng/editeng.hxx>

#include <com/sun/star/animations/XAnimate.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>

#include <animations/animationnodehelper.hxx>
#include <sdpage.hxx>
#include <comphelper/servicehelper.hxx>
#include <svx/unoshape.hxx>

using namespace ::com::sun::star;

namespace sd
{
struct RenderContext
{
    SdrModel& mrModel;

    EEControlBits mnSavedControlBits;
    ScopedVclPtrInstance<VirtualDevice> maVirtualDevice;

    RenderContext(unsigned char* pBuffer, SdrModel& rModel, SdrPage& rPage, Size const& rSlideSize)
        : mrModel(rModel)
        , maVirtualDevice(DeviceFormat::WITHOUT_ALPHA)
    {
        // Turn off spelling
        SdrOutliner& rOutliner = mrModel.GetDrawOutliner();
        mnSavedControlBits = rOutliner.GetControlWord();
        rOutliner.SetControlWord(mnSavedControlBits & ~EEControlBits::ONLINESPELLING);

        maVirtualDevice->SetBackground(Wallpaper(COL_TRANSPARENT));

        maVirtualDevice->SetOutputSizePixelScaleOffsetAndLOKBuffer(rSlideSize, Fraction(1.0),
                                                                   Point(), pBuffer);
        Size aPageSize(rPage.GetSize());

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
        // Generate single pass for background layer
        if (mrRenderState.meStage == RenderStage::Background)
        {
            mrRenderState.mbPassHasOutput = true;
            mrRenderState.mbSkipAllInThisPass = true;
            return;
        }

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

        if (mrRenderState.isObjectInAnimation(pObject))
        {
            // Animated object has to be only one in the render
            mrRenderState.mbSkipAllInThisPass = true;

            // Animated object cannot be attached to the previous object
            if (!mrRenderState.mbFirstObjectInPass)
                return;
        }

        if (mrRenderState.meStage == RenderStage::Master && hasFields(pObject)
            && mrRenderState.mbStopRenderingWhenField && !mrRenderState.mbFirstObjectInPass)
        {
            mrRenderState.mbStopRenderingWhenField = false;
            mrRenderState.mbSkipAllInThisPass = true;
            return;
        }

        mrRenderState.mpCurrentTarget = pObject;

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
    maRenderState.meStage = RenderStage::Background;
    setupAnimations();
}

void SlideshowLayerRenderer::setupAnimations()
{
    auto* pSdPage = dynamic_cast<SdPage*>(&mrPage);

    if (!pSdPage)
        return;

    std::vector<uno::Reference<animations::XAnimationNode>> aAnimationVector;
    anim::create_deep_vector(pSdPage->getAnimationNode(), aAnimationVector);

    for (uno::Reference<animations::XAnimationNode> const& rNode : aAnimationVector)
    {
        switch (rNode->getType())
        {
            // filter out the most obvious
            case animations::AnimationNodeType::CUSTOM:
            case animations::AnimationNodeType::ANIMATE:
            case animations::AnimationNodeType::SET:
            case animations::AnimationNodeType::ANIMATEMOTION:
            case animations::AnimationNodeType::ANIMATECOLOR:
            case animations::AnimationNodeType::ANIMATETRANSFORM:
            case animations::AnimationNodeType::TRANSITIONFILTER:
            case animations::AnimationNodeType::ANIMATEPHYSICS:
            {
                uno::Reference<animations::XAnimate> xAnimate(rNode, uno::UNO_QUERY);
                if (xAnimate.is())
                {
                    uno::Any aAny = xAnimate->getTarget();

                    uno::Reference<drawing::XShape> xShape;
                    SvxShape* pShape = nullptr;
                    SdrObject* pObject = nullptr;

                    if ((aAny >>= xShape) && xShape.is())
                    {
                        pShape = comphelper::getFromUnoTunnel<SvxShape>(xShape);
                        if (pShape)
                        {
                            pObject = pShape->GetSdrObject();
                            maRenderState.maInAnimation.insert(pObject);
                        }
                    }
                    else // if target is not a shape
                    {
                        presentation::ParagraphTarget aParagraphTarget;
                        if ((aAny >>= aParagraphTarget) && aParagraphTarget.Shape.is())
                        {
                            //sal_Int32 nParagraph = aParagraphTarget.Paragraph;

                            xShape = aParagraphTarget.Shape;

                            pShape = comphelper::getFromUnoTunnel<SvxShape>(xShape);
                            if (pShape)
                            {
                                pObject = pShape->GetSdrObject();
                                maRenderState.maInAnimation.insert(pObject);
                            }
                        }
                    }

                    if (pObject)
                    {
                        bool bVisible;

                        if (anim::getVisibilityProperty(xAnimate, bVisible))
                        {
                            // if initial anim sets shape visible, set it
                            // to invisible. If we're asked for the final
                            // state, don't do anything obviously
                            bVisible = !bVisible;

                            maRenderState.maInitiallyVisible[pObject] = bVisible;
                        }

                        if (aAny.hasValue())
                        {
                            OStringBuffer sTmp;
                            anim::convertTarget(sTmp, aAny);
                            maRenderState.maAnimationTargetHash[pObject]
                                = static_cast<OString>(sTmp);
                        }
                    }
                }
            }
        }
    }
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

static void writeContentNode(::tools::JsonWriter& aJsonWriter)
{
    auto aContentNode = aJsonWriter.startNode("content");
    aJsonWriter.put("type", "%IMAGETYPE%");
    aJsonWriter.put("checksum", "%IMAGECHECKSUM%");
}

static void writeBoundingBox(::tools::JsonWriter& aJsonWriter, SdrObject* pObject)
{
    auto aContentNode = aJsonWriter.startNode("bounds");
    ::tools::Rectangle aRectmm100 = pObject->GetCurrentBoundRect();
    ::tools::Rectangle aRect = o3tl::convert(aRectmm100, o3tl::Length::mm100, o3tl::Length::twip);
    aJsonWriter.put("x", aRect.getX());
    aJsonWriter.put("y", aRect.getY());
    aJsonWriter.put("width", aRect.GetWidth());
    aJsonWriter.put("height", aRect.GetHeight());
}

void SlideshowLayerRenderer::writeJSON(OString& rJsonMsg)
{
    ::tools::JsonWriter aJsonWriter;
    aJsonWriter.put("group", maRenderState.stageString());
    aJsonWriter.put("index", maRenderState.currentIndex());
    aJsonWriter.put("slideHash", GetInterfaceHash(GetXDrawPageForSdrPage(&mrPage)));

    SdrObject* pObject = maRenderState.currentTarget();

    bool bIsAnimated = maRenderState.isObjectInAnimation(pObject);
    if (bIsAnimated)
    {
        assert(pObject);
        aJsonWriter.put("type", "animated");
        {
            auto aContentNode = aJsonWriter.startNode("content");
            aJsonWriter.put("hash", maRenderState.maAnimationTargetHash.at(pObject));
            aJsonWriter.put("initVisible", maRenderState.isObjectInitiallyVisible(pObject));
            aJsonWriter.put("type", "bitmap");
            writeContentNode(aJsonWriter);
            writeBoundingBox(aJsonWriter, pObject);
        }
    }
    else
    {
        if (pObject && hasFields(pObject))
            aJsonWriter.put("isField", true); // TODO: to be removed, implement properly
        aJsonWriter.put("type", "bitmap");
        writeContentNode(aJsonWriter);
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

    if (maRenderState.meStage == RenderStage::Background)
        maRenderState.meStage = RenderStage::Master;

    if (hasEmptyMaster(mrPage))
        maRenderState.meStage = RenderStage::Slide;

    return true;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

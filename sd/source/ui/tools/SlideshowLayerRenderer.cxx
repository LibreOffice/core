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
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdpagv.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <svx/unoshape.hxx>

#include <vcl/virdev.hxx>
#include <tools/json_writer.hxx>
#include <editeng/editeng.hxx>
#include <animations/animationnodehelper.hxx>
#include <sdpage.hxx>
#include <comphelper/servicehelper.hxx>

#include <com/sun/star/animations/XAnimate.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <com/sun/star/animations/AnimationNodeType.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/presentation/ParagraphTarget.hpp>

#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/BufferedDecompositionPrimitive2D.hxx>
#include <drawinglayer/primitive2d/texthierarchyprimitive2d.hxx>
#include <drawinglayer/primitive2d/texthierarchyprimitive2d.hxx>

#include <drawinglayer/tools/primitive2dxmldump.hxx>

using namespace ::com::sun::star;

namespace sd
{
struct RenderContext
{
    SdrModel& mrModel;

    EEControlBits mnSavedControlBits;
    ScopedVclPtrInstance<VirtualDevice> maVirtualDevice;

    RenderContext(unsigned char* pBuffer, SdrModel& rModel, SdrPage& rPage, Size const& rSlideSize,
                  const Fraction& rScale)
        : mrModel(rModel)
        , maVirtualDevice(DeviceFormat::WITHOUT_ALPHA)
    {
        // Turn off spelling
        SdrOutliner& rOutliner = mrModel.GetDrawOutliner();
        mnSavedControlBits = rOutliner.GetControlWord();
        rOutliner.SetControlWord(mnSavedControlBits & ~EEControlBits::ONLINESPELLING);

        maVirtualDevice->SetBackground(Wallpaper(COL_TRANSPARENT));

        maVirtualDevice->SetOutputSizePixelScaleOffsetAndLOKBuffer(rSlideSize, rScale, Point(),
                                                                   pBuffer);
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

void changePolyPolys(drawinglayer::primitive2d::Primitive2DContainer& rContainer,
                     bool bRenderObject)
{
    for (auto& pBasePrimitive : rContainer)
    {
        if (pBasePrimitive->getPrimitive2DID() == PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D
            || pBasePrimitive->getPrimitive2DID() == PRIMITIVE2D_ID_POLYPOLYGONGRADIENTPRIMITIVE2D
            || pBasePrimitive->getPrimitive2DID() == PRIMITIVE2D_ID_POLYPOLYGONGRAPHICPRIMITIVE2D
            || pBasePrimitive->getPrimitive2DID() == PRIMITIVE2D_ID_POLYPOLYGONHATCHPRIMITIVE2D
            || pBasePrimitive->getPrimitive2DID() == PRIMITIVE2D_ID_POLYPOLYGONHAIRLINEPRIMITIVE2D)
        {
            pBasePrimitive->setVisible(bRenderObject);
        }
    }
}

void changeBackground(drawinglayer::primitive2d::Primitive2DContainer const& rContainer,
                      bool bRenderObject)
{
    for (size_t i = 0; i < rContainer.size(); i++)
    {
        drawinglayer::primitive2d::BasePrimitive2D* pBasePrimitive = rContainer[i].get();
        if (pBasePrimitive->getPrimitive2DID() == PRIMITIVE2D_ID_SDRRECTANGLEPRIMITIVE2D)
        {
            drawinglayer::primitive2d::Primitive2DContainer aPrimitiveContainer;
            pBasePrimitive->get2DDecomposition(aPrimitiveContainer,
                                               drawinglayer::geometry::ViewInformation2D());
            changePolyPolys(aPrimitiveContainer, bRenderObject);
        }
    }
}

drawinglayer::primitive2d::TextHierarchyBlockPrimitive2D*
findTextBlock(drawinglayer::primitive2d::Primitive2DContainer const& rContainer,
              drawinglayer::geometry::ViewInformation2D const& rViewInformation2D)
{
    for (size_t i = 0; i < rContainer.size(); i++)
    {
        drawinglayer::primitive2d::BasePrimitive2D* pBasePrimitive = rContainer[i].get();
        if (pBasePrimitive->getPrimitive2DID() == PRIMITIVE2D_ID_TEXTHIERARCHYBLOCKPRIMITIVE2D)
        {
            auto* pPrimitive
                = dynamic_cast<drawinglayer::primitive2d::TextHierarchyBlockPrimitive2D*>(
                    pBasePrimitive);
            if (pPrimitive)
                return pPrimitive;
        }
        else
        {
            auto* pGroupPrimitive
                = dynamic_cast<drawinglayer::primitive2d::GroupPrimitive2D*>(pBasePrimitive);
            if (pGroupPrimitive)
            {
                auto* pTextBlock
                    = findTextBlock(pGroupPrimitive->getChildren(), rViewInformation2D);
                if (pTextBlock)
                    return pTextBlock;
            }

            auto* pBufferedPrimitive
                = dynamic_cast<drawinglayer::primitive2d::BufferedDecompositionPrimitive2D*>(
                    pBasePrimitive);
            if (pBufferedPrimitive)
            {
                // try to decompose
                drawinglayer::primitive2d::Primitive2DContainer aPrimitiveContainer;
                pBasePrimitive->get2DDecomposition(aPrimitiveContainer, rViewInformation2D);
                auto* pTextBlock = findTextBlock(aPrimitiveContainer, rViewInformation2D);
                if (pTextBlock)
                    return pTextBlock;
            }
        }
    }
    return nullptr;
}

void modifyParagraphs(drawinglayer::primitive2d::Primitive2DContainer& rContainer,
                      drawinglayer::geometry::ViewInformation2D const& rViewInformation2D,
                      std::deque<sal_Int32> const& rPreserveIndices, bool bRenderObject)
{
    auto* pTextBlock = findTextBlock(rContainer, rViewInformation2D);

    if (pTextBlock)
    {
        auto& rPrimitives = const_cast<drawinglayer::primitive2d::Primitive2DContainer&>(
            pTextBlock->getChildren());
        sal_Int32 nIndex = 0;
        for (auto& pPrimitive : rPrimitives)
        {
            if (pPrimitive->getPrimitive2DID() == PRIMITIVE2D_ID_TEXTHIERARCHYPARAGRAPHPRIMITIVE2D)
            {
                auto& pParagraphPrimitive2d
                    = static_cast<drawinglayer::primitive2d::TextHierarchyParagraphPrimitive2D&>(
                        *pPrimitive);

                // find the index
                auto aIterator
                    = std::find(rPreserveIndices.begin(), rPreserveIndices.end(), nIndex);

                // is index in preserve list - if false, hide the primitive
                bool bHideIndex = aIterator == rPreserveIndices.end();

                pParagraphPrimitive2d.setVisible(!bHideIndex);
            }
            nIndex++;
        }

        changeBackground(rContainer, bRenderObject);
    }
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
            mrRenderState.mnRenderedObjectsInPass++;
            mrRenderState.mbSkipAllInThisPass = true;
            return;
        }

        if (mrRenderState.mbSkipAllInThisPass)
            return;

        SdrObject* pObject = rOriginal.GetViewContact().TryToGetSdrObject();

        drawinglayer::geometry::ViewInformation2D const& rViewInformation2D
            = rOriginal.GetObjectContact().getViewInformation2D();

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
            if (mrRenderState.isFirstObjectInPass())
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

        // Paragraph rendering switches
        bool bRenderOtherParagraphs = false;
        std::deque<sal_Int32> nOtherParagraphs;

        // check if object is in animation
        auto aIterator = mrRenderState.maAnimationRenderInfoList.find(pObject);
        if (aIterator != mrRenderState.maAnimationRenderInfoList.end())
        {
            // Animated object has to be only one in the render
            mrRenderState.mbSkipAllInThisPass = true; // skip all next objects

            // Force a new layer
            if (!mrRenderState.isFirstObjectInPass())
                return;

            AnimationRenderInfo aInfo = aIterator->second;

            if (mrRenderState.maParagraphsToRender.empty()
                && !aInfo.maParagraphs.empty()) // we need to render paragraphs
            {
                auto* pTextObject = dynamic_cast<SdrTextObj*>(pObject);
                if (pTextObject)
                {
                    sal_Int32 nNumberOfParagraphs = pTextObject->GetOutlinerParaObject()->Count();

                    for (sal_Int32 nParagraph = 0; nParagraph < nNumberOfParagraphs; ++nParagraph)
                        nOtherParagraphs.push_back(nParagraph);

                    for (sal_Int32 nParagraph : aInfo.maParagraphs)
                    {
                        mrRenderState.maParagraphsToRender.push_back(nParagraph);
                        std::erase(nOtherParagraphs, nParagraph);
                    }
                    bRenderOtherParagraphs = true;
                }
            }
        }
        else if (mrRenderState.meStage == RenderStage::Master && hasFields(pObject)
                 && mrRenderState.mbStopRenderingWhenField && !mrRenderState.isFirstObjectInPass())
        {
            mrRenderState.mbStopRenderingWhenField = false;
            mrRenderState.mbSkipAllInThisPass = true;
            return;
        }

        mrRenderState.mpCurrentTarget = pObject;

        // render the object
        sdr::contact::ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(
            rOriginal, rDisplayInfo, rVisitor);

        if (!mrRenderState.maParagraphsToRender.empty())
        {
            auto rContainer
                = static_cast<drawinglayer::primitive2d::Primitive2DContainer&>(rVisitor);

            if (bRenderOtherParagraphs)
            {
                modifyParagraphs(rContainer, rViewInformation2D, nOtherParagraphs,
                                 true); // render the object
                mrRenderState.mnCurrentTargetParagraph = -1;
            }
            else
            {
                sal_Int32 nParagraph = mrRenderState.maParagraphsToRender.front();
                mrRenderState.maParagraphsToRender.pop_front();

                std::deque<sal_Int32> aPreserveParagraphs{ nParagraph };
                mrRenderState.mnCurrentTargetParagraph = nParagraph;
                // render only the paragraphs
                modifyParagraphs(rContainer, rViewInformation2D, aPreserveParagraphs, false);
            }
        }

        if (mrRenderState.maParagraphsToRender.empty())
        {
            mrRenderState.mnRenderedObjectsInPass++;
            mrRenderState.maObjectsDone.insert(pObject);
        }
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

SdrObject* getObjectForShape(uno::Reference<drawing::XShape> const& xShape)
{
    if (!xShape.is())
        return nullptr;
    SvxShape* pShape = comphelper::getFromUnoTunnel<SvxShape>(xShape);
    if (pShape)
        return pShape->GetSdrObject();
    return nullptr;
}

} // end anonymous namespace

SlideshowLayerRenderer::SlideshowLayerRenderer(SdrPage& rPage)
    : mrPage(rPage)
    , mrModel(rPage.getSdrModelFromSdrPage())
{
    maRenderState.meStage = RenderStage::Background;
    setupAnimations();
}

void SlideshowLayerRenderer::resolveEffect(CustomAnimationEffectPtr const& rEffect)
{
    SdrObject* pObject = nullptr;
    sal_Int32 nParagraph = -1;
    uno::Reference<drawing::XShape> xShape;

    uno::Any aTargetAny(rEffect->getTarget());

    if ((aTargetAny >>= xShape) && xShape.is())
    {
        pObject = getObjectForShape(xShape);
    }
    else // if target is not a shape - could be paragraph target containing a shape
    {
        presentation::ParagraphTarget aParagraphTarget;
        if ((aTargetAny >>= aParagraphTarget) && aParagraphTarget.Shape.is())
        {
            nParagraph = aParagraphTarget.Paragraph;
            pObject = getObjectForShape(aParagraphTarget.Shape);
        }
    }

    if (!pObject)
        return;

    AnimationRenderInfo aAnimationInfo;
    auto aIterator = maRenderState.maAnimationRenderInfoList.find(pObject);
    if (aIterator != maRenderState.maAnimationRenderInfoList.end())
        aAnimationInfo = aIterator->second;

    AnimationLayerInfo aLayerInfo;

    std::optional<bool> bVisible;
    uno::Any aAny
        = rEffect->getProperty(animations::AnimationNodeType::SET, u"Visibility", EValue::To);
    if (aAny.hasValue())
    {
        // if initial anim sets shape visible, set it
        // to invisible. If we're asked for the final
        // state, don't do anything obviously
        bVisible = !anim::getVisibilityPropertyForAny(aAny);
    }
    aLayerInfo.moInitiallyVisible = bVisible;

    OStringBuffer aStringBuffer;
    anim::convertTarget(aStringBuffer, aTargetAny);
    aLayerInfo.msHash = aStringBuffer.makeStringAndClear();

    // We have paragraphs
    if (nParagraph >= 0)
    {
        auto aParagraphIterator = std::find(aAnimationInfo.maParagraphs.begin(),
                                            aAnimationInfo.maParagraphs.end(), nParagraph);

        // Check if paragraph already exists
        if (aParagraphIterator == aAnimationInfo.maParagraphs.end())
        {
            // We have a paragraph, so write the hash for the paragraph
            aAnimationInfo.maParagraphs.push_back(nParagraph);
            aAnimationInfo.maParagraphInfos.emplace(nParagraph, aLayerInfo);
        }
    }
    else
    {
        if (!aAnimationInfo.moObjectInfo)
            aAnimationInfo.moObjectInfo = aLayerInfo;
    }

    maRenderState.maAnimationRenderInfoList[pObject] = std::move(aAnimationInfo);
}

void SlideshowLayerRenderer::setupAnimations()
{
    auto* pSdPage = dynamic_cast<SdPage*>(&mrPage);

    if (!pSdPage)
        return;

    auto const& rMain = pSdPage->getMainSequence();

    for (auto const& rEffect : rMain->getSequence())
    {
        resolveEffect(rEffect);
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

namespace
{
void writeContentNode(::tools::JsonWriter& aJsonWriter)
{
    auto aContentNode = aJsonWriter.startNode("content");
    aJsonWriter.put("type", "%IMAGETYPE%");
    aJsonWriter.put("checksum", "%IMAGECHECKSUM%");
}

void writeBoundingBox(::tools::JsonWriter& aJsonWriter, SdrObject* pObject)
{
    auto aContentNode = aJsonWriter.startNode("bounds");
    ::tools::Rectangle aRectmm100 = pObject->GetCurrentBoundRect();
    ::tools::Rectangle aRect = o3tl::convert(aRectmm100, o3tl::Length::mm100, o3tl::Length::twip);
    aJsonWriter.put("x", aRect.getX());
    aJsonWriter.put("y", aRect.getY());
    aJsonWriter.put("width", aRect.GetWidth());
    aJsonWriter.put("height", aRect.GetHeight());
}

void writeAnimated(::tools::JsonWriter& aJsonWriter, AnimationLayerInfo const& rLayerInfo,
                   SdrObject* pObject)
{
    aJsonWriter.put("type", "animated");
    {
        bool bInitiallyVisible = true;
        if (rLayerInfo.moInitiallyVisible.has_value())
            bInitiallyVisible = *rLayerInfo.moInitiallyVisible;

        auto aContentNode = aJsonWriter.startNode("content");
        aJsonWriter.put("hash", rLayerInfo.msHash);
        aJsonWriter.put("initVisible", bInitiallyVisible);
        aJsonWriter.put("type", "bitmap");
        writeContentNode(aJsonWriter);
        writeBoundingBox(aJsonWriter, pObject);
    }
}

} // end anonymous namespace

void SlideshowLayerRenderer::writeJSON(OString& rJsonMsg)
{
    ::tools::JsonWriter aJsonWriter;
    aJsonWriter.put("group", maRenderState.stageString());
    aJsonWriter.put("index", maRenderState.currentIndex());
    aJsonWriter.put("slideHash", GetInterfaceHash(GetXDrawPageForSdrPage(&mrPage)));

    SdrObject* pObject = maRenderState.currentTarget();
    sal_Int32 nParagraph = maRenderState.currentTargetParagraph();

    auto aIterator = maRenderState.maAnimationRenderInfoList.find(pObject);
    if (aIterator != maRenderState.maAnimationRenderInfoList.end())
    {
        AnimationRenderInfo& rInfo = aIterator->second;
        assert(pObject);

        if (nParagraph >= 0)
        {
            auto aParagraphInfoIterator = rInfo.maParagraphInfos.find(nParagraph);
            if (aParagraphInfoIterator != rInfo.maParagraphInfos.end())
            {
                writeAnimated(aJsonWriter, aParagraphInfoIterator->second, pObject);
            }
        }
        else if (rInfo.moObjectInfo)
        {
            writeAnimated(aJsonWriter, *rInfo.moObjectInfo, pObject);
        }
        else // No object hash and paragraph hash -> Non-animated part of the object (non-animated paragraphs)
        {
            aJsonWriter.put("type", "bitmap");
            writeContentNode(aJsonWriter);
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

bool SlideshowLayerRenderer::render(unsigned char* pBuffer, double& rScale, OString& rJsonMsg)
{
    // We want to render one pass (one iteration through objects)

    // Reset state for this pass
    maRenderState.resetPass();

    RenderContext aRenderContext(pBuffer, mrModel, mrPage, maSlideSize, Fraction(rScale));
    createViewAndDraw(aRenderContext);

    // Check if we are done rendering all passes and there is no more output
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

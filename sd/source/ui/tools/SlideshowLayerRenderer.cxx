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
#include <drawdoc.hxx>
#include <unokywds.hxx>
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
/// Sets up the virtual device for rendering, and cleans up afterwards
class RenderContext
{
private:
    SdrModel& mrModel;
    SdrPage& mrPage;

    EEControlBits mnSavedControlBits;

public:
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

/// Sets visible for all kinds of polypolys in the container
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

/// Searches for rectangle primitive and changes if the background should be rendered
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

/// Find the text block in the primitive containder, decompose if necessary
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

/// show/hide paragraphs in the container
void modifyParagraphs(drawinglayer::primitive2d::Primitive2DContainer& rContainer,
                      drawinglayer::geometry::ViewInformation2D const& rViewInformation2D,
                      std::deque<sal_Int32> const& rPreserveIndices, bool bRenderObject)
{
    auto* pTextBlock = findTextBlock(rContainer, rViewInformation2D);

    if (pTextBlock)
    {
        auto& rPrimitives = const_cast<drawinglayer::primitive2d::Primitive2DContainer&>(
            pTextBlock->getChildren());
        size_t nIndex = 0;
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

/// Analyze the renderng and create rendering passes
class AnalyzeRenderingRedirector : public sdr::contact::ViewObjectContactRedirector
{
private:
    RenderState& mrRenderState;

    RenderPass* mpCurrentRenderPass;
    RenderStage mePreviousStage = RenderStage::Master;

    // Adds a new rendering pass to the list and returns it
    RenderPass* newRenderPass()
    {
        mrRenderState.maRenderPasses.emplace_back();
        return &mrRenderState.maRenderPasses.back();
    }

    // Closes current rendering pass, and creates a new empty current one
    void closeRenderPass()
    {
        if (mpCurrentRenderPass->maObjectsAndParagraphs.empty())
            return;

        mpCurrentRenderPass = newRenderPass();
    }

    OUString getMasterTextFieldString(SdrObject* pObject)
    {
        OUString aType;

        uno::Reference<drawing::XShape> xShape = pObject->getUnoShape();
        if (!xShape.is())
            return aType;

        OUString sShapeType = xShape->getShapeType();

        if (mrRenderState.mbSlideNumberEnabled
            && sShapeType == u"com.sun.star.presentation.SlideNumberShape")
            aType = u"SlideNumber"_ustr;
        else if (mrRenderState.mbFooterEnabled
                 && sShapeType == u"com.sun.star.presentation.FooterShape")
            aType = u"Footer"_ustr;
        else if (mrRenderState.mbDateTimeEnabled
                 && sShapeType == u"com.sun.star.presentation.DateTimeShape")
            aType = u"DateTime"_ustr;

        return aType;
    }

public:
    AnalyzeRenderingRedirector(RenderState& rRenderState)
        : mrRenderState(rRenderState)
        , mpCurrentRenderPass(newRenderPass())
    {
    }

    void finalizeRenderPasses()
    {
        // Last rendering pass might be empty - delete
        if (mrRenderState.maRenderPasses.back().isEmpty())
            mrRenderState.maRenderPasses.pop_back();

        for (auto& rRenderWork : mrRenderState.maTextFields)
        {
            mrRenderState.maRenderPasses.push_back(rRenderWork);
        }
    }

    virtual void createRedirectedPrimitive2DSequence(
        const sdr::contact::ViewObjectContact& rOriginal,
        const sdr::contact::DisplayInfo& rDisplayInfo,
        drawinglayer::primitive2d::Primitive2DDecompositionVisitor& /*rVisitor*/) override
    {
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

        // Determine the current stage, depending on the page
        RenderStage eCurrentStage
            = pPage->IsMasterPage() ? RenderStage::Master : RenderStage::Slide;

        // We switched from master objecst to slide objects
        if (eCurrentStage == RenderStage::Slide && mePreviousStage == RenderStage::Master)
            closeRenderPass();

        OUString sTextFieldString = getMasterTextFieldString(pObject);

        // check if object is in an animation
        auto aIterator = mrRenderState.maAnimationRenderInfoList.find(pObject);
        if (aIterator != mrRenderState.maAnimationRenderInfoList.end())
        {
            closeRenderPass();

            AnimationRenderInfo aInfo = aIterator->second;

            if (!aInfo.maParagraphs.empty()) // we need to render paragraphs
            {
                auto* pTextObject = dynamic_cast<SdrTextObj*>(pObject);
                if (pTextObject)
                {
                    sal_Int32 nNumberOfParagraphs = pTextObject->GetOutlinerParaObject()->Count();

                    std::deque<sal_Int32> nOtherParagraphs;
                    for (sal_Int32 nParagraph = 0; nParagraph < nNumberOfParagraphs; ++nParagraph)
                    {
                        if (std::find(aInfo.maParagraphs.begin(), aInfo.maParagraphs.end(),
                                      nParagraph)
                            == aInfo.maParagraphs.end())
                            nOtherParagraphs.push_back(nParagraph);
                    }
                    // Add the non-animated part of the object that has animated paragraphs
                    mpCurrentRenderPass->maObjectsAndParagraphs.emplace(pObject, nOtherParagraphs);
                    mpCurrentRenderPass->meStage = eCurrentStage;
                    mpCurrentRenderPass->mbRenderObjectBackground = true;
                    mpCurrentRenderPass->mbAnimation = true;
                    mpCurrentRenderPass->mpObject = pObject;
                    closeRenderPass();

                    // Add all the animated paragraphs
                    for (sal_Int32 nParagraph : aInfo.maParagraphs)
                    {
                        mpCurrentRenderPass->maObjectsAndParagraphs.emplace(
                            pObject, std::deque<sal_Int32>{ nParagraph });
                        mpCurrentRenderPass->meStage = eCurrentStage;
                        mpCurrentRenderPass->mbAnimation = true;
                        mpCurrentRenderPass->mpObject = pObject;
                        mpCurrentRenderPass->mnParagraph = nParagraph;
                        closeRenderPass();
                    }
                }
            }
            else
            {
                // Add the animated object - paragraphs are not animated
                mpCurrentRenderPass->maObjectsAndParagraphs.emplace(pObject,
                                                                    std::deque<sal_Int32>());
                mpCurrentRenderPass->meStage = eCurrentStage;
                mpCurrentRenderPass->mbAnimation = true;
                mpCurrentRenderPass->mpObject = pObject;
                closeRenderPass();
            }
        }
        // Check if the object has slide number, footer, date/time
        else if (eCurrentStage == RenderStage::Master && !sTextFieldString.isEmpty())
        {
            closeRenderPass();

            mpCurrentRenderPass->maObjectsAndParagraphs.emplace(pObject, std::deque<sal_Int32>());
            mpCurrentRenderPass->meStage = eCurrentStage;
            mpCurrentRenderPass->mbPlaceholder = true;
            mpCurrentRenderPass->maFieldType = sTextFieldString;
            mpCurrentRenderPass->mpObject = pObject;
            closeRenderPass();

            RenderPass aTextFieldPass;
            aTextFieldPass.maObjectsAndParagraphs.emplace(pObject, std::deque<sal_Int32>());
            aTextFieldPass.meStage = RenderStage::TextFields;
            aTextFieldPass.maFieldType = sTextFieldString;
            aTextFieldPass.mpObject = pObject;

            mrRenderState.maTextFields.push_back(aTextFieldPass);
        }
        // Check if the object has fields
        else if (eCurrentStage == RenderStage::Master && hasFields(pObject))
        {
            closeRenderPass();

            mpCurrentRenderPass->maObjectsAndParagraphs.emplace(pObject, std::deque<sal_Int32>());
            mpCurrentRenderPass->meStage = eCurrentStage;
            mpCurrentRenderPass->mpObject = pObject;
            closeRenderPass();
        }
        // No specal handling is needed, just add the object to the current rendering pass
        else
        {
            mpCurrentRenderPass->maObjectsAndParagraphs.emplace(pObject, std::deque<sal_Int32>());
            mpCurrentRenderPass->meStage = eCurrentStage;
        }
        mePreviousStage = eCurrentStage;
    }
};

/// Render one render pass
class RenderPassObjectRedirector : public sdr::contact::ViewObjectContactRedirector
{
protected:
    RenderState& mrRenderState;
    RenderPass const& mrRenderPass;

public:
    RenderPassObjectRedirector(RenderState& rRenderState, RenderPass const& rRenderPass)
        : mrRenderState(rRenderState)
        , mrRenderPass(rRenderPass)
    {
    }

    virtual void createRedirectedPrimitive2DSequence(
        const sdr::contact::ViewObjectContact& rOriginal,
        const sdr::contact::DisplayInfo& rDisplayInfo,
        drawinglayer::primitive2d::Primitive2DDecompositionVisitor& rVisitor) override
    {
        SdrObject* pObject = rOriginal.GetViewContact().TryToGetSdrObject();

        if (!pObject)
            return;

        // check if object is in animation
        auto aIterator = mrRenderPass.maObjectsAndParagraphs.find(pObject);
        if (aIterator == mrRenderPass.maObjectsAndParagraphs.end())
            return;

        // render the object
        sdr::contact::ViewObjectContactRedirector::createRedirectedPrimitive2DSequence(
            rOriginal, rDisplayInfo, rVisitor);

        auto const& rParagraphs = aIterator->second;

        // A render pass for the non-animated part of a text shapes whose paragraphs are all animated
        // has no paragraphs (rParagraphs.empty()) anyway it still needs to be modified in order to
        // render the text shape background only; on the contrary it will render all paragraphs.
        if (!rParagraphs.empty() || mrRenderPass.mbRenderObjectBackground)
        {
            auto const& rViewInformation2D = rOriginal.GetObjectContact().getViewInformation2D();
            auto rContainer
                = static_cast<drawinglayer::primitive2d::Primitive2DContainer&>(rVisitor);
            modifyParagraphs(rContainer, rViewInformation2D, rParagraphs,
                             mrRenderPass.mbRenderObjectBackground);
        }
    }
};

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
    setupMasterPageFields();
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

    maRenderState.maAnimationRenderInfoList[pObject] = aAnimationInfo;
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

    for (auto const& rISequence : rMain->getInteractiveSequenceVector())
    {
        for (auto const& rEffect : rISequence->getSequence())
        {
            resolveEffect(rEffect);
        }
    }
}

void SlideshowLayerRenderer::setupMasterPageFields()
{
    auto* pSdPage = dynamic_cast<SdPage*>(&mrPage);

    if (!pSdPage)
        return;

    SdDrawDocument& rDocument(static_cast<SdDrawDocument&>(pSdPage->getSdrModelFromSdrPage()));

    if (rDocument.GetMasterPageCount())
    {
        SdrLayerAdmin& rLayerAdmin = rDocument.GetLayerAdmin();
        SdrLayerIDSet aVisibleLayers = pSdPage->TRG_GetMasterPageVisibleLayers();
        maRenderState.mbShowMasterPageObjects
            = aVisibleLayers.IsSet(rLayerAdmin.GetLayerID(sUNO_LayerName_background_objects));
    }

    if (maRenderState.mbShowMasterPageObjects)
    {
        const sd::HeaderFooterSettings& rSettings = pSdPage->getHeaderFooterSettings();

        if (rSettings.mbFooterVisible && !rSettings.maFooterText.isEmpty())
            maRenderState.mbFooterEnabled = true;

        if (rSettings.mbDateTimeVisible)
        {
            maRenderState.mbDateTimeEnabled = true;

            if (rSettings.mbDateTimeIsFixed && rSettings.maDateTimeText.isEmpty())
                maRenderState.mbDateTimeEnabled = false;
        }

        maRenderState.mbSlideNumberEnabled = rSettings.mbSlideNumberVisible;
    }
}

Size SlideshowLayerRenderer::calculateAndSetSizePixel(Size const& rDesiredSizePixel)
{
    double fRatio = double(mrPage.GetHeight()) / mrPage.GetWidth();
    Size aSize(rDesiredSizePixel.Width(), ::tools::Long(rDesiredSizePixel.Width() * fRatio));
    maSlideSize = aSize;

    return maSlideSize;
}

void SlideshowLayerRenderer::createViewAndDraw(
    RenderContext& rRenderContext, sdr::contact::ViewObjectContactRedirector* pRedirector)
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
    aView.CompleteRedraw(rRenderContext.maVirtualDevice, aRegion, pRedirector);
}

namespace
{
void writeContentNode(::tools::JsonWriter& aJsonWriter)
{
    ::tools::ScopedJsonWriterNode aContentNode = aJsonWriter.startNode("content");
    aJsonWriter.put("type", "%IMAGETYPE%");
    aJsonWriter.put("checksum", "%IMAGECHECKSUM%");
}

void writeBoundingBox(::tools::JsonWriter& aJsonWriter, SdrObject* pObject)
{
    ::tools::ScopedJsonWriterNode aContentNode = aJsonWriter.startNode("bounds");
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

        ::tools::ScopedJsonWriterNode aContentNode = aJsonWriter.startNode("content");
        aJsonWriter.put("hash", rLayerInfo.msHash);
        aJsonWriter.put("initVisible", bInitiallyVisible);
        aJsonWriter.put("type", "bitmap");
        writeContentNode(aJsonWriter);
        writeBoundingBox(aJsonWriter, pObject);
    }
}

} // end anonymous namespace

void SlideshowLayerRenderer::writeBackgroundJSON(OString& rJsonMsg)
{
    ::tools::JsonWriter aJsonWriter;
    aJsonWriter.put("group", maRenderState.stageString());
    aJsonWriter.put("index", maRenderState.currentIndex());
    aJsonWriter.put("slideHash", GetInterfaceHash(GetXDrawPageForSdrPage(&mrPage)));
    aJsonWriter.put("type", "bitmap");
    writeContentNode(aJsonWriter);
    rJsonMsg = aJsonWriter.finishAndGetAsOString();
    maRenderState.incrementIndex();
}

void SlideshowLayerRenderer::writeJSON(OString& rJsonMsg, RenderPass const& rRenderPass)
{
    ::tools::JsonWriter aJsonWriter;
    aJsonWriter.put("group", maRenderState.stageString());
    aJsonWriter.put("index", maRenderState.currentIndex());
    aJsonWriter.put("slideHash", GetInterfaceHash(GetXDrawPageForSdrPage(&mrPage)));

    SdrObject* pObject = rRenderPass.mpObject;
    sal_Int32 nParagraph = rRenderPass.mnParagraph;

    auto aIterator = maRenderState.maAnimationRenderInfoList.find(pObject);
    // Animation object
    if (aIterator != maRenderState.maAnimationRenderInfoList.end())
    {
        AnimationRenderInfo& rInfo = aIterator->second;

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

        if (rRenderPass.mbPlaceholder)
        {
            aJsonWriter.put("type", "placeholder");
            {
                ::tools::ScopedJsonWriterNode aContentNode = aJsonWriter.startNode("content");
                aJsonWriter.put("type", rRenderPass.maFieldType);
            }
        }
        else if (rRenderPass.meStage == RenderStage::TextFields)
        {
            ::tools::ScopedJsonWriterNode aContentNode = aJsonWriter.startNode("content");
            aJsonWriter.put("type", rRenderPass.maFieldType);
            writeContentNode(aJsonWriter);
        }
        else
        {
            aJsonWriter.put("type", "bitmap");
            writeContentNode(aJsonWriter);
        }
    }

    rJsonMsg = aJsonWriter.finishAndGetAsOString();

    maRenderState.incrementIndex();
}

bool SlideshowLayerRenderer::render(unsigned char* pBuffer, OString& rJsonMsg)
{
    // We want to render one pass (one iteration through objects)

    RenderContext aRenderContext(pBuffer, mrModel, mrPage, maSlideSize);

    // Render Background and analyze other passes
    if (maRenderState.meStage == RenderStage::Background)
    {
        // Render no objects, just the background, but analyze and create rendering passes
        AnalyzeRenderingRedirector aRedirector(maRenderState);
        createViewAndDraw(aRenderContext, &aRedirector);
        aRedirector.finalizeRenderPasses();

        // Write JSON for the Background layer
        writeBackgroundJSON(rJsonMsg);

        maRenderState.meStage = RenderStage::Master;
    }
    else
    {
        if (maRenderState.maRenderPasses.empty())
            return false;

        auto const& rRenderPass = maRenderState.maRenderPasses.front();
        maRenderState.meStage = rRenderPass.meStage;
        if (!rRenderPass.mbPlaceholder) // no need to render if placehodler
        {
            RenderPassObjectRedirector aRedirector(maRenderState, rRenderPass);
            createViewAndDraw(aRenderContext, &aRedirector);
        }

        writeJSON(rJsonMsg, rRenderPass);

        maRenderState.maRenderPasses.pop_front();
    }

    return true;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

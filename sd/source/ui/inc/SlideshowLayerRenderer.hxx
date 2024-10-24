/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/string.hxx>
#include <sal/log.hxx>
#include <sddllapi.h>
#include <svx/unoapi.hxx>
#include <tools/gen.hxx>
#include <tools/helpers.hxx>

#include <CustomAnimationEffect.hxx>

#include <deque>
#include <vector>
#include <optional>
#include <unordered_map>
#include <unordered_set>

class SdrPage;
class SdrModel;
class SdrObject;
class Size;

namespace com::sun::star::animations
{
class XAnimate;
}

namespace sdr::contact
{
class ViewObjectContactRedirector;
}

namespace sd
{
class RenderContext;

enum class RenderStage
{
    Background = 0,
    Master = 1,
    Slide = 2,
    TextFields = 3,
};

struct AnimationLayerInfo
{
    OString msHash;
    std::optional<bool> moInitiallyVisible;
};

struct AnimationRenderInfo
{
    std::optional<AnimationLayerInfo> moObjectInfo;
    std::vector<sal_Int32> maParagraphs;
    std::unordered_map<sal_Int32, AnimationLayerInfo> maParagraphInfos;
};

// Holds information used when doing one rendering pass
struct RenderPass
{
    RenderStage meStage = RenderStage::Background;
    std::unordered_map<SdrObject*, std::deque<sal_Int32>> maObjectsAndParagraphs;
    bool mbRenderObjectBackground = false;

    bool mbAnimation = false;
    SdrObject* mpObject = nullptr;
    sal_Int32 mnParagraph = -1;
    bool mbPlaceholder = false;
    OUString maFieldType;

    bool isEmpty() { return maObjectsAndParagraphs.empty(); }
};

/** Holds rendering state, properties and switches through all rendering passes */
struct RenderState
{
    std::deque<RenderPass> maRenderPasses;
    std::vector<RenderPass> maTextFields;

    RenderStage meStage = RenderStage::Background;

    std::unordered_map<SdrObject*, AnimationRenderInfo> maAnimationRenderInfoList;

    std::array<sal_Int32, 4> maIndices = { 0, 0, 0, 0 };

    SdrObject* mpCurrentTarget = nullptr;
    sal_Int32 mnCurrentTargetParagraph = -1;

    bool mbShowMasterPageObjects = false;
    bool mbFooterEnabled = false;
    bool mbDateTimeEnabled = false;
    bool mbSlideNumberEnabled = false;

    /// increments index depending on the current render stage
    void incrementIndex() { maIndices[size_t(meStage)]++; }

    /// returns the current stage as string
    OString stageString() const
    {
        if (meStage == RenderStage::Master)
            return "MasterPage"_ostr;
        else if (meStage == RenderStage::Background)
            return "Background"_ostr;
        else if (meStage == RenderStage::TextFields)
            return "TextFields"_ostr;
        return "DrawPage"_ostr;
    }

    /// returns the current index depending on the current render stage
    sal_Int32 currentIndex() const { return maIndices[size_t(meStage)]; }

    /// should include background in rendering
    bool includeBackground() const { return meStage == RenderStage::Background; }

    static std::string getObjectHash(SdrObject* pObject)
    {
        css::uno::Reference<css::drawing::XShape> xShape = GetXShapeForSdrObject(pObject);
        if (xShape.is())
        {
            css::uno::Reference<css::uno::XInterface> xRef;
            css::uno::Any(xShape) >>= xRef;
            if (xRef.is())
                return GetInterfaceHash(xRef);
        }

        SAL_WARN("sd", "RenderState::getObjectHash: failed");
        return std::string();
    }
};

/** Renders a slide */
class SD_DLLPUBLIC SlideshowLayerRenderer
{
private:
    SdrPage& mrPage;
    SdrModel& mrModel;
    Size maSlideSize;
    RenderState maRenderState;

    void createViewAndDraw(RenderContext& rRenderContext,
                           sdr::contact::ViewObjectContactRedirector* pRedirector);
    void writeBackgroundJSON(OString& rJsonMsg);
    void writeJSON(OString& rJsonMsg, RenderPass const& rRenderPass);

    void setupAnimations();
    void setupMasterPageFields();
    void resolveEffect(CustomAnimationEffectPtr const& rEffect);

public:
    SlideshowLayerRenderer(SdrPage& rPage);

    /** Calculate and set the slide size depending on input desired size (in pixels)
     *
     * Input the desired size in pixels, and the actual size pixels will be calculated
     * depending on the size of the slide and the desired size. The size can differ,
     * because the it must match the slide aspect ratio.
     **/
    Size calculateAndSetSizePixel(Size const& rDesiredSizePixel);

    /** Renders one layer
     *
     * The slide layer is rendered into the input buffer, which must be the byte size
     * of the calculated size in pixels * 4 (RGBA).
     * The properties of the layer are written to the input string in JSON format.
     *
     * @returns false, if nothing was rendered and rendering is done */
    bool render(unsigned char* pBuffer, OString& rJsonMsg);
};

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

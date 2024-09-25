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

namespace sd
{
struct RenderContext;

enum class RenderStage
{
    Background,
    Master,
    Slide,
    TextFields,
    Count
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

/** Holds rendering state, properties and switches through all rendering passes */
struct RenderState
{
    RenderStage meStage = RenderStage::Background;

    bool mbStopRenderingWhenField = true;

    std::unordered_set<SdrObject*> maObjectsDone;

    std::unordered_map<SdrObject*, AnimationRenderInfo> maAnimationRenderInfoList;

    sal_Int32 mnIndex[static_cast<unsigned>(RenderStage::Count)] = { 0, 0, 0, 0 };
    SdrObject* mpCurrentTarget = nullptr;
    sal_Int32 mnCurrentTargetParagraph = -1;

    sal_Int32 mnRenderedObjectsInPass = 0;

    bool mbSkipAllInThisPass = false;

    sal_Int32 mnCurrentPass = 0;

    std::deque<sal_Int32> maParagraphsToRender;

    /// increments index depending on the current render stage
    void incrementIndex() { mnIndex[static_cast<unsigned>(meStage)]++; }

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
    sal_Int32 currentIndex() const { return mnIndex[static_cast<unsigned>(meStage)]; }

    /// returns the current target element for which layer is created if any
    SdrObject* currentTarget() const { return mpCurrentTarget; }

    /// returns the current target paragraph index or -1 if paragraph is not relevant
    sal_Int32 currentTargetParagraph() const { return mnCurrentTargetParagraph; }

    /// resets properties that are valid for one pass
    void resetPass()
    {
        mnRenderedObjectsInPass = 0;
        mbSkipAllInThisPass = false;
        mpCurrentTarget = nullptr;
        mnCurrentTargetParagraph = -1;
    }

    bool hasPassAnyRenderedOutput() const { return mnRenderedObjectsInPass > 0; }

    /// is first rendered object in pass
    bool isFirstObjectInPass() const { return mnRenderedObjectsInPass == 0; }

    /// return if there was no rendering output in the pass
    bool noMoreOutput() const
    {
        // no output and we didn't skip anything and nothing was rendered
        return !hasPassAnyRenderedOutput() && !mbSkipAllInThisPass;
    }

    /// should include background in rendering
    bool includeBackground() const { return meStage == RenderStage::Background; }

    bool isObjectAlreadyRendered(SdrObject* pObject) const
    {
        return maObjectsDone.find(pObject) != maObjectsDone.end();
    }

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

    void createViewAndDraw(RenderContext& rRenderContext);
    void writeJSON(OString& rJsonMsg);

    void setupAnimations();
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

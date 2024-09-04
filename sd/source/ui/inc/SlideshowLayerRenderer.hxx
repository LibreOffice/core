/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sddllapi.h>
#include <tools/gen.hxx>
#include <rtl/string.hxx>
#include <deque>
#include <unordered_set>

class SdrPage;
class SdrModel;
class SdrObject;

class Size;

namespace sd
{
struct RenderContext;

enum class RenderStage
{
    Master,
    Slide
};

/** Holds rendering state, properties and switches through all rendering passes */
struct RenderState
{
    RenderStage meStage = RenderStage::Master;

    sal_Int32 mnMasterIndex = 0;
    bool mbStopRenderingWhenField = true;

    std::unordered_set<SdrObject*> maObjectsDone;
    std::unordered_set<SdrObject*> maInAnimation;
    sal_Int32 mnIndex = 0;
    SdrObject* mpCurrentTarget = nullptr;

    bool mbFirstObjectInPass = true;
    bool mbPassHasOutput = false;
    bool mbSkipAllInThisPass = false;

    sal_Int32 mnCurrentPass = 0;

    /// increments index depending on the current render stage
    void incrementIndex()
    {
        if (meStage == RenderStage::Master)
            mnMasterIndex++;
        else
            mnIndex++;
    }

    /// returns the current stage as string
    OString stageString() const
    {
        if (meStage == RenderStage::Master)
            return "MasterPage"_ostr;
        return "DrawPage"_ostr;
    }

    /// returns the current index depending on the current render stage
    sal_Int32 currentIndex() const
    {
        if (meStage == RenderStage::Master)
            return mnMasterIndex;
        return mnIndex;
    }

    /// returns the current target element for which layer is created if any
    SdrObject* currentTarget() const { return mpCurrentTarget; }

    /// resets properties that are valid for one pass
    void resetPass()
    {
        mbFirstObjectInPass = true;
        mbPassHasOutput = false;
        mbSkipAllInThisPass = false;
        mpCurrentTarget = nullptr;
    }

    /// return if there was no rendering output in the pass
    bool noMoreOutput() const
    {
        // no output and we don't skip anything
        return !mbPassHasOutput && !mbSkipAllInThisPass;
    }

    /// should include background in rendering
    bool includeBackground() const
    {
        // include background only if we are rendering the first pass
        return mnCurrentPass == 0;
    }

    bool isObjectAlreadyRendered(SdrObject* pObject) const
    {
        return maObjectsDone.find(pObject) != maObjectsDone.end();
    }

    bool isObjectInAnimation(SdrObject* pObject) const
    {
        return maInAnimation.find(pObject) != maInAnimation.end();
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

public:
    SlideshowLayerRenderer(SdrPage& rPage);

    /** Calculate and set the slide size depending on input desired size (in pixels)
     *
     * Input the desired size in pixels, and the actual size pixels will be caluclated
     * depending on the size of the slide and the desired size. The size can differ,
     * because the it must match the slide aspect ratio.
     **/
    Size calculateAndSetSizePixel(Size const& rDesiredSizePixel);

    /** Renders one layer
     *
     * The slide layer is rendered into the input buffer, which must be the byte size
     * of the calcualted size in pixels * 4 (RGBA).
     * The properties of the layer are written to the input string in JSON format.
     *
     * @returns false, if nothing was rendered and rendering is done */
    bool render(unsigned char* pBuffer, OString& rJsonMsg);
};

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

class SdrPage;
class SdrModel;

class Size;

namespace sd
{
struct RenderContext;
struct RenderOptions;

enum class SlideRenderStage
{
    Master,
    Slide
};

class SD_DLLPUBLIC SlideshowLayerRenderer
{
    SdrPage& mrPage;
    SdrModel& mrModel;
    Size maSlideSize;

    std::deque<SlideRenderStage> maRenderStages;

    void cleanupRendering(RenderContext& rRenderContext);
    void setupRendering(unsigned char* pBuffer, RenderContext& rRenderContext);
    void createViewAndDraw(RenderContext& rRenderContext, RenderOptions const& rRenderOptions);

public:
    SlideshowLayerRenderer(SdrPage& rPage);
    Size calculateAndSetSizePixel(Size const& rDesiredSizePixel);
    bool render(unsigned char* pBuffer, OString& rJsonMsg);
    bool renderMaster(unsigned char* pBuffer, OString& rJsonMsg);
    bool renderSlide(unsigned char* pBuffer, OString& rJsonMsg);
};

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

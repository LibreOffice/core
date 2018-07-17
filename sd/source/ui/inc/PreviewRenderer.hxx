/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SD_SOURCE_UI_INC_PREVIEWRENDERER_HXX
#define INCLUDED_SD_SOURCE_UI_INC_PREVIEWRENDERER_HXX

#include "drawview.hxx"
#include <vcl/image.hxx>
#include <vcl/virdev.hxx>
#include <svl/listener.hxx>
#include <memory>

class OutputDevice;
class SdPage;
class VirtualDevice;

namespace sd {

class DrawDocShell;
class DrawView;

class PreviewRenderer
    : public SfxListener
{
public:
    /** Create a new preview renderer that takes some of its initial values
        from the given output device.
        @param bPaintFrame
            When <TRUE/> (the default) then a frame is painted around the
            preview.  This makes the actual preview smaller.
    */
    PreviewRenderer(const bool bPaintFrame = true);

    virtual ~PreviewRenderer() override;

    /** Render a page with the given pixel size.
        Use this version when only the width of the preview is known to the
        caller. The height is then calculated according to the aspect
        ratio of the given page.
        @param pPage
            The page to render.
        @param nWidth
            The width of the preview in device coordinates.
         The high contrast mode of the application is
         ignored and the preview is rendered in normal mode.
    */
    Image RenderPage (
        const SdPage* pPage,
        const sal_Int32 nWidth);

    /** Render a page with the given pixel size.
        @param pPage
            The page to render.
        @param aPreviewPixelSize
            The size in device coordinates of the preview.
        @param bObeyHighContrastMode
            When <FALSE/> then the high contrast mode of the application is
            ignored and the preview is rendered in normal mode.  When
            <TRUE/> and high contrast mode is active then the preview is
            rendered in high contrast mode.
        @param bDisplayPresentationObjects
            When <FALSE/> then the PresObj place holders are not displayed
            in the returned preview.
    */
    Image RenderPage (
        const SdPage* pPage,
        const Size aPreviewPixelSize,
        const bool bObeyHighContrastMode,
        const bool bDisplayPresentationObjects = true);

    /** Render an image that contains the given substitution text instead of a
        slide preview.
        @param aPreviewPixelSize
            The size in device coordinates of the image.
    */
    Image RenderSubstitution (
        const Size& rPreviewPixelSize,
        const OUString& sSubstitutionText);

    /** Scale the given bitmap by keeping its aspect ratio to the desired
        width.  Add a frame to it afterwards.
    */
    Image ScaleBitmap (
        const BitmapEx& rBitmap,
        int nWidth);

protected:
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

private:
    ScopedVclPtr<VirtualDevice> mpPreviewDevice;
    ::std::unique_ptr<DrawView> mpView;
    DrawDocShell* mpDocShellOfView;
    const Color maFrameColor;
    const bool mbHasFrame;
    static const int snSubstitutionTextSize;
    // Width of the frame that is painted around the preview.
    static const int snFrameWidth;

    bool Initialize (
        const SdPage* pPage,
        const Size& rPixelSize,
        const bool bObeyHighContrastMode);
    void PaintPage (
        const SdPage* pPage,
        const bool bDisplayPresentationObjects);
    void PaintSubstitutionText (const OUString& rSubstitutionText);
    void PaintFrame();

    /** Set up the map mode so that the given page is renderer into a bitmap
        with the specified width.
        @param rPage
            The page for which the preview is created.
        @param rPixelSize
            The size of the resulting preview bitmap.  Note that this size
            includes the frame.  The actual preview is smaller accordingly.
     */
    void SetupOutputSize (const SdPage& rPage, const Size& rPixelSize);

    /** When mpView is empty then create a new view and initialize it.
        Otherwise just initialize it.
    */
    void ProvideView (DrawDocShell* pDocShell);
};

} // end of namespace ::sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

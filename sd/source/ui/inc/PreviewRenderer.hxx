/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SD_PREVIEW_RENDERER_HXX
#define SD_PREVIEW_RENDERER_HXX

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
    /** Create a new preview renderer that takes some of its inital values
        from the given output device.
        @param pTemplate
            May be NULL.
        @param bPaintFrame
            When <TRUE/> (the default) then a frame is painted around the
            preview.  This makes the actual preview smaller.
    */
    PreviewRenderer (
        OutputDevice* pTemplate = NULL,
        const bool bPaintFrame = true);

    ~PreviewRenderer (void);

    /** Render a page with the given pixel size.
        Use this version when only the width of the preview is known to the
        caller.  The height is then calculated according to the aspect
        ration of the given page.
        @param pPage
            The page to render.
        @param nWidth
            The width of the preview in device coordinates.
        @param sSubstitutionText
            When the actual preview can not be created for some reason, then
            this text is painted in an empty rectangle of the requested size
            instead.
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
        const sal_Int32 nWidth,
        const String& sSubstitutionText,
        const bool bObeyHighContrastMode = true,
        const bool bDisplayPresentationObjects = true);

    /** Render a page with the given pixel size.
        @param pPage
            The page to render.
        @param aPreviewPixelSize
            The size in device coordinates of the preview.
        @param sSubstitutionText
            When the actual preview can not be created for some reason, then
            this text is painted in an empty rectangle of the requested size
            instead.
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
        const String& sSubstitutionText,
        const bool bObeyHighContrastMode = true,
        const bool bDisplayPresentationObjects = true);

    /** Render an image that contains the given substitution text instead of a
        slide preview.
        @param aPreviewPixelSize
            The size in device coordinates of the image.
    */
    Image RenderSubstitution (
        const Size& rPreviewPixelSize,
        const String& sSubstitutionText);

    /** Scale the given bitmap by keeping its aspect ratio to the desired
        width.  Add a frame to it afterwards.
    */
    Image ScaleBitmap (
        const BitmapEx& rBitmap,
        int nWidth);

protected:
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

private:
    ::std::auto_ptr<VirtualDevice> mpPreviewDevice;
    ::std::auto_ptr<DrawView> mpView;
    DrawDocShell* mpDocShellOfView;
    int mnWidthOfView;
    const Color maFrameColor;
    const bool mbHasFrame;
    static const int snSubstitutionTextSize;
    // Width of the frame that is painted arround the preview.
    static const int snFrameWidth;

    bool Initialize (
        const SdPage* pPage,
        const Size& rPixelSize,
        const bool bObeyHighContrastMode);
    void Cleanup (void);
    void PaintPage (
        const SdPage* pPage,
        const bool bDisplayPresentationObjects);
    void PaintSubstitutionText (const String& rSubstitutionText);
    void PaintFrame (void);

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

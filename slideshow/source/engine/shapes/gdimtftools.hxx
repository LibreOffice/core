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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_GDIMTFTOOLS_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_GDIMTFTOOLS_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>

#include <basegfx/range/b2drectangle.hxx>

#include <vcl/virdev.hxx>

#include <tools.hxx>

#include <utility>
#include <vector>

class MetaAction;
class GDIMetaFile;
class Graphic;


namespace slideshow::internal
    {
        /// meta file loading specialities:
        enum mtf_load_flags {
            /// no flags
            MTF_LOAD_NONE = 0,
            /// the source of the metafile might be a foreign
            /// application. The metafile is checked against unsupported
            /// content, and, if necessary, returned as a pre-rendered
            /// bitmap.
            MTF_LOAD_FOREIGN_SOURCE = 2,
            /// retrieve a meta file for the page background only
            MTF_LOAD_BACKGROUND_ONLY = 4,
            /// retrieve the drawing layer scroll text metafile
            MTF_LOAD_SCROLL_TEXT_MTF = 8
        };

        // Animation info
        // ==============

        struct MtfAnimationFrame
        {
            MtfAnimationFrame( GDIMetaFileSharedPtr        xMtf,
                               double                      nDuration ) :
                mpMtf(std::move( xMtf )),
                mnDuration( nDuration )
            {
            }

            /// Enables STL algos to be used for duration extraction
            double getDuration() const
            {
                return mnDuration;
            }

            GDIMetaFileSharedPtr    mpMtf;
            double                  mnDuration;
        };

        typedef ::std::vector< MtfAnimationFrame > VectorOfMtfAnimationFrames;


        /** Retrieve a meta file for the given shape

            @param xShape
            XShape to retrieve a metafile for.

            @param xContainingPage
            The page that contains this shape. Needed for proper
            import (currently, the UnoGraphicExporter needs this
            information).

        */
        GDIMetaFileSharedPtr getMetaFile( const css::uno::Reference< css::lang::XComponent >&        xSource,
                                          const css::uno::Reference< css::drawing::XDrawPage >&      xContainingPage,
                                          int                                                        mtfLoadFlags,
                                          const css::uno::Reference< css::uno::XComponentContext >&  rxContext );

        /** Gets the next action offset for iterating meta actions which is most
            often returns 1.
        */
        sal_Int32 getNextActionOffset( MetaAction * pCurrAct );

        /** Extract a vector of animation frames from given Graphic.
            It can be used to extract only a few frames, and can be
            called later to extract more. (If the Animation is big)

            @param o_rFrames
            Resulting vector of animated metafiles

            @param o_rLoopCount
            Number of times the bitmap animation shall be repeated

            @param pGraphic
            Input graphic object, to extract animations from

            @param pVDev, pVDevMask
            Virtual devices. We don't want to create new everytime we load some frames.

            @param nLoadedFrames
            The count of loaded Frames.

            @param nFramesToLoad
            The count of Frames need to be extracted now.
            Bigger nFramesToLoad not result an error, the function will
            stop extracting at the end of the animation anyway.
         */

        bool getAnimationFromGraphic(VectorOfMtfAnimationFrames& o_rFrames,
                                     sal_uInt32&                 o_rLoopCount,
                                     std::shared_ptr<Graphic>    pGraphic,
                                     ScopedVclPtrInstance<VirtualDevice> &pVDev,
                                     ScopedVclPtrInstance<VirtualDevice> &pVDevMask,
                                     sal_uInt16&                 nLoadedFrames,
                                     sal_uInt16                  nFramesToLoad);

        /** Retrieve scroll text animation rectangles from given metafile

            @return true, if both rectangles have been found, false
            otherwise.
         */
        bool getRectanglesFromScrollMtf( ::basegfx::B2DRectangle&       o_rScrollRect,
                                         ::basegfx::B2DRectangle&       o_rPaintRect,
                                         const GDIMetaFileSharedPtr&    rMtf );
}

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_GDIMTFTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

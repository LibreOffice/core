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
#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_PROCESSOR2DTOOLS_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_PROCESSOR2DTOOLS_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <vcl/bitmapex.hxx>
#include <memory>

namespace drawinglayer::geometry { class ViewInformation2D; }
namespace drawinglayer::processor2d { class BaseProcessor2D; }

class OutputDevice;

namespace drawinglayer::processor2d
    {
        /** create the best available pixel based BaseProcessor2D
            (which may be system-dependent) for a given pixel size
            and format

            @param rViewInformation2D
            The ViewInformation to use

            @param nPixelWidth, nPixelHeight
            The dimensions in Pixels

            @param bUseRGBA
            Define RGBA (true) or RGB (false)

            @return
            the created BaseProcessor2D OR empty (ownership change)
        */
        DRAWINGLAYER_DLLPUBLIC std::unique_ptr<BaseProcessor2D> createPixelProcessor2DFromScratch(
            const drawinglayer::geometry::ViewInformation2D& rViewInformation2D,
            sal_uInt32 nPixelWidth,
            sal_uInt32 nPixelHeight,
            bool bUseRGBA);

        /** create the best available pixel based BaseProcessor2D
            (which may be system-dependent)

            @param rTargetOutDev
            The target OutputDevice

            @param rViewInformation2D
            The ViewInformation to use

            @return
            the created BaseProcessor2D (ownership change)
        */
        DRAWINGLAYER_DLLPUBLIC std::unique_ptr<BaseProcessor2D> createPixelProcessor2DFromOutputDevice(
            OutputDevice& rTargetOutDev,
            const drawinglayer::geometry::ViewInformation2D& rViewInformation2D);

        /** create a BaseProcessor2D dependent on some states of the
            given OutputDevice. If metafile is recorded, the needed
            VclMetafileProcessor2D will be created. If a pixel renderer
            is requested, the best one is incarnated

            @param rTargetOutDev
            The target OutputDevice

            @param rViewInformation2D
            The ViewInformation to use

            @return
            the created BaseProcessor2D (ownership change)
        */
        DRAWINGLAYER_DLLPUBLIC std::unique_ptr<BaseProcessor2D> createProcessor2DFromOutputDevice(
            OutputDevice& rTargetOutDev,
            const drawinglayer::geometry::ViewInformation2D& rViewInformation2D);

        /** extract the pixel data from a given BaseProcessor2D to
            a BitmapEx. This may fail due to maybe system-dependent

            @param rProcessor
            A unique_ptr to a BaseProcessor2D from which to extract

            @return
            a BitmapEx, may be empty, so check result
        */
        DRAWINGLAYER_DLLPUBLIC BitmapEx extractBitmapExFromBaseProcessor2D(const std::unique_ptr<BaseProcessor2D>& rProcessor);


} // end of namespace drawinglayer::processor2d

#endif //INCLUDED_DRAWINGLAYER_PROCESSOR2D_PROCESSOR2DTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

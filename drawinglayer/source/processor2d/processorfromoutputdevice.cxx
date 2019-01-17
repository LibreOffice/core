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

#include <vcl/outdev.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2drange.hxx>
#include <vcl/gdimtf.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>
#include "vclmetafileprocessor2d.hxx"
#include "vclpixelprocessor2d.hxx"
#include <vcl/window.hxx>

using namespace com::sun::star;

namespace drawinglayer
{
    namespace processor2d
    {
        std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> createBaseProcessor2DFromOutputDevice(
            OutputDevice& rTargetOutDev,
            const drawinglayer::geometry::ViewInformation2D& rViewInformation2D)
        {
            const GDIMetaFile* pMetaFile = rTargetOutDev.GetConnectMetaFile();
            const bool bOutputToRecordingMetaFile(pMetaFile && pMetaFile->IsRecord() && !pMetaFile->IsPause());

            if(bOutputToRecordingMetaFile)
            {
                // create MetaFile Vcl-Processor and process
                return std::make_unique<drawinglayer::processor2d::VclMetafileProcessor2D>(rViewInformation2D, rTargetOutDev);
            }
            else
            {
                // create Pixel Vcl-Processor
                return std::make_unique<drawinglayer::processor2d::VclPixelProcessor2D>(rViewInformation2D, rTargetOutDev);
            }
        }
    } // end of namespace processor2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

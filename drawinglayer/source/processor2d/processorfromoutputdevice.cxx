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
#include <basegfx/tools/canvastools.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>
#include <drawinglayer/processor2d/vclmetafileprocessor2d.hxx>
#include <drawinglayer/processor2d/vclpixelprocessor2d.hxx>
#include <drawinglayer/processor2d/canvasprocessor.hxx>
#include <vcl/window.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        drawinglayer::processor2d::BaseProcessor2D* createBaseProcessor2DFromOutputDevice(
            OutputDevice& rTargetOutDev,
            const drawinglayer::geometry::ViewInformation2D& rViewInformation2D)
        {
            const GDIMetaFile* pMetaFile = rTargetOutDev.GetConnectMetaFile();
            const bool bOutputToRecordingMetaFile(pMetaFile && pMetaFile->IsRecord() && !pMetaFile->IsPause());

            if(bOutputToRecordingMetaFile)
            {
                // create MetaFile Vcl-Processor and process
                return new drawinglayer::processor2d::VclMetafileProcessor2D(rViewInformation2D, rTargetOutDev);
            }
            else
            {
#ifdef WIN32
                // for a first AA incarnation VCL-PixelRenderer will be okay since
                // simple (and fast) GDIPlus support over VCL will be used.
                // Leaving the code below as a hint for what to do when we will
                // use canvas renderers in the future

                //static SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;

                //if(false && aSvtOptionsDrawinglayer.IsAntiAliasing())
                //{
                //  // for WIN32 AA, create cairo canvas processor
                //  return new drawinglayer::processor2d::canvasProcessor2D(rViewInformation2D, rTargetOutDev);
                //}
                //else
                //{
                    // create Pixel Vcl-Processor
                    return new drawinglayer::processor2d::VclPixelProcessor2D(rViewInformation2D, rTargetOutDev);
                //}
#else
                static bool bTryTestCanvas(false);

                if(bTryTestCanvas)
                {
                    // create test-cancas-Processor
                    return new drawinglayer::processor2d::canvasProcessor2D(rViewInformation2D, rTargetOutDev);
                }
                else
                {
                    // create Pixel Vcl-Processor
                    return new drawinglayer::processor2d::VclPixelProcessor2D(rViewInformation2D, rTargetOutDev);
                }
#endif
            }
        }
    } // end of namespace processor2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

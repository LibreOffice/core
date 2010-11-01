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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/sdr/contact/objectcontacttools.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2drange.hxx>
#include <vcl/gdimtf.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <drawinglayer/processor2d/vclmetafileprocessor2d.hxx>
#include <drawinglayer/processor2d/vclpixelprocessor2d.hxx>
#include <drawinglayer/processor2d/canvasprocessor.hxx>
#include <vcl/window.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
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
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

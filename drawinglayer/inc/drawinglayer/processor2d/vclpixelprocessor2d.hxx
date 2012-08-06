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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLPIXELPROCESSOR2D_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLPIXELPROCESSOR2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/processor2d/vclprocessor2d.hxx>
#include <vcl/outdev.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        /** VclPixelProcessor2D class

            This processor derived from VclProcessor2D is the base class for rendering
            all feeded primitives to a VCL Window. It is the currently used renderer
            for all VCL editing output from the DrawingLayer.
         */
        class DRAWINGLAYER_DLLPUBLIC VclPixelProcessor2D : public VclProcessor2D
        {
        private:
            /*  The Pixel renderer resets the original MapMode from the OutputDevice.
                For some situations it is necessary to get it again, so it is rescued here
             */
            MapMode                             maOriginalMapMode;

        protected:
            /*  the local processor for BasePrinitive2D-Implementation based primitives,
                called from the common process()-implementation
             */
            virtual void processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate);

        public:
            /// constructor/destructor
            VclPixelProcessor2D(
                const geometry::ViewInformation2D& rViewInformation,
                OutputDevice& rOutDev);
            virtual ~VclPixelProcessor2D();

            /// data read access
            const MapMode& getOriginalMapMode() const { return maOriginalMapMode; }
        };
    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLPIXELPROCESSOR2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

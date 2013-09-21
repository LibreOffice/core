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
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

class OutputDevice;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        /** create the best available pixel based BaseProcessor2D
            (which may be system-dependent)

            @param rTargetOutDev
            The target OutputDevice

            @param rViewInformation2D
            The ViewInformation to use

            @return
            the created BaseProcessor2D (ownership change) or null if
            something went wrong
        */
        DRAWINGLAYER_DLLPUBLIC BaseProcessor2D* createPixelProcessor2DFromOutputDevice(
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
            the created BaseProcessor2D (ownership change) or null if
            something went wrong
        */
        DRAWINGLAYER_DLLPUBLIC BaseProcessor2D* createProcessor2DFromOutputDevice(
            OutputDevice& rTargetOutDev,
            const drawinglayer::geometry::ViewInformation2D& rViewInformation2D);

    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PROCESSOR2D_PROCESSOR2DTOOLS_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

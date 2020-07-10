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

#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_PROCESSORFROMOUTPUTDEVICE_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_PROCESSORFROMOUTPUTDEVICE_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <memory>

class OutputDevice;

namespace drawinglayer::processor2d {
    class BaseProcessor2D;
}

namespace drawinglayer::geometry { class ViewInformation2D; }

namespace drawinglayer::processor2d
    {
        // create a mating VCL-Processor for given OutputDevice. This includes
        // looking for MetaFile-recording. The returned renderer changes owner,
        // deletion is duty of the caller
        DRAWINGLAYER_DLLPUBLIC std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> createBaseProcessor2DFromOutputDevice(
            OutputDevice& rTargetOutDev,
            const drawinglayer::geometry::ViewInformation2D& rViewInformation2D);

} // end of namespace drawinglayer::processor2d

#endif // INCLUDED_DRAWINGLAYER_PROCESSOR2D_PROCESSORFROMOUTPUTDEVICE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

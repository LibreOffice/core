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

#ifndef _PROCESSOR_FROM_OUTPUTDEVICE_HXX
#define _PROCESSOR_FROM_OUTPUTDEVICE_HXX

#include <drawinglayer/geometry/viewinformation2d.hxx>
#include "drawinglayer/drawinglayerdllapi.h"

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class OutputDevice;

namespace drawinglayer { namespace processor2d {
    class BaseProcessor2D;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace processor2d
    {
        // create a mating VCL-Provessor for given OutputDevice. This includes
        // looking for MetaFile-recording. The returned renderer changes owner,
        // deletion is duty of the caller
        DRAWINGLAYER_DLLPUBLIC drawinglayer::processor2d::BaseProcessor2D* createBaseProcessor2DFromOutputDevice(
            OutputDevice& rTargetOutDev,
            const drawinglayer::geometry::ViewInformation2D& rViewInformation2D);

    } // end of namespace processor2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //_PROCESSOR_FROM_OUTPUTDEVICE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

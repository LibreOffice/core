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


#include <svx/sdr/overlay/overlayprimitive2dsequenceobject.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/processor2d/vclpixelprocessor2d.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        drawinglayer::primitive2d::Primitive2DSequence OverlayPrimitive2DSequenceObject::createOverlayObjectPrimitive2DSequence()
        {
            return getSequence();
        }

        OverlayPrimitive2DSequenceObject::OverlayPrimitive2DSequenceObject(const drawinglayer::primitive2d::Primitive2DSequence& rSequence)
        :   OverlayObjectWithBasePosition(basegfx::B2DPoint(), Color(COL_BLACK)),
            maSequence(rSequence)
        {
        }

        OverlayPrimitive2DSequenceObject::~OverlayPrimitive2DSequenceObject()
        {
        }
    } // end of namespace overlay
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

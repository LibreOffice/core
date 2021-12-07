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

#ifndef INCLUDED_SVX_SDR_OVERLAY_OVERLAYPRIMITIVE2DSEQUENCEOBJECT_HXX
#define INCLUDED_SVX_SDR_OVERLAY_OVERLAYPRIMITIVE2DSEQUENCEOBJECT_HXX

#include <svx/sdr/overlay/overlayobject.hxx>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>


namespace sdr::overlay
    {
        class SVXCORE_DLLPUBLIC OverlayPrimitive2DSequenceObject final : public OverlayObjectWithBasePosition
        {
            // the sequence of primitives to show
            const drawinglayer::primitive2d::Primitive2DContainer    maSequence;

            // geometry creation for OverlayObject
            virtual drawinglayer::primitive2d::Primitive2DContainer createOverlayObjectPrimitive2DSequence() override;

        public:
            explicit OverlayPrimitive2DSequenceObject(drawinglayer::primitive2d::Primitive2DContainer&& rSequence);

            virtual ~OverlayPrimitive2DSequenceObject() override;
        };

} // end of namespace sdr::overlay


#endif // INCLUDED_SVX_SDR_OVERLAY_OVERLAYPRIMITIVE2DSEQUENCEOBJECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

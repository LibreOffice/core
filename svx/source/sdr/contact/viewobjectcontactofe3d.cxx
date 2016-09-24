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

#include <sdr/contact/viewobjectcontactofe3d.hxx>
#include <svx/sdr/contact/viewcontactofe3d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <drawinglayer/primitive3d/modifiedcolorprimitive3d.hxx>
#include <sdr/contact/viewobjectcontactofe3dscene.hxx>
#include <drawinglayer/primitive2d/embedded3dprimitive2d.hxx>
#include <svx/sdr/contact/viewcontactofe3dscene.hxx>
#include <comphelper/sequence.hxx>

namespace sdr
{
    namespace contact
    {
        ViewObjectContactOfE3d::ViewObjectContactOfE3d(ObjectContact& rObjectContact, ViewContact& rViewContact)
        :   ViewObjectContactOfSdrObj(rObjectContact, rViewContact)
        {
        }

        ViewObjectContactOfE3d::~ViewObjectContactOfE3d()
        {
        }

        drawinglayer::primitive3d::Primitive3DContainer ViewObjectContactOfE3d::createPrimitive3DContainer(const DisplayInfo& rDisplayInfo) const
        {
            // get the view-independent Primitive from the viewContact
            const ViewContactOfE3d& rViewContactOfE3d(dynamic_cast< const ViewContactOfE3d& >(GetViewContact()));
            drawinglayer::primitive3d::Primitive3DContainer xRetval(rViewContactOfE3d.getViewIndependentPrimitive3DContainer());

            // handle ghosted
            if(isPrimitiveGhosted(rDisplayInfo))
            {
                const ::basegfx::BColor aRGBWhite(1.0, 1.0, 1.0);
                const ::basegfx::BColorModifierSharedPtr aBColorModifier(
                    new basegfx::BColorModifier_interpolate(
                        aRGBWhite,
                        0.5));
                const drawinglayer::primitive3d::Primitive3DReference xReference(
                    new drawinglayer::primitive3d::ModifiedColorPrimitive3D(
                        xRetval,
                        aBColorModifier));

                xRetval = { xReference };
            }

            return xRetval;
        }

        drawinglayer::primitive2d::Primitive2DContainer ViewObjectContactOfE3d::createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const
        {
            const ViewContactOfE3d& rViewContact = static_cast< const ViewContactOfE3d& >(GetViewContact());

            // get 3d primitive vector, isPrimitiveVisible() is done in 3d creator
            return rViewContact.impCreateWithGivenPrimitive3DContainer(getPrimitive3DContainer(rDisplayInfo));
        }

        drawinglayer::primitive3d::Primitive3DContainer const & ViewObjectContactOfE3d::getPrimitive3DContainer(const DisplayInfo& rDisplayInfo) const
        {
            drawinglayer::primitive3d::Primitive3DContainer xNewPrimitive3DSeq(createPrimitive3DContainer(rDisplayInfo));

            // local up-to-date checks. New list different from local one?
            if(mxPrimitive3DContainer != xNewPrimitive3DSeq)
            {
                // has changed, copy content
                const_cast< ViewObjectContactOfE3d* >(this)->mxPrimitive3DContainer = xNewPrimitive3DSeq;
            }

            // return current Primitive2DContainer
            return mxPrimitive3DContainer;
        }
    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <sdr/contact/viewcontactofgroup.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdpage.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <sdr/contact/viewobjectcontactofgroup.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/color/bcolor.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>
#include <tools/debug.hxx>


namespace sdr
{
    namespace contact
    {
        // Create a Object-Specific ViewObjectContact, set ViewContact and
        // ObjectContact. Always needs to return something.
        ViewObjectContact& ViewContactOfGroup::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = new ViewObjectContactOfGroup(rObjectContact, *this);
            DBG_ASSERT(pRetval, "ViewContactOfGroup::CreateObjectSpecificViewObjectContact() failed (!)");

            return *pRetval;
        }

        ViewContactOfGroup::ViewContactOfGroup(SdrObjGroup& rGroup)
        :   ViewContactOfSdrObj(rGroup)
        {
        }

        ViewContactOfGroup::~ViewContactOfGroup()
        {
        }

        drawinglayer::primitive2d::Primitive2DContainer ViewContactOfGroup::createViewIndependentPrimitive2DSequence() const
        {
            drawinglayer::primitive2d::Primitive2DContainer xRetval;
            const sal_uInt32 nObjectCount(GetObjectCount());

            if(nObjectCount)
            {
                // collect all sub-primitives
                for(sal_uInt32 a(0); a < nObjectCount; a++)
                {
                    const ViewContact& rCandidate(GetViewContact(a));
                    const drawinglayer::primitive2d::Primitive2DContainer& aCandSeq(rCandidate.getViewIndependentPrimitive2DContainer());

                    xRetval.insert(xRetval.end(), aCandSeq.begin(), aCandSeq.end());
                }
            }
            else
            {
                // append an invisible outline for the cases where no visible content exists
                const tools::Rectangle aCurrentBoundRect(GetSdrObjGroup().GetLastBoundRect());
                const basegfx::B2DRange aCurrentRange(
                    aCurrentBoundRect.Left(), aCurrentBoundRect.Top(),
                    aCurrentBoundRect.Right(), aCurrentBoundRect.Bottom());

                const drawinglayer::primitive2d::Primitive2DReference xReference(
                    drawinglayer::primitive2d::createHiddenGeometryPrimitives2D(
                        false, aCurrentRange));

                xRetval = drawinglayer::primitive2d::Primitive2DContainer { xReference };
            }

            return xRetval;
        }
    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <svx/sdr/contact/viewobjectcontactofe3d.hxx>
#include <svx/sdr/contact/viewcontactofe3d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <drawinglayer/primitive3d/modifiedcolorprimitive3d.hxx>
#include <svx/sdr/contact/viewobjectcontactofe3dscene.hxx>
#include <drawinglayer/primitive2d/embedded3dprimitive2d.hxx>
#include <svx/sdr/contact/viewcontactofe3dscene.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

//////////////////////////////////////////////////////////////////////////////

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

        drawinglayer::primitive3d::Primitive3DSequence ViewObjectContactOfE3d::createPrimitive3DSequence(const DisplayInfo& rDisplayInfo) const
        {
            // get the view-independent Primitive from the viewContact
            const ViewContactOfE3d& rViewContactOfE3d(dynamic_cast< const ViewContactOfE3d& >(GetViewContact()));
            drawinglayer::primitive3d::Primitive3DSequence xRetval(rViewContactOfE3d.getViewIndependentPrimitive3DSequence());

            // handle ghosted
            if(isPrimitiveGhosted(rDisplayInfo))
            {
                const ::basegfx::BColor aRGBWhite(1.0, 1.0, 1.0);
                const ::basegfx::BColorModifier aBColorModifier(aRGBWhite, 0.5, ::basegfx::BCOLORMODIFYMODE_INTERPOLATE);
                const drawinglayer::primitive3d::Primitive3DReference xReference(new drawinglayer::primitive3d::ModifiedColorPrimitive3D(xRetval, aBColorModifier));
                xRetval = drawinglayer::primitive3d::Primitive3DSequence(&xReference, 1);
            }

            return xRetval;
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContactOfE3d::createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const
        {
            const ViewContactOfE3d& rViewContact = static_cast< const ViewContactOfE3d& >(GetViewContact());

            // get 3d primitive vector, isPrimitiveVisible() is done in 3d creator
            return rViewContact.impCreateWithGivenPrimitive3DSequence(getPrimitive3DSequence(rDisplayInfo));
        }

        drawinglayer::primitive3d::Primitive3DSequence ViewObjectContactOfE3d::getPrimitive3DSequence(const DisplayInfo& rDisplayInfo) const
        {
            drawinglayer::primitive3d::Primitive3DSequence xNewPrimitive3DSeq(createPrimitive3DSequence(rDisplayInfo));

            // local up-to-date checks. New list different from local one?
            if(!drawinglayer::primitive3d::arePrimitive3DSequencesEqual(mxPrimitive3DSequence, xNewPrimitive3DSeq))
            {
                // has changed, copy content
                const_cast< ViewObjectContactOfE3d* >(this)->mxPrimitive3DSequence = xNewPrimitive3DSeq;
            }

            // return current Primitive2DSequence
            return mxPrimitive3DSequence;
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <svx/sdr/overlay/overlayobject.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <vcl/outdev.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        void OverlayObject::objectChange()
        {
            const basegfx::B2DRange aPreviousRange(maBaseRange);
            maBaseRange.reset();
            setPrimitive2DSequence(drawinglayer::primitive2d::Primitive2DSequence());

            if(getOverlayManager() && !aPreviousRange.isEmpty())
            {
                getOverlayManager()->invalidateRange(aPreviousRange);
            }

            const basegfx::B2DRange& rCurrentRange = getBaseRange();

            if(getOverlayManager() && rCurrentRange != aPreviousRange && !rCurrentRange.isEmpty())
            {
                getOverlayManager()->invalidateRange(rCurrentRange);
            }
        }

        // OverlayObject implementations.
        drawinglayer::primitive2d::Primitive2DSequence OverlayObject::createOverlayObjectPrimitive2DSequence()
        {
            // Default implementation has to assert a missing implementation. It cannot
            // be useful to have overlay object derivations which have no visualisation
            // at all
            OSL_FAIL("OverlayObject derivation without visualisation definition (missing createOverlayObjectPrimitive2DSequence implementation) (!)");
            return drawinglayer::primitive2d::Primitive2DSequence();
        }

        void OverlayObject::allowAntiAliase(bool bNew)
        {
            if(bNew != (bool)mbAllowsAntiAliase)
            {
                // remember new value
                mbAllowsAntiAliase = bNew;

                // register change (after change)
                objectChange();
            }
        }

        OverlayObject::OverlayObject(Color aBaseColor)
        :   Event(0),
            mpOverlayManager(0),
            maBaseColor(aBaseColor),
            mbIsVisible(true),
            mbIsHittable(true),
            mbAllowsAnimation(false),
            mbAllowsAntiAliase(true)
        {
        }

        OverlayObject::~OverlayObject()
        {
            OSL_ENSURE(0 == getOverlayManager(), "OverlayObject is destructed which is still registered at OverlayManager (!)");
        }

        drawinglayer::primitive2d::Primitive2DSequence OverlayObject::getOverlayObjectPrimitive2DSequence() const
        {
            if(!getPrimitive2DSequence().hasElements())
            {
                // no existing sequence; create one
                const_cast< OverlayObject* >(this)->setPrimitive2DSequence(
                    const_cast< OverlayObject* >(this)->createOverlayObjectPrimitive2DSequence());
            }

            return getPrimitive2DSequence();
        }

        const basegfx::B2DRange& OverlayObject::getBaseRange() const
        {
            if(getOverlayManager() && maBaseRange.isEmpty())
            {
                const drawinglayer::primitive2d::Primitive2DSequence& rSequence = getOverlayObjectPrimitive2DSequence();

                if(rSequence.hasElements())
                {
                    const drawinglayer::geometry::ViewInformation2D aViewInformation2D(getOverlayManager()->getCurrentViewInformation2D());

                    const_cast< sdr::overlay::OverlayObject* >(this)->maBaseRange =
                        drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(rSequence, aViewInformation2D);
                }
            }

            return maBaseRange;
        }

        void OverlayObject::setVisible(bool bNew)
        {
            if(bNew != (bool)mbIsVisible)
            {
                // remember new value
                mbIsVisible = bNew;

                // register change (after change)
                objectChange();
            }
        }

        void OverlayObject::setHittable(bool bNew)
        {
            if(bNew != (bool)mbIsHittable)
            {
                // remember new value
                mbIsHittable = bNew;

                // register change (after change)
                objectChange();
            }
        }

        void OverlayObject::setBaseColor(Color aNew)
        {
            if(aNew != maBaseColor)
            {
                // remember new value
                maBaseColor = aNew;

                // register change (after change)
                objectChange();
            }
        }

        void OverlayObject::Trigger(sal_uInt32 /*nTime*/)
        {
            // default does not register again
        }

        void OverlayObject::stripeDefinitionHasChanged()
        {
            // default does not need to do anything
        }
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        OverlayObjectWithBasePosition::OverlayObjectWithBasePosition(const basegfx::B2DPoint& rBasePos, Color aBaseColor)
        :   OverlayObject(aBaseColor),
            maBasePosition(rBasePos)
        {
        }

        OverlayObjectWithBasePosition::~OverlayObjectWithBasePosition()
        {
        }

        void OverlayObjectWithBasePosition::setBasePosition(const basegfx::B2DPoint& rNew)
        {
            if(rNew != maBasePosition)
            {
                // remember new value
                maBasePosition = rNew;

                // register change (after change)
                objectChange();
            }
        }
    } // end of namespace overlay
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

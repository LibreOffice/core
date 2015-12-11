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

#ifndef INCLUDED_SVX_SDR_OVERLAY_OVERLAYOBJECT_HXX
#define INCLUDED_SVX_SDR_OVERLAY_OVERLAYOBJECT_HXX

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drange.hxx>
#include <tools/color.hxx>
#include <rtl/ref.hxx>
#include <svx/sdr/animation/scheduler.hxx>
#include <svx/svxdllapi.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

#include <vector>

class OutputDevice;

namespace sdr
{
    namespace overlay
    {
        class OverlayManager;
    } // end of namespace overlay
}

namespace basegfx
{
    class B2DPolygon;
    class B2DPolyPolygon;
    class B2DRange;
}

namespace sdr
{
    namespace overlay
    {
        class SVX_DLLPUBLIC OverlayObject : public sdr::animation::Event
        {
        private:
            OverlayObject(const OverlayObject&) = delete;
            OverlayObject& operator=(const OverlayObject&) = delete;

            // Manager is allowed access to private Member mpOverlayManager
            friend class                                    OverlayManager;

            // pointer to OverlayManager, if object is added. Changed by
            // OverlayManager, do not change Yourself.
            OverlayManager*                                 mpOverlayManager;

            // Primitive2DContainer of the OverlayObject
            drawinglayer::primitive2d::Primitive2DContainer  maPrimitive2DSequence;

        protected:
            // access methods to maPrimitive2DSequence. The usage of this methods may allow
            // later thread-safe stuff to be added if needed. Only to be used by getPrimitive2DSequence()
            // implementations for buffering the last decomposition.
            const drawinglayer::primitive2d::Primitive2DContainer& getPrimitive2DSequence() const { return maPrimitive2DSequence; }
            void setPrimitive2DSequence(const drawinglayer::primitive2d::Primitive2DContainer& rNew) { maPrimitive2DSequence = rNew; }

            // the creation method for Primitive2DContainer. Called when getPrimitive2DSequence()
            // sees that maPrimitive2DSequence is empty. Needs to be supported by all
            // OverlayObject implementations. Default implementation will assert
            // a missing implementation
            virtual drawinglayer::primitive2d::Primitive2DContainer createOverlayObjectPrimitive2DSequence();

            // #i53216# check blink time value range (currently 25 < mnBlinkTime < 10000)
            static sal_uInt32 impCheckBlinkTimeValueRange(sal_uInt64 nBlinkTime);

            // region in logical coordinates
            basegfx::B2DRange                               maBaseRange;

            // base color of this OverlayObject
            Color                                           maBaseColor;

            // bitfield
            // Flag for visibility
            bool                                            mbIsVisible : 1;

            // Flag to control hittability
            bool                                            mbIsHittable : 1;

            // Flag to hold info if this objects supports animation. Default is
            // false. If true, the Trigger() method should be overridden
            // to implement the animation effect and to re-initiate the event.
            bool                                            mbAllowsAnimation : 1;

            // Flag tocontrol if this OverlayObject allows AntiAliased visualisation.
            // Default is true, but e.g. for selection visualisation in SC and SW,
            // it is switched to false
            bool                                            mbAllowsAntiAliase : 1;

            // set changed flag. Call after change, since the old range is invalidated
            // and then the new one is calculated and invalidated, too. This will only
            // work after the change.
            void objectChange();

            // write access to AntiAliase flag. This is protected since
            // only implementations are allowed to change this, preferably in their
            // constructor
            void allowAntiAliase(bool bNew);

        public:
            explicit OverlayObject(Color aBaseColor);
            virtual ~OverlayObject();

            // get OverlayManager
            OverlayManager* getOverlayManager() const { return mpOverlayManager; }

            // the access method for Primitive2DContainer. Will use createPrimitive2DSequence and
            // setPrimitive2DSequence if needed. Overriding may be used to allow disposal of last
            // created primitives to react on changed circumstances and to re-create primitives
            virtual drawinglayer::primitive2d::Primitive2DContainer getOverlayObjectPrimitive2DSequence() const;

            // access to visibility state
            bool isVisible() const { return mbIsVisible; }
            void setVisible(bool bNew);

            // access to hittable flag
            bool isHittable() const { return mbIsHittable; }
            void setHittable(bool bNew);

            // read access to AntiAliase flag
            bool allowsAntiAliase() const { return mbAllowsAntiAliase; }

            // read access to baseRange. This may trigger createBaseRange() if
            // object is changed.
            const basegfx::B2DRange& getBaseRange() const;

            // access to baseColor
            Color getBaseColor() const { return maBaseColor; }
            void setBaseColor(Color aNew);

            // execute event from base class sdr::animation::Event. Default
            // implementation does nothing and does not create a new event.
            virtual void Trigger(sal_uInt32 nTime) override;

            // access to AllowsAnimation flag
            bool allowsAnimation() const { return mbAllowsAnimation; }

            // stripe definition has changed. The OverlayManager does have
            // support data to draw graphics in two colors striped. This
            // method notifies the OverlayObject if that change takes place.
            // Default implementation does nothing.
            virtual void stripeDefinitionHasChanged();
        };

        // typedefs for a vector of OverlayObjects
        typedef ::std::vector< OverlayObject* > OverlayObjectVector;

    } // end of namespace overlay
} // end of namespace sdr

namespace sdr
{
    namespace overlay
    {
        class SVX_DLLPUBLIC OverlayObjectWithBasePosition : public OverlayObject
        {
        protected:
            // base position in logical coordinates
            basegfx::B2DPoint                       maBasePosition;

        public:
            OverlayObjectWithBasePosition(const basegfx::B2DPoint& rBasePos, Color aBaseColor);
            virtual ~OverlayObjectWithBasePosition();

            // access to basePosition
            const basegfx::B2DPoint& getBasePosition() const { return maBasePosition; }
            void setBasePosition(const basegfx::B2DPoint& rNew);
        };
    } // end of namespace overlay
} // end of namespace sdr

#endif // INCLUDED_SVX_SDR_OVERLAY_OVERLAYOBJECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: overlayobject.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _SDR_OVERLAY_OVERLAYOBJECT_HXX
#define _SDR_OVERLAY_OVERLAYOBJECT_HXX

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drange.hxx>
#include <tools/color.hxx>
#include <svx/sdr/animation/scheduler.hxx>
#include "svx/svxdllapi.h"

#include <vector>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class OutputDevice;

namespace sdr
{
    namespace overlay
    {
        class OverlayManager;
    } // end of namespace overlay
} // end of namespace sdr

namespace basegfx
{
    class B2DPolygon;
    class B2DPolyPolygon;
    class B2DRange;
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace overlay
    {
        class SVX_DLLPUBLIC OverlayObject : public ::sdr::animation::Event
        {
            // Manager is allowed access to private Members, especially
            // pNext and pPrevious are used form the manager to handle the
            // OverlayObject.
            friend class                            OverlayManager;

            // pointer to OverlayManager, if object is added. Changed by
            // OverlayManager, do not chnge Yourself.
            OverlayManager*                         mpOverlayManager;

            // Chaining of IAO's, used by OverlayManager. These will be
            // used form the OverlayManager, so do not change them Yourself.
            OverlayObject*                          mpNext;
            OverlayObject*                          mpPrevious;

        protected:
            // region in logical coordinates
            basegfx::B2DRange                       maBaseRange;

            // base color of this OverlayObject
            Color                                   maBaseColor;

            // bitfield
            // Flag for visibility
            unsigned                                mbIsVisible : 1;

            // Flag for validity
            unsigned                                mbIsChanged : 1;

            // Flag to control hittability
            unsigned                                mbIsHittable : 1;

            // Flag to hold info if this objects supports animation. Default is
            // sal_False. If sal_True, the Trigger() method should be overloaded
            // to implement the animation effect and to re-initiate the event.
            unsigned                                mbAllowsAnimation : 1;

            // Draw geometry
            virtual void drawGeometry(OutputDevice& rOutputDevice) = 0;

            // Create the BaseRange. This method needs to calculate maBaseRange.
            virtual void createBaseRange(OutputDevice& rOutputDevice) = 0;

            // set changed flag. Call after change, since the old range is invalidated
            // and then the new one is calculated and invalidated, too. This will only
            // work after the change.
            void objectChange();

            // support method to draw striped geometries
            void ImpDrawRangeStriped(OutputDevice& rOutputDevice, const basegfx::B2DRange& rRange);
            void ImpDrawLineStriped(OutputDevice& rOutputDevice, double x1, double y1, double x2, double y2);
            void ImpDrawLineStriped(OutputDevice& rOutputDevice, const basegfx::B2DPoint& rStart, const basegfx::B2DPoint& rEnd);
            void ImpDrawPolygonStriped(OutputDevice& rOutputDevice, const basegfx::B2DPolygon& rPolygon);

        public:
            OverlayObject(Color aBaseColor);
            virtual ~OverlayObject();

            // get OverlayManager
            OverlayManager* getOverlayManager() const { return mpOverlayManager; }

            // Hittest with logical coordinates. Default tests against maBaseRange.
            virtual sal_Bool isHit(const basegfx::B2DPoint& rPos, double fTol = 0.0) const;

            // access to visibility state
            sal_Bool isVisible() const { return mbIsVisible; }
            void setVisible(sal_Bool bNew);

            // read access to changed flag
            sal_Bool isChanged() const { return mbIsChanged; }

            // access to hittable flag
            sal_Bool isHittable() const { return mbIsHittable; }
            void setHittable(sal_Bool bNew);

            // read access to baseRange. This may trigger createBaseRange() if
            // object is changed.
            const basegfx::B2DRange& getBaseRange() const;

            // access to baseColor
            Color getBaseColor() const { return maBaseColor; }
            void setBaseColor(Color aNew);

            // execute event from base class ::sdr::animation::Event. Default
            // implementation does nothing and does not create a new event.
            virtual void Trigger(sal_uInt32 nTime);

            // acces to AllowsAnimation flag
            sal_Bool allowsAnimation() const { return mbAllowsAnimation; }

            // transform object coordinates.
            virtual void transform(const basegfx::B2DHomMatrix& rMatrix) = 0;

            // Zoom has changed. If the objects logical size
            // depends on the MapMode of the used OutputDevice, use this call
            // to invalidate the range in logical coordinates. Default is no
            // change.
            virtual void zoomHasChanged();

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

//////////////////////////////////////////////////////////////////////////////

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

            // transform object coordinates. Transforms maBasePosition
            // and invalidates on change
            virtual void transform(const basegfx::B2DHomMatrix& rMatrix);
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_OVERLAY_OVERLAYOBJECT_HXX

// eof

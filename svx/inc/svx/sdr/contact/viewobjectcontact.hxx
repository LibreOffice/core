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

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACT_HXX
#define _SDR_CONTACT_VIEWOBJECTCONTACT_HXX

#include <sal/types.h>

#include <vector>
#include <tools/debug.hxx>
#include <tools/gen.hxx>
#include "svx/svxdllapi.h"
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class Region;

namespace sdr { namespace contact {
    class DisplayInfo;
    class ObjectContact;
    class ViewContact;
    class ViewObjectContactRedirector;
}}

namespace sdr { namespace animation {
    class PrimitiveAnimation;
}}

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ViewObjectContact
        {
        private:
            // must-exist and constant contacts
            ObjectContact&                                  mrObjectContact;
            ViewContact&                                    mrViewContact;

            // This range defines the object's BoundRect
            basegfx::B2DRange                               maObjectRange;

            // PrimitiveSequence of the ViewContact. This contains all necessary information
            // for the graphical visualisation and needs to be supported by all VCs which
            // can be visualized.
            drawinglayer::primitive2d::Primitive2DSequence  mxPrimitive2DSequence;

            // the PrimitiveAnimation if Primitive2DSequence contains animations
            sdr::animation::PrimitiveAnimation*             mpPrimitiveAnimation;

            // bitfield
            // This bool gets set when the object gets invalidated by ActionChanged() and
            // can be used from the OC to late-invalidates
            unsigned                                        mbLazyInvalidate : 1;

        protected:
            // make redirector a protected friend, it needs to call createPrimitives as default action
            friend class ViewObjectContactRedirector;

            // Called from getPrimitive2DSequence() when vector has changed. Evaluate object animation
            // and setup accordingly
            void checkForPrimitive2DAnimations();

            // This method is responsible for creating the graphical visualisation data which is
            // stored/cached in the local primitive. Default gets view-independent Primitive
            // from the ViewContact using ViewContact::getViewIndependentPrimitive2DSequence(), takes care of
            // visibility, handles glue and ghosted.
            // This method will not handle included hierarchies and not check geometric visibility.
            virtual drawinglayer::primitive2d::Primitive2DSequence createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const;

            // method for flushing Primitive2DSequence for VOC implementations
            void flushPrimitive2DSequence() { mxPrimitive2DSequence.realloc(0); }

        public:
            // basic constructor.
            ViewObjectContact(ObjectContact& rObjectContact, ViewContact& rViewContact);
            virtual ~ViewObjectContact();

            // access to ObjectContact
            ObjectContact& GetObjectContact() const { return mrObjectContact; }

            // access to ViewContact
            ViewContact& GetViewContact() const { return mrViewContact; }

            // get the oebject's size range
            const basegfx::B2DRange& getObjectRange() const;

            // A ViewObjectContact was deleted and shall be forgotten.
            void RemoveViewObjectContact(ViewObjectContact& rVOContact);

            // React on changes of the object of this ViewContact
            virtual void ActionChanged();

            // LazyInvalidate handling
            void triggerLazyInvalidate();

            // Check if this primitive is animated in any OC (View) which means it has
            // generated a PrimitiveAnimation
            bool isAnimated() const { return (0 != mpPrimitiveAnimation); }

            // Take some action when new objects are inserted
            virtual void ActionChildInserted(ViewContact& rChild);

            // access to the local primitive. This will ensure that the local primitive is
            // current in comparing the local one with a fresh created incarnation
            // This method will not handle included hierarchies and not check visibility.
            drawinglayer::primitive2d::Primitive2DSequence getPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const;

            // test this VOC for visibility concerning model-view stuff like e.g. Layer
            virtual bool isPrimitiveVisible(const DisplayInfo& rDisplayInfo) const;

            // test this VOC for ghosted mode
            virtual bool isPrimitiveGhosted(const DisplayInfo& rDisplayInfo) const;

            // process this primitive: Eventually also recursively travel an existing hierarchy,
            // e.g. for group objects, scenes or pages. This method will test geometrical visibility.
            virtual drawinglayer::primitive2d::Primitive2DSequence getPrimitive2DSequenceHierarchy(DisplayInfo& rDisplayInfo) const;

            // just process the sub-hierarchy, used as tooling from getPrimitive2DSequenceHierarchy
            drawinglayer::primitive2d::Primitive2DSequence getPrimitive2DSequenceSubHierarchy(DisplayInfo& rDisplayInfo) const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_VIEWOBJECTCONTACT_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

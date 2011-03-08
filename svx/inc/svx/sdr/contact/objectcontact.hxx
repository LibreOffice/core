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

#ifndef _SDR_CONTACT_OBJECTCONTACT_HXX
#define _SDR_CONTACT_OBJECTCONTACT_HXX

#include <svx/sdr/animation/objectanimator.hxx>
#include "svx/svxdllapi.h"
#include <drawinglayer/geometry/viewinformation2d.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SetOfByte;
class Rectangle;
class SdrPageView;
class OutputDevice;

namespace sdr { namespace contact {
    class DisplayInfo;
    class ViewContact;
    class ViewObjectContactRedirector;
}}

namespace sdr { namespace event {
    class TimerEventHandler;
}}

namespace basegfx {
    class B2DRange;
    class B2DHomMatrix;
}

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class SVX_DLLPUBLIC ObjectContact
        {
        private:
            // make ViewObjectContact a friend to exclusively allow it to use
            // AddViewObjectContact/RemoveViewObjectContact
            friend class ViewObjectContact;

            // All VOCs which are created using this OC, thus remembering this OC
            // as a reference. All those VOCs need to be deleted when the OC goes down.
            // Registering and de-registering is done in the VOC constructors/destructors.
            std::vector< ViewObjectContact* >               maViewObjectContactVector;

            // A new ViewObjectContact was created and shall be remembered.
            void AddViewObjectContact(ViewObjectContact& rVOContact);

            // A ViewObjectContact was deleted and shall be forgotten.
            virtual void RemoveViewObjectContact(ViewObjectContact& rVOContact);

            // the primitiveAnimator which is used if this View and/or the contained primitives
            // support animatedSwitchPrimitives
            sdr::animation::primitiveAnimator               maPrimitiveAnimator;

            // the EventHandler for e.g. asynchronious loading of graphics
            sdr::event::TimerEventHandler*                  mpEventHandler;

            // The redirector. If set it is used to pipe all supported calls
            // to the redirector
            ViewObjectContactRedirector*                    mpViewObjectContactRedirector;

            // the Primitive2DParameters containing view information
            drawinglayer::geometry::ViewInformation2D       maViewInformation2D;

            // bitfield
            // flag for preview renderer
            unsigned                                        mbIsPreviewRenderer : 1;

            // method to create a EventHandler. Needs to give a result.
            sdr::event::TimerEventHandler* CreateEventHandler();

        protected:
            // Interface to allow derivates to travel over the registered VOC's
            sal_uInt32 getViewObjectContactCount() const { return maViewObjectContactVector.size(); }
            ViewObjectContact* getViewObjectContact(sal_uInt32 a) const { return maViewObjectContactVector[a]; }

            // interface to allow derivates to set PreviewRenderer flag
            void setPreviewRenderer(bool bNew) { mbIsPreviewRenderer = bNew; }

            // interface to allow derivates to set ViewInformation2D
            void updateViewInformation2D(const drawinglayer::geometry::ViewInformation2D& rViewInformation2D) { maViewInformation2D = rViewInformation2D; }

        public:
            // basic constructor
            ObjectContact();
            virtual ~ObjectContact();

            // LazyInvalidate request. This is used from the VOCs to mark that they
            // got invalidated by an ActionCanged() call. An active view needs to remember
            // this and take action on it. Default implementation directly calls back
            // triggerLazyInvalidate() wich promptly handles the request
            virtual void setLazyInvalidate(ViewObjectContact& rVOC);

            // call this to support evtl. preparations for repaint. Default does nothing
            virtual void PrepareProcessDisplay();

            // Process the whole displaying
            virtual void ProcessDisplay(DisplayInfo& rDisplayInfo);

            // test if visualizing of entered groups is switched on at all. Default
            // implementation returns sal_False.
            virtual bool DoVisualizeEnteredGroup() const;

            // get active group's (the entered group) ViewContact
            virtual const ViewContact* getActiveViewContact() const;

            // Invalidate given rectangle at the window/output which is represented by
            // this ObjectContact. Default does nothing.
            virtual void InvalidatePartOfView(const basegfx::B2DRange& rRange) const;

            // Get info if given Rectangle is visible in this view
            virtual bool IsAreaVisible(const basegfx::B2DRange& rRange) const;

            // Get info about the need to visualize GluePoints. The default
            // is that it is not necessary.
            virtual bool AreGluePointsVisible() const;

            // method to get the primitiveAnimator
            sdr::animation::primitiveAnimator& getPrimitiveAnimator();

            // method to get the EventHandler. It will
            // return a existing one or create a new one using CreateEventHandler().
            sdr::event::TimerEventHandler& GetEventHandler() const;

            // delete the EventHandler
            void DeleteEventHandler();

            // test if there is an EventHandler without creating one on demand
            bool HasEventHandler() const;

            // check if text animation is allowed. Default is sal_true.
            virtual bool IsTextAnimationAllowed() const;

            // check if graphic animation is allowed. Default is sal_true.
            virtual bool IsGraphicAnimationAllowed() const;

            // check if asynchronious graphis loading is allowed. Default is sal_False.
            virtual bool IsAsynchronGraphicsLoadingAllowed() const;

            // access to ViewObjectContactRedirector
            ViewObjectContactRedirector* GetViewObjectContactRedirector() const;
            void SetViewObjectContactRedirector(ViewObjectContactRedirector* pNew);

            // check if buffering of MasterPages is allowed. Default is sal_False.
            virtual bool IsMasterPageBufferingAllowed() const;

            // print? Default is false
            virtual bool isOutputToPrinter() const;

            // window? Default is true
            virtual bool isOutputToWindow() const;

            // VirtualDevice? Default is false
            virtual bool isOutputToVirtualDevice() const;

            // recording MetaFile? Default is false
            virtual bool isOutputToRecordingMetaFile() const;

            // pdf export? Default is false
            virtual bool isOutputToPDFFile() const;

            // gray display mode
            virtual bool isDrawModeGray() const;

            // gray display mode
            virtual bool isDrawModeBlackWhite() const;

            // high contrast display mode
            virtual bool isDrawModeHighContrast() const;

            // check if this is a preview renderer. Default is sal_False.
            bool IsPreviewRenderer() const { return mbIsPreviewRenderer; }

            // get Primitive2DParameters for this view
            const drawinglayer::geometry::ViewInformation2D& getViewInformation2D() const { return maViewInformation2D; }

            // access to SdrPageView. May return 0L like the default implementations do. Needs to be overloaded as needed.
            virtual SdrPageView* TryToGetSdrPageView() const;

            // access to OutputDevice. May return 0L like the default implementations do. Needs to be overloaded as needed.
            virtual OutputDevice* TryToGetOutputDevice() const;

            // reset ViewPort at internal ViewInformation2D. This is needed when the OC is used
            // not for ProcessDisplay() but to get a VOC associated with it. When trying to get
            // a sequence of primitives from the VOC then, the last initialized ViewPort from
            // the last ProcessDisplay() is used for geometric visibility testing. If this is not
            // wanted (like in such cases) this method is used. It will reuse the current
            // ViewInformation2D, but clear the ViewPort (no ViewPort means all is visible)
            void resetViewPort();
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_OBJECTCONTACT_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

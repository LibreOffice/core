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

#ifndef _SDR_CONTACT_OBJECTCONTACTOFPAGEVIEW_HXX
#define _SDR_CONTACT_OBJECTCONTACTOFPAGEVIEW_HXX

#include <svx/sdr/contact/objectcontact.hxx>
#include <tools/gen.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class SdrPageWindow;
class SdrPage;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        class ObjectContactOfPageView : public ObjectContact, public Timer
        {
        protected:
            // the owner of this ObjectContactOfPageView. Set from constructor and not
            // to be changed in any way.
            SdrPageWindow&                                      mrPageWindow;

            // Process the whole displaying, the real version
            void DoProcessDisplay(DisplayInfo& rDisplayInfo);

        public:
            // access to SdrPageWindow
            SdrPageWindow& GetPageWindow() const { return mrPageWindow; }

            // access to SdrPage of PageView
            SdrPage* GetSdrPage() const;

            // basic constructor, used from SdrPageView.
            ObjectContactOfPageView(SdrPageWindow& rPageWindow);
            virtual ~ObjectContactOfPageView();

            // LazyInvalidate request. This is used from the VOCs to mark that they
            // got invalidated by an ActionCanged() call. An active view needs to remember
            // this and take action on it. Default implementation directly calls back
            // triggerLazyInvalidate() wich promptly handles the request
            virtual void setLazyInvalidate(ViewObjectContact& rVOC);

            // call this to support evtl. preparations for repaint
            virtual void PrepareProcessDisplay();

            // From baseclass Timer, the timeout call triggered by te LazyInvalidate mechanism
            virtual void Timeout();

            // Process the whole displaying
            virtual void ProcessDisplay(DisplayInfo& rDisplayInfo);

            // test if visualizing of entered groups is switched on at all
            virtual bool DoVisualizeEnteredGroup() const;

            // get active group's (the entered group) ViewContact
            virtual const ViewContact* getActiveViewContact() const;

            // Invalidate given rectangle at the window/output which is represented by
            // this ObjectContact.
            virtual void InvalidatePartOfView(const basegfx::B2DRange& rRange) const;

            // Get info if given Rectangle is visible in this view
            virtual bool IsAreaVisible(const basegfx::B2DRange& rRange) const;

            // Get info about the need to visualize GluePoints. The default
            // is that it is not necessary.
            virtual bool AreGluePointsVisible() const;

            // check if text animation is allowed.
            virtual bool IsTextAnimationAllowed() const;

            // check if graphic animation is allowed.
            virtual bool IsGraphicAnimationAllowed() const;

            // check if asynchronious graphis loading is allowed. Default is sal_False.
            virtual bool IsAsynchronGraphicsLoadingAllowed() const;

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

            // overloaded access to SdrPageView
            virtual SdrPageView* TryToGetSdrPageView() const;

            // access to OutputDevice. May return 0L like the default implementations do. Needs to be overloaded as needed.
            virtual OutputDevice* TryToGetOutputDevice() const;

            /** sets all UNO controls which are associated with this ObjectContact to
                design or alive mode.
            */
            void    SetUNOControlsDesignMode( bool _bDesignMode ) const;
        };
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_OBJECTCONTACTOFPAGEVIEW_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

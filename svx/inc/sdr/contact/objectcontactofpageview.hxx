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

#ifndef INCLUDED_SVX_SDR_CONTACT_OBJECTCONTACTOFPAGEVIEW_HXX
#define INCLUDED_SVX_SDR_CONTACT_OBJECTCONTACTOFPAGEVIEW_HXX

#include <svx/sdr/contact/objectcontact.hxx>


// predeclarations

class SdrPageWindow;
class SdrPage;



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
            explicit ObjectContactOfPageView(SdrPageWindow& rPageWindow);
            virtual ~ObjectContactOfPageView();

            // LazyInvalidate request. This is used from the VOCs to mark that they
            // got invalidated by an ActionCanged() call. An active view needs to remember
            // this and take action on it. Default implementation directly calls back
            // triggerLazyInvalidate() which promptly handles the request
            virtual void setLazyInvalidate(ViewObjectContact& rVOC) SAL_OVERRIDE;

            // call this to support evtl. preparations for repaint
            virtual void PrepareProcessDisplay() SAL_OVERRIDE;

            // From baseclass Timer, the timeout call triggered by te LazyInvalidate mechanism
            virtual void Timeout() SAL_OVERRIDE;

            // Process the whole displaying
            virtual void ProcessDisplay(DisplayInfo& rDisplayInfo) SAL_OVERRIDE;

            // test if visualizing of entered groups is switched on at all
            virtual bool DoVisualizeEnteredGroup() const SAL_OVERRIDE;

            // get active group's (the entered group) ViewContact
            virtual const ViewContact* getActiveViewContact() const SAL_OVERRIDE;

            // Invalidate given rectangle at the window/output which is represented by
            // this ObjectContact.
            virtual void InvalidatePartOfView(const basegfx::B2DRange& rRange) const SAL_OVERRIDE;

            // Get info if given Rectangle is visible in this view
            virtual bool IsAreaVisible(const basegfx::B2DRange& rRange) const SAL_OVERRIDE;

            // Get info about the need to visualize GluePoints. The default
            // is that it is not necessary.
            virtual bool AreGluePointsVisible() const SAL_OVERRIDE;

            // check if text animation is allowed.
            virtual bool IsTextAnimationAllowed() const SAL_OVERRIDE;

            // check if graphic animation is allowed.
            virtual bool IsGraphicAnimationAllowed() const SAL_OVERRIDE;

            // check if asynchronious graphis loading is allowed. Default is sal_False.
            virtual bool IsAsynchronGraphicsLoadingAllowed() const SAL_OVERRIDE;

            // check if buffering of MasterPages is allowed. Default is sal_False.
            virtual bool IsMasterPageBufferingAllowed() const SAL_OVERRIDE;

            // print? Default is false
            virtual bool isOutputToPrinter() const SAL_OVERRIDE;

            // window? Default is true
            virtual bool isOutputToWindow() const SAL_OVERRIDE;

            // VirtualDevice? Default is false
            virtual bool isOutputToVirtualDevice() const SAL_OVERRIDE;

            // recording MetaFile? Default is false
            virtual bool isOutputToRecordingMetaFile() const SAL_OVERRIDE;

            // pdf export? Default is false
            virtual bool isOutputToPDFFile() const SAL_OVERRIDE;

            // gray display mode
            virtual bool isDrawModeGray() const SAL_OVERRIDE;

            // gray display mode
            virtual bool isDrawModeBlackWhite() const SAL_OVERRIDE;

            // high contrast display mode
            virtual bool isDrawModeHighContrast() const SAL_OVERRIDE;

            // overloaded access to SdrPageView
            virtual SdrPageView* TryToGetSdrPageView() const SAL_OVERRIDE;

            // access to OutputDevice. May return 0L like the default implementations do. Needs to be overloaded as needed.
            virtual OutputDevice* TryToGetOutputDevice() const SAL_OVERRIDE;

            /** sets all UNO controls which are associated with this ObjectContact to
                design or alive mode.
            */
            void    SetUNOControlsDesignMode( bool _bDesignMode ) const;
        };
    } // end of namespace contact
} // end of namespace sdr



#endif // INCLUDED_SVX_SDR_CONTACT_OBJECTCONTACTOFPAGEVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

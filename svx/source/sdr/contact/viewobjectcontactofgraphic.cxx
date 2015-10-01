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

#include <sdr/contact/viewobjectcontactofgraphic.hxx>
#include <sdr/contact/viewcontactofgraphic.hxx>
#include <svx/svdograf.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>

#include "eventhandler.hxx"

namespace sdr
{
    namespace event
    {
        class AsynchGraphicLoadingEvent : public BaseEvent
        {
            // the ViewContactOfGraphic to work with
            sdr::contact::ViewObjectContactOfGraphic&       mrVOCOfGraphic;

        public:
            // basic constructor.
            AsynchGraphicLoadingEvent(EventHandler& rEventHandler, sdr::contact::ViewObjectContactOfGraphic& rVOCOfGraphic);

            // destructor
            virtual ~AsynchGraphicLoadingEvent();

            // the called method if the event is triggered
            virtual void ExecuteEvent() SAL_OVERRIDE;
        };

        AsynchGraphicLoadingEvent::AsynchGraphicLoadingEvent(
            EventHandler& rEventHandler, sdr::contact::ViewObjectContactOfGraphic& rVOCOfGraphic)
        :   BaseEvent(rEventHandler),
            mrVOCOfGraphic(rVOCOfGraphic)
        {
        }

        AsynchGraphicLoadingEvent::~AsynchGraphicLoadingEvent()
        {
            mrVOCOfGraphic.forgetAsynchGraphicLoadingEvent(this);
        }

        void AsynchGraphicLoadingEvent::ExecuteEvent()
        {
            mrVOCOfGraphic.doAsynchGraphicLoading();
        }
    } // end of namespace event
} // end of namespace sdr



namespace sdr
{
    namespace contact
    {
        // Test graphics state and eventually trigger a SwapIn event or an Asynchronous
        // load event. Return value gives info if SwapIn was triggered or not
        bool ViewObjectContactOfGraphic::impPrepareGraphicWithAsynchroniousLoading()
        {
            bool bRetval(false);
            SdrGrafObj& rGrafObj = getSdrGrafObj();

            if(rGrafObj.IsSwappedOut())
            {
                if(rGrafObj.IsLinkedGraphic())
                {
                    // update graphic link
                    rGrafObj.ImpUpdateGraphicLink();
                }
                else
                {
                    // SwapIn needs to be done. Decide if it can be done asynchronious.
                    bool bSwapInAsynchronious(false);
                    ObjectContact& rObjectContact = GetObjectContact();

                    // only when allowed from configuration
                    if(rObjectContact.IsAsynchronGraphicsLoadingAllowed())
                    {
                        // direct output or vdev output (PageView buffering)
                        if(rObjectContact.isOutputToWindow() || rObjectContact.isOutputToVirtualDevice())
                        {
                            // only when no metafile recording
                            if(!rObjectContact.isOutputToRecordingMetaFile())
                            {
                                // allow asynchronious loading
                                bSwapInAsynchronious = true;
                            }
                        }
                    }

                    if(bSwapInAsynchronious)
                    {
                        // maybe it's on the way, then do nothing
                        if(!mpAsynchLoadEvent)
                        {
                            // Trigger asynchronious SwapIn.
                            sdr::event::TimerEventHandler& rEventHandler = rObjectContact.GetEventHandler();

                            mpAsynchLoadEvent = new sdr::event::AsynchGraphicLoadingEvent(rEventHandler, *this);
                        }
                    }
                    else
                    {
                        if(rObjectContact.isOutputToPrinter() || rObjectContact.isOutputToPDFFile())
                        {
                            // #i76395# preview mechanism is only active if
                            // swapin is called from inside paint preparation, so mbInsidePaint
                            // has to be false to be able to print with high resolution
                            rGrafObj.ForceSwapIn();
                        }
                        else
                        {
                            // SwapIn direct
                            rGrafObj.mbInsidePaint = true;
                            rGrafObj.ForceSwapIn();
                            rGrafObj.mbInsidePaint = false;
                        }

                        bRetval = true;
                    }
                }
            }
            else
            {
                // it is not swapped out, somehow[1] it was loaded. In that case, forget
                // about an existing triggered event
                if (mpAsynchLoadEvent)
                {
                    // just delete it, this will remove it from the EventHandler and
                    // will trigger forgetAsynchGraphicLoadingEvent from the destructor
                    delete mpAsynchLoadEvent;

                    // Invalidate paint areas.
                    // [1] If a calc document with graphics is loaded then OnLoad will
                    // be emitted before the graphic are due to be swapped in asynchronously
                    // In sfx2 we generate a preview on receiving onload, which forces
                    // the graphics to be swapped in to generate the preview. When
                    // the timer triggers it find the graphics already swapped in. So
                    // we should still invalidate the paint area on finding the graphic
                    // swapped in seeing as we're still waiting in calc to draw the
                    // graphics on receipt of their contents.
                    GetViewContact().ActionChanged();
                }
            }

            return bRetval;
        }

        // Test graphics state and eventually trigger a SwapIn event. Return value
        // gives info if SwapIn was triggered or not
        bool ViewObjectContactOfGraphic::impPrepareGraphicWithSynchroniousLoading()
        {
            bool bRetval(false);
            SdrGrafObj& rGrafObj = getSdrGrafObj();

            if(rGrafObj.IsSwappedOut())
            {
                if(rGrafObj.IsLinkedGraphic())
                {
                    // update graphic link
                    rGrafObj.ImpUpdateGraphicLink( false );
                }
                else
                {
                    ObjectContact& rObjectContact = GetObjectContact();

                    if(rObjectContact.isOutputToPrinter() || rObjectContact.isOutputToPDFFile())
                    {
                        // #i76395# preview mechanism is only active if
                        // swapin is called from inside paint preparation, so mbInsidePaint
                        // has to be false to be able to print with high resolution
                        rGrafObj.ForceSwapIn();
                    }
                    else
                    {
                        // SwapIn direct
                        rGrafObj.mbInsidePaint = true;
                        rGrafObj.ForceSwapIn();
                        rGrafObj.mbInsidePaint = false;
                    }

                    bRetval = true;
                }
            }

            return bRetval;
        }

        // This is the call from the asynch graphic loading. This may only be called from
        // AsynchGraphicLoadingEvent::ExecuteEvent(). Do load the graphics. The event will
        // be deleted (consumed) and forgetAsynchGraphicLoadingEvent will be called.
        void ViewObjectContactOfGraphic::doAsynchGraphicLoading()
        {
            DBG_ASSERT(mpAsynchLoadEvent, "ViewObjectContactOfGraphic::doAsynchGraphicLoading: I did not trigger a event, why am i called (?)");

            // swap it in
            SdrGrafObj& rGrafObj = getSdrGrafObj();
            rGrafObj.ForceSwapIn();

            // #i103720# forget event to avoid possible deletion by the following ActionChanged call
            // which may use createPrimitive2DSequence/impPrepareGraphicWithAsynchroniousLoading again.
            // Deletion is actually done by the scheduler who leaded to coming here
            mpAsynchLoadEvent = 0;

            // Invalidate all paint areas and check existing animation (which may have changed).
            GetViewContact().ActionChanged();
        }

        // This is the call from the destructor of the asynch graphic loading event.
        // No one else has to call this. It is needed to let this object forget about
        // the event. The parameter allows checking for the correct event.
        void ViewObjectContactOfGraphic::forgetAsynchGraphicLoadingEvent(sdr::event::AsynchGraphicLoadingEvent* pEvent)
        {
            (void) pEvent; // suppress warning

            if(mpAsynchLoadEvent)
            {
                OSL_ENSURE(!pEvent || mpAsynchLoadEvent == pEvent,
                    "ViewObjectContactOfGraphic::forgetAsynchGraphicLoadingEvent: Forced to forget another event then I have scheduled (?)");

                // forget event
                mpAsynchLoadEvent = 0;
            }
        }

        const SdrGrafObj& ViewObjectContactOfGraphic::getSdrGrafObj() const
        {
            return static_cast<const ViewContactOfGraphic& >(GetViewContact()).GetGrafObject();
        }

        SdrGrafObj& ViewObjectContactOfGraphic::getSdrGrafObj()
        {
            return static_cast<ViewContactOfGraphic& >(GetViewContact()).GetGrafObject();
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContactOfGraphic::createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const
        {
            // prepare primitive generation with evtl. loading the graphic when it's swapped out
            SdrGrafObj& rGrafObj = const_cast< ViewObjectContactOfGraphic* >(this)->getSdrGrafObj();
            bool bDoAsynchronGraphicLoading(rGrafObj.GetModel() && rGrafObj.GetModel()->IsSwapGraphics());

            if( bDoAsynchronGraphicLoading && rGrafObj.IsSwappedOut() )
            {
                // sometimes it is needed that each graphic is completely available and swapped in
                // for these cases a ForceSwapIn is called later at the graphic object
                if ( rGrafObj.GetPage() && rGrafObj.GetPage()->IsMasterPage() )
                {
                    // #i102380# force Swap-In for GraphicObjects on MasterPage to have a nicer visualisation
                    bDoAsynchronGraphicLoading = false;
                }
                else if ( GetObjectContact().isOutputToPrinter()
                    || GetObjectContact().isOutputToRecordingMetaFile()
                    || GetObjectContact().isOutputToPDFFile() )
                {
                    bDoAsynchronGraphicLoading = false;
                }
            }
            if( bDoAsynchronGraphicLoading )
            {
                const_cast< ViewObjectContactOfGraphic* >(this)->impPrepareGraphicWithAsynchroniousLoading();
            }
            else
            {
                const_cast< ViewObjectContactOfGraphic* >(this)->impPrepareGraphicWithSynchroniousLoading();
            }

            // get return value by calling parent
            drawinglayer::primitive2d::Primitive2DSequence xRetval = ViewObjectContactOfSdrObj::createPrimitive2DSequence(rDisplayInfo);

            if(xRetval.hasElements())
            {
                // #i103255# suppress when graphic needs draft visualisation and output
                // is for PDF export/Printer
                const ViewContactOfGraphic& rVCOfGraphic = static_cast< const ViewContactOfGraphic& >(GetViewContact());

                if(rVCOfGraphic.visualisationUsesDraft())
                {
                    const ObjectContact& rObjectContact = GetObjectContact();

                    if(rObjectContact.isOutputToPDFFile() || rObjectContact.isOutputToPrinter())
                    {
                        xRetval = drawinglayer::primitive2d::Primitive2DSequence();
                    }
                }
            }

            return xRetval;
        }

        ViewObjectContactOfGraphic::ViewObjectContactOfGraphic(ObjectContact& rObjectContact, ViewContact& rViewContact)
        :   ViewObjectContactOfSdrObj(rObjectContact, rViewContact),
            mpAsynchLoadEvent(0)
        {
        }

        ViewObjectContactOfGraphic::~ViewObjectContactOfGraphic()
        {
            // evtl. delete the asynch loading event
            if(mpAsynchLoadEvent)
            {
                // just delete it, this will remove it from the EventHandler and
                // will trigger forgetAsynchGraphicLoadingEvent from the destructor
                delete mpAsynchLoadEvent;
            }
        }
    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

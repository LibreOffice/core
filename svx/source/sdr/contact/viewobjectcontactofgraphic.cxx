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


#include <svx/sdr/contact/viewobjectcontactofgraphic.hxx>
#include <svx/sdr/contact/viewcontactofgraphic.hxx>
#include <svx/sdr/event/eventhandler.hxx>
#include <svx/svdograf.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace event
    {
        class AsynchGraphicLoadingEvent : public BaseEvent
        {
            
            sdr::contact::ViewObjectContactOfGraphic&       mrVOCOfGraphic;

        public:
            
            AsynchGraphicLoadingEvent(EventHandler& rEventHandler, sdr::contact::ViewObjectContactOfGraphic& rVOCOfGraphic);

            
            virtual ~AsynchGraphicLoadingEvent();

            
            virtual void ExecuteEvent();
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
    } 
} 

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        
        
        bool ViewObjectContactOfGraphic::impPrepareGraphicWithAsynchroniousLoading()
        {
            bool bRetval(false);
            SdrGrafObj& rGrafObj = getSdrGrafObj();

            if(rGrafObj.IsSwappedOut())
            {
                if(rGrafObj.IsLinkedGraphic())
                {
                    
                    rGrafObj.ImpUpdateGraphicLink();
                }
                else
                {
                    
                    bool bSwapInAsynchronious(false);
                    ObjectContact& rObjectContact = GetObjectContact();

                    
                    if(rObjectContact.IsAsynchronGraphicsLoadingAllowed())
                    {
                        
                        if(rObjectContact.isOutputToWindow() || rObjectContact.isOutputToVirtualDevice())
                        {
                            
                            if(!rObjectContact.isOutputToRecordingMetaFile())
                            {
                                
                                bSwapInAsynchronious = true;
                            }
                        }
                    }

                    if(bSwapInAsynchronious)
                    {
                        
                        if(!mpAsynchLoadEvent)
                        {
                            
                            sdr::event::TimerEventHandler& rEventHandler = rObjectContact.GetEventHandler();

                            mpAsynchLoadEvent = new sdr::event::AsynchGraphicLoadingEvent(rEventHandler, *this);
                        }
                    }
                    else
                    {
                        if(rObjectContact.isOutputToPrinter() || rObjectContact.isOutputToPDFFile())
                        {
                            
                            
                            
                            rGrafObj.ForceSwapIn();
                        }
                        else
                        {
                            
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
                
                
                if(mpAsynchLoadEvent)
                {
                    
                    
                    delete mpAsynchLoadEvent;
                }
            }

            return bRetval;
        }

        
        
        bool ViewObjectContactOfGraphic::impPrepareGraphicWithSynchroniousLoading()
        {
            bool bRetval(false);
            SdrGrafObj& rGrafObj = getSdrGrafObj();

            if(rGrafObj.IsSwappedOut())
            {
                if(rGrafObj.IsLinkedGraphic())
                {
                    
                    rGrafObj.ImpUpdateGraphicLink( false );
                }
                else
                {
                    ObjectContact& rObjectContact = GetObjectContact();

                    if(rObjectContact.isOutputToPrinter() || rObjectContact.isOutputToPDFFile())
                    {
                        
                        
                        
                        rGrafObj.ForceSwapIn();
                    }
                    else
                    {
                        
                        rGrafObj.mbInsidePaint = true;
                        rGrafObj.ForceSwapIn();
                        rGrafObj.mbInsidePaint = false;
                    }

                    bRetval = true;
                }
            }

            return bRetval;
        }

        
        
        
        void ViewObjectContactOfGraphic::doAsynchGraphicLoading()
        {
            DBG_ASSERT(mpAsynchLoadEvent, "ViewObjectContactOfGraphic::doAsynchGraphicLoading: I did not trigger a event, why am i called (?)");

            
            SdrGrafObj& rGrafObj = getSdrGrafObj();
            rGrafObj.ForceSwapIn();

            
            
            
            mpAsynchLoadEvent = 0;

            
            GetViewContact().ActionChanged();
        }

        
        
        
        void ViewObjectContactOfGraphic::forgetAsynchGraphicLoadingEvent(sdr::event::AsynchGraphicLoadingEvent* pEvent)
        {
            (void) pEvent; 

            if(mpAsynchLoadEvent)
            {
                OSL_ENSURE(!pEvent || mpAsynchLoadEvent == pEvent,
                    "ViewObjectContactOfGraphic::forgetAsynchGraphicLoadingEvent: Forced to forget another event then i have scheduled (?)");

                
                mpAsynchLoadEvent = 0;
            }
        }

        SdrGrafObj& ViewObjectContactOfGraphic::getSdrGrafObj()
        {
            return static_cast< ViewContactOfGraphic& >(GetViewContact()).GetGrafObject();
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewObjectContactOfGraphic::createPrimitive2DSequence(const DisplayInfo& rDisplayInfo) const
        {
            
            SdrGrafObj& rGrafObj = const_cast< ViewObjectContactOfGraphic* >(this)->getSdrGrafObj();
            bool bDoAsynchronGraphicLoading(rGrafObj.GetModel() && rGrafObj.GetModel()->IsSwapGraphics());
            bool bSwapInDone(false);
            bool bSwapInExclusive(false);

            if( bDoAsynchronGraphicLoading && rGrafObj.IsSwappedOut() )
            {
                
                
                if ( rGrafObj.GetPage() && rGrafObj.GetPage()->IsMasterPage() )
                {
                    
                    bDoAsynchronGraphicLoading = false;
                }
                else if ( GetObjectContact().isOutputToPrinter()
                    || GetObjectContact().isOutputToRecordingMetaFile()
                    || GetObjectContact().isOutputToPDFFile() )
                {
                    bDoAsynchronGraphicLoading = false;
                    bSwapInExclusive = true;
                }
            }
            if( bDoAsynchronGraphicLoading )
            {
                bSwapInDone = const_cast< ViewObjectContactOfGraphic* >(this)->impPrepareGraphicWithAsynchroniousLoading();
            }
            else
            {
                bSwapInDone = const_cast< ViewObjectContactOfGraphic* >(this)->impPrepareGraphicWithSynchroniousLoading();
            }

            
            drawinglayer::primitive2d::Primitive2DSequence xRetval = ViewObjectContactOfSdrObj::createPrimitive2DSequence(rDisplayInfo);

            if(xRetval.hasElements())
            {
                
                
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

            
            if( bSwapInDone && bSwapInExclusive )
            {
                rGrafObj.ForceSwapOut();
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
            
            if(mpAsynchLoadEvent)
            {
                
                
                delete mpAsynchLoadEvent;
            }
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

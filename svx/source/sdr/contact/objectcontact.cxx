/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <svx/sdr/contact/objectcontact.hxx>
#include <tools/debug.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/svdpage.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/event/eventhandler.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdr/animation/objectanimator.hxx>



using namespace com::sun::star;



namespace sdr
{
    namespace contact
    {
        ObjectContact::ObjectContact()
        :   maViewObjectContactVector(),
            maPrimitiveAnimator(),
            mpEventHandler(0),
            mpViewObjectContactRedirector(0),
            maViewInformation2D(uno::Sequence< beans::PropertyValue >()),
            mbIsPreviewRenderer(false)
        {
        }

        ObjectContact::~ObjectContact()
        {
            
            
            
            
            std::vector< ViewObjectContact* > aLocalVOCList(maViewObjectContactVector);
            maViewObjectContactVector.clear();

            while(!aLocalVOCList.empty())
            {
                ViewObjectContact* pCandidate = aLocalVOCList.back();
                aLocalVOCList.pop_back();
                DBG_ASSERT(pCandidate, "Corrupted ViewObjectContactList (!)");

                
                
                
                delete pCandidate;
            }

            
            DBG_ASSERT(maViewObjectContactVector.empty(), "Corrupted ViewObjectContactList (!)");

            
            DeleteEventHandler();
        }

        
        
        void ObjectContact::setLazyInvalidate(ViewObjectContact& rVOC)
        {
            rVOC.triggerLazyInvalidate();
        }

        
        void ObjectContact::PrepareProcessDisplay()
        {
        }

        
        void ObjectContact::AddViewObjectContact(ViewObjectContact& rVOContact)
        {
            maViewObjectContactVector.push_back(&rVOContact);
        }

        
        void ObjectContact::RemoveViewObjectContact(ViewObjectContact& rVOContact)
        {
            std::vector< ViewObjectContact* >::iterator aFindResult = std::find(maViewObjectContactVector.begin(), maViewObjectContactVector.end(), &rVOContact);

            if(aFindResult != maViewObjectContactVector.end())
            {
                maViewObjectContactVector.erase(aFindResult);
            }
        }

        
        void ObjectContact::ProcessDisplay(DisplayInfo& /*rDisplayInfo*/)
        {
            
        }

        
        bool ObjectContact::DoVisualizeEnteredGroup() const
        {
            
            return false;
        }

        
        const ViewContact* ObjectContact::getActiveViewContact() const
        {
            
            return 0;
        }

        
        
        void ObjectContact::InvalidatePartOfView(const basegfx::B2DRange& /*rRange*/) const
        {
            
        }

        
        bool ObjectContact::IsAreaVisible(const basegfx::B2DRange& /*rRange*/) const
        {
            
            return true;
        }

        
        bool ObjectContact::AreGluePointsVisible() const
        {
            return false;
        }

        
        sdr::event::TimerEventHandler* ObjectContact::CreateEventHandler()
        {
            
            return new sdr::event::TimerEventHandler();
        }

        
        sdr::animation::primitiveAnimator& ObjectContact::getPrimitiveAnimator()
        {
            return maPrimitiveAnimator;
        }

        
        
        sdr::event::TimerEventHandler& ObjectContact::GetEventHandler() const
        {
            if(!HasEventHandler())
            {
                const_cast< ObjectContact* >(this)->mpEventHandler = const_cast< ObjectContact* >(this)->CreateEventHandler();
                DBG_ASSERT(mpEventHandler, "ObjectContact::GetEventHandler(): Got no EventHandler (!)");
            }

            return *mpEventHandler;
        }

        
        void ObjectContact::DeleteEventHandler()
        {
            if(mpEventHandler)
            {
                
                delete mpEventHandler;
                mpEventHandler = 0L;
            }
        }

        
        bool ObjectContact::HasEventHandler() const
        {
            return (0L != mpEventHandler);
        }

        
        bool ObjectContact::IsTextAnimationAllowed() const
        {
            return true;
        }

        
        bool ObjectContact::IsGraphicAnimationAllowed() const
        {
            return true;
        }

        
        bool ObjectContact::IsAsynchronGraphicsLoadingAllowed() const
        {
            return false;
        }

        
        ViewObjectContactRedirector* ObjectContact::GetViewObjectContactRedirector() const
        {
            return mpViewObjectContactRedirector;
        }

        void ObjectContact::SetViewObjectContactRedirector(ViewObjectContactRedirector* pNew)
        {
            if(mpViewObjectContactRedirector != pNew)
            {
                mpViewObjectContactRedirector = pNew;
            }
        }

        
        bool ObjectContact::IsMasterPageBufferingAllowed() const
        {
            return false;
        }

        
        bool ObjectContact::isOutputToPrinter() const
        {
            return false;
        }

        
        bool ObjectContact::isOutputToWindow() const
        {
            return true;
        }

        
        bool ObjectContact::isOutputToVirtualDevice() const
        {
            return false;
        }

        
        bool ObjectContact::isOutputToRecordingMetaFile() const
        {
            return false;
        }

        
        bool ObjectContact::isOutputToPDFFile() const
        {
            return false;
        }

        
        bool ObjectContact::isDrawModeGray() const
        {
            return false;
        }

        
        bool ObjectContact::isDrawModeBlackWhite() const
        {
            return false;
        }

        
        bool ObjectContact::isDrawModeHighContrast() const
        {
            return false;
        }

        
        SdrPageView* ObjectContact::TryToGetSdrPageView() const
        {
            return 0;
        }

        
        OutputDevice* ObjectContact::TryToGetOutputDevice() const
        {
            return 0;
        }

        void ObjectContact::resetViewPort()
        {
            const drawinglayer::geometry::ViewInformation2D& rCurrentVI2D = getViewInformation2D();

            if(!rCurrentVI2D.getViewport().isEmpty())
            {
                const basegfx::B2DRange aEmptyRange;

                drawinglayer::geometry::ViewInformation2D aNewVI2D(
                    rCurrentVI2D.getObjectTransformation(),
                    rCurrentVI2D.getViewTransformation(),
                    aEmptyRange,
                    rCurrentVI2D.getVisualizedPage(),
                    rCurrentVI2D.getViewTime(),
                    rCurrentVI2D.getExtendedInformationSequence());

                updateViewInformation2D(aNewVI2D);
            }
        }

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

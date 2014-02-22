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

#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/color/bcolor.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdr/contact/objectcontactofpageview.hxx>
#include <tools/debug.hxx>



namespace sdr
{
    namespace contact
    {
        
        
        
        ViewObjectContact& ViewContact::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            return *(new ViewObjectContact(rObjectContact, *this));
        }

        ViewContact::ViewContact()
        :   maViewObjectContactVector(),
            mxViewIndependentPrimitive2DSequence()
        {
        }

        
        
        
        void ViewContact::StartGettingViewed()
        {
        }

        void ViewContact::StopGettingViewed()
        {
        }

        ViewContact::~ViewContact()
        {
            deleteAllVOCs();
        }

        void ViewContact::deleteAllVOCs()
        {
            
            
            
            
            std::vector< ViewObjectContact* > aLocalVOCList(maViewObjectContactVector);
            maViewObjectContactVector.clear();

            while(!aLocalVOCList.empty())
            {
                ViewObjectContact* pCandidate = aLocalVOCList.back();
                aLocalVOCList.pop_back();
                DBG_ASSERT(pCandidate, "Corrupted ViewObjectContactList in VC (!)");

                
                
                
                delete pCandidate;
            }

            
            DBG_ASSERT(maViewObjectContactVector.empty(), "Corrupted ViewObjectContactList in VC (!)");
        }

        
        
        ViewObjectContact& ViewContact::GetViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = 0L;
            const sal_uInt32 nCount(maViewObjectContactVector.size());

            
            for(sal_uInt32 a(0); !pRetval && a < nCount; a++)
            {
                ViewObjectContact* pCandidate = maViewObjectContactVector[a];
                DBG_ASSERT(pCandidate, "Corrupted ViewObjectContactList (!)");

                if(&(pCandidate->GetObjectContact()) == &rObjectContact)
                {
                    pRetval = pCandidate;
                }
            }

            if(!pRetval)
            {
                
                
                pRetval = &CreateObjectSpecificViewObjectContact(rObjectContact);
            }

            return *pRetval;
        }

        
        void ViewContact::AddViewObjectContact(ViewObjectContact& rVOContact)
        {
            maViewObjectContactVector.push_back(&rVOContact);

            if(1L == maViewObjectContactVector.size())
            {
                StartGettingViewed();
            }
        }

        
        void ViewContact::RemoveViewObjectContact(ViewObjectContact& rVOContact)
        {
            std::vector< ViewObjectContact* >::iterator aFindResult = std::find(maViewObjectContactVector.begin(), maViewObjectContactVector.end(), &rVOContact);

            if(aFindResult != maViewObjectContactVector.end())
            {
                maViewObjectContactVector.erase(aFindResult);

                if(maViewObjectContactVector.empty())
                {
                    
                    
                    StopGettingViewed();
                }
            }
        }

        
        
        bool ViewContact::HasViewObjectContacts(bool bExcludePreviews) const
        {
            const sal_uInt32 nCount(maViewObjectContactVector.size());

            if(bExcludePreviews)
            {
                for(sal_uInt32 a(0); a < nCount; a++)
                {
                    if(!maViewObjectContactVector[a]->GetObjectContact().IsPreviewRenderer())
                    {
                        return true;
                    }
                }

                return false;
            }
            else
            {
                return (0L != nCount);
            }
        }

        
        
        bool ViewContact::isAnimatedInAnyViewObjectContact() const
        {
            const sal_uInt32 nCount(maViewObjectContactVector.size());

            for(sal_uInt32 a(0); a < nCount; a++)
            {
                if(maViewObjectContactVector[a]->isAnimated())
                {
                    return true;
                }
            }

            return false;
        }

        
        
        
        sal_uInt32 ViewContact::GetObjectCount() const
        {
            
            return 0;
        }

        ViewContact& ViewContact::GetViewContact(sal_uInt32 /*nIndex*/) const
        {
            
            OSL_FAIL("ViewContact::GetViewContact: This call needs to be overloaded when GetObjectCount() can return results != 0 (!)");
            return (ViewContact&)(*this);
        }

        ViewContact* ViewContact::GetParentContact() const
        {
            
            return 0;
        }

        void ViewContact::ActionChildInserted(ViewContact& rChild)
        {
            
            
            const sal_uInt32 nCount(maViewObjectContactVector.size());

            for(sal_uInt32 a(0); a < nCount; a++)
            {
                ViewObjectContact* pCandidate = maViewObjectContactVector[a];
                DBG_ASSERT(pCandidate, "ViewContact::GetViewObjectContact() invalid ViewObjectContactList (!)");

                
                
                pCandidate->ActionChildInserted(rChild);
            }
        }

        
        void ViewContact::ActionChanged()
        {
            
            
            const sal_uInt32 nCount(maViewObjectContactVector.size());

            for(sal_uInt32 a(0); a < nCount; a++)
            {
                ViewObjectContact* pCandidate = maViewObjectContactVector[a];
                DBG_ASSERT(pCandidate, "ViewContact::GetViewObjectContact() invalid ViewObjectContactList (!)");

                pCandidate->ActionChanged();
            }
        }

        
        
        SdrObject* ViewContact::TryToGetSdrObject() const
        {
            return 0L;
        }

        SdrPage* ViewContact::TryToGetSdrPage() const
        {
            return 0L;
        }

        
        

        drawinglayer::primitive2d::Primitive2DSequence ViewContact::createViewIndependentPrimitive2DSequence() const
        {
            
            
            
            
            
            OSL_FAIL("ViewContact::createViewIndependentPrimitive2DSequence(): Never call the fallback base implementation, this is always an error (!)");
            const basegfx::B2DPolygon aOutline(basegfx::tools::createPolygonFromRect(basegfx::B2DRange(1000.0, 1000.0, 5000.0, 3000.0)));
            const basegfx::BColor aYellow(1.0, 1.0, 0.0);
            const drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(aOutline, aYellow));

            return drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContact::getViewIndependentPrimitive2DSequence() const
        {
            
            drawinglayer::primitive2d::Primitive2DSequence xNew(createViewIndependentPrimitive2DSequence());

            if(xNew.hasElements())
            {
                
                xNew = embedToObjectSpecificInformation(xNew);
            }

            if(!drawinglayer::primitive2d::arePrimitive2DSequencesEqual(mxViewIndependentPrimitive2DSequence, xNew))
            {
                
                const_cast< ViewContact* >(this)->mxViewIndependentPrimitive2DSequence = xNew;
            }

            
            return mxViewIndependentPrimitive2DSequence;
        }

        
        drawinglayer::primitive2d::Primitive2DSequence ViewContact::createGluePointPrimitive2DSequence() const
        {
            
            return drawinglayer::primitive2d::Primitive2DSequence();
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContact::embedToObjectSpecificInformation(const drawinglayer::primitive2d::Primitive2DSequence& rSource) const
        {
            
            return rSource;
        }

        void ViewContact::flushViewObjectContacts(bool bWithHierarchy)
        {
            if(bWithHierarchy)
            {
                
                const sal_uInt32 nCount(GetObjectCount());

                for(sal_uInt32 a(0); a < nCount; a++)
                {
                    ViewContact& rChild = GetViewContact(a);
                    rChild.flushViewObjectContacts(bWithHierarchy);
                }
            }

            
            deleteAllVOCs();
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        // Create a Object-Specific ViewObjectContact, set ViewContact and
        // ObjectContact. Always needs to return something. Default is to create
        // a standard ViewObjectContact containing the given ObjectContact and *this
        ViewObjectContact& ViewContact::CreateObjectSpecificViewObjectContact(ObjectContact& rObjectContact)
        {
            return *(new ViewObjectContact(rObjectContact, *this));
        }

        ViewContact::ViewContact()
        :   maViewObjectContactVector(),
            mxViewIndependentPrimitive2DSequence()
        {
        }

        // Methods to react on start getting viewed or stop getting
        // viewed. This info is derived from the count of members of
        // registered ViewObjectContacts. Default does nothing.
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
            // get rid of all VOCs
            // #i84257# To avoid that each 'delete pCandidate' again uses
            // the local RemoveViewObjectContact with a search and removal in the
            // vector, simply copy and clear local vector.
            std::vector< ViewObjectContact* > aLocalVOCList(maViewObjectContactVector);
            maViewObjectContactVector.clear();

            while(!aLocalVOCList.empty())
            {
                ViewObjectContact* pCandidate = aLocalVOCList.back();
                aLocalVOCList.pop_back();
                DBG_ASSERT(pCandidate, "Corrupted ViewObjectContactList in VC (!)");

                // ViewObjectContacts only make sense with View and Object contacts.
                // When the contact to the SdrObject is deleted like in this case,
                // all ViewObjectContacts can be deleted, too.
                delete pCandidate;
            }

            // assert when there were new entries added during deletion
            DBG_ASSERT(maViewObjectContactVector.empty(), "Corrupted ViewObjectContactList in VC (!)");
        }

        // get a Object-specific ViewObjectContact for a specific
        // ObjectContact (->View). Always needs to return something.
        ViewObjectContact& ViewContact::GetViewObjectContact(ObjectContact& rObjectContact)
        {
            ViewObjectContact* pRetval = 0L;
            const sal_uInt32 nCount(maViewObjectContactVector.size());

            // first search if there exists a VOC for the given OC
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
                // create a new one. It's inserted to the local list from the
                // VieObjectContact constructor via AddViewObjectContact()
                pRetval = &CreateObjectSpecificViewObjectContact(rObjectContact);
            }

            return *pRetval;
        }

        // A new ViewObjectContact was created and shall be remembered.
        void ViewContact::AddViewObjectContact(ViewObjectContact& rVOContact)
        {
            maViewObjectContactVector.push_back(&rVOContact);

            if(1L == maViewObjectContactVector.size())
            {
                StartGettingViewed();
            }
        }

        // A ViewObjectContact was deleted and shall be forgotten.
        void ViewContact::RemoveViewObjectContact(ViewObjectContact& rVOContact)
        {
            std::vector< ViewObjectContact* >::iterator aFindResult = std::find(maViewObjectContactVector.begin(), maViewObjectContactVector.end(), &rVOContact);

            if(aFindResult != maViewObjectContactVector.end())
            {
                maViewObjectContactVector.erase(aFindResult);

                if(maViewObjectContactVector.empty())
                {
                    // This may need to get asynchron later since it eventually triggers
                    // deletes of OCs where the VOC is still added.
                    StopGettingViewed();
                }
            }
        }

        // Test if this ViewContact has ViewObjectContacts at all. This can
        // be used to test if this ViewContact is visualized ATM or not
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

        // Test if this ViewContact has ViewObjectContacts at all. This can
        // be used to test if this ViewContact is visualized ATM or not
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

        // Access to possible sub-hierarchy and parent. GetObjectCount() default is 0L
        // and GetViewContact default pops up an assert since it's an error if
        // GetObjectCount has a result != 0 and it's not overloaded.
        sal_uInt32 ViewContact::GetObjectCount() const
        {
            // no sub-objects
            return 0;
        }

        ViewContact& ViewContact::GetViewContact(sal_uInt32 /*nIndex*/) const
        {
            // This is the default implementation; call would be an error
            OSL_FAIL("ViewContact::GetViewContact: This call needs to be overloaded when GetObjectCount() can return results != 0 (!)");
            return (ViewContact&)(*this);
        }

        ViewContact* ViewContact::GetParentContact() const
        {
            // default has no parent
            return 0;
        }

        void ViewContact::ActionChildInserted(ViewContact& rChild)
        {
            // propagate change to all exsisting visualisations which
            // will force a VOC for the new child and invalidate it's range
            const sal_uInt32 nCount(maViewObjectContactVector.size());

            for(sal_uInt32 a(0); a < nCount; a++)
            {
                ViewObjectContact* pCandidate = maViewObjectContactVector[a];
                DBG_ASSERT(pCandidate, "ViewContact::GetViewObjectContact() invalid ViewObjectContactList (!)");

                // take action at all VOCs. At the VOCs ObjectContact the initial
                // rectangle will be invalidated at the associated OutputDevice.
                pCandidate->ActionChildInserted(rChild);
            }
        }

        // React on changes of the object of this ViewContact
        void ViewContact::ActionChanged()
        {
            // propagate change to all existing VOCs. This will invalidate
            // all drawn visualisations in all known views
            const sal_uInt32 nCount(maViewObjectContactVector.size());

            for(sal_uInt32 a(0); a < nCount; a++)
            {
                ViewObjectContact* pCandidate = maViewObjectContactVector[a];
                DBG_ASSERT(pCandidate, "ViewContact::GetViewObjectContact() invalid ViewObjectContactList (!)");

                pCandidate->ActionChanged();
            }
        }

        // access to SdrObject and/or SdrPage. May return 0L like the default
        // implementations do. Needs to be overloaded as needed.
        SdrObject* ViewContact::TryToGetSdrObject() const
        {
            return 0L;
        }

        SdrPage* ViewContact::TryToGetSdrPage() const
        {
            return 0L;
        }

        //////////////////////////////////////////////////////////////////////////////
        // primitive stuff

        drawinglayer::primitive2d::Primitive2DSequence ViewContact::createViewIndependentPrimitive2DSequence() const
        {
            // This is the default impelemtation and should never be called (see header). If this is called,
            // someone implemented a ViewContact (VC) visualisation object without defining the visualisation by
            // providing a seqence of primitives -> which cannot be correct.
            // Since we have no access to any known model data here, the default implementation creates a yellow placeholder
            // hairline polygon with a default size of (1000, 1000, 5000, 3000)
            OSL_FAIL("ViewContact::createViewIndependentPrimitive2DSequence(): Never call the fallback base implementation, this is always an error (!)");
            const basegfx::B2DPolygon aOutline(basegfx::tools::createPolygonFromRect(basegfx::B2DRange(1000.0, 1000.0, 5000.0, 3000.0)));
            const basegfx::BColor aYellow(1.0, 1.0, 0.0);
            const drawinglayer::primitive2d::Primitive2DReference xReference(
                new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(aOutline, aYellow));

            return drawinglayer::primitive2d::Primitive2DSequence(&xReference, 1);
        }

        drawinglayer::primitive2d::Primitive2DSequence ViewContact::getViewIndependentPrimitive2DSequence() const
        {
            // local up-to-date checks. Create new list and compare.
            const drawinglayer::primitive2d::Primitive2DSequence xNew(createViewIndependentPrimitive2DSequence());

            if(!drawinglayer::primitive2d::arePrimitive2DSequencesEqual(mxViewIndependentPrimitive2DSequence, xNew))
            {
                // has changed, copy content
                const_cast< ViewContact* >(this)->mxViewIndependentPrimitive2DSequence = xNew;
            }

            // return current Primitive2DSequence
            return mxViewIndependentPrimitive2DSequence;
        }

        // add Gluepoints (if available)
        drawinglayer::primitive2d::Primitive2DSequence ViewContact::createGluePointPrimitive2DSequence() const
        {
            // default returns empty reference
            return drawinglayer::primitive2d::Primitive2DSequence();
        }

        void ViewContact::flushViewObjectContacts(bool bWithHierarchy)
        {
            if(bWithHierarchy)
            {
                // flush DrawingLayer hierarchy
                const sal_uInt32 nCount(GetObjectCount());

                for(sal_uInt32 a(0); a < nCount; a++)
                {
                    ViewContact& rChild = GetViewContact(a);
                    rChild.flushViewObjectContacts(bWithHierarchy);
                }
            }

            // delete local VOCs
            deleteAllVOCs();
        }
    } // end of namespace contact
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

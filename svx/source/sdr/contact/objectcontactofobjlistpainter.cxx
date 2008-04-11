/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: objectcontactofobjlistpainter.cxx,v $
 * $Revision: 1.11 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdobj.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/svdmodel.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        ObjectContactPainter::ObjectContactPainter(sal_Bool bBufferingAllowed)
        :   mbIsInitialized(sal_False),
            mbBufferingAllowed(bBufferingAllowed)
        {
        }

        // The destructor.
        ObjectContactPainter::~ObjectContactPainter()
        {
        }

        // Update Draw Hierarchy data
        void ObjectContactPainter::EnsureValidDrawHierarchy(DisplayInfo& /*rDisplayInfo*/)
        {
            const sal_uInt32 nCount(maDrawHierarchy.Count());

            // #i35614#
            if(mbIsInitialized && !nCount)
            {
                // DrawHierarchy is not only invalid, but also completely
                // removed. Thus, we need to change the mbIsInitialized flag
                // to express that state.
                mbIsInitialized = sal_False;
            }

            if(mbIsInitialized)
            {
                // no new StartPoint, is the invalid flag set which means
                // somewhere the sub-hierarchy is invalid?
                if(!IsDrawHierarchyValid())
                {
                    // Yes, check the sub-hierarchies

                    for(sal_uInt32 a(0L); a < nCount; a++)
                    {
                        ViewObjectContact& rVOContact = *(maDrawHierarchy.GetObject(a));

                        rVOContact.CheckDrawHierarchy(*this);
                    }
                }
            }
            else
            {
                // build new hierarchy
                const sal_uInt32 nCount2(GetPaintObjectCount());

                for(sal_uInt32 a(0L); a < nCount2; a++)
                {
                    ViewContact& rViewContact = GetPaintObjectViewContact(a);
                    ViewObjectContact& rViewObjectContact = rViewContact.GetViewObjectContact(*this);

                    // set parent at ViewObjectContact
                    rViewObjectContact.SetParent(0L);

                    // build sub-hierarchy
                    if(rViewContact.GetObjectCount())
                    {
                        rViewObjectContact.BuildDrawHierarchy(*this, rViewContact);
                    }

                    // Add to list
                    maDrawHierarchy.Append(&rViewObjectContact);
                }

                // remember to be initialized
                mbIsInitialized = sal_True;
            }

            // set DrawHierarchy valid
            mbDrawHierarchyValid = sal_True;
        }

        // Process the whole displaying
        void ObjectContactPainter::ProcessDisplay(DisplayInfo& rDisplayInfo)
        {
            if(!IsDrawHierarchyValid())
            {
                // The default implementation ensures a valid draw hierarchy.
                EnsureValidDrawHierarchy(rDisplayInfo);
            }

            if( mbIsInitialized
                && IsDrawHierarchyValid()
                && rDisplayInfo.GetPaintInfoRec()
                && rDisplayInfo.GetOutputDevice()
                && GetPaintObjectCount())
            {
                // This class is normally used for producing a single output. Thus,
                // buffering makes no sense and is switched off here.
                rDisplayInfo.SetBufferingAllowed(mbBufferingAllowed);

                // Paint Hierarchy
                for(sal_uInt32 a(0L); a < maDrawHierarchy.Count(); a++)
                {
                    ViewObjectContact& rViewObjectContact = *(maDrawHierarchy.GetObject(a));

                    // paint Hierarchy
                    rViewObjectContact.PaintObjectHierarchy(rDisplayInfo);
                }
            }
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        sal_uInt32 ObjectContactOfObjListPainter::GetPaintObjectCount() const
        {
            return maStartObjects.size();
        }

        ViewContact& ObjectContactOfObjListPainter::GetPaintObjectViewContact(sal_uInt32 nIndex) const
        {
            const SdrObject* pObj = maStartObjects[nIndex];
            DBG_ASSERT(pObj, "ObjectContactOfObjListPainter::EnsureValidDrawHierarchy: Corrupt SdrObjectVector (!)");
            return pObj->GetViewContact();
        }

        ObjectContactOfObjListPainter::ObjectContactOfObjListPainter(
            const SdrObjectVector& rObjects,
            sal_Bool bBufferingAllowed)
        :   ObjectContactPainter(bBufferingAllowed),
            maStartObjects(rObjects)
        {
        }

        ObjectContactOfObjListPainter::~ObjectContactOfObjListPainter()
        {
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        sal_uInt32 ObjectContactOfPagePainter::GetPaintObjectCount() const
        {
            return 1L;
        }

        ViewContact& ObjectContactOfPagePainter::GetPaintObjectViewContact(sal_uInt32 /*nIndex*/) const
        {
            DBG_ASSERT(mpStartPage,
                "ObjectContactOfPagePainter::GetPaintObjectViewContact: no mpStartPage set (!)");
            return mpStartPage->GetViewContact();
        }

        ObjectContactOfPagePainter::ObjectContactOfPagePainter(
            const SdrPage* pPage,
            sal_Bool bBufferingAllowed)
        :   ObjectContactPainter(bBufferingAllowed),
            mpStartPage(pPage)
        {
        }

        ObjectContactOfPagePainter::~ObjectContactOfPagePainter()
        {
        }

        void ObjectContactOfPagePainter::SetStartPage(const SdrPage* pPage)
        {
            if(pPage && pPage != mpStartPage)
            {
                ClearDrawHierarchy();
                mpStartPage = pPage;
                mbIsInitialized = sal_False;
            }
        }
    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof

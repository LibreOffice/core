/*************************************************************************
 *
 *  $RCSfile: objectcontactofobjlistpainter.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-17 09:46:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SDR_CONTACT_OBJECTCONTACTOFOBJLISTPAINTER_HXX
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#endif

#ifndef _SDR_CONTACT_DISPLAYINFO_HXX
#include <svx/sdr/contact/displayinfo.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWOBJECTCONTACT_HXX
#include <svx/sdr/contact/viewobjectcontact.hxx>
#endif

#ifndef _SVDPAGE_HXX
#include <svdpage.hxx>
#endif

#ifndef _SVDOBJ_HXX
#include <svdobj.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWCONTACT_HXX
#include <svx/sdr/contact/viewcontact.hxx>
#endif

#ifndef _SVDMODEL_HXX
#include <svdmodel.hxx>
#endif

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
        void ObjectContactPainter::EnsureValidDrawHierarchy(DisplayInfo& rDisplayInfo)
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
                const sal_uInt32 nCount(GetPaintObjectCount());

                for(sal_uInt32 a(0L); a < nCount; a++)
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
            SdrObject* pObj = maStartObjects[nIndex];
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

        ViewContact& ObjectContactOfPagePainter::GetPaintObjectViewContact(sal_uInt32 nIndex) const
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

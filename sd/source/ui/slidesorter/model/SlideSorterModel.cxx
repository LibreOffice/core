/*************************************************************************
 *
 *  $RCSfile: SlideSorterModel.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:24:07 $
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

#include "model/SlideSorterModel.hxx"

#include "model/SlsPageDescriptor.hxx"
#include "model/SlsPageEnumeration.hxx"
#include "controller/SlsPageObjectFactory.hxx"

#ifndef _DRAWDOC_HXX
#include "drawdoc.hxx"
#endif
#ifndef _SDPAGE_HXX
#include "sdpage.hxx"
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

#ifdef DEBUG
#define DUMP_MODEL ::DumpSlideSorterModel(*this)
namespace {
using namespace ::sd::slidesorter::model;
void DumpSlideSorterModel (const SlideSorterModel& rModel)
{
    OSL_TRACE ("SlideSorterModel has %d pages", rModel.GetPageCount());
    if (rModel.GetEditMode() == EM_PAGE)
        OSL_TRACE ("edit mode is EM_PAGE");
    else if (rModel.GetEditMode() == EM_MASTERPAGE)
        OSL_TRACE ("    edit mode is EM_MASTERPAGE");
    else
        OSL_TRACE ("    edit mode is unknown");
    for (int i=0; i<rModel.GetPageCount(); i++)
    {
        PageDescriptor* pDescriptor = rModel.GetRawPageDescriptor(i);
        OSL_TRACE ("    page %d points to %x", i, pDescriptor);
        if (pDescriptor != NULL)
            OSL_TRACE ("        focused %d, selected %d, visible %d",
                pDescriptor->IsFocused()?1:0,
                pDescriptor->IsSelected()?1:0,
                pDescriptor->IsVisible()?1:0);
    }
}
}
#else
#define DUMP_MODEL
#endif
#undef DUMP_MODEL
#define DUMP_MODEL

namespace sd { namespace slidesorter { namespace model {

SlideSorterModel::SlideSorterModel (
    SdDrawDocument& rDocument,
    PageKind ePageKind,
    EditMode eEditMode)
    : mrDocument (rDocument),
      mePageKind (ePageKind),
      meEditMode (eEditMode),
      maPageDescriptors(0),
      mpPageObjectFactory(NULL)
{
    AdaptSize ();
    DUMP_MODEL;
}




SlideSorterModel::~SlideSorterModel (void)
{
    ClearDescriptorList ();
}




SdDrawDocument* SlideSorterModel::GetDocument (void)
{
    return &mrDocument;
}




bool SlideSorterModel::SetEditMode (EditMode eEditMode)
{
    bool bEditModeChanged = false;
    if (meEditMode!=eEditMode)
    {
        meEditMode = eEditMode;
        ClearDescriptorList();
        AdaptSize();
        bEditModeChanged = true;
    }
    DUMP_MODEL;
    return bEditModeChanged;
}




EditMode SlideSorterModel::GetEditMode (void) const
{
    return meEditMode;
}




PageKind SlideSorterModel::GetPageType (void) const
{
    return mePageKind;
}




int SlideSorterModel::GetPageCount (void) const
{
    return maPageDescriptors.size();
}




PageDescriptor* SlideSorterModel::GetPageDescriptor (int nPageIndex) const
{
    ::osl::MutexGuard aGuard (maMutex);

    PageDescriptor* pDescriptor = NULL;
    if (nPageIndex>=0 && nPageIndex<GetPageCount())
    {
        pDescriptor = maPageDescriptors[nPageIndex];
        if (pDescriptor == NULL)
        {
            SdPage* pPage;
            if (meEditMode == EM_PAGE)
                pPage = mrDocument.GetSdPage (nPageIndex, mePageKind);
            else
                pPage = mrDocument.GetMasterSdPage (nPageIndex, mePageKind);
            pDescriptor = new PageDescriptor (
                *pPage,
                GetPageObjectFactory());
            maPageDescriptors[nPageIndex] = pDescriptor;
        }
    }
    return pDescriptor;
}




PageDescriptor* SlideSorterModel::GetRawPageDescriptor (int nPageIndex) const
{
    ::osl::MutexGuard aGuard (maMutex);

    PageDescriptor* pDescriptor = NULL;
    if (nPageIndex>=0 && nPageIndex<GetPageCount())
        pDescriptor = maPageDescriptors[nPageIndex];
    return pDescriptor;
}




PageDescriptor* SlideSorterModel::FindPageDescriptor (
        const Reference<drawing::XDrawPage>& rxPage) const
{
    ::osl::MutexGuard aGuard (maMutex);

    PageDescriptor* pDescriptor = NULL;
    for (int i=0; i<GetPageCount(); i++)
    {
        pDescriptor = GetPageDescriptor(i);
        if (pDescriptor != NULL)
        {
            Reference<drawing::XDrawPage> xPage (
                pDescriptor->GetPage()->getUnoPage(), UNO_QUERY);
            if (xPage == rxPage)
                break;
        }
    }
    return pDescriptor;
}




SlideSorterModel::Enumeration
    SlideSorterModel::GetAllPagesEnumeration (void) const
{
    return PageEnumeration::Create(*this, PageEnumeration::PET_ALL);
}




SlideSorterModel::Enumeration
    SlideSorterModel::GetSelectedPagesEnumeration (void) const
{
    return PageEnumeration::Create(*this, PageEnumeration::PET_SELECTED);
}




SlideSorterModel::Enumeration
    SlideSorterModel::GetVisiblePagesEnumeration (void) const
{
    return PageEnumeration::Create(*this, PageEnumeration::PET_VISIBLE);
}




/** For now this method uses a trivial algorithm: throw away all descriptors
    and create them anew (on demand).
*/
void SlideSorterModel::Resync (void)
{
    ::osl::MutexGuard aGuard (maMutex);
    OSL_TRACE("Resync in");
    DUMP_MODEL;
    ClearDescriptorList ();
    AdaptSize();
    DUMP_MODEL;
    OSL_TRACE("Resync out");
}




void SlideSorterModel::AdaptSize ()
{
    ::osl::MutexGuard aGuard (maMutex);

    if (meEditMode == EM_PAGE)
        maPageDescriptors.resize (
            mrDocument.GetSdPageCount(mePageKind),
            NULL);
    else
        maPageDescriptors.resize (
            mrDocument.GetMasterSdPageCount(mePageKind),
            NULL);
}



void SlideSorterModel::ClearDescriptorList (void)
{
    ::osl::MutexGuard aGuard (maMutex);

    // Clear the cache of page descriptors.
    DescriptorContainer::iterator I;
    for (I=maPageDescriptors.begin(); I!=maPageDescriptors.end(); I++)
    {
        if (*I != NULL)
        {
            delete *I;
            *I = NULL;
        }
    }
}




void SlideSorterModel::SynchronizeDocumentSelection (void)
{
    ::osl::MutexGuard aGuard (maMutex);

    Enumeration aAllPages (GetAllPagesEnumeration());
    while (aAllPages.HasMoreElements())
    {
        PageDescriptor& rDescriptor (aAllPages.GetNextElement());
        rDescriptor.GetPage()->SetSelected (rDescriptor.IsSelected());
    }
}




void SlideSorterModel::SynchronizeModelSelection (void)
{
    ::osl::MutexGuard aGuard (maMutex);

    Enumeration aAllPages (GetAllPagesEnumeration());
    while (aAllPages.HasMoreElements())
    {
        PageDescriptor& rDescriptor (aAllPages.GetNextElement());
        if (rDescriptor.GetPage()->IsSelected())
            rDescriptor.Select ();
        else
            rDescriptor.Deselect ();
    }
}




void SlideSorterModel::SetPageObjectFactory(
    ::std::auto_ptr<controller::PageObjectFactory> pPageObjectFactory)
{
    ::osl::MutexGuard aGuard (maMutex);

    mpPageObjectFactory = pPageObjectFactory;
    // When a NULL pointer was given then create a default factory.
    const controller::PageObjectFactory& rFactory (GetPageObjectFactory());
    Enumeration aAllPages (GetAllPagesEnumeration());
    while (aAllPages.HasMoreElements())
    {
        PageDescriptor& rDescriptor (aAllPages.GetNextElement());
        rDescriptor.SetPageObjectFactory(rFactory);
    }
}




const controller::PageObjectFactory&
    SlideSorterModel::GetPageObjectFactory (void) const
{
    ::osl::MutexGuard aGuard (maMutex);

    if (mpPageObjectFactory.get() == NULL)
    {
        // We have to creat a new factory.  The pointer is mutable so we are
        // alowed to do so.  Note that we pass NULL as pointer to the
        // preview cache because we, as a model, have no access to the
        // cache.  This makes this object clearly a fallback when the
        // controller does not provide a factory where the cache is properly
        // set.
        mpPageObjectFactory = ::std::auto_ptr<controller::PageObjectFactory> (
            new controller::PageObjectFactory(NULL));
    }
    return *mpPageObjectFactory.get();
}




::osl::Mutex& SlideSorterModel::GetMutex (void)
{
    return maMutex;
}

} } } // end of namespace ::sd::slidesorter::model

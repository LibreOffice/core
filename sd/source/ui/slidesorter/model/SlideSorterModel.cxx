/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlideSorterModel.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 13:45:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "model/SlideSorterModel.hxx"

#include "model/SlsPageDescriptor.hxx"
#include "model/SlsPageEnumeration.hxx"
#include "controller/SlsPageObjectFactory.hxx"
#include "taskpane/SlideSorterCacheDisplay.hxx"

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
        SharedPageDescriptor pDescriptor = rModel.GetRawPageDescriptor(i);
        OSL_TRACE ("    page %d points to %x", i, pDescriptor.get());
        if (pDescriptor.get() != NULL)
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




SharedPageDescriptor SlideSorterModel::GetPageDescriptor (int nPageIndex) const
{
    ::osl::MutexGuard aGuard (maMutex);

    SharedPageDescriptor pDescriptor;
    if (nPageIndex>=0 && nPageIndex<GetPageCount())
    {
        pDescriptor = maPageDescriptors[nPageIndex];
        if (pDescriptor == NULL)
        {
            SdPage* pPage;
            if (meEditMode == EM_PAGE)
                pPage = mrDocument.GetSdPage ((USHORT)nPageIndex, mePageKind);
            else
                pPage = mrDocument.GetMasterSdPage ((USHORT)nPageIndex, mePageKind);
            pDescriptor.reset(new PageDescriptor (
                *pPage,
                GetPageObjectFactory()));
            maPageDescriptors[nPageIndex] = pDescriptor;
        }
    }
    return pDescriptor;
}




SharedPageDescriptor SlideSorterModel::GetRawPageDescriptor (int nPageIndex) const
{
    ::osl::MutexGuard aGuard (maMutex);

    SharedPageDescriptor pDescriptor;
    if (nPageIndex>=0 && nPageIndex<GetPageCount())
        pDescriptor = maPageDescriptors[nPageIndex];
    return pDescriptor;
}




SharedPageDescriptor SlideSorterModel::FindPageDescriptor (
        const Reference<drawing::XDrawPage>& rxPage) const
{
    ::osl::MutexGuard aGuard (maMutex);

    SharedPageDescriptor pDescriptor;
    for (int i=0; i<GetPageCount(); i++)
    {
        pDescriptor = GetPageDescriptor(i);
        if (pDescriptor.get() != NULL)
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
    and create them anew (on demand).  The main problem that we are facing
    when designing a better algorithm is that we can not compare pointers to
    pages stored in the PageDescriptor objects and those obtained from the
    document: pages may have been deleted and others may have been created
    at the exact same memory locations.
*/
void SlideSorterModel::Resync (void)
{
    ::osl::MutexGuard aGuard (maMutex);
    DUMP_MODEL;
    ClearDescriptorList ();
    AdaptSize();
    DUMP_MODEL;
}




void SlideSorterModel::AdaptSize ()
{
    ::osl::MutexGuard aGuard (maMutex);

    if (meEditMode == EM_PAGE)
        maPageDescriptors.resize(mrDocument.GetSdPageCount(mePageKind));
    else
        maPageDescriptors.resize(mrDocument.GetMasterSdPageCount(mePageKind));

#ifdef USE_SLIDE_SORTER_PAGE_CACHE
    toolpanel::SlideSorterCacheDisplay* pDisplay = toolpanel::SlideSorterCacheDisplay::Instance(&mrDocument);
    if (pDisplay != NULL)
        pDisplay->SetPageCount(mrDocument.GetSdPageCount(mePageKind));
#endif
}



void SlideSorterModel::ClearDescriptorList (void)
{
    ::osl::MutexGuard aGuard (maMutex);

    // Clear the cache of page descriptors.
    DescriptorContainer::iterator I;
    for (I=maPageDescriptors.begin(); I!=maPageDescriptors.end(); I++)
    {
        if (I->get() != NULL)
        {
            if ( ! I->unique())
            {
                OSL_TRACE("SlideSorterModel::ClearDescriptorList: trying to delete page descriptor  that is still used with count %d", I->use_count());
                // No assertion here because that can hang the office when
                // opening a dialog from here.
            }
            I->reset();
        }
    }
}




void SlideSorterModel::SynchronizeDocumentSelection (void)
{
    ::osl::MutexGuard aGuard (maMutex);

    Enumeration aAllPages (GetAllPagesEnumeration());
    while (aAllPages.HasMoreElements())
    {
        SharedPageDescriptor pDescriptor (aAllPages.GetNextElement());
        pDescriptor->GetPage()->SetSelected (pDescriptor->IsSelected());
    }
}




void SlideSorterModel::SynchronizeModelSelection (void)
{
    ::osl::MutexGuard aGuard (maMutex);

    Enumeration aAllPages (GetAllPagesEnumeration());
    while (aAllPages.HasMoreElements())
    {
        SharedPageDescriptor pDescriptor (aAllPages.GetNextElement());
        if (pDescriptor->GetPage()->IsSelected())
            pDescriptor->Select ();
        else
            pDescriptor->Deselect ();
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
        SharedPageDescriptor pDescriptor (aAllPages.GetNextElement());
        pDescriptor->SetPageObjectFactory(rFactory);
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
            new controller::PageObjectFactory(::boost::shared_ptr<cache::PageCache>()));
    }
    return *mpPageObjectFactory.get();
}




::osl::Mutex& SlideSorterModel::GetMutex (void)
{
    return maMutex;
}

} } } // end of namespace ::sd::slidesorter::model

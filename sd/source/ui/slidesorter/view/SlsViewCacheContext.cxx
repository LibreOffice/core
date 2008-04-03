/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsViewCacheContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:45:03 $
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

#include "SlsViewCacheContext.hxx"

#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsPageObjectViewObjectContact.hxx"
#include "sdpage.hxx"
#include "Window.hxx"
#include "drawdoc.hxx"
#include "tools/IdleDetection.hxx"
#include <svx/svdpage.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <vcl/window.hxx>

namespace sd { namespace slidesorter { namespace view {


ViewCacheContext::ViewCacheContext (
    model::SlideSorterModel& rModel,
    SlideSorterView& rView)
    : mrModel(rModel),
      mrView(rView)
{
}




ViewCacheContext::~ViewCacheContext (void)
{
}




void ViewCacheContext::NotifyPreviewCreation (
    cache::CacheKey aKey,
    const ::boost::shared_ptr<BitmapEx>& rPreview)
{
    (void)rPreview;
    const model::SharedPageDescriptor pDescriptor (GetDescriptor(aKey));
    if (pDescriptor.get() != NULL)
    {
        const Rectangle aDirtyBox (pDescriptor->GetViewObjectContact()->GetViewContact().GetPaintRectangle());
        mrView.InvalidateAllWin(aDirtyBox);
    }
    else
    {
        OSL_ASSERT(pDescriptor.get() != NULL);
    }
}




bool ViewCacheContext::IsIdle (void)
{
    sal_Int32 nIdleState (tools::IdleDetection::GetIdleState(mrView.GetWindow()));
    if (nIdleState == tools::IdleDetection::IDET_IDLE)
        return true;
    else
        return false;
}




bool ViewCacheContext::IsVisible (cache::CacheKey aKey)
{
    return GetDescriptor(aKey)->IsVisible();
}




const SdrPage* ViewCacheContext::GetPage (cache::CacheKey aKey)
{
    return static_cast<const SdrPage*>(aKey);
}




::boost::shared_ptr<std::vector<cache::CacheKey> > ViewCacheContext::GetEntryList (bool bVisible)
{
    ::boost::shared_ptr<std::vector<cache::CacheKey> > pKeys (new std::vector<cache::CacheKey>());

    model::PageEnumeration aPageEnumeration (
        bVisible
            ? model::PageEnumerationProvider::CreateVisiblePagesEnumeration(mrModel)
            : model::PageEnumerationProvider::CreateAllPagesEnumeration(mrModel));

    while (aPageEnumeration.HasMoreElements())
    {
        model::SharedPageDescriptor pDescriptor (aPageEnumeration.GetNextElement());
        pKeys->push_back(pDescriptor->GetPage());
    }

    return pKeys;
}




sal_Int32 ViewCacheContext::GetPriority (cache::CacheKey aKey)
{
    return - (static_cast<const SdrPage*>(aKey)->GetPageNum()-1) / 2;
}




model::SharedPageDescriptor ViewCacheContext::GetDescriptor (cache::CacheKey aKey)
{
    sal_uInt16 nPageIndex ((static_cast<const SdrPage*>(aKey)->GetPageNum() - 1) / 2);
    return mrModel.GetPageDescriptor(nPageIndex);
}




::com::sun::star::uno::Reference<com::sun::star::uno::XInterface> ViewCacheContext::GetModel (void)
{
    if (mrModel.GetDocument() == NULL)
        return NULL;
    else
        return mrModel.GetDocument()->getUnoModel();
}

} } } // end of namespace ::sd::slidesorter::view


/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsPageEnumeration.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:41:26 $
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

#include "precompiled_sd.hxx"

#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"

using namespace ::sd::slidesorter;
using namespace ::sd::slidesorter::model;

namespace {

class PageEnumerationImpl
    : public Enumeration<SharedPageDescriptor>
{
public:
    inline PageEnumerationImpl (
        const SlideSorterModel& rModel,
        const PageEnumeration::PagePredicate& rPredicate);
    virtual ~PageEnumerationImpl (void);
    /** Create a copy of the called enumeration object.
    */
    virtual inline ::std::auto_ptr<Enumeration<SharedPageDescriptor> > Clone (void);

    virtual inline bool HasMoreElements (void) const;
    virtual inline SharedPageDescriptor GetNextElement (void);
    virtual inline void Rewind (void);

private:
    const SlideSorterModel& mrModel;
    const PageEnumeration::PagePredicate maPredicate;
    int mnIndex;

    /** This constructor sets the internal page index to the given value.
        It does not call AdvanceToNextValidElement() to skip elements that
        do not fullfill Predicate.
    */
    inline PageEnumerationImpl (
        const SlideSorterModel& rModel,
        const PageEnumeration::PagePredicate& rPredicate,
        int nIndex);

    /** Skip all elements that do not fullfill Predicate starting with the
        one pointed to by mnIndex.
    */
    inline void AdvanceToNextValidElement (void);

    // Default constructor not implemented.
    PageEnumerationImpl (void);
    // Assignment operator not implemented.
    PageEnumerationImpl& operator= (const PageEnumerationImpl&);
};

} // end of anonymouse namespace




namespace sd { namespace slidesorter { namespace model {


PageEnumeration PageEnumeration::Create (
    const SlideSorterModel& rModel,
    const PagePredicate& rPredicate)
{
    return PageEnumeration(::std::auto_ptr<Enumeration<SharedPageDescriptor> >(
        new PageEnumerationImpl(rModel, rPredicate)));
}




PageEnumeration::PageEnumeration (
    ::std::auto_ptr<Enumeration<SharedPageDescriptor> > pImpl)
    : mpImpl(pImpl)
{
}




PageEnumeration::PageEnumeration (
    PageEnumeration& rEnumeration,
    bool bCloneImpl)
{

    if( bCloneImpl )
    {
        mpImpl = rEnumeration.mpImpl->Clone();
    }
    else
    {
        mpImpl = rEnumeration.mpImpl;
    }
}




PageEnumeration::PageEnumeration (const PageEnumeration& rEnumeration )
: sd::slidesorter::model::Enumeration<sd::slidesorter::model::SharedPageDescriptor>()
{
    mpImpl = rEnumeration.mpImpl->Clone();
}




PageEnumeration::~PageEnumeration (void)
{
}




PageEnumeration& PageEnumeration::operator= (
    const PageEnumeration& rEnumeration)
{
    mpImpl = rEnumeration.mpImpl->Clone();
    return *this;
}




::std::auto_ptr<Enumeration<SharedPageDescriptor> > PageEnumeration::Clone (void)
{
    return ::std::auto_ptr<Enumeration<SharedPageDescriptor> >(
        new PageEnumeration (*this, true));
}




bool PageEnumeration::HasMoreElements (void) const
{
    return mpImpl->HasMoreElements();
}



SharedPageDescriptor PageEnumeration::GetNextElement (void)
{
    return mpImpl->GetNextElement();
}




void PageEnumeration::Rewind (void)
{
    return mpImpl->Rewind();
}

} } } // end of namespace ::sd::slidesorter::model




namespace {

PageEnumerationImpl::PageEnumerationImpl (
    const SlideSorterModel& rModel,
    const PageEnumeration::PagePredicate& rPredicate)
    : mrModel(rModel),
      maPredicate(rPredicate),
      mnIndex(0)
{
    Rewind();
}




PageEnumerationImpl::PageEnumerationImpl (
    const SlideSorterModel& rModel,
    const PageEnumeration::PagePredicate& rPredicate,
    int nIndex)
    : mrModel(rModel),
      maPredicate(rPredicate),
      mnIndex(nIndex)
{
}




PageEnumerationImpl::~PageEnumerationImpl (void)
{
}




::std::auto_ptr<Enumeration<SharedPageDescriptor> >
    PageEnumerationImpl::Clone (void)
{
    return ::std::auto_ptr<Enumeration<SharedPageDescriptor> >(
        new PageEnumerationImpl(mrModel,maPredicate,mnIndex));
}




bool PageEnumerationImpl::HasMoreElements (void) const
{
    return (mnIndex < mrModel.GetPageCount());
}




SharedPageDescriptor PageEnumerationImpl::GetNextElement (void)
{
    SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(mnIndex));

    // Go to the following valid element.
    mnIndex += 1;
    AdvanceToNextValidElement();

    return pDescriptor;
}




void PageEnumerationImpl::Rewind (void)
{
    // Go to first valid element.
    mnIndex = 0;
    AdvanceToNextValidElement();
}





void PageEnumerationImpl::AdvanceToNextValidElement (void)
{
    while (mnIndex < mrModel.GetPageCount())
    {
        SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(mnIndex));

        // Test for the predicate being fullfilled.
        if (pDescriptor.get()!=NULL && maPredicate(pDescriptor))
        {
            // This predicate is valid.
            break;
        }
        else
        {
            // Advance to next predicate.
            mnIndex += 1;
        }
    }
}

} // end of anonymous namespace


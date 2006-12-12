/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsPageEnumeration.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 18:37:15 $
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

using namespace ::sd::slidesorter;
using namespace ::sd::slidesorter::model;

namespace {
template <class Predicate>
class PageEnumerationImpl
    : public Enumeration<SharedPageDescriptor>
{
public:
    inline PageEnumerationImpl (const SlideSorterModel& rModel);
    virtual ~PageEnumerationImpl (void);
    /** Create a copy of the called enumeration object.
    */
    virtual inline ::std::auto_ptr<Enumeration<SharedPageDescriptor> > Clone (void);

    virtual inline bool HasMoreElements (void) const;
    virtual inline SharedPageDescriptor GetNextElement (void);
    virtual inline void Rewind (void);

private:
    const SlideSorterModel& mrModel;
    int mnIndex;

    /** This constructor sets the internal page index to the given value.
        It does not call AdvanceToNextValidElement() to skip elements that
        do not fullfill Predicate.
    */
    inline PageEnumerationImpl (const SlideSorterModel& rModel, int nIndex);

    /** Skip all elements that do not fullfill Predicate starting with the
        one pointed to by mnIndex.
    */
    inline void AdvanceToNextValidElement (void);

    // Default constructor not implemented.
    PageEnumerationImpl (void);
    // Assignment operator not implemented.
    PageEnumerationImpl& operator= (const PageEnumerationImpl&);
};


class AllPagesPredicate
{
public:
    inline bool operator() (const PageDescriptor& rDescriptor);
};


class SelectedPagesPredicate
{
public:
    inline bool operator() (const PageDescriptor& rDescriptor);
};


class VisiblePagesPredicate
{
public:
    inline bool operator() (const PageDescriptor& rDescriptor);
};


} // end of anonymouse namespace




namespace sd { namespace slidesorter { namespace model {


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

PageEnumeration::~PageEnumeration()
{
}

PageEnumeration& PageEnumeration::operator= (
    const PageEnumeration& rEnumeration)
{
    mpImpl = rEnumeration.mpImpl->Clone();
    return *this;
}




PageEnumeration PageEnumeration::Create (
    const SlideSorterModel& rModel,
    PageEnumerationType eType)
{
    Enumeration<SharedPageDescriptor>* pImpl;
    switch (eType)
    {
        case PET_ALL:
        default:
            pImpl = new PageEnumerationImpl<AllPagesPredicate>(rModel);
            break;

        case PET_SELECTED:
            pImpl = new PageEnumerationImpl<SelectedPagesPredicate>(rModel);
            break;

        case PET_VISIBLE:
            pImpl = new PageEnumerationImpl<VisiblePagesPredicate>(rModel);
            break;
    }

    return PageEnumeration (
        ::std::auto_ptr<Enumeration<SharedPageDescriptor> > (pImpl));
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

template <class Predicate>
PageEnumerationImpl<Predicate>::PageEnumerationImpl (
    const SlideSorterModel& rModel)
    : mrModel (rModel)
{
    Rewind();
}




template <class Predicate>
PageEnumerationImpl<Predicate>::PageEnumerationImpl (
    const SlideSorterModel& rModel,
    int nIndex)
    : mrModel (rModel),
      mnIndex (nIndex)
{}



template <class Predicate>
PageEnumerationImpl<Predicate>::~PageEnumerationImpl (void)
{
}




template <class Predicate>
::std::auto_ptr<Enumeration<SharedPageDescriptor> >
    PageEnumerationImpl<Predicate>::Clone (void)
{
    return ::std::auto_ptr<Enumeration<SharedPageDescriptor> >(
        new PageEnumerationImpl<Predicate>(mrModel,mnIndex));
}




template <class Predicate>
bool PageEnumerationImpl<Predicate>::HasMoreElements (void) const
{
    return (mnIndex < mrModel.GetPageCount());
}




template <class Predicate>
SharedPageDescriptor PageEnumerationImpl<Predicate>::GetNextElement (void)
{
    SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(mnIndex));

    // Go to the following valid element.
    mnIndex += 1;
    AdvanceToNextValidElement();

    return pDescriptor;
}




template <class Predicate>
void PageEnumerationImpl<Predicate>::Rewind (void)
{
    // Go to first valid element.
    mnIndex = 0;
    AdvanceToNextValidElement();
}





template <class Predicate>
void PageEnumerationImpl<Predicate>::AdvanceToNextValidElement (void)
{
    while (mnIndex < mrModel.GetPageCount())
    {
        SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor (mnIndex));
        if (pDescriptor.get()!=NULL && Predicate()(*pDescriptor))
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




// Predicate operators.

bool AllPagesPredicate::operator() (const PageDescriptor& )
{
    return true;
}



bool SelectedPagesPredicate::operator() (const PageDescriptor& rDescriptor)
{
    return rDescriptor.IsSelected();
}




bool VisiblePagesPredicate::operator() (const PageDescriptor& rDescriptor)
{
    return rDescriptor.IsVisible();
}



} // end of anonymous namespace


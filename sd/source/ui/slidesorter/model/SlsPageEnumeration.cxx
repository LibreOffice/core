/*************************************************************************
 *
 *  $RCSfile: SlsPageEnumeration.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:24:49 $
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

using namespace ::sd::slidesorter;
using namespace ::sd::slidesorter::model;

namespace {
template <class Predicate>
class PageEnumerationImpl
    : public Enumeration<PageDescriptor>
{
public:
    inline PageEnumerationImpl (const SlideSorterModel& rModel);
    virtual ~PageEnumerationImpl (void);
    /** Create a copy of the called enumeration object.
    */
    virtual inline Enumeration<PageDescriptor>* Clone (void);

    virtual inline bool HasMoreElements (void) const;
    virtual inline PageDescriptor& GetNextElement (void);
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
    ::std::auto_ptr<Enumeration<PageDescriptor> > pImpl)
    : mpImpl(pImpl)
{
}




PageEnumeration::PageEnumeration (
    PageEnumeration& rEnumeration,
    bool bCloneImpl)
    : mpImpl (bCloneImpl
        ? ::std::auto_ptr<Enumeration<PageDescriptor> >(
            rEnumeration.mpImpl->Clone())
        : rEnumeration.mpImpl)
{
}



PageEnumeration::PageEnumeration (const PageEnumeration& rEnumeration)
    : mpImpl (::std::auto_ptr<Enumeration<PageDescriptor> >(
        rEnumeration.mpImpl->Clone()))
{
}



PageEnumeration& PageEnumeration::operator= (
    const PageEnumeration& rEnumeration)
{
    mpImpl = ::std::auto_ptr<Enumeration<PageDescriptor> >(
        rEnumeration.mpImpl->Clone());
    return *this;
}




PageEnumeration PageEnumeration::Create (
    const SlideSorterModel& rModel,
    PageEnumerationType eType)
{
    Enumeration<PageDescriptor>* pImpl;
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
        ::std::auto_ptr<Enumeration<PageDescriptor> > (pImpl));
}




PageEnumeration* PageEnumeration::Clone (void)
{
    return new PageEnumeration (*this, true);
}




bool PageEnumeration::HasMoreElements (void) const
{
    return mpImpl->HasMoreElements();
}



PageDescriptor& PageEnumeration::GetNextElement (void)
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
Enumeration<PageDescriptor>* PageEnumerationImpl<Predicate>::Clone (void)
{
    return new PageEnumerationImpl<Predicate> (mrModel,mnIndex);
}




template <class Predicate>
bool PageEnumerationImpl<Predicate>::HasMoreElements (void) const
{
    return (mnIndex < mrModel.GetPageCount());
}




template <class Predicate>
PageDescriptor& PageEnumerationImpl<Predicate>::GetNextElement (void)
{
    PageDescriptor& rDescriptor (*mrModel.GetPageDescriptor (mnIndex));

    // Go to the following valid element.
    mnIndex += 1;
    AdvanceToNextValidElement();

    return rDescriptor;
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
        PageDescriptor* pDescriptor (mrModel.GetPageDescriptor (mnIndex));
        if (pDescriptor!= NULL && Predicate()(*pDescriptor))
            // This predicate is valid.
            break;
        else
            // Advance to next predicate.
            mnIndex += 1;
    }
}




// Predicate operators.

bool AllPagesPredicate::operator() (const PageDescriptor& rDescriptor)
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


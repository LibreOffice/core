/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <utility>

#include <boost/noncopyable.hpp>

#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"

using namespace ::sd::slidesorter;
using namespace ::sd::slidesorter::model;

namespace {

class PageEnumerationImpl
    : public Enumeration<SharedPageDescriptor>, private boost::noncopyable
{
public:
    inline PageEnumerationImpl (
        const SlideSorterModel& rModel,
        const PageEnumeration::PagePredicate& rPredicate);
    virtual ~PageEnumerationImpl();
    /** Create a copy of the called enumeration object.
    */
    virtual ::std::unique_ptr<Enumeration<SharedPageDescriptor> > Clone() override;

    virtual bool HasMoreElements() const override;
    virtual SharedPageDescriptor GetNextElement() override;
    virtual void Rewind() override;

private:
    const SlideSorterModel& mrModel;
    const PageEnumeration::PagePredicate maPredicate;
    int mnIndex;

    /** This constructor sets the internal page index to the given value.
        It does not call AdvanceToNextValidElement() to skip elements that
        do not fulfill Predicate.
    */
    inline PageEnumerationImpl (
        const SlideSorterModel& rModel,
        const PageEnumeration::PagePredicate& rPredicate,
        int nIndex);

    /** Skip all elements that do not fulfill Predicate starting with the
        one pointed to by mnIndex.
    */
    inline void AdvanceToNextValidElement();
};

} // end of anonymouse namespace

namespace sd { namespace slidesorter { namespace model {

PageEnumeration PageEnumeration::Create (
    const SlideSorterModel& rModel,
    const PagePredicate& rPredicate)
{
    return PageEnumeration(::std::unique_ptr<Enumeration<SharedPageDescriptor> >(
        new PageEnumerationImpl(rModel, rPredicate)));
}

PageEnumeration::PageEnumeration (
    ::std::unique_ptr<Enumeration<SharedPageDescriptor> > && pImpl)
    : mpImpl(std::move(pImpl))
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
        mpImpl = std::move(rEnumeration.mpImpl);
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

::std::unique_ptr<Enumeration<SharedPageDescriptor> > PageEnumeration::Clone()
{
    return ::std::unique_ptr<Enumeration<SharedPageDescriptor> >(
        new PageEnumeration (*this, true));
}

bool PageEnumeration::HasMoreElements() const
{
    return mpImpl->HasMoreElements();
}

SharedPageDescriptor PageEnumeration::GetNextElement()
{
    return mpImpl->GetNextElement();
}

void PageEnumeration::Rewind()
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

PageEnumerationImpl::~PageEnumerationImpl()
{
}

::std::unique_ptr<Enumeration<SharedPageDescriptor> >
    PageEnumerationImpl::Clone()
{
    return ::std::unique_ptr<Enumeration<SharedPageDescriptor> >(
        new PageEnumerationImpl(mrModel,maPredicate,mnIndex));
}

bool PageEnumerationImpl::HasMoreElements() const
{
    return (mnIndex < mrModel.GetPageCount());
}

SharedPageDescriptor PageEnumerationImpl::GetNextElement()
{
    SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(mnIndex));

    // Go to the following valid element.
    mnIndex += 1;
    AdvanceToNextValidElement();

    return pDescriptor;
}

void PageEnumerationImpl::Rewind()
{
    // Go to first valid element.
    mnIndex = 0;
    AdvanceToNextValidElement();
}

void PageEnumerationImpl::AdvanceToNextValidElement()
{
    while (mnIndex < mrModel.GetPageCount())
    {
        SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(mnIndex));

        // Test for the predicate being fulfilled.
        if (pDescriptor.get()!=nullptr && maPredicate(pDescriptor))
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

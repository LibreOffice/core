/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


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
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    virtual inline ::std::auto_ptr<Enumeration<SharedPageDescriptor> > Clone (void);
    SAL_WNODEPRECATED_DECLARATIONS_POP

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

    
    PageEnumerationImpl& operator= (const PageEnumerationImpl&);
};

} 




namespace sd { namespace slidesorter { namespace model {

SAL_WNODEPRECATED_DECLARATIONS_PUSH
PageEnumeration PageEnumeration::Create (
    const SlideSorterModel& rModel,
    const PagePredicate& rPredicate)
{
    return PageEnumeration(::std::auto_ptr<Enumeration<SharedPageDescriptor> >(
        new PageEnumerationImpl(rModel, rPredicate)));
}
SAL_WNODEPRECATED_DECLARATIONS_POP


SAL_WNODEPRECATED_DECLARATIONS_PUSH
PageEnumeration::PageEnumeration (
    ::std::auto_ptr<Enumeration<SharedPageDescriptor> > pImpl)
    : mpImpl(pImpl)
{
}
SAL_WNODEPRECATED_DECLARATIONS_POP



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



SAL_WNODEPRECATED_DECLARATIONS_PUSH
::std::auto_ptr<Enumeration<SharedPageDescriptor> > PageEnumeration::Clone (void)
{
    return ::std::auto_ptr<Enumeration<SharedPageDescriptor> >(
        new PageEnumeration (*this, true));
}
SAL_WNODEPRECATED_DECLARATIONS_POP



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

} } } 




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



SAL_WNODEPRECATED_DECLARATIONS_PUSH
::std::auto_ptr<Enumeration<SharedPageDescriptor> >
    PageEnumerationImpl::Clone (void)
{
    return ::std::auto_ptr<Enumeration<SharedPageDescriptor> >(
        new PageEnumerationImpl(mrModel,maPredicate,mnIndex));
}
SAL_WNODEPRECATED_DECLARATIONS_POP



bool PageEnumerationImpl::HasMoreElements (void) const
{
    return (mnIndex < mrModel.GetPageCount());
}




SharedPageDescriptor PageEnumerationImpl::GetNextElement (void)
{
    SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(mnIndex));

    
    mnIndex += 1;
    AdvanceToNextValidElement();

    return pDescriptor;
}




void PageEnumerationImpl::Rewind (void)
{
    
    mnIndex = 0;
    AdvanceToNextValidElement();
}





void PageEnumerationImpl::AdvanceToNextValidElement (void)
{
    while (mnIndex < mrModel.GetPageCount())
    {
        SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(mnIndex));

        
        if (pDescriptor.get()!=NULL && maPredicate(pDescriptor))
        {
            
            break;
        }
        else
        {
            
            mnIndex += 1;
        }
    }
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

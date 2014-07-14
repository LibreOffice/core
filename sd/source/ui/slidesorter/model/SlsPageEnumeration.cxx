/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
        do not fulfill Predicate.
    */
    inline PageEnumerationImpl (
        const SlideSorterModel& rModel,
        const PageEnumeration::PagePredicate& rPredicate,
        int nIndex);

    /** Skip all elements that do not fulfill Predicate starting with the
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
    return (mnIndex < (sal_Int32)mrModel.GetPageCount());
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
    while (mnIndex < (sal_Int32)mrModel.GetPageCount())
    {
        SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(mnIndex));

        // Test for the predicate being fulfilled.
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


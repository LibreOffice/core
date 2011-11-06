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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

// This is a definition file of a template class.  It is therefore
// included by other files and thus has to be guarded against multiple
// inclusion.

#ifndef SD_TOOLPANEL_CONSTRAINED_ITERATOR_CXX
#define SD_TOOLPANEL_CONSTRAINED_ITERATOR_CXX

namespace sd { namespace toolpanel {


template <class Container>
ConstrainedIterator<Container>::value_type&
    ConstrainedIterator<Container>::operator* (void)
{
    return *maIterator;
}




template <class Container>
const ConstrainedIterator<Container>::value_type&
    ConstrainedIterator<Container>::operator* (void)
    const
{
    return *maIterator;
}




template <class Container>
ConstrainedIterator<Container>::value_type&
    ConstrainedIterator<Container>::operator-> (void)
{
    return *maIterator;
}




template <class Container>
const ConstrainedIterator<Container>::value_type&
    ConstrainedIterator<Container>::operator-> (void)
    const
{
    return *maIterator;
}





template <class Container>
ConstrainedIterator<Container>
    ::ConstrainedIterator (void)
    : mpContainer (NULL)
{
}




template <class Container>
ConstrainedIterator<Container>::ConstrainedIterator (
    const Container& rContainer,
    const Container::iterator& rIterator)
    : mpContainer(&rContainer),
      maIterator (rIterator),
      mpConstraint (NULL)
{
    AdvanceToNextValidElement();
}




template <class Container>
ConstrainedIterator<Container>::ConstrainedIterator (
    const Container& rContainer,
    const Container::iterator& rIterator,
    const Constraint<Container>& rConstraint)
    : mpContainer(&rContainer),
      maIterator (rIterator),
      mpConstraint (&rConstraint)
{
    AdvanceToNextValidElement();
}




template <class Container>
ConstrainedIterator<Container>::ConstrainedIterator (
    const ConstrainedIterator& rIterator)
    : mpContainer (rIterator.mpContainer),
      maIterator (rIterator.maIterator),
      mpConstraint (rIterator.mpConstraint)
{
    // Everything has been done in the initializer
}




template <class Container>
ConstrainedIterator<Container>&
    ConstrainedIterator<Container>
    ::operator= (const ConstrainedIterator& rIterator)
{
    mpContainer = rIterator.mpContainer;
    maIterator = rIterator.maIterator;
    mpConstraint = rIterator.mpConstraint;

    AdvanceToNextValidElement();

    return *this;
}




template <class Container>
bool ConstrainedIterator<Container>::operator== (
    const ConstrainedIterator& aIterator) const
{
    return ! operator!=(aIterator);
}




template <class Container>
bool ConstrainedIterator<Container>::operator!= (
    const ConstrainedIterator& aIterator) const
{
    return maIterator != aIterator.maIterator;
}




template <class Container>
ConstrainedIterator<Container>&
    ConstrainedIterator<Container>::operator++ (void)
{
    maIterator++;
    AdvanceToNextValidElement();
    return *this;
}




template <class Container>
ConstrainedIterator<Container>
    ConstrainedIterator<Container>::operator++ (int)
{
    ConstrainedIterator aIterator (*this);
    ++(*this);
    return aIterator;
}




template <class Container>
ConstrainedIterator<Container>&
    ConstrainedIterator<Container>::operator-- (void)
{
    maIterator--;
    AdvanceToPreviousValidElement();
    return *this;
}




template <class Container>
ConstrainedIterator<Container>
    ConstrainedIterator<Container>::operator-- (int)
{
    ConstrainedIterator aIterator (*this);
    --(*this);
    return aIterator;
}




template <class Container>
ConstrainedIterator<Container>
    ConstrainedIterator<Container>::operator+ (int nValue) const
{
    return ConstrainedIterator (*mpContainer, maIterator+nValue);
}



template <class Container>
ConstrainedIterator<Container>
    ConstrainedIterator<Container>::operator- (int nValue) const
{
    return ConstrainedIterator (*mpContainer, maIterator-nValue);
}



template <class Container>
void ConstrainedIterator<Container>::AdvanceToNextValidElement (void)
{
    if (mpContainer!=NULL && mpConstraint!=NULL)
    {
        while (maIterator != mpContainer->end()
            && ! mpConstraint->operator()(*mpContainer, maIterator))
            ++maIterator;
    }
}




template <class Container>
void ConstrainedIterator<Container>::AdvanceToPreviousValidElement (void)
{
    if (mpContainer!=NULL && mpConstraint!=NULL)
    {
        while (maIterator != mpContainer->begin()
            && ! mpConstraint->operator()(*mpContainer, maIterator))
            --maIterator;
    }
}


} } // end of namespace ::sd::toolpanel

#endif

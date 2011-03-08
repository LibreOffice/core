/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConstrainedIterator.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 19:12:04 $
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

/*************************************************************************
 *
 *  $RCSfile: ConstrainedIterator.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:30:04 $
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

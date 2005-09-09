/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConstrainedIterator.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 06:29:10 $
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

#ifndef SD_TOOLPANEL_CONSTRAINED_ITERATOR_HXX
#define SD_TOOLPANEL_CONSTRAINED_ITERATOR_HXX

#include <iterator>

namespace sd { namespace toolpanel {


template <class Container>
class Constraint
{
public:
    virtual bool operator() (
        const Container& rContainer,
        const Container::iterator& rIterator) const = 0;
};




/** This iterator is a bidirectional iterator with something of random
    access thrown in.  It uses a constraint object to jump over
    elements in the underlying container that do not meet the
    constraint.
*/
template <class Container>
class ConstrainedIterator
    : public ::std::bidirectional_iterator_tag
{
public:
    typedef Container::value_type value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;

    ConstrainedIterator (void);
    ConstrainedIterator (
        const Container& rContainer,
        const Container::iterator& rIterator);
    ConstrainedIterator (
        const Container& rContainer,
        const Container::iterator& rIterator,
        const Constraint<Container>& pConstraint);
    ConstrainedIterator (
        const ConstrainedIterator& rIterator);

    ConstrainedIterator& operator= (
        const ConstrainedIterator& aIterator);

    reference operator* (void);
    const_reference operator* (void) const;
    reference operator-> (void);
    const_reference operator-> (void) const;

    bool operator== (const ConstrainedIterator& aIterator) const;
    bool operator!= (const ConstrainedIterator& aIterator) const;

    ConstrainedIterator& operator++ (void);
    ConstrainedIterator operator++ (int);
    ConstrainedIterator& operator-- (void);
    ConstrainedIterator operator-- (int);

    ConstrainedIterator operator+ (int nValue) const;
    ConstrainedIterator operator- (int nValue) const;


private:
    const Container* mpContainer;
    Container::iterator maIterator;
    const Constraint<Container>* mpConstraint;

    void AdvanceToNextValidElement (void);
    void AdvanceToPreviousValidElement (void);
};

} } // end of namespace ::sd::toolpanel

#endif

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

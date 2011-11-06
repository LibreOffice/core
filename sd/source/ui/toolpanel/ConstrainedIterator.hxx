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

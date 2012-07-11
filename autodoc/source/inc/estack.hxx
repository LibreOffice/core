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

#ifndef ARY_ESTACK_HXX
#define ARY_ESTACK_HXX



// USED SERVICES
    // BASE CLASSES
#include <list>
    // COMPONENTS
    // PARAMETERS



template <class ELEM>
class EStack : private std::list<ELEM>
{
  private:
    typedef std::list<ELEM>    base;
    const base &        Base() const            { return *this; }
    base &              Base()                  { return *this; }

  public:
    typedef ELEM                                    value_type;
    typedef typename std::list<ELEM>::size_type    size_type;

    // LIFECYCLE
                        EStack()                {}
                        EStack(
                            const EStack &      i_rStack )
                                                :   base( (const base &)(i_rStack) ) {}
                        ~EStack()               {}
    // OPERATORS
    EStack &            operator=(
                            const EStack &      i_rStack )
                                                { base::operator=( i_rStack.Base() );
                                                  return *this; }
    bool                operator==(
                            const EStack<ELEM> & ) const
                                                { return std::operator==( Base(), this->i_rStack.Base() ); }
    bool                operator<(
                            const EStack<ELEM> & ) const
                                                { return std::operator<( Base(), this->i_rStack.Base() ); }
    // OPERATIONS
    void                push(
                            const value_type &  i_rElem )
                                                { base::push_front(i_rElem); }
    void                pop()                   { base::pop_front(); }
    void                erase_all()             { while (NOT empty()) pop(); }

    // INQUIRY
    const value_type &  top() const             { return base::front(); }
    size_type           size() const            { return base::size(); }
    bool                empty() const           { return base::empty(); }

    // ACCESS
    value_type &        top()                   { return base::front(); }
};



// IMPLEMENTATION


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

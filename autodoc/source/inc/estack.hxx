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

#ifndef ARY_ESTACK_HXX
#define ARY_ESTACK_HXX



// USED SERVICES
    // BASE CLASSES
#include <slist>
    // COMPONENTS
    // PARAMETERS



template <class ELEM>
class EStack : private std::slist<ELEM>
{
  private:
    typedef std::slist<ELEM>    base;
    const base &        Base() const            { return *this; }
    base &              Base()                  { return *this; }

  public:
    typedef ELEM                                    value_type;
    typedef typename std::slist<ELEM>::size_type    size_type;

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
                            const EStack<ELEM> &
                                                i_r2 ) const
                                                { return std::operator==( Base(), this->i_rStack.Base() ); }
    bool                operator<(
                            const EStack<ELEM> &
                                                i_r2 ) const
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
